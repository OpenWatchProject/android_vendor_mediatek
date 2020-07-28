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

package com.mediatek.settings.op01;


import java.util.List;

import android.content.Context;
import android.content.IntentFilter;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import com.mediatek.settings.ext.DefaultMobileNetworkSettingsExt;

/**
 * Other settings info plugin.
 */
public class Op01MobileNetworkSettingExt extends DefaultMobileNetworkSettingsExt {


    private static final String TAG = "Op01MobileNetworkSettingExt";
    private Context mContext;
    private static final String[] MCCMNC_TABLE_TYPE_CMCC = {
        "46000", "46002", "46004", "46007", "46008"};
    private static final String MULTI_IMS_SUPPORT = "persist.vendor.mims_support";
    private static final String PACKAGE_NAME = "com.mediatek.settings.op01";

    public Op01MobileNetworkSettingExt(Context context) {
        mContext = context;
    }

    @Override
    public boolean customizeAutoSelectNetworkTypePreference() {
        Log.i(TAG, "customizeAutoSelectNetworkTypePreference return true");
        return true;
    }

    @Override
    public boolean customizeVolteState(int subId) {
        int phoneId = SubscriptionManager.getPhoneId(subId);
        Log.i(TAG, "customizeVolteState subid = " + subId
                + " phoneId = " + phoneId);
        if (isCMCCCard(subId) || isTestSim(mContext, phoneId)) {
            Log.i(TAG, "customizeVolteState return false, is not cmcc card");
            return false;
        }
        return true;
    }

    @Override
    public boolean customizeVolteStateReceiveIntent() {
        Log.i(TAG, "customizeVolteStateReceiveIntent return true");
        return true;
    }
    /**
     * app use to judge the Card is CMCC
     * @param slotId
     * @return true is CMCC
     */
    private boolean isCMCCCard(int subId) {
        Log.d(TAG, "isCMCCCard, subId = " + subId);
        String simOperator = null;
        simOperator = getSimOperatorBySubId(subId);
        if (simOperator != null) {
            Log.d(TAG, "isCMCCCard, simOperator =" + simOperator);
            for (String mccmnc : MCCMNC_TABLE_TYPE_CMCC) {
                if (simOperator.equals(mccmnc)) {
                    return true;
                }
            }
        }
        Log.d(TAG, "isCMCCCard, false");
        return false;
    }

    private String getSimOperatorBySubId(int subId) {
        if (subId < 0) {
            return null;
        }
        String simOperator = null;
        int status = TelephonyManager.SIM_STATE_UNKNOWN;
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
             status = TelephonyManager.getDefault().getSimState(slotId);
        }
        if (status == TelephonyManager.SIM_STATE_READY) {
            simOperator = TelephonyManager.getDefault().getSimOperator(subId);
        }
        Log.d(TAG, "getSimOperatorBySubId, simOperator = "
                + simOperator + " subId = " + subId);
        return simOperator;
    }
    /// M: VILTE enable not dependent on data enable @{
    private static boolean isTestSim(Context context, int phoneId) {
        if (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) == 1) {
            phoneId = getMainCapabilityPhoneId(context);
        }
        boolean isTestSim = false;
        switch (phoneId) {
            case PhoneConstants.SIM_ID_1:
                isTestSim = "1".equals(SystemProperties.get("vendor.gsm.sim.ril.testsim", "0"));
                break;
            case PhoneConstants.SIM_ID_2:
                isTestSim = "1".equals(SystemProperties.get("vendor.gsm.sim.ril.testsim.2", "0"));
                break;
            case PhoneConstants.SIM_ID_3:
                isTestSim = "1".equals(SystemProperties.get("vendor.gsm.sim.ril.testsim.3", "0"));
                break;
            case PhoneConstants.SIM_ID_4:
                isTestSim = "1".equals(SystemProperties.get("vendor.gsm.sim.ril.testsim.4", "0"));
                break;
        }
        return isTestSim;
    }
    /// @}

    /**
     * Get the phone id with main capability.
     */
    private static int getMainCapabilityPhoneId(Context context) {
        int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        IMtkTelephonyEx telephony = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));
        if (telephony != null) {
            try {
                phoneId = telephony.getMainCapabilityPhoneId();
            } catch (RemoteException e) {
                Log.d(TAG, "getMainCapabilityPhoneId: remote exception");
            }
        } else {
            Log.d(TAG, "IMtkTelephonyEx service not ready!");

            phoneId = SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
            if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
                phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
            }
            Log.d(TAG, "getMainCapabilityPhoneId: phoneId = " + phoneId);

        }
        return phoneId;
    }
    /// @}

    /**
     * return true means mobile data can not show.
     * must cmcc + non-cmcc
     * or can not know sim card type.
     * or simoff.
     */
