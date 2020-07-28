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

import android.graphics.SurfaceTexture;
import android.hardware.Camera.AFDataCallback;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.hardware.Camera.Size;
import android.os.SystemProperties;
import android.view.MotionEvent;
import android.view.Surface;

import com.android.camera.FeatureSwitcher;
import com.android.camera.R;

import com.mediatek.camera.ICameraAddition.AdditionActionType;
import com.mediatek.camera.ICameraMode.ModeState;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.mode.PhotoMode;
import com.mediatek.camera.mode.pip.PipController.State;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.AutoFocusMvCallback;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.util.Log;

import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class PipPhotoMode extends PhotoMode implements PipController.Listener,
        ICameraAppUi.GestureListener {
    private static final String TAG = "PipPhotoMode";

    public static final int UNKNOWN = -1;
    public static final int MSG_SET_PREVIEW_ASPECT_RATIO = 2;
    public static final int MSG_CAMERA_PARAMETERS_READY = 3;

    private int mBottomPictureWidth;
    private int mBottomPictureHeight;
    private int mTopPictureWidth;
    private int mTopPictureHeight;
    private int mCaptureOrientation;
    private boolean mIsGestureEnable = false;
    private boolean mModeOpened = false;
    private static int mIsSaveRawJpegEnable = SystemProperties.getInt(
            "camera.pip.save.raw.jpeg.enable", 0);

    /*********************** Communicate with pip controller ******************************/
    private final PipController mPipController;

    public PipPhotoMode(ICameraContext cameraContext) {
        super(cameraContext);
        Log.d(TAG, "[PipPhotoMode]constructor...");
        mModeOpened = true;
        mCameraCategory = new PipCameraCategory();
        mICameraAppUi.setGestureListener(this);
        mPipController = PipController.instance(cameraContext.getActivity());
        mPipController.init(cameraContext, this);
        setPipSettingRules(mICameraContext);
    }

    @Override
    public void resume() {
        Log.d(TAG, "[resume]");
        mModeOpened = true;
        mPipController.resume();
    }

    @Override
    public void pause() {
        Log.d(TAG, "[pause]");
        mModeOpened = false;
        mPipController.pause();
    }

    @Override
    public boolean isRestartCamera() {
        return true;
    }

    @Override
    public boolean isNeedDualCamera() {
        Log.d(TAG, "isNeedDualCamera");
        return true;
    }

    @Override
    public boolean open() {
        Log.d(TAG, "[open]...");
        return true;
    }

    @Override
    public boolean close() {
        Log.d(TAG, "[close]...");
        mModeOpened = false;
        mICameraAppUi.setSwipeEnabled(true);
        mICameraAppUi.restoreViewState();
        // pip <--> pip, there is no need to recreate pip controller
        if (CameraModeType.EXT_MODE_VIDEO_PIP != mIModuleCtrl.getNextMode()) {
            mPipController.unInit(mICameraContext.getActivity());
        }
        mPipController.stopSwitchPip();
        super.close();
        return true;
    }

    @Override
    public boolean execute(ActionType type, Object... arg) {
        Log.d(TAG, "[execute]type = " + type);
        mAdditionManager.execute(type, true, arg);
        boolean returnValue = true;
        switch (type) {
        case ACTION_ON_CAMERA_OPEN:
            super.updateDevice();
            mCameraClosed = false;
            break;

        case ACTION_ORITATION_CHANGED:
            mPipController.onGSensorOrientationChanged((Integer) arg[0]);
            break;

        case ACTION_ON_COMPENSATION_CHANGED:
            mPipController.onViewOrienationChanged((Integer) arg[0]);
            break;

        case ACTION_SET_DISPLAYROTATION:
            setDisplayOrientation((Integer) arg[0]);
            break;

        case ACTION_ON_BACK_KEY_PRESS:
            if (mPipController.isPipEffectShowing()) {
                mPipController.closeEffects();
                return true;
            }
            return false;

        case ACTION_ON_PREVIEW_BUFFER_SIZE_CHANGED:
            onPreviewBufferSizeChanged((Integer) arg[0], (Integer) arg[1]);
            break;

        case ACTION_SWITCH_DEVICE:
            switchDevice();
            return false;

        case ACTION_NOTIFY_SURFCEVIEW_DISPLAY_IS_READY:
            mPipController.setPreviewSurface(mIModuleCtrl.getPreviewSurface());
            break;

        case ACTION_NOTIFY_SURFCEVIEW_DESTROYED:
            mPipController.notifySurfaceViewDestroyed((Surface) arg[0]);
            break;

        case ACTION_FACE_DETECTED:
            // Do-Noting,Because not need show super's entry FB icon
            break;

        case ACTION_ON_FULL_SCREEN_CHANGED:
            mPipController.hideModeViews(!(Boolean) arg[0]);
            break;

        case ACTION_ON_SETTING_BUTTON_CLICK:
            mIsGestureEnable = !(Boolean) arg[0];
            mPipController.hideModeViews((Boolean) arg[0]);
            break;

        case ACTION_SHUTTER_BUTTON_LONG_PRESS:
            mICameraAppUi.showInfo(mActivity
                    .getString(R.string.pip_continuous_not_supported));
            break;

        case ACTION_ON_SELFTIMER_STATE:
            mIsGestureEnable = !(Boolean) arg[0];
            mPipController.hideModeViews((Boolean) arg[0]);
            return false;

        case ACTION_ON_CAMERA_PARAMETERS_READY:
            super.executeAction(type, arg);
            mPipController.setState(State.STATE_IDLE);
            break;

        case ACTION_PHOTO_SHUTTER_BUTTON_CLICK:
            if (State.STATE_IDLE == mPipController.getState()) {
                mPipController.closeEffects();
                super.executeAction(type, arg);
            }
            break;

        case ACTION_CAN_DO_AUTO_FOCUS:
            returnValue = ModeState.STATE_CAPTURING != getModeState();
            break;

        default:
            super.executeAction(type, arg);
            break;
        }

        return returnValue;
    }

    @Override
    public int getGSensorOrientation() {
        return mIModuleCtrl.getOrientation();
    }

    @Override
    public int getViewRotation() {
        return mIModuleCtrl.getOrientationCompensation();
    }

    @Override
    public int getButtomGraphicCameraId() {
        return mICameraDeviceManager.getCurrentCameraId();
    }

    @Override
    public void switchPIP() { // switch big and smaller picture
        Log.d(TAG, "[switchPIP]...");
        setAfDataCallback(null);
        if (mIFocusManager != null) {
            mIFocusManager.cancelAutoFocus();
            mIFocusManager.clearFocusAndFaceUi();
        }
        setAfMvCallback(null);
        mIModuleCtrl.switchCameraDevice();
        mIModuleCtrl.applyFocusParameters(false);
        updateDevice();
        setAfMvCallback(mAutoFocusMoveCallback);
        setAfDataCallback(mAFDataCallback);
    }

    @Override
    public void onPIPPictureTaken(byte[] jpegData) {
        Log.d(TAG, "[onPIPPictureTaken]...");
        if (jpegData == null) {
            Log.i(TAG, "[onPIPPictureTaken]jpegData is null,return!");
            return;
        }
        mIFileSaver.savePhotoFile(jpegData, null, mCaptureStartTime,
                mIModuleCtrl.getLocation(), 0, null);
        jpegData = null;
        System.gc();
    }

    @Override
    public void canDoStartPreview() {
        Log.d(TAG, "[canDoStartPreview]mCameraClosed = " + mCameraClosed
                + ", mModeResumed = " + mModeOpened);
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (!mCameraClosed && mModeOpened) {
                    restartPreview(false);
                }
            }
        });
    }

    @Override
    public boolean isDisplayUseSurfaceView() {
        Log.d(TAG, "[isDisplayUseSurfaceView]");
        return true;
    }

    @Override
    public boolean isDeviceUseSurfaceView() {
        Log.d(TAG, "[isDeviceUseSurfaceView]");
        return false;
    }

    @Override
    public SurfaceTexture getBottomSurfaceTexture() {
        return mPipController.getBottomSurfaceTexture();
    }

    @Override
    public SurfaceTexture getTopSurfaceTexture() {
        return mPipController.getTopSurfaceTexture();
    }

    @Override
    public void autoFocus() {
        Log.d(TAG, "[autoFocus]");
        super.autoFocus();
        mPipController.closeEffects();
    }

    @Override
    public void cancelAutoFocus() {
        Log.d(TAG, "[cancelAutoFocus]");
        super.cancelAutoFocus();
    }

    @Override
    protected void startPreview(boolean needStop) {
        Log.d(TAG, "[startPreview] needStop = " + needStop);
        mPipController.stopSwitchPip();
        super.startPreview(needStop);
        mICameraDeviceManager.getCameraDevice(getTopCameraId()).startPreview();
        mIsGestureEnable = true;
    }

    @Override
    protected void stopPreview() {
        Log.d(TAG, "[stopPreview]...");
        mPipController.stopSwitchPip();
        mIsGestureEnable = false;
        super.stopPreview();
        mICameraDeviceManager.getCameraDevice(getTopCameraId()).stopPreview();
    }

    @Override
    protected PictureCallback getUncompressedImageCallback() {
        return null;
    }
    private void setDisplayOrientation(int displayRotation) {
        Log.d(TAG, "setDisplayOrientation displayRotation = " + displayRotation);
        mPipController.setDisplayRotation(displayRotation);
    }

    private void onPreviewBufferSizeChanged(int width, int height) {
        mPipController.setPreviewTextureSize(width, height);
    }

    private int getTopCameraId() {
        return mICameraDeviceManager.getCurrentCameraId() == mICameraDeviceManager
                .getBackCameraId() ? mICameraDeviceManager.getFrontCameraId()
                : mICameraDeviceManager.getBackCameraId();
    }

    private PictureCallback mBottomJpegPictureCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] jpegdata,
                android.hardware.Camera camera) {
            Log.d(TAG, "[onPictureTaken]mBottomJpegPictureCallback...");
            if (mPipController == null) {
                Log.e(TAG,
                        "[onPictureTaken]mBottomJpegPictureCallback,mPipController is null!");
                return;
            }
            mPipController.takePicture(jpegdata, mBottomPictureWidth,
                    mBottomPictureHeight, true, mCaptureOrientation);
            // add for save jpeg
            if (mIsSaveRawJpegEnable > 0) {
                saveRawJpeg(jpegdata, "/sdcard/bottom.jpg");
            }
            jpegdata = null;
        }
    };

    private final ShutterCallback mShutterCallback = new ShutterCallback() {
        @Override
        public void onShutter() {
        }
    };

    private PictureCallback mTopJpegPictureCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] jpegdata,
                android.hardware.Camera camera) {
            Log.d(TAG, "[onPictureTaken]mTopJpegPictureCallback...");
            if (mPipController == null) {
                Log.e(TAG,
                        "[onPictureTaken]mTopJpegPictureCallback,mPipController is null!");
                return;
            }
            mPipController.takePicture(jpegdata, mTopPictureWidth,
                    mTopPictureHeight, false, mCaptureOrientation);
            // add for save jpeg
            if (mIsSaveRawJpegEnable > 0) {
                saveRawJpeg(jpegdata, "/sdcard/top.jpg");
            }
            jpegdata = null;
        }
    };

    private void saveRawJpeg(byte[] jepgdata, String path) {
        Log.d(TAG, "[saveRawJpeg]path = " + path);
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(path);
            out.write(jepgdata);
            out.close();
        } catch (IOException e) {
            Log.e(TAG, "[saveRawJpeg]Failed to write image,exception:", e);
        } finally {
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    Log.e(TAG, "[saveRawJpeg]ioexception:", e);
                }
            }
        }
    }

    class PipCameraCategory extends CameraCategory {
        private PipCameraCategory() {
        }

        @Override
        public void takePicture() {
            Log.d(TAG, "[takePicture]...");
            mPipController.stopSwitchPip();
            mCaptureOrientation = mIModuleCtrl.getOrientation();

            updateTopPictureSize();
            updateBottomPictureSize();
            mPipController.setPictureSize(
                    new android.util.Size(mBottomPictureWidth, mBottomPictureHeight),
                    new android.util.Size(mTopPictureWidth, mTopPictureHeight));

            // take top picture Async
            mAdditionManager.execute(AdditionActionType.ACTION_TAKEN_PICTURE);
            mICameraDeviceManager.getCameraDevice(getTopCameraId())
                    .takePictureAsync(mShutterCallback, null, null,
                            mTopJpegPictureCallback);
            // take bottom picture
            mICameraDeviceManager.getCameraDevice(
                    mICameraDeviceManager.getCurrentCameraId()).takePicture(
                    null, null, null, mBottomJpegPictureCallback);
            mICameraAppUi.setViewState(ViewState.VIEW_STATE_CAPTURE);
        }
    }

    private void updateBottomPictureSize() {
        Log.d(TAG, "[updateBottomPictureSize]...");
        Size size = mICameraDeviceManager
                .getCameraDevice(mICameraDeviceManager.getCurrentCameraId())
                .getParameters().getPictureSize();
        if (size == null) {
            Log.d(TAG, "updateBottomPictureSize size==null");
            return;
        }
        if (mCaptureOrientation % 180 == 0) {
            mBottomPictureWidth = size.height;
            mBottomPictureHeight = size.width;
        } else {
            mBottomPictureWidth = size.width;
            mBottomPictureHeight = size.height;
        }
    }

    private void updateTopPictureSize() {
        Log.d(TAG, "[updateTopPictureSize]...");
        Size size = mICameraDeviceManager.getCameraDevice(getTopCameraId())
                .getParameters().getPictureSize();
        if (size == null) {
            Log.w(TAG, "[updateTopPictureSize]size == null");
            return;
        }
        if (mCaptureOrientation % 180 == 0) {
            mTopPictureWidth = size.height;
            mTopPictureHeight = size.width;
        } else {
            mTopPictureWidth = size.width;
            mTopPictureHeight = size.height;
        }
    }

    private void setAfMvCallback(AutoFocusMvCallback autoFocusMvCb) {
        Log.d(TAG, "[setAfMvCallback]...mICameraDevice = " + mICameraDevice);
        if (mICameraDevice != null) {
            mICameraDevice.setAutoFocusMoveCallback(autoFocusMvCb);
        }
    }

    private void setAfDataCallback(AFDataCallback afDataCallback) {
        String value = mISettingCtrl.getSettingValue(SettingConstants.KEY_AF);
        if (mActivity.getResources().getString(R.string.af_multi_mode).equals(value)
                && mICameraDevice != null && mICameraDevice.getCamera() != null) {
            mICameraDevice.getCamera().setAFDataCallback(afDataCallback);
        }
    }

    private void setPipSettingRules(ICameraContext cameraContext) {
        Log.d(TAG, "[setPipSettingRules]...");
        PipVideoQualityRule videoQualityRule = new PipVideoQualityRule(
                cameraContext, SettingConstants.KEY_PHOTO_PIP);
        videoQualityRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_PHOTO_PIP,
                SettingConstants.KEY_VIDEO_QUALITY, videoQualityRule);

        PipPreviewSizeRule previewSizeRule = new PipPreviewSizeRule(
                cameraContext);
        previewSizeRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_PHOTO_PIP,
                SettingConstants.KEY_PICTURE_RATIO, previewSizeRule);

        PipPictureSizeRule pictureSizeRule = new PipPictureSizeRule(
                cameraContext);
        pictureSizeRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_PHOTO_PIP,
                SettingConstants.KEY_PICTURE_SIZE, pictureSizeRule);

