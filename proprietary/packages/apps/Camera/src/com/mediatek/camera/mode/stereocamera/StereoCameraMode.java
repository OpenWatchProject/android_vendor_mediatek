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

package com.mediatek.camera.mode.stereocamera;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.hardware.Camera;
//TODO: Dng BSP build error
//import android.hardware.Camera.MetadataCallback;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureResult;

import android.location.Location;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Size;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.android.camera.R;
import com.android.camera.Util;
import com.android.camera.ui.FocusIndicatorRotateLayout;
import com.android.camera.ui.RotateLayout;
//import com.mediatek.accessor.StereoInfoAccessor;
//import com.mediatek.accessor.data.StereoCaptureInfo;
import com.mediatek.camera.ICameraAddition.AdditionActionType;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ICameraMode.CameraModeType;
import com.mediatek.camera.mode.PhotoMode;
import com.mediatek.camera.platform.ICameraAppUi.SpecViewType;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.StereoDataCallback;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.StereoWarningCallback;
import com.mediatek.camera.platform.ICameraView;
import com.mediatek.camera.platform.IFileSaver.FILE_TYPE;
import com.mediatek.camera.platform.IFileSaver.OnFileSavedListener;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.mode.stereocamera.settingrule.StereoFdRule;
import com.mediatek.camera.mode.stereocamera.settingrule.StereoPictureSizeRule;
import com.mediatek.camera.mode.stereocamera.settingrule.StereoPreviewRatioRule;
import com.mediatek.camera.mode.stereocamera.settingrule.StereoSettingRule;
import com.mediatek.camera.mode.stereocamera.settingrule.StereoVideoQualityRule;
import com.mediatek.camera.mode.stereocamera.settingrule.StereoZsdRule;
import com.mediatek.camera.util.Log;


import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * This class used for Stereo Camera capture.
 */
public class StereoCameraMode extends PhotoMode implements StereoView.Listener {
    private static final String TAG = "StereoCameraMode";
    // Stereo Photo warning message
    private static final int DUAL_CAMERA_LENS_COVERED = 0;
    private static final int DUAL_CAMERA_LOW_LIGHT = 1;
    private static final int DUAL_CAMERA_TOO_CLOSE = 2;
    private static final int DUAL_CAMERA_READY = 3;
    private static final int PASS_NUM = 0;
    private static final int FAIL_NUM = 2;
    private static final int MSG_INIT_VIEW = 10003;
    private static final int MSG_WRITE_XMP = 10004;
    private static final int MSG_CONFIGURATION_CHANGED = 10005;
    private static final int TAG_REFOCUS_IMAGE = 1;
    private static final int TAG_NORAML_IMAGE = 0;
    private static final int VS_DOF_CALLBACK_NUM = 6;
    private static final int REFOCUS_CALLBACK_NUM = 3;
    private static final int TIME_MILLS = 1000;
    //Event: notify view update
    private static final int TOUCH_EVENT = 0;

    private static final String KEY_REFOCUS_PICTURE_SIZE = "refocus-picture-size";
    private static final String KEY_VS_DOF_LEVEL = "stereo-dof-level";
    private static final String REFOCUS_TAG = "refocus";
    private static final String SUBFFIX_JPG_TAG = ".jpg";
    private static final String SUBFFIX_DNG_TAG = ".dng";
    private static final String SUBFFIX_STEREO_TAG = "_STEREO";
    private static final String SUBFFIX_RAW_TAG = "_RAW";
    private static final String GEO_QUALITY = "Geometric Quality: ";
    private static final String PHO_QUALITY = "Photo Quality: ";
    private static final String PASS = "Pass";
    private static final String WARNING = "Pass(limited)";
    private static final String FAIL = "Fail";

    private boolean mIsDualCameraReady = true;
    private boolean mIsStereoCapture = true;
    private boolean mIsDngCapture = false;
    private final StereoPhotoDataCallback mStereoPhotoDataCallback = new StereoPhotoDataCallback();
    private final WarningCallback mStereoCameraWarningCallback = new WarningCallback();
    private final Handler mHandler;
    //private StereoInfoAccessor mAccessor;
    private final SaveHandler mSaveHandler;

