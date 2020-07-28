/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

package com.mediatek.wfo.impl;

import java.lang.reflect.Field;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.net.Uri;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.net.wifi.ScanResult;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.RemoteCallbackList;
import android.os.storage.StorageManager;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

import android.text.TextUtils;


import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.ims.MtkImsConstants;
import com.mediatek.wfo.DisconnectCause;
import com.mediatek.wfo.WifiOffloadManager;
import com.mediatek.wfo.IWifiOffloadService;
import com.mediatek.wfo.IWifiOffloadListener;

import com.mediatek.wfo.op.OpWosCustomizationUtils;
import com.mediatek.wfo.op.OpWosCustomizationFactoryBase;
import com.mediatek.wfo.op.IWosExt;
import com.mediatek.wfo.ril.MwiRIL;

import java.util.List;

/**
 * Top-level Application class for the WifiOffload app.
 */
public class WfcHandler extends Handler {
    private static final String TAG = "WfcHandler";
    private static final int MAX_VALID_SIM_COUNT = 4;

    private static final String AID_SETTING_URI_STR = "wfc_aid_value";
    private static final Uri AID_SETTING_URI = Settings.Global.getUriFor(AID_SETTING_URI_STR);

    private final static String RADIO_MANAGER_POWER_ON_MODEM =
        "mediatek.intent.action.WFC_POWER_ON_MODEM";
    private final static String EXTRA_POWER_ON_MODEM_KEY = "mediatek:POWER_ON_MODEM";

    private static final String ACTION_OPERATOR_CONFIG_CHANGED =
        "com.mediatek.common.carrierexpress.operator_config_changed";
    private static final String WFC_STATUS_CHANGED =
        "com.mediatek.intent.action.WFC_STATUS_CHANGED";

    private static final String IMS_REG_ERROR_NOTIFICATION =
        "com.android.imsconnection.DISCONNECTED";

    private static final String IMS_REG_ERROR_NOTIFICATION_PERMISSION =
        "com.mediatek.permission.IMS_ERR_NOTIFICATION";

    private static final int INVALID = -1;
    private static final int REGISTERED = 1;
    private static final int NOT_REGISTERED = 2;
    private static final int REGISTERING = 3;

    private static final String INTENT_KEY_PROP_KEY = "simPropKey";

    private static WfcHandler mInstance = null;

    private static final String MTK_KEY_DISABLE_WFC_AFTER_AUTH_FAIL =
        "mtk_carrier_disable_wfc_after_auth_fail_bool";

    private Context mContext;
    private int mSimCount;
    private boolean mIsWifiEnabled;
    private boolean mIsWifiL2Connected = false;
    private boolean[] mIsWfcSettingsOn;
    private int mPartialScanCount;
    private boolean mIsWifiConnected;
    private boolean mHasWiFiDisabledPending = false;

    private WifiPdnHandler mWifiPdnHandler;
    private DisconnectCause mDisconnectCause[];
    private String mWfcEccAid;
    private MwiRIL[] mMwiRil;

    private int[] mWfcState;

    // Notify ePDG screen state
    private int mScreenState;
    enum ScreenState {
        USER_PRESENT,
        SCREEN_OFF,
        SCREEN_ON;
    }

    IWosExt mWosExt = null;

    private RemoteCallbackList<IWifiOffloadListener> mListeners =
        new RemoteCallbackList<IWifiOffloadListener>();

    // For OOS URC to check is IMS APN or not
    private ConnectivityManager mConnectivityManager;

    // Message codes. See mHandler below.
    private static final int BASE = 2000;
    private static final int EVENT_HANDLE_MODEM_POWER               = BASE + 0;
    private static final int EVENT_SET_WFC_EMERGENCY_ADDRESS_ID     = BASE + 1;
    private static final int EVENT_NOTIFY_WIFI_NO_INTERNET          = BASE + 2;
    private static final int EVENT_HANDLE_WIFI_STATE_CHANGE         = BASE + 3;
    // Notify ePDG screen state
    private static final int EVENT_NOTIFY_EPDG_SCREEN_STATE         = BASE + 4;
    private static final int EVENT_INITIALIZE                       = BASE + 5;

    public static final int EVENT_ON_PDN_HANDOVER                   = BASE + 100;
    public static final int EVENT_ON_PDN_ERROR                      = BASE + 101;
    public static final int EVENT_ON_ROVE_OUT                       = BASE + 102;
    public static final int EVENT_ON_WFC_PDN_STATE_CHANGED          = BASE + 103;
    public static final int EVENT_ON_WIFI_PDN_OOS                   = BASE + 104;
    public static final int EVENT_HANDLE_WFC_STATE_CHANGED          = BASE + 105;

    private static final int RESPONSE_SET_WFC_EMERGENCY_ADDRESS_ID  = BASE + 200;
    private static final int EVENT_ON_LOCATION_TIMEOUT              = BASE + 201;
    private static final int EVENT_ON_ALLOW_WIFI_OFF                = BASE + 202;
    // Notify ePDG screen state
    private static final int RESPONSE_NOTIFY_EPDG_SCREEN_STATE      = BASE + 203;

