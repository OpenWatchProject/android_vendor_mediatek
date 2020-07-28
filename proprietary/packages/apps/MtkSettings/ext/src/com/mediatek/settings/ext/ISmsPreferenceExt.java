package com.mediatek.settings.ext;

import android.content.Context;
import android.support.v7.preference.ListPreference;


public interface ISmsPreferenceExt {

    /**
    * Called whilke setting summary from DefaultSmsPreference
    * @return whether summary can be set or not
    * @internal
    */
    public boolean canSetSummary();


    /**
    * Called while instantiate of DefaultSmsPreference
    * @param context Context of the activity
    * @param listPreference to be shown in the list
    * @internal
    */
    public void createBroadcastReceiver(Context context, ListPreference listPreference);

    /**
    * Called when one item is selected from the list from DefaultSmsPreference
    * @param context Context of the activity
    * @param newValue Value selected from the list
    * @return whether to set default application or not
    * @internal
    */
    public boolean getBroadcastIntent(Context context, String newValue);


    /**
    * Called while deinstantiate of DefaultSmsPreference
    * @param context Context of the activity
    * @internal
    */
    public void deregisterBroadcastReceiver(Context context);
}
