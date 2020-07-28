/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 */

package com.mediatek.settings.cdma;

import android.content.Context;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.telephony.MtkTelephonyManagerEx;

public class CdmaUtils {

    private static final String TAG = "CdmaUtils";

    private static final String CDMA_SIM_DIALOG = "com.mediatek.settings.cdma.SIM_DIALOG";
    public static final int CT_SIM = MtkTelephonyManagerEx.APP_FAM_3GPP2;
    public static final int GSM_SIM = MtkTelephonyManagerEx.APP_FAM_3GPP;
    public static final int C_G_SIM = CT_SIM | GSM_SIM;
    public static final int SIM_TYPE_NONE = MtkTelephonyManagerEx.APP_FAM_NONE;

    /**
     * check whether the card inserted is a CDMA card and
     * working in CDMA mode (the modem support CDMA).
     * Also see {@link #isCdmaCard(int)}
     * @param subId sub Id
     * @return
     */
    public static boolean isSupportCdma(int subId) {
        boolean isSupportCdma = false;
        if (TelephonyManager.getDefault().getCurrentPhoneType(subId)
                == TelephonyManager.PHONE_TYPE_CDMA) {
            isSupportCdma = true;
        }
        Log.d(TAG, "isSupportCdma=" + isSupportCdma + ", subId=" + subId);
        return isSupportCdma;
    }

    /**
     * check whether the card inserted is really a CDMA card.
     * NOTICE that it will return true even the card is a CDMA card but working as a GSM SIM.
     * Also see {@link #isSupportCdma(int)}
     * @param slotId slot Id
     * @return
     */
    public static boolean isCdmaCard(int slotId) {
        int simType = getSimType(slotId);
        boolean isCdma = (simType == CT_SIM || simType == C_G_SIM);
        Log.d(TAG, "isCdmaCard, simType=" + simType + ", isCdma=" + isCdma);
        return isCdma;
    }

    /**
     * Get sim type.
     * @param slotId sim phone id.
     * @return sim type.
     */

    public static int getSimType(int slotId) {
        int simType = SIM_TYPE_NONE;
        MtkTelephonyManagerEx telephonyManagerEx = MtkTelephonyManagerEx.getDefault();
        if (telephonyManagerEx != null) {
            simType = telephonyManagerEx.getIccAppFamily(slotId);
        }
        Log.d(TAG, "simType=" + simType + ", slotId=" + slotId);
        return simType;
    }

    /**
     * For C2K C+C case, only one SIM card register network, other card can recognition.
     * and can not register the network
     * 1. two CDMA cards.
     * 2. two cards is competitive. only one modem can register CDMA network.
     * @param context context
     * @return true
     */
    public static boolean isCdmaCardCompetion(Context context) {
        boolean isCdma = true;
        boolean isCompetition = true;
        int simCount = 0;
        if (context != null) {
            simCount = TelephonyManager.from(context).getSimCount();
        }
        if (simCount == 2) {
            for (int i = 0; i < simCount ; i++) {
                isCdma = isCdma && isCdmaCard(i);
                SubscriptionInfo subscriptionInfo =
                        SubscriptionManager.from(context).
                        getActiveSubscriptionInfoForSimSlotIndex(i);
                if (subscriptionInfo != null) {
                    isCompetition = isCompetition &&
                            MtkTelephonyManagerEx.getDefault().isInHomeNetwork(
                                    subscriptionInfo.getSubscriptionId());
                } else {
                    isCompetition = false;
                    break;
                }
            }
        } else {
            isCdma = false;
            isCompetition = false;
        }
        Log.d(TAG, "isCdma=" + isCdma + ", isCompletition=" + isCompetition);
        return isCdma && isCompetition;
    }

    /**
     * 1. two CDMA cards.
     * 2. two cards is competitive. only one modem can register CDMA network.
     * @param context Context
     * @return true
     */
    public static boolean isCdmaCardCompetionForData(Context context) {
        return isCdmaCardCompetion(context);
    }
}
