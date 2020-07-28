
package com.mediatek.settings.ext;

import android.content.Context;
import android.support.v7.preference.ListPreference;


public class DefaultSmsPreferenceExt implements ISmsPreferenceExt {
    public boolean canSetSummary() {
        return true;
    }
    public void createBroadcastReceiver(Context context, ListPreference listPreference){}
    public boolean getBroadcastIntent(Context context, String newValue) {
        return true;
    }
    public void deregisterBroadcastReceiver(Context context){}

}
