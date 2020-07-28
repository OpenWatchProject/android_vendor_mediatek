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

package com.mediatek.camera.addition.continuousshot;

import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import com.android.camera.R;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ICameraMode.ActionType;
import com.mediatek.camera.addition.CameraAddition;
import com.mediatek.camera.addition.continuousshot.MemoryManager.MemoryAction;
import com.mediatek.camera.platform.ICameraAppUi.SpecViewType;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.ContinuousShotListener;
import com.mediatek.camera.platform.ICameraView;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.util.CaptureSound;
import com.mediatek.camera.util.Log;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;

/**
 * The controller of continuous shot: flow, UI and so on.
 */
@SuppressWarnings("deprecation")
public class ContinuousShot extends CameraAddition implements ContinuousShotListener {
    private static final String TAG = "ContinuousShot";

    private static final int DEFAULT_CONINUOUS_CAPTURE_NUM = 20;
    // Reduce the refresh interval of thumb nail to 500ms, to save system loading.
    private static final int THUMBNAIL_REFRESH_CONTINUOUS = 500;
    private static final int THUMBNAIL_REFRESH_NORMAL = 0;

    // Message ID for UI thread handler
    private static final int HIDE_SPEED_INDICATOR = 1000;
    private static final int BACK_TO_NORMAL_VIEW = 1001;
    private static final int RESTART_PREVIEW = 1002;
    private static final int SPEED_INDICATOR_SHOE_TIME = 1 * 1000;
    private static final long LOW_STORAGE_THRESHOLD_NORMAL = 50000000;
    private static final long LOW_STORAGE_THRESHOLD_GMO_ROM = 10000000;

    private String[] mFeatureKey = { null, SettingConstants.KEY_HDR, SettingConstants.KEY_ASD };
    private int[] mFeatureResId = { R.string.normal_camera_continuous_not_supported,
            R.string.pref_camera_hdr_title, R.string.pref_camera_capturemode_entry_asd };

    // Parameter key which is define with native
    private static final String KEY_CSHOT_INDICATOR = "cshot-indicator";
    private static final String FILE_NAME_CONNECTOR = "_";
    private static final String FILE_NAME_SUFFIX = "CS.jpg";
    private static final String CONTINUOUS_SHOT_MODE = "continuousshot";
    private static final String SUPPORT_INDICATOR = "true";
    private static final String FEATURE_ON = "on";
    private static final String MTK_CHIP_0321 = "0321";
    // focus mode for continuous shot, it is continuous-picture new.
    private static final String FOCUS_MODE = "continuous-picture";
    // Get the max capture number from setting, normal mode user can chose 40 or 99;
    // RAM optimize load only a default 20 can be used.
    private int mMaxCaptureNum;
    private int mCurrentShotsNum;

    private long mLowStorageThreshold = LOW_STORAGE_THRESHOLD_NORMAL;

    // it must make sure one continuous shot
    // use one and only time to generate file name,
    // so it will be set in shutter callback.
    private Date mPictureTakenDate = new Date();

    private boolean mIsClearMemoryLimit;
    private boolean mIsSupportIndicator;

    /**
     * Continuous shot state.
     * {@link #setState}
     */
    private enum State {
        STATE_INIT, STATE_OPENED,
        STATE_CAPTURE_STARTED,
        STATE_CAPTURING,
        STATE_STOPPED,
        STATE_SAVING,
    }

    private State mState = State.STATE_INIT;

    private ICameraView mICameraView;

    private Listener mListener;
    private Thread mWaitSavingDoneThread;
    private Handler mHandler;
    private SimpleDateFormat mFormat;
    private MemoryManager mMemoryManager;
    private CaptureSound mCaptureSound;
    private boolean mIsDngOpenedBeforeCs = false;
    /**
     * Initialize memory manage, capture sound and view.
     * @param context camera context to get the feature configuration information.
     */
    public ContinuousShot(ICameraContext context) {
        super(context);

        mMemoryManager = new MemoryManager(mActivity);
        mHandler = new MainHandler(mActivity.getMainLooper());
        mCaptureSound = new CaptureSound(mActivity);
        mFormat = new SimpleDateFormat(mActivity.getString(R.string.image_file_name_format),
                Locale.ENGLISH);
        mICameraView = mICameraAppUi.getCameraView(SpecViewType.ADDITION_CONTINUE_SHOT);
        int orientation = mIModuleCtrl.getOrientationCompensation();
        mICameraView.onOrientationChanged(orientation);

        if (mIFeatureConfig.isMtkFatOnNandSupport() || mIFeatureConfig.isGmoRomOptSupport()) {
            mLowStorageThreshold = LOW_STORAGE_THRESHOLD_GMO_ROM;
            Log.d(TAG, "[ContinuousShot]LOW_STORAGE_THRESHOLD= 10000000");
        }
    }

