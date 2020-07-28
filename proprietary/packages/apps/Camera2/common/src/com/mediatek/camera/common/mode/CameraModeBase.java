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

package com.mediatek.camera.common.mode;

import android.content.Intent;
import android.hardware.Camera;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.KeyEvent;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.portability.SystemProperties;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * A common abstract {@link ICameraMode} implementation that contains some
 * utility functions and plumbing we don't want every sub-class of {@link ICameraMode}
 * to duplicate. Hence all {@link ICameraMode} implementation should sub-class this class
 * instead.
 */
public abstract class CameraModeBase implements
        ICameraMode,
        IAppUiListener.OnShutterButtonListener,
        IApp.BackPressedListener,
        IApp.OnOrientationChangeListener,
        IApp.KeyEventListener {
    private LogUtil.Tag mTag;
    private static final String CAMERA_FACING_BACK = "back";
    private static final String CAMERA_FACING_FRONT = "front";
    private static final String EXTRA_USE_FRONT_CAMERA_FOR_GOOGLE
            = "com.google.assistant.extra.USE_FRONT_CAMERA";
    private static final String EXTRA_USE_FRONT_CAMERA_FOR_ANDROID
            = "android.intent.extra.USE_FRONT_CAMERA";
    protected static final String BACK_CAMERA_ID = "0";
    protected static final String FRONT_CAMERA_ID = "1";

    protected static final String KEY_CAMERA_SWITCHER = "key_camera_switcher";
    private static final int LOWEST_PRIORITY = -1;
    private static final int MSG_MODE_ON_SHUTTER_BUTTON_CLICK = 0;
    protected IApp mIApp;
    protected ICameraContext mICameraContext;
    protected DataStore mDataStore;
    protected CameraApi mCameraApi;
    protected DeviceUsage mCurrentModeDeviceUsage;
    protected DeviceUsage mNextModeDeviceUsage;
    protected ArrayList<String> mNeedCloseCameraIds = new ArrayList<>();
    protected boolean mNeedCloseSession;

    private volatile String mModeDeviceStatus = MODE_DEVICE_STATE_UNKNOWN;
    protected ModeHandler mModeHandler;
    private static final List<String> sSupportDualZoomModeList = Arrays.asList(
            "com.mediatek.camera.common.mode.photo.PhotoMode",
            "com.mediatek.camera.common.mode.video.VideoMode");

    @Override
    public void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext,
                     boolean isFromLaunch) {
        mTag = new LogUtil.Tag(getClass().getSimpleName());
        updateModeDefinedCameraApi();
        mIApp = app;
        mIApp.getAppUi().applyAllUIEnabled(false);
        updateModeDeviceState(MODE_DEVICE_STATE_UNKNOWN);
        mICameraContext = cameraContext;
        mDataStore = cameraContext.getDataStore();
        mModeHandler = new ModeHandler(Looper.myLooper());

        app.registerBackPressedListener(this, IApp.DEFAULT_PRIORITY);
        app.registerKeyEventListener(this, IApp.DEFAULT_PRIORITY);
        app.registerOnOrientationChangeListener(this);
        app.getAppUi().registerOnShutterButtonListener(this, IAppUi.DEFAULT_PRIORITY);
    }

    @Override
    public void resume(@Nonnull DeviceUsage deviceUsage) {
        mIApp.getAppUi().applyAllUIEnabled(false);
        mCurrentModeDeviceUsage = deviceUsage;
    }

    @Override
    public void pause(@Nonnull DeviceUsage nextModeDeviceUsage) {
        mIApp.getAppUi().applyAllUIEnabled(false);
        //must use the old device usage get the camera id.
        //because when pause activity, the newModeDeviceUsage is null.
        //so you can not change to as:newModeDeviceUsage.getNeedClosedCameraIds(
        // mCurrentModeDeviceUsage);
        mNextModeDeviceUsage = nextModeDeviceUsage;
        mNeedCloseCameraIds = mCurrentModeDeviceUsage.getNeedClosedCameraIds(nextModeDeviceUsage);
        mNeedCloseSession = mNextModeDeviceUsage != null &&
                !(mCurrentModeDeviceUsage.getBufferFlowType() == DeviceUsage.BUFFER_FLOW_TYPE_NORMAL
                        && mNextModeDeviceUsage.getBufferFlowType()
                        == DeviceUsage.BUFFER_FLOW_TYPE_NORMAL);
    }

    @Override
    public void unInit() {
        mIApp.unRegisterBackPressedListener(this);
        mIApp.unRegisterKeyEventListener(this);
        mIApp.unregisterOnOrientationChangeListener(this);
        mIApp.getAppUi().unregisterOnShutterButtonListener(this);
    }

    @Override
    public boolean onCameraSelected(@Nonnull String newCameraId) {
        ArrayList<String> cameraList = new ArrayList<>();
        cameraList.add(newCameraId);
        mCurrentModeDeviceUsage.updateCameraIdList(cameraList);
        return false;
    }

    @Override
    public String getModeKey() {
        return getClass().getName();
    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        return false;
    }

    @Override
    public boolean onShutterButtonClick() {
        LogHelper.d(mTag, "[onShutterButtonClick] UI thread");
        if (mModeHandler.hasMessages(MSG_MODE_ON_SHUTTER_BUTTON_CLICK)) {
            return true;
        }
        mModeHandler.sendEmptyMessage(MSG_MODE_ON_SHUTTER_BUTTON_CLICK);
        // TODO when return ture and when return false
        return true;
    }

    @Override
    public boolean onShutterButtonLongPressed() {
        return false;
    }

    @Override
    public boolean onBackPressed() {
        return false;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
            case KeyEvent.KEYCODE_VOLUME_DOWN:
            case KeyEvent.KEYCODE_DPAD_CENTER:
            case KeyEvent.KEYCODE_ENTER:
            case KeyEvent.KEYCODE_CAMERA:
                return true;
            default:
                break;
        }
        return false;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
            case KeyEvent.KEYCODE_VOLUME_DOWN:
            case KeyEvent.KEYCODE_DPAD_CENTER:
            case KeyEvent.KEYCODE_ENTER:
            case KeyEvent.KEYCODE_CAMERA:
                mIApp.getAppUi().triggerShutterButtonClick(LOWEST_PRIORITY);
                return true;
            default:
                break;
        }
        return false;
    }

    @Override
    public void onOrientationChanged(int orientation) {
    }

    @Override
    public CameraApi getCameraApi() {
        updateModeDefinedCameraApi();
        return mCameraApi;
    }

    @Override
    public DeviceUsage getDeviceUsage(@Nonnull DataStore dataStore, DeviceUsage oldDeviceUsage) {
        ArrayList<String> openedCameraIds = new ArrayList<>();
        String cameraId = getCameraIdByFacing(dataStore.getValue(
                KEY_CAMERA_SWITCHER, null, dataStore.getGlobalScope()));
        openedCameraIds.add(cameraId);
        updateModeDefinedCameraApi();
        return new DeviceUsage(DeviceUsage.DEVICE_TYPE_NORMAL, mCameraApi, openedCameraIds);
    }

    @Override
    public boolean isModeIdle() {
        return true;
    }

    @Override
    public boolean onUserInteraction() {
        if (mIApp != null) {
            mIApp.enableKeepScreenOn(false);
        }
        return true;
    }

    /**
     * Update mode defined camera api.
     */
    protected void updateModeDefinedCameraApi() {
        if (mCameraApi == null) {
            mCameraApi = CameraApiHelper.getCameraApiType(getClass().getSimpleName());
        }
    }

    /**
     * Get setting manager instance.
     *
     * @return the setting manager instance.
     */
    protected abstract ISettingManager getSettingManager();

    /**
     * Get the camera id according to the camera facing info.
     *
     * @param cameraFacing The input camera facing.
     * @return The camera id which has the input facing.
     */
    protected String getCameraIdByFacing(String cameraFacing) {
        String cameraId = BACK_CAMERA_ID;
        if (mIApp != null && mIApp.getActivity() != null) {
            Intent currentIntent = mIApp.getActivity().getIntent();
            if (currentIntent.getBooleanExtra(EXTRA_USE_FRONT_CAMERA_FOR_ANDROID, false) ||
                    currentIntent.getBooleanExtra(EXTRA_USE_FRONT_CAMERA_FOR_GOOGLE, false)) {
                cameraId = FRONT_CAMERA_ID;
                if (mDataStore != null) {
                    mDataStore.setValue(KEY_CAMERA_SWITCHER, CAMERA_FACING_FRONT,
                            mDataStore.getGlobalScope(), true);
                }
                LogHelper.i(mTag, "Open front camera only for test");
                return cameraId;
            }
        }

        if (cameraFacing == null || CAMERA_FACING_BACK.equals(cameraFacing)) {
            if (sSupportDualZoomModeList.contains(getModeKey())
                    && CameraUtil.getDualZoomId() != null) {
                cameraId = CameraUtil.getDualZoomId();
            } else {
                cameraId = BACK_CAMERA_ID;
            }
        } else if (CAMERA_FACING_FRONT.equals(cameraFacing)) {
            cameraId = FRONT_CAMERA_ID;
        }
        return cameraId;
    }

    protected void updateModeDeviceState(final String state) {
        mModeDeviceStatus = state;
        final String modeName = getClass().getSimpleName();
        ISettingManager settingManager = getSettingManager();
        if (settingManager != null) {
            settingManager.updateModeDeviceStateToSetting(modeName, state);
        }
        mIApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                LogHelper.d(mTag, "Change mode device state to " + state);
                mIApp.getAppUi().getShutterRootView().setContentDescription(
                        modeName + " is " + state);

            }
        });
    }

    protected String getModeDeviceStatus() {
        return mModeDeviceStatus;
    }

    /**
     * Need close camera sync:
     * device usage type is different.
     * camera id list is different:such as number or value is different.
     *
     * @return if normal is vsdof and stereo mode when exit camera the value is true.
     * change mode case,if next mode device type and api is same as is false,
     * other case is true.
     */
    protected boolean needCloseCameraSync() {
        String currentType = mCurrentModeDeviceUsage.getDeviceType();

        // if mNextModeDeviceUsage === null means just exit camera activity.
        if (mNextModeDeviceUsage == null) {
            //but stereo type need close camera sync.
            // normal case means async, such as normal photo/video/pip mode.
            boolean isStereo = DeviceUsage.DEVICE_TYPE_STEREO.equals(currentType);
            boolean isVsdof = DeviceUsage.DEVICE_TYPE_STEREO_VSDOF.equals(currentType);
            return isStereo || isVsdof;
        }

        //change mode case:
        //if device type and api both are same as, don't need close camera sync.
        boolean isSameType = currentType.equals(mNextModeDeviceUsage.getDeviceType());
        boolean isSameApi = mCurrentModeDeviceUsage.getCameraApi().equals(
                mNextModeDeviceUsage.getCameraApi());
        return !isSameType || !isSameApi || isTeleDevice();
    }

    /**
     * if current camera id equal back camera id or
     * front camera id, return false, else return true.
     * Tele device need close camera sync,
     * may be lead to open too much camera or surface
     * not ready.eg:panorama(tele) switch to PIP
     */
    private boolean isTeleDevice() {
        if (mDataStore == null) {
            LogHelper.i(mTag, "[isTeleDevice] null mDataStore!");
            return false;
        }
        String cameraId = mDataStore.getValue(KEY_CAMERA_SWITCHER, null,
                mDataStore.getGlobalScope());
        LogHelper.d(mTag, "[isTeleDevice] cameraId:" + cameraId);
        return !(BACK_CAMERA_ID.equals(cameraId) || FRONT_CAMERA_ID.equals(cameraId));
    }

    /**
     * Mode handler run in mode thread.
     */
    protected class ModeHandler extends Handler {
        public ModeHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_MODE_ON_SHUTTER_BUTTON_CLICK:
                    doShutterButtonClick();
                    break;

                default:
                    break;
            }
        }
    }

    protected boolean doShutterButtonClick() {
        return false;
    }
}