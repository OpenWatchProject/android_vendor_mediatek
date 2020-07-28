/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ims.pco;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.UserManager;
import android.util.Log;

import static com.mediatek.ims.pco.PCOConfig.DEBUG;

public class PCOSharedPreference {
    static protected final String TAG = "PCOSharedPreference";
    static private final String SHARED_PREFERENCES_NAME = "ims_pco_information";

    /*
     * Keys for Share Preference
     */
    static private final String ACTIVATED_VIRTUAL_LINES = "ActivatedVirtualLines";
    static private final String DEVICE_NAME = "DeviceName";
    static private final String ICCID = "Iccid";

    static private final String IAM_USER_REFRESH_TOKEN = "IAMUserRefreshToken";
    static private final String PCO_ACTIVATION_STATUS = "PCOActivationStatusToken";

    private static final String IS_FIRST_BOOT = "IsFirstBoot";
    private static final String IS_ACTIVATED = "IsActivated";

    private static volatile PCOSharedPreference sPCOPref;
    private static Context sContext;
    private SharedPreferences mPref;

    /**
     *
     * @param mContext
     */
    private PCOSharedPreference(Context mContext) {

        sContext = mContext;
        mPref = mContext.getSharedPreferences(
                SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
    }

    /**
     *  To get the context of share preference.
     * @return Context of share preferences.
     */
    public static PCOSharedPreference getInstance(Context context) {

        if (sPCOPref != null) {
            return sPCOPref;
        }
        PCOSharedPreference mPCOPref = new PCOSharedPreference(context);
        sPCOPref = mPCOPref;
        return sPCOPref;
    }

    /**
     *  To get the context of share preference.
     * @return Context of share preferences.
     */
    private SharedPreferences getSharedPreferences() {
            return mPref;
    }

    private boolean getSharePrefBoolenValue(String key) {
        return getSharedPreferences().getBoolean(key, false);
    }

    private void setSharePrefBoolenValue(String key, boolean value) {
        SharedPreferences.Editor editor = getSharedPreferences().edit();
        editor.putBoolean(key, value);
        editor.commit();
    }


    private int getSharePrefIntValue(String key) {
        return getSharedPreferences().getInt(key, -1);
    }

    private void setSharePrefIntValue(String key, int value) {
        SharedPreferences.Editor editor = getSharedPreferences().edit();
        editor.putInt(key, value);
        editor.apply();
    }

    private String getSharePrefStringValue(String key) {
        return getSharedPreferences().getString(key, "DEFAULT");
    }

    private void setSharePrefStringValue(String key, String value) {
        SharedPreferences.Editor editor = getSharedPreferences().edit();
        editor.putString(key, value);
        editor.apply();
    }


    /**
     * Not is used. It will be useful in PCO=3's development.
     * @param name Device Name.
     */
    public void saveDeviceName(String name) {
       log("saveDeviceName(), name:" + name);

        Editor editor = mPref.edit();
        editor.putString(DEVICE_NAME, name);
        editor.apply();
    }

    /**
     * Not is used. It will be useful in PCO=3's development.
     * @return Device name.
     */
    public String getDeviceName() {
        String deviceName = mPref.getString(DEVICE_NAME, "");

        if (deviceName.length() == 0) {

            // Initialize with user name
            String userName = UserManager.get(sContext).getUserName();
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

    /**
     * Not is used. It will be useful in PCO=3's development.
      * @param iccid of SIM.
     */
    public void saveIccid(String iccid) {
        log("saveIccid(), iccid:" + iccid);
        setSharePrefStringValue(ICCID, iccid);
    }

    /**
     * Not is used. It will be useful in PCO=3's development.
     * @return Iccid
     */
    public String getIccid() {
        return getSharePrefStringValue(ICCID);
    }

    /**
     *  To update the IMS PCO status.
     * @param PCOState Current Activation status.
     */
    public void savePCOState(int mPCOState) {
        log("savePCOState(), PCOState:" + mPCOState);
        setSharePrefIntValue(PCO_ACTIVATION_STATUS, mPCOState);
    }

    /**
     * Gives Activation status and error details if any.
     * @return
     */
    public int getSavedPCOState() {
        int mState = getSharePrefIntValue(PCO_ACTIVATION_STATUS);
        log("getSavedPCOState(), PCOState:" + mState);
        return mState;
    }


    /**
     * This will be updapted after the first boot.
     * @param status  out-of-box status.
     */
    public void saveFirstBoot(boolean status) {
        log("saveFirstBoot() " + status);
        setSharePrefBoolenValue(IS_FIRST_BOOT, status);
    }

    /**
     * To get the information if the phone is booted for first time.
     * @return Gives if the phone is out-of-box and it is not activated.
     */
    public boolean isFirstBoot() {
        boolean isFirstBoot = getSharedPreferences().getBoolean(IS_FIRST_BOOT, true);
        log("isFirstBoot(), " + isFirstBoot);
        return isFirstBoot;
    }

    /**
     *  Activation status is updated using this API. this is called from service.
     * @param status Activation status of the phone.
     */
    public void saveActivated(boolean status) {
        log("saveActivated" + status);
        setSharePrefBoolenValue(IS_ACTIVATED, status);
    }

    /**
     * To get the current activation status of the phone.
     * @return Gives the activation status of phone if phone is activated or not.
     */
    public boolean isActivated() {
        boolean isActivated = getSharedPreferences().getBoolean(IS_ACTIVATED, false);
        log("isActivated(), " + isActivated);
        return isActivated;
    }

    private static void log(String s) {
        if (DEBUG) {
            Log.d(TAG, s);
        }
    }

}
