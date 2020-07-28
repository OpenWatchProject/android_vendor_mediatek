package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.SharedPreferences;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;


public class DefaultOpSmsPreferenceActivityExt implements IOpSmsPreferenceActivityExt {

    @Override
    public void setMessagePreferences(Activity hostActivity, PreferenceCategory pC, int simCount) {

    }

    public void restoreDefaultPreferences(Activity hostActivity, SharedPreferences.Editor editor) {

    }

    @Override
    public void onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {

    }

    @Override
    public void changeSingleCardKeyToSimRelated() {

    }

    @Override
    public void onCreate(PreferenceActivity activity) {

    }

    @Override
    public void setMultiCardPreference() {
    }

    @Override
    public boolean addSmsInputModePreference(Preference.OnPreferenceChangeListener listener) {
        return false;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object arg1) {
        return false;
    }
}
