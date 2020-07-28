package com.mediatek.digits;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.Network;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.digits.utils.CallbackScheduler;
import com.mediatek.internal.telephony.MtkPhoneConstants;



public class DigitsConfigManager {
    static protected final String TAG = "DigitsConfigManager";

    // Declare internal message command
    static final private int CMD_UPDATE_DEVICE_CONFIG = 0;

    static final private String INTENT_ACTION_UPDATE_DEVICE_CONFIG =
        "com.mediatek.digits.UPDATE_DEVICE_CONFIG";

    static final private int GET_DEVICE_CONFIG_FAIL_RETRY_MILLISECOND = 15 * 1000;  // 15s
    static final private int GET_DEVICE_CONFIG_FAIL_RETRY_MAX_TIME = 4; // 1 mins
    private int mRetryCount;

    private Context mContext;
    private SubscriptionManager mSubscriptionManager;

    private CallbackScheduler mUpdateConfigScheduler;

    boolean mNetworkAvailable;
    boolean mSimLoaded;
    boolean mEverRetriveSucceed;

    ConfigListener mListener;

    /** Interface for callback to notify config event */
    public interface ConfigListener {
        /**
         * callback to request to retrieve device config file
         */
        void onRetrieveConfig();

        /**
         * callback to refresh device info when SIM ready
         */
        void onRefreshDeviceInfo(boolean simSwap);

        /**
         * callback to notify SIM loaded or not
         */
        void onSimLoaded(boolean loaded);
    }

    public DigitsConfigManager(Context context, ConfigListener listener) {
        Log.d(TAG, "DigitsConfigManager() Constructor");

        mContext = context;
        mListener = listener;

        mSubscriptionManager = SubscriptionManager.from(mContext);

        registerDefaultNetwork();
        registerForBroadcast();

        initCallbackScheduler();
    }

    public void notifyGetConfigResult(boolean succeed) {
        Log.d(TAG, "notifyGetConfigResult(), succeed: " + succeed +
            ", mEverRetriveSucceed:" + mEverRetriveSucceed + ", mRetryCount:" + mRetryCount);

        if (succeed) {
            mEverRetriveSucceed = true;

            DigitsConfig config = DigitsConfig.getInstance();
            int refreshTime = config.getConfigRefreshTime();
            Log.d(TAG, "refreshTime: " + refreshTime);

            // Remove existed message and re-start periodic update timer
            mUpdateConfigScheduler.stop();
            mUpdateConfigScheduler.start(refreshTime * 1000);
        } else {

            // retry to get device config again
            if (!mEverRetriveSucceed && mRetryCount < GET_DEVICE_CONFIG_FAIL_RETRY_MAX_TIME) {

                // Remove existed message and re-start retry timer, only retry once
                mUpdateConfigScheduler.stop();
                mUpdateConfigScheduler.start(GET_DEVICE_CONFIG_FAIL_RETRY_MILLISECOND);
                mRetryCount ++;
            }
        }
    }

    private void initCallbackScheduler() {
        Log.d(TAG, "initCallbackScheduler()");

        mUpdateConfigScheduler = new CallbackScheduler(mContext,
            INTENT_ACTION_UPDATE_DEVICE_CONFIG,
            new CallbackScheduler.Callback() {

                public void run(String action) {
                    Log.d(TAG, "run: " + action);
                    mHandler.sendMessage(mHandler.obtainMessage(CMD_UPDATE_DEVICE_CONFIG));
                }

            });
    }

    private void registerForBroadcast() {
        IntentFilter filter = new IntentFilter();

        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);

        mContext.registerReceiver(mReceiver, filter);
    }


    private void registerDefaultNetwork() {

        ConnectivityManager cm = ConnectivityManager.from(mContext);
        cm.registerDefaultNetworkCallback(new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                Log.d(TAG, "NetworkCallback.onAvailable(), mEverRetriveSucceed: " +
                    mEverRetriveSucceed);

                mNetworkAvailable = true;

                // Reset retry count when network is avaiable everytime
                mRetryCount = 0;

                if (!mEverRetriveSucceed) {

                    // Callback to retrive the device config file
                    getDeviceConfig();
                }
            }

            @Override
            public void onLost(Network network) {
                Log.d(TAG, "NetworkCallback.onLost()");

                mNetworkAvailable = false;
            }
        });
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

            if (intent.getAction().equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {

                String state = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                int slot = intent.getIntExtra(PhoneConstants.SLOT_KEY, -1);

                Log.d(TAG, "onReceive() SIM_STATE_CHANGED, slot:" + slot +
                    ", state:" + state);

                // Ignore non main SIM state
                if (slot == getMainCapabilityPhoneId()) {

                    handleSimSwap(state);
                }
            }
        }
    };


    private void handleSimSwap(String state) {

        Log.d(TAG, "handleSimSwap(), state:" + state + ", mNetworkAvailable:" + mNetworkAvailable);

        boolean loaded = state.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED) ? true : false;

        if (loaded == mSimLoaded) {
            return;
        }

        mSimLoaded = loaded;

        // Callback to notify if SIM is loaded
        mListener.onSimLoaded(mSimLoaded);

        if (mSimLoaded) {

            String iccid = DigitsSharedPreference.getInstance(mContext).getIccid();
            String newIccid = getIccId();
            DigitsSharedPreference.getInstance(mContext).saveIccid(newIccid);

            Log.d(TAG, "iccid:" + iccid + ", newIccid:" + newIccid);

            if (iccid != null) {
                if (iccid.equals(newIccid)) {
                    // Callback to refresh device info
                    mListener.onRefreshDeviceInfo(false);

                    if (mEverRetriveSucceed) {
                        // No need to get device config again
                        return;
                    }
                } else {

                    // Callback to refresh device info
                    mListener.onRefreshDeviceInfo(true);

                }
            }

            // reset for the new SIM
            mEverRetriveSucceed = false;

            getDeviceConfig();
        }
    }

    private void getDeviceConfig() {

        if (mNetworkAvailable && mSimLoaded) {

            mListener.onRetrieveConfig();

        } else {
            Log.d(TAG, "getDeviceConfig(), mNetworkAvailable:" + mNetworkAvailable +
                ", mSimLoaded:" + mSimLoaded);
        }
    }

    private String getIccId() {
        if (mSubscriptionManager == null) {
            Log.d(TAG, "getIccId(), mSubscriptionManager is null");
            return null;
        }

        int mainCapabilityPhoneId = getMainCapabilityPhoneId();
        SubscriptionInfo subInfo = mSubscriptionManager.getActiveSubscriptionInfoForSimSlotIndex(
                mainCapabilityPhoneId);

        if (subInfo != null) {
            Log.d(TAG, "ICCID = " + subInfo.getIccId());
            return subInfo.getIccId();
        } else {
            Log.d(TAG, "getIccId(), subInfo is null");
            return null;
        }
    }

    private int getMainCapabilityPhoneId() {
       int phoneId = SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
       if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
           phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
       }
       Log.d(TAG, "getMainCapabilityPhoneId = " + phoneId);
       return phoneId;
    }

    /** Handler to handle internal message command*/
    Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "handleMessage: " + messageToString(msg));

            switch (msg.what) {
                case CMD_UPDATE_DEVICE_CONFIG:

                    getDeviceConfig();
                    break;
            }
        }

        private String messageToString(Message msg) {

            switch (msg.what) {
                case CMD_UPDATE_DEVICE_CONFIG:
                    return "CMD_UPDATE_DEVICE_CONFIG";
                default:
                    return Integer.toString(msg.what);
            }
        }
    };



}
