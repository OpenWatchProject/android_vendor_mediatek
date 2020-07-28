package com.mediatek.mms.plugin;

import android.app.Activity;
import android.content.Context;
import android.preference.PreferenceCategory;

import com.mediatek.mms.ext.DefaultOpGeneralPreferenceActivityExt;

/**
 * Op01GeneralPreferenceActivityExt.
 *
 */
public class Op01GeneralPreferenceActivityExt extends
        DefaultOpGeneralPreferenceActivityExt {

    /**
     * Construction.
     * @param context Context
     */
    public Op01GeneralPreferenceActivityExt(Context context) {
        super(context);
    }

    @Override
    public void setMessagePreferences(Activity hostActivity,
            PreferenceCategory pc) {
        Op01MmsPreference mmsPreferenceExt = new Op01MmsPreference(this);
        mmsPreferenceExt.configGeneralPreference(hostActivity, pc);
    }
}
