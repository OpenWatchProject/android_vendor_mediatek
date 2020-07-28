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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.server;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.os.storage.StorageVolume;
import android.os.storage.VolumeInfo;
import android.util.Slog;
import com.android.server.StorageManagerService;

import com.google.android.collect.Lists;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

class MtkStorageManagerService extends StorageManagerService {

    private static final String TAG = "MtkStorageManagerService";

    public MtkStorageManagerService(Context context) {
        super(context);
        registerPrivacyProtectionReceiver();
    }

    public static class MtkStorageManagerServiceLifecycle extends StorageManagerService.Lifecycle {

        public MtkStorageManagerServiceLifecycle(Context context) {
            super(context);
        }

        @Override
        public void onStart() {
            mStorageManagerService = new MtkStorageManagerService(getContext());
            publishBinderService("mount", mStorageManagerService);
            mStorageManagerService.start();
        }

    }

    private static final String PRIVACY_PROTECTION_WIPE
      = "com.mediatek.ppl.NOTIFY_MOUNT_SERVICE_WIPE";
    private static final String PRIVACY_PROTECTION_WIPE_DONE
      = "com.mediatek.ppl.MOUNT_SERVICE_WIPE_RESPONSE";
    private static final Object FORMAT_LOCK = new Object();

    private void registerPrivacyProtectionReceiver() {
        final IntentFilter privacyProtectionFilter = new IntentFilter();
        privacyProtectionFilter.addAction(PRIVACY_PROTECTION_WIPE);
        mContext.registerReceiver(mPrivacyProtectionReceiver,
                                  privacyProtectionFilter,
                                  "com.mediatek.permission.MOUNT_SERVICE_WIPE",
                                  mHandler);
    }

    private ArrayList<VolumeInfo> findVolumeListNeedFormat() {
        Slog.i(TAG, "findVolumeListNeedFormat");
        ArrayList<VolumeInfo> tempVolumes = Lists.newArrayList();
        synchronized (mLock) {
            for (int i = 0; i < mVolumes.size(); i++) {
                final VolumeInfo vol = mVolumes.valueAt(i);
                // if external sd card is formatted as internal storage
                // it shoule be formatted, but don't format data partition
                // which will be formatted by factory reset
                if ((!isUSBOTG(vol) && vol.isVisible()
                        && vol.getType() == VolumeInfo.TYPE_PUBLIC)
                        || (vol.getType() == VolumeInfo.TYPE_PRIVATE
                        && vol.getDiskId() != null)) {
                    tempVolumes.add(vol);
                    Slog.i(TAG, "i will try to format volume= " + vol);
                }
            }
        }
        return tempVolumes;
    }

    private void formatPhoneStorageAndExternalSDCard() {
        final ArrayList<VolumeInfo> tempVolumes = findVolumeListNeedFormat();
        new Thread() {
            public void run() {
                synchronized (FORMAT_LOCK) {
                    int userId = mCurrentUserId;
                    for (int i = 0; i < tempVolumes.size(); i++) {
                        final VolumeInfo vol = tempVolumes.get(i);
                        // if external sd card is formatted as internal storage
                        // we just format it as public storage
                        if (vol.getType() == VolumeInfo.TYPE_PRIVATE
                                && vol.getDiskId() != null) {
                            Slog.i(TAG, "use partition public to format, volume= " + vol);
                            partitionPublic(vol.getDiskId());
                            if (vol.getFsUuid() != null) {
                                forgetVolume(vol.getFsUuid());
                            }
                            continue;
                        }

                        // first need to wait checking state if needed
                        if (vol.getState() == VolumeInfo.STATE_CHECKING) {
                            Slog.i(TAG, "volume is checking, wait..");
                            for (int j = 0; j < 30; j++) {
                                try {
                                    sleep(1000);
                                } catch (InterruptedException ex) {
                                    Slog.e(TAG, "Exception when wait!", ex);
                                }
                                if (vol.getState() != VolumeInfo.STATE_CHECKING) {
                                    Slog.i(TAG, "volume wait checking done!");
                                    break;
                                }
                            }
                        }
                        // then unmount if needed
                        if (vol.getState() == VolumeInfo.STATE_MOUNTED) {
                            Slog.i(TAG, "volume is mounted, unmount firstly"
                                    + ", volume=" + vol);
                            unmount(vol.getId());
                            for (int j = 0; j < 30; j++) {
                                try {
                                    sleep(1000);
                                } catch (InterruptedException ex) {
                                    Slog.e(TAG, "Exception when wait!", ex);
                                }
                                if (vol.getState() == VolumeInfo.STATE_UNMOUNTED) {
                                    Slog.i(TAG, "wait unmount done!");
                                    break;
                                }
                            }
                        }
                        // then unshare if needed
                        /*if (vol.getState() == 9 ) {
                            Slog.i(TAG, "volume is shared, unshared firstly"
                                    + " volume=" + vol);
                            doShareUnshareVolume(vol.getId(), false);
                            for (int j = 0; j < 30; j++) {
                                try {
                                    sleep(1000);
                                } catch (InterruptedException ex) {
                                    Slog.e(TAG, "Exception when wait!", ex);
                                }
                                if (vol.getState() == VolumeInfo.STATE_UNMOUNTED) {
                                    Slog.i(TAG, "wait unshare done!");
                                    break;
                                }
                            }
                        }*/

                        format(vol.getId());
                        Slog.d(TAG, "format Succeed! volume=" + vol);
                    }

                    // notify Privacy Protection that format done
                    Intent intent = new Intent(PRIVACY_PROTECTION_WIPE_DONE);
                    mContext.sendBroadcast(intent, "com.mediatek.permission.MOUNT_SERVICE_WIPE");
                    Slog.d(TAG, "Privacy Protection wipe: send " + intent);
                }
            }
        } .start();
    }

    private final BroadcastReceiver mPrivacyProtectionReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(PRIVACY_PROTECTION_WIPE)) {
                Slog.i(TAG, "Privacy Protection wipe!");
                formatPhoneStorageAndExternalSDCard();
            }
        }
    };

    /**
     * check if volume is USB OTG
     * @return
     */
    public boolean isUSBOTG(VolumeInfo vol) {
        String diskID = vol.getDiskId();
        if (diskID != null) {
            // for usb otg, the disk id same as disk:8:x
            String[] idSplit = diskID.split(":");
            if (idSplit != null && idSplit.length == 2) {
                if (idSplit[1].startsWith("8,")) {
                    Slog.d(TAG, "this is a usb otg");
                    return true;
                }
            }
        }
        return false;
    }
}
