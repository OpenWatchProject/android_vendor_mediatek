package com.mediatek.op07.phone;

import android.content.Context;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.util.Log;

import com.android.phone.CallFeaturesSetting;
import com.mediatek.phone.ext.DefaultCallFeaturesSettingExt;


public class OP07CallFeaturesSettingExt extends DefaultCallFeaturesSettingExt {
    private static final String TAG = "Op07CallFeaturesSettingExt";
    private static final String KEY_WFC_SETTINGS = "button_wifi_calling_settings_key";
    private Context mContext;

    public OP07CallFeaturesSettingExt(Context context) {
        mContext = context;
    }

    @Override
    public void initOtherCallFeaturesSetting(PreferenceActivity activity, Object phone) {
        Log.d(TAG, "initOtherCallFeaturesSetting");
        if (isEntitlementEnabled()) {
            PreferenceScreen prefScreen = ((CallFeaturesSetting) activity).getPreferenceScreen();
            Preference wfcSettingsPreference = prefScreen.findPreference(KEY_WFC_SETTINGS);
            Log.d(TAG, "remove wfc preference:" + wfcSettingsPreference);
            if (wfcSettingsPreference != null) {
                Log.d(TAG, "Removing AOSP WFC preference");
                prefScreen.removePreference(wfcSettingsPreference);
            }
        }
    }

    public static boolean isEntitlementEnabled() {
        boolean isEntitlementEnabled = (1 == SystemProperties.getInt
                ("persist.vendor.entitlement_enabled", 1) ? true : false);
        Log.d(TAG, "isEntitlementEnabled:" + isEntitlementEnabled);

        return isEntitlementEnabled;

    }
}
