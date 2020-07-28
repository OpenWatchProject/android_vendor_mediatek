/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

package com.android.browser.preferences;

import android.content.Intent;
import android.content.SharedPreferences;

import android.content.res.Resources;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.webkit.GeolocationPermissions;
import android.webkit.ValueCallback;
import android.webkit.WebStorage;

import com.android.browser.BrowserActivity;
import com.android.browser.BrowserSettings;
import com.android.browser.Extensions;
import com.android.browser.PreferenceKeys;
import com.android.browser.R;
import com.mediatek.browser.ext.IBrowserFeatureIndexExt;
import com.mediatek.browser.ext.IBrowserMiscExt;
import com.mediatek.browser.ext.IBrowserSettingExt;

import java.util.Map;
import java.util.Set;

public class AdvancedPreferencesFragment extends PreferenceFragment
        implements Preference.OnPreferenceChangeListener {

    /// M: Add TAG for XLOG
    private static final String XLOG = "browser/AdvancedPreferencesFragment";
    /// M: Browser setting plugin. @{
    private IBrowserSettingExt mBrowserSettingExt = null;
    private IBrowserMiscExt mBrowserMiscExt = null;
    /// @}

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Load the XML preferences file
        addPreferencesFromResource(R.xml.advanced_preferences);

        PreferenceScreen searchEngineSettings = (PreferenceScreen) findPreference(
                BrowserSettings.PREF_SEARCH_ENGINE);
        searchEngineSettings.setFragment(SearchEngineSettings.class.getName());

        PreferenceScreen websiteSettings = (PreferenceScreen) findPreference(
                PreferenceKeys.PREF_WEBSITE_SETTINGS);
        websiteSettings.setFragment(WebsiteSettingsFragment.class.getName());

        mBrowserSettingExt = Extensions.getSettingPlugin(getActivity());
        Preference e = findPreference(PreferenceKeys.PREF_RESET_DEFAULT_PREFERENCES);
        e.setOnPreferenceChangeListener(this);

        e = findPreference(PreferenceKeys.PREF_SEARCH_ENGINE);
        e.setOnPreferenceChangeListener(this);

        e = findPreference(PreferenceKeys.PREF_PLUGIN_STATE);
        e.setOnPreferenceChangeListener(this);
        updateListPreferenceSummary((ListPreference) e);

        /// M: Remove enable plug-ins due to webview not support
        getPreferenceScreen().removePreference(e);
        /// @}

        /// M: Customize the advanced setting preference. @{
        mBrowserSettingExt.customizePreference(
            IBrowserFeatureIndexExt.CUSTOM_PREFERENCE_ADVANCED, getPreferenceScreen(),
            this, BrowserSettings.getInstance().getPreferences(), this);
        /// @}

        /// M: Operator Feature get the customer load mode. @{
        SharedPreferences mPrefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        CheckBoxPreference cbp = (CheckBoxPreference) findPreference(
                BrowserSettings.PREF_LOAD_PAGE);
        cbp.setChecked(mPrefs.getBoolean(PreferenceKeys.PREF_LOAD_PAGE, true));
        /// @}
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        /// M: Handle the customized activity result. @{
        mBrowserMiscExt = Extensions.getMiscPlugin(getActivity());
        mBrowserMiscExt.onActivityResult(requestCode, resultCode, data, this);
        /// @}
    }

    void updateListPreferenceSummary(ListPreference e) {
        e.setSummary(e.getEntry());
    }

    /*
     * We need to set the PreferenceScreen state in onResume(), as the number of
     * origins with active features (WebStorage, Geolocation etc) could have
     * changed after calling the WebsiteSettingsActivity.
     */
    @Override
    public void onResume() {
        super.onResume();
        final PreferenceScreen websiteSettings = (PreferenceScreen) findPreference(
                PreferenceKeys.PREF_WEBSITE_SETTINGS);
        websiteSettings.setEnabled(false);
        WebStorage.getInstance().getOrigins(new ValueCallback<Map>() {
            @Override
            public void onReceiveValue(Map webStorageOrigins) {
                if ((webStorageOrigins != null) && !webStorageOrigins.isEmpty()) {
                    websiteSettings.setEnabled(true);
                }
            }
        });
        GeolocationPermissions.getInstance().getOrigins(new ValueCallback<Set<String> >() {
            @Override
            public void onReceiveValue(Set<String> geolocationOrigins) {
                if ((geolocationOrigins != null) && !geolocationOrigins.isEmpty()) {
                    websiteSettings.setEnabled(true);
                }
            }
        });
    }

    @Override
    public boolean onPreferenceChange(Preference pref, Object objValue) {
        if (getActivity() == null) {
            // We aren't attached, so don't accept preferences changes from the
            // invisible UI.
            Log.w("PageContentPreferences", "onPreferenceChange called from detached fragment!");
            return false;
        }

        if (pref.getKey().equals(PreferenceKeys.PREF_RESET_DEFAULT_PREFERENCES)) {
            Boolean value = (Boolean) objValue;
            if (value.booleanValue()) {
                startActivity(new Intent(BrowserActivity.ACTION_RESTART, null,
                        getActivity(), BrowserActivity.class));
                return true;
            }
        } else if (pref.getKey().equals(PreferenceKeys.PREF_PLUGIN_STATE)
                || pref.getKey().equals(PreferenceKeys.PREF_SEARCH_ENGINE)) {
            ListPreference lp = (ListPreference) pref;
            lp.setValue((String) objValue);
            updateListPreferenceSummary(lp);
            return false;
        }

        /// M: Update the customized preference item. @{
        mBrowserSettingExt = Extensions.getSettingPlugin(getActivity());
        if (mBrowserSettingExt.updatePreferenceItem(pref, objValue)) {
            return true;
        }
        /// @}

        return false;
    }
}
