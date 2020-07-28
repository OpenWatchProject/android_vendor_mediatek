package com.mediatek.camera.feature.setting.slowmotionquality;

import android.content.Context;
import android.preference.CheckBoxPreference;
import android.preference.Preference;

import com.mediatek.camera.R;

public class RadioPreference extends CheckBoxPreference {
    public RadioPreference(Context context) {
        super(context);
        setWidgetLayoutResource(R.layout.slowmotionquality_radio_preference_widget);
        setOnPreferenceChangeListener(
                new Preference.OnPreferenceChangeListener() {
                    @Override
                    public boolean onPreferenceChange(android.preference.Preference preference,
                                                      Object newValue) {
                        boolean value = (Boolean) newValue;
                        return value;
                    }
                }
        );
    }
}

