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

package com.mediatek.camera.v2.control.exposure;

import android.app.Activity;
import android.graphics.RectF;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.view.ViewGroup;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.control.IControl.IAaaListener;
import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.ISettingServant.ISettingChangedListener;
import com.mediatek.camera.v2.setting.SettingConvertor;
import com.mediatek.camera.v2.util.SettingKeys;

import java.util.ArrayList;
import java.util.Map;

public class AutoExposure implements IExposure, ISettingChangedListener {
    private static final Tag TAG = new Tag(AutoExposure.class.getSimpleName());
    private static final String    ON = "on";
    private static final String    AUTO = "auto";
    private final IAaaListener     mIaaaListener;
    private final ISettingServant  mSettingServant;
    private Activity               mActivity;
    private ArrayList<String>      mCaredSettingChangedKeys = new ArrayList<String>();

    private int                    mAEMode = CameraMetadata.CONTROL_AE_MODE_ON;
    private int                    mFlashMode = CameraMetadata.FLASH_MODE_OFF;
    private String                 mExposureCompensation = null;
    private int mAntiBandingMode = CameraMetadata.CONTROL_AE_ANTIBANDING_MODE_AUTO;
    private String mSensitivity;
    private boolean                mNeedAePretrigger = false;
    private boolean                mAePreTriggerAndCaptureEnabled = false;
    private boolean                mAePreTriggerRequestProcessed = false;

    public AutoExposure(ISettingServant settingServant, IAaaListener aaaListener) {
        mIaaaListener = aaaListener;
        mSettingServant = settingServant;
    }

    @Override
    public void open(Activity activity, ViewGroup parentView,
            boolean isCaptureIntent) {
        mActivity = activity;
        updateCaredSettingChangedKeys();
        mSettingServant.registerSettingChangedListener(this, mCaredSettingChangedKeys,
                ISettingChangedListener.MIDDLE_PRIORITY);
    }

    @Override
    public void resume() {
        mAEMode = CameraMetadata.CONTROL_AE_MODE_ON;
        mFlashMode = CameraMetadata.FLASH_MODE_OFF;
        mExposureCompensation = null;
        mNeedAePretrigger = false;
        mAePreTriggerAndCaptureEnabled = false;
        mAePreTriggerRequestProcessed = false;
    }

    @Override
    public void pause() {

    }

    @Override
    public void close() {
        mSettingServant.unRegisterSettingChangedListener(this);
    }

    @Override
    public void onPreviewAreaChanged(RectF previewArea) {

    }

    @Override
    public void onSingleTapUp(float x, float y) {

    }

    @Override
    public void configuringSessionRequest(RequestType requestType, Builder requestBuilder,
            CaptureType captureType, boolean bottomCamera) {
        LogHelper.i(TAG, "[configuringSessionRequests]+" +
                " mAePretriggerRequested:" + mNeedAePretrigger);
        updateExposureCompensation();
        updateAeFlashMode(requestType);
        updateAntiBandingMode();
        updateSensitivity();
        requestBuilder.set(CaptureRequest.FLASH_MODE, mFlashMode);
        requestBuilder.set(CaptureRequest.CONTROL_AE_MODE, mAEMode);
        if (mExposureCompensation != null && mAEMode != CameraMetadata.CONTROL_AE_MODE_OFF) {
            requestBuilder.set(CaptureRequest.CONTROL_AE_EXPOSURE_COMPENSATION,
                    Integer.parseInt(mExposureCompensation));
        }
        requestBuilder.set(CaptureRequest.CONTROL_AE_ANTIBANDING_MODE, mAntiBandingMode);
        if (mSensitivity != null && !mSensitivity.equals(AUTO)) {
            requestBuilder.set(CaptureRequest.SENSOR_SENSITIVITY, Integer.parseInt(mSensitivity));
        }
        setAePreCaptureTriggerValue(captureType, requestBuilder);
        LogHelper.i(TAG, "[configuringSessionRequests]- requestType = " + requestType
                + " AEMode = " + mAEMode + " FlashMode = " + mFlashMode
                + " mExposureCompensation = " + mExposureCompensation
                + " mAntiBandingMode = " + mAntiBandingMode
                + " mSensitivity = " + mSensitivity);
    }

    @Override
    public void onPreviewCaptureStarted(CaptureRequest request, long timestamp,
            long frameNumber) {

    }

    @Override
    public void onPreviewCaptureCompleted(CaptureRequest request,
            TotalCaptureResult result) {
        checkAeState(request, result);
    }

    @Override
    public void aePreTriggerAndCapture() {
        LogHelper.i(TAG, "[aePreTriggerAndCapture]+");
        String flash = mSettingServant.getSettingValue(SettingKeys.KEY_FLASH);
        if (!ON.equals(flash) && !AUTO.equals(flash)) {
            // only flash on/auto do ae pre trigger
            // else do capture immediately
            mIaaaListener.requestChangeCaptureRequets(false,
                    RequestType.STILL_CAPTURE, CaptureType.CAPTURE);
            LogHelper.i(TAG, "[aePreTriggerAndCapture]- flash:" + flash);
            return;
        }
        mNeedAePretrigger = true;
        mAePreTriggerAndCaptureEnabled = true;
        mIaaaListener.requestChangeCaptureRequets(true,
                mIaaaListener.getRepeatingRequestType(), CaptureType.CAPTURE);
        mIaaaListener.requestChangeCaptureRequets(true,
                mIaaaListener.getRepeatingRequestType(), CaptureType.REPEATING_REQUEST);
        LogHelper.i(TAG, "[aePreTriggerAndCapture]-");
    }

