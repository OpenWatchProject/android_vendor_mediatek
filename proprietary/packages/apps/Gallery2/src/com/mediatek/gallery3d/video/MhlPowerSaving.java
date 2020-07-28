package com.mediatek.gallery3d.video;

import android.content.Context;
import android.view.Window;

import com.mediatek.gallery3d.util.Log;

public class MhlPowerSaving extends PowerSaving {
    private static final String TAG = "VP_MhlPowerSaving";
    private static final int MHL_DELAY_TIME = 10 *1000;

    public MhlPowerSaving(final Context context, final Window window) {
        super(context, window);
    }

    @Override
    protected int getPowerSavingMode() {
        Log.v(TAG, "getWfdPowerSavingMode()");
        return POWER_SAVING_MODE_OFF;
    }

    @Override
    protected int getDelayTime() {
        Log.v(TAG, "getPowerSavingDelay()");
        return MHL_DELAY_TIME;
    }
}
