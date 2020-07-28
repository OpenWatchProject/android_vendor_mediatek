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
package com.mediatek.settings;

import android.content.Context;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.phone.PhoneGlobals;
import com.android.phone.PhoneUtils;
import com.android.phone.settings.SuppServicesUiUtil;

import com.mediatek.settings.cdma.TelephonyUtilsEx;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.List;

/**
 * Telephony utils class.
 */
public class TelephonyUtils {
    private static final String TAG = "TelephonyUtils";
    public static final String USIM = "USIM";

    private final static String ONE = "1";
    public static final String ACTION_NETWORK_CHANGED =
            "com.mediatek.intent.action.ACTION_NETWORK_CHANGED";

    /**
     * Check if the subscription card is USIM or SIM.
     * @param context using for query phone
     * @param subId according to the phone
     * @return true if is USIM card
     */
    public static boolean isUSIMCard(Context context, int subId) {
        log("isUSIMCard()... subId = " + subId);
        String type = MtkTelephonyManagerEx.getDefault().getIccCardType(subId);
        log("isUSIMCard()... type = " + type);
        return USIM.equals(type);
    }

    /**
     * Check if the sim state is ready or not.
     * @param slot sim slot
     * @return true if sim state is ready.
     */
    public static boolean isSimStateReady(int slot) {
        boolean isSimStateReady = false;
        isSimStateReady = TelephonyManager.SIM_STATE_READY == TelephonyManager.
                getDefault().getSimState(slot);
        log("isSimStateReady: "  + isSimStateReady);
        return isSimStateReady;
    }

    /**
     * check the radio is on or off by sub id.
     *
     * @param subId the sub id
     * @param context Context
     * @return true if radio on
     */
    public static boolean isRadioOn(int subId, Context context) {
        log("[isRadioOn]subId:" + subId);
        boolean isRadioOn = false;
        final ITelephony iTel = ITelephony.Stub.asInterface(
                ServiceManager.getService(Context.TELEPHONY_SERVICE));
        if (iTel != null && PhoneUtils.isValidSubId(subId)) {
            try {
                isRadioOn = iTel.isRadioOnForSubscriber(subId, context.getPackageName());
            } catch (RemoteException e) {
                log("[isRadioOn] failed to get radio state for sub " + subId);
                isRadioOn = false;
            }
        } else {
            log("[isRadioOn]failed to check radio");
        }
        log("[isRadioOn]isRadioOn:" + isRadioOn);

        return isRadioOn && !isAirplaneModeOn(PhoneGlobals.getInstance());
    }

    /**
     * Return whether the project is Gemini or not.
     * @return If Gemini, return true, else return false
     */
    public static boolean isGeminiProject() {
        boolean isGemini = TelephonyManager.getDefault().isMultiSimEnabled();
        log("isGeminiProject : " + isGemini);
        return isGemini;
    }

    /**
     * Add for [MTK_Enhanced4GLTE].
     * Get the phone is inCall or not.
     * @param context Context
     * @return true if in call
     */
    public static boolean isInCall(Context context) {
        TelecomManager manager = (TelecomManager) context.getSystemService(
                Context.TELECOM_SERVICE);
        boolean inCall = false;
        if (manager != null) {
            inCall = manager.isInCall();
        }
        log("[isInCall] = " + inCall);
        return inCall;
    }

    /**
     * Add for [MTK_Enhanced4GLTE].
     * Get the phone is inCall or not.
     * @param context Context
     * @return true if airplane mode on
     */
    public static boolean isAirplaneModeOn(Context context) {
        return Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }

    ///M: Add for [VoLTE_SS] @{
    /**
     * Get whether the IMS is IN_SERVICE.
     * @param context Context
     * @param subId the sub which one user selected.
     * @return true if the ImsPhone is IN_SERVICE, else false.
     */
    public static boolean isImsServiceAvailable(Context context, int subId) {
        boolean available = false;
        if (PhoneUtils.isValidSubId(subId)) {
             available = MtkTelephonyManagerEx.getDefault().isImsRegistered(subId);
        }
        log("isImsServiceAvailable[" + subId + "], available = " + available);
        return available;
    }

    /**
     * Return whether the phone is hot swap or not.
     * @param originaList old subscription info list
     * @param currentList current subscription info list
     * @return If hot swap, return true, else return false
     */
    public static boolean isHotSwapHanppened(List<SubscriptionInfo> originaList,
            List<SubscriptionInfo> currentList) {
        boolean result = false;
        if (originaList.size() != currentList.size()) {
            return true;
        }
        for (int i = 0; i < currentList.size(); i++) {
            SubscriptionInfo currentSubInfo = currentList.get(i);
            SubscriptionInfo originalSubInfo = originaList.get(i);
            if (!(currentSubInfo.getIccId()).equals(originalSubInfo.getIccId())) {
                result = true;
                break;
            } else {
                result = false;
            }
        }

        log("isHotSwapHanppened : " + result);
        return result;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    /**
     * When SS from VoLTE we should make the Mobile Data Connection open, if don't open,
     * the query will fail, so we should give users a tip, tell them how to get SS successfully.
     * This function is get the point, whether we should show a tip to user. Conditions:
     * 1. VoLTE condition / CMCC support VoLTE card, no mater IMS enable/not
     * 2. Mobile Data Connection is not enable
     * @param subId the given subId
     * @return true if should show tip, else false.
     */
    public static boolean shouldShowOpenMobileDataDialog(Context context, int subId) {
        boolean result = false;
        if (!PhoneUtils.isValidSubId(subId)) {
            log("[shouldShowOpenMobileDataDialog] invalid subId!!!  " + subId);
            return result;
        }

        PersistableBundle carrierConfig =
                   PhoneGlobals.getInstance().getCarrierConfigForSubId(subId);
        if (carrierConfig.getBoolean(
                CarrierConfigManager.KEY_CALL_FORWARDING_OVER_UT_WARNING_BOOL) ||
                CallSettingUtils.isCtVolte4gSim(subId)) {
            Phone phone = PhoneUtils.getPhoneUsingSubId(subId);
            // Data should open or not for singe volte.
            if (SuppServicesUiUtil.isSsOverUtPrecautions(context, phone)) {
                result = true;
            // Add for dual volte feature, data traffic dialog.
            } else if (SuppServicesUiUtil.isSsOverUtPrecautionsExt(context, phone)) {
                result = true;
            }
        }
        log("[shouldShowOpenMobileDataDialog] subId: " + subId + ",result: " + result);
        return result;
    }
}