    @Override
    public void setListener(Listener listener) {
        mListener = listener;
    }

    @Override
    public void open() {
        State state = getCurrentState();
        Log.d(TAG, "[open]state = " + state);
        if (state == State.STATE_OPENED) {
            return;
        }
        mCaptureSound.load();
        mIsSupportIndicator = SUPPORT_INDICATOR.equals(mICameraDevice
                .getParameter(KEY_CSHOT_INDICATOR));
        setState(State.STATE_OPENED);
        Log.d(TAG, "[open]Indicator support is " + mIsSupportIndicator);
    }

    @Override
    public boolean isOpen() {
        boolean isOpen = false;
        if (State.STATE_INIT != mState) {
            isOpen = true;
        }
        Log.d(TAG, "[isOpen] isOpen:" + isOpen);
        return isOpen;
    }

    @Override
    public void close() {
        State state = getCurrentState();
        Log.d(TAG, "[close]state = " + state);
        if (state == State.STATE_INIT) {
            return;
        } else if (state == State.STATE_CAPTURING) {
            // if camera is closed before focus up, we will call stop and done.
            stopContinuousShot(true);
            onConinuousShotDone(mCurrentShotsNum);
        } else if (state == State.STATE_STOPPED) {
            // if camera is closed before onConinuousShotDone(), the callback
            // may can't notify to application, so we fake one here.
            onConinuousShotDone(mCurrentShotsNum);
        }
        mCaptureSound.release();
        mHandler.removeMessages(RESTART_PREVIEW);
        setState(State.STATE_INIT);
    }

    @Override
    public boolean isSupport() {
        updateCameraDevice();
        if (mICameraDevice == null) {
            return false;
        }
        List<String> captureMode = mICameraDevice.getParameters().getSupportedCaptureMode();
        boolean isSupport = captureMode == null ? false
                : captureMode.indexOf(CONTINUOUS_SHOT_MODE) >= 0;
        Log.i(TAG, "[isSupport]isSupport = " + isSupport);

        return isSupport;
    }

    @Override
    public boolean execute(ActionType type, Object... arg) {
        Log.d(TAG, "[execute] ActionType = " + type);
        switch (type) {
        case ACTION_SHUTTER_BUTTON_LONG_PRESS:
            // long press shutter button to start CS.
            if (canShot()) {
                startContinuousShot();
            }
            return true;
        case ACTION_SHUTTER_BUTTON_FOCUS:
            // release shutter button to stop CS.
            if (arg.length != 1) {
                Log.e(TAG, "[execute]Shutter button focus parameter error!");
                return false;
            }
            boolean press = (Boolean) arg[0];
            Log.d(TAG, "[execute]press is " + press);
            if (!press) {
                stopContinuousShot(false);
            }
            return true;

        case ACTION_ON_COMPENSATION_CHANGED:
            Log.d(TAG, "[execute]onOrientation = " + arg[0]);
            if (mIsSupportIndicator) {
                mICameraView.onOrientationChanged((Integer) arg[0]);
            }
            break;

        case ACTION_ON_BACK_KEY_PRESS:
            State state = getCurrentState();
            Log.d(TAG, "[execute]state = " + state);
            // ignore back action while we're taking continuous shot and saving.
            if (state == State.STATE_CAPTURING || state == State.STATE_STOPPED
                    || state == State.STATE_SAVING) {
                return true;
            }
            break;

        case ACTION_ON_CAMERA_PARAMETERS_READY:
            updateParameters();
            updateFocusManager();
            mICameraView.onOrientationChanged(mIModuleCtrl.getOrientationCompensation());
            break;

        default:
            break;
        }
        return false;
    }

