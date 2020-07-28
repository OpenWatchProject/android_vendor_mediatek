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
package com.mediatek.camera;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.hardware.Camera.Face;
import android.view.KeyEvent;
import android.view.Surface;
import android.view.View;


import com.mediatek.camera.ICameraMode.ActionType;
import com.mediatek.camera.ICameraMode.CameraModeType;
import com.mediatek.camera.ICameraMode.ModeState;
import com.mediatek.camera.mode.DummyMode;
import com.mediatek.camera.mode.ModeFactory;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.IFeatureConfig;
import com.mediatek.camera.platform.IFileSaver;
import com.mediatek.camera.platform.IFocusManager;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.platform.ISelfTimeManager;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingCtrl;
import com.mediatek.camera.util.Log;

public class ModuleManager {
    private static final String TAG = "ModuleManager";

    private final Activity mActivity;
    private CameraModeType mCurrentMode;

    private final ICameraContext mICameraContext = new CameraContextImpl();
    private final ICameraAppUi mICameraAppUi;
    private final IFeatureConfig mIFeatureConfig;
    private final IFileSaver mIFileSaver;
    private final ICameraDeviceManager mICameraDeviceManager;

    // TODO:the following variable is null when we do ModuleManager
    // constructor...
    private IModuleCtrl mIModuleCtrl;
    private IFocusManager mIFocusManager;
    private ICameraMode mICameraMode;
    private ISettingCtrl mISettingCtrl;
    private ISelfTimeManager mISelfTimeManager;

    private AdditionManager mAdditionManager;

    public ModuleManager(Activity activity, IFileSaver fileSaver, ICameraAppUi cameraAppUi,
            IFeatureConfig featureConfig, ICameraDeviceManager deviceManager,
            IModuleCtrl moduleCtrl, ISelfTimeManager iSelfTimeManager) {
        mActivity = activity;
        mICameraMode = new DummyMode();
        mIModuleCtrl = moduleCtrl;
        mICameraAppUi = cameraAppUi;
        mIFeatureConfig = featureConfig;
        mIFileSaver = fileSaver;
        mICameraDeviceManager = deviceManager;
        mISelfTimeManager = iSelfTimeManager;
        // mAdditionManager must be after assigned value for mIFeatureConfig
        mISettingCtrl = new SettingCtrl(mICameraContext);
        mAdditionManager = new AdditionManager(mICameraContext);
    }

    public void resume() {
        mICameraMode.resume();
        mAdditionManager.resume();
    }

    public void pause() {
        mICameraMode.pause();
        mAdditionManager.pause();
    }

    public void destory() {
        mAdditionManager.destory();
    }

    public void createMode(CameraModeType newMode) {
        Log.d(TAG, "[createMode],newMode:" + newMode + ",mCurrentMode:" + mCurrentMode);

        if (mCurrentMode == newMode) {
            return;
        }
        mICameraMode.close();
        mCurrentMode = newMode;
        mICameraMode = ModeFactory.getInstance().createMode(newMode, mICameraContext);
        mAdditionManager.setCurrentMode(newMode);
        mICameraMode.open();
    }

    public ISettingCtrl getSettingController() {
        return mISettingCtrl;
    }

    public boolean closeMode() {
        Log.d(TAG, "[closeMode]");
        mICameraMode.close();
        mICameraMode = new DummyMode();
        mCurrentMode = null;
        return true;
    }

    public ModeState getModeState() {
        return mICameraMode.getModeState();
    }

    public boolean onCameraOpen() {
        Log.d(TAG, "[onCameraOpen]...");
        mAdditionManager.execute(ActionType.ACTION_ON_CAMERA_OPEN, false);
        return mICameraMode.execute(ActionType.ACTION_ON_CAMERA_OPEN);
    }

    public void onCameraClose() {
        Log.d(TAG, "[onCameraClose]...");
        mAdditionManager.execute(ActionType.ACTION_ON_CAMERA_CLOSE, false);
        mICameraMode.execute(ActionType.ACTION_ON_CAMERA_CLOSE);
    }

    public void onCameraParameterReady(boolean isRestartPreview) {
        mAdditionManager.onCameraParameterReady(false);
        mICameraMode.execute(ActionType.ACTION_ON_CAMERA_PARAMETERS_READY, isRestartPreview);
    }

    public void setModuleCtrl(IModuleCtrl moduleCtrl) {
        mIModuleCtrl = moduleCtrl;
    }

    public void setFocusManager(IFocusManager focusManager) {
        mIFocusManager = focusManager;
    }

    public void onOrientationChanged(int orientation) {
        mAdditionManager.execute(ActionType.ACTION_ORITATION_CHANGED, false, orientation);
        mICameraMode.execute(ActionType.ACTION_ORITATION_CHANGED, orientation);
    }

    public void onCompensationChanged(int compensation) {
        mAdditionManager.execute(ActionType.ACTION_ON_COMPENSATION_CHANGED, false, compensation);
        mICameraMode.execute(ActionType.ACTION_ON_COMPENSATION_CHANGED, compensation);
    }

