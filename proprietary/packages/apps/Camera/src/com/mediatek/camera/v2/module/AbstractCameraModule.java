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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

package com.mediatek.camera.v2.module;

import android.app.Activity;
import android.graphics.RectF;
import android.hardware.camera2.CameraDevice.StateCallback;

import com.android.camera.R;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.control.ControlImpl;
import com.mediatek.camera.v2.control.IControl.IAaaController;
import com.mediatek.camera.v2.detection.IDetectionManager;
import com.mediatek.camera.v2.mode.AbstractCameraMode;
import com.mediatek.camera.v2.mode.ModeController.ModeGestureListener;
import com.mediatek.camera.v2.mode.normal.CaptureMode;
import com.mediatek.camera.v2.mode.pip.PipMode;
import com.mediatek.camera.v2.platform.ModeChangeListener;
import com.mediatek.camera.v2.platform.app.AppController;
import com.mediatek.camera.v2.platform.app.AppUi;
import com.mediatek.camera.v2.platform.app.AppUi.OkCancelClickListener;
import com.mediatek.camera.v2.platform.app.AppUi.PlayButtonClickListener;
import com.mediatek.camera.v2.platform.app.AppUi.RetakeButtonClickListener;
import com.mediatek.camera.v2.platform.app.AppUi.ShutterEventsListener;
import com.mediatek.camera.v2.platform.module.ModuleController;
import com.mediatek.camera.v2.platform.module.ModuleUi.PreviewAreaChangedListener;
import com.mediatek.camera.v2.services.CameraServices;
import com.mediatek.camera.v2.services.storage.IStorageService;
import com.mediatek.camera.v2.services.storage.IStorageService.IStorageStateListener;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.ISettingServant.ISettingChangedListener;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.stream.StreamManager;
import com.mediatek.camera.v2.ui.CountDownView;
import com.mediatek.camera.v2.util.SettingKeys;

import junit.framework.Assert;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Semaphore;

