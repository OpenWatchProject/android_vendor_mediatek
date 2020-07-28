/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.phone.ext;

import android.content.Context;
import android.util.Log;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import java.util.ArrayList;
import java.util.List;

public class OpPhoneCustomizationUtils {
    private static final String LOG_TAG = "OpPhoneCustomizationUtils";

    // list every operator's factory path and name.
    private static final List<OperatorFactoryInfo> sOperatorFactoryInfoList
            = new ArrayList<OperatorFactoryInfo>();

    static OpPhoneCustomizationFactoryBase sFactory = null;

    static {
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP01TeleService.apk",             // apk name
                        "com.mediatek.op01.phone.plugin.Op01PhoneCustomizationFactory",
                                                                           // factory class
                        "com.mediatek.op01.phone.plugin",                  // apk's package
                        "OP01"                                             // operator id
                )
        );

        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP07TeleService.apk", // apk name
                        "com.mediatek.op07.phone.OP07PhoneCustomizationFactory", // factory class
                        "com.mediatek.op07.phone", // apk's package name
                        "OP07" // operator id
                )
        );
       sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP08TeleService.apk",    // apk name
                         "com.mediatek.op08.phone.Op08PhoneCustomizationFactory",
                                                                           // factory class name
                         "com.mediatek.op08.phone",                        // apk's package
                         "OP08"                                            // operator id
                                                                           // operator segment
                )
        );
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP12TeleService.apk",    // apk name
                         "com.mediatek.op12.phone.Op12PhoneCustomizationFactory",
                                                                           // factory class name
                         "com.mediatek.op12.phone",                 // apk's package
                         "OP12"                                            // operator id
                                                                           // operator segment
                )
        );
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP18TeleService.apk",    // apk name
                         "com.mediatek.op18.phone.Op18PhoneCustomizationFactory",
                                                                           // factory class name
                         "com.mediatek.op18.phone",                 // apk's package
                         "OP18"                                            // operator id
                )
        );
        sOperatorFactoryInfoList.add(
                new OperatorFactoryInfo("OP20TeleService.apk",    // apk name
                         "com.mediatek.op20.phone.Op20PhoneCustomizationFactory",
                                                                  // factory class name
                         "com.mediatek.op20.phone",               // apk's package
                         "OP20"                                   // operator id
                )
        );
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

    public static synchronized OpPhoneCustomizationFactoryBase getOpFactory(Context context) {
        if (sFactory == null) {

            sFactory = (OpPhoneCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                        .loadFactory(context, sOperatorFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpPhoneCustomizationFactoryBase();
            }
        }
        return sFactory;
    }

    public static synchronized void resetOpFactory(Context context) {
        log("resetOpFactory");
        sFactory = null;
    }
}
