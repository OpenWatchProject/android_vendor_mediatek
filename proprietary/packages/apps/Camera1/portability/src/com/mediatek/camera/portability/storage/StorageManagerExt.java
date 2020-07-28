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
package com.mediatek.camera.portability.storage;

import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.ServiceManager.ServiceNotFoundException;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;

import com.mediatek.storage.StorageManagerEx;

/**
 * This class used to implement mtk add interface.
 */
public class StorageManagerExt {
    private static StorageManager sStorageManager;
    /**
     * Get the default storage path.
     * @return the default path
     */
    public static String getDefaultPath() {
        return StorageManagerEx.getDefaultPath();
    }

    /**
     * Get the storage Manager.
     * @param context camera context.
     */
    public static void initStorageManager(Context context) {
        if (sStorageManager == null) {
            try {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                    sStorageManager = new StorageManager(context, null);
                } else {
                    sStorageManager = new StorageManager(null, null);
                }
            } catch (IllegalStateException e) {
                e.printStackTrace();
            } catch (ServiceNotFoundException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Get the Storage Volume state.
     * @param context the context.
     * @param mountPoint the mount point.
     * @return state.
     */
    public static String getVolumeState(Context context, String mountPoint) {
        initStorageManager(context);
        return sStorageManager.getVolumeState(mountPoint);
    }

    /**
     * Charge whether the intent is same with sMountPoint.
     * @param intent action intent.
     * @param sMountPoint mount point.
     * @return Charge result.
     */
    public static boolean isSameStorage(Intent intent, String sMountPoint) {
        StorageVolume storage = (StorageVolume) intent
                .getParcelableExtra(StorageVolume.EXTRA_STORAGE_VOLUME);
        boolean same = false;
        String mountPoint = null;
        String intentPath = null;
        if (storage != null) {
            mountPoint = sMountPoint;
            intentPath = storage.getPath();
            if (mountPoint != null && mountPoint.equals(intentPath)) {
                same = true;
            }
        }
        return same;
    }
}
