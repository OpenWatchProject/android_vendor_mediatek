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

package com.mediatek.camera.mode.facebeauty;

import android.hardware.Camera;
import android.hardware.Camera.AFDataCallback;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.Face;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;

import com.android.camera.R;

import com.mediatek.camera.AdditionManager;
import com.mediatek.camera.ICameraAddition;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ICameraAddition.AdditionActionType;
import com.mediatek.camera.mode.CameraMode;
import com.mediatek.camera.mode.facebeauty.FaceBeautyParametersHelper.ParameterListener;
import com.mediatek.camera.platform.ICameraAppUi.SpecViewType;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.AutoFocusMvCallback;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.cFbOriginalCallback;
import com.mediatek.camera.platform.ICameraView;
import com.mediatek.camera.platform.IFocusManager;
import com.mediatek.camera.platform.IFileSaver.FILE_TYPE;
import com.mediatek.camera.platform.IFileSaver.OnFileSavedListener;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;

import junit.framework.Assert;

import java.util.ArrayList;

public class FaceBeautyMode extends CameraMode implements ICameraAddition.Listener,
        IFocusManager.FocusListener {
    private static final String TAG = "FaceBeautyMode";

    protected static final int SUPPORTED_FB_PROPERTIES_MAX_NUMBER = 4;

    protected static final int ON_CAMERA_PARAMETERS_READY = 101;
    protected static final int INFO_FACE_DETECTED = 102;
    protected static final int ORIENTATION_CHANGED = 103;
    protected static final int ON_BACK_PRESSED = 104;
    protected static final int HIDE_EFFECTS_ITEM = 105;
    protected static final int ON_FULL_SCREEN_CHANGED = 106;
    protected static final int ON_CAMERA_CLOSED = 107;
    protected static final int ON_SETTING_BUTTON_CLICK = 108;
    protected static final int ON_LEAVE_FACE_BEAUTY_MODE = 109;
    protected static final int ON_SELFTIMER_CAPTUEING = 110;
    protected static final int IN_PICTURE_TAKEN_PROGRESS = 111;
    protected static final int REMVOE_BACK_TO_NORMAL = 112;

    private static final int ON_CONFIGURATION_CHANGED = 201;

    private CfbCallback mCfbCallback = new CfbCallback();
    private Handler mHandler;
    private FaceBeautyPreviewSize mFaceBeautyPreviewSize;
    private FaceBeautyParametersHelper mFaceBeautyParametersHelper;
    private ParameterListener mParameterListener;

    private ICameraView mICameraView;
    private ArrayList<Integer> mVfbFacesPoint = new ArrayList<Integer>();
    private AdditionManager mAdditionManager;

    private boolean mIsAutoFocusCallback = false;
    // record the image count in compressing stage, it will increase when onCanCapture,
    // and it will decrease when onPictureTaken(Jpeg);
    // usually,Application restartPreview in onPictureTaken(Jpeg),
    // but if application receive onCanCapture firstly, move the restartPreview to onCanCapture.
    private int mCapturedImageCount = 0;

    public FaceBeautyMode(ICameraContext cameraContext) {
        super(cameraContext);
        Log.i(TAG, "[FaceBeautyMode]constructor...");
        // first check the camera device

        mFaceBeautyParametersHelper = new FaceBeautyParametersHelper(cameraContext);
        mParameterListener = mFaceBeautyParametersHelper.getListener();

        if (mIFeatureConfig.isVfbEnable()) {
            // just Vfb have View,so if current project is only support cFB,so
            // not have view
            mICameraView = mICameraAppUi.getCameraView(SpecViewType.MODE_FACE_BEAUTY);
            mICameraView.init(mActivity, mICameraAppUi, mIModuleCtrl);
            mICameraView.setListener(mParameterListener);
            mICameraAppUi.changeBackToVFBModeStatues(false);
        } else if (mIFeatureConfig.isSlowMotionSupport()) {
            setCfbSlowMotionRule();
        }
        mHandler = new MainHandler(mActivity.getMainLooper());
        mAdditionManager = cameraContext.getAdditionManager();
        if (mICameraContext.getFeatureConfig().isLowRamOptSupport()) {
            setFBLowRamRule();
        }
        if (mICameraContext.getFeatureConfig().isGmoRamOptSupport()) {
            setFBGmoRule();
        }
        if (mIFeatureConfig.isVfbEnable()) {
            setVFBPreviewSizeRule();
        }
    }

    @Override
    public void pause() {
        super.pause();
        Log.i(TAG, "[pause()] mICameraView = " + mICameraView);
        // Need hide the view when activity is onPause
        if (mICameraView != null) {
            mICameraView.hide();
        }
    }

    @Override
    public void resume() {
        super.resume();
        Log.i(TAG, "[resume()]");
        // for case: pressing home key to exit when capturing, and can not
        // slide to gallery after re-lunch.
        mICameraAppUi.setSwipeEnabled(true);
    }

    @Override
    public boolean close() {
        Log.i(TAG, "[closeMode]NextMode = " + mIModuleCtrl.getNextMode());
        if (mIModuleCtrl.getNextMode() != null) {
            // VFB photo mode to Video VR,not need set the face-beauty= false
            // 1:because video not know before mode is VFB;
            // 2:if set face-beauty= false,will found the video former parts not
            // have the face beauty effects
            if (mIFeatureConfig.isVfbEnable()) {
                if (CameraModeType.EXT_MODE_VIDEO == mIModuleCtrl.getNextMode()) {
                    //first need update the effects item if current effects is expanded
                    mICameraView.update(HIDE_EFFECTS_ITEM);
                } else {
                    setVFBPs(false);
                }
            }

            if (mHandler != null) {
                mHandler.sendEmptyMessage(ON_LEAVE_FACE_BEAUTY_MODE);
            }
            if (mICameraView != null) {
                mICameraView.update(REMVOE_BACK_TO_NORMAL);
            }

            // when leave out face beauty mode,need change the face view beauty
            // tag; because into this mode ,you have set true;
            changeFaceBeautyStatues(false);
        }
        // when close the mode ,need remove all the Msg when not execute
        removeAllMsg();
        mAdditionManager.close(true);

        return true;
    }

    @Override
    public boolean execute(ActionType type, Object... arg) {
        boolean returnValue = true;

        mAdditionManager.execute(type, true, arg);
        switch (type) {
        case ACTION_ON_CAMERA_OPEN:
            super.updateDevice();
            break;

        case ACTION_ON_CAMERA_CLOSE:
            mAdditionManager.close(true);
            setModeState(ModeState.STATE_CLOSED);
            if (mIFeatureConfig.isVfbEnable()) {
                mICameraView.update(ON_CAMERA_CLOSED);
            }
            break;

        case ACTION_ON_START_PREVIEW:
            Assert.assertTrue(arg.length == 1);
            startPreview((Boolean) arg[0]);
            break;

        case ACTION_ON_CAMERA_PARAMETERS_READY:
            super.updateDevice();
            super.updateFocusManager();
            if (mIFocusManager != null) {
                mIFocusManager.setListener(this);
            }
            String value = mISettingCtrl.getSettingValue(SettingConstants.KEY_AF);
            if (mActivity.getResources().getString(R.string.af_multi_mode).equals(value)) {
                mICameraDevice.getCamera().setAFDataCallback(mAFDataCallback);
            } else {
                mICameraDevice.getCamera().setAFDataCallback(null);
            }
            setModeState(ModeState.STATE_IDLE);

            // when user change the face beauty value to :Off need back to
            // PhotoMode
            if (isVfbOff()) {
                // leave faceBeauty mode
                mICameraAppUi.setCurrentMode(CameraModeType.EXT_MODE_PHOTO);
                break;
            }

            // just Vfb need stopFD,because when native will get the preview
            // buffer itself will
            // get the FD Client,if we not stopFD,means the FD client is catched
            // by us;
            if (mIFeatureConfig.isVfbEnable()) {
                mIModuleCtrl.stopFaceDetection();
            }
            mFaceBeautyParametersHelper.updateParameters(mICameraDevice);
            mHandler.sendEmptyMessage(ON_CAMERA_PARAMETERS_READY);

            mAdditionManager.onCameraParameterReady(true);
            break;

        case ACTION_FACE_DETECTED:
            Face[] faces = (Face[]) arg;
            if (mIFeatureConfig.isVfbEnable() && faces != null) {
                mICameraView.update(INFO_FACE_DETECTED, faces.length);
                // update the face beauty point
                storeFaceBeautyLocation(faces);
                // PhotoActor/3rd party not set the face, because you have call
                // stopFD
                mIModuleCtrl.setFaces(faces);
            }
            break;

        case ACTION_ON_COMPENSATION_CHANGED:
            if (mICameraView != null) {
                mICameraView.update(ORIENTATION_CHANGED, mIModuleCtrl.getOrientationCompensation());
            }
            break;

        case ACTION_ON_FULL_SCREEN_CHANGED:
            if (mICameraView != null) {
                mICameraView.update(ON_FULL_SCREEN_CHANGED, (Boolean) arg[0]);
            }
            break;

        case ACTION_SHUTTER_BUTTON_FOCUS:
            //when focus is detected, not need to do AF
            break;

        case ACTION_PHOTO_SHUTTER_BUTTON_CLICK:
            if (ModeState.STATE_CAPTURING != getModeState()) {
                // first need hide the effects item if the effects have expanded
                if (mICameraView != null) {
                    mICameraView.update(HIDE_EFFECTS_ITEM);
                }
                if (mIFocusManager != null) {
                    mIFocusManager.focusAndCapture();
                }
            }
            break;

        case ACTION_SHUTTER_BUTTON_LONG_PRESS:
            mICameraAppUi.showInfo(mActivity.getString(R.string.pref_camera_capturemode_enrty_fb)
                    + mActivity.getString(R.string.camera_continuous_not_supported));
            break;

        case ACTION_ON_SINGLE_TAP_UP:
            Assert.assertTrue(arg.length == 3);
            onSinlgeTapUp((View) arg[0], (Integer) arg[1], (Integer) arg[2]);
            break;

        case ACTION_ON_BACK_KEY_PRESS:
            // need callback ,if true means not need activity action on-back
            // pressed
            // when just supported Cfb, so need the supper onbackpressed
            if (mICameraView != null && !mICameraView.update(ON_BACK_PRESSED)
                    || !mIFeatureConfig.isVfbEnable()) {
                // returnValue is false means need super action the
                // onBackPressed
                returnValue = false;
            }
            break;

        case ACTION_ON_SETTING_BUTTON_CLICK:
            // when user go to setting turn off the face beauty,before change
            // the setting we have hide the VFB UI, when the setting have
            // changed,this time not need show the UI
            if (mICameraView != null && !isVfbOff()) {
                mICameraView.update(ON_SETTING_BUTTON_CLICK, arg[0]);
            }
            break;

        case ACTION_ON_CONFIGURATION_CHANGED:
            if (mHandler != null) {
                mHandler.sendEmptyMessage(ON_CONFIGURATION_CHANGED);
            }
            break;

        case ACTION_ON_SELFTIMER_STATE:
            if (mICameraView != null) {
                mICameraView.update(ON_SELFTIMER_CAPTUEING, (Boolean) arg[0]);
            }
            break;

        case ACTION_CAN_DO_AUTO_FOCUS:
            returnValue = ModeState.STATE_CAPTURING != getModeState();
            break;

        default:
            return false;
        }

        if (ActionType.ACTION_FACE_DETECTED != type) {
            Log.d(TAG, "[execute]type =" + type + ",returnValue = " + returnValue);
        }
        return returnValue;
    }

    @Override
    public boolean open() {
        mAdditionManager.setListener(this);
        mAdditionManager.open(true);
        super.open();
        return true;
    }

    //------> ICameraAddition.Listener
    @Override
    public boolean capture() {
        startCapture();
        return true;
    }

    @Override
    public boolean restartPreview(boolean needStop) {
        return false;
    }

    @Override
    public void onFileSaveing() {
    }
    //<------ICameraAddition.Listener

    //------> IFocusManager.FocusListener
    @Override
    public void autoFocus() {
        Log.d(TAG, "[autoFocus]...");
        mICameraDevice.autoFocus(mAutoFocusCallback);
        mICameraAppUi.setViewState(ViewState.VIEW_STATE_FOCUSING);
        setModeState(ModeState.STATE_FOCUSING);
    }

    @Override
    public void cancelAutoFocus() {
        if (!mIsAutoFocusCallback) {
            mICameraDevice.cancelAutoFocus();
            mIsAutoFocusCallback = true;
        }
        setFocusParameters();
        if (ModeState.STATE_CAPTURING != getModeState()) {
            mICameraAppUi.restoreViewState();
            setModeState(ModeState.STATE_IDLE);
        }
    }

    @Override
    public void startFaceDetection() {
        mIModuleCtrl.startFaceDetection();
    }

    @Override
    public void stopFaceDetection() {
        mIModuleCtrl.stopFaceDetection();
    }

    @Override
    public void setFocusParameters() {
        mIModuleCtrl.applyFocusParameters(!mIsAutoFocusCallback);
        mIsAutoFocusCallback = false;
    }

    @Override
    public void playSound(int soundId) {
        mCameraSound.play(soundId);
    }

    // <------IFocusManager.FocusListener

    private void removeAllMsg() {
        if (mHandler != null) {
            mHandler.removeMessages(ON_CAMERA_PARAMETERS_READY);
            mHandler.removeMessages(INFO_FACE_DETECTED);
            mHandler.removeMessages(ORIENTATION_CHANGED);
        }
    }

    private class MainHandler extends Handler {

        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "[handleMessage],msg = " + msg.what);

            switch (msg.what) {
            // this msg just used for VFB,so if you want use cFB,please be
            // careful
            case ON_CAMERA_PARAMETERS_READY:
                if (mICameraView != null) {
                    mICameraView.update(ON_CAMERA_PARAMETERS_READY);
                    mICameraView.show();
                }
                // because you have call stopFD,so PhotoActor/3rd party maybe
                // have clear the face view
                // so you need initialize yourself
                mIModuleCtrl.initializeFrameView(false);
                setVFBParameters();
                //after set the parameters to native ,enable face beauty frame
                changeFaceBeautyStatues(true);

                break;

            case ON_LEAVE_FACE_BEAUTY_MODE:
                if (mICameraView != null) {
                    mICameraView.update(ON_LEAVE_FACE_BEAUTY_MODE);
                }
                break;

            case ON_CONFIGURATION_CHANGED:
                // because configuration change,so need re-inflate the view
                // layout
                if (mICameraView != null) {
                    mICameraView.uninit();
                    mICameraView.init(mActivity, mICameraAppUi, mIModuleCtrl);
                }
                break;

            default:
                break;
            }
        }
    }

    private void setVFBParameters() {
        if (mICameraDevice != null && mIModuleCtrl.isNonePickIntent()
                && mIFeatureConfig.isVfbEnable()) {
            // if face-beauty value is false,means current need to start VFB
            if (isOnlyMultiFaceBeautySupported()) {
                mICameraDevice.setParameter(Util.KEY_FB_EXTREME_BEAUTY, "false");
            }
            setVFBPs(true);
        }
    }

    private void changeFaceBeautyStatues(boolean enable) {
        /**
         * true : means will set face beauty view to true, so the UI will show a
         * face beauty frame false: means will not show the full color face
         * frame,just is white color
         */
        mIModuleCtrl.setFaceBeautyEnalbe(enable);
    }

    private boolean isOnlyMultiFaceBeautySupported() {
        // fb-extreme-beauty-supported is false means just supported Multi face
        // mode,
        // so need remove the single face mode in settings
        //boolean isOnlySupported = "false".equals(mICameraDevice.getParameters().get(
        //        Util.KEY_FB_EXTEME_BEAUTY_SUPPORTED));
        boolean isOnlySupported = "false".equals(mICameraDevice
                .getParameter(Util.KEY_FB_EXTEME_BEAUTY_SUPPORTED));
        Log.d(TAG, "isOnlyMultiFaceBeautySupported = " + isOnlySupported);
        return isOnlySupported;
    }

    private void setVFBPs(boolean isStart) {
        String value = isStart ? Util.VIDEO_FACE_BEAUTY_ENABLE : Util.VIDEO_FACE_BEAUTY_DISABLE;
        mICameraDevice.setParameter(Util.KEY_VIDEO_FACE_BEAUTY, value);
        // before set the ps,need update the ps
        mICameraDevice.applyParameters();
    }

    private void setFBLowRamRule() {
        FaceBeautyPictureSizeRule pictureSizeRule = new FaceBeautyPictureSizeRule(mISettingCtrl,
                mICameraContext);
        mISettingCtrl.addRule(SettingConstants.KEY_FACE_BEAUTY, SettingConstants.KEY_PICTURE_SIZE,
                pictureSizeRule);
        pictureSizeRule.addLimitation("on", null, null);
    }

    private void setFBGmoRule() {
        FaceBeautyZsdRule zsdRule = new FaceBeautyZsdRule(mISettingCtrl, mICameraContext);
        mISettingCtrl.addRule(SettingConstants.KEY_FACE_BEAUTY, SettingConstants.KEY_CAMERA_ZSD,
                zsdRule);
        zsdRule.addLimitation("on", null, null);
    }

    private void setCfbSlowMotionRule() {
        CaptureFaceBeautyRule slowMotionRule = new CaptureFaceBeautyRule(mICameraContext);
        mISettingCtrl.addRule(SettingConstants.KEY_FACE_BEAUTY,
                SettingConstants.KEY_SLOW_MOTION, slowMotionRule);
        slowMotionRule.addLimitation("on", null, null);
    }

    private void setVFBPreviewSizeRule() {
        mFaceBeautyPreviewSize = new FaceBeautyPreviewSize(mICameraContext);
        mISettingCtrl.addRule(SettingConstants.KEY_FACE_BEAUTY, SettingConstants.KEY_PICTURE_RATIO,
                mFaceBeautyPreviewSize);
        mFaceBeautyPreviewSize.addLimitation("on", null, null);
    }

    private void onShutterButtonLongPress() {
        mICameraAppUi.showInfo(mActivity.getString(R.string.pref_camera_capturemode_enrty_fb)
                + mActivity.getString(R.string.camera_continuous_not_supported));
    }

    private void onSinlgeTapUp(View view, int x, int y) {
        if (ModeState.STATE_IDLE != getModeState()) {
            return;
        }
        String focusMode = null;
        if (mIFocusManager != null) {
            focusMode = mIFocusManager.getFocusMode();
        }
        if (mICameraDevice == null || focusMode == null
                || (Parameters.FOCUS_MODE_INFINITY.equals(focusMode))) {
            return;
        }
        if (!mIFocusManager.getFocusAreaSupported()) {
            return;
        }
        if (mICameraView != null) {
            mICameraView.update(HIDE_EFFECTS_ITEM);
        }
        mIFocusManager.onSingleTapUp(x, y);
    }

    private boolean startCapture() {
        if ((ModeState.STATE_IDLE != getModeState()) || !isEnoughSpace()) {
            Log.d(TAG, "[startCapture],invalid state, return!");
            return false;
        }
        if (mICameraView != null) {
            mICameraView.update(IN_PICTURE_TAKEN_PROGRESS, true);
        }
        // need set FB original capture callback and here
        mICameraAppUi.setSwipeEnabled(false);
        mICameraAppUi.showRemaining();
        mICameraAppUi.setViewState(ViewState.VIEW_STATE_CAPTURE);

        mICameraDevice.setcFBOrignalCallback(mCfbCallback);
        if (mIFeatureConfig.isVfbEnable()) {
            // because native not know the face beauty location when take
            // picture,
            // so need set the beauty location
            setvFBFacePoints();
        }
        setModeState(ModeState.STATE_CAPTURING);
        mICameraDevice.takePicture(mShutterCallback, mRawPictureCallback, mPostViewPictureCallback,
                mJpegPictureCallback);

        return true;
    }

    private ShutterCallback mShutterCallback = new ShutterCallback() {

        @Override
        public void onShutter() {
        }
    };

    private PictureCallback mRawPictureCallback = new PictureCallback() {

        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
        }
    };

    private PictureCallback mPostViewPictureCallback = new PictureCallback() {

        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
        }
    };

    private final PictureCallback mUncompressedImageCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
            Log.d(TAG, "[UncompressedImageCallback]onCanCapture");
            ++mCapturedImageCount;
            startPreview(false);
        }
    };

    private PictureCallback mJpegPictureCallback = new PictureCallback() {

        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
            Log.d(TAG, "VFBCallback[mJpegPictureCallback], time = " + System.currentTimeMillis()
                    + ",data = " + data);

            if (ModeState.STATE_CLOSED != getModeState()) {
                // Ensure focus indicator
                mIFocusManager.updateFocusUI();
                // Need Restart preview ,synchronize Normal photo
                // ZSD don't call stop preview when Capture done.
                boolean needRestartPreivew = !"on".equals(mISettingCtrl
                        .getSettingValue(SettingConstants.KEY_CAMERA_ZSD))
                        && (ModeState.STATE_CAPTURING == getModeState()
                        && (mCapturedImageCount == 0));
                startPreview(needRestartPreivew);
                mIModuleCtrl.startFaceDetection();
                // check due to the onCanCapture don't send to application in some condition.
                if (mCapturedImageCount > 0) {
                    --mCapturedImageCount;
                }
            }
            if (mICameraView != null) {
                mICameraView.update(IN_PICTURE_TAKEN_PROGRESS, false);
            }
            if (data != null) {
                // prepare the save request
                mIFileSaver.init(FILE_TYPE.JPEG, 0, null, -1);
                long time = System.currentTimeMillis();
                mIFileSaver.savePhotoFile(data, null, time, mIModuleCtrl.getLocation(),
                        0, mFileSavedListener);
            }
        }
    };

    private void storeFaceBeautyLocation(Face[] faces) {
        int index = 0;
        // First :clear last time stored values
        if (mVfbFacesPoint != null && mVfbFacesPoint.size() != 0) {
            mVfbFacesPoint.clear();
        }
        // Seconded: store the new values to the list
        if (faces != null) {
            for (int i = 0; i < faces.length; i++) {
                if (100 == faces[i].score) {
                    int x = faces[i].rect.left + (faces[i].rect.right - faces[i].rect.left) / 2;
                    int y = faces[i].rect.top + (faces[i].rect.bottom - faces[i].rect.top) / 2;
                    mVfbFacesPoint.add(index++, x);
                    mVfbFacesPoint.add(index++, y);
                }
            }
        }
    }

    private void setvFBFacePoints() {
        if (mVfbFacesPoint == null) {
            return;
        }
        String value = setFacePose();
        if (value != null) {
            mICameraDevice.setParameter(FaceBeautyParametersHelper.KEY_VIDED_FACE_BEAUTY_FACE,
                    value);
            mICameraDevice.applyParameters();
        }
    }

    private String setFacePose() {
        String value = "";
        for (int i = 0; i < mVfbFacesPoint.size(); i++) {
            value += mVfbFacesPoint.get(i);
            // why need (i +1) != mvFBFacesPoint.size() ?
            // because at the end of value,not need any symbol
            // the value format is: xxx:yyy,x1:y1
            if ((i + 1) != mVfbFacesPoint.size()) {
                if (i % 2 != 0) {
                    value += ",";
                } else {
                    value += ":";
                }
            }
        }
        return value;
    }

    private OnFileSavedListener mFileSavedListener = new OnFileSavedListener() {
        @Override
        public void onFileSaved(Uri uri) {
        }
    };

    // CFB
    public class CfbCallback implements cFbOriginalCallback {

        @Override
        public void onOriginalCallback(byte[] data) {
            Log.d(TAG, "cFBCallback,[onOriginalCallback],data.length = " + data.length);
            if (!mIFeatureConfig.isVfbEnable()) {
                mIFileSaver.init(FILE_TYPE.JPEG, 0, null, -1);
                long time = System.currentTimeMillis();
                mIFileSaver.savePhotoFile(data, null, time, mIModuleCtrl.getLocation(), 0,
                        mFileSavedListener);
            }
        }
    }

    private boolean isVfbOff() {
        boolean isOff = mActivity.getResources().getString(R.string.pref_face_beauty_mode_off)
                .equals(mISettingCtrl.getSettingValue(SettingConstants.KEY_MULTI_FACE_BEAUTY));

        return isOff;
    }


    private final AutoFocusCallback mAutoFocusCallback = new AutoFocusCallback() {

        @Override
        public void onAutoFocus(boolean success, Camera camera) {
            if (ModeState.STATE_CLOSED == getModeState()) {
                return;
            }
            if (ModeState.STATE_FOCUSING == getModeState()) {
                mICameraAppUi.restoreViewState();
                setModeState(ModeState.STATE_IDLE);
            }
            mIFocusManager.onAutoFocus(success);
            mIsAutoFocusCallback = true;
        }
    };

    private final AutoFocusMvCallback mAutoFocusMvCallback = new AutoFocusMvCallback() {

        @Override
        public void onAutoFocusMoving(boolean start, Camera camera) {
            mIFocusManager.onAutoFocusMoving(start);
        }
    };

    private final AFDataCallback mAFDataCallback = new AFDataCallback() {
        @Override
        public void onAFData(byte[] data, Camera camera) {
            String value = mISettingCtrl.getSettingValue(SettingConstants.KEY_AF);
            boolean isMultiAfMode = mActivity.getResources().getString(R.string.af_multi_mode)
                    .equals(value) ? true : false;
            if (data != null && data.length > 0 && isMultiAfMode) {
                mIFocusManager.setAfData(data);
            } else {
                Log.w(TAG, "onAFData AF data is got in single AF mode with isMultiAfMode = "
                        + isMultiAfMode);
                mIFocusManager.setAfData(null);
            }
        }
    };

    private void startPreview(boolean needStop) {
        Log.d(TAG, "[startPreview]needStop = " + needStop);

        mIsAutoFocusCallback = false;

        mActivity.runOnUiThread(new Runnable() {
            public void run() {
                mIFocusManager.resetTouchFocus();
            }
        });
        if (needStop) {
            stopPreview();
        }
        mIFocusManager.setAeLock(false); // Unlock AE and AWB.
        mIFocusManager.setAwbLock(false);

        mIModuleCtrl.applyFocusParameters(false);

        mICameraDevice.startPreview();
        mICameraAppUi.restoreViewState();
        mICameraDevice.setAutoFocusMoveCallback(mAutoFocusMvCallback);
        mIFocusManager.onPreviewStarted();
        mAdditionManager.execute(AdditionActionType.ACTION_ON_START_PREVIEW);
        setModeState(ModeState.STATE_IDLE);
        mICameraDevice.setUncompressedImageCallback(mUncompressedImageCallback);
    }

    private void stopPreview() {
        Log.d(TAG, "[stopPreview]mCurrentState = " + getModeState());
        if (ModeState.STATE_CLOSED == getModeState()) {
            return;
        }
        mICameraDevice.cancelAutoFocus(); // Reset the focus.
        mICameraDevice.stopPreview();
        if (mIFocusManager != null) {
            mActivity.runOnUiThread(new Runnable() {
                public void run() {
                    mIFocusManager.onPreviewStopped();
                }
            });
        }
    }
}
