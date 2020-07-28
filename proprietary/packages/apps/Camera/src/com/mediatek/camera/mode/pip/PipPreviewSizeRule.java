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

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.util.Log;

import java.util.ArrayList;
import java.util.List;

public class PipPreviewSizeRule implements ISettingRule {
    private static final String TAG = "PipPreviewSizeRule";

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
    private boolean      mSwitchingPip = false;
    private String       mCurrentPreviewRatio = null;

    public PipPreviewSizeRule(ICameraContext cameraContext) {
        Log.d(TAG, "[PipPreviewSizeRule]constructor...");
        mCameraContext = cameraContext;
    }

    @Override
    public void execute() {
        deviceManager = mCameraContext
                .getCameraDeviceManager();
        mBackCamDevice = deviceManager.getCameraDevice(deviceManager
                .getCurrentCameraId());
        if (mTopCamDevice != null) {
            mSwitchingPip = (mTopCamDevice.getCameraId() == deviceManager.getCurrentCameraId());
        } else {
            mSwitchingPip = false;
        }
        mTopCamDevice = deviceManager.getCameraDevice(getTopCameraId());
        mISettingCtrl = mCameraContext.getSettingController();
        mActivity = mCameraContext.getActivity();
        mParameters = mBackCamDevice.getParameters();
        if (mTopCamDevice != null) {
            mTopParameters = mTopCamDevice.getParameters();
        }
        String conditionValue = mISettingCtrl.getSettingValue(SettingConstants.KEY_PHOTO_PIP);
        int index = conditionSatisfied(conditionValue);
        Log.d(TAG, "[execute]index = " + index + " mSwitchingPip = " + mSwitchingPip);
        String previewRatio = null;
        if (mSwitchingPip) {
            previewRatio = mCurrentPreviewRatio;
            // restore pip preview ratio
            mISettingCtrl.setSettingValue(SettingConstants.KEY_PICTURE_RATIO, previewRatio,
                    deviceManager.getCurrentCameraId());
            ListPreference pref = mISettingCtrl
                    .getListPreference(SettingConstants.KEY_PICTURE_RATIO);
            if (pref != null) {
                pref.setValue(previewRatio);
            }
        } else {
            previewRatio = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_PICTURE_RATIO);
        }
        mCurrentPreviewRatio = previewRatio;
        if (index == -1) {
            SettingUtils.setPreviewSize(mActivity, mParameters, previewRatio);
        } else {
            setPreviewSize(previewRatio);
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

    public void setPreviewSize(String value) {
        SettingUtils.setPipPreviewSize(mActivity, mParameters,
                mTopParameters, mISettingCtrl, value);
    }

    private int conditionSatisfied(String conditionValue) {
        int index = mConditions.indexOf(conditionValue);
        return index;
    }

    private int getTopCameraId() {
        return deviceManager.getCurrentCameraId() == deviceManager.getBackCameraId() ?
                deviceManager.getFrontCameraId() : deviceManager.getBackCameraId();
   }
}
