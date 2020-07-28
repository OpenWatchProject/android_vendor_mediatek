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

package com.mediatek.op06.settings;

import android.app.QueuedWork;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.IBinder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.PreferenceManager;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.ims.config.ImsConfigContract;
import com.mediatek.ims.internal.MtkImsConfig;
import com.mediatek.wfo.IWifiOffloadService;
import com.mediatek.wfo.WifiOffloadManager;


/** Service to set WFC settings in Settings provider  during provisioning & de-provisioning.
 * Invoked by WifiCallingReceiver.
 */
public class WifiCallingReceiverService extends Service {
    private static final String TAG = "OP06WifiCallingReceiverService";
    private static final String IMS_ENABLE_DEFAULT_VALUE = "ims_enable_default_value";
    private static final String IMS_MODE_DEFAULT_VALUE = "ims_mode_default_value";
    private static final String LAST_IMS_PROVISIONED_VALUE = "last_provisioned_value";

    private static final String OMACP_SETTING_ACTION = "com.mediatek.omacp.settings";
    private static final String OMACP_SETTING_ACTION_RESULT = "com.mediatek.omacp.settings.result";
    private static final String EXTRA_APP_ID = "appId";
    private static final String EXTRA_RESULT = "result";
    private static final String APP_ID = "ims";

