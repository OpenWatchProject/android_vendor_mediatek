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
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import androidx.preference.Preference;
import androidx.preference.SwitchPreference;

import com.android.settings.network.telephony.MobileNetworkUtils;
import com.android.settings.network.telephony.TelephonyTogglePreferenceController;

import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;
import com.mediatek.settings.sim.TelephonyUtils;

/**
 * Preference controller for "Enable 4G Data"
 */
public class Enable4gDataPreferenceController extends TelephonyTogglePreferenceController {

    private static final String TAG = "Enable4gDataPreferenceController";

    private int mPhoneId;
    private TelephonyManager mTelephonyManager;

    private SwitchPreference mPreference;

    public Enable4gDataPreferenceController(Context context, String key) {
        super(context, key);
        mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        mPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        boolean lteSupported = RatConfiguration.isLteFddSupported()
                || RatConfiguration.isLteTddSupported();
        boolean c2kSupported = RatConfiguration.isC2kSupported();
        if (!lteSupported || !c2kSupported) {
            return CONDITIONALLY_UNAVAILABLE;
        }

        boolean is93Md = TelephonyUtils.is93OrLaterModem();
        if (is93Md) {
            return CONDITIONALLY_UNAVAILABLE;
        }

        boolean isCtSim = MobileNetworkUtils.isCtSim(mPhoneId);
        if (!isCtSim) {
            return CONDITIONALLY_UNAVAILABLE;
        }

        return AVAILABLE;
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        mPreference = (SwitchPreference) preference;

        int mainPhoneId = TelephonyUtils.getMainCapabilityPhoneId();
        boolean ps2SupportLte = RadioCapabilitySwitchUtil.isPS2SupportLTE();
        boolean enabled = MobileNetworkUtils.isCt4gSim(mPhoneId)
                && (mPhoneId == mainPhoneId || ps2SupportLte);

        Log.d(TAG, "updateState, enabled=" + enabled + ", subId=" + mSubId
                + ", phoneId=" + mPhoneId + ", mainPhoneId=" + mainPhoneId
                + ", ps2SupportLte=" + ps2SupportLte);

        mPreference.setEnabled(enabled
                && MobileNetworkUtils.shouldEnableMobileNetworkOption(mPhoneId));
        mPreference.setChecked(TelephonyUtils.isLteNetworkTypeEnabled(mContext, mSubId));
    }

    @Override
    public boolean setChecked(boolean isChecked) {
        Log.i(TAG, "setChecked, isChecked=" + isChecked);
        mPreference.setEnabled(false);
        int settingsMode = isChecked ? TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA
                : TelephonyManager.NETWORK_MODE_GLOBAL;
        AsyncTask.execute(
                new PreferredNetworkTypeSyncTask(mTelephonyManager,
                        mSubId, settingsMode));
        return true;
    }

    @Override
    public boolean isChecked() {
        return TelephonyUtils.isLteNetworkTypeEnabled(mContext, mSubId);
    }

    public void init(int subId) {
        mSubId = subId;
        mPhoneId = SubscriptionManager.getPhoneId(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
    }

    private static final int EVENT_SET_PREFERRED_NETWORK_TYPE_DONE = 1;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SET_PREFERRED_NETWORK_TYPE_DONE:
                    Log.e(TAG, "handleMessage, set preferred network type done.");
                    if (mPreference != null) {
                        updateState(mPreference);
                    }
                    break;

                default:
                    break;
            }
        }
    };

    private final class PreferredNetworkTypeSyncTask implements Runnable {
        private TelephonyManager mTelephonyManager;
        private int mSubId;
        private int mNetworkType;

        PreferredNetworkTypeSyncTask(TelephonyManager telephonyManager,
            int subId, int networkType) {
            mTelephonyManager = telephonyManager;
            mSubId = subId;
            mNetworkType = networkType;
        }

        @Override
        public void run() {
            if (mTelephonyManager.setPreferredNetworkType(mSubId, mNetworkType)) {
                Settings.Global.putInt(mContext.getContentResolver(),
                        Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                        mNetworkType);
                Log.e(TAG, "PreferredNetworkTypeSyncTask.run, set preferred network type success"
                        + ", subId=" + mSubId + ", networkType=" + mNetworkType);
            } else {
                Log.e(TAG, "PreferredNetworkTypeSyncTask.run, set preferred network type failed"
                        + ", subId=" + mSubId + ", networkType=" + mNetworkType);
            }
            Message msg = mHandler.obtainMessage(EVENT_SET_PREFERRED_NETWORK_TYPE_DONE);
            msg.sendToTarget();
        }
    }
}
