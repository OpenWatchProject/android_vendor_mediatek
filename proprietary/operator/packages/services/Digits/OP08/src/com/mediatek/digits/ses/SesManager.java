package com.mediatek.digits.ses;

import android.app.AlertDialog;
import android.content.Context;
import android.provider.Settings;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.digits.DigitsConfig;
import com.mediatek.digits.DigitsSharedPreference;
import com.mediatek.digits.DigitsUtil;
import com.mediatek.digits.utils.Utils;
import com.mediatek.digits.utils.CallbackScheduler;
import com.mediatek.digits.R;

import java.lang.Boolean;
import java.lang.StringBuilder;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.text.ParseException;
import java.text.SimpleDateFormat;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONException;

public class SesManager extends Handler {
    private static final String TAG = "sesManager";

    private static final String DEVICE_GROUP_ID = "MTK-NSDS-Bianco-TMO-1.00";

    public static final int CMD_GET_DEVICE_CONFIG = 0;
    public static final int CMD_ON_BOARDING = 1;
    public static final int CMD_REFRESH_PUSH_TOKEN = 2;
    public static final int CMD_GET_ACCESS_TOKEN = 3;
    public static final int CMD_REGISTERED_DEVICESE = 4;
    public static final int CMD_REGISTERED_MSISDN= 5;
    public static final int CMD_INIT_SUBSCRIBE = 6;
    public static final int CMD_DELETE_DEVICE = 7;
    public static final int CMD_SET_DEVICE_NAME = 8;
    public static final int CMD_SET_LINE_NAME = 9;
    public static final int CMD_PROVISION_LINE = 10;
    public static final int CMD_REMOVE_LINE = 11;
    public static final int CMD_INITIATE_TMOID_REATION = 12;
    public static final int CMD_RENEW_MSISDN_SIT = 13;
    public static final int CMD_UPDATE_MSISDN_SIT = 14;
    public static final int CMD_ADDRESS_REGISTRATION = 15;
    public static final int CMD_USER_LOGOUT = 30;

    public static final int CMD_DIAL611_DIALOG_TEST = 101;
    public static final int CMD_OK_DIALOG_TEST = 102;

    public static final int PUSH_TOKEN_EVENT_CONN_MGMT = 0;
    public static final int PUSH_TOKEN_EVENT_CONN_MGR = 1;
    public static final int PUSH_TOKEN_EVENT_CONN_INFO = 2;
    public static final int PUSH_TOKEN_EVENT_APP_MGMT = 3;

    public static final int SERVICE_NAME_CONN_MGR = 0;
    public static final int SERVICE_NAME_CONN_VOWIFI = 1;
    public static final int SERVICE_NAME_CONN_VVM = 2;
    public static final int SERVICE_NAME_CONN_MESSAGING = 3;
    public static final int SERVICE_NAME_VIDEO_APP = 4;
    public static final int SERVICE_NAME_SHAREPOINT_APP = 5;
    public static final int SERVICE_NAME_TIMEREPORTING_APP = 6;

    /*
     */
    public static final int EVENT_GET_DEVICE_CONFIG_RESULT = 0;
    /*
     * 1. key: EVENT_KEY_NATIVE_MSISDN, value: native msisdn
     * 2. key: EVENT_KEY_SERVER_URL value: server url - if need E911 address
     * 3. key: EVENT_KEY_SERVER_DATA value: server data - if need E911 address
     * 4. key: EVENT_KEY_DEVICE_MAP
     */
    public static final int EVENT_ON_BOARDING_RESULT = 1;

    /*
     * 1. key: EVENT_KEY_DEVICE_MAP: a hashmap using device ID as key
     * 2. key: EVENT_KEY_LINE_MAP: a hashmap using msisdn as a key
     * 3. key: EVENT_KEY_NATIVE_MSISDN
     */
    public static final int EVENT_INIT_SUBSCRIBE_RESULT = 3;
    /*
     * 1. GCM token
     */
    public static final int EVENT_REFRESH_PUSH_TOKEN_RESULT = 4;
    /*
     * 1. Activiated device list, key: EVENT_KEY_DEVICE_MAP
     */
    public static final int EVENT_REGISTERED_DEVICES_RESULT = 5;
    /*
     * 1. Activiated line list
     */
    public static final int EVENT_REGISTERED_MSISDN_RESULT = 6;
    public static final int EVENT_PREPARE_LOGOUT_RESULT = 7;
    /*
     * 1. key: EVENT_KEY_NAME, value: Modified device name
     */
    public static final int EVENT_SET_DEVICE_NAME_RESULT = 8;
    /*
     * 1. key: EVENT_KEY_MSISDN, value: msisdn
     * 2. key: EVENT_KEY_NAME, value: Modified line name
     */
    public static final int EVENT_SET_LINE_NAME_RESULT = 9;
    /*
     * 1. key: EVENT_KEY_MSISDN, value: msisdn
     * 2. key: EVENT_KEY_SIT, value: SIT
     * 3. key: EVENT_KEY_DEVICE_MAP
     */
    public static final int EVENT_PROVISION_LINE_RESULT = 10;
    /*
     * 1. MSISDN
     */
    public static final int EVENT_REMOVE_LINE_RESULT = 11;
    /*
     * 1. key: MSISDN
     * 2. key: SIT
     * 3. key: EVENT_KEY_DEVICE_MAP
     */
    public static final int EVENT_UPDATE_MSISDN_SIT_RESULT = 12;
    /*
     * 1. key: MSISDN
     * 2. key: SIT
     */
    public static final int EVENT_SIT_UPDATED = 13;
    /*
     * None
     */
    public static final int EVENT_DELETE_DEVICE_RESULT = 14;

    /*
     * 1. key: EVENT_KEY_SERVER_URL value: server url - if need E911 address
     * 2. key: EVENT_KEY_SERVER_DATA value: server data - if need E911 address
     */
    public static final int EVENT_ON_ADDRESS_REGISTRATION = 15;

    /*
     * key: MSISDN
     */
    public static final int EVENT_GET_MSISDN_RESULT = 25;

    /*
     * None
     */
    public static final int EVENT_USER_LOGOUT = 20;

    /*
     * 1. key: EVENT_KEY_MSISDN, value: msisdn
     * 2. key: EVENT_KEY_SIT, value: SIT
     * 3. key: EVENT_KEY_DEVICE_MAP
     */
    public static final int EVENT_SIT_RENEW = 21;

    /*
     * None
     */
    public static final int EVENT_SES_HTTP_AUTH_FAIL = 22;

    public static final String EVENT_KEY_LOCATION_STATUS = "location-status";
    public static final String EVENT_KEY_SERVER_URL = "server_url";
    public static final String EVENT_KEY_SERVER_DATA = "server_data";
    public static final String EVENT_KEY_MSISDN = "msisdn";
    public static final String EVENT_KEY_NAME = "name";
    public static final String EVENT_KEY_SIT = "sit";

    public static final String EVENT_KEY_NATIVE_MSISDN = "native_msisdn";
    public static final String EVENT_KEY_GCM_SENDER_ID = "gcm_sender_id";
    public static final String EVENT_KEY_DEVICE_MAP = "device_map";
    public static final String EVENT_KEY_LINE_MAP = "line_map";

    public static final int EVENT_RESULT_SUCCEED = 0;
    public static final int EVENT_RESULT_FAIL = 1;

    private final SesServerApi mApi;
    private String mMSISDN;
    private SesListener mListener = null;
    private String mServiceFingerprint;

    private String mDeviceGroupId;

    private String mAppToken;    // GCM token for push token parameter
    private String mAccessToken;  // manageConnectivityRequest header
    private String mServiceToken; //managePushTokenRequest header
    private String mVIMSI;

    private HashMap<String, DeviceInfo> mDeviceInfo = new HashMap<String, DeviceInfo>();
    private HashMap<String, RegisteredMSISDN> mRegisteredMSISDN = new HashMap<String, RegisteredMSISDN>();
    private HashMap<String, MsisdnSIT> mMsisdnSits = new HashMap<String, MsisdnSIT>();
    ServiceInstance mPrimaryServiceInstance;
    private CallbackScheduler mSITRefreshTimer;
    private AlertDialog mAlertDialog = null;

    /** Interface for callback to notify SES event */
    public interface SesListener {
        /**
         * callback when any events happened.
         */
        void onSesEvent(int event, int result, Bundle bundle);
    }

    private class LineName {
        private String mMsisdn;
        private String mName;
        public LineName(String msisdn, String name) {
            mMsisdn = msisdn;
            mName = name;
        }

        public String getMsisdn() {
            return mMsisdn;
        }

        public String getName() {
            return mName;
        }
    }

    private class MsisdnSIT {
        private String mMsisdn;
        private String mLineSIT;
        private String mExpireTime;
        private long mExpireTimeMillion;
        SimpleDateFormat mSdf = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'");

        public MsisdnSIT(String msisdn, String lineSIT, String expireTime) {
            mMsisdn = msisdn;
            mLineSIT = lineSIT;
            mExpireTime = expireTime;
            try {
                long currentTime = System.currentTimeMillis();
                Date date = mSdf.parse(mExpireTime);
                mExpireTimeMillion = (long)((date.getTime() - currentTime) * 0.9) + currentTime;
            } catch (ParseException e) {
                Log.e(TAG, "Date ParseException error: ", e);
            }
            Log.d(TAG, "MsisdnSIT mMsisdn: " + mMsisdn + ", expireTime: " + expireTime +
                  ", mExpireTimeMillion: " + mExpireTimeMillion);
        }

        public String getMsisdn() {
            return mMsisdn;
        }

        public String getLineSIT() {
            return mLineSIT;
        }

        public String geExpireTime() {
            return mExpireTime;
        }

        public long geExpireTimeMillion() {
            return mExpireTimeMillion;
        }

        public String toString() {
            return new String("mMsisdn : " + mMsisdn +
                              ", mLineSIT : " + mLineSIT +
                              ", mExpireTime : " + mExpireTime);
        }
    }

    public class ServiceInstance {
        public String mServiceName;
        public String mServiceInstanceId;
        public boolean mIsOwner;
        public String mServiceOwnerId;
        public String mMsisdn;
        public String mFriendlyName;

