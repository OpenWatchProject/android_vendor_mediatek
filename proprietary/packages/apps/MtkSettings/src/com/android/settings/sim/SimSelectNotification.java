/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.android.settings.sim;

import static android.telephony.TelephonyManager.ACTION_PRIMARY_SUBSCRIPTION_LIST_CHANGED;
import static android.telephony.TelephonyManager.EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE;
import static android.telephony.TelephonyManager.EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE_ALL;
import static android.telephony.TelephonyManager.EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE_DATA;
import static android.telephony.TelephonyManager.EXTRA_SUBSCRIPTION_ID;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
/// M: Add for supporting RSIM.
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import androidx.core.app.NotificationCompat;

import com.android.settings.R;
import com.android.settings.Settings.SimSettingsActivity;
import com.android.settings.Utils;

/// M: Add for SIM Lock feature.
import com.mediatek.internal.telephony.MtkIccCardConstants;
/// M: Add for checking detect type.
import com.mediatek.internal.telephony.MtkSubscriptionManager;
/// M: Add for supporting RSIM.
import com.mediatek.internal.telephony.MtkTelephonyProperties;
/// M: Add for SIM Settings plugin.
import com.mediatek.settings.UtilsExt;
/// M: Add for SIM Settings plugin.
import com.mediatek.settings.ext.ISettingsMiscExt;
/// M: Add for checking airplane mode.
import com.mediatek.settings.sim.TelephonyUtils;

import java.util.List;

public class SimSelectNotification extends BroadcastReceiver {
    private static final String TAG = "SimSelectNotification";
    private static final int NOTIFICATION_ID = 1;

    private static final String SIM_SELECT_NOTIFICATION_CHANNEL =
            "sim_select_notification_channel";

    private static final String ACTION_SIM_SLOT_SIM_MOUNT_CHANGE =
            "com.mediatek.settings.sim.ACTION_SIM_SLOT_SIM_MOUNT_CHANGE";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.d(TAG, "onReceive, action=" + action);

        /// M: Add for opeator dual with broadcast @{
        UtilsExt.getSimManagementExt(context).customBroadcast(intent);
        /// @}

        /// M: Add for auto sanity @{
        if (UtilsExt.shouldDisableForAutoSanity()) {
            Log.d(TAG, "disable for auto sanity.");
            return;
        }
        /// @}

        /// M: Add for ALPS02811539, ignore this event when airplane mode is on. @{
        if (TelephonyUtils.isAirplaneModeOn(context)) {
            Log.d(TAG, "airplane mode is on, ignore.");
            return;
        }
        /// @}

        /// M: Add for ALPS03213464, do not show any dialog when RSIM is enabled. @{
        if (SystemProperties.getInt("ro.vendor.mtk_external_sim_support", 0) == 1) {
            int ignoreDialog = SystemProperties.getInt(
                    MtkTelephonyProperties.PROPERTY_EXTERNAL_DISABLE_SIM_DIALOG, 0);
            if (ignoreDialog == 1) {
                Log.d(TAG, "RSIM present, ignore.");
                return;
            }
        }
        /// @}

        final TelephonyManager telephonyManager = (TelephonyManager)
                context.getSystemService(Context.TELEPHONY_SERVICE);
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(context);
        final int numSlots = telephonyManager.getSimCount();
        List<SubscriptionInfo> sil = subscriptionManager.getActiveSubscriptionInfoList(true);
        int numSims = (sil == null ? 0 : sil.size());

        final boolean isInProvisioning = !Utils.isDeviceProvisioned(context);
        Log.d(TAG, "numSlots=" + numSlots + ", isInProvisioning=" + isInProvisioning);

        // Do not create notifications on single SIM devices or when provisioning i.e. Setup Wizard.
        if (numSlots < 2 || isInProvisioning) {
            return;
        }

        /// M: Add for SIM Lock feature. @{
        final int simLockPolicy = TelephonyUtils.getSimLockPolicy();
        int[] simLockSimValid = null;
        boolean[] simLockSimInserted = null;

        switch (simLockPolicy) {
            case TelephonyUtils.SIM_LOCK_POLICY_UNKNOWN:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT1_ONLY:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT2_ONLY:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT1_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT2_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_CS:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_REVERSE:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_ECC:
                if (!ACTION_SIM_SLOT_SIM_MOUNT_CHANGE.equals(action)) {
                    Log.d(TAG, "policy=" + simLockPolicy + ", ignore action=" + action);
                    return;
                }

                simLockSimValid = new int[numSlots];
                simLockSimInserted = new boolean[numSlots];
                for (int i = 0; i < numSlots; i++) {
                    simLockSimValid[i] = TelephonyUtils.getSimLockSimValid(i);
                    simLockSimInserted[i] = false;
                }
                if (sil != null && sil.size() > 0) {
                    SubscriptionInfo sir;
                    for (int i = 0; i < sil.size(); i++) {
                        sir = sil.get(i);
                        if (sir != null) {
                            simLockSimInserted[sir.getSimSlotIndex()] = true;
                        }
                    }
                }
                break;

            default:
                if (!ACTION_PRIMARY_SUBSCRIPTION_LIST_CHANGED.equals(action)) {
                    Log.d(TAG, "policy=" + simLockPolicy + ", ignore action=" + action);
                    return;
                }
                break;
        }
        /// @}

