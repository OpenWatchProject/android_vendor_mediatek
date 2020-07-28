/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.settings.sim;

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.ITelephony;
import com.android.settings.R;

import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.ISimManagementExt;

/**
 * Radio power manager to control radio state.
 */
public class RadioPowerController {

    private static final String TAG = "RadioPowerController";
    private Context mContext;
    private static final int MODE_PHONE1_ONLY = 1;
    private ISimManagementExt mExt;
    private static RadioPowerController sInstance = null;
    private static final boolean ENG_LOAD = SystemProperties.get("ro.build.type").equals("eng")
            ? true : false || Log.isLoggable(TAG, Log.DEBUG);

   /**
    * Constructor.
    * @param context Context
    */
    private RadioPowerController(Context context) {
        mContext = context;
        mExt = UtilsExt.getSimManagementExt(mContext);
    }

    private static synchronized void createInstance(Context context) {
        if (sInstance == null) {
            sInstance = new RadioPowerController(context);
        }
    }

    public static RadioPowerController getInstance(Context context) {
        if (sInstance == null) {
            createInstance(context);
        }
        return sInstance;
    }

    public boolean setRadionOn(int subId, boolean turnOn) {
        logInEng("setRadioOn, turnOn=" + turnOn + ", subId=" + subId);
        boolean isSuccessful = false;

        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            return isSuccessful;
        }

        // Check ECC state according to TeleService state.
        // Don't allow opetate radio off during ECC to avoid ATD after EFUN
        boolean isInEcc = TelecomManager.from(mContext).isInEmergencyCall();

        if (!turnOn && isInEcc) {
            Log.d(TAG, "Not allow to operate radio power during emergency call");
            Toast.makeText(mContext.getApplicationContext(),
                    R.string.radio_off_during_emergency_call, Toast.LENGTH_LONG).show();
            return false;
        }

        ITelephony telephony = ITelephony.Stub.asInterface(ServiceManager.getService(
                Context.TELEPHONY_SERVICE));
        if (telephony != null) {
            try {
                if (telephony.isRadioOnForSubscriber(
                        subId, mContext.getPackageName()) != turnOn) {
                    isSuccessful = telephony.setRadioForSubscriber(subId, turnOn);
                    if (isSuccessful) {
                        updateRadioMsimDb(subId, turnOn);
                        // Add for SIM Settings plugin.
                        mExt.setRadioPowerState(subId, turnOn);
                    }
                }
            } catch (RemoteException e) {
                Log.e(TAG, "setRadionOn, RemoteException=" + e);
            }
        } else {
            logInEng("telephony is null.");
        }

        logInEng("setRadionOn, isSuccessful=" + isSuccessful);
        return isSuccessful;
    }

    private void updateRadioMsimDb(int subId, boolean turnOn) {
        int priviousSimMode = Settings.Global.getInt(mContext.getContentResolver(),
                MtkSettingsExt.Global.MSIM_MODE_SETTING, -1);
        logInEng("updateRadioMsimDb, the current msim_mode=" + priviousSimMode
                + ", subId=" + subId);
        int currentSimMode;
        boolean isPriviousRadioOn = false;
        int slot = SubscriptionManager.getSlotIndex(subId);
        int modeSlot = MODE_PHONE1_ONLY << slot;
        if ((priviousSimMode & modeSlot) > 0) {
            currentSimMode = priviousSimMode & (~modeSlot);
            isPriviousRadioOn = true;
        } else {
            currentSimMode = priviousSimMode | modeSlot;
            isPriviousRadioOn = false;
        }

        logInEng("updateRadioMsimDb, currentSimMode=" + currentSimMode
                + ", isPriviousRadioOn=" + isPriviousRadioOn + ", turnOn=" + turnOn);
        if (turnOn != isPriviousRadioOn) {
            Settings.Global.putInt(mContext.getContentResolver(),
                    MtkSettingsExt.Global.MSIM_MODE_SETTING, currentSimMode);
        } else {
            logInEng("updateRadioMsimDb, quickly click don't allow.");
        }
    }

    /**
     * whether radio switch finish on subId, according to the radio state.
     */
    public boolean isRadioSwitchComplete(final int subId) {
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            return false;
        }

        boolean radioOn = TelephonyUtils.isRadioOn(subId, mContext);

        return isRadioSwitchComplete(subId, radioOn);
    }

    /**
     * whether radio switch finish on subId, according to the radio state.
     */
    public boolean isRadioSwitchComplete(final int subId, boolean radioOn) {
        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            return false;
        }
        int slotId = SubscriptionManager.getSlotIndex(subId);

        logInEng("isRadioSwitchComplete, slot=" + slotId + ", radioOn=" + radioOn);
        if (!radioOn || (isExpectedRadioStateOn(slotId) && radioOn)) {
            logInEng("isRadioSwitchComplete, done.");
            return true;
        }
        return false;
    }

    public boolean isExpectedRadioStateOn(int slot) {
        int currentSimMode = Settings.Global.getInt(mContext.getContentResolver(),
                MtkSettingsExt.Global.MSIM_MODE_SETTING, -1);
        boolean expectedRadioOn = (currentSimMode & (MODE_PHONE1_ONLY << slot)) != 0;
        logInEng("isExpectedRadioStateOn, slot=" + slot +
                ", expectedRadioOn=" + expectedRadioOn);
        return expectedRadioOn;
    }

    private void logInEng(String s) {
        if (ENG_LOAD) {
            Log.d(TAG, s);
        }
    }
}
