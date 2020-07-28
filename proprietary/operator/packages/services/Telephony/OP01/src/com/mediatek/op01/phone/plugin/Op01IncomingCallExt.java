package com.mediatek.op01.phone.plugin;

import android.content.Context;
import android.util.Log;

import com.mediatek.phone.ext.DefaultIncomingCallExt;

public class Op01IncomingCallExt extends DefaultIncomingCallExt {
    private static final String LOG_TAG = "Op01IncomingCallExt";
    protected Context mContext;

    public Op01IncomingCallExt (Context context) {
        log("Op01IncomingCallExt");
        mContext = context;
    }

    /**
     * change the disconnect cause when user reject a call.
     * @param disconnectCause disconnectCause
     * @return disconnectCause after modified
     */
    @Override
    public int changeDisconnectCause(int disconnectCause) {
        if (disconnectCause == android.telephony.DisconnectCause.INCOMING_REJECTED) {
            disconnectCause = android.telephony.DisconnectCause.INCOMING_MISSED;
            log("changeDisconnectCause() disconnectCause: " + disconnectCause);
        }
        return disconnectCause;
    }

    /**
     * Log the message
     * @param msg the message will be printed
     */
    void log(String msg) {
        Log.d(LOG_TAG, msg);
    }
}