    private ServiceHandler mServiceHandler;
    private Looper mServiceLooper;

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "WifiCallingReceiverService onCreate");
        // Start up the thread running the service. Note that we create a
        // separate thread because the service normally runs in the process's
        // main thread, which we don't want to block receiver's thread.
        HandlerThread thread = new HandlerThread("OP06WificallingReceiverService",
                Process.THREAD_PRIORITY_BACKGROUND);
        thread.start();
        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "WifiCallingReceiverService onStartCommand");
        Message msg = mServiceHandler.obtainMessage();
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);
        return Service.START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "WifiCallingReceiverService onDestroy");
        mServiceLooper.quit();
    }

    /** handler class to set WFC Settings in settings Provider
     * during provisioning & de-provisioning.
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
                if (OMACP_SETTING_ACTION.equals(action)) {
                    handleOmacpSettings(intent);
                } else if (ImsConfigContract.ACTION_IMS_CONFIG_CHANGED.equals(action)
                        || ImsConfigContract.ACTION_CONFIG_LOADED.equals(action)) {
                    Context context = WifiCallingReceiverService.this;
                    SharedPreferences sp = PreferenceManager
                            .getDefaultSharedPreferences(context);
                    Editor ed = sp.edit();
                    int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                            SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                    int phoneId = intent.getIntExtra(ImsConfigContract.EXTRA_PHONE_ID, 0);
                    boolean isWfcProvisioned = WifiCallingUtils
                            .isWifiCallingProvisioned(context, phoneId);
                    boolean defaultImsEnabled = sp.getBoolean(IMS_ENABLE_DEFAULT_VALUE, false);
                    int defaultWfcMode = sp.getInt(IMS_MODE_DEFAULT_VALUE,
                            ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED);
                    boolean lastProvisionedValue = sp.getBoolean(LAST_IMS_PROVISIONED_VALUE, false);
                    ed.putBoolean(LAST_IMS_PROVISIONED_VALUE, isWfcProvisioned);
                    ed.commit();

                        boolean imsEnabled;
                    int mode;
                    if (isWfcProvisioned) {
                        if (lastProvisionedValue) {
                            imsEnabled = ImsManager.isWfcEnabledByUser(context);
                            mode = ImsManager.getWfcMode(context);
                        } else {
                            imsEnabled = defaultImsEnabled;
                            mode = defaultWfcMode;
                        }
                    } else {
                        imsEnabled = defaultImsEnabled;
                        mode = defaultWfcMode;
                    }
                    String fqdn = WifiCallingUtils.getFQDN(context, phoneId);
                    Log.d(TAG, "lastProvisionedValue:" + lastProvisionedValue);
                    Log.d(TAG, "imsEnable:" + imsEnabled + ", wfcMode:" + mode + ", FQDN:" + fqdn);
                    sendEpdgParams(subId, imsEnabled, fqdn);
                    ImsManager.setWfcSetting(context, imsEnabled);
                    ImsManager.setWfcMode(context, mode);
                } else if (CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED.equals(action)) {
                    int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                            SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                    int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                    if (!WifiCallingUtils
                            .isImsProvSupported(WifiCallingReceiverService.this, subId)) {
                        Log.d(TAG, "Reset ims Config");
                        final ImsConfig imsConfig
                                = getImsConfig(SubscriptionManager.getPhoneId(subId));
                        if (imsConfig != null) {
                            QueuedWork.queue(new Runnable() {
                                public void run() {
                                    try {
                                        imsConfig.setProvisionedValue(ImsConfig.ConfigConstants
                                                               .VOICE_OVER_WIFI_SETTING_ENABLED, 0);
                                        imsConfig.setProvisionedStringValue(
                                                MtkImsConfig.ConfigConstants.EPDG_ADDRESS, "");
                                    } catch (ImsException e) {
                                        Log.e(TAG, "WFC_SETTING_ENABLED provision exception:");
                                        e.printStackTrace();
                                    }
                                }
                            }, false);
                        }
                    }
                }
            }
            WifiCallingReceiverService.this.stopSelf();
        }
    }

    private void handleOmacpSettings(Intent intent) {
        String epdgFqdn = intent.getStringExtra("epdg-fqdn");
        int subId = intent.getIntExtra("subId", -1);
        Log.d(TAG, "fqdn:" + epdgFqdn + ", subId:" + subId);
        if (WifiCallingUtils.isImsProvSupported(this, subId)
                && (epdgFqdn.equals("") || WifiCallingUtils.isValidFqdn(epdgFqdn))) {
            configureImsParams(intent);
        } else {
            sendOmacpSettingResult(false);
        }
    }

    private void configureImsParams(Intent intent) {
        boolean result = true;
        int subId = intent.getIntExtra("subId", -1);
        final String epdgFqdn = intent.getStringExtra("epdg-fqdn");
        //Log.d(TAG, "SubId:" + subId);
        //Log.d(TAG, "fqdn:" + epdgFqdn);
        final ImsConfig imsConfig = getImsConfig(SubscriptionManager.getPhoneId(subId));
        if (imsConfig == null) {
            sendOmacpSettingResult(false);
            Log.e(TAG, "ImsConfig service unavailable");
        } else {
            final boolean newValue = epdgFqdn.equals("") ? false : true;
            Log.d(TAG, "newValue: " + newValue);

            QueuedWork.queue(new Runnable() {
                public void run() {
                    try {
                        imsConfig.setProvisionedValue(
                           ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED,
                                newValue ? 1 : 0);
                        imsConfig.setProvisionedStringValue(
                                MtkImsConfig.ConfigConstants.EPDG_ADDRESS, epdgFqdn);
                        sendOmacpSettingResult(true);
                    } catch (ImsException e) {
                        Log.e(TAG, "WFC_SETTING_ENABLED provision exception:");
                        e.printStackTrace();
                        sendOmacpSettingResult(false);
                    }
                }
            }, false);
        }
    }

    private ImsConfig getImsConfig(int phoneId) {
        ImsConfig imsConfig = null;
        ImsManager imsManager = ImsManager.getInstance(this, phoneId);
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

    private void sendOmacpSettingResult(boolean result) {
        Log.d(TAG, "sendOmacpSettingResult: " + result);
        Intent it = new Intent();
        it.setAction(OMACP_SETTING_ACTION_RESULT);
        it.putExtra(EXTRA_APP_ID, APP_ID);
        it.putExtra(EXTRA_RESULT, result);

        WifiCallingReceiverService.this.sendBroadcast(it);
    }

    private void sendEpdgParams(int subId, boolean imsEnabled, String fqdn) {
        IBinder b = ServiceManager.getService(WifiOffloadManager.WFO_SERVICE);
        IWifiOffloadService wfoService = IWifiOffloadService.Stub.asInterface(b);
        if (wfoService == null) {
            Log.e(TAG, "Epdg params failed to send,wfo service unavailable");
        } else {
            try {
                wfoService.setEpdgFqdn(subId, fqdn, imsEnabled);
            } catch (RemoteException e) {
                Log.e(TAG, "WFO exception:");
                e.printStackTrace();
            }
        }
    }
}

