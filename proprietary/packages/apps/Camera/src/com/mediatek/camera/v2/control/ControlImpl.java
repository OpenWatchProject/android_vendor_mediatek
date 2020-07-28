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

package com.mediatek.camera.v2.control;

import android.app.Activity;
import android.graphics.RectF;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.view.ViewGroup;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.control.IControl.FocusStateListener;
import com.mediatek.camera.v2.control.IControl.IAaaController;
import com.mediatek.camera.v2.control.IControl.IAaaListener;
import com.mediatek.camera.v2.control.exposure.AutoExposure;
import com.mediatek.camera.v2.control.exposure.IExposure;
import com.mediatek.camera.v2.control.focus.AutoFocus;
import com.mediatek.camera.v2.control.focus.IFocus;
import com.mediatek.camera.v2.control.whitebalance.AutoWhiteBalance;
import com.mediatek.camera.v2.control.whitebalance.IWhiteBalance;
import com.mediatek.camera.v2.module.ModuleListener;
import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.platform.app.AppController;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.ISettingServant.ISettingChangedListener;
import com.mediatek.camera.v2.setting.SettingConvertor;
import com.mediatek.camera.v2.setting.SettingConvertor.SceneMode;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.util.SettingKeys;
import com.mediatek.camera.v2.util.Utils;

import junit.framework.Assert;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

