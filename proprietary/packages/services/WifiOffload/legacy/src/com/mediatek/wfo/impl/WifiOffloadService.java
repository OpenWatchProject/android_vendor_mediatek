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

import java.io.IOException;
import java.lang.Character;
import java.lang.Exception;
import java.lang.InterruptedException;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Base64;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import java.util.concurrent.CopyOnWriteArraySet;
import java.util.NoSuchElementException;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.location.Address;
import android.location.Geocoder;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.net.ConnectivityManager;
import android.net.INetworkManagementEventObserver;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.net.NetworkRequest;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.HwBinder;
import android.os.INetworkManagementService;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.os.PatternMatcher;
import android.os.PersistableBundle;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.storage.StorageManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

import android.telephony.AccessNetworkConstants;
import android.telephony.AccessNetworkConstants.TransportType;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.NetworkRegistrationInfo.Domain;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.ims.ImsManager;
import com.android.ims.ImsConfig;
import com.android.ims.ImsConfigListener;
import com.android.ims.ImsException;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.server.net.BaseNetworkObserver;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.ims.internal.MtkImsConfig;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.RadioManager;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import com.mediatek.wfo.DisconnectCause;
import com.mediatek.wfo.IWifiOffloadService;
import com.mediatek.wfo.IWifiOffloadListener;
import com.mediatek.wfo.WifiOffloadManager;
import com.mediatek.wfo.util.RssiMonitoringProcessor;

import vendor.mediatek.hardware.wfo.V1_0.IWifiOffload;

import com.mediatek.wfo.op.OpWosCustomizationUtils;
import com.mediatek.wfo.op.OpWosCustomizationFactoryBase;
import com.mediatek.wfo.op.IWosExt;
import mediatek.telephony.MtkCarrierConfigManager;

import java.nio.charset.CharsetDecoder;
import java.nio.charset.Charset;
import java.nio.charset.CharacterCodingException;
import java.nio.CharBuffer;
import java.nio.ByteBuffer;
import java.util.Arrays;

public class WifiOffloadService extends IWifiOffloadService.Stub {

    private class HandoverMessage {
        public int mSimIdx;
        public int mStage;
        public int mRatType;
        public HandoverMessage(int simIdx, int stage, int ratType) {
            mSimIdx = simIdx;
            mStage = stage;
            mRatType = ratType;
        }
    }

    private class RoveOutMessage {
        public int mSimIdx;
        public boolean mRoveOut;
        public int mRssi;
        public RoveOutMessage(int simIdx, boolean roveOut, int rssi) {
            mSimIdx = simIdx;
            mRoveOut = roveOut;
            mRssi = rssi;
        }
    }

    static final String TAG = "WifiOffloadService";

    // Sensitive log task
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean USR_BUILD = TextUtils.equals(Build.TYPE, "user")
            || TextUtils.equals(Build.TYPE, "userdebug");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private RemoteCallbackList<IWifiOffloadListener> mListeners = new
            RemoteCallbackList<IWifiOffloadListener>();

    private Context mContext;
    private WifiManager mWifiManager;
    private ConnectivityManager mConnectivityManager;
    private SubscriptionManager mSubscriptionManager;
    private TelephonyManager mTelephonyManager;
    private CarrierConfigManager mCarrierConfigManager;
    private LocationManager mLocationManager;

    // Indicator for native MAL connection.
    private int mNativeMalState;

    private static final int MAL_STATE_UNAVAILABLE = 0;
    private static final int MAL_STATE_INITIALIZING = 1;
    private static final int MAL_STATE_READY = 2;

    private static final int NETWORK_LOCATION_UPDATE_TIME = 0;

    private static final int MCCMNC_MODE_RESOURCE = 0;
    private static final int MCCMNC_MODE_SYSTEM_PROPERTY = 1;
    private static final int MCCMNC_MODE_BOTH = 2;


    // Wos internal state, should sync with MAL.
    private int mRatType[];
    private DisconnectCause mDisconnectCause[];

    // from user settings
    private boolean mIsVolteEnabled[];
    private boolean mIsWfcEnabled[];
    private boolean mIsVilteEnabled[];
    private boolean mIsAllowTurnOffIms[];
    private int mWfcMode;
    private boolean mIsWifiEnabled;
    private boolean mHasWiFiDisabledPending;

    // for VoWiFi Provisioning utilising SMS
    private String mFqdn[];

    // wifi state
    private boolean mIsWifiConnected = false;
    private boolean mIsWifiL2Connected = false;
    private String mWifiApMac = "";
    private String mWifiIpv4Address = "";
    private String mWifiIpv6Address = "";
    private String mIfName ="";
    private static int WIFI_NO_INTERNET_ERROR_CODE = 1081;
    private static int WIFI_NO_INTERNET_TIMEOUT = 8000;


    // airplane mode state
    private boolean mIsAirplaneModeOn = false;

    // data roaming, service states.
    private int mSimCount;
    private int mDataRoamingEnabled = SubscriptionManager.DATA_ROAMING_DISABLE;
    private boolean[] mIsCurDataRoaming;
    private int[] mRadioTechnology;
    private boolean[] mIsCurVoiceRoaming;
    private int[] mVoiceRadioTechnology;

    private int[] mRadioState;

    private String[] mLocatedPlmn;

    // subId -> PhoneStateListener
    private Map<Integer, PhoneStateListener> mPhoneServicesStateListeners
            = new ConcurrentHashMap<Integer, PhoneStateListener>();

    // for Wos <-> MAL(RDS) connection setup.
    private static int MAL_CONNECTION_SETUP_RETRY_TIMEOUT = 2000;


    private static int WIFI_SCAN_DELAY = 3000; // 3s

    // for Wos -> RadioManager to power on/off modem.
    private final static String RADIO_MANAGER_POWER_ON_MODEM =
            "mediatek.intent.action.WFC_POWER_ON_MODEM";
    private final static String EXTRA_POWER_ON_MODEM_KEY = "mediatek:POWER_ON_MODEM";

    // for Wos -> Wi-Fi fwk to notify defer Wi-Fi disabled action for WFC de-initialization.
    private static final String WFC_STATUS_CHANGED =
            "com.mediatek.intent.action.WFC_STATUS_CHANGED";
    private static final String EXTRA_WFC_STATUS_KEY = "wfc_status";

    private static final int NO_NEED_DEFER = 0;
    private static final int NEED_DEFER = 1;
    private static final int WFC_NOTIFY_GO = 2;

    private int mWifiLockCount = 0;

    private boolean mDeferredNotificationToWifi = false;

    private static final int WIFI_STATE_UI_DISABLING = WifiManager.WIFI_STATE_DISABLING + 9900;

    // for ImsConfig
    private final static String EXTRA_PHONE_ID = "phone_id";

    // System properties
    private static final String PROPERTY_VOLTE_ENABLE = "persist.vendor.mtk.volte.enable";
    private static final String PROPERTY_WFC_ENABLE = "persist.vendor.mtk.wfc.enable";
    private static final String PROPERTY_IMS_VIDEO_ENABLE = "persist.vendor.mtk.ims.video.enable";

    private static final String PROPERTY_MULTI_IMS_SUPPORT = "persist.vendor.mims_support";
    //for easy config
    private static final String PROPERTY_FORCE_ENALBE_WFC_SUPPORTED =
            "persist.enable_wfc_supported";
    private static final String PROPERTY_WFC_MCCMNC_ALLOWLIST = "persist.vendor.mtk.wfc.mccmnc_list";

    private boolean[] mWifiPdnExisted;

    private static final int RADIO_STATE_DEFUALT = -1;

    // Add for keep initmalconnection thread
    Thread mInitThred = null;

     /**
     * @see ServiceState
     * STATE_IN_SERVICE, STATE_OUT_OF_SERVICE, STATE_EMERGENCY_ONLY, STATE_POWER_OFF
     */
    private int[] mDataRegState;
    private int[] mVoiceRegState;

    private WifiManager.WifiLock mWifiLock;

    // To record each SIM's state.
    private String mSimState[];
    private int mActiveSubId[];

    private boolean mIsSimWfcSupported[];

    // for HIDL implementation
    private IWifiOffload mWifiOffload;
    private static final String WFO_HIDL_SERVICE_NAME = "wfo_hidl_service";
    final WifiOffloadDeathRecipient mWifiOffloadDeathRecipient;
    WifiOffloadHIDLCallback mWifiOffloadHIDLCallback;

    // for operater add-on
    private OpWosCustomizationFactoryBase  mWosFactory = null;
    private IWosExt mWosExt = null;

    RssiMonitoringProcessor mRssiMonitoringProcessor;

    // Keep last RSSI record value.
    private int mLastRssi;
    // Sync with RDS, define the SNR_NOINFO as 60
    private static final int SNR_NOINFO = 60;

    private SettingsObserver mSettingsObserver = new SettingsObserver(null);

    private boolean mWifiMgrInitDone = false;

    private ArrayList<Message> mPendingMsgs = new ArrayList<>();
    private Object mLock = new Object();

    private WFOServHandler mHandler;

    enum WifiLockSource {
        WIFI_STATE_CHANGE,
        MODEM_STATE_CHANGE
    }

    private WifiManager.StaStateCallback mStaStateCallback = new  WifiManager.StaStateCallback() {
        @Override
        public void onStaToBeOff()  {
            log("WifiManager.StaStateCallback - onStaToBeOff");
            Message msg = mHandler.obtainMessage(EVENT_WIFI_STATE_CHANGE, WIFI_STATE_UI_DISABLING, 0);
            msg.sendToTarget();
        }
    };

    private class CfgListener extends ImsConfigListener.Stub {
        int mPhoneId;

        CfgListener(int phoneId) {
            mPhoneId = phoneId;
        }

