package com.mediatek.settings.ext;

import android.support.v7.preference.PreferenceScreen;

public interface IAppsExt {

    /**
     * @param prefScreen
     *            customized prefScreen
     * @internal
     */
    void launchApp(PreferenceScreen prefScreen, String packageName);
}