    @Override
    public boolean execute(AdditionActionType type, Object... arg) {
        Log.d(TAG, "[execute] AdditionActionType = " + type);
        switch (type) {
        case ACTION_TAKEN_PICTURE:
            return takePicture();

        case ACTION_ON_STOP_PREVIEW:
            stopContinuousShot(true);
            return true;

        default:
            break;
        }
        return false;
    }

    // Running in main thread
    @Override
    public void onConinuousShotDone(int capNum) {
        Log.d(TAG, "[onContinuousShotCallback]Capture number = " + capNum);
        showSavingProcess(true);
        if (mIsSupportIndicator) {
            mICameraView.hide();
        }
        mICameraDevice.setContinuousShotCallback(null);
        setState(State.STATE_SAVING);
        mWaitSavingDoneThread = new WaitSavingDoneThread();
        mWaitSavingDoneThread.start();
    }

    private void updateParameters() {
        mICameraDevice = mICameraDeviceManager.getCameraDevice(mICameraDeviceManager
                .getCurrentCameraId());
    }

    // Running in main thread
    private void startContinuousShot() {
        Log.d(TAG, "[startContinuousShot]State = " + getCurrentState());
        if (getCurrentState() != State.STATE_OPENED) {
            return;
        }
        // storage space check
        if (!mIFileSaver.isEnoughSpace()) {
            Log.w(TAG, "[startContinuousShot]Don't have enough storage!");
            return;
        }
        if (mICameraContext.getFeatureConfig().isLowRamOptSupport()) {
            mMaxCaptureNum = DEFAULT_CONINUOUS_CAPTURE_NUM;
        } else {
            // get mMaxCaptureNum from setting controller
            mMaxCaptureNum = Integer.valueOf(mISettingCtrl
                    .getSettingValue(SettingConstants.KEY_CONTINUOUS_NUMBER));
        }

        if (isDngOpened()) {
            mIsDngOpenedBeforeCs = true;
            closeDng();
        }

        mCurrentShotsNum = 0;
        clearMemoryLimit();
        mMemoryManager.init(getLeftStorage());
        mIModuleCtrl.disableOrientationListener();
        mICameraAppUi.setThumbnailRefreshInterval(THUMBNAIL_REFRESH_CONTINUOUS);
        mICameraDevice.getParameters().setBurstShotNum(mMaxCaptureNum);
        mICameraDevice.getParameters().setCaptureMode(CONTINUOUS_SHOT_MODE);
        if (MTK_CHIP_0321.equals(mICameraContext.getFeatureConfig().whichDeanliChip())
                && "on".equals(mISettingCtrl.getSettingValue(SettingConstants.KEY_CAMERA_ZSD))) {
            if (getSupportedValues(mICameraDevice.getParameters(), "3dnr-mode-values").indexOf(
                    "off") >= 0) {
                mICameraDevice.getParameters().set("3dnr-mode", "off");
            }
        }
        mICameraDevice.applyParameters();
        mICameraDevice.setContinuousShotCallback(this);
        // must set capturing state before capture to distinguish with normal
        // capture.
        setState(State.STATE_CAPTURE_STARTED);
        // clear focus view and face view before CS.
        mIFocusManager.clearFocusAndFaceUi();
        mIFocusManager.focusAndCapture();
    }

