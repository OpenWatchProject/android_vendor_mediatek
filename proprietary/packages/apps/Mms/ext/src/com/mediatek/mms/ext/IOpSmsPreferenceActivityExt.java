package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.SharedPreferences;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;

public interface IOpSmsPreferenceActivityExt {
    static final String SMS_SETTINGS = "pref_key_sms_settings";
    static final String SMS_MANAGE_SIM_MESSAGES = "pref_key_manage_sim_messages";
    static final String SMS_INPUT_MODE = "pref_key_sms_input_mode";
    static final String SETTING_INPUT_MODE = "Automatic";

    /**
     * @internal
     */
    void onCreate(PreferenceActivity activity);
    /**
     * @internal
     */
    void setMessagePreferences(Activity hostActivity, PreferenceCategory pC, int simCount);
    /**
     * @internal
     */
    void restoreDefaultPreferences(Activity hostActivity, SharedPreferences.Editor editor);
    /**
     * @internal
     */
    void onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference);
    /**
     * @internal
     */
    void changeSingleCardKeyToSimRelated();
    /**
     * @internal
     */
    void setMultiCardPreference();
    /**
     * @internal
     */
    boolean addSmsInputModePreference(Preference.OnPreferenceChangeListener listener);
    /**
     * @internal
     */
    boolean onPreferenceChange(Preference preference, Object arg1);
}
