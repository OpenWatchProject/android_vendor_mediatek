package com.debug.loggerui.settings;

import android.os.Bundle;
import android.preference.PreferenceFragment;

import com.debug.loggerui.utils.Utils;

/**
 * @author MTK81255
 *
 */
public class SettingsPreferenceFragement extends PreferenceFragment {
    private static final String TAG = Utils.TAG + "/SettingsPreferenceFragement";

    private ISettingsActivity mSettingsActivity = null;
    private int mResourceId = -1;


    /**
     * @param settingsActivity
     *            ISettingsActivity
     * @param resourceId
     *            int
     * @return SettingsPreferenceFragement
     */
    public static SettingsPreferenceFragement getInstance(ISettingsActivity settingsActivity,
            int resourceId) {
        SettingsPreferenceFragement settingsPreferenceFragement = new SettingsPreferenceFragement();
        settingsPreferenceFragement.mSettingsActivity = settingsActivity;
        settingsPreferenceFragement.mResourceId = resourceId;
        return settingsPreferenceFragement;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Utils.logd(TAG, "onCreate() Start");
        super.onCreate(savedInstanceState);
        if (mResourceId == -1 || mSettingsActivity == null) {
            Utils.logw(TAG, "onCreate() null end!");
            return;
        }
        addPreferencesFromResource(mResourceId);
        mSettingsActivity.findViews();
        mSettingsActivity.initViews();
        mSettingsActivity.setListeners();
        Utils.logd(TAG, "onCreate() end");
    }

}
