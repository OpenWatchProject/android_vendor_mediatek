/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.mode;

import android.app.Activity;
import android.content.Intent;
import android.provider.MediaStore;

import com.mediatek.camera.common.CameraContext;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.FeatureProvider;
import com.mediatek.camera.common.loader.FeatureProvider.FeatureLoadDoneListener;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * Used to manage camera mode type features,the main responsibilities are as follows:
 * 1.Register available mode features to mode list ui.
 * 2.Do camera mode switch.
 */
public class ModeManager implements IModeListener, IAppUiListener.OnModeChangeListener {
    private static final Tag TAG = new Tag(ModeManager.class.getSimpleName());

    private static final String EXTRA_CAPTURE_MODE = "extra_capture_mode";
    private static final String DEFAULT_CAMERA_MODE =
            "com.mediatek.camera.common.mode.photo.PhotoModeEntry";
    private static final String DEFAULT_INTENT_PHOTO_MODE =
            "com.mediatek.camera.common.mode.photo.intent.IntentPhotoModeEntry";
    private static final String DEFAULT_INTENT_VIDEO_MODE =
            "com.mediatek.camera.common.mode.video.intentvideo.IntentVideoModeEntry";
    private final FeatureLoadListener mPluginLoadListener =
            new FeatureLoadListener();

    private String mCurrentEntryKey;
    private CameraApi mCurrentCameraApi;
    private ICameraMode mNewMode;
    private ICameraMode mOldMode;
    private ArrayList<ICameraMode> mBusyModeList = new ArrayList<ICameraMode>();

    private IApp mApp;
    private IAppUi mAppUi;
    private ICameraContext mCameraContext;
    private DeviceUsage mCurrentModeDeviceUsage = null;

    private boolean mResumed = false;

    @Override
    public void create(@Nonnull IApp app) {
        LogHelper.d(TAG, "[create]+");
        mApp = app;
        mCameraContext = new CameraContext();
        mCameraContext.create(mApp, mApp.getActivity());
        mAppUi = app.getAppUi();
        mAppUi.setModeChangeListener(this);

        String defaultModeKey = getDefaultModeKey();
        LogHelper.i(TAG, "[create], default mode:" + defaultModeKey);

        mNewMode = createMode(defaultModeKey);
        //this can't be after mOldMode = mNewMode;
        mCurrentModeDeviceUsage = createDeviceUsage(mNewMode);
        mCurrentCameraApi = mNewMode.getCameraApi();
        mOldMode = mNewMode;
        mCameraContext.getFeatureProvider().registerFeatureLoadDoneListener(mPluginLoadListener);

        mNewMode.init(mApp, mCameraContext, true);
        LogHelper.d(TAG, "[create]-");
    }

    @Override
    public void resume() {
        LogHelper.i(TAG, "[resume]");
        mCameraContext.resume();
        mResumed = true;
        mCameraContext.getFeatureProvider().updateCurrentModeKey(mNewMode.getModeKey());
        mNewMode.resume(mCurrentModeDeviceUsage);
    }

    @Override
    public void pause() {
        LogHelper.i(TAG, "[pause]");
        mResumed = false;
        mNewMode.pause(null);
        mCameraContext.pause();
        mCameraContext.getFeatureProvider().updateCurrentModeKey(null);
    }

    @Override
    public void destroy() {
        LogHelper.i(TAG, "[destroy]");
        mNewMode.unInit();
        mAppUi.setModeChangeListener(null);
        mCameraContext.getFeatureProvider().unregisterPluginLoadDoneListener(
                mPluginLoadListener);
        mCameraContext.destroy();
    }

    @Override
    public void onModeSelected(@Nonnull String newModeKey) {
        LogHelper.i(TAG, "[onModeSelected], (" + mCurrentEntryKey + " -> " + newModeKey + ")");
        if (newModeKey.equals(mCurrentEntryKey)) {
            return;
        }
        if (!mResumed) {
            LogHelper.d(TAG, "[onModeSelected], don't do mode change for state isn't resumed," +
                    " so return");
            return;
        }

        mNewMode = createMode(newModeKey);
        DeviceUsage newUsage = createDeviceUsage(mNewMode);
        mOldMode.pause(newUsage);

        mOldMode.unInit();
        mAppUi.updateCurrentMode(mCurrentEntryKey);
        mNewMode.init(mApp, mCameraContext, false);
        mNewMode.resume(newUsage);
        //cache stereo mode avoid GC if not idle and remove when idle.
        cacheModeByIdleStatus();
        mCurrentModeDeviceUsage = newUsage;
        mOldMode = mNewMode;
    }

    @Override
    public boolean onCameraSelected(@Nonnull String cameraFacing) {
        LogHelper.i(TAG, "[onCameraSelected], switch to camera:" + cameraFacing);
        return mNewMode.onCameraSelected(cameraFacing);
    }

    @Override
    public boolean onUserInteraction() {
        return mNewMode.onUserInteraction();
    }

