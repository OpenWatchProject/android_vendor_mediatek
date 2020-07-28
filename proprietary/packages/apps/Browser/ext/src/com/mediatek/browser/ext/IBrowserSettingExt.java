package com.mediatek.browser.ext;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.preference.Preference.OnPreferenceChangeListener;
import android.webkit.WebSettings;

public interface IBrowserSettingExt {

    /**
     * Customize the preference
     * @param index the preference index
     * @param prefSc the preference prefSc
     * @param onPreferenceChangeListener the preference change listener
     * @param sharedPref the shared preference
     * @param prefFrag the preferenceFragment prefFrag
     */
    void customizePreference(int index, PreferenceScreen prefSc,
            OnPreferenceChangeListener onPreferenceChangeListener,
            SharedPreferences sharedPref, PreferenceFragment prefFrag);

    /**
     * Update the preference item
     * @param pref the preference update needed
     * @param objValue the preference value
     * @return true to update the state of the preference with the new value
     */
    boolean updatePreferenceItem(Preference pref, Object objValue);

    /**
     * Get the customized homepage website url
     * @return the homepage url
     */
    String getCustomerHomepage();

    /**
     * Get the default download folder name
     * @return the folder name
     */
    String getDefaultDownloadFolder();

    /**
     * Get the customized search engine
     * @param pref the shared preference
     * @param context the context
     * @return the search engine name
     */
    String getSearchEngine(SharedPreferences pref, Context context);

    /**
     * Get the customized user agent
     * @param defaultUA the default user agent
     * @return the customized user agent
     */
    String getOperatorUA(String defaultUA);

    /**
     * Set the only landscape mode
     * @param pref the shared preference
     * @param activity the activity
     */
    void setOnlyLandscape(SharedPreferences pref, Activity activity);

    /**
     * Set the standard font family
     * @param settings the websettings
     * @param pref the shared preference
     */
    void setStandardFontFamily(WebSettings settings, SharedPreferences pref);

    /**
     * Set the customized text encoding choices
     * @param pref the list preference
     */
    void setTextEncodingChoices(ListPreference pref);

}