    private ICameraView mStereoView;
    private int mCurrentNum = 0;
    private byte[] mJpsData;
    private byte[] mMaskAndConfigData;
    private byte[] mDepthMap;
    private byte[] mClearImage;
    private byte[] mLdcData;
    private byte[] mN3dData;
    private byte[] mDepthWrapperData;
    private byte[] mOriginalJpegData;
    private byte[] mXmpJpegData;

    private long mRawPictureCallbackTime;
    private long mShutterCallbackTime;
    private Date mCaptureDate = new Date();
    private SimpleDateFormat mFormat;
    private String mImageName;
    private long mLastDate = 0;
    private int mSameSecondCount = 0;
    private Thread mWaitSavingDoneThread;

    /**
     * Create a stereo camera mode.
     * @param cameraContext camera context instance.
     */
    public StereoCameraMode(ICameraContext cameraContext) {
        super(cameraContext);
        Log.d(TAG, "[StereoCameraMode]constructor...");
        mHandler = new StereoPhotoHandler(mActivity.getMainLooper());
        mCameraCategory = new StereoPhotoCategory();
        //mAccessor = new StereoInfoAccessor();
        mFormat = new SimpleDateFormat(mActivity.getString(R.string.image_file_name_format),
                Locale.ENGLISH);
        HandlerThread ht = new HandlerThread("Stereo Save Handler Thread");
        ht.start();
        mSaveHandler = new SaveHandler(ht.getLooper());
        setRefocusSettingRules(cameraContext);
        setModeState(ModeState.STATE_CLOSED);
    }

    @Override
    public boolean open() {
        Log.d(TAG, "[openMode] ...");
        super.open();
        mHandler.sendEmptyMessage(MSG_INIT_VIEW);
        mICameraAppUi.setGestureListener(mStereoGestureListener);
        return true;
    }

    @Override
    public boolean close() {
        Log.d(TAG, "[closeMode]...");
        if (mICameraDevice != null && ParametersHelper.isVsDofSupported(
                mICameraDevice.getParameters())) {
            if (CameraModeType.EXT_MODE_VIDEO_STEREO != mIModuleCtrl.getNextMode()) {
                mStereoView.reset();
            }
            uninitStereoView();
        }
        mICameraAppUi.setGestureListener(null);
        mWaitSavingDoneThread = new WaitSavingDoneThread();
        mWaitSavingDoneThread.start();
        if (mSaveHandler != null) {
            mSaveHandler.getLooper().quit();
        }
        super.close();
        return true;
    }

    @Override
    public boolean execute(ActionType type, Object... arg) {
        if (type != ActionType.ACTION_ORITATION_CHANGED) {
            Log.d(TAG, "[execute]type = " + type);
        }
        switch (type) {
        case ACTION_SHUTTER_BUTTON_LONG_PRESS:
            mICameraAppUi.showInfo(mActivity.getString(R.string.accessibility_switch_to_dual_camera)
                            + mActivity.getString(R.string.camera_continuous_not_supported));
            break;

        case ACTION_ON_CAMERA_OPEN:
            updateDevice();
            mCameraClosed = false;
            mICameraDevice.setStereoWarningCallback(mStereoCameraWarningCallback);
            if (ParametersHelper.isVsDofSupported(mICameraDevice.getParameters())) {
                ParametersHelper.setVsDofMode(mICameraDevice.getParameters(), true);
                ParametersHelper.setStereoCaptureMode(mICameraDevice.getParameters(), true);
            } else {
                ParametersHelper.setStereoCaptureMode(mICameraDevice.getParameters(), true);
                ParametersHelper.setVsDofMode(mICameraDevice.getParameters(), false);
            }
            ParametersHelper.setDenoiseMode(mICameraDevice.getParameters(), false);
            break;

        case ACTION_ON_START_PREVIEW:
            super.execute(type, arg);
            mHandler.sendEmptyMessage(MSG_INIT_VIEW);
            break;

        case ACTION_ON_CONFIGURATION_CHANGED:
            if (mHandler != null) {
                mHandler.sendEmptyMessage(MSG_CONFIGURATION_CHANGED);
            }
            break;

        case ACTION_ON_SINGLE_TAP_UP:
            Parameters params = mICameraDevice.getParameters();
            if (params == null || (params != null &&
                    params.getMaxNumFocusAreas() > 0)) {
                return super.execute(type, arg);
            }
            if (!mCameraClosed && ModeState.STATE_IDLE == getModeState()) {
                mStereoView.update(TOUCH_EVENT, (Integer) arg[1], (Integer) arg[2]);
            }
            break;

        default:
            return super.execute(type, arg);
        }
        return true;
    }
    @Override
    public void resume() {

    }
    @Override
    public void pause() {
    }

