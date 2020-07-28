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

package com.mediatek.internal.telephony;

import android.content.res.Resources;
import android.os.AsyncResult;
import android.telephony.Rlog;
import java.util.BitSet;
import java.util.List;
import java.util.ArrayList;
import android.text.TextUtils;

import com.android.internal.telephony.HardwareConfig;
import com.android.internal.telephony.TelephonyDevController;

import com.mediatek.internal.telephony.MtkHardwareConfig;

public class MtkTelephonyDevController extends TelephonyDevController {
    protected static final String LOG_TAG = "MtkTDC";

    public MtkTelephonyDevController() {
        super();
        logd("MtkTelephonyDevController constructor");
    }

    @Override
    protected void initFromResource() {
        Resources resource = Resources.getSystem();
        String[] hwStrings = resource.getStringArray(
            com.android.internal.R.array.config_telephonyHardware);
        if (hwStrings != null) {
            for (String hwString : hwStrings) {
                HardwareConfig hw = new MtkHardwareConfig(hwString);
                if (hw != null) {
                    if (hw.type == HardwareConfig.DEV_HARDWARE_TYPE_MODEM) {
                        updateOrInsert(hw, mModems);
                    } else if (hw.type == HardwareConfig.DEV_HARDWARE_TYPE_SIM) {
                        updateOrInsert(hw, mSims);
                    }
                }
            }
        }
    }

    @Override
    protected void handleGetHardwareConfigChanged(AsyncResult ar) {
        if ((ar.exception == null) && (ar.result != null)) {
            List hwcfg = (List)ar.result;
            for (int i = 0 ; i < hwcfg.size() ; i++) {
                HardwareConfig hw = null;

                hw = (HardwareConfig) hwcfg.get(i);
                if (hw != null) {
                    String str = hw.type + "," + hw.uuid + "," + hw.state;
                    if (hw.type == HardwareConfig.DEV_HARDWARE_TYPE_MODEM) {
                        str = str + "," + hw.rilModel
                                  //+ "," + hw.rat
                                  + ",0"
                                  + "," + hw.maxActiveVoiceCall
                                  + "," + hw.maxActiveDataCall
                                  + "," + hw.maxStandby;
                    } else {
                        str = str + "," + hw.modemUuid;
                    }
                    MtkHardwareConfig mtkHwCfg = new MtkHardwareConfig(str);
                    if (mtkHwCfg.type == HardwareConfig.DEV_HARDWARE_TYPE_MODEM) {
                        updateOrInsert(mtkHwCfg, mModems);
                    } else if (hw.type == HardwareConfig.DEV_HARDWARE_TYPE_SIM) {
                        updateOrInsert(mtkHwCfg, mSims);
                    }
                }
            }
        } else {
            /* error detected, ignore.  are we missing some real time configutation
             * at this point?  what to do...
             */
            loge("handleGetHardwareConfigChanged - returned an error.");
        }
    }
}
