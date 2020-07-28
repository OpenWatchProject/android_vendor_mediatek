package com.mediatek.miravision.ui;

import static android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE;
import static android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE_AUTOMATIC;
import static android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL;
import android.app.Activity;
import android.os.Bundle;
import android.preference.SwitchPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.Preference.OnPreferenceChangeListener;
import android.provider.Settings;
import android.util.Log;

import com.mediatek.miravision.setting.MiraVisionJni;
import com.mediatek.miravision.utils.CurrentUserTracker;

public class AalSettingsFragment extends PreferenceFragment implements OnPreferenceChangeListener {

    private static final String TAG = "Miravision/AalSettingsFragment";

    // Preference keys
    private static final String KEY_LIGHT_SENSITIVE_PREF = "light_sensitive_bright_pref";
    private static final String KEY_BRIGHTNESS_PREF = "brightness_pref";
    private static final String KEY_SENSITIVITY_PREF = "sensitivity_pref";
    private static final String KEY_CONTENT_SENSITIVE_PREF = "content_sensitive_bright_pref";
    private static final String KEY_READABILITY_ENHANCER_PREF = "readability_enhancer_pref";

    private SwitchPreference mLightSensitivePref;
    private Preference mBrightnessPref;
    private Preference mSensitivityPref;
    private SwitchPreference mContentSensitivePref;
    private SwitchPreference mReadabilityEnhancerPref;
    private CurrentUserTracker mUserTracker;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.aal_settings);
        initializeAllPreferences();
        Activity activity = getActivity();
        mUserTracker = new CurrentUserTracker(activity) {
            @Override
            public void onUserSwitched(int newUserId) {
                updateSwitchPrefStatus();
            }
        };

    }

    @Override
    public void onResume() {
        super.onResume();
        updateSwitchPrefStatus();
        updatePreferenceStatus(mLightSensitivePref.isChecked());
    }

    @Override
    public void onDestroy() {
        mUserTracker.stopTracking();
        super.onDestroy();
    }

    private void initializeAllPreferences() {
        mLightSensitivePref = (SwitchPreference) findPreference(KEY_LIGHT_SENSITIVE_PREF);
        mLightSensitivePref.setOnPreferenceChangeListener(this);
        mBrightnessPref = findPreference(KEY_BRIGHTNESS_PREF);
        mSensitivityPref = findPreference(KEY_SENSITIVITY_PREF);
        mContentSensitivePref = (SwitchPreference) findPreference(KEY_CONTENT_SENSITIVE_PREF);
        mContentSensitivePref.setOnPreferenceChangeListener(this);
        mReadabilityEnhancerPref = (SwitchPreference) findPreference(KEY_READABILITY_ENHANCER_PREF);
        mReadabilityEnhancerPref.setOnPreferenceChangeListener(this);
    }

    private void updateSwitchPrefStatus() {
        if (mLightSensitivePref != null) {
            int brightnessMode = Settings.System.getInt(getActivity().getContentResolver(),
                    SCREEN_BRIGHTNESS_MODE, SCREEN_BRIGHTNESS_MODE_MANUAL);
            mLightSensitivePref.setChecked(brightnessMode != SCREEN_BRIGHTNESS_MODE_MANUAL);
        }
        int aalFunc = MiraVisionJni.getAALFunction();
        Log.d(TAG, "updateSwitchPrefStatus, mode AAL function = " + aalFunc);
        mContentSensitivePref.setChecked((aalFunc & MiraVisionJni.AAL_FUNC_CABC) != 0);
        mReadabilityEnhancerPref.setChecked((aalFunc & MiraVisionJni.AAL_FUNC_DRE) != 0);
        ///M:ALPS03683084 showing menu according to light sensor
        if (!isLightSensorAvailable()){
            getPreferenceScreen().removePreference(mReadabilityEnhancerPref);
        }
    }
    public boolean isLightSensorAvailable() {
        return getActivity().getResources().getBoolean(
                com.android.internal.R.bool.config_automatic_brightness_available);
    }

    private void updatePreferenceStatus(boolean status) {
        mSensitivityPref.setEnabled(status);
        mBrightnessPref.setEnabled(!status);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Log.d(TAG, "onPreferenceChange preference: " + preference.getKey() + " newValue: "
                + newValue);
        boolean returnVaule = true;
        boolean status = (Boolean) newValue;
        if (preference == mLightSensitivePref) {
            Settings.System.putInt(getActivity().getContentResolver(), SCREEN_BRIGHTNESS_MODE,
                    status ? SCREEN_BRIGHTNESS_MODE_AUTOMATIC : SCREEN_BRIGHTNESS_MODE_MANUAL);
            updatePreferenceStatus(status);
        } else if (preference == mContentSensitivePref) {
            setAalFunction(MiraVisionJni.AAL_FUNC_CABC, status);
        } else if (preference == mReadabilityEnhancerPref) {
            setAalFunction(MiraVisionJni.AAL_FUNC_DRE, status);
        } else {
            returnVaule = false;
        }
        return returnVaule;
    }

    public static void setAalFunction(int function, boolean status) {
        int oldAal = MiraVisionJni.getAALFunction();
        Log.d(TAG, "setAalFunction, oldAal function = " + oldAal);
        int newAal = status ? (oldAal | function) : (oldAal & ~function);
        Log.d(TAG, "setAalFunction, newAal function = " + newAal);
        MiraVisionJni.setAALFunction(newAal);
    }
}
