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

package com.mediatek.wfo.plugin;

import java.lang.reflect.Constructor;

import android.content.Context;
import android.os.SystemProperties;
import android.util.Log;

import com.mediatek.wfo.plugin.impl.LegacyComponentFactoryBase;

import dalvik.system.PathClassLoader;


/**
 * This class is used to create IMS related plug-in. It serves IMS apk
 * modules like ImsService. The purpose is to centralize and allow dynamic
 * loading for plug-in of MTK extension & OEM/customer extension
 */
public class ExtensionFactory {

    public static final String LOG_TAG = "WfoExtensionFactory";

    // Class name of Legacy Component
    private static final String LEGACY_COMPONENT_NAME =
            "com.mediatek.wfo.legacy.LegacyComponentFactoryImpl";

    private static final String LEGACY_COMPONENT_CLASS_PATH =
            "/system/framework/mediatek-wfo-legacy.jar";

    // MTK Legacy Componment Factory
    private static LegacyComponentFactory sLegacyComponentFactory;

    /**
     * Make/Get a instance of LegacyComponentFactory (Legacy Architecture) We will
     * try to load plugin from external package, and load default if fail
     *
     * @return Instance of LegacyComponentFactory
     */
    public static LegacyComponentFactory makeLegacyComponentFactory(Context context) {

        if(sLegacyComponentFactory == null) {
            if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
                Log.d(LOG_TAG, "Gen93 detected !");
            }
            else {
                if (SystemProperties.get("ro.vendor.mtk_telephony_add_on_policy", "0").equals("0")) {
                    try {
                        PathClassLoader pathClassLoader = new PathClassLoader(
                                LEGACY_COMPONENT_CLASS_PATH,
                                context.getClassLoader());
                        Class<?> clazz = Class.forName(
                                LEGACY_COMPONENT_NAME, false, pathClassLoader);
                        Constructor<?> constructor = clazz.getConstructor();
                        sLegacyComponentFactory = (LegacyComponentFactory) constructor.newInstance();
                        Log.d(LOG_TAG, "Use Legacy's LegacyComponentFactory");
                    } catch (Exception e) {
                        Log.d(LOG_TAG, "Cannot load legacy factory");
                    }
                }
            }

            if(sLegacyComponentFactory == null) {
                sLegacyComponentFactory = new LegacyComponentFactoryBase();
            }
        }

        return sLegacyComponentFactory;
    }
}