    private static int WIFI_NO_INTERNET_ERROR_CODE = 1081;
    private static int WIFI_NO_INTERNET_TIMEOUT = 8000;
    private static final int RETRY_TIMEOUT = 3000;

    private static int CODE_WFC_EPDG_IPSEC_SETUP_ERROR = 1082;
    private static int SUB_CAUSE_IKEV2_24 = 24;

    // log task
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean USR_BUILD = TextUtils.equals(Build.TYPE, "user")
            || TextUtils.equals(Build.TYPE, "userdebug");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    // for Wos -> Wi-Fi fwk to do partial scan to switch to another wifi ap
    private static final String WFC_REQUEST_PARTIAL_SCAN =
        "com.mediatek.intent.action.WFC_REQUEST_PARTIAL_SCAN";

    @Override
    public void handleMessage(Message msg) {

        log("handleMessage: " + messageToString(msg));
        AsyncResult ar;
        switch (msg.what) {
            /* handle internal event start */
            case EVENT_HANDLE_MODEM_POWER:
                handleModemPower();
                break;
            case EVENT_SET_WFC_EMERGENCY_ADDRESS_ID:
                setEmergencyAddressId();
                break;
            case EVENT_NOTIFY_WIFI_NO_INTERNET:
                checkIfShowNoInternetError(true);
                break;
                // Notify ePDG screen state
            case EVENT_NOTIFY_EPDG_SCREEN_STATE:
                notifyEPDGScreenState(mScreenState);
                break;
            /* handle internal event end */
            /* handle UNSOL start */
            case EVENT_ON_PDN_HANDOVER:
                onPdnHandover(msg);
                break;
            case EVENT_ON_PDN_ERROR:
                onWfcPdnError(msg);
                break;
            case EVENT_ON_ROVE_OUT:
                onWifiRoveout(msg);
                break;
            case EVENT_ON_WFC_PDN_STATE_CHANGED:
                onWfcPdnStateChanged(msg);
                break;
            case EVENT_ON_WIFI_PDN_OOS:
                onWifiPdnOOS(msg);
                break;
            /* handle UNSOL end */
            case RESPONSE_SET_WFC_EMERGENCY_ADDRESS_ID:
                ar = (AsyncResult) msg.obj;
                handleRetry(EVENT_SET_WFC_EMERGENCY_ADDRESS_ID, ar);
                break;
            case EVENT_ON_LOCATION_TIMEOUT:
                notifyLocationTimeout();
                break;
            case EVENT_HANDLE_WIFI_STATE_CHANGE:
                updateWifiEnabled();
                break;
            case EVENT_ON_ALLOW_WIFI_OFF:
                notifyOnAllowWifiOff();
                break;
             // Notify ePDG screen state
            case RESPONSE_NOTIFY_EPDG_SCREEN_STATE:
                handleRetry(EVENT_NOTIFY_EPDG_SCREEN_STATE, null);
                break;
            case EVENT_HANDLE_WFC_STATE_CHANGED:
                onWfcStatusChanged();
                break;
            case EVENT_INITIALIZE: {
                initialize();
                break;
            }
            default:
                break;
        }
    }

    private void handleRetry(int msgId, AsyncResult result) {
        if (hasMessages(msgId)) {
            return;
        }

        if (result != null && result.exception != null) {
            sendEmptyMessageDelayed(msgId, RETRY_TIMEOUT);
        }
    }

    private String messageToString(Message msg) {
        switch (msg.what) {

            case EVENT_HANDLE_MODEM_POWER:
                return "EVENT_HANDLE_MODEM_POWER";
            case EVENT_SET_WFC_EMERGENCY_ADDRESS_ID:
                return "EVENT_SET_WFC_EMERGENCY_ADDRESS_ID";
            case EVENT_NOTIFY_EPDG_SCREEN_STATE:
                return "EVENT_NOTIFY_EPDG_SCREEN_STATE";
            case EVENT_NOTIFY_WIFI_NO_INTERNET:
                return "EVENT_NOTIFY_WIFI_NO_INTERNET";
            case EVENT_ON_PDN_HANDOVER:
                return "EVENT_ON_PDN_HANDOVER";
            case EVENT_ON_PDN_ERROR:
                return "EVENT_ON_PDN_ERROR";
            case EVENT_ON_ROVE_OUT:
                return "EVENT_ON_ROVE_OUT";
            case EVENT_ON_WFC_PDN_STATE_CHANGED:
                return "EVENT_ON_WFC_PDN_STATE_CHANGED";
            case RESPONSE_SET_WFC_EMERGENCY_ADDRESS_ID:
                return "RESPONSE_SET_WFC_EMERGENCY_ADDRESS_ID";
            case EVENT_ON_WIFI_PDN_OOS:
                return "EVENT_ON_WIFI_PDN_OOS";
            case EVENT_ON_LOCATION_TIMEOUT:
                return "EVENT_ON_LOCATION_TIMEOUT";
            case EVENT_HANDLE_WIFI_STATE_CHANGE:
                return "EVENT_HANDLE_WIFI_STATE_CHANGE";
            case EVENT_ON_ALLOW_WIFI_OFF:
                return "EVENT_ON_ALLOW_WIFI_OFF";
            case RESPONSE_NOTIFY_EPDG_SCREEN_STATE:
                return "RESPONSE_NOTIFY_EPDG_SCREEN_STATE";
            case EVENT_HANDLE_WFC_STATE_CHANGED:
                return "EVENT_HANDLE_WFC_STATE_CHANGED";
            case EVENT_INITIALIZE:
                return "EVENT_INITIALIZE";
            default:
                return "UNKNOWN";
        }
    }

