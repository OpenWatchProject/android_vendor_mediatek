package com.mediatek.op18.mms;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;

import com.mediatek.mms.ext.DefaultOpSubSelectActivityExt;

/**
 *  Op18SubSelectActivityExt.
 *
 */
public class  Op18SubSelectActivityExt extends DefaultOpSubSelectActivityExt {

    private  Op18MmsPreference mMmsPreferenceExt = null;
    private static final String TAG = "Mms/Op18SubSelectActivityExt";

    /**
     * Construction.
     * @param context Context
     */
    public  Op18SubSelectActivityExt(Context context) {
        mMmsPreferenceExt = Op18MmsPreference.getInstance(context);
    }

    @Override
    public void onCreate(Activity hostActivity) {
        mMmsPreferenceExt.configSelectCardPreferenceTitle(hostActivity);
    }

    @Override
    public boolean onListItemClick(Activity hostActivity, final int subId) {
        return mMmsPreferenceExt.handleSelectCardPreferenceTreeClick(hostActivity, subId);
    }

    @Override
    public boolean getView(CheckBox subCheckBox, String preferenceKey) {
        Log.d(TAG, "getView preferenceKey = " + preferenceKey);
        if (Op18MmsPreference.SMS_VALIDITY_PERIOD_PREFERENCE_KEY.equals(preferenceKey) ||
                Op18MmsPreference.MMS_VALIDITY_PERIOD_PREFERENCE_KEY.equals(preferenceKey)) {
            subCheckBox.setVisibility(View.GONE);
            return false;
        }
        return true;
    }

    public void onSimStateChanged() {
        mMmsPreferenceExt.onSimStateChanged();
    }
}
