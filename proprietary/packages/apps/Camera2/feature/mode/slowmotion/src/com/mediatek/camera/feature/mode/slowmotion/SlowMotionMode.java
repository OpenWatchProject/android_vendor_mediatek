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
package com.mediatek.camera.feature.mode.slowmotion;


import android.content.ContentValues;
import android.view.View;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.mode.video.VideoMode;
import com.mediatek.camera.common.mode.video.recorder.IRecorder;
import com.mediatek.camera.common.mode.video.videoui.IVideoUI;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.feature.setting.slowmotionquality.SlowMotionSpec;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * Slow motion mode is use for slow motion record.
 */

public class SlowMotionMode extends VideoMode {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SlowMotionMode.class.getSimpleName());

    private static final int BURST_RECORDING_MILL_SECONDS = 1000;
    private static final int VIDEO_FRAME_RATE = 30;
    private static final String KEY_VIDEO_QUALITY = "key_video_quality";
    private static final String KEY_FLASH = "key_flash";
    private static final String KEY_PREVIEW = "preview";
    private static final String VALUE_OFF = "off";
    private static final String VALUE_ON = "on";

    private static final SlowMotionGestureImpl mGestureListener = new SlowMotionGestureImpl();
    private boolean mCanDoBurst = false;
    private SlowMotionDevice mSlowMotionDevice2Controller;

    private View.OnClickListener mBurstListener = new View.OnClickListener() {
        public void onClick(View view) {
            LogHelper.i(TAG, "[mBurstListener] click video state = "
                    + mVideoState + "mCanDoBurst = " + mCanDoBurst);
            if (getVideoState() == VideoState.STATE_RECORDING && mCanDoBurst) {
                mAppUi.animationStart(IAppUi.AnimationType.TYPE_CAPTURE, null);
                mCameraDevice.updateGSensorOrientation(mApp.getGSensorOrientation());
                burstRecording();
                mModeHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        stopBurstRecording();
                    }
                }, BURST_RECORDING_MILL_SECONDS);
                mCanDoBurst = false;
            }
        }
    };

    private SlowMotionDevice.BurstSavedCallback mBurstSavedCallback =
            new SlowMotionDevice.BurstSavedCallback() {
                @Override
                public void onBurstSaved() {
                    mVideoUi.enableBurst();
                    mCanDoBurst = true;
                }
            };

    @Override
    public void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext,
                     boolean isFromLaunch) {
        super.init(app, cameraContext, isFromLaunch);
        mCameraApi = CameraApi.API2;
        mAppUi.registerGestureListener(mGestureListener, 0);
    }

    @Override
    public void unInit() {
        super.unInit();
        mAppUi.unregisterGestureListener(mGestureListener);
    }

    @Override
    public boolean onShutterButtonClick() {
        boolean res = super.onShutterButtonClick();
        if (getVideoState() == VideoState.STATE_RECORDING
                && mSlowMotionDevice2Controller.getSlowMotionSpec().isBurstSupported) {
            mCanDoBurst = true;
        }
        return res;
    }

    @Override
    public String getModeKey() {
        return getClass().getName();
    }

    @Override
    protected IRecorder.RecorderSpec modifyRecorderSpec(
            IRecorder.RecorderSpec recorderSpec, boolean isRecording) {
        SlowMotionSpec spec = mSlowMotionDevice2Controller.getSlowMotionSpec();
        if (spec.pattern == SlowMotionSpec.Pattern.NORMAL) {
            recorderSpec.captureRate = getProfile().videoFrameRate;
        } else {
            recorderSpec.captureRate = spec.meta.fpsMax;
        }
        recorderSpec.profile = getProfile();
        recorderSpec.videoFrameRate = VIDEO_FRAME_RATE;
        return recorderSpec;
    }

    @Override
    protected IVideoUI.UISpec modifyUISpec(IVideoUI.UISpec spec) {
        SlowMotionSpec slowMotionSpec = mSlowMotionDevice2Controller.getSlowMotionSpec();
        switch (slowMotionSpec.pattern) {
            case NORMAL:
                break;
            case CONTINUOUS:
                spec.isSupportedVss = slowMotionSpec.isVssSupported;
                spec.isSupportedBurst = slowMotionSpec.isBurstSupported;
                break;
            case BURST:
                spec.isSupportedVss = slowMotionSpec.isVssSupported;
                spec.isSupportedBurst = slowMotionSpec.isBurstSupported;
                spec.normalFps = slowMotionSpec.fpsRangeForRecording.getLower();
                spec.burstFps = slowMotionSpec.fpsRangeForBurstRecording.getUpper();
                spec.burstListener = mBurstListener;
                break;
            default:
                break;
        }
        return spec;
    }

    @Override
    protected void initCameraDevice(CameraApi api) {
        mCameraDevice = new SlowMotionDevice(mApp.getActivity(), mCameraContext);
        mSlowMotionDevice2Controller = (SlowMotionDevice) mCameraDevice;
    }

    @Override
    protected ContentValues modifyContentValues(ContentValues contentValues) {
        return contentValues;
    }

    @Override
    protected Relation getPreviewedRestriction() {
        Relation relation
                = SlowMotionRestriction.getPreviewRelation().getRelation(KEY_PREVIEW, true);
        String flashValue = mSettingManager.getSettingController().queryValue(KEY_FLASH);
        if (!VALUE_ON.equals(flashValue)) {
            flashValue = VALUE_OFF;
        }
        relation.addBody(KEY_FLASH, flashValue, VALUE_ON + "," + VALUE_OFF);
        String qualityValue = mSettingManager.getSettingController().queryValue(KEY_VIDEO_QUALITY);
        relation.addBody(KEY_VIDEO_QUALITY, qualityValue, qualityValue);
        return relation;
    }

    @Override
    protected List<Relation> getRecordedRestriction(boolean isRecording) {
        return null;
    }

    @Override
    protected void updateModeDefinedCameraApi() {
        mCameraApi = CameraApi.API2;
    }

    private void burstRecording() {
        LogHelper.i(TAG, "[burstRecording] +");
        mSlowMotionDevice2Controller.burstRecording(mBurstSavedCallback);
        mVideoUi.updateUIState(IVideoUI.VideoUIState.STATE_BURST_RECORDING);
        LogHelper.i(TAG, "[burstRecording] -");
    }

    private void stopBurstRecording() {
        LogHelper.i(TAG, "[stopBurstRecording] +");
        mSlowMotionDevice2Controller.stopBurstRecording();
        mVideoUi.updateUIState(IVideoUI.VideoUIState.STATE_RESUME_RECORDING_AFTER_BURST);
        LogHelper.i(TAG, "[stopBurstRecording] -");
    }

    @Override
    protected void onSettingConfigCallback(Size previewSize) {
        LogHelper.d(TAG, "[onSettingConfigCallback] mSurface = " + mSurface
                + ", release and set it as null");
        if (mSurface != null) {
            mSurface.release();
            mSurface = null;
        }
    }
}