public class ControlImpl implements IControl,
                        IAaaListener,
                        IAaaController,
                        ISettingChangedListener {
    private static final Tag TAG = new Tag(ControlImpl.class.getSimpleName());
    private final  Activity             mActivity;
    private final  ViewGroup            mParentViewGroup;
    private final  ModuleListener       mModuleListener;
    private final  ISettingServant      mSettingServant;
    private final  IFocus               mFocus;
    private final  IExposure            mExposure;
    private final  IWhiteBalance        mWhiteBalance;

    private String                      mSceneMode = null;
    private String                      mInitializedCameraId;

    public ControlImpl(AppController app, ModuleListener moduleListener,
            boolean isAutoCtrl, String cameraId) {
        LogHelper.i(TAG, "ControlImpl cameraId: " + cameraId);
        Assert.assertNotNull(app);
        Assert.assertNotNull(moduleListener);
        mActivity         = app.getActivity();
        mParentViewGroup  = app.getCameraAppUi().getModuleLayoutRoot();
        mModuleListener   = moduleListener;
        mInitializedCameraId = cameraId;
        mSettingServant  = app.getServices().getSettingController().getSettingServant(cameraId);
        if (isAutoCtrl) {
            mFocus = new AutoFocus(
                    mSettingServant,
                    app.getServices().getSoundPlayback(),
                    app,
                    this,
                    mParentViewGroup);
            mExposure = new AutoExposure(mSettingServant, this);
            mWhiteBalance = new AutoWhiteBalance(mSettingServant, this);
        } else {
            //TODO new manual mode instead
            mFocus = new AutoFocus(mSettingServant,
                    app.getServices().getSoundPlayback(),
                    app,
                    this,
                    mParentViewGroup);
            mExposure = new AutoExposure(mSettingServant, this);
            mWhiteBalance = new AutoWhiteBalance(mSettingServant, this);
        }
    }

    @Override
    public void open(Activity activity, ViewGroup parentView,
            boolean isCaptureIntent) {
        mSettingServant.registerSettingChangedListener(this, null,
                ISettingChangedListener.MIDDLE_PRIORITY);
        mFocus.open(activity, parentView, isCaptureIntent);
        mExposure.open(activity, parentView, isCaptureIntent);
        mWhiteBalance.open(activity, parentView, isCaptureIntent);
    }

    @Override
    public void resume() {
        updateSceneMode();
        mFocus.resume();
        mExposure.resume();
        mWhiteBalance.resume();
    }

    @Override
    public void pause() {
        mFocus.pause();
        mExposure.pause();
        mWhiteBalance.pause();
    }

    @Override
    public void close() {
        mSettingServant.unRegisterSettingChangedListener(this);
        mFocus.close();
        mExposure.close();
        mWhiteBalance.close();
    }

    @Override
    public void onOrientationChanged(int orientation) {
        int orientationCompensation = (orientation +
                Utils.getDisplayRotation(mActivity)) % 360;
        mFocus.onOrientationCompensationChanged(orientationCompensation);
    }

    @Override
    public void onPreviewAreaChanged(RectF previewArea) {
        mFocus.onPreviewAreaChanged(previewArea);
    }

    @Override
    public void onSingleTapUp(float x, float y) {
        LogHelper.i(TAG, "onSingleTapUp");
        mFocus.onSingleTapUp(x, y);
    }

    @Override
    public void configuringSessionRequests(
            Map<RequestType, Builder> requestBuilders,
            CaptureType captureType,
            boolean bottomCamera) {
        int controlMode = CaptureRequest.CONTROL_MODE_AUTO;
        int nativeSceneMode = SettingConvertor.convertStringToEnum(
                SettingKeys.KEY_SCENE_MODE, mSceneMode);
        if (mSceneMode != null && !SceneMode.AUTO.toString().equalsIgnoreCase(mSceneMode)) {
            controlMode = CaptureRequest.CONTROL_MODE_USE_SCENE_MODE;
        }
        Set<RequestType> keySet = requestBuilders.keySet();
        Iterator<RequestType> iterator = keySet.iterator();
        LogHelper.i(TAG, "configuringSessionRequests control mode : " + controlMode +
                " SceneMode : " + nativeSceneMode + " size : " + keySet.size());

        while (iterator.hasNext()) {
            RequestType requestType = iterator.next();
            CaptureRequest.Builder requestBuilder = requestBuilders.get(requestType);
            requestBuilder.set(CaptureRequest.CONTROL_MODE, controlMode);
            requestBuilder.set(CaptureRequest.CONTROL_SCENE_MODE, nativeSceneMode);

            mFocus.configuringSessionRequest(requestType, requestBuilder,
                    captureType, bottomCamera);
            mExposure.configuringSessionRequest(requestType, requestBuilder, captureType,
                    bottomCamera);
            mWhiteBalance.configuringSessionRequest(requestType, requestBuilder, bottomCamera);
        }
    }

    @Override
    public void onPreviewCaptureStarted(CaptureRequest request, long timestamp,
            long frameNumber) {
        mFocus.onPreviewCaptureStarted(request, timestamp, frameNumber);
        mExposure.onPreviewCaptureStarted(request, timestamp, frameNumber);
        mWhiteBalance.onPreviewCaptureStarted(request, timestamp, frameNumber);
    }

    @Override
    public void onPreviewCaptureProgressed(CaptureRequest request,
            CaptureResult partialResult) {
        mFocus.onPreviewCaptureProgressed(request, partialResult);
    }

    @Override
    public void onPreviewCaptureCompleted(CaptureRequest request,
            TotalCaptureResult result) {
        mFocus.onPreviewCaptureCompleted(request, result);
        mExposure.onPreviewCaptureCompleted(request, result);
        mWhiteBalance.onPreviewCaptureCompleted(request, result);
    }


    @Override
    public void clearFocusUi() {
        mFocus.clearFocusUi();
    }

    @Override
    public void setFocusStateListener(FocusStateListener listener) {
        mFocus.setFocusStateListener(listener);
    }

    @Override
    public void clearFocusStateListener(FocusStateListener listener) {
        mFocus.clearFocusStateListener(listener);
    }

    @Override
    public void onSettingChanged(Map<String, String> result) {
        String sceneMode = result.get(SettingKeys.KEY_SCENE_MODE);
        if (sceneMode != null) {
            updateSceneMode();
        }
    }

    @Override
    public RequestType getRepeatingRequestType() {
        return mModuleListener.getRepeatingRequestType();
    }

    @Override
    public void requestChangeCaptureRequets(boolean sync,
            RequestType requestType,
            CaptureType captureType) {
        if (mInitializedCameraId == null) {
            mModuleListener.requestChangeCaptureRequets(sync, requestType, captureType);
        } else if (SettingCtrl.BACK_CAMERA.equals(mInitializedCameraId)) {
            mModuleListener.requestChangeCaptureRequets(true, sync, requestType, captureType);
        } else if (SettingCtrl.FRONT_CAMERA.equals(mInitializedCameraId)) {
            mModuleListener.requestChangeCaptureRequets(false, sync, requestType, captureType);
        }
    }

    @Override
    public void aePreTriggerAndCapture() {
        if (mExposure != null) {
            mExposure.aePreTriggerAndCapture();
        }
    }

    private void updateSceneMode() {
        LogHelper.i(TAG, "[updateSceneMode]+");
        String currentsceneMode = mSettingServant.getSettingValue(SettingKeys.KEY_SCENE_MODE);
        if (currentsceneMode != null && !currentsceneMode.equals(mSceneMode)) {
            mSceneMode = currentsceneMode;
            requestChangeCaptureRequets(false, getRepeatingRequestType(),
                    CaptureType.REPEATING_REQUEST);
        }
        LogHelper.i(TAG, "[updateSceneMode]- ");
    }
}