        if (ACTION_PRIMARY_SUBSCRIPTION_LIST_CHANGED.equals(action)) {
            /// M: Ignore this broadcast when it is invalid. @{
            final int selectType = intent.getIntExtra(
                    EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE, 0);
            Log.d(TAG, "sub info update, type=" + selectType + ", subs=" + sil);
            if (selectType != EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE_ALL
                    && selectType != EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE_DATA) {
                Log.d(TAG, "select type is not supported.");
                return;
            }
            /// @}

            /// M: Ignore this broadcast when the SIM is changed again. @{
            if (selectType == EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE_ALL) {
                final int primarySubId = intent.getIntExtra(EXTRA_SUBSCRIPTION_ID,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                if (primarySubId == SubscriptionManager.INVALID_SUBSCRIPTION_ID
                        || numSims != 1
                        || primarySubId != sil.get(0).getSubscriptionId()) {
                    Log.d(TAG, "SIM is changed again, primarySubId=" + primarySubId
                            + ", numSims=" + numSims);
                    return;
                }
            } else if (selectType == EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE_DATA) {
                if (numSims < 2) {
                    Log.d(TAG, "SIM is changed again, numSims=" + numSims);
                    return;
                }
            } else {
                Log.d(TAG, "select type is not supported.");
                return;
            }
            /// @}
        }

        /// M: Add for SIM Lock feature. @{
        if (ACTION_SIM_SLOT_SIM_MOUNT_CHANGE.equals(action)) {
            final int detectedType = intent.getIntExtra(
                    MtkIccCardConstants.INTENT_KEY_SML_SLOT_DETECTED_TYPE, 0);
            final int simCount = intent.getIntExtra(
                    MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM_COUNT, 0);
            final int simValid1 = intent.getIntExtra(
                    MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM1_VALID,
                    TelephonyUtils.SIM_LOCK_SIM_VALID_UNKNOWN);
            final int simValid2 = intent.getIntExtra(
                    MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM2_VALID,
                    TelephonyUtils.SIM_LOCK_SIM_VALID_UNKNOWN);

            Log.d(TAG, "Check SIM info, detectedType=" + detectedType
                    + ", extraSimCount=" + simCount
                    + ", currentSimCount=" + numSims
                    + ", extraSim1Valid=" + simValid1
                    + ", currentSim1Valid=" + simLockSimValid[0]
                    + ", extraSim2Valid=" + simValid2
                    + ", currentSim2Valid=" + simLockSimValid[1]);

            if (detectedType == MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE
                    || simCount != numSims || simValid1 != simLockSimValid[0]
                    || simValid2 != simLockSimValid[1]) {
                Log.d(TAG, "SIM info is changed again");
                return;
            }

            int numConfirmed = 0;
            for (int i = 0; i < numSlots; i++) {
                switch (simLockSimValid[i]) {
                    case TelephonyUtils.SIM_LOCK_SIM_VALID_UNKNOWN:
                        Log.d(TAG, "SIM[" + i + "] validity is unknown");
                        return;

                    case TelephonyUtils.SIM_LOCK_SIM_VALID_YES:
                    case TelephonyUtils.SIM_LOCK_SIM_VALID_NO:
                        numConfirmed++;
                        break;

                    default:
                        break;
                }
            }

            if (numConfirmed != numSims) {
                Log.d(TAG, "Confirmed SIM count is changed again, numConfirmed="
                        + numConfirmed);
                return;
            }
        }
        /// @}

        // Cancel any previous notifications
        cancelNotification(context);

        if (sil == null || sil.size() < 1) {
            Log.d(TAG, "Subscription list is empty");
            return;
        }

        boolean dataSelected = SubscriptionManager.isUsableSubIdValue(
                SubscriptionManager.getDefaultDataSubscriptionId());
        boolean callSelected = SubscriptionManager.isUsableSubIdValue(
                SubscriptionManager.getDefaultVoiceSubscriptionId());
        boolean smsSelected = SubscriptionManager.isUsableSubIdValue(
                SubscriptionManager.getDefaultSmsSubscriptionId());
        Log.d(TAG, "dataSelected=" + dataSelected + ", callSelected=" + callSelected
                + ", smsSelected=" + smsSelected);

        // If data/call/sms defaults are selected, don't show notification
        if (dataSelected && callSelected && smsSelected) {
            Log.d(TAG, "Data/Call/SMS default sims are selected. No notification");
            return;
        }

        // Create a notification to tell the user that some defaults are missing
        createNotification(context);

        /// M: Add for SIM Settings plugin. @{
        if (!UtilsExt.getSimManagementExt(context).isSimDialogNeeded()
                || !isDialogAllowedForSimLock(simLockPolicy, numSlots,
                        simLockSimInserted, simLockSimValid)) {
            Log.d(TAG, "sim dialog not needed, return.");
            return;
        }
        /// @}

        if (sil.size() == 1) {
            // If there is only one subscription, ask if user wants to use if for everything
            Log.d(TAG, "sim size == 1, SimDialogActivity shown.");
            Intent newIntent = new Intent(context, SimDialogActivity.class);
            newIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
                    Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
            newIntent.putExtra(SimDialogActivity.DIALOG_TYPE_KEY, SimDialogActivity.PREFERRED_PICK);
            newIntent.putExtra(SimDialogActivity.PREFERRED_SIM, sil.get(0).getSimSlotIndex());
            context.startActivity(newIntent);
        } else if (!dataSelected) {
            // If there are multiple, ensure they pick default data
            Log.d(TAG, "SimDialogActivity shown for multiple sims.");
            Intent newIntent = new Intent(context, SimDialogActivity.class);
            newIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
                    Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
            newIntent.putExtra(SimDialogActivity.DIALOG_TYPE_KEY, SimDialogActivity.DATA_PICK);
            context.startActivity(newIntent);
        }
    }

