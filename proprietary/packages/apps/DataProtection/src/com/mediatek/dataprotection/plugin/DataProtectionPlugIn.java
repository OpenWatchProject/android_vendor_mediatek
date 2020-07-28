package com.mediatek.dataprotection.plugin;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceGroup;
import android.util.Log;

import com.mediatek.dataprotection.R;

public class DataProtectionPlugIn extends ContextWrapper {

    private static final String TAG = "DataProtectionPlugIn";

    private Preference mPreference;

    private Context mContext;

    public DataProtectionPlugIn(Context context) {
        super(context);
        mContext = context;
    }

    public void addDataPrf(PreferenceGroup prefGroup) {
        Log.d(TAG, "addDataPrf for dataprotection plugin");
        if (mPreference == null) {
            mPreference = new Preference(prefGroup.getPreferenceManager()
                    .getContext());
            mPreference.setTitle(mContext.getString(R.string.app_name));
            mPreference.setSummary(mContext
                    .getString(R.string.data_protection_summary));
            Intent intent = new Intent();
            intent.setAction("com.mediatek.dataprotection.ACTION_START_MAIN");
            /*
             * must add the flag , or will have the exception: Calling
             * startActivity() from outside of an Activity context requires the
             * FLAG_ACTIVITY_NEW_TASK flag.
             */
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mPreference.setIntent(intent);
        }

        if (prefGroup instanceof PreferenceGroup) {
            prefGroup.addPreference(mPreference);
        }
    }
}
