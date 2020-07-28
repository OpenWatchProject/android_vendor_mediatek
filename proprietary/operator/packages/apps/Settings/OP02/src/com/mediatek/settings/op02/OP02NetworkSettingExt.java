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
package com.mediatek.settings.op02;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.OperatorInfo;
import com.mediatek.settings.op02.R;
import com.mediatek.settings.ext.DefaultNetworkSettingExt;
import com.mediatek.internal.telephony.MtkSubscriptionManager;

import java.util.ArrayList;
import java.util.List;

/**
 * CU feature,when sim1 is CU card,if searched network is not CU network, must add "forbidden".
 */
public class OP02NetworkSettingExt extends DefaultNetworkSettingExt {

    private static final String TAG = "OP02NetworkSettingExt";

    private static final String CU_NUMERIC_1 = "46001";
    private static final String CU_NUMERIC_2 = "46009";
    private static final String CT_4G_NUMERIC = "46011";
    ///M: GSM_R is for CMCC (railway data simcard),CU need forbidden it
    private static final String GSM_R = "46020";
    private Context mOp02Context;

    /**
     * Construct method.
     * @param context context
     */
    public OP02NetworkSettingExt(Context context) {
        mOp02Context = context;
        Log.d(TAG, "OP02NetworkSettingExt");
    }

    /**
     * If user insert CU card, set CMCC/CT operator networks as forbidden.
     * @param operatorInfoList old operatorInfoList
     * @param subId The sub id user selected
     * @return new list OperatorInfo
     */

//    @Override
//    public String customizeNetworkName(
//            OperatorInfo operatorInfo, int subId, String networkName) {
//        Log.d(TAG, "OP02NetworkSettingExt,customizeNetworkName");
//        if (null == operatorInfo) {
//            Log.d(TAG, "customizeNetworkList return null list");
//            return networkName;
//        }
//        String iccNumeric;
//        TelephonyManager telephonyManager = (TelephonyManager) mOp02Context
//                .getSystemService(Context.TELEPHONY_SERVICE);
//        iccNumeric = telephonyManager.getSimOperator(subId);
//        Log.d(TAG, "customizeNetworkList subId" + subId + ",iccNumeric=" + iccNumeric);
//        //if inserting non CU card in CU load, do not care the situation.
//        if (iccNumeric == null ||
//                (!iccNumeric.equals(CU_NUMERIC_1) && !iccNumeric.equals(CU_NUMERIC_2))) {
//            return networkName;
//        }
//        String cmccOpName =
//            mOp02Context.getResources().getString(com.mediatek.R.string.oper_long_46000);
//        String ctOpName =
//            mOp02Context.getResources().getString(com.mediatek.R.string.oper_long_46003);
//        if (operatorInfo.getOperatorAlphaLong().contains(cmccOpName)
//                || operatorInfo.getOperatorAlphaLong().contains(ctOpName)
//                || operatorInfo.getOperatorAlphaLong().contains(CT_4G_NUMERIC)
//                || operatorInfo.getOperatorAlphaLong().contains(GSM_R)) {
//            return networkName + mOp02Context.getString(R.string.network_name_forbidden);
//        }
//        return networkName;
//    }
    @Override
    public boolean onPreferenceTreeClick(OperatorInfo operatorInfo, int subId) {
        String iccNumeric;
        TelephonyManager telephonyManager =
            (TelephonyManager) mOp02Context.getSystemService(Context.TELEPHONY_SERVICE);
        iccNumeric = telephonyManager.getSimOperator(subId);
        Log.d(TAG, "onPreferenceTreeClick subId" + subId
                + ", iccNumeric=" + iccNumeric);
        if (iccNumeric == null ||
                (!iccNumeric.equals(CU_NUMERIC_1) && !iccNumeric.equals(CU_NUMERIC_2))) {
            return false;
        }
        String cmccOpName =
            mOp02Context.getResources().getString(com.mediatek.R.string.oper_long_46000);
        String ctOpName =
            mOp02Context.getResources().getString(com.mediatek.R.string.oper_long_46003);
        if (operatorInfo.getOperatorAlphaLong().contains(cmccOpName)
                || operatorInfo.getOperatorAlphaLong().contains(ctOpName)
                || operatorInfo.getOperatorAlphaLong().contains(CT_4G_NUMERIC)
                || operatorInfo.getOperatorAlphaLong().contains(GSM_R)) {
            Log.i(TAG, "toast a dialog");
            Toast.makeText(mOp02Context,
                    mOp02Context.getString(R.string.network_do_not_register), 1).show();
            return true;        }
        return false;
    }
}