    private void createNotification(Context context){
        final Resources resources = context.getResources();

        NotificationChannel notificationChannel = new NotificationChannel(
                SIM_SELECT_NOTIFICATION_CHANNEL,
                resources.getString(R.string.sim_selection_channel_title),
                NotificationManager.IMPORTANCE_LOW);

        NotificationCompat.Builder builder =
                new NotificationCompat.Builder(context, SIM_SELECT_NOTIFICATION_CHANNEL)
                .setSmallIcon(R.drawable.ic_sim_card_alert_white_48dp)
                .setColor(context.getColor(R.color.sim_noitification))
                .setContentTitle(resources.getText(R.string.sim_notification_title))
                .setContentText(resources.getText(R.string.sim_notification_summary))
                .setAutoCancel(true);
        Intent resultIntent = new Intent(Settings.ACTION_WIRELESS_SETTINGS);
        resultIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        PendingIntent resultPendingIntent = PendingIntent.getActivity(context, 0, resultIntent,
                PendingIntent.FLAG_CANCEL_CURRENT);
        builder.setContentIntent(resultPendingIntent);
        NotificationManager notificationManager =
                (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.createNotificationChannel(notificationChannel);
        notificationManager.notify(NOTIFICATION_ID, builder.build());
    }

    public static void cancelNotification(Context context) {
        NotificationManager notificationManager =
                (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.cancel(NOTIFICATION_ID);
    }

    /// M: Add for SIM Lock feature. @{
    private boolean isDialogAllowedForSimLock(int policy, int numSlot,
            boolean[] simInserted, int[] simValid) {
        boolean ret = true;
        int caseId = TelephonyUtils.SIM_LOCK_CASE_NONE;

        switch (policy) {
            case TelephonyUtils.SIM_LOCK_POLICY_UNKNOWN:
                ret = false;
                break;

            case TelephonyUtils.SIM_LOCK_POLICY_SLOT1_ONLY:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT2_ONLY:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_CS:
                caseId = TelephonyUtils.getSimLockCase(numSlot, simInserted, simValid);
                if (caseId != TelephonyUtils.SIM_LOCK_CASE_ALL_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_INVALID_N_VALID) {
                    ret = false;
                }
                break;

            case TelephonyUtils.SIM_LOCK_POLICY_SLOT1_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT2_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED:
            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_WITH_ECC:
                caseId = TelephonyUtils.getSimLockCase(numSlot, simInserted, simValid);
                if (caseId != TelephonyUtils.SIM_LOCK_CASE_ALL_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_INVALID_1_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_INVALID_N_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_UNKNOWN_1_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_UNKNOWN_N_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_UNKNOWN_INVALID_1_VALID
                        && caseId != TelephonyUtils.SIM_LOCK_CASE_UNKNOWN_INVALID_N_VALID) {
                    ret = false;
                }
                break;

            case TelephonyUtils.SIM_LOCK_POLICY_SLOT_ALL_LINKED_REVERSE:
                caseId = TelephonyUtils.getSimLockCase(numSlot, simInserted, simValid);
                if (caseId != TelephonyUtils.SIM_LOCK_CASE_ALL_VALID) {
                    ret = false;
                }
                break;

            default:
                break;
        }

        Log.d(TAG, "isDialogAllowedForSimLock, ret=" + ret
                + ", policy=" + TelephonyUtils.getSimLockPolicyString(policy)
                + ", case=" + TelephonyUtils.getSimLockCaseString(caseId));
        return ret;
    }
    /// @}

}
