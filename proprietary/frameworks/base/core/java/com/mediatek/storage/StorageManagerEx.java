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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.storage;

import android.annotation.ProductApi;
import android.os.Environment;
import android.os.Process;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.util.Log;

import java.io.File;


public class StorageManagerEx {
    private static final String TAG = "StorageManagerEx";

    private static final String PROP_SD_DEFAULT_PATH = "persist.vendor.sys.sd.defaultpath";
    private static final String PROP_SD_EXTERNAL_PATH = "vold.path.external_sd";
    private static final String PROP_SD_SWAP = "vold.swap.state";
    private static final String PROP_SD_SWAP_TRUE = "1";
    private static final String PROP_SD_SWAP_FALSE = "0";
    private static final String PROP_DEVICE_TYPE = "ro.build.characteristics";
    private static final String PROP_DEVICE_TABLET = "tablet";

    private static final String STORAGE_PATH_SD1 = "/storage/sdcard0";
    private static final String STORAGE_PATH_SD2 = "/storage/sdcard1";
    private static final String STORAGE_PATH_EMULATED = "/storage/emulated/";
    private static final String STORAGE_PATH_SHARE_SD = "/storage/emulated/0";
    private static final String STORAGE_PATH_SD1_ICS = "/mnt/sdcard";
    private static final String STORAGE_PATH_SD2_ICS = "/mnt/sdcard2";

    private static final String DIR_ANDROID = "Android";
    private static final String DIR_DATA = "data";
    private static final String DIR_CACHE = "cache";

    /**
     * Returns default path for writing.
     * @hide
     * @internal
     */
    @ProductApi
    public static String getDefaultPath() {
        String path = STORAGE_PATH_SD1;
        boolean deviceTablet = false;
        boolean supportMultiUsers = false;

        // Modifications for O1.BSP+ as default path feature is not supported.
        path = Environment.getExternalStorageDirectory().getAbsolutePath();
        Log.i(TAG, " Default path taken as primary storage, path=" + path);
        // Modifications complete for O1.BSP+.
        return path;

    }

    /**
     * set default path for APP to storage data.
     * this ONLY can used by settings.
     * @hide
     * @internal
     */
    public static void setDefaultPath(String path) {
        Log.i(TAG, "setDefaultPath path=" + path);
        if (path == null) {
            Log.e(TAG, "setDefaultPath error! path=null");
            return;
        }

        try {
            SystemProperties.set(PROP_SD_DEFAULT_PATH, path);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException when set default path:" + e);
        }
    }

    /**
     * Generates the path to Gallery.
     * @hide
     * @internal
     */
    public static File getExternalCacheDir(String packageName) {
        if (null == packageName) {
            Log.w(TAG, "packageName = null!");
            return null;
        }

        File externalCacheDir = new File(getDefaultPath());
        if (null == externalCacheDir) {
            Log.w(TAG, "create default path File fail!");
            return null;
        }

        externalCacheDir = Environment.buildPath(externalCacheDir,
                                                 DIR_ANDROID, DIR_DATA, packageName, DIR_CACHE);
        Log.d(TAG, "getExternalCacheDir path = " + externalCacheDir);
        return externalCacheDir;
    }

    /**
        * Returns external SD card path.
        * @hide
        * @internal
        */
    @ProductApi
    public static String getExternalStoragePath() {
        String path = null;
        try {
            path = SystemProperties.get(PROP_SD_EXTERNAL_PATH);
            Log.i(TAG, "getExternalStoragePath path=" + path);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException when getExternalStoragePath:" + e);
        }
        Log.d(TAG, "getExternalStoragePath path=" + path);
        return path ;
    }

    /**
        * Returns internal Storage path.
        * @hide
        * @internal
        */
    public static String getInternalStoragePath() {
        Log.d(TAG, "getInternalStoragePath path= null");
        return null ;
    }

    /**
        * For log tool only.
        * modify internal path to "/storage/emulated/0" for multi user
        * @hide
        * @internal
        */
    public static String getInternalStoragePathForLogger() {
        String path = getInternalStoragePath();
        Log.i(TAG, "getInternalStoragePathForLogger raw path=" + path);
        // if path start with "/storage/emulated/"
        // means MTK_SHARED_SDCARD==true, MTK_2SDCARD_SWAP==false
        // so just check path directly
        if (path != null && path.startsWith(STORAGE_PATH_EMULATED)) {
            path = "/storage/emulated/0";
        }
        Log.i(TAG, "getInternalStoragePathForLogger path=" + path);
        return path;
    }
}