    private void stopContinuousShot(boolean isPreviewStopped) {
        State state = getCurrentState();
        Log.d(TAG, "[stopContinuousShot]state = " + state);
        if (state != State.STATE_CAPTURING
                && state != State.STATE_CAPTURE_STARTED) {
            return;
        }
        // continuous will be stopped when stop preview.
        if (!isPreviewStopped) {
            mICameraDevice.cancelContinuousShot();
        }
        mICameraDevice.getParameters().setCaptureMode(ParametersHelper.KEY_SCENE_MODE_NORMAL);
        if (MTK_CHIP_0321.equals(mICameraContext.getFeatureConfig().whichDeanliChip())
                && "on".equals(mISettingCtrl.getSettingValue(SettingConstants.KEY_CAMERA_ZSD))) {
            if (getSupportedValues(mICameraDevice.getParameters(), "3dnr-mode-values").indexOf(
                    mISettingCtrl.getSettingValue(SettingConstants.KEY_VIDEO_3DNR)) >= 0) {
                mICameraDevice.getParameters().set("3dnr-mode",
                        mISettingCtrl.getSettingValue(SettingConstants.KEY_VIDEO_3DNR));
            }
        }
        mICameraDevice.applyParameters();
        // if no image receive, don't show saving process
        if (mCurrentShotsNum != 0) {
            showSavingProcess(false);
        }
        // the takePicture maybe lost during auto focus, so we need to restore
        // the ui.
        if (state == State.STATE_CAPTURE_STARTED) {
            mHandler.sendEmptyMessage(BACK_TO_NORMAL_VIEW);
            setState(State.STATE_OPENED);
        } else {
            mCaptureSound.stop();
            setState(State.STATE_STOPPED);
        }
    }

    // Running in main thread
    private boolean takePicture() {
        Log.d(TAG, "[takePicture]...");
        if (getCurrentState() != State.STATE_CAPTURE_STARTED) {
            Log.w(TAG, "[takePicture]Don't in Continuous Shot mode!");
            return false;
        }
        // specify the focus mode for continuous shot.
        List<String> supportedFocusModes = mICameraDevice.getParameters().getSupportedFocusModes();
        if (supportedFocusModes != null
                && supportedFocusModes.indexOf(FOCUS_MODE) >= 0) {
            mICameraDevice.getParameters().setFocusMode(FOCUS_MODE);
        }
        mICameraDevice.applyParameters();
        mICameraDevice.takePicture(mShutterCallback, null, null, mJpegPictureCallback);
        mICameraAppUi.setViewState(ViewState.VIEW_STATE_CONTINUOUS_CAPTURE);
        setState(State.STATE_CAPTURING);
        return true;
    }

    // Running in main thread
    private final ShutterCallback mShutterCallback = new ShutterCallback() {
        public void onShutter() {
            Log.d(TAG, "[onShutter]mState = " + getCurrentState());
            // start play sound only at first time when continuous shot
            if (getCurrentState() == State.STATE_CAPTURING) {
                mMemoryManager.start();
                mCaptureSound.play();
                mPictureTakenDate.setTime(System.currentTimeMillis());
                // start to show speed indicator 0/MaxNum
                // showSpeedIndicator();
            }
        }
    };

    // Running in main thread
    private final PictureCallback mJpegPictureCallback = new PictureCallback() {
        public void onPictureTaken(byte[] jpegData, Camera camera) {
            // state check
            if (getCurrentState() != State.STATE_CAPTURING) {
                Log.w(TAG, "[onPictureTaken]Continuous Shot haven't start or have stopped!");
                return;
            }
            // Jpeg data check, if it can't allocate enough memory, need to stop
            // shots
            if (jpegData == null) {
                Log.w(TAG, "[onPictureTaken]Data is null!");
                stopContinuousShot(false);
                return;
            }
            // storage space check
            if (!mIFileSaver.isEnoughSpace()) {
                Log.w(TAG, "[onPictureTaken]Don't have enough storage!");
                stopContinuousShot(false);
                return;
            }
            String name = createFileName(++mCurrentShotsNum);
            showSpeedIndicator();
            mIFileSaver.savePhotoFile(jpegData, name, System.currentTimeMillis(),
                                        mIModuleCtrl.getLocation(), 0, null);
            if (mCurrentShotsNum == mMaxCaptureNum) {
                stopContinuousShot(false);
            }
            MemoryAction action = mMemoryManager.getMemoryAction(jpegData.length,
                    mIFileSaver.getWaitingDataSize());
            if (action == MemoryAction.STOP) {
                stopContinuousShot(false);
            } else if (action == MemoryAction.ADJSUT_SPEED) {
                mICameraDevice.setContinuousShotSpeed(mMemoryManager
                        .getSuitableContinuousShotSpeed());
            }
            Log.i(TAG, "[onPictureTaken]mCurrentShotsNum = " + mCurrentShotsNum);
        }
    };

