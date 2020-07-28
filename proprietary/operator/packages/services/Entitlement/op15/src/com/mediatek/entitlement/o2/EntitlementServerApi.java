package com.mediatek.entitlement.o2;

import android.content.Context;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import com.android.okhttp.MediaType;
import com.android.okhttp.OkHttpClient;
import com.android.okhttp.Request;
import com.android.okhttp.RequestBody;
import com.android.okhttp.Response;
import com.mediatek.entitlement.EapAka;
import com.mediatek.entitlement.Utils;

public class EntitlementServerApi {
    private static final String TAG = "EntitlementServerApi";
    private static final boolean DEBUG = true;
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean ENG = "eng".equals(Build.TYPE);
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private final Context mContext;
    private final String mApiEndpoint;
    private final String mDeviceId;
    private final String mDeviceName;
    private int mSlotId;

    private String mCurAkaToken;

    private HandlerThread mThread;
    private Handler mThreadHandler;

    static final int METHOD_3GPPAUTH = 1;
    static final int METHOD_MANAGE_CONN = 2;
    static final int METHOD_MANAGE_SERVICE = 3;
    static final int METHOD_LOCATION_TC = 4;
    static final int METHOD_MANAGE_TOKEN = 5;
    static final int METHOD_GET_MSISDN = 6;
    static final int METHOD_CHECK_ENTITLEMENT = 7;
    static final int MAX_METHOD_NUM = 7;

    public static final MediaType JSON
        = MediaType.parse("application/json");

    public EntitlementServerApi(Context context, String apiEndpoint, String deviceName, int slotId) {
        mContext = context;
        mApiEndpoint = apiEndpoint;
        mDeviceName = deviceName;

        TelephonyManager telephonyManager =
                (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        String deviceId = telephonyManager.getDeviceId();
        if (deviceId == null ) {
            loge("getDeviceId failed!");
            mDeviceId = null;
        } else {
            mDeviceId = Utils.base64Encode(deviceId);
        }
        mSlotId = slotId;

        logTel("EntitlementServerApi: mDeviceName=" + mDeviceName + ", mDeviceId=" + mDeviceId);

        mThread = new HandlerThread("EntitlementServerApi-worker");
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

    public EntitlementMethodBuilder createMethod() {
        return new EntitlementMethodBuilder(this);
    }

    public void resetAllState() {
        log("resetAllState!!");
        mCurAkaToken = null;
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
                loge("get json data exception: ", e);
            }
        }

        return null;
    }

