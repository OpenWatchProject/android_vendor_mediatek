package com.mediatek.op18.settings;

import android.content.Context;
import android.content.SharedPreferences;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceScreen;
import android.util.Log;

import com.mediatek.settings.ext.DefaultDisplaySettingsExt;

public class Op18DisplaySettingsExt extends DefaultDisplaySettingsExt {

    private static final String TAG = "Op18DisplaySettingsExt";
    private Context mContext;

    /**
     * Initialize plugin context.
     * @param context context
     */
    public Op18DisplaySettingsExt(Context context) {
        super(context);
        mContext = context;
        Log.d(TAG, "@M_mContext = " + mContext);
    }

    @Override
    public boolean isCustomPrefPresent() {
          return true;
    }
}