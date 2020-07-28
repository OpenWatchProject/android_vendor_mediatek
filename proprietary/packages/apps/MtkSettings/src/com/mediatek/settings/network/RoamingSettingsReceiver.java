/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.settings.network;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.List;

import mediatek.telephony.MtkCarrierConfigManager;

/**
 * Receiver Class to receive intents related to Roaming settings.
 */
public class RoamingSettingsReceiver extends BroadcastReceiver {

    private static final String TAG = "OP20RoamingSettingsReceiver";
    private static final String ROAMING_INIT = "ROAMING_INIT";
    private static final String PRECISE_DATA_CONNECTION_ACTION
        = "android.intent.action.PRECISE_DATA_CONNECTION_STATE_CHANGED";
    private static final String ANY_DATA_STATE_ACTION
        = "android.intent.action.ANY_DATA_STATE";
    private static final String BOOT_COMPLETED_ACTION
        = "android.intent.action.BOOT_COMPLETED";
    private static PhoneConstants.DataState sDataState = PhoneConstants.DataState.DISCONNECTED;
    private static int sRoamingState = ServiceState.ROAMING_TYPE_NOT_ROAMING;
    private static Context sContext;
    private static MtkTelephonyManagerEx mTelephonyManagerEx;

    @Override
    public void onReceive(Context context, Intent intent) {
        sContext = context.getApplicationContext();
        String action = intent.getAction();

        if (TextUtils.isEmpty(action)) {
            return;
        }

        /*M: Used SIM STATE CHANGED instead of SUBINFO RECORD UPDATE as
            the intent is not able to receive when SIM changed*/
        if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
            String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            int slotId = intent.getIntExtra(PhoneConstants.SLOT_KEY, 0);
            int subId = 0;
            int[] subIds = SubscriptionManager.getSubId(slotId);
            if ((subIds != null) && (subIds.length != 0)) {
                subId = subIds[0];
            }
            Log.d(TAG, "Subinfo Record Update... " + subId);
            if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID &&
                IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra)) {

                CarrierConfigManager configMgr = (CarrierConfigManager) sContext.
                        getSystemService(Context.CARRIER_CONFIG_SERVICE);
                PersistableBundle carrierConfig = configMgr.getConfigForSubId(subId);
                if (null == carrierConfig) {
                    Log.d(TAG, "null == carrierConfig No need to update value");
                    return;
                }
                if (!carrierConfig.getBoolean(MtkCarrierConfigManager.
                        MTK_KEY_ROAMING_BAR_GUARD_BOOL)) {
                    Log.d(TAG, "No need to update value");
                    return;
                }
                mTelephonyManagerEx = MtkTelephonyManagerEx.getDefault();
                setRoamingEnabled(subId);
                try {
                    int phoneId = SubscriptionManager.getPhoneId(subId);
                    Log.d(TAG, "Settings for roaming");
                    int domesticVoice =  android.provider.Settings.Global.getInt(
                            sContext.getContentResolver(),
                            MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING + subId, 1);
                    int domesticData = android.provider.Settings.Global.getInt(
                            sContext.getContentResolver(),
                            MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING + subId, 0);
                    int domesticLteData = android.provider.Settings.Global.getInt(
                            sContext.getContentResolver(),
                        MtkSettingsExt.Global.DOMESTIC_LTE_DATA_ROAMING + subId, 0);
                    int internationalVoice = android.provider.Settings.Global.getInt(
                            sContext.getContentResolver(),
                        MtkSettingsExt.Global.INTERNATIONAL_VOICE_TEXT_ROAMING + subId, 1);
                    int internationalData = android.provider.Settings.Global.getInt(
                            sContext.getContentResolver(),
                        MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING + subId, 1);
                    Log.d(TAG, "DOMESTIC_VOICE_TEXT_ROAMING: " + domesticVoice);
                    Log.d(TAG, "DOMESTIC_DATA_ROAMING: " + domesticData);
                    Log.d(TAG, "DOMESTIC_LTE_DATA_ROAMING: " + domesticLteData);
                    Log.d(TAG, "INTERNATIONAL_VOICE_TEXT_ROAMING: " + internationalVoice);
                    Log.d(TAG, "INTERNATIONAL_DATA_ROAMING: " + internationalData);
                    int dvoig = android.provider.Settings.Global.getInt(
                            sContext.getContentResolver(),
                            MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING_GUARD + subId, 0);
                    int ddatg = android.provider.Settings.Global.getInt(
                            sContext.getContentResolver(),
                            MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING_GUARD + subId, 0);
                    int ivoig = android.provider.Settings.Global.getInt(
                            sContext.getContentResolver(),
                            MtkSettingsExt.Global.INTERNATIONAL_VOICE_ROAMING_GUARD + subId, 1);
                    int idatg = android.provider.Settings.Global.getInt(
                            sContext.getContentResolver(),
                            MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING_GUARD + subId, 0);
                    Log.d(TAG, "DOMESTIC_VOICE_guard: " + dvoig);
                    Log.d(TAG, "DOMESTIC_DATA_guard: " + ddatg);
                    Log.d(TAG, "internatioanl voice guard: " + ivoig);
                    Log.d(TAG, "INTERNATIONAL data guard: " + idatg);
                //}
                } catch (Exception snfe) {
                    Log.e(TAG, "getDataOnRoamingEnabled: SettingNofFoundException snfe=" + snfe);
                }
            }
        } else if (action.equals(ANY_DATA_STATE_ACTION)) {
            Log.d(TAG, "onReceive android.intent.action.ANY_DATA_STATE");
            if (!checkCarrierConfig()) {
                return;
            }
            Log.d(TAG, "onReceive android.intent.action.PRECISE_DATA_CONNECTION_STATE_CHANGED");
            handlePreciseDataConnectionStateChange(intent);
        } else if (action.equals("android.intent.action.SERVICE_STATE")) {
            if (!checkCarrierConfig()) {
                return;
            }
            Log.d(TAG, "onReceive android.intent.action.SERVICE_STATE");
            handleServiceStateChange(intent);
        } else if (action.equals(BOOT_COMPLETED_ACTION)) {
            Log.d(TAG, "onReceive BOOT completed");
            if (!checkCarrierConfig()) {
                return;
            }
            sContext.registerReceiver(this, new IntentFilter(ANY_DATA_STATE_ACTION));
        }
    }

    private boolean checkCarrierConfig() {
        List<SubscriptionInfo> simList
                = SubscriptionManager.from(sContext)
                        .getActiveSubscriptionInfoList();
        if (simList == null) {
            Log.d(TAG, "checkCarrierConfig return false" +
                    " because SubscriptionInfo list = null");
            return false;
        }
        final CarrierConfigManager configManager = (CarrierConfigManager) sContext
            .getSystemService(Context.CARRIER_CONFIG_SERVICE);
        if (null == configManager) {
            return false;
        }
        PersistableBundle pb = configManager.getConfig();
        if (null == pb) {
            return false;
        }
        if (!pb.getBoolean(MtkCarrierConfigManager.
                MTK_KEY_ROAMING_BAR_GUARD_BOOL)) {
            Log.d(TAG, "Not Valid for this OP MCC/MNC, no handling further");
            return false;
        }
        return true;
    }

    /**
     * Enable roaming settings and guard settings.
     * @param subId subId for which change occurred
     */
    public void setRoamingEnabled(int subId) {
        List<SubscriptionInfo> simList = SubscriptionManager.from(sContext).
                getActiveSubscriptionInfoList();
        if (simList == null) {
            return;
        }
        if (android.provider.Settings.Global.getInt(sContext.getContentResolver(),
                ROAMING_INIT + subId, 0) == 1) {
            Log.d(TAG, "for subId " + subId + "and value already 1");
            return;
        } else {
            Log.d(TAG, "for subId " + subId + "and update roaming value");
            android.provider.Settings.Global.putInt(sContext.getContentResolver(),
                ROAMING_INIT + subId, 1);
        }
        try {
            // Inspite of checking simCount check Sim count from TelephonyManger
            // to know whether the hardware supports single Sim or not.
            final int phoneId = SubscriptionManager.getPhoneId(subId);
            Log.d(TAG, "update for subId " + subId + "and update roaming value");
            // Initialize Roaming settings
            android.provider.Settings.Global.putInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING + subId, 1);
            android.provider.Settings.Global.putInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING + subId, 0);
            android.provider.Settings.Global.putInt(
                    sContext.getContentResolver(),
                    MtkSettingsExt.Global.DOMESTIC_LTE_DATA_ROAMING + subId, 0);
            android.provider.Settings.Global.putInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.INTERNATIONAL_VOICE_TEXT_ROAMING + subId, 1);
            android.provider.Settings.Global.putInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING + subId, 1);
            // Initialize Guard settings
            android.provider.Settings.Global.putInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING_GUARD + subId, 0);
            android.provider.Settings.Global.putInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING_GUARD + subId, 0);
            android.provider.Settings.Global.putInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.INTERNATIONAL_VOICE_ROAMING_GUARD + subId, 1);
            android.provider.Settings.Global.putInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING_GUARD + subId, 0);
            /*android.provider.Settings.Global.putInt(phone.getContext().getContentResolver(),
                Settings.Global.INTERNATIONAL_TEXT_ROAMING_GUARD + subId, 1);*/
            /**
             * response[0] : phone id
             * response[1] : international_voice_text_roaming (0,1)
             * response[2] : international_data_roaming (0,1)
             * response[3] : domestic_voice_text_roaming (0,1)
             * response[4] : domestic_data_roaming (0,1)
             * response[5] : domestic_LTE_data_roaming (1) same as domestic data*/
            final int roamingSettingsArray[] = {phoneId, 1, 1, 1, 0, 0};