    // Client doesn't need to call 3GppAuthentication, every HTTPS request needs to do this.
    private JSONObject init3GppAuthentication(JSONObject json) {
        try {
            json.put("message-id", METHOD_3GPPAUTH);
            json.put("method", "3gppAuthentication");
            json.put("device-id", mDeviceId);
            json.put("device-type", 0);
            json.put("os-type", 0);
            json.put("device-name", mDeviceName);

            TelephonyManager telephonyManager =
                    (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
            if (telephonyManager != null) {
                String imsiEap = EapAka.getImsiEapWithSlotId(telephonyManager, mSlotId);
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
            loge("put json data exception: ", e);
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
            loge("isFull3GppAuthNeeded: get json data exception: ", e);
        }

        log("isFull3GppAuthNeeded: no need to do full-3gppAuthentication");
        return false;
    }

    private JSONArray doFull3GppAuthentication(OkHttpClient httpClient,
            JSONArray challenge, JSONArray originalMethods) {
        logTel("doFull3GppAuthentication: challenge=" + challenge);

        if (challenge == null) {
            loge("no challenge?!");
            return null;
        }

        mCurAkaToken = null;

        JSONObject jsonChallenge = find3GppAuthObject(challenge);
        if (jsonChallenge == null) {
            loge("failed to find AKA challenge object");
            return null;
        }

        JSONObject jsonResponse = find3GppAuthObject(originalMethods);
        if (jsonResponse == null) {
            loge("failed find original AKA response object");
            return null;
        }

        TelephonyManager telephonyManager =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (telephonyManager == null) {
            loge("No telephonyManager?!");
            return null;
        }

        // Now start to do aka-challenge!
        try {
            String akaChallenge = jsonChallenge.getString("aka-challenge");
            String akaResponse = EapAka.doEapAkaAuthenticationWithSlotId(
                telephonyManager, akaChallenge, mSlotId);

            if (akaResponse == null) {
                loge("failed to calculate response of AKA challenge");
                return null;
            }

            jsonResponse.remove("aka-token");
            jsonResponse.put("aka-challenge-rsp", akaResponse);
        } catch (JSONException e) {
            loge("JSON exception", e);
            return null;
        }

        JSONArray rsp = sendHttpRequest(httpClient, mApiEndpoint, originalMethods);

        mCurAkaToken = new Response(rsp).getAkaToken();
        if (mCurAkaToken == null) {
            loge("failed to do EAP-AKA from remote");
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

        return true;
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
                    loge("getJsonByMsgId: exception when get data!", e);
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
                loge("Response error code: " + errCode);
                return false;
            }
            return true;
        }

        public int getResponseCode(int msgId) {
            if (jsonArray == null) return ErrorCodes.INVALID_RESPONSE;

            //set error code
            int errCode = getInt(msgId, "response-code");
            ErrorCodes.setErrorCode(errCode, mSlotId);
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
                loge("JSON exception: ", e);
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
                loge("getString() exception: ", e);
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
                loge("getStrings() exception: ", e);
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
                loge("getBoolean() exception: ", e);
            }
            return ret;
        }

        public boolean getEntitlementState(int msgId, String queriedService) {
            boolean ret = false;

            try {
                if (jsonArray != null) {
                    JSONObject json = getJsonByMsgId(msgId);
                    if (json == null) {
                        log("getEntitlementState: can't find object for msg-" + msgId);
                        return false;
                    }

                    JSONArray array = json.getJSONArray("service-entitlement");

                    if (array != null) {
                        int size = array.length();
                        log("Check: " + queriedService + "service-entitlement obj = " + size);
                        for (int i = 0; i < size; i++) {
                            JSONObject obj = array.getJSONObject(i);

                            if (obj != null) {
                                String serviceName = obj.getString("service-name");
                                int entitlementStatus = obj.getInt("entitlement-status");
                                log("service: " + serviceName + "status = " + entitlementStatus);

                                if (serviceName != null &&
                                    serviceName.equalsIgnoreCase(queriedService)
                                    && entitlementStatus == 1000) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            } catch (JSONException e) {
                loge("getEntitlementState() exception: ", e);
            }
            return ret;
        }

        public String getAkaToken() {
            if (jsonArray != null) {
                JSONObject _3gppMethod = find3GppAuthObject(jsonArray);
                if (_3gppMethod == null) {
                    log("getAkaToken(): cannot find 3gppMethod");
                    return null;
                }

                try {
                    return _3gppMethod.getString("aka-token");
                } catch (JSONException e) {
                    loge("getAkaToken() exception: ", e);
                }
            } else {
                log("getAkaToken(): jsonArray is null");
            }
            return null;
        }
    }

    public class EntitlementMethodBuilder {
        private JSONArray jsonArray = new JSONArray();
        private int mMsgCount = 0;
        private EntitlementServerApi mService;

        private EntitlementMethodBuilder(EntitlementServerApi service) {
            mService = service;
        }

        public boolean execute(Callback callback) {
            if (mService != null) {
                return mService.execute(jsonArray, callback);
            }
            return false;
        }

        /*public EntitlementMethodBuilder addManageConnectivity(int operation, String deviceGroup) {
            log("manageConnectivity:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_MANAGE_CONN);
                    json.put("method", "manageConnectivity");
                    json.put("device-id", mDeviceId);
                    json.put("operation", operation);
                    if (deviceGroup != null) {
                        json.put("device-group", deviceGroup);
                    }
                    jsonArray.put(json);
                } catch (JSONException e) {
                    loge("put json data exception: ", e);
                }
            } else {
                log("manageConnectivity: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public EntitlementMethodBuilder addManageService(int operation, String serviceName) {
            log("manageService:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_MANAGE_SERVICE);
                    json.put("method", "manageService");
                    json.put("device-id", mDeviceId);
                    json.put("operation", operation);
                    if (serviceName != null) {
                        JSONObject serviceItem = new JSONObject();
                        serviceItem.put("service-name", serviceName);
                        json.put("service-item", serviceItem);
                    }
                    jsonArray.put(json);
                } catch (JSONException e) {
                    loge("put json data exception: ", e);
                }
            } else {
                log("manageService: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public EntitlementMethodBuilder addManageLocationAndTC(String svsFingerPrint) {
            log("manageLocationAndTC:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_LOCATION_TC);
                    json.put("method", "manageLocationAndTC");
                    json.put("device-id", mDeviceId);
                    json.put("service-fingerprint", svsFingerPrint);
                    jsonArray.put(json);
                } catch (JSONException e) {
                    loge("put json data exception: ", e);
                }
            } else {
                log("manageService: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public EntitlementMethodBuilder addManagePushToken(String msisdn, String serviceName, int operation, String pushToken) {
            log("managePushToken:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_MANAGE_TOKEN);
                    json.put("method", "managePushToken");
                    json.put("device-id", mDeviceId);
                    json.put("msisdn", msisdn);
                    json.put("service-name", serviceName);
                    json.put("operation", operation);
                    json.put("push-token", Utils.base64Encode(pushToken));
                    jsonArray.put(json);
                } catch (JSONException e) {
                    loge("put json data exception: ", e);
                }
            } else {
                log("manageService: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        } */

        public EntitlementMethodBuilder addGetMSISDN() {
            log("getMSISDN:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_GET_MSISDN);
                    json.put("method", "getMSISDN");
                    json.put("device-id", mDeviceId);
                    jsonArray.put(json);
                } catch (JSONException e) {
                    loge("put json data exception: ", e);
                }
            } else {
                log("getMSISDN: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
        }

        public EntitlementMethodBuilder addServiceEntitlementStatus(String serviceList[]) {
            log("serviceEntitlementStatus:");

            if (mMsgCount + 1 < MAX_METHOD_NUM) {
                mMsgCount++;

                try {
                    JSONObject json = new JSONObject();
                    json.put("message-id", METHOD_CHECK_ENTITLEMENT);
                    json.put("method", "serviceEntitlementStatus");
                    json.put("device-id", mDeviceId);

                    JSONArray jarry = new JSONArray();
                    if (serviceList != null) {
                        for (String service : serviceList) {
                            jarry.put(service);
                        }
                    }
                    json.put("service-list", jarry);
                    jsonArray.put(json);
                } catch (JSONException e) {
                    loge("put json data exception: ", e);
                }
            } else {
                log("serviceEntitlementStatus: method is full, mMsgCount=" + mMsgCount);
            }

            return this;
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
                    loge("put json data exception: ", e);
                }

                //JSONObject rsp = sendHttpRequest("http://gnehcic.azurewebsites.net/sample/SampleWebAPI.php", json);
                //log("sendTestAPI get return: " + rsp);
            }
        });
        return true;
    }

    /**
     * This function shall not be run at main thread.
     */
    private JSONArray sendHttpRequest(OkHttpClient client, String url, JSONArray json) {
        logTel("sendHttpRequest: url=" + url);
        logTel("sendHttpRequest: json=" + json);
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
            com.android.okhttp.Response response = client.newCall(request).execute();

            if (response.isSuccessful()) {
                byte[] data = response.body().bytes();
                if (data == null) {
                    log("resEntity: empty byte array");
                    return null;
                }

                String responseString = Utils.gzipDecompress(data);
                logTel("sendHttpRequest: response=" + responseString);
                result = new JSONArray(responseString);
            } else {
                loge("Bad HTTP response:" + response);
            }
            response.body().close();
        } catch (Exception e) {
            loge("sendHttpRequest: Exception:", e);
        }

        return result;
    }

    private boolean checkNoHandlerThread() {
        if (mThreadHandler == null) {
            log("No Handler Thread, return directly");
            return true;
        } else {
            return false;
        }
    }

    private void log(String s) {
        if (DEBUG) Log.d(TAG, "[" + mSlotId + "]" + s);
    }

    private void loge(String s, Exception e) {
        Log.e(TAG, "[" + mSlotId + "]" + s, e);
    }

    private void loge(String s) {
        Log.e(TAG, "[" + mSlotId + "]" + s);
    }

    private void logTel(String s) {
        if (ENG || TELDBG) {
            Log.d(TAG,  "[" + mSlotId + "]" + s.replaceAll("[0-9a-fA-F]{7}", "***"));
        }
    }
}
