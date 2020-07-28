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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

package com.mediatek.camera.v2.services.storage;

import android.content.Context;
import android.content.Intent;
import android.os.Environment;
import android.os.ServiceManager.ServiceNotFoundException;
import android.os.StatFs;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.util.Log;

import com.android.camera.FeatureSwitcher;
import com.android.camera.R;
import com.mediatek.storage.StorageManagerEx;

import java.io.File;
/**
 * Static class to provide storage relate info.
 *
 */
 class Storage {
    public static final long LOW_STORAGE_THRESHOLD;
    public static final long RECORD_LOW_STORAGE_THRESHOLD;

    private static final String TAG = "Storage";
    private static final String DCIM =
            Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).toString();
    public static final String DIRECTORY = DCIM + "/Camera";
    private static final String FOLDER_PATH = "/" + Environment.DIRECTORY_DCIM + "/Camera";

    private static final int CAPTURE_THRESHOLD_1 = 10 * 1024 * 1024;
    private static final int CAPTURE_THRESHOLD_2 = 50 * 1024 * 1024;
    private static final int RECORD_THRESHOLD_1 = 9 * 1024 * 1024;
    private static final int RECORD_THRESHOLD_2 = 48 * 1024 * 1024;

    private static final int STORAGE_STATE_OK = 1;
    private static final long STORAGE_STATE_UNAVAILABLE = -1L;
    private static final long STORAGE_STATE_PREPARING = -2L;
    private static final long STORAGE_STATE_UNKNOWN_SIZE = -3L;
    private static final long STORAGE_STATE_FULL_SDCARD = -4L;

    private static boolean sIsStorageReady;
    private static StorageManager sStorageManager;
    private static Context sContext;
    private static String sMountPoint;
    private static boolean isExtendStorageCanUse = false;

    static {
        if (FeatureSwitcher.isMtkFatOnNand() || FeatureSwitcher.isGmoROM()) {
            LOW_STORAGE_THRESHOLD = CAPTURE_THRESHOLD_1;
            RECORD_LOW_STORAGE_THRESHOLD = RECORD_THRESHOLD_1;
            Log.i(TAG, "LOW_STORAGE_THRESHOLD = " + CAPTURE_THRESHOLD_1);
        } else {
            LOW_STORAGE_THRESHOLD = CAPTURE_THRESHOLD_2;
            RECORD_LOW_STORAGE_THRESHOLD = RECORD_THRESHOLD_2;
            Log.i(TAG, "LOW_STORAGE_THRESHOLD = " + CAPTURE_THRESHOLD_2);
        }
    }
    /**
     * Use to set context for storage manager initialize.
     * @param context the context set to storage manager
     */
     static void setContext(Context context) {
        sContext = context;
        initializeStorageManager();
    }
    /**
     * Use to get storage space.
     * @return storage space,if storage not ready return its error state value.
     */
     static long getAvailableSpace() {
        String state;
        state = sStorageManager.getVolumeState(sMountPoint);
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
            return stat.getAvailableBlocks() * (long) stat.getBlockSize();
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "Fail to access external storage", e);
        }
        return STORAGE_STATE_UNKNOWN_SIZE;
    }
    /**
     * Get storage hint info for show string.
     * if storage is ready and space is enough to use
     * return STORAGE_STATE_OK so don't show string
     * @return string id if storage space is not bigger than 0;
     */
     static int getStorageHintInfo() {
         long leftSpace = getAvailableSpace();
         if (leftSpace > LOW_STORAGE_THRESHOLD) {
             leftSpace = leftSpace - LOW_STORAGE_THRESHOLD;
         } else if (leftSpace > 0) {
             leftSpace = 0;
         }
         if (leftSpace == STORAGE_STATE_UNAVAILABLE
                 || leftSpace == STORAGE_STATE_UNKNOWN_SIZE
                 || leftSpace == STORAGE_STATE_PREPARING) {
             return R.string.can_not_use_storage;
         } else if (leftSpace <= 0) {
             return R.string.storage_full;
         } else {
             return STORAGE_STATE_OK;
         }
    }
    /**
     * OSX requires plugged-in USB storage to have path /DCIM/NNNAAAAA to be
     * imported. This is a temporary fix for bug#1655552.
     */
     static void ensureOSXCompatible() {
        File nnnAAAAA = new File(DCIM, "100ANDRO"); // should check dcim
        if (!(nnnAAAAA.exists() || nnnAAAAA.mkdirs())) {
            Log.e(TAG, "Failed to create " + nnnAAAAA.getPath());
        }
    }
    /**
     * Charge storage is ready to use.
     * @return whether can use
     */
     static boolean isStorageReady() {
        Log.i(TAG, "isStorageReady() mount point = " + sMountPoint + ", return " + sIsStorageReady);
        return sIsStorageReady;
    }
    /**
     * Update default storage directory.
     */
     static void updateDefaultDirectory() {
        mkFileDir(getFileDirectory());
        initializeStorageState();
    }
    /**
     * Get file directory for storage user.
     * @return file path
     */
     static String getFileDirectory() {
         Log.i(TAG, " isExtendStorageCanUse = " + isExtendStorageCanUse);
         String path;
         if (isExtendStorageCanUse) {
             path = sMountPoint + FOLDER_PATH;
         } else {
             path = DIRECTORY;
         }
        return path;
    }
    /**
     * Charge whether the intent is same with default storage.
     * @param intent include storage path.
     * @return whether is same with default storage.
     */
     static boolean isSameStorage(Intent intent) {
        StorageVolume storage = (StorageVolume) intent
                .getParcelableExtra(StorageVolume.EXTRA_STORAGE_VOLUME);

        boolean same = false;
        String mountPoint = null;
        String intentPath = null;
        if (storage != null) {
            if (isExtendStorageCanUse) {
                mountPoint = sMountPoint;
                intentPath = storage.getPath();
                if (mountPoint != null && mountPoint.equals(intentPath)) {
                    same = true;
                }
            }
        }
        Log.d(TAG, "isSameStorage() mountPoint=" + mountPoint + ", intentPath=" + intentPath
                + ", return " + same);
        return same;
    }

    private static void setStorageReady(boolean ready) {
        Log.d(TAG, "setStorageReady(" + ready + ") sIsStorageReady=" + sIsStorageReady);
        sIsStorageReady = ready;
    }

    private static void initializeStorageState() {
        String defaultPath = null;
        try {
            defaultPath = StorageManagerEx.getDefaultPath();
            isExtendStorageCanUse = true;
        } catch (Exception e) {
            e.printStackTrace();
        }
        if (defaultPath != null) {
            sMountPoint = defaultPath;
            String state = sStorageManager.getVolumeState(sMountPoint);
            setStorageReady(Environment.MEDIA_MOUNTED.equals(state));
        } else {
            String state = Environment.getExternalStorageState();
            setStorageReady(Environment.MEDIA_MOUNTED.equals(state));
        }

        Log.d(TAG, "initializeStorageState()  sMountPoint=" + sMountPoint);
    }

    private static void initializeStorageManager() {
        if (sStorageManager == null) {
            try {
                sStorageManager = new StorageManager(sContext, null);
            } catch (IllegalStateException e) {
                e.printStackTrace();
            } catch (ServiceNotFoundException e) {
                e.printStackTrace();
            }
        }
    }

    private static void mkFileDir(String path) {
        File dir = new File(path);
        if (!dir.exists()) {
            Log.d(TAG, "dir not exit,will create this, path = " + path);
            dir.mkdirs();
        }
    }
}
