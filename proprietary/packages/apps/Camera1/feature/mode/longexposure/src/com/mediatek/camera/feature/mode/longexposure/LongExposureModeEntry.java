/*
 *
 *  *   Copyright Statement:
 *  *
 *  *     This software/firmware and related documentation ("MediaTek Software") are
 *  *     protected under relevant copyright laws. The information contained herein is
 *  *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *  *     the prior written permission of MediaTek inc. and/or its licensors, any
 *  *     reproduction, modification, use or disclosure of MediaTek Software, and
 *  *     information contained herein, in whole or in part, shall be strictly
 *  *     prohibited.
 *  *
 *  *     MediaTek Inc. (C) 2017. All rights reserved.
 *  *
 *  *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *  *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *  *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *  *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *  *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *  *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *  *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *  *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *  *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *  *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *  *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *  *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *  *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *  *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *  *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *  *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *  *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *  *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *  *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *  *
 *  *     The following software/firmware and/or related documentation ("MediaTek
 *  *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *  *     any receiver's applicable license agreements with MediaTek Inc.
 *
 */

package com.mediatek.camera.feature.mode.longexposure;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.hardware.Camera;
import android.hardware.camera2.CameraCharacteristics;
import android.util.Range;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi.ModeItem;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.loader.FeatureEntryBase;
import com.mediatek.camera.common.mode.ICameraMode;

import java.util.concurrent.ConcurrentHashMap;

import javax.annotation.Nonnull;

/**
 * Long exposure mode entry.
 */

public class LongExposureModeEntry extends FeatureEntryBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(LongExposureModeEntry.class
            .getSimpleName());
    private String mCameraId = "0";
    private static final long MIN_EXPOSURE_TIME = 1000000000L; // 1s
    private static final int MODE_ITEM_PRIORITY = 35;
    private static final String KEY_EXPOSURE_TIME_MAX = "max-exposure-time"; //ms
    private static final String KEY_MANUAL_CAP_VALUES = "manual-cap-values";
    private static final int MS_TO_S = 1000;
    private static final long MIN_SHUTTER_SPEED_S = 1L; // 1s


    /**
     * create an entry.
     *
     * @param context   current activity.
     * @param resources current resources.
     */
    public LongExposureModeEntry(Context context, Resources resources) {
        super(context, resources);
    }

    @Override
    public void notifyBeforeOpenCamera(@Nonnull String cameraId, @Nonnull CameraApi cameraApi) {
        super.notifyBeforeOpenCamera(cameraId, cameraApi);
        mCameraId = cameraId;
        LogHelper.d(TAG, "[notifyBeforeOpenCamera] mCameraId = " + mCameraId);
    }

    @Override
    public boolean isSupport(CameraApi currentCameraApi, Activity activity) {
        switch (currentCameraApi) {
            case API1:
                Camera.Parameters parameters
                        = mDeviceSpec.getDeviceDescriptionMap().get(mCameraId).getParameters();
                return !isThirdPartyIntent(activity)
                        && isSupportInAPI1(parameters);
            case API2:
                return !isThirdPartyIntent(activity) && isSupportInAPI2(mCameraId);
            default:
                return false;
        }
    }

    /**
     * Get the advance feature name.
     *
     * @return Feature name.
     */
    @Override
    public String getFeatureEntryName() {
        return LongExposureModeEntry.class.getName();
    }

    /**
     * Get feature name, the type is define as a Class.
     * For example,
     * if the feature is a mode ,it will implement the ICameraMode, so the
     * feature type will  be the ICameraMode.class.
     * if it is a setting, the type will be the ICameraSetting.class.
     *
     * @return Feature type.
     */
    @Override
    public Class getType() {
        return ICameraMode.class;
    }

    /**
     * Create the feature instance.
     *
     * @return Return The ICameraMode or ICameraSetting implementer instance.
     */
    @Override
    public Object createInstance() {
        return new LongExposureMode();
    }

    /**
     * Get mode item.
     *
     * @return the mode item info.
     */
    @Override
    public ModeItem getModeItem() {
        ModeItem modeItem = new ModeItem();
        modeItem.mType = "Picture";
        modeItem.mModeUnselectedIcon = mContext.getResources().getDrawable(R.drawable
                .ic_exposure_mode_unselected);
        modeItem.mModeSelectedIcon = mContext.getResources().getDrawable(R.drawable
                .ic_exposure_mode_selected);
        modeItem.mPriority = MODE_ITEM_PRIORITY;
        modeItem.mClassName = getFeatureEntryName();
        modeItem.mModeName = String.valueOf(mResources.getString(R.string
                .long_exposure_motion_title));
        // Note:Only back camera supported long exposure currently
        modeItem.mSupportedCameraIds = new String[]{"0"};
        return modeItem;
    }

    private boolean isSupportInAPI2(String cameraId) {
        ConcurrentHashMap<String, DeviceDescription>
                deviceDescriptionMap = mDeviceSpec.getDeviceDescriptionMap();
        if (cameraId == null || deviceDescriptionMap == null || deviceDescriptionMap.size() <= 0) {
            LogHelper.w(TAG, "[isSupportInAPI2] cameraId = " + cameraId + ",deviceDescriptionMap " +
                    "" + deviceDescriptionMap);
            return false;
        }
        if (!deviceDescriptionMap.containsKey(cameraId)) {
            LogHelper.w(TAG, "[isSupportInAPI2] cameraId " + cameraId + " does not in device map");
            return false;
        }
        CameraCharacteristics characteristics = deviceDescriptionMap.get(cameraId)
                .getCameraCharacteristics();
        if (characteristics == null) {
            LogHelper.d(TAG, "[isSupportInAPI2] characteristics is null");
            return false;
        }
        return isLongExposureSupported(characteristics);
    }

    private boolean isLongExposureSupported(CameraCharacteristics characteristics) {
        if (characteristics == null) {
            LogHelper.w(TAG, "characteristics is null");
            return false;
        }
        Range<Long> exposureTimeRange =
                characteristics.get(CameraCharacteristics.SENSOR_INFO_EXPOSURE_TIME_RANGE);
        LogHelper.d(TAG, "");
        return exposureTimeRange != null && exposureTimeRange.getUpper() >= MIN_EXPOSURE_TIME;
    }

    private boolean isSupportInAPI1(Camera.Parameters parameters) {
        if (parameters == null) {
            LogHelper.i(TAG, "[isSupportInAPI1] parameters is null!");
            return false;
        }
        return isLongExposureSupported(parameters);
    }

    private boolean isLongExposureSupported(Camera.Parameters originalParameters) {
        if (originalParameters == null) {
            LogHelper.w(TAG, "[isLongExposureSupported] originalParameters is null");
            return false;
        }
        boolean isManualCapSupported = originalParameters.get(KEY_MANUAL_CAP_VALUES) != null &&
                originalParameters.get(KEY_MANUAL_CAP_VALUES).contains("on");
        if (!isManualCapSupported) {
            LogHelper.w(TAG, "[isLongExposureSupported] isManualCapSupported is false");
            return false;
        }
        String maxExposureTime = originalParameters.get(KEY_EXPOSURE_TIME_MAX);
        if (maxExposureTime == null) {
            LogHelper.w(TAG, "[isLongExposureSupported] maxExposureTime is null");
            return false;
        }
        LogHelper.w(TAG, "[isLongExposureSupported] maxExposureTime = " + maxExposureTime);
        return maxExposureTime != null
                && (Integer.parseInt(maxExposureTime) / MS_TO_S) >= MIN_SHUTTER_SPEED_S;
    }
}