//            MtkGsmCdmaPhone gsmCdmaphone =
//                    (MtkGsmCdmaPhone) PhoneFactory.getPhone(phone.getPhoneId());
//            gsmCdmaphone.setRoamingEnable(roamingSettingsArray, null);
            new AsyncTask<Void, Void, Boolean>() {
                @Override
                protected Boolean doInBackground(Void... voids) {
                    Log.d(TAG, "doInBackground init phoneId = " + phoneId
                            + " responseArray = " + roamingSettingsArray.toString());
                    boolean isSucess = mTelephonyManagerEx
                            .setRoamingEnable(phoneId, roamingSettingsArray);
                    return isSucess;
                }

                @Override
                protected void onPostExecute(Boolean result) {
                    Log.d(TAG, "onPostExecute handleSetRoamingSettings onPostExecute = " + result);
                }
            }.execute();
        } catch (Exception snfe) {
            Log.e(TAG, "getDataOnRoamingEnabled: SettingNofFoundException snfe=" + snfe);
        }
    }

    private void handlePreciseDataConnectionStateChange(Intent intent) {
        String state = intent.getStringExtra(PhoneConstants.STATE_KEY);
        PhoneConstants.DataState newState;
        if (state != null) {
            newState = Enum.valueOf(PhoneConstants.DataState.class, state);
        } else {
            newState = PhoneConstants.DataState.DISCONNECTED;
        }
        /*int newState = intent.getIntExtra(PhoneConstants.STATE_KEY,
            TelephonyManager.DATA_UNKNOWN);*/
        String apnType = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);
        Log.d(TAG, "handlePreciseDataConnectionStateChange: apnType= " + apnType
            + ", newState= " + newState
            + ", currentState= " + sDataState);
        if (newState != sDataState && newState == PhoneConstants.DataState.CONNECTED
            && apnType.equals(PhoneConstants.APN_TYPE_DEFAULT)) {
            sDataState = newState;
            if (checkRoamingSetting()) {
                showAlertDialog(sRoamingState);
            }
        } else if (newState != sDataState && newState == PhoneConstants.DataState.DISCONNECTED
            && apnType.equals(PhoneConstants.APN_TYPE_DEFAULT)) {
            sDataState = newState;
        }
    }

    private void handleServiceStateChange(Intent intent) {
        Bundle extras = intent.getExtras();
        if (extras != null) {
            ServiceState serviceState = ServiceState.newFromBundle(extras);
            if (serviceState != null) {
                Log.d(TAG, "handleServiceStateChange, serviceState = " + serviceState);
                if (serviceState.getDataRoaming()) {
                    if (serviceState.getDataRoamingType() == ServiceState.ROAMING_TYPE_DOMESTIC) {
                        sRoamingState = ServiceState.ROAMING_TYPE_DOMESTIC; //2
                    } else if (serviceState.getDataRoamingType() ==
                        ServiceState.ROAMING_TYPE_INTERNATIONAL) {
                        sRoamingState = ServiceState.ROAMING_TYPE_INTERNATIONAL; //3
                    }
                    Log.d(TAG, "ServiceStateChanged:In Roaming,sRoamingState= " + sRoamingState);
                    if (checkRoamingSetting()) {
                        showAlertDialog(sRoamingState);
                    }
                } else {
                    Log.d(TAG, "handleServiceStateChange: getDataRoaming() returns false");
                    sRoamingState = ServiceState.ROAMING_TYPE_NOT_ROAMING; //0
                }
            }
        }
    }

    /**
     * Check if Data Roaming Setting & Roaming Guards are enabled by user.
     * returns true if enabled, false otherwise
     */
    private static boolean checkRoamingSetting() {
        if (sRoamingState == ServiceState.ROAMING_TYPE_NOT_ROAMING) {
            Log.d(TAG, "User Not in roaming, so return false");
            return false;
        }
        if (sDataState != PhoneConstants.DataState.CONNECTED) {
            Log.d(TAG, "Data not connected, so return false, sDataState = " + sDataState);
            return false;
        }
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        int domesticDataRoamingSetting = Settings.Global.getInt(sContext.getContentResolver(),
            MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING + subId, 0);
        int internationalDataRoamingSetting = Settings.Global.getInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING + subId, 0);
        int domesticDataRoamingGuard = Settings.Global.getInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING_GUARD + subId, 0);
        int internationalDataRoamingGuard = Settings.Global.getInt(sContext.getContentResolver(),
                MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING_GUARD + subId, 0);
        Log.d(TAG, "domesticDataRoamingSetting = " + domesticDataRoamingSetting +
            ", internationalDataRoamingSetting = " + internationalDataRoamingSetting +
            ", domesticDataRoamingGuard = " + domesticDataRoamingGuard +
            ", internationalDataRoamingGuard = " + internationalDataRoamingGuard +
            " ,subId= " + subId);
        if ((sRoamingState == ServiceState.ROAMING_TYPE_DOMESTIC &&
            domesticDataRoamingSetting == 1 && domesticDataRoamingGuard == 1)) {
            Log.d(TAG, "Need show domestic roaming dialog");
            return true;
        } else if (sRoamingState == ServiceState.ROAMING_TYPE_INTERNATIONAL &&
            internationalDataRoamingSetting == 1 && internationalDataRoamingGuard == 1) {
            Log.d(TAG, "Need show international roaming dialog");
            return true;
        } else {
            Log.d(TAG, "Roaming setting not enabled, return false");
            return false;
        }
    }

    private static void showAlertDialog(int roamingAlertType) {
        Intent intent = new Intent(sContext, RoamingAlertDialog.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra("Roaming type", roamingAlertType);
        sContext.startActivity(intent);
    }

    /**
     * notify when RoamingSettings are Changed.
     * @param prefKey Preference changed
     */
    public static void notifyRoamingSettingsChanged(String prefKey) {
        if (sRoamingState == ServiceState.ROAMING_TYPE_NOT_ROAMING) {
            Log.d(TAG, "Not in roaming,don't handle settings changed ,prefKey = " + prefKey);
        } else if (sRoamingState == ServiceState.ROAMING_TYPE_DOMESTIC) {
            if (prefKey.equals(RoamingSettings.DOMESTIC_DATA_ROAMING_SETTINGS) ||
                prefKey.equals(GuardSettings.DOMESTIC_DATA_ROAMING_GUARD)) {
                Log.d(TAG, "In domestic roaming,Handle setting change,prefKey = " + prefKey);
                if (checkRoamingSetting()) {
                    showAlertDialog(sRoamingState);
                }
            } else {
                Log.d(TAG, "In domestic roaming,don't handle international,prefKey = " + prefKey);
            }
        } else if (sRoamingState == ServiceState.ROAMING_TYPE_INTERNATIONAL) {
            if (prefKey.equals(RoamingSettings.INTERNATIONAL_DATA_ROAMING_SETTINGS) ||
                prefKey.equals(GuardSettings.INTERNATIONAL_DATA_ROAMING_GUARD)) {
                Log.d(TAG, "In international roaming,Handle setting change,prefKey = " + prefKey);
                if (checkRoamingSetting()) {
                    showAlertDialog(sRoamingState);
                }
            } else {
                Log.d(TAG, "In international roaming,don't handle domestic,prefKey = " + prefKey);
            }
        }
    }
}
