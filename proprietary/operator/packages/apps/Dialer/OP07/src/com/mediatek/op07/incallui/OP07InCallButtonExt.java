package com.mediatek.op07.incallui;

import android.content.Context;
import android.telecom.VideoProfile;
import android.util.Log;

import com.android.incallui.call.state.DialerCallState;
import com.android.incallui.call.DialerCall;
import com.android.incallui.videotech.utils.SessionModificationState;
import com.mediatek.incallui.ext.DefaultInCallButtonExt;

/**
 * Plug in implementation for OP07 InCallButton interfaces.
 */
public class OP07InCallButtonExt extends DefaultInCallButtonExt {
    private static final String TAG = "OP07InCallButtonExt";
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP07InCallButtonExt(Context context) {
        mContext = context;
    }

    /**
     * Checks if the one way video upgrade is supported.
     * @param call the call object.
     * @return true if one way video upgrade is supported.
     */
    @Override
    public boolean isOneWayVideoSupportedForCall(Object call) {
        Log.d(TAG, "isOneWayVideoSupportedForCall");
        if (call instanceof DialerCall) {
            DialerCall dialerCall = (DialerCall) call;
            if (dialerCall.isVideoCall()) {
                return false;
            }
            if (!dialerCall.getVideoTech().isAvailable(mContext, dialerCall.getAccountHandle())) {
                Log.d(TAG, "Sip due to not available.");
                return false;
            }
            if (dialerCall.isRemotelyHeld()
                    || dialerCall.getState() != DialerCallState.ACTIVE) {
                return false;
            }
            int state = dialerCall.getVideoTech().getSessionModificationState();
            boolean sessionProgress =
                state == SessionModificationState.WAITING_FOR_UPGRADE_TO_VIDEO_RESPONSE
                || state == SessionModificationState.RECEIVED_UPGRADE_TO_VIDEO_REQUEST
                || state == SessionModificationState.WAITING_FOR_RESPONSE
                || state == SessionModificationState.WAITING_FOR_CANCEL_UPGRADE_RESPONSE;
            if (sessionProgress) {
                Log.d(TAG, "Sip due to session progress.");
                return false;
            }
            return true;
        }
        return false;
    }

    /**
     * Checks if the one way video upgrade is supported on device.
     * @return true if one way video upgrade is supported.
     */
    @Override
    public boolean isOneWayVideoSupported() {
        Log.d(TAG, "isOneWayVideoSupported");
        return true;
    }

    /**
     * Check if should show upgrade Button.
     * @param the obj of the DialerCall instance
     * @return true if need to show upgrade button
     */
    @Override
    public boolean shouldShowUpgradeButton(Object obj) {
        if (obj instanceof DialerCall) {
            DialerCall dialerCall = (DialerCall) obj;
            int videoState = dialerCall.getVideoState();
            return  VideoProfile.isTransmissionEnabled(videoState)
                    && !VideoProfile.isBidirectional(videoState);
        }
        return false;
    }
}