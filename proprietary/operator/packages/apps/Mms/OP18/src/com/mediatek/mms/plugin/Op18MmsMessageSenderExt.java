package com.mediatek.op18.mms;


import android.content.SharedPreferences;
import android.util.Log;

import com.mediatek.mms.ext.DefaultOpMmsMessageSenderExt;

public class  Op18MmsMessageSenderExt extends DefaultOpMmsMessageSenderExt {

    private static final String TAG = "Mms/ Op18MmsMessageSenderExt";

    @Override
    public long updatePreferencesHeaders(long defaultExpiry, int subId, SharedPreferences prefs) {
        final long[] validities = {//Op18MmsPreference.VALIDITY_PERIOD_NO_DURATION,
                                    Op18MmsPreference.EXPIRY_ONE_HOUR,
                                    Op18MmsPreference.EXPIRY_SIX_HOURS,
                                    Op18MmsPreference.EXPIRY_TWELVE_HOURS,
                                    Op18MmsPreference.EXPIRY_ONE_DAY,
                                    Op18MmsPreference.EXPIRY_MAX_DURATION};

        final String validityKey = Integer.toString(subId) + "_" +
                        Op18MmsPreference.MMS_VALIDITY_PERIOD_PREFERENCE_KEY;;
        Log.d(TAG, "updatePreferencesHeaders key :- " + validityKey);
        int index = prefs.getInt(validityKey, 0);
        if (index > validities.length) {
            index = 0;
        }
        Log.d(TAG, "updatePreferencesHeaders: result index = " + index +
                                       ", validity =" + validities[index]);
        return validities[index];
    }
}

