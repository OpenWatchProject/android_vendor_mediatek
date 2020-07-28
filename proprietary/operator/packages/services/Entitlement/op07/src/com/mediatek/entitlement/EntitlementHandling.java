package com.mediatek.entitlement;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.util.Log;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;
import java.text.DateFormat;
import java.text.SimpleDateFormat;

import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONException;

class EntitlementHandling {
    private static final String TAG = "EntitlementHandling";
    private static final boolean DEBUG = true;

    private static final String VOWIFI_SERVIVE = "vowifi";

    public interface Listener {
        /**
         * Notify state change.
         */
        void onStateChange(int slotId, int state, boolean silence);

        /**
         * Websheet callback.
         */
        void onWebsheetPost(int slotId, String url, String serverData);

        /**
         * To pass information discovered during the entitlement process.
         */
        void onInfo(int slotId, Bundle info);
    }

    private final Listener mListener;

    private final EntitlementServerApi mApi;

    static final int STATE_NOT_ENTITLED = 0;
    static final int STATE_ENTITLEMENT_CHECKING = 1;
    static final int STATE_ACTIVATING = 2;
    static final int STATE_PENDING_WITH_NO_TC = 3;
    static final int STATE_PENDING_WITH_NO_ADDRESS = 4;
    static final int STATE_ENTITLED = 5;
    static final int STATE_DEACTIVATING = 6;
    static final int STATE_DEACTIVATING_PENDING_CHECK = 7;

    private int mState = STATE_NOT_ENTITLED;

    private Context mContext;

    private static final int EVENT_WAIT_FCM_TOKEN = 0;
    private static final int DELAY_WAIT_FCM_TOKEN = 3000; // 3s

    private static final String DEVICE_GROUP_MEDIATEK = "Mediatek";
    private static final String WFC_AID_VALUE = "wfc_aid_value";

    public static final String ENTITLEMENT_CHECK_RETRY_TIMES = "entitlement-check-retry-times";
    public static final String ENTITLEMENT_CHECK_RETRY_SILENCE = "entitlement-check-retry-silence";

    private static final int ENTITLEMENT_CHECK_ERROR_NOT_PROVISIONED = 1048;
    private static final int ENTITLEMENT_CHECK_ERROR_SUSPENDED = 1063;
    private static final int ENTITLEMENT_CHECK_ERROR_SERVER_ERROR = 1111;

    private String mFcmToken; // Get from OP07Entitlement-Fcm.apk, used for managePushNotification.
    private String mMSISDN; // Get from device_activation.
    private String mServiceFingerprint; // Get from device_activation, used for location_registration.
    private String mAidExpiration; // Get from location_registration, used for AID expired checking.

    String mWebsheetToken;
    String mWebsheetUrl;
    int mLastStatus;
    int mSlotId;

    boolean mSilence;
    boolean mPendingSilence;

    EntitlementHandling(int slotId, Context context, Listener listener) {
        mSlotId = slotId;
        mContext = context;
        mListener = listener;

        mApi = new EntitlementServerApi(mContext, getEntitlementApiEndpoint(), getDeviceId(mContext), slotId);
    }

    // These 'public' methods will be called by any thread:

    synchronized void startEntitlementCheck(boolean silence) {
        log("startEntitlementCheck: state = " + stateToString(mState) +
            ", silence:" + silence);


        switch (mState) {
            case STATE_NOT_ENTITLED:
            case STATE_ENTITLED:
            case STATE_PENDING_WITH_NO_TC:
            case STATE_PENDING_WITH_NO_ADDRESS:

                mSilence = silence;

                doEntitlementCheck();
                break;

            case STATE_ENTITLEMENT_CHECKING:
            case STATE_ACTIVATING:

                if (!silence) {
                    mSilence = silence;
                }

                log("Ignore request to do entitlement check.");
                break;

            case STATE_DEACTIVATING:
                log("Wait the deactivation to finish before starting entitlement check.");
                updateState(STATE_DEACTIVATING_PENDING_CHECK);

                mPendingSilence = silence;

                break;

            case STATE_DEACTIVATING_PENDING_CHECK:
                log("Already has a pending entitlement check.");
                if (!silence) {
                    mPendingSilence = silence;
                }
                break;

            default:
                loge("Unhandled state!");
                break;
        }

        // avoid infinite loop, so remove it now and delete the source code after a while
        // validateAid();
    }