//    @Override
//    public boolean customizaMobileDataState(int subId) {
//        Log.i(TAG, "customizaMobileDataState subId = " + subId);
//        return isAllowDataEnable(subId);
//    }

    public boolean isAllowDataEnable(int subId) {
        Log.i(TAG, "isAllowDataEnable, cursubId = " + subId);
        if (subId < 0) {
            return true;
        }
        List<SubscriptionInfo> si = SubscriptionManager.from(mContext)
            .getActiveSubscriptionInfoList();
        if (si != null && si.size() > 1) {
            int otherId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
            for (int i = 0; i < si.size(); i++) {
                SubscriptionInfo subInfo = si.get(i);
                int curId = subInfo.getSubscriptionId();
                if (curId != subId) {
                    otherId = curId;
                    break;
                }
            }
            MtkTelephonyManagerEx telephonyManagerEx = MtkTelephonyManagerEx.getDefault();
            boolean isSimOnOffEnabled = telephonyManagerEx.isSimOnOffEnabled();
            if (isSimOnOffEnabled) {
                /// subid is sim off
                if (!isSimOn(subId)) {
                    Log.i(TAG, "disable mobile data because " +
                            "  sim off subid = " + subId);
                    return true;
                }
                /// self is sim on, other is sim off.
                if (isSimOn(subId) && !isSimOn(otherId)) {
                    Log.i(TAG, "do not disable mobile data because" +
                            " self subid sim on and other subid sim off" +
                            " subId = " + subId + " otherId = " + otherId);
                    return false;
                }
                if (isSimOn(subId) && isSimOn(otherId)) {
                    if (isCMCCCard(otherId) && !isCMCCCard(subId)) {
                        Log.i(TAG, "disable mobile data because "
                                + " self subid sim on and other subid sim on"
                                + " but self subid is not CMCC card, other is cmcc "
                                + " subId = " + subId + " otherId = " + otherId);
                        return true;
                    }
                    String plmn = getSimOperatorBySubId(otherId);
                    if ((plmn == null || plmn.equals("")) && !isCMCCCard(subId)) {
                        Log.i(TAG, "disable mobile data  ," +
                                " because self subid sim on and other subid sim on"
                                + " but self subid is not CMCC card, other card unknown "
                                + " subId = " + subId + " otherId = " + otherId);
                        return true;
                    }
                }
            } else {
                /// subid is radio off
                if (!isTargetSubRadioOn(subId)) {
                    Log.i(TAG, "isAllowDataEnable," +
                            " disable raido off subid = " + subId);
                    return true;
                }

                if (isCMCCCard(otherId) && !isCMCCCard(subId)) {
                    if (isTargetSubRadioOn(otherId)) {
                        Log.i(TAG, "isAllowDataEnable, close switch");
                        return true;
                    }
                }

                String plmn = getSimOperatorBySubId(otherId);
                if ((plmn == null || plmn.equals("")) && !isCMCCCard(subId)) {
                    Log.i(TAG, "isAllowDataEnable, disable because can not get simopeator");
                    return true;
                }
            }
        }
        return false;
    }

    private boolean isSimOn(int subId) {
        MtkTelephonyManagerEx telephonyManagerEx = MtkTelephonyManagerEx.getDefault();
        boolean isSimOnOffEnabled = telephonyManagerEx.isSimOnOffEnabled();
        int slotId = SubscriptionManager.getSlotIndex(subId);
        Log.i(TAG, "isTargetSubRadioOn isSimOnOffEnabled = " + isSimOnOffEnabled);
        boolean isSimOn = false;
        if (isSimOnOffEnabled) {
            isSimOn = telephonyManagerEx.getSimOnOffState(slotId)
                        == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON;
            Log.i(TAG, "isTargetSubRadioOn isSimOn = " + isSimOn
                    + " telephonyManagerEx.getSimOnOffState(slotId) = "
                    + telephonyManagerEx.getSimOnOffState(slotId));
            return isSimOn;
        } else {
            return isSimOn;
        }
    }

    public static boolean isTargetSubRadioOn(int subId) {
        boolean radioOn = true;
        try {
            ITelephony iTel = ITelephony.Stub.asInterface(
                ServiceManager.getService(Context.TELEPHONY_SERVICE));
            if (null == iTel) {
                Log.i(TAG, "isTargetSubRadioOn = false because iTel = null");
                return false;
            }
            radioOn = iTel.isRadioOnForSubscriber(subId, PACKAGE_NAME);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        }
        Log.i(TAG, "isTargetSubRadioOn radioOn = " + radioOn + " sub = " + subId);
        return radioOn;
    }

}
