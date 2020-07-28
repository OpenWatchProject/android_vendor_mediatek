package com.mediatek.op01.incallui;
import android.util.Log;

import com.mediatek.incallui.ext.DefaultVilteAutoTestHelperExt;


public class Op01VilteAutoTestHelperExt extends DefaultVilteAutoTestHelperExt {
    private static final String TAG = "Op01VilteAutoTestHelperExt";

    public void log(String msg) {
        Log.d(TAG, msg);
    }

    @Override
    public boolean isAllowVideoCropped() {
        log("op01 plugin allow video cropped");
        return false;
    }
}

