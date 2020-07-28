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

import android.content.Intent;
import android.net.Uri;

import com.mediatek.camera.common.CameraContext;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.app.IApp;

/**
 * The implement IStorageService.
 * use for those who want to get storage info
 *
 */
public class StorageServiceImpl implements IStorageService {

    private final StorageMonitor mStorageMonitor;
    private final Storage mStorage;
    private final IAppUi mAppUi;
    private final IApp mApp;
    private IAppUi.HintInfo mStorageHint;

    /**
     * The constructor.
     * @param app IApp use for constructor StorageMonitor.
     * @param cameraContext the camera context.
     */
    public StorageServiceImpl(IApp app, CameraContext cameraContext) {
        mStorage = Storage.getStorage(app.getActivity());
        mStorageMonitor = new StorageMonitor(app.getActivity(), mStorage);
        mStorage.updateDefaultDirectory();
        mApp = app;
        mAppUi = app.getAppUi();
        cameraContext.getMediaSaver().addMediaSaverListener(mMediaSaverListener);

        mStorageHint = new IAppUi.HintInfo();
        int id = app.getActivity().getResources().getIdentifier("hint_text_background",
                "drawable", app.getActivity().getPackageName());
        mStorageHint.mBackground = app.getActivity().getDrawable(id);
        mStorageHint.mType = IAppUi.HintType.TYPE_ALWAYS_BOTTOM;
    }

    /**
     * resume function used to register intent filter.
     * this function should not been used by any features
     * only by activity or those file which can know activity onResume().
     */
    public void resume() {
        updateStorageHint();
        mStorageMonitor.registerIntentFilter();
        mStorageMonitor.registerStorageStateListener(mStorageStateListener);
    }
    /**
     * resume function used to unregister intent filter.
     * this function should not been used by any features
     * only by activity or those file which can know activity onPause().
     */
    public void pause() {
        mAppUi.hideScreenHint(mStorageHint);
        mStorageMonitor.unRegisterStorageStateListener(mStorageStateListener);
        mStorageMonitor.unregisterIntentFilter();
    }

    @Override
    public String getFileDirectory() {
        return mStorage.getFileDirectory();
    }

    @Override
    public long getCaptureStorageSpace() {
        long leftSpace = mStorage.getAvailableSpace();
        if (leftSpace > mStorage.getCaptureThreshold()) {
            leftSpace = leftSpace - mStorage.getCaptureThreshold();
        } else if (leftSpace > 0) {
            leftSpace = 0;
        }
        return leftSpace;
    }

    @Override
    public long getRecordStorageSpace() {
        long leftSpace = mStorage.getAvailableSpace();
        if (leftSpace > mStorage.getRecordThreshold()) {
            leftSpace = leftSpace - mStorage.getRecordThreshold();
        } else if (leftSpace > 0) {
            leftSpace = 0;
        }
        return leftSpace;
    }

    @Override
    public void registerStorageStateListener(IStorageStateListener listener) {
        mStorageMonitor.registerStorageStateListener(listener);
    }

    @Override
    public void unRegisterStorageStateListener(IStorageStateListener listener) {
        mStorageMonitor.unRegisterStorageStateListener(listener);
    }

    private MediaSaver.MediaSaverListener mMediaSaverListener =
                                   new MediaSaver.MediaSaverListener() {
        @Override
        public void onFileSaved(Uri uri) {
            updateStorageHint();
        }
    };

    private void updateStorageHint() {
        long leftSpace = mStorage.getAvailableSpace();
        leftSpace = computeStorage(leftSpace);
        if (leftSpace < 0) {
            int id = mApp.getActivity().getResources().getIdentifier("can_not_use_storage",
                    "string", mApp.getActivity().getPackageName());
            mStorageHint.mHintText = mApp.getActivity().getString(id);
            mAppUi.showScreenHint(mStorageHint);
        } else if (leftSpace == 0) {
            int id = mApp.getActivity().getResources().getIdentifier("storage_full",
                    "string", mApp.getActivity().getPackageName());
            mStorageHint.mHintText = mApp.getActivity().getString(id);
            mAppUi.showScreenHint(mStorageHint);
        } else {
            mAppUi.hideScreenHint(mStorageHint);
        }
    }

    private long computeStorage(long avaliableSpace) {
        if (avaliableSpace > mStorage.getCaptureThreshold()) {
            avaliableSpace = (avaliableSpace - mStorage.getCaptureThreshold());
        } else if (avaliableSpace > 0) {
            avaliableSpace = 0;
        }
        return avaliableSpace;
    }

    protected IStorageStateListener mStorageStateListener = new IStorageStateListener() {
        @Override
        public void onStateChanged(int storageState, Intent intent) {
            updateStorageHint();
        }
    };
}
