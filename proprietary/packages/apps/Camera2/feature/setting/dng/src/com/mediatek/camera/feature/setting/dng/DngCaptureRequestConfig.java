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

package com.mediatek.camera.feature.setting.dng;

import android.annotation.TargetApi;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.DngCreator;
import android.hardware.camera2.TotalCaptureResult;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Size;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.debug.profiler.IPerformanceProfile;
import com.mediatek.camera.common.debug.profiler.PerformanceTracker;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.utils.CameraUtil;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


/**
 * This is for dng capture flow in camera API2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class DngCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure,
        IDngConfig {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            DngCaptureRequestConfig.class.getSimpleName());
    private static final int MAX_RAW_CAPTURE_IMAGES = 3;
    private final Object mCaptureObject = new Object();
    private OnDngValueUpdateListener mDngValueListener;
    private boolean mIsDngOn;
    private boolean mIsTakePicture;
    private List<String> mDngList = new ArrayList<String>();

    private boolean mCaptureResultReady;
    private boolean mRawDataReady;
    private ImageReader mRawImageReader;
    private Surface mRawCaptureSurface;
    private Size mRawSize;
    private TotalCaptureResult mCaptureResult;
    private Image mRawImage;
    private CameraCharacteristics mCharas;
    private ISettingManager.SettingDevice2Requester mSettingDevice2Requester;
    private final Handler mModeHandler;

    /**
     * Constructor of dng capture config in api2.
     * @param settingDevice2Requester device requester.
     */
    public DngCaptureRequestConfig(ISettingManager.SettingDevice2Requester
                                           settingDevice2Requester) {
        mSettingDevice2Requester = settingDevice2Requester;
        mModeHandler = new Handler(Looper.myLooper());
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mDngList.clear();
        mDngList.add(DNG_OFF);
        if (isDngSupported(characteristics)) {
            mDngList.add(DNG_ON);
            mDngValueListener.onDngValueUpdate(mDngList, true);
        } else {
            mDngValueListener.onDngValueUpdate(mDngList, false);
        }
        mCharas = characteristics;
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        LogHelper.d(TAG, "[configCaptureRequest] mIsDngOn : " + mIsDngOn);
        if (mIsDngOn && CameraUtil.isStillCaptureTemplate(captureBuilder)
                && mRawCaptureSurface != null) {
            captureBuilder.addTarget(mRawCaptureSurface);
        }
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {
        LogHelper.d(TAG, "[configSessionSurface] mIsDngOn: " + mIsDngOn);
        if (mIsDngOn) {
            surfaces.add(configRawSurface());
        } else {
            releaseRawSurface();
        }
    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return mCaptureCallback;
    }

    @Override
    public void setDngValueUpdateListener(OnDngValueUpdateListener l) {
        mDngValueListener = l;
    }

    @Override
    public void requestChangeOverrideValues() {

    }

    @Override
    public void setDngStatus(boolean isOn, boolean isTakePicture) {
        mIsDngOn = isOn;
        mIsTakePicture = isTakePicture;
    }

    @Override
    public void notifyOverrideValue(boolean isOn) {}

    @Override
    public void onModeClosed() {
        releaseRawSurface();
        resetDngCaptureStatus();
    }

    private ImageReader.OnImageAvailableListener mRawCaptureImageListener
            = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            LogHelper.i(TAG, "mRawCaptureImageListener reader = " + reader);
            mDngValueListener.onDngCreatorStateUpdate(true);
            mRawImage = reader.acquireLatestImage();
            mRawDataReady = true;
            convertRawToDng(mRawImage);
        }
    };

    private CameraCaptureSession.CaptureCallback mCaptureCallback
            = new CameraCaptureSession.CaptureCallback() {

        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            if (mIsDngOn
                && CameraUtil.isStillCaptureTemplate(result)) {
                LogHelper.i(TAG, "onCaptureCompleted");
                mCaptureResult = result;
                mCaptureResultReady = true;
                mDngValueListener.onDngCreatorStateUpdate(true);
                convertRawToDng(mRawImage);
            }
        }
    };

    @Override
    public void sendSettingChangeRequest() {
        LogHelper.d(TAG, "[sendSettingChangeRequest] mIsDngOn : "
                + mIsDngOn + ", mIsTakePicture" + mIsTakePicture);
        if (!mIsTakePicture) {
            mSettingDevice2Requester.requestRestartSession();
        }
    }

    @Override
    public Surface configRawSurface() {
        LogHelper.i(TAG, "configRawSurface mIsDngOn = " + mIsDngOn);
        if (!mIsDngOn) {
            return null;
        }
        if (mRawCaptureSurface == null) {
            mRawImageReader = mRawImageReader.newInstance(mRawSize.getWidth(), mRawSize.getHeight(),
                    ImageFormat.RAW_SENSOR, MAX_RAW_CAPTURE_IMAGES);
            mRawImageReader.setOnImageAvailableListener(mRawCaptureImageListener,
                    mModeHandler);
            mRawCaptureSurface = mRawImageReader.getSurface();
            LogHelper.d(TAG, "[configRawSurface], new mRawImageReader = " +
                    mRawImageReader);
        }
        return mRawCaptureSurface;
    }

    private void releaseRawSurface() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                synchronized (mCaptureObject) {
                    if (mRawCaptureSurface != null) {
                        mRawImageReader.close();
                        LogHelper.d(TAG, "[releaseCaptureSurface], release mRawImageReader = " +
                                mRawImageReader);
                        mRawCaptureSurface.release();
                        mRawImageReader = null;
                        mRawCaptureSurface = null;
                    }
                    LogHelper.i(TAG, "[releaseRawSurface] called");
                }
            }
        }).start();
    }

    private void resetDngCaptureStatus() {
        mRawDataReady = false;
        mCaptureResultReady = false;
        mIsTakePicture = false;
        mDngValueListener.onDngCreatorStateUpdate(false);
    }

    private boolean isDngSupported(CameraCharacteristics characteristics) {
        List<Integer> capList = DngUtils.getAvailableCapablities(characteristics);
        if (!capList.contains(CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES_RAW)) {
            LogHelper.e(TAG, "[isDngSupported] RAW capablity do not support");
            return false;
        }

        if (!DngUtils.isDngCaptureSizeAvailable(characteristics)) {
            LogHelper.e(TAG, "[isDngSupported] No capture sizes available for raw format");
            return false;
        }

        mRawSize = DngUtils.getRawSize(characteristics);
        return true;
    }

    private void convertRawToDng(Image image) {
        if (!mCaptureResultReady || !mRawDataReady) {
            return;
        }
        if (DngUtils.getRawSize(mCharas) == null) {
            LogHelper.e(TAG, "[convertRawToDng], get raw size error");
            return;
        }
        synchronized (mCaptureObject) {
            if (mRawCaptureSurface != null) {
                mRawSize = DngUtils.getRawSize(mCharas);
                int dngOrientation;
                dngOrientation = DngUtils.getDngOrientation(
                        mDngValueListener.onDisplayOrientationUpdate());
                ByteArrayOutputStream outputStream;
                byte[] dngData = null;
                IPerformanceProfile profile = PerformanceTracker.create(TAG, "dngcreator").start();
                try {
                    DngCreator dngCreator = new DngCreator(mCharas, mCaptureResult);
                    dngCreator.setOrientation(dngOrientation);
                    outputStream = new ByteArrayOutputStream();
                    dngCreator.writeImage(outputStream, image);
                    dngData = outputStream.toByteArray();
                    outputStream.close();
                    image.close();
                } catch (IOException e) {
                    LogHelper.e(TAG, "[convertRawToDng], dng write error");
                }
                profile.stop();
                LogHelper.i(TAG, "[convertRawToDng]");
                mDngValueListener.onSaveDngImage(dngData, mRawSize);
            }
        }
        resetDngCaptureStatus();
    }
}