    public boolean onShutterButtonFocus(boolean pressed) {
        mAdditionManager.execute(ActionType.ACTION_SHUTTER_BUTTON_FOCUS, false, pressed);
        return mICameraMode.execute(ActionType.ACTION_SHUTTER_BUTTON_FOCUS, pressed);
    }

    public boolean onPhotoShutterButtonClick() {
        mAdditionManager.execute(ActionType.ACTION_PHOTO_SHUTTER_BUTTON_CLICK, false);
        return mICameraMode.execute(ActionType.ACTION_PHOTO_SHUTTER_BUTTON_CLICK);
    }

    public boolean onVideoShutterButtonClick() {
        mAdditionManager.execute(ActionType.ACTION_VIDEO_SHUTTER_BUTTON_CLICK, false);
        return mICameraMode.execute(ActionType.ACTION_VIDEO_SHUTTER_BUTTON_CLICK);
    }

    public boolean onShutterButtonLongPressed() {
        mAdditionManager.execute(ActionType.ACTION_SHUTTER_BUTTON_LONG_PRESS, false);
        return mICameraMode.execute(ActionType.ACTION_SHUTTER_BUTTON_LONG_PRESS);
    }

    public void onPreviewVisibilityChanged(int visibility) {
        mAdditionManager.execute(ActionType.ACTION_PREVIEW_VISIBLE_CHANGED, false, visibility);
        mICameraMode.execute(ActionType.ACTION_PREVIEW_VISIBLE_CHANGED, visibility);
    }

    public void onPreviewDisplaySizeChanged(int width, int height) {
        mAdditionManager.execute(ActionType.ACTION_ON_PREVIEW_DISPLAY_SIZE_CHANGED, false, width,
                height);
        mICameraMode.execute(ActionType.ACTION_ON_PREVIEW_DISPLAY_SIZE_CHANGED, width, height);
    }

    public void onPreviewBufferSizeChanged(int width, int height) {
        mICameraMode.execute(ActionType.ACTION_ON_PREVIEW_BUFFER_SIZE_CHANGED, width, height);
    }

    public void onEffectClick() {
        if (mCurrentMode == CameraModeType.EXT_MODE_PHOTO) {
            mAdditionManager.onEffectClick();
        }
    }

    /**
     * when PhotoModule or VideoModule have detected the click preview action
     * need transfer this to mode
     *
     * @param x
     *            : the position of x
     * @param y
     *            : the position of y
     * @return
     */
    public boolean onSingleTapUp(View view, int x, int y) {
        mAdditionManager.execute(ActionType.ACTION_ON_SINGLE_TAP_UP, false, view, x, y);
        return mICameraMode.execute(ActionType.ACTION_ON_SINGLE_TAP_UP, view, x, y);
    }

    public boolean onLongPress(View view, int x, int y) {
        mAdditionManager.execute(ActionType.ACTION_ON_LONG_PRESS, false, view, x, y);
        return mICameraMode.execute(ActionType.ACTION_ON_LONG_PRESS, view, x, y);
    }

    public boolean onOkButtonPress() {
        mAdditionManager.execute(ActionType.ACTION_OK_BUTTON_CLICK, false);
        return mICameraMode.execute(ActionType.ACTION_OK_BUTTON_CLICK);
    }

    public boolean onCancelButtonPress() {
        mAdditionManager.execute(ActionType.ACTION_CANCEL_BUTTON_CLICK, false);
        return mICameraMode.execute(ActionType.ACTION_CANCEL_BUTTON_CLICK);
    }

    public void setSurfaceTextureReady(boolean ready) {
        mICameraMode.execute(ActionType.ACTION_ON_SURFACE_TEXTURE_READY, ready);
    }

    public boolean onBackPressed() {
        boolean result = mAdditionManager.execute(ActionType.ACTION_ON_BACK_KEY_PRESS, false);
        result |= mICameraMode.execute(ActionType.ACTION_ON_BACK_KEY_PRESS);
        Log.i(TAG, "onBackPressed, reslult = " + result);
        return result;
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        mAdditionManager.execute(ActionType.ACTION_ON_KEY_EVENT_PRESS, false, keyCode, event);
        return mICameraMode.execute(ActionType.ACTION_ON_KEY_EVENT_PRESS, keyCode, event);
    }

    public void onMediaEject() {
        mICameraMode.execute(ActionType.ACTION_ON_MEDIA_EJECT);
    }

    public void onRestoreSettings() {
        mICameraMode.execute(ActionType.ACTION_ON_RESTORE_SETTINGS);
    }

    public boolean onUserInteraction() {
        return mICameraMode.execute(ActionType.ACTION_ON_USER_INTERACTION);
    }

    public void setDisplayRotation(int displayRotation) {
        mICameraMode.execute(ActionType.ACTION_SET_DISPLAYROTATION, displayRotation);
    }

    public void onFaceDetected(Face[] faces) {
        mICameraMode.execute(ActionType.ACTION_FACE_DETECTED, (Object[]) faces);
    }

    public void onSelfTimerState(boolean state) {
        mICameraMode.execute(ActionType.ACTION_ON_SELFTIMER_STATE, state);
    }

    public SurfaceTexture getBottomSurfaceTexture() {
        return mICameraMode.getBottomSurfaceTexture();
    }

