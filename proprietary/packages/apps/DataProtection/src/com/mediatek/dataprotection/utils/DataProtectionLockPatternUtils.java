package com.mediatek.dataprotection.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import com.mediatek.dataprotection.utils.Log;

import com.android.internal.widget.LockPatternUtils;

public class DataProtectionLockPatternUtils {

    private static final String TAG = "DataProtectionLockPatternUtils";
    private static final long TARGET_VERSION = 23;
    private Context mContext = null;

    public DataProtectionLockPatternUtils(Context context) {
        mContext = context;
    }

    public boolean saveLockPattern(String pattern) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = prefs.edit();
        if (pattern == null) {
            editor.clear();
            editor.commit();
            return true;
        }

        final byte[] hash = LockPatternUtils.patternToHash(LockPatternUtils
                .stringToPattern(pattern));

        editor.putString("password", hashToHex(hash));
        editor.putLong("target", TARGET_VERSION);
        boolean result = editor.commit();
        Log.d(TAG, "saveLockPattern...result " + result);

        return result;
    }

    public String hashToHex(byte[] hash) {
        StringBuilder hexString = new StringBuilder();
        for (int i = 0; i < hash.length; i ++) {
            hexString.append(String.valueOf(hash[i]));
        }
        return hexString.toString();
    }

    public boolean isPatternSet() {
        Log.d(TAG, "isPatternSet...");
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(mContext);

        String pattern = prefs.getString("password", null);

        return pattern != null;
    }

    public boolean checkPattern(String pattern) {
        Log.d(TAG, "checkPattern...");
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(mContext);

        String oldPattern = prefs.getString("password", null);
        long targetVersion = prefs.getLong("target", 0);
        if(targetVersion < TARGET_VERSION) {
            Log.d(TAG, "checkPattern... targetVersion: " + targetVersion);
            saveLockPattern(pattern);
            return true;
        }
        final byte[] hash = LockPatternUtils.patternToHash(LockPatternUtils
                .stringToPattern(pattern));
        String newPattern = hashToHex(hash);
        return oldPattern.equals(newPattern);
    }

    public static boolean hasPattern(Context ctx) {
        DataProtectionLockPatternUtils util = new DataProtectionLockPatternUtils(ctx);
        return util.isPatternSet();
    }

    public static String getPattern(Context ctx) {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(ctx);

        return prefs.getString("password", null);
    }
}
