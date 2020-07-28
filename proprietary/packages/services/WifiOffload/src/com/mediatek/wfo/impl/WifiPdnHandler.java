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

import java.lang.Integer;
import java.lang.StringBuilder;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Collections;
import java.util.ArrayList;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.RouteInfo;
import android.net.LinkAddress;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.NetworkInfo;

import android.os.AsyncResult;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.ServiceManager;
import android.os.storage.StorageManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneStateListener;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import android.telephony.TelephonyManager;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.ims.MtkImsConstants;

import com.mediatek.wfo.ril.MwiRIL;
import com.mediatek.wfo.util.PacketKeepAliveProcessor;
import com.mediatek.wfo.util.PacketKeepAliveProcessor.KeepAliveConfig;
import com.mediatek.wfo.util.RssiMonitoringProcessor;

/**
 * Top-level Application class for the WifiOffload app.
 */
public class WifiPdnHandler extends Handler {
    private static final String TAG = "WifiPdnHandler";
    private static final String WIFI_IF_NAME = "wlan0";
    private static final String NONE_SSID = "<unknown ssid>";

    private Context mContext;
    private int mSimCount;
    private boolean mHasWiFiDisabledPending;

    private WifiManager mWifiManager;
    private int mRatType[];
    private boolean mIsWifiEnabled;
    private boolean[] mWifiPdnExisted;
    private boolean[] mModemReqWifiLock;
    private WifiManager.WifiLock mWifiLock;
    private int mWifiLockCount = 0;
    private MwiRIL[] mMwiRil;

    private ConnectivityManager mConnectivityManager;
    private RssiMonitoringProcessor mRssiMonitoringProcessor;
    private PacketKeepAliveProcessor mPacketKeepAliveProcessor;

    private TelephonyManager mTelephonyManager;

    private PingDns mPingDns;

    // wifi state
    private boolean mIsWifiConnected = false;
    private boolean mOldWifiConnectedFromNetworkInfo = false;
    private String mWifiApMac = "";
    private static String mWifiUeMac = "02:00:00:00:00:00";
    private String mWifiIpv4Address = "";
    private String mWifiIpv6Address = "";
    private String mWifiIpv4Gateway = "";
    private String mWifiIpv6Gateway = "";
    private int mWifiIpv4PrefixLen = -1;
    private int mWifiIpv6PrefixLen = -1;
    private int mMtu = 0;
    private List<InetAddress> mDnsServers = null;
    private String mIfName = "";
    private String mSsid = "";
    private int mLastRssi;
    private boolean mIsAirplaneModeOn = false;
    private boolean mIsAirplaneModeChange = false;
    private boolean mDeferredNotificationToWifi = false;
    private WfcHandler mWfcHandler = null;

    private static final String PROPERTY_WFC_ENABLE = "persist.vendor.mtk.wfc.enable";
    private static final String PROPERTY_MIMS_SUPPORT = "persist.vendor.mims_support";

    // for Wos -> Wi-Fi fwk to notify defer Wi-Fi disabled action for WFC de-initialization.
    private static final String WFC_STATUS_CHANGED =
        "com.mediatek.intent.action.WFC_STATUS_CHANGED";
    private static final String EXTRA_WFC_STATUS_KEY = "wfc_status";

    private static final int NO_NEED_DEFER = 0;
    private static final int NEED_DEFER = 1;
    private static final int WFC_NOTIFY_GO = 2;

    // for Wos -> Wi-Fi fwk to do partial scan to switch to another wifi ap
    private static final String WFC_REQUEST_PARTIAL_SCAN =
        "com.mediatek.intent.action.WFC_REQUEST_PARTIAL_SCAN";

    // for add-on decouple with MtkCarrierConfigManager.MTK_KEY_WOS_SUPPORT_WFC_IN_FLIGHTMODE
    private static final String MTK_KEY_WOS_SUPPORT_WFC_IN_FLIGHTMODE =
        "wos_flight_mode_support_bool";

    public static final int SNR_UNKNOWN = 60;

    // Message codes. See mHandler below.
    private static final int BASE = 1000;
    private static final int EVENT_WIFI_NETWORK_STATE_CHANGE            = BASE + 0;
    private static final int EVENT_SET_WIFI_SIGNAL_STRENGTH             = BASE + 1;
    private static final int EVENT_SET_WIFI_ENABLED                     = BASE + 2;
    private static final int EVENT_SET_WIFI_ASSOC                       = BASE + 3;
    private static final int EVENT_SET_WIFI_IP_ADDR                     = BASE + 4;
    private static final int EVENT_RADIO_AVAILABLE                      = BASE + 5;
    private static final int EVENT_WIFI_SCAN                            = BASE + 6;
    public static final int EVENT_SET_NATT_STATUS                       = BASE + 7;
    private static final int EVENT_RETRY_INIT                           = BASE + 8;
    private static final int EVENT_RETRY_UPDATE_WIFI_CONNTECTED_INFO    = BASE + 9;
    private static final int EVENT_RETRY_CHECK_IF_START_WIFI_SCAN       = BASE + 10;
    private static final int EVENT_RETRY_UPDATE_LAST_RSSI               = BASE + 11;
    private static final int EVENT_WIFI_SCAN_AVAILABLE                  = BASE + 12;
    private static final int EVENT_WIFI_STATE_CHANGE                    = BASE + 13;
    private static final int EVENT_SET_WIFI_UE_MAC                      = BASE + 14;
    private static final int EVENT_INITIALIZE                           = BASE + 15;

    public static final int EVENT_ON_WIFI_MONITORING_THRESHOLD_CHANGED  = BASE + 101;
    public static final int EVENT_ON_WIFI_PDN_ACTIVATE                  = BASE + 102;
    public static final int EVENT_ON_NATT_KEEP_ALIVE_CHANGED            = BASE + 103;

    public static final int EVENT_ON_WIFI_PING_REQUEST                 = BASE + 150;
    public static final int EVENT_SET_WIFI_PING_RESULT                 = BASE + 151;
    public static final int EVENT_HANDLE_AIRPLANE_MODE                 = BASE + 152;
    public static final int EVENT_ON_WIFI_LOCK                         = BASE + 153;

    private static final int RESPONSE_SET_WIFI_ENABLED                  = BASE + 200;
    private static final int RESPONSE_SET_WIFI_SIGNAL_LEVEL             = BASE + 201;
    private static final int RESPONSE_SET_WIFI_ASSOC                    = BASE + 202;
    private static final int RESPONSE_SET_WIFI_IP_ADDR                  = BASE + 203;
    private static final int RESPONSE_SET_NATT_KEEP_ALIVE_STATUS        = BASE + 204;
    private static final int RESPONSE_SET_WIFI_PING_RESULT              = BASE + 205;
    private static final int RESPONSE_SET_WIFI_UE_MAC                   = BASE + 206;

    private static final int RETRY_TIMEOUT = 3000;
    private static final int WIFI_SCAN_DELAY = 3000; // 3s
    private static final int DEFAULT_MTU_SIZE = 1500;

    private static final int WIFI_STATE_UI_DISABLING = WifiManager.WIFI_STATE_DISABLING + 9900;

    // log task
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean USR_BUILD = TextUtils.equals(Build.TYPE, "user")
            || TextUtils.equals(Build.TYPE, "userdebug");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    enum WifiLockSource {
        WIFI_STATE_CHANGE,
        MODEM_STATE_CHANGE
    }
    private static final int EWIFIEN_NEED_SEND_AP_MODE      = 1 << 4;
    private static final int EWIFIEN_AP_MODE_STATE          = 1 << 3;
    private static final int EWIFIEN_NEED_SEND_WIFI_ENABLED = 1 << 2;
    private static final int EWIFIEN_WIFI_ENABLED_STATE     = 1 << 1;
    private static final int EWIFIEN_CAUSE                  = 1 << 0;

    private int mRadioState = TelephonyManager.RADIO_POWER_UNAVAILABLE;

    private ArrayList<Message> mPendingMsgs = new ArrayList<>();
    private Object mLock = new Object();

    private PhoneStateListener[] mPhoneStateListener = null;

    private Network mNetwork = null;

