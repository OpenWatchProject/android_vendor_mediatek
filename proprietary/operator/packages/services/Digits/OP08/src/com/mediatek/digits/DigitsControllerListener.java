package com.mediatek.digits;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Message;
import android.os.Handler;
import android.util.Log;

import com.mediatek.digits.DigitsConfigManager.ConfigListener;
import com.mediatek.digits.gcm.GcmManager;
import com.mediatek.digits.gcm.GcmManager.GcmListener;
import com.mediatek.digits.iam.IamManager;
import com.mediatek.digits.iam.IamManager.IamListener;
import com.mediatek.digits.ses.SesManager;
import com.mediatek.digits.ses.SesManager.SesListener;
import com.mediatek.digits.wsg.WsgManager;
import com.mediatek.digits.wsg.WsgManager.WsgListener;


public class DigitsControllerListener{

    static protected final String TAG = "DigitsControllerListener";

    static final private String INTENT_ACTION_RCS_STARTED = "com.mediatek.rcse.action.STARTED";

    Context mContext;
    Handler mCtrlHandler;

    boolean mGcmEventEnabled;

    public DigitsControllerListener(Context context, Handler ctrlHandler) {

        Log.d(TAG, "DigitsControllerListener() Constructor");

        mContext = context;
        mCtrlHandler = ctrlHandler;

        registerForBroadcast();
    }

    /** public API */
    public SesListener getSesListener() {
        return mSesListener;
    }

    public WsgListener getWsgListener() {
        return mWsgListener;
    }

    public IamListener getIamListener() {
        return mIamListener;
    }

    public GcmListener getGcmListener() {
        return mGcmListener;
    }

    public ConfigListener getConfigListener() {
        return mConfigListener;
    }

    public void enableGcmEvent(boolean enable) {

        mGcmEventEnabled = enable;
    }

    private void registerForBroadcast() {
        IntentFilter filter = new IntentFilter();

        filter.addAction(INTENT_ACTION_RCS_STARTED);
        filter.addAction(Intent.ACTION_SHUTDOWN);

        mContext.registerReceiver(mReceiver, filter);
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "onReceive() " + intent.getAction());

