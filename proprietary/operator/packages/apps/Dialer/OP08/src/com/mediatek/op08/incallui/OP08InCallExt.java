package com.mediatek.op08.incallui;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.telecom.DisconnectCause;
import android.util.Log;
import android.widget.Toast;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.os.SystemProperties;

import com.android.ims.ImsManager;
import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
import com.mediatek.incallui.ext.DefaultInCallExt;
import com.android.incallui.call.TelecomAdapter;

import com.mediatek.op08.dialer.R;

import java.util.List;

/**
 * Plugin implementation for InCallExt.
 */
public class OP08InCallExt extends DefaultInCallExt {
    private static final String TAG = "OP08InCallExt";
    private Context mContext;

    /**
     * event string to notify Telecom user device switch success.
     */
    public static final String EVENT_DEVICE_SWITCH_SUCCESS =
            "mediatek.telecom.event.EVENT_DEVICE_SWITCH_SUCCESS";

    /**
     * event string to notify Telecom user device switch failed.
     */
    public static final String EVENT_DEVICE_SWITCH_FAILED =
            "mediatek.telecom.event.EVENT_DEVICE_SWITCH_FAILED";

    /**
     * Action to dismiss device switch screen after success or fail event.
     */
    public static final String ACTION_DEVICE_SWITCH_DISMISS =
            "mediatek.incallui.event.ACTION_DEVICE_SWITCH_DISMISS";

    /**
     * event string to notify Telecom user device switch success.
     */
    public static final String EVENT_DEVICE_SWITCH_SEND =
            "mediatek.incallui.event.EVENT_DEVICE_SWITCH_SUCCESS";

    /**
     * event string to notify Telecom user device switch failed.
     */
    public static final String EVENT_DEVICE_SWITCH_CANCEL =
            "mediatek.incallui.event.EVENT_DEVICE_SWITCH_CANCEL";

    /**
     * extra infomation to indicate verify status of caller.
     */
    private static final String KEY_EXTRA_VERSTAT = "verstat";

    /** Constructor.
     * @param context context
     */
    public OP08InCallExt(Context context) {
        mContext = context;
    }

    @Override
    public void onInCallPresenterSetUp(Object statusbarNotifier, Object state, Object callList) {
        registerReceiver();
    }

    @Override
    public void onInCallPresenterTearDown() {
        unregisterReceiver();
        boolean mIsDigitsSupported = 
                SystemProperties.get("persist.vendor.mtk_digits_support").equals("1");
        if(mIsDigitsSupported)
        dismissDeviceSwitchScreen(mContext, false);
    }

    @Override
    public boolean showCongratsPopup(DisconnectCause disconnectCause) {
        Log.d(TAG, "showCongratsPopup disconnectCause = " + disconnectCause.getCode());
        DialerCall call = CallList.getInstance().getDisconnectedCall();
        if (ImsManager.isWfcEnabledByUser(mContext)
                && !(disconnectCause.getCode() == DisconnectCause.OTHER)
                && !(disconnectCause.getCode() == DisconnectCause.REJECTED)
                && !(disconnectCause.getCode() == DisconnectCause.MISSED)
                && !(disconnectCause.getCode() == DisconnectCause.ERROR)
                && !(disconnectCause.getCode() == DisconnectCause.BUSY)
                && call != null && call.hasProperty(android.telecom.Call.Details.PROPERTY_WIFI)) {
            showCongratsPopup();
            return true;
        }
        return false;
    }

    /**
     * show congrats popup.
     */
    private void showCongratsPopup() {
        int timer = 500;
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "CongratsPopup shown");
                Intent intent = new Intent(mContext, AlertDialogActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mContext.startActivity(intent);
            }
        };
        Handler handler  = new Handler();
        handler.postDelayed(runnable, timer);
    }

    public void onConnectionEvent(Object call, String event, Bundle extras) {
        switch (event) {
        case EVENT_DEVICE_SWITCH_SUCCESS: {
            Log.d(TAG, "onConnectionEvent, EVENT_DEVICE_SWITCH_SUCCESS");
            dismissDeviceSwitchScreen(mContext, true);
            break;
        }
        case EVENT_DEVICE_SWITCH_FAILED: {
            Log.d(TAG, "onConnectionEvent, EVENT_DEVICE_SWITCH_FAILED");
            dismissDeviceSwitchScreen(mContext, false);
            break;
        }
        default:
            break;
        }
    }

    private void dismissDeviceSwitchScreen(Context context, boolean status) {
        Log.d(TAG, "dismissDeviceSwitchScreen entry");
        if(context != null) {
            Intent explicit = new Intent();
            ComponentName cn = new ComponentName("com.mediatek.op08.dialer",
                    "com.mediatek.op08.deviceswitch.DeviceSwitchScreenController");
            explicit.setComponent(cn);
            explicit.putExtra("switchStatus", status);
            explicit.setAction(ACTION_DEVICE_SWITCH_DISMISS);
            context.sendBroadcast(explicit);
        } else {
            Log.d(TAG, "dismissDeviceSwitchScreen, else");
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "onReceive for device switch, ");
            String action = intent.getAction();
            Log.v(TAG, action);
            if (intent.getAction().equals(EVENT_DEVICE_SWITCH_SEND)) {
                String mCallId = intent.getStringExtra("callId");
                String number = intent.getStringExtra("number");
                if(number != null && !number.isEmpty()) {
                    number = "+" + number;
                }
                String deviceId = intent.getStringExtra("deviceId");
                Log.d(TAG, "onReceive, EVENT_DEVICE_SWITCH_SEND, callId" + mCallId
                        + "number=" + number + "deviceid=" + deviceId);
                //TelecomAdapter.getInstance().deviceSwitch(mCallId, number, deviceId);
            } else if (intent.getAction().equals(EVENT_DEVICE_SWITCH_CANCEL)) {
                String mCallId = intent.getStringExtra("callId");
                Log.d(TAG, "onReceive, EVENT_DEVICE_SWITCH_CANCEL, mCallId " + mCallId);
                //TelecomAdapter.getInstance().cancelDeviceSwitch(mCallId);
            }
        }
    };

    /** Register receiver for battery status.
     */
    public void registerReceiver() {
        Log.i(TAG, "registerReceiver");
        IntentFilter filter = new IntentFilter();
        filter.addAction(EVENT_DEVICE_SWITCH_SEND);
        filter.addAction(EVENT_DEVICE_SWITCH_CANCEL);
        mContext.registerReceiver(mReceiver, filter);
    }

    /** Un-register receiver for battery status.
     */
    public void unregisterReceiver() {
        Log.i(TAG, "unregisterReceiver");
        if (mReceiver != null) {
            mContext.unregisterReceiver(mReceiver);
        }
    }

    /**
     * Add intent list to process intent.
     * @param intentBundle the intentExtraBundle key list
     * @param extraBundle the extraBundle key list
     */
    @Override
    public void addFilterBundle(List<String> intentBundle, List<String> extraBundle) {
      ///extraBundle.add(KEY_EXTRA_VERSTAT);
    }
}