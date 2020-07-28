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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import android.location.Address;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.location.Geocoder;

import android.net.ConnectivityManager;
import android.net.Network;
import android.net.Uri;

import android.os.AsyncResult;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.SystemProperties;

import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.Rlog;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import java.lang.System;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.ims.MtkImsConstants;

import com.mediatek.wfo.ril.MwiRIL;

public class WfcLocationHandler extends Handler {
    private static final String TAG = "WfcLocationHandler";
    private static final int MAX_VALID_SIM_COUNT = 4;

    // Geo Location: the payload defined as LocationInfo and handled in handleLocationInfo().
    private static final int MSG_REG_IMSA_REQUEST_GEO_LOCATION_INFO = 96009;
    private static final int MSG_REG_IMSA_RESPONSE_GETO_LOCATION_INFO = 91030;
    private static int MAX_NUM_OF_GET_LOCATION_TASKS_THREAD = 3;
    private static int REQUEST_GEOLOCATION_FROM_NETWORK_TIMEOUT = 60000;
    private static final boolean ENGLOAD = "eng".equals(Build.TYPE);

    private static int MAX_GEOCODING_FAILURE_RETRY = 5;
    private static int MAX_NETWORK_LOCATION_RETRY  = 15;

    /* Event Message definition for hanlder */
    private static final int BASE = 3000;
    public static final int EVENT_GET_LOCATION_REQUEST                 = BASE + 0;
    private static final int EVENT_GET_LAST_KNOWN_LOCATION             = BASE + 1;
    private static final int EVENT_HANDLE_NETWORK_LOCATION_RESPONSE    = BASE + 2;
    private static final int EVENT_HANDLE_LAST_KNOWN_LOCATION_RESPONSE = BASE + 3;
    private static final int EVENT_SET_LOCATION_INFO                   = BASE + 4;
    private static final int EVENT_SET_COUNTRY_CODE                    = BASE + 5;
    private static final int EVENT_DIALING_E911                        = BASE + 6;
    private static final int EVENT_RETRY_GET_LOCATION_REQUEST          = BASE + 7;
    private static final int EVENT_ALL_RETRY_GET_LOCATION_REQUST       = BASE + 8;
    private static final int EVENT_LOCATION_MODE_CHANGED               = BASE + 9;
    private static final int EVENT_REQUEST_NETWORK_LOCATION            = BASE + 10;
    private static final int EVENT_LOCATION_CACHE                      = BASE + 11;
    private static final int EVENT_RETRY_NETWORK_LOCATION_REQUEST      = BASE + 12;
    private static final int EVENT_LOCATION_PROVIDERS_CHANGED          = BASE + 13;

    private static final int RESPONSE_SET_LOCATION_INFO                = BASE + 100;
    private static final int RESPONSE_SET_LOCATION_ENABLED             = BASE + 101;

    private static int NETWORK_LOCATION_UPDATE_TIME = 5000;

    private static final String ACTION_LOCATED_PLMN_CHANGED
            = "com.mediatek.intent.action.LOCATED_PLMN_CHANGED";
    private static final String EXTRA_ISO = "iso";

    private Context mContext;
    private Context mDeviceContext;
    private int mSimCount;
    private TelephonyManager mTelephonyManager;
    private TelecomManager mTelecomManager;
    private CallStateListener mCallStateListener = new CallStateListener();

    private ArrayList<LocationInfo> mLocationInfoQueue = new ArrayList<LocationInfo>();

    private ArrayList<LocationInfo> mNetworkLocationTasks = new ArrayList<LocationInfo>();
    private LocationManager mLocationManager;
    private Geocoder mGeoCoder;
    private LocationListenerImp mLocationListener = new LocationListenerImp();


    // For prevent set the same country code to MD.
    private String mPlmnCountryCode = "";

    private String mWifiMacAddr = "";

    private WfcHandler mWfcHandler;
    private WifiPdnHandler mWifiPdnHandler;
    private MwiRIL[] mMwiRil;

    private boolean mNetworkAvailable = false;

    private static final int REQUEST_NETWORK_LOCATION_RETRY_TIMEOUT = 3000;
    private static final int REQUEST_LOCATION_RETRY_TIMEOUT = 5000;
    private ArrayList <Message> mPendingLocationRequest = new ArrayList<>();
    private Object mLocationRequestLock = new Object();

    private boolean mLocationTimeout = false;
    private Object mLocationTimeoutLock = new Object();

    private boolean mLocationSetting = false;
    // The last location setting
    private boolean mLastLocationSetting = false;

    // log task
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean USR_BUILD = TextUtils.equals(Build.TYPE, "user")
            || TextUtils.equals(Build.TYPE, "userdebug");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private static final String COUNTRY_CODE_HK = "HK";

    private static final String ACTION_LOCATION_CACHE = "com.mediatek.intent.action.LOCATION_HANDLE";
    private static final String CACHE_ENABLE_EXTRA = "enable_location_handle";
    private static final int CACHE_ENABLE = 1;
    private static final int CACHE_DISABLE = 0;
    private static final String KEY_LOCATION_CACHE = "key_ocation_cache";
    private static final String KEY_LOCATION_CACHE_ACCOUNTID = "key_accountid";
    private static final String KEY_LOCATION_CACHE_BROADCASTFLAG = "key_broadcastflag";
    private static final String KEY_LOCATION_CACHE_LATITUDE = "key_latitude";
    private static final String KEY_LOCATION_CACHE_LONGTITUDE = "key_longitude";
    private static final String KEY_LOCATION_CACHE_ACCURACY = "key_accuracy";
    private static final String KEY_LOCATION_CACHE_METHOD = "key_method";
    private static final String KEY_LOCATION_CACHE_CITY = "key_city";
    private static final String KEY_LOCATION_CACHE_STATE = "key_state";
    private static final String KEY_LOCATION_CACHE_ZIP = "key_zip";
    private static final String KEY_LOCATION_CACHE_COUNTRYCODE = "key_countrycode";

