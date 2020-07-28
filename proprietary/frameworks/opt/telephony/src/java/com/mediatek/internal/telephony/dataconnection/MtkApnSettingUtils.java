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

package com.mediatek.internal.telephony.dataconnection;

import android.os.Bundle;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.data.ApnSetting;
import android.telephony.data.ApnSetting.ApnType;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.dataconnection.ApnSettingUtils;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.uicc.IccRecords;

import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.OpTelephonyCustomizationFactoryBase;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;
import com.mediatek.internal.telephony.uicc.MtkSIMRecords;

import mediatek.telephony.data.MtkApnSetting;

import java.util.List;

public class MtkApnSettingUtils extends ApnSettingUtils {
    private static final String LOG_TAG = "MtkApnSettingUtils";
    private static final boolean DBG = true;

    // M: For OP plug-in
    private static IDataConnectionExt sDataConnectionExt = null;

    // reflection for ApnSettingUtils.mvnoMatches
    private static boolean mvnoMatchesEx(IccRecords r, int mvnoType, String mvnoMatchData) {
        if (mvnoType == MtkApnSetting.MVNO_TYPE_PNN) {
            //M: Support MVNO pnn type
            if ((r.isOperatorMvnoForEfPnn() != null) &&
                    r.isOperatorMvnoForEfPnn().equalsIgnoreCase(mvnoMatchData)) {
                return true;
           }
        }
        return false;
    }

    // reflection for ApnSetting.isMeteredApnType
    private static Bundle isMeteredApnTypeEx(@ApnType int apnType, Phone phone) {
        boolean isRoaming = phone.getServiceState().getDataRoaming();
        boolean useEx = false;
        boolean result = false;

        if (sDataConnectionExt == null) {
            try {
                OpTelephonyCustomizationFactoryBase factoryBase =
                        OpTelephonyCustomizationUtils.getOpFactory(phone.getContext());
                sDataConnectionExt = factoryBase.makeDataConnectionExt(phone.getContext());
            } catch (Exception e) {
                Rlog.e(LOG_TAG, "sDataConnectionExt init fail. e: " + e);
                sDataConnectionExt = null;
            }
        }

        if (sDataConnectionExt != null && sDataConnectionExt.isMeteredApnTypeByLoad()) {
            useEx = true;
            result = sDataConnectionExt.isMeteredApnType(
                    ApnSetting.getApnTypeString(apnType), isRoaming);
        }

        Bundle b = new Bundle();
        b.putBoolean("useEx", useEx);
        b.putBoolean("result", result);
        return b;
    }
}