            if (intent.getAction().equals(INTENT_ACTION_RCS_STARTED)) {

                mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                    DigitsController.CMD_RIL_SET_DIGITS_LINE));

            } else if (intent.getAction().equals(Intent.ACTION_SHUTDOWN)) {

                mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_USER_LOGOUT, 0, 0));
            }
        }
    };


    /** Interface for callback to notify SES event */
    private SesListener mSesListener = new SesListener() {

        @Override
        public void onSesEvent(int event, int result, Bundle bundle) {
            Log.d(TAG, "onSesEvent(), event:" + SesManager.eventToString(event) +
                ", result:" + result);

            switch (event) {
                case SesManager.EVENT_GET_DEVICE_CONFIG_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_GET_DEVICE_CONFIG_RESULT, result, 0));
                    break;

                case SesManager.EVENT_ON_BOARDING_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_ON_BOARDING_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_INIT_SUBSCRIBE_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_SUBSCRIBE_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_UPDATE_MSISDN_SIT_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_UPDATE_MSISDN_SIT_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_PROVISION_LINE_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_PROVISION_LINE_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_REGISTERED_DEVICES_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_REGISTERED_DEVICES_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_REGISTERED_MSISDN_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_REGISTERED_MSISDN_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_REMOVE_LINE_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_REMOVE_LINE_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_SET_DEVICE_NAME_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_SET_DEVICE_NAME_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_SET_LINE_NAME_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_SET_LINE_NAME_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_REFRESH_PUSH_TOKEN_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_REFRESH_PUSH_TOKEN_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_DELETE_DEVICE_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_DELETE_DEVICE_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_USER_LOGOUT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_USER_LOGOUT, 1, 1));
                    break;
                case SesManager.EVENT_GET_MSISDN_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_GET_MSISDN_RESULT, 0, 0));
                    break;

                case SesManager.EVENT_ON_ADDRESS_REGISTRATION:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_UPDATE_LOCATION_AND_TC_RESULT, result, 0, bundle));
                    break;

                case SesManager.EVENT_SIT_RENEW:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_SIT_RENEW, result, 0, bundle));
                    break;
                default:

            }
        }
    };

    /** Interface for callback to notify WSG event */
    private WsgListener mWsgListener = new WsgListener() {

        @Override
        public void onWsgEvent(int event, int result, Bundle bundle) {
            Log.d(TAG, "onWsgEvent(), event:" + WsgManager.eventToString(event) +
                ", result:" + result);

            switch (event) {
                case WsgManager.EVENT_ADD_HUNT_GROUP_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_ADD_HUNT_GROUP_RESULT, result, 0, bundle));
                    break;

                case WsgManager.EVENT_REMOVE_HUNT_GROUP_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_REMOVE_HUNT_GROUP_RESULT, result, 0, bundle));
                    break;
                case WsgManager.EVENT_GET_TRN_RESULT:

                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_GET_TRN_RESULT, result, 0, bundle));
                    break;
                default:
            }
        }
    };

    /** Interface for callback to notify IAM event */
    private IamListener mIamListener = new IamListener() {

        @Override
        public void onIamEvent(int event, int result, Bundle bundle) {
            Log.d(TAG, "onIamEvent(), event:" + IamManager.eventToString(event) +
                ", result:" + result);

            switch (event) {
                case IamManager.EVENT_GET_ACCESS_TOKEN_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_GET_ACCESS_TOKEN_RESULT, result, 0, bundle));
                    break;

                case IamManager.EVENT_REFRESH_ACCESS_TOKEN_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_REFRESH_ACCESS_TOKEN_RESULT, result, 0, bundle));
                    break;

                case IamManager.EVENT_GET_CONSUMER_PROFILE_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_GET_CONSUMER_PROFILE_RESULT, result, 0, bundle));
                    break;

                case IamManager.EVENT_CANCEL_TRANSACTION_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_CANCEL_TRANSACTION_RESULT, result, 0, bundle));
                    break;

                case IamManager.EVENT_INIT_MSISDN_AUTH_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_INIT_MSISDN_AUTH_RESULT, result, 0, bundle));
                    break;

                case IamManager.EVENT_GET_MSISDN_AUTH_LIST_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_GET_MSISDN_AUTH_LIST_RESULT, result, 0, bundle));
                    break;

                case IamManager.EVENT_USER_LOGOUT_RESULT:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_SIGNOUT_RESULT, result, 0));
                    break;
            }

        }
    };

    /** Interface for callback to notify GCM event */
    private GcmListener mGcmListener = new GcmListener() {

        @Override
        public void onGcmEvent(int event, Bundle bundle) {
            Log.d(TAG, "onGcmEvent(), mGcmEventEnabled:" + mGcmEventEnabled +
                ", event:" + GcmManager.eventToString(event));

            if (!mGcmEventEnabled) {
                return;
            }

            switch (event) {
                case GcmManager.EVENT_VOWIFI_SUBSCRIPTION_REMOVED_CUSTOMER_CARE:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_MSISDN_AUTH_REMOVED, bundle));
                    break;

                case GcmManager.EVENT_VOWIFI_SUBSCRIPTION_REMOVED:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_REGISTERED_DEVICES, 0, 0));
                    break;

                case GcmManager.EVENT_LINE_NAME_UPDATED:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_PUSH_LINE_NAME_UPDATED, bundle));
                    break;

                case GcmManager.EVENT_NEW_LINE_INSTANCE_ACTIVATION:
                    //mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                    //    DigitsController.CMD_REGISTERED_DEVICES, 1, 0));
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_REFRESH_DEVICE_INFO, 0, 0));
                    break;

                case GcmManager.EVENT_CALL_ANSWERED:
                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_PUSH_CALL_ANSWERED, bundle));
                    break;

                case GcmManager.EVENT_MSISDN_AUTH_CHANGED:
                    mCtrlHandler.sendMessage(
                        mCtrlHandler.obtainMessage(DigitsController.CMD_MSISDN_AUTH_CHANGED, bundle));
                    break;

                case GcmManager.EVENT_MUTUALLY_AGREE_LOGOUT:

                    mCtrlHandler.sendMessage(mCtrlHandler.obtainMessage(
                        DigitsController.CMD_USER_LOGOUT, 0, 0));
                    break;
                default:
            }
        }
    };

    /** Interface for callback to notify SES event */
    private ConfigListener mConfigListener = new ConfigListener() {

        @Override
        public void onRetrieveConfig() {
            Log.d(TAG, "onRetrieveConfig()");

            mCtrlHandler.sendMessage(
                mCtrlHandler.obtainMessage(DigitsController.CMD_GET_DEVICE_CONFIG, 0, 0));
        }

        @Override
        public void onRefreshDeviceInfo(boolean simSwap) {
            Log.d(TAG, "onRefreshDeviceInfo(), simSwap:" + simSwap);

            mCtrlHandler.sendMessage(
                mCtrlHandler.obtainMessage(DigitsController.CMD_REFRESH_DEVICE_INFO,
                    simSwap ? 1 : 0, 0));
        }

        @Override
        public void onSimLoaded(boolean loaded) {
            Log.d(TAG, "onSimLoaded(), loaded:" + loaded);

            mCtrlHandler.sendMessage(
                mCtrlHandler.obtainMessage(DigitsController.CMD_SIM_LOADED_EVENT,
                    loaded ? 1 : 0, 0));
        }

    };
}

