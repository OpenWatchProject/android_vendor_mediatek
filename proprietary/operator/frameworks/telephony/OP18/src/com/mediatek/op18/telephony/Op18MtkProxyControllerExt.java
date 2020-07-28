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
import android.os.SystemProperties;
import android.telephony.Rlog;

import com.mediatek.internal.telephony.IMtkProxyControllerExt;
import com.mediatek.internal.telephony.MtkProxyControllerExt;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;

/**
 * Implement methods to support China Mobile requirements.
 *
 * @return
 */
public class Op18MtkProxyControllerExt implements IMtkProxyControllerExt {
    private static final String LOG_TAG = "ProxyControllerOP18";
    private static boolean DBG = true;
    protected Context mContext;

    public Op18MtkProxyControllerExt() {
    }

    public Op18MtkProxyControllerExt(Context context) {
        mContext = context;
    }

    @Override
    public boolean isNeedSimSwitch(int majorPhoneId, int phoneNum) {
        int isSubsidyLockSupport = SystemProperties.getInt("ro.vendor.mtk_subsidy_lock_support", 0);
        log("Op18IsNeedSimSwitch, isSubsidyLockSupport = " + isSubsidyLockSupport);
        if (isSubsidyLockSupport == 1) {
            return true;
        } else {
            return !RadioCapabilitySwitchUtil.isSkipCapabilitySwitch(
                    majorPhoneId, phoneNum, mContext);
        }
    }

    static private void log(String txt) {
        if (DBG) {
            Rlog.d(LOG_TAG, txt);
        }
    }
}