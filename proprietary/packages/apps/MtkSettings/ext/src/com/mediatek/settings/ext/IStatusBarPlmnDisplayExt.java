package com.mediatek.settings.ext;

import android.support.v7.preference.PreferenceScreen;

public interface IStatusBarPlmnDisplayExt {

    /**
     * set the roaming warning msg
     * @param pref PreferenceScreen
     * @param order added preference's order
     * @internal
     */
   void createCheckBox(PreferenceScreen pref, int order);

}
