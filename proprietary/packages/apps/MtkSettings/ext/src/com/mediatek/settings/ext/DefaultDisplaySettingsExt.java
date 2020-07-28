package com.mediatek.settings.ext;

import android.content.Context;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceScreen;
import android.util.Log;

/** Customize the Font Setting.
 */
public class DefaultDisplaySettingsExt implements IDisplaySettingsExt {
    private static final String TAG = "DefaultDisplaySettingsExt";
    private Context mContext;
    private static final String KEY_CUSTOM_FONT_SIZE = "custom_font_size";
    /**
     * DefaultDisplaySettingsExt.
     * @param context The Context
     */
    public DefaultDisplaySettingsExt(Context context) {
        mContext = context;
    }

    @Override
    public void addPreference(Context context, PreferenceScreen screen) {
    }

    @Override
    public void removePreference(Context context, PreferenceScreen screen) {
        Preference customFontSizePref = screen.findPreference(KEY_CUSTOM_FONT_SIZE);
        screen.removePreference(customFontSizePref);
        Log.d(TAG, "removePreference KEY_CUSTOM_FONT_SIZE");
    }

    @Override
    public boolean isCustomPrefPresent() {
        return false;
    }

    @Override
    public String[] getFontEntries(String[] defaultStr) {
         return defaultStr;
    }

    @Override
    public String[] getFontEntryValues(String[] defaultStr) {
         return defaultStr;
    }
}