    private WifiManager.StaStateCallback mStaStateCallback = new  WifiManager.StaStateCallback() {
        @Override
        public void onStaToBeOff()  {
            log("WifiManager.StaStateCallback - onStaToBeOff");
            Message msg = obtainMessage(EVENT_WIFI_STATE_CHANGE, WIFI_STATE_UI_DISABLING, 0);
            msg.sendToTarget();
        }
    };

    @Override
    public void handleMessage(Message msg) {

        AsyncResult ar;
        switch (msg.what) {
            /* handle internal event start */
            case EVENT_WIFI_NETWORK_STATE_CHANGE: {
                int isConnected = msg.arg1;
                if (0 == isConnected) {
                    log("wifi is disconnect, notify packet keep alive to stop");
                    mPacketKeepAliveProcessor.notifyWifiDisconnect();
                } else {
                    // Update UE Wi-Fi mac
                    String currentUeMac = updateWlanMacAddr();
                    if (TextUtils.equals(currentUeMac, "02:00:00:00:00:00")){
                        // exception, keep original UE mac address
                        // default value is "02:00:00:00:00:00"
                    }
                    else if (!TextUtils.equals(mWifiUeMac, currentUeMac)) {
                        mWifiUeMac = currentUeMac;
                        log("WifiPdnHandler updateWlanMacAddr mWifiUeMac: " + maskString(mWifiUeMac));
                    }
                }
                updateWifiConnectedInfo(isConnected);
                break;
            }
            case EVENT_SET_WIFI_SIGNAL_STRENGTH:
                setWifiSignalLevel();
                break;
            case EVENT_SET_WIFI_ENABLED:
                setWifiEnabled();
                break;
            case EVENT_SET_WIFI_ASSOC:
                setWifiAssoc();
                break;
            case EVENT_SET_WIFI_UE_MAC:
                setWifiUeMac();
                break;
            case EVENT_SET_WIFI_IP_ADDR:
                setWifiIpAddress();
                break;
            case EVENT_RADIO_AVAILABLE:
                // TRM happened, sync information to RIL
                log("Sync airplane mode to MD: " + mIsAirplaneModeChange);
                if (mIsAirplaneModeChange) {
                    mIsAirplaneModeChange = false;
                    setWifiEnabledWithSyncAPMode();
                }

                setWifiAssoc();
                setWifiEnabled();
                if (mIsWifiConnected) {
                    setWifiSignalLevel();
                    setWifiIpAddress();
                }
                break;
            case EVENT_WIFI_SCAN:
                // Check if start wifi scan to improve MOBIKE
                checkIfstartWifiScan(true);
                break;
            case EVENT_SET_NATT_STATUS:
                setNattKeepAliveStatus(msg);
                break;
            case EVENT_SET_WIFI_PING_RESULT:
                setWifiPingResult(msg);
                break;
            /* handle internal event end */
            /* handle UNSOL start */
            case EVENT_ON_WIFI_MONITORING_THRESHOLD_CHANGED:
                onWifiMonitoringThreshouldChanged(msg);
                break;
            case EVENT_ON_WIFI_PDN_ACTIVATE:
                onWifiPdnActivate(msg);
                break;
            case EVENT_ON_NATT_KEEP_ALIVE_CHANGED:
                onNattKeepAliveChanged(msg);
                break;
            case EVENT_ON_WIFI_PING_REQUEST:
                onWifiPingRequest(msg);
                break;
            case EVENT_ON_WIFI_LOCK:
                onWifiLock(msg);
                break;
            /* handle UNSOL end */
            /* handle request response start */
            case RESPONSE_SET_WIFI_ENABLED:
                ar = (AsyncResult) msg.obj;
                handleRetry(EVENT_SET_WIFI_ENABLED, ar);
                break;
            case RESPONSE_SET_WIFI_SIGNAL_LEVEL:
                ar = (AsyncResult) msg.obj;
                handleRetry(EVENT_SET_WIFI_SIGNAL_STRENGTH, ar);
                break;
            case RESPONSE_SET_WIFI_ASSOC:
                ar = (AsyncResult) msg.obj;
                handleRetry(EVENT_SET_WIFI_ASSOC, ar);
                break;
            case RESPONSE_SET_WIFI_UE_MAC:
                ar = (AsyncResult) msg.obj;
                handleRetry(EVENT_SET_WIFI_UE_MAC, ar);
                break;
            case RESPONSE_SET_WIFI_IP_ADDR:
                ar = (AsyncResult) msg.obj;
                handleRetry(EVENT_SET_WIFI_IP_ADDR, ar);
                break;
            case RESPONSE_SET_NATT_KEEP_ALIVE_STATUS:
                ar = (AsyncResult) msg.obj;
                handleRetry(EVENT_SET_NATT_STATUS, ar);
                break;
            case RESPONSE_SET_WIFI_PING_RESULT:
                ar = (AsyncResult) msg.obj;
                handleRetry(EVENT_SET_WIFI_PING_RESULT, ar);
                break;
            case EVENT_HANDLE_AIRPLANE_MODE: {
                handleAirplaneMode();
                break;
            }
            case EVENT_RETRY_INIT: {
                initWifiManager();
                break;
            }
            case EVENT_RETRY_UPDATE_WIFI_CONNTECTED_INFO: {
                int isConnected = msg.arg1;
                updateWifiConnectedInfo(isConnected);
                break;
            }
            case EVENT_RETRY_CHECK_IF_START_WIFI_SCAN: {
                boolean scanImmediately = (boolean) msg.obj;
                Rlog.d(TAG, "Retry checkIfstartWifiScan, scanImmediately: " + scanImmediately);
                checkIfstartWifiScan(scanImmediately);
                break;
            }
            case EVENT_RETRY_UPDATE_LAST_RSSI: {
                updateLastRssi();
                setWifiSignalLevel();
                break;
            }
            case EVENT_WIFI_SCAN_AVAILABLE:{
                synchronized (mLock) {
                    for (Message retryMsg: mPendingMsgs) {
                        log("Retry: " + messageToString(retryMsg));
                        retryMsg.sendToTarget();
                    }
                    mPendingMsgs.clear();
                }
                break;
            }
            case EVENT_WIFI_STATE_CHANGE: {
                handleWifiStateChange(msg.arg1);
                break;
            }
            case EVENT_INITIALIZE: {
                initialize();
                break;
            }
            /* handle request response end */
            default:
                break;
        }
    }

    private void handleWifiStateChange(int wifiState) {
        if (mWifiManager == null) {
            log("Unexpected error, mWifiManager is null!");
            Message msg = obtainMessage(EVENT_WIFI_STATE_CHANGE, wifiState, 0);
            synchronized (mLock) {
                mPendingMsgs.add(msg);
            }
            return;
        }

        log("handleWifiStateChange wifiState: " + wifiState);

        try {
            boolean isAirplaneModeOn = getAirplaneModeFromSettings();
            if (mIsAirplaneModeOn != isAirplaneModeOn) {
                mIsAirplaneModeOn = isAirplaneModeOn;
                mIsAirplaneModeChange = true;
                log("handleWifiStateChange change due to airplane mode change");
            }
        } catch (SettingNotFoundException e) {
            Rlog.e(TAG, "Can not get AIRPLANE_MODE_ON from provider.");
        }

         handleWifiDefferOff(WifiLockSource.WIFI_STATE_CHANGE, wifiState);
         if (wifiState == WIFI_STATE_UI_DISABLING) { //notify modem to handover
             mIsWifiEnabled = false;
             setWifiEnabled();
         } else {
             boolean isWifiEnabled = mWifiManager.isWifiEnabled();
             //ture if only if WifiManager.WIFI_STATE_ENABLED
             if (isWifiEnabled != mIsWifiEnabled) {
                 mIsWifiEnabled = isWifiEnabled;
                 setWifiEnabled();
             }
         }
    }

    private void handleAirplaneMode() {
        boolean isAirplaneModeOn = false;
        try {
            isAirplaneModeOn = getAirplaneModeFromSettings(); //modify
        } catch (SettingNotFoundException e) {
            Rlog.e(TAG, "Can not get AIRPLANE_MODE_ON from provider.");
        }

        log("handleAirplaneMode mIsAirplaneModeOn: " + mIsAirplaneModeOn);

        if (mIsAirplaneModeOn != isAirplaneModeOn) {
            mIsAirplaneModeOn = isAirplaneModeOn;
            mIsAirplaneModeChange = true;
        }
    }