        protected void fetchFeatureValue(int feature, int value) {
             boolean isEnable = (value == ImsConfig.FeatureValueConstants.ON)?
                    true: false;

             int multiIMSSupportNum = SystemProperties.getInt(PROPERTY_MULTI_IMS_SUPPORT, 0);
             Log.d(TAG, "multiIMSSupportNum:" + multiIMSSupportNum);

             switch (feature) {
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE:
                    if (multiIMSSupportNum > 1) {
                        mIsVolteEnabled[mPhoneId] = isEnable;
                    } else {
                        for (int i = 0; i < mSimCount; i++) {
                            mIsVolteEnabled[i] = isEnable;
                        }
                    }
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE:
                    if (multiIMSSupportNum > 1) {
                        mIsVilteEnabled[mPhoneId] = isEnable;
                    } else {
                        for (int i = 0; i < mSimCount; i++) {
                            mIsVilteEnabled[i] = isEnable;
                        }
                    }
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI:

                    if (multiIMSSupportNum > 1) {
                        mIsWfcEnabled[mPhoneId] = isEnable;
                    } else {
                        for (int i = 0; i < mSimCount; i++) {
                            mIsWfcEnabled[i] = isEnable;
                        }
                    }

                    notifyMalWfcSupported(mPhoneId);
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_HANDLE_MODEM_POWER));
                    checkIfShowNoInternetError(false);
                    break;
                default:
                    Log.d(TAG, "onGetFeatureResponse: unknown feature:" + feature);
                    return;
             }
        }

        @Override
        public void onGetFeatureResponse(int feature, int network, int value, int status) {
            if (status == ImsConfig.OperationStatusConstants.FAILED) {
                Log.d(TAG, "onGetFeatureResponse: get feature failed:" + feature);
                return;
            }
            log("onGetFeatureResponse: sim=" + mPhoneId + ", feature=" + feature
                    + ", value=" + value);
            fetchFeatureValue(feature, value);
            mHandler.sendMessage(mHandler.obtainMessage(EVENT_NOTIRY_MAL_USER_PROFILE, new Integer(mPhoneId)));
        }

        @Override
        public void onSetFeatureResponse(int feature, int network, int value, int status) {
        }

        @Override
        public void onGetVideoQuality(int status, int quality) {
        }

        @Override
        public void onSetVideoQuality(int status) {
        }
    }
    private CfgListener mCfgListeners[];

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
                Message msg = mHandler.obtainMessage(EVENT_WIFI_STATE_CHANGE, wifiState, 0);
                msg.sendToTarget();
            } else if (intent.getAction().equals(TelephonyIntents.ACTION_LOCATED_PLMN_CHANGED)) {
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                if (phoneId == -1) {
                    Log.d(TAG, "invalid phoneId: -1, return directly");
                    return;
                }

                mLocatedPlmn[phoneId] = intent.getStringExtra(TelephonyIntents.EXTRA_PLMN);
                notifyMalServiceState(phoneId, getModemIdFromPhoneId(phoneId));
            } else if (intent.getAction().equalsIgnoreCase(
                    TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                if (checkInvalidSimIdx(phoneId, "invalid phoneId: " + phoneId)) return;

                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                if (!SubscriptionManager.isValidSubscriptionId(subId)) {
                    Log.d(TAG, "invalid subId: " + subId + ", return directly");
                    return;
                }

                String simState = intent.getStringExtra((IccCardConstants.INTENT_KEY_ICC_STATE));
                mSimState[phoneId] = simState;
                mActiveSubId[phoneId] = subId;

                log("phoneId: " + phoneId + ", subId:" + subId + ", simState:" + simState);

                notifyMalSimInfo(phoneId);
                notifyMalWfcSupported(phoneId);

                if (simState.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
                    updateIsAllowTurnOffIms(phoneId, subId);
                }

            } else if (intent.getAction().equals(
                    CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                if (checkInvalidSimIdx(phoneId, "invalid phoneId: " + phoneId)) return;

                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, -1);
                if (!SubscriptionManager.isValidSubscriptionId(subId)) {
                    Log.d(TAG, "invalid subId: " + subId + ", return directly");
                    return;
                }

                updateIsAllowTurnOffIms(phoneId, subId);

                handleModemPower();

                mHandler.sendMessage(mHandler.obtainMessage(EVENT_NOTIRY_MAL_USER_PROFILE));

            } else if (intent.getAction().equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
                Parcelable parcelableExtra = intent
                    .getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
                if (null != parcelableExtra) {
                    NetworkInfo networkInfo = (NetworkInfo) parcelableExtra;
                    State state = networkInfo.getState();
                    mIsWifiL2Connected = state == State.CONNECTED;

                    checkIfShowNoInternetError(false);
                }
            } else if (intent.getAction().equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                mIsAirplaneModeOn = intent.getBooleanExtra("state", false);
                handleAirPlaneModeChange();
            } else if (intent.getAction().equals(WifiManager.WIFI_SCAN_AVAILABLE)) {
                int state = intent.getIntExtra(
                    WifiManager.EXTRA_SCAN_AVAILABLE, WifiManager.WIFI_STATE_UNKNOWN);
                log("Receive WIFI_SCAN_AVAILABLE, state: " + state);
                Message msg = mHandler.obtainMessage(EVENT_WIFI_SCAN_AVAILABLE);
                mHandler.sendMessage(msg);
            } else if (intent.getAction().equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                String propKey = intent.getStringExtra(MtkSubscriptionManager.INTENT_KEY_PROP_KEY);
                if (SubscriptionManager.WFC_IMS_MODE.equals(propKey) ||
                        SubscriptionManager.WFC_IMS_ROAMING_MODE.equals(propKey)) {
                    handleWfcModeChange();
                }
            }
        }
    };

    private void handleWfcModeChange() {
        if (mTelephonyManager != null) {
            mWfcMode = ImsManager.getWfcMode(mContext, mTelephonyManager.isNetworkRoaming());
        } else {
            Log.e(TAG, "handleWfcModeChange: telephony manager null");
            mWfcMode = ImsManager.getWfcMode(mContext);
        }
        Log.d(TAG, "handleWfcModeChange: mWfcMode = " + mWfcMode);
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_NOTIRY_MAL_USER_PROFILE));
    }

    private void handleAirPlaneModeChange() {
        Log.d(TAG, "handleAirPlaneModeChange: mIsAirplaneModeOn = " + mIsAirplaneModeOn);
        if (mIsAirplaneModeOn && !allowWfcInAirplaneMode()) {
            try {
                mWifiOffload.nativeSetWifiStatus(false, mIfName,
                        mWifiIpv4Address, mWifiIpv6Address, mWifiApMac);
            } catch (RemoteException e) {
                Log.e(TAG, "Wos handleAirPlaneModeChange Exception:", e);
            }
        }
    }

    private BroadcastReceiver mFeatureValueReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null || intent.getAction() == null) {
                return;
            }

            if (intent.getAction().equals(ImsConfig.ACTION_IMS_FEATURE_CHANGED)) {
                int feature = intent.getIntExtra(ImsConfig.EXTRA_CHANGED_ITEM, -1);
                int phoneId = intent.getIntExtra(EXTRA_PHONE_ID, -1);
                Log.d(TAG,
                    "onRecevied IMS feature changed phoneId: " + phoneId + ", feature: " + feature);

                if (checkInvalidSimIdx(phoneId, "ignore it for invalid SIM id")) return;
                if (checkNullObject(mCfgListeners[phoneId], "no CfgListener")) return;

                ImsManager imsMgr = ImsManager.getInstance(mContext, phoneId);
                if (checkNullObject(imsMgr, "can't get ImsManager")) return;

                try {
                    MtkImsConfig imsCfg = ((MtkImsManager)imsMgr).getConfigInterfaceEx();
                    if (checkNullObject(imsCfg, "can't get ImsConfig")) return;

                    imsCfg.getFeatureValue(
                            feature, getNetworkTypeByFeature(feature), mCfgListeners[phoneId]);
                } catch (ImsException e) {
                    Log.e(TAG, "getFeatureValue has exception: " + e);
                    return;
                }
            }
        }
    };

    private class SettingsObserver extends ContentObserver {
        private final Uri WFC_MODE_URI = Settings.Global
                .getUriFor(android.provider.Settings.Global.WFC_IMS_MODE);
        private final Uri WFC_ROAMING_MODE_URI = Settings.Global
                .getUriFor(android.provider.Settings.Global.WFC_IMS_ROAMING_MODE);

        public SettingsObserver(Handler handler) {
            super(handler);
        }

        private void register() {
            ContentResolver resolver = mContext.getContentResolver();
            resolver.registerContentObserver(WFC_MODE_URI, false, this);
            resolver.registerContentObserver(WFC_ROAMING_MODE_URI, false, this);
        }

        private void unregister() {
            ContentResolver resolver = mContext.getContentResolver();
            resolver.unregisterContentObserver(this);
        }

        @Override
        public void onChange(boolean selfChange) {
            onChange(selfChange, null);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            if (WFC_MODE_URI.equals(uri) || WFC_ROAMING_MODE_URI.equals(uri)) {
                if (mTelephonyManager != null) {
                    mWfcMode = ImsManager.getWfcMode(mContext, mTelephonyManager.isNetworkRoaming());
                } else {
                    Log.e(TAG, "onChange: telephony manager null");
                    mWfcMode = ImsManager.getWfcMode(mContext);
                }
            }

            mHandler.sendMessage(mHandler.obtainMessage(EVENT_NOTIRY_MAL_USER_PROFILE));
        }
    }

    private SubscriptionManager.OnSubscriptionsChangedListener mSubscriptionsChangedlistener =
            new SubscriptionManager.OnSubscriptionsChangedListener() {

            @Override
            public void onSubscriptionsChanged() {
                mHandler.sendMessage(mHandler.obtainMessage(EVENT_SUBSCRIPTION_INFO_CHANGED));
           }
    };

    private class WosLocationListener implements LocationListener {
        private int mSimIdx;

        public WosLocationListener(int simIdx) {
            mSimIdx = simIdx;
        }

        @Override
        public void onLocationChanged(Location location) {
            log("onLocationChanged location: " + location);
            cancelNetworkLocationRequest(mSimIdx);
            if (location == null) {
                return;
            } else {
                Message msg = mHandler.obtainMessage(EVENT_LOCATION_CHANGED, mSimIdx, 0, location);
                mHandler.sendMessage(msg);
            }
        }

        @Override
        public void onProviderDisabled(String provider) {
        }

        @Override
        public void onProviderEnabled(String provider) {
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
        }
    }

    private final HashMap<Integer, WosLocationListener> mLocationListeners =
        new HashMap<Integer, WosLocationListener>();

    // Message codes. See mHandler below.
    private static final int EVENT_WIFI_NETWORK_STATE_CHANGE = 1;
    private static final int EVENT_SUBSCRIPTION_INFO_CHANGED = 2;
    private static final int EVENT_NOTIFY_SERVICE_STATE_CHANGE = 3;

    private static final int EVENT_NATIVE_MAL_CONNECTION_RESET = 4;
    private static final int EVENT_NATIVE_MAL_CONNECTION_READY = 5;

    private static final int EVENT_WIFI_SIGNAL_STRENGTH_CHANGE = 6;

    private static final int EVENT_ON_HANDOVER = 7;
    private static final int EVENT_ON_ROVE_OUT = 8;
    private static final int EVENT_ON_REQUEST_IMS_SWITCH = 9;

    private static final int EVENT_LOCATION_CHANGED = 10;
    private static final int EVENT_REQUEST_LOCATION = 11;
    private static final int EVENT_HANDLE_MODEM_POWER = 12;
    private static final int EVENT_WIFI_SCAN = 13;
    private static final int EVENT_NOTIFY_WIFI_NO_INTERNET = 14;

    private static final int EVENT_RETRY_INIT = 15;
    private static final int EVENT_WIFI_SCAN_AVAILABLE = 16;
    private static final int EVENT_RETRY_CHECK_IF_START_WIFI_SCAN = 17;
    private static final int EVENT_RETRY_UPDATE_WIFI_CONNTECTED_INFO = 18;
    private static final int EVENT_WIFI_STATE_CHANGE = 19;

    private static final int EVENT_NOTIRY_MAL_USER_PROFILE = 20;

    private class WFOServHandler extends Handler {
        public WFOServHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            log("handleMessage: " + messageToString(msg) + " = " + msg);
            switch (msg.what) {
                case EVENT_WIFI_NETWORK_STATE_CHANGE:
                    updateWifiConnectedInfo(msg.arg1);
                    break;
                case EVENT_SUBSCRIPTION_INFO_CHANGED:
                    updateServiceStateListeners();
                    updateDataRoamingSetting();
                    break;
                case EVENT_NOTIFY_SERVICE_STATE_CHANGE:
                    updateServiceState(msg.arg1, (ServiceState)msg.obj);
                    updateVoiceState(msg.arg1, (ServiceState)msg.obj);
                    break;
                case EVENT_NATIVE_MAL_CONNECTION_RESET:
                    mNativeMalState = MAL_STATE_UNAVAILABLE;
                    initMalConnection();
                    break;
                case EVENT_NATIVE_MAL_CONNECTION_READY:
                    mNativeMalState = MAL_STATE_READY;

                    notifyMalWifiState();

                    for (int i = 0; i < mSimCount; i++) {
                        mFqdn[i] = getProvisioningFqdn();
                        notifyMalUserProfile(i);
                        notifyMalServiceState(i, getModemIdFromPhoneId(i));
                        notifyMalVoiceState(i, getModemIdFromPhoneId(i));
                        notifyMalSimInfo(i);
                        notifyMalRadioState(i);
                        notifyMalWfcSupported(i);
                    }
                    break;
                case EVENT_WIFI_SIGNAL_STRENGTH_CHANGE:
                    int rssi = msg.arg1;
                    int snr = SNR_NOINFO;
                    notifyMalWifiQuality(rssi, snr);
                    break;
                case EVENT_ON_HANDOVER:
                    HandoverMessage hoMsg = (HandoverMessage)msg.obj;
                    notifyOnHandover(hoMsg.mSimIdx, hoMsg.mStage, hoMsg.mRatType);
                    break;
                case EVENT_ON_ROVE_OUT:
                    RoveOutMessage roMsg = (RoveOutMessage)msg.obj;
                    notifyOnRoveOut(roMsg.mSimIdx, roMsg.mRoveOut, roMsg.mRssi);
                    break;
                case EVENT_ON_REQUEST_IMS_SWITCH:
                    int simIdx = msg.arg1;
                    boolean isImsOn = msg.arg2 == 1;
                    notifyOnRequestImsSwitch(simIdx, isImsOn);
                    break;
                case EVENT_LOCATION_CHANGED:
                    Location location = (Location)msg.obj;
                    int simId = msg.arg1;
                    String countryId = getCountryIdFromLocation(location);
                    if (countryId == null) {
                        Log.d(TAG, "onRequestLocationInfo countryId is null");
                    } else {
                        if (checkHidlService()) {
                            try {
                                mWifiOffload.nativeSetLocationInfo(
                                    (new Integer(simId)).byteValue(), countryId);
                            } catch (RemoteException e){
                                Log.e(TAG, "Wos HIDL Exception:", e);
                            }
                        }
                    }
                    break;
                case EVENT_REQUEST_LOCATION:
                    int simIndex = msg.arg1;
                    requestLocationFromNetwork(simIndex);
                    break;
                case EVENT_HANDLE_MODEM_POWER:
                    handleModemPower();
                    break;
                case EVENT_WIFI_SCAN:

                    // Check if start wifi scan to improve MOBIKE
                    checkIfstartWifiScan(true);
                    break;
                case EVENT_RETRY_CHECK_IF_START_WIFI_SCAN: {
                    boolean scanImmediately = (boolean) msg.obj;
                    log("Retry checkIfstartWifiScan, scanImmediately: " + scanImmediately);
                    checkIfstartWifiScan(scanImmediately);
                    break;
                }
                case EVENT_NOTIFY_WIFI_NO_INTERNET:
                    checkIfShowNoInternetError(true);
                    break;
                case EVENT_WIFI_SCAN_AVAILABLE:{
                    synchronized (mLock) {
                        for (Message retryMsg: mPendingMsgs) {
                            retryMsg.sendToTarget();
                        }
                        mPendingMsgs.clear();
                    }
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
                case EVENT_WIFI_STATE_CHANGE: {
                    handleWifiStateChange(msg.arg1);
                    break;
                }
                case EVENT_NOTIRY_MAL_USER_PROFILE: {
                    Integer sim_id = (Integer) msg.obj;
                    if (sim_id == null) {
                        notifyMalUserProfile();
                    } else {
                        notifyMalUserProfile(sim_id);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        private void handleWifiStateChange(int wifiState) {
            if (mWifiManager == null) {
                log("Unexpected error, mWifiManager is null!");
                Message msg = mHandler.obtainMessage(EVENT_WIFI_STATE_CHANGE, wifiState, 0);
                synchronized (mLock) {
                    mPendingMsgs.add(msg);
                }
                return;
            }

            log("handleWifiStateChange wifiState: " + wifiState);
            handleWifiDefferOff(WifiLockSource.WIFI_STATE_CHANGE, wifiState);

            if (wifiState == WIFI_STATE_UI_DISABLING) {
                mIsWifiEnabled = false;
                mHandler.sendMessage(mHandler.obtainMessage(EVENT_NOTIRY_MAL_USER_PROFILE));
            } else {
                boolean isWifiEnabled = mWifiManager.isWifiEnabled();
                if (isWifiEnabled != mIsWifiEnabled) {
                    mIsWifiEnabled = isWifiEnabled;
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_NOTIRY_MAL_USER_PROFILE));
                }

                mHandler.sendMessage(mHandler.obtainMessage(EVENT_HANDLE_MODEM_POWER));
            }
        }

        private String messageToString(Message msg) {
            switch (msg.what) {
                case EVENT_WIFI_NETWORK_STATE_CHANGE:
                    return "EVENT_WIFI_NETWORK_STATE_CHANGE";
                case EVENT_SUBSCRIPTION_INFO_CHANGED:
                    return "EVENT_SUBSCRIPTION_INFO_CHANGED";
                case EVENT_NOTIFY_SERVICE_STATE_CHANGE:
                    return "EVENT_NOTIFY_SERVICE_STATE_CHANGE";
                case EVENT_NATIVE_MAL_CONNECTION_RESET:
                    return "EVENT_NATIVE_MAL_CONNECTION_RESET";
                case EVENT_NATIVE_MAL_CONNECTION_READY:
                    return "EVENT_NATIVE_MAL_CONNECTION_READY";
                case EVENT_WIFI_SIGNAL_STRENGTH_CHANGE:
                    return "EVENT_WIFI_SIGNAL_STRENGTH_CHANGE";
                case EVENT_ON_HANDOVER:
                    return "EVENT_ON_HANDOVER";
                case EVENT_ON_ROVE_OUT:
                    return "EVENT_ON_ROVE_OUT";
                case EVENT_ON_REQUEST_IMS_SWITCH:
                    return "EVENT_ON_REQUEST_IMS_SWITCH";
                case EVENT_LOCATION_CHANGED:
                    return "EVENT_LOCATION_CHANGED";
                case EVENT_REQUEST_LOCATION:
                    return "EVENT_REQUEST_LOCATION";
                case EVENT_HANDLE_MODEM_POWER:
                    return "EVENT_HANDLE_MODEM_POWER";
                case EVENT_WIFI_SCAN:
                    return "EVENT_WIFI_SCAN";
                case EVENT_NOTIFY_WIFI_NO_INTERNET:
                    return "EVENT_NOTIFY_WIFI_NO_INTERNET";
                case EVENT_RETRY_INIT:
                    return "EVENT_RETRY_INIT";
                case EVENT_WIFI_SCAN_AVAILABLE:
                    return "EVENT_WIFI_SCAN_AVAILABLE";
                case EVENT_RETRY_CHECK_IF_START_WIFI_SCAN:
                    return "EVENT_RETRY_CHECK_IF_START_WIFI_SCAN";
                case EVENT_RETRY_UPDATE_WIFI_CONNTECTED_INFO:
                    return "EVENT_RETRY_UPDATE_WIFI_CONNTECTED_INFO";
                case EVENT_WIFI_STATE_CHANGE:
                    return "EVENT_WIFI_STATE_CHANGE";
                case EVENT_NOTIRY_MAL_USER_PROFILE:
                    return "EVENT_NOTIRY_MAL_USER_PROFILE";
                default:
                    return "UNKNOWN";
            }
        }
    };

    public WifiOffloadService(Context context) {
        mContext = context;


        // init wfo service handler
        HandlerThread mHandlerThread = new HandlerThread("WFOServiceHandlerThread");
        mHandlerThread.start();
        mHandler = new WFOServHandler(mHandlerThread.getLooper());

        // register for WFC settings
        mContext.getContentResolver().registerContentObserver(
            Settings.Global.getUriFor(Settings.Global.WFC_IMS_ENABLED), false, mContentObserver);

        initWifiManager();

        mConnectivityManager = ConnectivityManager.from(mContext);
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);

        mSubscriptionManager = SubscriptionManager.from(mContext);
        mSubscriptionManager.addOnSubscriptionsChangedListener(mSubscriptionsChangedlistener);

        mTelephonyManager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        mLocationManager = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);
        if (mTelephonyManager != null) {
            mWfcMode = ImsManager.getWfcMode(mContext, mTelephonyManager.isNetworkRoaming());
        } else {
            Log.e(TAG, "WOS: telephony manager null");
            mWfcMode = ImsManager.getWfcMode(mContext);
        }

        mDataRoamingEnabled = getDataRoamingSetting();
        mWifiOffloadDeathRecipient = new WifiOffloadDeathRecipient();
        mWifiOffloadHIDLCallback = new WifiOffloadHIDLCallback(this);
        mRssiMonitoringProcessor = new RssiMonitoringProcessor(mConnectivityManager);

        // for operator add-on
        mWosFactory = OpWosCustomizationUtils.getOpFactory(mContext);
        mWosExt = mWosFactory.makeWosExt(mContext);

        if (mTelephonyManager != null) {
            mSimCount = mTelephonyManager.getSimCount();
            mIsCurDataRoaming = new boolean[mSimCount];
            mRadioTechnology = new int[mSimCount];
            mDataRegState = new int[mSimCount];
            mIsCurVoiceRoaming = new boolean[mSimCount];
            mVoiceRadioTechnology = new int[mSimCount];
            mVoiceRegState = new int[mSimCount];
            mLocatedPlmn = new String[mSimCount];
            mIsVolteEnabled = new boolean[mSimCount];
            mIsWfcEnabled = new boolean[mSimCount];
            mIsVilteEnabled = new boolean[mSimCount];
            mIsAllowTurnOffIms = new boolean[mSimCount];
            mFqdn= new String[mSimCount];
            mWifiPdnExisted = new boolean[mSimCount];
            mRatType = new int[mSimCount];
            mDisconnectCause = new DisconnectCause[mSimCount];

            mCfgListeners = new CfgListener[mSimCount];
            for (int i = 0; i < mSimCount; i++) {
                mCfgListeners[i] = new CfgListener(i);
                mFqdn[i] = "";
                // Carrier config may be not loaded, need set true as default value.
                mIsAllowTurnOffIms[i] = true;
            }

            mRadioState = new int[mSimCount];
            Arrays.fill(mRadioState, RADIO_STATE_DEFUALT);

            mSimState = new String[mSimCount];
            mActiveSubId = new int[mSimCount];

            mRssiMonitoringProcessor.initialize(mSimCount);

            mIsSimWfcSupported = new boolean[mSimCount];
        } else {
            // ToDo: is it possible?
            Log.d(TAG, "Initialize failed, telephony is null");
        }

        mSettingsObserver.register();
        registerForBroadcast();
        setupCallbacksForWifiStatus();
        setupImsConfigFeatureValueMonitoring();
        updateServiceStateListeners();

        updateFeatureValue();
        initHidlService();
        initMalConnection();

        // If the WoS is died in flightmode and suddenly the Wifi turned on in the WoS died period,
        // power on the modem.
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_HANDLE_MODEM_POWER));

        if (isTestNetwork()) {
            try {
                // Only overridden the default value if there is no default value.
                // Disable CAPTIVE_PORTAL_MODE for test networi.
                int defaultValue2 = Settings.Global.getInt(mContext.getContentResolver(),
                   Settings.Global.CAPTIVE_PORTAL_MODE, -2);
                if (defaultValue2 == -2) {
                    Settings.Global.putInt(mContext.getContentResolver(),
                        Settings.Global.CAPTIVE_PORTAL_MODE, 0);
                }
            } catch (Exception ie) {
                ie.printStackTrace();
            }
        }
        log("Initialize finish");
    }

    public void close() {
        log("close()");
        if (mContext != null) {
            mContext.unregisterReceiver(mReceiver);
        }
        if (checkHidlService()) {
            try {
                mWifiOffload.nativeClose();
            } catch (RemoteException e) {
                Log.e(TAG, "Wos HIDL Exception:", e);
            }
        }
    }

    /**
     * Note: This function should be done in main thread.
     */
    private void initMalConnection() {
        if (mNativeMalState != MAL_STATE_UNAVAILABLE) {
            Log.d(TAG, "initMalConnection: current MAL state is " + mNativeMalState
                    + "return directly");
            return;
        }
        mNativeMalState = MAL_STATE_INITIALIZING;

        if (!checkHidlService()) {
            Log.d(TAG, "initMalConnection: HIDL service is null! ");
            initHidlService();
            mHandler.sendMessageDelayed(mHandler.obtainMessage(EVENT_NATIVE_MAL_CONNECTION_RESET),
                    MAL_CONNECTION_SETUP_RETRY_TIMEOUT);
            return;
        }

        mInitThred = new Thread() {
            public void run() {
                try {
                    log("initMalConnection: enter thread ");
                    while (mWifiOffload.nativeConnectToMal() == 0) {
                        log("nativeConnectToMal return 0, wait for "
                                + MAL_CONNECTION_SETUP_RETRY_TIMEOUT + "ms");
                        Thread.sleep(MAL_CONNECTION_SETUP_RETRY_TIMEOUT);
                    }
                    log("nativeConnectToMal success");
                } catch (InterruptedException e) {
                    Log.d(TAG, "thread interrupt! continue to do MAL init!");
                } catch (RemoteException e) {
                    Log.e(TAG, "Wos HIDL Exception:", e);
                } catch (Exception e) {
                    Log.e(TAG, "Wos HIDL Exception-2:", e);
                }
                mHandler.sendMessage(mHandler.obtainMessage(EVENT_NATIVE_MAL_CONNECTION_READY));
            }
        };
        mInitThred.start();
    }

    private void updateServiceStateListeners() {
        if (mSubscriptionManager == null) {
            Log.d(TAG, "Unexpected error, mSubscriptionManager=null");
            return;
        }
        if (mTelephonyManager == null) {
            Log.d(TAG, "Unexpected error, mTelephonyManager=null");
            return;
        }

        HashSet<Integer> unUsedSubscriptions =
                new HashSet<Integer>(mPhoneServicesStateListeners.keySet());
        final List<SubscriptionInfo> slist = mSubscriptionManager.getActiveSubscriptionInfoList();

        if (slist != null) {
            for (SubscriptionInfo subInfoRecord : slist) {
                int subId = subInfoRecord.getSubscriptionId();
                if (mPhoneServicesStateListeners.get(subId) == null) {
                    // Create listeners for new subscriptions.
                    log("create ServicesStateListener for " + subId);
                    PhoneStateListener listener = new PhoneStateListener() {
                            @Override
                            public void onServiceStateChanged(ServiceState serviceState) {
                                if (serviceState == null) {
                                    Log.d(TAG, "onServiceStateChanged-" + this.mSubId
                                            + ": serviceState is null");
                                    return;
                                }
                                mHandler.sendMessage(
                                mHandler.obtainMessage(EVENT_NOTIFY_SERVICE_STATE_CHANGE,
                                        this.mSubId, 0, serviceState));
                            }
                    };

                    mTelephonyManager.createForSubscriptionId(subId).listen(
                            listener, PhoneStateListener.LISTEN_SERVICE_STATE);
                    mPhoneServicesStateListeners.put(subId, listener);
                } else {
                    // this is still a valid subscription.
                    log("ServicesStateListener-" + subId + " is used.");
                    unUsedSubscriptions.remove(subId);
                }
            }
        }

        for (Integer key: unUsedSubscriptions) {
            log("remove unused ServicesStateListener for " + key);
            mTelephonyManager.listen(mPhoneServicesStateListeners.get(key), 0);
            mPhoneServicesStateListeners.remove(key);
        }
    }

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

    private int getRatTypeFromMal(int simIdx) {
        if (checkInvalidSimIdx(simIdx, "getRatType: invalid SIM:" + simIdx)) {
            return WifiOffloadManager.RAN_TYPE_UNSPEC;
        }

        if (mNativeMalState == MAL_STATE_READY) {
            if (checkHidlService()) {
                try {
                    mRatType[simIdx] =
                        mWifiOffload.nativeGetRatType(
                                (new Integer(simIdx)).byteValue());
                } catch (RemoteException e) {
                    Log.e(TAG, "Wos HIDL Exception:", e);
                }
            }
        } else {
            Log.d(TAG, "getRatType return directly due to MAL isn't ready yet.");
        }

        log("simIdx: " + simIdx + ", rat type is " + mRatType[simIdx]);
        return mRatType[simIdx];
    }

    @Override
    public int getRatType(int simIdx) {
        if (checkInvalidSimIdx(simIdx, "getRatType: invalid SIM:" + simIdx)) {
            return WifiOffloadManager.RAN_TYPE_UNSPEC;
        }

        log("simIdx: " + simIdx + ", rat type is " + mRatType[simIdx]);
        return mRatType[simIdx];
    }

    @Override
    public DisconnectCause getDisconnectCause(int simIdx) {
        if (checkInvalidSimIdx(simIdx, "getDisconnectCause: invalid SIM:" + simIdx)) return null;

        log("disconnect cause is " + mDisconnectCause[simIdx]);
        return mDisconnectCause[simIdx];
    }

    @Override
    public boolean isWifiConnected() {
        log("isWifiConnected: " + mIsWifiConnected);
        return mIsWifiConnected;
    }

    @Override
    public void setEpdgFqdn(int simIdx, String fqdn, boolean wfcEnabled) {
        if (checkInvalidSimIdx(simIdx, "setEpdgFqdn: invalid SIM:" + simIdx)) return;

        if (fqdn == null) {
            log("fqdn is null");
            return;
        }

        mFqdn[simIdx] = fqdn;
        mIsWfcEnabled[simIdx] = wfcEnabled;
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_NOTIRY_MAL_USER_PROFILE,
                new Integer(simIdx)));
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_HANDLE_MODEM_POWER));
    }

    @Override
    public void updateCallState(int simIdx, int callId, int callType, int callState) {
        notifyMalCallState(simIdx, callId, callType, callState);
    }

    @Override
    public void updateRadioState(int simIdx, int radioState) {
        mRadioState[simIdx] = radioState;
        notifyMalRadioState(simIdx);
    }

    /*
     * @param allowList Mcc Mnc allowlist array
     * @return true: Systemproperties set success.
     *         false: AllowList is empty or length over the limit of Systemproperties restriction.
     */
    @Override
    public boolean setMccMncAllowList(String[] allowList) {
        Log.d(TAG, "Not support setMccMncAllowList");

        return false;
    }

    /*
     * @param mode: 0 - arrays.xml only|1 - system properties only|2 - both
     * @return Mcc Mnc allow list string array
     */
    @Override
    public String[] getMccMncAllowList(int mode) {
        String[] result = {};
        log("getMccMncAllowList: mode = " + mode);
        switch (mode) {
            case MCCMNC_MODE_RESOURCE:
                result = getMccMncFromResource();
                break;

            case MCCMNC_MODE_SYSTEM_PROPERTY:
                result = getMccMncFromSystemProperty();

                break;
            case MCCMNC_MODE_BOTH:
                String[] fromRes = getMccMncFromResource();
                String[] fromProperties = getMccMncFromSystemProperty();

                int propLength = (fromProperties != null) ? fromProperties.length : 0;

                String[] total = new String[fromRes.length + propLength];
                System.arraycopy(fromRes, 0, total, 0, fromRes.length);

                if (propLength != 0 ) {
                    System.arraycopy(fromProperties, 0, total, fromRes.length, propLength);
                }

                result = total;
                break;
            default:
                break;
        }

        return result;
    }

    @Override
    public void factoryReset() {
        Log.d(TAG, "factoryReset() isn't supported");
    }

    @Override
    public boolean setWifiOff() {
        //Todo: dummy API
        return false;
    }

    private String[] getMccMncFromResource() {
        //return mContext.getResources().getStringArray(R.array.wfcSupportedMccMncList);
        return WifiOffloadServiceResource.wfcSupportedMccMncList;
    }

    private String[] getMccMncFromSystemProperty() {
        String mccMncAllowList = SystemProperties.get(PROPERTY_WFC_MCCMNC_ALLOWLIST, "");
        return validateMccMnc(mccMncAllowList.split(":"));
    }

    private String[] validateMccMnc(String[] allowList) {

        ArrayList<String> validList = new ArrayList<String>();

        for (String mccmnc : allowList) {

            // Check if lenght is 5 or 6
            if (mccmnc.length() != 5 && mccmnc.length() != 6) {
                continue;
            }

            // Check if all digits
            boolean valid = true;
            for (char c : mccmnc.toCharArray()) {
                if (!Character.isDigit(c)) {
                    valid = false;
                    break;
                }
            }
            if (!valid) {
                continue;
            }

            // Add to valid list
            validList.add(mccmnc);
        }

        log("validateMccMnc(), output: " + validList);

        return (validList.size() == 0) ? null : validList.toArray(new String[validList.size()]);
    }

    private void registerForBroadcast() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        filter.addAction(TelephonyIntents.ACTION_LOCATED_PLMN_CHANGED);
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        filter.addAction(WifiManager.WIFI_SCAN_AVAILABLE);
        filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        mContext.registerReceiver(mReceiver, filter);
    }

    private void checkIfstartWifiScan(boolean scanImmediately) {

        boolean wifiPdnExisted = isWifiPdnExisted();

        if (!mIsWifiConnected && wifiPdnExisted) {

            if (scanImmediately) {
                log("call WifiManager.startScan()");
                if (mWifiManager != null) {
                    mWifiManager.startScan();
                } else {
                    Log.e(TAG, "checkIfstartWifiScan(): WifiManager null");
                    Message msg = mHandler.obtainMessage(EVENT_RETRY_CHECK_IF_START_WIFI_SCAN,
                        (Object) scanImmediately);
                    synchronized (mLock) {
                        mPendingMsgs.add(msg);
                    }
                    return;
                }
            }

            if (!mHandler.hasMessages(EVENT_WIFI_SCAN)) {
                log("start 3s delay to trigger wifi scan");
                mHandler.sendMessageDelayed(
                    mHandler.obtainMessage(EVENT_WIFI_SCAN), WIFI_SCAN_DELAY);
            }
        } else {
            mHandler.removeMessages(EVENT_WIFI_SCAN);
        }
    }

    private void checkIfShowNoInternetError(boolean showImmediately) {

        int mainCapabilityPhoneId = getMainCapabilityPhoneId();
        int ratType = getRatTypeFromMal(mainCapabilityPhoneId);

        if (mIsWfcEnabled[mainCapabilityPhoneId] &&
            ratType == WifiOffloadManager.RAN_TYPE_UNSPEC &&
            mIsWifiL2Connected && !mIsWifiConnected) {

            if (showImmediately) {
                mWosExt.showPDNErrorMessages(WIFI_NO_INTERNET_ERROR_CODE);
                return;
            }

            if (!mHandler.hasMessages(EVENT_NOTIFY_WIFI_NO_INTERNET)) {
                log("checkIfShowNoInternetError(): start 8s timeout");
                mHandler.sendMessageDelayed(
                    mHandler.obtainMessage(EVENT_NOTIFY_WIFI_NO_INTERNET),
                    WIFI_NO_INTERNET_TIMEOUT);
            }

        } else {
            if (mHandler.hasMessages(EVENT_NOTIFY_WIFI_NO_INTERNET)) {
                log("checkIfShowNoInternetError(): cancel 8s timeout");
                mHandler.removeMessages(EVENT_NOTIFY_WIFI_NO_INTERNET);
            }
        }
    }

    private void updateWifiConnectedInfo(int isConnected) {
        boolean changed = false;

        if (isConnected == 0) {
            if (mIsWifiConnected) {
                mIsWifiConnected = false;
                mWifiApMac = "";
                mWifiIpv4Address = "";
                mWifiIpv6Address = "";
                mIfName ="";
                changed = true;
            }
        } else {
            String wifiApMac = "", ipv4Address = "", ipv6Address = "", ifName = "";
            mIsWifiConnected = true;

            // get MAC address of the current access point
            WifiInfo wifiInfo = null;
            if (mWifiManager != null) {
                wifiInfo = mWifiManager.getConnectionInfo();
            } else {
                Message msg = mHandler.obtainMessage(EVENT_RETRY_UPDATE_WIFI_CONNTECTED_INFO,
                        isConnected, 0);
                synchronized (mLock) {
                    mPendingMsgs.add(msg);
                }
            }

            //log("wifi info: " + wifiInfo);
            if (wifiInfo != null) {
                wifiApMac = wifiInfo.getBSSID();
                if (!mWifiApMac.equals(wifiApMac)) {
                    mWifiApMac = (wifiApMac == null ? "" : wifiApMac);
                    changed = true;
                }
            }

            //log("all NW: " + mConnectivityManager.getAllNetworks());
            // get ip
            for (Network nw : mConnectivityManager.getAllNetworks()) {
                LinkProperties prop = mConnectivityManager.getLinkProperties(nw);
                // MAL only care about wlan
                if (prop == null || prop.getInterfaceName() == null
                        || !prop.getInterfaceName().startsWith("wlan")) {
                    continue;
                }
                for (InetAddress address : prop.getAddresses()) {
                    if (address instanceof Inet4Address && !address.isLoopbackAddress()) {
                        ipv4Address = address.getHostAddress();
                    } else if (address instanceof Inet6Address && !address.isLinkLocalAddress()
                        && !address.isLoopbackAddress()) {
                        // Filters out link-local address. If cannot find non-link-local address,
                        // pass empty string to MAL.
                        ipv6Address = address.getHostAddress();
                    } else {
                        /*
                        log("ignore address= " + address +
                            " isLoopbackAddr: " + address.isLinkLocalAddress());
                         */
                    }
                }
                // get interface name
                ifName = prop.getInterfaceName();
            }
            if (!mWifiIpv4Address.equals(ipv4Address)) {
                mWifiIpv4Address = (ipv4Address == null ? "" : ipv4Address);
                changed = true;
            }
            if (!mWifiIpv6Address.equals(ipv6Address)) {
                mWifiIpv6Address = (ipv6Address == null ? "" : ipv6Address);
                changed = true;
            }
            if (!mIfName.equals(ifName)) {
                mIfName = (ifName == null ? "" : ifName);
                changed = true;
            }
        }
        checkIfShowNoInternetError(false);

        // Check if start wifi scan to improve MOBIKE
        checkIfstartWifiScan(false);

        if (changed) {
            notifyMalWifiState();
        }
    }

    // Currently it only care about MD1's data roaming setting.
    private int getDataRoamingSetting() {
        if (mSubscriptionManager == null) {
            log("Unexpected error, mSubscriptionManager=null");
            return SubscriptionManager.DATA_ROAMING_DISABLE;
        }

        int mainCapabilityPhoneId = getMainCapabilityPhoneId();
        SubscriptionInfo subInfo = mSubscriptionManager.getActiveSubscriptionInfoForSimSlotIndex(
                mainCapabilityPhoneId);
        log("getDataRoamingSetting: mainCapabilityPhoneId=" + mainCapabilityPhoneId
                + " , subInfo=" + subInfo);

        if (subInfo != null) {
            return subInfo.getDataRoaming();
        } else {
            log("Cannot get subscription information for slot:" + mainCapabilityPhoneId);
            return SubscriptionManager.DATA_ROAMING_DISABLE;
        }
    }

    private void updateDataRoamingSetting() {
        int isDataRoamingEnabled = getDataRoamingSetting();
        if (isDataRoamingEnabled != mDataRoamingEnabled) {
            mDataRoamingEnabled = isDataRoamingEnabled;
            mHandler.sendMessage(mHandler.obtainMessage(EVENT_NOTIRY_MAL_USER_PROFILE));
        }
    }

    private void updateServiceState(int subId, ServiceState state) {
        // Data service state
        boolean isDataRoaming = state.getDataRoaming();
        int radioTechnology = ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
        final NetworkRegistrationInfo regPsState = state.getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_PS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (regPsState != null) {
            int accessNetworkTechnology = regPsState.getAccessNetworkTechnology();

            Log.d(TAG, "AccessNetworkTechnology " + accessNetworkTechnology);
            radioTechnology = state.networkTypeToRilRadioTechnology(accessNetworkTechnology);
        }

        int dataRegState = state.getDataRegState();

        if (radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN) {
            dataRegState = ServiceState.STATE_OUT_OF_SERVICE;
        }

        int simId = SubscriptionManager.getPhoneId(subId);
        int mdIdx = getModemIdFromPhoneId(simId);

        if (checkInvalidSimIdx(simId, "ignore for sim: " + simId + " sub: " + subId)) return;

        // Check if update data service state to MAL
        if (mIsCurDataRoaming[simId] != isDataRoaming
                || mRadioTechnology[simId] != radioTechnology
                || mDataRegState[simId] != dataRegState) {
            mIsCurDataRoaming[simId] = isDataRoaming;
            mRadioTechnology[simId] = radioTechnology;
            mDataRegState[simId] = dataRegState;
            notifyMalServiceState(simId, mdIdx);
        }

    }

    private void updateVoiceState(int subId, ServiceState state) {

        // Voice service state
        boolean isVoiceRoaming = state.getVoiceRoaming();
        int voiceRadioTechnology = state.getRilVoiceRadioTechnology();
        int voiceRegState = state.getVoiceRegState();

        int simId = SubscriptionManager.getPhoneId(subId);
        int mdIdx = getModemIdFromPhoneId(simId);

        if (checkInvalidSimIdx(simId, "ignore for sim: " + simId + " sub: " + subId)) return;

        // Check if update voice service state to MAL
        if (mIsCurVoiceRoaming[simId] != isVoiceRoaming
            || mVoiceRadioTechnology[simId] != voiceRadioTechnology
            || mVoiceRegState[simId] != voiceRegState) {

            mIsCurVoiceRoaming[simId] = isVoiceRoaming;
            mVoiceRadioTechnology[simId] = voiceRadioTechnology;
            mVoiceRegState[simId] = voiceRegState;

            notifyMalVoiceState(simId, mdIdx);
        }
    }

    private void updateIsAllowTurnOffIms(int phoneId, int subId) {
        try {
            mIsAllowTurnOffIms[phoneId] = getBooleanCarrierConfig(
                    CarrierConfigManager.KEY_CARRIER_ALLOW_TURNOFF_IMS_BOOL, subId);

            log("updateIsAllowTurnOffIms(), phoneId:" + phoneId + ", subId:" + subId +
                ", mIsAllowTurnOffIms: " + mIsAllowTurnOffIms[phoneId]);

            mHandler.sendMessage(mHandler.obtainMessage(EVENT_NOTIRY_MAL_USER_PROFILE,
                    new Integer(phoneId)));
        } catch (Exception e) {
            Log.d(TAG, "get CARRIER_ALLOW_TURNOFF_IMS_BOOL for "
                    + phoneId + " failed-" + e);
        }
    }

    // notify for all SIM.
    private void notifyMalUserProfile() {
        for (int i = 0; i < mSimCount; i++) {
            notifyMalUserProfile(i);
        }
    }

    private void notifyMalUserProfile(int simIdx) {
        if (mNativeMalState != MAL_STATE_READY) {
            Log.d(TAG, "notifyMalUserProfile(" + simIdx
                    +") return directly due to MAL isn't ready yet.");
            return;
        }

        if (checkInvalidSimIdx(simIdx, "notifyMalUserProfile(" + simIdx
                    +")return invalid sim id.")) return;

        int wfcMode = ImsManager.getInstance(mContext, simIdx).getWfcMode();

        log("notifyMalUserProfile(" + simIdx
                + "): mIsVolteEnabled: " + mIsVolteEnabled[simIdx]
                + ", mIsVilteEnabled: " + mIsVilteEnabled[simIdx]
                + " mIsWfcEnabled: " + mIsWfcEnabled[simIdx] + " mFqdn: " + mFqdn[simIdx]
                + " mIsWifiEnabled: " + mIsWifiEnabled
                + " mHasWiFiDisabledPending: " + mHasWiFiDisabledPending
                + " mWfcMode: " + mWfcMode + " mDataRoamingEnabled: " + mDataRoamingEnabled
                + " mIsAllowTurnOffIms: " + mIsAllowTurnOffIms[simIdx]);

        if (checkHidlService()) {
            try {
                mWifiOffload.nativeSetWosProfile((new Integer(simIdx)).byteValue(),
                    mIsVolteEnabled[simIdx], mIsWfcEnabled[simIdx], mIsVilteEnabled[simIdx],
                    mFqdn[simIdx], mIsWifiEnabled, (new Integer(mWfcMode)).byteValue(),
                    (new Integer(mDataRoamingEnabled)).byteValue(), mIsAllowTurnOffIms[simIdx]);
            } catch (RemoteException e) {
                Log.e(TAG, "Wos HIDL Exception:", e);
            }
        }
    }

    private void notifyMalWifiState() {
        boolean wfcInApMode = allowWfcInAirplaneMode();
        if (!wfcInApMode) {
            Log.d(TAG, "Do not allowWfcInAirplaneMode");
            return;
        }

        if (mNativeMalState != MAL_STATE_READY) {
            Log.d(TAG, "notifyMalWifiState return directly due to MAL isn't ready yet.");
            return;
        }

        log("notifyMalWifiState mIsWifiConnected: " + mIsWifiConnected
                + " mIfaceName: " + mIfName
                + " mWifiIpv4Address: " + maskString(mWifiIpv4Address)
                + " mWifiIpv6Address: " + maskString(mWifiIpv6Address)
                + " mWifiApMac: " + maskString(mWifiApMac)
                );
        if (checkHidlService()) {
            try {
                mWifiOffload.nativeSetWifiStatus(mIsWifiConnected, mIfName,
                        mWifiIpv4Address, mWifiIpv6Address, mWifiApMac);
            } catch (RemoteException e) {
                Log.e(TAG, "Wos HIDL Exception:", e);
            }
        }
    }

    private boolean allowWfcInAirplaneMode() {
        boolean wfcInApMode = true;
        String mccMnc = mTelephonyManager.getSimOperator(mActiveSubId[getMainCapabilityPhoneId()]);
        int subId = mActiveSubId[getMainCapabilityPhoneId()];

        if (mIsAirplaneModeOn) {
            try {
                wfcInApMode = getBooleanCarrierConfig(
                    MtkCarrierConfigManager.MTK_KEY_WOS_SUPPORT_WFC_IN_FLIGHTMODE, subId);
            } catch (Exception e) {
                Log.d(TAG, "get MTK_KEY_WOS_SUPPORT_WFC_IN_FLIGHTMODE for subid : " + subId +
                        " failed-" + e);
            }
        }
        Log.d(TAG, "allowWfcInAirplaneMode mIsAirplaneModeOn = " + mIsAirplaneModeOn + ", subId = " + subId
            + ", wfcInApMode = " + wfcInApMode);
        return wfcInApMode;
    }

    private void notifyMalCallState(int simIdx, int callId, int callType, int callState) {
        if (mNativeMalState != MAL_STATE_READY) {
            Log.d(TAG, "notifyMalCallState return directly due to MAL isn't ready yet.");
            return;
        }

        log("notifyMalCallState for sim: " + simIdx + " callId: " + callId
                + " call state: " + callState + " callType: " + callType);
        if (checkHidlService()) {
            try {
                mWifiOffload.nativeSetCallState((new Integer(simIdx)).byteValue(),
                        callId, callType, callState);
            } catch (RemoteException e) {
                Log.e(TAG, "Wos HIDL Exception:", e);
            }
        }
    }

    private void notifyMalServiceState(int simIdx, int mdIdx) {
        if (mNativeMalState != MAL_STATE_READY) {
            Log.d(TAG, "notifyMalServiceState return directly due to MAL isn't ready yet.");
            return;
        }

        log("nativeSetServiceState simIdx: " + simIdx
                + " mdIdx: " + mdIdx
                + " mIsCurDataRoaming: " + mIsCurDataRoaming[simIdx]
                + " mRadioTechnology: " + mRadioTechnology[simIdx]
                + " mDataRegState: " + mDataRegState[simIdx]
                + " mLocatedPlmn: " + mLocatedPlmn[simIdx]);
        if (checkHidlService()) {
            try {
                mWifiOffload.nativeSetServiceState((new Integer(mdIdx)).byteValue(),
                        (new Integer(simIdx)).byteValue(), mIsCurDataRoaming[simIdx],
                        (new Integer(mRadioTechnology[simIdx])).byteValue(),
                        (new Integer(mDataRegState[simIdx])).byteValue(),
                        (mLocatedPlmn[simIdx] == null) ? "" : mLocatedPlmn[simIdx]);
            } catch (RemoteException e) {
                Log.e(TAG, "Wos HIDL Exception:", e);
            }
        }
    }

    private void notifyMalVoiceState(int simIdx, int mdIdx) {
        if (mNativeMalState != MAL_STATE_READY) {
            Log.d(TAG, "notifyMalVoiceState return directly due to MAL isn't ready yet.");
            return;
        }

        log("nativeSetVoiceState simIdx: " + simIdx
                + " mdIdx: " + mdIdx
                + " mIsCurVoiceRoaming: " + mIsCurVoiceRoaming[simIdx]
                + " mVoiceRadioTechnology: " + mVoiceRadioTechnology[simIdx]
                + " mVoiceRegState: " + mVoiceRegState[simIdx]);

        if (checkHidlService()) {
            try {
                mWifiOffload.nativeSetVoiceState((new Integer(mdIdx)).byteValue(),
                        (new Integer(simIdx)).byteValue(), mIsCurVoiceRoaming[simIdx],
                        (new Integer(mVoiceRadioTechnology[simIdx])).byteValue(),
                        (new Integer(mVoiceRegState[simIdx])).byteValue());
            } catch (RemoteException e) {
                Log.e(TAG, "Wos HIDL Exception:", e);
            }
        }
    }

    private void notifyMalSimInfo(int slotId) {
        if (mNativeMalState != MAL_STATE_READY) {
            Log.d(TAG, "notifyMalServiceState return directly due to MAL isn't ready yet.");
            return;
        }

        if (mTelephonyManager == null) {
            Log.d(TAG, "Unexpected error, mTelephonyManager=null");
            return;
        }

        int subId = mActiveSubId[slotId];
        String simState = mSimState[slotId];

        if (simState == null) {
            Log.d(TAG, "notifyMalSimInfo: ignore sim state because it is null");
            return;
        }

        String imei = "";
        String imsi = "";
        String mccMnc = "";
        String impi = "";
        String simTypeStr = "";
        int simType = 2; // 0: USIM, 1: ISIM, 2: unknown
        boolean simReady;

        boolean isMainSim = (slotId == getMainCapabilityPhoneId());

        mIsSimWfcSupported[slotId] = false;

        if (simState.equalsIgnoreCase(IccCardConstants.INTENT_VALUE_ICC_ABSENT)) {
            simReady = false;
        } else if (simState.equalsIgnoreCase(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
            simReady = true;

            imei = mTelephonyManager.getDeviceId(slotId);
            imei = (imei == null) ? "" : imei;
            imsi = mTelephonyManager.getSubscriberId(subId);
            imsi = (imsi == null) ? "" : imsi;
            mccMnc = mTelephonyManager.getSimOperator(subId);
            mccMnc = (mccMnc == null) ? "" : mccMnc;
            impi = MtkTelephonyManagerEx.getDefault().getIsimImpi(subId);
            impi = (impi == null) ? "" : impi;
            simTypeStr = MtkTelephonyManagerEx.getDefault().getIccCardType(subId);
            mIsSimWfcSupported[slotId] = isSimSupportWfc(mccMnc) || isIccIdSupportWfc(getIccId());

            if (simTypeStr == null) {
                Log.d(TAG, "notifyMalSimInfo: unexpected result, simType=null, return directly");
                return;
            } else if (simTypeStr.equals("SIM")) {
                simType = 0;
            } else if (simTypeStr.equals("USIM")) {
                simType = 1;
            }

            if (!checkAsciiValid(impi)) {
                log("impi invalid");
                return;
            }
        } else {
            log("notifyMalSimInfo: ignore sim state: " + simState);
            return;
        }

        if (!USR_BUILD || TELDBG) {
            log("notifyMalSimInfo simIdx: " + slotId + " subId: " + subId
                    + " simReady: " + simReady + " isMainSim: " + isMainSim
                    + " ei: " + encryptString(imei) + " si: " + encryptString(imsi) + " mccMnc: " + mccMnc
                    + " pi: " + encryptString(impi) + " simType: " + simTypeStr
                    + " wfcSupported: " + mIsSimWfcSupported[slotId]);
        } else {
            log("notifyMalSimInfo simIdx: " + slotId + " subId: " + subId
                    + " simReady: " + simReady + " isMainSim: " + isMainSim
                    + " imei: [hidden] imsi: [hidden] mccMnc: " + mccMnc
                    + " impi: [hidden] simType: " + simTypeStr
                    + " wfcSupported: " + mIsSimWfcSupported[slotId]);
        }
        if (checkHidlService()) {
            try {
                mWifiOffload.nativeSetSimInfo((new Integer(slotId)).byteValue(),
                    imei, imsi, mccMnc, impi, simType, simReady, isMainSim);
            } catch (RemoteException e) {
                Log.e(TAG, "Wos HIDL Exception:", e);
            }
        }
    }

    private boolean checkAsciiValid(String impi) {
        byte[] impiBytes = impi.getBytes();

        CharsetDecoder decoder = Charset.forName("US-ASCII").newDecoder();

        try {
            CharBuffer buffer = decoder.decode(ByteBuffer.wrap(impiBytes));
            return true;
        } catch (CharacterCodingException e) {
            log("The information contains a non ASCII character(s).");
            //return false;
        }

        return false;
    }

    private void notifyMalRadioState(int simIdx) {
        if (mNativeMalState != MAL_STATE_READY) {
            Log.d(TAG, "notifyMalRadioInfo return directly due to MAL isn't ready yet.");
            return;
        }

        int mainCapabilityPhoneId = getMainCapabilityPhoneId();
        int mdIdx = mainCapabilityPhoneId==simIdx? 0: 1;

        if (mRadioState[simIdx] >= 0) {
            log("notifyMalRadioInfo simIdx: " + simIdx + " mdIdx: " + mdIdx
                       + " radio: " + mRadioState[simIdx]);
            if (checkHidlService()) {
                try {
                    mWifiOffload.nativeSetRadioState(
                        (new Integer(simIdx)).byteValue(), (new Integer(mdIdx)).byteValue(),
                        (new Integer(mRadioState[simIdx])).byteValue());
                } catch (RemoteException e) {
                    Log.e(TAG, "Wos HIDL Exception:", e);
                }
            }
        }
    }

    private void notifyMalWifiQuality(int rssi, int snr) {
        if (mNativeMalState != MAL_STATE_READY) {
            Log.d(TAG, "notifyMalWifiQuality return directly due to MAL isn't ready yet.");
            return;
        }

        log("notifyMalWifiQuality rssi: " + rssi + " snr: " + snr);
        if (checkHidlService()) {
            try {
                mWifiOffload.nativeSetWifiQuality(rssi, snr);
            } catch (RemoteException e) {
                Log.e(TAG, "Wos HIDL Exception:", e);
            }
        }
    }

    private void notifyMalWfcSupported(int simId) {
        if (mNativeMalState != MAL_STATE_READY) {
            Log.d(TAG, "notifyMalWfcSupported return directly due to MAL isn't ready yet.");
            return;
        }

        log("notifyMalWfcSupported: simId: " + simId +
            ", supported= " + mIsSimWfcSupported[simId] + ", isEnabled= " + mIsWfcEnabled[simId]);
        int res = (mIsSimWfcSupported[simId] && mIsWfcEnabled[simId])? 1: 0;
        if (checkHidlService()) {
            try {
                mWifiOffload.nativeSetWfcSupported(
                        (new Integer(simId)).byteValue(), res);
            } catch (RemoteException e) {
                Log.e(TAG, "Wos HIDL Exception:", e);
            }
        }
    }

    /**
     * callback from MAL when IMS PDN handover.
     * @param stage handover start/end
     * @param ratType current rat type
     */
    protected void onHandover(int simIdx, int stage, int ratType) {
        HandoverMessage hoMsg = new HandoverMessage(simIdx, stage, ratType);
        Message msg = mHandler.obtainMessage(EVENT_ON_HANDOVER, hoMsg);
        mHandler.sendMessage(msg);
    }

    /**
     * callback from MAL when rove out condition meets.
     * @param roveOut is rove out or not
     * @param rssi current WiFi rssi
     */
    protected void onRoveOut(int simIdx, boolean roveOut, int rssi) {
        RoveOutMessage roMsg = new RoveOutMessage(simIdx, roveOut, rssi);
        Message msg = mHandler.obtainMessage(EVENT_ON_ROVE_OUT, roMsg);
        mHandler.sendMessage(msg);
    }

    /**
     * callback from MAL when IMS PDN is lost
     */
    private void onLost() {
        // TODO: broadcast
    }

    /**
     * callback from MAL when IMS PDN is unavailable
     */
    private void onUnavailable() {
        // TODO: broadcast
    }

    /**
     * callback from MAL when PDN over ePDG is active.
     * @param active is PDN over ePDG active or in-active.
     */
    protected void onPdnStateChanged(int simIdx, boolean active) {
        boolean preWifiPdnExited = false;
        log("onPdnStateChanged simIdx: " + simIdx + " active: " + active);


        if (checkInvalidSimIdx(simIdx, "onPdnStateChanged: invalid SIM id")) return;

        if (mWifiLock == null) {
            Log.d(TAG, "Unexpected error, mWifiLock is null");
        }

        preWifiPdnExited =  isWifiPdnExisted();

        if (active) {
            mWifiPdnExisted[simIdx] = true;
        } else {
            mWifiPdnExisted[simIdx] = false;
        }

        // Check if start wifi scan to improve MOBIKE
        checkIfstartWifiScan(false);

        if (preWifiPdnExited != isWifiPdnExisted()) {
            //handling the WiFi deffer Off
            handleWifiDefferOff(WifiLockSource.MODEM_STATE_CHANGE, 0);
        }

        mHandler.sendMessage(mHandler.obtainMessage(EVENT_HANDLE_MODEM_POWER));
    }

    // ToDo: should be removed when dual-IMS.
    /**
     * Now RDS still has no dual-IMS capability, so the simIdx of onPdnStateChanged
     * is a invalid value. At this case, we workaround it here by syncing the PDN
     * state among all SIMs.
     */
    private void forceSyncPdnStateForAllSim(boolean wifiPdnState) {
        for (int i = 0; i < mSimCount; i++) {
            mWifiPdnExisted[i] = wifiPdnState;
        }
    }

    /**
     * callback from MAL when PDN ran type is changed.
     * @param interface interface id.
     * @param ranType ran type.
     */
    protected void onPdnRanTypeChanged(int simIdx, int interfaceId, int ranType) {
        log("onPdnRanTypeChanged simIdx: " + simIdx
                + " interfaceId: " + interfaceId + " ranType: " + ranType);

        if (ranType != 0) {
            // If ran type isn't 0, it means the PDN is activated
            // so the error notification should be cleaned.
            mRatType[simIdx] = ranType;
            mWosExt.clearPDNErrorMessages();
        }
    }

    /**
     * callback from MAL when MAL is reset.
     */
    protected void onMalReset() {
        log("onMalReset");
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_NATIVE_MAL_CONNECTION_RESET));
    }

    /**
     * callback from MAL when disconnect cause is changed.
     */
    protected void onDisconnectCauseNotify(int simIdx, int lastErr, int lastSubErr) {
        if (checkInvalidSimIdx(simIdx, "onDisconnectCauseNotify: invalid SIM id")) return;

        log("onDisconnectCauseNotify: simIdx=" + simIdx +
                ", lastErr=" + lastErr + ", lastSubErr=" + lastSubErr);

        mDisconnectCause[simIdx] = new DisconnectCause(lastErr, lastSubErr);
        mWosExt.showPDNErrorMessages(lastErr);
    }

    /**
     * callback from MAL when to let Wos disable WiFi.
     */
    protected void onRequestSetWifiDisabled(int pdnCount) {
        log("onRequestSetWifiDisabled pdnCount: " + pdnCount);
    }

    protected void onRequestImsSwitch(int simIdx, boolean isImsOn) {
        Message msg = mHandler.obtainMessage(EVENT_ON_REQUEST_IMS_SWITCH, simIdx, (isImsOn)? 1: 0);
        mHandler.sendMessage(msg);
    }

    protected void onRequestLocationInfo(int simIdx) {
        log("onRequestLocationInfo simIdx: " + simIdx);
        Message msg = mHandler.obtainMessage(EVENT_REQUEST_LOCATION, simIdx, 0);
        mHandler.sendMessage(msg);
    }

    /**
     * setup callbacks from ConnectivityService when WiFi is changed.
     */
    private void setupCallbacksForWifiStatus() {
        if (mConnectivityManager == null) {
            Log.d(TAG, "Unexpected error, mConnectivityManager=null");
            return;
        }

        NetworkRequest request = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED)
                .build();
        mConnectivityManager.registerNetworkCallback(request,
            new ConnectivityManager.NetworkCallback() {
            /**
             * @param network
             */
            @Override
            public void onAvailable(Network network) {
                Message msg = mHandler.obtainMessage(
                        EVENT_WIFI_NETWORK_STATE_CHANGE,
                        1,  // isConnected
                        0, null);
                mHandler.sendMessage(msg);
            }

            /**
             * @param network
             */
            @Override
            public void onLost(Network network) {
                Message msg = mHandler.obtainMessage(
                        EVENT_WIFI_NETWORK_STATE_CHANGE,
                        0,  // isConnected
                        0, null);
                mHandler.sendMessage(msg);
            }

            @Override
            public void onCapabilitiesChanged(final Network network,
                        final NetworkCapabilities networkCapabilities) {
                if (networkCapabilities == null) {
                    Log.d(TAG, "NetworkCallback.onCapabilitiesChanged, Capabilities=null");
                    return;
                }

                int rssi = networkCapabilities.getSignalStrength();

                if (mLastRssi == rssi) {
                    return;
                }
                mLastRssi = rssi;

                Message msg = mHandler.obtainMessage(
                            EVENT_WIFI_SIGNAL_STRENGTH_CHANGE,
                            rssi, 0, null);
                mHandler.sendMessage(msg);
            }

            /**
             * @param network
             */
            @Override
            public void onLinkPropertiesChanged(Network network,
                LinkProperties linkProperties) {
                if (mIsWifiConnected) {
                    // At this timing, goes to check if IP address is updated.
                    Message msg = mHandler.obtainMessage(
                            EVENT_WIFI_NETWORK_STATE_CHANGE,
                            1,  // isConnected
                            0, null);
                    mHandler.sendMessage(msg);
                }
            }
        });
    }

    private void setupImsConfigFeatureValueMonitoring() {
        log("setupImsConfigFeatureValueMonitoring()");
        IntentFilter filter = new IntentFilter();
        filter.addAction(ImsConfig.ACTION_IMS_FEATURE_CHANGED);
        mContext.registerReceiver(mFeatureValueReceiver, filter);
    }

    /**
    * callback from MAL for RDS to configure RSSI monitor thresholds.
    */
    protected void onRssiMonitorRequest(int simId, int size, int[] rssiThresholds) {
        mRssiMonitoringProcessor.registerRssiMonitoring(simId, size, rssiThresholds);

        // Notify IWLAN the current RSSI as IWLAN request
        updateLastRssi();
        log("onRssiMonitorRequest() rssi = " + mLastRssi);

        Message msg = mHandler.obtainMessage(
                            EVENT_WIFI_SIGNAL_STRENGTH_CHANGE,
                            mLastRssi, 0, null);
        mHandler.sendMessage(msg);
    }

    /**
     * to get main capability phone id.
     *
     * @return The phone id with highest capability.
     */
    private int getMainCapabilityPhoneId() {
       int phoneId = SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
       if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
           phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
       }
       log("getMainCapabilityPhoneId = " + phoneId);
       return phoneId;
    }

        // Monitor wfc settings
    private final ContentObserver mContentObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {

            mHandler.sendMessage(mHandler.obtainMessage(EVENT_HANDLE_MODEM_POWER));

        }
    };

    private boolean isWfcEnabledByAnyPhone() {
        boolean isOn = false;
        for (int i = 0; i < mSimCount; i++) {
            isOn |= mIsWfcEnabled[i];
        }
        Log.d(TAG, "isWfcEnabledByAnyPhone(), isOn:" + isOn);
        return isOn;
    }

    private void handleModemPower() {
        boolean isWfcEnabled = isWfcEnabledByAnyPhone();
        log("handleModemPower(): mIsWifiEnabled:" + mIsWifiEnabled
                + " isWifiPdnExist: " + isWifiPdnExisted()
                + " isWfcEnabledByAnyPhone: " + isWfcEnabled);
        /* If the flight mode is off, RadioManager will ignore the notify from WoS.
         * If the flight mode is on, handle the modem power by following rules:
         * 1. WFC enabled and Wifi is enabled, turn on modem
         * 2. WFC settings enabled and Wifi is enabled, turn on modem
         * 3. If one of Wfc settings / Wfc enabled / Wifi on and WifiPdn not existed, turn off modem
         */
        if (mIsWifiEnabled && isWfcEnabled) {
            notifyPowerOnModem(true);
            return;
        }

        if (!isWifiPdnExisted()) {
            notifyPowerOnModem(false);
        }
    }

    private void notifyPowerOnModem(boolean isModemOn) {
        if (!RadioManager.isFlightModePowerOffModemConfigEnabled()) {
            log("modem always on, no need to control it!");
            return;
        }

        if (mContext == null) {
            Log.d(TAG, "context is null, can't control modem!");
            return;
        }

        Intent intent = new Intent(RADIO_MANAGER_POWER_ON_MODEM);
        intent.putExtra(EXTRA_POWER_ON_MODEM_KEY, isModemOn);
        mContext.sendBroadcast(intent);
        log("notifyPowerOnModem power:" + isModemOn);
    }

    /**
     * get the modem ID by given phone ID. Modem Id is 0 based.
     *
     * @param phoneId phone ID
     * @return modem ID
     */
    private int getModemIdFromPhoneId(int phoneId) {
        int mainCapabilityPhoneId = getMainCapabilityPhoneId();
        return mainCapabilityPhoneId==phoneId?0:1;
    }

    private int getNetworkTypeByFeature(int imsFeatureType) {
        switch (imsFeatureType) {
            case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE:
                return TelephonyManager.NETWORK_TYPE_LTE;

            case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE:
                return TelephonyManager.NETWORK_TYPE_LTE;

            case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI:
                return TelephonyManager.NETWORK_TYPE_IWLAN;
            default:
                return TelephonyManager.NETWORK_TYPE_UNKNOWN;
        }
    }

    private boolean checkInvalidSimIdx(int simIdx, String dbgMsg) {
        if (simIdx < 0 || simIdx >= mSimCount) {
            Log.d(TAG, dbgMsg);
            return true;
        }
        return false;
    }

    private boolean checkNullObject(Object obj, String dbgMsg) {
        if (obj == null) {
            Log.d(TAG, dbgMsg);
            return true;
        }
        return false;
    }

    private boolean getBooleanCarrierConfig(String key, int subId) throws Exception {
        if (mContext == null) {
            throw new Exception("getBooleanCarrierConfig: no mContext = null");
        }

        if (mCarrierConfigManager == null) {
            mCarrierConfigManager = (CarrierConfigManager) mContext.getSystemService(
                    Context.CARRIER_CONFIG_SERVICE);
        }
        PersistableBundle b = null;
        if (mCarrierConfigManager != null) {
            b = mCarrierConfigManager.getConfigForSubId(subId);
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

    private boolean isSimSupportWfc(String mccMnc) {
        boolean easyConfig = SystemProperties.getBoolean(
                PROPERTY_FORCE_ENALBE_WFC_SUPPORTED, false);
        if (easyConfig) {
            log("isSimSupportWfc: easyConfig to enable WFC!");
            return true;
        }

        final String[] wfcSupportedMccMncList = getMccMncAllowList(MCCMNC_MODE_BOTH);

        ArrayList<String> wfcSupportedList =
            new ArrayList<String>(Arrays.asList(wfcSupportedMccMncList));
        return wfcSupportedList.contains(mccMnc);
    }

    private void updateFeatureValue() {
        int multiIMSSupportNum = SystemProperties.getInt(PROPERTY_MULTI_IMS_SUPPORT, 0);
        log("multiIMSSupportNum:" + multiIMSSupportNum);

        for (int i = 0; i < mSimCount; i++) {
            int propValueBit = 0;
            if (multiIMSSupportNum > 1) {
                propValueBit = i;
            }
            mIsVolteEnabled[i] =
                (SystemProperties.getInt(PROPERTY_VOLTE_ENABLE, 0) & (1 << propValueBit)) > 0;
            mIsWfcEnabled[i]   =
                (SystemProperties.getInt(PROPERTY_WFC_ENABLE, 0) & (1 << propValueBit)) > 0;
            mIsVilteEnabled[i] =
                (SystemProperties.getInt(PROPERTY_IMS_VIDEO_ENABLE, 0) & (1 << propValueBit)) > 0;
            try {
                int[] subIds = SubscriptionManager.getSubId(i);
                int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
                if (subIds.length != 0) {
                    subId = subIds[0];
                }
                mIsAllowTurnOffIms[i] = getBooleanCarrierConfig(
                    CarrierConfigManager.KEY_CARRIER_ALLOW_TURNOFF_IMS_BOOL, subId);
            } catch (Exception e) {
                Log.d(TAG, "get CARRIER_ALLOW_TURNOFF_IMS_BOOL for " + i + " failed-" + e);
            }
        }
    }

    private boolean isWifiPdnExisted() {
        for (int i = 0; i < mSimCount; i++) {
            if (mWifiPdnExisted[i]) {
                log("isWifiPdnExisted: found WiFi PDN on SIM: " + i);
                return true;
            }
        }
        return false;
    }

    private String getProvisioningFqdn() {
        if (mContext == null) {
            Log.d(TAG, "getProvisioningFqdn: no context!");
            return "";
        }

        String wfcFqdn = null;
        ImsConfig imsConfig = null;

        int phoneId = getMainCapabilityPhoneId();
        ImsManager imsManager = ImsManager.getInstance(mContext, phoneId);

        if (imsManager == null) {
            Log.d(TAG, "getProvisioningFqdn: no ims manager!");
            return "";
        }

        try {
            imsConfig = imsManager.getConfigInterface();
            if (imsConfig == null) {
                Log.d(TAG, "getProvisioningFqdn: no ImsConfig!");
                return "";
            }
            // O migration
            //wfcFqdn = imsConfig.getProvisionedStringValue(ImsConfig.ConfigConstants.EPDG_ADDRESS);
        } catch (ImsException e) {
            Log.e(TAG, "getProvisioningFqdn: ImsConfig error:" + e);
        }

        if (wfcFqdn == null) {
            wfcFqdn = "";
        }

        log("getProvisioningFqdn: FQDN=" + wfcFqdn);
        return wfcFqdn;
    }

    private void notifyOnHandover(int simIdx, int stage, int ratType) {
        log("onHandover simIdx: " + simIdx + " stage: " + stage + " rat: " + ratType);
        if (stage == WifiOffloadManager.HANDOVER_END) {
            mRatType[simIdx] = ratType;
        }
        int i = mListeners.beginBroadcast();
        while (i > 0) {
            i--;
            try {
                mListeners.getBroadcastItem(i).onHandover(simIdx, stage, ratType);
            } catch (RemoteException e) {
                // The RemoteCallbackList will take care of removing
                // the dead object for us.
                Log.e(TAG, "onHandover: RemoteException occurs!");
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
                Log.e(TAG, "onRoveOut: RemoteException occurs!");
            }
        }
        mListeners.finishBroadcast();
    }

    private void notifyOnRequestImsSwitch(int simIdx, boolean isImsOn) {
        log("onRequestImsSwitch simIdx: " + simIdx + ", isImsOn: " + isImsOn);

        int i = mListeners.beginBroadcast();
        while (i > 0) {
            i--;
            try {
                mListeners.getBroadcastItem(i).onRequestImsSwitch(simIdx, isImsOn);
            } catch (RemoteException e) {
                // The RemoteCallbackList will take care of removing
                // the dead object for us.
                Log.e(TAG, "onRequestImsSwitch: RemoteException occurs!");
            }
        }
        mListeners.finishBroadcast();
    }

    private boolean isTestNetwork() {
        boolean isTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1") ||
                SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1") ||
                SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1") ||
                SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1");

        // TBD: Check specail IODT load, such as OP1001 (Ericsson) & OP1002 (Nokia)

        return isTestSim;
    }

    private boolean requestLocationFromNetwork(int simIdx) {
        log("requestLocationFromNetwork simIdx: " + simIdx);
        if (mLocationManager == null) {
            Log.d(TAG, "requestLocationFromNetwork: empty locationManager, return");
            return false;
        }

        if (!mLocationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
            Log.d(TAG, "requestLocationFromNetwork:"
                    + "this system has no networkProvider implementation!");
            return false;
        }
        WosLocationListener locationListener = mLocationListeners.get(simIdx);
        if (locationListener == null) {
            locationListener = new WosLocationListener(simIdx);
            mLocationListeners.put(simIdx, locationListener);
        }
        mLocationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER,
                NETWORK_LOCATION_UPDATE_TIME, 0, locationListener);
        log("requestLocationFromNetwork: request networkLocation update");
        return true;
    }

    private void cancelNetworkLocationRequest(int simIdx) {
        log("cancelNetworkLocationRequest simIdx: " + simIdx);
        if (mLocationManager == null) {
            Log.d(TAG, "cancelNetworkLocationRequest: empty locationManager, return");
            return;
        }
        WosLocationListener locationListener = mLocationListeners.get(simIdx);
        if (locationListener == null) {
            return;
        }
        mLocationManager.removeUpdates(locationListener);
    }

    private String getCountryIdFromLocation(Location location) {
        log("getCountryIdFromLocation");
        if (location == null) {
            Log.d(TAG, "getCountryIdFromLocation location is null");
            return null;
        }
        String countryId = null;
        final Geocoder geocoder = new Geocoder(mContext);
        try {
            final List<Address> addresses = geocoder.getFromLocation(
                    location.getLatitude(), location.getLongitude(), 1);
            if (addresses != null && addresses.size() > 0) {
                countryId = addresses.get(0).getCountryCode();
            }
        } catch (IOException e) {
            Log.e(TAG, "Exception occurred when getting geocoded country from location");
        }
        log("getCountryIdFromLocation countryId: " + countryId);
        return countryId;
    }

    private String getIccId() {
        if (mSubscriptionManager == null) {
            Log.d(TAG, "Unexpected error, mSubscriptionManager=null");
            return null;
        }

        int mainCapabilityPhoneId = getMainCapabilityPhoneId();
        SubscriptionInfo subInfo = mSubscriptionManager.getActiveSubscriptionInfoForSimSlotIndex(
                mainCapabilityPhoneId);

        if (subInfo != null) {
            return subInfo.getIccId();
        } else {
            Log.d(TAG, "Cannot get subscription information for slot:" + mainCapabilityPhoneId);
            return null;
        }
    }

    private boolean isIccIdSupportWfc(String mIccId) {
        if (null == mIccId) {
            return false;
        }

        ArrayList<String> wfcSupportedList =
            new ArrayList<String>(Arrays.asList(WifiOffloadServiceResource.wfcSupportedIccIdList));
        return wfcSupportedList.contains(mIccId.substring(0, 7));
    }

    private void initHidlService() {
        log("initHidlService()");
        try {
            mWifiOffload = IWifiOffload.getService(WFO_HIDL_SERVICE_NAME, false);
            if (checkHidlService()) {
                mWifiOffload.linkToDeath(mWifiOffloadDeathRecipient, 0);
                mWifiOffload.setWifiOffloadCallback(mWifiOffloadHIDLCallback);
                mWifiOffload.nativeInit();
            }
        } catch (RemoteException e) {
            mWifiOffload = null;
            Log.e(TAG, "initHidlService() RemoteException: " + e);
        } catch (NoSuchElementException e) {
            mWifiOffload = null;
            Log.e(TAG, "initHidlService() NoSuchElementException: " + e);
        }

        if (checkHidlService()) {
            log("initHidlService() succeed");
        } else {
            Log.e(TAG, "initHidlService() fail !!!");
        }
    }

    private boolean checkHidlService() {
        if (mWifiOffload != null) {
            return true;
        } else {
            Log.e(TAG, "checkHidlService(), mWifiOffload is null !!!!!");
            return false;
        }
    }

    final class WifiOffloadDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // HILD service shouldn't die. debug why !
            Log.e(TAG, WFO_HIDL_SERVICE_NAME + " Died");
        }
    }

    protected void log(String s) {
        if (!USR_BUILD || TELDBG) {
            Log.d(TAG, s);
        }
    }

    private void updateLastRssi() {
        if (mWifiManager == null) {
            Log.e(TAG, "updateLastRssi(): WifiManager null");
            return;
        }

        WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
        if (wifiInfo != null) {
            mLastRssi = wifiInfo.getRssi();
        }
    }

    private void initWifiManager() {
        if (mWifiManager != null) {
            return;
        }

        log("initWifiManager.");
        // prevent ServiceNotFoundException
        if (!StorageManager.inCryptKeeperBounce()) {
            mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        }
        if (mWifiManager != null) {
            boolean isWifiEnabled = mWifiManager.isWifiEnabled();
            if (isWifiEnabled != mIsWifiEnabled) {
                mIsWifiEnabled = isWifiEnabled;
                mHandler.sendMessage(mHandler.obtainMessage(EVENT_NOTIRY_MAL_USER_PROFILE));
            }

            mHandler.sendMessage(mHandler.obtainMessage(EVENT_HANDLE_MODEM_POWER));

            mWifiLock = mWifiManager.createWifiLock("WifiOffloadService-Wifi Lock");
            if (mWifiLock != null) {
                mWifiLock.setReferenceCounted(false);
            }
        } else {
            log("WifiManager null");
            mIsWifiEnabled = false;
            mWifiLock = null;
            Message msg = mHandler.obtainMessage(EVENT_RETRY_INIT);
            synchronized (mLock) {
                mPendingMsgs.add(msg);
            }
        }
    }

    private boolean isWifiDeferOffNeeded () {
        return (isWifiPdnExisted());
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
                            mWifiManager.registerStaStateCallback(mStaStateCallback, mHandler);
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
                                mWifiManager.registerStaStateCallback(mStaStateCallback, mHandler);
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

    private void broadcastWfcStatusIntent(int wfcStatus) {
        // send broadcast only if "defer wifi off" had been enabled
        switch (wfcStatus) {
            case NEED_DEFER:
                log("Notify Wi-Fi fwk to enable defer Wi-Fi off process (NEED_DEFER)");
                break;
            case NO_NEED_DEFER:
                log("Notify Wi-Fi fwk to disable defer Wi-Fi off process (NO_NEED_DEFER)");
                break;
            case WFC_NOTIFY_GO:
                log("Notify Wi-Fi fwk OK to off (WFC_NOTIFY_GO)");
                break;
            default:
                return;
        }

        Intent sendIntent = new Intent(WFC_STATUS_CHANGED);
        sendIntent.putExtra(EXTRA_WFC_STATUS_KEY, wfcStatus);
        mContext.sendBroadcast(sendIntent);

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

    private String encryptString(String message) {
        if (message == null || message.length() == 0) {
            return "";
        }

        Base64.Encoder encoder = Base64.getEncoder();
        byte[] textByte = null;
        try {
            textByte = message.getBytes("UTF-8");
        } catch (Exception e) {
            e.printStackTrace();
            textByte = null;
        }

        if (textByte == null) return "";

        String encryptedString = encoder.encodeToString(textByte);
        return encryptedString == null ? "" : encryptedString;
    }
}