    synchronized void stopEntitlementCheck() {
        log("stopEntitlementCheck: state = " + stateToString(mState));

        mSilence = false;

        switch (mState) {
            case STATE_ACTIVATING:
            case STATE_ENTITLEMENT_CHECKING:
            case STATE_PENDING_WITH_NO_TC:
            case STATE_PENDING_WITH_NO_ADDRESS:
                log("Stop on-going processing");

                mHandler.removeMessages(EVENT_WAIT_FCM_TOKEN);
                updateState(STATE_NOT_ENTITLED);
                break;

            case STATE_ENTITLED:
            case STATE_NOT_ENTITLED:
                log("Already in a stopped state.");
                break;

            case STATE_DEACTIVATING:
                log("Ignored in deactivation.");
                break;

            case STATE_DEACTIVATING_PENDING_CHECK:
                updateState(STATE_DEACTIVATING);
                break;

            default:
                loge("Unhandled state!");
                break;
        }
    }

    synchronized void updateLocationAndTc(boolean forceUpdateAddress) {
        log("updateLocationAndTc: state = " + stateToString(mState) +
            ", forceUpdateAddress:" + forceUpdateAddress);

        mSilence = false;

        switch (mState) {
            case STATE_ENTITLED:
                doAddressRegistration(forceUpdateAddress);
                break;
            case STATE_ACTIVATING:
            case STATE_ENTITLEMENT_CHECKING:
            case STATE_PENDING_WITH_NO_TC:
            case STATE_PENDING_WITH_NO_ADDRESS:
            case STATE_NOT_ENTITLED:
                log("Cannot updateLocationAndTc at current state");
                break;

            case STATE_DEACTIVATING:
            case STATE_DEACTIVATING_PENDING_CHECK:
                log("Ignored in deactivation.");
                break;

            default:
                loge("Unhandled state!");
                break;
        }
    }

    synchronized void deactivateService(boolean silence) {
        log("deactivateService: state = " + stateToString(mState) + ", silence:" + silence);

        if (mFcmToken != null) {
            switch (mState) {
                case STATE_ENTITLED:
                case STATE_ACTIVATING:
                case STATE_ENTITLEMENT_CHECKING:
                case STATE_PENDING_WITH_NO_TC:
                case STATE_PENDING_WITH_NO_ADDRESS:
                case STATE_NOT_ENTITLED:

                    mSilence = silence;
                    mHandler.removeMessages(EVENT_WAIT_FCM_TOKEN);
                    updateState(STATE_DEACTIVATING);
                    doDeactivateService();
                    break;

                case STATE_DEACTIVATING:
                case STATE_DEACTIVATING_PENDING_CHECK:

                    log("Already in deactivation.");
                    if (!silence) {
                        mSilence = silence;
                    }
                    break;

                default:
                    loge("Unhandled state!");
                    break;
            }
        } else {
            loge("Push token is null. Do nothing.");
        }
    }

    synchronized void updateFcmToken(String token) {

        boolean isFcmTokenChanged = (mFcmToken != null && !token.equals(mFcmToken));
        boolean isFcmTokenNull = (mFcmToken == null);

        log("updateFcmToken(): state = " + stateToString(mState) +
            ", isChanged:" + isFcmTokenChanged + ", mFcmToken:" + mFcmToken + ", token:" + token );

        mFcmToken = token;

        mHandler.removeMessages(EVENT_WAIT_FCM_TOKEN);

        // retry activate device again
        if (isFcmTokenNull && mState == STATE_ACTIVATING) {
            activateDevice();
        } else if (isFcmTokenChanged) {
            doSendFcmToken(mFcmToken);
        }
    }

    synchronized void validateAid() {
        log("validateAid: state = " + stateToString(mState) + ", mAidExpiration:" + mAidExpiration);

        if (aidExpiresIn48Hours(mAidExpiration) && mFcmToken != null) {
            log("AID expired soon. Check entitlement status again");
            startEntitlementCheck(true);
        }
    }

    synchronized void handleFcmNotification(String payload) {
        log("onHandleFcmNotification: " + payload);

        /*
         Data: {message={"eventList":{"date":"2019-07-04T07:08:09+0000","events":[{"type":"E911_ADDR_UPDATE"}]}}}
         payload: {"eventList":{"date":"2019-07-04T07:08:09+0000","events":[{"type":"E911_ADDR_UPDATE"}]}}
        */

        if (payload != null) {

            if (payload.contains("entitlement-changed") ||
                payload.contains("E911_ADDR_UPDATE") ||
                payload.contains("ENTMT_UPDATE")) {

                // Do SES check and manageLocationAndTC() check
                startEntitlementCheck(true);

            }
        }
    }

    synchronized int getState() {
        return mState;
    }

