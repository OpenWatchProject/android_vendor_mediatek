package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.preference.PreferenceCategory;

public class DefaultOpGeneralPreferenceActivityExt extends ContextWrapper
        implements IOpGeneralPreferenceActivityExt {

    public DefaultOpGeneralPreferenceActivityExt(Context base) {
        super(base);
    }

    @Override
    public void setMessagePreferences(Activity hostActivity,
            PreferenceCategory pc) {

    }

}
