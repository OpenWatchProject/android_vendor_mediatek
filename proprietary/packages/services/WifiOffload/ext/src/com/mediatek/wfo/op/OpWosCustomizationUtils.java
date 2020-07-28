/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.wfo.op;

import android.content.Context;
import android.util.Log;

import java.lang.ClassCastException;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

public class OpWosCustomizationUtils {
    private static final String TAG = "OpWosCustomizationUtils";

    //list every operator's factory path and name.
    private static final List<OperatorFactoryInfo> sOperatorFactoryInfoList
        = new ArrayList<OperatorFactoryInfo>();

    static OpWosCustomizationFactoryBase sFactory = null;

    static {
        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP07Wos.apk",
                    "com.mediatek.op07.wfo.Op07WosCustomizationFactory",
                    "com.mediatek.op07.wfo",
                    "OP07"
                    ));

        sOperatorFactoryInfoList.add(new OperatorFactoryInfo("OP08Wos.apk",
                    "com.mediatek.op08.wfo.Op08WosCustomizationFactory",
                    "com.mediatek.op08.wfo",
                    "OP08"
                    ));
    }

    public static synchronized OpWosCustomizationFactoryBase getOpFactory(Context context) {
        try {
            sFactory = (OpWosCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                .loadFactory(context, sOperatorFactoryInfoList);
        } catch (ClassCastException e) {
            Log.e(TAG, "OpWosCustomizationFactoryBase ClassCastException" , e);
        }
        if (sFactory == null) {
            sFactory = new OpWosCustomizationFactoryBase();
        }

        return sFactory;
    }

}
