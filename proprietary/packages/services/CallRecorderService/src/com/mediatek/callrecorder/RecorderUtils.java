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

package com.mediatek.callrecorder;

import android.content.Context;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.StatFs;
import android.os.storage.StorageManager;
import android.util.Slog;

import com.mediatek.storage.StorageManagerEx;

import java.io.File;

public final class RecorderUtils {
    private static final String TAG = RecorderUtils.class.getSimpleName();
    // unit is BYTE, totally 2MB
    private static final long CALL_RECORD_LOW_STORAGE_THRESHOLD = 2L * 1024L * 1024L;

    public static boolean diskSpaceAvailable(long sizeAvailable) {
        return (getDiskAvailableSize() - sizeAvailable) > 0;
    }

    public static boolean diskSpaceAvailable(String defaultPath) {
        Slog.d(TAG, "defaultPath = " + defaultPath);
        if (null == defaultPath) {
            return diskSpaceAvailable(CALL_RECORD_LOW_STORAGE_THRESHOLD);
        } else {
            File sdCardDirectory = new File(defaultPath);
            StatFs statfs;
            try {
                if (sdCardDirectory.exists() && sdCardDirectory.isDirectory()) {
                    statfs = new StatFs(sdCardDirectory.getPath());
                } else {
                //    log("-----diskSpaceAvailable: sdCardDirectory is null----");
                    return false;
                }
            } catch (IllegalArgumentException e) {
             //   log("-----diskSpaceAvailable: IllegalArgumentException----");
                return false;
            }
            long blockSize = statfs.getBlockSize();
            long availBlocks = statfs.getAvailableBlocks();
            long totalSize = blockSize * availBlocks;
            Slog.d(TAG, "totalSize = " + totalSize);
            return (totalSize - CALL_RECORD_LOW_STORAGE_THRESHOLD) > 0;
        }
    }

    public static boolean isExternalStorageMounted(Context context) {
        StorageManager storageManager =
                (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        if (null == storageManager) {
           // log("-----story manager is null----");
            return false;
        }
        //String storageState = storageManager.getVolumeState(StorageManagerEx.getDefaultPath());
        String storageState = Environment.getExternalStorageState();
        Slog.d(TAG, "isExternalStorageMounted = " + storageState);
        return storageState.equals(Environment.MEDIA_MOUNTED) ? true : false;
    }

    public static String getExternalStorageDefaultPath(Context context) {
        //return StorageManagerEx.getDefaultPath();
//        StorageManager storageManager =
//            (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
//        return storageManager.getPrimaryStorageVolume().toString();
        return Environment.getExternalStorageDirectory().toString();
    }

    public static long getDiskAvailableSize() {
        //File sdCardDirectory = new File(StorageManagerEx.getDefaultPath());
        File sdCardDirectory = Environment.getExternalStorageDirectory();
        StatFs statfs;
        try {
            if (sdCardDirectory.exists() && sdCardDirectory.isDirectory()) {
                statfs = new StatFs(sdCardDirectory.getPath());
            } else {
             //   log("-----diskSpaceAvailable: sdCardDirectory is null----");
                return -1;
            }
        } catch (IllegalArgumentException e) {
         //   log("-----diskSpaceAvailable: IllegalArgumentException----");
            return -1;
        }
        long blockSize = statfs.getBlockSize();
        long availBlocks = statfs.getAvailableBlocks();
        long totalSize = blockSize * availBlocks;
        Slog.d(TAG, "total size in getDiskAvailableSize() = " + totalSize);
        return totalSize;
    }

    public static boolean isStorageAvailable(Context context) {
        if (!isExternalStorageMounted(context)) {
            Slog.e(TAG, "-----Please insert an SD card----");
            return false;
        }

        if (!diskSpaceAvailable(CALL_RECORD_LOW_STORAGE_THRESHOLD)) {
            Slog.e(TAG, "-----SD card storage is full----");
            return false;
        }

        return true;
    }
}
