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

package com.mediatek.settings.network;

import android.content.Context;
import android.content.Intent;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;

import androidx.preference.Preference;

import com.android.settings.SettingsActivity;
import com.android.settings.network.telephony.MobileNetworkUtils;
import com.android.settings.network.telephony.TelephonyBasePreferenceController;

import com.mediatek.settings.FeatureOption;
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.sim.TelephonyUtils;

/**
 * Preference controller for "Open PLMN list"
 */
public class PlmnPreferenceController extends TelephonyBasePreferenceController {

    private static final String TAG = "PlmnPreferenceController";

    private static final String PLMN_ACTION = "mediatek.settings.PLMN_SETTINGS";

    public PlmnPreferenceController(Context context, String key) {
        super(context, key);
        mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        boolean customerATTvisible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                .customizeATTNetworkModePreference(subId);
        if (customerATTvisible) {
            return customerATTvisible ? CONDITIONALLY_UNAVAILABLE : AVAILABLE;
        }
        final boolean ctaSet = FeatureOption.MTK_CTA_SET;
        final boolean isGsmOpt = MobileNetworkUtils.isGsmOptions(mContext, subId);
        final boolean ctVolte = TelephonyUtils.isCtVolteEnabled()
                && MobileNetworkUtils.isCt4gSim(mPhoneId);
        final boolean enabled = ctaSet && isGsmOpt && !ctVolte;

        Log.i(TAG, "getAvailabilityStatus, subId=" + subId + ", enabled=" + enabled
                + ", ctaSet=" + ctaSet + ", isGsmOpt=" + isGsmOpt
                + ", ctVolte=" + ctVolte);

        return enabled ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        preference.setEnabled(
                MobileNetworkUtils.shouldEnableMobileNetworkOption(mPhoneId));
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (TextUtils.equals(preference.getKey(), getPreferenceKey())) {
            // This activity runs in phone process, we must use intent to start
            final Intent intent = new Intent(PLMN_ACTION);
            // This will setup the Home and Search affordance
            intent.putExtra(SettingsActivity.EXTRA_SHOW_FRAGMENT_AS_SUBSETTING, true);
            intent.putExtra(PlmnSettings.SUB_ID, mSubId);
            mContext.startActivity(intent);
            return true;
        }

        return false;
    }

    public void init(int subId) {
        mSubId = subId;
        mPhoneId = SubscriptionManager.getPhoneId(subId);
    }
}
