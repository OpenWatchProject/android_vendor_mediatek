package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.SharedPreferences;
import android.preference.PreferenceCategory;

public class DefaultOpMmsPreferenceActivityExt extends ContextWrapper implements
        IOpMmsPreferenceActivityExt {

    public DefaultOpMmsPreferenceActivityExt(Context base) {
        super(base);
    }

    @Override
    public void setMessagePreferences(Activity hostActivity,
            PreferenceCategory pC, int simCount) {

    }

    @Override
    public void restoreDefaultPreferences(Activity hostActivity, SharedPreferences.Editor editor) {

    }
}
