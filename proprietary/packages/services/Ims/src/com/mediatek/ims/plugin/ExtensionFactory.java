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

package com.mediatek.ims.plugin;

import java.lang.reflect.Constructor;

import android.content.Context;
import android.os.SystemProperties;
import android.util.Log;

import com.mediatek.ims.ImsConstants;
import com.mediatek.ims.plugin.impl.ExtensionPluginFactoryBase;
import com.mediatek.ims.plugin.impl.OemPluginFactoryBase;
import com.mediatek.ims.plugin.impl.LegacyComponentFactoryBase;

import dalvik.system.PathClassLoader;

/**
 * This class is used to create IMS related plug-in. It serves IMS apk
 * modules like ImsService. The purpose is to centralize and allow dynamic
 * loading for plug-in of MTK extension & OEM/customer extension
 */
public class ExtensionFactory {

    public static final String LOG_TAG = "ImsExtensionFactory";

    // Class name of OEM Plug-in
    private static final String OEM_PLUG_IN_NAME =
            "com.mediatek.imsplugin.OemPluginFactoryImpl";
    private static final String OEM_PLUG_IN_CLASS_PATH =
            "/system/framework/mediatek-ims-oem-plugin.jar";

    // Class name of Extension Plug-in
    private static final String EXTENSION_PLUG_IN_NAME =
            "com.mediatek.imsplugin.ExtensionPluginFactoryImpl";
    private static final String EXTENSION_PLUG_IN_CLASS_PATH =
            "/system/framework/mediatek-ims-extension-plugin.jar";

    // Class name of Legacy Component
    private static final String LEGACY_COMPONENT_NAME =
            "com.mediatek.ims.legacy.LegacyComponentFactoryImpl";
    private static final String LEGACY_COMPONENT_CLASS_PATH =
            "/system/framework/mediatek-ims-legacy.jar";

    // OEM Plug-in Instance Factory
    private static OemPluginFactory sOemPluginFactory;

    // MTK Extension Plug-in Factory
    private static ExtensionPluginFactory sExtensionPluginFactory;

    // MTK Legacy Componment Factory
    private static LegacyComponentFactory sLegacyComponentFactory;

    /**
     * Make/Get a instance of OemPluginFactory (Customer/OEM Logic) We will try
     * to load plug-in from external package, and load default if fail
     *
     * @return Instance of OemPluginFactory
     */
    public static OemPluginFactory makeOemPluginFactory(Context mContext) {

        if (sOemPluginFactory == null) {

            try {
                PathClassLoader pathClassLoader = new PathClassLoader(OEM_PLUG_IN_CLASS_PATH,
                        mContext.getClassLoader());
                Log.d(LOG_TAG , "pathClassLoader = " + pathClassLoader);

                Class<?> clazz = Class.forName(OEM_PLUG_IN_NAME, false, pathClassLoader);
                Constructor<?> constructor = clazz.getConstructor();
                OemPluginFactory instance = (OemPluginFactory) constructor.newInstance();
                sOemPluginFactory = instance;
                Log.d(LOG_TAG, "Use customer's OemPluginFactory");
            } catch (Exception e) {
                Log.d(LOG_TAG, "Use default OemPluginFactory");
            }

            if (sOemPluginFactory == null) {
                sOemPluginFactory = new OemPluginFactoryBase();
            }
        }

        return sOemPluginFactory;
    }

    /**
     * Make/Get a instance of ExtensionPluginFactory (MTK Addon Logic) We will
     * try to load plugin from external package, and load default if fail
     *
     * @return Instance of ExtensionPluginFactory
     */
    public static ExtensionPluginFactory makeExtensionPluginFactory(Context mContext) {

        if (sExtensionPluginFactory == null) {
            if (SystemProperties.get("ro.vendor.mtk_telephony_add_on_policy", "0").equals("0")) {
                try {
                    PathClassLoader pathClassLoader = new PathClassLoader(
                        EXTENSION_PLUG_IN_CLASS_PATH,
                            mContext.getClassLoader());
                    Class<?> clazz = Class.forName(EXTENSION_PLUG_IN_NAME, false, pathClassLoader);
                    Constructor<?> constructor = clazz.getConstructor();
                    ExtensionPluginFactory inst = (ExtensionPluginFactory)constructor.newInstance();
                    sExtensionPluginFactory = inst;
                    Log.d(LOG_TAG, "Use MTK's ExtensionPluginFactory");
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }

            if (sExtensionPluginFactory == null) {
                Log.d(LOG_TAG, "Use default ExtensionPluginFactory");
                sExtensionPluginFactory = new ExtensionPluginFactoryBase();
            }
        }

        return sExtensionPluginFactory;
    }

    /**
     * Make/Get a instance of ExtensionPluginFactory (MTK Addon Logic) We will
     * try to load plugin from external package, and load default if fail
     *
     * @return Instance of ExtensionPluginFactory
     */
    public static LegacyComponentFactory makeLegacyComponentFactory(Context mContext) {

        if(sLegacyComponentFactory == null) {
            if (SystemProperties.get(ImsConstants.SYS_PROP_MD_AUTO_SETUP_IMS).equals("1")) {
                Log.d(LOG_TAG, "Gen93 detected !");
            } else {
                if (SystemProperties.get("ro.vendor.mtk_telephony_add_on_policy", "0").equals("0")){
                    try {
                        PathClassLoader pathClassLoader = new PathClassLoader(
                                LEGACY_COMPONENT_CLASS_PATH,
                                mContext.getClassLoader());
                        Class<?> clazz = Class.forName(
                                LEGACY_COMPONENT_NAME, false, pathClassLoader);
                        Constructor<?> constructor = clazz.getConstructor();
                        sLegacyComponentFactory = (LegacyComponentFactory)constructor.newInstance();
                        Log.d(LOG_TAG, "Use Legacy's LegacyComponentFactory");
                    } catch (Exception e) {
                        Log.d(LOG_TAG, "Cannot load legacy factory");
                    }
                }
            }

            if(sLegacyComponentFactory == null) {
                Log.d(LOG_TAG, "Use default LegacyComponentFactory");
                sLegacyComponentFactory = new LegacyComponentFactoryBase();
            }
        }

        return sLegacyComponentFactory;
    }
}
