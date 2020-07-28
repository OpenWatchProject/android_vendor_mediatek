package com.mediatek.op12.telecom;

import android.content.Context;
import android.util.Log;

import com.mediatek.server.telecom.ext.DefaultCallMgrExt;

/**
 * Plug in implementation for OP12 Call Mananger interfaces.
 */
public class Op12CallMgrExt extends DefaultCallMgrExt {
    private static final String LOG_TAG = "Op12CallMgrExt";

    /** Constructor.
     * @param context context
     */
    public Op12CallMgrExt(Context context) {
    }

    @Override
    public boolean shouldDisconnectCallsWhenEcc() {
        Log.d(LOG_TAG, "shouldDisconnectCallsWhenEcc called");
        return false;
    }
}
