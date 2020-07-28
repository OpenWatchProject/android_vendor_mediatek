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
package com.mediatek.camera.feature.setting.exposure;

import android.hardware.Camera;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * This class used to configure exposure value to capture request.
 * Just used for api1.
 */
public class ExposureParameterConfigure implements ICameraSetting.IParametersConfigure, IExposure
        .Listener {

    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(ExposureParameterConfigure.class.getSimpleName());
    private int mCurrentEv = 0;
    protected int mMinExposureCompensation = 0;
    protected int mMaxExposureCompensation = 0;
    protected float mExposureCompensationStep = 1.0f;
    private final Exposure mExposure;
    private boolean mAeLockSupported;
    private boolean mAeLock;
    private ISettingManager.SettingDeviceRequester mDeviceRequester;

    /**
     * The construction function.
     *
     * @param exposure        the Exposure class object.
     * @param deviceRequester deviceRequester The implementer of
     *                        {@link ISettingManager.SettingDeviceRequester}.
     */
    public ExposureParameterConfigure(Exposure exposure, ISettingManager.SettingDeviceRequester
            deviceRequester) {
        mExposure = exposure;
        mDeviceRequester = deviceRequester;
    }


    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        updateCapabilities(originalParameters);
        buildExposureCompensation();
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        if (mExposureCompensationStep != 0) {
            int exposureCompensationIndex = (int) (mCurrentEv / mExposureCompensationStep);
            LogHelper.d(TAG, "[configParameters] exposureCompensationIndex = " +
                    exposureCompensationIndex);
            parameters.setExposureCompensation(exposureCompensationIndex);
        }
        if (mAeLockSupported) {
            LogHelper.d(TAG, "[configParameters] setAutoExposureLock " + mAeLock);
            parameters.setAutoExposureLock(mAeLock);
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    @Override
    public void sendSettingChangeRequest() {
        if (mDeviceRequester != null) {
            mDeviceRequester.requestChangeSettingValue(mExposure.getKey());
        }
    }

    @Override
    public void updateEv(int value) {
        LogHelper.d(TAG, "[updateEv] + value " + value);
        if (value >= mMinExposureCompensation && value <= mMaxExposureCompensation) {
            if (value != mCurrentEv) {
                mCurrentEv = value;
                mExposure.setValue(String.valueOf(mCurrentEv));
            }
            LogHelper.d(TAG, "[updateEv] - mCurrentEv " + mCurrentEv);
        } else {
            LogHelper.w(TAG, "[updateEv] - invalid exposure range: " + value);
        }
    }

    @Override
    public boolean needConsiderAePretrigger() {
        return false;
    }

    @Override
    public boolean checkTodoCapturAfterAeConverted() {
        return false;
    }

    @Override
    public void setAeLock(boolean lock) {
        if (!mAeLockSupported) {
            LogHelper.w(TAG, "[setAeLock] not fail, AE lock not supported");
            return;
        }
        mAeLock = lock;
    }

    @Override
    public boolean getAeLock() {
        return mAeLock;
    }
    @Override
    public void overrideExposureValue(String currentValue, List<String> supportValues) {
        int value = Integer.valueOf(currentValue);
        if (value >= mMinExposureCompensation && value <= mMaxExposureCompensation) {
            mCurrentEv = value;
        } else {
            LogHelper.w(TAG, "[overrideExposureValue] invalid exposure range: " + value);
        }
    }

    private void updateCapabilities(Camera.Parameters originalParameters) {
        if (originalParameters == null) {
            LogHelper.w(TAG, "[updateCapabilities] characteristics is null");
            return;
        }
        mAeLockSupported = originalParameters.isAutoExposureLockSupported();
        mMaxExposureCompensation = originalParameters.getMaxExposureCompensation();
        mMinExposureCompensation = originalParameters.getMinExposureCompensation();
        mExposureCompensationStep = originalParameters.getExposureCompensationStep();
    }

    private void buildExposureCompensation() {
        if (mMaxExposureCompensation == 0 && mMinExposureCompensation == 0) {
            return;
        }
        LogHelper.d(TAG, "[buildExposureCompensation] + exposure compensation range (" +
                mMinExposureCompensation + ", "
                + mMaxExposureCompensation + "),with step " + mExposureCompensationStep);
        int maxValue = (int) Math.floor(mMaxExposureCompensation * mExposureCompensationStep);
        int minValue = (int) Math.ceil(mMinExposureCompensation * mExposureCompensationStep);
        ArrayList<String> values = new ArrayList<String>();
        for (int i = minValue; i <= maxValue; ++i) {
            values.add(String.valueOf(i));
        }
        initPlatformSupportedValues(values);
        int finalSize = values.size();
        int[] entryValues = new int[finalSize];
        for (int i = 0; i < finalSize; i++) {
            entryValues[i] = Integer.parseInt(values.get(finalSize - i - 1));
        }
        mExposure.initExposureCompensation(entryValues);
        LogHelper.d(TAG, "[buildExposureCompensation] - values = " + values);
    }

    private void initPlatformSupportedValues(ArrayList<String> values) {
        int defaultEv = 0;
        mCurrentEv = defaultEv;
        mExposure.setValue(String.valueOf(defaultEv));
        mExposure.setSupportedPlatformValues(values);
        mExposure.setSupportedEntryValues(values);
        mExposure.setEntryValues(values);
    }

}