        public String toString() {
            return new String("service-name : " + mServiceName +
                              ", service-instance-id : " + mServiceInstanceId +
                              ", is-owner : " + mIsOwner +
                              ", service-owner-id : " + mServiceOwnerId +
                              ", msisdn : " + mMsisdn +
                              ", friendly-name : " + mFriendlyName);
        }
    }

    public class DeviceInfo {
        public String mDeviceId;
        public int mDeviceType;
        public String mDeviceName;
        public String mDeviceOwnerId;
        public Map<String, ServiceInstance> mServiceInstances
            = new HashMap<String, ServiceInstance>();

        public String getServiceInstId(String msisdn, String serviceName) {
            ServiceInstance serviceInst = mServiceInstances.get(msisdn);
            if (serviceInst != null) {
                if (serviceInst.mServiceName.equals(serviceName)) {
                    return serviceInst.mServiceInstanceId;
                }
            }

            return null;
        }

        public String toString() {
            StringBuilder out = new StringBuilder();
            out.append("device-id : " + mDeviceId);
            out.append(", device-type : " + mDeviceType);
            out.append(", device-name : " + mDeviceName);
            out.append(", device-owner-id : " + mDeviceOwnerId);

            Iterator it = mServiceInstances.entrySet().iterator();
            while (it.hasNext()) {
                Map.Entry pair = (Map.Entry)it.next();
                out.append(", {mServiceInstances[" + pair.getKey() + "]:"
                           + pair.getValue().toString() +
                           "}");
            }
            return out.toString();
        }
    }

    public class RegisteredMSISDN {
        public boolean mDefaultAccount;
        public String mLineName;
        public boolean mIsMobilityEnabled;
        public String mSourceOwnerId;
        public String mMsisdn;
        public String mImsi;
        public String mLineMetadata;
        public boolean mIsOwner;
        public String mServiceFingerprint;

        public String toString() {
            return new String("default-account : " + mDefaultAccount +
                              ", line-name : " + mLineName +
                              ", is-mobility-enabled : " + mIsMobilityEnabled +
                              ", source-owner-id : " + mSourceOwnerId +
                              ", msisdn : " + mMsisdn +
                              ", imsi : " + mImsi +
                              ", line-metadata : " + mLineMetadata +
                              ", is-owner : " + mIsOwner +
                              ", service-fingerprint : " + mServiceFingerprint);
        }
    }

    Context mContext;

    public SesManager(Context context, Looper looper) {
        super(looper);
        mContext = context;
        mApi = new SesServerApi(mContext, getSesApiEndpoint(),
                                DigitsUtil.getInstance(mContext).getDeviceName());
        mDeviceGroupId = getDeviceGroupId();
        initTokenRefresher();
    }

    public SesManager(Context context, Looper looper, SesListener listener) {
        super(looper);
        mContext = context;
        TelephonyManager telephonyManager =
            (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);

        mContext = context;
        mListener = listener;
        mVIMSI = telephonyManager.getSubscriberId() + "@msg.pc.t-mobile.com";
        mApi = new SesServerApi(mContext, getSesApiEndpoint(),
                                DigitsUtil.getInstance(mContext).getDeviceName());
        mDeviceGroupId = getDeviceGroupId();
        initTokenRefresher();
    }

    private void initTokenRefresher() {
        mSITRefreshTimer = new CallbackScheduler(
            mContext,
            "com.mediatek.digits.refreshSIT",
        new CallbackScheduler.Callback() {
            public void run(String action) {
                refreshMsisdnSit();
            }
        });
    }

    @Override
    public void handleMessage(Message message) {
        String deviceName;
        String msisdn;

        switch(message.what) {
            case CMD_GET_DEVICE_CONFIG:
                doGetDeviceConfig();
                break;
            case CMD_ON_BOARDING:
                Boolean logout = (Boolean)message.obj;
                doOnBoarding(logout.booleanValue());
                break;
            case CMD_ADDRESS_REGISTRATION:
                Boolean forceUpdate = (Boolean)message.obj;
                doAddressRegistration(forceUpdate.booleanValue());
                break;
            case CMD_REFRESH_PUSH_TOKEN:
                doRefreshPushToken();
                break;
            case CMD_INIT_SUBSCRIBE:
                doInitSubscribe();
                break;
            case CMD_REGISTERED_DEVICESE:
                doRegisteredDevices();
                break;
            case CMD_REGISTERED_MSISDN:
                doRegisteredMsisdn();
                break;
            case CMD_DELETE_DEVICE:
                doDeleteDevice();
                break;
            case CMD_SET_DEVICE_NAME:
                deviceName = (String)message.obj;
                doSetDeviceName(deviceName);
                break;
            case CMD_SET_LINE_NAME:
                LineName lineName = (LineName)message.obj;
                doSetLineName(lineName.getMsisdn(), lineName.getName());
                break;
            case CMD_PROVISION_LINE:
                msisdn = (String)message.obj;
                doProvisionLine(msisdn, false);
                break;
            case CMD_REMOVE_LINE:
                msisdn = (String)message.obj;
                doRemoveLine(msisdn, false);
                break;
            case CMD_RENEW_MSISDN_SIT:
                msisdn = (String)message.obj;
                if (!doRenewMsisdnSit(msisdn)) {
                    refreshMsisdnSit();
                }
                break;
            case CMD_UPDATE_MSISDN_SIT:
                msisdn = (String)message.obj;
                doUpdateMsisdnSit(msisdn);
                break;
            case CMD_USER_LOGOUT:
                mListener.onSesEvent(EVENT_USER_LOGOUT, EVENT_RESULT_FAIL, null);
                break;
            case CMD_DIAL611_DIALOG_TEST:
                handleSesError(SesServerApi.METHOD_GET_MSISDN, SesServerApi.RES_AKA_AUTH_FAILED, 0, "");
                break;
            case CMD_OK_DIALOG_TEST:
                handleSesError(SesServerApi.METHOD_GET_MSISDN, SesServerApi.RES_INVALID_REQUEST, 0, "");
            default:
                Log.e(TAG, "Unhandled message:" + message.what);
                break;
        }
    }

    /**
     * Get device config. It's an asynchronous API.
     *
     */
    public void getDeviceConfig() {
        Message message = obtainMessage(CMD_GET_DEVICE_CONFIG);
        message.sendToTarget();
    }

    // Call when boot up. It's an asynchronous API.
    public void onBoarding(boolean logout, String appToken) {
        mAppToken = appToken;
        Message message = obtainMessage(CMD_ON_BOARDING, new Boolean(logout));
        message.sendToTarget();
    }

    // Call when user wants to update E911 address
    public void addressRegistration(boolean forceUpdatre) {
        Message message = obtainMessage(CMD_ADDRESS_REGISTRATION, new Boolean(forceUpdatre));
        message.sendToTarget();
    }

    // Call when refresh push token. It's an asynchronous API.
    public void refreshPushToken(String appToken) {
        mAppToken = appToken;
        Message message = obtainMessage(CMD_REFRESH_PUSH_TOKEN);
        message.sendToTarget();
    }

    // Call when log in and SIM ready. It's an asynchronous API.
    public void initSubscribe () {
        Message message = obtainMessage(CMD_INIT_SUBSCRIBE);
        message.sendToTarget();
    }

    // Call when need. It's an asynchronous API.
    public void registeredDevices () {
        Message message = obtainMessage(CMD_REGISTERED_DEVICESE);
        message.sendToTarget();
    }

    // Call when need. It's an asynchronous API.
    public void registeredMsisdn () {
        Message message = obtainMessage(CMD_REGISTERED_MSISDN);
        message.sendToTarget();
    }

    // Call when need. It's an asynchronous API.
    public void getMsisdn() {

    }

    /**
     * Delete device. It's an asynchronous API.
     *
     */
    public void deleteDevice() {
        // used to flush cache data
        Message message = obtainMessage(CMD_DELETE_DEVICE);
        message.sendToTarget();
    }


    /**
     * Set device name. It's an asynchronous API.
     *
     * @param name a friendly device name
     */
    public void setDeviceName(String name) {
        Message message = obtainMessage(CMD_SET_DEVICE_NAME, (Object)name);
        message.sendToTarget();
    }

    /**
     * Set line name. It's an asynchronous API.
     *
     * @param msisdn the line number
     * @param name the line name
     */
    public void setLineName(String msisdn, String name) {
        Message message = obtainMessage(CMD_SET_LINE_NAME,
                                        (Object)new LineName(msisdn, name));
        message.sendToTarget();
    }

    /**
     * Activate a line. It's an asynchronous API.
     *
     * @param msisdn the line number.
     * @param isVirtual if it's a virtual line
     */
    public void provisionLine(String msisdn) {
        Message message = obtainMessage(CMD_PROVISION_LINE,
                                        (Object)msisdn);
        message.sendToTarget();
    }

    /**
     * Remove a line. It's an asynchronous API.
     *
     * @param msisdn the line number.
     */
    public void removeLine(String msisdn) {
        Message message = obtainMessage(CMD_REMOVE_LINE, (Object)msisdn);
        message.sendToTarget();
    }

    /**
     * Update the SIT of the MSISDN vowifi service
     *
     * @param msisdn the line number.
     */
    public void updateMsisdnSit(String msisdn) {
        Message message = obtainMessage(CMD_UPDATE_MSISDN_SIT, (Object)msisdn);
        message.sendToTarget();
    }

    private synchronized void refreshMsisdnSit() {
        String refreshMsisdn = null;
        long callbackTime = 0;

        // Retrive next (nearest time) expire SIT
        Entry<String, MsisdnSIT>min = null;
        for (Entry<String, MsisdnSIT> entry : mMsisdnSits.entrySet()) {
            if (min == null ||
                    min.getValue().geExpireTimeMillion() > entry.getValue().geExpireTimeMillion()) {
                min = entry;
            }
        }

        callbackTime = min.getValue().geExpireTimeMillion() - System.currentTimeMillis();
        Log.d(TAG, "refreshMsisdnSit callbackTime = " + callbackTime);
        if (callbackTime > 300000) {
            mSITRefreshTimer.stop();
            mSITRefreshTimer.start(callbackTime);
        } else {
            // Expired within 5 minutes
            Log.d(TAG, "refreshMsisdnSit" + min.getKey() + " will be expired");
            Message message = obtainMessage(CMD_RENEW_MSISDN_SIT, (Object)min.getKey());
            sendMessageDelayed(message, callbackTime);
            mMsisdnSits.remove(min.getKey());
        }
    }