    private void activateDevice() {
        log("activateDevice()");

        updateState(STATE_ACTIVATING);

        if (mFcmToken == null) {
            log("mFcmToken is empty, send message: EVENT_WAIT_FCM_TOKEN");

            mHandler.sendMessageDelayed(
                mHandler.obtainMessage(EVENT_WAIT_FCM_TOKEN), DELAY_WAIT_FCM_TOKEN);

        } else if (mMSISDN == null || mServiceFingerprint == null) {

            log("run devivce activation");
            doDeviceActivation();
        } else {

            log("run address registration (use msisdn and ServiceFingerprint)");
            doAddressRegistration(false);
        }
    }

    private void doDeviceActivation() {
        mApi.createMethod()
                .addManageConnectivity(0 /* activate */, DEVICE_GROUP_MEDIATEK)
                .addGetMSISDN()
                .execute(new EntitlementServerApi.Callback() {
                    @Override
                    public void callback(EntitlementServerApi.Response rsp) {
                        onDeviceActivationResponse(rsp);
                    }
                });
    }

    private synchronized void onDeviceActivationResponse(EntitlementServerApi.Response rsp) {
        if (mState != STATE_ACTIVATING) {
            log("Probably stopped");
            return;
        }

        if (rsp == null || !rsp.isSuccessful(EntitlementServerApi.METHOD_GET_MSISDN)) {
            loge("doDeviceActivation: receive failed callback, stop procedure now");
            updateState(STATE_NOT_ENTITLED);
            return;
        }

        mMSISDN = rsp.getString(EntitlementServerApi.METHOD_GET_MSISDN, "msisdn");
        mServiceFingerprint = rsp.getString(EntitlementServerApi.METHOD_GET_MSISDN, "service-fingerprint");

        doAddressRegistration(false);
    }

    private void doAddressRegistration(boolean forceUpdateAddress) {
        mApi.createMethod()
                .addManageLocationAndTC(mServiceFingerprint)
                .addManagePushToken(mMSISDN, "vowifi+vvm", 0, mFcmToken)
                .execute(new EntitlementServerApi.Callback() {
                    @Override
                    public void callback(EntitlementServerApi.Response rsp) {
                        onAddressRegistrationResponse(rsp, forceUpdateAddress);
                    }
                });
    }

    private synchronized void onAddressRegistrationResponse(
        EntitlementServerApi.Response rsp, boolean forceUpdateAddress) {

        if (mState != STATE_ACTIVATING && mState != STATE_ENTITLED
                && mState != STATE_PENDING_WITH_NO_TC && mState != STATE_PENDING_WITH_NO_ADDRESS) {
            log("Probably stopped");
            return;
        }
        if (rsp == null || !rsp.isSuccessful(EntitlementServerApi.METHOD_LOCATION_TC)) {
            log("doAddressRegistration: receive failed callback, stop procedure now");
            updateState(STATE_NOT_ENTITLED);
            return;
        }

        mWebsheetToken = rsp.getString(EntitlementServerApi.METHOD_LOCATION_TC, "server-data");
        mWebsheetUrl = rsp.getString(EntitlementServerApi.METHOD_LOCATION_TC, "server-url");
        String aid = rsp.getString(EntitlementServerApi.METHOD_LOCATION_TC, "address-id");
        mAidExpiration = rsp.getString(EntitlementServerApi.METHOD_LOCATION_TC, "aid-expiration");

        boolean needCheckTc = rsp.checkValueExisted(EntitlementServerApi.METHOD_LOCATION_TC, "tc-status");
        boolean tcStatus = rsp.getBoolean(EntitlementServerApi.METHOD_LOCATION_TC, "tc-status");
        boolean needCheckLocation = rsp.checkValueExisted(EntitlementServerApi.METHOD_LOCATION_TC, "location-status");
        boolean locationStatus = rsp.getBoolean(EntitlementServerApi.METHOD_LOCATION_TC, "location-status");
        saveAidValue(mContext, aid);

        log("doAddressRegistration: response received: " +
                ", needCheckTc=" + needCheckTc + ", tcStatus=" + tcStatus +
                ", needCheckLocation=" + needCheckLocation + ", locationStatus=" + locationStatus +
                ", aid=" + aid +
                ", isAidValid()=" + isAidValid(aid) +
                ", forceUpdateAddress:" + forceUpdateAddress +
                ", mSilence:" + mSilence);

        if (needCheckTc && !tcStatus) {
            updateState(STATE_PENDING_WITH_NO_TC);
        } else if (needCheckLocation && (!locationStatus || !isAidValid(aid))) {
            updateState(STATE_PENDING_WITH_NO_ADDRESS);
        } else if (!forceUpdateAddress){

            updateState(STATE_ENTITLED);
        }

        if ((mState == STATE_PENDING_WITH_NO_TC || mState == STATE_PENDING_WITH_NO_ADDRESS ||
            forceUpdateAddress) && !mSilence) {

             onWebsheetPost();
        }
    }

