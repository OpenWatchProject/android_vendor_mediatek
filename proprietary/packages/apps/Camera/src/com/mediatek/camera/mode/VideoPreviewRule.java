/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.mode;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Point;
import android.hardware.Camera.Size;
import android.media.CamcorderProfile;
import android.provider.MediaStore;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ICameraMode.CameraModeType;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

public class VideoPreviewRule implements ISettingRule {
    private String TAG = "VideoPreviewRule";

    private Activity mActivity;
    private ISettingCtrl mISettingCtrl;
    private ICameraDeviceManager mICameraDeviceManager;
    private ICameraAppUi mICameraAppUI;
    private IModuleCtrl mIMoudleCtrl;
    private ICameraContext mICameraContext;
    private ICameraDevice mICameraDevice;
    private ICameraDevice mTopICameraDevice;
    private Parameters mParameters;
    private Parameters mTopParameters;
    private CamcorderProfile mProfile;
    private Point previewSize;
    private CameraModeType mMode;
    private String mConditionSettingKey = SettingConstants.KEY_VIDEO;

    private List<String> mConditions = new ArrayList<String>();
    private List<List<String>> mResults = new ArrayList<List<String>>();
    private List<MappingFinder> mMappingFinder = new ArrayList<MappingFinder>();

    private String              mCurrentVideoQuality;
    private boolean             mSwitchingPip = false;
    private boolean             mHasOverride = false;

    public VideoPreviewRule(ICameraContext cameraContext, CameraModeType mode) {
        Log.i(TAG, "[VideoPreviewRule]constructor...");
        mActivity = cameraContext.getActivity();
        mISettingCtrl = cameraContext.getSettingController();
        mICameraDeviceManager = cameraContext.getCameraDeviceManager();
        mICameraAppUI = cameraContext.getCameraAppUi();
        mIMoudleCtrl = cameraContext.getModuleController();
        mMode = mode;
        mICameraContext = cameraContext;
        if (mMode == CameraModeType.EXT_MODE_VIDEO_PIP) {
            mConditionSettingKey = SettingConstants.KEY_VIDEO_PIP;
        } else if (mMode == CameraModeType.EXT_MODE_VIDEO_STEREO) {
            mConditionSettingKey = SettingConstants.KEY_VIDEO_STEREO;
        }
    }

    @Override
    public void execute() {
        String value = mISettingCtrl.getSettingValue(mConditionSettingKey);
        getCameraDevice();
        int index = conditionSatisfied(value);
        if (MTK_CHIP_0321.equals(mICameraContext.getFeatureConfig().whichDeanliChip())) {
            pipDenaliZSDRule(index);
        }
        // index = -1 means leave video mode and restore preview size
        Log.i(TAG, "execute index = " + index);
        if (index == -1) {
            if (!mHasOverride) {
                return;
            }
            mHasOverride = false;
            if (!mIMoudleCtrl.isNonePickIntent()) {
                return;
            }
            String ratio = mISettingCtrl.getSettingValue(SettingConstants.KEY_PICTURE_RATIO);
            mParameters = mICameraDevice.getParameters();
            if (mParameters == null) {
                return;
            }
            SettingUtils.setPreviewSize(mActivity, mParameters, ratio);

            String pictureSize = mISettingCtrl.getSettingValue(SettingConstants.KEY_PICTURE_SIZE);
            int i = pictureSize.indexOf('x');
            int width = Integer.parseInt(pictureSize.substring(0, i));
            int height = Integer.parseInt(pictureSize.substring(i + 1));
            mParameters.setPictureSize(width, height);
        } else {
            setVideoPreviewSize();
            mHasOverride = true;
        }

    }

    @Override
    public void addLimitation(String condition, List<String> result, MappingFinder mappingFinder) {
        mConditions.add(condition);
        mResults.add(result);
        mMappingFinder.add(mappingFinder);
    }