    @Override
    public void onVsDofLevelChanged(String level) {
        setVsDofLevelParameter(level);
    }

    @Override
    public void onTouchPositionChanged(String value) {
        setTouchPositionParameter(value);
    }

    @Override
    protected PictureCallback getUncompressedImageCallback() {
        return null;
    }

    @Override
    public boolean capture() {
        Log.d(TAG, "capture()");
        mCurrentNum = 0;
        mIsStereoCapture = mIsDualCameraReady;
        if ("on".equals(mISettingCtrl.getSettingValue(SettingConstants.KEY_DNG))) {
            mIFileSaver.setRawFlagEnabled(true);
            mIFileSaver.init(FILE_TYPE.RAW, 0, null, -1);
            mIFileSaver.init(FILE_TYPE.JPEG, 0, null, -1);
          //TODO: Dng BSP build error
//            mICameraDevice.setRawImageCallback(
//                    mRawMetadataCallback, mRawPictureCallback);
            mIsDngCapture = true;
        } else {
            mIFileSaver.setRawFlagEnabled(false);
            mIFileSaver.init(FILE_TYPE.JPEG, 0, null, -1);
          //TODO: Dng BSP build error
//            mICameraDevice.setRawImageCallback(null, null);
            mIsDngCapture = false;
        }
        mCaptureStartTime = System.currentTimeMillis();
        mCaptureDate.setTime(mCaptureStartTime);
        mImageName = createName();
        mICameraAppUi.showRemaining();
        mCameraCategory.takePicture();
        setModeState(ModeState.STATE_CAPTURING);
        return true;
    }

