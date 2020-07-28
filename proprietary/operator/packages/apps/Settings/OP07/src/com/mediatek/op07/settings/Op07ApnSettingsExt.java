package com.mediatek.op07.settings;

import android.content.Context;
import android.database.ContentObserver;
import android.os.Handler;
import android.provider.Settings;
import android.util.Log;
import androidx.preference.PreferenceFragmentCompat;

import com.android.settings.network.ApnSettings;
import com.mediatek.settings.ext.DefaultApnSettingsExt;
import com.mediatek.settings.ext.IApnSettingsExt;

/**
 * Plugin implementation for APN Settings plugin
 */

public class Op07ApnSettingsExt extends DefaultApnSettingsExt {

    private static final String TAG = "Op07ApnSettingsExt";
    public static final String PREFERRED_APN_URI =
        "content://telephony/carriers/preferapn";
    private Context mContext;
    private PreferenceFragmentCompat mApnSettings = null;
    private ContentObserver mContentObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            Log.d(TAG, "Content Observer changed for Preferred APN, so update APN UI");
            ((ApnSettings)mApnSettings).fillList();
        }
    };


    /** Constructor.
     * @param context context
     */
    public Op07ApnSettingsExt(Context context) {
        super();
        mContext = context;
    }

    /**
     * Init APN Settings object.
     * @return
     */
    @Override
    public void initTetherField(PreferenceFragmentCompat pref) {
        Log.d(TAG, "initTetherField, init APN Settings object");
        mApnSettings = pref;
    }

    /**
     * Should select first APN when reset.
     * @return
     */
    @Override
    public boolean shouldSelectFirstApn() {
        Log.d(TAG, "shouldSelectFirstApn");
        return false;
    }

    /**
     * Apn Settings event.
     * @param event resume/pause
     */
    @Override
    public void onApnSettingsEvent(int event) {
        Log.d(TAG, "onApnSettingsEvent with event : " + event);
        if (event == IApnSettingsExt.RESUME) {
            mContext.getContentResolver().registerContentObserver(
                    Settings.Global.getUriFor(PREFERRED_APN_URI),
                    true,
                    mContentObserver);
        } else if (event == IApnSettingsExt.PAUSE) {
            mContext.getContentResolver().unregisterContentObserver(mContentObserver);
        }
    }
}
