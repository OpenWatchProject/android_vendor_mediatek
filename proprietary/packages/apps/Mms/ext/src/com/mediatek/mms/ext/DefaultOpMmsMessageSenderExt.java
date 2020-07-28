package com.mediatek.mms.ext;


import android.content.SharedPreferences;


public class DefaultOpMmsMessageSenderExt implements IOpMmsMessageSenderExt {

    @Override
    public long updatePreferencesHeaders(long defaultExpiry, int subId, SharedPreferences prefs) {
        return defaultExpiry;
    }
}