    private boolean getAirplaneModeFromSettings() throws SettingNotFoundException {
        try {
            boolean value = Settings.Global.getInt(
                    mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON) == 1 ?
                    true : false;
            log("getAirplaneModeFromSettings: " + value);
            checkRadioPowerState();
            if (isEccInProgress() || mRadioState == TelephonyManager.RADIO_POWER_ON) {
                log("Disable airplane mode after get from setting if radio is on");
                value = false;
            }
            return value;
        } catch (SettingNotFoundException e) {
            Rlog.e(TAG, "Can not get AIRPLANE_MODE_ON from provider.");
            throw e;
        }
    }

    private void checkRadioPowerState() {
        if (mTelephonyManager == null && mContext != null) {
            mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        }

        if (mTelephonyManager != null) {
            mRadioState = mTelephonyManager.getRadioPowerState();
        } else {
            log("mTelephonyManager is null, keep the original RadioState");
        }

        log("checkRadioPowerState: " + mRadioState);
    }

    private void handleRetry(int msgId, AsyncResult result) {
        /*
        if (hasMessages(msgId)) {
            return;
        }

        if (result != null && result.exception != null) {
            sendEmptyMessageDelayed(msgId, RETRY_TIMEOUT);
        }
        */
    }

    private String messageToString(Message msg) {
        switch (msg.what) {

            case EVENT_WIFI_NETWORK_STATE_CHANGE:
                return "EVENT_WIFI_NETWORK_STATE_CHANGE";
            case EVENT_SET_WIFI_SIGNAL_STRENGTH:
                return "EVENT_SET_WIFI_SIGNAL_STRENGTH";
            case EVENT_SET_WIFI_ENABLED:
                return "EVENT_SET_WIFI_ENABLED";
            case EVENT_SET_WIFI_ASSOC:
                return "EVENT_SET_WIFI_ASSOC";
            case EVENT_SET_WIFI_IP_ADDR:
                return "EVENT_SET_WIFI_IP_ADDR";
            case EVENT_RADIO_AVAILABLE:
                return "EVENT_RADIO_AVAILABLE";
            case EVENT_ON_WIFI_MONITORING_THRESHOLD_CHANGED:
                return "EVENT_ON_WIFI_MONITORING_THRESHOLD_CHANGED";
            case EVENT_ON_WIFI_PDN_ACTIVATE:
                return "EVENT_ON_WIFI_PDN_ACTIVATE";
            case EVENT_ON_NATT_KEEP_ALIVE_CHANGED:
                return "EVENT_ON_NATT_KEEP_ALIVE_CHANGED";
            case EVENT_WIFI_SCAN:
                return "EVENT_WIFI_SCAN";
            case EVENT_SET_NATT_STATUS:
                return "EVENT_SET_NATT_STATUS";
            case EVENT_SET_WIFI_PING_RESULT:
                return "EVENT_SET_WIFI_PING_RESULT";
            case EVENT_ON_WIFI_PING_REQUEST:
                return "EVENT_ON_WIFI_PING_REQUEST";
            case EVENT_ON_WIFI_LOCK:
                return "EVENT_ON_WIFI_LOCK";
            case RESPONSE_SET_WIFI_ENABLED:
                return "RESPONSE_SET_WIFI_ENABLED";
            case RESPONSE_SET_WIFI_SIGNAL_LEVEL:
                return "RESPONSE_SET_WIFI_SIGNAL_LEVEL";
            case RESPONSE_SET_WIFI_ASSOC:
                return "RESPONSE_SET_WIFI_ASSOC";
            case RESPONSE_SET_WIFI_IP_ADDR:
                return "RESPONSE_SET_WIFI_IP_ADDR";
            case RESPONSE_SET_NATT_KEEP_ALIVE_STATUS:
                return "RESPONSE_SET_NATT_KEEP_ALIVE_STATUS";
            case RESPONSE_SET_WIFI_PING_RESULT:
                return "RESPONSE_SET_WIFI_PING_RESULT";
            case EVENT_HANDLE_AIRPLANE_MODE:
                return "EVENT_HANDLE_AIRPLANE_MODE";
            case EVENT_RETRY_INIT:
                return "EVENT_RETRY_INIT";
            case EVENT_RETRY_UPDATE_WIFI_CONNTECTED_INFO:
                return "EVENT_RETRY_UPDATE_WIFI_CONNTECTED_INFO";
            case EVENT_RETRY_CHECK_IF_START_WIFI_SCAN:
                return "EVENT_RETRY_CHECK_IF_START_WIFI_SCAN";
            case EVENT_RETRY_UPDATE_LAST_RSSI:
                return "EVENT_RETRY_UPDATE_LAST_RSSI";
            case EVENT_WIFI_SCAN_AVAILABLE:
                return "EVENT_WIFI_SCAN_AVAILABLE";
            case EVENT_WIFI_STATE_CHANGE:
                return "EVENT_WIFI_STATE_CHANGE";
            case EVENT_INITIALIZE:
                return "EVENT_INITIALIZE";
            default:
                return "UNKNOWN:" + msg.what;
        }
    }

    public Handler getHandler() {
        return this;
    }

    private PhoneStateListener getPhoneStateListener(int phoneId, int subId) {
        final int i = phoneId;
        final int subIdListened = subId;
        mPhoneStateListener[phoneId]  = new PhoneStateListener() {
            @Override
            public void onRadioPowerStateChanged(int state) {
                log("PhoneStateListener.onRadioPowerStateChanged, state=" + state
                        + ", phoneId=" + i);
                if (state != TelephonyManager.RADIO_POWER_UNAVAILABLE) {
                    log("send EVENT_RADIO_AVAILABLE");
                    //M: 04116599:Set airplane mode to off if radio is ON and airplane mode is on
                    boolean isInEcc = isEccInProgress();
                    if (!isInEcc && state == TelephonyManager.RADIO_POWER_ON &&
                            mIsAirplaneModeOn) {
                        log("Disable airplane mode if radio is on");
                        mIsAirplaneModeOn = false;
                        mIsAirplaneModeChange = true;
                    }
                    Message msg = obtainMessage(EVENT_RADIO_AVAILABLE);
                    sendMessage(msg);
                }
            }
        };
        return mPhoneStateListener[phoneId];
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null || intent.getAction() == null) {
                return;
            }
            log("onReceive action:" + intent.getAction());
            if (intent.getAction().equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
                int wifiState = intent.getIntExtra(
                        WifiManager.EXTRA_WIFI_STATE, WifiManager.WIFI_STATE_UNKNOWN);
                Message msg = obtainMessage(EVENT_WIFI_STATE_CHANGE, wifiState, 0);
                msg.sendToTarget();
            } else if (intent.getAction().equals(
                           TelephonyIntents.ACTION_RADIO_STATE_CHANGED)) {
                int intRadioState = intent.getIntExtra(
                                        "radioState", TelephonyManager.RADIO_POWER_ON);
                log("RADIO_STATE_CHANGED radioState: " + intRadioState);
                mRadioState = intRadioState;
                if (intRadioState != TelephonyManager.RADIO_POWER_UNAVAILABLE) {
                    log("send EVENT_RADIO_AVAILABLE");
                    boolean isInEcc = isEccInProgress();
                    //M: 04116599:Set airplane mode to off if radio is ON and airplane mode is on
                    if (isInEcc || intRadioState == TelephonyManager.RADIO_POWER_ON &&
                            mIsAirplaneModeOn) {
                        log("Disable airplane mode if radio is on");
                        mIsAirplaneModeOn = false;
                        mIsAirplaneModeChange = true;
                    }
                    Message msg = obtainMessage(EVENT_RADIO_AVAILABLE);
                    sendMessage(msg);
                }
            } else if (intent.getAction().equals(
                           Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                boolean isAirplaneModeOn = intent.getBooleanExtra("state", false);
                log("ACTION_AIRPLANE_MODE_CHANGED isAirplaneModeOn: " + isAirplaneModeOn);
                Message msg = obtainMessage(EVENT_HANDLE_AIRPLANE_MODE);
                sendMessage(msg);
            } else if (intent.getAction().equals(WifiManager.WIFI_SCAN_AVAILABLE)) {
                int state = intent.getIntExtra(
                    WifiManager.EXTRA_SCAN_AVAILABLE, WifiManager.WIFI_STATE_UNKNOWN);
                Rlog.d(TAG, "Receive WIFI_SCAN_AVAILABLE, state: " + state);
                Message msg = obtainMessage(EVENT_WIFI_SCAN_AVAILABLE);
                sendMessage(msg);
            } else if (intent.getAction().equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
                NetworkInfo networkInfo =
                        (NetworkInfo) intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
                boolean isWifiConnected = (networkInfo != null && networkInfo.isConnected());
                log("Receive NETWORK_STATE_CHANGED_ACTION, mIsWifiConnected: "+ mIsWifiConnected
                        + " networkInfo.isConnected(): " + isWifiConnected);
                if (mIsWifiConnected && mOldWifiConnectedFromNetworkInfo) {
                    Message msg = obtainMessage(
                                      EVENT_WIFI_NETWORK_STATE_CHANGE,
                                      (isWifiConnected)?1:0,
                                      0, null);
                    sendMessage(msg);
                }
                mOldWifiConnectedFromNetworkInfo = isWifiConnected;
            } else if (intent.getAction().equals(
                           CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
                log("ACTION_CARRIER_CONFIG_CHANGED mIsAirplaneModeOn: " + mIsAirplaneModeOn);
                if (mIsAirplaneModeOn) {
                    setWifiEnabled();
                }
            }
        }
    };

