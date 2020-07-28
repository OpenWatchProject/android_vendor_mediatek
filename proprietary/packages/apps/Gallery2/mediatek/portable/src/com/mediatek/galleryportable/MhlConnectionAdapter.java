package com.mediatek.galleryportable;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemClock;

public class MhlConnectionAdapter {
    private static final String TAG = "VP_MhlAdapter";
    private static final boolean DEBUG = false;

    private static final int MHL_DISPLAY_STATE_CONNECTED = 1;
    private static final int MHL_DISPLAY_STATE_NOT_CONNECTED = 0;

    public interface StateChangeListener {
        void stateNotConnected();
        void stateConnected();
    }

    private static boolean sIsMhlApiExisted = false;
    private static boolean sHasChecked = false;

    private StateChangeListener mStateChangeListener;

    private static boolean isMhlApiExisted() {
        if (!sHasChecked) {
            long checkStart = SystemClock.elapsedRealtime();
            try {
                Intent.class.getDeclaredField("ACTION_HDMI_PLUG");
                sIsMhlApiExisted = true;
            } catch (NoSuchFieldException e) {
                sIsMhlApiExisted = false;
                Log.e(TAG, e.toString());
            }
            sHasChecked = true;
            Log.d(TAG, "isMhlApiExisted, sIsMhlApiExisted = " + sIsMhlApiExisted);
            if (DEBUG) {
                Log.d(TAG, "isMhlApiExisted elapsed time = "
                        + (SystemClock.elapsedRealtime() - checkStart));
            }
        }
        return sIsMhlApiExisted;
    }

    public MhlConnectionAdapter(StateChangeListener stateChangeListener) {
        this.mStateChangeListener = stateChangeListener;
    }

    public void registerReceiver(Context context) {
    }

    public void unRegisterReceiver(Context context) {
    }

    private static String getIntentConst(String field) {
        try {
            return (String)Intent.class.getField(field).get(null);
        } catch (Exception e) {
            android.util.Log.w("getConst occur error:", e);
        }
        return null;
    }

}
