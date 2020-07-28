/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.internal.telephony;

import static android.telephony.TelephonyManager.EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE_DATA;
import static android.telephony.TelephonyManager.EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE_NONE;

import android.content.Context;
import android.os.SystemProperties;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import android.util.Log;

import com.android.internal.telephony.MultiSimSettingController;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.util.ArrayUtils;

import com.mediatek.internal.telephony.datasub.DataSubSelector;
import com.mediatek.internal.telephony.datasub.DataSubSelectorUtil;
import com.mediatek.internal.telephony.datasub.IDataSubSelectorOPExt;

import java.util.List;

/**
 * This class will make sure below setting rules are coordinated across different subscriptions
 * and phones in multi-SIM case:
 *
 * 1) Grouped subscriptions will have same settings for MOBILE_DATA and DATA_ROAMING.
 * 2) Default settings updated automatically. It may be cleared or inherited within group.
 *    If default subscription A switches to profile B which is in the same group, B will
 *    become the new default.
 * 3) For primary subscriptions, only default data subscription will have MOBILE_DATA on.
 *
 * M: We change aosp behavior as:
 * 1) we don't change the data switch on sim as default data sim.
 * 2) we don't change none default sim data switch as off, so default data switch value is on.
 */
public class MtkMultiSimSettingController extends MultiSimSettingController {
    private static final String LOG_TAG = "MtkMultiSimSettingController";
    private static final boolean DBG = true;
    /// M: the length of "N/A" for no sim string in iccid array
    private static final int NO_SIM_STRING_LENGTH = 3;
    public MtkMultiSimSettingController(Context context, SubscriptionController sc) {
        super(context, sc);
    }

    /**
     * Make sure MOBILE_DATA of subscriptions in same group are synced.
     *
     * If user is enabling a non-default non-opportunistic subscription, make it default
     * data subscription.
     */
    @Override
    protected void onUserDataEnabled(int subId, boolean enable) {
        if (DBG) log("onUserDataEnabled");
        // Make sure MOBILE_DATA of subscriptions in same group are synced.
        setUserDataEnabledForGroup(subId, enable);

        // If user is enabling a non-default non-opportunistic subscription, make it default.
        /// M: we don't want this behavior
        /*
        if (mSubController.getDefaultDataSubId() != subId && !mSubController.isOpportunistic(subId)
                && enable) {
            mSubController.setDefaultDataSubId(subId);
        }
        */
    }

