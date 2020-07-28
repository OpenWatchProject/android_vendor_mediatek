/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.cellbroadcastreceiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.PreferenceManager;
import android.provider.Telephony;
import android.telephony.CellBroadcastMessage;
import android.telephony.ServiceState;
import android.telephony.SmsCbMessage;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaSmsCbProgramData;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.cdma.sms.SmsEnvelope;

import com.mediatek.common.carrierexpress.CarrierExpressManager;

import com.mediatek.cmas.ext.ICmasDuplicateMessageExt;
import com.mediatek.cmas.ext.ICmasMessageInitiationExt;
import com.mediatek.cmas.ext.OpCmasCustomizationFactoryBase;

public class CellBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "[CMAS]CellBroadcastReceiver";
    static final boolean DBG = true;    // STOPSHIP: change to false before ship
    private static boolean isBootCompleted = false;
    private static int sServiceState = -1;

    public static final String SMS_STATE_CHANGED_ACTION =
        "android.provider.Telephony.SMS_STATE_CHANGED";

    public static final String RMT_PREFERENCE_CHANGED =
            "com.mediatek.cmasengmode.rmtkey";
    public static final String EXERCISE_PREFERENCE_CHANGED =
            "com.mediatek.cmasengmode.exerkey";
    public static final String OPERATOR_CHOICE_PREFERENCE_CHANGED =
            "com.mediatek.cmasengmode.operatorChoicekey";

    public static int sSlotId = 0;
    public static Message sEnableCbMsg = null;

    @Override
    public void onReceive(Context context, Intent intent) {
        String systemAction = intent.getAction();
        Log.d(TAG, "onReceive systemaction"  + systemAction);
        if (SMS_STATE_CHANGED_ACTION.equals(systemAction) ||
            Intent.ACTION_BOOT_COMPLETED.equals(systemAction) ||
            Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(systemAction) ||
            CellBroadcastConfigService.ACTION_SIM_STATE_CHANGED.equals(systemAction) ||
            Intent.ACTION_LOCALE_CHANGED.equals(systemAction) ||
            TelephonyIntents.ACTION_SERVICE_STATE_CHANGED.equals(systemAction) ||
            CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED.equals(systemAction)) {
            onReceiveWithPrivilege(context, intent, false);
        }
    }

    protected void onReceiveWithPrivilege(Context context, Intent intent, boolean privileged) {
        if (DBG) {
            log("onReceiveWithPrivilege " + intent + " " + privileged);
        }

        String action = intent.getAction();

        //if (Sms.Intents.SMS_STATE_CHANGED_ACTION.equals(action)){
        if (SMS_STATE_CHANGED_ACTION.equals(action)) {
            boolean isReady = intent.getBooleanExtra("ready", false);
            sSlotId = intent.getIntExtra("simId", 0);

            if (DBG) {
                log("SMS_STATE_CHANGED_ACTION " + isReady + " " + sSlotId);
            }
            if (isReady) {
                intent.setClass(context, CellBroadcastConfigService.class);
                context.startService(intent);
            }
        } else if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            isBootCompleted = true;
            startConfigService(context);
        } else if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
            isBootCompleted = false;
            boolean airplaneModeOn = intent.getBooleanExtra("state", false);
            Log.d(TAG, "ACTION_AIRPLANE_MODE_CHANGED, airplaneModeOn "  + airplaneModeOn);
            if (!airplaneModeOn) {
                startConfigService(context);
            } else {
                intent.setClass(context, CellBroadcastAlertService.class);
                context.startService(intent);
            }
        } else if (Telephony.Sms.Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION.equals(action) ||
                Telephony.Sms.Intents.SMS_CB_RECEIVED_ACTION.equals(action)) {
            // ignore incoming messages if CMAS option is off.
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
            boolean enableCB = prefs.getBoolean(
                    CheckBoxAndSettingsPreference.KEY_ENABLE_CELLBROADCAST, true);
            if (!enableCB) {
                ICmasMessageInitiationExt initMsg = (ICmasMessageInitiationExt)
                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);
                if (CmasConfigManager.isTwProfile() ||
                        (initMsg != null && initMsg.needPresidentAlert())) {
                    log("Disable all alert but Need to enable President alert");
                     Bundle extras = intent.getExtras();
                        if (extras == null) {
                            Log.e(TAG, "Taiwan: received SMS_CB_RECEIVED_ACTION with no extras!");
                            return;
                        }

                        final SmsCbMessage message = (SmsCbMessage) extras.get("message");

                        if (message == null) {
                            Log.e(TAG, "TWC:received SMS_CB_RECEIVED_ACTION with no message extra");
                            return;
                        }

                        final CellBroadcastMessage cbm = new CellBroadcastMessage(message);
                        if (!(cbm.getServiceCategory() == 4370 ||
                                cbm.getServiceCategory() == 4383)) {
                            log("TWC:ignore CB RECEIVED ACTION because disabled cell_broadcast");
                            return;
                        }

                } else {
                    log("ignore CB RECEIVED ACTION because disabled enable_cell_broadcast");
                    return;
                }

            }

            // If 'privileged' is false, it means that the intent was delivered to the base
            // no-permissions receiver class.  If we get an SMS_CB_RECEIVED message that way, it
            // means someone has tried to spoof the message by delivering it outside the normal
            // permission-checked route, so we just ignore it.
            if (privileged) {
                intent.setClass(context, CellBroadcastAlertService.class);
                context.startService(intent);
            } else {
                Log.e(TAG, "ignoring unprivileged action received " + action);
            }
        } else if (Telephony.Sms.Intents.SMS_SERVICE_CATEGORY_PROGRAM_DATA_RECEIVED_ACTION
                .equals(action)) {
            if (privileged) {
                CdmaSmsCbProgramData[] programDataList = (CdmaSmsCbProgramData[])
                        intent.getParcelableArrayExtra("program_data_list");
                if (programDataList != null) {
                    handleCdmaSmsCbProgramData(context, programDataList);
                } else {
                    Log.e(TAG, "SCPD intent received with no program_data_list");
                }
            } else {
                Log.e(TAG, "ignoring unprivileged action received " + action);
            }
        } else if (CellBroadcastConfigService.ACTION_SIM_STATE_CHANGED.equals(action)) {
            TelephonyManager tm = (TelephonyManager) context.getSystemService(
                    Context.TELEPHONY_SERVICE);
            Log.d(TAG, "ACTION_SIM_STATE_CHANGED " + tm.getSimState());
            String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            Log.d(TAG, "ICC key state " + stateExtra);
            if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra)) {
                intent.setClass(context, CellBroadcastConfigService.class);
                context.startService(intent);
            }
        } else if (Intent.ACTION_LOCALE_CHANGED.equals(action)) {
            Log.d(TAG, "action = " + action);
            setPluginClientLocaleChanged(true);
        } else if (RMT_PREFERENCE_CHANGED.equals(action)) {
            Log.d(TAG, "RMT_PREFERENCE_CHANGED = " + action);
            Boolean rmtKey = intent.getBooleanExtra(CellBroadcastConfigService.RMT_KEY, false);
            intent.setClass(context, CellBroadcastConfigService.class);
            intent.putExtra(CellBroadcastConfigService.RMT_KEY, rmtKey);
            context.startService(intent);
        } else if (EXERCISE_PREFERENCE_CHANGED.equals(action)) {
            Log.d(TAG, "EXERCISE_PREFERENCE_CHANGED = " + action);
            Boolean exerKey = intent.getBooleanExtra(CellBroadcastConfigService.EXERCISE_KEY,
                    false);
            intent.setClass(context, CellBroadcastConfigService.class);
            intent.putExtra(CellBroadcastConfigService.EXERCISE_KEY, exerKey);
            context.startService(intent);
        } else if (OPERATOR_CHOICE_PREFERENCE_CHANGED.equals(action)) {
            Log.d(TAG, "OPERATOR_CHOICE_PREFERENCE_CHANGED = " + action);
            Boolean cmspKey = intent.getBooleanExtra(CellBroadcastConfigService.OPERATOR_CHOICE_KEY,
                    false);
            intent.setClass(context, CellBroadcastConfigService.class);
            intent.putExtra(CellBroadcastConfigService.OPERATOR_CHOICE_KEY, cmspKey);
            context.startService(intent);
        } else if (TelephonyIntents.ACTION_SERVICE_STATE_CHANGED.equals(action)) {
            if (DBG) {
                log("Intent: " + action);
            }
            ServiceState serviceState = ServiceState.newFromBundle(intent.getExtras());
            if (serviceState != null) {
                int newState = serviceState.getState();
                if (newState != sServiceState) {
                    Log.d(TAG, "Service state changed! " + newState + " Full: " + serviceState +
                            " Current state=" + sServiceState);
                    sServiceState = newState;
                    if (((newState == ServiceState.STATE_IN_SERVICE) ||
                            (newState == ServiceState.STATE_EMERGENCY_ONLY))) {
                        isBootCompleted = true;
                        startConfigService(context);
                    }
                }
            }
        } else if (CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED.equals(action)) {
            Log.d(TAG, "onReceive() reset plugin");
            OpCmasCustomizationFactoryBase.resetOpFactory();
            CellBroadcastPluginManager.resetPluginInterface();
            CellBroadcastPluginManager.initPlugins(context);
            startConfigService(context);
        } else {
            Log.w(TAG, "onReceive() unexpected action " + action);
        }
    }

    /**
     * Handle Service Category Program Data message.
     * TODO: Send Service Category Program Results response message to sender
     *
     * @param context
     * @param programDataList
     */
    private void handleCdmaSmsCbProgramData(Context context,
            CdmaSmsCbProgramData[] programDataList) {
        for (CdmaSmsCbProgramData programData : programDataList) {
            switch (programData.getOperation()) {
                case CdmaSmsCbProgramData.OPERATION_ADD_CATEGORY:
                    tryCdmaSetCategory(context, programData.getCategory(), true);
                    break;

                case CdmaSmsCbProgramData.OPERATION_DELETE_CATEGORY:
                    tryCdmaSetCategory(context, programData.getCategory(), false);
                    break;

                case CdmaSmsCbProgramData.OPERATION_CLEAR_CATEGORIES:
                    tryCdmaSetCategory(context,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_EXTREME_THREAT, false);
                    tryCdmaSetCategory(context,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_SEVERE_THREAT, false);
                    tryCdmaSetCategory(context,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_CHILD_ABDUCTION_EMERGENCY, false);
                    tryCdmaSetCategory(context,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_TEST_MESSAGE, false);
                    break;

                default:
                    Log.e(TAG, "Ignoring unknown SCPD operation " + programData.getOperation());
            }
        }
    }

    private void tryCdmaSetCategory(Context context, int category, boolean enable) {
        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(context);

        switch (category) {
            case SmsEnvelope.SERVICE_CATEGORY_CMAS_EXTREME_THREAT:
                sharedPrefs.edit().putBoolean(
                        CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_EXTREME_ALERTS, enable)
                        .apply();
                break;

            case SmsEnvelope.SERVICE_CATEGORY_CMAS_SEVERE_THREAT:
                sharedPrefs.edit().putBoolean(
                        CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_SEVERE_ALERTS, enable)
                        .apply();
                break;

            case SmsEnvelope.SERVICE_CATEGORY_CMAS_CHILD_ABDUCTION_EMERGENCY:
                sharedPrefs.edit().putBoolean(
                        CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_AMBER_ALERTS, enable).apply();
                break;

            case SmsEnvelope.SERVICE_CATEGORY_CMAS_TEST_MESSAGE:
//                sharedPrefs.edit().putBoolean(
//                       CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_TEST_ALERTS, enable).apply();
                break;

            default:
                Log.w(TAG, "Ignoring SCPD command to " + (enable ? "enable" : "disable")
                        + " alerts in category " + category);
        }
    }

    private void setPluginClientLocaleChanged(boolean status) {
        ICmasDuplicateMessageExt optDetection = (ICmasDuplicateMessageExt)
        CellBroadcastPluginManager.getCellBroadcastPluginObject(
        CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_DUPLICATE_MESSAGE);

        if (optDetection != null) {
            optDetection.setCmasLocaleChange(status);
        }
    }

    /**
     * Tell {@link CellBroadcastConfigService} to enable the CB channels.
     * @param context the broadcast receiver context
     */
    public static void startConfigService(Context context) {
        Intent serviceIntent = new Intent(CellBroadcastConfigService.ACTION_ENABLE_CHANNELS,
                null, context, CellBroadcastConfigService.class);
        serviceIntent.putExtra("isBootCompleted", isBootCompleted);
        context.startService(serviceIntent);
    }

    /**
     * @return true if the phone is a CDMA phone type
     */
    public static boolean phoneIsCdma(Context context) {
        boolean isCdma = false;
        /*try {
            ITelephony phone = ITelephony.Stub.asInterface(ServiceManager.checkService("phone"));
            if (phone != null) {
                isCdma = (phone.getActivePhoneType() == TelephonyManager.PHONE_TYPE_CDMA);
            }
        } catch (RemoteException e) {
            Log.w(TAG, "phone.getActivePhoneType() failed", e);
        }*/
        TelephonyManager tm = (TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE);
        if (tm.getPhoneType() == TelephonyManager.PHONE_TYPE_CDMA) {
            isCdma = true;
        }
        return isCdma;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