    /**
     * This class used for Stereo view.
     */
    private class StereoPhotoHandler extends Handler {
        public StereoPhotoHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "[handleMessage]msg.what= " + msg.what);
            switch (msg.what) {
            case MSG_INIT_VIEW:
                if (mICameraDevice != null &&
                ParametersHelper.isVsDofSupported(mICameraDevice.getParameters())) {
                    initStereoView();
                }
                break;
            case MSG_CONFIGURATION_CHANGED:
                // because configuration change,so need re-inflate the view
                // layout
                reInitStereoView();
                break;
            default:
                break;
            }
        }
    }

    /**
     * This class used for write jpeg to xmp and saving.
     */
    private class SaveHandler extends Handler {
        SaveHandler(Looper looper) {
            super(looper);
        }
        @Override
        public void handleMessage(final Message msg) {
            Log.d(TAG, "Save handleMessage msg.what = " + msg.what
                 + ", msg.obj = " + msg.obj);
            switch (msg.what) {
            case MSG_WRITE_XMP:
                StereoDataGroup mDataGroup = (StereoDataGroup) msg.obj;
                /*StereoCaptureInfo captureInfo = new StereoCaptureInfo();
                captureInfo.debugDir = mDataGroup.getPictureName();
                captureInfo.jpsBuffer = mDataGroup.getJpsData();
                captureInfo.jpgBuffer = mDataGroup.getOriginalJpegData();
                captureInfo.configBuffer = mDataGroup.getMaskAndConfigData();
                captureInfo.clearImage = mDataGroup.getClearImage();
                captureInfo.depthMap = mDataGroup.getDepthMap();
                captureInfo.ldc = mDataGroup.getLdcData();
                captureInfo.debugBuffer = mDataGroup.getN3dDebugData();
                captureInfo.depthBuffer = mDataGroup.getDepthWrapper();
                mXmpJpegData = mAccessor.writeStereoCaptureInfo(captureInfo);*/
                Log.d(TAG, "notifyMergeData mXmpJpegData: " + mXmpJpegData);
                if (mXmpJpegData != null) {
                    saveFile(mXmpJpegData, TAG_REFOCUS_IMAGE, mDataGroup.getPictureName());
                }
                break;
            default:
                break;
            }
        }
    }

    private OnFileSavedListener mFileSaverListener = new OnFileSavedListener() {
        @Override
        public void onFileSaved(Uri uri) {
            Log.d(TAG, "[onFileSaved]uri= " + uri);
        }
    };

    /**
     * This class used for wait file saving done.
     */
    private class WaitSavingDoneThread extends Thread {
        @Override
        public void run() {
            Log.d(TAG, "[WaitSavingDoneThread]wait");
            mIFileSaver.waitDone();
            Log.d(TAG, "[WaitSavingDoneThread]waitDone!");
        }
    }

    private void saveFile(byte[] data, int refocus, String fileName) {
        Log.i(TAG, "[saveFile]...");
        Location location = mIModuleCtrl.getLocation();
        mIFileSaver.savePhotoFile(data, fileName, mCaptureStartTime, location, refocus,
                mFileSaverListener);
    }

    private void setRefocusSettingRules(ICameraContext cameraContext) {
        Log.d(TAG, "[setRefocusSettingRules]...");
        StereoSettingRule previewSizeRule = new StereoPreviewRatioRule(cameraContext,
                StereoSettingRule.STEREO_CAPTURE);
        previewSizeRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_REFOCUS,
                SettingConstants.KEY_PICTURE_RATIO, previewSizeRule);
        StereoSettingRule pictureSizeRule = new StereoPictureSizeRule(cameraContext,
                StereoSettingRule.STEREO_CAPTURE);
        pictureSizeRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_REFOCUS,
                SettingConstants.KEY_PICTURE_SIZE, pictureSizeRule);
        // For VsDof Capture
        StereoSettingRule zsdRule = new StereoZsdRule(cameraContext,
                StereoSettingRule.VSDOF);
        zsdRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_REFOCUS,
                SettingConstants.KEY_CAMERA_ZSD, zsdRule);
        StereoSettingRule fdRule = new StereoFdRule(cameraContext,
                StereoSettingRule.VSDOF);
        fdRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_REFOCUS,
                SettingConstants.KEY_CAMERA_FACE_DETECT, fdRule);
    }

    /**
     * This class used for Stereo data callback.
     */
    private class StereoPhotoDataCallback implements StereoDataCallback {
        public void onJpsCapture(byte[] jpsData) {
            if (jpsData == null) {
                Log.w(TAG, "JPS data is null");
                return;
            }
            Log.d(TAG, "onJpsCapture jpsData:" + jpsData.length);
            mJpsData = jpsData;
            notifyMergeData();
        }
        public void onMaskCapture(byte[] maskData) {
            if (maskData == null) {
                Log.w(TAG, "Mask data is null");
                return;
            }
            Log.d(TAG, "onMaskCapture maskData:" + maskData.length);
            mMaskAndConfigData = maskData;
//            mAccessor.setJsonBuffer(mMaskAndConfigData);
//            showQualityStatus(mAccessor.getGeoVerifyLevel(mMaskAndConfigData),
//                    mAccessor.getPhoVerifyLevel(mMaskAndConfigData));
            notifyMergeData();
        }
        public void onDepthMapCapture(byte[] depthData) {
            if (depthData == null) {
                Log.w(TAG, "depth data is null");
                return;
            }
            Log.d(TAG, "onDepthMapCapture depthData:" + depthData.length);
            mDepthMap = depthData;
            notifyMergeData();
        }
        public void onClearImageCapture(byte[] clearImageData) {
            if (clearImageData == null) {
                Log.w(TAG, " clearImage data is null");
                return;
            }
            Log.d(TAG, "onClearImageCapture clearImageData:" + clearImageData.length);
            mClearImage = clearImageData;
            notifyMergeData();
        }
        public void onLdcCapture(byte[] ldcData) {
            if (ldcData == null) {
                Log.w(TAG, " ldc data is null");
                return;
            }
            Log.d(TAG, "onLdcCapture ldcData:" + ldcData.length);
            mLdcData = ldcData;
            notifyMergeData();
        }

        public void onN3dCapture(byte[] n3dData) {
            if (n3dData == null) {
                Log.w(TAG, " n3d data is null");
                return;
            }
            Log.d(TAG, "onN3dCapture n3dData:" + n3dData.length);
            mN3dData = n3dData;
            notifyMergeData();
        }

        public void onDepthWrapperCapture(byte[] depthWrapper) {
            if (depthWrapper == null) {
                Log.w(TAG, " depth wrapper data is null");
                return;
            }
            Log.d(TAG, "onDepthWrapperCapture depthWrapper:" + depthWrapper.length);
            mDepthWrapperData = depthWrapper;
            notifyMergeData();
        }
    }

    private void notifyMergeData() {
        Log.d(TAG, "notifyMergeData mCurrentNum = " + mCurrentNum);
        mCurrentNum++;
        if (ParametersHelper.isVsDofSupported(mICameraDevice.getParameters())) {
            if (mCurrentNum == VS_DOF_CALLBACK_NUM) {
                Log.d(TAG, "notifyMergeData Vs Dof");
                restartPreview(true);
                if (mIsStereoCapture) {
                    String dofName = generateName(SUBFFIX_STEREO_TAG);
                    StereoDataGroup mDataGroup = new StereoDataGroup(dofName,
                            mOriginalJpegData, mJpsData, mMaskAndConfigData,
                            mDepthMap, mClearImage, mLdcData, mN3dData, mDepthWrapperData);
                    mSaveHandler.obtainMessage(MSG_WRITE_XMP, mDataGroup).sendToTarget();
                }
                mCurrentNum = 0;
            }
        } else {
           if (mCurrentNum == REFOCUS_CALLBACK_NUM) {
                Log.d(TAG, "notifyMergeData refocus");
                restartPreview(true);
                if (mIsStereoCapture) {
                    String refocusMame = generateName(SUBFFIX_STEREO_TAG);
                    /*StereoCaptureInfo captureInfo = new StereoCaptureInfo();
                    captureInfo.debugDir = refocusMame;
                    captureInfo.jpsBuffer = mJpsData;
                    captureInfo.jpgBuffer = mOriginalJpegData;
                    captureInfo.configBuffer = mMaskAndConfigData;
                    captureInfo.clearImage = null;
                    captureInfo.depthMap = null;
                    captureInfo.ldc = null;
                    captureInfo.debugBuffer = null;
                    captureInfo.depthBuffer = null;
                    mXmpJpegData = mAccessor.writeStereoCaptureInfo(captureInfo);*/
                    if (mXmpJpegData != null) {
                        saveFile(mXmpJpegData, TAG_REFOCUS_IMAGE, refocusMame);
                    }
                }
                mCurrentNum = 0;
            }
        }
    }

    /**
     * This class used for Stereo warning callback.
     */
    private class WarningCallback implements StereoWarningCallback {
        public void onWarning(int type) {
            Log.d(TAG, "onWarning type = " + type);
            switch (type) {
            case DUAL_CAMERA_LOW_LIGHT:
                mICameraAppUi.showToast(R.string.dual_camera_lowlight_toast);
                mIsDualCameraReady = false;
                break;
            case DUAL_CAMERA_READY:
                mIsDualCameraReady = true;
                break;
            case DUAL_CAMERA_TOO_CLOSE:
                mICameraAppUi.showToast(R.string.dual_camera_too_close_toast);
                mIsDualCameraReady = false;
                break;
            case DUAL_CAMERA_LENS_COVERED:
                mICameraAppUi.showToast(R.string.dual_camera_lens_covered_toast);
                mIsDualCameraReady = false;
                break;
            default:
                Log.w(TAG, "Warning message don't need to show");
                break;
            }
        }
    };

    private final PictureCallback mJpegPictureCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] jpegData, Camera camera) {
            Log.d(TAG, "[mJpegPictureCallback]");
            if (mCameraClosed) {
                Log.w(TAG, "[mJpegPictureCallback] mCameraClosed:" + mCameraClosed);
                mICameraAppUi.setSwipeEnabled(true);
                mICameraAppUi.restoreViewState();
                return;
            }
            if (jpegData == null) {
                Log.w(TAG, "[mJpegPictureCallback] jpegData is null");
                mICameraAppUi.setSwipeEnabled(true);
                mICameraAppUi.restoreViewState();
                restartPreview(false);
                return;
            }
            mOriginalJpegData = jpegData;
            mIFocusManager.updateFocusUI(); // Ensure focus indicator
            if (!mIsStereoCapture) {
                saveFile(mOriginalJpegData, TAG_NORAML_IMAGE, null);
            }
            notifyMergeData();
            Log.d(TAG, "[mJpegPictureCallback] end");
        }
    };

    private String generateName(String type) {
        String name = null;
        if (type == SUBFFIX_RAW_TAG) {
            if (mICameraDevice != null
                && ParametersHelper.isVsDofSupported(mICameraDevice
                           .getParameters())) {
                if (mIsStereoCapture) {
                    name = mImageName + SUBFFIX_STEREO_TAG + SUBFFIX_RAW_TAG
                            + SUBFFIX_DNG_TAG;
                }
            }
        } else {
            if (mICameraDevice != null
                    && ParametersHelper.isVsDofSupported(mICameraDevice
                        .getParameters())) {
                if (mIsDngCapture && mIsStereoCapture) {
                    name = mImageName + SUBFFIX_STEREO_TAG + SUBFFIX_RAW_TAG
                            + SUBFFIX_JPG_TAG;
                } else if (!mIsDngCapture && mIsStereoCapture) {
                    name = mImageName + SUBFFIX_STEREO_TAG + SUBFFIX_JPG_TAG;
                }
             }
        }
        Log.d(TAG, "generateName type = " + type + ", name = " + name);
        return name;
    }

    private final ShutterCallback mShutterCallback = new ShutterCallback() {
        @Override
        public void onShutter() {
            mShutterCallbackTime = System.currentTimeMillis();
            long shutterLag = mShutterCallbackTime - mCaptureStartTime;
            Log.d(TAG, "[mShutterCallback] mShutterLag = " + shutterLag + "ms");
            Log.d(TAG, "[mShutterCallback]");
        }
    };

    private final PictureCallback mRawPictureCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
            mRawPictureCallbackTime = System.currentTimeMillis();
            Log.d(TAG, "mShutterToRawCallbackTime = "
                    + (mRawPictureCallbackTime - mShutterCallbackTime) + "ms");
            if (data == null) {
                Log.d(TAG, "[mRawPictureCallback] data is null ");
                return;
            }
            mDngHelper.setRawdata(data);
            getDngImageAndSaved(null);
        }
    };

  //TODO: Dng BSP build error
