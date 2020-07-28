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
package com.mediatek.camera.mode.pip;

import android.app.Activity;
import android.hardware.Camera.Size;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.preference.ListPreference;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class PipPictureSizeRule implements ISettingRule {
    private static final String TAG = "PipPictureSizeRule";

    private static final String ZSD_MODE_ON = "on";
    private static final int NOT_FOUND = -1;
    private long PICTURE_SIZE_3M = 1024 * 1024 * 3;
    private long PICTURE_SIZE_8M = 1024 * 1024 * 8;
    private CharSequence[] mOriEntryValues;

    private Activity mActivity;
    private List<String> mConditions = new ArrayList<String>();
    private List<List<String>> mResults = new ArrayList<List<String>>();
    private List<MappingFinder> mMappingFinders = new ArrayList<MappingFinder>();
    private ICameraDevice mBackCamDevice;
    private ICameraDevice mTopCamDevice;
    private ISettingCtrl mISettingCtrl;
    private Parameters mParameters;
    private Parameters mTopParameters;
    private ICameraDeviceManager deviceManager;
    private ICameraContext mCameraContext;

    public PipPictureSizeRule(ICameraContext cameraContext) {
        Log.d(TAG, "[PreviewSizeRule]constructor...");
        mCameraContext = cameraContext;
    }

    @Override
    public void execute() {
        deviceManager = mCameraContext
                .getCameraDeviceManager();
        mBackCamDevice = deviceManager.getCameraDevice(deviceManager
                .getCurrentCameraId());
        if (mBackCamDevice == null) {
            Log.w(TAG, "[execute] mBackCamDevice is null!");
            return;
        }
        mTopCamDevice = deviceManager.getCameraDevice(getTopCameraId());
        mISettingCtrl = mCameraContext.getSettingController();
        mActivity = mCameraContext.getActivity();
        mParameters = mBackCamDevice.getParameters();
        if (mParameters == null) {
            Log.w(TAG, "[execute] mParameters is null!");
            return;
        }
        if (mTopCamDevice != null) {
            mTopParameters = mTopCamDevice.getParameters();
        }
        String conditionValue = mISettingCtrl
                .getSettingValue(SettingConstants.KEY_PHOTO_PIP);
        int index = conditionSatisfied(conditionValue);
        // pip D1 size rule ,this will modify after
        pipPictureSizeRule(index);
        String size = mISettingCtrl
                .getSettingValue(SettingConstants.KEY_PICTURE_SIZE);
        Log.d(TAG, "[execute]index = " + index);
        if (index == -1) {
            List<Size> supported = mParameters.getSupportedPictureSizes();
            String pictureSize = mISettingCtrl.getSettingValue(SettingConstants.KEY_PICTURE_SIZE);
            String ratio = mISettingCtrl.getSettingValue(SettingConstants.KEY_PICTURE_RATIO);
            SettingUtils.setCameraPictureSize(pictureSize, supported,
                    mParameters, ratio, mActivity);
        } else {
            setPictureSize(size);
        }
    }

    @Override
    public void addLimitation(String condition, List<String> result,
            MappingFinder mappingFinder) {
        Log.d(TAG, "[addLimitation]condition = " + condition);
        mConditions.add(condition);
        mResults.add(result);
        mMappingFinders.add(mappingFinder);
    }

    private int conditionSatisfied(String conditionValue) {
        int index = mConditions.indexOf(conditionValue);
        return index;
    }

    private void setPreviewFrameRate() {
        String zsdValue = mISettingCtrl
                .getSettingValue(SettingConstants.KEY_CAMERA_ZSD);
        List<Integer> pipFrameRates = null;
        List<Integer> pipTopFrameRates = null;
        if (ZSD_MODE_ON.equals(zsdValue)) {
            pipFrameRates = mParameters.getPIPFrameRateZSDOn();
            if (mTopParameters != null) {
                pipTopFrameRates = mTopParameters.getPIPFrameRateZSDOn();
            }
            Log.d(TAG, "getPIPFrameRateZSDOn pipFrameRates " + pipFrameRates
                    + " pipTopFrameRates = " + pipTopFrameRates);
        } else {
            pipFrameRates = mParameters.getPIPFrameRateZSDOff();
            if (mTopParameters != null) {
                pipTopFrameRates = mTopParameters.getPIPFrameRateZSDOff();
            }
            Log.d(TAG, "getPIPFrameRateZSDOff pipFrameRates = " + pipFrameRates
                    + " pipTopFrameRates = " + pipTopFrameRates);
        }
        // close dynamic frame rate, if dynamic frame rate is supported
        closeDynamicFrameRate(mParameters);
        closeDynamicFrameRate(mTopParameters);

        if (pipFrameRates != null) {
            Integer backFramerate = Collections.max(pipFrameRates);
            mParameters.setPreviewFrameRate(backFramerate);

        }
        if (mTopParameters != null) {
            Integer frontFramerate = Collections.max(pipTopFrameRates);
            mTopParameters.setPreviewFrameRate(frontFramerate);
        }
    }

    private void closeDynamicFrameRate(Parameters parameters) {
        if (parameters == null) {
            Log.w(TAG, "closeDynamicFrameRate but why parameters is null");
            return;
        }
        boolean support = parameters.isDynamicFrameRateSupported();
        if (support) {
            parameters.setDynamicFrameRate(false);
        }
        Log.d(TAG, "closeDynamicFrameRate support = " + support);
    }

    private void setPictureSize(String value) {
        Log.d(TAG, "setPictureSize(" + value + ")");
        // Set Bottom sensor Picture size.
        List<Size> supported = mParameters.getSupportedPictureSizes();
        // SettingUtils.setPictureSize(value, supported, mParameters, );
        String pictureSize = mISettingCtrl.getSettingValue(SettingConstants.KEY_PICTURE_SIZE);
        String ratio = mISettingCtrl.getSettingValue(SettingConstants.KEY_PICTURE_RATIO);
        SettingUtils.setCameraPictureSize(pictureSize, supported,
                mParameters, ratio, mActivity);

        // Set Top sensor Picture size.
        if (mTopParameters != null) {
            setTopCameraPictureSize(mParameters.getPictureSize());
        }

        // Set Preview Frame Rate
        setPreviewFrameRate();
    }

    private void setTopCameraPictureSize(Size targetPictureSize) {
        Log.d(TAG, "setTopCameraPictureSize targetPictureSize width = "
                + targetPictureSize.width + " height = "
                + targetPictureSize.height);
        if (mTopParameters != null) {
            Size miniPictureSize = getMininalPIPTopSize(
                    mTopParameters.getSupportedPictureSizes(),
                    (double) targetPictureSize.width / targetPictureSize.height);
            if (miniPictureSize == null) {
                miniPictureSize = targetPictureSize;
            }
            mTopParameters.setPictureSize(miniPictureSize.width,
                    miniPictureSize.height);
            Log.d(TAG, "setTopCameraPictureSize miniPictureSize width = "
                    + miniPictureSize.width + " height = "
                    + miniPictureSize.height);
        }
    }
    private int getTopCameraId() {
        return deviceManager.getCurrentCameraId() == deviceManager.getBackCameraId() ?
                deviceManager.getFrontCameraId() : deviceManager.getBackCameraId();
   }

    // Try to find a size matches aspect ratio and has the smallest size(preview
    // size & picture size)
    public Size getMininalPIPTopSize(List<Size> sizes, double targetRatio) {
        if (sizes == null || targetRatio < 0) {
            Log.w(TAG, "getMininalPIPTopSize error sizes = " + sizes
                    + " targetRatio = " + targetRatio);
            return null;
        }
        Size optimalSize = null;
        for (Size size : sizes) {
            double ratio = (double) size.width / size.height;
            Log.d(TAG, "getMininalPIPTopSize width = " + size.width
                    + " height = " + size.height);
            if (Math.abs(ratio - targetRatio) > 0.02)
                continue;
            if (optimalSize == null || size.width < optimalSize.width) {
                optimalSize = size;
            }
        }
        return optimalSize;
    }

    private static final String MTK_CHIP_0321 = "0321";
    private void pipPictureSizeRule(int index) {
        if (null == mCameraContext.getFeatureConfig().whichDeanliChip()) {
            Log.w(TAG, "not D1 chip ");
            return;
        }
        SettingItem pictureSetting = mISettingCtrl.getSetting(SettingConstants.KEY_PICTURE_SIZE);
        SettingItem pipSetting = mISettingCtrl.getSetting(SettingConstants.KEY_PHOTO_PIP);
        String resultValue = pictureSetting.getValue();
        int type = pictureSetting.getType();
        long pictureSizeRestriction = 0 ;
        if (mCameraContext.getFeatureConfig().isLowRamOptSupport()) {
            pictureSizeRestriction = PICTURE_SIZE_3M ;
        } else if (MTK_CHIP_0321.equals(mCameraContext.getFeatureConfig().whichDeanliChip())) {
            pictureSizeRestriction = PICTURE_SIZE_8M ;
        }
        Log.d(TAG, "pictureSizeRestriction = " + pictureSizeRestriction);
        if (pictureSizeRestriction == 0) {
            return;
        }
        if (index != -1) {

            ListPreference pref = pictureSetting.getListPreference();
            CharSequence[] entryValues = pref.getEntryValues();
            List<String> overValues = new ArrayList<String>();
            mOriEntryValues = pref.getEntryValues();
            String near3MSize = null;
            long near3M = 0L;
            int indexNum = 0;
            int width;
            int height;
            // here get entry values from listPreference and then remove size max 4M
            for (int i = 0; i < entryValues.length; i++) {
                indexNum = entryValues[i].toString().indexOf('x');
                width = Integer.parseInt(entryValues[i].toString().substring(0, indexNum));
                height = Integer.parseInt(entryValues[i].toString().substring(indexNum + 1));
                if (pictureSizeRestriction >= width * height) {
                    // remember the maximum size which is not bigger 4M
                    if (near3M < width * height) {
                        near3M = width * height;
                        near3MSize = "" + width + "x" + height;
                    }
                    overValues.add("" + width + "x" + height);
                }
            }
            // if resultValue is not bigger than 4M use it or use near4MSize
            if (0 > overValues.indexOf(resultValue)) {
                resultValue = near3MSize;
            }

            String[] values = new String[overValues.size()];
            String overrideValue = SettingUtils.buildEnableList(overValues.toArray(values),
                    resultValue);

            if (pictureSetting.isEnable()) {
                setResultSettingValue(type, resultValue, overrideValue, true, pictureSetting);
            }

            Record record = pictureSetting.new Record(resultValue, overrideValue);
            pictureSetting.addOverrideRecord(SettingConstants.KEY_PHOTO_PIP, record);
        } else {
            // restore picture size after set hdr off
            int overrideCount = pictureSetting.getOverrideCount();
            Record record = pictureSetting.getOverrideRecord(SettingConstants.KEY_PHOTO_PIP);
            if (record == null) {
                return;
            }
            Log.d(TAG, "overrideCount:" + overrideCount);
            pictureSetting.removeOverrideRecord(SettingConstants.KEY_PHOTO_PIP);
            overrideCount--;

            if (overrideCount > 0) {
                Record topRecord = pictureSetting.getTopOverrideRecord();
                if (topRecord != null) {
                    if (pictureSetting.isEnable()) {
                        String value = topRecord.getValue();
                        String overrideValue = topRecord.getOverrideValue();
                        // may be the setting's value is changed, the value in record is old.
                        ListPreference pref = pictureSetting.getListPreference();
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
                        setResultSettingValue(type, value, overrideValue, true, pictureSetting);
                    }
                }
            } else {
                mISettingCtrl.executeRule(SettingConstants.KEY_PICTURE_RATIO,
                        SettingConstants.KEY_PICTURE_SIZE);
            }
        }
    }


    private void setResultSettingValue(int settingType, String value,
            String overrideValue, boolean restoreSupported, SettingItem item) {

        int currentCameraId = deviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = deviceManager.getCameraDevice(currentCameraId);
        Parameters parameters = cameraDevice.getParameters();
        item.setValue(value);
        ListPreference pref = item.getListPreference();

        if (SettingUtils.RESET_STATE_VALUE_DISABLE.equals(overrideValue)) {
            if (pref != null) {
                pref.setEnabled(false);
            }
        } else {
            if (pref != null) {
                pref.setOverrideValue(overrideValue, restoreSupported);
            }
            ParametersHelper.setParametersValue(parameters, currentCameraId,
                    item.getKey(), value);
        }
    }

}
