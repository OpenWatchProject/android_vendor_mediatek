package com.mediatek.settings.op01;

import android.content.Context;
import android.content.Intent;
import android.os.UserManager;

import com.android.settings.location.LocationEnabler;
import com.android.settingslib.RestrictedSwitchPreference;
import com.android.settingslib.core.AbstractPreferenceController;

import android.provider.Settings;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.util.Log;

import com.android.settingslib.core.lifecycle.Lifecycle;



public class OP01AGPRSPreferenceController extends AbstractPreferenceController implements LocationEnabler.LocationModeChangeListener{

    private static final String KEY_GPS_SETTINGS_BUTTON = "gps_settings_button";
    private static final String TAG = "OP01AGPRSPreferenceController";
    private Preference mPreference;
    private Context mContext;
    private Context mOP01Context;
    protected final LocationEnabler mLocationEnabler;

    public OP01AGPRSPreferenceController(Context context, Context op01Context, Lifecycle lifecycle) {
        super(context);
        mContext = context;
        mOP01Context = op01Context;
        mLocationEnabler = new LocationEnabler(context, this /* listener */, lifecycle);
    }

    @Override
    public boolean isAvailable() {
        return true;
    }

    @Override
    public String getPreferenceKey() {
        return KEY_GPS_SETTINGS_BUTTON;
    }

    public void displayPreference(PreferenceScreen screen) {
        mPreference = (Preference)screen.findPreference(KEY_GPS_SETTINGS_BUTTON);
        Log.i(TAG, "displayPreference  mPreference = " + mPreference);
        if (null != mPreference) {
            mPreference.setVisible(true);
        }
    }

    public void updateState(Preference preference) {
        Log.i(TAG, "updateState");
        if (null != preference) {
            int mode = Settings.Secure.getInt(mContext.getContentResolver(), Settings.Secure.LOCATION_MODE,
                    Settings.Secure.LOCATION_MODE_OFF);
            final UserManager um = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
            boolean restricted = um.hasUserRestriction(UserManager.DISALLOW_SHARE_LOCATION);
            boolean enabled = false;
            if ((mode == Settings.Secure.LOCATION_MODE_HIGH_ACCURACY)
                || (mode == Settings.Secure.LOCATION_MODE_SENSORS_ONLY)) {
                enabled = true;
            }
            Log.i(TAG, "updateState enabled = " + enabled
                    + " restricted = " + restricted);
            preference.setEnabled(enabled && !restricted);
        }
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        Log.i(TAG, "handlePreferenceTreeClick");
        if (KEY_GPS_SETTINGS_BUTTON.equals(preference.getKey())) {
            Log.i(TAG, "handlePreferenceTreeClick true,,,");
            Intent intent = new Intent(mOP01Context, AgpsSettingEnter.class);
            mContext.startActivity(intent);
            return true;
        }
        return false;
    }

    @Override
    public void onLocationModeChanged(int mode, boolean restricted) {
        Log.i(TAG, "onLocationModeChanged");
        if (null != mPreference) {
            updateState(mPreference);
        }
    }

}