    private int mGeocodingFailRetry;
    private int mNetworkLocationRetry;
    private String mGnssProxyPackageName;
    private PackageManager mPackageManager;
    private CarrierConfigManager mConfigManager;
    private static final String LOCATION_PERMISSION_NAME =
            "android.permission.ACCESS_FINE_LOCATION";
    // for add-on decouple with MtkCarrierConfigManager.MTK_KEY_WFC_GET_LOCATION_ALWAYS
    public static final String MTK_KEY_WFC_GET_LOCATION_ALWAYS =
            "mtk_carrier_wfc_get_location_always";

    class CallStateListener extends PhoneStateListener {
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            log("onCallStateChanged state=" + state);
            if (state == TelephonyManager.CALL_STATE_OFFHOOK && isEccInProgress()) {
                log("E911 is dialing");
                if (!mWifiPdnHandler.isWifiConnected()) {
                    log("Wi-Fi isn't connected");
                    return;
                }
                obtainMessage(EVENT_DIALING_E911).sendToTarget();
            }
        }
    }

    private class LocationListenerImp implements LocationListener {
        @Override
        public void onLocationChanged(Location location) {
            log("onLocationChanged: " + location);

            long newNlpTime = location.getTime();
            log("onLocationChanged newNlpTime: " + newNlpTime);
            boolean isCache = false;
            for (LocationInfo locationInfo : mLocationInfoQueue) {
                log("onLocationChanged locationInfo time: " + locationInfo.mTime);
                if (locationInfo.mTime == newNlpTime) {
                    isCache = true;
                }
            }

            if (isCache) {
                log("onLocationChanged isCache: " + isCache);
                mGeocodingFailRetry++; // add retry time back.
            }

            cancelNetworkGeoLocationRequest();
            synchronized (mLocationTimeoutLock) {
                mLocationTimeout = false;
            }
            // Remove get last known location if callback within delay time
            log("removeMessages: EVENT_GET_LAST_KNOWN_LOCATION");
            removeMessages(EVENT_GET_LAST_KNOWN_LOCATION);

            obtainMessage(EVENT_HANDLE_NETWORK_LOCATION_RESPONSE, 0, 0, location)
                .sendToTarget();
        }

        @Override
        public void onProviderDisabled(String provider) {
            log("onProviderDisabled: " + provider);
        }

        @Override
        public void onProviderEnabled(String provider) {
            log("onProviderEnabled: " + provider);
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            log("onStatusChanged: " + provider + ", status=" + status);
        }
    }

    public class LocationInfo {
        public int mSimIdx;
        public int mAccountId;
        public int mBroadcastFlag;

        public double mLatitude;
        public double mLongitude;
        public double mAccuracy;
        public String mMethod = "";
        public String mCity = "";
        public String mState = "";
        public String mZip = "";
        public String mCountryCode = "";
        public long mTime;

        LocationInfo(int simIdx, int accountId,
                     int broadcastFlag, double latitude, double longitude, double accuracy) {
            mSimIdx = simIdx;
            mAccountId = accountId;
            mBroadcastFlag = broadcastFlag;
            mLatitude = latitude;
            mLongitude = longitude;
            mAccuracy = accuracy;
            mTime = 0;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("[LocationInfo objId: ");
            sb.append(System.identityHashCode(this));
            sb.append(", phoneId: " + mSimIdx);
            sb.append(", transationId: " + mAccountId);
            //sb.append(", latitude: " + mLatitude);
            //sb.append(", longitude: " + mLongitude);
            sb.append(", accuracy: " + mAccuracy);
            sb.append(", broadcastFlag: " + mBroadcastFlag);
            sb.append(", method: " + mMethod);
            sb.append(", city: " + mCity);
            sb.append(", state: " + mState);
            sb.append(", zip: " + mZip);
            sb.append(", countryCode: " + mCountryCode);
            sb.append(", time: " + mTime);
            return sb.toString();
        }
    }

    @Override
    public void handleMessage(Message msg) {
        LocationInfo info;

        log("handleMessage: msg= " + messageToString(msg));
        switch (msg.what) {
            case EVENT_GET_LOCATION_REQUEST:
                if(isCtaNotAllow()){
                    return;
                }
                if (!mWifiPdnHandler.isWifiConnected() && !mNetworkAvailable) {
                    log("Wi-Fi isn't connected and network unavailable.");
                    addRetryLocationRequest(msg);
                    return;
                }
                handleLocationRequest(msg);
                break;
            case EVENT_RETRY_GET_LOCATION_REQUEST:
                handleRetryLocationRequest(msg);
                break;
            case EVENT_ALL_RETRY_GET_LOCATION_REQUST:
                handleAllRetryLocationRequest();
                break;
            case EVENT_HANDLE_NETWORK_LOCATION_RESPONSE:
            case EVENT_HANDLE_LAST_KNOWN_LOCATION_RESPONSE: {
                Location location = (Location) msg.obj;
                handleNetworkLocationUpdate(location);
                break;
            }
            case EVENT_SET_LOCATION_INFO: {
                LocationInfo locationInfo = (LocationInfo) msg.obj;
                setLocationInfo(locationInfo);
                break;
            }
            case EVENT_SET_COUNTRY_CODE:
                String iso = (String) msg.obj;
                if (!TextUtils.isEmpty(iso)) {
                    if (TextUtils.isEmpty(mPlmnCountryCode)) {
                        // First camp on network, set country to MD
                        setCountryCode(iso);
                    } else if (!iso.equals(mPlmnCountryCode)) {
                        if (mWifiPdnHandler.isWifiConnected() || mNetworkAvailable) {
                            // Country code changed, query network location
                            dispatchLocationRequest(new LocationInfo(0, 0, 0, 0, 0, 0));
                        } else {
                            setCountryCode(iso);
                        }
                    }
                    mPlmnCountryCode = iso;
                }
                break;
            case EVENT_LOCATION_MODE_CHANGED:
                mLocationSetting = mLocationManager.isLocationEnabled();
                // Not send same location setting to modem
                if (mLocationSetting != mLastLocationSetting) {
                    setLocationEnabled();
                } else {
                    log("Same location setting:" + mLocationSetting);
                }
                break;
            case EVENT_LOCATION_PROVIDERS_CHANGED: //fall through
            case EVENT_REQUEST_NETWORK_LOCATION: // fall through
            case EVENT_DIALING_E911:
                // Try to get network location and update to MD
                dispatchLocationRequest(new LocationInfo(0, 0, 0, 0, 0, 0));
                break;
            case EVENT_GET_LAST_KNOWN_LOCATION:
                synchronized (mLocationTimeoutLock) {
                    mLocationTimeout = true;
                }
                if (mWfcHandler != null) {
                    mWfcHandler.onLocationTimeout();
                } else {
                    Rlog.e(TAG, "EVENT_GET_LAST_KNOWN_LOCATION: WfcHandler null");
                }
                info = (LocationInfo) msg.obj;
                boolean getLocationSuccess = getLastKnownLocation(info);

                if (getLocationSuccess) {
                    cancelNetworkGeoLocationRequest();
                }
                break;
            case RESPONSE_SET_LOCATION_INFO:
            case RESPONSE_SET_LOCATION_ENABLED:
                // Do nothing
                break;
            case EVENT_LOCATION_CACHE: {
                int cacheEnable = msg.arg1;
                Rlog.d(TAG, "cacheEnable: " + cacheEnable);
                SharedPreferences sp =
                        PreferenceManager.getDefaultSharedPreferences(mDeviceContext);
                SharedPreferences.Editor editor = sp.edit();
                if (cacheEnable == CACHE_ENABLE) {
                    getLocationFromSysProp(new LocationInfo(0, 0, 0, 0, 0, 0));
                    setLocationCacheEnable(true);
                    LocationInfo locationInfo = new LocationInfo(
                            0, 8, 0, 0.0, 0.0, 0.0);
                    proccessLocationFromNetwork(locationInfo);
                } else {
                    setLocationCacheEnable(false);
                    if (!editor.commit()) {
                        log("Failed to commit location cache");
                    }
                    cancelNetworkGeoLocationRequest();
                }
                break;
            }
            case EVENT_RETRY_NETWORK_LOCATION_REQUEST: {
                info = (LocationInfo) msg.obj;
                if (!requestGeoLocationFromNetworkLocation()) {
                    if (mNetworkLocationRetry-- > 0) {
                        log("EVENT_RETRY_NETWORK_LOCATION_REQUEST retry.");
                        sendMessageDelayed(
                                obtainMessage(EVENT_RETRY_NETWORK_LOCATION_REQUEST, 0, 0, info),
                                REQUEST_NETWORK_LOCATION_RETRY_TIMEOUT);
                    } else {
                        log("EVENT_RETRY_NETWORK_LOCATION_REQUEST retry fail, skip.");
                        mNetworkLocationTasks.remove(info);
                        mNetworkLocationRetry = MAX_NETWORK_LOCATION_RETRY;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    private String messageToString(Message msg) {
        switch (msg.what) {

            case EVENT_GET_LOCATION_REQUEST:
                return "EVENT_GET_LOCATION_REQUEST";
            case EVENT_HANDLE_NETWORK_LOCATION_RESPONSE:
                return "EVENT_HANDLE_NETWORK_LOCATION_RESPONSE";
            case EVENT_SET_LOCATION_INFO:
                return "EVENT_SET_LOCATION_INFO";
            case EVENT_SET_COUNTRY_CODE:
                return "EVENT_SET_COUNTRY_CODE";
            case EVENT_GET_LAST_KNOWN_LOCATION:
                return "EVENT_GET_LAST_KNOWN_LOCATION";
            case EVENT_HANDLE_LAST_KNOWN_LOCATION_RESPONSE:
                return "EVENT_HANDLE_LAST_KNOWN_LOCATION_RESPONSE";
            case EVENT_DIALING_E911:
                return "EVENT_DIALING_E911";
            case RESPONSE_SET_LOCATION_INFO:
                return "RESPONSE_SET_LOCATION_INFO";
            case RESPONSE_SET_LOCATION_ENABLED:
                return "RESPONSE_SET_LOCATION_ENABLED";
            case EVENT_REQUEST_NETWORK_LOCATION:
                return "EVENT_REQUEST_NETWORK_LOCATION";
            case EVENT_RETRY_GET_LOCATION_REQUEST:
                return "EVENT_RETRY_GET_LOCATION_REQUEST";
            case EVENT_ALL_RETRY_GET_LOCATION_REQUST:
                return "EVENT_ALL_RETRY_GET_LOCATION_REQUST";
            case EVENT_LOCATION_MODE_CHANGED:
                return "EVENT_LOCATION_MODE_CHANGED";
            case EVENT_LOCATION_PROVIDERS_CHANGED:
                return "EVENT_LOCATION_PROVIDERS_CHANGED";
            case EVENT_LOCATION_CACHE:
                return "EVENT_LOCATION_CACHE";
            case EVENT_RETRY_NETWORK_LOCATION_REQUEST:
                return "EVENT_RETRY_NETWORK_LOCATION_REQUEST";
            default:
                return "UNKNOWN";
        }
    }

    public Handler getHandler() {
        return this;
    }

    public WfcLocationHandler(Context context, WfcHandler wfcHandler,
        WifiPdnHandler wifiPdnHandler, int simCount, Looper looper, MwiRIL[] mwiRil) {

        super(looper);
        mContext = context;
        mDeviceContext = mContext.createDeviceProtectedStorageContext();
        if (mDeviceContext == null) {
            mDeviceContext = mContext;
            log("replace mContext to mDeviceContext");
        }
        mWfcHandler = wfcHandler;
        mWifiPdnHandler = wifiPdnHandler;
        mSimCount = (simCount <= MAX_VALID_SIM_COUNT) ? simCount : MAX_VALID_SIM_COUNT;
        mMwiRil = mwiRil;

        mGeocodingFailRetry = MAX_GEOCODING_FAILURE_RETRY;
        mNetworkLocationRetry = MAX_NETWORK_LOCATION_RETRY;

        mGeoCoder = new Geocoder(mContext, Locale.US);
        mLocationManager = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);

        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        mTelephonyManager.listen(mCallStateListener, PhoneStateListener.LISTEN_CALL_STATE);

        mLocationSetting = mLocationManager.isLocationEnabled();
        // 1st time send location setting to modem
        log("1st time send location setting to modem, mLocationSetting:" + mLocationSetting);
        setLocationEnabled();

        mTelecomManager = (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
        mGnssProxyPackageName = loadProxyNameFromCarrierConfig();
        mPackageManager = mContext.getPackageManager();
        mConfigManager = (CarrierConfigManager)
                mContext.getSystemService(Context.CARRIER_CONFIG_SERVICE);

        registerForBroadcast();
        registerIndication();
        registerDefaultNetwork();
    }

    protected void log(String s) {
        if (!USR_BUILD || TELDBG) {
            Rlog.d(TAG, s);
        }
    }

    private void addRetryLocationRequest(Message msg) {
        synchronized(mLocationRequestLock) {
            AsyncResult ar = (AsyncResult) msg.obj;
            String[] result = (String[]) ar.result;
            String[] retryRet = new String[6];
            String[] delayRet = new String[6];
            System.arraycopy(result, 0, retryRet, 0, result.length);
            System.arraycopy(result, 0, delayRet, 0, result.length);
            AsyncResult retryAr = new AsyncResult(null, retryRet, null);
            AsyncResult delayAr = new AsyncResult(null, delayRet, null);

            Message retryMsg = obtainMessage(EVENT_RETRY_GET_LOCATION_REQUEST, retryAr);
            mPendingLocationRequest.add(retryMsg);
            log("add, current PendingLocationRequest size: " + mPendingLocationRequest.size());
            Message delayMsg = obtainMessage(EVENT_RETRY_GET_LOCATION_REQUEST, delayAr);
            sendMessageDelayed(delayMsg, REQUEST_LOCATION_RETRY_TIMEOUT);
        }
    }

    private void handleRetryLocationRequest(Message msg) {
        synchronized(mLocationRequestLock) {
            log("Current PendingLocationRequest size: " + mPendingLocationRequest.size());
            if (mWifiPdnHandler.isWifiConnected() || mNetworkAvailable) {
                handleLocationRequest(msg);
            } else {
                log("Network not available, ignore EVENT_RETRY_GET_LOCATION_REQUEST.");
            }
        }
    }

    private void handleAllRetryLocationRequest() {
        log("handleAllRetryLocationRequest mPendingLocationRequest.size(): " + mPendingLocationRequest.size());
        synchronized(mLocationRequestLock) {
            if (hasMessages(EVENT_RETRY_GET_LOCATION_REQUEST)) {
                removeMessages(EVENT_RETRY_GET_LOCATION_REQUEST);
            }
            for (Message msg : mPendingLocationRequest) {
                if (mWifiPdnHandler.isWifiConnected() || mNetworkAvailable) {
                    handleLocationRequest(msg);
                } else {
                    log("Network not available, ignore EVENT_RETRY_GET_LOCATION_REQUEST.");
                }
            }
            mPendingLocationRequest.clear();
        }
    }

    private void registerDefaultNetwork() {
        ConnectivityManager cm = ConnectivityManager.from(mContext);
        cm.registerDefaultNetworkCallback(new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                log("NetworkCallback.onAvailable()");
                mNetworkAvailable = true;
                sendEmptyMessage(EVENT_ALL_RETRY_GET_LOCATION_REQUST);
            }

            @Override
            public void onLost(Network network) {
                log("NetworkCallback.onLost()");
                mNetworkAvailable = false;
            }
        });
    }


    /** Geo Location **/
    private void handleLocationRequest(Message msg) {
        if (null == msg.obj) {
            Rlog.e(TAG, "handleLocationInfo(): msg.obj is null");
            return;
        }
        AsyncResult ar = (AsyncResult) msg.obj;
        String[] result = (String[]) ar.result;

        if (null == result) {
            Rlog.e(TAG, "handleLocationInfo(): result is null");
            return;
        }

        try {
            int accId = Integer.parseInt(result[0]);
            int broadcastFlag = Integer.parseInt(result[1]);
            double latitude = Double.parseDouble(result[2]);
            double longitude = Double.parseDouble(result[3]);
            double accuracy = Double.parseDouble(result[4]);
            int simIdx = Integer.parseInt(result[5]);


            LocationInfo locationInfo = new LocationInfo(
                simIdx, accId, broadcastFlag, latitude, longitude, accuracy);

            log("handleGeoLocationRequest(): " + locationInfo);
            dispatchLocationRequest(locationInfo);
        } catch (Exception e) {
            log("handleGeoLocationRequest()[" + result.length + "]" + result[0] + " " +
                result[1] + " " +result[2] + " " + result[3] + " " + result[4] + " " + result[5]);
        }

    }

    private void dispatchLocationRequest(LocationInfo info) {
        double latitude = info.mLatitude;
        double longitude = info.mLongitude;
        double accuracy = info.mAccuracy;
        // if no gps signal, use Wifi location
        if (latitude == 0 && longitude == 0 && accuracy == 0) {
            proccessLocationFromNetwork(info);
        } else {
            cancelNetworkGeoLocationRequest();
            synchronized (mLocationTimeoutLock) {
                mLocationTimeout = false;
            }
            log("removeMessages: EVENT_GET_LAST_KNOWN_LOCATION");
            removeMessages(EVENT_GET_LAST_KNOWN_LOCATION);
            info.mMethod = "GPS";
            mLocationInfoQueue.add(info);
            pollLocationInfo();
        }
        log("dispatchLocationRequest(): " + info.mMethod);
    }

    private void handleNetworkLocationUpdate(Location location) {
        if (location == null) {
            log("network location get null, unexpected result");
            return;
        }

        double latitude = location.getLatitude();
        double longitude = location.getLongitude();
        double accuracy = location.getAccuracy();
        long time = location.getTime();
        log("update all LocationInfo with "
            + " time: " + time
            + " accuracy: " + accuracy
            );

        ArrayList<LocationInfo> duplicatedInfo = new ArrayList<LocationInfo>();
        for (LocationInfo locationInfo : mNetworkLocationTasks) {
            locationInfo.mLatitude = latitude;
            locationInfo.mLongitude = longitude;
            locationInfo.mAccuracy = accuracy;
            locationInfo.mTime = time;
            duplicatedInfo.clear();

            for (LocationInfo gpsLocationInfo : mLocationInfoQueue) {
                if (gpsLocationInfo.mAccountId == locationInfo.mAccountId) {
                    duplicatedInfo.add(gpsLocationInfo);
                }
            }
            for (LocationInfo gpsLocationInfo : duplicatedInfo) {
                mLocationInfoQueue.remove(gpsLocationInfo);
            }
            mLocationInfoQueue.add(locationInfo);
        }

        pollLocationInfo();
        mNetworkLocationTasks.clear();
    }

    private void proccessLocationFromNetwork(LocationInfo info) {
        info.mMethod = "Network";
        mNetworkLocationTasks.add(info);
        if (!requestGeoLocationFromNetworkLocation()) {
            log("requestGeoLocationFromNetworkLocation failed");
            // Only country code is valid, set to MD
            setLocationInfo(info);

            if (hasMessages(EVENT_RETRY_NETWORK_LOCATION_REQUEST)) {
                removeMessages(EVENT_RETRY_NETWORK_LOCATION_REQUEST);
                mNetworkLocationRetry = MAX_NETWORK_LOCATION_RETRY;
            }
            if (mNetworkLocationRetry-- > 0) {
                log("requestGeoLocationFromNetworkLocation retry.");
                sendMessageDelayed(
                        obtainMessage(EVENT_RETRY_NETWORK_LOCATION_REQUEST, 0, 0, info),
                        REQUEST_NETWORK_LOCATION_RETRY_TIMEOUT);
            } else {
                log("requestGeoLocationFromNetworkLocation retry fail, skip.");
                mNetworkLocationTasks.remove(info);
                mNetworkLocationRetry = MAX_NETWORK_LOCATION_RETRY;
            }
        }
        if (!hasMessages(EVENT_GET_LAST_KNOWN_LOCATION)) {
            // Trying to get last known location if no locationChanges callback after delay time
            log("Add delayed message: EVENT_GET_LAST_KNOWN_LOCATION");
            sendMessageDelayed(obtainMessage(EVENT_GET_LAST_KNOWN_LOCATION, 0, 0, info),
                                            REQUEST_GEOLOCATION_FROM_NETWORK_TIMEOUT);
        }
    }

    private void pollLocationInfo() {
        if (mLocationInfoQueue.isEmpty()) {
            log("No GeoLocation task");
            return;
        }

        // Avoid race condition
        List<LocationInfo> LocationInfoQueueCopy = new ArrayList<LocationInfo>(mLocationInfoQueue);
        mLocationInfoQueue.clear();

        new Thread(new Runnable() {
            @Override
            public void run() {
                for (LocationInfo gpsLocationInfo : LocationInfoQueueCopy) {
                    Boolean retry = false;
                    LocationInfo res = getGeoLocationFromLatLong(gpsLocationInfo, retry);
                    if (res != null) {
                        obtainMessage(EVENT_SET_LOCATION_INFO, 0, 0, res).sendToTarget();
                    }

                    if (retry) {
                        log("GeoCoding fail, retry = " + mGeocodingFailRetry);
                        if (mWifiPdnHandler.isWifiConnected() && mGeocodingFailRetry > 0) {
                            mGeocodingFailRetry--;
                            obtainMessage(EVENT_REQUEST_NETWORK_LOCATION).sendToTarget();
                        }
                    }
                }
            }
        }).start();

    }

    private void setLocationInfo(LocationInfo info) {
        if (TextUtils.isEmpty(info.mState)) {
            info.mState = "Unknown";
        }
        // Use lastest PLMN country code
        if ((!TextUtils.isEmpty(mPlmnCountryCode) && TextUtils.length(info.mCountryCode) != 2)
                || COUNTRY_CODE_HK.equals(mPlmnCountryCode)) {
            info.mCountryCode = mPlmnCountryCode;
        } else if (TextUtils.isEmpty(mPlmnCountryCode) &&
                TextUtils.length(info.mCountryCode) != 2) {
            // Use sim country code if location and PLMN country code are empty
            info.mCountryCode = getSimCountryCode();
        } else if (TextUtils.length(info.mCountryCode) == 2) {
            // Update country code from location
            mPlmnCountryCode = info.mCountryCode;
        }
        log("setLocationInfo info=" + info + ", mPlmnCountryCode:" + mPlmnCountryCode);

        Message result = obtainMessage(RESPONSE_SET_LOCATION_INFO);
        getMwiRil().setLocationInfo(
            Integer.toString(info.mAccountId),
            Integer.toString(info.mBroadcastFlag),
            String.valueOf(info.mLatitude),
            String.valueOf(info.mLongitude),
            String.valueOf(info.mAccuracy),
            info.mMethod,
            info.mCity,
            info.mState,
            info.mZip,
            info.mCountryCode,
            WifiPdnHandler.getUeWlanMacAddr(), result);
        pollLocationInfo();
    }
    /**
     * This function will be executed in worker thread.
     */
    private LocationInfo getGeoLocationFromLatLong(LocationInfo location, Boolean retry) {
        retry = false;
        if (mGeoCoder == null) {
            log("getGeoLocationFromLatLong: empty geoCoder, return an empty location");
            return location;
        }

        if (!mGeoCoder.isPresent()) {
            log("getGeoLocationFromLatLong: this system has no GeoCoder implementation!!");
            return location;
        }

        double lat = location.mLatitude;
        double lng = location.mLongitude;

        List<Address> lstAddress = null;
        try {
            lstAddress = mGeoCoder.getFromLocation(lat, lng, 1);
        } catch (IOException e) {
            log("mGeoCoder.getFromLocation throw exception:" + e);
        } catch (Exception e2) {
            log("mGeoCoder.getFromLocation throw exception:" + e2);
        }

        if (lstAddress == null || lstAddress.isEmpty()) {
            log("getGeoLocationFromLatLong: get empty address");
            if (getLocationCacheEnable()) {
                getLocationFromSysProp(location);
                if ("".equals(location.mCity)) {
                    return null;
                }
                return location;
            } else {
                retry = true;
                return location;
            }
        }

        location.mCity = lstAddress.get(0).getLocality();
        if (TextUtils.isEmpty(location.mCity)) {
            location.mCity = lstAddress.get(0).getSubAdminArea();
        }
        location.mState = lstAddress.get(0).getAdminArea();  // 'CA';
        if (TextUtils.isEmpty(location.mState)) {
            location.mState = lstAddress.get(0).getCountryName();
        }
        location.mZip = lstAddress.get(0).getPostalCode();
        location.mCountryCode = lstAddress.get(0).getCountryCode();

        if (getLocationCacheEnable()) {
            saveLocationToSysProp(location);
        }

        log("getGeoLocationFromLatLong: location=" + location);

        return location;
    }

    private String getSimCountryCode() {
        String simCountryCode = mTelephonyManager.getSimCountryIso().toUpperCase(Locale.US);

        log("getSimCountryCode: " + simCountryCode);
        return simCountryCode;
    }

    private void setLocationCacheEnable(boolean enable) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mDeviceContext);
        SharedPreferences.Editor editor = sp.edit();

        editor.putBoolean(KEY_LOCATION_CACHE, enable);

        if (!editor.commit()) {
            log("Failed to commit location cache");
        }
    }

    private boolean getLocationCacheEnable() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mDeviceContext);
        return sp.getBoolean(KEY_LOCATION_CACHE, false);
    }

    private void saveLocationToSysProp(LocationInfo location) {
        log("saveLocationToSysProp");

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mDeviceContext);
        SharedPreferences.Editor editor = sp.edit();

        editor.putInt(KEY_LOCATION_CACHE_ACCOUNTID, location.mAccountId);
        editor.putInt(KEY_LOCATION_CACHE_BROADCASTFLAG, location.mBroadcastFlag);

        editor.putLong(KEY_LOCATION_CACHE_LATITUDE, (long) location.mLatitude);
        editor.putLong(KEY_LOCATION_CACHE_LONGTITUDE, (long) location.mLongitude);
        editor.putLong(KEY_LOCATION_CACHE_ACCURACY, (long) location.mAccuracy);

        editor.putString(KEY_LOCATION_CACHE_METHOD, location.mMethod);
        editor.putString(KEY_LOCATION_CACHE_CITY, location.mCity);
        editor.putString(KEY_LOCATION_CACHE_STATE, location.mState);
        editor.putString(KEY_LOCATION_CACHE_ZIP, location.mZip);
        editor.putString(KEY_LOCATION_CACHE_COUNTRYCODE, location.mCountryCode);

        if (!editor.commit()) {
            log("Failed to commit saveLocationToSysProp");
        }

        LocationInfo tmplocation = new LocationInfo(0, 0, 0, 0, 0, 0);
        getLocationFromSysProp(tmplocation);
    }

    private LocationInfo getLocationFromSysProp(LocationInfo location) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mDeviceContext);

        location.mAccountId = sp.getInt(KEY_LOCATION_CACHE_ACCOUNTID, 0);

        if (location.mBroadcastFlag == 0) {
            location.mBroadcastFlag = sp.getInt(KEY_LOCATION_CACHE_BROADCASTFLAG, 0);
        }

        location.mLatitude = (double) sp.getLong(KEY_LOCATION_CACHE_LATITUDE, (long) 0);

        location.mLongitude = (double) sp.getLong(KEY_LOCATION_CACHE_LONGTITUDE, (long) 0);

        location.mAccuracy = (double) sp.getLong(KEY_LOCATION_CACHE_ACCURACY, (long) 0);

        location.mMethod = sp.getString(KEY_LOCATION_CACHE_METHOD, "");
        location.mCity = sp.getString(KEY_LOCATION_CACHE_CITY, "");
        location.mState = sp.getString(KEY_LOCATION_CACHE_STATE, "");
        location.mZip = sp.getString(KEY_LOCATION_CACHE_ZIP, "");
        location.mCountryCode = sp.getString(KEY_LOCATION_CACHE_COUNTRYCODE, "");

        log("getGeoLocationFromLatLong (from cache): location=" + location);
        return location;
    }

    private boolean getLastKnownLocation(LocationInfo info) {
        log("getLastKnownLocation");

        if (mLocationManager == null) {
            log("getLastKnownLocation: empty locationManager, return");
            return false;
        }

        // Avoid "provider doesn't exist: network" JE
        if (mLocationManager.getProvider(LocationManager.GPS_PROVIDER) == null) {
            log("getLastKnownLocation: GPS_PROVIDER doesn't exist or not ready");
            return false;
        }

        // Get cached GPS location
        Location gpsLocation =
            mLocationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);

        if (gpsLocation != null) {

            log("GPS location: " + gpsLocation);
            if (System.currentTimeMillis() - gpsLocation.getTime() < 1800000) {
                obtainMessage(
                        EVENT_HANDLE_LAST_KNOWN_LOCATION_RESPONSE, 0, 0, gpsLocation)
                        .sendToTarget();
                return true;
            }
        }

        // Avoid "provider doesn't exist: network" JE
        if (mLocationManager.getProvider(LocationManager.NETWORK_PROVIDER) == null) {
            log("getLastKnownLocation: NETWORK_PROVIDER doesn't exist or not ready");
            return false;
        }


        // Get cached network location
        Location networkLocation =
            mLocationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);

        if (networkLocation != null) {

            log("Network location: " + networkLocation);
            if (System.currentTimeMillis() - networkLocation.getTime() < 1800000) {
                obtainMessage(
                        EVENT_HANDLE_LAST_KNOWN_LOCATION_RESPONSE, 0, 0, networkLocation)
                        .sendToTarget();
                return true;
            }
        }

        log("getLastKnownLocation: no last known location");
        // Maybe only country code is valid, set to MD
        setLocationInfo(info);
        return false;
    }

    private boolean requestGeoLocationFromNetworkLocation() {
        if (mLocationManager == null) {
            log("requestGeoLocationFromNetworkLocation failed: empty locationManager");
            return false;
        }

        // Avoid "provider doesn't exist: network" JE
        if (mLocationManager.getProvider(LocationManager.NETWORK_PROVIDER) == null) {
            log("requestGeoLocationFromNetworkLocation failed: NETWORK_PROVIDER not ready");
            return false;
        }

        if(isCtaNotAllow()) {
            log("requestGeoLocationFromNetworkLocation failed: CTA not allow");
            return false;
        }

        boolean isProxyAppPermissionGranted = checkLocationProxyAppPermission();
        boolean isEcc = isEccInProgress();
        boolean isGetLocAlways = isGetLocationAlways();
        boolean mustGetLocation = isEcc || isGetLocAlways;
        if(mustGetLocation) {
            addPackageInLocationSettingsWhitelist();
        }
        if(mustGetLocation || isProxyAppPermissionGranted) {
            LocationRequest request = LocationRequest.createFromDeprecatedProvider(
                    LocationManager.NETWORK_PROVIDER, NETWORK_LOCATION_UPDATE_TIME /*minTime*/,
                    0 /*minDistance*/, false/*oneShot*/);
            request.setHideFromAppOps(true);
            request.setLocationSettingsIgnored(mustGetLocation);
            mLocationManager.requestLocationUpdates(request,mLocationListener,this.getLooper());
            Rlog.d(TAG, "requestGeoLocationFromNetworkLocation: success");
            return true;
        } else {
            Rlog.d(TAG, "requestGeoLocationFromNetworkLocation failed:"
                + " is NOT in ECC & non-framework location proxy app is NOT granted");
            return false;
        }
    }

    private void cancelNetworkGeoLocationRequest() {
        log("cancelNetworkGeoLocationRequest");
        if (hasMessages(EVENT_RETRY_NETWORK_LOCATION_REQUEST)) {
            removeMessages(EVENT_RETRY_NETWORK_LOCATION_REQUEST);
        }
        if (mLocationManager == null) {
            log("cancelNetworkGeoLocationRequest: empty locationManager, return");
            return;
        }
        mLocationManager.removeUpdates(mLocationListener);
        removePackageInLocationSettingsWhitelist();
        Rlog.d(TAG, "cancelNetworkGeoLocationRequest");
    }

    private void addPackageInLocationSettingsWhitelist() {
        final String LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST =
                "location_ignore_settings_package_whitelist";
        String packageName = mContext.getPackageName();

        String whitelist = Settings.Global.getString(
                mContext.getContentResolver(),
                LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST);
        if (whitelist == null || whitelist.indexOf(packageName) == -1) {
            String outStr = (whitelist == null) ? "" : whitelist + ",";
            outStr += packageName;
            log("Add WFC in location setting whitelist:" + outStr);
            Settings.Global.putString(
                    mContext.getContentResolver(),
                    LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST,
                    outStr);
        }
    }

    private void removePackageInLocationSettingsWhitelist() {
        final String LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST =
                "location_ignore_settings_package_whitelist";
        String packageName = mContext.getPackageName();

        String whitelist = Settings.Global.getString(
                mContext.getContentResolver(),
                LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST);
        int index = -1;
        String outStr = "";
        if (whitelist != null) {
            index = whitelist.indexOf("," + packageName);
            if (index != -1) { /// found ','+package
                outStr = whitelist.replace("," + packageName, "");
            } else { /// not found, try to find package name only
                index = whitelist.indexOf(packageName);
                if(index != -1) {
                    outStr = whitelist.replace(packageName, "");
                }
            }
        }

        if (index != -1) { /// outStr is replaced as new whitelist
            log("Remove WFC in location setting whitelist:" + outStr);
            Settings.Global.putString(
                    mContext.getContentResolver(),
                    LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST,
                    outStr);
        }
    }

    private void utGeoLocationRequest() {
        LocationInfo locationInfo = new LocationInfo(
            0, 8, 0, 212.0, 147.0, 1);
        dispatchLocationRequest(locationInfo);
    }

    private void utNetworkLocationRequest() {
        LocationInfo locationInfo = new LocationInfo(
            0, 8, 0, 0.0, 0.0, 0.0);
        dispatchLocationRequest(locationInfo);
    }

    // Send location setting to modem
    private void setLocationEnabled() {
        Message result = obtainMessage(RESPONSE_SET_LOCATION_ENABLED);

        log("setLocationEnabled(): last location setting:" + mLastLocationSetting
            + ", new location setting:" + mLocationSetting);

        // Send location setting to modem
        getMwiRil().setWfcConfig(MwiRIL.WfcConfigType.WFC_SETTING_LOCATION_SETTING.ordinal(),
            "locenable",(mLocationSetting)? "1":"0",result);

        // After send location setting to modem, update the mLastLocationSetting
        mLastLocationSetting = mLocationSetting;
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
        log("getMainCapabilityPhoneId = " + phoneId);
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

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null || intent.getAction() == null) {
                return;
            }
            log("onReceive action:" + intent.getAction());
            if (intent.getAction().equals(ACTION_LOCATED_PLMN_CHANGED)) {
                String lowerCaseCountryCode = (String) intent.getExtra(EXTRA_ISO);
                if (lowerCaseCountryCode != null && !TextUtils.isEmpty(lowerCaseCountryCode)) {
                    String isoCountryCode = lowerCaseCountryCode.toUpperCase();
                    log("ACTION_LOCATED_PLMN_CHANGED, iso: " + isoCountryCode);
                    if(!isCtaNotAllow()) {
                        obtainMessage(EVENT_SET_COUNTRY_CODE, isoCountryCode).sendToTarget();
                    }
                } else {
                    log("iso country code is null");
                }
            } else if (intent.getAction().equals(LocationManager.MODE_CHANGED_ACTION)) {
                obtainMessage(EVENT_LOCATION_MODE_CHANGED).sendToTarget();
            } else if (intent.getAction().equals(LocationManager.PROVIDERS_CHANGED_ACTION)) {
                boolean isNlpEnabled
                        = mLocationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
                synchronized (mLocationTimeoutLock) {
                    log("LocationManager.PROVIDERS_CHANGED_ACTION isNlpEnabled: " + isNlpEnabled
                            + ", location timeout = " + mLocationTimeout);
                    if (mLocationTimeout && isNlpEnabled) {
                        obtainMessage(EVENT_LOCATION_PROVIDERS_CHANGED).sendToTarget();
                    }
                }
            } else if (intent.getAction().equals(ACTION_LOCATION_CACHE)) {
                int cacheEnable = intent.getIntExtra(CACHE_ENABLE_EXTRA, CACHE_DISABLE);
                Message msg = obtainMessage(EVENT_LOCATION_CACHE, cacheEnable, 0);
                msg.sendToTarget();
            }
        }
    };

    private void setCountryCode(String iso) {
        LocationInfo info = new LocationInfo(0, 0, 0, 0, 0, 0);
        info.mCountryCode = iso;
        setLocationInfo(info);
    }

    private void registerForBroadcast() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_LOCATED_PLMN_CHANGED);
        filter.addAction(LocationManager.MODE_CHANGED_ACTION);
        filter.addAction(LocationManager.PROVIDERS_CHANGED_ACTION);
        filter.addAction(ACTION_LOCATION_CACHE);
        mContext.registerReceiver(mReceiver, filter);
    }

    private void registerIndication() {

        for(int i = 0 ; i < mSimCount ; i++) {
            mMwiRil[i].registerRequestGeoLocation(this,
                    EVENT_GET_LOCATION_REQUEST, null);
        }
    }

    private boolean isEccInProgress() {
        boolean isInEcc = false;
        if (mTelecomManager != null) {
            isInEcc = mTelecomManager.isInEmergencyCall();
        }
        log("isEccInProgress: " + isInEcc);
        return isInEcc;
    }

    /* Specific operator request WFC to get location info always.
     * Whether or not it is ECC.
     * Whether or not user allow device to get location info.
     * return: true, if operator request WFC to get location info always.
     */
    private boolean isGetLocationAlways() {
        mConfigManager = (CarrierConfigManager)
                        mContext.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        if (mConfigManager == null) {
            log("isGetLocationAlways: Carrier Config service is NOT ready");
            return false;
        }

        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        PersistableBundle configs = SubscriptionManager.isValidSubscriptionId(subId)
                ? mConfigManager.getConfigForSubId(subId) : null;
        if (configs == null) {
            log("isGetLocationAlways: SIM not ready, use default carrier config");
            configs = CarrierConfigManager.getDefaultConfig();
        }

        boolean getLocationAlways
            = configs.getBoolean(MTK_KEY_WFC_GET_LOCATION_ALWAYS);
        log("isGetLocationAlways: " + getLocationAlways);
        return getLocationAlways;

    }

    /* For CTA security requirement, we ONLY allow device to get the location information
     * when the location setting enable
     * return: true, it is CTA load and location setting disabled, NOT allow to get location info
     */
    private boolean isCtaNotAllow(){
        boolean isCtaNotAllow = false;

        // Feature option: MTK_CTA_SET
        boolean isCtaSet = SystemProperties.getInt("ro.vendor.mtk_cta_set", 0) == 1;
        // Feature option: MTK_MOBILE_MANAGEMENT
        boolean isCtaSecurity = SystemProperties.getInt("ro.vendor.mtk_mobile_management", 0) == 1;
        // Check AOSP location setting
        boolean isNlpEnabled
                = mLocationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
        log("isCtaNotAllow: isCtaSet:" + isCtaSet
                + ", isCtaSecurity:" + isCtaSecurity
                + ", isNlpEnabled:" + isNlpEnabled);
        if (isCtaSet && isCtaSecurity && !isNlpEnabled) {
            isCtaNotAllow = true;
        }
        log("isCtaNotAllow: " + isCtaNotAllow);
        return isCtaNotAllow;
    }

    private String loadProxyNameFromCarrierConfig() {
        mConfigManager = (CarrierConfigManager)
                        mContext.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        if (mConfigManager == null) {
            log("loadProxyNameFromCarrierConfig: Carrier Config service is NOT ready");
            return "";
        }

        int ddSubId = SubscriptionManager.getDefaultDataSubscriptionId();
        PersistableBundle configs = SubscriptionManager.isValidSubscriptionId(ddSubId)
                ? mConfigManager.getConfigForSubId(ddSubId) : null;
        if (configs == null) {
            log("SIM not ready, use default carrier config");
            configs = CarrierConfigManager.getDefaultConfig();
        }

        String value = (String) configs.get(CarrierConfigManager.Gps.KEY_NFW_PROXY_APPS_STRING);
        log(CarrierConfigManager.Gps.KEY_NFW_PROXY_APPS_STRING + ": " + value);
        if (value != null) {
            String[] strings = value.trim().split(" ");
            /// use first one package as proxy package
            return strings[0];
        } else {
            log("Cannot get location proxy APP package name");
            return "";
        }
    }

    private boolean checkLocationProxyAppPermission() {
        mGnssProxyPackageName = loadProxyNameFromCarrierConfig();
        boolean proxyAppLocationGranted =
            (mPackageManager.checkPermission(LOCATION_PERMISSION_NAME, mGnssProxyPackageName)
                == PackageManager.PERMISSION_GRANTED) ? true:false;
        log("proxyAppLocationGranted: " + proxyAppLocationGranted);
        return proxyAppLocationGranted;
    }
}
