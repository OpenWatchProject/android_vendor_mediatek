/*
 * Copyright (C) 2016 The Android Open Source Project
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
 * limitations under the License.
 */

package com.mediatek.settings.datetime;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.location.LocationManager;
import android.provider.Settings;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.util.Log;
import com.android.settings.datetime.AutoTimePreferenceController;
import com.android.settings.datetime.UpdateTimeAndDateCallback;
import com.android.settings.R;
import com.android.settings.RestrictedListPreference;
import com.android.settingslib.RestrictedLockUtilsInternal;

import static android.provider.Settings.Global.AUTO_TIME;
import static com.mediatek.provider.MtkSettingsExt.Global.AUTO_TIME_GPS;

public class AutoTimeExtPreferenceController extends AutoTimePreferenceController implements
        DialogInterface.OnClickListener {

    public interface GPSPreferenceHost extends UpdateTimeAndDateCallback {
        void showGPSConfirmDialog();
    }

    private static final String TAG = "AutoTimeExtPreferenceContr";
    private static final String KEY_AUTO_TIME = "auto_time";
    private final GPSPreferenceHost mCallback;
    private RestrictedListPreference mAutoTimePref;

    public static final int DIALOG_GPS_CONFIRM = 2;
    private static final int AUTO_TIME_NETWORK_INDEX = 0;
    private static final int AUTO_TIME_GPS_INDEX = 1;
    private static final int AUTO_TIME_OFF_INDEX = 2;

    public AutoTimeExtPreferenceController(Context context,
            GPSPreferenceHost callback) {
        super(context, callback);
        mCallback = callback;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        super.displayPreference(screen);
        mAutoTimePref = (RestrictedListPreference) screen.findPreference(KEY_AUTO_TIME);
    }

    @Override
    public void updateState(Preference preference) {
        if (!(preference instanceof RestrictedListPreference)) {
            return;
        }

        if (!((RestrictedListPreference) preference).isDisabledByAdmin())
        {
            ((RestrictedListPreference) preference).setDisabledByAdmin(
                getEnforcedAdminProperty());
        }

        int index = 0;
        if (isAutoTimeNetworkEnabled()) {
            index = AUTO_TIME_NETWORK_INDEX;
        } else if (isAutoTimeGPSEnabled()) {
            index = AUTO_TIME_GPS_INDEX;
        } else {
             index = AUTO_TIME_OFF_INDEX;
        }
        updateSummaryAndValue(index);
    }


    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        int index = Integer.parseInt((String) newValue);
        switch (index) {
            case 0:
                setAutoState(AUTO_TIME, 1);
                setAutoState(AUTO_TIME_GPS, 0);
                break;
            case 1:
                if (!isAutoTimeGPSEnabled()) {
                    mCallback.showGPSConfirmDialog();
                }
                break;
            case 2:
                setAutoState(AUTO_TIME, 0);
                setAutoState(AUTO_TIME_GPS, 0);
            default:
                break;
        }
        updateSummaryAndValue(index);
        mCallback.updateTimeAndDateDisplay(mContext);
        return true;
    }

    public AlertDialog buildGPSConfirmDialog(Activity activity) {
        int msg;
        if (isGpsEnabled()) {
            msg = R.string.gps_time_sync_attention_gps_on;
        } else {
            msg = R.string.gps_time_sync_attention_gps_off;
        }
        return new AlertDialog.Builder(activity)
                .setMessage(activity.getResources().getString(msg))
                .setTitle(R.string.proxy_error)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setPositiveButton(android.R.string.yes, this)
                .setNegativeButton(android.R.string.no, this)
                .create();
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        if (which == DialogInterface.BUTTON_POSITIVE) {
            Log.d(TAG, "Enable GPS time sync");
            if (!isGpsEnabled()) {
                enableGPS();
            }
            setAutoState(AUTO_TIME, 0);
            setAutoState(AUTO_TIME_GPS, 1);
            mAutoTimePref.setValueIndex(AUTO_TIME_GPS_INDEX);
            mAutoTimePref.setSummary(mAutoTimePref.getEntries()[AUTO_TIME_GPS_INDEX]);
            mCallback.updateTimeAndDateDisplay(mContext);
        } else if (which == DialogInterface.BUTTON_NEGATIVE) {
            Log.d(TAG, "DialogInterface.BUTTON_NEGATIVE");
            reSetAutoTimePref();
        }
    }

    public void reSetAutoTimePref() {
        boolean autoTimeEnabled = isAutoTimeNetworkEnabled();
        boolean autoTimeGpsEnabled = isAutoTimeGPSEnabled();
        int index = 0;
        if (autoTimeEnabled) {
            index = AUTO_TIME_NETWORK_INDEX;
        } else if (autoTimeGpsEnabled) {
            index = AUTO_TIME_GPS_INDEX;
        } else {
            index = AUTO_TIME_OFF_INDEX;
        }
        mAutoTimePref.setValueIndex(index);
        mAutoTimePref.setSummary(mAutoTimePref.getEntries()[index]);
        Log.d(TAG, "reset AutoTimePref as cancel the selection, index = " + index);
        mCallback.updateTimeAndDateDisplay(mContext);
    }

    @Override
    public boolean isEnabled() {
        boolean autoEnabled = isAutoTimeNetworkEnabled() | isAutoTimeGPSEnabled();
        Log.d(TAG, "network : " + isAutoTimeNetworkEnabled() + ", GPS :" + isAutoTimeGPSEnabled());
        return autoEnabled;
    }

    private RestrictedLockUtilsInternal.EnforcedAdmin getEnforcedAdminProperty() {
        return RestrictedLockUtilsInternal.checkIfAutoTimeRequired(mContext);
    }

    private boolean isGpsEnabled(){
        return Settings.Secure.isLocationProviderEnabled(
                        mContext.getContentResolver(), LocationManager.GPS_PROVIDER);
    }

    private void enableGPS(){
        Log.d(TAG, "enable GPS");
        Settings.Secure.setLocationProviderEnabled(
                mContext.getContentResolver(), LocationManager.GPS_PROVIDER, true);
    }

    private boolean isAutoTimeNetworkEnabled() {
        return Settings.Global.getInt(mContext.getContentResolver(), AUTO_TIME, 0) > 0;
    }

    private boolean isAutoTimeGPSEnabled() {
        return Settings.Global.getInt(mContext.getContentResolver(), AUTO_TIME_GPS, 0) > 0;
    }

    private void setAutoState(String name, int value) {
        Settings.Global.putInt(mContext.getContentResolver(), name, value);
    }
    private void updateSummaryAndValue(int type) {
        mAutoTimePref.setValueIndex(type);
        mAutoTimePref.setSummary(mAutoTimePref.getEntries()[type]);
    }
}
