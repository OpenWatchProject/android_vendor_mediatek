package com.mediatek.exchange.smartpush;

import android.content.Context;
import android.content.SharedPreferences;

public class SmartPushPreferences {
    // Preferences file
    public static final String PREFERENCES_FILE = "SmartPush.Main";
    // Preferences field names
    private static final String LAST_CALCULATE_TIME = "lastCalculateTime";

    private static SmartPushPreferences sPreferences;
    private final SharedPreferences mSharedPreferences;

    private SmartPushPreferences(Context context) {
        mSharedPreferences = context.getSharedPreferences(PREFERENCES_FILE, Context.MODE_PRIVATE);
    }

    public static synchronized SmartPushPreferences getPreferences(Context context) {
        if (sPreferences == null) {
            sPreferences = new SmartPushPreferences(context);
        }
        return sPreferences;
    }

    public static SharedPreferences getSharedPreferences(Context context) {
        return getPreferences(context).mSharedPreferences;
    }

    public long getLastCalculateTime() {
        return mSharedPreferences.getLong(LAST_CALCULATE_TIME, 0);
    }

    public void settLastCalculateTime(long time) {
        mSharedPreferences.edit().putLong(LAST_CALCULATE_TIME, time).apply();
    }

    public void removeLastCalculateTime() {
        mSharedPreferences.edit().remove(LAST_CALCULATE_TIME).apply();
    }
}