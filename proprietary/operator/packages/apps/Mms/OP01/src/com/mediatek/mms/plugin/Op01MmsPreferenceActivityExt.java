package com.mediatek.mms.plugin;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceCategory;

import com.mediatek.mms.ext.DefaultOpMmsPreferenceActivityExt;

/**
 * Op01MmsPreferenceActivityExt.
 *
 */
public class Op01MmsPreferenceActivityExt extends
        DefaultOpMmsPreferenceActivityExt {
    private Op01MmsPreference mMmsPreferenceExt;

    /**
     * Construction.
     * @param context Context.
     */
    public Op01MmsPreferenceActivityExt(Context context) {
        super(context);
        mMmsPreferenceExt = new Op01MmsPreference(this);
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
