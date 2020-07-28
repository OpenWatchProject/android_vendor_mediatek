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

package com.mediatek.op09clib.telecom;

import android.os.SystemProperties;
import android.util.Log;

import com.mediatek.server.telecom.ext.DefaultCallMgrExt;


public class Op09ClibCallMgrExt extends DefaultCallMgrExt {
    private static final String LOG_TAG = "Op09ClibCallMgrExt";

    /**
     * Should disconnect call calls when dial an ECC out.
     * Only for CMCC Volte PCT test
     *
     * @return whether to disconnect or not
     *
     */
    public boolean shouldDisconnectCallsWhenEcc() {
        return !isTestSim();
    }

    private boolean isTestSim() {
        boolean isTestSim = false;
        isTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1") ||
                   SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1") ||
                   SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1") ||
                   SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1");
        log("isTestSim " + isTestSim);
        return isTestSim;
    }


    /**
     * should resume hold call.
     *
     * @return whether resume hold call or not.
     *
     */
    @Override
    public boolean shouldResumeHoldCall() {
        //always resume hold call in denali svlte project
        return isSvlte();
    }

    private boolean isSvlte() {
        boolean isSvlte = false;
        isSvlte = (SystemProperties.get("ro.vendor.mtk_c2k_lte_mode").equals("1"));
        log("isSvlte=" + isSvlte);
        return isSvlte;
    }
    public void log(String msg) {
        Log.d(LOG_TAG, msg);
    }
}
