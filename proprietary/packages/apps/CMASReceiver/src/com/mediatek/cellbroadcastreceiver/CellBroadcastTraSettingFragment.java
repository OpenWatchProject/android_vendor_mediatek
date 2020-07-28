package com.mediatek.cellbroadcastreceiver;

import java.io.IOException;
import java.util.List;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.Preference.OnPreferenceChangeListener;
import android.util.Log;

public class CellBroadcastTraSettingFragment extends PreferenceFragment
                                            implements OnPreferenceChangeListener {

    private static final String TAG = "[CMAS]CellBroadcastTraSettingFragment";

    private Context mContext;
    private CheckBoxPreference mAmberCheckBox;
    private CheckBoxPreference mRmtCheckBox;
    private CheckBoxPreference mSpeechCheckBox;
    private CheckBoxPreference mRepeatCheckBox;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = getContext();
        addPreferencesFromResource(R.xml.preferences_tra);

        mAmberCheckBox = (CheckBoxPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_AMBER_ALERTS);
        mAmberCheckBox.setOnPreferenceChangeListener(this);

        mRmtCheckBox = (CheckBoxPreference)
            findPreference(CellBroadcastConfigService.ENABLE_CMAS_RMT_SUPPORT);
        mRmtCheckBox.setOnPreferenceChangeListener(this);

        mSpeechCheckBox = (CheckBoxPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_ALERT_SPEECH);
        mSpeechCheckBox.setOnPreferenceChangeListener(this);

        mRepeatCheckBox = (CheckBoxPreference)
            findPreference(CheckBoxAndSettingsPreference.KEY_ENABLE_REPEAT_ALERT);
        mRepeatCheckBox.setOnPreferenceChangeListener(this);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.i(TAG, "onPause");

        AlertTonePreviewDialog alertPreviewDialog = (AlertTonePreviewDialog)
                findPreference("cmas_preview_alert_tone");
        alertPreviewDialog.onDialogClosed(false);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        // TODO Auto-generated method stub
        boolean value = (Boolean) newValue;
        SharedPreferences prefs =
                PreferenceManager.getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = prefs.edit();
        String key = preference.getKey();
        editor.putBoolean(key,value);
        editor.commit();
        Log.i(TAG, "onPreferenceChange key = " + key + ",value = " + value);
        if (key.equals("enable_cmas_amber_threat_alerts") ||
                key.equals("enable_cmas_rmt_support")) {
            CellBroadcastReceiver.startConfigService(mContext);
        }
        return true;
    }

}
