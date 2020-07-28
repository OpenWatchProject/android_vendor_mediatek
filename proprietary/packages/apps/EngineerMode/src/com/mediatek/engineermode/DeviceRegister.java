package com.mediatek.engineermode;

import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;

import com.mediatek.engineermode.Elog;

public class DeviceRegister extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener {

    private static final String TAG = "DeviceRegister";

    private static final String KEY_SMS_REGISTER_SWITCH = "ct_sms_register_switch";

    private static final String PROPERTY_KEY_SMSREG = "persist.vendor.radio.selfreg";
    private static final String TURN_ON = "On";
    private static final String TURN_OFF = "Off";
    private ListPreference mListPreference;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.device_register);

        mListPreference = (ListPreference) findPreference(KEY_SMS_REGISTER_SWITCH);
        mListPreference.setOnPreferenceChangeListener(this);
        final int savedCTAValue = getSavedCTAValue();
        final String summary = savedCTAValue == 1 ? TURN_ON : TURN_OFF;
        mListPreference.setSummary(summary);
        mListPreference.setValue(String.valueOf(savedCTAValue));
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference.getKey().equals(KEY_SMS_REGISTER_SWITCH)) {
            try {
                setCTAValue((String) newValue);
            } catch (NumberFormatException ex) {
                Elog.e(TAG, "setCTAValue NumberFormatException");
            }
            final boolean isEnabled = getSavedCTAValue() == 1;
            mListPreference.setValue(isEnabled ? "1" : "0");
            final String summary = isEnabled ? TURN_ON : TURN_OFF;
            mListPreference.setSummary(summary);
        }
        return false;
    }

    private int getSavedCTAValue() {
        String config = getSelfRegConfig();
        if (config.charAt(0) == '1') {
            return 1;
        }
        return 0;
    }

    private void setCTAValue(String cta) {
        String newString = cta + getSelfRegConfig().charAt(1);
        SystemProperties.set(PROPERTY_KEY_SMSREG, newString);
        Elog.i(TAG, "save CTA [" + newString + "]");
    }

    /**
     * get value of persist.vendor.radio.selfreg.
     *   index 0: for devreg via SMS. (1: enable, 0: disable)
     *   index 1: for ct4g via http.  (1: enable, 0: disable)
     */
    private String getSelfRegConfig() {
        String config = SystemProperties.get(PROPERTY_KEY_SMSREG, "11");
        if (!config.equals("11") && !config.equals("10") && !config.equals("01")
                && !config.equals("00")) {
            config = "11";
        }
        Elog.i(TAG, PROPERTY_KEY_SMSREG + ": " + config);
        return config;
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        // TODO Auto-generated method stub
        Elog.i(TAG, "fragmentName is " + fragmentName);
        return false;
    }

}
