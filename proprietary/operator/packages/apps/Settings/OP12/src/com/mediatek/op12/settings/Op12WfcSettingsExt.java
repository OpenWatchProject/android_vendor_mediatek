package com.mediatek.op12.settings;

import android.content.Context;
import android.util.Log;

import com.mediatek.settings.ext.DefaultWfcSettingsExt;

/**
 * Plugin implementation for WFC Settings.
 */

public class Op12WfcSettingsExt extends DefaultWfcSettingsExt {

    private static final String TAG = "Op12WfcSettingsExt";

    private Context mContext;

    public Op12WfcSettingsExt(Context context) {
        mContext = context;
    }

    @Override
    public boolean customizedVWZWfcVisable() {
        Log.i(TAG, "customizedVWZWfcVisable TRUE");
        return true;
    }
}