    public void updateProfile() {
        int profileCameraId = mICameraDeviceManager.getCurrentCameraId();
        fetchProfile(getQuality(), profileCameraId);
    }

    public CamcorderProfile getProfile() {
        return mProfile;
    }

    public String getConditionKey() {
        return mConditionSettingKey;
    }

    // set video preview size ,video preview frame rate and picture size
    // reviseVideoCapability will set right videoBitRate and videoFrameRate for
    // media recorder
    private void setVideoPreviewSize() {
        int profileCameraId = mICameraDeviceManager.getCurrentCameraId();
        // pip video always use back camera's profile
        if (mMode == CameraModeType.EXT_MODE_VIDEO_PIP) {
            profileCameraId = mICameraDeviceManager.getBackCameraId();
        }
        fetchProfile(getQuality(), profileCameraId);
        mParameters = mICameraDevice.getParameters();
        setPreviewSize(mProfile, mParameters);
        reviseVideoCapability(mParameters, mProfile);
        setPreviewFrameRate(mParameters, mProfile.videoFrameRate);
        setPictureSize(mProfile, mParameters);
        //pip is not video mode for camera native so
        //don't set video sizeo
        if (mMode != CameraModeType.EXT_MODE_VIDEO_PIP) {
            setVideoSize(mProfile, mParameters);
        }
        // pip mode should apply top camera Parameter , back camera or other
        // mode will apply parameter auto
        if (mMode == CameraModeType.EXT_MODE_VIDEO_PIP) {
            mTopICameraDevice.applyParameters();
        }
    }

