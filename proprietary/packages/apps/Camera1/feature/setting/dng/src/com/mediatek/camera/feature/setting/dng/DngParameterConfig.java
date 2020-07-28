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

package com.mediatek.camera.feature.setting.dng;

import android.hardware.Camera;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureResult;
import android.os.Message;
import android.util.Size;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.device.v1.CameraProxy.VendorDataCallback;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.portability.CameraEx;

import java.util.ArrayList;
import java.util.List;

/**
 * This is for dng capture flow in camera API1.
 */
@SuppressWarnings("deprecation")
public class DngParameterConfig implements ICameraSetting.IParametersConfigure,
        IDngConfig {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            DngParameterConfig.class.getSimpleName());
    // Message with Device
    private static final int MTK_CAMERA_COMMAND_ENABLE_DNG = 0x10000001;
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONE = 0x00000016;
    private static final int MTK_CAMERA_MSG_EXT_DATA_RAW16 = 0x00000013;

    private List<String> mDngList = new ArrayList<>();
    private OnDngValueUpdateListener mDngValueUpdateListener;
    private boolean mIsDngOn;
    private boolean mLastDngStatus;
    private boolean mIsTakePicture;
    // RAW data
    private boolean mCaptureResultReady;
    private boolean mRawDataReady;
    private byte[] mPictureData;
    private CameraCharacteristics mCharas;
    private CaptureResult mCaptureResult;
    private Size mRawSize;

    private ISettingManager.SettingDeviceRequester mSettingDeviceRequester;

    /**
     * Constructor of dng parameter config in api1.
     * @param settingDeviceRequester device requester.
     */
    public DngParameterConfig(ISettingManager.SettingDeviceRequester settingDeviceRequester) {
        mSettingDeviceRequester = settingDeviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        String supported = originalParameters.get("dng-supported");
        mDngList.clear();
        mDngList.add(DNG_OFF);
        if (supported != null && supported.equals("true")) {
            mDngList.add(DNG_ON);
            mDngValueUpdateListener.onDngValueUpdate(mDngList, true);
        } else {
            mDngValueUpdateListener.onDngValueUpdate(mDngList, false);
        }
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {
        LogHelper.d(TAG, "[configCommand] mIsDngOn = " + mIsDngOn);
        if (mIsDngOn) {
            cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONE,
                    mVendorDataCallback);
            cameraProxy.sendCommand(MTK_CAMERA_COMMAND_ENABLE_DNG, 1, 0);
        } else {
            cameraProxy.setVendorDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONE, null);
            cameraProxy.sendCommand(MTK_CAMERA_COMMAND_ENABLE_DNG, 0, 0);
        }
        mLastDngStatus = mIsDngOn;
    }

    @Override
    public void setDngValueUpdateListener(OnDngValueUpdateListener dngListener) {
        mDngValueUpdateListener = dngListener;
    }

    @Override
    public void requestChangeOverrideValues() {
        if (mSettingDeviceRequester != null) {
            mSettingDeviceRequester.requestChangeSettingValue(IDngConfig.KEY_DNG);
        }
    }

    @Override
    public void setDngStatus(boolean isOn, boolean isTakePicture) {
        LogHelper.d(TAG, "[setDngStatus], isOn:" + isOn + ", isTakePicture :" + isTakePicture);
        mIsDngOn = isOn;
        mIsTakePicture = isTakePicture;
    }

    @Override
    public void notifyOverrideValue(boolean isOn) {
        if (mLastDngStatus != mIsDngOn) {
            LogHelper.d(TAG, "[notifyOverrideValue]");
            mSettingDeviceRequester.requestChangeCommand(IDngConfig.KEY_DNG);
        }
    }

    @Override
    public void onModeClosed() {
        resetDngCaptureStatus();
    }

    @Override
    public void sendSettingChangeRequest() {
        if (!mIsTakePicture) {
            mSettingDeviceRequester.requestChangeCommand(IDngConfig.KEY_DNG);
        }
    }

    private final VendorDataCallback mVendorDataCallback
            = new VendorDataCallback() {

        @Override
        public void onDataTaken(Message message) {
            if (message == null || message.obj == null) {
                LogHelper.e(TAG, "[onDataTaken] invalid callback value, return null");
                return;
            }
            LogHelper.d(TAG, "raw meta callback ");
            CameraEx.MessageInfo msgInfo = (CameraEx.MessageInfo) message.obj;
            mCaptureResultReady = true;
            mCharas = (CameraCharacteristics) msgInfo.mArg2;
            mCaptureResult = (CaptureResult) msgInfo.mArg1;
            mDngValueUpdateListener.onDngCreatorStateUpdate(true);
            convertRawToDng();
        }

        @Override
        public void onDataCallback(int msgId, byte[] data, int arg1, int arg2) {
            long rawPictureCallbackTime = System.currentTimeMillis();
            LogHelper.d(TAG, "rawPictureCallbackTime = " + rawPictureCallbackTime + "ms");
            mRawDataReady = true;
            mPictureData = data;
            mDngValueUpdateListener.onDngCreatorStateUpdate(true);
            convertRawToDng();
        }
    };

    private void resetDngCaptureStatus() {
        mCaptureResultReady = false;
        mRawDataReady = false;
        mDngValueUpdateListener.onDngCreatorStateUpdate(false);
    }

    private void convertRawToDng() {
        if (!mCaptureResultReady || !mRawDataReady) {
            LogHelper.i(TAG, "[convertRawToDng] not ready");
            return;
        }

        if (DngUtils.getRawSize(mCharas) == null) {
            LogHelper.e(TAG, "[convertRawToDng], get raw size error");
            return;
        }
        LogHelper.i(TAG, "[convertRawToDng]");
        mRawSize = DngUtils.getRawSize(mCharas);
        int dngOrientation
                = DngUtils.getDngOrientation(mDngValueUpdateListener.onDisplayOrientationUpdate());
        byte[] dngData = DngUtils.getDngDataFromCreator(mPictureData, mCharas,
                mCaptureResult, mRawSize, dngOrientation);
        mDngValueUpdateListener.onSaveDngImage(dngData, mRawSize);
        resetDngCaptureStatus();
    }
}
