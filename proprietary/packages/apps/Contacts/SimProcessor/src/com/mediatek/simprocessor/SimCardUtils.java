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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.simprocessor;

import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.simprocessor.Log;

import java.util.HashMap;
import java.util.List;


public class SimCardUtils {
    private static final String TAG = "SimCardUtils";

    public interface SimType {
        String SIM_TYPE_USIM_TAG = "USIM";
        String SIM_TYPE_SIM_TAG = "SIM";
        String SIM_TYPE_RUIM_TAG = "RUIM";
        String SIM_TYPE_CSIM_TAG = "CSIM";

        int SIM_TYPE_SIM = 0;
        int SIM_TYPE_USIM = 1;
        int SIM_TYPE_RUIM = 2;
        int SIM_TYPE_CSIM = 3;
        int SIM_TYPE_UNKNOWN = -1;
    }

    static final String[] UICCCARD_PROPERTY_TYPE = {
        "gsm.ril.uicctype",
        "gsm.ril.uicctype2",
        "gsm.ril.uicctype3",
        "gsm.ril.uicctype4",
    };

    /**
     * check PhoneBook State is ready if ready, then return true.
     *
     * @param subId
     * @return
     */
    public static boolean isPhoneBookReady(int subId) {
        final IMtkTelephonyEx telephonyEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        if (null == telephonyEx) {
            Log.w(TAG, "[isPhoneBookReady]phoneEx == null");
            return false;
        }
        boolean isPbReady = false;
        try {
            isPbReady = telephonyEx.isPhbReady(subId);
        } catch (RemoteException e) {
            Log.e(TAG, "[isPhoneBookReady]catch exception:");
            e.printStackTrace();
        }
        Log.d(TAG, "[isPhoneBookReady]subId:" + subId + ", isPbReady:" + isPbReady);
        return isPbReady;
    }

    /**
     * [Gemini+] get sim type integer by subId sim type is integer defined in
     * SimCardUtils.SimType
     *
     * @param subId
     * @return SimCardUtils.SimType
     */
    public static int getSimTypeBySubId(int subId) {
        int simType = -1;
        final IMtkTelephonyEx iTel = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        if (iTel == null) {
            Log.w(TAG, "[getSimTypeBySubId]iTel == null");
            return simType;
        }
        try {
            String iccCardType = iTel.getIccCardType(subId);
            if (iccCardType == null || iccCardType.isEmpty()) {
                iccCardType = getSimTypeByProperty(subId);
            }
            if (SimType.SIM_TYPE_USIM_TAG.equals(iccCardType)) {
                simType = SimType.SIM_TYPE_USIM;
            } else if (SimType.SIM_TYPE_RUIM_TAG.equals(iccCardType)) {
                simType = SimType.SIM_TYPE_RUIM;
            } else if (SimType.SIM_TYPE_SIM_TAG.equals(iccCardType)) {
                simType = SimType.SIM_TYPE_SIM;
            } else if (SimType.SIM_TYPE_CSIM_TAG.equals(iccCardType)) {
                simType = SimType.SIM_TYPE_CSIM;
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[getSimTypeBySubId]catch exception:");
            e.printStackTrace();
        }

        return simType;
    }

    private static String getSimTypeByProperty(int subId) {
        int slotId = SubInfoUtils.getSlotIdUsingSubId(subId);
        String cardType = null;

        if (slotId >= 0 && slotId < 4) {
            cardType = SystemProperties.get(UICCCARD_PROPERTY_TYPE[slotId]);
        }
        Log.d(TAG, "[getSimTypeByProperty]slotId=" + slotId + ", cardType=" + cardType);
        return cardType;
    }

    public static String getIccCardType(int subId) {
        final IMtkTelephonyEx iTel = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        if (iTel == null) {
            Log.w(TAG, "[getIccCardType]iTel == null");
            return null;
        }
        String iccCardType = null;
        try {
            iccCardType = iTel.getIccCardType(subId);
        } catch (RemoteException e) {
            Log.e(TAG, "[getIccCardType]catch exception:");
            e.printStackTrace();
        }
        return iccCardType;
    }

    /**
     * [Gemini+] check whether a slot is insert a usim or csim card
     *
     * @param subId
     * @return true if it is usim or csim card
     */
    public static boolean isUsimOrCsimType(int subId) {
        boolean isUsimOrCsim = false;
        final IMtkTelephonyEx iTel = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        if (iTel == null) {
            Log.w(TAG, "[isUsimOrCsimType]iTel == null");
            return isUsimOrCsim;
        }
        try {
            if (SimType.SIM_TYPE_USIM_TAG.equals(iTel.getIccCardType(subId))
                    || SimType.SIM_TYPE_CSIM_TAG.equals(iTel.getIccCardType(subId))) {
                isUsimOrCsim = true;
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[isUsimOrCsimType]catch exception:");
            e.printStackTrace();
        }
        Log.d(TAG, "[isUsimOrCsimType]subId:" + subId + ",isUsimOrCsim:" + isUsimOrCsim);

        return isUsimOrCsim;
    }
}
