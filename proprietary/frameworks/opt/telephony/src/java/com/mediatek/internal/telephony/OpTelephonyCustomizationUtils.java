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

import android.content.Context;
import android.telephony.Rlog;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;
import com.mediatek.internal.telephony.rsu.MtkRsuSmlFactory;

import dalvik.system.PathClassLoader;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

public class OpTelephonyCustomizationUtils {

    //list every operator's factory path and name.
    private static final List<OperatorFactoryInfo> sOperatorFactoryInfoList
            = new ArrayList<OperatorFactoryInfo>();

    static volatile OpTelephonyCustomizationFactoryBase sFactory = null;

    static {
        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP01Telephony.jar",
                "com.mediatek.op01.telephony.Op01TelephonyCustomizationFactory",
                null,
                "OP01"
        ));

        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP02Telephony.jar",
                "com.mediatek.op02.telephony.Op02TelephonyCustomizationFactory",
                null,
                "OP02"
        ));

        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP08Telephony.jar",
                "com.mediatek.op08.telephony.Op08TelephonyCustomizationFactory",
                null,
                "OP08"
        ));

        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP09CTelephony.jar",
                "com.mediatek.op09c.telephony.Op09CTelephonyCustomizationFactory",
                null,
                "OP09",
                "SEGC"
        ));

        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP12Telephony.jar",
                "com.mediatek.op12.telephony.Op12TelephonyCustomizationFactory",
                null,
                "OP12"
        ));

        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP07Telephony.jar",
                "com.mediatek.op07.telephony.Op07TelephonyCustomizationFactory",
                null,
                "OP07"
        ));

        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP18Telephony.jar",
                "com.mediatek.op18.telephony.Op18TelephonyCustomizationFactory",
                null,
                "OP18"
        ));

        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP129Telephony.jar",
                "com.mediatek.op129.telephony.Op129TelephonyCustomizationFactory",
                null,
                "OP129"
        ));
    }

    public static OpTelephonyCustomizationFactoryBase getOpFactory(Context context) {
        synchronized (OpTelephonyCustomizationFactoryBase.class) {
            sFactory =
                (OpTelephonyCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                .loadFactory(context, sOperatorFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpTelephonyCustomizationFactoryBase();
            }
        }

        return sFactory;
    }

    public static MtkRsuSmlFactory getRsuSmlFactory(Context context) {
        return MtkRsuSmlFactory.getFactory(context);
    }
}
