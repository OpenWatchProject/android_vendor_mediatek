package com.mediatek.op08.incallui;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.SystemProperties;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.PhoneNumberUtils;
import android.util.Log;

import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
import com.mediatek.incallui.ext.DefaultInCallButtonExt;
import mediatek.telecom.MtkCall;

/**
 * Plugin implementation for InCallExt.
 */
public class OP08InCallButtonExt extends DefaultInCallButtonExt {
    private static final String TAG = "OP08InCallButtonExt";
    private Context mContext;
    private static int BUTTON_DEVICE_SWITCH = 21;
    private DialerCall mCall;
    public static final int ACTIVE = 3;
    private boolean mIsDigitsSupported;
    public static final String ACTION_INIT_DIGITS_MANAGER =
            "mediatek.incallui.event.ACTION_INIT_DIGITS_MANAGER";
    public static final String ACTION_LAUNCH_DEVICE_SWITCH =
            "mediatek.incallui.event.ACTION_LAUNCH_DEVICE_SWITCH";
    private boolean mInitDigitsManager;

    /** Constructor.
     * @param context context
     */
    public OP08InCallButtonExt(Context context) {
        mContext = context;
    }

    public boolean isDeviceSwitchSupported(Object callObj) {
        Log.d(TAG, "isDeviceSwitchSupported entry" );
        if(callObj == null) {
            return false;
        }
        DialerCall call = (DialerCall) callObj;
        if(call != null) {
            mCall = call;
        }
        mIsDigitsSupported = SystemProperties.get("persist.vendor.mtk_digits_support").equals("1");
        boolean isCallActive = call.getState() == ACTIVE;
        boolean isConferenceCall = call.isConferenceCall();
        boolean currentHeldState = call.isRemotelyHeld();
        boolean isVoLteCall = call.getTelecomCall().getDetails().
                hasProperty(MtkCall.MtkDetails.MTK_PROPERTY_VOLTE);
        boolean isWFCCall = call.hasProperty(android.telecom.Call.Details.PROPERTY_WIFI);
        boolean isCSCall = !(isVoLteCall || isWFCCall);
        final boolean showDeviceSwitch = isCallActive
                && !currentHeldState && !isConferenceCall && !isCSCall;
        Log.d(TAG, "isDeviceSwitchSupported states isCallActive:" + isCallActive
                + "currentHeldState:" + currentHeldState + "mIsDigitsSupported"
                +  mIsDigitsSupported + "isConferenceCall" + isConferenceCall
                + "isVoLteCall" + isVoLteCall + "showDeviceSwitch" + showDeviceSwitch
                + "isCSCall" + isCSCall);
        if(showDeviceSwitch && mIsDigitsSupported) {
            initDigitsManagerIntent(mContext);
            return true;
        }
        return false;
    }

    public boolean onMenuItemClick(int id) {
        Log.d(TAG, "onMenuItemClick entry, id=" + id);
        if(id == BUTTON_DEVICE_SWITCH) {
            launchDeviceSwitchScreen(mContext, mCall.getTelecomCall()
                    .getDetails().getTelecomCallId());
            Log.d(TAG, "onMenuItemClick BUTTON_DEVICE_SWITCH, clicked");
        }
        return false;
    }

    private void initDigitsManagerIntent(Context context) {
        Log.d(TAG, "initDigitsManagerIntent entry" + mInitDigitsManager);
        if(context != null && !mInitDigitsManager) {
            mInitDigitsManager = true;
            Intent explicit = new Intent();
            ComponentName cn = new ComponentName("com.mediatek.op08.dialer",
                    "com.mediatek.op08.deviceswitch.DeviceSwitchScreenController");
            explicit.setComponent(cn);
            explicit.setAction(ACTION_INIT_DIGITS_MANAGER);
            context.sendBroadcast(explicit);
        }
    }

    private void launchDeviceSwitchScreen(Context context, String callId) {
        Log.d(TAG, "launchDeviceSwitchScreen callId" + callId);
        if(context != null && mInitDigitsManager && mCall != null) {
            PhoneAccountHandle accountHandle = mCall.getAccountHandle();
            TelecomManager telecomManager = (TelecomManager) context.
                    getSystemService(Context.TELECOM_SERVICE);
            PhoneAccount account = telecomManager.getPhoneAccount(accountHandle);
            Uri address = account.getAddress();
            String activeLine =
                    PhoneNumberUtils.extractNetworkPortion(Uri.decode(address.toString()));
            Log.d(TAG, "launchDeviceSwitchScreen address" + address + "activeLine" + activeLine);
            Intent explicit = new Intent();
            ComponentName cn = new ComponentName("com.mediatek.op08.dialer",
                    "com.mediatek.op08.deviceswitch.DeviceSwitchScreenController");
            explicit.setComponent(cn);
            explicit.setAction(ACTION_LAUNCH_DEVICE_SWITCH);
            explicit.putExtra("callId", callId);
            explicit.putExtra("activeLine", activeLine);
            context.sendBroadcast(explicit);
        } else {
            Log.d(TAG, "launchDeviceSwitchScreen, else");
        }
    }
}