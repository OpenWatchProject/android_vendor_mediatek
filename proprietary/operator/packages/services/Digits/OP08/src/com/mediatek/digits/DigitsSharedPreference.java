package com.mediatek.digits;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.UserManager;
import android.util.Log;

import java.util.HashSet;
import java.util.Set;


public class DigitsSharedPreference {
    static protected final String TAG = "DigitsSharedPreference";

    /*
     * Key for Share Prefernece
     */
    static private final String ACTIVATED_VIRTUAL_LINES = "ActivatedVirtualLines";
    static private final String DEVICE_NAME = "DeviceName";
    static private final String ICCID = "Iccid";

    static private final String IAM_USER_REFRESH_TOKEN = "IAMUserRefreshToken";

    private static DigitsSharedPreference sDigitsPref;
    private Context mContext;
    private SharedPreferences mPref;

    // Private constructor
    private DigitsSharedPreference(Context context) {
        mContext = context;

        mPref = context.getSharedPreferences(TAG, Context.MODE_PRIVATE);

    }

    public static DigitsSharedPreference getInstance(Context context) {

        if (sDigitsPref != null) {
            return sDigitsPref;
        }

        DigitsSharedPreference digitsPref = new DigitsSharedPreference(context);
        sDigitsPref = digitsPref;

        return digitsPref;
    }

    public void saveActivatedVirtualLines(Set<String> msisdns) {
        Log.d(TAG, "saveActivatedVirtualLines(), msisdns:" + msisdns);

        Editor editor = mPref.edit();

        editor.putStringSet(ACTIVATED_VIRTUAL_LINES, msisdns);

        editor.apply();
    }

    public Set<String> getActivatedVirtualLines() {
        Set<String> msisdns = mPref.getStringSet(ACTIVATED_VIRTUAL_LINES, new HashSet<String>());
        Log.d(TAG, "getActivatedVirtualLines(), msisdns: " + msisdns);

        return msisdns;
    }

    public void saveDeviceName(String name) {
        Log.d(TAG, "saveDeviceName(), name:" + name);

        Editor editor = mPref.edit();
        editor.putString(DEVICE_NAME, name);
        editor.apply();
    }

    public String getDeviceName() {
        String deviceName = mPref.getString(DEVICE_NAME, "");

        if (deviceName.length() == 0) {

            // Initialize with user name
            String userName = UserManager.get(mContext).getUserName();
            Log.d(TAG, "UserManager.getUserName() is " + userName);

            if (userName == null || userName.length() == 0) {
                deviceName = "My phone";
            } else {
                deviceName = userName + "\'s phone";
            }

            // Write to shared preference
            Editor editor = mPref.edit();
            editor.putString(DEVICE_NAME, deviceName);
            editor.apply();
        }


        return deviceName;
    }

    public void saveIccid(String iccid) {
        Log.d(TAG, "saveIccid(), iccid:" + iccid);

        Editor editor = mPref.edit();
        editor.putString(ICCID, iccid);
        editor.apply();
    }

    public String getIccid() {

        return mPref.getString(ICCID, null);
    }

    public void saveRefreshToken(String tokenValue) {
        Log.d(TAG, "saveRefreshTokenValue(), tokenValue:" + tokenValue);
        Editor editor = mPref.edit();

        if (tokenValue == null) {
            editor.remove(IAM_USER_REFRESH_TOKEN).commit();
        } else {
            editor.putString(IAM_USER_REFRESH_TOKEN, tokenValue);
            editor.apply();
        }
    }

    public String getRefreshToken() {
        return mPref.getString(IAM_USER_REFRESH_TOKEN, null);
    }

}
