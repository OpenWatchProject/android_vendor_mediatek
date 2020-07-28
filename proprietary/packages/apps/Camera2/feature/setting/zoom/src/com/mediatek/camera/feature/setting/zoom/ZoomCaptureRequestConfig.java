/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.feature.setting.zoom;

import android.annotation.TargetApi;
import android.graphics.Rect;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.List;
import java.util.Locale;

/**
 * This is for zoom perform for api2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class ZoomCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure,
        IZoomConfig {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(ZoomCaptureRequestConfig.class.getSimpleName());
    private static final float ZOOM_UNSUPPORTED_DEFAULT_VALUE = 1.0f;
    private static final float DEFAULT_VALUE = -1.0f;
    private Rect mSensorRect;
    private double mDistanceRatio;
    private OnZoomLevelUpdateListener mZoomUpdateListener;
    private boolean mIsUserInteraction;
    private float mLastZoomRatio = DEFAULT_VALUE;
    private float mBasicZoomRatio = ZOOM_UNSUPPORTED_DEFAULT_VALUE;
    private float mCurZoomRatio = ZOOM_UNSUPPORTED_DEFAULT_VALUE;
    private float mMaxZoom;
    private ISettingManager.SettingDevice2Requester mSettingDevice2Requester;

    /**
     * Constructor of zoom parameter config in api2.
     * @param settingDevice2Requester device requester.
     */
    public ZoomCaptureRequestConfig(ISettingManager.SettingDevice2Requester
                                            settingDevice2Requester) {
        mSettingDevice2Requester = settingDevice2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mSensorRect = characteristics.get(CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);
        mMaxZoom = characteristics.get(CameraCharacteristics.SCALER_AVAILABLE_MAX_DIGITAL_ZOOM);
        LogHelper.d(TAG, "[setCameraCharacteristics] MaxZoom: " + mMaxZoom);
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        if (ZOOM_OFF.equals(mZoomUpdateListener.onGetOverrideValue())) {
            LogHelper.d(TAG, "[configCaptureRequest] this: " + this);
            reset(captureBuilder);
            return;
        }
        mCurZoomRatio = calculateZoomRatio(mDistanceRatio);
        // apply crop region
        captureBuilder.set(CaptureRequest.SCALER_CROP_REGION, cropRegionForZoom(mCurZoomRatio));
        mLastZoomRatio = mCurZoomRatio;
        if (mIsUserInteraction) {
            mZoomUpdateListener.onZoomLevelUpdate(getPatternRatio());
        }
        LogHelper.d(TAG, "[configCaptureRequest] this: " + this + ", mCurZoomRatio = "
                + mCurZoomRatio + ", mDistanceRatio = " + mDistanceRatio);
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {
    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;
    }

    @Override
    public void sendSettingChangeRequest() {
        if (isZoomValid()) {
            LogHelper.d(TAG, "[sendSettingChangeRequest]");
            mSettingDevice2Requester.createAndChangeRepeatingRequest();
        }
    }

    @Override
    public void setZoomUpdateListener(OnZoomLevelUpdateListener zoomUpdateListener) {
        mZoomUpdateListener = zoomUpdateListener;
    }

    @Override
    public void onScalePerformed(double distanceRatio) {
        mDistanceRatio = distanceRatio;
    }

    @Override
    public void onScaleStatus(boolean isBegin) {
        mIsUserInteraction = isBegin;
        //must set to 0, since if not scale, it should not zoom.
        mDistanceRatio = 0;
        calculateBasicRatio();
    }

    private boolean isZoomValid() {
        LogHelper.d(TAG, "[isZoomValid] mCurZoomRatio = " + mCurZoomRatio + ", zoomRatio = "
            + calculateZoomRatio(mDistanceRatio) + ", mLastZoomRatio = " + mLastZoomRatio);
        boolean needZoom = mCurZoomRatio >= ZOOM_UNSUPPORTED_DEFAULT_VALUE
                && mCurZoomRatio <= mMaxZoom
                && calculateZoomRatio(mDistanceRatio) != mLastZoomRatio;
        LogHelper.d(TAG, "[isZoomValid] needZoom = " + needZoom);
        return needZoom;
    }

    private void calculateBasicRatio() {
        if (mLastZoomRatio == DEFAULT_VALUE) {
            mBasicZoomRatio = ZOOM_UNSUPPORTED_DEFAULT_VALUE;
        } else {
            mBasicZoomRatio = mLastZoomRatio;
        }
    }

    /**
     * Calculates sensor crop region for a zoom level (zoom >= 1.0).
     * @param ratio the zoom level.
     * @return Crop region.
     */
    private Rect cropRegionForZoom(float ratio) {
        int xCenter = mSensorRect.width() / 2;
        int yCenter = mSensorRect.height() / 2;
        int xDelta = (int) (0.5f * mSensorRect.width() / ratio);
        int yDelta = (int) (0.5f * mSensorRect.height() / ratio);
        return new Rect(xCenter - xDelta, yCenter - yDelta, xCenter + xDelta, yCenter + yDelta);
    }

    private void reset(CaptureRequest.Builder captureBuilder) {
        LogHelper.d(TAG, "[reset]");
        // apply crop region
        captureBuilder.set(CaptureRequest.SCALER_CROP_REGION,
                cropRegionForZoom(ZOOM_UNSUPPORTED_DEFAULT_VALUE));
        mLastZoomRatio = ZOOM_UNSUPPORTED_DEFAULT_VALUE;
    }

    private String getPatternRatio() {
        return "x" + String.format(Locale.ENGLISH, PATTERN, mCurZoomRatio);
    }

    private float calculateZoomRatio(double distanceRatio) {
        float find = ZOOM_UNSUPPORTED_DEFAULT_VALUE; // if not find, return 1.0f.
        float maxRatio = mMaxZoom;
        float minRatio = ZOOM_UNSUPPORTED_DEFAULT_VALUE;
        float curRatio = (float) (mBasicZoomRatio + (maxRatio - minRatio) * distanceRatio);
        if (curRatio <= minRatio) {
            find = minRatio;
        } else if (curRatio >= maxRatio) {
            find = maxRatio;
        } else {
            find = curRatio;
        }
        return find;
    }
}
