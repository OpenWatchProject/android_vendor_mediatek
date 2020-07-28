package com.mediatek.settings.op09clib;

import android.content.Context;
import android.net.wifi.WifiManager;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import androidx.preference.PreferenceCategory;
import android.util.Log;

import com.android.mtksettingslib.wifi.WifiTracker;

import com.mediatek.settings.ext.DefaultWifiSettingsExt;

public class Op09WifiSettingsExt extends DefaultWifiSettingsExt {

    private static final String TAG = "OP09WIFISETTINGSEXT";
    private Context mContext;
    private static final String KEY_REFRESH_PREFERENCE = "refersh_preference_key";
    private static final String KEY_CONFIGURE_SETTINGS = "configure_settings";
    private static WifiTracker mWifiTracker = null;
    private Preference mRefreshPreference;
    /**
     * Op09WifiSettingsExt.
     * @param context Context
     */
    public Op09WifiSettingsExt(Context context) {
        super();
        mContext = context;
        Log.d("@M_" + TAG, "WifiSettingsExt mContext = " + mContext);
    }

    @Override
    public void addRefreshPreference(PreferenceScreen screen,
            Object wifiTracker,
            boolean isUiRestricted) {
        Log.d(TAG, "addRefreshPreference, isUiRestricted = " + isUiRestricted);
        if (isUiRestricted) {
            return;
        } else {
            mWifiTracker = (WifiTracker)wifiTracker;
            Preference configPreference
                = screen.findPreference(KEY_CONFIGURE_SETTINGS);
            if (null == mRefreshPreference) {
                mRefreshPreference = new Preference(
                        configPreference.getPreferenceManager().getContext());
                mRefreshPreference.setKey(KEY_REFRESH_PREFERENCE);
                mRefreshPreference.setTitle(mContext.getResources()
                        .getString(R.string.menu_stats_refresh));
                //mRefreshPreference.setOrder(configPreference.getOrder() - 1);
            }
            if (null != mRefreshPreference) {
                if (null != screen.findPreference(KEY_REFRESH_PREFERENCE)) {
                	screen.removePreference(mRefreshPreference);
                }
                screen.addPreference(mRefreshPreference);
                mRefreshPreference.setEnabled(mWifiTracker.isWifiEnabled());
            }
        }
    }

    @Override
    public boolean customRefreshButtonClick(Preference preference) {
        if (null != preference
                && KEY_REFRESH_PREFERENCE.equals(preference.getKey())) {
            Log.d(TAG, "customRefreshButtonClick");
            if (null != mWifiTracker) {
                Log.d(TAG, "customRefreshButtonClick, mWifiTracker.forceScan()");
                //todo need forceScran api ready
                mWifiTracker.forceScan();
            }
            return true;
        }
        return false;
    }

    @Override
    public void customRefreshButtonStatus(boolean checkStatus) {
        Log.d(TAG, "customRefreshButtonStatus checkStatus = " + checkStatus);
        if (null != mRefreshPreference) {
            Log.d(TAG, "customRefreshButtonStatus checkStatus1 = " + checkStatus);
            mRefreshPreference.setEnabled(checkStatus);
        }
    }
}

