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
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/

package com.mediatek.op09c.telephony;

import android.telephony.TelephonyManager;

import com.android.internal.telephony.TelephonyProperties;
import com.mediatek.internal.telephony.ServiceStateTrackerExt;

/**
 * Customization from CT for spn display rule.
 */
public class Op09CServiceStateTrackerExt extends ServiceStateTrackerExt{

    private static final String TAG = "Op09CServiceStateTrackerExt";
    private static final String[] CU_PLMN_SIM = {"46001", "46006", "46009", "45407", "00101"};

    private static final int[] LTE_RSRP_THRESHOLDS = new int[] {
            -123, /* SIGNAL_STRENGTH_POOR */
            -118, /* SIGNAL_STRENGTH_MODERATE */
            -114, /* SIGNAL_STRENGTH_GOOD */
            -105, /* SIGNAL_STRENGTH_GREAT */
    };

    private static final int[] LTE_RSSNR_THRESHOLDS = new int[] {
            -50, /* SIGNAL_STRENGTH_POOR */
            -30, /* SIGNAL_STRENGTH_MODERATE */
            10,  /* SIGNAL_STRENGTH_GOOD */
            90,  /* SIGNAL_STRENGTH_GREAT */
    };

    private static final int[] LTE_RSRP_THRESHOLDS_CU = new int[] {
            -120, /* SIGNAL_STRENGTH_POOR */
            -118, /* SIGNAL_STRENGTH_MODERATE */
            -114, /* SIGNAL_STRENGTH_GOOD */
            -105, /* SIGNAL_STRENGTH_GREAT */
    };

    @Override
    public boolean allowSpnDisplayed() {
        return false;
    }

    @Override
    public boolean needSpnRuleShowPlmnOnly() {
        return true;
    }

    @Override
    public boolean getMtkRsrpOnly() {
        if (isCUCardInserted()) {
            return true;
        } else {
            return false;
        }
    }

    @Override
    public int[] getMtkLteRsrpThreshold() {
        if (isCUCardInserted()) {
            return LTE_RSRP_THRESHOLDS_CU;
        } else {
            return LTE_RSRP_THRESHOLDS;
        }
    }

    @Override
    public int[] getMtkLteRssnrThreshold() {
        if (isCUCardInserted()) {
            return null;
        } else {
            return LTE_RSSNR_THRESHOLDS;
        }
    }

    private boolean isCUCardInserted() {
        int phoneId;
        String simOperator = null;
        for (phoneId = 0; phoneId < TelephonyManager.getDefault().getPhoneCount(); phoneId++) {
            simOperator = TelephonyManager.getTelephonyProperty(phoneId,
                    TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "");
            if (simOperator != null && !simOperator.equals("")) {
                for (String plmn : CU_PLMN_SIM) {
                    if (simOperator.equals(plmn)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
}
