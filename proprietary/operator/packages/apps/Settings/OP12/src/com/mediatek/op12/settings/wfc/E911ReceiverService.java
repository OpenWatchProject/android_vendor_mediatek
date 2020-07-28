/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.op12.settings.wfc;

import android.app.QueuedWork;
import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;



/** Service to reset wfc provisioning & mdn in system property.
 * Invoked by E911Receiver.
 */
public class E911ReceiverService extends Service {
    private static final String TAG = "OP12E911ReceiverService";

    private static final String PROPERTY_VOWIFI_MDN = "persist.vendor.sys.vowifiMdn";

    private ServiceHandler mServiceHandler;
    private Looper mServiceLooper;

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        // Start up the thread running the service. Note that we create a
        // separate thread because the service normally runs in the process's
        // main thread, which we don't want to block receiver's thread.
        HandlerThread thread = new HandlerThread("OP12E911ReceiverService",
                Process.THREAD_PRIORITY_BACKGROUND);
        thread.start();
        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand");
        Message msg = mServiceHandler.obtainMessage();
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);
        return Service.START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        mServiceLooper.quit();
    }

    /** Service handler class.
     */
    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Intent intent = (Intent) msg.obj;
            if (intent != null) {
                String action = intent.getAction();
                Log.d(TAG, "action:" + action);
                if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)) {
                    handleSimStateChange(intent);
                }
            }
            E911ReceiverService.this.stopSelf();
        }
    }

    private void handleSimStateChange(Intent intent) {
        String simState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
        Log.d(TAG, "simState:" + simState);
        if (simState.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
            TelephonyManager telephonyManager = TelephonyManager.from(this);
            String simMdn = telephonyManager.getCdmaMdn(SubscriptionManager
                    .getDefaultVoiceSubscriptionId());
            String cachedMdn = SystemProperties.get(PROPERTY_VOWIFI_MDN);
            Log.d(TAG, "simMdn:" + simMdn);
            Log.d(TAG, "cachedMdn:" + cachedMdn);
            if (cachedMdn != null) {
                if (!cachedMdn.equals(simMdn)) {
                    // reset mdn in system property & reset prov value of WFC
                    resetMdn();
                }
            }
        }
    }

    private void resetMdn() {
        //SystemProperties.set(PROPERTY_VOWIFI_MDN, "0");
        final ImsConfig imsConfig = getImsConfig();
        if (imsConfig != null) {
            QueuedWork.queue(new Runnable() {
                public void run() {
                    try {
                        imsConfig.setProvisionedValue(
                            ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED, 0);
                    } catch (ImsException e) {
                        Log.e(TAG, " VOICE_OVER_WIFI_SETTING_ENABLED provision exception:");
                        e.printStackTrace();
                    }
                }
            }, false);
        }
    }

    private ImsConfig getImsConfig() {
        ImsConfig imsConfig = null;
        ImsManager imsManager = ImsManager.getInstance(this, SubscriptionManager
                .getDefaultVoicePhoneId());
        if (imsManager != null) {
            try {
                imsConfig = imsManager.getConfigInterface();
            } catch (ImsException e) {
                e.printStackTrace();
            }
        } else {
           Log.e(TAG, "ImsManager null");
        }
        return imsConfig;
    }

}
