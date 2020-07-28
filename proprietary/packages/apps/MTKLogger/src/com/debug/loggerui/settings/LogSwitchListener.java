package com.debug.loggerui.settings;

import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;

/**
 * @author MTK81255
 *
 */
public class LogSwitchListener implements OnPreferenceChangeListener, OnCheckedChangeListener {

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object obj) {
        return false;
    }

}
