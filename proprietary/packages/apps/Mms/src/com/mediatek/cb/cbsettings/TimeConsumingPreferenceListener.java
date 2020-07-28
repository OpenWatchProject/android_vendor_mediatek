package com.mediatek.cb.cbsettings;

import android.preference.Preference;


public interface  TimeConsumingPreferenceListener {
    void onStarted(Preference preference, boolean reading);
    void onFinished(Preference preference, boolean reading);
    void onError(Preference preference, int error);
}
