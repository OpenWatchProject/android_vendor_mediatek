package com.mediatek.digits.ses;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.text.TextUtils;

import com.android.okhttp.MediaType;
import com.android.okhttp.OkHttpClient;
import com.android.okhttp.Request;
import com.android.okhttp.RequestBody;
import com.android.okhttp.Response;

import com.mediatek.digits.DigitsUtil;
import com.mediatek.digits.utils.Utils;
import com.mediatek.digits.utils.EapAka;
import com.mediatek.digits.utils.ErrorCodes;

import java.io.IOException;
import java.net.UnknownHostException;
import java.net.Socket;
import java.net.URI;

import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.UnrecoverableKeyException;

import java.util.List;
import java.util.ArrayList;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class SesServerApi {
    private static final String TAG = "SesServerApi";
    private static final boolean DEBUG = true;

    static final int SERVER_ERROR_RETRY = 4;

    private final Context mContext;
    private final String mApiEndpoint;
    private String mDeviceName;
    private String mAccessToken;

    private String mCurAkaToken;

    private HandlerThread mThread;
    private Handler mThreadHandler;
    private int mHttpErrCode;

    static final int METHOD_AUTHORIZE_DEVICE = 1;
    static final int METHOD_MANAGE_CONN = 2;
    static final int METHOD_MANAGE_SERVICE = 3;
    static final int METHOD_REGISTER_MSISDN = 4;
    static final int METHOD_REGISTER_DEVICES = 5;
    static final int METHOD_LOCATION_TC = 6;
    static final int METHOD_MANAGE_TOKEN = 7;
    static final int METHOD_GET_MSISDN = 8;
    static final int METHOD_3GPPAUTH = 9;
    static final int METHOD_CHECK_ENTITLEMENT = 10;
    static final int METHOD_GET_TOKEN = 11;
    static final int MAX_METHOD_NUM = 10;

    public static final MediaType JSON
        = MediaType.parse("application/json");

    // authorizeDevice Request device-type
    static final int DEVICE_TYPE_HANDHELD_WITH_SIM             = 0;
    static final int DEVICE_TYPE_HANDHELD_WITHOUT_SIM          = 1;
    static final int DEVICE_TYPE_TABLET_WITH_SIM               = 2;
    static final int DEVICE_TYPE_TABLET_WITHOUT_SIM            = 3;
    static final int DEVICE_TYPE_WEARABLE_WITH_SIM             = 4;
    static final int DEVICE_TYPE_WEARABLE_WITHOUT_SIM          = 5;
    static final int DEVICE_TYPE_SET_OF_BOX_WITH_SIM           = 6;
    static final int DEVICE_TYPE_SET_OF_BOX_WITHOUT_SIM        = 7;
    static final int DEVICE_TYPE_LAPTOP_WITH_SIM               = 8;
    static final int DEVICE_TYPE_LAPTOP_WITHOUT_SIM            = 9;
    static final int DEVICE_TYPE_DESKTOP_WITH_SIM              = 10;
    static final int DEVICE_TYPE_DESKTOP_WITHOUT_SIM           = 11;
    static final int DEVICE_TYPE_APPLIANCE_WITH_SIM            = 12;
    static final int DEVICE_TYPE_APPLIANCE_WITHOUT_SIM         = 13;
    static final int DEVICE_TYPE_AUTOMOTIVE_WITH_SIM           = 14;
    static final int DEVICE_TYPE_AUTOMOTIVE_WITHOUT_SIM        = 15;
    //[16-29] RESERVED FOR FUTURE ALLOCATIONS
    static final int DEVICE_TYPE_OTHER_UNSPECIFIED_WITH_SIM    = 30;
    static final int DEVICE_TYPE_OTHER_UNSPECIFIED_WITHOUT_SIM = 31;

    //manageConnectivity Request operation
    static final int MANAGE_CONN_OPERATION_ACTIVATE  = 0;
    static final int MANAGE_CONN_OPERATION_RENEW     = 1;
    static final int MANAGE_CONN_OPERATION_DELETE    = 2;
    static final int MANAGE_CONN_OPERATION_CONFIGURE = 3;
    static final int MANAGE_CONN_OPERATION_ADD_OAUTH = 4;
    //[5 ¡V 10] = Reserved for future allocation

    // manageService Request operation
    static final int MANAGE_SERV_OPERATION_PROVISION      = 0;
    static final int MANAGE_SERV_OPERATION_RENEW          = 1;
    static final int MANAGE_SERV_OPERATION_REMOVE         = 2;
    static final int MANAGE_SERV_OPERATION_SUSPEND        = 3;
    static final int MANAGE_SERV_OPERATION_COPY_SEND      = 4;
    static final int MANAGE_SERV_OPERATION_INSTANCE_TOKEN = 5;
    static final int MANAGE_SERV_OPERATION_AUTHORIZATION  = 6;
    static final int MANAGE_SERV_OPERATION_ASSIGN_NAME    = 7;
    //[8 ¡V 15] = Reserved for future allocation

    // registeredMSISDN Request operation
    static final int REG_MSISDN_OPERATION_AVAIABLE = 0;
    static final int REG_MSISDN_OPERATION_CREATE   = 1;

    // managePushToken Request operation
    static final int MANAGE_PUSH_TOKEN_OPERATION_REGISTER = 0;
    static final int MANAGE_PUSH_TOKEN_OPERATION_REMOVE  = 1;

    // result code
    static final int RES_REQUEST_SUCCESSFUL = 1000;
    static final int RES_AKA_CHALLENGE  = 1003;
    static final int RES_INVALID_REQUEST = 1004;
    static final int RES_AKA_AUTH_FAILED = 1006;
    static final int RES_FORBIDDEN_REQUEST = 1007;
    static final int RES_CERT_FAILURE = 1022;
    static final int RES_INVALID_OWNER_ID = 1024;
    static final int RES_INVALID_CSR = 1025;
    static final int RES_INVALID_DEVICE_STATUS = 1029;
    static final int RES_MAX_SERVICES_REACHED = 1040;
    static final int RES_INVALID_SERVICE_FINGERPRINT = 1041;
    static final int RES_INVALID_TARGET_DEVICE = 1042;
    static final int RES_MAX_SERVICE_INSTANCE_REACHED = 1044;
    static final int RES_INVALID_SERVICE_NAME = 1046;
    static final int RES_SERVICE_NOT_ENTITLED = 1048;
    static final int RES_INVALID_SERVICE_INSTANCE_ID = 1053;
    static final int RES_INVALID_DEVICE_GROUP = 1054;
    static final int RES_NO_MSISDN = 1060;
    static final int RES_ERROR_IN_MSISDN_CREATION = 1061;
    static final int RES_SERVER_ERROR = 1111;

    public SesServerApi(Context context, String apiEndpoint, String deviceName) {
        mContext = context;
        mApiEndpoint = apiEndpoint;
        mDeviceName = deviceName;

        log("SesServerApi: mDeviceName=" + mDeviceName);

        mThread = new HandlerThread("SesServerApi-worker");
        mThread.start();
        mThreadHandler = new Handler(mThread.getLooper());
    }

    protected void finalize() throws Throwable {
        try {
            mThread.quitSafely();
        } finally {
            super.finalize();
        }
    }

    public SesServerMethodBuilder createMethod() {
        return new SesServerMethodBuilder(this);
    }

    public void resetAllState() {
        log("resetAllState!!");
        mCurAkaToken = null;
    }

    public void setAccessToken(String token) {
        mAccessToken = token;
    }

    public void setDeviceName(String name) {
        mDeviceName = name;
    }

    private JSONObject find3GppAuthObject(JSONArray array) {
        int length = array.length();
        for (int i = 0; i < length; i++) {
            try {
                JSONObject json = array.getJSONObject(i);
                if (json == null || json.getInt("message-id") != METHOD_3GPPAUTH) {
                    continue;
                }

                return json;
            } catch (JSONException e) {
                Log.e(TAG, "get json data exception: ", e);
            }
        }

        return null;
    }

    // Client doesn't need to call 3GppAuthentication, every HTTPS request needs to do this.
    private JSONObject init3GppAuthentication(JSONObject json) {
        try {
            json.put("message-id", METHOD_3GPPAUTH);
            json.put("method", "3gppAuthentication");
            json.put("device-id", getBase64DeviceId());
            json.put("device-type", 0);
            json.put("os-type", 0);
            json.put("device-name", mDeviceName);

            TelephonyManager telephonyManager =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
            if (telephonyManager != null) {
                String imsiEap = EapAka.getImsiEap(telephonyManager);
                if (imsiEap != null) {
                    json.put("imsi-eap", imsiEap);
                } else {
                    log("3GppAuthentication: get empty imsiEap, stop directly...");
                    return null;
                }
            }
            if (mCurAkaToken != null) {
                log("3GppAuthentication: have existed token, do fast 3GppAuthentication!");

                json.put("aka-token", mCurAkaToken);
            }
        } catch (JSONException e) {
            Log.e(TAG, "put json data exception: ", e);
        }

        return json;
    }

    private boolean isFull3GppAuthNeeded(JSONArray response) {
        if (response == null) {
            log("isFull3GppAuthNeeded: no input, return directly..");
            return false;
        }

        JSONObject json = find3GppAuthObject(response);

        if (json == null) {
            log("isFull3GppAuthNeeded: cannot find 3gppAuthentication method!");
            return false;
        }

        try {
            int responseCode = json.getInt("response-code");
            String akaChallenge = json.getString("aka-challenge");

            if (responseCode == ErrorCodes.AKA_CHALLENGE && akaChallenge != null) {
                log("isFull3GppAuthNeeded: need to do full-3gppAuthentication");
                return true;
            }
        } catch (JSONException e) {
            if (!e.getMessage().contains("No value for aka-challenge")) {
                // Print stack trace only for abnormal exception
                Log.e(TAG, "isFull3GppAuthNeeded: get json data exception: ", e);
            }
        }

        log("isFull3GppAuthNeeded: no need to do full-3gppAuthentication");
        return false;
    }

    private JSONArray doFull3GppAuthentication(OkHttpClient httpClient, JSONArray challenge, JSONArray originalMethods) {
        log("doFull3GppAuthentication: challenge=" + challenge);

        if (challenge == null) {
            Log.e(TAG, "no challenge?!");
            return null;
        }

        mCurAkaToken = null;

        JSONObject jsonChallenge = find3GppAuthObject(challenge);
        if (jsonChallenge == null) {
            Log.e(TAG, "failed to find AKA challenge object");
            return null;
        }

        JSONObject jsonResponse = find3GppAuthObject(originalMethods);
        if (jsonResponse == null) {
            Log.e(TAG, "failed find original AKA response object");
            return null;
        }

        TelephonyManager telephonyManager =
            (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (telephonyManager == null) {
            Log.e(TAG, "No telephonyManager?!");
            return null;
        }

        // Now start to do aka-challenge!
        try {
            String akaChallenge = jsonChallenge.getString("aka-challenge");
            String akaResponse = EapAka.doEapAkaAuthentication(telephonyManager, akaChallenge);
            if (akaResponse == null) {
                Log.e(TAG, "failed to calculate response of AKA challenge");
                return null;
            }

            jsonResponse.remove("aka-token");
            jsonResponse.put("aka-challenge-rsp", akaResponse);
        } catch (JSONException e) {
            Log.e(TAG, "JSON exception", e);
            return null;
        }

        JSONArray rsp = sendHttpRequest(httpClient, mApiEndpoint, originalMethods);
        if (rsp == null)
            return null;

        mCurAkaToken = new Response(rsp).getAkaToken();
        if (mCurAkaToken == null) {
            Log.e(TAG, "failed to do EAP-AKA from remote");
            return null;
        }
        return rsp;
    }

    boolean execute(final JSONArray jsonArray, final Callback callback) {
        if (checkNoHandlerThread()) return false;

        mThreadHandler.post(new Runnable() {
            public void run() {
                OkHttpClient httpClient = new OkHttpClient();
                Response finalResult = null;

                // 1. initial a 3gppAuthentication.
                JSONObject json = init3GppAuthentication(new JSONObject());
                if (json != null) {
                    jsonArray.put(json);
                }

                JSONArray rsp = sendHttpRequest(httpClient, mApiEndpoint, jsonArray);

                do {
                    if (rsp == null) break;

                    if (isFull3GppAuthNeeded(rsp)) {
                        rsp = doFull3GppAuthentication(httpClient, rsp, jsonArray);
                    }

                    if (rsp == null) break;
                    finalResult = new Response(rsp);
                } while (false);

                if (callback != null) {
                    log("execute: callback the response");
                    callback.callback(finalResult);
                }
            }
        });

        log("execute: done");

        return true;
    }

    Response sendAndWait (final JSONArray jsonArray) {
        OkHttpClient httpClient = new OkHttpClient();
        Response finalResult = null;
        JSONArray rsp = null;

        // 1. initial a 3gppAuthentication.
        JSONObject json = find3GppAuthObject(jsonArray);
        if (json == null) {
            json = init3GppAuthentication(new JSONObject());
            if (json != null) {
                jsonArray.put(json);
            }
        }

        // 3gppAuthentication error retry
        for (int i = 0 ; i < SERVER_ERROR_RETRY ; i++) {
            rsp = sendHttpRequest(httpClient, mApiEndpoint, jsonArray);
            if (rsp == null) {
                log("sendAndWait: sendHttpRequest failed");
                return null;
            }
            finalResult = new Response(rsp);
            // 2 kinds of error response should retry
            if (finalResult.getResponseCode(METHOD_3GPPAUTH) != RES_SERVER_ERROR &&
                    finalResult.getResponseCode(METHOD_3GPPAUTH) != RES_FORBIDDEN_REQUEST) {
                break;
            }
            log("sendAndWait: 3gpp auth retry:" + i);
        }

        do {
            if (isFull3GppAuthNeeded(rsp)) {
                rsp = doFull3GppAuthentication(httpClient, rsp, jsonArray);
            }

            if (rsp == null) {
                log("sendAndWait: sendHttpRequest doFull3GppAuthentication failed");
                return null;
            }
            finalResult = new Response(rsp);
        } while (false);

        return finalResult;
    }

    public interface Callback {
        void callback(Response rsp);
    }

    public class Response {
        private JSONArray jsonArray;

        Response(JSONArray _jsonArray) {
            jsonArray = _jsonArray;
        }

        private JSONObject getJsonByMsgId(int msgId) {
            if (jsonArray == null) return null;

            for (int i = 0; i < jsonArray.length(); i++) {
                try {
                    JSONObject json = jsonArray.getJSONObject(i);
                    if (json != null && json.getInt("message-id") == msgId) {
                        return json;
                    }
                } catch (JSONException e) {
                    Log.e(TAG, "getJsonByMsgId: exception when get data!", e);
                    return null;
                }
            }
            return null;
        }

        public boolean isSuccessful(int msgId) {
            if (jsonArray == null) return false;

            // always check METHOD_3GPPAUTH first.
            if (getJsonByMsgId(msgId) != null && getResponseCode(METHOD_3GPPAUTH) != ErrorCodes.REQUEST_SUCCESSFUL) {
                return false;
            }

            int errCode = getResponseCode(msgId);
            if (errCode != ErrorCodes.REQUEST_SUCCESSFUL) {
                Log.e(TAG, "Response error code: " + errCode);
                return false;
            }
            return true;
        }

        public int getResponseCode(int msgId) {
            if (jsonArray == null) return ErrorCodes.INVALID_RESPONSE;

            //set error code
            int errCode = getInt(msgId, "response-code");
            ErrorCodes.setErrorCode(errCode);
            return errCode;
        }

        boolean checkValueExisted(int msgId, String key) {
            if (jsonArray != null) {
                JSONObject json = getJsonByMsgId(msgId);
                if (json != null) {
                    return !json.isNull(key);
                }
            }
            return false;
        }

        public int getInt(int msgId, String key) {
            int ret = -1;

            try {
                if (jsonArray != null) {
                    JSONObject json = getJsonByMsgId(msgId);
                    if (json != null) {
                        ret = json.getInt(key);
                    }
                }
            } catch (JSONException e) {
                Log.e(TAG, "JSON exception: ", e);
            }
            return ret;
        }

        public String getString(int msgId, String key) {
            String ret = "";

            try {
                if (jsonArray != null) {
                    JSONObject json = getJsonByMsgId(msgId);
                    if (json != null) {
                        ret = json.getString(key);
                    }
                }
            } catch (JSONException e) {
                Log.e(TAG, "getString() exception: ", e);
            }
            return ret;
        }

        public String[] getStrings(int msgId, String key) {
            String[] arr = null;

            try {
                if (jsonArray != null) {
                    JSONObject json = getJsonByMsgId(msgId);
                    if (json != null) {
                        JSONArray arrJson = json.getJSONArray(key);
                        arr = new String[arrJson.length()];
                        for (int i = 0; i < arrJson.length(); i++) {
                            arr[i] = arrJson.getString(i);
                        }
                    }
                }
            } catch (JSONException e) {
                Log.e(TAG, "getStrings() exception: ", e);
            }
            return arr;
        }

        public boolean getBoolean(int msgId, String key) {
            boolean ret = false;

            try {
                if (jsonArray != null) {
                    JSONObject json = getJsonByMsgId(msgId);
                    if (json != null) {
                        ret = json.getBoolean(key);
                    }
                }
            } catch (JSONException e) {
                Log.e(TAG, "getBoolean() exception: ", e);
            }
            return ret;
        }

        public boolean getBoolean(int msgId, String key, boolean defVal) {
            boolean ret = false;

            if (jsonArray != null) {
                JSONObject json = getJsonByMsgId(msgId);
                if (json != null) {
                    ret = json.optBoolean(key, defVal);
                }
            }
            return ret;
        }

        public String getAkaToken() {
            JSONObject _3gppMethod = find3GppAuthObject(jsonArray);
            if (_3gppMethod == null) {
                log("getAkaToken(): cannot find 3gppMethod");
                return null;
            }

            try {
                return _3gppMethod.getString("aka-token");
            } catch (JSONException e) {
                Log.e(TAG, "getAkaToken() exception: ", e);
            }
            return null;
        }
    }

    public class SesServerMethodBuilder {
        private JSONArray jsonArray = new JSONArray();
        private int mMsgCount = 0;
        private SesServerApi mService;
        private int mMgeConnOpt, mMgeServOpt;

        private SesServerMethodBuilder(SesServerApi service) {
            mService = service;
        }

        public boolean execute(Callback callback) {
            if (mService != null) {
                return mService.execute(jsonArray, callback);
            }
            return false;
        }

        public Response sendAndWait() {
            if (mService != null) {
                Response result = mService.sendAndWait(jsonArray);

                for (int i = 0 ; i < SERVER_ERROR_RETRY ; i++) {
                    if (!isServerErrRetryNeeded(result)) {
                        break;
                    }
                    log("sendAndWait: retry:" + i);
                    result = mService.sendAndWait(jsonArray);
                    if (result == null) {
                        log("sendAndWait: sendHttpRequest retry  failed");
                        return null;
                    }
                }

                return result;
            }
            return null;
        }

        private boolean isServerErrRetryNeeded(Response rsp) {
            if (rsp == null ||
                    rsp.getResponseCode(METHOD_REGISTER_MSISDN) == RES_SERVER_ERROR ||
                    rsp.getResponseCode(METHOD_GET_MSISDN) == RES_SERVER_ERROR ||
                    rsp.getResponseCode(METHOD_LOCATION_TC) == RES_SERVER_ERROR ||
                    rsp.getResponseCode(METHOD_MANAGE_TOKEN) == RES_SERVER_ERROR ||
                    rsp.getResponseCode(METHOD_REGISTER_DEVICES) == RES_SERVER_ERROR ||
                    rsp.getResponseCode(METHOD_CHECK_ENTITLEMENT) == RES_SERVER_ERROR) {
                return true;
            } else if (rsp.getResponseCode(METHOD_MANAGE_CONN) == RES_SERVER_ERROR) {
                log("manageConnectivity server error, opt code" + mMgeConnOpt);
                switch(mMgeConnOpt) {
                    case MANAGE_CONN_OPERATION_ACTIVATE:
                    case MANAGE_CONN_OPERATION_CONFIGURE:
                        return true;
                }
            } else if (rsp.getResponseCode(METHOD_MANAGE_SERVICE) == RES_SERVER_ERROR) {
                log("manageService server error, opt code" + mMgeServOpt);
                switch(mMgeServOpt) {
                    case MANAGE_SERV_OPERATION_PROVISION:
                        return true;
                }
            }
            return false;
        }

        public SesServerMethodBuilder addAuthorizeDevice(int deviceType, String devicName) {
            log("manageConnectivity:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_AUTHORIZE_DEVICE);
                    json.put("method", "authorizeDevice");
                    json.put("device-id", getBase64DeviceId());
                    json.put("device-type", deviceType);
                    json.put("os-type", 0);
                    json.put("device-name", devicName);
                    jsonArray.put(json);
                } catch (JSONException e) {
                    Log.e(TAG, "put json data exception: ", e);
                }
            } else {
                log("authorizeDevice: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public SesServerMethodBuilder addManageConnectivity(int operation,
                String deviceGroup,
                String vImsi) {
            log("manageConnectivity:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_MANAGE_CONN);
                    json.put("method", "manageConnectivity");
                    json.put("device-id", getBase64DeviceId());
                    json.put("operation", operation);
                    mMgeConnOpt = operation;
                    if (deviceGroup != null) {
                        json.put("device-group", deviceGroup);
                    }
                    if (vImsi != null) {
                        json.put("vimsi", vImsi);
                    }
                    jsonArray.put(json);
                } catch (JSONException e) {
                    Log.e(TAG, "put json data exception: ", e);
                }
            } else {
                log("manageConnectivity: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public SesServerMethodBuilder addManageService(int operation,
                SesServiceItem serviceItem,
                SesServiceInstance serviceIntance) {
            log("manageService:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_MANAGE_SERVICE);
                    json.put("method", "manageService");
                    json.put("device-id", getBase64DeviceId());
                    json.put("operation", operation);
                    mMgeServOpt = operation;
                    if (serviceItem != null) {
                        JSONObject jServiceItem = new JSONObject();
                        jServiceItem.put("service-name", serviceItem.getServiceName());
                        if (serviceItem.getServiceFingerprint() != null) {
                            jServiceItem.put("service-fingerprint",  serviceItem.getServiceFingerprint());
                        }
                        json.put("service-item", jServiceItem);
                    }
                    if (serviceIntance != null) {
                        JSONObject jServiceIns = new JSONObject();
                        jServiceIns.put("service-name", serviceIntance.getServiceName());
                        if (serviceIntance.getServiceInstanceId() != null) {
                            jServiceIns.put("service-instance-id",  serviceIntance.getServiceInstanceId());
                        }
                        if (serviceIntance.getFriendlyName() != null) {
                            jServiceIns.put("friendly-name",  serviceIntance.getFriendlyName());
                        }
                        if (serviceIntance.getMsisdn() != null) {
                            jServiceIns.put("msisdn",  serviceIntance.getMsisdn());
                        }
                        json.put("service-instance", jServiceIns);
                    }
                    jsonArray.put(json);
                } catch (JSONException e) {
                    Log.e(TAG, "put json data exception: ", e);
                }
            } else {
                log("manageService: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public SesServerMethodBuilder addRegisteredMSISDN(int operation) {
            log("manageService:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_REGISTER_MSISDN);
                    json.put("method", "registeredMSISDN");
                    json.put("device-id", getBase64DeviceId());
                    json.put("operation", operation);
                    jsonArray.put(json);
                } catch (JSONException e) {
                    Log.e(TAG, "put json data exception: ", e);
                }
            } else {
                log("addRegisteredMSISDN: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public SesServerMethodBuilder addRegisteredDevices() {
            log("manageService:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_REGISTER_DEVICES);
                    json.put("method", "registeredDevices");
                    json.put("device-id", getBase64DeviceId());
                    jsonArray.put(json);
                } catch (JSONException e) {
                    Log.e(TAG, "put json data exception: ", e);
                }
            } else {
                log("addRegisteredDevices: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public SesServerMethodBuilder addManageLocationAndTC(String svsFingerPrint) {
            log("manageLocationAndTC:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_LOCATION_TC);
                    json.put("method", "manageLocationAndTC");
                    json.put("device-id", getBase64DeviceId());
                    json.put("service-fingerprint", svsFingerPrint);
                    jsonArray.put(json);
                } catch (JSONException e) {
                    Log.e(TAG, "put json data exception: ", e);
                }
            } else {
                log("addManageLocationAndTC: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public SesServerMethodBuilder addManagePushToken(String msisdn,
                String serviceName,
                int operation,
                String pushToken,
                String event,
                int authType) {
            log("managePushToken:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_MANAGE_TOKEN);
                    json.put("method", "managePushToken");
                    json.put("device-id", getBase64DeviceId());
                    json.put("msisdn", msisdn);
                    json.put("service-name", serviceName);
                    json.put("operation", operation);
                    json.put("push-token", pushToken == null ? "" : Utils.base64Encode(pushToken));
                    json.put("event", event);
                    if (authType > 0) {
                        json.put("auth-type", authType);
                    }
                    jsonArray.put(json);
                } catch (JSONException e) {
                    Log.e(TAG, "put json data exception: ", e);
                }
            } else {
                log("addManagePushToken: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public SesServerMethodBuilder addGetMSISDN() {
            log("getMSISDN:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_GET_MSISDN);
                    json.put("method", "getMSISDN");
                    json.put("device-id", getBase64DeviceId());
                    jsonArray.put(json);
                } catch (JSONException e) {
                    Log.e(TAG, "put json data exception: ", e);
                }
            } else {
                log("getMSISDN: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }
    }

    public class SesServiceItem {
        private String mServiceName;
        private String mServiceFingerprint;

        public SesServiceItem(String serviceName, String serviceFingerprint) {
            mServiceName = serviceName;
            mServiceFingerprint = serviceFingerprint;
        }

        public String getServiceName() {
            return mServiceName;
        }

        public String getServiceFingerprint() {
            return mServiceFingerprint;
        }
    }

    public class SesServiceInstance {
        private String mServiceName;
        private String mServiceInstanceId;
        private String mFriendlyName;
        private String mMsisdn;

        public SesServiceInstance(String serviceName, String serviceInstanceId) {
            mServiceName = serviceName;
            mServiceInstanceId = serviceInstanceId;
        }

        public SesServiceInstance(String serviceName,
                                  String serviceInstanceId,
                                  String friendlyName) {
            mServiceName = serviceName;
            mServiceInstanceId = serviceInstanceId;
            mFriendlyName = friendlyName;
        }


        public SesServiceInstance(String serviceName,
                                  String serviceInstanceId,
                                  String msisdn,
                                  String friendlyName) {
            mServiceName = serviceName;
            mServiceInstanceId = serviceInstanceId;
            mFriendlyName = friendlyName;
            mMsisdn = msisdn;
        }

        public String getServiceName() {
            return mServiceName;
        }

        public String getServiceInstanceId() {
            return mServiceInstanceId;
        }

        public String getFriendlyName() {
            return mFriendlyName;
        }

        public String getMsisdn() {
            return mMsisdn;
        }
    }

    public boolean sendTestAPI() {
        log("sendTestAPI in!");

        if (checkNoHandlerThread()) return false;

        mThreadHandler.post(new Runnable() {
            public void run() {
                JSONObject json = new JSONObject();

                try {
                    json.put("title", "Mr.");
                    json.put("name", "gnehcic");
                } catch (JSONException e) {
                    Log.e(TAG, "put json data exception: ", e);
                }

                //JSONObject rsp = sendHttpRequest("http://gnehcic.azurewebsites.net/sample/SampleWebAPI.php", json);
                //log("sendTestAPI get return: " + rsp);
            }
        });
        return true;
    }

    /**     * This function shall not be run at main thread.     */
    private JSONArray sendHttpRequest(OkHttpClient httpClient, String url, JSONArray json) {
        //mAccessToken = SystemProperties.get("vendor.com.mtk.digits.testtoken", mAccessToken);
        log("sendHttpRequest: url=" + url);
        log("sendHttpRequest: mAccessToken=" + mAccessToken);
        log("sendHttpRequest: json=" + json);
        JSONArray result = null;
        RequestBody body = null;
        try {
            if (json != null) {
                byte[] gzipBytes = null;
                gzipBytes = Utils.gzipCompress(json.toString());

                if (gzipBytes == null) {
                    log("gzipCompress: get empty byte array");
                    return null;
                }

                body = RequestBody.create(JSON, gzipBytes);
            }

            Request request = new Request.Builder()
            .url(url)
            .header("Content-type", "application/json")
            .header("Accept-Encoding", "gzip")
            .header("Content-Encoding", "gzip")
            .header("Accept", "application/json")
            .header("x-generic-protocol-version", "1.0")
            .post(body)
            .build();

            if (!TextUtils.isEmpty(mAccessToken)) {
                Request authRequest = new Request.Builder()
                .url(url)
                .headers(request.headers())
                .header("authorization", mAccessToken)
                .post(body)
                .build();
                request = authRequest;
            }

            com.android.okhttp.Response response = httpClient.newCall(request).execute();
            mHttpErrCode = response.code();

            if (response.isSuccessful()) {
                byte[] data = response.body().bytes();
                if (data == null) {
                    log("resEntity: empty byte array");
                    return null;
                }

                String responseString = Utils.gzipDecompress(data);
                log("sendHttpRequest: response=" + responseString);
                result = new JSONArray(responseString);
            } else {
                Log.e(TAG, "Bad HTTP response:" + response);
            }
            response.body().close();
        } catch (Exception e) {
            Log.e(TAG, "sendHttpRequest: Exception:", e);
            return null;
        }

        return result;
    }

    public int getHttpErrCode() {
        return mHttpErrCode;
    }

    private String getBase64DeviceId() {
        return Utils.base64Encode(DigitsUtil.getInstance(mContext).getDeviceId());
    }

    private boolean checkNoHandlerThread() {
        if (mThreadHandler == null) {
            log("No Handler Thread, return directly");
            return true;
        } else {
            return false;
        }
    }

    private static void log(String s) {
        if (DEBUG) Log.d(TAG, s);
    }
}
