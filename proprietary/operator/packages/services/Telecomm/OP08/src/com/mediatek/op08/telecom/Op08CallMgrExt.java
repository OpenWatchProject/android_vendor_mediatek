package com.mediatek.op08.telecom;

import android.content.Context;
import android.util.Log;

import com.mediatek.server.telecom.ext.DefaultCallMgrExt;

/**
 * Plug in implementation for OP08 Call Mananger interfaces.
 */
public class Op08CallMgrExt extends DefaultCallMgrExt {
    private static final String LOG_TAG = "Op08CallMgrExt";
    private Context mContext = null;

    /** Constructor.
     * @param context context
     */
    public Op08CallMgrExt(Context context) {
       mContext = context;
    }

    @Override
    public boolean shouldDisconnectCallsWhenEcc() {
        Log.d(LOG_TAG, "shouldDisconnectCallsWhenEcc called");
        return false;
    }
}
