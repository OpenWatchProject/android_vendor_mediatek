package com.mediatek.op07.incallui;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.telecom.DisconnectCause;
import android.telecom.VideoProfile;
import android.util.Log;

import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;

import com.mediatek.incallui.ext.DefaultInCallExt;
import com.mediatek.wfo.WifiOffloadManager;

import java.util.List;

/**
 * Plug in implementation for OP07 InCallUI interfaces.
 */
public class OP07InCallExt extends DefaultInCallExt implements CallList.Listener {
    private static final String TAG = "OP07InCallExt";
    private static final String ACTION_CLOSE_DIALOG =
            "com.mediatek.incallui.plugin.ACTION_CLOSE_DIALOG";
    private Context mContext = null;
    private DialerCall mCall = null;

    /**
     * extra infomation to indicate verify status of caller.
     */
    private static final String KEY_EXTRA_VERSTAT = "verstat";

    /** Constructor.
     * @param context context
     */
    public OP07InCallExt(Context context) {
        mContext = context;
    }

    @Override
    public boolean maybeShowErrorDialog(DisconnectCause disconnectCause) {
        Log.d(TAG, "maybeShowErrorDialog disconnectCause = " + disconnectCause);
        if (disconnectCause.getCode() == mediatek.telecom.MtkDisconnectCause.SIP_INVITE_ERROR) {
            return true;
        }
        return false;
    }

    /**
     * Showing handover dialog to user.
     * @param handler handler
     * @param stage stage
     * @param ratType rat type
     */
    @Override
    public void showHandoverNotification(Handler handler, int stage, int ratType) {
        Log.v(TAG, "show handover notification to user in video call");
        CallList callList = CallList.getInstance();
        DialerCall call = callList.getActiveOrBackgroundCall();
        if (call != null && call.isVideoCall() && call.hasProperty(
                android.telecom.Call.Details.PROPERTY_WIFI)) {
            if (stage == WifiOffloadManager.HANDOVER_START &&
                    ratType == WifiOffloadManager.RAN_TYPE_MOBILE_3GPP) {
                mCall = call;
                Intent errorIntent = new Intent(mContext, AlertDialogActivity.class);
                errorIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mContext.startActivity(errorIntent);
                callList.addListener(this);
            }
        }
    }

    @Override
    public void onIncomingCall(DialerCall call) {
    }

    @Override
    public void onUpgradeToVideo(DialerCall call) {
    }

    @Override
    public void onSessionModificationStateChange(DialerCall call) {
    }

    @Override
    public void onCallListChange(CallList callList) {
    }

    @Override
    public void onDisconnect(DialerCall call) {
        Log.v(TAG, "onDisconnect");
        if (call != mCall) {
            return;
        }
        CallList.getInstance().removeListener(this);
        Intent intent = new Intent(ACTION_CLOSE_DIALOG);
        mContext.sendBroadcast(intent);
        mCall = null;
    }

    @Override
    public void onWiFiToLteHandover(DialerCall call){
    }

    @Override
    public void onHandoverToWifiFailed(DialerCall call) {
    }

    @Override
    public void onInternationalCallOnWifi(DialerCall call) {
    }

    /**
     * Add intent list to process intent.
     * @param intentBundle the intentExtraBundle key list
     * @param extraBundle the extraBundle key list
     */
    @Override
    public void addFilterBundle(List<String> intentBundle, List<String> extraBundle) {
        extraBundle.add(KEY_EXTRA_VERSTAT);
    }

    @Override
    public void onCallDetailChanged(Object obj, android.telecom.Call.Details oldDetails,
            android.telecom.Call.Details newDetails) {
        DialerCall call = (com.android.incallui.call.DialerCall) obj;
        if (VideoProfile.isVideo(oldDetails.getVideoState())
                && !VideoProfile.isBidirectional(oldDetails.getVideoState())
                && VideoProfile.isBidirectional(newDetails.getVideoState())) {
            Log.d(TAG, "onCallDetailChanged, clear pause state");
            call.setVideoPauseState(false);
        }
    }
}
