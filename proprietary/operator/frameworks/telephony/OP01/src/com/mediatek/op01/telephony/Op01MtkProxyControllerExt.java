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
import android.content.SharedPreferences;
import android.os.SystemProperties;
import android.telephony.Rlog;

import com.mediatek.internal.telephony.IMtkProxyControllerExt;
import com.mediatek.internal.telephony.MtkProxyControllerExt;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;

import java.util.ArrayList;
import java.util.Arrays;

/**
 * Implement methods to support China Mobile requirements.
 *
 * @return
 */
public class Op01MtkProxyControllerExt implements IMtkProxyControllerExt {
    private static final String LOG_TAG = "ProxyControllerOP01";
    private static Context mContext = null;
    private static boolean DBG = true;

    protected static SharedPreferences mSimSwitchPreference;
    private static final String PREF_CATEGORY_RADIO_STATUS = "RADIO_STATUS";
    private static final String PROPERTY_ICCID = "vendor.ril.iccid.sim";

    public Op01MtkProxyControllerExt() {
    }

    public Op01MtkProxyControllerExt(Context context) {
        mSimSwitchPreference = context.getSharedPreferences(PREF_CATEGORY_RADIO_STATUS, 0);
        mContext = context;
    }

    @Override
    public boolean isNeedSimSwitch(int majorPhoneId, int phoneNum) {
        int[] simOpInfo = new int[phoneNum];
        int[] simType = new int[phoneNum];
        int insertedSimCount = 0;
        int insertedStatus = 0;
        String[] currIccId = new String[phoneNum];
        int[] priority = new int[phoneNum];
        int enabledSimCount = 0;
        int tSimCount = 0;
        int wSimCount = 0;
        int cSimCount = 0;

        for (int i = 0; i < phoneNum; i++) {
            currIccId[i] = SystemProperties.get(PROPERTY_ICCID + (i + 1));
            if (currIccId[i] != null && (!("".equals(currIccId[i])))
                    && (!("N/A".equals(currIccId[i])))) {
                ++insertedSimCount;
                insertedStatus = insertedStatus | (1 << i);
                if (mSimSwitchPreference.contains(
                        RadioCapabilitySwitchUtil.getHashCode(currIccId[i])) == false) {
                    ++enabledSimCount;
                }
            }
        }
        log("Op01IsNeedSimSwitch, " + ", enabledSimCount = " + enabledSimCount
                + ", insertedSimCount = " + insertedSimCount + ", insertedStatus = "
                + insertedStatus + ", mPhoneNum = " + phoneNum);

        if (RadioCapabilitySwitchUtil.getSimInfo(simOpInfo, simType, insertedStatus) == false) {
            log("Op01IsNeedSimSwitch, fail to get sim info");
            return true;
        }

        log("Op01IsNeedSimSwitch, simOpInfo:" + Arrays.toString(simOpInfo));
        for (int i = 0; i < phoneNum; i++) {
            if (RadioCapabilitySwitchUtil.SIM_OP_INFO_OP01 == simOpInfo[i]) {
                tSimCount++;
            } else if (RadioCapabilitySwitchUtil.isCdmaCard(i, simOpInfo[i], mContext)) {
                cSimCount++;
            } else if (RadioCapabilitySwitchUtil.SIM_OP_INFO_UNKNOWN!= simOpInfo[i]){
                wSimCount++;
            }
        }

        // w + w / w + empty --> don't need to capability switch
        if (wSimCount == insertedSimCount) {
            log("Op01IsNeedSimSwitch, don't sim switch for non-CMCC gsm sim");
            return false;
        }

        return true;
    }

    static private void log(String txt) {
        if (DBG) {
            Rlog.d(LOG_TAG, txt);
        }
    }
}
