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

package com.mediatek.op01.telephony;

import android.content.Context;
import android.content.res.Resources;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.internal.telephony.MtkTelephonyProperties;
import com.mediatek.internal.telephony.ServiceStateTrackerExt;

import mediatek.telephony.MtkServiceState;

/**
 * Implement methods to support China Mobile requirements.
 *
 * @return
 */
public class Op01ServiceStateTrackerExt extends ServiceStateTrackerExt {
    private static final int[] LTE_RSRP_THRESHOLDS = new int[] {
            -120, /* SIGNAL_STRENGTH_POOR */
            -113, /* SIGNAL_STRENGTH_MODERATE */
            -105, /* SIGNAL_STRENGTH_GOOD */
            -97,  /* SIGNAL_STRENGTH_GREAT */
    };

    private Context mContext;

    public Op01ServiceStateTrackerExt() {
    }

    public Op01ServiceStateTrackerExt(Context context) {
        mContext = context;
    }

    @Override
    public String onUpdateSpnDisplay(String plmn, MtkServiceState ss, int phoneId) {
        //[ALPS01663902]-Start
        if (plmn == null) {
            return plmn;
        }
        //[ALPS01663902]-End
        int radioTechnology;
        boolean isRoming;

        radioTechnology = ss.getRilVoiceRadioTechnology();
        isRoming = ss.getRoaming();
        log("onUpdateSpnDisplay: radioTechnology = " + radioTechnology
                + ", phoneId = " + phoneId + ",isRoming = " + isRoming);

        // for LTE
        if (radioTechnology == MtkServiceState.RIL_RADIO_TECHNOLOGY_LTE
                && plmn != Resources.getSystem().getText(
                        com.android.internal.R.string.lockscreen_carrier_default).toString()) {
            plmn = plmn + " 4G";
        } else if (radioTechnology > MtkServiceState.RIL_RADIO_TECHNOLOGY_EDGE
                && radioTechnology != MtkServiceState.RIL_RADIO_TECHNOLOGY_GSM
                && plmn != Resources.getSystem().getText(
                        com.android.internal.R.string.lockscreen_carrier_default).toString()) {
            plmn = plmn + " 3G";
        }
        if (isRoming) {
            String prop1 = TelephonyManager.getTelephonyProperty(
                    phoneId, TelephonyProperties.PROPERTY_ICC_OPERATOR_ALPHA, "");
            log("getSimOperatorName simId = " + phoneId + " prop1 = " + prop1);
            if (prop1.equals("")) {
                String prop2 = TelephonyManager.getTelephonyProperty(
                        phoneId, MtkTelephonyProperties.PROPERTY_ICC_OPERATOR_DEFAULT_NAME, "");
                log("getMTKdefinedSimOperatorName simId = " + phoneId + " prop2 = " + prop2);
                if (!prop2.equals("")) {
                    plmn = plmn + "(" + prop2 + ")";
                }
            } else {
                plmn = plmn + "(" + prop1 + ")";
            }
        }
        log("Current PLMN: " + plmn);
        return plmn;
    }

    @Override
    public boolean needSpnRuleShowPlmnOnly() {
        return true;
    }

    @Override
    public int needAutoSwitchRatMode(int phoneId, String nwPlmn) {
        return -1;
    }

    @Override
    public boolean allowSpnDisplayed() {
        return false;
    }

    @Override
    public int[] getMtkLteRsrpThreshold() {
        return LTE_RSRP_THRESHOLDS;
    }
}