    private String getDefaultModeKey() {
        String defaultModeKey = DEFAULT_CAMERA_MODE;
        Activity activity = mApp.getActivity();
        Intent intent = activity.getIntent();
        String action = intent.getAction();
        if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)) {
            defaultModeKey = DEFAULT_INTENT_PHOTO_MODE;
        } else if (MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
            defaultModeKey = DEFAULT_INTENT_VIDEO_MODE;
        }
        // Check capture mode is assigned by 3rd party APP or not.
        String extraCaptureMode = intent.getStringExtra(EXTRA_CAPTURE_MODE);
        LogHelper.i(TAG, "[getDefaultModeKey]extraCaptureMode = " + extraCaptureMode);
        if (extraCaptureMode != null) {
            defaultModeKey = extraCaptureMode;
        }
        return defaultModeKey;
    }

    private ICameraMode createMode(String entryKey) {
        String tempEntryKey = entryKey;
        ICameraMode cameraMode = mCameraContext.getFeatureProvider().getInstance(
                new FeatureProvider.Key<>(tempEntryKey, ICameraMode.class),
                null,
                false);
        // if current entry key can not create mode, back to default camera mode.
        if (cameraMode == null) {
            tempEntryKey = DEFAULT_CAMERA_MODE;
            cameraMode = mCameraContext.getFeatureProvider().getInstance(
                new FeatureProvider.Key<>(tempEntryKey, ICameraMode.class),
                null,
                false); // don't check support, because camera may not opened.
        }
        mCurrentEntryKey = tempEntryKey;
        mCameraContext.getFeatureProvider().updateCurrentModeKey(cameraMode.getModeKey());
        LogHelper.i(TAG, "[createMode] entryKey:" + mCurrentEntryKey);
        return cameraMode;
    }

    private DeviceUsage createDeviceUsage(ICameraMode currentMode) {
        if (mOldMode != null) {
            //update current old mode device usage again.
            mCurrentModeDeviceUsage = mOldMode.getDeviceUsage(mCameraContext.getDataStore(), null);
            mCurrentModeDeviceUsage = mCameraContext.getFeatureProvider().updateDeviceUsage(
                    mOldMode.getModeKey(), mCurrentModeDeviceUsage);
        }
        DeviceUsage newDeviceUsage = currentMode.getDeviceUsage(mCameraContext.getDataStore(),
                mCurrentModeDeviceUsage);
        String modeKey = currentMode.getModeKey();
        return mCameraContext.getFeatureProvider().updateDeviceUsage(modeKey, newDeviceUsage);
    }

    /**
     * An implement of {@link FeatureLoadDoneListener} for mode.
     */
    private class FeatureLoadListener implements FeatureLoadDoneListener {
        @Override
        public void onBuildInLoadDone(String cameraId, CameraApi cameraApi) {
            LogHelper.d(TAG, "[onBuildInLoadDone]+ api:" + cameraApi +
                    ", current api:" + mCurrentCameraApi +
                    ",camId:" + cameraId);
            List<IAppUi.ModeItem> modeItems = new ArrayList<>();
            if (cameraApi.equals(mCurrentCameraApi)) {
                modeItems =
                        mCameraContext.getFeatureProvider().getAllModeItems(mCurrentCameraApi);
                if (modeItems.size() > 0) {
                    mAppUi.registerMode(modeItems);
                    mAppUi.updateCurrentMode(mCurrentEntryKey);
                }
            }
            LogHelper.d(TAG, "[onBuildInLoadDone]- modes:" + modeItems.size());
        }

        @Override
        public void onPluginLoadDone(String cameraId, CameraApi cameraApi) {
            LogHelper.d(TAG, "[onPluginLoadDone]+ api:" + cameraApi +
                    ", current api:" + mCurrentCameraApi +
                    ",camId:" + cameraId);
            List<IAppUi.ModeItem> modeItems = new ArrayList<>();
            if (cameraApi.equals(mCurrentCameraApi)) {
                modeItems =
                        mCameraContext.getFeatureProvider().getAllModeItems(mCurrentCameraApi);
                if (modeItems.size() > 0) {
                    mAppUi.registerMode(modeItems);
                    mAppUi.updateCurrentMode(mCurrentEntryKey);
                }
            }
            LogHelper.d(TAG, "[onPluginLoadDone]- mode num:" + modeItems.size());
        }
    }

    private void cacheModeByIdleStatus() {
        LogHelper.d(TAG, "[cacheModeByIdleStatus] idle:" + mNewMode.isModeIdle() + ",size:"
                 + mBusyModeList.size());
        if (!mNewMode.isModeIdle()) {
            mBusyModeList.add(mNewMode);
        }
        for (int i = 0; i < mBusyModeList.size(); i++) {
            if (mBusyModeList.get(i).isModeIdle()) {
                LogHelper.d(TAG, "[cacheModeByIdleStatus] mBusyModeList :" + mBusyModeList.get(i));
                mBusyModeList.remove(i);
            }
        }
    }
}