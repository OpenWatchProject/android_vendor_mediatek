/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.common.storage;

import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Environment;
import android.os.StatFs;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.portability.SystemProperties;
import com.mediatek.camera.portability.storage.StorageManagerExt;

import java.io.File;
import java.lang.reflect.Method;

/**
 * A class used to provide storage relate info, this class will
 * be initialized in mtk platform, support unplug sdcard operation.
 */
class Storage {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(Storage.class.getSimpleName());

    private static final String DCIM_CAMERA_FOLDER_ABSOLUTE_PATH =
            Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DCIM).toString() + "/Camera";
    private static final String FOLDER_PATH = "/" + Environment.DIRECTORY_DCIM + "/Camera";

    private static final long CAPTURE_LOW_STORAGE_THRESHOLD;
    private static final long RECORD_LOW_STORAGE_THRESHOLD;

    private static final int CAPTURE_THRESHOLD_1 = 10 * 1024 * 1024;
    private static final int CAPTURE_THRESHOLD_2 = 50 * 1024 * 1024;
    private static final int RECORD_THRESHOLD_1 = 9 * 1024 * 1024;
    private static final int RECORD_THRESHOLD_2 = 48 * 1024 * 1024;

    private static final long STORAGE_STATE_UNAVAILABLE = -1L;
    private static final long STORAGE_STATE_PREPARING = -2L;
    private static final long STORAGE_STATE_UNKNOWN_SIZE = -3L;
    private static final long STORAGE_STATE_FULL_SDCARD = -4L;
    private boolean mIsDefaultPathCanUsed = false;

    static {
        if (isMtkFatOnNand() || isGmoROM()) {
            CAPTURE_LOW_STORAGE_THRESHOLD = CAPTURE_THRESHOLD_1;
            RECORD_LOW_STORAGE_THRESHOLD = RECORD_THRESHOLD_1;
            LogHelper.i(TAG, "CAPTURE_LOW_STORAGE_THRESHOLD = " + CAPTURE_THRESHOLD_1);
        } else {
            CAPTURE_LOW_STORAGE_THRESHOLD = CAPTURE_THRESHOLD_2;
            RECORD_LOW_STORAGE_THRESHOLD = RECORD_THRESHOLD_2;
            LogHelper.i(TAG, "CAPTURE_LOW_STORAGE_THRESHOLD = " + CAPTURE_THRESHOLD_2);
        }
    }

    private static final String CLASS_NAME = "com.mediatek.storage.StorageManagerEx";
    private static final String METHOD_NAME = "getDefaultPath";
    private static Method sGetDefaultPath;
    private static String sMountPoint;
    private static Storage sStorage;
    private static Context sContext;

    static {
        try {
            Class cls = Class.forName(CLASS_NAME);
            if (cls != null) {
                sGetDefaultPath = cls.getDeclaredMethod(METHOD_NAME);
            }
            if (sGetDefaultPath != null) {
                sGetDefaultPath.setAccessible(true);
            }
        } catch (NoSuchMethodException e) {
            LogHelper.e(TAG, "NoSuchMethodException: " + METHOD_NAME);
        } catch (ClassNotFoundException e) {
            LogHelper.e(TAG, "ClassNotFoundException: " + CLASS_NAME);
        }
    }

    /**
     * Singleton pattern.
     * @param context the Context
     */
    private Storage(Context context) {
        if (isExtendStorageCanUsed()) {
            LogHelper.d(TAG, "[Storage] init internal storage");
            initializeStorageManager(context);
        }
    }

    /**
     * Use singleton pattern to create Storage.
     * @param context the Context.
     * @return storage that create.
     */
    static Storage getStorage(Context context) {
        if (sStorage == null) {
            sStorage = new Storage(context);
        }
        sContext = context;
        return sStorage;
    }

    /**
     * Update default storage directory.
     * If camera can write to sd card then after plug in/out sd card and
     * default storage path changed should call this function.
     */
    void updateDefaultDirectory() {
        mkFileDir(getFileDirectory());
    }

    /**
     * Get file directory for storage user.
     * @return file path
     * For mediatek platform and sdk version is bigger than M
     * camera may be have the permission to write data to sd card ,so in this
     * case if default storage is sd card will return sd card path for camera
     * to use,otherwise will follow google design.
     */
    String getFileDirectory() {
        if (isExtendStorageCanUsed()) {
            return sMountPoint + FOLDER_PATH;
        } else {
            return DCIM_CAMERA_FOLDER_ABSOLUTE_PATH;
        }
    }

    /**
     * Use to get storage space.
     *
     * @return storage space,if storage not ready return its error state value.
     * If camera can write data to sd card and default storage is sd card then will
     * return sd card available space,otherwise will return system storage space.
     */
    long getAvailableSpace() {
        if (isExtendStorageCanUsed()) {
            return getAvailableSpace(StorageManagerExt.getVolumeState(sContext, sMountPoint));
        } else {
            return getAvailableSpace(Environment.getExternalStorageState());
        }
    }

    /**
     * Charge whether the intent is same with default storage.
     * @param intent include storage path.
     * @return whether is same with default storage.
     */
    boolean isSameStorage(Intent intent) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && mIsDefaultPathCanUsed) {
            return StorageManagerExt.isSameStorage(intent, sMountPoint);
        } else {
            return false;
        }
    }

    /**
     * Use to return the threshold for video record.
     * @return the result
     * for normal recording if the return value bigger than zero it
     * means can recording,otherwise will not allow to recording.
     * other feature want use this function will add itself's condition.
     */
    long getRecordThreshold() {
        return RECORD_LOW_STORAGE_THRESHOLD;
    }

    /**
     * Use to return the threshold for capture.
     * @return the result.
     * for normal capture if the return value bigger than zero it
     * means can capture,otherwise will not allow to recording.
     * other feature want use this function will add itself's condition.
     */
    long getCaptureThreshold() {
        return CAPTURE_LOW_STORAGE_THRESHOLD;
    }

    private boolean isExtendStorageCanUsed() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && isDefaultPathCanUsed();
    }

    @SuppressWarnings("ResultOfMethodCallIgnored")
    private boolean isDefaultPathCanUsed() {
        if (sGetDefaultPath != null) {
            try {
                sMountPoint = StorageManagerExt.getDefaultPath();
                File dir = new File(sMountPoint + FOLDER_PATH);
                dir.mkdirs();
                boolean isDirectory = dir.isDirectory();
                boolean canWrite = dir.canWrite();
                if (!isDirectory || !canWrite) {
                    mIsDefaultPathCanUsed = false;
                } else {
                    mIsDefaultPathCanUsed = true;
                }
            } catch (Exception e) {
                e.printStackTrace();
                mIsDefaultPathCanUsed = false;
            }
        }
        return mIsDefaultPathCanUsed;
    }

    private long getAvailableSpace(String state) {
        if (Environment.MEDIA_CHECKING.equals(state)) {
            return STORAGE_STATE_PREPARING;
        }
        if (!Environment.MEDIA_MOUNTED.equals(state)) {
            return STORAGE_STATE_UNAVAILABLE;
        }
        File dir = new File(getFileDirectory());
        dir.mkdirs();
        if (!dir.isDirectory() || !dir.canWrite()) {
            return STORAGE_STATE_FULL_SDCARD;
        }
        try {
            StatFs stat = new StatFs(getFileDirectory());
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
                return stat.getAvailableBlocksLong() *  stat.getBlockSizeLong();
            } else {
                return stat.getAvailableBlocks() * (long) stat.getBlockSize();
            }
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, "Fail to access external storage", e);
        }
        return STORAGE_STATE_UNKNOWN_SIZE;
    }

    private void initializeStorageManager(Context context) {
        StorageManagerExt.initStorageManager(context);
    }

    private static void mkFileDir(String path) {
        File dir = new File(path);
        if (!dir.exists()) {
            LogHelper.d(TAG, "dir not exit,will create this, path = " + path);
            dir.mkdirs();
        }
    }

    private static boolean isGmoROM() {
        boolean enable = SystemProperties.getInt("ro.vendor.gmo.rom_optimize", 0) == 1;
        LogHelper.d(TAG, "isGmoRom() return " + enable);
        return enable;
    }

    private static boolean isMtkFatOnNand() {
        boolean enable = SystemProperties.getInt("ro.vendor.mtk_fat_on_nand", 0) == 1;
        LogHelper.d(TAG, "isMtkFatOnNand() return " + enable);
        return enable;
    }
}
