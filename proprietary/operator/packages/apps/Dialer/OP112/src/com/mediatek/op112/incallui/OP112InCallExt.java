package com.mediatek.op112.incallui;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.telecom.DisconnectCause;
import android.util.Log;

import com.mediatek.incallui.ext.DefaultInCallExt;

/**
 * Plug in implementation for OP112 InCallUI interfaces.
 */
public class OP112InCallExt extends DefaultInCallExt {
    private static final String TAG = "OP112InCallExt";

    /** Constructor.
     * @param context context
     */
    public OP112InCallExt(Context context) {}

    @Override
    public boolean maybeShowErrorDialog(DisconnectCause disconnectCause) {
        Log.d(TAG, "maybeShowErrorDialog disconnectCause = " + disconnectCause);
        if (disconnectCause.getCode() == mediatek.telecom.MtkDisconnectCause.SIP_INVITE_ERROR) {
            return true;
        }
        return false;
    }
}
