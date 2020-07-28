package com.mediatek.gallery3d.video;

import android.content.Context;
import android.view.Window;

import com.mediatek.gallery3d.util.Log;
import com.mediatek.galleryportable.WfdConnectionAdapter;

public class WfdPowerSaving extends PowerSaving {
    private static final String TAG = "VP_WfdPowerSaving";
    private static final int EXTENSION_MODE_LIST_START = 10;
    private static final int EXTENSION_MODE_LIST_END = 12;

    public WfdPowerSaving(final Context context, final Window window) {
        super(context, window);
    }

    @Override
    protected int getPowerSavingMode() {
        int mode = WfdConnectionAdapter.getPowerSavingMode(mContext);
        if ((mode >= EXTENSION_MODE_LIST_START)
                && (mode <= EXTENSION_MODE_LIST_END)) {
            mode = mode - EXTENSION_MODE_LIST_START;
        }
        Log.v(TAG, "getWfdPowerSavingMode(): " + mode);
        return mode;
    }

    @Override
    protected int getDelayTime() {
        int delayTime = WfdConnectionAdapter.getPowerSavingDelay(mContext);
        return delayTime * 1000;
    }

}