    private void setState(State newState) {
        // state change flow: INIT --> OPENED --> CAPTURING --> SAVING
        if (mState == newState) {
            return;
        }
        switch (newState) {
        case STATE_OPENED:
            // INIT --> OPENED when open() is called or
            // SAVING --> OPENED when saving done.
            // CAPTURE_STARTED --> OPENED start and cancel CS during focusing.
            if (mState != State.STATE_INIT && mState != State.STATE_SAVING
                    && mState != State.STATE_CAPTURE_STARTED) {
                Log.e(TAG, "[setState]Error!");
            }
            break;
        case STATE_CAPTURE_STARTED:
            // OPENED --> CAPTURE_STARTED when start continuous shot.
            if (mState != State.STATE_OPENED) {
                Log.e(TAG, "[setState]Error!");
            }
            break;
        case STATE_CAPTURING:
            // CAPTURE_STARTED --> CAPTURING when takePicture.
            if (mState != State.STATE_CAPTURE_STARTED) {
                Log.e(TAG, "[setState]Error!");
            }
            break;
        case STATE_STOPPED:
            // CAPTURING --> STOPPED when stop continuous shot.
            if (mState != State.STATE_CAPTURING) {
                Log.e(TAG, "[setState]Error!");
            }
            break;
        case STATE_SAVING:
            // STOPPED --> SAVING when continuous shot done.
            if (mState != State.STATE_STOPPED) {
                Log.e(TAG, "[setState]Error!");
            }
            break;
        case STATE_INIT:
            // OPENED --> INIT when close() is called.
            // SAVING --> INIT close() is called when saving.
            if (mState != State.STATE_OPENED && mState != State.STATE_SAVING) {
                Log.e(TAG, "[setState]Error!");
            }
            break;
        default:
            break;
        }
        mState = newState;
    }

    private State getCurrentState() {
        return mState;
    }

    private class WaitSavingDoneThread extends Thread {
        @Override
        public void run() {
            mIFileSaver.waitDone();
            if (mIsDngOpenedBeforeCs) {
                mIsDngOpenedBeforeCs = false;
                openDng();
            }
            ContinuousShot.State state = getCurrentState();
            Log.i(TAG, "[WaitSavingDoneThread]state = " + state);
            if (state == ContinuousShot.State.STATE_SAVING) {
                setState(ContinuousShot.State.STATE_OPENED);
                mHandler.sendEmptyMessage(RESTART_PREVIEW);
            }
            mHandler.sendEmptyMessage(BACK_TO_NORMAL_VIEW);
        }
    }

    // Must running in main thread
    private void showSavingProcess(boolean isShotDone) {
        // add the check to avoid for scene that
        // quickly press up before continuous sound is playing.
        if (isShotDone && mCurrentShotsNum == 0) {
            Log.d(TAG, "[showSavingProcess]CurrentNum = " + mCurrentShotsNum);
            return;
        }
        String message = mActivity.getString(R.string.saving);
        if (isShotDone) {
            message = String.format(Locale.ENGLISH,
                    mActivity.getString(R.string.continuous_saving_pictures), mCurrentShotsNum);
        }
        mICameraAppUi.showProgress(message);
        if (mListener != null) {
            mListener.onFileSaveing();
        }
    }

    // Must running in main thread
    private void dismissSavingProgress() {
        Log.d(TAG, "[dismissSavingProgress]");
        mICameraAppUi.dismissProgress();
        mICameraAppUi.restoreViewState();
        mICameraAppUi.setSwipeEnabled(true);
    }

    // Must running in main thread
    private void showSpeedIndicator() {
        if (!mIsSupportIndicator) {
            return;
        }
        if (getCurrentState() != State.STATE_CAPTURING) {
            Log.w(TAG, "[showSpeedIndicator]ContinuousShot don't run!");
            return;
        }
        Log.d(TAG, "[showSpeedIndicator]mCurrentShotsNum = " + mCurrentShotsNum);
        mICameraView.update(0, String.format(Locale.ENGLISH, "%02d", mCurrentShotsNum)
                                + "/" + Integer.toString(mMaxCaptureNum));
        mHandler.removeMessages(HIDE_SPEED_INDICATOR);
        mHandler.sendEmptyMessageDelayed(HIDE_SPEED_INDICATOR, SPEED_INDICATOR_SHOE_TIME);
    }

