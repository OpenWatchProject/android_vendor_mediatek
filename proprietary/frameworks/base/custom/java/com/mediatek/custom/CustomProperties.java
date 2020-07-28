/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.custom;

import android.annotation.ProductApi;

// import android.util.Log;
import dalvik.system.PathClassLoader;

public class CustomProperties {
    public static final int PROP_MODULE_MAX = 32;
    public static final int PROP_NAME_MAX = 64;

    /**
      * for module Browser usage
      * @internal
      */
    public static final String MODULE_BROWSER = "browser";
    /**
      * for module MMS usage
      * @internal
      */
    public static final String MODULE_MMS     = "mms";
    /**
      * for module HTTP Streaming usage
      * @internal
      */
    public static final String MODULE_HTTP_STREAMING = "http_streaming";
    /**
      * for module RTSP Steaming usage
      * @internal
      */
    public static final String MODULE_RTSP_STREAMING = "rtsp_streaming";
    /**
      * for module CMMB usage
      * @internal
      */
    public static final String MODULE_CMMB    = "cmmb";
    /**
      * for module WIFI AP usage
      * @internal
      */
    public static final String MODULE_WLAN    = "wlan";
    /**
      * for module FM Transmitter usage
      * @internal
      */
    public static final String MODULE_FMTRANSMITTER = "fmtransmitter";
    /**
      * for module Bluetooth usage
      * @internal
      */
    public static final String MODULE_BLUETOOTH = "bluetooth";
    /**
      * for module DM usage
      * @internal
      */
    public static final String MODULE_DM = "dm";
    /**
      * for system wide module usage
      * @internal
      */
    public static final String MODULE_SYSTEM = "system";

    /**
      * keyword for User Agent
      * @internal
      */
    public static final String USER_AGENT = "UserAgent";
    /**
      * keyword for UA Profile URL
      * @internal
      */
    public static final String UAPROF_URL = "UAProfileURL";
    /**
      * keyword for RDS Value
      * @internal
      */
    public static final String RDS_VALUE  = "RDSValue";
    /**
      * keyword for Manufacturer
      * @internal
      */
    public static final String MANUFACTURER = "Manufacturer";
    /**
      * keyword for Product Model
      * @internal
      */
    public static final String MODEL = "Model";
    /**
      * keyword for WIFI AP SSID
      * @internal
      */
    public static final String SSID = "SSID";
    /**
      * keyword for Host Name
      * @internal
      */
    public static final String HOST_NAME = "HostName";

    static ClassLoader mLoader;

    static {
        System.loadLibrary("custom_jni");
        mLoader = new PathClassLoader("/system/framework/CustomPropInterface.jar", ClassLoader.getSystemClassLoader());
    }

    /**
      * Get configuration value by only indicating keyword
      *
      * @param name indicate Keyword field in the configuration
      * @return String configuration value found, could be empty if nothing match
      *
      * @internal
      */
    public static String getString(String name) {
        return getString(null, name, null);
    }

    /**
      * Get configuration value by indicating Module and keyword
      *
      * @param module indicate Module field in the configuration
      * @param name indicate Keyword field in the configuration
      * @return String configuration value found, could be empty if nothing match
      *
      * @internal
      */
    @ProductApi
    public static String getString(String module, String name) {
        return getString(module, name, null);
    }

    /**
      * Get configuration value by indicating Module and keyword
      *
      * @param module indicate Module field in the configuration
      * @param name indicate Keyword field in the configuration
      * @param defaultValue indicate user defined value in case of nothing matches
      * @return String configuration value found, defaultValue return when nothing match
      *
      * @internal
      */
    @ProductApi
    public static String getString(String module, String name, String defaultValue) {
        if ((module != null) && (module.length() > PROP_MODULE_MAX)) {
            throw new IllegalArgumentException("module.length >" + PROP_MODULE_MAX);
        }
        if ((name == null) || (name.length() > PROP_NAME_MAX)) {
            throw new IllegalArgumentException("name.length > " + PROP_NAME_MAX);
        }

        return native_get_string(module, name, defaultValue);
    }

    private static Class loadInterface() {
        Class clazz;

        try {
            clazz = mLoader.loadClass("com.mediatek.custom.CustomPropInterface");
        } catch (ClassNotFoundException e) {
            clazz = null;
            e.printStackTrace();
        }

        System.out.println("[CustomProp]loadInterface->clazz:" + clazz);

        return clazz;
    }

    private static native String native_get_string(String module, String name, String defaultValue);
}
