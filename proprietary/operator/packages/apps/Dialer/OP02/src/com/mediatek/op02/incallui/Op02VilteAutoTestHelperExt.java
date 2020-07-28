package com.mediatek.op02.incallui;
import android.util.Log;

import com.mediatek.incallui.ext.DefaultVilteAutoTestHelperExt;


public class Op02VilteAutoTestHelperExt extends DefaultVilteAutoTestHelperExt {
    private static final String TAG = "Op02VilteAutoTestHelperExt";

    public void log(String msg) {
        Log.d(TAG, msg);
    }

    @Override
    public boolean isAllowVideoCropped() {
        log("op02 plugin allow video cropped");
        return false;
    }
}

