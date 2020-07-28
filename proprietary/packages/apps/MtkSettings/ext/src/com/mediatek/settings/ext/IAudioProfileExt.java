package com.mediatek.settings.ext;

import android.support.v14.preference.PreferenceFragment;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceScreen;

public interface IAudioProfileExt {

    /**
     * Add customized Preference.
     * @param preferenceScreen The root PreferenceScreen to add preference
     * @internal
     */
    void addCustomizedPreference(PreferenceScreen preferenceScreen);

    /**
     * Add customized Preference.
     * @param preference The preference that was clicked.
     * @return Whether the click was handled.
     * @internal
     */
    boolean onPreferenceTreeClick(Preference preference);

    /**
     * Call back of AudioProfileSetting fragment resumed.
     * @param fragment The instance of AudioProifileSettings fragment
     * @internal
     */
    void onAudioProfileSettingResumed(PreferenceFragment fragment);

    /**
     * Call back of AudioProfileSettin Activity paused.
     * @param fragment The instance of AudioProifileSettings fragment
     * @internal
     */
    void onAudioProfileSettingPaused(PreferenceFragment fragment);
}
