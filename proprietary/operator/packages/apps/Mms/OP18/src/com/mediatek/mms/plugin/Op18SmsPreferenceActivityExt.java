package com.mediatek.op18.mms;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceCategory;

import com.mediatek.mms.ext.DefaultOpSmsPreferenceActivityExt;

/**
 *  Op18SmsPreferenceActivityExt.
 *
 */
public class  Op18SmsPreferenceActivityExt extends
        DefaultOpSmsPreferenceActivityExt {

    private static final String TAG = "Mms/Op18SmsPreferenceActivityExt";

    private  Op18MmsPreference mMmsPreferenceExt;

    /**
     * Construction.
     * @param context Context
     */
    public  Op18SmsPreferenceActivityExt(Context context) {
        mMmsPreferenceExt = new  Op18MmsPreference(context);
    }

    @Override
    public void setMessagePreferences(Activity hostActivity,
            PreferenceCategory pC, int simCount) {
        mMmsPreferenceExt.configSmsPreference(hostActivity, pC, simCount);
    }

    @Override
    public void restoreDefaultPreferences(Activity hostActivity, SharedPreferences.Editor editor) {
        mMmsPreferenceExt.configSmsPreferenceEditorWhenRestore(hostActivity, editor);
    }
}