    private synchronized void onWebsheetPost() {
        log("onWebsheetPost(), url: " + mWebsheetUrl + ", token:" + mWebsheetToken);

        mListener.onWebsheetPost(mSlotId, mWebsheetUrl, mWebsheetToken);

    }

    private void doEntitlementCheck() {

        final int preState = mState;

        updateState(STATE_ENTITLEMENT_CHECKING);

        mApi.createMethod()
                .addServiceEntitlementStatus(new String[]{VOWIFI_SERVIVE})
                .execute(new EntitlementServerApi.Callback() {
                    @Override
                    public void callback(EntitlementServerApi.Response rsp) {
                        onEntitlementCheckResponse(rsp, preState);
                    }
                });
    }

    private synchronized void onEntitlementCheckResponse(
        EntitlementServerApi.Response rsp, int preState) {

        if (mState != STATE_ENTITLEMENT_CHECKING) {
            log("Probably stopped");
            return;
        }

        boolean isEntitled;
        if (rsp == null || !rsp.isSuccessful(EntitlementServerApi.METHOD_CHECK_ENTITLEMENT)) {
            loge("Receive failed callback, rsp:" + rsp);
            isEntitled = false;
        } else {
            isEntitled = rsp.getEntitlementState(EntitlementServerApi.METHOD_CHECK_ENTITLEMENT, VOWIFI_SERVIVE);
        }
        log("Entitlement result: " + isEntitled + ", mLastStatus:" + mLastStatus);


        if (isEntitled) {
            mLastStatus = ErrorCodes.REQUEST_SUCCESSFUL;

            // Go on to run SES flow
            activateDevice();

        } else {

            // prepare next state
            switch (preState) {
                case STATE_NOT_ENTITLED:
                case STATE_PENDING_WITH_NO_TC:
                case STATE_PENDING_WITH_NO_ADDRESS:
                    log("Keep in the same state even though entitlement check failed");
                    break;
                case STATE_ENTITLED:
                    log("entitled -> not_entitled (entitlement check failed)");
                    preState = STATE_NOT_ENTITLED;
                    break;

                default:
                    loge("Unhandled state!");
                    break;
            }

            int status = -1;
            // LTE_BTR-5-7250: 1063 and 1111
            if (rsp != null) {
                status = rsp.getEntitlementStateValue(
                    EntitlementServerApi.METHOD_CHECK_ENTITLEMENT, VOWIFI_SERVIVE);

                // step2: avoid websheet request after 1063 and no retry
                // step4: avoid websheet request after 1111 and retry 4 times

                if (status == ENTITLEMENT_CHECK_ERROR_NOT_PROVISIONED /* 1048 */) {

                    // Go on to run SES flow
                    activateDevice();

                } else if (status == ENTITLEMENT_CHECK_ERROR_SUSPENDED /* 1063 */) {

                    onInfoEntitlementCheckRetryTimes(0);

                    // for toggle wfc off
                    updateState(STATE_NOT_ENTITLED);

                } else if (status == ENTITLEMENT_CHECK_ERROR_SERVER_ERROR /* 1111 */) {

                    // Retry 4 times when receives 1111 for the first time
                    if (mLastStatus != status) {
                        onInfoEntitlementCheckRetryTimes(4);
                    }
                    updateState(preState);

                } else {
                    updateState(preState);
                }

            } else {
                updateState(preState);
            }

            mLastStatus = status;
        }
    }

    private void doSendFcmToken(String token) {
        mApi.createMethod()
                .addManagePushToken(mMSISDN, "vowifi+vvm", 0, token)
                .execute(new EntitlementServerApi.Callback() {
                    @Override
                    public void callback(EntitlementServerApi.Response rsp) {
                        if (rsp == null || !rsp.isSuccessful(EntitlementServerApi.METHOD_MANAGE_TOKEN)) {
                            log("doSendFcmToken: receive failed callback, stop procedure now");
                        }
                    }
                });
    }

    private void doDeactivateService() {
        mApi.createMethod()
                .addManagePushToken(mMSISDN, "vowifi+vvm", 1, mFcmToken)
                .execute(new EntitlementServerApi.Callback() {
                    @Override
                    public void callback(EntitlementServerApi.Response rsp) { onDeactivateServiceResponse(rsp); }
                });
    }

