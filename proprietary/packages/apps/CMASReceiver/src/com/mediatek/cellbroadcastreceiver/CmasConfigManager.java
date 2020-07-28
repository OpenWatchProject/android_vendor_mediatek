package com.mediatek.cellbroadcastreceiver;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.util.Log;

public class CmasConfigManager {
    private static final String TAG = "[CMAS]CmasConfigManager";

    private static int sSettingProfileId = 0;
    private static int[] sAdditionalChannels = {};
    private static int[] sVibrationPattern = {};
    private static String sRingtone;
    private static boolean sUseFullVolume = false;
    private static boolean sGeminiSupport = false;

    public static final int PROFILE_ID_COMMON = 0;
    public static final int PROFILE_ID_TW     = 1;
    public static final int PROFILE_ID_TRA    = 2;
    public static final int PROFILE_ID_CHILE  = 3;

    public static void updateConfigInfos(Context context) {
        sSettingProfileId = context.getResources().getInteger(R.integer.cmas_setting_profile_id);
        sAdditionalChannels = context.getResources().getIntArray(R.array.additional_cbs_channels);
        sVibrationPattern = context.getResources().getIntArray(R.array.default_vibration_pattern);
        sRingtone = context.getResources().getString(R.string.defalut_alert_ringtone);
        sUseFullVolume = context.getResources().getBoolean(R.bool.use_full_volume);
        sGeminiSupport = isGeminiSupport();
    }

    /**
      * @get SimConfig by TelephonyManager.getDefault().getMultiSimConfiguration().
      * @return true if the device has 2 or more slots
      */
    private static boolean isGeminiSupport() {
        TelephonyManager.MultiSimVariants mSimConfig = TelephonyManager.
                getDefault().getMultiSimConfiguration();
        if (mSimConfig == TelephonyManager.MultiSimVariants.DSDS ||
                mSimConfig == TelephonyManager.MultiSimVariants.DSDA) {
            return true;
        }
        return false;
    }

    public static int getSettingProfileId() {
        Log.d(TAG, "getSettingProfileId as " + sSettingProfileId);
        return sSettingProfileId;
    }

    public static int[] getAdditionalChannels() {
        return sAdditionalChannels;
    }

    public static int[] getVibrationPattern() {
        return sVibrationPattern;
    }

    public static String getRingtone() {
        return sRingtone;
    }

    public static boolean getUseFullVolume() {
        return sUseFullVolume;
    }

    public static boolean getGeminiSupport() {
        return sGeminiSupport;
    }

    public static boolean isTwProfile()  {
        return sSettingProfileId == PROFILE_ID_TW;
    }

    public static boolean isChileProfile() {
        return sSettingProfileId == PROFILE_ID_CHILE;
    }

    public static boolean isTraProfile() {
        return sSettingProfileId == PROFILE_ID_TRA;
    }

    public static boolean isAlertsEnabled(Context context, String slotKey) {
        boolean enableAlerts = true;
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        switch (sSettingProfileId) {
        case PROFILE_ID_COMMON:
        case PROFILE_ID_CHILE:
        case PROFILE_ID_TW:
            enableAlerts = prefs.getBoolean(slotKey +
                    CheckBoxAndSettingsPreference.KEY_ENABLE_CELLBROADCAST, true);
            break;
        case PROFILE_ID_TRA:
            enableAlerts = true;
            break;
        default:
            break;
        }
        return enableAlerts;
    }

    public static boolean isEtwsEnabled(Context context) {
        boolean enableAlerts = true;
        switch (sSettingProfileId) {
        case PROFILE_ID_COMMON:
        case PROFILE_ID_CHILE:
        case PROFILE_ID_TW:
            enableAlerts = true;
            break;
        case PROFILE_ID_TRA:
            enableAlerts = false;
            break;
        default:
            break;
        }
        return enableAlerts;
    }

    public static boolean isExtremEnable(Context context, String slotKey) {
        boolean enableAlerts = true;
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        switch (sSettingProfileId) {
        case PROFILE_ID_COMMON:
        case PROFILE_ID_CHILE:
            String extremKey = slotKey +
                CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_EXTREME_ALERTS;
            enableAlerts = prefs.getBoolean(extremKey, true);
            break;
        case PROFILE_ID_TW:
            enableAlerts = prefs.getBoolean("enable_message_identifiers", true);
            break;
        case PROFILE_ID_TRA:
            enableAlerts = true;
            break;
        default:
            break;
        }
        return enableAlerts;
    }

    public static boolean isSevereEnable(Context context, String slotKey) {
        boolean enableAlerts = true;
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        switch (sSettingProfileId) {
        case PROFILE_ID_COMMON:
        case PROFILE_ID_CHILE:
            String severeKey = slotKey +
                CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_SEVERE_ALERTS;
            enableAlerts = prefs.getBoolean(severeKey, true);
            break;
        case PROFILE_ID_TW:
            enableAlerts = prefs.getBoolean("enable_message_identifiers", true);
            break;
        case PROFILE_ID_TRA:
            enableAlerts = true;
            break;
        default:
            break;
        }
        return enableAlerts;
    }

    public static boolean isAmberEnable(Context context, String slotKey) {
        boolean enableAlerts = true;
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        switch (sSettingProfileId) {
        case PROFILE_ID_COMMON:
        case PROFILE_ID_CHILE:
            String amberKey = slotKey +
                CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_AMBER_ALERTS;
            enableAlerts = prefs.getBoolean(amberKey, true);
            break;
        case PROFILE_ID_TW:
            enableAlerts = prefs.getBoolean("enable_message_identifiers", true);
            break;
        case PROFILE_ID_TRA:
            enableAlerts = prefs.getBoolean(
                CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_AMBER_ALERTS,true);
            break;
        default:
            break;
        }
        return enableAlerts;
    }

    public static boolean isRmtEnable(Context context, String slotKey) {
        boolean enableAlerts = true;
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        switch (sSettingProfileId) {
        case PROFILE_ID_COMMON:
        case PROFILE_ID_CHILE:
            String rmtKey = slotKey +
                CellBroadcastConfigService.ENABLE_CMAS_RMT_SUPPORT;
            enableAlerts = prefs.getBoolean(rmtKey, false);
            break;
        case PROFILE_ID_TW:
        case PROFILE_ID_TRA:
            enableAlerts = prefs.getBoolean(
                CellBroadcastConfigService.ENABLE_CMAS_RMT_SUPPORT, false);
            break;
        default:
            break;
        }
        return enableAlerts;
    }

    public static boolean isExerciseEnable(Context context, String slotKey) {
        boolean enableAlerts = true;
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        switch (sSettingProfileId) {
        case PROFILE_ID_COMMON:
        case PROFILE_ID_CHILE:
        case PROFILE_ID_TW:
            String exerciseKey = slotKey +
                            CellBroadcastConfigService.ENABLE_CMAS_EXERCISE_SUPPORT;
            enableAlerts = prefs.getBoolean(exerciseKey, false);
            break;
        case PROFILE_ID_TRA:
            enableAlerts = true;
            break;
        default:
            break;
        }
        return enableAlerts;
    }
}
