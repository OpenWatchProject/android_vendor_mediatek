package com.mediatek.op08.dialer.setting;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.SparseBooleanArray;
import android.util.SparseArray;
import android.util.Log;

public class SharedPreferencesHelper {
    private static final String TAG = "SharedPreferencesHelper";

    public static SharedPreferences getSharedPreferences(Context context) {
        if (context == null) {
            return null;
        }
        return context.getSharedPreferences(
                SettingConstant.RCS_CALL_PREF_NAME, Context.MODE_PRIVATE);
    }

    public static boolean readBooleanFromPref(Context context, String key) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(
                SettingConstant.RCS_CALL_PREF_NAME, Context.MODE_PRIVATE);
        boolean ret = sharedPreferences.getBoolean(key, false);
        return ret;
    }

    public static String readStringFromPref(Context context, String key) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(
                SettingConstant.RCS_CALL_PREF_NAME, Context.MODE_PRIVATE);
        String ret = sharedPreferences.getString(key, null);
        return ret;
    }


    public static void setBooleanToPref(Context context, String key, boolean value) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(
                SettingConstant.RCS_CALL_PREF_NAME, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putBoolean(key, value);
        editor.apply();
    }


    public static void setStringToPref(Context context, String key, String value) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(
                SettingConstant.RCS_CALL_PREF_NAME, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(key, value);
        editor.apply();
    }

    public static void readStatusFromPref(Context context,
            SparseBooleanArray status) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(
                SettingConstant.RCS_CALL_PREF_NAME, Context.MODE_PRIVATE);
        status.put(SettingConstant.TYPE_MASTER_SWITCH,
                sharedPreferences.getBoolean(SettingConstant.KEY_MASTER_SWITCH_STATUS, false));
        status.put(SettingConstant.TYPE_IMPORTANT_INDICATOR,
                sharedPreferences.getBoolean(SettingConstant.KEY_INDICATOR_STATUS, false));
        status.put(SettingConstant.TYPE_SUBJECT,
                sharedPreferences.getBoolean(SettingConstant.KEY_SUBJECT_STATUS, false));
        status.put(SettingConstant.TYPE_LOCATION,
                sharedPreferences.getBoolean(SettingConstant.KEY_LOCATION_STATUS, false));
        status.put(SettingConstant.TYPE_IMAGE,
                sharedPreferences.getBoolean(SettingConstant.KEY_IMAGE_STATUS, false));
    }

    public static void readContentsFromPref(Context context,
            SparseArray<String> contents) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(
                SettingConstant.RCS_CALL_PREF_NAME, Context.MODE_PRIVATE);
        contents.put(SettingConstant.TYPE_MASTER_SWITCH,
                sharedPreferences.getString(SettingConstant.KEY_MASTER_SWITCH_CONTENT, null));
        contents.put(SettingConstant.TYPE_IMPORTANT_INDICATOR,
                sharedPreferences.getString(SettingConstant.KEY_INDICATOR_CONTENT, null));
        contents.put(SettingConstant.TYPE_SUBJECT,
                sharedPreferences.getString(SettingConstant.KEY_SUBJECT_CONTENT, null));
        contents.put(SettingConstant.TYPE_IMAGE,
                sharedPreferences.getString(SettingConstant.KEY_IMAGE_CONTENT, null));
        contents.put(SettingConstant.TYPE_LOCATION,
                sharedPreferences.getString(SettingConstant.KEY_LOCATION_CONTENT, null));
    }
}