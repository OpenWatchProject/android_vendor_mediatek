package com.mediatek.op08.deviceswitch;

import java.text.DateFormat;
import java.util.Date;

import android.util.Log;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Toast;

import com.mediatek.digits.DigitsManager;
import com.mediatek.digits.DigitsConst;

import com.mediatek.op08.dialer.R;

public class DeviceSwitchScreenController extends BroadcastReceiver{

    private static final String TAG = "DeviceSwitchScreenController";
    private DeviceSwitchScreen mDeviceSwitchDialog;
    public static DeviceSwitchScreenController sInstance;
    private DigitsManager mManager;
    private static Context mContext;
    public static int mState;
    public static int mServiceState;
    public static String mTelecomCallId;

    /**
     * Service is in login status (subscribed) and Digits lines (native, virtual) are SIP-registered
     */
    public static final int STATE_REGISTERED = 3;
    public static final String ACTION_INIT_DIGITS_MANAGER =
            "mediatek.incallui.event.ACTION_INIT_DIGITS_MANAGER";
    public static final String ACTION_LAUNCH_DEVICE_SWITCH =
            "mediatek.incallui.event.ACTION_LAUNCH_DEVICE_SWITCH";
    public static final String ACTION_DEVICE_SWITCH_DISMISS =
            "mediatek.incallui.event.ACTION_DEVICE_SWITCH_DISMISS";

    public void showDialog(Context context, String exrtaParam, String activeLine) {
        Log.i(TAG, "mManager:" + mManager + "mState:" + mState + "serviceState:"
                + mServiceState);
        if(mManager == null || mManager.getServiceState() != DigitsConst.STATE_SUBSCRIBED
                || mState != DigitsConst.RESULT_SUCCEED) {
            Toast.makeText(context, R.string.digits_service_not_ready,
                    Toast.LENGTH_LONG).show();
            return;
        }
        Intent intent = new Intent(context, DeviceSwitchScreen.class);
        intent.putExtra("CallId", exrtaParam);
        intent.putExtra("activeLine", activeLine);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    public void initDigitsManager(Context context) {
        if(mManager == null) {
            Log.d(TAG, "initDigitsManager->entry = ");
            mManager = new DigitsManager(context);
            mManager.registerEventListener(mDigitsServiceListener);
        }
    }

    private DigitsManager.EventListener mDigitsServiceListener = new DigitsManager.EventListener() {
        @Override
        public void onEvent(int event, int result, Bundle extras) {
            Log.i(TAG, "onEvent(), event:" + event + ", result:" + result + ", extras:" + extras);
            switch (event) {
                case DigitsConst.EVENT_SERVICE_CONNECTION_RESULT:
                    if (result == DigitsConst.RESULT_SUCCEED) {
                        mState = DigitsConst.RESULT_SUCCEED;
                        Log.i(TAG, "state:" + mState);
                    }
                    break;
                default:
                    break;
            }
            mServiceState = mManager.getServiceState();
       }
    };

    public static synchronized DeviceSwitchScreenController getInstance() {
        if (sInstance == null) {
            sInstance = new DeviceSwitchScreenController();
        }
        return sInstance;
    }

    public void setDeviceSwitchDialog(Object o) {
        // If there has one "Dialog" already, dismiss it first. quick-click may cause this.
        if (o instanceof DeviceSwitchScreen) {
            if (mDeviceSwitchDialog != null) {
                mDeviceSwitchDialog.finish();
            }
            mDeviceSwitchDialog = (DeviceSwitchScreen) o;
        }
    }

    public void clearDeviceSwitchDialog(Object o) {
        if (o instanceof DeviceSwitchScreenController && mDeviceSwitchDialog != null
                && mDeviceSwitchDialog.isFinishing()) {
            mDeviceSwitchDialog = null;
        }
    }

    public void dismissDialog() {
        if (mDeviceSwitchDialog != null) {
            mDeviceSwitchDialog.finish();
            mDeviceSwitchDialog = null;
        }
    }

    public boolean isDialogShown() {
        if (mDeviceSwitchDialog != null) {
            return true;
        }
        return false;
    }

    public DigitsManager getDigitsManager() {
        if(mManager != null) {
            return mManager;
        }
        return null;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if(mContext == null) {
            mContext = context.getApplicationContext();
        }
        String action = intent.getAction();
        Log.d(TAG, "onReceive device switch, action" + action);
        if (intent.getAction().equals(ACTION_INIT_DIGITS_MANAGER)) {
            Log.d(TAG, "onReceive init switch");
            DeviceSwitchScreenController.getInstance().initDigitsManager(mContext);
        } else if (intent.getAction().equals(ACTION_LAUNCH_DEVICE_SWITCH)) {
            mTelecomCallId = intent.getStringExtra("callId");
            String activeLine = intent.getStringExtra("activeLine");
            Log.d(TAG, "onReceive launch switch,callId=" + mTelecomCallId);
            DeviceSwitchScreenController.getInstance()
                        .showDialog(mContext, mTelecomCallId, activeLine);
        } else if (intent.getAction().equals(ACTION_DEVICE_SWITCH_DISMISS)
                && DeviceSwitchScreenController.getInstance().isDialogShown()) {
            boolean switchStatus = intent.getBooleanExtra("switchStatus", false);
            Log.d(TAG, "onReceive->entry , ACTION_DEVICE_SWITCH_DISMISS= " + switchStatus);
            if(switchStatus) {
                String successString =
                        mContext.getResources().getString(R.string.device_switch_successfull);
                String deviceSwitchTime = DateFormat.getDateTimeInstance().format(new Date());
                String deviceName = DeviceSwitchScreen.mSelectedDeviceName;
                if(deviceName == null) {
                    deviceName = mContext.getResources().getString(R.string.all_my_devices);
                }
                successString = successString + " " + deviceName +
                        " at "+ deviceSwitchTime;
                Toast.makeText(mContext,successString,Toast.LENGTH_LONG).show();
            } else {
                Toast.makeText(mContext, R.string.device_switch_failed,
                        Toast.LENGTH_LONG).show();
            }
            DeviceSwitchScreenController.getInstance().dismissDialog();
        }
    }
}

