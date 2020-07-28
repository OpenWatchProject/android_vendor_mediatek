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

package com.mediatek.internal.telephony.rsu;

import android.content.Context;
import android.os.SystemProperties;
import android.telephony.Rlog;

import com.android.internal.telephony.CommandsInterface;

import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;
import com.mediatek.internal.telephony.uicc.IMtkRsuSml;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.telephony.MtkTelephonyManagerEx;

public class MtkRsuSmlFactory {
    private static final String TAG = "MtkRsuSmlFactory";
    private static final int OP08_POLICY =
            MtkIccCardConstants.SML_SLOT_LOCK_POLICY_LK_SLOTA_RSU_TMO;

    public static MtkRsuSmlFactory getFactory(Context context) {
        return new MtkRsuSmlFactory();
    }

    public static IMtkRsuSml makeRsuSml(Context context, CommandsInterface[] ci) {
        int policy = MtkTelephonyManagerEx.getDefault().getSimLockPolicy();
        String mode = SystemProperties.get("ro.vendor.sim_me_lock_mode", "");
        logD("[RSU-SIMLOCK] mode: " + mode + ", policy=" + policy + ", ci=" + ci);
        if (mode.equalsIgnoreCase("800") || policy == OP08_POLICY) {
            return new Op08RsuSml15(context, ci);
        } else {
            return OpTelephonyCustomizationUtils.getOpFactory(context).makeRsuSml(context, ci);
        }
    }

    public static boolean isOperatorRsu(IMtkRsuSml rsu, String operator) {
        String op = "";
        if (rsu instanceof Op08RsuSml15) {
            return op.equalsIgnoreCase(operator);
        }
        return false;
    }

    public static IMtkRsuSml makeRsuSml(Context context, CommandsInterface[] ci, String operator) {
        if (operator == null) {
            return makeRsuSml(context, ci);
        } else if(operator.equalsIgnoreCase("OP08")) {
            logD("Create new OP08 rsu instance.");
            return new Op08RsuSml15(context, ci);
        }
        return null;
    }

    /**
     * Check if the rsu is instance of the operator and create a new one if it is not.
     * @param rsu
     * @param context
     * @param ci
     * @param operator
     * @return
     */
    public static IMtkRsuSml checkAndMakeRsuSml(IMtkRsuSml rsu, Context context,
                                                CommandsInterface[] ci, String operator) {
        if (rsu == null) {
            return makeRsuSml(context, ci, operator);
        } else if(isOperatorRsu(rsu, operator)) {
            return rsu;
        }

        return makeRsuSml(context, ci, operator);
    }

    private static void logD(String log)
    {
        Rlog.d(TAG, log);
    }
}
