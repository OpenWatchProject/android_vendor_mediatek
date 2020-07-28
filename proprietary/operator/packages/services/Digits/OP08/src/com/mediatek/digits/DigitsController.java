package com.mediatek.digits;


import android.content.Context;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Message;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.util.Log;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.internal.util.StateMachine;
import com.android.internal.util.State;

import com.mediatek.internal.telephony.MtkPhoneConstants;

import com.mediatek.digits.DigitsConfig;
import com.mediatek.digits.DigitsConfigManager;

import com.mediatek.digits.DigitsStorage;
import com.mediatek.digits.gcm.DigitsGcmHandler;
import com.mediatek.digits.gcm.GcmManager;

import com.mediatek.digits.iam.AccessToken;
import com.mediatek.digits.iam.IamManager;
import com.mediatek.digits.iam.MsisdnAuth;
import com.mediatek.digits.iam.UserProfile;

import com.mediatek.digits.ril.DigitsRIL;
import com.mediatek.digits.ses.SesManager;
import com.mediatek.digits.wsg.WsgManager;

import com.mediatek.digits.utils.CallbackScheduler;

import java.util.Arrays;
import java.util.List;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class DigitsController {

    static protected final String TAG = "DigitsController";

    // Declare internal message command
    static final protected int CMD_GET_DEVICE_CONFIG = 1;
    static final protected int CMD_GET_DEVICE_CONFIG_RESULT = 2;
    static final protected int CMD_ON_BOARDING = 3;
    static final protected int CMD_ON_BOARDING_RESULT = 4;

    static final protected int CMD_GET_ACCESS_TOKEN_RESULT = 6;
    static final protected int CMD_REFRESH_ACCESS_TOKEN_RESULT = 7;
    static final protected int CMD_GET_CONSUMER_PROFILE_RESULT = 8;
    static final protected int CMD_GET_MSISDN_AUTH_LIST_RESULT = 9;
    static final protected int CMD_CANCEL_TRANSACTION_RESULT = 10;

    static final protected int CMD_SUBSCRIBE_RESULT = 11;
    static final protected int CMD_UPDATE_MSISDN_SIT = 12;
    static final protected int CMD_UPDATE_MSISDN_SIT_RESULT = 13;
    static final protected int CMD_SIT_RENEW = 14;

    static final protected int CMD_ACTIVATE_LINE = 15;
    static final protected int CMD_DEACTIVATE_LINE = 16;

    static final protected int CMD_PROVISION_LINE_RESULT = 17;
    static final protected int CMD_ADD_HUNT_GROUP = 18;
    static final protected int CMD_ADD_HUNT_GROUP_RESULT = 19;

    static final protected int CMD_REMOVE_HUNT_GROUP_RESULT = 21;
    static final protected int CMD_REMOVE_LINE = 22;
    static final protected int CMD_REMOVE_LINE_RESULT = 23;
    static final protected int CMD_GET_TRN_RESULT = 24;

    static final protected int CMD_SET_DEVICE_NAME_RESULT = 26;
    static final protected int CMD_SET_LINE_NAME_RESULT = 27;

    static final protected int CMD_INIT_MSISDN_AUTH_RESULT = 28;

    static final protected int CMD_DELETE_DEVICE_RESULT = 31;
    static final protected int CMD_USER_LOGOUT = 32;
    static final protected int CMD_SIGNOUT_RESULT = 33;
    static final protected int CMD_GET_MSISDN_RESULT = 34;
    static final protected int CMD_UPDATE_LOCATION_AND_TC_RESULT = 35;

    static final protected int CMD_REGISTERED_DEVICES = 36;
    static final protected int CMD_REGISTERED_DEVICES_RESULT = 37;
    static final protected int CMD_REGISTERED_MSISDN = 38;
    static final protected int CMD_REGISTERED_MSISDN_RESULT = 39;

    static final protected int CMD_REFRESH_DEVICE_INFO = 41;
    static final protected int CMD_SIM_LOADED_EVENT = 42;

    static final protected int CMD_REFRESH_PUSH_TOKEN = 46;
    static final protected int CMD_REFRESH_PUSH_TOKEN_RESULT = 47;

    static final protected int CMD_PUSH_LINE_NAME_UPDATED = 51;
    static final protected int CMD_PUSH_CALL_ANSWERED = 52;
    static final protected int CMD_MSISDN_AUTH_CHANGED = 53;
    static final protected int CMD_MSISDN_AUTH_REMOVED = 54;

    static final protected int CMD_RIL_SET_TRN = 101;
    static final protected int CMD_RIL_SET_DIGITS_LINE = 102;
    static final protected int CMD_RIL_SET_DIGITS_LINE_RESPONSE = 103;
    static final protected int CMD_RIL_SET_TRN_RESPONSE = 104;
    static final protected int CMD_RIL_DIGITS_LINE_INDICATION = 105;
    static final protected int CMD_RIL_GET_TRN_INDICATION = 106;

    static final private String INTENT_ACTION_REFRESH_PUSH_TOKEN =
        "com.mediatek.digits.REFRESH_PUSH_TOKEN";
    static final private String INTENT_ACTION_REFRESH_REGISTERED_DEVICE =
        "com.mediatek.digits.REFRESH_REGISTERED_DEVICE";

    static final private int PERIODICAL_REGISTERED_DEVICE_MILLIS = 48 * 60 * 60 * 1000;
    static final private int PERIODICAL_RETRY_ON_BOARDING_MILLIS = 2 * 1000;

    private boolean mPeriodicRegisteredDevice;

    private boolean mRegisteredDeviceRunning;
    private boolean mRegisteredMsisdnRunning;
    private DigitsLine[] mRegisteredLineLastTime;

    static final private int LINE_REQUEST_REASON_NONE = 0;
    static final private int LINE_REQUEST_REASON_SUBSCRIBE = 1;
    static final private int LINE_REQUEST_REASON_ACTIVATE_LINE = 2;
    static final private int LINE_REQUEST_REASON_DEACTIVATE_LINE = 3;
    static final private int LINE_REQUEST_REASON_LOGOUT = 4;
    private int mLineReqReason;

    private boolean mSimLoaded;
    private boolean mNoE911AddressOnFile;

    private int mState;
    private int mJanskyService;

    // activate, deactivate line request
    private class LineRequest {
        String mMsisdn;
        int mResult;
    }
    private ArrayList<LineRequest> mLineReqList = new ArrayList<LineRequest>();
    private int mCurLineRequestIndex;

    // set device name request
    private String mDeviceNameReq;
    private String mAddLineReq;

    private boolean mSubscribeReq;
    private boolean mLogoutReq;
    private boolean mGetDeviceConfigReq;

    // set line name request
    private class LineNameReq {
        String mMsisdn;
        String mName;
    }
    private LineNameReq mLineNameReq;

    // get trn request
    private class GetTrnReq {
        String mFrom;
        String mTo;
    }
    private GetTrnReq mGetTrnReq;

    // Msisdn auth removed list: wait to deactivate
    private ArrayList<String> mAuthRemovedMsisdns = new ArrayList<String>();

    // Decalare 4 managers
    private IamManager mIamManager;
    private SesManager mSesManager;
    private WsgManager mWsgManager;
    private GcmManager mGcmManager;
    private DigitsConfigManager mConfigManager;
    private DigitsControllerListener mCtrlListener;

    private Context mContext;
    private Handler mClientHandler;
    private Handler mHandler;

    private DigitsStorage mStorage;
    private DigitsRIL mRil;
    private int mRilSerial;

    HandlerThread mCtrlHandlerThread;


    private CallbackScheduler mPushTokenScheduler;
    private CallbackScheduler mRegisteredDeviceScheduler;

    public DigitsController(Context context, Handler clientHandler) {

        Log.d(TAG, "DigitsController() Constructor");

        mContext = context;
        mClientHandler = clientHandler;

        // Start handler thread for DigitsController handler
        mCtrlHandlerThread = new HandlerThread("DigitsController");
        mCtrlHandlerThread.start();
        initHandler();

        initManager();

        mStorage = new DigitsStorage(context);

        registerRil();

        initCallbackScheduler();

        setJanskyService(DigitsConst.JANSKY_SERVICE_UNKNOWN);
        enterState(DigitsConst.STATE_IDLE);
    }

    private void initManager() {
        Log.d(TAG, "initManager()");

        // init controller listener
        mCtrlListener = new DigitsControllerListener(mContext, mHandler);


        // Start 3 handler thread & handler
        HandlerThread iamHandlerThread = new HandlerThread("IamManager");
        iamHandlerThread.start();

        mIamManager = new IamManager(mContext,
            iamHandlerThread.getLooper(), mCtrlListener.getIamListener());

        HandlerThread sesHandlerThread = new HandlerThread("SesManager");
        sesHandlerThread.start();

        mSesManager = new SesManager(mContext,
            sesHandlerThread.getLooper(), mCtrlListener.getSesListener());

        HandlerThread wsgHandlerThread = new HandlerThread("WsgManager");
        wsgHandlerThread.start();

        mWsgManager = new WsgManager(mContext,
            wsgHandlerThread.getLooper(), mCtrlListener.getWsgListener());

        mGcmManager = new GcmManager(mContext, mCtrlListener.getGcmListener());

        mConfigManager = new DigitsConfigManager(mContext, mCtrlListener.getConfigListener());

    }


    private void registerRil() {
        Log.d(TAG, "registerRil()");
        mRil = new DigitsRIL(mContext, getMainCapabilityPhoneId());

        mRil.registerDigitsLineIndication(mHandler, CMD_RIL_DIGITS_LINE_INDICATION, null);
        mRil.registerGetTrnIndication(mHandler, CMD_RIL_GET_TRN_INDICATION, null);
    }

    private void initCallbackScheduler() {
        Log.d(TAG, "initCallbackScheduler()");

        mPushTokenScheduler = new CallbackScheduler(mContext,
            INTENT_ACTION_REFRESH_PUSH_TOKEN,
            new CallbackScheduler.Callback() {

                public void run(String action) {
                    Log.d(TAG, "run: " + action);
                    mHandler.sendMessage(mHandler.obtainMessage(CMD_REFRESH_PUSH_TOKEN));
                }

            });

        mRegisteredDeviceScheduler = new CallbackScheduler(mContext,
            INTENT_ACTION_REFRESH_REGISTERED_DEVICE,
            new CallbackScheduler.Callback() {

                public void run(String action) {
                    Log.d(TAG, "run: " + action);
                    mHandler.sendMessage(mHandler.obtainMessage(CMD_REGISTERED_DEVICES, 1, 0));
                }

            });
    }

    private int getMainCapabilityPhoneId() {
       int phoneId = SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
       if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
           phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
       }
       Log.d(TAG, "getMainCapabilityPhoneId = " + phoneId);
       return phoneId;
    }

    private void getDeviceConfigResult(int result) {
        Log.d(TAG, "getDeviceConfigResult() result: " + result +
            ", mState:" + stateToString(mState));

        // Update result to config manager
        mConfigManager.notifyGetConfigResult(
            (result == SesManager.EVENT_RESULT_SUCCEED) ? true : false);

        // Notify caller
        if (mGetDeviceConfigReq) {

            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_GET_DEVICE_CONFIG_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0));
        }

        if (result != SesManager.EVENT_RESULT_SUCCEED) {
            // Do nothing
            return;
        }

        DigitsConfig config = DigitsConfig.getInstance();
        boolean on = config.isJanskyServerAllowed();

        setJanskyService(on ? DigitsConst.JANSKY_SERVICE_ON : DigitsConst.JANSKY_SERVICE_OFF);

        if (inState(DigitsConst.STATE_IDLE) && on) {
            onBoarding(false);

        } else if (inState(DigitsConst.STATE_READY) && !on) {

            enterState(DigitsConst.STATE_IDLE);

        } else if (inState(DigitsConst.STATE_SUBSCRIBED)) {

            if (on) {
                // TODO: update config parameter
            } else {
                mHandler.sendMessage(mHandler.obtainMessage(CMD_USER_LOGOUT, 0, 0));
            }
        }
    }

    private void onBoarding(boolean logout) {

        // Init GCM token, only can run on non-main thread
        String gcmSenderId = DigitsConfig.getInstance().getGCMSenderID();
        DigitsGcmHandler.getInstance(mContext).initGcmToken(gcmSenderId);

        // Call SES API to on boarding
        String gcmToken = DigitsGcmHandler.getInstance(mContext).getGcmToken();

        Log.d(TAG, "onBoarding() logout: " + logout + ", gcmToken: " + gcmToken);

        if (gcmToken == null) {
            // Retry to get access token again
            mHandler.sendMessageDelayed(
                mHandler.obtainMessage(CMD_ON_BOARDING, logout ? 1: 0, 0),
                PERIODICAL_RETRY_ON_BOARDING_MILLIS);

        } else {
            mSesManager.onBoarding(logout, gcmToken);
        }
    }

    private void onBoardingResult(int result, Bundle bundle) {
        Log.d(TAG, "onBoardingResult(), result: " + result + ", bundle:" + bundle +
            ", mState:" + stateToString(mState));

        if (result != SesManager.EVENT_RESULT_SUCCEED) {

            if (inState(DigitsConst.STATE_LOGOUT)) {

                if (mJanskyService == DigitsConst.JANSKY_SERVICE_OFF) {
                    enterState(DigitsConst.STATE_IDLE);
                } else {
                    enterState(DigitsConst.STATE_READY);
                }

                // Notify caller
                if (mLogoutReq) {
                    mClientHandler.sendMessage(mClientHandler.obtainMessage(
                        DigitsServiceImpl.CMD_LOGOUT_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0));
                }
            }
            return;
        }

        if (!e911AddressOnFile(bundle)) {

            return;
        }

        String nativeMsisdn = bundle.getString(SesManager.EVENT_KEY_MSISDN);
        String nativeSit = bundle.getString(SesManager.EVENT_KEY_SIT);

        // Save to storage
        mStorage.setNativeMsisdn(nativeMsisdn);
        mStorage.setLineSit(nativeMsisdn, nativeSit);

        if (inState(DigitsConst.STATE_IDLE)) {

            // Start push token refresh timer
            startPushTokenTimer();

            if (!autoLogin()) {

                enterState(DigitsConst.STATE_READY);

                // SIP reg for native line
                setDigitsLine();
            }

        } else if (inState(DigitsConst.STATE_LOGOUT)) {

            // SIP reg for native line
            setDigitsLine();

            if (mJanskyService == DigitsConst.JANSKY_SERVICE_OFF) {
                enterState(DigitsConst.STATE_IDLE);
            } else {
                enterState(DigitsConst.STATE_READY);
            }

            // Notify caller
            if (mLogoutReq) {
                // Notify caller
                mClientHandler.sendMessage(mClientHandler.obtainMessage(
                    DigitsServiceImpl.CMD_LOGOUT_RESULT, DigitsConst.RESULT_SUCCEED, 0));
            }
        } else {
            // check why state is wrong
        }
    }

    private void updateLocationAndTcResult(int result, Bundle bundle) {
        Log.d(TAG, "updateLocationAndTcResult(), result: " + result + ", bundle:" + bundle +
            ", mState:" + stateToString(mState));

        if (result != SesManager.EVENT_RESULT_SUCCEED) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_UPDATE_LOCATION_AND_TC_RESULT,
                DigitsConst.RESULT_FAIL_UNKNOWN, 0));
            return;
        }

        String serverData = bundle.getString(SesManager.EVENT_KEY_SERVER_DATA);
        String serverUrl = bundle.getString(SesManager.EVENT_KEY_SERVER_URL);

        Bundle output = new Bundle();
        output.putString(DigitsConst.EXTRA_STRING_SERVER_URL, serverUrl);
        output.putString(DigitsConst.EXTRA_STRING_SERVER_DATA, serverData);

        // Notify caller
        mClientHandler.sendMessage(mClientHandler.obtainMessage(
            DigitsServiceImpl.CMD_UPDATE_LOCATION_AND_TC_RESULT,
            DigitsConst.RESULT_SUCCEED, 0, output));

    }

    private boolean e911AddressOnFile(Bundle bundle) {

        String serverData = bundle.getString(SesManager.EVENT_KEY_SERVER_DATA);
        String serverUrl = bundle.getString(SesManager.EVENT_KEY_SERVER_URL);

        if (serverData != null && serverUrl != null) {
            Log.e(TAG, "e911AddressOnFile(), data: " + serverData + ", url:" + serverUrl);

            mNoE911AddressOnFile = true;

            Bundle output = new Bundle();
            output.putString(DigitsConst.EXTRA_STRING_SERVER_URL, serverUrl);
            output.putString(DigitsConst.EXTRA_STRING_SERVER_DATA, serverData);

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_NO_E911_ADDRESS_ON_FILE, DigitsConst.RESULT_SUCCEED, 0, output));

            return false;
        }

        mNoE911AddressOnFile = false;
        return true;
    }

    private boolean autoLogin() {

        UserProfile userProfile = mIamManager.getCurrentUser();
        boolean login = userProfile.getRefreshToken() == null ? false : true;
        Log.d(TAG, "autoLogin(), login: " + login);

        // Check if auto login
        if (login) {

            mSubscribeReq = false;

            Log.d(TAG, "call IamManager.refreshAccessToken()");
            // Call IAM API to refresh access token
            mIamManager.refreshAccessToken();
            return true;
        }
        return false;
    }

    private void startPushTokenTimer() {
        Log.d(TAG, "startPushTokenTimer()");

        // Start refresh timer
        int updateTime = DigitsConfig.getInstance().getGCMUpdateTokenTTL();
        Log.d(TAG, "push token updateTime: " + updateTime);

        mPushTokenScheduler.stop();
        mPushTokenScheduler.start(updateTime * 1000 * 9 / 10);
    }

    private void refreshPushToken() {

        String gcmToken = DigitsGcmHandler.getInstance(mContext).getGcmToken();

        Log.d(TAG, "refreshPushToken() gcmToken: " + gcmToken);

        // Call SES API to refresh push token
        mSesManager.refreshPushToken(gcmToken);
    }

    private void refreshPushTokenResult(int result, Bundle bundle) {
        Log.d(TAG, "refreshPushTokenResult(), result: " + result + ", bundle:" + bundle);

        // re-start refresh timer for this new token
        startPushTokenTimer();
    }

    private void refreshAccessTokenResult(int result, Bundle bundle) {
        Log.d(TAG, "refreshAccessTokenResult(), result: " + result + ", bundle:" + bundle);

        if (result != SesManager.EVENT_RESULT_SUCCEED) {

            if (inState(DigitsConst.STATE_IDLE)) {
                enterState(DigitsConst.STATE_READY);
            }

            if (mSubscribeReq) {
                // Notify caller
                mClientHandler.sendMessage(mClientHandler.obtainMessage(
                    DigitsServiceImpl.CMD_SUBSCRIBE_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0));
            }
            return;
        }

        // Call IAM API to get consumer profile
        Log.d(TAG, "call IamManager.getConsumerProfile()");
        mIamManager.getConsumerProfile();
    }

    private void getAccessTokenResult(int result, Bundle bundle) {
        Log.d(TAG, "getAccessTokenResult(), result: " + result + ", bundle:" + bundle);

        if (result != SesManager.EVENT_RESULT_SUCCEED) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_SUBSCRIBE_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0));
            return;
        }

        // Call IAM API to get consumer profile
        Log.d(TAG, "call IamManager.getConsumerProfile()");
        mIamManager.getConsumerProfile();
    }

    private void getConsumerProfileResult(int result, Bundle bundle) {
        Log.d(TAG, "getConsumerProfileResult(), result: " + result + ", bundle:" + bundle);

        if (result != IamManager.EVENT_RESULT_SUCCEED) {

            if (inState(DigitsConst.STATE_IDLE)) {
                enterState(DigitsConst.STATE_READY);
            }

            if (mSubscribeReq) {
                // Notify caller
                mClientHandler.sendMessage(mClientHandler.obtainMessage(
                    DigitsServiceImpl.CMD_SUBSCRIBE_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0));
            }
            return;
        }

        HashMap<String, UserProfile> profile = (HashMap<String, UserProfile>)
            bundle.getSerializable(IamManager.EVENT_KEY_CONSUMER_PROFILE);

        // Save consumer profile to storage
        mStorage.setConsumerProfile(profile);

        initSesSubscribe();
    }

    private void getMsisdnAuthListResult(int result, Bundle bundle) {
        Log.d(TAG, "getMsisdnAuthListResult(), result: " + result + ", bundle:" + bundle);

        if (result != IamManager.EVENT_RESULT_SUCCEED || !inState(DigitsConst.STATE_SUBSCRIBED)) {
            // Notify caller
            Bundle output = new Bundle();
            output.putString(DigitsConst.EXTRA_STRING_MSISDN, mAddLineReq);

            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_ADD_NEW_LINE_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0, output));

            mAddLineReq = null;
            return;
        }

        HashMap<String, MsisdnAuth> msisdnAuth = (HashMap<String, MsisdnAuth>)
            bundle.getSerializable(IamManager.EVENT_KEY_MSISDNLIST);

        // Init msisdn auth list to storage
        mStorage.initMsisdnAuthList(msisdnAuth);

        doInitMsisdnAuth();
    }

    private void doInitMsisdnAuth() {

        MsisdnAuth auth = mStorage.getNextMsisdnAuth(mAddLineReq);

        if (auth != null) {
            // Call IAM API to cancel the previous request
            Log.d(TAG, "call IamManager.cancelTransaction()");
            mIamManager.cancelTransaction(auth.getTransactionId(), mAddLineReq);
        } else {
            Log.d(TAG, "call IamManager.initMsisdnAuth()");
            mIamManager.initMsisdnAuth(mAddLineReq);
        }
    }

    private void initMsisdnAuthResult(int result, Bundle bundle) {
        Log.d(TAG, "initMsisdnAuthResult(), result: " + result + ", bundle:" + bundle +
            ", mState:" + stateToString(mState));

        if (result != IamManager.EVENT_RESULT_SUCCEED || !inState(DigitsConst.STATE_SUBSCRIBED)) {
            // Notify caller
            Bundle output = new Bundle();
            output.putString(DigitsConst.EXTRA_STRING_MSISDN, mAddLineReq);

            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_ADD_NEW_LINE_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0, output));

            mAddLineReq = null;
            return;
        }

        HashMap<String, MsisdnAuth> msisdnAuth = (HashMap<String, MsisdnAuth>)
                bundle.getSerializable(IamManager.EVENT_KEY_MSISDNLIST);

        Bundle output = new Bundle();
        output.putString(DigitsConst.EXTRA_STRING_MSISDN, mAddLineReq);

        mClientHandler.sendMessage(mClientHandler.obtainMessage(
            DigitsServiceImpl.CMD_ADD_NEW_LINE_RESULT, DigitsConst.RESULT_SUCCEED, 0, output));

        notifyRegisteredMsisdnChanged();
        mAddLineReq = null;
    }

    private void cancelTransactionResult(int result, Bundle bundle) {
        Log.d(TAG, "cancelTransactionResult(), result: " + result + ", bundle:" + bundle);

        if (result != IamManager.EVENT_RESULT_SUCCEED || !inState(DigitsConst.STATE_SUBSCRIBED)) {
            // Notify caller
            Bundle output = new Bundle();
            output.putString(DigitsConst.EXTRA_STRING_MSISDN, mAddLineReq);

            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_ADD_NEW_LINE_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0, output));

            mAddLineReq = null;
            return;
        }

        HashMap<String, MsisdnAuth> msisdnAuth = (HashMap<String, MsisdnAuth>)
            bundle.getSerializable(IamManager.EVENT_KEY_MSISDNLIST);

        for (Map.Entry<String, MsisdnAuth> entry : msisdnAuth.entrySet()) {

            String tid = entry.getKey();
            MsisdnAuth auth = entry.getValue();

            // Remove the msisdn auth record
            mStorage.removeMsisdnAuth(tid);

            doInitMsisdnAuth();

            // There is only one input
            break;
        }
    }

    private void initSesSubscribe() {

        AccessToken token = mIamManager.getAccessToken();

        String accessToken = token.getTokenType() + " " + token.getTokenValue();

        mSesManager.setAccessToken(accessToken);

        // Call SES API to subscribe
        Log.d(TAG, "call SesManager.initSubscribe(), accessToken:" + accessToken);
        mSesManager.initSubscribe();
    }

    private void deleteDeviceResult(int result) {
        Log.d(TAG, "deleteDeviceResult(), result: " + result + ", mState:" + stateToString(mState));

        // Call IAM API to signout
        Log.d(TAG, "call IamManager.logout()");
        mIamManager.logout();
    }

    private void signoutResult(int result) {
        Log.d(TAG, "signoutResult(), result: " + result + ", mState:" + stateToString(mState));

        // Clear cached data
        if (mLogoutReq) {
            Log.d(TAG, "call IamManager.clearCachedData()");
            mIamManager.clearCachedData();
            mSesManager.setAccessToken(null);

            onBoarding(true);
            mLogoutReq = false;
        }
    }

    private void doLogout(boolean logoutReq, boolean serverError) {
        Log.d(TAG, "doLogout(), logoutReq: " + logoutReq + ", mState:" + stateToString(mState));

        mLogoutReq = logoutReq;

        if (!inState(DigitsConst.STATE_SUBSCRIBED) && !serverError) {

            if (mLogoutReq) {
                // Notify caller
                mClientHandler.sendMessage(mClientHandler.obtainMessage(
                    DigitsServiceImpl.CMD_LOGOUT_RESULT, DigitsConst.RESULT_FAIL_WRONG_STATE, 0));
            }
            return;
        }

        enterState(DigitsConst.STATE_LOGOUT);

        setLogoutLineReq();

        if (mLineReqList.isEmpty()) {

            Log.d(TAG, "call SesManager.deleteDevice()");
            mSesManager.deleteDevice();

        } else {

            LineRequest req = mLineReqList.get(mCurLineRequestIndex);

            removeHuntGroup(req.mMsisdn);
        }

        /*
        1. remove hunt group for each virtual line
        2. manageConnectivity(operation: 2(delete))
        3. Signout
        4. 3gppAuthentication
           manageConnectivity (operation: 0(activate))
           managePushToken (service-name: conn-mgr, operation: 0(register))
           getMSISDN
           manageLocationAndTC
           manageService (operation: 0(provision))
           managePushToken (service-name: vowifi, operation: 0(register))
           manageService (operation: 5(instance-token))
        */
    }

    private void subscribeResult(int result, Bundle bundle) {
        Log.d(TAG, "subscribeResult(), result: " + result + ", bundle:" + bundle);

        if (result != SesManager.EVENT_RESULT_SUCCEED) {

            if (inState(DigitsConst.STATE_IDLE)) {
                enterState(DigitsConst.STATE_READY);
            }

            if (mSubscribeReq) {
                // Notify caller
                mClientHandler.sendMessage(mClientHandler.obtainMessage(
                    DigitsServiceImpl.CMD_SUBSCRIBE_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0));
            }
            return;
        }

        String nativeMsisdn = bundle.getString(SesManager.EVENT_KEY_MSISDN);

        setRegisteredDevice(bundle, true);

        // Save line list
        HashMap<String, SesManager.RegisteredMSISDN> msisdnInfo =
            (HashMap<String, SesManager.RegisteredMSISDN>)
            bundle.getSerializable(SesManager.EVENT_KEY_LINE_MAP);
        mStorage.initLineMap(msisdnInfo);


        // Get activated virtual line
        DigitsSharedPreference pref = DigitsSharedPreference.getInstance(mContext);
        Set<String> msisdns = pref.getActivatedVirtualLines();

        if (!msisdns.isEmpty()) {
            // Get line sit
            getVirtualLineSit(msisdns);
        } else {

            enterState(DigitsConst.STATE_SUBSCRIBED);

            setDigitsLine();

            // notify subscribed
            notifySubscribeResult();
        }

    }

    private void getVirtualLineSit(Set<String> msisdns) {
        Log.d(TAG, "getVirtualLineSit(), msisdns:" + msisdns);

        // Query SIT for activated virtual line
        String[] msisdnsArray = msisdns.toArray(new String[msisdns.size()]);

        if (!lineRequestCheck(msisdnsArray, LINE_REQUEST_REASON_SUBSCRIBE)) {

            if (inState(DigitsConst.STATE_IDLE)) {
                enterState(DigitsConst.STATE_READY);
            }
            return;
        }

        mHandler.sendMessage(mHandler.obtainMessage(CMD_UPDATE_MSISDN_SIT));
    }

    private void updateMsisdnSit() {
        LineRequest req = mLineReqList.get(mCurLineRequestIndex);

        Log.d(TAG, "updateMsisdnSit(), reqIdx:" + mCurLineRequestIndex + ", msisdn:" + req.mMsisdn);

        mSesManager.updateMsisdnSit(req.mMsisdn);
    }

    private void updateMsisdnSitResult(int result, Bundle bundle) {
        Log.d(TAG, "updateMsisdnSitResult(), result: " + result + ", bundle:" + bundle);

        String msisdn = bundle.getString(SesManager.EVENT_KEY_MSISDN);

        LineRequest req = mLineReqList.get(mCurLineRequestIndex);

        if (msisdn.equals(req.mMsisdn) && result == SesManager.EVENT_RESULT_SUCCEED) {

            // Store SIT
            String sit = bundle.getString(SesManager.EVENT_KEY_SIT);
            mStorage.setLineSit(msisdn, sit);

            setRegisteredDevice(bundle, false);

        } else {
            req.mResult = DigitsConst.RESULT_FAIL_UNKNOWN;
        }


        // Handle next line request
        mCurLineRequestIndex ++;

        if (mCurLineRequestIndex < mLineReqList.size()) {

            updateMsisdnSit();
        } else {

            Set<String> msisdns = getFailLineRequestList();

            // Clear line request
            mLineReqList.clear();
            mLineReqReason = LINE_REQUEST_REASON_NONE;

            enterState(DigitsConst.STATE_SUBSCRIBED);

            setDigitsLine();

            // notify subscribed
            notifySubscribeResult();

            if (!msisdns.isEmpty()) {
                // Try to restore the activated line in shared preference
                doActivateLine(msisdns.toArray(new String[msisdns.size()]));
            }
        }
    }

    private void renewSitResult(int result, Bundle bundle) {
        Log.d(TAG, "renewSitResult(), result:" + result + ", bundle:" + bundle +
            ", mState:" + stateToString(mState));

        if (!inState(DigitsConst.STATE_SUBSCRIBED)) {
            // Do nothing when logout
            return;
        }

        String msisdn = bundle.getString(SesManager.EVENT_KEY_MSISDN);
        String sit = bundle.getString(SesManager.EVENT_KEY_SIT);

        // Store line sit
        mStorage.setLineSit(msisdn, sit);

        setRegisteredDevice(bundle, false);

        // TODO: Add hunt group

        // Set new SIT to MD
        setDigitsLine();
    }

    private void provisionLine(String msisdn) {
        Log.d(TAG, "provisionLine(), msisdn: " + msisdn + ", mState:" + stateToString(mState));

        if (!inState(DigitsConst.STATE_SUBSCRIBED)) {
            // Do nothing when logout
            return;
        }

        // Call SES API to provision line
        mSesManager.provisionLine(msisdn);
    }

    private boolean hasSucceedLineRequest() {

        for (LineRequest req : mLineReqList) {
            if (req.mResult == DigitsConst.RESULT_SUCCEED) {
                return true;
            }
        }
        return false;
    }

    private Set<String> getFailLineRequestList() {

        Set<String> msisdns = new HashSet<String>();

        for (LineRequest req : mLineReqList) {
            if (req.mResult != DigitsConst.RESULT_SUCCEED) {
                msisdns.add(req.mMsisdn);
            }
        }

        return msisdns;
    }

    private void restoreWaitActivateAsApprovedLine() {

        for (LineRequest req : mLineReqList) {
            if (req.mResult != DigitsConst.RESULT_SUCCEED) {
                mStorage.setApprovedAuthLine(req.mMsisdn, false);
            }
        }
    }

    private void setDigitsLine() {

        Log.d(TAG, "setDigitsLine(), mState:" + stateToString(mState));

        // native msisdn is unknown
        if (inState(DigitsConst.STATE_IDLE)) {
            return;
        }

        ArrayList<String> rilMsisdns = mStorage.getNativeAndSitMsisdn();

        for (String msisdn : rilMsisdns) {

            boolean hasNext = (rilMsisdns.indexOf(msisdn) < rilMsisdns.size() - 1) ? true : false;

            boolean isNative = mStorage.getIsNative(msisdn);

            boolean isLogout = inState(DigitsConst.STATE_SUBSCRIBED) ? false : true;

            String sit = mStorage.getLineSit(msisdn);
            sit = (sit == null) ? "" : sit;

            mRil.setDigitsLine(getMainCapabilityPhoneId(), mRilSerial, isLogout,
                hasNext, isNative, msisdn, sit,
                mHandler.obtainMessage(CMD_RIL_SET_DIGITS_LINE_RESPONSE));
        }

        // increase for the next request
        mRilSerial ++;

        // Sync to shared preference and storage
        mStorage.setSitLineActivated();

        notifyRegisteredMsisdnChanged();
    }

    private void setDigitsLineResponse(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        if (ar != null && ar.exception != null) {
            Log.d(TAG, "setDigitsLineResponse(), exception:" + ar.exception);
        }
    }

    private void onDigitsLineIndication(Message msg) {

        AsyncResult ar = (AsyncResult) msg.obj;
        String[] result = (String[]) ar.result;

        int simId = Integer.parseInt(result[0]);
        int serial = Integer.parseInt(result[1]);
        int msisdnNum = Integer.parseInt(result[2]);

        Log.d(TAG, "onDigitsLineIndication(), serial:" + serial + ", msisdnNum:" + msisdnNum +
            ", mState:" + stateToString(mState));


        int msisdnIdx = 3;
        int activeIdx = 13;

        // Collect RIL URC activated line
        for (int count = 0; count < msisdnNum; count ++) {
            boolean registered = (Integer.parseInt(result[activeIdx + count]) == 0) ? false : true;

            String msisdn = result[msisdnIdx + count];
            mStorage.updateRegisteredStatus(msisdn, registered);
        }
        notifyRegisteredMsisdnChanged();
    }

    public void onGetTrn(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        String[] result = (String[]) ar.result;

        String fromMsisdn = result[0];
        String toMsisdn = result[1];

        Log.d(TAG, "onGetTrn(), from: " + fromMsisdn + ", to: " + toMsisdn +
            ", mState:" + stateToString(mState));

        if (mGetTrnReq != null || !inState(DigitsConst.STATE_SUBSCRIBED)) {

            Log.e(TAG, "setTrn(), busy or wrong state !");

            mRil.setTrn(fromMsisdn, toMsisdn, "", mHandler.obtainMessage(CMD_RIL_SET_TRN_RESPONSE));
            return;
        }

        // Save request
        mGetTrnReq = new GetTrnReq();
        mGetTrnReq.mFrom = fromMsisdn;
        mGetTrnReq.mTo = toMsisdn;

        String nativeMsisdn = mStorage.getNativeMsisdn();

        String sit = mStorage.getLineSit(fromMsisdn);

        // Call WSG API to get trn
        Log.d(TAG, "call WsgManager.getTrn(), fromMsisdn:" + fromMsisdn + ", toMsisdn:" + toMsisdn
            + ", nativeMsisdn:" + nativeMsisdn + ", sit:" + sit);
        mWsgManager.getTrn(fromMsisdn, toMsisdn, nativeMsisdn, sit);
    }

    private void getTrnResult(int result, Bundle bundle) {
        Log.d(TAG, "getTrnResult(), result:" + result + ", bundle:" + bundle);

        String from = bundle.getString(WsgManager.EVENT_KEY_FROM);
        String to = bundle.getString(WsgManager.EVENT_KEY_TO);
        String trn = bundle.getString(WsgManager.EVENT_KEY_TRN);

        Message response = mHandler.obtainMessage(CMD_RIL_SET_TRN_RESPONSE);

        if (!from.equals(mGetTrnReq.mFrom) || !to.equals(mGetTrnReq.mTo) ||
            result != WsgManager.EVENT_RESULT_SUCCEED) {

            Log.e(TAG, "setTrn(), result is fail");


            mRil.setTrn(mGetTrnReq.mFrom, mGetTrnReq.mTo, "", response);

            mGetTrnReq = null;
            return;
        }

        mRil.setTrn(mGetTrnReq.mFrom, mGetTrnReq.mTo, trn, response);
        mGetTrnReq = null;
    }

    private void notifySubscribeResult() {
        Log.d(TAG, "notifySubscribeResult(), mSubscribeReq:" + mSubscribeReq);

        if (mSubscribeReq) {
            Bundle output = new Bundle();

            DigitsProfile profile = mStorage.getConsumerProfile();
            output.putParcelable(DigitsConst.EXTRA_OBJECT_DIGITS_PROFILE, profile);

            DigitsDevice[] deviceList = mStorage.getRegisteredDevice();
            output.putParcelableArray(DigitsConst.EXTRA_OBJECT_ARRAY_DIGITS_DEVICE, deviceList);

            DigitsLine[] lineList = mStorage.getRegisteredLine(false);
            output.putParcelableArray(DigitsConst.EXTRA_OBJECT_ARRAY_DIGITS_LINE, lineList);

            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_SUBSCRIBE_RESULT, DigitsConst.RESULT_SUCCEED, 0, output));
        }

        notifyRegisteredDevicesChanged();
        notifyRegisteredMsisdnChanged();
    }

    private void callbackLineStatusResult(boolean succeed) {
        Log.d(TAG, "callbackLineStatusResult(), succeed:" + succeed);

        Bundle output = new Bundle();

        int event = lineReqReasonToMsg(mLineReqReason);
        int resResult = succeed ? DigitsConst.RESULT_SUCCEED : DigitsConst.RESULT_FAIL_UNKNOWN;

        if (succeed) {
            DigitsLine[] lineList = mStorage.getRegisteredLine(false);
            output.putParcelableArray(DigitsConst.EXTRA_OBJECT_ARRAY_DIGITS_LINE, lineList);
        }

        mClientHandler.sendMessage(mClientHandler.obtainMessage(
            event, resResult, 0, output));


        // clear request
        mLineReqList.clear();
        mLineReqReason = LINE_REQUEST_REASON_NONE;

        // Check if there is pending line
        handlePendingMsisdn();
    }

    private int lineReqReasonToMsg(int reason) {
        if (reason == LINE_REQUEST_REASON_SUBSCRIBE) {
            return DigitsServiceImpl.CMD_SUBSCRIBE_RESULT;
        } else if (reason == LINE_REQUEST_REASON_ACTIVATE_LINE) {
            return DigitsServiceImpl.CMD_ACTIVATE_LINE_RESULT;
        } else if (reason == LINE_REQUEST_REASON_DEACTIVATE_LINE) {
            return DigitsServiceImpl.CMD_DEACTIVATE_LINE_RESULT;
        }

        Log.e(TAG, "lineReqReasonToMsg(), wrong reason:" + reason);
        return -1;
    }

    private void removeNextHuntGroup() {
        Log.d(TAG, "removeNextHuntGroup(), idx:" + mCurLineRequestIndex + ", size:" +
            mLineReqList.size() + ", mState:" + stateToString(mState));

        if (mCurLineRequestIndex + 1 < mLineReqList.size()) {

            // Handle next line request
            mCurLineRequestIndex ++;

            LineRequest nextReq = mLineReqList.get(mCurLineRequestIndex);

            removeHuntGroup(nextReq.mMsisdn);

        } else {

            // Clear request
            mLineReqList.clear();
            mLineReqReason = LINE_REQUEST_REASON_NONE;

            // Clear line sit
            ArrayList<String> msisdns = mStorage.getNativeAndSitMsisdn();
            for (String msisdn : msisdns) {
                mStorage.setLineSit(msisdn, null);
            }

            // Call SES API to delete device
            Log.d(TAG, "call SesManager.deleteDevice()");
            mSesManager.deleteDevice();
        }

    }

    private void handleLineRequest() {
        Log.d(TAG, "handleLineRequest(), idx:" + mCurLineRequestIndex + ", size:" +
            mLineReqList.size() + ", mState:" + stateToString(mState));

        if (mCurLineRequestIndex + 1 < mLineReqList.size()) {

            // Handle next line request
            mCurLineRequestIndex ++;

            LineRequest nextReq = mLineReqList.get(mCurLineRequestIndex);

            if (mLineReqReason == LINE_REQUEST_REASON_ACTIVATE_LINE) {

                provisionLine(nextReq.mMsisdn);

            } else if (mLineReqReason == LINE_REQUEST_REASON_DEACTIVATE_LINE) {

                removeHuntGroup(nextReq.mMsisdn);

            } else {
                Log.e(TAG, "handleLineRequest(), Wrong mLineReqReason:" + mLineReqReason);
            }

        } else {

            // Do before line request is cleared (callbackLineStatusResult)
            if (mLineReqReason == LINE_REQUEST_REASON_ACTIVATE_LINE) {
                restoreWaitActivateAsApprovedLine();
            }

            // Go to SIP state or callback
            if (hasSucceedLineRequest()) {

                setDigitsLine();

                callbackLineStatusResult(true);

            } else {
                callbackLineStatusResult(false);
            }
        }
    }

    private void doActivateLine(String[] msisdns) {
        Log.d(TAG, "doActivateLine(), msisdns:" + Arrays.toString(msisdns) +
            ", mState:" + stateToString(mState));

        if (!lineRequestCheck(msisdns, LINE_REQUEST_REASON_ACTIVATE_LINE)) {

            return;
        } else if (!inState(DigitsConst.STATE_SUBSCRIBED)) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_ACTIVATE_LINE_RESULT,
                DigitsConst.RESULT_FAIL_WRONG_STATE, 0, null));

            return;
        }

        LineRequest req = mLineReqList.get(mCurLineRequestIndex);

        provisionLine(req.mMsisdn);
    }

    private void doDeactivateLine(String[] msisdns) {
        Log.d(TAG, "doDeactivateLine(), msisdns:" + Arrays.toString(msisdns) +
            ", mState:" + stateToString(mState));

        if (!lineRequestCheck(msisdns, LINE_REQUEST_REASON_DEACTIVATE_LINE)) {

            return;
        } else if (!inState(DigitsConst.STATE_SUBSCRIBED)) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_DEACTIVATE_LINE_RESULT,
                DigitsConst.RESULT_FAIL_WRONG_STATE, 0, null));

            return;
        }

        LineRequest req = mLineReqList.get(mCurLineRequestIndex);

        removeHuntGroup(req.mMsisdn);

    }

    private void provisionLineResult(int result, Bundle bundle) {
        Log.d(TAG, "provisionLineResult(), result:" + result + ", bundle:" + bundle +
            ", mState:" + stateToString(mState));

        if (!inState(DigitsConst.STATE_SUBSCRIBED)) {
            // Do nothing when logout
            return;
        }

        String msisdn = bundle.getString(SesManager.EVENT_KEY_MSISDN);

        LineRequest req = mLineReqList.get(mCurLineRequestIndex);

        if (!msisdn.equals(req.mMsisdn) || result != SesManager.EVENT_RESULT_SUCCEED) {

            req.mResult = DigitsConst.RESULT_FAIL_UNKNOWN;

            handleLineRequest();
            return;
        }

        // Store SIT
        String sit = bundle.getString(SesManager.EVENT_KEY_SIT);
        mStorage.setLineSit(msisdn, sit);

        // Sometimes GCM will receive EVENT_NEW_LINE_INSTANCE_ACTIVATION but sometimes not
        setRegisteredDevice(bundle, false);

        mHandler.sendMessage(mHandler.obtainMessage(CMD_ADD_HUNT_GROUP));

    }

    private void refreshDeviceInfo(boolean simSwap) {
        Log.d(TAG, "refreshDeviceInfo(), simSwap:" + simSwap + ", state:" + stateToString(mState));

        if (simSwap) {

            if (inState(DigitsConst.STATE_SUBSCRIBED)) {

                mHandler.sendMessage(mHandler.obtainMessage(CMD_USER_LOGOUT, 0, 0));

            } else if (inState(DigitsConst.STATE_READY)){

                // Enter IDLE to wait onBoarding after device-config is retrived for the new SIM
                setJanskyService(DigitsConst.JANSKY_SERVICE_UNKNOWN);
                enterState(DigitsConst.STATE_IDLE);
            } else if (inState(DigitsConst.STATE_IDLE)) {

                // Clear cached data
                Log.d(TAG, "call IamManager.clearCachedData()");
                mIamManager.clearCachedData();
            }

        } else {

            // Refresh info when user inserts the same SIM
            if (inState(DigitsConst.STATE_SUBSCRIBED)) {

                registeredDevice(false);
                registeredMsisdn();

                Log.d(TAG, "call SesManager.getMsisdn()");
                mSesManager.getMsisdn();
            } else if (inState(DigitsConst.STATE_READY)){

                Log.d(TAG, "call SesManager.getMsisdn()");
                mSesManager.getMsisdn();
            }
        }
    }

    private void onSimLoaded(boolean loaded) {
        Log.d(TAG, "onSimLoaded(), loaded:" + loaded + ", mSimLoaded:" + mSimLoaded);

        if (mSimLoaded != loaded) {
            mSimLoaded = loaded;

            // Notify app to query again
            notifyRegisteredMsisdnChanged();
        }
    }

    private void registeredMsisdn() {
        Log.d(TAG, "registeredMsisdn(), mRegisteredMsisdnRunning:" + mRegisteredMsisdnRunning);

        // avoid too frequently update
        if (mRegisteredMsisdnRunning) {
            return;
        }

        mSesManager.registeredMsisdn();

        mRegisteredMsisdnRunning = true;
    }


    private void registeredMsisdnResult(int result, Bundle bundle) {
        Log.d(TAG, "registeredMsisdnResult(), result:" + result + ", bundle:" + bundle);

        mRegisteredMsisdnRunning = false;

        if (result != SesManager.EVENT_RESULT_SUCCEED) {

            return;
        }

        // Save line list
        HashMap<String, SesManager.RegisteredMSISDN> msisdnInfo =
            (HashMap<String, SesManager.RegisteredMSISDN>)
            bundle.getSerializable(SesManager.EVENT_KEY_LINE_MAP);

        mStorage.updateLineMap(msisdnInfo);

        notifyRegisteredMsisdnChanged();

        // Check if there is pending line
        handlePendingMsisdn();

    }

    private void handlePendingMsisdn() {

        // deactivate pending msisdn
        validateAuthRemovedList();

        if (!mAuthRemovedMsisdns.isEmpty()) {

            Log.d(TAG, "handlePendingMsisdn(), wait to deactivate msisdns:" + mAuthRemovedMsisdns);

            if (mLineReqList.isEmpty()) {

                doDeactivateLine(mAuthRemovedMsisdns.toArray(new String[mAuthRemovedMsisdns.size()]));

                clearAuthRemovedList();
            }
            return;
        }

        // activate pending msisdn
        ArrayList<String> msisdns = mStorage.getWaitForActivateMsisdn();
        if (!msisdns.isEmpty()) {

            Log.d(TAG, "handlePendingMsisdn(), wait to activate msisdns:" + msisdns);

            if (mLineReqList.isEmpty()) {
                doActivateLine(msisdns.toArray(new String[msisdns.size()]));
            }
            return;
        }
    }

    private void registeredDevice(boolean periodical) {
        Log.d(TAG, "registeredDevice(), periodical:" + periodical +
            ", mRegisteredDeviceRunning:" + mRegisteredDeviceRunning);

        // avoid too frequently update
        if (mRegisteredDeviceRunning) {
            return;
        }

        mSesManager.registeredDevices();

        mRegisteredDeviceRunning = true;

        if (!mPeriodicRegisteredDevice && periodical) {
            mPeriodicRegisteredDevice = true;
        }

    }

    private void registeredDevicesResult(int result, Bundle bundle) {
        Log.d(TAG, "registeredDevicesResult(), result:" + result + ", bundle:" + bundle +
            ", periodic:" + mPeriodicRegisteredDevice);

        mRegisteredDeviceRunning = false;

        if (mPeriodicRegisteredDevice) {

            mRegisteredDeviceScheduler.stop();
            mRegisteredDeviceScheduler.start(PERIODICAL_REGISTERED_DEVICE_MILLIS);
        }

        if (result != SesManager.EVENT_RESULT_SUCCEED) {

            return;
        }

        setRegisteredDevice(bundle, false);
    }

    private void getMsisdnResult(int result, Bundle bundle) {
        Log.d(TAG, "getMsisdnResult(), result:" + result + ", bundle:" + bundle);

        if (result != SesManager.EVENT_RESULT_SUCCEED) {

            return;
        }

        String msisdn = bundle.getString(SesManager.EVENT_KEY_MSISDN);

        String nativeMsisdn = mStorage.getNativeMsisdn();

        if (!nativeMsisdn.equals(msisdn)) {
            Log.d(TAG, "check why different");
        }
    }

    private void addHuntGroup() {
        Log.d(TAG, "addHuntGroup(), mState:" + stateToString(mState));

        if (!inState(DigitsConst.STATE_SUBSCRIBED)) {
            // Do nothing when logout
            return;
        }

        LineRequest req = mLineReqList.get(mCurLineRequestIndex);

        String nativeMsisdn = mStorage.getNativeMsisdn();
        String sit = mStorage.getLineSit(req.mMsisdn);

        // Call WSG API to add hunt group
        Log.d(TAG, "call IamManager.addHuntGroup(), msisdn:" + req.mMsisdn +
            ", nativeMsisdn:" + nativeMsisdn + ", sit:" + sit);

        mWsgManager.addHuntGroup(req.mMsisdn, nativeMsisdn, sit);
    }

    private void addHuntGroupResult(int result, Bundle bundle) {
        Log.d(TAG, "addHuntGroupResult(), result:" + result + ", bundle:" + bundle +
            ", mState:" + stateToString(mState));

        if (!inState(DigitsConst.STATE_SUBSCRIBED)) {
            // Do nothing when logout
            return;
        }

        LineRequest req = mLineReqList.get(mCurLineRequestIndex);

        String msisdn = bundle.getString(WsgManager.EVENT_KEY_MSISDN);

        if (!msisdn.equals(req.mMsisdn) || result != WsgManager.EVENT_RESULT_SUCCEED) {

            req.mResult = DigitsConst.RESULT_FAIL_UNKNOWN;
        }

        handleLineRequest();
    }

    private void removeHuntGroup(String msisdn) {
        Log.d(TAG, "removeHuntGroup(), msisdn:" + msisdn);

        String nativeMsisdn = mStorage.getNativeMsisdn();
        String sit = mStorage.getLineSit(msisdn);

        // Call WSG API to remove hunt group
        Log.d(TAG, "call IamManager.removeHuntGroup(), msisdn:" + msisdn +
            ", nativeMsisdn:" + nativeMsisdn + ", sit:" + sit);
        mWsgManager.removeHuntGroup(msisdn, nativeMsisdn, sit);
    }

    private void removeHuntGroupResult(int result, Bundle bundle) {
        Log.d(TAG, "removeHuntGroupResult(), result:" + result + ", bundle:" + bundle);

        if (mLineReqReason == LINE_REQUEST_REASON_LOGOUT) {

            // Don't care the result, just go ahead
            removeNextHuntGroup();
            return;
        }

        LineRequest req = mLineReqList.get(mCurLineRequestIndex);

        String msisdn = bundle.getString(WsgManager.EVENT_KEY_MSISDN);

        if (!msisdn.equals(req.mMsisdn) || result != WsgManager.EVENT_RESULT_SUCCEED) {

            req.mResult = DigitsConst.RESULT_FAIL_UNKNOWN;

            handleLineRequest();
            return;
        }

        mHandler.sendMessage(mHandler.obtainMessage(CMD_REMOVE_LINE));
    }

    private void removeLine() {

        if (!inState(DigitsConst.STATE_SUBSCRIBED)) {
            // Do nothing when logout
            return;
        }

        LineRequest req = mLineReqList.get(mCurLineRequestIndex);

        // Call SES API to remove line
        Log.d(TAG, "removeLine(), msisdn:" + req.mMsisdn);
        mSesManager.removeLine(req.mMsisdn);
    }

    private void removeLineResult(int result, Bundle bundle) {
        Log.d(TAG, "removeLineResult(), result:" + result + ", bundle:" + bundle +
            ", mState:" + stateToString(mState));

        if (!inState(DigitsConst.STATE_SUBSCRIBED)) {
            // Do nothing when logout
            return;
        }

        String msisdn = bundle.getString(SesManager.EVENT_KEY_MSISDN);

        LineRequest req = mLineReqList.get(mCurLineRequestIndex);

        if (!msisdn.equals(req.mMsisdn) || result != SesManager.EVENT_RESULT_SUCCEED) {

            req.mResult = DigitsConst.RESULT_FAIL_UNKNOWN;
        }

        // Clear SIT
        mStorage.setLineSit(msisdn, null);

        handleLineRequest();
    }

    private void doGetDeviceConfig(boolean getDeviceConfigReq) {

        mGetDeviceConfigReq = getDeviceConfigReq;

        String deviceId = DigitsUtil.getInstance(mContext).getDeviceId();
        if (deviceId == null) {
            Log.d(TAG, "no imei. 3gppAuthentication needs Device id (IMEI)");
            return;
        }

        // Call SES API to get device config
        mSesManager.getDeviceConfig();
    }

    private void setDeviceNameResult(int result, Bundle bundle) {
        Log.d(TAG, "setDeviceNameResult(), result:" + result + ", bundle:" + bundle);

        String deviceName = bundle.getString(SesManager.EVENT_KEY_NAME);

        int res;

        if (result != SesManager.EVENT_RESULT_SUCCEED || !deviceName.equals(mDeviceNameReq)) {
            res = DigitsConst.RESULT_FAIL_UNKNOWN;
        } else {
            res = DigitsConst.RESULT_SUCCEED;

            // Store the device name
            mStorage.setDeviceName(mDeviceNameReq);
        }

        Bundle output = new Bundle();
        output.putString(DigitsConst.EXTRA_STRING_DEVICE_NAME, mDeviceNameReq);

        // Notify caller
        mClientHandler.sendMessage(mClientHandler.obtainMessage(
            DigitsServiceImpl.CMD_SET_DEVICE_NAME_RESULT, res, 0, output));

        mDeviceNameReq = null;

        // Notify registered device changed
        notifyRegisteredDevicesChanged();
    }

    private void setLineNameResult(int result, Bundle bundle) {
        Log.d(TAG, "setLineNameResult(), result:" + result + ", bundle:" + bundle +
            ", mState:" + stateToString(mState));

        String msisdn = bundle.getString(SesManager.EVENT_KEY_MSISDN);
        String lineName = bundle.getString(SesManager.EVENT_KEY_NAME);

        int res;

        if (result != SesManager.EVENT_RESULT_SUCCEED || !msisdn.equals(mLineNameReq.mMsisdn) ||
            !lineName.equals(mLineNameReq.mName) || !inState(DigitsConst.STATE_SUBSCRIBED)) {

            res = DigitsConst.RESULT_FAIL_UNKNOWN;
        } else {
            res = DigitsConst.RESULT_SUCCEED;

            // Store the line name
            mStorage.setLineName(msisdn, lineName);

        }

        Bundle output = new Bundle();
        output.putString(DigitsConst.EXTRA_STRING_MSISDN, mLineNameReq.mMsisdn);
        output.putString(DigitsConst.EXTRA_STRING_LINE_NAME, mLineNameReq.mName);

        mLineNameReq = null;

        // Notify caller
        mClientHandler.sendMessage(mClientHandler.obtainMessage(
            DigitsServiceImpl.CMD_SET_LINE_NAME_RESULT, res, 0, output));

        // Notify registered msisdn changed
        notifyRegisteredMsisdnChanged();
    }

    private void lineNameUpdated(Bundle bundle) {
        Log.d(TAG, "lineNameUpdated(), bundle:" + bundle);

        String msisdn = bundle.getString(GcmManager.EVENT_KEY_MSISDN);
        String lineName = bundle.getString(GcmManager.EVENT_KEY_NAME);

        mStorage.setLineName(msisdn, lineName);

        // Notify registered msisdn changed
        notifyRegisteredMsisdnChanged();
    }

    private void callAnswered(Bundle bundle) {

        Log.d(TAG, "callAnswered(), bundle:" + bundle);

        String imei = bundle.getString(GcmManager.EVENT_KEY_IMEI);


        Bundle output = new Bundle();
        output.putString(DigitsConst.EXTRA_STRING_DEVICE_ID, imei);

        // Notify caller
        mClientHandler.sendMessage(mClientHandler.obtainMessage(
            DigitsServiceImpl.CMD_CALL_ANSWERED, DigitsConst.RESULT_SUCCEED, 0, output));
    }

    private void msisdnAuthChanged(Bundle bundle) {

        Log.d(TAG, "msisdnAuthChanged(), bundle:" + bundle);

        String msisdn = bundle.getString(GcmManager.EVENT_KEY_MSISDN);
        String changeType = bundle.getString(GcmManager.EVENT_KEY_CHANGE_TYPE);

        if (msisdn.length() == 10) {
            msisdn = "1" + msisdn;
        }

        boolean isNative = mStorage.getIsNative(msisdn);

        if (!isNative) {

            if (changeType.equals("GRANT")) {
                mStorage.setApprovedAuthLine(msisdn, true);

                // Update approved line attribute
                registeredMsisdn();
            } else if (changeType.equals("REVOKE")) {

                registeredMsisdn();
                registeredDevice(false);
            }
        }
    }

    private void msisdnAuthRemoved(Bundle bundle) {

        Log.d(TAG, "msisdnAuthRemoved(), bundle:" + bundle);

        String msisdn = bundle.getString(GcmManager.EVENT_KEY_MSISDN);
        String deviceId = bundle.getString(GcmManager.EVENT_KEY_DEVICE_ID);

        if (msisdn.length() == 10) {
            msisdn = "1" + msisdn;
        }

        boolean myDevice = mStorage.getIsMyDevice(deviceId);
        boolean isNative = mStorage.getIsNative(msisdn);

        if (myDevice && !isNative) {

            if (mStorage.getLineSit(msisdn) != null) {
                addToAuthRemovedList(msisdn);
            }

            // Run registeredmsisdn API
            registeredMsisdn();
        }

        // Update registeredDevice
        registeredDevice(false);
    }

    private void addToAuthRemovedList(String msisdn) {

        if (!mAuthRemovedMsisdns.contains(msisdn)) {

            Log.d(TAG, "addToAuthRemovedList(), msisdn:" + msisdn);

            mAuthRemovedMsisdns.add(msisdn);
        }
    }

    private void validateAuthRemovedList() {

        for (String msisdn : new ArrayList<String>(mAuthRemovedMsisdns)) {

            if (mStorage.getLineSit(msisdn) == null) {

                Log.d(TAG, "validateAuthRemovedList(), remove msisdn:" + msisdn);
                mAuthRemovedMsisdns.remove(msisdn);
            }
        }
    }

    private void clearAuthRemovedList() {
        Log.d(TAG, "clearAuthRemovedList()");

        mAuthRemovedMsisdns.clear();
    }

    private void notifyRegisteredMsisdnChanged() {

        DigitsLine[] digitsLine = getRegisteredLine();

        if (mStorage.isRegisteredLineUpdated(digitsLine, mRegisteredLineLastTime)) {

            Log.d(TAG, "notifyRegisteredMsisdnChanged()");

            mRegisteredLineLastTime = digitsLine;

            Bundle output = new Bundle();

            output.putParcelableArray(
                DigitsConst.EXTRA_OBJECT_ARRAY_DIGITS_LINE, digitsLine);

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_REGISTERED_MSISDN_CHANGED,
                DigitsConst.RESULT_SUCCEED, 0, output));
        }
    }

    private void setRegisteredDevice(Bundle bundle, boolean subscribe) {
        Log.d(TAG, "setRegisteredDevice(), subscribe:" + subscribe);
        // Save device list
        HashMap<String, SesManager.DeviceInfo> deviceInfo = (HashMap<String, SesManager.DeviceInfo>)
            bundle.getSerializable(SesManager.EVENT_KEY_DEVICE_MAP);
        mStorage.initDeviceMap(deviceInfo);

        if (!subscribe) {
            // Notify registered device changed
            notifyRegisteredDevicesChanged();
        }
    }

    private void notifyRegisteredDevicesChanged() {
        Log.d(TAG, "notifyRegisteredDevicesChanged()");

        Bundle output = new Bundle();

        output.putParcelableArray(
            DigitsConst.EXTRA_OBJECT_ARRAY_DIGITS_DEVICE, getRegisteredDevice());

        // Notify caller
        mClientHandler.sendMessage(mClientHandler.obtainMessage(
            DigitsServiceImpl.CMD_REGISTERED_DEVICES_CHANGED, DigitsConst.RESULT_SUCCEED, 0, output));
    }

    private void setLogoutLineReq() {

        // Callback failure
        if (!mLineReqList.isEmpty()) {
            callbackLineStatusResult(false);
        }

        ArrayList<String> msisdns = mStorage.getNativeAndSitMsisdn();
        msisdns.remove(mStorage.getNativeMsisdn());

        // Save request
        for (String msisdn : msisdns) {
            LineRequest req = new LineRequest();
            req.mMsisdn = msisdn;
            req.mResult = DigitsConst.RESULT_SUCCEED;
            mLineReqList.add(req);
        }
        mCurLineRequestIndex = 0;

        // Assign line request reason
        mLineReqReason = mLineReqList.isEmpty() ?
            LINE_REQUEST_REASON_NONE : LINE_REQUEST_REASON_LOGOUT;
    }

    private boolean lineRequestCheck(String[] msisdns, int reason) {

        if (!mLineReqList.isEmpty()) {
            // Notify caller
            Bundle output = new Bundle();
            output.putStringArray(DigitsConst.EXTRA_STRING_ARRAY_MSISDN, msisdns);
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                lineReqReasonToMsg(reason), DigitsConst.RESULT_FAIL_BUSY, 0, output));

            Log.d(TAG, "lineRequestCheck(), line request list isn't empty !");
            return false;
        }

        // Save request
        for (String msisdn : msisdns) {
            LineRequest req = new LineRequest();
            req.mMsisdn = msisdn;
            req.mResult = DigitsConst.RESULT_SUCCEED;
            mLineReqList.add(req);
        }

        mCurLineRequestIndex = 0;

        // Assign line request reason
        mLineReqReason = reason;

        return true;
    }

    private void setJanskyService(int jansky) {
        Log.d(TAG, "setJanskyService(), from:" + janskyToString(mJanskyService) +
            ", to:" + janskyToString(jansky));

        mJanskyService = jansky;
    }


    private void enterState(int state) {
        Log.d(TAG, "enterState(), from:" + stateToString(mState) + ", to:" + stateToString(state));

        if (state == mState) {
            return;
        }

        mState = state;

        // Enable & disable GCM event
        mCtrlListener.enableGcmEvent(mState == DigitsConst.STATE_SUBSCRIBED ? true : false);

        Bundle output = new Bundle();
        output.putInt(DigitsConst.EXTRA_INT_SERVICE_STATE, mState);
        output.putInt(DigitsConst.EXTRA_INT_JANSKY_SERVICE, mJanskyService);

        // Notify caller
        mClientHandler.sendMessage(mClientHandler.obtainMessage(
            DigitsServiceImpl.CMD_SERVICE_STATE_CHANGED, DigitsConst.RESULT_SUCCEED, 0, output));

    }

    private boolean inState(int state) {
        return (state == mState) ? true : false;
    }

    private String janskyToString(int jansky) {
        switch (jansky) {
            case DigitsConst.JANSKY_SERVICE_UNKNOWN:
                return "UNKNOWN";
            case DigitsConst.JANSKY_SERVICE_OFF:
                return "OFF";
            case DigitsConst.JANSKY_SERVICE_ON:
                return "ON";
            default:
                return "Wrong jansky:" + jansky;

        }
    }

    private String stateToString(int state) {
        switch (state) {
            case DigitsConst.STATE_IDLE:
                return "STATE_IDLE";
            case DigitsConst.STATE_READY:
                return "STATE_READY";
            case DigitsConst.STATE_SUBSCRIBED:
                return "STATE_SUBSCRIBED";
            case DigitsConst.STATE_LOGOUT:
                return "STATE_LOGOUT";
            default:
                return "Wrong State:" + state;

        }
    }

    private void initHandler() {
        /** Handler to handle internal message command, run on controller thread */
         mHandler = new Handler(mCtrlHandlerThread.getLooper()) {

            @Override
            public void handleMessage(Message msg) {
                Log.d(TAG, "handleMessage: " + messageToString(msg));

                switch (msg.what) {
                    case CMD_GET_DEVICE_CONFIG:
                        doGetDeviceConfig(msg.arg1 == 0 ? false : true);
                        break;

                    case CMD_GET_DEVICE_CONFIG_RESULT:
                        getDeviceConfigResult(msg.arg1);
                        break;

                    case CMD_ON_BOARDING:
                        onBoarding(msg.arg1 == 0 ? false : true);
                        break;

                    case CMD_ON_BOARDING_RESULT:
                        onBoardingResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_GET_ACCESS_TOKEN_RESULT:
                        getAccessTokenResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_REFRESH_ACCESS_TOKEN_RESULT:
                        refreshAccessTokenResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_GET_CONSUMER_PROFILE_RESULT:
                        getConsumerProfileResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_GET_MSISDN_AUTH_LIST_RESULT:
                        getMsisdnAuthListResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_CANCEL_TRANSACTION_RESULT:
                        cancelTransactionResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_SUBSCRIBE_RESULT:
                        subscribeResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_UPDATE_MSISDN_SIT:
                        updateMsisdnSit();
                        break;

                    case CMD_UPDATE_MSISDN_SIT_RESULT:
                        updateMsisdnSitResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_SIT_RENEW:
                        renewSitResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_INIT_MSISDN_AUTH_RESULT:
                        initMsisdnAuthResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_ACTIVATE_LINE:
                        doActivateLine((String [])msg.obj);
                        break;

                    case CMD_DEACTIVATE_LINE:
                        doDeactivateLine((String [])msg.obj);
                        break;

                    case CMD_PROVISION_LINE_RESULT:
                        provisionLineResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_ADD_HUNT_GROUP:
                        addHuntGroup();
                        break;

                    case CMD_ADD_HUNT_GROUP_RESULT:
                        addHuntGroupResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_REMOVE_HUNT_GROUP_RESULT:
                        removeHuntGroupResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_REMOVE_LINE:
                        removeLine();
                        break;

                    case CMD_REMOVE_LINE_RESULT:
                        removeLineResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_REGISTERED_DEVICES:
                        registeredDevice(msg.arg1 == 0 ? false : true);
                        break;

                    case CMD_REGISTERED_DEVICES_RESULT:
                        registeredDevicesResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_REGISTERED_MSISDN:
                        registeredMsisdn();
                        break;

                    case CMD_REGISTERED_MSISDN_RESULT:
                        registeredMsisdnResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_REFRESH_DEVICE_INFO:
                        refreshDeviceInfo(msg.arg1 == 0 ? false : true);
                        break;

                    case CMD_SIM_LOADED_EVENT:
                        onSimLoaded(msg.arg1 == 0 ? false : true);
                        break;

                    case CMD_RIL_SET_DIGITS_LINE:
                        setDigitsLine();
                        break;

                    case CMD_RIL_SET_DIGITS_LINE_RESPONSE:
                        setDigitsLineResponse(msg);
                        break;

                    case CMD_RIL_DIGITS_LINE_INDICATION:
                        onDigitsLineIndication(msg);
                        break;

                    case CMD_RIL_GET_TRN_INDICATION:
                        onGetTrn(msg);
                        break;

                    case CMD_GET_TRN_RESULT:
                        getTrnResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_SET_DEVICE_NAME_RESULT:
                        setDeviceNameResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_SET_LINE_NAME_RESULT:
                        setLineNameResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_DELETE_DEVICE_RESULT:
                        deleteDeviceResult(msg.arg1);
                        break;

                    case CMD_REFRESH_PUSH_TOKEN:
                        refreshPushToken();
                        break;

                    case CMD_REFRESH_PUSH_TOKEN_RESULT:
                        refreshPushTokenResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_PUSH_LINE_NAME_UPDATED:
                        lineNameUpdated((Bundle)msg.obj);
                        break;

                    case CMD_PUSH_CALL_ANSWERED:
                        callAnswered((Bundle)msg.obj);
                        break;

                    case CMD_MSISDN_AUTH_CHANGED:
                        msisdnAuthChanged((Bundle)msg.obj);
                        break;

                    case CMD_MSISDN_AUTH_REMOVED:
                        msisdnAuthRemoved((Bundle)msg.obj);
                        break;

                    case CMD_USER_LOGOUT:
                        doLogout(msg.arg1 == 0 ? false : true, msg.arg2 == 0 ? false : true);
                        break;

                    case CMD_SIGNOUT_RESULT:
                        signoutResult(msg.arg1);
                        break;

                    case CMD_GET_MSISDN_RESULT:
                        getMsisdnResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    case CMD_UPDATE_LOCATION_AND_TC_RESULT:
                        updateLocationAndTcResult(msg.arg1, (Bundle)msg.obj);
                        break;

                    default:
                        break;
                }
            }

            private String messageToString(Message msg) {
                switch (msg.what) {
                    case CMD_GET_DEVICE_CONFIG:
                        return "CMD_GET_DEVICE_CONFIG";

                    case CMD_GET_DEVICE_CONFIG_RESULT:
                        return "CMD_GET_DEVICE_CONFIG_RESULT";

                    case CMD_ON_BOARDING:
                        return "case CMD_ON_BOARDING";

                    case CMD_ON_BOARDING_RESULT:
                        return "CMD_ON_BOARDING_RESULT";

                    case CMD_GET_ACCESS_TOKEN_RESULT:
                        return "CMD_GET_ACCESS_TOKEN_RESULT";

                    case CMD_REFRESH_ACCESS_TOKEN_RESULT:
                        return "CMD_REFRESH_ACCESS_TOKEN_RESULT";

                    case CMD_GET_CONSUMER_PROFILE_RESULT:
                        return "CMD_GET_CONSUMER_PROFILE_RESULT";

                    case CMD_GET_MSISDN_AUTH_LIST_RESULT:
                        return "CMD_GET_MSISDN_AUTH_LIST_RESULT";

                    case CMD_CANCEL_TRANSACTION_RESULT:
                        return "CMD_CANCEL_TRANSACTION_RESULT";

                    case CMD_SUBSCRIBE_RESULT:
                        return "CMD_SUBSCRIBE_RESULT";

                    case CMD_UPDATE_MSISDN_SIT:
                        return "CMD_UPDATE_MSISDN_SIT";

                    case CMD_UPDATE_MSISDN_SIT_RESULT:
                        return "CMD_UPDATE_MSISDN_SIT_RESULT";

                    case CMD_SIT_RENEW:
                        return "CMD_SIT_RENEW";

                    case CMD_INIT_MSISDN_AUTH_RESULT:
                        return "CMD_INIT_MSISDN_AUTH_RESULT";

                    case CMD_ACTIVATE_LINE:
                        return "CMD_ACTIVATE_LINE";

                    case CMD_DEACTIVATE_LINE:
                        return "CMD_DEACTIVATE_LINE";

                    case CMD_PROVISION_LINE_RESULT:
                        return "CMD_PROVISION_LINE_RESULT";

                    case CMD_ADD_HUNT_GROUP:
                        return "CMD_ADD_HUNT_GROUP";

                    case CMD_ADD_HUNT_GROUP_RESULT:
                        return "CMD_ADD_HUNT_GROUP_RESULT";

                    case CMD_REMOVE_HUNT_GROUP_RESULT:
                        return "CMD_REMOVE_HUNT_GROUP_RESULT";

                    case CMD_REMOVE_LINE:
                        return "CMD_REMOVE_LINE";

                    case CMD_REMOVE_LINE_RESULT:
                        return "CMD_REMOVE_LINE_RESULT";

                    case CMD_REGISTERED_DEVICES:
                        return "CMD_REGISTERED_DEVICES";

                    case CMD_REGISTERED_DEVICES_RESULT:
                        return "CMD_REGISTERED_DEVICES_RESULT";

                    case CMD_REGISTERED_MSISDN:
                        return "CMD_REGISTERED_MSISDN";

                    case CMD_REGISTERED_MSISDN_RESULT:
                        return "CMD_REGISTERED_MSISDN_RESULT";

                    case CMD_REFRESH_DEVICE_INFO:
                        return "CMD_REFRESH_DEVICE_INFO";

                    case CMD_SIM_LOADED_EVENT:
                        return "CMD_SIM_LOADED_EVENT";

                    case CMD_RIL_SET_DIGITS_LINE:
                        return "CMD_RIL_SET_DIGITS_LINE";

                    case CMD_RIL_SET_DIGITS_LINE_RESPONSE:
                        return "CMD_RIL_SET_DIGITS_LINE_RESPONSE";

                    case CMD_RIL_DIGITS_LINE_INDICATION:
                        return "CMD_RIL_DIGITS_LINE_INDICATION";

                    case CMD_RIL_GET_TRN_INDICATION:
                        return "CMD_RIL_GET_TRN_INDICATION";

                    case CMD_GET_TRN_RESULT:
                        return "CMD_GET_TRN_RESULT";

                    case CMD_SET_DEVICE_NAME_RESULT:
                        return "CMD_SET_DEVICE_NAME_RESULT";

                    case CMD_SET_LINE_NAME_RESULT:
                        return "CMD_SET_LINE_NAME_RESULT";

                    case CMD_REFRESH_PUSH_TOKEN:
                        return "CMD_REFRESH_PUSH_TOKEN";

                    case CMD_DELETE_DEVICE_RESULT:
                        return "CMD_DELETE_DEVICE_RESULT";

                    case CMD_REFRESH_PUSH_TOKEN_RESULT:
                        return "CMD_REFRESH_PUSH_TOKEN_RESULT";

                    case CMD_PUSH_LINE_NAME_UPDATED:
                        return "CMD_PUSH_LINE_NAME_UPDATED";

                    case CMD_PUSH_CALL_ANSWERED:
                        return "CMD_PUSH_CALL_ANSWERED";

                    case CMD_MSISDN_AUTH_CHANGED:
                        return "CMD_MSISDN_AUTH_CHANGED";

                    case CMD_MSISDN_AUTH_REMOVED:
                        return "CMD_MSISDN_AUTH_REMOVED";

                    case CMD_USER_LOGOUT:
                        return "CMD_USER_LOGOUT";

                    case CMD_SIGNOUT_RESULT:
                        return "CMD_SIGNOUT_RESULT";

                    case CMD_GET_MSISDN_RESULT:
                        return "CMD_GET_MSISDN_RESULT";

                    case CMD_UPDATE_LOCATION_AND_TC_RESULT:
                        return "CMD_UPDATE_LOCATION_AND_TC_RESULT";

                    default:
                        return Integer.toString(msg.what);
                }
            }
        };
    }

    /** public API */
    public String getTmoidLoginURL() {
        String url = mIamManager.getTmoidLoginURL();
        Log.d(TAG, "[public] getTmoidLoginURL(), url: " + url);
        return url;
    }

    public String getMsisdnAuthURL() {
        String url = mIamManager.getMsisdnAuthURL();
        Log.d(TAG, "[public] getMsisdnAuthURL(), url: " + url);
        return url;
    }

    public String getTmoidSignupURL() {
        String url = mIamManager.getTmoidSignupURL();
        Log.d(TAG, "[public] getTmoidSignupURL(), url: " + url);
        return url;
    }

    public String getTmoidLogoutURL() {
        String url = mIamManager.getTmoidLogoutURL();
        Log.d(TAG, "[public] getTmoidLogoutURL(), url: " + url);
        return url;
    }

    public void notifyAddressOnFile() {
        Log.d(TAG, "[public] notifyAddressOnFile(), mNoE911AddressOnFile:" + mNoE911AddressOnFile
            + ", mState:" + stateToString(mState));

        if (mNoE911AddressOnFile &&
            (inState(DigitsConst.STATE_IDLE) || inState(DigitsConst.STATE_LOGOUT))) {

            boolean logout = inState(DigitsConst.STATE_LOGOUT);

            mHandler.sendMessage(mHandler.obtainMessage(CMD_ON_BOARDING, logout ? 1: 0, 0));
        }
    }

    public void getDeviceConfig() {
        Log.d(TAG, "[public] getDeviceConfig(), mState:" + stateToString(mState));


        mHandler.sendMessage(mHandler.obtainMessage(CMD_GET_DEVICE_CONFIG, 1, 0));

    }

    public void updateLocationAndTc() {
        Log.d(TAG, "[public] updateLocationAndTc(), mState:" + stateToString(mState));

        if (inState(DigitsConst.STATE_IDLE)) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_UPDATE_LOCATION_AND_TC_RESULT,
                DigitsConst.RESULT_FAIL_WRONG_STATE, 0));

            return;
        }

        mSesManager.addressRegistration(true);
    }

    public void subscribe(String authCode) {
        Log.d(TAG, "[public] subscribe(), authCode: " + authCode +
            ", mState:" + stateToString(mState));

        if (inState(DigitsConst.STATE_READY)) {

            // Set authCode to IAM. IAM will request access token from network
            mSubscribeReq = true;
            mIamManager.setAuthCode(authCode);

        } else {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_SUBSCRIBE_RESULT, DigitsConst.RESULT_FAIL_WRONG_STATE, 0));
            return;
        }


    }

    public void logout() {
        Log.d(TAG, "[public] logout(), mState:" + stateToString(mState));

        mHandler.sendMessage(mHandler.obtainMessage(CMD_USER_LOGOUT, 1, 0));
    }

    public void setDeviceName(String name) {
        Log.d(TAG, "[public] setDeviceName(), name: " + name + ", mState:" + stateToString(mState));

        Bundle output = new Bundle();
        output.putString(DigitsConst.EXTRA_STRING_DEVICE_NAME, name);

        if (mDeviceNameReq != null) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_SET_DEVICE_NAME_RESULT, DigitsConst.RESULT_FAIL_BUSY, 0, output));
            return;
        } else if (!inState(DigitsConst.STATE_SUBSCRIBED)) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_SET_DEVICE_NAME_RESULT, DigitsConst.RESULT_FAIL_WRONG_STATE, 0, output));
            return;
        }

        // Call SES API to set device name
        mSesManager.setDeviceName(name);
        mDeviceNameReq = name;
    }

    public void setLineName(String msisdn, String name) {
        Log.d(TAG, "[public] setLineName(), msisdn: " + msisdn + ", name: " + name +
            ", mState:" + stateToString(mState));

        Bundle output = new Bundle();
        output.putString(DigitsConst.EXTRA_STRING_MSISDN, msisdn);
        output.putString(DigitsConst.EXTRA_STRING_LINE_NAME, name);

        if (mLineNameReq != null) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_SET_LINE_NAME_RESULT, DigitsConst.RESULT_FAIL_BUSY, 0, output));
            return;
        } else if (!inState(DigitsConst.STATE_SUBSCRIBED)) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_SET_LINE_NAME_RESULT, DigitsConst.RESULT_FAIL_WRONG_STATE, 0, output));
            return;
        }

        mLineNameReq = new LineNameReq();
        mLineNameReq.mMsisdn = msisdn;
        mLineNameReq.mName = name;

        // Call SES API to set line name
        mSesManager.setLineName(msisdn, name);

    }

    public void setLineColor(String msisdn, int color) {
        Log.d(TAG, "[public] setLineColor(), msisdn: " + msisdn + ", color: " + color +
            ", mState:" + stateToString(mState));

        Bundle output = new Bundle();
        output.putString(DigitsConst.EXTRA_STRING_MSISDN, msisdn);
        output.putInt(DigitsConst.EXTRA_INT_LINE_COLOR, color);

        if (!inState(DigitsConst.STATE_SUBSCRIBED)) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_SET_LINE_COLOR_RESULT,
                DigitsConst.RESULT_FAIL_WRONG_STATE, 0, output));
            return;
        }

        // Set line color
        mStorage.setLineColor(msisdn, color);

        // Notify caller
        mClientHandler.sendMessage(mClientHandler.obtainMessage(
            DigitsServiceImpl.CMD_SET_LINE_COLOR_RESULT, DigitsConst.RESULT_SUCCEED, 0, output));

        // Notify registered msisdn changed
        notifyRegisteredMsisdnChanged();
    }

    public DigitsLine[] getRegisteredLine() {
        if (mSimLoaded && inState(DigitsConst.STATE_SUBSCRIBED)) {
            return mStorage.getRegisteredLine(false);
        } else if (mSimLoaded && inState(DigitsConst.STATE_READY)) {
            return mStorage.getRegisteredLine(true);
        } else {
            Log.d(TAG, "[public] getRegisteredLine() is null, mSimLoaded:" + mSimLoaded +
                ", mState:" + stateToString(mState));
            return null;
        }
    }

    public DigitsDevice[] getRegisteredDevice() {
        if (inState(DigitsConst.STATE_SUBSCRIBED)) {
            return mStorage.getRegisteredDevice();
        } else {
            Log.d(TAG, "[public] getRegisteredDevice() is null, mState:" + stateToString(mState));
            return null;
        }
    }

    public DigitsProfile getConsumerProfile() {
        if (inState(DigitsConst.STATE_SUBSCRIBED)) {
            return mStorage.getConsumerProfile();
        } else {
            Log.d(TAG, "[public] getConsumerProfile() is null, mState:" + stateToString(mState));
            return null;
        }
    }

    public void initMsisdnAuth(String msisdn) {
        Log.d(TAG, "[public] initMsisdnAuth(), msisdn: " + msisdn);

        Bundle output = new Bundle();
        output.putString(DigitsConst.EXTRA_STRING_MSISDN, msisdn);

        if (mAddLineReq != null) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_ADD_NEW_LINE_RESULT, DigitsConst.RESULT_FAIL_BUSY, 0, output));
            return;
        } else if (!inState(DigitsConst.STATE_SUBSCRIBED)) {

            // Notify caller
            mClientHandler.sendMessage(mClientHandler.obtainMessage(
                DigitsServiceImpl.CMD_ADD_NEW_LINE_RESULT, DigitsConst.RESULT_FAIL_WRONG_STATE, 0, output));
            return;
        }


        // Call IAM API to get msisdn auth list
        mIamManager.getMsisdnAuthList();
        mAddLineReq = msisdn;

        //TODO
        //public void initTmoidCreation(String msisdn)
        //public void initMsisdnAuthAndUpdateTmoId(String msisdn, String email, String password)
    }

    public void activateLine(String[] msisdns) {
        Log.d(TAG, "[public] activateLine(), msisdns: " + Arrays.toString(msisdns) +
            ", mState:" + stateToString(mState));

        mHandler.sendMessage(mHandler.obtainMessage(CMD_ACTIVATE_LINE, 0, 0, msisdns));
    }

    public void deactivateLine(String[] msisdns) {
        Log.d(TAG, "[public] deactivateLine(), msisdns: " + Arrays.toString(msisdns) +
            ", mState:" + stateToString(mState));

        mHandler.sendMessage(mHandler.obtainMessage(CMD_DEACTIVATE_LINE, 0, 0, msisdns));
    }

    public int getState() {
        Log.d(TAG, "getState(), state:" + stateToString(mState));
        return mState;
    }

    public void setRefreshToken(String refreshToken) {
        Log.d(TAG, "setRefreshToken(), refreshToken: " + refreshToken +
            ", mState:" + stateToString(mState));

        if (inState(DigitsConst.STATE_READY)) {

            // Set refresh token to IAM.
            mIamManager.setRefreshToken(refreshToken);

            // Auto login again
            autoLogin();
        }
    }

}