public abstract class AbstractCameraModule implements
                                ModuleController,
                                ModuleListener,
                                ISettingChangedListener,
                                IStorageStateListener,
                                CountDownView.OnCountDownStatusListener,
                                PreviewAreaChangedListener {
    private static final Tag TAG = new Tag(AbstractCameraModule.class.getSimpleName());
    public static final int UNKNOWN = -1;
    private   final CameraServices mServices;
    protected boolean mPaused;
    protected final AppController        mAppController;
    protected final SettingCtrl          mSettingController;
    protected ISettingServant            mSettingServant;
    protected IStorageService             mStorageService;
    protected final AppUi                mAppUi;
    protected AbstractModuleUi           mAbstractModuleUI;
    protected IDetectionManager           mDetectionManager;
    protected StreamManager              mStreamManager;

    protected boolean                    mIsCaptureIntent;
    protected AbstractCameraMode         mCurrentMode;
    protected int                        mOldModeIndex;
    protected int                        mCurrentModeIndex;
    private ModeGestureListener          mModeGestureListener;

    protected ControlImpl mAaaControl;
    protected String mCameraId;
    // The activity is going to switch to the specified camera id. This is
    // needed because texture copy is done in GL thread. -1 means camera is not
    // switching.
    protected int mPendingSwitchCameraId = UNKNOWN;
    // if surface is changed,this parameters will be update to true.
    // but as follow case : change module,the camera will be execute close->
    // open.if the surface is not change, so the preview surface not change, and
    // also not will execute the parameters ready = false. in this case ,the
    // mPreviewSurfaceIsReadyForOpen also need to be true.
    protected boolean mPreviewSurfaceIsReadyForOpen = false;
    // Fair semaphore held when opening or closing the camera.
    private final Semaphore mCameraOpenCloseLock = new Semaphore(1, true);
    public AbstractCameraModule(AppController app) {
        Assert.assertNotNull(app);
        mAppController           = app;
        mAppUi                   = app.getCameraAppUi();
        mServices                = app.getServices();
        mSettingController       = mServices.getSettingController();
        mStorageService          = mServices.getStorageService();
        mSettingServant          = mSettingController.getSettingServant(null);
        mStreamManager           = StreamManager.getInstance(app.getActivity());
        mCurrentModeIndex        = app.getCurrentModeIndex();
        mOldModeIndex            = app.getOldModeIndex();
        createCurrentMode(mCurrentModeIndex);
    }

    @Override
    public void open(Activity activity, boolean isSecureCamera, boolean isCaptureIntent) {
        mIsCaptureIntent         = isCaptureIntent;
        if (mIsCaptureIntent) {
            mAppUi.setSwipeEnabled(false);
        }
        mAaaControl.open(activity, mAppUi.getModuleLayoutRoot(), isCaptureIntent);
        mDetectionManager.open(activity, mAppUi.getModuleLayoutRoot(), isCaptureIntent);
        mStreamManager.open(mAppUi.getModuleLayoutRoot(), isCaptureIntent);
        mCurrentMode.open(mStreamManager, mAppUi.getModuleLayoutRoot(), isCaptureIntent);
        mSettingServant.registerSettingChangedListener(this, null,
                ISettingChangedListener.LOW_PRIORITY);

        // add shutter listener
        mAppUi.setShutterEventListener(mPhotoShutterEventsListener, false);
        mAppUi.setShutterEventListener(mVideoShutterEventsListener, true);
        mAppUi.setOkCancelClickListener(mOkCancelClickListener);
        mAppUi.setPlayButtonClickListener(mPlayButtonClickListener);
        mAppUi.setRetakeButtonClickListener(mRetakeButtonClickListener);
        // Change module always cause mode changing.
        doModeChange(mOldModeIndex, mCurrentModeIndex);
    }

    @Override
    public void close() {
        mAaaControl.close();
        mDetectionManager.close();
        mStreamManager.close(mAppController.getActivity());
        mCurrentMode.close();
        mOldModeIndex = mCurrentModeIndex;
        mSettingServant.unRegisterSettingChangedListener(this);
    }

    @Override
    public void onBeforeCameraPicked(String newCameraId) {
    }

    @Override
    public void onCameraPicked(String newCameraId) {
    }

    @Override
    public void resume() {
        mPaused = false;
        mAaaControl.resume();
        mDetectionManager.resume();
        mCurrentMode.resume();
        mStorageService.registerStorageStateListener(this);
    }

    @Override
    public void pause() {
        if (mAbstractModuleUI.isCountingDown()) {
            mAbstractModuleUI.cancelCountDown();
            switchCommonUiByCountingDown(false);
        }
        mPaused = true;
        mAaaControl.pause();
        mDetectionManager.pause();
        mCurrentMode.pause();
        mStorageService.unRegisterStorageStateListener(this);
    }

    @Override
    public void onStorageStateChanged(int storageState) {
        LogHelper.i(TAG, "onStorageStateChange storageState = " + storageState);

        switch (storageState) {

        case IStorageService.STORAGE_IS_OUT_OF_SERVICES:
             mCurrentMode.onMediaEjected();
             mAppController.updateStorageSpaceAndHint();
             mAppUi.forceUpdateThumbnail();
            break;

        case IStorageService.STORAGE_IS_IN_SERVICES:
            mAppController.updateStorageSpaceAndHint();
            mAppUi.forceUpdateThumbnail();
            break;

        case IStorageService.STORAGE_IS_PREPARING:
            mAppUi.showHint(mAppController.getActivity().getResources().getString(R.string.wait));
            break;

        default:
            break;
        }
    }
    @Override
    public void onPreviewAreaChanged(RectF previewArea) {
        mDetectionManager.onPreviewAreaChanged(previewArea);
        mAaaControl.onPreviewAreaChanged(previewArea);
        mAbstractModuleUI.onPreviewAreaChanged(previewArea);
        mCurrentMode.onPreviewAreaChanged(previewArea);
    }

    @Override
    public void onPreviewVisibilityChanged(int visibility) {
        mCurrentMode.onPreviewVisibilityChanged(visibility);
    }

    @Override
    public boolean onBackPressed() {
        if (mAbstractModuleUI.isCountingDown()) {
            mAbstractModuleUI.cancelCountDown();
            switchCommonUiByCountingDown(false);
            return true;
        }
        if (mCurrentMode.onBackPressed()) {
            return true;
        }
        return false;
    }

    // module registers the lowest setting change callback
    // this onSettingChanged callback just be used to handle CameraActions
    // send from other components
    @Override
    public void onSettingChanged(Map<String, String> result) {
        LogHelper.i(TAG, "[onSettingChanged]+ ");
        LogHelper.i(TAG, "[onSettingChanged]-");
    }

    @Override
    public void onRemainingSecondsChanged(int remainingSeconds) {
        if (remainingSeconds == 1) {
            mServices.getSoundPlayback().play(R.raw.timer_final_second, 0.6f);
        } else if (remainingSeconds == 2 || remainingSeconds == 3) {
            mServices.getSoundPlayback().play(R.raw.timer_increment, 0.6f);
        }
    }

    @Override
    public void onCountDownFinished() {
        if (checkSatisfyCaptureCondition()) {
            switchCommonUiByCountingDown(false);
            mCurrentMode.onShutterClicked(false/**is video shutter**/);
        }
    }

    @Override
    public synchronized void onPreviewSurfaceReady() {
        mPreviewSurfaceIsReadyForOpen = true;
    }

    @Override
    public void requestChangeCaptureRequets(boolean sync, RequestType requestType,
            CaptureType captureType) {

    }

    @Override
    public void requestChangeCaptureRequets(boolean isMainCamera, boolean sync,
            RequestType requestType, CaptureType captureType) {

    }

    @Override
    public void requestChangeSessionOutputs(boolean sync) {

    }

    @Override
    public void requestChangeSessionOutputs(boolean sync, boolean isMainCamera) {

    }

    @Override
    public IAaaController get3AController(String cameraId) {
        return null;
    }


    @Override
    public void requestUpdateRecordingStatus(boolean isRecordingStated) {
        // TODO Auto-generated method stub

    }

    @Override
    public void requestUpdateCaptureStatus(boolean isCapturing) {

    }


    @Override
    public void requestSessionOutputs(boolean sync, String camerId) {

    }

    /**
     * change current request,such as preview and capture.
     * @param cameraId
     *            which camera need to configure the request.
     * @param sync
     *            true request immediately. false wait all requests been
     *            submitted and remove the same request current design is only
     *            for
     *            {@link ISettingChangedListener#onSettingChanged(java.util.Map)}
     * @param requestType
     *            the required request, which is one of the {@link RequestType}
     * @param captureType
     *            the required capture, which is one of the {@link CaptureType}
     */
    @Override
    public void requestCaptureRequets(String cameraId, boolean sync, RequestType requestType,
            CaptureType captureType) {

    }

    public boolean onDown(float x, float y) {
        LogHelper.i(TAG, "onDown " + this + " x:" + x + " y:" + y +
                " mModeGestureListener : " + mModeGestureListener);
        if (mModeGestureListener != null) {
            return mModeGestureListener.onDown(x, y);
        }
        return false;
    }

    public boolean onUp() {
        if (mModeGestureListener != null) {
            return mModeGestureListener.onUp();
        }
        return false;
    }

    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        if (mModeGestureListener != null) {
            return mModeGestureListener.onScroll(dx, dy, totalX, totalY);
        }
        return false;
    }

    /**
     * single tap up gesture received
     * @param x x's position
     * @param y y's position
     */
    public boolean onSingleTapUp(float x, float y) {
        if (mModeGestureListener != null) {
            return mModeGestureListener.onSingleTapUp(x, y);
        }
        return false;
    }
    /**
     * The user has performed a down {@link MotionEvent} and not performed
     * a move or up yet. This event is commonly used to provide visual
     * feedback to the user to let them know that their action has been
     * recognized i.e. highlight an element.
     *
     * @param x long press x's position
     * @param y long press y's position
     */
    public boolean onLongPress(float x, float y) {
        if (mModeGestureListener != null) {
            return mModeGestureListener.onLongPress(x, y);
        }
        return false;
    }

    /**
     * @return An instance containing common services to be used by the module.
     */
    protected CameraServices getServices() {
        return mServices;
    }

    protected void switchToNewMode(int modeIndex) {
        LogHelper.d(TAG, "switchToNewMode old --> new : "
                + mCurrentModeIndex + " --> " + modeIndex);
        if (mCurrentModeIndex != modeIndex) {
            closeMode(mCurrentMode);
            createCurrentMode(modeIndex);
            mOldModeIndex = mCurrentModeIndex;
            mCurrentModeIndex = modeIndex;
            openMode(mCurrentMode);
        }
    }

    protected void closeMode(AbstractCameraMode mode) {

    }

    protected void createCurrentMode(int modeIndex) {
        LogHelper.i(TAG, "[createCurrentMode]+ modeIndex: " + modeIndex);
        switch (modeIndex) {
        case ModeChangeListener.MODE_CAPTURE:
            mCurrentMode = new CaptureMode(mAppController, this);
            break;
        case ModeChangeListener.MODE_PIP:
            mCurrentMode = new PipMode(mAppController, this);
            break;

        default:
            mCurrentMode = new CaptureMode(mAppController, this);
            break;
        }
        mModeGestureListener = mCurrentMode.getModeGestureListener();
        LogHelper.i(TAG, "[createCurrentMode]- mModeGestureListener: " + mModeGestureListener);
    }

    protected void doModeChange(int oldModeIndex, int newModeIndex) {
        LogHelper.d(TAG, "[doModeChange], oldModeIndex:" + oldModeIndex + ", " +
                "newModeIndex:" + newModeIndex);
        Map<String, String> changedModes = new HashMap<String, String>();
        switch(oldModeIndex) {
        case ModeChangeListener.MODE_PIP:
            changedModes.put(SettingKeys.KEY_PHOTO_PIP, "off");
            break;
        default:
            break;
        }

        switch(newModeIndex) {
        case ModeChangeListener.MODE_PIP:
            changedModes.put(SettingKeys.KEY_PHOTO_PIP, "on");
            break;
        default:
            break;
        }
        LogHelper.d(TAG, "[doModeChange], changedModes:" + changedModes);
        mSettingController.doSettingChange(changedModes);
    }

    protected void showErrorAndFinish(int error) {
        if (StateCallback.ERROR_CAMERA_IN_USE == error
                    || StateCallback.ERROR_MAX_CAMERAS_IN_USE == error) {
            mAppController.showErrorAndFinish(R.string.cannot_connect_camera_new);
        } else if (StateCallback.ERROR_CAMERA_DISABLED == error
                    || StateCallback.ERROR_CAMERA_DEVICE == error
                    || StateCallback.ERROR_CAMERA_SERVICE == error) {
            mAppController.showErrorAndFinish(R.string.camera_disabled);
        }
    }

    protected void acquireOpenCloseLock() {
        try {
            mCameraOpenCloseLock.acquire();
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted " +
                    "while waiting to acquire camera-open lock.", e);
        }
    }

    protected void releaseOpenCloseLock() {
        mCameraOpenCloseLock.release();
    }

    private void cancelCountDown() {
        if (mAbstractModuleUI.isCountingDown()) {
            mAbstractModuleUI.cancelCountDown();
            switchCommonUiByCountingDown(false);
        }
    }

    protected void openMode(AbstractCameraMode mode) {

    }

    protected boolean checkSatisfyCaptureCondition() {
        return true;
    }


    protected void switchCommonUiByCountingDown(boolean isCountingDown) {
        if (isCountingDown) {
            mAppUi.setShutterButtonEnabled(false, false/**videoShutter**/);
            mAppUi.setShutterButtonEnabled(false, true/**videoShutter**/);
            mAppUi.setSwipeEnabled(false);
            mAppUi.hideModeOptionsUi();
            mAppUi.hideSettingUi();
            mAppUi.hidePickerManagerUi();
            mAppUi.hideThumbnailManagerUi();
            mAppUi.hideIndicatorManagerUi();
        } else {
            mAppUi.setShutterButtonEnabled(true, false/**videoShutter**/);
            mAppUi.setShutterButtonEnabled(true, true/**videoShutter**/);
            mAppUi.setSwipeEnabled(true);
            mAppUi.showModeOptionsUi();
            mAppUi.showSettingUi();
            mAppUi.showPickerManagerUi();
            mAppUi.showThumbnailManagerUi();
            mAppUi.showIndicatorManagerUi();
        }
    }

    private ShutterEventsListener   mPhotoShutterEventsListener = new ShutterEventsListener() {
        @Override
        public void onShutterReleased() {
            mCurrentMode.onShutterReleased(false);
        }
        @Override
        public void onShutterPressed() {
            mCurrentMode.onShutterPressed(false);
        }
        @Override
        public void onShutterLongPressed() {
            mCurrentMode.onShutterLongPressed(false);
        }
        @Override
        public void onShutterClicked() {
            if (!checkSatisfyCaptureCondition()) {
                return;
            }
            String seflTimer = mSettingServant.getSettingValue(SettingKeys.KEY_SELF_TIMER);
            LogHelper.i(TAG, "seflTimer = " + seflTimer);
            int mTimerDuration = Integer.valueOf(seflTimer) / 1000;
            if (mTimerDuration > 0) {
                switchCommonUiByCountingDown(true);
                mAbstractModuleUI.setCountdownFinishedListener(AbstractCameraModule.this);
                mAbstractModuleUI.startCountdown(mTimerDuration);
            } else {
                mCurrentMode.onShutterClicked(false);
            }
        }
    };

    private ShutterEventsListener   mVideoShutterEventsListener = new ShutterEventsListener() {
        @Override
        public void onShutterReleased() {
            mCurrentMode.onShutterReleased(true);
        }
        @Override
        public void onShutterPressed() {
            mCurrentMode.onShutterPressed(true);
        }
        @Override
        public void onShutterLongPressed() {
            mCurrentMode.onShutterLongPressed(true);
        }
        @Override
        public void onShutterClicked() {
            mCurrentMode.onShutterClicked(true);
        }
    };

    private OkCancelClickListener mOkCancelClickListener = new OkCancelClickListener() {

        @Override
        public void onOkClick() {
            LogHelper.i(TAG, "[onOkClick]");
            mCurrentMode.onOkClick();
        }

        @Override
        public void onCancelClick() {
            LogHelper.i(TAG, "[onCancelClick]");
            mCurrentMode.onCancelClick();
        }
    };

    private PlayButtonClickListener mPlayButtonClickListener = new PlayButtonClickListener() {

        @Override
        public void onPlay() {
            LogHelper.i(TAG, "[onPlay]");
            mCurrentMode.onPlay();
        }
    };

    private RetakeButtonClickListener mRetakeButtonClickListener = new RetakeButtonClickListener() {

        @Override
        public void onRetake() {
            LogHelper.i(TAG, "[onRetake]");
            mCurrentMode.onRetake();
        }
    };
}
