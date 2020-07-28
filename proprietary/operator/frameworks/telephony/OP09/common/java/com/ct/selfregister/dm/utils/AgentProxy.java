package com.ct.selfregister.dm.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.util.Log;

import com.ct.selfregister.dm.Const;

public class AgentProxy {

    private static final String TAG = Const.TAG_PREFIX + "AgentProxy";

    private static final String PRE_FILE_NAME = "preference_self_register";

    private static final String PRE_FINGERPRINT = "pre_fingerprint";
    private static final String PRE_PERIOD_TRIGGER_TIME = "pre_period_trigger_time";
    private static final String PRE_REGISTER_FLAG = "pre_register_flag";
    private static final String PRE_SAVED_ICCID = "pre_saved_iccid";
    private static final String PRE_REGISTER_MESSAGE = "pre_register_message";


    private SharedPreferences mSharedPreferences;

    public AgentProxy(Context context) {
        mSharedPreferences = getUniquePreferences(context);
    }

    //------------------------------------------------------
    //  Register flag
    //------------------------------------------------------

    public void resetRegisterFlag() {
        setRegisterFlag(false);
    }

    public boolean isRegistered() {
        return getRegisterFlag();
    }

    private boolean getRegisterFlag() {
        return mSharedPreferences.getBoolean(PRE_REGISTER_FLAG, false);
    }

    public void setRegisterFlag(boolean flag) {
        Log.d(TAG, "setRegisterFlag " + flag);
        mSharedPreferences.edit().putBoolean(PRE_REGISTER_FLAG, flag).commit();
    }

    //------------------------------------------------------
    //  IccIds
    //------------------------------------------------------

    /**
     * Note: String.split("A,B,") is {A, B} not {A, B, ""}
     * @return an ICCID array saved in SharedPreference
     */
    public String[] getSavedIccId() {
        return mSharedPreferences.getString(PRE_SAVED_ICCID, "").split(",");
    }

    /**
     * Serialize current ICCID array and save to SharedPreference
     * Note: As String.split("A,B,") is {A, B}, no need to remove the trailing ","
     * @param iccidArray: current ICCID array
     */
    public void setSavedIccId(String[] iccidArray ) {
        String result = "";
        for (String iccid: iccidArray) {
            result += iccid + ",";
        }
        mSharedPreferences.edit().putString(PRE_SAVED_ICCID, result).commit();
    }

    public void setSavedFingerPrint(String fingerprint) {
        Log.i(TAG, "setSavedFingerPrint " + fingerprint);
        mSharedPreferences.edit().putString(PRE_FINGERPRINT, fingerprint).commit();
    }

    public String getSavedFingerPrint() {
        return mSharedPreferences.getString(PRE_FINGERPRINT, "");
    }

    public SharedPreferences getUniquePreferences(Context context) {
        return context.getSharedPreferences(PRE_FILE_NAME, Context.MODE_PRIVATE);
    }

    public String getCurrentFingerPrint() {
        return Build.FINGERPRINT;
    }

    public void setPeriodTriggerTime(long interval) {
        Log.i(TAG, "setPeriodTriggerTime " + interval);
        long triggerTime = System.currentTimeMillis() + interval;
        mSharedPreferences.edit().putLong(PRE_PERIOD_TRIGGER_TIME, triggerTime).commit();
    }

    public long getPeriodTriggerTime() {
        return mSharedPreferences.getLong(PRE_PERIOD_TRIGGER_TIME, 0);
    }

    public void setRegisterMessage(String message) {
        mSharedPreferences.edit().putString(PRE_REGISTER_MESSAGE, message).commit();
    }
}
