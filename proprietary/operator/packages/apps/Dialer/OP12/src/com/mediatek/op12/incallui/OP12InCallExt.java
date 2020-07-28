package com.mediatek.op12.incallui;

import android.content.Context;
import android.util.Log;

import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;

import com.mediatek.incallui.ext.DefaultInCallExt;

/**
 * Plug in implementation for OP12 InCallUI interfaces.
 */
public class OP12InCallExt extends DefaultInCallExt {
    private static final String TAG = "OP12InCallExt";
    private Context mContext = null;

    /** Constructor.
     * @param context context
     */
    public OP12InCallExt(Context context) {
        mContext = context;
    }

    /**
     * Called when receve upgrade during incoming call.
     * @param obj Object
     * @return true if need to decline the video upgrade during incoming or waiting call
     */
    @Override
    public boolean declineUpgradeDuringIncoming(Object obj) {
        Log.d(TAG, "declineUpgradeDuringIncoming, return false");
        return false;
    }

    /**
     * Called when answer the incoming call.
     * @param isUpgrade if it is incoming upgrade request
     * @param obj the CallList instance used for plugin
     */
    @Override
    public void onAnswer(boolean isUpgrade, Object obj) {
        if (isUpgrade) {
            return;
        }

        if (obj instanceof CallList) {
            CallList callList = (CallList) obj;
            DialerCall call = callList.getVideoUpgradeRequestCall();
            if (call != null) {
                Log.d(TAG, "onAnswer, decline video request at the sametime");
                call.getVideoTech().declineVideoRequest();
            }
        }
    }
}