//    private final MetadataCallback mRawMetadataCallback = new MetadataCallback() {
//        public void onMetadataReceived(CaptureResult result,
//                CameraCharacteristics characteristic) {
//            Log.i(TAG, "onMetadataReceived");
//            if (result == null || characteristic == null) {
//                Log.w(TAG, "onMetadataReceived, invalid callback value, return null");
//                return;
//            }
//            mDngHelper.setMetadata(result, characteristic);
//            getDngImageAndSaved(null);
//        }
//    };

    /**
     * This class used for Stereo photo take picture.
     */
    class StereoPhotoCategory extends CameraCategory {
        public StereoPhotoCategory() {
        }

        public void takePicture() {
            mAdditionManager.execute(AdditionActionType.ACTION_TAKEN_PICTURE);
            mICameraDevice.setStereoDataCallback(mStereoPhotoDataCallback);
            mICameraDevice.getParameters().setRefocusJpsFileName(REFOCUS_TAG);
            mICameraDevice.takePicture(mShutterCallback, null, null, mJpegPictureCallback);
            mICameraAppUi.setViewState(ViewState.VIEW_STATE_CAPTURE);
        }
    }

    private void showQualityStatus(int geoFlag, int photoFlag) {
        if (!isDebugOpened()) {
            return;
        }
        String msg = null;
        msg = formateShow(geoFlag, photoFlag);
        DialogInterface.OnClickListener buttonListener =
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        };

        new AlertDialog.Builder(mActivity).setCancelable(false)
                .setIconAttribute(android.R.attr.alertDialogIcon).setTitle("")
                .setMessage(msg)
                .setNeutralButton(R.string.dialog_ok, buttonListener)
                .show();
    }

    private String formateShow(int geoFlag, int photoFlag) {
        Log.d(TAG, "geoFlag = " + geoFlag +  "photoFlag = " + photoFlag);
        String geo = null;
        String photo = null;
        if (geoFlag == PASS_NUM) {
            geo = PASS;
        } else if (geoFlag == FAIL_NUM) {
            geo = FAIL;
        } else {
            geo = WARNING;
        }
        if (photoFlag == PASS_NUM) {
            photo = PASS;
        } else if (photoFlag == FAIL_NUM) {
            photo = FAIL;
        } else {
            photo = WARNING;
        }
        return GEO_QUALITY + geo + "\n" + PHO_QUALITY + photo;
    }

    private boolean isDebugOpened() {
        boolean enable = SystemProperties
                .getInt("debug.STEREO.enable_verify", 0) == 1 ? true : false;
        Log.d(TAG, "[isDebugOpened]return :" + enable);
        return enable;
    }

    private void initStereoView() {
        if (mStereoView == null) {
            mStereoView = mICameraAppUi.getCameraView(SpecViewType.MODE_STEREO);
            mStereoView.init(mActivity, mICameraAppUi, mIModuleCtrl);
            mStereoView.setListener(this);
            mStereoView.show();
        } else {
            mStereoView.refresh();
        }
    }

    private void reInitStereoView() {
        if (mStereoView != null) {
            mStereoView.uninit();
            mStereoView = mICameraAppUi.getCameraView(SpecViewType.MODE_STEREO);
            mStereoView.init(mActivity, mICameraAppUi, mIModuleCtrl);
            mStereoView.setListener(this);
            mStereoView.show();
        }
    }

    private void uninitStereoView() {
        if (mStereoView != null) {
            mStereoView.uninit();
        }
    }

    private void setVsDofLevelParameter(String level) {
        Log.d(TAG, "[setVsDofLevelParameter] level = " + level);
        mICameraDevice.setParameter(KEY_VS_DOF_LEVEL, level);
        mICameraDevice.applyParameters();
    }

    private void setTouchPositionParameter(String value) {
        Log.i(TAG, "[setTouchPositionParameter] value = " + value);
        mICameraDevice.setParameter("stereo-touch-position", value);
        mICameraDevice.applyParameters();
    }

    private String createName() {
        String result = mFormat.format(mCaptureDate);
        // If the last name was generated for the same second,
        // we append _1, _2, etc to the name.
        long captureTime = mCaptureStartTime;
        if (captureTime / TIME_MILLS == mLastDate / TIME_MILLS) {
            mSameSecondCount++;
            result += "_" + mSameSecondCount;
        } else {
            mLastDate = captureTime;
            mSameSecondCount = 0;
        }
        Log.d(TAG, "[createName] result = " + result);
        return result;
    }

    private StereoGestureListener mStereoGestureListener = new StereoGestureListener() {
        @Override
        public boolean onSingleTapUp(float x, float y) {
            return false;
        }

        @Override
        public boolean onLongPress(float x, float y) {
            return false;
        }
    };
}
