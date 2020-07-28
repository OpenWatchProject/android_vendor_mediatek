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

package com.mediatek.camera.feature.setting.zoom;

import android.hardware.Camera;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.List;
import java.util.Locale;

/**
 * zoom perform implementer for api1.
 * Support two ways to perform zoom, one is parameter perform, the other is
 * smooth zoom.
 */

@SuppressWarnings("deprecation")
public class ZoomParameterConfig implements ICameraSetting.IParametersConfigure,
        IZoomConfig {
    private static final Tag TAG = new Tag(ZoomParameterConfig.class.getSimpleName());
    private static final int RESET_VALUE = 0;
    //Zoom ratios list device supported. Such as 100/115/...400.
    private List<Integer> mZoomRatios;
    private int mZoomLevel;
    private CameraProxy mCameraProxy;
    private double mDistanceRatio;
    private boolean mIsZoomSupported;
    private boolean mIsSmoothZoomSupported;
    private int mLastZoomLevel = -1;
    private int mBasicZoomRatio;
    private boolean mIsZoomStopped = true;
    private int mMaxZoom;
    private OnZoomLevelUpdateListener mZoomUpdateListener;
    private boolean mIsUserInteraction;
    private Object mSyncSmoothState = new Object();
    private ISettingManager.SettingDeviceRequester mSettingDeviceRequester;

    /**
     * Constructor of zoom parameter config in api1.
     * @param settingDeviceRequester device requester.
     */
    public ZoomParameterConfig(ISettingManager.SettingDeviceRequester settingDeviceRequester) {
        mSettingDeviceRequester = settingDeviceRequester;
    }

    /**
     * This is onScale to prepare the zoom level.
     *
     * @param distanceRatio the zoom distance ratio.
     */
    public void onScalePerformed(double distanceRatio) {
        mDistanceRatio = distanceRatio;
    }

    /**
     * This is to notify scale status.
     *
     * @param isBegin the scale begin or not
     */
    public void onScaleStatus(boolean isBegin) {
        mIsUserInteraction = isBegin;
        mDistanceRatio = 0;
        calculateBasicRatio();
    }

    /**
     * This is to register zoom changed listener.
     *
     * @param zoomUpdateListener the zoom update listener.
     */
    public void setZoomUpdateListener(OnZoomLevelUpdateListener zoomUpdateListener) {
        mZoomUpdateListener = zoomUpdateListener;
    }

    /**
     * called after camera open.
     * @param parameters camera parameter.
     */
    @Override
    public void setOriginalParameters(Camera.Parameters parameters) {

        mZoomRatios = parameters.getZoomRatios();
        mIsZoomSupported = parameters.isZoomSupported();
        mIsSmoothZoomSupported = parameters.isSmoothZoomSupported();
        mMaxZoom = parameters.getMaxZoom();
        LogHelper.d(TAG, "[setOriginalParameters] mIsZoomSupported = " + mIsZoomSupported);
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        if (!mIsZoomSupported) {
            return false;
        }
        if (ZOOM_OFF.equals(mZoomUpdateListener.onGetOverrideValue())) {
            reset(parameters);
            return false;
        }
        mZoomLevel = calculateZoomLevel(mDistanceRatio);
        parameters.setZoom(mZoomLevel);
        mLastZoomLevel = mZoomLevel;
        if (mIsUserInteraction) {
            mZoomUpdateListener.onZoomLevelUpdate(getZoomRatio());
        }
        LogHelper.d(TAG, "[configParameters] this: " + this + ", mZoomLevel = " + mZoomLevel
                + ", mDistanceRatio = " + mDistanceRatio);
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {
        if (!mIsSmoothZoomSupported) {
            return;
        }
        mCameraProxy = cameraProxy;
        mZoomLevel = calculateZoomLevel(mDistanceRatio);
        cameraProxy.setZoomChangeListener(mZoomListener);
        synchronized (mSyncSmoothState) {
            cameraProxy.startSmoothZoom(mZoomLevel);
            mIsZoomStopped = false;
            mLastZoomLevel = mZoomLevel;
        }
        if (mIsUserInteraction) {
            mZoomUpdateListener.onZoomLevelUpdate(getZoomRatio());
        }
        LogHelper.d(TAG, "[configCommand] mZoomLevel = " + mZoomLevel
                + ", mDistanceRatio = " + mDistanceRatio + ", cameraProxy = " + cameraProxy);
    }

    @Override
    public void sendSettingChangeRequest() {
        if (isZoomValid()) {
            if (mIsZoomSupported) {
                LogHelper.d(TAG, "[sendSettingChangeRequest]");
                mSettingDeviceRequester.requestChangeSettingValue(IZoomConfig.KEY_CAMERA_ZOOM);
            } else if (mIsSmoothZoomSupported) {
                mSettingDeviceRequester.requestChangeCommand(IZoomConfig.KEY_CAMERA_ZOOM);
            }

        }
    }

    private boolean isZoomValid() {
        boolean needZoom = mZoomLevel >= 0 && mZoomLevel <= mMaxZoom
                && calculateZoomLevel(mDistanceRatio) != mLastZoomLevel && mIsZoomStopped;
        return needZoom;
    }

    private void reset(Camera.Parameters parameters) {
        LogHelper.i(TAG, "[reset]");
        parameters.setZoom(RESET_VALUE);
        mLastZoomLevel = RESET_VALUE;
    }

    private final Camera.OnZoomChangeListener mZoomListener = new Camera.OnZoomChangeListener() {
        @Override
        public void onZoomChange(int zoomValue, boolean stopped, Camera arg2) {
            LogHelper.d(TAG, "[onZoomChange] zoomValue = " + zoomValue + ", stopped = " + stopped);
            if (stopped) {
                synchronized (mSyncSmoothState) {
                    mIsZoomStopped = true;
                    mCameraProxy.stopSmoothZoom();
                }
            }
        }
    };

    private void calculateBasicRatio() {
        if (mZoomRatios != null) {
            if (mLastZoomLevel == -1) {
                mBasicZoomRatio = mZoomRatios.get(RESET_VALUE);
            } else {
                mBasicZoomRatio = mZoomRatios.get(mLastZoomLevel);
            }
        }
        LogHelper.d(TAG, "[calculateBasicRatio] mBasicZoomRatio = " + mBasicZoomRatio
                + ", mLastZoomLevel = " + mLastZoomLevel);
    }

    private int calculateZoomLevel(double distanceRatio) {
        int find = 0; // if not find, return 0
        if (mZoomRatios != null) {
            int len = mZoomRatios.size();
            int maxRatio = mZoomRatios.get(len - 1);
            int minRatio = mZoomRatios.get(0);
            if (len == 1) {
                find = 0;
            }
            int curRatio = (int) (mBasicZoomRatio + (maxRatio - minRatio) * distanceRatio);
            if (curRatio <= minRatio) {
                find = 0;
            } else if (curRatio >= maxRatio) {
                find = len - 1;
            } else {
                for (int i = 0; i < len - 1; i++) {
                    int cur = mZoomRatios.get(i);
                    int next = mZoomRatios.get(i + 1);
                    if (curRatio >= cur && curRatio < next) {
                        find = i;
                        break;
                    }
                }
            }
        }
        return find;
    }

    private String getZoomRatio() {
        float zoomValue = 1;
        if (mZoomRatios != null) {
            zoomValue = ((float) mZoomRatios.get(mLastZoomLevel)) / 100;
        }
        return "x" + String.format(Locale.ENGLISH, PATTERN, zoomValue);
    }
}
