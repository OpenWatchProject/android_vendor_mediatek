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

package com.mediatek.server.telecom.ext;

import android.content.Context;
import android.telecom.Log;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import java.util.ArrayList;
import java.util.List;

public class OpTelecomCustomizationUtils {

    private static final String TAG = "OpTelecomCustomizationFactoryBase";

    // list every operator's factory path and name.
    private static final List<OperatorFactoryInfo> sOperatorFactoryInfoList
            = new ArrayList<OperatorFactoryInfo>();
    /// Operator owner register operator info @{
    static {
        if (isOpFactoryLoaderAvailable()) {
            sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo(
                        "OP01Telecom.apk",
                        "com.mediatek.op01.telecom.Op01TelecomCustomizationFactory",
                        "com.mediatek.op01.telecom",
                        "OP01"));
            sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo(
                        "OP07Telecomm.apk",
                        "com.mediatek.op07.telecom.OP07TelecomCustomizationFactory",
                        "com.mediatek.op07.telecom",
                        "OP07"));
            sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo(
                        "OP08Telecom.apk",
                        "com.mediatek.op08.telecom.Op08TelecomCustomizationFactory",
                        "com.mediatek.op08.telecom",
                        "OP08"));
            sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo(
                        "OP09ClibTelecom.apk",
                        "com.mediatek.op09clib.telecom.Op09ClibTelecomCustomizationFactory",
                        "com.mediatek.op09clib.telecom",
                        "OP09",
                        "SEGC"));
            sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo(
                        "OP12Telecomm.apk",
                        "com.mediatek.op12.telecom.Op12TelecomCustomizationFactory",
                        "com.mediatek.op12.telecom",
                        "OP12"));
            sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo(
                        "OP18Telecomm.apk",
                        "com.mediatek.op18.telecom.OP18TelecomCustomizationFactory",
                        "com.mediatek.op18.telecom",
                        "OP18"));
        }
    };
    /// @}


    static OpTelecomCustomizationFactoryBase sFactory = null;
    public static synchronized OpTelecomCustomizationFactoryBase getOpFactory(Context context) {
        if (sFactory == null && isOpFactoryLoaderAvailable()) {
            sFactory = (OpTelecomCustomizationFactoryBase)
                    OperatorCustomizationFactoryLoader.loadFactory(context, sOperatorFactoryInfoList);
        }
        if (sFactory == null) {
            Log.i(TAG, "return default OpTelecomCustomizationFactoryBase");
            sFactory = new OpTelecomCustomizationFactoryBase();
        }
        return sFactory;
    }

    /**
     * For portable.
     * @return
     */
    private static boolean isOpFactoryLoaderAvailable() {
        try {
            Class.forName("com.mediatek.common.util.OperatorCustomizationFactoryLoader");
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    public static synchronized void resetOpFactory(Context context) {
        Log.d(TAG, "resetOpFactory");
        sFactory = null;
    }
}
