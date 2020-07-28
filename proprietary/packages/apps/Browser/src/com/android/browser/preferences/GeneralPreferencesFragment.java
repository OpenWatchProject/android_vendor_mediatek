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

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.res.Resources;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.text.InputType;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import com.android.browser.BrowserFeatureOption;
import com.android.browser.BrowserPreferencesPage;
import com.android.browser.BrowserSettings;
import com.android.browser.PreferenceKeys;
import com.android.browser.R;
import com.android.browser.UrlUtils;
import com.android.browser.homepages.HomeProvider;
import com.android.browser.sitenavigation.SiteNavigation;

public class GeneralPreferencesFragment extends PreferenceFragment
        implements Preference.OnPreferenceChangeListener {

    static final String TAG = "PersonalPreferencesFragment";

    public static final String BLANK_URL = "about:blank";
    public static final String CURRENT = "current";
    public static final String BLANK = "blank";
    public static final String DEFAULT = "default";
    public static final String MOST_VISITED = "most_visited";
    public static final String OTHER = "other";
    // M: add for site navigation
    public static final String SITE_NAVIGATION = "site_navigation";

    public static final String PREF_HOMEPAGE_PICKER = "homepage_picker";

    String[] mChoices, mValues;
    String mCurrentPage;

    /**
     * M: Add site navigation selection in settings.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Resources res = getActivity().getResources();
        // M: Add site navigation selection, if support site navigation @{
        if (BrowserFeatureOption.BROWSER_SITE_NAVIGATION_SUPPORT) {
             mChoices = res.getStringArray(R.array.pref_homepage_choices_site_navigation);
             mValues = res.getStringArray(R.array.pref_homepage_values_site_navigation);
        } else {
        mChoices = res.getStringArray(R.array.pref_homepage_choices);
        mValues = res.getStringArray(R.array.pref_homepage_values);
        }
        // @}
        mCurrentPage = getActivity().getIntent()
                .getStringExtra(BrowserPreferencesPage.CURRENT_PAGE);

        // Load the XML preferences file
        // M: Add site navigation selection, if support site navigation @{
        if (BrowserFeatureOption.BROWSER_SITE_NAVIGATION_SUPPORT) {
            addPreferencesFromResource(R.xml.general_preferences_site_navigation);
        } else {
        addPreferencesFromResource(R.xml.general_preferences);
        }
        // @}
        ListPreference pref = (ListPreference) findPreference(PREF_HOMEPAGE_PICKER);
        String keyValue = pref.getValue();
        if (keyValue == null) {
            pref.setValue(DEFAULT);
        } else {
            if (changeHomapagePicker(keyValue)) {
                pref.setValue(getHomepageValue());
            }
        }
        keyValue = pref.getValue();
        pref.setSummary(getHomepageSummary(keyValue));
        pref.setOnPreferenceChangeListener(this);

        /// M: Remove auto-fill due to the function is phase out
        getPreferenceScreen().removePreference(findPreference(PreferenceKeys.PREF_AUTOFILL_TITLE));
        /// @}
    }

    private boolean changeHomapagePicker(String keyValue) {
        BrowserSettings settings = BrowserSettings.getInstance();
        String homepage = settings.getHomePage();

        if (keyValue.equals(DEFAULT)) {
            String defaultHomepage = BrowserSettings.getFactoryResetHomeUrl(getActivity());
            if (TextUtils.equals(defaultHomepage, homepage)) {
                return false;
            }
        }

        if (keyValue.equals(CURRENT)) {
            if (TextUtils.equals(mCurrentPage, homepage)) {
                return false;
            }
        }
        if (keyValue.equals(OTHER)) {
            return false;
        }
        return true;
    }

    /**
     * M: Save if settings change.
     */
    @Override
    public boolean onPreferenceChange(Preference pref, Object objValue) {
        if (getActivity() == null) {
            // We aren't attached, so don't accept preferences changes from the
            // invisible UI.
            Log.w("PageContentPreferences", "onPreferenceChange called from detached fragment!");
            return false;
        }

        if (pref.getKey().equals(PREF_HOMEPAGE_PICKER)) {
            BrowserSettings settings = BrowserSettings.getInstance();
            if (CURRENT.equals(objValue)) {
                settings.setHomePage(mCurrentPage);
            } else if (BLANK.equals(objValue)) {
                settings.setHomePage(BLANK_URL);
            } else if (DEFAULT.equals(objValue)) {
                settings.setHomePage(BrowserSettings.getFactoryResetHomeUrl(
                        getActivity()));
            } else if (MOST_VISITED.equals(objValue)) {
                settings.setHomePage(HomeProvider.MOST_VISITED);
                // M: If selected site navigaton, set homepage site navigation @{
            } else if (BrowserFeatureOption.BROWSER_SITE_NAVIGATION_SUPPORT &&
                    SITE_NAVIGATION.equals(objValue)) {
                settings.setHomePage(SiteNavigation.SITE_NAVIGATION);
                // @}
            } else if (OTHER.equals(objValue)) {
                promptForHomepage((ListPreference) pref, (String) objValue);
                return false;
            }
            pref.setSummary(getHomepageSummary((String) objValue));
        }

        return true;
    }

    void promptForHomepage(final ListPreference pref, final String keyValue) {
        final BrowserSettings settings = BrowserSettings.getInstance();
        final EditText editText = new EditText(getActivity());
        editText.setInputType(InputType.TYPE_CLASS_TEXT
                | InputType.TYPE_TEXT_VARIATION_URI);
        editText.setLongClickable(false);
        editText.setText(settings.getHomePage());
        editText.setSelectAllOnFocus(true);
        editText.setSingleLine(true);
        editText.setImeActionLabel(null, EditorInfo.IME_ACTION_DONE);
        final AlertDialog dialog = new AlertDialog.Builder(getActivity())
                .setView(editText)
                .setPositiveButton(android.R.string.ok, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        String homepage = editText.getText().toString().trim();
                        homepage = UrlUtils.smartUrlFilter(homepage);
                        settings.setHomePage(homepage);
                        pref.setValue(keyValue);
                        pref.setSummary(getHomepageSummary(keyValue));
                    }
                })
                .setNegativeButton(android.R.string.cancel, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.cancel();
                    }
                })
                .setTitle(R.string.pref_set_homepage_to)
                .create();
        editText.setOnEditorActionListener(new OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    dialog.getButton(AlertDialog.BUTTON_POSITIVE).performClick();
                    return true;
                }
                return false;
            }
        });
        dialog.getWindow().setSoftInputMode(
                WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
        dialog.show();
    }

    /**
     * M: If homepage settings is Site navigation, return Site Navigation.
     */
    String getHomepageValue() {
        BrowserSettings settings = BrowserSettings.getInstance();
        String homepage = settings.getHomePage();
        if (TextUtils.isEmpty(homepage) || BLANK_URL.endsWith(homepage)) {
            return BLANK;
        }
        if (HomeProvider.MOST_VISITED.equals(homepage)) {
            return MOST_VISITED;
        }
        // M: if homepage settings is Site navigation, retunr site navigation @{
        if (BrowserFeatureOption.BROWSER_SITE_NAVIGATION_SUPPORT &&
                    SiteNavigation.SITE_NAVIGATION.equals(homepage)) {
            return SITE_NAVIGATION;
        }
        // @}
        String defaultHomepage = BrowserSettings.getFactoryResetHomeUrl(
                getActivity());
        if (TextUtils.equals(defaultHomepage, homepage)) {
            return DEFAULT;
        }
        if (TextUtils.equals(mCurrentPage, homepage)) {
            return CURRENT;
        }
        return OTHER;
    }

    String getHomepageSummary(String keyValue) {

        if (keyValue == null || keyValue.length() <= 0) {
            return null;
        }

        BrowserSettings settings = BrowserSettings.getInstance();
        if (settings.useMostVisitedHomepage()) {
            return getHomepageLabel(MOST_VISITED);
        }
        String homepage = settings.getHomePage();
        if (TextUtils.isEmpty(homepage) || BLANK_URL.equals(homepage)) {
            keyValue = BLANK;
        }
        if (keyValue.equals(CURRENT) || keyValue.equals(OTHER)) {
            return homepage;
        }

        return getHomepageLabel(keyValue);
    }

    String getHomepageLabel(String value) {
        for (int i = 0; i < mValues.length; i++) {
            if (value.equals(mValues[i])) {
                return mChoices[i];
            }
        }
        return null;
    }

    @Override
    public void onResume() {
        super.onResume();

    }
}