//        PipAntiFlickRule antiflickRule = new PipAntiFlickRule(cameraContext);
//        antiflickRule.addLimitation("on", null, null);
//        mISettingCtrl.addRule(SettingConstants.KEY_PHOTO_PIP,
//                SettingConstants.KEY_ANTI_BANDING, antiflickRule);

        PipZsdRule zsdRule = new PipZsdRule(cameraContext);
        zsdRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_PHOTO_PIP,
                SettingConstants.KEY_CAMERA_ZSD, zsdRule);

//        FDRule fdRule = new FDRule(cameraContext);
//        fdRule.addLimitation("on", null, null);
//        mISettingCtrl.addRule(SettingConstants.KEY_PHOTO_PIP,
//                SettingConstants.KEY_CAMERA_FACE_DETECT, fdRule);

        PipFlashRule flashRule = new PipFlashRule(cameraContext);
        flashRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_PHOTO_PIP,
                SettingConstants.KEY_FLASH, flashRule);

        PipCameraModeRule cameraModeRule = new PipCameraModeRule(cameraContext);
        cameraModeRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_PHOTO_PIP,
                SettingConstants.KEY_CAMERA_MODE, cameraModeRule);
    }

    private class PipAntiFlickRule implements ISettingRule {
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
        private boolean mSwitchingPip = false;
        private String mCurrentAntiFlickValue = null;

        public PipAntiFlickRule(ICameraContext cameraContext) {
            Log.d(TAG, "[PipAntiFlickRule]constructor...");
            mCameraContext = cameraContext;
        }

        @Override
        public void execute() {
            deviceManager = mCameraContext.getCameraDeviceManager();
            mBackCamDevice = deviceManager.getCameraDevice(deviceManager
                    .getCurrentCameraId());
            if (mTopCamDevice != null) {
                mSwitchingPip = (mTopCamDevice.getCameraId() == deviceManager
                        .getCurrentCameraId());
            } else {
                mSwitchingPip = false;
            }
            mTopCamDevice = deviceManager.getCameraDevice(getTopCameraId());
            mISettingCtrl = mCameraContext.getSettingController();
            mParameters = mBackCamDevice.getParameters();
            if (mTopCamDevice != null) {
                mTopParameters = mTopCamDevice.getParameters();
            }
            String conditionValue = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_PHOTO_PIP);
            int index = conditionSatisfied(conditionValue);
            String antiFlickValue = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_ANTI_BANDING);
            Log.d(TAG, "[execute]PipAntiFlickRule index = " + index
                    + " antiFlickValue = " + antiFlickValue
                    + " mSwitchingPip = " + mSwitchingPip);
            if (index == -1) {
                mParameters.setAntibanding(antiFlickValue);
            } else {
                if (mSwitchingPip) {
                    antiFlickValue = mCurrentAntiFlickValue;
                    mISettingCtrl.setSettingValue(
                            SettingConstants.KEY_ANTI_BANDING, antiFlickValue,
                            deviceManager.getCurrentCameraId());
                    ListPreference pref = mISettingCtrl
                            .getListPreference(SettingConstants.KEY_ANTI_BANDING);
                    if (pref != null) {
                        pref.setValue(antiFlickValue);
                    }
                }
                mCurrentAntiFlickValue = antiFlickValue;
                mParameters.setAntibanding(antiFlickValue);
                if (mTopParameters != null) {
                    mTopParameters.setAntibanding(antiFlickValue);
                }
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
    }

    private class PipZsdRule implements ISettingRule {
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
        private boolean mSwitchingPip = false;
        private String mCurrentZsdValue = null;

        public PipZsdRule(ICameraContext cameraContext) {
            Log.d(TAG, "[PipZsdRule]constructor...");
            mCameraContext = cameraContext;
        }

        @Override
        public void execute() {
            deviceManager = mCameraContext.getCameraDeviceManager();
            mBackCamDevice = deviceManager.getCameraDevice(deviceManager
                    .getCurrentCameraId());
            if (mTopCamDevice != null) {
                mSwitchingPip = (mTopCamDevice.getCameraId() == deviceManager
                        .getCurrentCameraId());
            } else {
                mSwitchingPip = false;
            }
            mTopCamDevice = deviceManager.getCameraDevice(getTopCameraId());
            mISettingCtrl = mCameraContext.getSettingController();
            mParameters = mBackCamDevice.getParameters();
            if (mTopCamDevice != null) {
                mTopParameters = mTopCamDevice.getParameters();
            }

            String previewRatio = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_PICTURE_RATIO);
            String conditionValue = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_PHOTO_PIP);
            int index = conditionSatisfied(conditionValue);
            if (MTK_CHIP_0321.equals(mICameraContext.getFeatureConfig().whichDeanliChip())
                    || mICameraContext.getFeatureConfig().isGmoRamOptSupport()) {
                pipDenaliZSDRule(index);
                return;
            }
            String zsdValue = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_CAMERA_ZSD);
            if (zsdValue == null) {
                Log.d(TAG, "[PipZsdRule.execute] don't support zsd!");
                return;
            }
            if (mSwitchingPip) {
                zsdValue = mCurrentZsdValue;
                mISettingCtrl.setSettingValue(SettingConstants.KEY_CAMERA_ZSD,
                        zsdValue, deviceManager.getCurrentCameraId());
                ListPreference pref = mISettingCtrl
                        .getListPreference(SettingConstants.KEY_CAMERA_ZSD);
                if (pref != null) {
                    pref.setValue(zsdValue);
                }
            }
            mCurrentZsdValue = zsdValue;
            Log.d(TAG, "[execute]PipZsdRule index = " + index);
            if (index == -1) {
                mParameters.setZSDMode(zsdValue);
            } else {
                mParameters.setZSDMode(zsdValue);
                if (mTopParameters != null) {
                    mTopParameters.setZSDMode(zsdValue);
                }
                SettingUtils.setPipPreviewSize(mCameraContext.getActivity(),
                        mParameters, mTopParameters, mISettingCtrl,
                        previewRatio);

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
    }

    private class PipFlashRule implements ISettingRule {
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

        public PipFlashRule(ICameraContext cameraContext) {
            Log.d(TAG, "[PipFlashRule]constructor...");
            mCameraContext = cameraContext;
        }

        @Override
        public void execute() {
            deviceManager = mCameraContext.getCameraDeviceManager();
            mBackCamDevice = deviceManager.getCameraDevice(deviceManager
                    .getCurrentCameraId());
            mTopCamDevice = deviceManager.getCameraDevice(getTopCameraId());
            mISettingCtrl = mCameraContext.getSettingController();
            mParameters = mBackCamDevice.getParameters();
            if (mTopCamDevice != null) {
                mTopParameters = mTopCamDevice.getParameters();
            }

            String conditionValue = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_PHOTO_PIP);
            int index = conditionSatisfied(conditionValue);
            String currentFlashValue = mISettingCtrl.getSetting(
                    SettingConstants.KEY_FLASH,
                    deviceManager.getCurrentCameraId()).getValue();
            SettingItem flashSetting = mISettingCtrl.getSetting(
                    SettingConstants.KEY_FLASH, getTopCameraId());
            String topFlashValue = null;
            if (flashSetting != null) {
                topFlashValue = flashSetting.getValue();
            }
            Log.d(TAG, "[execute]PipFlashRule index = " + index);
            if (index == -1) {
                // do nothing
            } else {
                Log.d(TAG, "[execute]PipFlashRule currentFlashValue = "
                        + currentFlashValue + ", topFlashValue = "
                        + topFlashValue);
                if (currentFlashValue != null) {
                    mParameters.setFlashMode(currentFlashValue);
                }
                if (mTopParameters != null && topFlashValue != null) {
                    mTopParameters.setFlashMode(topFlashValue);
                }
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
    }

    private class PipCameraModeRule implements ISettingRule {
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

        public PipCameraModeRule(ICameraContext cameraContext) {
            Log.d(TAG, "[PipCameraModeRule]constructor...");
            mCameraContext = cameraContext;
        }

        @Override
        public void execute() {
            deviceManager = mCameraContext.getCameraDeviceManager();
            mBackCamDevice = deviceManager.getCameraDevice(deviceManager
                    .getCurrentCameraId());
            mTopCamDevice = deviceManager.getCameraDevice(getTopCameraId());
            mISettingCtrl = mCameraContext.getSettingController();
            mParameters = mBackCamDevice.getParameters();
            if (mTopCamDevice != null) {
                mTopParameters = mTopCamDevice.getParameters();
            }

            String conditionValue = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_PHOTO_PIP);
            int index = conditionSatisfied(conditionValue);
            int currentCameraMode = Parameters.CAMERA_MODE_MTK_PRV;
            Log.d(TAG, "[execute]PipCameraModeRule index = " + index);
            if (index == -1) {
                // do nothing
            } else {
                Log.d(TAG, "[execute]PipCameraModeRule currentCameraMode = "
                        + currentCameraMode);
                mParameters.setCameraMode(currentCameraMode);
                if (mTopParameters != null) {
                    mTopParameters.setCameraMode(currentCameraMode);
                }
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
    }

    private class FDRule implements ISettingRule {
        private List<String> mConditions = new ArrayList<String>();
        private ISettingCtrl mISettingCtrl;
        private ICameraContext mCameraContext;

        public FDRule(ICameraContext cameraContext) {
            mCameraContext = cameraContext;
        }

        @Override
        public void execute() {
            mISettingCtrl = mCameraContext.getSettingController();
            String conditionValue = mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_PHOTO_PIP);
            int index = conditionSatisfied(conditionValue);
            Log.d(TAG, "[execute] FDRule index = " + index);
            SettingItem fdSetting = mISettingCtrl
                    .getSetting(SettingConstants.KEY_CAMERA_FACE_DETECT);
            ListPreference pref = mISettingCtrl
                    .getListPreference(SettingConstants.KEY_CAMERA_FACE_DETECT);
            if (index == -1) {
                int overrideCount = fdSetting.getOverrideCount();
                Record record = fdSetting
                        .getOverrideRecord(SettingConstants.KEY_PHOTO_PIP);
                if (record == null) {
                    return;
                }
                fdSetting.removeOverrideRecord(SettingConstants.KEY_PHOTO_PIP);
                overrideCount--;
                String value = null;
                String overrideValue = null;
                if (overrideCount > 0) {
                    Record topRecord = fdSetting.getTopOverrideRecord();
                    if (topRecord != null) {
                        value = topRecord.getValue();
                        overrideValue = topRecord.getOverrideValue();
                    }
                } else {
                    if (pref != null) {
                        value = pref.getValue();
                    }
                }
                mISettingCtrl.setSettingValue(
                        SettingConstants.KEY_CAMERA_FACE_DETECT, value,
                        mICameraDeviceManager.getCurrentCameraId());
                if (pref != null) {
                    pref.setOverrideValue(overrideValue);
                }
            } else if (!enableFD()) {
                mISettingCtrl.setSettingValue(
                        SettingConstants.KEY_CAMERA_FACE_DETECT, "off",
                        mICameraDeviceManager.getCurrentCameraId());
                if (pref != null) {
                    pref.setOverrideValue("off");
                }
                Record record = fdSetting.new Record("off", "off");
                fdSetting.addOverrideRecord(SettingConstants.KEY_PHOTO_PIP,
                        record);
            }
        }

        @Override
        public void addLimitation(String condition, List<String> result,
                MappingFinder mappingFinder) {
            Log.d(TAG, "[addLimitation]condition = " + condition);
            mConditions.add(condition);
        }

        private int conditionSatisfied(String conditionValue) {
            int index = mConditions.indexOf(conditionValue);
            return index;
        }
    }

    private void switchDevice() {
        Log.d(TAG, "[switchDevice]...");
        if (mPipController != null) {
            mPipController.switchPIP();
        }
    }

    private static final String MTK_CHIP_0321 = "0321";

    private void pipDenaliZSDRule(int index) {
        SettingItem zsdSetting = mISettingCtrl
                .getSetting(SettingConstants.KEY_CAMERA_ZSD);
        String resultValue = zsdSetting.getValue();
        int type = zsdSetting.getType();
        if (mICameraContext.getFeatureConfig().isGmoRamOptSupport()) {
            Log.i(TAG, "Gmo rom pip not support zsd");
        } else if (!MTK_CHIP_0321.equals(mICameraContext.getFeatureConfig()
                .whichDeanliChip())
                || mICameraContext.getFeatureConfig().isLowRamOptSupport()) {
            return;
        }
        if (index != -1) {
            if (zsdSetting.isEnable()) {
                setResultSettingValue(type, "off", "off", true,
                        zsdSetting);
            }
            Record record = zsdSetting.new Record(resultValue, "off");
            zsdSetting
                    .addOverrideRecord(SettingConstants.KEY_PHOTO_PIP, record);
        } else {
            int overrideCount = zsdSetting.getOverrideCount();
            Record record = zsdSetting
                    .getOverrideRecord(SettingConstants.KEY_PHOTO_PIP);
            if (record == null) {
                return;
            }
            Log.d(TAG, "overrideCount:" + overrideCount);
            zsdSetting.removeOverrideRecord(SettingConstants.KEY_PHOTO_PIP);
            overrideCount--;

            if (overrideCount > 0) {
                Record topRecord = zsdSetting.getTopOverrideRecord();
                if (topRecord != null) {
                    if (zsdSetting.isEnable()) {
                        String value = topRecord.getValue();
                        String overrideValue = topRecord.getOverrideValue();
                        // may be the setting's value is changed, the value in
                        // record is old.
                        ListPreference pref = zsdSetting.getListPreference();
                        if (pref != null
                                && SettingUtils.isBuiltList(overrideValue)) {
                            pref.setEnabled(true);
                            String prefValue = pref.getValue();
                            List<String> list = SettingUtils
                                    .getEnabledList(overrideValue);
                            if (list.contains(prefValue)) {
                                if (!prefValue.equals(value)) {
                                    String[] values = new String[list.size()];
                                    overrideValue = SettingUtils
                                            .buildEnableList(
                                                    list.toArray(values),
                                                    prefValue);
                                }
                                value = prefValue;
                            }
                        }
                        setResultSettingValue(type, value, overrideValue, true,
                                zsdSetting);
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

    private boolean enableFD() {
        // default main camera support FD, customer can custom which camera
        // supports FD
        boolean enable = mICameraDeviceManager.getCameraDevice(mICameraDeviceManager
                .getCurrentCameraId()) == mICameraDeviceManager
                .getCameraDevice(mICameraDeviceManager.getBackCameraId());
        Log.d(TAG, "[enableFD] = " + enable);
        return enable;
    }

    private boolean disableGesture() {
        return !mIsGestureEnable || mICameraAppUi.isSettingShowing()
                || getModeState() == ModeState.STATE_FOCUSING;
    }

    /***************************** Gesture Listener **************************************/
    @Override
    public boolean onDown(float x, float y, int width, int height) {
        if (disableGesture()) {
            return true;
        }
        if (mPipController != null) {
            return mPipController.onDown(x, y, width, height);
        }
        return false;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
            float velocityY) {
        return false;
    }

    @Override
    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        if (mPipController != null) {
            return mPipController.onScroll(dx, dy, totalX, totalY);
        }
        return false;
    }

    @Override
    public boolean onSingleTapUp(float x, float y) {
        if (mPipController != null && getModeState() != ModeState.STATE_FOCUSING) {
            mAdditionManager.execute(AdditionActionType.ACTION_ON_SWITCH_PIP);
            return mPipController.onSingleTapUp(x, y);
        }
        return false;
    }

    @Override
    public boolean onSingleTapConfirmed(float x, float y) {
        return false;
    }

    @Override
    public boolean onUp() {
        if (mPipController != null) {
            return mPipController.onUp();
        }
        return false;
    }

    @Override
    public boolean onDoubleTap(float x, float y) {
        return false;
    }

    @Override
    public boolean onScale(float focusX, float focusY, float scale) {
        return false;
    }

    @Override
    public boolean onScaleBegin(float focusX, float focusY) {
        return false;
    }

    @Override
    public boolean onLongPress(float x, float y) {
        if (mPipController != null && getModeState() != ModeState.STATE_FOCUSING) {
            return mPipController.onLongPress(x, y);
        }
        return false;
    }

}