    public Handler getHandler() {
        return this;
    }

    private IWifiOffloadService mWfoService = new IWifiOffloadService.Stub() {
        @Override
        public void registerForHandoverEvent(IWifiOffloadListener listener) {
            log("registerForHandoverEvent for " + listener.asBinder());
            mListeners.register(listener);
        }

        @Override
        public void unregisterForHandoverEvent(IWifiOffloadListener listener) {
            log("unregisterForHandoverEvent for " + listener.asBinder());
            mListeners.unregister(listener);
        }

        @Override
        public int getRatType(int simIdx) {
            log("getRatType() not supported");
            return 0;
        }

        @Override
        public DisconnectCause getDisconnectCause(int simIdx) {
            if (checkInvalidSimIdx(simIdx, "getDisconnectCause()")) {
                return null;
            }
            return mDisconnectCause[simIdx];
        }

        @Override
        public void setEpdgFqdn(int simIdx, String fqdn, boolean wfcEnabled) {
            log("setEpdgFqdn() not supported");
        }

        @Override
        public void updateCallState(int simIdx, int callId, int callType, int callState) {
            log("updateCallState() not supported");
        }

        @Override
        public boolean isWifiConnected() {
            return mWifiPdnHandler.isWifiConnected();
        }

        @Override
        public void updateRadioState(int simIdx, int radioState) {
            log("updateRadioState() not supported");
        }

        @Override
        public boolean setMccMncAllowList(String[] allowList) {
            log("setMccMncAllowList() not supported");
            return false;
        }

        @Override
        public String[] getMccMncAllowList(int mode) {
            log("getMccMncAllowList() not supported");
            return null;
        }

        @Override
        public void factoryReset() {
            log("factoryReset()");

            mWosExt.factoryReset();
        }

        @Override
        public boolean setWifiOff() {
            if (mWifiPdnHandler.isWifiPdnExisted()) {
                mHasWiFiDisabledPending = true;
                mWifiPdnHandler.setWifiOff();
            } else {
                //directly notify can wifi off
                mHasWiFiDisabledPending = false;
            }
            log("setWifiOff() mHasWiFiDisabledPending" + mHasWiFiDisabledPending);
            return mHasWiFiDisabledPending;
        }
    };

    private void notifyOnAllowWifiOff() {
        log("notifyOnAllowWifiOff");
        int i = mListeners.beginBroadcast();
        while (i > 0) {
            i--;
            try {
                mListeners.getBroadcastItem(i).onAllowWifiOff();
            } catch (RemoteException e) {
                // The RemoteCallbackList will take care of removing
                // the dead object for us.
                log("onHandover: RemoteException occurs!");
            } catch (IllegalStateException e) {
                log("onHandover: IllegalStateException occurs!");
            }
        }
        mListeners.finishBroadcast();
    }

    public static WfcHandler getInstance() {
        return mInstance;
    }

    public static WfcHandler getInstance(
            Context context, WifiPdnHandler wifiHandler, int simCount,
            Looper looper, MwiRIL[] mwiRil) {
        if (mInstance == null) {
            mInstance = new WfcHandler(context, wifiHandler, simCount, looper, mwiRil);
        }
        return mInstance;
    }

    public WfcHandler(
        Context context, WifiPdnHandler wifiHandler, int simCount,
        Looper looper, MwiRIL[] mwiRil) {

        super(looper);
        mContext = context;
        mWifiPdnHandler = wifiHandler;
        if (mWifiPdnHandler != null) {
            mWifiPdnHandler.setWfcHandler(this);
        }
        mSimCount = (simCount <= MAX_VALID_SIM_COUNT) ? simCount : MAX_VALID_SIM_COUNT;
        mMwiRil = mwiRil;

        mDisconnectCause = new DisconnectCause[mSimCount];

        mWfcState = new int[mSimCount];

        Message msg = obtainMessage(EVENT_INITIALIZE);
        msg.sendToTarget();
    }

    private void initialize() {
        mConnectivityManager = ConnectivityManager.from(mContext);
        mIsWfcSettingsOn = new boolean[mSimCount];

        // handle modem if the Mwis restart
        updateWfcUISetting();

        sendMessage(obtainMessage(EVENT_HANDLE_WIFI_STATE_CHANGE));

        // Create WWOP instance
        createWosExt();
        mIsWifiConnected = false;
        mPartialScanCount = 0;

        registerForBroadcast();
        registerIndication();

        // register for WFC settings
        registerForWfcAidObserver();
    }

    private void registerForWfcAidObserver() {
        if (mContext == null) {
            return;
        }
        mContext.getContentResolver().registerContentObserver(
            AID_SETTING_URI, false, mContentObserver);
        log("registerForWfcAidObserver()");
    }

