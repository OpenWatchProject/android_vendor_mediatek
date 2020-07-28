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
package com.mediatek.camera.common.mode.video.recorder;

import android.media.MediaRecorder;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;

import java.io.IOException;


/**
 * Normal recorder is use for any platform to record.
 * and other platform can extend it or implement IRecorder to realize new feature.
 */
public class NormalRecorder implements IRecorder {
    private static final Tag TAG = new Tag(NormalRecorder.class.getSimpleName());
    private static final int ORIENTATION_270 = 270;
    private static final int ORIENTATION_180 = 180;
    private static final int ORIENTATION_90 = 90;
    private static final int ORIENTATION_0 = 0;
    protected MediaRecorder mMediaRecorder;

    /**
     * the construction for NormalRecorder.
     */
    public NormalRecorder() {
        mMediaRecorder = new MediaRecorder();
    }

    @Override
    public void init(RecorderSpec spec) {
        StringBuffer recorderSpec = new StringBuffer();
        checkRecorderSpec(spec);
        if (spec.camera != null) {
            mMediaRecorder.setCamera(spec.camera);
        }

        mMediaRecorder.setVideoSource(spec.videoSource);
        if (spec.isRecordAudio) {
            mMediaRecorder.setAudioSource(spec.audioSource);
            mMediaRecorder.setProfile(spec.profile);
        } else {
            mMediaRecorder.setOutputFormat(spec.profile.fileFormat);
            if (spec.videoFrameRate != 0) {
                mMediaRecorder.setVideoFrameRate(spec.videoFrameRate);
            } else {
                mMediaRecorder.setVideoFrameRate(spec.profile.videoFrameRate);
            }
            mMediaRecorder.setVideoEncodingBitRate(spec.profile.videoBitRate);
            mMediaRecorder.setVideoSize(spec.profile.videoFrameWidth,
                spec.profile.videoFrameHeight);
            mMediaRecorder.setVideoEncoder(spec.profile.videoCodec);
        }
        if (spec.captureRate != 0) {
            mMediaRecorder.setCaptureRate(spec.captureRate);
        }
        mMediaRecorder.setMaxDuration(spec.maxDurationMs);
        if (spec.location != null) {
            mMediaRecorder.setLocation((float) spec.location.getLatitude(),
                    (float) spec.location.getLongitude());
        }
        try {
            mMediaRecorder.setMaxFileSize(spec.maxFileSizeBytes);
        } catch (RuntimeException e) {
            e.printStackTrace();
        }
        if (spec.outFileDes != null) {
            mMediaRecorder.setOutputFile(spec.outFileDes);
            recorderSpec.append("  filePath = " + spec.outFileDes);
        } else {
            mMediaRecorder.setOutputFile(spec.outFilePath);
            recorderSpec.append("  filePath = " + spec.outFilePath);
        }
        mMediaRecorder.setOrientationHint(spec.orientationHint);
        mMediaRecorder.setOnErrorListener(spec.errorListener);
        mMediaRecorder.setOnInfoListener(spec.infoListener);
        recorderSpec.append("  spec.captureRate = " + spec.captureRate);
        recorderSpec.append("  spec.videoFrameRate = " + spec.videoFrameRate);
        recorderSpec.append("  spec.orientationHint = " + spec.orientationHint);
        recorderSpec.append("  spec.profile.videoFrameRate = " + spec.profile.videoFrameRate);
        recorderSpec.append("  spec.profile.videoFrameWidth = " + spec.profile.videoFrameWidth);
        recorderSpec.append("  spec.profile.videoFrameHeight = " + spec.profile.videoFrameHeight);
        LogHelper.d(TAG, "[init] " + recorderSpec.toString());
    }

    @Override
    public MediaRecorder getMediaRecorder() {
        return mMediaRecorder;
    }

    @Override
    public Surface getSurface() {
        return mMediaRecorder.getSurface();
    }

    @Override
    public void prepare() {
        try {
            mMediaRecorder.prepare();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public void start() {
        mMediaRecorder.start();
    }

    @Override
    public void stop() {
        mMediaRecorder.stop();
    }

    @Override
    public void reset() {
        mMediaRecorder.reset();
    }

    @Override
    public void release() {
        mMediaRecorder.setOnErrorListener(null);
        mMediaRecorder.setOnInfoListener(null);
        mMediaRecorder.release();
    }

    private void checkRecorderSpec(RecorderSpec spec) {
        if (spec.profile == null || (spec.outFileDes == null && spec.outFilePath == null)) {
            LogHelper.e(TAG, "profile = " + spec.profile + " outFileDes = "
                            + spec.outFileDes + " outFilePath = " + spec.outFilePath);
            throw new IllegalArgumentException();
        }
        switch (spec.orientationHint) {
            case ORIENTATION_0:
            case ORIENTATION_90:
            case ORIENTATION_180:
            case ORIENTATION_270:
                break;
            default:
                LogHelper.e(TAG, "orientationHint = " + spec.orientationHint);
                throw new IllegalArgumentException();
        }
        switch (spec.videoSource) {
            case MediaRecorder.VideoSource.CAMERA:
            case MediaRecorder.VideoSource.DEFAULT:
            case MediaRecorder.VideoSource.SURFACE:
                break;
            default:
                LogHelper.e(TAG, "videoSource = " + spec.videoSource);
                throw new IllegalArgumentException();
        }
    }
}
