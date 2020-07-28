/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.ims;

import android.content.Context;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.SystemProperties;

import android.telephony.Rlog;
import android.telephony.ims.stub.ImsEcbmImplBase;

import com.android.ims.ImsManager;

import com.mediatek.ims.common.SubscriptionManagerHelper;
import com.mediatek.ims.config.internal.ImsConfigUtils;

import com.mediatek.ims.ril.ImsCommandsInterface;
import com.mediatek.ims.ImsService;

public class ImsEcbmProxy extends ImsEcbmImplBase {
    private static final String LOG_TAG = "ImsEcbmProxy";

    private static final boolean MTK_VZW_SUPPORT
            = "OP12".equals(SystemProperties.get("persist.vendor.operator.optr", "OM"));

    protected static final int EVENT_ON_ENTER_ECBM = 1;
    protected static final int EVENT_ON_EXIT_ECBM = 2;
    protected static final int EVENT_ON_NO_ECBM = 3;

    private ImsCommandsInterface mImsRILAdapter;
    private ImsServiceCallTracker mImsServiceCT;

    private Context mContext;
    private int mPhoneId;
    private boolean mHandleExitEcbmInd;

    private final Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_ON_ENTER_ECBM:
                    ImsEcbmProxy.this.enteredEcbm();
                    break;
                case EVENT_ON_EXIT_ECBM:
                    if (mHandleExitEcbmInd == false) return;
                    mHandleExitEcbmInd = false;
                    ImsEcbmProxy.this.exitedEcbm();
                    ImsEcbmProxy.this.tryTurnOffVolteAfterE911();
                    break;
                case EVENT_ON_NO_ECBM:
                    ImsEcbmProxy.this.tryTurnOffVolteAfterE911();
                default:
                    break;
            }
        }
    };

    ImsEcbmProxy(Context context, ImsCommandsInterface adapter, int phoneId) {
        logWithPhoneId("new EcbmProxy");
        mContext = context;
        mImsRILAdapter = adapter;
        mPhoneId = phoneId;
        mImsServiceCT = ImsServiceCallTracker.getInstance(mPhoneId);

        if (mImsRILAdapter == null) {
            return;
        }

        mImsRILAdapter.registerForOnEnterECBM(mHandler, EVENT_ON_ENTER_ECBM, null);
        mImsRILAdapter.registerForOnExitECBM(mHandler, EVENT_ON_EXIT_ECBM, null);
        mImsRILAdapter.registerForOnNoECBM(mHandler, EVENT_ON_NO_ECBM, null);
    }

    /**
     * Requests Modem to come out of ECBM mode.
     */
    @Override
    public void exitEmergencyCallbackMode() {
        if (mImsRILAdapter != null) {
           logWithPhoneId("request exit ECBM");
           mHandleExitEcbmInd = true;
           mImsRILAdapter.requestExitEmergencyCallbackMode(null);
        } else {
           logWithPhoneId("request exit ECBM failed");
        }
    }

    /// M: E911 During VoLTE off @{
    private void tryTurnOffVolteAfterE911() {
        ImsManager imsManager = ImsManager.getInstance(mContext, mPhoneId);
        boolean volteEnabledByPlatform = imsManager.isVolteEnabledByPlatform();
        boolean volteEnabledByUser = imsManager.isEnhanced4gLteModeSettingEnabledByUser();
        if (mImsServiceCT.getEnableVolteForImsEcc()
            && (!volteEnabledByPlatform || !volteEnabledByUser)) {
            ImsConfigUtils.triggerSendCfgForVolte(mContext, mImsRILAdapter, mPhoneId, 0);
            mImsServiceCT.setEnableVolteForImsEcc(false);
        }
    }
    /// @}

    private void logWithPhoneId(String msg) {

        Rlog.d(LOG_TAG, "[PhoneId = " + mPhoneId + "] " + msg);
    }
}
