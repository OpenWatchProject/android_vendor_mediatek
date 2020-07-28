package com.mediatek.settings.op09clib;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.settings.ext.DefaultSimManagementExt;

public class Op09SimManagementExt extends DefaultSimManagementExt {

    private static final String TAG = "Op09SimManagementExt";
    private Context mContext;

    public Op09SimManagementExt(Context context) {
        mContext = context;
    }

    @Override
    public boolean customizeSimCardForPhoneState() {
        Log.i(TAG, "customizeSimCardForPhoneState return true");
        return true;
    }

    @Override
    public boolean customizeCallStateNotInCall() {
        boolean inNotCall = isInNotCall();
        Log.i(TAG, "customizeCallState inNotCall = " + inNotCall);
        return inNotCall;
    }
    public static boolean isInNotCall() {
        TelephonyManager tm = TelephonyManager.getDefault();
        if (tm == null) {
            return false;
        }
        int phoneCount = tm.getPhoneCount();
        boolean[] isIdle = new boolean[phoneCount];
        for (int i = 0; i < phoneCount; i++) {
            int callState = tm.getCallStateForSlot(i);
            Log.i(TAG , "isInNotCall i = " + i
                    + " callState = " + callState);
            if (callState != TelephonyManager.CALL_STATE_IDLE) {
                return false;
            }
        }
        return true;
    }
}