    /**
     * Handler let some task execute in main thread.
     *
     */
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "[handleMessage]msg.what = " + msg.what);
            switch (msg.what) {
            case HIDE_SPEED_INDICATOR:
                if (mIsSupportIndicator) {
                    mICameraView.uninit();
                }
                break;

            case BACK_TO_NORMAL_VIEW:
                mICameraAppUi.setThumbnailRefreshInterval(THUMBNAIL_REFRESH_NORMAL);
                mIModuleCtrl.enableOrientationListener();
                dismissSavingProgress();
                break;

            case RESTART_PREVIEW:
                if (mListener != null) {
                    if (getCurrentState() ==  State.STATE_OPENED) {
                        mListener.restartPreview(false);
                    }
                } else {
                    Log.e(TAG, "[handleMessage]mListener is null, can't restart preview!");
                }
                break;

            default:
                break;
            }
        }
    }

    boolean showContinuousNonsupportInfo(String featureKey, int resId) {
        String info = null;
        if (featureKey == null) {
            if (mIModuleCtrl.isImageCaptureIntent() || !isSupport()) {
                info = mActivity.getString(resId);
                mICameraAppUi.showInfo(info);
                Log.d(TAG, "[showContinuousNonsupportInfo]" + info);
                return true;
            }
        } else {
            if (FEATURE_ON.equals(mISettingCtrl.getSettingValue(featureKey))) {
                info = mActivity.getString(resId)
                        + mActivity.getString(R.string.camera_continuous_not_supported);
                mICameraAppUi.showInfo(info);
                Log.d(TAG, "[showContinuousNonsupportInfo]info: " + info);
                return true;
            }
        }
        return false;
    }

    // Must running in main thread
    private boolean canShot() {
        int featureNum = mFeatureKey.length;
        Log.d(TAG, "[canShot]featureNum = " + featureNum);
        for (int i = 0; i < featureNum; i++) {
            if (showContinuousNonsupportInfo(mFeatureKey[i], mFeatureResId[i])) {
                return false;
            }
        }
        return true;
    }

    private String createFileName(int count) {
        return mFormat.format(mPictureTakenDate) +
                FILE_NAME_CONNECTOR + count + FILE_NAME_SUFFIX;
    }

    private void clearMemoryLimit() {
        if (mIsClearMemoryLimit) {
            Log.d(TAG, "[clearMemoryLimit]Clearing");
        }
        long start = System.currentTimeMillis();
        dalvik.system.VMRuntime.getRuntime().clearGrowthLimit();
        long duration = System.currentTimeMillis() - start;
        mIsClearMemoryLimit = true;
    }

    private long getLeftStorage() {
        long pictureRemaining = mIFileSaver.getAvailableSpace() - mLowStorageThreshold;
        return pictureRemaining;
    }

    private static ArrayList<String> split(String str) {
        ArrayList<String> substrings = null;
        if (str != null) {
            TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
            splitter.setString(str);
            substrings = new ArrayList<String>();
            for (String s : splitter) {
                substrings.add(s);
            }
        }
        return substrings;
    }

    private static List<String> getSupportedValues(Parameters parameters, String key) {
        List<String> supportedList = null;
        if (parameters != null) {
            String str = parameters.get(key);
            supportedList = split(str);
        }
        return supportedList;
    }

    private boolean isDngOpened() {
        return "on".equalsIgnoreCase(mICameraContext.getSettingController()
                .getSettingValue(SettingConstants.KEY_DNG));
    }

    private void closeDng() {
        Log.d(TAG, "[closeDng]...");
        mIFileSaver.setRawFlagEnabled(false);
      //TODO: Dng BSP build error
//        mICameraDevice.setRawImageCallback(null, null);
        mICameraContext.getSettingController().setSettingValue(SettingConstants.KEY_DNG,
                "off", mICameraDevice.getCameraId());
    }

    private void openDng() {
        Log.d(TAG, "[openDng]");
        // Do not need to set raw image callback and file saver flag here, because that
        // they are set when capture.
        mICameraContext.getSettingController().setSettingValue(SettingConstants.KEY_DNG,
                "on", mICameraDevice.getCameraId());
    }
}
