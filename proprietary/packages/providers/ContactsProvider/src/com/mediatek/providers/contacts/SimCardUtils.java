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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.providers.contacts;

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import com.mediatek.provider.MtkContactsContract.Aas;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.util.SparseArray;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.phb.IMtkIccPhoneBook;

/**
 * Add this class for SIM support.
 */
public class SimCardUtils {

    public static final String TAG = "ProviderSimCardUtils";
    private static final String ACCOUNT_TYPE_POSTFIX = " Account";
    public static TelephonyManager sTelephonyManager;

    /**
     * M: Structure function.
     * @param context context
     */
    public SimCardUtils(Context context) {
        sTelephonyManager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
    }

    /**
     * M: add for mark SIM type.
     */
    public interface SimType {
        String SIM_TYPE_SIM_TAG = "SIM";
        int SIM_TYPE_SIM = 0;

        String SIM_TYPE_USIM_TAG = "USIM";
        int SIM_TYPE_USIM = 1;

        // UIM
        int SIM_TYPE_UIM = 2;
        int SIM_TYPE_CSIM = 3;
        String SIM_TYPE_UIM_TAG = "RUIM";
        // UIM
        // UICC TYPE
        String SIM_TYPE_CSIM_TAG = "CSIM";
        // UICC TYPE
    }

    /**
     * M: [Gemini+] all possible icc card type are put in this array. it's a map
     * of SIM_TYPE => SIM_TYPE_TAG like SIM_TYPE_SIM => "SIM"
     */
    private static final SparseArray<String> SIM_TYPE_ARRAY = new SparseArray<String>();
    static {
        SIM_TYPE_ARRAY.put(SimType.SIM_TYPE_SIM, SimType.SIM_TYPE_SIM_TAG);
        SIM_TYPE_ARRAY.put(SimType.SIM_TYPE_USIM, SimType.SIM_TYPE_USIM_TAG);
        SIM_TYPE_ARRAY.put(SimType.SIM_TYPE_UIM, SimType.SIM_TYPE_UIM_TAG);
        SIM_TYPE_ARRAY.put(SimType.SIM_TYPE_CSIM, SimType.SIM_TYPE_CSIM_TAG);
    }

    /**
     * M: [Gemini+] get the readable sim account type, like "SIM Account".
     *
     * @param simType
     * the integer sim type
     * @return the string like "SIM Account"
     */
    public static String getSimAccountType(int simType) {
        return SIM_TYPE_ARRAY.get(simType) + ACCOUNT_TYPE_POSTFIX;
    }

    /// M: Add for AAS @{
    private static final String MTK_PHONE_BOOK_SERVICE_NAME = "mtksimphonebook";

    private static IMtkIccPhoneBook getIMtkIccPhoneBook() {
        Log.d(TAG, "[getIMtkIccPhoneBook]");
        String serviceName = MTK_PHONE_BOOK_SERVICE_NAME;
        final IMtkIccPhoneBook iIccPhb = IMtkIccPhoneBook.Stub.asInterface(ServiceManager
                .getService(serviceName));
        return iIccPhb;
    }

    /**
     * The function to get AAS by sub id and the index in SIM.
     */
    public static String getAASLabel(String indicator) {
        final String DECODE_SYMBOL = Aas.ENCODE_SYMBOL;
        if (!indicator.contains(DECODE_SYMBOL) || indicator.indexOf(DECODE_SYMBOL) == 0
                || indicator.indexOf(DECODE_SYMBOL) == (indicator.length() - 1)) {
            Log.w(TAG, "[getAASLabel] return;");
            return "";
        }
        String aas = "";
        String keys[] = indicator.split(DECODE_SYMBOL);
        int subId = Integer.valueOf(keys[0]);
        int index = Integer.valueOf(keys[1]);
        Log.d(TAG, "[getAASLabel] subId: " + subId + ",index: " + index);
        if (subId > 0 && index > 0) {//if have the aas label.the index value must > 0.
            try {
                final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
                if (iIccPhb != null) {
                    aas = iIccPhb.getUsimAasById(subId, index);
                }
            } catch (RemoteException e) {
                Log.e(TAG, "[getAASLabel] RemoteException");
            }
        }
        if (aas == null) {
            aas = "";
        }
        Log.d(TAG, "[getAASLabel] aas=" + aas);
        return aas;
    }

    /// @}
}

