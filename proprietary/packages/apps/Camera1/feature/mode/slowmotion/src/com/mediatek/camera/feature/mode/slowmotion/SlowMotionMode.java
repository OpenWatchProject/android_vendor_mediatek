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

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.mode.video.VideoMode;
import com.mediatek.camera.common.mode.video.recorder.IRecorder;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.Relation;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * Slow motion mode is use for slow motion record.
 */

public class SlowMotionMode extends VideoMode {
    private static final int VIDEO_FRAME_RATE = 30;
    private static final String KEY_VIDEO_QUALITY = "key_video_quality";
    private static final String KEY_FLASH = "key_flash";
    private static final String KEY_PREVIEW = "preview";
    private static final String VALUE_OFF = "off";
    private static final String VALUE_ON = "on";

    private static final SlowMotionGestureImpl mGestureListener = new SlowMotionGestureImpl();

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
    public String getModeKey() {
        return getClass().getName();
    }

    @Override
    protected IRecorder.RecorderSpec modifyRecorderSpec(
            IRecorder.RecorderSpec recorderSpec, boolean isRecording) {
        recorderSpec.captureRate = getProfile().videoFrameRate;
        recorderSpec.profile = getProfile();
        recorderSpec.videoFrameRate = VIDEO_FRAME_RATE;
        return recorderSpec;
    }

    @Override
    protected void initCameraDevice(CameraApi api) {
        mCameraDevice = new SlowMotionDevice(mApp.getActivity(), mCameraContext);
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
}
