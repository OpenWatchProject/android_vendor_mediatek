package com.mediatek.op09clib.incallui;
import android.util.Log;

import com.mediatek.incallui.ext.DefaultVilteAutoTestHelperExt;

public class Op09ClibVilteAutoTestHelperExt extends DefaultVilteAutoTestHelperExt {
    private static final String TAG = "Op09ClibVilteAutoTestHelperExt";

    public void log(String msg) {
        Log.d(TAG, msg);
    }

    @Override
    public boolean isAllowVideoCropped() {
        log("op09 clib plugin not allow video cropped");
        return false;
    }
}