    private int getQuality() {
        String settingQuality;
        int quality;
        if ("on".equals(mISettingCtrl.getSettingValue(SettingConstants.KEY_SLOW_MOTION))
                && !mSwitchingPip) {
            settingQuality = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_SLOW_MOTION_VIDEO_QUALITY);
        } else if (mSwitchingPip) {
            settingQuality = mCurrentVideoQuality;
            mISettingCtrl.setSettingValue(SettingConstants.KEY_VIDEO_QUALITY, settingQuality,
                    mICameraDeviceManager.getCurrentCameraId());
        } else if ("on".equals(mISettingCtrl.getSettingValue(SettingConstants.KEY_VIDEO_STEREO))) {
            settingQuality = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_REFOCUS_VIDEO_QUALITY);
        } else {
            settingQuality = mISettingCtrl.getSettingValue(SettingConstants.KEY_VIDEO_QUALITY);
        }
            mCurrentVideoQuality = settingQuality;
            // Set video quality according client app's requirement.
            quality = Integer.valueOf(settingQuality);
            Intent intent = mIMoudleCtrl.getIntent();
            boolean userLimitQuality = intent.hasExtra(MediaStore.EXTRA_VIDEO_QUALITY);
            if (userLimitQuality) {
                int extraVideoQuality = intent.getIntExtra(MediaStore.EXTRA_VIDEO_QUALITY, 0);
                if (extraVideoQuality > 0) {
                    if (CamcorderProfile.hasProfile(mICameraDeviceManager.getCurrentCameraId(),
                            extraVideoQuality)) {
                        quality = extraVideoQuality;
                    } else {
                        if (CamcorderProfile.hasProfile(mICameraDeviceManager.getCurrentCameraId(),
                                CamcorderProfile.QUALITY_720P)) {
                            quality = CamcorderProfile.QUALITY_720P;
                        } else {
                            quality = CamcorderProfile.QUALITY_LOW;
                        }
                    }
                } else {
                    quality = CamcorderProfile.QUALITY_LOW;
                }
            }
        Log.i(TAG, "[getQuality] quality = " + quality);
        return quality;
    }

    private CamcorderProfile fetchProfile(int quality, int cameraId) {
        Log.i(TAG, "[fetchProfile](" + quality + ", " + " cameraId = " + cameraId + ")");
        mProfile = CamcorderProfile.get(cameraId, quality); // TODO
        if (mProfile != null) {
            Log.i(TAG, "[fetchProfile()] mProfile.videoFrameRate=" + mProfile.videoFrameRate
                    + ", mProfile.videoFrameWidth=" + mProfile.videoFrameWidth
                    + ", mProfile.videoFrameHeight=" + mProfile.videoFrameHeight
                    + ", mProfile.audioBitRate=" + mProfile.audioBitRate
                    + ", mProfile.videoBitRate=" + mProfile.videoBitRate + ", mProfile.quality="
                    + mProfile.quality + ", mProfile.duration=" + mProfile.duration);
        }
        mICameraAppUI.setCamcorderProfile(mProfile);
        return mProfile;
    }

    private void setPictureSize(CamcorderProfile profile, Parameters parameters) {
        if (mMode == CameraModeType.EXT_MODE_VIDEO_PIP) {
            parameters.setPictureSize(profile.videoFrameWidth, profile.videoFrameHeight);
            updateTopParameters();
            setTopCameraPictureSize(mTopParameters, parameters.getPictureSize());
            Log.i(TAG, "[setPictureSize] width " + profile.videoFrameWidth + " * height "
                    + profile.videoFrameHeight);
        } else {

            if (parameters.isVideoSnapshotSupported()) {
                List<Size> supported = parameters.getSupportedPictureSizes();
                Size optimalSize = Util.getOptimalVideoSnapshotPictureSize(supported,
                        (double) previewSize.x / previewSize.y);
                Size original = parameters.getPictureSize();
                if (optimalSize != null) {
                    if (!original.equals(optimalSize)) {
                        parameters.setPictureSize(optimalSize.width, optimalSize.height);
                    }
                } else {
                    Log.i(TAG, "error optimalSize is null");
                }
            } else {
                parameters.setPictureSize(previewSize.x, previewSize.y);
                Log.i(TAG, "[[setPictureSize] width " + previewSize.x + " *  height ="
                        + previewSize.y);
            }
        }

    }

    private void setVideoSize(CamcorderProfile profile, Parameters parameters) {
        if (profile != null && parameters != null) {
            String videoSize = Integer.toString(mProfile.videoFrameWidth) +
                    "x" + Integer.toString(mProfile.videoFrameHeight);
            Log.i(TAG, "video-size = " + videoSize);
            parameters.set("video-size", videoSize);
        }
    }

    private void setTopCameraPictureSize(Parameters topParameters, Size targetPictureSize) {
        Log.i(TAG, "[setTopCameraPictureSize] targetPictureSize width = " + targetPictureSize.width
                + " height = " + targetPictureSize.height);
        if (topParameters != null) {
            Size miniPictureSize = getMininalPIPTopSize(topParameters.getSupportedPictureSizes(),
                    (double) targetPictureSize.width / targetPictureSize.height);
            if (miniPictureSize == null) {
                miniPictureSize = targetPictureSize;
            }
            topParameters.setPictureSize(miniPictureSize.width, miniPictureSize.height);
            Log.i(TAG, "[setTopCameraPictureSize] miniPictureSize width = " + miniPictureSize.width
                    + " height = " + miniPictureSize.height);
        }
    }

    private void setPreviewSize(CamcorderProfile profile, Parameters parameters) {
        if (mMode != CameraModeType.EXT_MODE_VIDEO_PIP) {
            previewSize = computeDesiredPreviewSize(profile, parameters);
            parameters.setPreviewSize(previewSize.x, previewSize.y);
            Log.i(TAG, "[setPreviewSize] width " + previewSize.x + " *  height =" + previewSize.y);
        } else {
            parameters.setPreviewSize(profile.videoFrameWidth, profile.videoFrameHeight);
            updateTopParameters();
            if (mTopParameters != null) {
                mTopParameters.setPreviewSize(profile.videoFrameWidth, profile.videoFrameHeight);
                Log.i(TAG, "[setPreviewSize] width " + profile.videoFrameWidth + " *  height "
                        + profile.videoFrameHeight);
            }
        }
    }

    private void getCameraDevice() {
        if (mICameraDeviceManager != null) {
            int camerId = mICameraDeviceManager.getCurrentCameraId();
            mICameraDevice = mICameraDeviceManager.getCameraDevice(camerId);
            if (mTopICameraDevice != null) {
                mSwitchingPip = (mTopICameraDevice.getCameraId() == mICameraDeviceManager
                        .getCurrentCameraId());
            } else {
                mSwitchingPip = false;
            }
            updateTopParameters();
        }

    }

    private void updateTopParameters() {
        if (mMode == CameraModeType.EXT_MODE_VIDEO_PIP) {
            mTopICameraDevice = mICameraDeviceManager.getCameraDevice(getTopCameraId());
            if (mTopICameraDevice != null) {
                mTopParameters = mTopICameraDevice.getParameters();
            }
        }
    }

    private int getTopCameraId() {
        return mICameraDeviceManager.getCurrentCameraId() == mICameraDeviceManager
                .getBackCameraId() ? mICameraDeviceManager.getFrontCameraId()
                : mICameraDeviceManager.getBackCameraId();

    }

    private int conditionSatisfied(String conditionValue) {
        int index = mConditions.indexOf(conditionValue);
        Log.i(TAG, "[conditionSatisfied]limitation index:" + index + " conditionValue = "
                + conditionValue);
        return index;
    }

    // Try to find a size matches aspect ratio and has the smallest size(preview
    // size & picture size)
    private Size getMininalPIPTopSize(List<Size> sizes, double targetRatio) {
        if (sizes == null || targetRatio < 0) {
            Log.i(TAG, "[getMininalPIPTopSize] error sizes = " + sizes + " targetRatio = "
                    + targetRatio);
            return null;
        }
        Size optimalSize = null;
        for (Size size : sizes) {
            double ratio = (double) size.width / size.height;
            Log.i(TAG, "[getMininalPIPTopSize] (" + size.width + " ," + size.height + " )");
            if (Math.abs(ratio - targetRatio) > 0.02)
                continue;
            if (optimalSize == null || size.width < optimalSize.width) {
 optimalSize = size;
            }
        }
        return optimalSize;
    }

    private Point computeDesiredPreviewSize(CamcorderProfile profile, Parameters parameters) {
        int previewWidth = -1;
        int previewHeight = -1;
        if (parameters.getSupportedVideoSizes() == null) { // should be rechecked
                                                          // usedefault
            previewWidth = profile.videoFrameWidth;
            previewHeight = profile.videoFrameHeight;
        } else { // Driver supports separates outputs for preview and video.
            List<Size> sizes = parameters.getSupportedPreviewSizes();
            int product = 0;
            String slowMotionValue = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_SLOW_MOTION);
            if ("on".equals(slowMotionValue)) {
                product = profile.videoFrameWidth * profile.videoFrameHeight;
            } else {
                Size preferred = parameters.getPreferredPreviewSizeForVideo();
                product = preferred.width * preferred.height;
            }
            Iterator<Size> it = sizes.iterator();
            // Remove the preview sizes that are not preferred.
            while (it.hasNext()) {
                Size size = it.next();
                if (size.width * size.height > product) {
                    it.remove();
                }
            }
            // don't change target ratio
            Size optimalSize = Util.getOptimalPreviewSize(mActivity, sizes,
                    (double) profile.videoFrameWidth / profile.videoFrameHeight, true, false);
            if (optimalSize != null) {
                previewWidth = optimalSize.width;
                previewHeight = optimalSize.height;
            } else {
                previewWidth = profile.videoFrameWidth;
                previewHeight = profile.videoFrameHeight;
            }
        }
        Point desired = new Point(previewWidth, previewHeight);
        return desired;
    }

    private void reviseVideoCapability(Parameters parameters, CamcorderProfile profile) {
        Log.d(TAG, "[reviseVideoCapability()] begin with profile.videoFrameRate = "
                + profile.videoFrameRate);
        // set right videoBitRate and videoFrameRate for media recorder
        // Mediatek modify to meet lower frame sensor
        List<Integer> supportedFrameRates = parameters.getSupportedPreviewFrameRates();
        if (!isSupported(profile.videoFrameRate, supportedFrameRates)) {
            int maxFrame = getMaxSupportedPreviewFrameRate(supportedFrameRates);
            profile.videoBitRate = (profile.videoBitRate / profile.videoFrameRate) * maxFrame;
            profile.videoFrameRate = maxFrame;
        }
        // night mode should decrease the frame rate
        String sceneMode = mISettingCtrl.getSettingValue(SettingConstants.KEY_SCENE_MODE);
        if (Parameters.SCENE_MODE_NIGHT.equals(sceneMode)) {
            profile.videoFrameRate /= 2;
            profile.videoBitRate /= 2;
        }
        Log.d(TAG, "[reviseVideoCapability()] end with profile.videoFrameRate = "
                + profile.videoFrameRate);
    }

    private void setPreviewFrameRate(Parameters parameters, int supportedFramerate) {
        List<Integer> frameRates = null;
        List<Integer> topFrameRates = null;
        List<Integer> pipFrameRates = null;
        List<Integer> pipTopFrameRates = null;
        updateTopParameters();
        if (supportedFramerate > 0) {
            frameRates = new ArrayList<Integer>();
            frameRates.add(supportedFramerate);
        }
        if (mMode == CameraModeType.EXT_MODE_VIDEO_PIP) {
            pipFrameRates = mICameraDevice.getPIPFrameRateZSDOff();
            pipTopFrameRates = mTopICameraDevice.getPIPFrameRateZSDOff();
            Log.i(TAG, "[setPreviewFrameRate getPIPFrameRateZSDOff] pipFrameRates = "
                    + pipFrameRates + " pipTopFrameRates = " + pipTopFrameRates);
            frameRates = (pipFrameRates != null) ? pipFrameRates : frameRates;
            topFrameRates = (pipTopFrameRates != null) ? pipTopFrameRates : topFrameRates;
            // close dynamic frame rate, if dynamic frame rate is supported
            closeDynamicFrameRate(mICameraDevice);
            closeDynamicFrameRate(mTopICameraDevice);
        }
        if (frameRates == null) {
            // Reset preview frame rate to the maximum because it may be lowered
            // by
            // video camera application.
            frameRates = parameters.getSupportedPreviewFrameRates();
        }
        if (mTopParameters != null && topFrameRates == null) {
            topFrameRates = mTopParameters.getSupportedPreviewFrameRates();
        }
        if (frameRates != null) {
            Integer max = Collections.max(frameRates);
            parameters.setPreviewFrameRate(max);
            Log.i(TAG, "[setPreviewFrameRate] max = " + max + " frameRates = " + frameRates);
        }
        if (mTopParameters != null) {
            Integer topMax = Collections.max(topFrameRates);
            mTopParameters.setPreviewFrameRate(topMax);
            Log.i(TAG, "[top graphic] setPreviewFrameRate max = " + topMax + " topFrameRates = "
                    + topFrameRates);
        }
    }

    private void closeDynamicFrameRate(ICameraDevice iCameraDevice) {
        if (iCameraDevice == null) {
            Log.i(TAG, "[closeDynamicFrameRate] but why parameters is null");
            return;
        }
        boolean support = iCameraDevice.isDynamicFrameRateSupported();
        if (support) {
            iCameraDevice.setDynamicFrameRate(false);
        }
        Log.i(TAG, "[closeDynamicFrameRate] support = " + support);
    }

    private boolean isSupported(Object value, List<?> supported) {
        return supported == null ? false : supported.indexOf(value) >= 0;
    }

    private int getMaxSupportedPreviewFrameRate(List<Integer> supportedPreviewRate) {
        int maxFrameRate = 0;
        for (int rate : supportedPreviewRate) {
            if (rate > maxFrameRate) {
                maxFrameRate = rate;
            }
        }
        Log.d(TAG, "[getMaxSupportedPreviewFrameRate()] return " + maxFrameRate);
        return maxFrameRate;
    }

    private static final String MTK_CHIP_0321 = "0321";
    private void pipDenaliZSDRule(int index) {
        if (null == mICameraContext.getFeatureConfig().whichDeanliChip()) {
            Log.i(TAG, "not D1 chip ");
            return;
        }
        SettingItem zsdSetting = mISettingCtrl.getSetting(SettingConstants.KEY_CAMERA_ZSD);
        SettingItem pipSetting = mISettingCtrl.getSetting(SettingConstants.KEY_PHOTO_PIP);
        String resultValue = zsdSetting.getValue();
        int type = zsdSetting.getType();
        if (!MTK_CHIP_0321.equals(mICameraContext.getFeatureConfig().whichDeanliChip())
                || mICameraContext.getFeatureConfig().isLowRamOptSupport()) {
            return;
        }
        if (index != -1) {
            if (zsdSetting.isEnable()) {
                setResultSettingValue(type, "off", "off", true, zsdSetting);
            }
            Record record = zsdSetting.new Record(resultValue, "off");
            zsdSetting.addOverrideRecord(SettingConstants.KEY_PHOTO_PIP, record);
        } else {
            int overrideCount = zsdSetting.getOverrideCount();
            Record record = zsdSetting.getOverrideRecord(SettingConstants.KEY_PHOTO_PIP);
            if (record == null) {
                return;
            }
            Log.i(TAG, "overrideCount:" + overrideCount);
            zsdSetting.removeOverrideRecord(SettingConstants.KEY_PHOTO_PIP);
            overrideCount--;

            if (overrideCount > 0) {
                Record topRecord = zsdSetting.getTopOverrideRecord();
                if (topRecord != null) {
                    if (zsdSetting.isEnable()) {
                        String value = topRecord.getValue();
                        String overrideValue = topRecord.getOverrideValue();
                        // may be the setting's value is changed, the value in record is old.
                        ListPreference pref = zsdSetting.getListPreference();
                        if (pref != null && SettingUtils.isBuiltList(overrideValue)) {
                            pref.setEnabled(true);
                            String prefValue = pref.getValue();
                            List<String> list = SettingUtils.getEnabledList(overrideValue);
                            if (list.contains(prefValue)) {
                                if (!prefValue.equals(value)) {
                                    String[] values = new String[list.size()];
                                    overrideValue = SettingUtils.buildEnableList(
                                            list.toArray(values), prefValue);
                                }
                                value = prefValue;
                            }
                        }
                        setResultSettingValue(type, value, overrideValue, true, zsdSetting);
                    }
                }
            } else {
                ListPreference pref = zsdSetting.getListPreference();
                if (pref != null) {
                    resultValue = pref.getValue();
                }
                String overrideValue = null;
                if (zsdSetting.isEnable()) {
                        if (pref != null) {
                            pref.setEnabled(true);
                        }
                        setResultSettingValue(type, resultValue, overrideValue,
                                true, zsdSetting);
                    }
                }
            }
        }



    private void setResultSettingValue(int settingType, String value,
            String overrideValue, boolean restoreSupported, SettingItem item) {
        int currentCameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager
                .getCameraDevice(currentCameraId);
        Parameters parameters = cameraDevice.getParameters();
        item.setValue(value);
        ListPreference pref = item.getListPreference();
        if (pref != null) {
             pref.setOverrideValue(overrideValue, restoreSupported);
        }
        ParametersHelper.setParametersValue(parameters, currentCameraId,
                    item.getKey(), value);

    }
}