    /**
     * Automatically update default settings (data / voice / sms).
     *
     * Opportunistic subscriptions can't be default data / voice / sms subscription.
     *
     * 1) If the default subscription is still active, keep it unchanged.
     * 2) Or if there's another active primary subscription that's in the same group,
     *    make it the new default value.
     * 3) Or if there's only one active primary subscription, automatically set default
     *    data subscription on it. Because default data in Android Q is an internal value,
     *    not a user settable value anymore.
     * 4) If non above is met, clear the default value to INVALID.
     *
     * @param init whether the subscriptions are just initialized.
     */
    @Override
    protected void updateDefaults(boolean init) {
        if (DBG) log("updateDefaults");

        if (!mSubInfoInitialized) return;

        List<SubscriptionInfo> activeSubInfos = mSubController
                .getActiveSubscriptionInfoList(mContext.getOpPackageName());

        IDataSubSelectorOPExt opExt = DataSubSelector.getDataSubSelectorOpExt();
        /// M: currently OP01/OP02 add-on will skip this, they have own logic.
        boolean followAospData = (opExt == null || opExt.enableAospDefaultDataUpdate());
        if (DBG) log("updateDefaults, followAospData:" + followAospData);

        if (ArrayUtils.isEmpty(activeSubInfos)) {
            mPrimarySubList.clear();
            if (DBG) log("[updateDefaultValues] No active sub. Setting default to INVALID sub.");
            if (followAospData) {
                /// M: use mtk method to avoid sim switch, other module will do it auto.
                ((MtkSubscriptionController)mSubController)
                        .setDefaultDataSubIdWithoutCapabilitySwitch(
                                SubscriptionManager.INVALID_SUBSCRIPTION_ID);
            }
            mSubController.setDefaultVoiceSubId(SubscriptionManager.INVALID_SUBSCRIPTION_ID);
            mSubController.setDefaultSmsSubId(SubscriptionManager.INVALID_SUBSCRIPTION_ID);
            return;
        }

        int change = updatePrimarySubListAndGetChangeType(activeSubInfos, init);
        if (DBG) log("[updateDefaultValues] change: " + change);
        if (change == PRIMARY_SUB_NO_CHANGE) return;
        /// M: check sim status by iccid, sub can't detected all sims on time, it maybe detected
        // one sim a time, this is not friendly for common slot devices.
        if (change == PRIMARY_SUB_ADDED && isAnySimNotReady(mPrimarySubList.size())) {
            return;
        }

        // If there's only one primary subscription active, we trigger PREFERRED_PICK_DIALOG
        // dialog if and only if there were multiple primary SIM cards and one is removed.
        // Otherwise, if user just inserted their first SIM, or there's one primary and one
        // opportunistic subscription active (activeSubInfos.size() > 1), we automatically
        // set the primary to be default SIM and return.
        if (mPrimarySubList.size() == 1 && change != PRIMARY_SUB_REMOVED) {
            int subId = mPrimarySubList.get(0);
            if (DBG) log("[updateDefaultValues] to only primary sub " + subId);
            if (followAospData) {
                /// M: use mtk method to avoid sim switch, other module will do it auto.
                ((MtkSubscriptionController)mSubController)
                        .setDefaultDataSubIdWithoutCapabilitySwitch(subId);
            }
            mSubController.setDefaultVoiceSubId(subId);
            mSubController.setDefaultSmsSubId(subId);
            return;
        }

        if (DBG) log("[updateDefaultValues] records: " + mPrimarySubList);

        // Update default data subscription.
        if (DBG) log("[updateDefaultValues] Update default data subscription");
        boolean dataSelected = false;
        if (followAospData) {
            dataSelected= updateDefaultValue(mPrimarySubList,
                    mSubController.getDefaultDataSubId(),
                     /// M: use mtk method to avoid sim switch, other module will do it auto.
                    (newValue -> ((MtkSubscriptionController)mSubController)
                            .setDefaultDataSubIdWithoutCapabilitySwitch(newValue)));
        }

        // Update default voice subscription.
        if (DBG) log("[updateDefaultValues] Update default voice subscription");
        boolean voiceSelected = updateDefaultValue(mPrimarySubList,
                mSubController.getDefaultVoiceSubId(),
                (newValue -> mSubController.setDefaultVoiceSubId(newValue)));

        // Update default sms subscription.
        if (DBG) log("[updateDefaultValues] Update default sms subscription");
        boolean smsSelected = updateDefaultValue(mPrimarySubList,
                mSubController.getDefaultSmsSubId(),
                (newValue -> mSubController.setDefaultSmsSubId(newValue)));

        sendSubChangeNotificationIfNeeded(change, dataSelected, voiceSelected, smsSelected);
    }

    @Override
    protected int getSimSelectDialogType(int change, boolean dataSelected,
            boolean voiceSelected, boolean smsSelected) {
        int dialogType =
                super.getSimSelectDialogType(change, dataSelected, voiceSelected, smsSelected);
        if (dialogType == EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE_NONE) {
            // M: add an extra case to show select data dialog, boot up with 2 sims,
            // and there is no default data selected, should show the dialog.
            if (mPrimarySubList.size() > 1 && change == PRIMARY_SUB_INITIALIZED && !dataSelected) {
                dialogType = EXTRA_DEFAULT_SUBSCRIPTION_SELECT_TYPE_DATA;
            }
        }
        return dialogType;
    }

    @Override
    protected void disableDataForNonDefaultNonOpportunisticSubscriptions() {
        /// M: we don't want this behavior because of: @{
        //  1. Fix google temp data switch issue (Google temp data switch feature can't work due to
        //     non-DDS phone can't turn on data)
        //  2. Support dual ViLTE on L+L (ViLTE function can only be used when data switch in ON)
        //  3. Support on-demand request (ex: MMS) on non-DDS SIM
        //  4. Keep consistent UX behavior between P and Q
        //  5. Behavior still not clear on next android version R (Google is discussing this
        //     behavior and may change on android R)
        //  @}
        // super.disableDataForNonDefaultNonOpportunisticSubscriptions();
    }

    private boolean isAnySimNotReady(int subCount) {
        int simCount = 0;
        int maxCount = TelephonyManager.getDefault().getSimCount();
        for (int i = 0; i < maxCount; i++) {
            String iccid = SystemProperties.get(DataSubSelectorUtil.PROPERTY_ICCID[i], "");
            // if it is ready, it should be "N/A" means no sim, or iccid string.
            if (iccid.equals("")) {
                log("Not ready sim exist:" + i + ", max:" + maxCount);
                return true;
            } else if (iccid.length() > NO_SIM_STRING_LENGTH) {
                simCount++;
            }
        }
        log("simCount:" + simCount);
        return (subCount != simCount) ? true : false;
    }

    private void log(String msg) {
        Log.d(LOG_TAG, msg);
    }
}
