package com.mediatek.settings.ext;

import android.app.Activity;

import android.content.Context;
import android.content.ContextWrapper;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.util.Log;
import android.widget.ImageView;

/**Dummy implmentation , do nothing.
 */
public class DefaultSettingsMiscExt extends ContextWrapper implements ISettingsMiscExt {
    static final String TAG = "DefaultSettingsMiscExt";
    private static final String KEY_GPS_SETTINGS_BUTTON = "gps_settings_button";
    public DefaultSettingsMiscExt(Context base) {
        super(base);
    }

    public String customizeSimDisplayString(String simString, int slotId) {
        return simString;
    }

    public void initCustomizedLocationSettings(PreferenceScreen root, int order) {
    }

    public void updateCustomizedLocationSettings() {
    }

    public void setFactoryResetTitle(Object obj) {
    }

    public void setTimeoutPrefTitle(Preference pref) {

    }

    @Override
    public void addCustomizedItem(Object targetDashboardCategory, Boolean add) {
        android.util.Log.i(TAG, "DefaultSettingsMisc addCustomizedItem method going");
    }

   @Override
    public void customizeDashboardTile(Object tile, ImageView tileIcon) {
    }

   @Override
    public boolean isWifiOnlyModeSet() {
       return false;
    }

   @Override
    public String getNetworktypeString(String defaultString, int subId) {
        Log.d(TAG, "@M_getNetworktypeString defaultmethod return defaultString = " + defaultString);
        return defaultString;
    }

    @Override
    public String customizeMacAddressString(String macAddressString, String unavailable) {
        return macAddressString;
    }

    @Override
    public boolean doUpdateTilesList(Activity activity, boolean isAdmin, boolean somethingChanged) {
        Log.d(TAG, "default doUpdateTilesList");
        return somethingChanged;
    }

    @Override
    public void doWosFactoryReset() {
    }

    /**
     * For CMCC AGPRS feature.
     * @param controllers OM List controller.
     * @param preferenceController preferenceController.
     */
    public void addPreferenceController(Object controllers,
            Object preferenceController) {
    }

    /**
     * For create new PreferenceController.
     * @param context context
     * @param lifecycle LifeCycle
     * @return PreferenceController.
     */
    @Override
    public Object createPreferenceController(Context context, Object lifecycle) {
        return null;
    }

    /**
     * For add Preference.
     * @param preferenceScreen root PreferenceScreen
     */
    @Override
    public void customizeAGPRS(PreferenceScreen preferenceScreen) {
    }

}