    private synchronized void onDeactivateServiceResponse(EntitlementServerApi.Response rsp) {
        if (rsp == null || !rsp.isSuccessful(EntitlementServerApi.METHOD_MANAGE_TOKEN)) {
            loge("Received failed callback");
        }
        if (mState == STATE_DEACTIVATING_PENDING_CHECK) {

            updateState(STATE_NOT_ENTITLED);

            startEntitlementCheck(mPendingSilence);
            mPendingSilence = false;

        } else {
            updateState(STATE_NOT_ENTITLED);
        }
    }

    private synchronized void updateState(int state) {
        log("updateState: from " + stateToString(mState) + " to " + stateToString(state));
        mState = state;

        mListener.onStateChange(mSlotId, mState, mSilence);
    }

    private synchronized void onInfoEntitlementCheckRetryTimes(int times) {
        log("onInfoEntitlementCheckRetryTimes(), times:" + times);

        Bundle bundle = new Bundle();

        bundle.putInt(ENTITLEMENT_CHECK_RETRY_TIMES, times);
        bundle.putInt(ENTITLEMENT_CHECK_RETRY_SILENCE, mSilence ? 1 : 0);

        mListener.onInfo(mSlotId, bundle);

    }

    private boolean isAidValid(String aid) {
        return !(aid == null || aid.equals(""));
    }

    private String getDeviceId(Context context) {
        return Settings.Secure.getString(context.getContentResolver(), Settings.Secure.ANDROID_ID);
    }

    private void saveAidValue(Context context, String aid) {
        if (!android.provider.Settings.Global.putString(context.getContentResolver(),
            WFC_AID_VALUE, aid)) {
            loge("Save AID failed");
        }
    }

    public static String stateToString(int state) {
        switch (state) {
            case STATE_ACTIVATING:
                return "STATE_ACTIVATING";
            case STATE_NOT_ENTITLED:
                return "STATE_NOT_ENTITLED";
            case STATE_ENTITLEMENT_CHECKING:
                return "STATE_ENTITLEMENT_CHECKING";
            case STATE_PENDING_WITH_NO_TC:
                return "STATE_PENDING_WITH_NO_TC";
            case STATE_PENDING_WITH_NO_ADDRESS:
                return "STATE_PENDING_WITH_NO_ADDRESS";
            case STATE_ENTITLED:
                return "STATE_ENTITLED";
            case STATE_DEACTIVATING:
                return "STATE_DEACTIVATING";
            case STATE_DEACTIVATING_PENDING_CHECK:
                return "STATE_DEACTIVATING_PENDING_CHECK";
            default:
                return "UNKNOWN state";
        }
    }

    private boolean aidExpiresIn48Hours(String aidExpiration) {
        if (aidExpiration == null || aidExpiration.length() == 0) {
            return false;
        }

        boolean result = false;

        Date currentDate = new Date();

        Calendar cal = Calendar.getInstance();
        cal.setTime(currentDate);
        cal.add(Calendar.HOUR, 48);
        Date twoDaysLater = cal.getTime();

        TimeZone tz = TimeZone.getTimeZone("UTC");
        DateFormat df = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.getDefault()); // Quoted "Z" to indicate UTC, no timezone offset
        df.setTimeZone(tz);

        try {
            result = twoDaysLater.after(df.parse(aidExpiration));
            log("aidExpiresIn48Hours: " + result);
        } catch (java.text.ParseException e) {
            loge("UTC ISO8601 parse exception!" + e.getMessage());
        }
        return result;
    }

    private String getEntitlementApiEndpoint() {
        // Possible endpoints:
        // - AT&T Lab SES Server: https://sentitlement2.npc.mobilephone.net/WFC
        // - AT&T live Network SES Server: https://sentitlement2.mobile.att.net/WFC
        // - Ericson old SES Testing server: http://ses.ericsson-magic.net:10090/generic_devices
        // - Ericson new SES Testing server: http://ses.ericsson-magic.net:8080/generic_devices
        //
        // By default, connect to AT&T live network SES server.
        return SystemProperties.get("persist.vendor.entitlement.sesurl", "https://sentitlement2.mobile.att.net/WFC");
    }

    private final Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {

                case EVENT_WAIT_FCM_TOKEN:

                    log("handleMessage: EVENT_WAIT_FCM_TOKEN, mState: " + mState);
                    if (mState == STATE_ACTIVATING) {
                        updateState(STATE_NOT_ENTITLED);
                    }
                    break;
            }
        }
    };


    private void log(String s) {
        if (DEBUG) Log.d(TAG, "[" + mSlotId + "]" + s);
    }

    private void loge(String s) {
        Log.e(TAG, "[" + mSlotId + "]" + s);
    }
}
