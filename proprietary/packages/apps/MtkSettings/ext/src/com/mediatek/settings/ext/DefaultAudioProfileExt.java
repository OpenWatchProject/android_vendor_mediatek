package com.mediatek.settings.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.support.v14.preference.PreferenceFragment;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceScreen;

/**
 * The default implement for IAudioProfileExt.
 */
public class DefaultAudioProfileExt extends ContextWrapper implements
        IAudioProfileExt {

    /**
     * Constructor.
     * @param context Application context
     */
    public DefaultAudioProfileExt(Context context) {
        super(context);
    }

    @Override
    public void addCustomizedPreference(PreferenceScreen preferenceScreen) {
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        return false;
    }

    @Override
    public void onAudioProfileSettingResumed(PreferenceFragment fragment) {
    }

    @Override
    public void onAudioProfileSettingPaused(PreferenceFragment fragment) {
    }
}