    private void registerForBroadcast() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        filter.addAction(ACTION_OPERATOR_CONFIG_CHANGED);
        filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        filter.addAction(WifiManager.WIFI_SCAN_AVAILABLE);
        filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        // filter.addAction(WFC_STATUS_CHANGED);
        // Notify ePDG screen state
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_USER_PRESENT);

        mContext.registerReceiver(mReceiver, filter);
    }

    private void registerIndication() {

        for(int i = 0 ; i < mSimCount ; i++) {
            mMwiRil[i].registerWifiPdnHandover(this,
                    EVENT_ON_PDN_HANDOVER, null);
            mMwiRil[i].registerWifiPdnError(this,
                    EVENT_ON_PDN_ERROR, null);
            mMwiRil[i].registerWifiPdnRoveOut(this,
                    EVENT_ON_ROVE_OUT, null);
            mMwiRil[i].registerWfcPdnStateChanged(this,
                    EVENT_ON_WFC_PDN_STATE_CHANGED, null);
            mMwiRil[i].registerWifiPdnOos(this,
                    EVENT_ON_WIFI_PDN_OOS, null);
        }
    }


    private void onPdnHandover(Message msg) {

        AsyncResult ar = (AsyncResult) msg.obj;
        int[] result = (int[]) ar.result;

        if (null == result) {
            Rlog.e(TAG, "onPdnHandover(): result is null");
            return;
        }
        if (result.length < 5) {
            Rlog.e(TAG, "onPdnHandover(): Bad params");
            return;
        }
        int pdnType = result[0];
        if (pdnType != 0) {
            log("onPdnHandover(): Not IMS PDN, ignore");
            return;
        }

        int stage = result[1];
        int srcRat = result[2];
        int desRat = result[3];
        int simIdx = result[4];
        notifyOnHandover(simIdx, stage, desRat);

        if (stage == WifiOffloadManager.HANDOVER_END &&
                desRat != WifiOffloadManager.RAN_TYPE_WIFI) {
            mWosExt.clearPDNErrorMessages();
        }
    }

    private void onWfcPdnError(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        int[] result = (int[]) ar.result;

        if (null == result) {
            Rlog.e(TAG, "onWfcPdnError(): result is null");
            return;
        }
        int errCount = result[0];
        int mainErr = result[1];
        int subErr = result[2];
        int simIdx = result[3];
        mDisconnectCause[simIdx] = new DisconnectCause(mainErr, subErr);

        Rlog.e(TAG, "onWfcPdnError(): errCount = " + errCount + ", mainErr = " + mainErr
                + ", subErr = " + subErr + ", simIdx = " + simIdx);

        // CODE_WFC_EPDG_IPSEC_SETUP_ERROR: 1082
        // IKEv2 cause 24
        if ((mainErr == CODE_WFC_EPDG_IPSEC_SETUP_ERROR) && (subErr == SUB_CAUSE_IKEV2_24)) {
            ImsReasonInfo imsReasonInfo = new ImsReasonInfo(
                    ImsReasonInfo.CODE_EPDG_TUNNEL_ESTABLISH_FAILURE,
                    ImsReasonInfo.CODE_IKEV2_AUTH_FAILURE,
                    "WiFi_Error09-Unable to connect");

            Intent intent = new Intent(IMS_REG_ERROR_NOTIFICATION);
            intent.putExtra("result", (Parcelable) imsReasonInfo);
            intent.putExtra("stateChanged", NOT_REGISTERED);
            intent.putExtra("imsRat", TelephonyManager.NETWORK_TYPE_IWLAN);
            mContext.sendBroadcast(intent, IMS_REG_ERROR_NOTIFICATION_PERMISSION);

            boolean bVal = getBooleanCarrierConfig(
                        MTK_KEY_DISABLE_WFC_AFTER_AUTH_FAIL,
                        getSubIdBySlot(simIdx));
            if (bVal) {
                Rlog.d(TAG, "Set WFC setting OFF.");
                ImsManager mgr = ImsManager.getInstance(mContext, simIdx);
                mgr.setWfcSetting(false);
            }
        }

        if (errCount == 0) {
            mWosExt.clearPDNErrorMessages();
            return;
        }
        mWosExt.showPDNErrorMessages(mainErr);
    }


    private void onWifiRoveout(Message msg) {

        AsyncResult ar = (AsyncResult) msg.obj;
        String[] result = (String[]) ar.result;

        if (null == result) {
            Rlog.e(TAG, "onWifiRoveout(): result is null");
            return;
        }

        if (result.length == 3) {
            log("onWifiRoveout()[" + result.length + "] " + result[0] + " " +
                    result[1] + " " +result[2]);
        } else if (result.length == 4) {
            log("onWifiRoveout()[" + result.length + "] " + result[0] + " " +
                    result[1] + " " +result[2] + " " +result[3]);
        } else {
            Rlog.e(TAG, "onWifiRoveout(): Bad params [" + result.length + "] ");
            return;
        }

        try {
            String ifname = result[0];
            boolean roveOut = (Integer.parseInt(result[1]) == 1);
            boolean mobike_ind = (result.length == 4 && (Integer.parseInt(result[2]) == 1));
            int simIdx = Integer.parseInt(result[result.length - 1]);
            if (roveOut) {
                notifyOnRoveOut(simIdx, roveOut, mWifiPdnHandler.getLastRssi());
            }
            if (mobike_ind) {
                log("onWifiRoveout(): mobike_ind=1. count = " + mPartialScanCount
                        + ", connected = " + mIsWifiConnected);
                if ((!mIsWifiConnected) || (mPartialScanCount < 3) || (getCurrentAssociatedApCount() <= 1)) {
                    log("No need to partial scan.");
                } else {
                    log("Mobike disconnect+startscan");
                    WifiManager wifiMngr = (WifiManager)
                            mContext.getSystemService(Context.WIFI_SERVICE);
                    wifiMngr.disconnect();
                    wifiMngr.startScan();
                    mPartialScanCount = 0;
                }
            }
            log("onWifiRoveout: " + ifname + " " + roveOut + " " + mobike_ind);
        } catch (Exception e) {
            Rlog.e(TAG, "onWifiRoveout()[" + result.length + "]" + result[0] + " " +
                   result[1] + " " +result[2] + " e:" + e.toString());
        }
    }

    private void onWfcPdnStateChanged(Message msg) {

        AsyncResult ar = (AsyncResult) msg.obj;
        int[] result = (int[]) ar.result;

        if (null == result) {
            Rlog.e(TAG, "onWfcPdnStateChanged(): result is null");
            return;
        }
        int state  = result[0];
        int simIdx = result[1];

        if (simIdx < mSimCount) {
            mWfcState[simIdx] = state;
        }

        log("onWfcPdnStateChanged() state:" + state + " simIdx:" + simIdx);
        if (1 == state) {
            mWosExt.clearPDNErrorMessages();
        }

        int i = mListeners.beginBroadcast();
        while (i > 0) {
            i--;
            try {
                mListeners.getBroadcastItem(i).onWfcStateChanged(simIdx, state);
            } catch (RemoteException e) {
                // The RemoteCallbackList will take care of removing
                // the dead object for us.
                log("onWfcStateChanged: RemoteException occurs!");
            } catch (IllegalStateException e) {
                log("onWfcStateChanged: IllegalStateException occurs!");
            }
        }
        mListeners.finishBroadcast();
    }

    public int getWfcState(int simIdx) {
        log("getWfcState state:" + mWfcState[simIdx] + " simIdx:" + simIdx);
        return mWfcState[simIdx];
    }

    private void onWifiPdnOOS(Message msg) {

        AsyncResult ar = (AsyncResult) msg.obj;
        String[] result = (String[]) ar.result;

        if (null == result) {
            Rlog.e(TAG, "onWifiPdnOOS(): result is null");
            return;
        }
        if (result.length < 4) {
            Rlog.e(TAG, "onWifiPdnOOS(): Bad params");
            return;
        }

        try {
            String apn = result[0];
            int callId = Integer.parseInt(result[1]);
            int oosState = Integer.parseInt(result[2]);
            int simIdx = Integer.parseInt(result[3]);
            notifyOnWifiPdnOOS(apn, callId, oosState, simIdx);
        } catch (Exception e) {
            Rlog.e(TAG, "onWifiPdnOOS[" + result.length + "]"  +  result[0] + " " +
                result[1] + " " +result[2] + " " + result[3] + " e:" + e.toString());
        }
    }

    private boolean checkInvalidSimIdx(int simIdx, String dbgMsg) {
        if (simIdx < 0 || simIdx >= mSimCount) {
            log(dbgMsg);
            return true;
        }
        return false;
    }

    private void setEmergencyAddressId() {
        if (TextUtils.isEmpty(mWfcEccAid)) {
            log("Current AID is empty");
            return;
        }
        log("setEmergencyAddressId(), mWfcEccAid:" + mWfcEccAid);
        Message result = obtainMessage(RESPONSE_SET_WFC_EMERGENCY_ADDRESS_ID);
        getMwiRil().setEmergencyAddressId(mWfcEccAid, result);
    }

    private void handleModemPower() {
        log("handleModemPower() mIsWifiEnabled:" +
               mIsWifiEnabled + " mIsWfcSettingsOn: " + isWfcSettingsEnabledAny());
        /* If the flight mode is off, RadioManager will ignore the notify from WoS.
         * If the flight mode is on, handle the modem power by following rules:
         * 1. WFC enabled and Wifi is enabled, turn on modem
         * 2. WFC settings enabled and Wifi is enabled, turn on modem
         * 3. If Wfc settings or Wifi off, turn off modem
         */
        if (mIsWifiEnabled && isWfcSettingsEnabledAny()) {
            notifyPowerOnModem(true);
        } else {
            notifyPowerOnModem(false);
        }
    }

    private void notifyPowerOnModem(boolean isModemOn) {
        if (!SystemProperties.get("ro.vendor.mtk_flight_mode_power_off_md").equals("1")) {
            log("modem always on, no need to control it!");
            return;
        }

        if (mContext == null) {
            log("context is null, can't control modem!");
            return;
        }

        Intent intent = new Intent(RADIO_MANAGER_POWER_ON_MODEM);
        intent.setPackage("com.android.phone");
        intent.putExtra(EXTRA_POWER_ON_MODEM_KEY, isModemOn);
        mContext.sendBroadcast(intent);
    }

    private boolean isWfcSettingsEnabledAny() {
        for (int i = 0; i < mSimCount; ++i) {

            if (mIsWfcSettingsOn[i]) {
                log("isWfcSettingsEnabledAny: found Wfc settings enabled on SIM: " + i);
                return true;
            }
        }
        return false;
    }

    // Monitor wfc settings
    private final ContentObserver mContentObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange, Uri uri) {
            log("SettingsObserver.onChange(), selfChange:" + selfChange + ", uri:" + uri);


            if (AID_SETTING_URI.equals(uri)) {
                String aid = Settings.Global.getString(
                                 mContext.getContentResolver(), AID_SETTING_URI_STR);

                // ignore empty string
                if (TextUtils.isEmpty(aid)) {
                    log("empty aid: " + aid);
                    return;
                } else {
                    mWfcEccAid = aid;
                }
                log("mWfcEccAid: " + mWfcEccAid);

                // restore as empty string
                Settings.Global.putString(mContext.getContentResolver(), AID_SETTING_URI_STR, "");

                sendMessage(obtainMessage(EVENT_SET_WFC_EMERGENCY_ADDRESS_ID));
            }
        }
    };

    private boolean updateWfcUISetting() {
        boolean ret = false;
        if (SystemProperties.getInt(MtkImsConstants.MULTI_IMS_SUPPORT, 1) > 1) {
            for (int i = 0; i < mSimCount; ++i) {
                String key = SubscriptionManager.WFC_IMS_ENABLED;
                boolean defValue = getBooleanCarrierConfig(
                        CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ENABLED_BOOL,
                        getSubIdBySlot(i));
                boolean oldValue = mIsWfcSettingsOn[i];
                mIsWfcSettingsOn[i] = SubscriptionManager.getBooleanSubscriptionProperty(
                        getSubIdBySlot(i), key, defValue, mContext);

                log("WfcSetting simId: " + i + " enabled: " + mIsWfcSettingsOn[i]);
                if ((oldValue != mIsWfcSettingsOn[i]) || mIsWfcSettingsOn[i]) {
                    ret = true;
                    break;
                }
            }
        } else {
            int mainCapabilityPhoneId = getMainCapabilityPhoneId();
            log("mainCapabilityPhoneId = " + mainCapabilityPhoneId);
            if ((mainCapabilityPhoneId >= 0) && (mainCapabilityPhoneId < mSimCount)) {
                String key = SubscriptionManager.WFC_IMS_ENABLED;
                boolean defValue = getBooleanCarrierConfig(
                        CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ENABLED_BOOL,
                        getSubIdBySlot(mainCapabilityPhoneId));
                boolean oldValue = mIsWfcSettingsOn[mainCapabilityPhoneId];
                mIsWfcSettingsOn[mainCapabilityPhoneId] =
                        SubscriptionManager.getBooleanSubscriptionProperty(
                        getSubIdBySlot(mainCapabilityPhoneId), key, defValue, mContext);

                log("WfcSetting simId: " + mainCapabilityPhoneId + " enabled: "
                        + mIsWfcSettingsOn[mainCapabilityPhoneId]);
                if ((oldValue != mIsWfcSettingsOn[mainCapabilityPhoneId])
                        || mIsWfcSettingsOn[mainCapabilityPhoneId]) {
                    ret = true;
                }
            } else {
                Rlog.e(TAG, "updateWfcUISetting(): mainCapabilityPhoneId invalid");
            }
        }

        return ret;
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            log("onReceive action:" + intent.getAction());
            if (intent.getAction().equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
                sendMessage(obtainMessage(EVENT_HANDLE_WIFI_STATE_CHANGE));
            } else if (intent.getAction().equals(WifiManager.WIFI_SCAN_AVAILABLE)) {
                int state = intent.getIntExtra(
                    WifiManager.EXTRA_SCAN_AVAILABLE, WifiManager.WIFI_STATE_UNKNOWN);
                Rlog.d(TAG, "Receive WIFI_SCAN_AVAILABLE, state: " + state);
                sendMessage(obtainMessage(EVENT_HANDLE_WIFI_STATE_CHANGE));
            } else if (intent.getAction().equals(
                           CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);

                ImsManager mgr = ImsManager.getInstance(context, phoneId);
                if (mgr != null && !mgr.isWfcEnabledByPlatform()) {
                    log("isWfcEnabledByPlatform(" + phoneId +
                           ") is false, clearPDNErrorMessages");
                    mWosExt.clearPDNErrorMessages();
                }

                handleModemPower();
            } else if (intent.getAction().equals(ACTION_OPERATOR_CONFIG_CHANGED)) {

                // CXP switch so create WWOP instance again
                createWosExt();

            } else if (intent.getAction().equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
                Parcelable parcelableExtra = intent
                                             .getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
                if (null != parcelableExtra) {
                    NetworkInfo networkInfo = (NetworkInfo) parcelableExtra;
                    State state = networkInfo.getState();
                    mIsWifiL2Connected = state == State.CONNECTED;

                    checkIfShowNoInternetError(false);
                }
            } else if (intent.getAction().equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                String imsEnabledKey = intent.getStringExtra(INTENT_KEY_PROP_KEY);
                if (SubscriptionManager.WFC_IMS_ENABLED.equals(imsEnabledKey)) {
                    log("ACTION_SUBINFO_RECORD_UPDATED: WFC_IMS_ENABLED changes");
                    if (updateWfcUISetting()) {
                        sendMessage(obtainMessage(EVENT_HANDLE_MODEM_POWER));

                        checkIfShowNoInternetError(false);
                    }
                }
            } else if (intent.getAction().equals(WFC_STATUS_CHANGED)) {
                if (!mWifiPdnHandler.isWifiPdnExisted() && mHasWiFiDisabledPending) {
                    mHasWiFiDisabledPending = false;
                    sendMessage(obtainMessage(EVENT_ON_ALLOW_WIFI_OFF));
                }
            // Notify ePDG screen state
            } else if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
                mScreenState = ScreenState.SCREEN_OFF.ordinal();
                notifyEPDGScreenState(mScreenState);
            } else if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
                mScreenState = ScreenState.SCREEN_ON.ordinal();
                notifyEPDGScreenState(mScreenState);
            } else if (intent.getAction().equals(Intent.ACTION_USER_PRESENT)) {
                mScreenState = ScreenState.USER_PRESENT.ordinal();
                notifyEPDGScreenState(mScreenState);
            }
        }
    };

    // Notify ePDG screen state
    private void notifyEPDGScreenState(int state) {
        log("notifyEPDGScreenState(), state:" + state);

        Message result = obtainMessage(RESPONSE_NOTIFY_EPDG_SCREEN_STATE);
        getMwiRil().notifyEPDGScreenState(state, result);
    }

    private void notifyOnHandover(int simIdx, int stage, int ratType) {
        log("onHandover simIdx: " + simIdx + " stage: " + stage + " rat: " + ratType);
        int i = mListeners.beginBroadcast();
        while (i > 0) {
            i--;
            try {
                mListeners.getBroadcastItem(i).onHandover(simIdx, stage, ratType);
            } catch (RemoteException e) {
                // The RemoteCallbackList will take care of removing
                // the dead object for us.
                log("onHandover: RemoteException occurs!");
            } catch (IllegalStateException e) {
                log("onHandover: IllegalStateException occurs!");
            }
        }
        mListeners.finishBroadcast();
    }

    private void notifyOnRoveOut(int simIdx, boolean roveOut, int rssi) {
        log("onRoveOut simIdx: " + simIdx + " roveOut: " + roveOut + " rssi: " + rssi);
        int i = mListeners.beginBroadcast();
        while (i > 0) {
            i--;
            try {
                mListeners.getBroadcastItem(i).onRoveOut(simIdx, roveOut, rssi);
            } catch (RemoteException e) {
                log("onRoveOut: RemoteException occurs!");
            }
        }
        mListeners.finishBroadcast();
    }

    private void notifyOnWifiPdnOOS(String apn, int callId, int oosState, int simId) {
        log("onWifiPdnOOS apn: " + apn + " callId: " + callId + " oosState: " + oosState + " simId: " + simId);

        int i = mListeners.beginBroadcast();

        while (i > 0) {
            i--;
            try {
                mListeners.getBroadcastItem(i).onWifiPdnOOSStateChanged(simId, oosState);
                log("onWifiPdnOOSStateChanged");
            } catch (RemoteException e) {
                log("onRoveOut: RemoteException occurs!");
            }
        }
        mListeners.finishBroadcast();
    }

    public IWifiOffloadService getWfoInterface() {
        return mWfoService;
    }

    /**
     * to get main capability phone id.
     *
     * @return The phone id with highest capability.
     */
    private int getMainCapabilityPhoneId() {
        int phoneId = SystemProperties.getInt(MtkImsConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
            phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        }
        return phoneId;
    }

    private MwiRIL getMwiRil() {
        int phoneId = getMainCapabilityPhoneId();

        if (phoneId == SubscriptionManager.INVALID_PHONE_INDEX) {
            return null;
        } else {
            return mMwiRil[phoneId];
        }
    }

    private void checkIfShowNoInternetError(boolean showImmediately) {

        int mainCapabilityPhoneId = getMainCapabilityPhoneId();
        int subId = getSubIdBySlot(mainCapabilityPhoneId);

        boolean isImsReg = TelephonyManager.getDefault().isImsRegistered(subId);
        boolean isWifiConnected = mWifiPdnHandler.isWifiConnected();

        if (mIsWfcSettingsOn[mainCapabilityPhoneId] && !isImsReg &&
                mIsWifiL2Connected && !isWifiConnected) {

            if (showImmediately) {
                mWosExt.showPDNErrorMessages(WIFI_NO_INTERNET_ERROR_CODE);
                return;
            }

            if (!hasMessages(EVENT_NOTIFY_WIFI_NO_INTERNET)) {
                log("checkIfShowNoInternetError(): start 8s timeout");
                sendMessageDelayed(
                    obtainMessage(EVENT_NOTIFY_WIFI_NO_INTERNET),
                    WIFI_NO_INTERNET_TIMEOUT);
            }

        } else {
            if (hasMessages(EVENT_NOTIFY_WIFI_NO_INTERNET)) {
                log("checkIfShowNoInternetError(): cancel 8s timeout");
                removeMessages(EVENT_NOTIFY_WIFI_NO_INTERNET);
            }
        }
    }

    private int getSubIdBySlot(int slot) {
        int [] subId = SubscriptionManager.getSubId(slot);
        return (subId != null) ? subId[0] : SubscriptionManager.getDefaultSubscriptionId();
    }

    // for operator add-on
    private void createWosExt() {
        OpWosCustomizationFactoryBase wosFactory = OpWosCustomizationUtils.getOpFactory(mContext);
        mWosExt = wosFactory.makeWosExt(mContext);
        mWosExt.initialize(mContext);
    }

    protected void log(String s) {
        if (!USR_BUILD || TELDBG) {
            Rlog.d(TAG, s);
        }
    }

    public void onLocationTimeout() {
        Message msg = obtainMessage(EVENT_ON_LOCATION_TIMEOUT);
        sendMessage(msg);
    }

    private void notifyLocationTimeout() {
        if (mWosExt != null) {
            mWosExt.showLocationTimeoutMessage();
        } else {
            Rlog.e(TAG, "notifyLocationTimeout: mWosExt null");
        }
    }

    public void updatedWifiConnectedStatus(boolean isConnected) {
        log("updatedWifiConnectedStatus: " + isConnected);
        mIsWifiConnected = isConnected;
        if (!isConnected) {
            mPartialScanCount = 0;
        }
    }

    private int getCurrentAssociatedApCount() {
        WifiManager wifiManager;
        int count = 0;
        wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);

        if (wifiManager != null) {
            List<ScanResult> scanResults;
            scanResults = wifiManager.getScanResults();
            List<WifiConfiguration> wifiConfigList;
            wifiConfigList = wifiManager.getConfiguredNetworks();
            if (scanResults != null && wifiConfigList != null) {
                //log("getCurrentAssociatedApCount(): scan results: " + scanResults.size()
                //        + ", config list: " + wifiConfigList.size());
                for (ScanResult appInfo : scanResults) {
                    for (WifiConfiguration configuredAP : wifiConfigList) {
                        String strTrimmed = "";
                        if (configuredAP.SSID != null) {
                            strTrimmed = configuredAP.SSID.replace("\"",""); // remove \"
                        }
                        if (appInfo.SSID != null && appInfo.SSID.equals(strTrimmed)) {
                            // log("getCurrentAssociatedApCount(): Associated SSID: " + appInfo.SSID);
                            count++;
                        }
                    }
                }
            } else {
                Rlog.e(TAG, "getCurrentAssociatedApCount() scanResults = " + scanResults
                        + ", wifiConfigList = " + wifiConfigList);
            }
        } else {
            Rlog.e(TAG, "getCurrentAssociatedApCount() wifiManager null");
        }

        log("getCurrentAssociatedApCount(): count= " + count);
        return count;
    }

    private void updateWifiEnabled() {
        // prevent ServiceNotFoundException
        if (!StorageManager.inCryptKeeperBounce()) {
            WifiManager wifiMngr = (WifiManager)
                mContext.getSystemService(Context.WIFI_SERVICE);
            if (wifiMngr != null) {
                mIsWifiEnabled = wifiMngr.isWifiEnabled();
            } else {
                Rlog.e(TAG, "updateWifiEnabled: WifiManager null");
                mIsWifiEnabled = false;
            }
        } else {
            Rlog.e(TAG, "updateWifiEnabled: inCryptKeeperBounce");
            mIsWifiEnabled = false;
        }
        sendMessage(obtainMessage(EVENT_HANDLE_MODEM_POWER));
    }

    private boolean getBooleanCarrierConfig(String key, int subId) {
        CarrierConfigManager configManager = (CarrierConfigManager) mContext.getSystemService(
                Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle b = null;
        if (configManager != null) {
            b = configManager.getConfigForSubId(subId);
        }

        boolean ret;
        if (b != null) {
            ret = b.getBoolean(key);
        } else {
            // Return static default defined in CarrierConfigManager.
            log("getBooleanCarrierConfig: get from default config");
            ret = CarrierConfigManager.getDefaultConfig().getBoolean(key);
        }

        log("getBooleanCarrierConfig sub: " + subId + " key: " + key + " ret: " + ret);
        return ret;
    }

    private void onWfcStatusChanged() {
        if (!mWifiPdnHandler.isWifiPdnExisted() && mHasWiFiDisabledPending) {
            mHasWiFiDisabledPending = false;
            sendMessage(obtainMessage(EVENT_ON_ALLOW_WIFI_OFF));
        }
    }
}
