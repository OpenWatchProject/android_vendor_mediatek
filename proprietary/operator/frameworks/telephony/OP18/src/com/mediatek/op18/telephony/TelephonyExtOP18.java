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

package com.mediatek.op18.telephony;

import android.content.Context;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
//import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import com.mediatek.internal.telephony.DefaultTelephonyExt;

public class TelephonyExtOP18 extends DefaultTelephonyExt {
    private static final String TAG = "TelephonyExtOP18";
    public static final String SIM_IMPI = "simImpi";
    String[] mImsMccMncList;
    public TelephonyExtOP18(Context context) {
        super(context);
        mContext = context;
    }


    public boolean ignoreDataRoaming(String apnType) {
        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_IMS)) {
            Rlog.d(TAG, "ignoreDataRoaming, apnType = " + apnType);
            return true;
        }
        return false;
    }
    private void getMccMncList() {
        /*
        final String packName = "com.mediatek.op18.plugin";
        String arrayName = "ims_mcc_mnc_list";
        try {
            PackageManager manager = mContext.getPackageManager();
            Resources apkResources = manager.getResourcesForApplication(packName);
            int arrayResID = apkResources.getIdentifier(arrayName, "array",packName);
            String[] myArray = apkResources.getStringArray(arrayResID);
            if (myArray != null) {
                mImsMccMncList = myArray.clone();
            }
        }
        catch (NameNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        */
    }
    @Override
    /**
     * To get mcc&mnc from IMPI, see TelephonyManagerEx.getIsimImpi().
     * @param defaultValue default value
     * @param phoneId phoneId used to get IMPI.
     * @return
     */
    public String getOperatorNumericFromImpi(String defaultValue, int phoneId) {
        final String mccTag = "mcc";
        final String mncTag = "mnc";
        final int mccLength = 3;
        final int mncLength = 3;
        Log.d(TAG, "getOperatorNumbericFromImpi got default mccmnc: " + defaultValue);
        getMccMncList();
        //Log.d(TAG, "got mccMnc Array of size" + mImsMccMncList.length);
        if (mImsMccMncList == null  || mImsMccMncList.length == 0) {
            Log.d(TAG, "Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }
        String impi = null;
        int masterPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        Log.d(TAG, "Impi requested by phoneId: " + phoneId);
        Log.d(TAG, "masterPhoneId:" + masterPhoneId);
        /*if (masterPhoneId != phoneId) {
            Log.d(TAG, "Request from Secondry Sim So Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }*/
        int subIds[] = SubscriptionManager.getSubId(phoneId);
        impi = MtkTelephonyManagerEx.getDefault().getIsimImpi(subIds[0]);

        /*impi = MtkTelephonyManagerEx.getDefault()
                .getIsimImpi(SubscriptionManager.getSubIdUsingPhoneId(phoneId));*/

        if (impi == null  || impi.equals("")) {
            Log.d(TAG, "Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }
        int mccPosition = impi.indexOf(mccTag);
        int mncPosition = impi.indexOf(mncTag);
        if (mccPosition == -1 || mncPosition == -1) {
            Log.d(TAG, "Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }
        String masterMccMnc = impi.substring(mccPosition + mccTag.length(), mccPosition
                + mccTag.length() + mccLength) + impi.substring(mncPosition + mncTag.length(),
                mncPosition + mncTag.length() + mncLength);
        Log.d(TAG, "master MccMnc: " + masterMccMnc);
        if (masterMccMnc == null || masterMccMnc.equals("")) {
            Log.d(TAG, "Returning default mccmnc: " + defaultValue);
            return defaultValue;
        }
        for (String mccMnc : mImsMccMncList) {
            if (masterMccMnc.equals(mccMnc)) {
                Log.d(TAG, "mccMnc matched:" + mccMnc);
                Log.d(TAG, "Returning mccmnc from IMPI: " + masterMccMnc);
                return masterMccMnc;
            }
        }
        Log.d(TAG, "Returning default mccmnc: " + defaultValue);
        return defaultValue;
    }
}
