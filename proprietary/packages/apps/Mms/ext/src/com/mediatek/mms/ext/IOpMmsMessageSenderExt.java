package com.mediatek.mms.ext;

import android.content.SharedPreferences;

public interface IOpMmsMessageSenderExt {

    /**
     * @internal
     */
    public long updatePreferencesHeaders(long defaultExpiry, int subId, SharedPreferences prefs);
}
