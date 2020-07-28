package com.mediatek.settings.network;

import android.app.ActionBar;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.MenuItem;

import com.android.settings.R;

import com.mediatek.provider.MtkSettingsExt;

/**
 * A {@link PreferenceActivity} that presents a set of application settings. On
 * handset devices, settings are presented as a single list. On tablets,
 * settings are split by category, with category headers shown to the left of
 * the list of settings.
 * <p>
 * See <a href="http://developer.android.com/design/patterns/settings.html">
 * Android Design: Settings</a> for design guidelines and the <a
 * href="http://developer.android.com/guide/topics/ui/settings.html">Settings
 * API Guide</a> for more information on developing a Settings UI.
 */
public class GuardSettings extends PreferenceActivity implements
Preference.OnPreferenceChangeListener {
    //String keys for preference lookup
    public static final String TAG = "GuardSettings";
    private static Context mContext;
    public static final String SUB_ID_EXTRA =
            "com.android.phone.settings.SubscriptionInfoHelper.SubscriptionId";
    public static final String DOMESTIC_VOICE_ROAMING_GUARD = "domestic_voice_roaming_guard";
    public static final String DOMESTIC_DATA_ROAMING_GUARD = "domestic_data_roaming_guard";
    public static final String INTERNATIONAL_VOICE_ROAMING_GUARD =
            "international_voice_roaming_guard";
    public static final String INTERNATIONAL_DATA_ROAMING_GUARD =
            "international_data_roaming_guard";
    public static final String INTERNATIONAL_TEXT_ROAMING_GUARD =
            "international_messaging_roaming_guard";
    private static int sSubId;
    /**
     * Determines whether to always show the simplified settings UI, where
     * settings are presented in a single list. When false, settings are shown
     * as a master/detail two-pane view on tablets. When true, a single pane is
     * shown on tablets.
     */

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Intent intent = getIntent();
        sSubId = intent.getIntExtra(SUB_ID_EXTRA, SubscriptionManager.getDefaultSubscriptionId());
        super.onCreate(savedInstanceState);
        getFragmentManager().beginTransaction()
        .replace(android.R.id.content, new GuardSettingsFragment())
        .commit();
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in
            // onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
        mContext = getApplicationContext();
    }

    /**
     * SettingsFragment class to show settings for Roaming Guard.
     */
    public static class GuardSettingsFragment extends PreferenceFragment {
        private SwitchPreference mDomesticButtonVoiceRoamGuard;
        private SwitchPreference mDomesticButtonDataRoamGuard;
        private SwitchPreference mInternationalButtonVoiceRoamGuard;
        private SwitchPreference mInternationalButtonDataRoamGuard;
        //private RoamingSwitchPreference mInternationalTextRoamGuard;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.mtk_guard_prefernce);
            PreferenceScreen prefSet = getPreferenceScreen();
            mDomesticButtonVoiceRoamGuard =
                    (SwitchPreference) prefSet.findPreference(DOMESTIC_VOICE_ROAMING_GUARD);
            mDomesticButtonVoiceRoamGuard.setOnPreferenceChangeListener(mRoamingGuardListener);
            mDomesticButtonDataRoamGuard =
                    (SwitchPreference) prefSet.findPreference(DOMESTIC_DATA_ROAMING_GUARD);
            mDomesticButtonDataRoamGuard.setOnPreferenceChangeListener(mRoamingGuardListener);
            mInternationalButtonVoiceRoamGuard =
                    (SwitchPreference) prefSet.findPreference(
                    INTERNATIONAL_VOICE_ROAMING_GUARD);
            mInternationalButtonVoiceRoamGuard.setOnPreferenceChangeListener(mRoamingGuardListener);
            mInternationalButtonDataRoamGuard =
                    (SwitchPreference) prefSet.findPreference(
                    INTERNATIONAL_DATA_ROAMING_GUARD);
            mInternationalButtonDataRoamGuard.setOnPreferenceChangeListener(mRoamingGuardListener);
        }

        OnPreferenceChangeListener mRoamingGuardListener =
            new Preference.OnPreferenceChangeListener() {
                @Override
                public boolean onPreferenceChange(Preference preference, Object newValue) {

                    if (((SwitchPreference) preference).isChecked() == (Boolean) newValue) {
                    // State not changed
                        Log.i(TAG, preference.getKey() + " : " + String.valueOf(newValue) +
                                " not changed");
                        return false;
                    // return falseIf you don't want to save the preference.
                    }
                    String prefKey = preference.getKey();
                    int val = ((Boolean) newValue) ? 1 : 0;
                    switch (prefKey) {
                        case DOMESTIC_VOICE_ROAMING_GUARD:
                            Log.d(TAG, "pref: " + prefKey + "new value: " + newValue);
                            android.provider.Settings.Global.putInt(
                                    mContext.getContentResolver(),
                                    MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING_GUARD +
                                    sSubId,
                                    val);

                            Log.d(TAG, "pref: " + prefKey + "new value: " +
                                    android.provider.Settings.Global.getInt(
                                    mContext.getContentResolver(),
                                    MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING_GUARD +
                                    sSubId,
                                    -1) + "sub id" + sSubId);
                            return true;
                        case DOMESTIC_DATA_ROAMING_GUARD:
                            Log.d(TAG, "pref: " + prefKey + "new value: " + newValue);
                            android.provider.Settings.Global.putInt(
                                    mContext.getContentResolver(),
                                    MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING_GUARD + sSubId,
                                    val);
                            RoamingSettingsReceiver.notifyRoamingSettingsChanged(prefKey);
                            return true;
                        case INTERNATIONAL_VOICE_ROAMING_GUARD:
                            Log.d(TAG, "pref: " + prefKey + "new value: " + newValue);
                            android.provider.Settings.Global.putInt(
                                    mContext.getContentResolver(),
                                    MtkSettingsExt.Global.INTERNATIONAL_VOICE_ROAMING_GUARD +
                                    sSubId,
                                    val);
                            Log.d(TAG, "pref: " + prefKey + "new value: " +
                                    android.provider.Settings.Global.getInt(
                                    mContext.getContentResolver(),
                                    MtkSettingsExt.Global.INTERNATIONAL_VOICE_ROAMING_GUARD +
                                    sSubId,
                                    -1) + "sub id" + sSubId);
                            return true;
                        case INTERNATIONAL_DATA_ROAMING_GUARD:
                            Log.d(TAG, "pref: " + prefKey + "new value: " + newValue);
                            android.provider.Settings.Global.putInt(
                                    mContext.getContentResolver(),
                                    MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING_GUARD + sSubId,
                                    val);
                            RoamingSettingsReceiver.notifyRoamingSettingsChanged(prefKey);
                            return true;

                        default:
                            return false;
                    }

                }
        };
    }

    @Override
    public boolean onPreferenceChange(Preference pref, Object newValue) {
        return false;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            // Respond to the action bar's Up/Home button
            case android.R.id.home:
                finish();
                return true;
            default:
                return false;
        }
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        Log.d(TAG, "isValidFragment, fragment=" + fragmentName);
        return super.isValidFragment(fragmentName)
                || GuardSettingsFragment.class.getName().equals(fragmentName);
    }
}
