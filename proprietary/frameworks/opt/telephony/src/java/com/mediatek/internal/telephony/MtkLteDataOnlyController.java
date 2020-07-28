/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.internal.telephony;

import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.internal.telephony.MtkRILConstants;
/**
 * For check Lte data only mode that whether show dialog prompt.
 */
public class MtkLteDataOnlyController {
    private static final String TAG = "MtkLteDataOnlyController";
    private static final String CHECK_PERMISSION_SERVICE_PACKAGE =
            "com.android.phone";
    private static final String ACTION_CHECK_PERMISSISON_SERVICE =
            "com.mediatek.intent.action.LTE_DATA_ONLY_MANAGER";
    private static final String[] PROPERTY_RIL_FULL_UICC_TYPE = {
            "vendor.gsm.ril.fulluicctype",
            "vendor.gsm.ril.fulluicctype.2",
            "vendor.gsm.ril.fulluicctype.3",
            "vendor.gsm.ril.fulluicctype.4",
    };
    private Context mContext;

    private static final String CSIM = "CSIM";
    private static final String RUIM = "RUIM";
    private static final String USIM = "USIM";
    private static final String SIM = "SIM";

    public static final int CDMA_SIM = 5;
    public static final int CDMA4G_SIM = 4;
    public static final int CDMA3G_SIM = 3;
    public static final int GSM_SIM = 2;
    public static final int ERROR_SIM = -1;

    /**
     * For C2K SVLTE RAT controll, LTE preferred mode.
     */
    public static final int SVLTE_RAT_MODE_4G = 0;
    /**
     * For C2K SVLTE RAT controll, EVDO preferred mode, will disable LTE.
     */
    public static final int SVLTE_RAT_MODE_3G = 1;
    /**
     * For C2K SVLTE RAT controll, LTE Data only mode, will disable CDMA and only allow LTE PS.
     */
    public static final int SVLTE_RAT_MODE_4G_DATA_ONLY = 2;

    /**
     * Constructor method for MtkLteDataOnlyController.
     * @param context For start service.
     */
    public MtkLteDataOnlyController(Context context) {
        mContext = context;
    }

    /**
     * Check whether support enable TDD Data Only and currently TDD Data Only be enabled .
     * @return true when support enable TDD Data Only and currently TDD Data Only be enabled.
     */
    public boolean checkPermission() {
        if (isSupportTddDataOnlyCheck() && is4GDataOnly()) {
            startService();
            return false;
        }
        return true;
    }

    /**
     * Check whether the SIM card is CDMA slot and support tdd data only function .
     * @param subId The SIM card subscription Id.
     * @return true when the SIM card is CDMA slot and support tdd data only function.
     */
    public boolean checkPermission(int subId) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        int cdmaSlotId = SystemProperties.getInt("persist.vendor.radio.cdma_slot", -1) - 1;
        Rlog.d(TAG, "checkPermission subId=" + subId + ", slotId=" + slotId
                + " cdmaSlotId=" + cdmaSlotId);
        if (cdmaSlotId == slotId) {
            return checkPermission();
        } else {
            return true;
        }
    }

    private void startService() {
        int subId[] = SubscriptionManager
                .getSubId(getCdmaSlot());
        Intent serviceIntent = new Intent(ACTION_CHECK_PERMISSISON_SERVICE);
        serviceIntent.setPackage(CHECK_PERMISSION_SERVICE_PACKAGE);
        if (subId != null) {
            serviceIntent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId[0]);
        }
        if (mContext != null) {
            mContext.startService(serviceIntent);
        }
    }

    private boolean is4GDataOnly() {
        if (mContext == null) {
            return false;
        }
        int cdmaSlotId = SystemProperties.getInt("persist.vendor.radio.cdma_slot", -1) - 1;
        int subId[] = SubscriptionManager
                .getSubId(cdmaSlotId);
        if (subId != null) {
            int patternLteDataOnly = Settings.Global.getInt(
                    mContext.getContentResolver(),
                    android.provider.Settings.Global.PREFERRED_NETWORK_MODE + subId[0],
                    MtkRILConstants.PREFERRED_NETWORK_MODE);
            return (patternLteDataOnly == MtkRILConstants.NETWORK_MODE_LTE_TDD_ONLY);
        }
        return false;
    }

    /**
     * Get getFullIccCardTypeExt type .
     * @return sim string type
     */
    public static String getFullIccCardTypeExt() {
        int slotId = getCdmaSlot();
        if (slotId < 0 || slotId >= PROPERTY_RIL_FULL_UICC_TYPE.length) {
            slotId = 0;
        }
        final String cardType = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[slotId]);
        Rlog.d(TAG, "getFullIccCardTypeExt slotId = " + slotId + ",cardType = " + cardType);
        return cardType;
    }

    /**
     * Get sim type .
     * @return sim type
     */
    public static int getSimType() {
        String fullUiccType = getFullIccCardTypeExt();
        if (fullUiccType != null) {
            if (fullUiccType.contains(CSIM) || fullUiccType.contains(RUIM)) {
                if (fullUiccType.contains(CSIM)
                        || fullUiccType.contains(USIM)) {
                    return CDMA4G_SIM;
                } else if (fullUiccType.contains(SIM)) {
                    return CDMA3G_SIM;
                }
                return CDMA_SIM;
            } else if (fullUiccType.contains(SIM)
                    || fullUiccType.contains(USIM)) {
                return GSM_SIM;
            } else {
                return ERROR_SIM;
            }
        }
        return ERROR_SIM;
    }

    /**
     * Get the sim card is CDMA card or not.
     * @return boolean is CDMA card or not.
     */
    public static boolean isCdmaCardType() {
        return (getSimType() == CDMA4G_SIM) || (getSimType() == CDMA3G_SIM)
                || (getSimType() == CDMA_SIM);
    }

    /**
     * Get the sim card is CDMA 4g card or not.
     * @return boolean is CDMA 4g card or not.
     */
    public static boolean isCdmaLteCardType() {
        return getSimType() == CDMA4G_SIM;
    }

    /**
     * Get the SIM card is CDMA card or not.
     * @return boolean is CDMA card or not.
     */
    public static boolean isCdma3GCardType() {
        return getSimType() == CDMA3G_SIM;
    }

    private static int getCdmaSlot() {
        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            int phoneType = TelephonyManager.getDefault().getCurrentPhoneTypeForSlot(i);
            if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                return i;
            }
        }
        return -1;
    }

    private boolean isSupportTddDataOnlyCheck() {
        boolean isCdma4gCard = isCdmaLteCardType();
        boolean isCdmaLteDcSupport = SystemProperties.get(
                "ro.vendor.mtk_c2k_lte_mode").equals("1");
        boolean isSupport4gDataOnly = ("1").equals(SystemProperties.get(
                    "ro.vendor.mtk_tdd_data_only_support"));
        boolean checkResult = false;
        if (isCdma4gCard && isCdmaLteDcSupport && isSupport4gDataOnly) {
            checkResult = true;
        }
        Rlog.d(TAG, "isCdma4gCard : " + isCdma4gCard
            + ", isCdmaLteDcSupport : " + isCdmaLteDcSupport
            + ", isSupport4gDataOnly : " + isSupport4gDataOnly
            + ", isSupportTddDataOnlyCheck return " + checkResult);
        return checkResult;
    }
}