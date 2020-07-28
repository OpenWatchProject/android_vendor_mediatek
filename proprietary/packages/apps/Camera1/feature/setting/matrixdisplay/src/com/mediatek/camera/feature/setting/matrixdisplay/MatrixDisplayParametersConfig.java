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
package com.mediatek.camera.feature.setting.matrixdisplay;

import android.graphics.ImageFormat;
import android.hardware.Camera;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.ArrayList;
import java.util.List;

/**
 * Matrix display parameters configure.
 */

public class MatrixDisplayParametersConfig implements ICameraSetting.IParametersConfigure,
        IMatrixDisplayConfig {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(MatrixDisplayParametersConfig.class.getSimpleName());

    private final static int CACHE_BUFFER_NUM = 3;
    private IPreviewFrameCallback mCallback;
    private ISettingManager.SettingDeviceRequester mDeviceRequester;
    private ValueInitializedListener mListener;
    private String mKey;

    private byte[][] mPreviewCallbackBuffers = new byte[CACHE_BUFFER_NUM][];
    private String mCurrentEffect;
    private int mPreviewWidth;
    private int mPreviewHeight;

    private boolean mDisplayOpened = false;
    private boolean mIsStatusChanged = false;

    /**
     * Listener to listen value initialized.
     */
    interface ValueInitializedListener {
        /**
         * Callback when value is initialized.
         *
         * @param supportedEffects The supported color effects.
         * @param defaultEffect The color effect default value.
         * @param supportedPreviewSizes The supported preview sizes.
         */
        void onValueInitialized(List<String> supportedEffects, String defaultEffect,
                                List<String> supportedPreviewSizes);
    }

    /**
     * Parameters configure constructor.
     *
     * @param key The key of matrix display.
     * @param deviceRequester The instance of {@link ISettingManager.SettingDeviceRequester}.
     * @param listener The instance of {@link ValueInitializedListener}.
     */
    public MatrixDisplayParametersConfig(String key,
                                         ISettingManager.SettingDeviceRequester deviceRequester,
                                         ValueInitializedListener listener) {
        mKey = key;
        mDeviceRequester = deviceRequester;
        mListener = listener;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        List<String> supportedColorEffects = originalParameters.getSupportedColorEffects();
        String defaultColor = originalParameters.getColorEffect();
        List<String> supportedPreviewSizes = sizeToStr(
                originalParameters.getSupportedPreviewSizes());
        mListener.onValueInitialized(supportedColorEffects,
                defaultColor, supportedPreviewSizes);
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        LogHelper.d(TAG, "[configParameters], matrix display state is changed:" + mIsStatusChanged
                + ", current display is opened:" + mDisplayOpened
                + ", mCurrentEffect:" + mCurrentEffect);
        if (mCurrentEffect == null) {
            return false;
        }
        parameters.setColorEffect(mCurrentEffect);
        if (mDisplayOpened) {
            parameters.setPreviewSize(mPreviewWidth, mPreviewHeight);
            parameters.setPreviewFormat(ImageFormat.YV12);
            parameters.setRecordingHint(false);
        }
        if (mIsStatusChanged) {
            mIsStatusChanged = false;
            return true;
        }
        return false;
    }

    @Override
    public void configCommand(final CameraProxy cameraProxy) {
        LogHelper.d(TAG, "[configCommand], cameraProxy:" + cameraProxy
                + ", mPreviewWidth:" + mPreviewWidth
                + ", mPreviewHeight:" + mPreviewHeight);
        if (!mDisplayOpened) {
            cameraProxy.setPreviewCallback(null);
        } else {
            int bufferSize = mPreviewWidth * mPreviewHeight * 3 / 2;
            for (int i = 0; i < mPreviewCallbackBuffers.length; i++) {
                if (mPreviewCallbackBuffers[i] == null) {
                    mPreviewCallbackBuffers[i] = new byte[bufferSize];
                }
                cameraProxy.addCallbackBuffer(mPreviewCallbackBuffers[i]);
            }
            cameraProxy.setPreviewCallbackWithBuffer(new Camera.PreviewCallback() {
                @Override
                public void onPreviewFrame(byte[] bytes, Camera camera) {
                    if (bytes == null) {
                        LogHelper.e(TAG, "[onPreviewFrame], callback buffer is null");
                        return;
                    }
                    if (mCallback != null) {
                        mCallback.onPreviewFrameAvailable(bytes);
                        cameraProxy.addCallbackBuffer(bytes);
                    }
                }
            });
        }


    }

    @Override
    public void setPreviewSize(int width, int height) {
        if (mPreviewWidth != width || mPreviewHeight != height) {
            for (int i = 0; i < mPreviewCallbackBuffers.length; i++) {
                mPreviewCallbackBuffers[i] = null;
            }
        }
        mPreviewWidth = width;
        mPreviewHeight = height;
    }

    @Override
    public void setDisplayStatus(boolean opened) {
        mIsStatusChanged = mDisplayOpened ^ opened;
        mDisplayOpened = opened;
    }

    @Override
    public void setSelectedEffect(String effect) {
        mCurrentEffect = effect;
    }

    @Override
    public void sendSettingChangeRequest() {
        if (mDisplayOpened) {
            mDeviceRequester.requestChangeSettingValue(mKey);
            mDeviceRequester.requestChangeCommand(mKey);
        } else {
            //need first clear the preview callback and then request change setting value.
            //otherwise the preview callback maybe replace by here.
            mDeviceRequester.requestChangeCommand(mKey);
            mDeviceRequester.requestChangeSettingValue(mKey);
        }

    }

    /**
     * Set preview frame callback to receive preview frame.
     *
     * @param callback The instance of {@link IPreviewFrameCallback}.
     */
    public void setPreviewFrameCallback(IPreviewFrameCallback callback) {
        mCallback = callback;
    }

    private List<String> sizeToStr(List<Camera.Size> sizes) {
        List<String> sizeInStr = new ArrayList<>(sizes.size());
        for (Camera.Size size : sizes) {
            sizeInStr.add(size.width + "x" + size.height);
        }
        return sizeInStr;
    }
}
