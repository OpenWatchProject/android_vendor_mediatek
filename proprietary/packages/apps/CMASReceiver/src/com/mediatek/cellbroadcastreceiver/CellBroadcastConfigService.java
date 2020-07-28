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

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.telephony.CellBroadcastMessage;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.cdma.sms.SmsEnvelope;
import com.android.internal.telephony.gsm.SmsCbConstants;
import com.android.internal.telephony.gsm.SmsBroadcastConfigInfo;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.cmas.ext.ICmasMainSettingsExt;
import com.mediatek.cmas.ext.ICmasSimSwapExt;
import com.mediatek.internal.telephony.gsm.MtkSmsCbConstants;

import mediatek.telephony.MtkSmsManager;



//import static com.android.cellbroadcastreceiver.CellBroadcastReceiver.DBG;

/**
 * This service manages enabling and disabling ranges of message identifiers
 * that the radio should listen for. It operates independently of the other
 * services and runs at boot time and after exiting airplane mode.
 *
 * Note that the entire range of emergency channels is enabled. Test messages
 * and lower priority broadcasts are filtered out in CellBroadcastAlertService
 * if the user has not enabled them in settings.
 *
 * TODO: add notification to re-enable channels after a radio reset.
 */
public class CellBroadcastConfigService extends IntentService {
    private static final String TAG = "[CMAS]CellBroadcastConfigService";
    private static final boolean DBG = true;

    static final String ACTION_ENABLE_CHANNELS = "ACTION_ENABLE_CHANNELS";
    public static final String ACTION_SIM_STATE_CHANGED = "android.intent.action.SIM_STATE_CHANGED";
    public static final String STORED_SUBSCRIBER_ID = "stored_subscriber_id";
    public static final String ENABLE_CMAS_RMT_SUPPORT = "enable_cmas_rmt_support";
    public static final String ENABLE_CMAS_EXERCISE_SUPPORT = "enable_cmas_exercise_support";
    public static final String ENABLE_CMAS_OPERATOR_CHOICE_SUPPORT
                                 = "enable_cmas_operator_choice_support";
    public static final String RMT_KEY = "rmtkey";
    public static final String OPERATOR_CHOICE_KEY = "operatorchoicekey";
    public static final String EXERCISE_KEY = "exerkey";
    public static final int MESSAGE_ID_CMAS_ALERT_CHILE_EMERGENCY = 0x113B; //Chile 4411
    public CellBroadcastConfigService() {
        super(TAG);          // use class name for worker thread name
    }

