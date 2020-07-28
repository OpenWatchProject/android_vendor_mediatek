/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.ct.deviceregister.dm.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.util.Log;

import com.ct.deviceregister.dm.Const;

public class AgentProxy {

    private static final String TAG = Const.TAG_PREFIX + "AgentProxy";

    private SharedPreferences mSharedPreferences;

    public static final String PRE_FILE_NAME = "preference_device_register";

    public static final String PRE_FINGERPRINT = "pre_fingerprint";
    public static final String PRE_REGISTER_FLAG = "pre_register_flag";
    public static final String PRE_REGISTER_IMSI = "pre_register_imsi";
    public static final String PRE_SAVED_IMSI = "pre_saved_imsi";

    public AgentProxy(Context context) {
        mSharedPreferences = getUniquePreferences(context);
    }

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
        Log.i(TAG, "setRegisterFlag " + flag);
        mSharedPreferences.edit().putBoolean(PRE_REGISTER_FLAG, flag).commit();
    }

    public String getRegisterImsi() {
        return mSharedPreferences.getString(PRE_REGISTER_IMSI, "");
    }

    public void setRegisterImsi(String imsi) {
        Log.i(TAG, "setRegisterImsi " + PlatformManager.encryptMessage(imsi));
        mSharedPreferences.edit().putString(PRE_REGISTER_IMSI, imsi).commit();
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
        return context.getSharedPreferences(AgentProxy.PRE_FILE_NAME, Context.MODE_PRIVATE);
    }

    public void setSavedFingerPrint(String fingerprint) {
        Log.i(TAG, "setSavedFingerPrint " + fingerprint);
        mSharedPreferences.edit().putString(AgentProxy.PRE_FINGERPRINT, fingerprint).commit();
    }

    public String getSavedFingerPrint() {
        return mSharedPreferences.getString(AgentProxy.PRE_FINGERPRINT, "");
    }

    public String getCurrentFingerPrint() {
        return Build.FINGERPRINT;
    }

}