    private void scheduleRefreshMsisdnSit(String msisdn, String sit, String expireTime) {
        Log.d(TAG, "scheduleRefreshMsisdnSit msisdn = " + msisdn +
              ", expireTime = " + expireTime);
        mMsisdnSits.remove(msisdn);
        mMsisdnSits.put(msisdn, new MsisdnSIT(msisdn, sit, expireTime));
        refreshMsisdnSit();
    }

    /**
     * Stop all the activity. It's an synchronous API.
     *
     */
    public void stop() {
        removeCallbacksAndMessages(null);
    }

    public void setAccessToken(String accessToken) {
        Log.d(TAG, "setAccessToken = " + accessToken);
        mAccessToken = accessToken;
        mApi.setAccessToken(mAccessToken);
    }

    public void setServiceToken(String serviceToken) {
        Log.d(TAG, "setServiceToken = " + serviceToken);
        mServiceToken = serviceToken;
        mApi.setAccessToken(mAccessToken);
    }

    private String getSesApiEndpoint() {
        // By default, connect to T-MO stage 1 Server

        String sesUrl = SystemProperties.get("persist.vendor.digits.sesurl",
                                             "https://eas3.msg.t-mobile.com/generic_devices");
        Log.d(TAG, "sesUrl = " + sesUrl);
        return sesUrl;
    }

    private void updateMsisdn(SesServerApi.Response rsp) {
        mMSISDN = rsp.getString(SesServerApi.METHOD_GET_MSISDN, "msisdn");
        Log.d(TAG, "new mMSISDN = " + mMSISDN);
        mServiceFingerprint = rsp.getString(SesServerApi.METHOD_GET_MSISDN, "service-fingerprint");
    }

    private boolean updateDeviceInfo(SesServerApi.Response rsp) {
        String devInfos = rsp.getString(SesServerApi.METHOD_REGISTER_DEVICES, "device-info");
        String deviceId = DigitsUtil.getInstance(mContext).getDeviceId();
        boolean updatePrimaryServiceInst = false;

        Log.d(TAG, "devInfo: " + devInfos);
        if (devInfos != null) {
            try {
                mDeviceInfo.clear();
                JSONArray arDevInfo = new JSONArray(devInfos);
                for (int i = 0; i < arDevInfo.length(); i++) {
                    JSONObject jDevInfo = arDevInfo.getJSONObject(i);
                    DeviceInfo devInfo = new DeviceInfo();
                    boolean localDevInfo = false;
                    devInfo.mDeviceId =
                        new String(Utils.base64Decode(jDevInfo.optString("device-id", null)));
                    if (!TextUtils.isEmpty(deviceId) && devInfo.mDeviceId.equals(deviceId)) {
                        localDevInfo = true;
                    }
                    devInfo.mDeviceType = jDevInfo.optInt("device-type", -1);
                    devInfo.mDeviceName = jDevInfo.optString("device-name", null);
                    devInfo.mDeviceOwnerId = jDevInfo.optString("device-owner-id", null);

                    String strServiceInsts = jDevInfo.optString("service-instances", null);
                    if (strServiceInsts != null) {
                        JSONArray jServiceInsts = new JSONArray(strServiceInsts);
                        devInfo.mServiceInstances.clear();

                        for (int j = 0; j < jServiceInsts.length(); j++) {
                            JSONObject jServiceInst = jServiceInsts.getJSONObject(j);
                            ServiceInstance serviceInst = new ServiceInstance();
                            serviceInst.mServiceName =
                                jServiceInst.optString("service-name", null);
                            serviceInst.mServiceInstanceId =
                                jServiceInst.optString("service-instance-id", null);
                            serviceInst.mIsOwner =
                                jServiceInst.optBoolean("is-owner", false);
                            serviceInst.mServiceOwnerId =
                                jServiceInst.optString("service-owner-id", null);
                            serviceInst.mMsisdn =
                                jServiceInst.optString("msisdn", null);
                            if (localDevInfo && serviceInst.mMsisdn.equals(mMSISDN)) {
                                mPrimaryServiceInstance = serviceInst;
                            }
                            serviceInst.mFriendlyName =
                                jServiceInst.optString("friendly-name", null);
                            devInfo.mServiceInstances.put(serviceInst.mMsisdn, serviceInst);
                        }
                        // Somrtimes primary line info isn't contained in new coming DevInfo
                        // Put old info for primary serviceInstance for changing line name
                        // Device ID and MSISDN equal local's
                        if (localDevInfo && mPrimaryServiceInstance != null &&
                                devInfo.getServiceInstId(mMSISDN,
                                    getServiceNameString(SERVICE_NAME_CONN_VOWIFI)) == null) {
                            Log.d(TAG, "Put old mPrimaryServiceInstance");
                            devInfo.mServiceInstances.put(mMSISDN, mPrimaryServiceInstance);
                        }
                    }
                    Log.d(TAG, "devInfo[" + i + "]=" + devInfo.toString());
                    mDeviceInfo.put(devInfo.mDeviceId, devInfo);
                }
            } catch (JSONException e) {
                Log.e(TAG, "JSON object conversion error: ", e);
                return false;
            }
        }
        return true;
    }

    private boolean updateRegisteredMsisdn(SesServerApi.Response rsp) {
        String regMsisdns = rsp.getString(SesServerApi.METHOD_REGISTER_MSISDN, "registered-msisdns");
        Log.d(TAG, "registered-msisdns: " + regMsisdns);
        if (regMsisdns != null) {
            try {
                mRegisteredMSISDN.clear();
                JSONArray arRegMsisdns = new JSONArray(regMsisdns);
                for (int i = 0; i < arRegMsisdns.length(); i++) {
                    JSONObject jRegMsisdn = arRegMsisdns.getJSONObject(i);
                    RegisteredMSISDN regMsisdn = new RegisteredMSISDN();
                    regMsisdn.mDefaultAccount = jRegMsisdn.optBoolean("default-account", false);
                    regMsisdn.mLineName = jRegMsisdn.optString("line-name", null);
                    regMsisdn.mIsMobilityEnabled = jRegMsisdn.optBoolean("is-mobility-enabled", false);
                    regMsisdn.mSourceOwnerId = jRegMsisdn.optString("source-owner-id", null);
                    regMsisdn.mMsisdn = jRegMsisdn.optString("msisdn", null);
                    regMsisdn.mImsi = jRegMsisdn.optString("imsi", null);
                    String lineMetadata =
                        new String(Utils.base64Decode(jRegMsisdn.optString("line-metadata", null)));
                    regMsisdn.mLineMetadata = lineMetadata;
                    regMsisdn.mIsOwner = jRegMsisdn.optBoolean("is-owner", false);
                    regMsisdn.mServiceFingerprint = jRegMsisdn.optString("service-fingerprint", null);
                    Log.d(TAG, "regMsisdn[" + i + "]=" + regMsisdn.toString());
                    mRegisteredMSISDN.put(regMsisdn.mMsisdn, regMsisdn);
                }
            } catch (JSONException e) {
                Log.e(TAG, "JSON object conversion error: ", e);
                return false;
            }
        }
        return true;
    }

    private boolean updateRegisteredMsisdns(SesServerApi.Response rsp) {
        String msisdns = rsp.getString(SesServerApi.METHOD_REGISTER_MSISDN, "registered-msisdns");
        Log.d(TAG, "msisdns: " + msisdns);
        if (msisdns != null) {
            try {
                JSONArray arMsisdns = new JSONArray(msisdns);
                for (int i = 0; i < arMsisdns.length(); i++) {
                    String strMsisdn = arMsisdns.getString(i);
                    Log.d(TAG, "registered-msisdn : " + strMsisdn);
                }
            } catch (JSONException e) {
                Log.e(TAG, "JSON object conversion error: ", e);
                return false;
            }
        }
        return true;
    }

