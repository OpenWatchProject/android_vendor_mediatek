package com.mediatek.op07.incallui;

import android.content.Context;
import android.telecom.VideoProfile;
import android.util.Log;
import android.widget.Toast;

import com.android.incallui.call.DialerCall;

import com.mediatek.incallui.ext.DefaultVideoCallExt;
import com.mediatek.op07.dialer.R;

/**
 * Plugin implementation for VideoCall.
 */
public class OP07VideoCallExt extends DefaultVideoCallExt {
    private static final int COUNT_DOWN_SECONDS = 12;
    private static final String TAG = "OP07VideoCallExt";
    private Context mContext;
    private static final int SESSION_EVENT_BAD_DATA_BITRATE = 4008;
    /** Constructor.
     * @param context context
     */
    public OP07VideoCallExt(Context context) {
        super();
        mContext = context;
    }

    /**
     * called to show toast when call downgraded.
     * @param call  call object
     * @param event to notify call downgrade
     */
    @Override
    public void onCallSessionEvent(Object call, int event) {
        Log.d(TAG, "[onCallSessionEvent]downgrade due to insufficient bandwidth event"
                + event);
        DialerCall inCallUiCall = (DialerCall) call;
        if ((inCallUiCall != null) && (event == SESSION_EVENT_BAD_DATA_BITRATE)) {
            Toast.makeText(mContext,
                    mContext.getString(R.string.video_call_downgrade_insufficient_bandwidth),
                    Toast.LENGTH_SHORT).show();
            inCallUiCall.getVideoTech().downgradeToAudio();
        }
    }

    /**
     * called to change video call decline timer.
     * @return duration
     */
    @Override
    public int getDeclineTimer() {
        Log.v(TAG, "setDeclineTimer");
        return COUNT_DOWN_SECONDS;
    }

    /**
     * Check if should show video request.
     * @param videoState the profile videostate
     * @return true if need to show upgrade
     */
    @Override
    public boolean shouldShowVideoRequest(int videoState) {
        return VideoProfile.isBidirectional(videoState);
    }
}