    /**
     * Returns true if this is a standard or operator-defined emergency alert message.
     * This includes all ETWS and CMAS alerts, except for AMBER alerts.
     * @param message the message to test
     * @return true if the message is an emergency alert; false otherwise
     */
    static boolean isEmergencyAlertMessage(CellBroadcastMessage message) {
        Log.d(TAG, "enter isEmergencyAlertMessage");
        if (message.isEmergencyAlertMessage()) {
            return true;
        }

        if (message.getServiceCategory() ==
            SmsCbConstants.MESSAGE_ID_CMAS_ALERT_CHILD_ABDUCTION_EMERGENCY) {
            return true;
        }

        return false;
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        if (DBG) {
            log(" onHandleIntent " + intent);
        }
        if (ACTION_ENABLE_CHANNELS.equals(intent.getAction())) {
            try {

                SubscriptionManager subManager = SubscriptionManager.from(getApplicationContext());
                int subId = SubscriptionManager.getDefaultSmsSubscriptionId();
                if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                    subId = SubscriptionManager.getDefaultSubscriptionId();
                    if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID &&
                            subManager != null) {
                        int [] subIds = subManager.getActiveSubscriptionIdList();
                        if (subIds.length != 0) {
                            subId = subIds[0];
                        }
                    }
                }

                if (subManager != null) {
                    // Retrieve all the active sub ids. We only want to enable
                    // cell broadcast on the sub we are interested in and we'll disable
                    // it on other subs so the users will not receive duplicate messages from
                    // multiple carriers (e.g. for multi-sim users).
                    int [] subIds = subManager.getActiveSubscriptionIdList();
                    int slotIndex = intent.getIntExtra("SLOTKEY", -1);
                    if (subIds.length != 0) {
                        for (int id : subIds) {
                            SmsManager manager = SmsManager.getSmsManagerForSubscriptionId(id);
                            if (manager != null) {
                                //if dual sim support,only set one slot channel
                                //when app settings update.
                                if (CmasConfigManager.getGeminiSupport() && slotIndex >= 0) {
                                    int slotId = SubscriptionManager.getSlotIndex(id);
                                    log("slotId = " + slotId + ",slotIndex = " + slotIndex);
                                    if (slotId != slotIndex) {
                                        continue;
                                    }
                                }
                                if (id == subId) {
                                    // Enable cell broadcast messages on this sub.
                                    log("Enable CellBroadcast on sub " + id);
                                    setCellBroadcastOnSub(manager, id, true);
                                } else {
                                    // Disable all cell broadcast message on this sub.
                                    // This is only for multi-sim scenario. For single SIM device
                                    // we should not reach here.
                                    if (CmasConfigManager.getGeminiSupport()) {
                                        setCellBroadcastOnSub(manager, id, true);
                                    }
                                }
                            }
                        }
                    }
                    else {
                        // For no sim scenario.
                        SmsManager manager = SmsManager.getDefault();
                        if (manager != null) {
                            setCellBroadcastOnSub(manager,
                                    SubscriptionManager.INVALID_SUBSCRIPTION_ID, true);
                        }
                    }
                }
            } catch (Exception ex) {
                Log.e(TAG, "exception enabling cell broadcast channels", ex);
            }
        } else if (CellBroadcastReceiver.EXERCISE_PREFERENCE_CHANGED.equals(intent.getAction()) ||
            CellBroadcastReceiver.RMT_PREFERENCE_CHANGED.equals(intent.getAction()) ||
            CellBroadcastReceiver.OPERATOR_CHOICE_PREFERENCE_CHANGED.equals(intent.getAction())) {

            setEngModeChannelOnSub(intent);

        } else if (CellBroadcastReceiver.SMS_STATE_CHANGED_ACTION.equals(intent.getAction())) {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
            int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, 1);
            int slotId = intent.getIntExtra(PhoneConstants.PHONE_KEY, 0);
            String slotKey = CMASUtils.getSlotKey(slotId);
            boolean enableCB = false;
            if (!prefs.contains(slotKey + CheckBoxAndSettingsPreference.KEY_ENABLE_CELLBROADCAST)) {
                enableCB = true;
                Log.d(TAG, "do not contain the enable_cell_broadcast ");
            } else {
                enableCB = prefs.getBoolean(slotKey +
                        CheckBoxAndSettingsPreference.KEY_ENABLE_CELLBROADCAST, false);
                Log.d(TAG, "contain the enable_cell_broadcast,enableCB = " + enableCB);
            }
            Log.d(TAG, "SMS_STATE_CHANGED_ACTION enableCB " + enableCB);
            if (enableCB) {
                if (MtkSmsManager.getSmsManagerForSubscriptionId(subId).
                            activateCellBroadcastSms(true)) {
                    log("enable CB after SMS_STATE_CHANGED_ACTION arrived");
                    CellBroadcastReceiver.startConfigService(this);
                } else {
                    log("failed to enable CB after SMS_STATE_CHANGED_ACTION arrived");
                }
            }
        } else if (ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
            TelephonyManager tm = (TelephonyManager) getApplicationContext().getSystemService(
                    Context.TELEPHONY_SERVICE);
            Log.d(TAG, "ACTION_SIM_STATE_CHANGED " + tm.getSimState());
            String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            if (tm != null) {
                if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra)) {
                    int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                    String subscriberId = "";
                    subscriberId = tm.getSubscriberId(subId);
                    int slotId = intent.getIntExtra(PhoneConstants.PHONE_KEY, 0);
                    String slotKey = CMASUtils.getSlotKey(slotId);
                    Log.d(TAG, "subId = " + subId + " subscriberId = "
                            + subscriberId + " slotKey = " + slotKey);
                    SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
                    String storedScriberId = prefs.getString(slotKey + STORED_SUBSCRIBER_ID, "");
                    Log.d(TAG, "storedScriberId = " + storedScriberId);
                    if (!TextUtils.isEmpty(subscriberId)
                            && (TextUtils.isEmpty(storedScriberId) ||
                                !subscriberId.equals(storedScriberId))) {
                        SharedPreferences.Editor editor = prefs.edit();
                        editor.putString(slotKey + STORED_SUBSCRIBER_ID, subscriberId);
                        editor.commit();
                        editor.clear();
                        ICmasSimSwapExt iCmasSimSwap = (ICmasSimSwapExt) CellBroadcastPluginManager
                                .getCellBroadcastPluginObject(
                                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_SIM_SWAP);
                        iCmasSimSwap.simSwap(getApplicationContext(), R.xml.default_preference);
                    }
                    // CellBroadcastReceiver.startConfigService(this);
                }
            }
        }
    }

    private void setEngModeChannelOnSub(Intent intent) {
        int slotId = intent.getIntExtra(PhoneConstants.SLOT_KEY, 0);
        log("Enable CellBroadcast on slot id " + slotId);
        final int[] subIds = SubscriptionManager.getSubId(slotId);
        SmsManager smsManager = SmsManager.getDefault();
        if (subIds != null && subIds.length > 0) {
            int subId = subIds[0];
            Log.d(TAG, "valid subIds subId = " + subId);
            smsManager = SmsManager.getSmsManagerForSubscriptionId(subId);
        }
        String slotKey = CMASUtils.getSlotKey(slotId);

        if (CellBroadcastReceiver.EXERCISE_PREFERENCE_CHANGED.equals(intent.getAction())) {
            Boolean exerKey = intent.getBooleanExtra(EXERCISE_KEY, false);
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
            SharedPreferences.Editor editor = prefs.edit();
            editor.putBoolean(slotKey + ENABLE_CMAS_EXERCISE_SUPPORT, exerKey);
            editor.commit();
            editor.clear();
            setCellBroadcastRange(smsManager, exerKey,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE);
            setCellBroadcastRange(smsManager, exerKey,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE_LANGUAGE,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE_LANGUAGE);
            Log.d(TAG, "SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE"
                       + exerKey);
        } else if (CellBroadcastReceiver.RMT_PREFERENCE_CHANGED.equals(intent.getAction())) {
            Boolean rmtKey = intent.getBooleanExtra(RMT_KEY, false);
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
            SharedPreferences.Editor editor = prefs.edit();
            editor.putBoolean(slotKey + ENABLE_CMAS_RMT_SUPPORT, rmtKey);
            editor.commit();
            editor.clear();
            // Enable/Disable CDMA CMAS test messages.
            if (CellBroadcastReceiver.phoneIsCdma(getApplicationContext())) {
                setCellBroadcastRange(smsManager, rmtKey,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_CDMA,
                        SmsEnvelope.SERVICE_CATEGORY_CMAS_TEST_MESSAGE,
                        SmsEnvelope.SERVICE_CATEGORY_CMAS_TEST_MESSAGE);
            }
            setCellBroadcastRange(smsManager, rmtKey,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_REQUIRED_MONTHLY_TEST,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_REQUIRED_MONTHLY_TEST);
            setCellBroadcastRange(smsManager, rmtKey,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_REQUIRED_MONTHLY_TEST_LANGUAGE,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_REQUIRED_MONTHLY_TEST_LANGUAGE);
            Log.d(TAG, "SmsCbConstants.MESSAGE_ID_CMAS_ALERT_REQUIRED_MONTHLY_TEST"
                       + rmtKey);
        } else if (CellBroadcastReceiver.OPERATOR_CHOICE_PREFERENCE_CHANGED
                     .equals(intent.getAction())) {
            Boolean optChoiceKey = intent.getBooleanExtra(OPERATOR_CHOICE_KEY, false);
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
            SharedPreferences.Editor editor = prefs.edit();
            editor.putBoolean(slotKey + ENABLE_CMAS_OPERATOR_CHOICE_SUPPORT, optChoiceKey);
            editor.commit();
            editor.clear();
            setCellBroadcastRange(smsManager, optChoiceKey,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_OPERATOR_DEFINED_USE,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_OPERATOR_DEFINED_USE);
            setCellBroadcastRange(smsManager, optChoiceKey,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_OPERATOR_DEFINED_USE_LANGUAGE,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_OPERATOR_DEFINED_USE_LANGUAGE);
            Log.d(TAG, "SmsCbConstants.MESSAGE_ID_CMAS_ALERT_OPERATOR_DEFINED_USE"
                       + optChoiceKey);
        }
    }

    /**
     * Enable/disable cell broadcast messages id on one subscription
     * This includes all ETWS and CMAS alerts.
     * @param manager SMS manager
     * @param subId Subscription id
     * @param enableForSub True if want to enable messages on this sub (e.g default SMS). False
     *                     will disable all messages
     */
    private void setCellBroadcastOnSub(SmsManager manager, int subId, boolean enableForSub) {
        boolean isSetSuccess = false;
        int slotId = SubscriptionManager.getSlotIndex(subId);
        String slotKey = CMASUtils.getSlotKey(slotId);
        SharedPreferences prefs = PreferenceManager.
                        getDefaultSharedPreferences(this);
        boolean enableAlerts = CmasConfigManager.isAlertsEnabled(getApplicationContext(), slotKey);
        log("enable channels? " + enableAlerts + " subId = " + subId +
                " slotKey = " + slotKey);
        try {
            if (enableAlerts) {
                /** Enable CDMA CMAS series messages. */
                // Enable CDMA Presidential messages.
                if (CellBroadcastReceiver.phoneIsCdma(getApplicationContext())) {
                    setCellBroadcastRange(manager, true,
                            SmsManager.CELL_BROADCAST_RAN_TYPE_CDMA,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_PRESIDENTIAL_LEVEL_ALERT,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_PRESIDENTIAL_LEVEL_ALERT);
                }

                /** Enable GSM ETWS series messages. */
                boolean enableEtws = enableForSub &&
                        CmasConfigManager.isEtwsEnabled(getApplicationContext());
                // Enable/Disable GSM ETWS messages (4352 ~ 4354).
                setCellBroadcastRange(manager, enableEtws,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_ETWS_EARTHQUAKE_WARNING,
                        SmsCbConstants.MESSAGE_ID_ETWS_EARTHQUAKE_AND_TSUNAMI_WARNING);

                // Enable/Disable GSM ETWS other messages (4356).
                setCellBroadcastRange(manager, enableEtws,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_ETWS_OTHER_EMERGENCY_TYPE,
                        SmsCbConstants.MESSAGE_ID_ETWS_OTHER_EMERGENCY_TYPE);

                // Enable/Disable GSM ETWS test messages (4355).
                setCellBroadcastRange(manager, enableEtws,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_ETWS_TEST_MESSAGE,
                        SmsCbConstants.MESSAGE_ID_ETWS_TEST_MESSAGE);

                /** Enable GSM CMAS series messages. */

                // Enable/Disable GSM CMAS presidential message (4370).
                setCellBroadcastRange(manager, true,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_PRESIDENTIAL_LEVEL,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_PRESIDENTIAL_LEVEL);
                setCellBroadcastRange(manager, true,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_PRESIDENTIAL_LEVEL_LANGUAGE,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_PRESIDENTIAL_LEVEL_LANGUAGE);

                // enable channels by preferences

                boolean isExtremEnable = enableForSub && CmasConfigManager.
                    isExtremEnable(getApplicationContext(), slotKey);

                // Enable/Disable CDMA CMAS extreme messages.
                if (CellBroadcastReceiver.phoneIsCdma(getApplicationContext())) {
                    setCellBroadcastRange(manager, isExtremEnable,
                            SmsManager.CELL_BROADCAST_RAN_TYPE_CDMA,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_EXTREME_THREAT,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_EXTREME_THREAT);
                }

                // Enable/Disable GSM CMAS extreme messages (4371~4372).
                setCellBroadcastRange(manager, isExtremEnable,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXTREME_IMMEDIATE_OBSERVED,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXTREME_IMMEDIATE_LIKELY);

                // Enable/Disable GSM CMAS extreme messages for additional langs (4384~4385).
                setCellBroadcastRange(manager, isExtremEnable,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.
                        MESSAGE_ID_CMAS_ALERT_EXTREME_IMMEDIATE_OBSERVED_LANGUAGE,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXTREME_IMMEDIATE_LIKELY_LANGUAGE);

                boolean isSevereEnable = enableForSub && CmasConfigManager.
                    isSevereEnable(getApplicationContext(), slotKey);
                // Enable/Disable CDMA CMAS severe messages.
                if (CellBroadcastReceiver.phoneIsCdma(getApplicationContext())) {
                    setCellBroadcastRange(manager, isSevereEnable,
                            SmsManager.CELL_BROADCAST_RAN_TYPE_CDMA,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_SEVERE_THREAT,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_SEVERE_THREAT);
                }
                // Enable/Disable GSM CMAS severe messages (4373~4378).
                setCellBroadcastRange(manager, isSevereEnable,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXTREME_EXPECTED_OBSERVED,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_SEVERE_EXPECTED_LIKELY);
                // Enable/Disable GSM CMAS severe messages for additional languages (4386~4391).
                setCellBroadcastRange(manager, isSevereEnable,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXTREME_EXPECTED_OBSERVED_LANGUAGE,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_SEVERE_EXPECTED_LIKELY_LANGUAGE);

                boolean isAmberEnable = enableForSub && CmasConfigManager.
                    isAmberEnable(getApplicationContext(), slotKey);
                // Enable/Disable CDMA CMAS amber alert messages.
                if (CellBroadcastReceiver.phoneIsCdma(getApplicationContext())) {
                    setCellBroadcastRange(manager, isAmberEnable,
                            SmsManager.CELL_BROADCAST_RAN_TYPE_CDMA,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_CHILD_ABDUCTION_EMERGENCY,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_CHILD_ABDUCTION_EMERGENCY);
                }

                // Enable/Disable GSM CMAS amber alert messages (4379).
                setCellBroadcastRange(manager, isAmberEnable,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_CHILD_ABDUCTION_EMERGENCY,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_CHILD_ABDUCTION_EMERGENCY);
                // Enable/Disable GSM CMAS amber alert messages for additional languages (4392).
                setCellBroadcastRange(manager, isAmberEnable,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_CHILD_ABDUCTION_EMERGENCY_LANGUAGE,
                        SmsCbConstants.
                        MESSAGE_ID_CMAS_ALERT_CHILD_ABDUCTION_EMERGENCY_LANGUAGE);

                boolean isRmtEnable = enableForSub && CmasConfigManager.
                    isRmtEnable(getApplicationContext(), slotKey);
                // Enable/Disable CDMA CMAS test messages.
                if (CellBroadcastReceiver.phoneIsCdma(getApplicationContext())) {
                    setCellBroadcastRange(manager, isRmtEnable,
                            SmsManager.CELL_BROADCAST_RAN_TYPE_CDMA,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_TEST_MESSAGE,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_TEST_MESSAGE);
                }

                // Enable/Disable GSM CMAS test messages (4380~4382).
                setCellBroadcastRange(manager, isRmtEnable,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_REQUIRED_MONTHLY_TEST,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_REQUIRED_MONTHLY_TEST);
                setCellBroadcastRange(manager, isRmtEnable,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_REQUIRED_MONTHLY_TEST_LANGUAGE,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_REQUIRED_MONTHLY_TEST_LANGUAGE);

                boolean isExerciseEnable = enableForSub && CmasConfigManager.
                        isExerciseEnable(getApplicationContext(), slotKey);
                setCellBroadcastRange(manager, isExerciseEnable,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE);
                setCellBroadcastRange(manager, isExerciseEnable,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE_LANGUAGE,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE_LANGUAGE);

                boolean isCmspEnable = enableForSub && prefs.getBoolean(slotKey +
                         CellBroadcastConfigService.ENABLE_CMAS_OPERATOR_CHOICE_SUPPORT, false);
                setCellBroadcastRange(manager, isCmspEnable,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_OPERATOR_DEFINED_USE,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_OPERATOR_DEFINED_USE);
                setCellBroadcastRange(manager, isCmspEnable,
                    SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_OPERATOR_DEFINED_USE_LANGUAGE,
                    SmsCbConstants.MESSAGE_ID_CMAS_ALERT_OPERATOR_DEFINED_USE_LANGUAGE);
                if (CmasConfigManager.isTwProfile()) {
                    boolean isAlertMsgEnable = prefs.getBoolean(
                            CheckBoxAndSettingsPreference.KEY_ENABLE_ALERT_MESSAGE, true);
                    setCellBroadcastRange(manager, isAlertMsgEnable,
                            SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                            MtkSmsCbConstants.MESSAGE_ID_GSMA_ALLOCATED_CHANNEL_911,
                            MtkSmsCbConstants.MESSAGE_ID_GSMA_ALLOCATED_CHANNEL_911);
                    setCellBroadcastRange(manager, isAlertMsgEnable,
                            SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                            MtkSmsCbConstants.MESSAGE_ID_GSMA_ALLOCATED_CHANNEL_919,
                            MtkSmsCbConstants.MESSAGE_ID_GSMA_ALLOCATED_CHANNEL_919);
                }
                ICmasMainSettingsExt mainSettingPlugin = (ICmasMainSettingsExt)
                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS);
                if (mainSettingPlugin != null) {
                    mainSettingPlugin.configOpChannel(this, manager, slotKey);
                }
                setCellBroadcastRange(manager, true,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        4400, 4400);
                // set channels user configured in config.xml
                setAdditionalCbsChannels(manager, enableAlerts);
                Log.d(TAG, "enableEtws = " + enableEtws +
                    ",isExtremEnable = " + isExtremEnable +
                    ",isSevereEnable = " + isSevereEnable +
                    ",isAmberEnable = " + isAmberEnable +
                    ",isRmtEnable = " + isRmtEnable +
                    ",isExerciseEnable = " + isExerciseEnable +
                    ",isCmspEnable = " + isCmspEnable);
            } else {
                // No emergency channel system property, disable all
                // emergency channels except Presidential alert
                MtkSmsManager mtkSmsmanager = MtkSmsManager.getSmsManagerForSubscriptionId(subId);
                if (!mtkSmsmanager.queryCellBroadcastSmsActivation()) {
                    log("queryCellBroadcastSmsActivation=false");
                    boolean isActivateSuccess =
                        mtkSmsmanager.activateCellBroadcastSms(true);
                    log("  isActivateSuccess= " + isActivateSuccess);
                }

                if (CellBroadcastReceiver.phoneIsCdma(getApplicationContext())) {
                    setCellBroadcastRange(manager, true,
                            SmsManager.CELL_BROADCAST_RAN_TYPE_CDMA,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_PRESIDENTIAL_LEVEL_ALERT,
                            SmsEnvelope.SERVICE_CATEGORY_CMAS_PRESIDENTIAL_LEVEL_ALERT);
                }

                 // Enable/Disable GSM CMAS presidential message (4370).
                setCellBroadcastRange(manager, true,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_PRESIDENTIAL_LEVEL,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_PRESIDENTIAL_LEVEL);
                setCellBroadcastRange(manager, true,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_PRESIDENTIAL_LEVEL_LANGUAGE,
                        SmsCbConstants.MESSAGE_ID_CMAS_ALERT_PRESIDENTIAL_LEVEL_LANGUAGE);
                log("If no alert enable, enable president alert");
            }

        } catch (UnsupportedOperationException e) {
            log("UnsupportedOperationException: " + e);
            return;
        } catch (Exception e) {
            log("Exception: " + e);
            return;
        }
        if (!isSetSuccess && CellBroadcastReceiver.sEnableCbMsg != null) {
            Log.e(TAG, "faied when enabling cell broadcast channels");
            CellBroadcastReceiver.sEnableCbMsg.what = CheckBoxAndSettingsPreference.
                                                 MESSAGE_ENABLE_CB_END_ERROR;
        }

        if (CellBroadcastReceiver.sEnableCbMsg != null) {
            CellBroadcastReceiver.sEnableCbMsg.sendToTarget();
            CellBroadcastReceiver.sEnableCbMsg = null;
        }
    }

    /**
     * Enable/disable cell broadcast with messages id range.
     * @param manager SMS manager
     * @param enable True for enabling cell broadcast with id range, otherwise for disabling.
     * @param type GSM or CDMA
     * @param start Cell broadcast id range start
     * @param end Cell broadcast id range end
     */
    private boolean setCellBroadcastRange(
            SmsManager manager, boolean enable, int type, int start, int end) {
        if (enable) {
            return manager.enableCellBroadcastRange(start, end, type);
        } else {
            return manager.disableCellBroadcastRange(start, end, type);
        }
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    private void setAdditionalCbsChannels(SmsManager manager, boolean enableAlerts) {
        log("setAdditionalCbsChannels, enableAlerts:" + enableAlerts);
        int[] channels = CmasConfigManager.getAdditionalChannels();
        if (channels != null && channels.length > 0) {
            for (int ch : channels) {
                log("setAdditionalCbsChannels, channel = " + ch);
                setCellBroadcastRange(manager, enableAlerts,
                        SmsManager.CELL_BROADCAST_RAN_TYPE_GSM, ch, ch);
            }
        } else {
            log("setAdditionalCbsChannels, no user configured channels");
        }
    }

}
