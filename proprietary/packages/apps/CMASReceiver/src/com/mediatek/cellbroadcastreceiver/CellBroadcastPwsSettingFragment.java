package com.mediatek.cellbroadcastreceiver;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.util.Log;

public class CellBroadcastPwsSettingFragment extends PreferenceFragment
                                            implements OnPreferenceChangeListener {

    private static final String TAG = "[CMAS]CellBroadcastPwsSettingFragment";

    private Context mContext;
    private CheckBoxPreference mIdentifiersCheckBox;
    private CheckBoxPreference mAlertMessageCheckBox;
    private CheckBoxPreference mRmtCheckBox;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = getContext();
        addPreferencesFromResource(R.xml.preferences_pws);

        mIdentifiersCheckBox = (CheckBoxPreference)
            findPreference("enable_message_identifiers");
        mIdentifiersCheckBox.setOnPreferenceChangeListener(this);

        mAlertMessageCheckBox = (CheckBoxPreference)
            findPreference("enable_cmas_alerts_messages");
        mAlertMessageCheckBox.setOnPreferenceChangeListener(this);

        mRmtCheckBox = (CheckBoxPreference)
            findPreference("enable_cmas_rmt_support");
        mRmtCheckBox.setOnPreferenceChangeListener(this);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        // TODO Auto-generated method stub
        boolean value = (Boolean) newValue;
        SharedPreferences prefs =
                PreferenceManager.getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = prefs.edit();
        String key = preference.getKey();
        editor.putBoolean(key, value);
        editor.commit();
        Log.i(TAG, "onPreferenceChange key = " + key + ", value = " + value);
        CellBroadcastReceiver.startConfigService(mContext);
        return true;
    }

}