    public WifiPdnHandler(
        Context context, int simCount, Looper looper, MwiRIL[] mwiRil) {

        super(looper);
        mContext = context;
        mSimCount = simCount;
        mMwiRil = mwiRil;

        Message msg = obtainMessage(EVENT_INITIALIZE);
        msg.sendToTarget();
    }

    private void initialize() {
        initWifiManager();

        mConnectivityManager = ConnectivityManager.from(mContext);
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mRssiMonitoringProcessor = new RssiMonitoringProcessor(mConnectivityManager);
        mPacketKeepAliveProcessor = new PacketKeepAliveProcessor(
            mConnectivityManager, this);

        mWifiPdnExisted = new boolean[mSimCount];
        mModemReqWifiLock = new boolean[mSimCount];

        mRatType = new int[mSimCount];

        mRssiMonitoringProcessor.initialize(mSimCount);

        mIsAirplaneModeOn = Settings.System.getInt(mContext.getContentResolver(),
                   Settings.System.AIRPLANE_MODE_ON, 0) != 0;

        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);

        // mPhoneStateListener = new PhoneStateListener[mSimCount];

        // for (int i = 0; i < mSimCount; ++i) {
        //     int subId = getSubId(i);
        //     mTelephonyManager.createForSubscriptionId(subId).listen(getPhoneStateListener(
        //             i, subId), PhoneStateListener.LISTEN_RADIO_POWER_STATE_CHANGED);
        // }

        registerForBroadcast();
        registerIndication();

        setupCallbacksForWifiStatus();