    @Override
    public void onSettingChanged(Map<String, String> result) {
        String[] keys = {SettingKeys.KEY_EXPOSURE, SettingKeys.KEY_FLASH,
                SettingKeys.KEY_ANTI_BANDING, SettingKeys.KEY_ISO};
        String value;
        for (int i = 0; i < keys.length; i++) {
            value = result.get(keys[i]);
            if (value != null) {
                requestChangeCaptureRequets();
            }
        }
    }

    private void updateCaredSettingChangedKeys() {
        String[] keys = {SettingKeys.KEY_EXPOSURE, SettingKeys.KEY_FLASH,
                SettingKeys.KEY_ANTI_BANDING, SettingKeys.KEY_ISO};
        String key;
        for (int i = 0; i < keys.length; i++) {
            key = keys[i];
            if (key != null && !mCaredSettingChangedKeys.contains(key)) {
                mCaredSettingChangedKeys.add(key);
            }
        }
    }

    private void updateExposureCompensation() {
        String exposureCompensation = mSettingServant.getSettingValue(SettingKeys.KEY_EXPOSURE);
        LogHelper.i(TAG, "[updateExposureCompensation]+ EV=" + exposureCompensation);
        if (exposureCompensation != null && !exposureCompensation.equals(mExposureCompensation)) {
            mExposureCompensation = exposureCompensation;
        }
        LogHelper.i(TAG, "[updateExposureCompensation]- ");
    }

    private void updateAeFlashMode(RequestType curRequestType) {
        String flash = mSettingServant.getSettingValue(SettingKeys.KEY_FLASH);
        LogHelper.i(TAG, "updateAeFlashMode flash from setting" + flash);
        if (flash == null) {
            return;
        }
        if (ON.equalsIgnoreCase(flash)) {
            if (isRecordingRequestType(curRequestType)) {
                mAEMode = CameraMetadata.CONTROL_AE_MODE_ON;
                mFlashMode = CameraMetadata.FLASH_MODE_TORCH;
                return;
            }
            mAEMode = CameraMetadata.CONTROL_AE_MODE_ON_ALWAYS_FLASH;
            // AE mode -> ON_ALWAYS_FLASH, flash mode value doesn't affect
            mFlashMode = CameraMetadata.FLASH_MODE_OFF;
            return;
        }
        if (AUTO.equalsIgnoreCase(flash)) {
            mAEMode = CameraMetadata.CONTROL_AE_MODE_ON_AUTO_FLASH;
            // AE mode -> ON_AUTO_FLASH, flash mode value doesn't affect
            mFlashMode = CameraMetadata.FLASH_MODE_OFF;
            return;
        }
        // flash -> off, must need AE mode -> on
        mAEMode = CameraMetadata.CONTROL_AE_MODE_ON;
        mFlashMode = CameraMetadata.FLASH_MODE_OFF;
    }

    private boolean isRecordingRequestType(RequestType requestType) {
        return requestType == RequestType.RECORDING ||
                requestType == RequestType.VIDEO_SNAP_SHOT;
    }

    private void updateAntiBandingMode() {
        int antiBandingMode = SettingConvertor.convertStringToEnum(SettingKeys.KEY_ANTI_BANDING,
                mSettingServant.getSettingValue(SettingKeys.KEY_ANTI_BANDING));
        LogHelper.d(TAG, "[updateAntiBandingMode]+ antiBandingMode=" + antiBandingMode);
        mAntiBandingMode = antiBandingMode;
        LogHelper.d(TAG, "[updateAntiBandingMode]- ");
    }

    private void updateSensitivity() {
        mSensitivity = mSettingServant.getSettingValue(SettingKeys.KEY_ISO);
        LogHelper.d(TAG, "[updateSensitivity], mSensitivity=" + mSensitivity);
    }

    private void requestChangeCaptureRequets() {
        mIaaaListener.requestChangeCaptureRequets(false, mIaaaListener.getRepeatingRequestType(),
                CaptureType.REPEATING_REQUEST);
    }

    private void setAePreCaptureTriggerValue(CaptureType captureType, Builder requestBuilder) {
        if (mNeedAePretrigger && (captureType == captureType.CAPTURE)) {
            requestBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                    CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);
            mNeedAePretrigger = false;
            return;
        }
        // Caution: the value of CONTROL_AE_PRECAPTURE_TRIGGER from
        // preview CaptureRequest maybe Null, when not set CONTROL_AE_PRECAPTURE_TRIGGER
        // to preview CaptureRequest. So, we set IDLE to avoid this happens.
        requestBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_IDLE);
    }

    private void checkAeState(CaptureRequest request, TotalCaptureResult result) {
        Integer aePrecaptureTrigger = request.get(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER);
        Integer aeState = result.get(TotalCaptureResult.CONTROL_AE_STATE);
        LogHelper.i(TAG, "aeStateCheck aeState:" + aeState +
                " aePrecaptureTrigger:" + aePrecaptureTrigger);
        if (aeState == null || aePrecaptureTrigger == null) {
            return;
        }
        if (mAePreTriggerAndCaptureEnabled) {
            if (!mAePreTriggerRequestProcessed) {
                mAePreTriggerRequestProcessed = (aePrecaptureTrigger
                        == CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);
            }
            if (mAePreTriggerRequestProcessed &&
                    (aeState == CaptureResult.CONTROL_AE_STATE_CONVERGED
                        || aeState == CaptureResult.CONTROL_AE_STATE_FLASH_REQUIRED)) {
                mActivity.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        LogHelper.i(TAG, "Ae pre capture trigger completed submit still capture!");
                        mIaaaListener.requestChangeCaptureRequets(false, RequestType.STILL_CAPTURE,
                                CaptureType.CAPTURE);
                    }
                });
                mAePreTriggerAndCaptureEnabled = false;
                mAePreTriggerRequestProcessed = false;
            }
        }
    }
}
