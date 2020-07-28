package com.ct.deviceregister.dm.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import com.ct.deviceregister.dm.Const;

public class AgentProxyIms {

    private static final String TAG = Const.TAG_PREFIX + "AgentProxyIms";

    private SharedPreferences mSharedPreferences;

    private static final String PRE_FILE_NAME = "preference_device_register_ims";

    private static final String PRE_REGISTER_FLAG_0 = "pre_register_flag_0";
    private static final String PRE_REGISTER_FLAG_1 = "pre_register_flag_1";
    private static final String PRE_REGISTER_IMSI_0 = "pre_register_imsi_0";
    private static final String PRE_REGISTER_IMSI_1 = "pre_register_imsi_1";
    private static final String PRE_SAVED_IMSI = "pre_saved_imsi";

    public AgentProxyIms(Context context) {
        mSharedPreferences = getUniquePreferences(context);
    }

    public void resetRegisterFlag() {
        setRegisterFlag(Const.SLOT_ID_0, false);
        setRegisterFlag(Const.SLOT_ID_1, false);
    }

    public boolean isRegistered(int slot) {
        return getRegisterFlag(slot);
    }

    private boolean getRegisterFlag(int slot) {
        if (!isSlotValid(slot)) {
            return false;
        }

        if (slot == Const.SLOT_ID_0) {
            return mSharedPreferences.getBoolean(PRE_REGISTER_FLAG_0, false);
        }
        return mSharedPreferences.getBoolean(PRE_REGISTER_FLAG_1, false);
    }

    public void setRegisterFlag(int slot, boolean flag) {
        Log.i(TAG, "setRegisterFlag slot " + slot + " flag " + flag);
        if (!isSlotValid(slot)) {
            return;
        }

        if (slot == Const.SLOT_ID_0) {
            mSharedPreferences.edit().putBoolean(PRE_REGISTER_FLAG_0, flag).commit();
            return;
        }
        mSharedPreferences.edit().putBoolean(PRE_REGISTER_FLAG_1, flag).commit();
    }

    public String getRegisterImsi(int slot) {
        if (!isSlotValid(slot)) {
            return "";
        }

        if (slot == Const.SLOT_ID_0) {
            return mSharedPreferences.getString(PRE_REGISTER_IMSI_0, "");
        }
        return mSharedPreferences.getString(PRE_REGISTER_IMSI_1, "");
    }

    public void setRegisterImsi(int slot, String imsi) {
        Log.i(TAG, "setRegisterImsi " + slot + " imsi " + PlatformManager.encryptMessage(imsi));
        if (!isSlotValid(slot)) {
            return;
        }

        if (slot == Const.SLOT_ID_0) {
            mSharedPreferences.edit().putString(PRE_REGISTER_IMSI_0, imsi).commit();
            return;
        }
        mSharedPreferences.edit().putString(PRE_REGISTER_IMSI_1, imsi).commit();
    }

    /**
     * Note: String.split("A,B,") is {A, B} not {A, B, ""}
     * @return an IMSI array saved in SharedPreference
     */
    public String[] getSavedImsi() {
        return mSharedPreferences.getString(PRE_SAVED_IMSI, "").split(",");
    }

    /**
     * Serialize current IMSI array and save to SharedPreference
     * Note: As String.split("A,B,") is {A, B}, no need to remove the trailing ","
     * @param imsiArray: current IMSI array
     */
    public void setSavedImsi(String[] imsiArray) {
        String result = "";
        for (String imsi: imsiArray) {
            result += imsi + ",";
        }
        mSharedPreferences.edit().putString(PRE_SAVED_IMSI, result).commit();
    }

    private SharedPreferences getUniquePreferences(Context context) {
        return context.getSharedPreferences(PRE_FILE_NAME, Context.MODE_PRIVATE);
    }

    private boolean isSlotValid(int slot) {
        return PlatformManager.isSlotValid(slot);
    }
}
