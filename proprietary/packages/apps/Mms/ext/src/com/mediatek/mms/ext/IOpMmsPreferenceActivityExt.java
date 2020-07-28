package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.SharedPreferences;
import android.preference.PreferenceCategory;

public interface IOpMmsPreferenceActivityExt {
    /**
     * @internal
     */
    void setMessagePreferences(Activity hostActivity, PreferenceCategory pC, int simCount);
    /**
     * @internal
     */
    void restoreDefaultPreferences(Activity hostActivity, SharedPreferences.Editor editor);
}
