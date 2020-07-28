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

package com.mediatek.camera.feature.setting.dualcamerazoom;

import android.hardware.Camera;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.List;

/**
 * zoom perform implementer for api1.
 * Support two ways to perform zoom, one is parameter perform, the other is
 * smooth zoom.
 */

@SuppressWarnings("deprecation")
public class DualZoomParameterConfig implements ICameraSetting.IParametersConfigure,
        IDualZoomConfig {
    private static final Tag TAG = new Tag(DualZoomParameterConfig.class.getSimpleName());
    private static final String KEY_DUALCAM_SWITCH_ZOOM_RATIO = "dualcam-switch-zoom-ratio";
    private static final int ZOOM_LEVEL_DEFAULT = 0;
    //Zoom ratios list device supported. Such as 100/115/...400.
    private List<Integer> mZoomRatios;
    private int mZoomLevel;
    private double mDistanceRatio;
    private boolean mIsZoomSupported;
    private int mLastZoomLevel = ZOOM_LEVEL_DEFAULT;
    private int mBasicZoomRatio = RATIO_VALUE;
    private int mMaxZoom;
    private OnZoomLevelUpdateListener mZoomUpdateListener;
    private boolean mIsUserInteraction = true;
    private ISettingManager.SettingDeviceRequester mSettingDeviceRequester;
    private boolean mIsInit = false;
    private boolean mIsSwitch = false;
    private boolean mIsPinch = false;
    private int mSwitchRatio = 200;

    /**
     * Constructor of zoom parameter config in api1.
     *
     * @param settingDeviceRequester device requester.
     */
    public DualZoomParameterConfig(ISettingManager.SettingDeviceRequester settingDeviceRequester) {
        mSettingDeviceRequester = settingDeviceRequester;
    }

    @Override
    public void onScalePerformed(double distanceRatio) {
        mDistanceRatio = distanceRatio;
        calculateZoomLevel(distanceRatio);
    }

    @Override
    public boolean onScaleStatus(boolean isSwitch, boolean isInit) {
        LogHelper.d(TAG, "[onScaleStatus]  isSwitch = "
                + isSwitch + ", isInit = " + isInit);
        mIsUserInteraction = true;
        mDistanceRatio = 0;
        mIsInit = isInit;
        mIsSwitch = isSwitch;
        mBasicZoomRatio = mZoomRatios.get(mLastZoomLevel);
        return false;
    }

    @Override
    public void onScaleType(boolean isPinch) {
        mIsPinch = isPinch;
    }

    @Override
    public void onScaleTypeName(String typeName) {
    }

    @Override
    public void setZoomUpdateListener(OnZoomLevelUpdateListener zoomUpdateListener) {
        mZoomUpdateListener = zoomUpdateListener;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters parameters) {
        if (parameters.get(KEY_DUALCAM_SWITCH_ZOOM_RATIO) != null) {
            mSwitchRatio = Integer.parseInt(parameters.get(KEY_DUALCAM_SWITCH_ZOOM_RATIO));
            LogHelper.d(TAG, "[setOriginalParameters] mSwitchRatio = " + mSwitchRatio);
            mZoomUpdateListener.updateSwitchRatioSupported(mSwitchRatio);
        }
        mZoomRatios = parameters.getZoomRatios();
        mIsZoomSupported = parameters.isZoomSupported();
        mMaxZoom = parameters.getMaxZoom();
        mZoomUpdateListener.updateRatiosSupported(mZoomRatios);
        if (mZoomUpdateListener.isZoomTele()) {
            updateAfterInit(false, true, false);
        } else {
            updateAfterInit(true, false, true);
        }
        LogHelper.d(TAG, "[setOriginalParameters] mIsZoomSupported = " + mIsZoomSupported);
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        if (ZOOM_OFF.equals(mZoomUpdateListener.onGetOverrideValue())) {
            reset(parameters);
            return false;
        }
        parameters.setZoom(mZoomLevel);
        if (mZoomUpdateListener.isSingleMode()) {
            parameters.setZoom(0);
        }
        mLastZoomLevel = mZoomLevel;
        if (mIsUserInteraction && ZOOM_ON.equals(mZoomUpdateListener.onGetOverrideValue())) {
            mZoomUpdateListener.onZoomLevelUpdate(mZoomLevel);
        }
        LogHelper.d(TAG, "[configParameters] mZoomLevel = " + mZoomLevel
                + ", mDistanceRatio = " + mDistanceRatio);
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    @Override
    public void sendSettingChangeRequest() {
        if (isZoomValid()) {
            if (mIsZoomSupported) {
                mSettingDeviceRequester.requestChangeSettingValue(IDualZoomConfig.KEY_DUAL_ZOOM);
            }
        }
    }

    private boolean isZoomValid() {
        return mZoomLevel >= 0 && mZoomLevel <= mMaxZoom
                && mZoomLevel != mLastZoomLevel;
    }

    private void reset(Camera.Parameters parameters) {
        LogHelper.i(TAG, "[reset]");
        parameters.setZoom(ZOOM_LEVEL_DEFAULT);
        mLastZoomLevel = ZOOM_LEVEL_DEFAULT;
    }

    private void calculateZoomLevel(double distanceRatio) {
        if (mZoomRatios != null) {
            int len = mZoomRatios.size();
            int maxRatio = mZoomRatios.get(len - 1);
            int minRatio = mZoomRatios.get(0);
            if (!mIsPinch) {
                if (distanceRatio > 0) {
                    mZoomLevel = (int) Math.ceil((len - 1) * distanceRatio);
                } else {
                    mZoomLevel = (int) Math.floor((len - 1) * Math.abs(distanceRatio));
                }
            } else {
                int curRatio = (int) (mBasicZoomRatio
                        + (maxRatio - minRatio) * distanceRatio);
                if (curRatio <= minRatio) {
                    mZoomLevel = 0;
                } else if (curRatio >= maxRatio) {
                    mZoomLevel = len - 1;
                } else {
                    for (int i = 0; i < len - 1; i++) {
                        int cur = mZoomRatios.get(i);
                        int next = mZoomRatios.get(i + 1);
                        if (distanceRatio < 0 && curRatio > cur && curRatio <= next) {
                            mZoomLevel = i;
                            break;
                        }
                        if (distanceRatio > 0 && curRatio >= cur && curRatio < next) {
                            mZoomLevel = i + 1;
                            break;
                        }
                    }
                }
            }
            if (mIsSwitch) {
                if (mLastZoomLevel == ZOOM_LEVEL_DEFAULT) {
                    if (mZoomRatios.contains(mSwitchRatio)) {
                        mZoomLevel = findZoomLevel(mSwitchRatio);
                    }
                } else {
                    mZoomLevel = ZOOM_LEVEL_DEFAULT;
                }
            }
            if (mIsInit) {
                mZoomLevel = ZOOM_LEVEL_DEFAULT;
            }
            if (mZoomLevel > len - 1) {
                mZoomLevel = len - 1;
            } else if (mZoomLevel < ZOOM_LEVEL_DEFAULT) {
                mZoomLevel = ZOOM_LEVEL_DEFAULT;
            }
        }
        LogHelper.d(TAG, "mZoomLevel = " + mZoomLevel + ", lastLevel = "
                + mLastZoomLevel + ", distanceRatio = " + distanceRatio);
    }

    private int findZoomLevel(int ratio) {
        int len = mZoomRatios.size();
        for (int i = 0; i < len - 1; i++) {
            int cur = mZoomRatios.get(i);
            if (ratio == cur) {
                return i;
            }
        }
        return 0;
    }

    private void updateAfterInit(boolean isUser, boolean isSwitch, boolean isInit) {
        mIsUserInteraction = isUser;
        mDistanceRatio = 0;
        mIsInit = isInit;
        mIsSwitch = isSwitch;
        calculateZoomLevel(mDistanceRatio);
    }
}