        // sync status at the first time
        setWifiEnabled();
    }

    private void initWifiManager() {
        if (mWifiManager != null) {
            return;
        }

        Rlog.d(TAG, "initWifiManager.");
        // prevent ServiceNotFoundException
        if (!StorageManager.inCryptKeeperBounce()) {
            mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        }
        if (mWifiManager != null) {
            boolean isWifiEnabled = mWifiManager.isWifiEnabled();
            if (isWifiEnabled != mIsWifiEnabled) {
                mIsWifiEnabled = isWifiEnabled;
                setWifiEnabled();
            }
            mWifiLock = mWifiManager.createWifiLock("WifiOffloadService-Wifi Lock");
            if (mWifiLock != null) {
                mWifiLock.setReferenceCounted(false);
            }
        } else {
            log("WifiManager null");
            mIsWifiEnabled = false;
            mWifiLock = null;
            Message msg = obtainMessage(EVENT_RETRY_INIT);
            synchronized (mLock) {
                mPendingMsgs.add(msg);
            }
        }
    }

    public boolean isWifiConnected() {
        return mIsWifiConnected;
    }

    private void registerForBroadcast() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        log("register for radio state changed");
        filter.addAction(TelephonyIntents.ACTION_RADIO_STATE_CHANGED);
        filter.addAction(WifiManager.WIFI_SCAN_AVAILABLE);
        filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        mContext.registerReceiver(mReceiver, filter);
    }

    private void registerIndication() {
        for(int i = 0 ; i < mSimCount ; i++) {
            mMwiRil[i].registerRssiThresholdChanged(this,
                    EVENT_ON_WIFI_MONITORING_THRESHOLD_CHANGED, null);
            mMwiRil[i].registerWifiPdnActivated(this,
                    EVENT_ON_WIFI_PDN_ACTIVATE, null);
            mMwiRil[i].registerNattKeepAliveChanged(this,
                    EVENT_ON_NATT_KEEP_ALIVE_CHANGED, null);
            mMwiRil[i].registerWifiPingRequest(this,
                    EVENT_ON_WIFI_PING_REQUEST, null);
            //TODO: porting the interface
            mMwiRil[i].registerWifiLock(this,
                    EVENT_ON_WIFI_LOCK, null);
        }
    }


    private void onWifiMonitoringThreshouldChanged(Message msg) {

        AsyncResult ar = (AsyncResult) msg.obj;
        int[] result = (int[]) ar.result;

        if (null == result) {
            Rlog.e(TAG, "onWifiMonitoringThreshouldChanged(): result is null");
            return;
        }

        boolean enable = (result[0] == 1);
        int length = result.length;
        int simIdx = result[length - 1]; // last params

        if (!enable) {
            log("Turn off RSSI monitoring");
            mRssiMonitoringProcessor.unregisterAllRssiMonitoring(simIdx);
            return;
        }
        int count = result[1];
        if ((2 + count + 1) < length) { // <enable>,<count>,<rssi_1>,<rssi_2>,...,<simIdx>
            Rlog.e(TAG, "onWifiMonitoringThreshouldChanged(): Bad params");
            return;
        }
        if (checkInvalidSimIdx(simIdx, "onWifiMonitoringThreshouldChanged: invalid SIM id")) {
            return;
        }

        int[] rssi = new int[count];
        int offset = 2;
        for (int i = 0; i < count; ++i) {
            rssi[i] = result[i + offset];
            log("onWifiMonitoringThreshouldChanged(): rssi = " + rssi[i]);
        }

        onRssiMonitorRequest(simIdx, count, rssi);

        // Notify IWLAN the current RSSI as IWLAN request
        updateLastRssi();
        setWifiSignalLevel();
    }

    private void onWifiPdnActivate(Message msg) {

        AsyncResult ar = (AsyncResult) msg.obj;
        int[] result = (int[]) ar.result;
        boolean preWifiPdnExited = false;

        if (null == result) {
            Rlog.e(TAG, "onWifiPdnActivate(): result is null");
            return;
        }
        if (result.length < 2) {
            Rlog.e(TAG, "onWifiPdnActivate(): Bad params");
            return;
        }
        int pdnCount = result[0];
        int simIdx = result[1];

        if (checkInvalidSimIdx(simIdx, "onWifiPdnActivate(): invalid SIM id")) {
            return;
        }
        preWifiPdnExited =  isWifiPdnExisted();

        if (SystemProperties.getInt(PROPERTY_MIMS_SUPPORT, 0) < 2) {
            log("MIMS does not support, sync up pdn status to all slots.");
            for (int i = 0; i < mSimCount; i++) {
                mWifiPdnExisted[i] = (pdnCount > 0);
            }
        } else {
            log("MIMS supported, update pdn status to specific slot[" + simIdx + "].");
            mWifiPdnExisted[simIdx] = (pdnCount > 0);
        }

        // Check if start wifi scan to improve MOBIKE
        checkIfstartWifiScan(false);

        if (preWifiPdnExited != isWifiPdnExisted()) {
            //handling the WiFi deffer Off
            handleWifiDefferOff(WifiLockSource.MODEM_STATE_CHANGE, 0);
        }
    }

    private boolean isWifiDeferOffNeeded () {
        return (isWifiPdnExisted() || isModemReqWifiLock());
    }

    private void handleWifiDefferOff (WifiLockSource source, int state) {

        /*
        log ("old handleWifiDefferOff(): WifiLockSource: "+source
                + " state: " + state
                + " mHasWiFiDisabledPending: " + mHasWiFiDisabledPending
                + " isWifiDeferOffNeeded(): " + isWifiDeferOffNeeded()
                + " mDeferredNotificationToWifi: " + mDeferredNotificationToWifi
                );
        */
        switch (source) {

            case MODEM_STATE_CHANGE:
                if (!mDeferredNotificationToWifi) {
                    if (isWifiDeferOffNeeded()) {
                        broadcastWfcStatusIntent(NEED_DEFER);
                        if (mWifiLock != null) {
                            mWifiLock.acquire();
                            mWifiLockCount ++;
                            mWifiManager.registerStaStateCallback(mStaStateCallback, this);
                        }
                        mDeferredNotificationToWifi = true;
                    }
                }

                if (mHasWiFiDisabledPending) {
                    if (!isWifiDeferOffNeeded()) {
                        mHasWiFiDisabledPending = false;
                        broadcastWfcStatusIntent(WFC_NOTIFY_GO);
                    }
                }

                if (mDeferredNotificationToWifi) {
                    if (!isWifiDeferOffNeeded()) {
                        mDeferredNotificationToWifi = false;
                        broadcastWfcStatusIntent(NO_NEED_DEFER);
                        if (mWifiLock != null) {
                            mWifiLock.release();
                            mWifiManager.unregisterStaStateCallback(mStaStateCallback);
                            mWifiLockCount --;
                            if (mWifiLockCount >0) {
                                log("Warning: mWifiLockCount: "+ mWifiLockCount);
                            }
                        }
                    }
                }

                break;

            case WIFI_STATE_CHANGE:
                if (state == WIFI_STATE_UI_DISABLING) {
                    if (!mHasWiFiDisabledPending) {
                        if (isWifiDeferOffNeeded()) {
                            mHasWiFiDisabledPending = true; //wait for modem state change
                        } else {
                            broadcastWfcStatusIntent(WFC_NOTIFY_GO);
                        }
                    }
                } else if (state == WifiManager.WIFI_STATE_DISABLED) {
                    //wifi framework may turn off WiFi due to timeout, 3s.
                    if (mHasWiFiDisabledPending)
                        mHasWiFiDisabledPending = false; //reset
                    if (mDeferredNotificationToWifi) {
                        mDeferredNotificationToWifi = false; //reset
                        log("Wi-Fi fwk automaticlly disable defer Wi-Fi off process due to timeout");
                        if (mWifiLock != null) {
                            mWifiLock.release();
                            mWifiManager.unregisterStaStateCallback(mStaStateCallback);
                            mWifiLockCount --;
                            if (mWifiLockCount >0) {
                                log("Warning: mWifiLockCount: "+ mWifiLockCount);
                            }
                        }
                    }
                } else if (state == WifiManager.WIFI_STATE_ENABLED) {
                    //need to check if modem request locked and not release yet
                    if (!mDeferredNotificationToWifi) {
                        if (isWifiDeferOffNeeded()) {
                            broadcastWfcStatusIntent(NEED_DEFER);
                            if (mWifiLock != null) {
                                mWifiLock.acquire();
                                mWifiLockCount ++;
                                mWifiManager.registerStaStateCallback(mStaStateCallback, this);
                            }
                            mDeferredNotificationToWifi = true;
                        }
                    }
                }
                break;

            default:
                break;
            }
            log ("new handleWifiDefferOff(): WifiLockSource: "+source
                    + " state: " + state
                    + " mHasWiFiDisabledPending: " + mHasWiFiDisabledPending
                    + " isWifiDeferOffNeeded(): " + isWifiDeferOffNeeded()
                    + " mDeferredNotificationToWifi: " + mDeferredNotificationToWifi
                    );

    }
    private void onNattKeepAliveChanged(Message msg) {

        AsyncResult ar = (AsyncResult) msg.obj;
        String[] result = (String[]) ar.result;

        mPacketKeepAliveProcessor.handleKeepAliveChanged(result);
    }

    private void updateWifiConnectedInfo(int isConnected) {
        boolean changed = false;
        boolean ipAddrChanged = false;

        if (isConnected == 0) {
            if (mWfcHandler != null) {
                mWfcHandler.updatedWifiConnectedStatus(false);
            }

            if (mIsWifiConnected) {
                mIsWifiConnected = false;
                mWifiApMac = "";
                mWifiIpv4Address = "";
                mWifiIpv6Address = "";
                mWifiIpv4Gateway = "";
                mWifiIpv6Gateway = "";
                mWifiIpv4PrefixLen = -1;
                mWifiIpv6PrefixLen = -1;
                mDnsServers = null;
                mIfName = "";
                mSsid = "";
                mMtu = 0;
                changed = true;
                ipAddrChanged = true;
            }
        } else {
            if (mWfcHandler != null) {
                mWfcHandler.updatedWifiConnectedStatus(true);
            }

            String wifiApMac = "", ipv4Address = "", ipv6Address = "", ifName = "", ssid = "";
            int ipv4PrefixLen = -1;
            int ipv6PrefixLen = -1;
            RouteInfo routeIpv4 = null;
            RouteInfo routeIpv6 = null;
            String ipv4Gateway = "";
            String ipv6Gateway = "";

            List<InetAddress> dnsServers = null;
            int mtu = 0;
            mIsWifiConnected = true;

            // Set wifi enable to modem if wifi is enabled by WifiManager and VoWifi is deregistered
            // when wifi is connected again. Foget wifi AP may call setWifiOff() @{
            if (!mIsWifiEnabled && mWifiManager.isWifiEnabled()) {
                mIsWifiEnabled = true;
                setWifiEnabled();
            }

            // get MAC address of the current access point
            WifiInfo wifiInfo = null;
            if (mWifiManager != null) {
                wifiInfo = mWifiManager.getConnectionInfo();
            } else {
                Message msg = obtainMessage(EVENT_RETRY_UPDATE_WIFI_CONNTECTED_INFO,
                        isConnected, 0);
                synchronized (mLock) {
                    mPendingMsgs.add(msg);
                }
            }

            if (wifiInfo != null) {
                wifiApMac = wifiInfo.getBSSID();
                String tempSsid = wifiInfo.getSSID();
                if (NONE_SSID.equals(tempSsid)) {
                    tempSsid = "";
                    log("updateWifiConnectedInfo: <unknown ssid>");
                }
                ssid = updateSsidToHexString(tempSsid);
                //log("updateWifiConnectedInfo(): wifiApMac = " + wifiApMac);
                if (!mWifiApMac.equals(wifiApMac) &&
                        (!TextUtils.isEmpty(mWifiApMac) || (wifiApMac != null))) {
                    mWifiApMac = (wifiApMac == null ? "" : wifiApMac);
                    changed = true;
                    log("updateWifiConnectedInfo(): mWifiApMac = " + mWifiApMac);
                }
            }

            // get ip
            for (Network nw : mConnectivityManager.getAllNetworks()) {
                LinkProperties prop = mConnectivityManager.getLinkProperties(nw);

                // MAL only care about wlan
                if (prop == null || prop.getInterfaceName() == null
                        || !prop.getInterfaceName().startsWith("wlan")) {
                    continue;
                }

                //for (InetAddress address : prop.getAddresses()) {
                for (LinkAddress linkAddr : prop.getLinkAddresses()) {
                    InetAddress address = linkAddr.getAddress();
                    if (address instanceof Inet4Address && !address.isLoopbackAddress()) {
                        ipv4Address = address.getHostAddress();
                        ipv4PrefixLen = linkAddr.getPrefixLength();
                    } else if (address instanceof Inet6Address && !address.isLinkLocalAddress()
                               && !address.isLoopbackAddress()) {
                        // Filters out link-local address. If cannot find non-link-local address,
                        // pass empty string to MAL.
                        ipv6Address = address.getHostAddress();
                        ipv6PrefixLen = linkAddr.getPrefixLength();
                    } else {
                        /*
                        log("ignore address= " + address +
                               " isLoopbackAddr: " + address.isLinkLocalAddress());
                         */
                    }
                }

                dnsServers = prop.getDnsServers();
                mtu = prop.getMtu();

                for (RouteInfo routeInfo: prop.getRoutes()) {
                    if (routeInfo.isIPv4Default()){
                        routeIpv4 = routeInfo;
                        ipv4Gateway = (routeIpv4.getGateway()!=null)
                                ? routeIpv4.getGateway().getHostAddress() : "";
                    }else if (routeInfo.isIPv6Default()){
                        routeIpv6 = routeInfo;
                        ipv6Gateway = (routeIpv6.getGateway()!=null)
                                ? routeIpv6.getGateway().getHostAddress() : "";

                    }
                }

                ifName = prop.getInterfaceName();
            }

            if (!mWifiIpv4Address.equals(ipv4Address)) {
                mWifiIpv4Address = (TextUtils.isEmpty(ipv4Address)) ? "" : ipv4Address;
                mWifiIpv4PrefixLen = ipv4PrefixLen;
                mWifiIpv4Gateway = ipv4Gateway;
                ipAddrChanged = true;
                log("updateWifiConnectedInfo():"
                        + " mWifiIpv4Address = " + maskString(mWifiIpv4Address));
            }
            if (!mWifiIpv6Address.equals(ipv6Address)) {
                mWifiIpv6Address = (TextUtils.isEmpty(ipv6Address)) ? "" : ipv6Address;
                mWifiIpv6PrefixLen = ipv6PrefixLen;
                mWifiIpv6Gateway = ipv6Gateway;
                ipAddrChanged = true;
                log("updateWifiConnectedInfo():"
                        + " mWifiIpv6Address = " + maskString(mWifiIpv6Address));
            }
            //log("updateWifiConnectedInfo(): ifName = " + ifName);
            if (!mIfName.equals(ifName)) {
                mIfName = (ifName == null ? "" : ifName);
                changed = true;
                log("updateWifiConnectedInfo(): mIfName = " + mIfName);
            }
            //log("updateWifiConnectedInfo(): ssid = " + ssid);
            if (!mSsid.equals(ssid)) {
                mSsid = (ssid == null ? "" : ssid);
                changed = true;
                log("updateWifiConnectedInfo(): mSsid = " + mSsid);
            }
            //log("updateWifiConnectedInfo(): dnsServers =" + dnsServers);
            if (dnsServers != null) {
                if (mDnsServers == null) {
                    mDnsServers = dnsServers;
                    ipAddrChanged = true;
                    log("updateWifiConnectedInfo(): mDnsServers =" + mDnsServers);
                } else {
                    List<InetAddress> common = new ArrayList<InetAddress>(mDnsServers);
                    common.retainAll(dnsServers);
                    if (common.size() != dnsServers.size()) {
                        ipAddrChanged = true;
                        mDnsServers = dnsServers;
                        log("updateWifiConnectedInfo(): mDnsServers =" + mDnsServers);
                    }
                }
            }
            //log("updateWifiConnectedInfo(): mtu = " + mtu);
            if ((mtu >= 0) && (mMtu != mtu)) {
                mMtu = mtu;
                changed = true;
                log("updateWifiConnectedInfo(): mMtu = " + mMtu);
            }
        }

        // Check if start wifi scan to improve MOBIKE
        checkIfstartWifiScan(false);

        if (changed) {
            setWifiAssoc();
        }

        if (ipAddrChanged) {
            setWifiIpAddress();
        }
    }

    private int generateWifiEnableCause(boolean needAPMode, boolean apMode,
            boolean needWifiEnabled, boolean wifiEnabled, boolean bCause) {
        int cause = 0;

        if (needAPMode) {
            cause = cause | EWIFIEN_NEED_SEND_AP_MODE;
        }
        cause = cause | (apMode ? (EWIFIEN_AP_MODE_STATE) : 0);

        if (needWifiEnabled) {
            cause = cause | EWIFIEN_NEED_SEND_WIFI_ENABLED;
        }
        cause = cause | (wifiEnabled ? (EWIFIEN_WIFI_ENABLED_STATE) : 0);

        cause = cause | (bCause ? (EWIFIEN_CAUSE) : 0);

        log("generateWifiEnableCause(): " + cause);
        return cause;
    }

    private void setWifiEnabled() {
        Message result = obtainMessage(RESPONSE_SET_WIFI_ENABLED);
        boolean isInEcc = isEccInProgress();

        try {
            boolean isAirplaneModeOn = getAirplaneModeFromSettings();
            if (mIsAirplaneModeOn != isAirplaneModeOn) {
                mIsAirplaneModeOn = isAirplaneModeOn;
                mIsAirplaneModeChange = true;
                log("setWifiEnabled(): update mIsAirplaneModeOn from settings");
            }
        } catch (SettingNotFoundException e) {
            Rlog.e(TAG, "Can not get AIRPLANE_MODE_ON from provider.");
        }

        log("setWifiEnabled(): " + mIsWifiEnabled +
            ", mIsAirplaneModeOn: " + mIsAirplaneModeOn +
            ", isEccInProgress: " + isInEcc);

        boolean wifiEnable = mIsWifiEnabled;
        if (!allowWfcInAirplaneMode()) {
            log("allowWfcInAirplaneMode: false");
            wifiEnable = false;
        }

        boolean bCause = (SystemProperties.getInt(PROPERTY_WFC_ENABLE, 0) >= 1 &&
                mIsAirplaneModeOn && !isInEcc) ? true : false;
        int cause = generateWifiEnableCause(false, mIsAirplaneModeOn, true, wifiEnable, bCause);

        getMwiRil().setWifiEnabled(
                WIFI_IF_NAME, (wifiEnable) ? 1 : 0, cause, result);
    }

    private void setWifiEnabledWithSyncAPMode() {
        Message result = obtainMessage(RESPONSE_SET_WIFI_ENABLED);
        boolean isInEcc = isEccInProgress();

        try {
            boolean isAirplaneModeOn = getAirplaneModeFromSettings();
            if (mIsAirplaneModeOn != isAirplaneModeOn) {
                mIsAirplaneModeOn = isAirplaneModeOn;
                log("setWifiEnabledWithSyncAPMode(): update mIsAirplaneModeOn from settings");
            }
        } catch (SettingNotFoundException e) {
            Rlog.e(TAG, "Can not get AIRPLANE_MODE_ON from provider.");
        }

        log("setWifiEnabledWithSyncAPMode(): " + mIsWifiEnabled +
            ", mIsAirplaneModeOn: " + mIsAirplaneModeOn +
            ", isEccInProgress: " + isInEcc);

        boolean bCause = (SystemProperties.getInt(PROPERTY_WFC_ENABLE, 0) >= 1 &&
                mIsAirplaneModeOn && !isInEcc) ? true : false;

        int cause = generateWifiEnableCause(true, mIsAirplaneModeOn, true, mIsWifiEnabled, bCause);

        getMwiRil().setWifiEnabled(
                WIFI_IF_NAME, (mIsWifiEnabled) ? 1 : 0, cause, result);
    }

    private boolean allowWfcInAirplaneMode() {
        boolean wfcInApMode = true;
        if (mIsAirplaneModeOn) {
            if (SystemProperties.getInt(MtkImsConstants.MULTI_IMS_SUPPORT, 1) <= 1)  {
                wfcInApMode = getBooleanCarrierConfig(
                    MTK_KEY_WOS_SUPPORT_WFC_IN_FLIGHTMODE,
                    getMainCapabilityPhoneId(), true);
            } else {
                for (int i = 0; i < mSimCount; i++) {
                    wfcInApMode = getBooleanCarrierConfig(
                            MTK_KEY_WOS_SUPPORT_WFC_IN_FLIGHTMODE, i, true);
                    if (wfcInApMode == false) {
                        break;
                    }
                }
            }
        }

        log("allowWfcInAirplaneMode: mIsAirplaneModeOn = " + mIsAirplaneModeOn +
            ", wfcInApMode = " + wfcInApMode);
        return wfcInApMode;
    }

    private boolean getBooleanCarrierConfig(String key, int phoneId, boolean defaultValue) {
        if (mContext == null) {
            Rlog.e(TAG, "getBooleanCarrierConfig fail, mContext = null");
        }
        CarrierConfigManager configManager = (CarrierConfigManager) mContext.getSystemService(
                Context.CARRIER_CONFIG_SERVICE);
        int subId = getSubId(phoneId);
        log("getBooleanCarrierConfig: phoneId=" + phoneId + " subId=" + subId);
        PersistableBundle b = null;
        if (configManager != null) {
            b = configManager.getConfigForSubId(subId);
        }
        boolean ret = true;
        if (b != null) {
            ret = b.getBoolean(key, defaultValue);
        } else {
            // Return static default defined in CarrierConfigManager.
            log("getBooleanCarrierConfig: get from default config");
            ret = CarrierConfigManager.getDefaultConfig().getBoolean(key, defaultValue);
        }
        log("getBooleanCarrierConfig sub: " + subId + " key: " + key + " ret: " + ret);
        return ret;
    }

    private int getSubId(int phoneId) {
        int[] subIds = SubscriptionManager.getSubId(phoneId);
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        if (subIds != null && subIds.length >= 1) {
            subId = subIds[0];
        }
        return subId;
    }

    public void setWifiOff() {
        mIsWifiEnabled = false;
        setWifiEnabled();
    }

    private void setWifiSignalLevel() {
        Message result = obtainMessage(RESPONSE_SET_WIFI_SIGNAL_LEVEL);
        log("setWifiSignalLevel(): " + mLastRssi);
        getMwiRil().setWifiSignalLevel(mLastRssi, SNR_UNKNOWN, result);
    }

    private void setWifiAssoc() {
        Message result = obtainMessage(RESPONSE_SET_WIFI_ASSOC);
        int mtu = mMtu;
        if (mtu == 0) {
            //log("setWifiAssoc(): mtu=0");
            mtu = DEFAULT_MTU_SIZE;
        }
        log("setWifiAssoc() ifName: " + mIfName + " associated: " + mIsWifiConnected +
               " ssid: " + maskString(mSsid) + " apMac: " + maskString(mWifiApMac) + ", mtu = " + mtu);
        boolean isWifiConnected = mIsWifiConnected;
        if (TextUtils.isEmpty(mSsid)) {
            isWifiConnected = false;
        }
        getMwiRil().setWifiAssociated(
            WIFI_IF_NAME, isWifiConnected, mSsid, mWifiApMac, mtu, mWifiUeMac, result);

    }

    private void setWifiIpAddress() {
        Message result = obtainMessage(RESPONSE_SET_WIFI_IP_ADDR);
        log("setWifiIpAddr() ifName: " + mIfName + " ipv4Addr: " + maskString(mWifiIpv4Address) +
               " ipv6Addr: " + maskString(mWifiIpv6Address) +
               " ipv4PrefixLen: " + mWifiIpv4PrefixLen +
               " ipv6PrefixLen: " + mWifiIpv6PrefixLen );
        if (mDnsServers != null) {
            int dnsCount = mDnsServers.size();
            StringBuilder dnsServers = new StringBuilder();
            String dnsServerAddress;
            for (InetAddress address : mDnsServers) {
                if (dnsServers.length() > 0) {
                    dnsServers.append(",");
                }
                dnsServerAddress = address.getHostAddress();
                dnsServers.append("\"");
                dnsServers.append(dnsServerAddress);
                dnsServers.append("\"");
                log("setWifiIpAddress(): dnsServerAddress: " + dnsServerAddress);
            }

            getMwiRil().setWifiIpAddress(
                    WIFI_IF_NAME, mWifiIpv4Address, mWifiIpv6Address,
                    mWifiIpv4PrefixLen,
                    mWifiIpv6PrefixLen,
                    mWifiIpv4Gateway,
                    mWifiIpv6Gateway,
                    dnsCount,
                    dnsServers.toString(), result);
        } else {
            Rlog.e(TAG, "setWifiIpAddress(): mDnsServers = null");
            getMwiRil().setWifiIpAddress(
                    WIFI_IF_NAME, mWifiIpv4Address, mWifiIpv6Address,
                    mWifiIpv4PrefixLen,
                    mWifiIpv6PrefixLen,
                    mWifiIpv4Gateway,
                    mWifiIpv6Gateway,
                    0,
                    "\"\"", result);
            //getMwiRil().setWifiIpAddress(
            //    WIFI_IF_NAME, mWifiIpv4Address, mWifiIpv6Address, result);
        }
    }

    private void setNattKeepAliveStatus(Message msg) {
        Message result = obtainMessage(RESPONSE_SET_NATT_KEEP_ALIVE_STATUS);
        KeepAliveConfig config = (KeepAliveConfig) msg.obj;
        getMwiRil().setNattKeepAliveStatus(
            WIFI_IF_NAME, config.isEnabled(),
            config.getSrcIp(), config.getSrcPort(),
            config.getDstIp(), config.getDstPort(), result);
    }

    private void setWifiPingResult(Message msg) {
        Message result = obtainMessage(RESPONSE_SET_WIFI_PING_RESULT);

        //int latency = (int) mPingDns.getPingLatency();
        //int pktLoss = mPingDns.getPacketLoss();
        PingData data = (PingData) msg.obj;
        int latency = (int) data.getPingLatency();
        int pktLoss = data.getPacketLoss();
        int simIdx = msg.arg1;
        int rat = msg.arg2;
        log("setWifiPingResult() latency: = " + latency + ", packetLost: = " + pktLoss);

        mMwiRil[simIdx].setWifiPingResult(rat, latency, pktLoss, result);
    }

    private void setWifiUeMac() {
        Message result = obtainMessage(RESPONSE_SET_WIFI_UE_MAC);
        getMwiRil().setWfcConfig_WifiUeMac(WIFI_IF_NAME, mWifiUeMac, result);
    }

    private void checkIfstartWifiScan(boolean scanImmediately) {

        boolean wifiPdnExisted = isWifiPdnExisted();

        if (!mIsWifiConnected && wifiPdnExisted) {

            if (scanImmediately) {
                log("call WifiManager.startScan()");
                if (mWifiManager != null) {
                    mWifiManager.startScan();
                    return;
                } else {
                    Rlog.e(TAG, "checkIfstartWifiScan(): WifiManager null");
                    Message msg = obtainMessage(EVENT_RETRY_CHECK_IF_START_WIFI_SCAN,
                        (Object) scanImmediately);
                    synchronized (mLock) {
                        mPendingMsgs.add(msg);
                    }
                    return;
                }
            }

            if (!hasMessages(EVENT_WIFI_SCAN)) {
                log("start 3s delay to trigger wifi scan");
                sendMessageDelayed(
                    obtainMessage(EVENT_WIFI_SCAN), WIFI_SCAN_DELAY);
            }
        } else {
            removeMessages(EVENT_WIFI_SCAN);
        }
    }

    /**
     * callback from MD to configure RSSI monitor thresholds.
     */
    private void onRssiMonitorRequest(int simId, int size, int[] rssiThresholds) {
        mRssiMonitoringProcessor.registerRssiMonitoring(simId, size, rssiThresholds);
    }

    /**
     * setup callbacks from ConnectivityService when WiFi is changed.
     */
    private void setupCallbacksForWifiStatus() {
        if (mConnectivityManager == null) {
            log("Unexpected error, mConnectivityManager = null");
            return;
        }

        mConnectivityManager.registerDefaultNetworkCallback(
                new ConnectivityManager.NetworkCallback() {
            /**
             * @param network
             */
            @Override
            public void onAvailable(Network network) {
                NetworkCapabilities nc = mConnectivityManager.getNetworkCapabilities(network);

                if (nc == null) {
                    log("Empty network capability:" + network);
                    return;
                }

                if (!(nc.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) &&
                        nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED))) {
                    log("Without TRANSPORT_WIFI.");
                    if (mIsWifiConnected) {
                        log("TRANSPORT_WIFI lost.");
                        Message msg = obtainMessage(
                                  EVENT_WIFI_NETWORK_STATE_CHANGE,
                                  0,  // isConnected
                                  0, null);
                        sendMessage(msg);
                    }
                    return;
                }

                log("WIFI onAvailable.");

                Message msg = obtainMessage(
                                  EVENT_WIFI_NETWORK_STATE_CHANGE,
                                  1,  // isConnected
                                  0, null);
                sendMessage(msg);

                mNetwork = network;
            }

            /**
             * @param network
             */
            @Override
            public void onLost(Network network) {
                NetworkCapabilities nc = mConnectivityManager.getNetworkCapabilities(network);

                if (nc == null) {
                    if (mNetwork != null && !mNetwork.equals(network)) {
                        log("OnLost with " + network + ", mNetwork: " + mNetwork);
                        return;
                    }
                } else {
                    if (!(nc.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) &&
                            nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED))) {
                        log("OnLost without TRANSPORT_WIFI.");
                        return;
                    }
                }

                log("WIFI onLost.");

                Message msg = obtainMessage(
                                  EVENT_WIFI_NETWORK_STATE_CHANGE,
                                  0,  // isConnected
                                  0, null);
                sendMessage(msg);

                mNetwork = null;
            }

            @Override
            public void onCapabilitiesChanged(final Network network,
                                              final NetworkCapabilities networkCapabilities) {
                if (networkCapabilities == null) {
                    log("NetworkCallback.onCapabilitiesChanged, Capabilities=null");
                    return;
                }

                if (!(networkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) &&
                        networkCapabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED))) {
                    return;
                }

                int rssi = networkCapabilities.getSignalStrength();
                log("NetworkCallback.onCapabilitiesChanged, rssi == " + rssi);
                // The current connected AP is disappeared
                if (rssi == Integer.MIN_VALUE && isWifiPdnExisted()) {

                    log("NetworkCallback.onCapabilitiesChanged," +
                        " request partial scan to switch wifi ap");

                    //mWifiManager.disconnect();
                    //mWifiManager.startScan();

                    //Intent intent = new Intent(WFC_REQUEST_PARTIAL_SCAN);
                    //mContext.sendBroadcast(intent);
                }

                if (!mIsWifiConnected) {
                    Message msg = obtainMessage(
                                      EVENT_WIFI_NETWORK_STATE_CHANGE,
                                      1,  // isConnected
                                      0, null);
                    sendMessage(msg);
                }

                mNetwork = network;

                if (mLastRssi == rssi) {
                    return;
                }
                mLastRssi = rssi;
                sendEmptyMessage(EVENT_SET_WIFI_SIGNAL_STRENGTH);
            }

            /**
             * @param network
             */
            @Override
            public void onLinkPropertiesChanged(Network network,
                                                LinkProperties linkProperties) {
                NetworkCapabilities nc = mConnectivityManager.getNetworkCapabilities(network);

                if (nc == null) {
                    log("onLinkPropertiesChanged Empty network capability:" + network);
                    return;
                }

                if (!(nc.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) &&
                        nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED))) {
                    log("onLinkPropertiesChanged Without TRANSPORT_WIFI.");
                    return;
                }

                log("onLinkPropertiesChanged TRANSPORT_WIFI: " + mIsWifiConnected);

                if (mIsWifiConnected) {
                    // At this timing, goes to check if IP address is updated.
                    Message msg = obtainMessage(
                                      EVENT_WIFI_NETWORK_STATE_CHANGE,
                                      1,  // isConnected
                                      0, null);
                    sendMessage(msg);
                }

                mNetwork = network;
            }
        });
    }

    private boolean checkInvalidSimIdx(int simIdx, String dbgMsg) {
        if (simIdx < 0 || simIdx >= mSimCount) {
            log(dbgMsg);
            return true;
        }
        return false;
    }

    public boolean isWifiPdnExisted() {
        for (int i = 0; i < mSimCount; i++) {
            if (mWifiPdnExisted[i]) {
                log("isWifiPdnExisted: found WiFi PDN on SIM: " + i);
                return true;
            }
        }
        return false;
    }

    private boolean isModemReqWifiLock() {
        for (int i = 0; i < mSimCount; i++) {
            if (mModemReqWifiLock[i]) {
                // log("isModemReqWifiLock: modem ask WiFi lock on SIM: " + i);
                return true;
            }
        }
        return false;
    }

    private void updateLastRssi() {
        if (mWifiManager == null) {
            Rlog.e(TAG, "updateLastRssi(): WifiManager null");
            Message msg = obtainMessage(EVENT_RETRY_UPDATE_LAST_RSSI);
            synchronized (mLock) {
                mPendingMsgs.add(msg);
            }
            return;
        }

        WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
        if (wifiInfo != null) {
            mLastRssi = wifiInfo.getRssi();
        }
    }

    private String updateSsidToHexString(String ssid) {
        if (TextUtils.isEmpty(ssid)) {
            return "";
        }
        // take away the double quote added by android fwk
        if (ssid.startsWith("\"") && ssid.endsWith("\"")) {
            ssid = ssid.substring(1, ssid.length() - 1);
        }

        byte[] bytes = ssid.getBytes();
        StringBuilder str = new StringBuilder();
        for (int i = 0; i < bytes.length; ++i) {
            str.append(String.format("%02x", bytes[i]));
        }

        // log("updateSsidToHexString orig: " + ssid + " after: " + str.toString());
        return str.toString();
    }

    public int getLastRssi() {
        return mLastRssi;
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

    public static String getUeWlanMacAddr() {
        return mWifiUeMac;
    }

    private String updateWlanMacAddr() {
        try {
            NetworkInterface wnif = NetworkInterface.getByName("wlan0");
            if (wnif == null) {
                log("updateWlanMacAddr wnif == null");
                return "";
            } else {
                byte[] macBytes = wnif.getHardwareAddress();
                if (macBytes == null) {
                    log("updateWlanMacAddr macBytes == null");
                    return "";
                }

                StringBuilder res1 = new StringBuilder();
                for (byte b : macBytes) {
                    res1.append(String.format("%02X:",b));
                }

                if (res1.length() > 0) {
                    res1.deleteCharAt(res1.length() - 1);
                }
                return res1.toString();
            }
        } catch (Exception ex) {
        }
        return "02:00:00:00:00:00";
    }

    private void onWifiPingRequest(Message msg) {

        AsyncResult ar = (AsyncResult) msg.obj;
        int[] result = (int[]) ar.result;

        if (null == result) {
            Rlog.e(TAG, "onWifiPingRequest(): result is null");
            return;
        }
        if (result.length < 2) {
            Rlog.e(TAG, "onWifiPingRequest(): Bad params");
            return;
        }
        int rat = result[0];
        int simIdx = result[1];

        if (checkInvalidSimIdx(simIdx, "onWifiPingRequest(): invalid SIM id")) {
            return;
        }

        log("onWifiPingRequest: rat = " + rat + ", simIdx = " + simIdx);
        mPingDns = new PingDns(mContext, simIdx, rat, this);
        mPingDns.pingWifiGateway();
    }

    private void onWifiLock(Message msg) {

        AsyncResult ar = (AsyncResult) msg.obj;
        String[] result = (String[]) ar.result;

        if (null == result) {
            Rlog.e(TAG, "onWifiLock(): result is null");
            return;
        }
        if (result.length < 3) {
            Rlog.e(TAG, "onWifiLock(): Bad params");
            return;
        }

        try {
            String wlanIface = result[0];
            int enableLock = Integer.parseInt(result[1]);
            int simIdx = Integer.parseInt(result[2]);
            boolean preModemWifiLockState = isModemReqWifiLock();
            mModemReqWifiLock[simIdx] = (enableLock == 0) ? false : true;
            if (preModemWifiLockState != isModemReqWifiLock()) {
                handleWifiDefferOff(WifiLockSource.MODEM_STATE_CHANGE, 0);
            }
        } catch (Exception e) {
            Rlog.e(TAG, "onWifiLock[" + result.length + "]"  +  result[0] + " " +
                result[1] + " " +result[2] + " " + " e:" + e.toString());
        }
    }

    private boolean isEccInProgress() {
        boolean isInEcc = false;
        TelecomManager tm = (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
        if (tm != null) {
            isInEcc = tm.isInEmergencyCall();
        }
        return isInEcc;
    }

    protected void log(String s) {
        if (!USR_BUILD || TELDBG) {
            Rlog.d(TAG, s);
        }
    }

    public void setWfcHandler(WfcHandler wfcHandler) {
        mWfcHandler = wfcHandler;
    }

    private void broadcastWfcStatusIntent(int wfcStatus) {
        mWfcHandler.sendMessage(mWfcHandler.obtainMessage(WfcHandler.EVENT_HANDLE_WFC_STATE_CHANGED));
    }
    private String maskString(String s) {
        StringBuilder sb = new StringBuilder();
        if (TextUtils.isEmpty(s)) {
            return s;
        }
        int maskLength = s.length()/2;
        if (maskLength < 1) {
            sb.append("*");
            return (sb.toString());
        }
        for(int i = 0; i < maskLength ; i++) {
            sb.append("*");
        }
        return (sb.toString() + s.substring(maskLength));
    }
}
