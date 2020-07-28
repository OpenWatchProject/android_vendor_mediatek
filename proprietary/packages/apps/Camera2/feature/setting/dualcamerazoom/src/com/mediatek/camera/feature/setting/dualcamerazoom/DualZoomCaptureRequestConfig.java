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

package com.mediatek.camera.feature.setting.dualcamerazoom;

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Rect;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;

import java.math.BigDecimal;
import java.util.Arrays;
import java.util.List;

/**
 * This is for zoom perform for api2.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class DualZoomCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure,
        IDualZoomConfig {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(DualZoomCaptureRequestConfig.class.getSimpleName());
    private Rect mSensorRect;
    private double mDistanceRatio;
    private OnZoomLevelUpdateListener mZoomUpdateListener;
    private boolean mIsSwitch = false;
    private float mLastZoomRatio = DEFAULT_VALUE;
    private float mBasicZoomRatio;
    private float mCurZoomRatio;
    private float mMaxZoom;
    private float mMinZoom;
    private float[] mZoomRatios;
    private boolean mIsPinch = false;
    private SettingDevice2Requester mSettingDevice2Requester;
    private String mTypeName = IDualZoomConfig.TYPE_OTHER;

    private static final String VSDOF_KEY = "com.mediatek.multicamfeature.multiCamFeatureMode";
    private static final int[] VSDOF_KEY_VALUE = new int[]{0};
    private CaptureRequest.Key<int[]> mVsdofKey = null;
    private DualZoom mDualZoom;
    private Context mContext;

    /**
     * dual zoom capture request configure constructor.
     * @param dualZoom The instance of {@link DualZoom}.
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     * @param context The camera context.
     */
    public DualZoomCaptureRequestConfig(DualZoom dualZoom,
                                        SettingDevice2Requester device2Requester,
                                        Context context) {
        mDualZoom = dualZoom;
        mSettingDevice2Requester = device2Requester;
        mContext = context;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        if (!mZoomUpdateListener.isStereMode()) {
            DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(mContext)
                    .getDeviceDescriptionMap().get(String.valueOf(mDualZoom.getCameraId()));
            if (deviceDescription != null) {
                mVsdofKey = deviceDescription.getKeyVsdof();
            }
        }
        LogHelper.d(TAG, "[setCameraCharacteristics], mVsdofKey is " + mVsdofKey);
        mSensorRect = characteristics.get(CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);
        mMinZoom = ZOOM_MIN_VALUE;
        if (mVsdofKey == null) {
            float maxZoom
                    = characteristics.get(CameraCharacteristics.SCALER_AVAILABLE_MAX_DIGITAL_ZOOM);
            mMaxZoom = maxZoom > 0 ? maxZoom : ZOOM_MAX_VALUE_FRONT;
        } else {
            mMaxZoom = ZOOM_MAX_VALUE;
        }
        //support zoom below 1.0
        CameraCharacteristics.Key<float[]> mZoomRatiosKey =
                getCameraCharacteristicsKey(characteristics, "com" +
                        ".mediatek.multicamfeature.multiCamZoomSteps");
        if (mZoomRatiosKey == null) {
            LogHelper.d(TAG,
                    "[setCameraCharacteristics], mZoomRatiosKey is null");
        } else {
            mZoomRatios = characteristics.get(mZoomRatiosKey);
            //when zoom ratios length < 2 , can't switch zoom
            if (mZoomRatios == null || mZoomRatios.length < 2) {
                LogHelper.d(TAG,
                        "[setCameraCharacteristics], mZoomRatios is illegal");
            } else {
                mMinZoom = mZoomRatios[0];
                mZoomUpdateListener.updateMinZoomSupported(mMinZoom);
                LogHelper.d(TAG,
                        "[setCameraCharacteristics], mZoomRatios is " + Arrays.toString(mZoomRatios));
            }
        }
        LogHelper.d(TAG, "[setCameraCharacteristics], mMaxZoom is " + mMaxZoom);
        mZoomUpdateListener.updateMaxZoomSupported(mMaxZoom);
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (ZOOM_OFF.equals(mZoomUpdateListener.onGetOverrideValue())) {
            reset(captureBuilder);
            return;
        }
        if (mVsdofKey != null) {
            captureBuilder.set(mVsdofKey, VSDOF_KEY_VALUE);
            LogHelper.d(TAG, "[configCaptureRequest], set vsdof key value:" + VSDOF_KEY_VALUE[0]);
        }
        mCurZoomRatio = calculateZoomRatio(mDistanceRatio);
        // apply crop region
        captureBuilder.set(CaptureRequest.SCALER_CROP_REGION, cropRegionForZoom(mCurZoomRatio));
        mLastZoomRatio = mCurZoomRatio;
        mZoomUpdateListener.onZoomRatioUpdate(mCurZoomRatio);
        LogHelper.d(TAG, "[configCaptureRequest] mCurZoomRatio = " + mCurZoomRatio
                + ", mDistanceRatio = " + mDistanceRatio);
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {
    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;
    }

    @Override
    public void sendSettingChangeRequest() {
        //boolean isZoomValid = isZoomValid();
        //LogHelper.d(TAG, "[sendSettingChangeRequest], isZoomValid " + isZoomValid);
        //if (isZoomValid) {
        mSettingDevice2Requester.createAndChangeRepeatingRequest();
        //}
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
    public boolean onScaleStatus(boolean isSwitch, boolean isInit) {
        mIsSwitch = isSwitch;
        if (isSwitch && mLastZoomRatio != ZOOM_MIN_VALUE) {
            mDistanceRatio = 0;
            mBasicZoomRatio = ZOOM_MIN_VALUE;
            return true;
        }
        calculateBasicRatio();
        return false;
    }

    @Override
    public void onScaleType(boolean isPinch) {
        mIsPinch = isPinch;
    }

    private boolean isZoomValid() {
        /*return mCurZoomRatio >= mMinZoom && mCurZoomRatio <= mMaxZoom
                && calculateZoomRatio(mDistanceRatio) != mLastZoomRatio;*/
        return calculateZoomRatio(mDistanceRatio) != mLastZoomRatio;
    }

    private void calculateBasicRatio() {
        if (mLastZoomRatio == DEFAULT_VALUE) {
            mBasicZoomRatio = ZOOM_MIN_VALUE;
        } else {
            mBasicZoomRatio = mLastZoomRatio;
        }
    }

    /**
     * Calculates sensor crop region for a zoom level (zoom >= 1.0).
     *
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
                cropRegionForZoom(ZOOM_MIN_VALUE));
        mLastZoomRatio = ZOOM_MIN_VALUE;
    }

    private float calculateZoomRatio(double distanceRatio) {
        float find = ZOOM_MIN_VALUE; // if not find, return 1.0f.
        float maxRatio = mMaxZoom;
        float minRatio = mMinZoom;
        float curRatio;
        if (mTypeName.equals(IDualZoomConfig.TYPE_PINCH)) {
            LogHelper.d(TAG, "[calculateZoomRatio], TYPE_PINCH ");
            curRatio = (float) (mBasicZoomRatio + DEFAULT_ZOOM_RATIO * distanceRatio);
            if (curRatio <= minRatio) {
                find = minRatio;
            } else if (curRatio >= maxRatio) {
                find = maxRatio;
            } else {
                find = curRatio;
            }
        } else if (mIsSwitch) {
            LogHelper.d(TAG, "[calculateZoomRatio], switch, mLastZoomRatio " + mLastZoomRatio);
            //when zoom ratios length < 2 , can't switch zoom
            if (mZoomRatios == null || mZoomRatios.length < 2) {
                if (mLastZoomRatio != minRatio) {
                    find = minRatio;
                } else {
                    find = TELE_VALUE;
                }
            } else {
                for (int i = 0; i < mZoomRatios.length; i++) {
                    if (mLastZoomRatio == mZoomRatios[i]) {
                        if (i == mZoomRatios.length - 1) {
                            find = mZoomRatios[0];
                        } else {
                            find = mZoomRatios[i + 1];
                        }
                    }
                }
            }
            mIsSwitch = false;
        }  else if (mTypeName.equals(IDualZoomConfig.TYPE_DRAG)) {
            LogHelper.d(TAG, "[calculateZoomRatio], TYPE_DRAG");
            //If zoom supports less than 1.0, there will be slight error in distanceRatio, so
            //round the distanceRatio to one decimal place to make the switch UI display normally
            if (distanceRatio > 0) {
                find = round(minRatio + (float) ((maxRatio - minRatio) * distanceRatio));
            } else {
                find = round(minRatio + (float) ((maxRatio - minRatio) * Math.abs(distanceRatio)));
            }
        } else if (mLastZoomRatio != DEFAULT_VALUE) {
            LogHelper.d(TAG, "[calculateZoomRatio], others");
            find = mLastZoomRatio;
        }
        LogHelper.d(TAG, "[calculateZoomRatio], find " + find);
        return find;
    }

    /**
     * Get CameraCharacteristics key.
     *
     * @param characteristics the camera characteristics.
     * @param key             the request key.
     */
    public static CameraCharacteristics.Key<float[]>
    getCameraCharacteristicsKey(CameraCharacteristics characteristics, String key) {
        if (characteristics == null) {
            LogHelper.i(TAG, "[getRequestKey] characteristics is null");
            return null;
        }
        CameraCharacteristics.Key<float[]> keyP2NotificationRequest = null;
        for (CameraCharacteristics.Key<?> requestKey : characteristics.getKeys()) {
            if (requestKey.getName().equals(key)) {
                LogHelper.i(TAG, "[getRequestKey] key :" + key);
                keyP2NotificationRequest = (CameraCharacteristics.Key<float[]>) requestKey;
            }
        }
        return keyP2NotificationRequest;
    }

    @Override
    public void onScaleTypeName(String typeName) {
        mTypeName = typeName;
    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    /**
     * Round to one decimal place
     *
     * @param find zoom ratio
     */
    public static float round(float find) {
        BigDecimal a = new BigDecimal("1");
        BigDecimal b = new BigDecimal(find);
        return Float.parseFloat(b.divide(a, 1, BigDecimal.ROUND_HALF_UP).toString());
    }
}