    private void doGetDeviceConfig() {
        Log.d(TAG, "doGetDeviceConfig");
        SesServerApi.Response rsp = mApi.createMethod()
                                    .addManageConnectivity(SesServerApi.MANAGE_CONN_OPERATION_CONFIGURE,
                                            mDeviceGroupId, "")
                                    .sendAndWait();

        if (!handleSesResp(rsp, SesServerApi.METHOD_MANAGE_CONN,
                           SesServerApi.MANAGE_CONN_OPERATION_CONFIGURE,
                           "")) {
            Log.e(TAG, "doGetDeviceConfig: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_GET_DEVICE_CONFIG_RESULT, EVENT_RESULT_FAIL, null);
        } else {
            String deviceConfig64 = rsp.getString(SesServerApi.METHOD_MANAGE_CONN, "device-config");
            byte decodeddeviceConfig64[] = Utils.base64Decode(deviceConfig64);
            String deviceConfigStr = new String(decodeddeviceConfig64);
            Log.d(TAG, "deviceConfigStr: " + deviceConfigStr);
            DigitsConfig.createInstance(mContext, deviceConfigStr);
            DigitsConfig.getInstance().dump();
            mListener.onSesEvent(EVENT_GET_DEVICE_CONFIG_RESULT, EVENT_RESULT_SUCCEED, null);
        }
    }

    private void doOnBoarding(boolean logout) {
        Log.d(TAG, "doOnBoarding");
        if (DigitsConfig.getInstance() == null) {
            Log.e(TAG, "DigitsConfig not download yet");
            mListener.onSesEvent(EVENT_ON_BOARDING_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }
        SesServerApi.Response rsp = mApi.createMethod()
                                    .addManageConnectivity(SesServerApi.MANAGE_CONN_OPERATION_ACTIVATE,
                                            mDeviceGroupId,
                                            mVIMSI)
                                    .addGetMSISDN()
                                    .sendAndWait();
        if (!handleSesResp(rsp, SesServerApi.METHOD_MANAGE_CONN,
                           SesServerApi.MANAGE_CONN_OPERATION_ACTIVATE, mVIMSI) ||
                !handleSesResp(rsp, SesServerApi.METHOD_GET_MSISDN)) {
            Log.e(TAG, "onBoarding: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_ON_BOARDING_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }

        updateMsisdn(rsp);
        String serviceNames = rsp.getString(SesServerApi.METHOD_MANAGE_CONN, "service-names");
        Log.d(TAG, "serviceNames: " + serviceNames);
        if (serviceNames != null) {
            try {
                JSONArray arServiceNames = new JSONArray(serviceNames);
                for (int i = 0; i < arServiceNames.length(); i++) {
                    String service = arServiceNames.getString(i);
                    Log.d(TAG, "Service : " + service);
                }
            } catch (JSONException e) {
                Log.e(TAG, "JSON object conversion error: ", e);
                mListener.onSesEvent(EVENT_ON_BOARDING_RESULT, EVENT_RESULT_FAIL, null);
                return;
            }
        }

        SesServerApi.Response rspConnToken = mApi.createMethod()
                                             .addManagePushToken(mMSISDN,
                                                     getServiceNameString(SERVICE_NAME_CONN_MGR),
                                                     SesServerApi.MANAGE_PUSH_TOKEN_OPERATION_REGISTER,
                                                     mAppToken,
                                                     getPushTokenEventString(PUSH_TOKEN_EVENT_CONN_MGR),
                                                     0)
                                             .sendAndWait();
        if (!handleSesResp(rspConnToken, SesServerApi.METHOD_MANAGE_TOKEN)) {
            Log.e(TAG, "onBoarding: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_ON_BOARDING_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }

        if (!logout) {
            SesServerApi.Response rspVowifiToken = mApi.createMethod()
                                                   .addManagePushToken(mMSISDN,
                                                           getServiceNameString(SERVICE_NAME_CONN_VOWIFI),
                                                           SesServerApi.MANAGE_PUSH_TOKEN_OPERATION_REGISTER,
                                                           mAppToken,
                                                           getPushTokenEventString(PUSH_TOKEN_EVENT_CONN_MGR),
                                                           0)
                                                   .sendAndWait();
            if (!handleSesResp(rspVowifiToken, SesServerApi.METHOD_MANAGE_TOKEN)) {
                Log.e(TAG, "onBoarding: receive failed callback, stop procedure now");
                mListener.onSesEvent(EVENT_ON_BOARDING_RESULT, EVENT_RESULT_FAIL, null);
                return;
            }
        }

        SesServerApi.Response rspTc = mApi.createMethod()
                                      .addManageLocationAndTC(mServiceFingerprint)
                                      .sendAndWait();
        if (!handleSesResp(rspTc, SesServerApi.METHOD_LOCATION_TC)) {
            Log.e(TAG, "onBoarding: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_ON_BOARDING_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }

        SesServerApi.SesServiceItem servItem = mApi.new SesServiceItem(
                getServiceNameString(SERVICE_NAME_CONN_VOWIFI),
                mServiceFingerprint);

        SesServerApi.Response rspProv = mApi.createMethod()
                                        .addManageService(
                                            SesServerApi.MANAGE_SERV_OPERATION_PROVISION,
                                            servItem,
                                            null)
                                        .sendAndWait();
        if (!handleSesResp(rspProv, SesServerApi.METHOD_MANAGE_SERVICE,
                           SesServerApi.MANAGE_SERV_OPERATION_PROVISION)) {
            Log.e(TAG, "doGetDeviceConfig: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_ON_BOARDING_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }

        Bundle bundle = new Bundle();

        boolean locationStatus = rspTc.getBoolean(SesServerApi.METHOD_LOCATION_TC, "location-status", true);
        boolean tcStatus = rspTc.getBoolean(SesServerApi.METHOD_LOCATION_TC, "tc-status", true);
        if (!locationStatus || !tcStatus) {
            String serverData = rspTc.getString(SesServerApi.METHOD_LOCATION_TC, "server-data");
            String serverUrl = rspTc.getString(SesServerApi.METHOD_LOCATION_TC, "server-url");
            bundle.putString(EVENT_KEY_SERVER_DATA, serverData);
            bundle.putString(EVENT_KEY_SERVER_URL, serverUrl);
        }

        //Find service-instance of the MSISDN from the response
        String servInst = rspProv.getString(SesServerApi.METHOD_MANAGE_SERVICE, "service-instance");
        String serviceInstIdIn;
        try {
            JSONObject jServInst = new JSONObject(servInst);
            serviceInstIdIn = jServInst.optString("service-instance-id", null);
            Log.d(TAG, "serviceInstId=" + serviceInstIdIn);
        } catch (JSONException e) {
            Log.e(TAG, "JSON object conversion error: ", e);
            mListener.onSesEvent(EVENT_PROVISION_LINE_RESULT, EVENT_RESULT_FAIL, bundle);
            return;
        }

        String servInstToken = getServiceInstToken(mMSISDN, serviceInstIdIn);
        if (TextUtils.isEmpty(servInstToken)) {
            mListener.onSesEvent(EVENT_ON_BOARDING_RESULT, EVENT_RESULT_FAIL, bundle);
            return;
        }
        bundle.putString(EVENT_KEY_SIT, servInstToken);
        bundle.putString(EVENT_KEY_MSISDN, mMSISDN);
        bundle.putSerializable(EVENT_KEY_DEVICE_MAP, mDeviceInfo);
        mListener.onSesEvent(EVENT_ON_BOARDING_RESULT, EVENT_RESULT_SUCCEED, bundle);
    }

    private void doAddressRegistration(boolean forceUpdateAddress) {
        mApi.setAccessToken(null);
        SesServerApi.Response rspTc = mApi.createMethod()
                                      .addManageLocationAndTC(mServiceFingerprint)
                                      .sendAndWait();
        if (!handleSesResp(rspTc, SesServerApi.METHOD_LOCATION_TC)) {
            Log.e(TAG, "doAddressRegistration: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_ON_ADDRESS_REGISTRATION, EVENT_RESULT_FAIL, null);
            mApi.setAccessToken(mAccessToken);
            return;
        }

        Bundle bundle = new Bundle();

        boolean locationStatus = rspTc.getBoolean(SesServerApi.METHOD_LOCATION_TC, "location-status", true);
        boolean tcStatus = rspTc.getBoolean(SesServerApi.METHOD_LOCATION_TC, "tc-status", true);
        if (!locationStatus || !tcStatus || forceUpdateAddress) {
            String serverData = rspTc.getString(SesServerApi.METHOD_LOCATION_TC, "server-data");
            String serverUrl = rspTc.getString(SesServerApi.METHOD_LOCATION_TC, "server-url");
            bundle.putString(EVENT_KEY_SERVER_DATA, serverData);
            bundle.putString(EVENT_KEY_SERVER_URL, serverUrl);
        }
        mApi.setAccessToken(mAccessToken);
        mListener.onSesEvent(EVENT_ON_ADDRESS_REGISTRATION, EVENT_RESULT_SUCCEED, bundle);
    }

    private String getPushTokenEventString(int event) {
        switch(event) {
            case PUSH_TOKEN_EVENT_CONN_MGMT:
                return "conn-mgmt";
            case PUSH_TOKEN_EVENT_CONN_MGR:
                return "conn-mgr";
            case PUSH_TOKEN_EVENT_CONN_INFO:
                return "conn-info";
            case PUSH_TOKEN_EVENT_APP_MGMT:
                return "app-mgmt";
        }
        return "";
    }

    private String getServiceNameString(int servicename) {
        switch(servicename) {
            case SERVICE_NAME_CONN_MGR:
                return "conn-mgr";
            case SERVICE_NAME_CONN_VOWIFI:
                return "vowifi";
            case SERVICE_NAME_CONN_VVM:
                return "video-app";
            case SERVICE_NAME_CONN_MESSAGING:
                return "messaging";
            case SERVICE_NAME_VIDEO_APP:
                return "video-app";
            case SERVICE_NAME_SHAREPOINT_APP:
                return "sharepoint-app";
            case SERVICE_NAME_TIMEREPORTING_APP:
                return "timereporting-app";
        }
        return "";
    }

    private void doRefreshPushToken() {
        Log.d(TAG, "doRefreshPushToken");
        SesServerApi.Response rsp = mApi.createMethod()
                                    .addManagePushToken(mMSISDN,
                                            getServiceNameString(SERVICE_NAME_CONN_MGR),
                                            SesServerApi.MANAGE_PUSH_TOKEN_OPERATION_REGISTER,
                                            mAppToken,
                                            getPushTokenEventString(PUSH_TOKEN_EVENT_CONN_MGR),
                                            0)
                                    .sendAndWait();
        if (!handleSesResp(rsp, SesServerApi.METHOD_MANAGE_TOKEN)) {
            Log.e(TAG, "onBoarding: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_REFRESH_PUSH_TOKEN_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }

        SesServerApi.Response rspVowifiToken = mApi.createMethod()
                                               .addManagePushToken(mMSISDN,
                                                       getServiceNameString(SERVICE_NAME_CONN_VOWIFI),
                                                       SesServerApi.MANAGE_PUSH_TOKEN_OPERATION_REGISTER,
                                                       mAppToken,
                                                       getPushTokenEventString(PUSH_TOKEN_EVENT_CONN_MGR),
                                                       0)
                                               .sendAndWait();
        if (!handleSesResp(rspVowifiToken, SesServerApi.METHOD_MANAGE_TOKEN)) {
            Log.e(TAG, "onBoarding: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_REFRESH_PUSH_TOKEN_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }
        mListener.onSesEvent(EVENT_REFRESH_PUSH_TOKEN_RESULT, EVENT_RESULT_SUCCEED, null);
    }

    private void doInitSubscribe() {
        Log.d(TAG, "doInitSubscribe");
        mApi.setAccessToken(mAccessToken);
        SesServerApi.Response rsp1 = mApi.createMethod()
                                     .addManageConnectivity(SesServerApi.MANAGE_CONN_OPERATION_ADD_OAUTH,
                                             mDeviceGroupId,
                                             mVIMSI)
                                     .addRegisteredDevices()
                                     .addRegisteredMSISDN(SesServerApi.REG_MSISDN_OPERATION_AVAIABLE)
                                     .sendAndWait();
        if (!handleSesResp(rsp1, SesServerApi.METHOD_MANAGE_CONN,
                           SesServerApi.MANAGE_CONN_OPERATION_ADD_OAUTH, mVIMSI) ||
                !handleSesResp(rsp1, SesServerApi.METHOD_REGISTER_DEVICES) ||
                !handleSesResp(rsp1, SesServerApi.METHOD_REGISTER_MSISDN)) {
            Log.e(TAG, "doInitSubscribe: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_INIT_SUBSCRIBE_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }
        updateDeviceInfo(rsp1);
        updateRegisteredMsisdn(rsp1);

        SesServerApi.Response rsp2 = mApi.createMethod()
                                     .addGetMSISDN()
                                     .addRegisteredDevices()
                                     .addRegisteredMSISDN(SesServerApi.REG_MSISDN_OPERATION_AVAIABLE)
                                     .sendAndWait();
        if (!handleSesResp(rsp2, SesServerApi.METHOD_GET_MSISDN) ||
                !handleSesResp(rsp2, SesServerApi.METHOD_REGISTER_DEVICES) ||
                !handleSesResp(rsp2, SesServerApi.METHOD_REGISTER_MSISDN)) {
            Log.e(TAG, "doInitSubscribe: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_INIT_SUBSCRIBE_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }
        updateMsisdn(rsp2);
        updateDeviceInfo(rsp2);
        updateRegisteredMsisdn(rsp2);

        Bundle bundle = new Bundle();
        bundle.putString(EVENT_KEY_MSISDN, mMSISDN);
        bundle.putSerializable(EVENT_KEY_DEVICE_MAP, mDeviceInfo);
        bundle.putSerializable(EVENT_KEY_LINE_MAP, mRegisteredMSISDN);
        mListener.onSesEvent(EVENT_INIT_SUBSCRIBE_RESULT, EVENT_RESULT_SUCCEED, bundle);
    }

    private void doDeleteDevice() {
        Log.d(TAG, "doDeleteDevice");
        mApi.setAccessToken(mAccessToken);
        SesServerApi.Response rsp = mApi.createMethod()
                                    .addManageConnectivity(SesServerApi.MANAGE_CONN_OPERATION_DELETE,
                                            mDeviceGroupId,
                                            mVIMSI)
                                    .sendAndWait();
        if (!handleSesResp(rsp, SesServerApi.METHOD_MANAGE_CONN,
                           SesServerApi.MANAGE_CONN_OPERATION_DELETE, mVIMSI)) {
            Log.e(TAG, "doDeleteDevice: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_DELETE_DEVICE_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }
        mListener.onSesEvent(EVENT_DELETE_DEVICE_RESULT, EVENT_RESULT_SUCCEED, null);
    }

    private void doRegisteredDevices() {
        Log.d(TAG, "doRegisteredDevices");
        SesServerApi.Response rsp = mApi.createMethod()
                                    .addRegisteredDevices()
                                    .sendAndWait();
        if (!handleSesResp(rsp, SesServerApi.METHOD_REGISTER_DEVICES) || !updateDeviceInfo(rsp)) {
            Log.e(TAG, "doRegisteredDevices: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_REGISTERED_DEVICES_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }
        updateDeviceInfo(rsp);
        Bundle bundle = new Bundle();
        bundle.putSerializable(EVENT_KEY_DEVICE_MAP, mDeviceInfo);
        mListener.onSesEvent(EVENT_REGISTERED_DEVICES_RESULT, EVENT_RESULT_SUCCEED, bundle);
    }

    private void doRegisteredMsisdn() {
        Log.d(TAG, "doRegisteredMsisdn");
        SesServerApi.Response rsp = mApi.createMethod()
                                    .addRegisteredMSISDN(SesServerApi.REG_MSISDN_OPERATION_AVAIABLE)
                                    .sendAndWait();
        if (!handleSesResp(rsp, SesServerApi.METHOD_REGISTER_MSISDN)) {
            Log.e(TAG, "doInitSubscribe: METHOD_REGISTER_MSISDN failed , stop procedure now");
            mListener.onSesEvent(EVENT_INIT_SUBSCRIBE_RESULT, EVENT_RESULT_FAIL, null);
            return;
        } else if (!updateRegisteredMsisdns(rsp)) {
            Log.e(TAG, "doRegisteredDevices: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_REGISTERED_MSISDN_RESULT, EVENT_RESULT_FAIL, null);
            return;
        }

        updateRegisteredMsisdn(rsp);
        Bundle bundle = new Bundle();
        bundle.putSerializable(EVENT_KEY_LINE_MAP, mRegisteredMSISDN);
        mListener.onSesEvent(EVENT_REGISTERED_MSISDN_RESULT, EVENT_RESULT_SUCCEED, bundle);
    }

    private void doSetDeviceName(String name) {
        Log.d(TAG, "doSetDeviceName name:" + name);
        Bundle bundle = new Bundle();
        bundle.putString(EVENT_KEY_NAME, name);

        mApi.setDeviceName(name);
        SesServerApi.Response rsp = mApi.createMethod()
                                    .addGetMSISDN()
                                    .sendAndWait();
        if (!handleSesResp(rsp, SesServerApi.METHOD_GET_MSISDN)) {
            Log.e(TAG, "doSetDeviceName: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_SET_DEVICE_NAME_RESULT, EVENT_RESULT_FAIL, bundle);
            return;
        }
        updateMsisdn(rsp);
        mListener.onSesEvent(EVENT_SET_DEVICE_NAME_RESULT, EVENT_RESULT_SUCCEED, bundle);
    }

    private void doSetLineName(String msisdn, String name) {
        Log.d(TAG, "doSetLineName msisdn:" + msisdn + ", name:" + name);
        Bundle bundle = new Bundle();
        bundle.putString(EVENT_KEY_MSISDN, msisdn);
        bundle.putString(EVENT_KEY_NAME, name);

        //Find ServiceInstanceId of the MSISDN
        DeviceInfo devInfo = mDeviceInfo.get(DigitsUtil.getInstance(mContext).getDeviceId());
        if (devInfo == null || name.length() > 25) {
            Log.e(TAG, "Line name is not vaild");
            mListener.onSesEvent(EVENT_SET_LINE_NAME_RESULT, EVENT_RESULT_FAIL, bundle);
            return;
        }

        String serviceInstId = devInfo.getServiceInstId(msisdn,
                               getServiceNameString(SERVICE_NAME_CONN_VOWIFI));

        if (serviceInstId == null) {
            mListener.onSesEvent(EVENT_SET_LINE_NAME_RESULT, EVENT_RESULT_FAIL, bundle);
            Log.e(TAG, msisdn +"'s serviceInstId is null");
            return;
        }

        SesServerApi.SesServiceInstance inst = mApi.new SesServiceInstance(
                getServiceNameString(SERVICE_NAME_CONN_VOWIFI),
                serviceInstId, msisdn, name);

        SesServerApi.Response rspInstanceSrv = mApi.createMethod()
                                               .addManageService(
                                                       SesServerApi.MANAGE_SERV_OPERATION_ASSIGN_NAME,
                                                       null,
                                                       inst)
                                               .sendAndWait();
        if (!handleSesResp(rspInstanceSrv, SesServerApi.METHOD_MANAGE_SERVICE,
                           SesServerApi.MANAGE_SERV_OPERATION_ASSIGN_NAME)) {
            Log.e(TAG, "doSetLineName: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_SET_LINE_NAME_RESULT, EVENT_RESULT_FAIL, bundle);
            return;
        }

        mListener.onSesEvent(EVENT_SET_LINE_NAME_RESULT, EVENT_RESULT_SUCCEED, bundle);
    }

    private String getServiceInstToken(String msisdn, String serviceInstanceId) {
        // Use the service-instance-id to get the correspond service-instance-token as SIT
        SesServerApi.SesServiceInstance inst = mApi.new SesServiceInstance(
                getServiceNameString(SERVICE_NAME_CONN_VOWIFI),
                serviceInstanceId);

        SesServerApi.Response rspInstanceSrv = mApi.createMethod()
                                               .addManageService(
                                                       SesServerApi.MANAGE_SERV_OPERATION_INSTANCE_TOKEN,
                                                       null,
                                                       inst)
                                               .addRegisteredDevices()
                                               .sendAndWait();
        if (!handleSesResp(rspInstanceSrv, SesServerApi.METHOD_MANAGE_SERVICE,
                           SesServerApi.MANAGE_SERV_OPERATION_INSTANCE_TOKEN) ||
                !handleSesResp(rspInstanceSrv, SesServerApi.METHOD_REGISTER_DEVICES)) {
            Log.e(TAG, "getServiceInstToken: receive failed callback, stop procedure now");
            return null;
        } else {
            updateDeviceInfo(rspInstanceSrv);
        }

        //Match service name, service-token and return service-instance-token as SIT
        String instToken = rspInstanceSrv.getString(SesServerApi.METHOD_MANAGE_SERVICE, "instance-token");
        String servInstToken;
        String expireTime;
        try {
            JSONObject jInstToken = new JSONObject(instToken);
            servInstToken = jInstToken.optString("service-instance-token", null);
            expireTime = jInstToken.optString("expiration-time", null);
            Log.d(TAG, "getServiceInstToken serInstToken=" + servInstToken
                  + ", expireTime=" + expireTime);
        } catch (JSONException e) {
            Log.e(TAG, "JSON object conversion error: ", e);
            return null;
        }
        scheduleRefreshMsisdnSit(msisdn, servInstToken, expireTime);
        return servInstToken;

    }

    private boolean doProvisionLine(String msisdn, boolean renewSit) {
        Log.d(TAG, "doProvisionLine msisdn:" + msisdn + ", renewSit:" + renewSit);
        int SesEvent = EVENT_PROVISION_LINE_RESULT;
        if (renewSit) {
            SesEvent = EVENT_SIT_RENEW;
        }

        Bundle bundle = new Bundle();
        bundle.putString(EVENT_KEY_MSISDN, msisdn);

        //Find The MSISDN in list
        RegisteredMSISDN regMsisdn = mRegisteredMSISDN.get(msisdn);
        if (regMsisdn == null) {
            mListener.onSesEvent(SesEvent, EVENT_RESULT_FAIL, bundle);
            Log.e(TAG, msisdn + " is not found");
            return false;
        }

        //Find ServiceFingerprint of the MSISDN
        SesServerApi.SesServiceItem servItem = mApi.new SesServiceItem(
                getServiceNameString(SERVICE_NAME_CONN_VOWIFI),
                regMsisdn.mServiceFingerprint);

        SesServerApi.Response rspProvSrv = mApi.createMethod()
                                           .addManageService(
                                               SesServerApi.MANAGE_SERV_OPERATION_PROVISION,
                                               servItem,
                                               null)
                                           .addManagePushToken(regMsisdn.mMsisdn,
                                                   getServiceNameString(SERVICE_NAME_CONN_VOWIFI),
                                                   SesServerApi.MANAGE_PUSH_TOKEN_OPERATION_REGISTER,
                                                   mAppToken,
                                                   getPushTokenEventString(PUSH_TOKEN_EVENT_CONN_MGR),
                                                   0)
                                           .sendAndWait();
        if (!handleSesResp(rspProvSrv, SesServerApi.METHOD_MANAGE_SERVICE,
                           SesServerApi.MANAGE_SERV_OPERATION_PROVISION) ||
                !handleSesResp(rspProvSrv, SesServerApi.METHOD_MANAGE_TOKEN)) {
            Log.e(TAG, "doGetDeviceConfig: receive failed callback, stop procedure now");
            mListener.onSesEvent(SesEvent, EVENT_RESULT_FAIL, bundle);
            return false;
        }

        //Find service-instance of the MSISDN from the response
        String servInst = rspProvSrv.getString(SesServerApi.METHOD_MANAGE_SERVICE, "service-instance");
        String serviceInstIdIn;
        try {
            JSONObject jServInst = new JSONObject(servInst);
            serviceInstIdIn = jServInst.optString("service-instance-id", null);
            Log.d(TAG, "serviceInstId=" + serviceInstIdIn);
        } catch (JSONException e) {
            Log.e(TAG, "JSON object conversion error: ", e);
            mListener.onSesEvent(SesEvent, EVENT_RESULT_FAIL, bundle);
            return false;
        }

        String servInstToken = getServiceInstToken(msisdn, serviceInstIdIn);
        if (TextUtils.isEmpty(servInstToken)) {
            mListener.onSesEvent(SesEvent, EVENT_RESULT_FAIL, bundle);
            Log.e(TAG, msisdn + " servInstToken is empty");
            return false;
        }
        bundle.putSerializable(EVENT_KEY_DEVICE_MAP, mDeviceInfo);
        bundle.putString(EVENT_KEY_SIT, servInstToken);
        mListener.onSesEvent(SesEvent, EVENT_RESULT_SUCCEED, bundle);

        return true;
    }

    private boolean doRemoveLine(String msisdn, boolean renewSit) {
        Log.d(TAG, "doRemoveLine msisdn:" + msisdn + ", renewSit:" + renewSit);
        int SesEvent = EVENT_REMOVE_LINE_RESULT;
        if (renewSit) {
            SesEvent = EVENT_SIT_RENEW;
        }
        Bundle bundle = new Bundle();
        bundle.putString(EVENT_KEY_MSISDN, msisdn);
        //Find ServiceInstanceId of the MSISDN
        DeviceInfo devInfo = mDeviceInfo.get(DigitsUtil.getInstance(mContext).getDeviceId());
        if (devInfo == null) {
            Log.e(TAG, "MSISDN not found");
            mListener.onSesEvent(SesEvent, EVENT_RESULT_FAIL, bundle);
            return false;
        }

        String serviceInstId = devInfo.getServiceInstId(msisdn,
                               getServiceNameString(SERVICE_NAME_CONN_VOWIFI));

        if (serviceInstId == null) {
            mListener.onSesEvent(SesEvent, EVENT_RESULT_FAIL, bundle);
            Log.e(TAG, msisdn + " serviceInstId is empty");
            return false;
        }

        SesServerApi.SesServiceInstance inst = mApi.new SesServiceInstance(
                getServiceNameString(SERVICE_NAME_CONN_VOWIFI),
                serviceInstId);

        SesServerApi.Response rspInstanceSrv = mApi.createMethod()
                                               .addManageService(
                                                       SesServerApi.MANAGE_SERV_OPERATION_REMOVE,
                                                       null,
                                                       inst)
                                               .sendAndWait();
        if (!handleSesResp(rspInstanceSrv, SesServerApi.METHOD_MANAGE_SERVICE,
                           SesServerApi.MANAGE_SERV_OPERATION_REMOVE)) {
            Log.e(TAG, "doGetDeviceConfig: receive failed callback, stop procedure now");
            mListener.onSesEvent(SesEvent, EVENT_RESULT_FAIL, bundle);
            return false;
        }
        if (!renewSit) {
            mListener.onSesEvent(SesEvent, EVENT_RESULT_SUCCEED, bundle);
        }
        return true;
    }



    private boolean doRenewMsisdnSit(String msisdn) {
        Log.d(TAG, "doRenewMsisdnSit msisdn:" + msisdn);
        Bundle bundle = new Bundle();
        bundle.putString(EVENT_KEY_MSISDN, msisdn);

        //Find ServiceInstanceId of the MSISDN
        DeviceInfo devInfo = mDeviceInfo.get(DigitsUtil.getInstance(mContext).getDeviceId());
        if (devInfo == null) {
            Log.d(TAG, "devInfo is null");
            return false;
        }

        String serviceInstIdIn = devInfo.getServiceInstId(msisdn,
                                 getServiceNameString(SERVICE_NAME_CONN_VOWIFI));

        if (serviceInstIdIn == null) {
            Log.e(TAG, msisdn + " serviceInstIdIn is empty");
            return false;
        }

        // Use the service-instance-id to get the correspond service-instance-token as SIT
        SesServerApi.SesServiceInstance inst = mApi.new SesServiceInstance(
                getServiceNameString(SERVICE_NAME_CONN_VOWIFI),
                serviceInstIdIn);

        SesServerApi.Response rspInstanceSrv = mApi.createMethod()
                                               .addManageService(
                                                       SesServerApi.MANAGE_SERV_OPERATION_INSTANCE_TOKEN,
                                                       null,
                                                       inst)
                                               .sendAndWait();
        int rspCode = 0;
        if (rspInstanceSrv != null) {
            rspCode = rspInstanceSrv.getResponseCode(SesServerApi.METHOD_MANAGE_SERVICE);
        }
        if (rspCode == SesServerApi.RES_INVALID_REQUEST ||
                rspCode== SesServerApi.RES_INVALID_SERVICE_INSTANCE_ID) {
            Log.e(TAG, "doRenewMsisdnSit: receive failed callback, delete token");
            if (doRemoveLine(msisdn, true)) {
                return doProvisionLine(msisdn, true);
            }
            return false;
        } else if (!handleSesResp(rspInstanceSrv, SesServerApi.METHOD_MANAGE_SERVICE,
                                  SesServerApi.MANAGE_SERV_OPERATION_INSTANCE_TOKEN)) {
            Log.e(TAG, "doRenewMsisdnSit: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_SIT_RENEW, EVENT_RESULT_FAIL, bundle);
            return false;
        }
        //Match service name, service-token and return service-instance-token as SIT
        String instToken = rspInstanceSrv.getString(SesServerApi.METHOD_MANAGE_SERVICE, "instance-token");
        String servInstToken;
        String expireTime;
        try {
            JSONObject jInstToken = new JSONObject(instToken);
            servInstToken = jInstToken.optString("service-instance-token", null);
            expireTime = jInstToken.optString("expiration-time", null);
            Log.d(TAG, "doRenewMsisdnSit serInstToken=" + servInstToken
                  + ", expireTime=" + expireTime);
        } catch (JSONException e) {
            Log.e(TAG, "JSON object conversion error: ", e);
            return false;
        }

        scheduleRefreshMsisdnSit(msisdn, servInstToken, expireTime);
        bundle.putString(EVENT_KEY_SIT, servInstToken);
        mListener.onSesEvent(EVENT_SIT_RENEW, EVENT_RESULT_SUCCEED, bundle);
        return true;
    }

    private void doUpdateMsisdnSit(String msisdn) {
        Log.d(TAG, "doUpdateMsisdnSit msisdn:" + msisdn);
        Bundle bundle = new Bundle();
        bundle.putString(EVENT_KEY_MSISDN, msisdn);

        //Find ServiceInstanceId of the MSISDN
        DeviceInfo devInfo = mDeviceInfo.get(DigitsUtil.getInstance(mContext).getDeviceId());
        if (devInfo == null) {
            Log.d(TAG, "devInfo is null");
            mListener.onSesEvent(EVENT_UPDATE_MSISDN_SIT_RESULT, EVENT_RESULT_FAIL, bundle);
            return;
        }

        String serviceInstIdIn = devInfo.getServiceInstId(msisdn,
                                 getServiceNameString(SERVICE_NAME_CONN_VOWIFI));

        if (serviceInstIdIn == null) {
            mListener.onSesEvent(EVENT_UPDATE_MSISDN_SIT_RESULT, EVENT_RESULT_FAIL, bundle);
            Log.e(TAG, msisdn + " serviceInstIdIn is empty");
            return;
        }

        // Use the service-instance-id to get the correspond service-instance-token as SIT
        SesServerApi.SesServiceInstance inst = mApi.new SesServiceInstance(
                getServiceNameString(SERVICE_NAME_CONN_VOWIFI),
                serviceInstIdIn);

        SesServerApi.Response rspInstanceSrv = mApi.createMethod()
                                               .addManageService(
                                                       SesServerApi.MANAGE_SERV_OPERATION_INSTANCE_TOKEN,
                                                       null,
                                                       inst)
                                               .addRegisteredDevices()
                                               .sendAndWait();
        if (!handleSesResp(rspInstanceSrv, SesServerApi.METHOD_MANAGE_SERVICE,
                           SesServerApi.MANAGE_SERV_OPERATION_INSTANCE_TOKEN) ||
                !handleSesResp(rspInstanceSrv, SesServerApi.METHOD_REGISTER_DEVICES)) {
            Log.e(TAG, "doUpdateMsisdnSit: receive failed callback, stop procedure now");
            mListener.onSesEvent(EVENT_UPDATE_MSISDN_SIT_RESULT, EVENT_RESULT_FAIL, bundle);
            return;
        } else {
            updateDeviceInfo(rspInstanceSrv);
        }
        bundle.putSerializable(EVENT_KEY_DEVICE_MAP, mDeviceInfo);

        //Match service name, service-token and return service-instance-token as SIT
        String instToken = rspInstanceSrv.getString(SesServerApi.METHOD_MANAGE_SERVICE, "instance-token");
        String servInstToken;
        String expireTime;
        try {
            JSONObject jInstToken = new JSONObject(instToken);
            servInstToken = jInstToken.optString("service-instance-token", null);
            expireTime = jInstToken.optString("expiration-time", null);
            Log.d(TAG, "doUpdateMsisdnSit serInstToken=" + servInstToken
                  + ", expireTime=" + expireTime);
        } catch (JSONException e) {
            Log.e(TAG, "JSON object conversion error: ", e);
            mListener.onSesEvent(EVENT_UPDATE_MSISDN_SIT_RESULT, EVENT_RESULT_FAIL, bundle);
            return;
        }

        scheduleRefreshMsisdnSit(msisdn, servInstToken, expireTime);

        bundle.putString(EVENT_KEY_SIT, servInstToken);
        mListener.onSesEvent(EVENT_UPDATE_MSISDN_SIT_RESULT, EVENT_RESULT_SUCCEED, bundle);
    }

    private boolean handleSesResp(SesServerApi.Response rsp, int method) {
        return handleSesResp(rsp, method, 0, null);
    }

    private boolean handleSesResp(SesServerApi.Response rsp, int method, int optCode) {
        return handleSesResp(rsp, method, optCode, null);
    }

    private boolean handleSesResp(SesServerApi.Response rsp,
                                  int method, int optCode, String lineInfo) {
        Log.d(TAG, "handleSesResp  method=" + method + ", optCode=" + optCode +
              ", lineInfo=" + lineInfo);
        if (rsp == null) {
            Log.e(TAG, "handleSesResp: null response");
            if (mApi.getHttpErrCode() == 401) {
                Log.e(TAG, "handleSesResp: http authentication failed");
                //mListener.onSesEvent(EVENT_SES_HTTP_AUTH_FAIL, EVENT_RESULT_FAIL, null);
            }
            return false;
        } else if (!rsp.isSuccessful(SesServerApi.METHOD_3GPPAUTH)) {
            Log.e(TAG, "handleSesResp: METHOD_3GPPAUTH faile");
            handleSesError(SesServerApi.METHOD_3GPPAUTH, rsp.getResponseCode(SesServerApi.METHOD_3GPPAUTH), 0, null);
            return false;
        }

        if (!rsp.isSuccessful(method)) {
            handleSesError(method, rsp.getResponseCode(method), optCode, lineInfo);
            return false;
        }

        return true;
    }

    private void handleSesError(int method, int errorCode, int optCode, String lineInfo) {
        Log.d(TAG, "handleSesError  method=" + method + ", errorCode=" + errorCode +
              ", optCode=" + optCode + ", lineInfo=" + lineInfo + ", mAlertDialog=" + mAlertDialog);

        if (mAlertDialog != null) {
            if (mAlertDialog.isShowing()) {
                Log.e(TAG, "Dismissing existd AlertDialog!");
                mAlertDialog.dismiss();
            }
            mAlertDialog = null;
        }

        if (method == SesServerApi.METHOD_3GPPAUTH) {
            switch(errorCode) {
                case SesServerApi.RES_INVALID_REQUEST:
                case SesServerApi.RES_SERVER_ERROR:
                    mAlertDialog = SesDialogs.createOkDialog(mContext,
                                   R.string.title_authentication_unsuccessful,
                                   R.string.retry_later,
                                   R.string.ok);
                    break;
                case SesServerApi.RES_AKA_AUTH_FAILED:
                case SesServerApi.RES_FORBIDDEN_REQUEST:
                    mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                   R.string.title_authentication_unsuccessful,
                                   R.string.retry_or_dial611,
                                   R.string.call_611,
                                   R.string.ok);
                    break;
            }
        } else if (method == SesServerApi.METHOD_REGISTER_MSISDN) {
            switch(errorCode) {
                case SesServerApi.RES_INVALID_REQUEST:
                case SesServerApi.RES_INVALID_DEVICE_STATUS:
                case SesServerApi.RES_ERROR_IN_MSISDN_CREATION:
                case SesServerApi.RES_SERVER_ERROR:
                    mAlertDialog = SesDialogs.createOkLogoutDialog(mContext,
                                   R.string.title_line_retrieval_unsuccessful,
                                   R.string.jansky_temp_unavailable,
                                   R.string.log_out,
                                   R.string.not_now,
                                   (Handler)this);
                    break;
                case SesServerApi.RES_AKA_AUTH_FAILED:
                    mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                   R.string.title_authentication_unsuccessful,
                                   R.string.retry_or_dial611,
                                   R.string.call_611,
                                   R.string.ok);
                    break;
            }
        } else if (method == SesServerApi.METHOD_GET_MSISDN) {
            switch(errorCode) {
                case SesServerApi.RES_INVALID_REQUEST:
                case SesServerApi.RES_NO_MSISDN:
                case SesServerApi.RES_SERVER_ERROR:
                    mAlertDialog = SesDialogs.createOkDialog(mContext,
                                   R.string.title_service_not_ready,
                                   R.string.retry_later,
                                   R.string.ok);
                    break;
                case SesServerApi.RES_AKA_AUTH_FAILED:
                    mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                   R.string.title_authentication_unsuccessful,
                                   R.string.retry_or_dial611,
                                   R.string.call_611,
                                   R.string.ok);
                    break;
            }
        } else if (method == SesServerApi.METHOD_REGISTER_DEVICES) {
            switch(errorCode) {
                case SesServerApi.RES_INVALID_REQUEST:
                case SesServerApi.RES_INVALID_DEVICE_STATUS:
                case SesServerApi.RES_SERVER_ERROR:
                    SesDialogs.createToast(mContext,
                                           R.string.unable_to_retrieve_remote_devices,
                                           3000);
                    break;
                case SesServerApi.RES_AKA_AUTH_FAILED:
                    mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                   R.string.title_authentication_unsuccessful,
                                   R.string.retry_or_dial611,
                                   R.string.call_611,
                                   R.string.ok);
                    break;
            }
        } else if (method == SesServerApi.METHOD_LOCATION_TC) {
            switch(errorCode) {
                case SesServerApi.RES_INVALID_REQUEST:
                case SesServerApi.RES_INVALID_DEVICE_STATUS:
                case SesServerApi.RES_INVALID_SERVICE_FINGERPRINT:
                case SesServerApi.RES_SERVER_ERROR:
                    mAlertDialog = SesDialogs.createOkDialog(mContext,
                                   R.string.title_location_unavailable,
                                   R.string.location_temp_unavailable,
                                   R.string.ok);
                    break;
                case SesServerApi.RES_AKA_AUTH_FAILED:
                    mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                   R.string.title_authentication_unsuccessful,
                                   R.string.retry_or_dial611,
                                   R.string.call_611,
                                   R.string.ok);
                    break;
            }
        } else if (method == SesServerApi.METHOD_MANAGE_TOKEN) {
            switch(errorCode) {
                case SesServerApi.RES_INVALID_REQUEST:
                case SesServerApi.RES_INVALID_DEVICE_STATUS:
                case SesServerApi.RES_INVALID_SERVICE_NAME:
                case SesServerApi.RES_SERVER_ERROR:
                    mAlertDialog = SesDialogs.createOkDialog(mContext,
                                   R.string.title_device_activation_unsuccessful,
                                   R.string.device_temp_unavailable,
                                   R.string.ok);
                    break;
                case SesServerApi.RES_AKA_AUTH_FAILED:
                    mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                   R.string.title_authentication_unsuccessful,
                                   R.string.retry_or_dial611,
                                   R.string.call_611,
                                   R.string.ok);
                    break;
            }
        } else if (method == SesServerApi.METHOD_MANAGE_CONN) {
            switch(optCode) {
                case SesServerApi.MANAGE_CONN_OPERATION_ACTIVATE:
                    switch(errorCode) {
                        case SesServerApi.RES_INVALID_REQUEST:
                        case SesServerApi.RES_CERT_FAILURE:
                        case SesServerApi.RES_INVALID_CSR:
                            mAlertDialog = SesDialogs.createLineInfoOkDialog(mContext,
                                           R.string.title_line_activation_unsuccessful,
                                           R.string.retry_activate_line,
                                           R.string.ok,
                                           lineInfo);
                            break;
                        case SesServerApi.RES_AKA_AUTH_FAILED:
                            mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                           R.string.title_authentication_unsuccessful,
                                           R.string.retry_or_dial611,
                                           R.string.call_611,
                                           R.string.ok);
                            break;
                        case SesServerApi.RES_INVALID_DEVICE_GROUP:
                            mAlertDialog = SesDialogs.createOkDialog(mContext,
                                           R.string.title_unrecognized_device,
                                           R.string.retry_later,
                                           R.string.ok);
                            break;
                        case SesServerApi.RES_SERVER_ERROR:
                            mAlertDialog = SesDialogs.createOkDialog(mContext,
                                           R.string.title_line_activation_unsuccessful,
                                           R.string.retry_later,
                                           R.string.ok);
                            break;
                    }
                    break;
                case SesServerApi.MANAGE_CONN_OPERATION_RENEW:
                    break;
                case SesServerApi.MANAGE_CONN_OPERATION_DELETE:
                    break;
                case SesServerApi.MANAGE_CONN_OPERATION_CONFIGURE:
                    switch(errorCode) {
                        case SesServerApi.RES_INVALID_REQUEST:
                        case SesServerApi.RES_INVALID_DEVICE_GROUP:
                        case SesServerApi.RES_SERVER_ERROR:
                            mAlertDialog = SesDialogs.createOkDialog(mContext,
                                           R.string.title_device_configuration_unsuccessful,
                                           R.string.retry_later,
                                           R.string.ok);
                            break;
                        case SesServerApi.RES_AKA_AUTH_FAILED:
                            mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                           R.string.title_authentication_unsuccessful,
                                           R.string.retry_or_dial611,
                                           R.string.call_611,
                                           R.string.ok);
                            break;
                    }
                    break;
                case SesServerApi.MANAGE_CONN_OPERATION_ADD_OAUTH:
                    switch(errorCode) {
                        case SesServerApi.RES_INVALID_REQUEST:
                        case SesServerApi.RES_INVALID_DEVICE_STATUS:
                        case SesServerApi.RES_SERVER_ERROR:
                            mAlertDialog = SesDialogs.createOkLogoutDialog(mContext,
                                           R.string.title_line_retrieval_unsuccessful,
                                           R.string.jansky_temp_unavailable,
                                           R.string.log_out,
                                           R.string.not_now,
                                           this);
                            break;
                        case SesServerApi.RES_AKA_AUTH_FAILED:
                            mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                           R.string.title_authentication_unsuccessful,
                                           R.string.retry_or_dial611,
                                           R.string.call_611,
                                           R.string.ok);
                            break;
                    }
                    break;

            }
        } else if (method == SesServerApi.METHOD_MANAGE_SERVICE) {
            switch(optCode) {
                case SesServerApi.MANAGE_SERV_OPERATION_PROVISION:
                case SesServerApi.MANAGE_SERV_OPERATION_INSTANCE_TOKEN:
                    switch(errorCode) {
                        case SesServerApi.RES_INVALID_REQUEST:
                        case SesServerApi.RES_INVALID_DEVICE_STATUS:
                        case SesServerApi.RES_INVALID_SERVICE_FINGERPRINT:
                            mAlertDialog = SesDialogs.createLineInfoOkDialog(mContext,
                                           R.string.title_line_activation_unsuccessful,
                                           R.string.retry_activate_line,
                                           R.string.ok,
                                           lineInfo);
                            break;
                        case SesServerApi.RES_AKA_AUTH_FAILED:
                            mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                           R.string.title_authentication_unsuccessful,
                                           R.string.retry_or_dial611,
                                           R.string.call_611,
                                           R.string.ok);
                            break;
                        case SesServerApi.RES_INVALID_OWNER_ID:
                        case SesServerApi.RES_SERVICE_NOT_ENTITLED:
                        case SesServerApi.RES_INVALID_SERVICE_INSTANCE_ID: // Call care directlly for instance-token(5)?
                            mAlertDialog = SesDialogs.createLineInfoOkDial611Dialog(mContext,
                                           R.string.title_line_activation_unsuccessful,
                                           R.string.retry_activate_line_or_dial611,
                                           R.string.call_611,
                                           R.string.not_now,
                                           lineInfo);
                            break;
                        case SesServerApi.RES_MAX_SERVICES_REACHED:
                            mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                           R.string.title_maximum_number_of_lines,
                                           R.string.maximum_number_of_lines_dial611,
                                           R.string.call_611,
                                           R.string.ok);
                            break;
                    }

                    break;
                case SesServerApi.MANAGE_SERV_OPERATION_RENEW:
                    switch(errorCode) {
                        case SesServerApi.RES_INVALID_REQUEST:
                        case SesServerApi.RES_INVALID_OWNER_ID:
                        case SesServerApi.RES_INVALID_DEVICE_STATUS:
                        case SesServerApi.RES_SERVICE_NOT_ENTITLED:
                        case SesServerApi.RES_INVALID_SERVICE_INSTANCE_ID:
                        case SesServerApi.RES_SERVER_ERROR:
                            // TODO:
                            //No popup. Just display progress circle in the indicator
                            break;
                    }
                    break;
                case SesServerApi.MANAGE_SERV_OPERATION_REMOVE:
                    switch(errorCode) {
                        case SesServerApi.RES_INVALID_REQUEST:
                        case SesServerApi.RES_INVALID_OWNER_ID:
                        case SesServerApi.RES_INVALID_DEVICE_STATUS:
                        case SesServerApi.RES_SERVER_ERROR:
                            mAlertDialog = SesDialogs.createLineInfoOkDialog(mContext,
                                           R.string.title_deactivation_unsuccessful,
                                           R.string.retry_deactivate_line,
                                           R.string.ok,
                                           lineInfo);
                            break;
                        case SesServerApi.RES_AKA_AUTH_FAILED:
                            mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                           R.string.title_authentication_unsuccessful,
                                           R.string.retry_or_dial611,
                                           R.string.call_611,
                                           R.string.ok);
                            break;
                        case SesServerApi.RES_INVALID_SERVICE_INSTANCE_ID:
                            mAlertDialog = SesDialogs.createLineInfoOkDial611Dialog(mContext,
                                           R.string.title_deactivation_unsuccessful,
                                           R.string.deactivate_line_fail_dial611,
                                           R.string.call_611,
                                           R.string.ok,
                                           lineInfo);
                            break;
                    }
                    break;
                case SesServerApi.MANAGE_SERV_OPERATION_SUSPEND:
                    break;
                case SesServerApi.MANAGE_SERV_OPERATION_COPY_SEND:
                    break;
                case SesServerApi.MANAGE_SERV_OPERATION_AUTHORIZATION:
                    break;
                case SesServerApi.MANAGE_SERV_OPERATION_ASSIGN_NAME:
                    switch(errorCode) {
                        case SesServerApi.RES_INVALID_REQUEST:
                        case SesServerApi.RES_INVALID_OWNER_ID:
                        case SesServerApi.RES_INVALID_DEVICE_STATUS:
                        case SesServerApi.RES_INVALID_SERVICE_INSTANCE_ID:
                        case SesServerApi.RES_SERVER_ERROR:
                            mAlertDialog = SesDialogs.createOkDialog(mContext,
                                           R.string.title_line_name_sync_with_server_unsuccessful,
                                           R.string.line_name_sync_issue,
                                           R.string.ok);
                            break;
                        case SesServerApi.RES_AKA_AUTH_FAILED:
                            mAlertDialog = SesDialogs.createOkDial611Dialog(mContext,
                                           R.string.title_authentication_unsuccessful,
                                           R.string.retry_or_dial611,
                                           R.string.call_611,
                                           R.string.ok);
                            break;

                    }
                    break;
            }
        }
        if (mAlertDialog != null) {
            mAlertDialog.show();
        }
    }

    public static String eventToString(int event) {
        switch(event) {
            case EVENT_GET_DEVICE_CONFIG_RESULT:
                return "EVENT_GET_DEVICE_CONFIG_RESULT";
            case EVENT_ON_BOARDING_RESULT:
                return "EVENT_ON_BOARDING_RESULT";
            case EVENT_INIT_SUBSCRIBE_RESULT:
                return "EVENT_INIT_SUBSCRIBE_RESULT";
            case EVENT_REFRESH_PUSH_TOKEN_RESULT:
                return "EVENT_REFRESH_PUSH_TOKEN_RESULT";
            case EVENT_REGISTERED_DEVICES_RESULT:
                return "EVENT_REGISTERED_DEVICES_RESULT";
            case EVENT_REGISTERED_MSISDN_RESULT:
                return "EVENT_REGISTERED_MSISDN_RESULT";
            case EVENT_PREPARE_LOGOUT_RESULT:
                return "EVENT_PREPARE_LOGOUT_RESULT";
            case EVENT_SET_DEVICE_NAME_RESULT:
                return "EVENT_SET_DEVICE_NAME_RESULT";
            case EVENT_SET_LINE_NAME_RESULT:
                return "EVENT_SET_LINE_NAME_RESULT";
            case EVENT_PROVISION_LINE_RESULT:
                return "EVENT_PROVISION_LINE_RESULT";
            case EVENT_REMOVE_LINE_RESULT:
                return "EVENT_REMOVE_LINE_RESULT";
            case EVENT_UPDATE_MSISDN_SIT_RESULT:
                return "EVENT_UPDATE_MSISDN_SIT_RESULT";
            case EVENT_SIT_UPDATED:
                return "EVENT_SIT_UPDATED";
            case EVENT_DELETE_DEVICE_RESULT:
                return "EVENT_DELETE_DEVICE_RESULT";
            case EVENT_ON_ADDRESS_REGISTRATION:
                return "EVENT_ON_ADDRESS_REGISTRATION";
            case EVENT_USER_LOGOUT:
                return "EVENT_USER_LOGOUT";
            case EVENT_GET_MSISDN_RESULT:
                return "EVENT_GET_MSISDN_RESULT";
            default:
                return "UNKNOWN_EVENT(" + event + ")";
        }

    }

    private String getDeviceGroupId() {
        // By MTK internal, using DEVICE_GROUP_ID.
        return SystemProperties.get("persist.digits.ses.device_groupId", DEVICE_GROUP_ID);
    }

    public void testSesError(int type) {
        Log.d(TAG, "testSesError  type=" + type);
        switch(type) {
            case 0:
                SesDialogs.createToast(mContext,
                                       R.string.unable_to_retrieve_remote_devices,
                                       3000);
                break;
            case 1:
                SesDialogs.createOkDialog(mContext,
                                          R.string.title_line_retrieval_unsuccessful,
                                          R.string.jansky_temp_unavailable,
                                          R.string.ok).show();
                break;
            case 2:
                SesDialogs.createOkDial611Dialog(mContext,
                                                 R.string.title_authentication_unsuccessful,
                                                 R.string.retry_or_dial611,
                                                 R.string.call_611,
                                                 R.string.ok).show();
                break;
            case 3:
                SesDialogs.createLineInfoOkDialog(mContext,
                                                  R.string.title_line_activation_unsuccessful,
                                                  R.string.retry_activate_line_or_dial611,
                                                  R.string.ok,
                                                  "14258372191").show();
                break;
            case 4:
                SesDialogs.createLineInfoOkDial611Dialog(mContext,
                        R.string.title_deactivation_unsuccessful,
                        R.string.deactivate_line_fail_dial611,
                        R.string.call_611,
                        R.string.ok,
                        "14258372191").show();
                break;
            case 5:
                SesDialogs.createOkLogoutDialog(mContext,
                                                R.string.title_line_retrieval_unsuccessful,
                                                R.string.jansky_temp_unavailable,
                                                R.string.log_out,
                                                R.string.not_now,
                                                this).show();
                break;
        }
    }
}
