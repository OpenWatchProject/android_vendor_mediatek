/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.imsplugin;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.ims.ImsCallProfile;
import android.telephony.CarrierConfigManager;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;

import com.mediatek.ims.ImsCallSessionProxy;
import com.mediatek.ims.ImsService;
import com.mediatek.ims.ril.ImsCommandsInterface;

import com.mediatek.ims.plugin.impl.ImsSelfActivatorBase;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.selfactivation.ISelfActivation;
import com.mediatek.internal.telephony.MtkSubscriptionManager;

import java.util.List;
import java.lang.Integer;
import java.lang.Thread;

// for dynamic load class and method
import dalvik.system.PathClassLoader;
import java.lang.reflect.Method;

public class ImsSelfActivatorImpl extends ImsSelfActivatorBase {

    private static final String TAG = "ImsExtSelfActivatorImpl";

    private int mPhoneId;
    private Context mContext;
    private ImsCommandsInterface mImsRILAdapter;
    private ImsCallProfile mCallProfile;
    private String mDialString;
    private ImsCallSessionProxy mImsCallSessionProxy;
    private Handler mHandler;
    private ImsService mImsService;
    private boolean mIsDialed;

    private static final boolean MTK_VZW_SUPPORT
            = "OP12".equals(SystemProperties.get("persist.vendor.operator.optr", "OM"));

    private static final int EVENT_IMS_REGISTRATION_INFO = 1;
    private static final int EVENT_IMS_REGISTRATION_TIMEOUT = 2;

    private static final int IMS_REG_TIMEOUT = 10000; // 10 sec

    protected static final String ACTION_AIRPLANE_CHANGE_DONE =
            "com.mediatek.intent.action.AIRPLANE_CHANGE_DONE";
    protected static final String EXTRA_AIRPLANE_MODE = "airplaneMode";

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            if (ACTION_AIRPLANE_CHANGE_DONE.equals(intent.getAction())){
                boolean isFlightmode = intent.getBooleanExtra(EXTRA_AIRPLANE_MODE, false);
                Rlog.d(TAG, "ACTION_AIRPLANE_CHANGE_DONE: " + isFlightmode);
                if (isFlightmode == true) {
                    doSelfActivationDial(mDialString, mCallProfile, false);
                    mContext.unregisterReceiver(mBroadcastReceiver);
                }
            }
        }
    };

    public ImsSelfActivatorImpl(
        Context context, Handler handler, ImsCallSessionProxy callSessionProxy,
        ImsCommandsInterface imsRILAdapter, ImsService imsService, int phoneId) {

        super(context, handler, callSessionProxy, imsRILAdapter, imsService, phoneId);

        Rlog.d(TAG, "Construct ImsExtSelfActivatorImpl()");

        mContext = context;
        mImsCallSessionProxy = callSessionProxy;
        mImsRILAdapter = imsRILAdapter;
        mHandler = new MyHandler(handler.getLooper());
        mPhoneId = phoneId;
        mImsService = imsService;
    }

    private class MyHandler extends Handler {

        public MyHandler(Looper looper) {
            super(looper, null, true);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_IMS_REGISTRATION_INFO:
                    AsyncResult ar = (AsyncResult) msg.obj;
                    int socketId = ((int[]) ar.result)[2];
                    Rlog.d(TAG, "handle EVENT_IMS_REGISTRATION_INFO");

                    if (socketId != mPhoneId) {
                        Rlog.d(TAG, "Socket id and phone id not matched");
                        return;
                    }

                    if (((int[]) ar.result)[0] == 1) {
                        // In Service
                        dialAnyway();
                        mHandler.removeMessages(EVENT_IMS_REGISTRATION_TIMEOUT);
                    }
                    break;
                case EVENT_IMS_REGISTRATION_TIMEOUT:
                    dialAnyway();
                    break;
                default:
                    break;
            }
        }
    }

    public void doSelfActivationDial(String dialString, ImsCallProfile callProfile, boolean isEcc) {
        Rlog.d(TAG, "doSelfActivationDial()");
        mDialString = dialString;
        mCallProfile = callProfile;
        mIsDialed = false;

        if (tryLeaveFlightmodeBeforeDial()) {

            return;
        }

        mImsRILAdapter.registerForImsRegistrationInfo(mHandler, EVENT_IMS_REGISTRATION_INFO, null);
        mHandler.sendEmptyMessageDelayed(EVENT_IMS_REGISTRATION_TIMEOUT, IMS_REG_TIMEOUT);
        notifySelfActivationSMBeforeDial(isEcc);
    }

    private void notifySelfActivationSMBeforeDial(boolean isEcc) {
        IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));
        if (iTelEx == null) {
            Rlog.d(TAG, "Can not access ITelephonyEx");
            return;
        }

        Bundle extra = new Bundle();
        int callType = isEcc? ISelfActivation.CALL_TYPE_EMERGENCY: ISelfActivation.CALL_TYPE_NORMAL;
        extra.putInt(ISelfActivation.EXTRA_KEY_MO_CALL_TYPE, callType);
        Rlog.d(TAG, "notifySelfActivationSMBeforeDial");

        try {
            ImsService imsService = ImsService.getInstance(mContext);
            int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(mPhoneId);
            iTelEx.selfActivationAction(ISelfActivation.ACTION_MO_CALL, extra, subId);
        } catch (RemoteException ex) {
            Rlog.e(TAG, "RemoteException doSelfActivationDial()");
        }
    }

    public void close() {
        Rlog.d(TAG, "close()");
        mImsRILAdapter.unregisterForImsRegistrationInfo(mHandler);
        mHandler.removeMessages(EVENT_IMS_REGISTRATION_TIMEOUT);
        mImsCallSessionProxy = null;
        mCallProfile = null;
        mImsRILAdapter = null;
        mImsService = null;
    }

    public boolean shouldProcessSelfActivation(int phoneId) {
        if (mImsService.getImsServiceState(phoneId)  == ServiceState.STATE_IN_SERVICE) {
            Rlog.d(TAG, "shouldProcessSelfActivation() IMS is IN_SERVICE, return false");
            return false;
        }

        IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));
        if (iTelEx == null) {
            Rlog.d(TAG, "Can not access ITelephonyEx");
            return false;
        }

        int selfActivateState;
        try {
            ImsService imsService = ImsService.getInstance(mContext);
            int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(phoneId);
            selfActivateState = iTelEx.getSelfActivateState(subId);
        } catch (RemoteException ex) {
            Rlog.e(TAG, "RemoteException shouldProcessSelfActivation()");
            return false;
        }
        Rlog.d(TAG, "Self Activate State: " + selfActivateState);
        return (selfActivateState == ISelfActivation.STATE_NOT_ACTIVATED);
    }

    private void dialAnyway() {
        if (mIsDialed) {
            return;
        }
        mImsCallSessionProxy.start(mDialString, mCallProfile);
        mIsDialed = true;
    }

    private boolean tryLeaveFlightmodeBeforeDial() {
        boolean isInFlightmode = Settings.Global.getInt(
                mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0) > 0;
        Rlog.d(TAG, "tryLeaveFlightmodeBeforeDial() isInFlightmode: " + isInFlightmode);

        if (!isInFlightmode) return false;

        Settings.Global.putInt(mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0);

        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intent.putExtra("state", false);
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL);

        final IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_AIRPLANE_CHANGE_DONE);
        mContext.registerReceiver(mBroadcastReceiver, filter);
        return true;
    }
}
