package com.mediatek.settings.ext;

import android.content.Context;
import android.support.v7.preference.PreferenceScreen;


/**
 *  the class for dispaly settings feature plugin.
 */
public interface IDisplaySettingsExt {

    /**
     * add display extended preference.
     * @param context The Context of the screen
     * @param screen PreferenceScreen
     * @internal
     */
    void addPreference(Context context, PreferenceScreen screen);

    /**
     * remove display preference.
     * @param context The Context of the screen
     * @param screen PreferenceScreen
     * @internal
     */
    void removePreference(Context context, PreferenceScreen screen);

    /**
     * check whether alternative preference present.
     * @return whether present or not.
     * @internal
     */
    boolean isCustomPrefPresent();

    /**
     * customize font size values.
     * @param default string
     * @return custom string
     * @internal
     */
    public String[] getFontEntries(String[] defaultStr);

    /**
     * customize font size strings.
     * @param default font size
     * @return custom font size
     * @internal
     */
    public String[] getFontEntryValues(String[] defaultStr);

}
