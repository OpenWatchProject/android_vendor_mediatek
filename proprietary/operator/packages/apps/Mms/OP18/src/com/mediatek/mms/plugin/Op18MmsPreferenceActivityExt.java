package com.mediatek.op18.mms;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceCategory;

import com.mediatek.mms.ext.DefaultOpMmsPreferenceActivityExt;

/**
 *  Op18MmsPreferenceActivityExt.
 *
 */
public class  Op18MmsPreferenceActivityExt extends
        DefaultOpMmsPreferenceActivityExt {
    private  Op18MmsPreference mMmsPreferenceExt;

    /**
     * Construction.
     * @param context Context.
     */
    public  Op18MmsPreferenceActivityExt(Context context) {
        super(context);
        mMmsPreferenceExt = Op18MmsPreference.getInstance(context);
    }

    @Override
    public void setMessagePreferences(Activity hostActivity,
            PreferenceCategory pC, int simCount) {
        mMmsPreferenceExt.configMmsPreference(hostActivity, pC, simCount);
    }

    @Override
    public void restoreDefaultPreferences(Activity hostActivity, SharedPreferences.Editor editor) {
        mMmsPreferenceExt.configMmsPreferenceEditorWhenRestore(hostActivity, editor);
    }
}
