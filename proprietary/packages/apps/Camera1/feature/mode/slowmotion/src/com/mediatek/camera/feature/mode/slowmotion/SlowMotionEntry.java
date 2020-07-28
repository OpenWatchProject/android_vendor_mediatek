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

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.CamcorderProfile;
import android.os.Build;
import android.util.Range;
import android.util.Size;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.loader.FeatureEntryBase;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.portability.CamcorderProfileEx;
import com.mediatek.camera.portability.SystemProperties;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

/**
 * slow motion mode entry.
 */
public class SlowMotionEntry extends FeatureEntryBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SlowMotionEntry.class.getSimpleName());

    private static final int SLOW_MOTION_QUALITY_HIGH = 2222;
    private static final int SLOW_MOTION_QUALITY_LOW = 2220;
    private static final String MODE_ITEM_TYPE = "Video";
    private static final int MODE_ITEM_PRIORITY = 70;

    private static final int[] sMtkSlowQualities = new int[] {
            SLOW_MOTION_QUALITY_LOW,
            SLOW_MOTION_QUALITY_HIGH
    };

    private static final int[] sSlowQualities = new int[] {
            CamcorderProfile.QUALITY_HIGH_SPEED_480P,
            CamcorderProfile.QUALITY_HIGH_SPEED_720P,
            CamcorderProfile.QUALITY_HIGH_SPEED_1080P,
            CamcorderProfile.QUALITY_HIGH_SPEED_2160P
    };

    private List<String> mSupportedIdList = new ArrayList<>();
    private String [] mStringSupportedIds;

    /**
     * create an entry.
     *
     * @param context   current activity.
     * @param resources current resources.
     */
    public SlowMotionEntry(Context context, Resources resources) {
        super(context, resources);
    }


    @Override
    public boolean isSupport(CameraApi currentCameraApi, Activity activity) {
        return isSlowMotionSupported(activity);
    }

    @Override
    public String getFeatureEntryName() {
        return SlowMotionEntry.class.getName();
    }

    @Override
    public Class getType() {
        return ICameraMode.class;
    }

    @Override
    public Object createInstance() {
        return new SlowMotionMode();
    }

    @Override
    public IAppUi.ModeItem getModeItem() {
        IAppUi.ModeItem modeItem = new IAppUi.ModeItem();
        modeItem.mModeUnselectedIcon = mResources.getDrawable(R.drawable.ic_slow_motion_mode_off);
        modeItem.mModeSelectedIcon = mResources.getDrawable(R.drawable.ic_slow_motion_mode_on);
        modeItem.mShutterIcon = mResources.getDrawable(R.drawable.ic_slow_motion_shutter);
        modeItem.mType = MODE_ITEM_TYPE;
        modeItem.mPriority = MODE_ITEM_PRIORITY;
        modeItem.mClassName = getFeatureEntryName();
        modeItem.mModeName = String.valueOf(mResources.getString(R.string.slow_motion_title));
        modeItem.mSupportedCameraIds = mStringSupportedIds;
        return modeItem;
    }

    private boolean isSlowMotionSupported(Activity activity) {
        boolean isSupported =
            !isThirdPartyIntent(activity) && isFeatureOptionSupported() && isPlatFormSupported();
        LogHelper.i(TAG, "[isSlowMotionSupported] isSupported = " + isSupported);
        return isSupported;
    }

    private boolean isFeatureOptionSupported() {
        boolean enable =
                SystemProperties.getInt("ro.vendor.mtk_slow_motion_support", 0) == 1 ? true : false;
        LogHelper.d(TAG, "[isFeatureOptionSupported]  slow motion enable = " + enable);
        return enable;
    }
    private boolean isPlatFormSupported() {
        initPlatformSupportedState();
        boolean isSupported = false;
        if (mSupportedIdList != null && mSupportedIdList.size() >= 1) {
            mStringSupportedIds = new String[mSupportedIdList.size()];
            for (int i = 0; i < mSupportedIdList.size(); i++) {
                mStringSupportedIds[i] = mSupportedIdList.get(i);
                LogHelper.d(TAG, "supported slow motion id = " + mSupportedIdList.get(i));
            }
            isSupported = true;
        }
        mSupportedIdList.clear();
        LogHelper.d(TAG, "[isPlatFormSupported] isSupported = " + isSupported);
        return isSupported;
    }

    private void initPlatformSupportedState() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            LogHelper.e(TAG, "[initPlatformSupportedState] sdk version is smaller than 23");
            return;
        }
        ConcurrentHashMap<String, DeviceDescription> deviceDescriptionHashMap
                             = mDeviceSpec.getDeviceDescriptionMap();
        int cameraNum = deviceDescriptionHashMap.size();
        int[] availableCaps;
        List<Integer> capList;
        DeviceDescription deviceDescription;
        CameraCharacteristics cameraCharacteristics;
        for (int i = 0; i < cameraNum; i++) {
            deviceDescription = deviceDescriptionHashMap.get(String.valueOf(i));
            cameraCharacteristics  = deviceDescription.getCameraCharacteristics();
            if (cameraCharacteristics == null) {
                continue;
            }
            availableCaps = cameraCharacteristics.get(
                          CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES);
            capList = Arrays.asList(
                     convertPrimitiveArrayToObjectArray(availableCaps, Integer.class));
            boolean isContainHighSpeed = capList.contains(CameraCharacteristics
                    .REQUEST_AVAILABLE_CAPABILITIES_CONSTRAINED_HIGH_SPEED_VIDEO);
            LogHelper.d(TAG, "CAPABILITIES contain HIGH_SPEED_VIDEO = "
                                  + isContainHighSpeed + "  camera id = " + i);
            if (isContainHighSpeed) {
                if (checkerProfile(i, cameraCharacteristics) != null) {
                    mSupportedIdList.add(String.valueOf(i));
                }
            }
        }
    }


    /**
     * Convert a primitive input array into its object array version.
     * @param array Input array object
     * @param wrapperClass The boxed class it converts to
     * @return Boxed version of primitive array
     */
    private static <T> T[] convertPrimitiveArrayToObjectArray(final Object array,
                                                              final Class<T> wrapperClass) {
        // getLength does the null check and isArray check already.
        int arrayLength = Array.getLength(array);
        if (arrayLength == 0) {
            throw new IllegalArgumentException("Input array shouldn't be empty");
        }

        @SuppressWarnings("unchecked")
        final T[] result = (T[]) Array.newInstance(wrapperClass, arrayLength);
        for (int i = 0; i < arrayLength; i++) {
            Array.set(result, i, Array.get(array, i));
        }
        return result;
    }

    private CamcorderProfile checkerProfile(int cameraId,
                                            CameraCharacteristics cameraCharacteristics) {
        CamcorderProfile profile
                = findProfileForRange(cameraId, cameraCharacteristics, sMtkSlowQualities);
        if (profile == null) {
            profile = findProfileForRange(cameraId, cameraCharacteristics, sSlowQualities);
        }
        LogHelper.d(TAG, "[checkerProfile] cameraId =  " + cameraId + "profile = " + profile);
        return profile;
    }

    private Range<Integer> getHighSpeedFixedFpsRangeForSize(Size size,
                                          CameraCharacteristics cameraCharacteristics) {
        StreamConfigurationMap config =
                cameraCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        try {
            Range<Integer>[] availableFpsRanges = config.getHighSpeedVideoFpsRangesFor(size);
            for (Range<Integer> range : availableFpsRanges) {
                if (range.getLower().equals(range.getUpper())) {
                    LogHelper.d(TAG, "[getHighSpeedFpsRangeForSize] range = " + range.toString());
                    return range;
                }
            }
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
        return null;
    }

    private CamcorderProfile findProfileForRange(int cameraId,
                                                 CameraCharacteristics cameraCharacteristics,
                                                 int[] qualityMatrix) {
        CamcorderProfile profile = null;
        for (int i = 0; i < qualityMatrix.length; i++) {
            if (CamcorderProfile.hasProfile(cameraId, qualityMatrix[i])) {
                profile = CamcorderProfileEx.getProfile(cameraId, qualityMatrix[i]);
                Range<Integer> range = getHighSpeedFixedFpsRangeForSize(
                        new Size(profile.videoFrameWidth, profile.videoFrameHeight),
                        cameraCharacteristics);
                if (range != null && range.getLower() == profile.videoFrameRate) {
                    LogHelper.d(TAG, "find slow motion FrameRate is "
                                          + profile.videoFrameRate + "Camera id = " + cameraId);
                    return profile;
                }
            }
        }
        return profile;
    }
}