    public SurfaceTexture getTopSurfaceTexture() {
        return mICameraMode.getTopSurfaceTexture();
    }

    public boolean isRestartCamera() {
        return mICameraMode.isRestartCamera();
    }

    public boolean isNeedDualCamera() {
        return mICameraMode.isNeedDualCamera();
    }

    public boolean isDisplayUseSurfaceView() {
        return mICameraMode.isDisplayUseSurfaceView();
    }

    public boolean isDeviceUseSurfaceView() {
        return mICameraMode.isDeviceUseSurfaceView();
    }

    public boolean startPreview(boolean isNeedStop) {
        return mICameraMode.execute(ActionType.ACTION_ON_START_PREVIEW, isNeedStop);
    }

    public boolean stopPreview() {
        return mICameraMode.execute(ActionType.ACTION_ON_STOP_PREVIEW);
    }

    // Click the setting icon ,need notify current mode,such as FaceBeauty
    public void onSettingContainerShowing(boolean isShowing) {
        mICameraMode.execute(ActionType.ACTION_ON_SETTING_BUTTON_CLICK, isShowing);
    }

    public boolean switchDevice() {
        mAdditionManager.execute(ActionType.ACTION_SWITCH_DEVICE, false);
        return mICameraMode.execute(ActionType.ACTION_SWITCH_DEVICE);
    }

    public void notifySurfaceViewDisplayIsReady() {
        mICameraMode.execute(ActionType.ACTION_NOTIFY_SURFCEVIEW_DISPLAY_IS_READY);
    }

    public void notifySurfaceViewDestroyed(Surface surface) {
        mICameraMode.execute(ActionType.ACTION_NOTIFY_SURFCEVIEW_DESTROYED, surface);
    }
    public void configurationChanged() {
        mICameraMode.execute(ActionType.ACTION_ON_CONFIGURATION_CHANGED);
    }

    public void setModeSettingValue(CameraModeType mode, String value) {
        String modekey = getSettingValue(mode);
        if (modekey != null) {
            mISettingCtrl.onSettingChanged(modekey, value);
        }
    }

    public void setContinuousShotEnable(boolean enable) {
        mAdditionManager.setContinuousShotEnable(enable);
    }

    public void setVideoRecorderEnable(boolean enable) {
        mICameraAppUi.setVideoShutterEnabled(enable);
        mICameraAppUi.updateVideoShutterStatues(enable);
        if (!enable) {
            mICameraMode.execute(ActionType.ACTION_DISABLE_VIDEO_RECORD);
        }
    }

    public void onCameraCloseDone() {
        mICameraDeviceManager.onCameraCloseDone();
    }

    /**
     * Check current whether can do auto focus or not.
     * @return true means can do auto focus; otherwise is false.
     */
    public boolean canDoAutoFocus() {
        return mICameraMode.execute(ActionType.ACTION_CAN_DO_AUTO_FOCUS);
    }

    private class CameraContextImpl implements ICameraContext {
        public IFileSaver getFileSaver() {
            return mIFileSaver;
        }

        public ICameraDeviceManager getCameraDeviceManager() {
            return mICameraDeviceManager;
        }

        public IModuleCtrl getModuleController() {
            return mIModuleCtrl;
        }

        public IFocusManager getFocusManager() {
            return mIFocusManager;
        }

        public ICameraAppUi getCameraAppUi() {
            return mICameraAppUi;
        }

        public IFeatureConfig getFeatureConfig() {
            return mIFeatureConfig;
        }

        @Override
        public Activity getActivity() {
            return mActivity;
        }

        @Override
        public ISettingCtrl getSettingController() {
            return mISettingCtrl;
        }

        @Override
        public ISelfTimeManager getSelfTimeManager() {
            return mISelfTimeManager;
        }

        @Override
        public AdditionManager getAdditionManager() {
            return mAdditionManager;
        }

    }

    private String getSettingValue(CameraModeType mode) {
        String key = null;
        switch (mode) {
        case EXT_MODE_PHOTO:
            key = SettingConstants.KEY_NORMAL;
            break;
        case EXT_MODE_PANORAMA:
            key = SettingConstants.KEY_PANORAMA;
            break;
        case EXT_MODE_VIDEO:
            key = SettingConstants.KEY_VIDEO;
            break;
        case EXT_MODE_VIDEO_PIP:
            key = SettingConstants.KEY_VIDEO_PIP;
            break;
        case EXT_MODE_PHOTO_PIP:
            key = SettingConstants.KEY_PHOTO_PIP;
            break;
        case EXT_MODE_FACE_BEAUTY:
            key = SettingConstants.KEY_FACE_BEAUTY;
            break;
        case EXT_MODE_STEREO_CAMERA:
            key = SettingConstants.KEY_REFOCUS;
            break;
        case EXT_MODE_VIDEO_STEREO:
            key = SettingConstants.KEY_VIDEO_STEREO;
            break;
        case EXT_MODE_PHOTO_STEREO:
            key = SettingConstants.KEY_PHOTO_STEREO;
            break;
        default:
            break;
        }
        return key;
    }
}
