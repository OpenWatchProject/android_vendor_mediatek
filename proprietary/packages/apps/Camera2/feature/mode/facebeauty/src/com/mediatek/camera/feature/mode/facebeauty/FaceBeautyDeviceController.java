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

package com.mediatek.camera.feature.mode.facebeauty;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCaptureSession.CaptureCallback;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.OutputConfiguration;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Build;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.bgservice.BGServiceKeeper;
import com.mediatek.camera.common.bgservice.CaptureSurface;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManager;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraOpenException;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;
import com.mediatek.camera.common.device.v2.Camera2Proxy;
import com.mediatek.camera.common.device.v2.Camera2Proxy.StateCallback;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.mode.Device2Controller;
import com.mediatek.camera.common.mode.photo.HeifHelper;
import com.mediatek.camera.common.mode.photo.P2DoneInfo;
import com.mediatek.camera.common.mode.photo.ThumbnailHelper;
import com.mediatek.camera.common.mode.photo.device.IDeviceController;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Configurator;
import com.mediatek.camera.common.sound.ISoundPlayback;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.CoordinatesTransform;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.campostalgo.FeatureConfig;
import com.mediatek.campostalgo.FeatureParam;
import com.mediatek.campostalgo.FeaturePipeConfig;
import com.mediatek.campostalgo.FeatureResult;
import com.mediatek.campostalgo.ICamPostAlgoCallback;
import com.mediatek.campostalgo.StreamInfo;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;

import static android.hardware.camera2.CameraMetadata.LENS_FACING_BACK;
import static android.hardware.camera2.CameraMetadata.LENS_FACING_FRONT;

/**
 * An implementation of {@link IDeviceController} with Camera2Proxy.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
class FaceBeautyDeviceController extends Device2Controller implements
        IFaceBeautyDeviceController,
        CaptureSurface.ImageCallback,
        ISettingManager.SettingDevice2Requester {
    private static final Tag TAG = new Tag(FaceBeautyDeviceController.class.getSimpleName());
    private static final String KEY_PICTURE_SIZE = "key_picture_size";
    private static final int CAPTURE_MAX_NUMBER = 5;
    private static final int WAIT_TIME = 5;
    //add for quick preview
    private static final String QUICK_PREVIEW_KEY = "com.mediatek.configure.setting.initrequest";
    private static final int[] QUICK_PREVIEW_KEY_VALUE = new int[]{1};
    private CaptureRequest.Key<int[]> mQuickPreviewKey = null;
    //add for BG service
    private CaptureRequest.Key<int[]> mBGServicePrereleaseKey = null;
    private CaptureRequest.Key<int[]> mBGServiceImagereaderIdKey = null;
    private static final int[] BGSERVICE_PRERELEASE_KEY_VALUE = new int[]{1};
    //add for postAlgo Meta params
    public static final String MTK_POSTALGO_FACE_BEAUTY_ENLARGE_EYE
                              = "postalgo.facebeauty.enlargeeye";
    public static final String MTK_POSTALGO_FACE_BEAUTY_SLIM_FACE = "postalgo.facebeauty.slimface";
    public static final String MTK_POSTALGO_FACE_BEAUTY_SMOOTH = "postalgo.facebeauty.smooth";
    public static final String MTK_POSTALGO_FACE_BEAUTY_BRIGHT = "postalgo.facebeauty.bright";
    public static final String MTK_POSTALGO_FACE_BEAUTY_RUDDY = "postalgo.facebeauty.ruddy";
    public static final String MTK_POSTALGO_FACE_BEAUTY_PHYSICAL_ID
                             = "postalgo.facebeauty.physicalid";
    public static final String MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION_SIZE
                             = "postalgo.facebeauty.facedetectionsize";
    public static final String MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION
                             = "postalgo.facebeauty.facedetection";

    private static final String POSTALGO_PARAMS_JPEG_ORIENTATION_KEY
            = "postalgo.capture.jpegorientation";
    private static final String MTK_POSTALGO_FACE_BEAUTY_MIRROR
            = "postalgo.facebeauty.mirror";

    private final Activity mActivity;
    private final CameraManager mCameraManager;
    private final CaptureSurface mCaptureSurface;
    private final CaptureSurface mThumbnailSurface;
    private final ICameraContext mICameraContext;
    private final Object mSurfaceHolderSync = new Object();
    private final StateCallback mDeviceCallback = new DeviceStateCallback();

    private int mJpegRotation;
    private volatile int mPreviewWidth;
    private volatile int mPreviewHeight;
    private volatile int mCaptureWidth;
    private volatile int mCaptureHeight;
    private volatile Camera2Proxy mCamera2Proxy;
    private volatile Camera2CaptureSessionProxy mSession;

    private boolean mFirstFrameArrived = false;
    private boolean mIsPictureSizeChanged = false;
    private boolean mNeedSubSectionInitSetting = false;
    private volatile boolean mNeedFinalizeOutput = false;

    private Lock mLockState = new ReentrantLock();
    private Lock mDeviceLock = new ReentrantLock();
    private CameraState mCameraState = CameraState.CAMERA_UNKNOWN;

    private String mCurrentCameraId;
    private Surface mPreviewSurface;
    private Surface mCapturePostAlgoSurface;
    private CaptureDataCallback mCaptureDataCallback;
    private Object mSurfaceObject;
    private ISettingManager mSettingManager;
    private DeviceCallback mModeDeviceCallback;
    private SettingController mSettingController;
    private PreviewSizeCallback mPreviewSizeCallback;
    private CameraDeviceManager mCameraDeviceManager;
    private SettingDevice2Configurator mSettingDevice2Configurator;
    private CaptureRequest.Builder mBuilder = null;
    private CaptureRequest.Builder mDefaultBuilder = null;
    private String mZsdStatus = "on";
    private List<OutputConfiguration> mOutputConfigs;
    private CameraCharacteristics mCameraCharacteristics;
    private boolean mIsBGServiceEnabled = false;
    private BGServiceKeeper mBGServiceKeeper;
    private ConcurrentHashMap mCaptureFrameMap = new ConcurrentHashMap<String, Boolean>();
    private FeatureParam mFeatureParam;
    private Rect[] mFaceDetections = null;
    private DataStore mDataStore;

    /**
     * this enum is used for tag native camera open state.
     */
    private enum CameraState {
        CAMERA_UNKNOWN,
        CAMERA_OPENING,
        CAMERA_OPENED,
        CAMERA_CAPTURING,
        CAMERA_CLOSING,
    }

    /**
     * PhotoDeviceController may use activity to get display rotation.
     *
     * @param activity the camera activity.
     */
    FaceBeautyDeviceController(@Nonnull Activity activity, @Nonnull ICameraContext context) {
        LogHelper.d(TAG, "[FaceBeautyDeviceController]");
        mActivity = activity;
        mCameraManager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        mICameraContext = context;
        mBGServiceKeeper = mICameraContext.getBGServiceKeeper();
        if (mBGServiceKeeper != null) {
            DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(
                    mActivity.getApplicationContext()).getDeviceDescriptionMap().get("0");
            if (deviceDescription != null && !isThirdPartyIntent(mActivity)
                    && mBGServiceKeeper.getBGHidleService() != null) {
                mIsBGServiceEnabled = false;
                mBGServicePrereleaseKey = deviceDescription.getKeyBGServicePrerelease();
                mBGServiceImagereaderIdKey = deviceDescription.getKeyBGServiceImagereaderId();
            }
        }
        LogHelper.d(TAG, "mBGServiceKeeper = " + mBGServiceKeeper
                + ", isThirdPartyIntent = " + isThirdPartyIntent(mActivity)
                + ", mIsBGServiceEnabled = " + mIsBGServiceEnabled
                + ", mBGServicePrereleaseKey = " + mBGServicePrereleaseKey
                + ", mBGServiceImagereaderIdKey = " + mBGServiceImagereaderIdKey);
        if (mIsBGServiceEnabled) {
            mCaptureSurface = new CaptureSurface(mBGServiceKeeper.getBGCaptureHandler());
            LogHelper.d(TAG, "BG mCaptureSurface = " + mCaptureSurface);
        } else {
            mCaptureSurface = new CaptureSurface();
        }
        mCaptureSurface.setCaptureCallback(this);
        mThumbnailSurface = new CaptureSurface();
        mThumbnailSurface.setCaptureCallback(this);
        mThumbnailSurface.setFormat(ThumbnailHelper.FORMAT_TAG);
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
        mDataStore = mICameraContext.getDataStore();
    }

    @Override
    public void queryCameraDeviceManager() {
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
    }

    @Override
    public void openCamera(FaceBeautyDeviceInfo info) {
        String cameraId = info.getCameraId();
        boolean sync = info.getNeedOpenCameraSync();
        LogHelper.i(TAG, "[openCamera] cameraId : " + cameraId + ",sync = " + sync);
        if (canOpenCamera(cameraId)) {
            try {
                mDeviceLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
                mNeedSubSectionInitSetting = info.getNeedFastStartPreview();
                mCurrentCameraId = cameraId;
                updateCameraState(CameraState.CAMERA_OPENING);
                initSettingManager(info.getSettingManager());
                doOpenCamera(sync);
                if (mNeedSubSectionInitSetting) {
                    mSettingManager.createSettingsByStage(1);
                } else {
                    mSettingManager.createAllSettings();
                }
                mCameraCharacteristics
                        = mCameraManager.getCameraCharacteristics(mCurrentCameraId);
                mQuickPreviewKey = CameraUtil.getAvailableSessionKeys(
                        mCameraCharacteristics, QUICK_PREVIEW_KEY);
            } catch (CameraOpenException e) {
                if (CameraOpenException.ExceptionType.SECURITY_EXCEPTION == e.getExceptionType()) {
                    CameraUtil.showErrorInfoAndFinish(mActivity,
                            CameraUtil.CAMERA_HARDWARE_EXCEPTION);
                    updateCameraState(CameraState.CAMERA_UNKNOWN);
                    mCurrentCameraId = null;
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            } catch (CameraAccessException | IllegalArgumentException e) {
                CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_HARDWARE_EXCEPTION);
                updateCameraState(CameraState.CAMERA_UNKNOWN);
                mCurrentCameraId = null;
            } finally {
                mDeviceLock.unlock();
            }
        }
    }

    @Override
    public void updatePreviewSurface(Object surfaceObject) {
        LogHelper.d(TAG, "[updatePreviewSurface] surfaceHolder = " + surfaceObject + " state = "
                + mCameraState + ", session :" + mSession + ", mNeedSubSectionInitSetting:"
                + mNeedSubSectionInitSetting);
        synchronized (mSurfaceHolderSync) {
            if (surfaceObject instanceof SurfaceHolder) {
                mPreviewSurface = surfaceObject == null ? null :
                        ((SurfaceHolder) surfaceObject).getSurface();
            } else if (surfaceObject instanceof SurfaceTexture) {
                mPreviewSurface = surfaceObject == null ? null :
                        new Surface((SurfaceTexture) surfaceObject);
            }
            boolean isStateReady = CameraState.CAMERA_OPENED == mCameraState;
            if (isStateReady && mCamera2Proxy != null) {
                boolean onlySetSurface = mSurfaceObject == null && surfaceObject != null;
                mSurfaceObject = surfaceObject;
                if (surfaceObject == null) {
                    stopPreview();
                } else {
                    configureSession(false);
                }
            }
        }
    }

    @Override
    public void setDeviceCallback(DeviceCallback callback) {
        mModeDeviceCallback = callback;
    }

    @Override
    public void setPreviewSizeReadyCallback(PreviewSizeCallback callback) {
        mPreviewSizeCallback = callback;
    }

    /**
     * Set the new picture size.
     *
     * @param size current picture size.
     */
    @Override
    public void setPictureSize(Size size) {
        String formatTag = mSettingController.queryValue(HeifHelper.KEY_FORMAT);
        int format = HeifHelper.getCaptureFormat(formatTag);
        mCaptureSurface.setFormat(formatTag);
        mIsPictureSizeChanged = mCaptureSurface.updatePictureInfo(size.getWidth(),
                size.getHeight(), format, CAPTURE_MAX_NUMBER);
        mCaptureWidth = size.getWidth();
        mCaptureHeight = size.getHeight();
        LogHelper.i(TAG, "[setPictureSize] mCaptureWidth = " + mCaptureWidth + " mCaptureHeight = "
                + mCaptureHeight);
        if (mIsBGServiceEnabled) {
            mBGServiceKeeper.setBGCaptureSurface(mCaptureSurface);
        }
        double ratio = (double) size.getWidth() / size.getHeight();
        ThumbnailHelper.updateThumbnailSize(ratio);
        if (ThumbnailHelper.isPostViewSupported()) {
            mThumbnailSurface.updatePictureInfo(ThumbnailHelper.getThumbnailWidth(),
                    ThumbnailHelper.getThumbnailHeight(),
                    ThumbnailHelper.IMAGE_BUFFER_FORMAT,
                    CAPTURE_MAX_NUMBER);
        }
    }

    /**
     * Check whether can take picture or not.
     *
     * @return true means can take picture; otherwise can not take picture.
     */
    @Override
    public boolean isReadyForCapture() {
        boolean canCapture = mSession != null
                && mCamera2Proxy != null && getCameraState() == CameraState.CAMERA_OPENED;
        LogHelper.i(TAG, "[isReadyForCapture] canCapture = " + canCapture);
        return canCapture;
    }

    @Override
    public void destroyDeviceController() {
        if (mCaptureSurface != null) {
            releaseJpegCaptureSurface();
        }
        if (mThumbnailSurface != null) {
            mThumbnailSurface.release();
        }
    }

    @Override
    public void startPreview() {
        LogHelper.i(TAG, "[startPreview]");
        configureSession(false);
    }

    @Override
    public void stopPreview() {
        LogHelper.i(TAG, "[stopPreview]");
        abortOldSession();
    }


    @Override
    public void takePicture(@Nonnull CaptureDataCallback callback) {
        LogHelper.i(TAG, "[takePicture] mSession= " + mSession);
        if (mSession != null && mCamera2Proxy != null) {
            //face detection meta
            configFaceDetectionMetaParams();
            mCaptureDataCallback = callback;
            updateCameraState(CameraState.CAMERA_CAPTURING);
            try {
                Builder builder = doCreateAndConfigRequest(Camera2Proxy.TEMPLATE_STILL_CAPTURE);
                mSession.capture(builder.build(), mCaptureCallback, mModeHandler);
            } catch (CameraAccessException e) {
                e.printStackTrace();
                LogHelper.e(TAG, "[takePicture] error because create build fail.");
            }
        }
    }

    @Override
    public void updateGSensorOrientation(int orientation) {
        mJpegRotation = orientation;
    }

    @Override
    public void closeSession() {
        LogHelper.i(TAG, "[closeSession] +");
        if (mSession != null) {
            try {
                mSession.abortCaptures();
                mSession.close();
                stopPostAlgo();
            } catch (CameraAccessException e) {
                LogHelper.e(TAG, "[closeSession] exception", e);
            }
        }
        mSession = null;
        mBuilder = null;
        mDefaultBuilder = null;
        LogHelper.i(TAG, "[closeSession] -");
    }

    @Override
    public void closeCamera(boolean sync) {
        LogHelper.i(TAG, "[closeCamera] + sync = " + sync + " current state : " + mCameraState);
        if (CameraState.CAMERA_UNKNOWN != mCameraState) {
            try {
                mDeviceLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
                super.doCameraClosed(mCamera2Proxy);
                updateCameraState(CameraState.CAMERA_CLOSING);
                closeSession();
                if (mModeDeviceCallback != null) {
                    mModeDeviceCallback.beforeCloseCamera();
                }
                doCloseCamera(sync);
                updateCameraState(CameraState.CAMERA_UNKNOWN);
                recycleVariables();
                releaseJpegCaptureSurface();
                mThumbnailSurface.releaseCaptureSurface();
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                super.doCameraClosed(mCamera2Proxy);
                mDeviceLock.unlock();
            }
            recycleVariables();
        }
        mCurrentCameraId = null;
        LogHelper.i(TAG, "[closeCamera] -");
    }

    @Override
    public Size getPreviewSize(double targetRatio) {
        int oldPreviewWidth = mPreviewWidth;
        int oldPreviewHeight = mPreviewHeight;
        getTargetPreviewSize(targetRatio);
        boolean isSameSize = oldPreviewHeight == mPreviewHeight && oldPreviewWidth == mPreviewWidth;
        LogHelper.i(TAG, "[getPreviewSize] old size : " + oldPreviewWidth + " X " +
                oldPreviewHeight + " new  size :" + mPreviewWidth + " X " + mPreviewHeight);
        //if preview size don't change, but picture size changed,need do configure the surface.
        //if preview size changed,do't care the picture size changed,because surface will be
        //changed.
        if (isSameSize && mIsPictureSizeChanged) {
            configureSession(false);
        }
        return new Size(mPreviewWidth, mPreviewHeight);
    }

    @Override
    public void onPictureCallback(byte[] data,
                                  int format, String formatTag, int width, int height) {
        LogHelper.d(TAG, "<onPictureCallback> data = " + data + ", format = " + format
                + ", formatTag" + formatTag + ", width = " + width + ", height = " + height
                + ", mCaptureDataCallback = " + mCaptureDataCallback);
        if (mCaptureDataCallback != null) {
            DataCallbackInfo info = new DataCallbackInfo();
            info.data = data;
            info.needUpdateThumbnail = true;
            info.needRestartPreview = false;
            info.mBufferFormat = format;
            info.imageHeight = height;
            info.imageWidth = width;
            if (ThumbnailHelper.isPostViewSupported()) {
                info.needUpdateThumbnail = false;
            }
            if (ThumbnailHelper.FORMAT_TAG.equalsIgnoreCase(formatTag)) {
                mCaptureDataCallback.onPostViewCallback(data);
            } else {
                mCaptureDataCallback.onDataReceived(info);
                if (mIsBGServiceEnabled && mCaptureSurface != null) {
                    mCaptureSurface.decreasePictureNum();
                    if (mCaptureSurface.shouldReleaseCaptureSurface()
                            && mCaptureSurface.getPictureNumLeft() == 0) {
                        mCaptureSurface.releaseCaptureSurface();
                        mCaptureSurface.releaseCaptureSurfaceLater(false);
                    }
                }
            }
        }

    }

    @Override
    public void createAndChangeRepeatingRequest() {
        if (mCamera2Proxy == null || mCameraState != CameraState.CAMERA_OPENED) {
            LogHelper.e(TAG, "camera is closed or in opening state can't request ");
            return;
        }
        repeatingPreview(true);
    }

    @Override
    public Builder createAndConfigRequest(int templateType) {
        Builder builder = null;
        try {
            builder = doCreateAndConfigRequest(templateType);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        return builder;
    }

    @Override
    public CaptureSurface getModeSharedCaptureSurface() throws IllegalStateException {
        if (CameraState.CAMERA_UNKNOWN == getCameraState()
                || CameraState.CAMERA_CLOSING == getCameraState()) {
            throw new IllegalStateException("get invalid capture surface!");
        } else {
            return mCaptureSurface;
        }
    }

    @Override
    public Surface getModeSharedPreviewSurface() throws IllegalStateException {
        if (CameraState.CAMERA_UNKNOWN == getCameraState()
                || CameraState.CAMERA_CLOSING == getCameraState()) {
            throw new IllegalStateException("get invalid capture surface!");
        } else {
            return mPreviewSurface;
        }
    }

    @Override
    public Surface getModeSharedThumbnailSurface() throws IllegalStateException {
        if (CameraState.CAMERA_UNKNOWN == getCameraState()
                || CameraState.CAMERA_CLOSING == getCameraState()) {
            throw new IllegalStateException("get invalid capture surface!");
        } else {
            return mThumbnailSurface.getSurface();
        }
    }

    @Override
    public Camera2CaptureSessionProxy getCurrentCaptureSession() {
        return mSession;
    }

    @Override
    public void requestRestartSession() {
        configureSession(false);
    }

    @Override
    public int getRepeatingTemplateType() {
        return Camera2Proxy.TEMPLATE_PREVIEW;
    }

    /**
     * Judge current is launch by intent.
     *
     * @param activity the launch activity.
     * @return true means is launch by intent; otherwise is false.
     */
    protected boolean isThirdPartyIntent(Activity activity) {
        Intent intent = activity.getIntent();
        String action = intent.getAction();
        boolean value = MediaStore.ACTION_IMAGE_CAPTURE.equals(action) ||
                MediaStore.ACTION_VIDEO_CAPTURE.equals(action);
        return value;
    }

    private void releaseJpegCaptureSurface() {
        if (!mIsBGServiceEnabled) {
            mCaptureSurface.releaseCaptureSurface();
        } else {
            if (mCaptureSurface.getPictureNumLeft() != 0) {
                mCaptureSurface.releaseCaptureSurfaceLater(true);
            } else {
                mCaptureSurface.releaseCaptureSurface();
            }
        }
    }

    private void initSettingManager(ISettingManager settingManager) {
        mSettingManager = settingManager;
        settingManager.updateModeDevice2Requester(this);
        mSettingDevice2Configurator = settingManager.getSettingDevice2Configurator();
        mSettingController = settingManager.getSettingController();
    }

    private void doOpenCamera(boolean sync) throws CameraOpenException {
        if (sync) {
            mCameraDeviceManager.openCameraSync(mCurrentCameraId, mDeviceCallback, null);
        } else {
            mCameraDeviceManager.openCamera(mCurrentCameraId, mDeviceCallback, null);
        }
    }

    private void updateCameraState(CameraState state) {
        LogHelper.d(TAG, "[updateCameraState] new state = " + state + " old =" + mCameraState);
        mLockState.lock();
        try {
            mCameraState = state;
        } finally {
            mLockState.unlock();
        }
    }

    private CameraState getCameraState() {
        mLockState.lock();
        try {
            return mCameraState;
        } finally {
            mLockState.unlock();
        }
    }

    private void doCloseCamera(boolean sync) {
        if (sync) {
            mCameraDeviceManager.closeSync(mCurrentCameraId);
        } else {
            mCameraDeviceManager.close(mCurrentCameraId);
        }
        mCaptureFrameMap.clear();
        mCamera2Proxy = null;
        synchronized (mSurfaceHolderSync) {
            mSurfaceObject = null;
            mPreviewSurface = null;
        }
    }

    private void recycleVariables() {
        mCurrentCameraId = null;
        updatePreviewSurface(null);
        mCamera2Proxy = null;
        mIsPictureSizeChanged = false;
    }

    private boolean canOpenCamera(String newCameraId) {
        boolean isSameCamera = newCameraId.equalsIgnoreCase(mCurrentCameraId);
        boolean isStateReady = mCameraState == CameraState.CAMERA_UNKNOWN;
        boolean value = !isSameCamera && isStateReady;
        LogHelper.i(TAG, "[canOpenCamera] new id: " + newCameraId + " current camera :" +
                mCurrentCameraId + " isSameCamera = " + isSameCamera + " current state : " +
                mCameraState + " isStateReady = " + isStateReady + " can open : " + value);
        return value;
    }

    private void configureSession(boolean isFromOpen) {
        LogHelper.i(TAG, "[configureSession] +" + ", isFromOpen :" + isFromOpen);
        mDeviceLock.lock();
        mFirstFrameArrived = false;
        try {
            if (mCamera2Proxy != null) {
                abortOldSession();
                mCaptureSurface.releaseCaptureSurfaceLater(false);
                List<Surface> surfaces = new LinkedList<>();
                startPostAlgo();
                surfaces.add(mPreviewSurface);
                surfaces.add(mCapturePostAlgoSurface);
                if (ThumbnailHelper.isPostViewSupported()) {
                    surfaces.add(mThumbnailSurface.getSurface());
                }
                mNeedFinalizeOutput = false;
                mSettingDevice2Configurator.configSessionSurface(surfaces);
                LogHelper.d(TAG, "[configureSession] surface size : " + surfaces.size());
                mBuilder = doCreateAndConfigRequest(Camera2Proxy.TEMPLATE_PREVIEW);
                mCamera2Proxy.createCaptureSession(surfaces, mSessionCallback,
                        mModeHandler, mBuilder);
                mIsPictureSizeChanged = false;
            }
        } catch (CameraAccessException e) {
            LogHelper.e(TAG, "[configureSession] error");
        } finally {
            mDeviceLock.unlock();
        }
        LogHelper.d(TAG, "[configureSession] - ");
    }

    private void configSettingsByStage2() {
        mSettingManager.createSettingsByStage(2);
        mSettingDevice2Configurator.setCameraCharacteristics(mCameraCharacteristics);
        P2DoneInfo.setCameraCharacteristics(mActivity.getApplicationContext(),
                Integer.parseInt(mCurrentCameraId));
        mSettingDevice2Configurator.configCaptureRequest(mBuilder);
        mSettingController.addViewEntry();
        mSettingController.refreshViewEntry();
    }

    private void abortOldSession() {
        LogHelper.d(TAG, "[abortOldSession] +");
        if (mSession != null) {
            try {
                mSession.abortCaptures();
            } catch (CameraAccessException e) {
                LogHelper.e(TAG, "[abortOldSession] exception", e);
            }
        }
        stopPostAlgo();
        mSession = null;
        mBuilder = null;
        mDefaultBuilder = null;
        LogHelper.d(TAG, "[abortOldSession] -");
    }

    private void configureQuickPreview(Builder builder) {
        LogHelper.d(TAG, "configureQuickPreview mQuickPreviewKey:" + mQuickPreviewKey);
        if (mQuickPreviewKey != null) {
            builder.set(mQuickPreviewKey, QUICK_PREVIEW_KEY_VALUE);
        }
    }

    private void configureBGService(Builder builder) {
        if (mIsBGServiceEnabled) {
            if (mBGServicePrereleaseKey != null) {
                builder.set(mBGServicePrereleaseKey, BGSERVICE_PRERELEASE_KEY_VALUE);
            }
            if (mBGServiceImagereaderIdKey != null) {
                int[] value = new int[1];
                value[0] = mCaptureSurface.getImageReaderId();
                builder.set(mBGServiceImagereaderIdKey, value);
            }
        }
    }

    private void repeatingPreview(boolean needConfigBuiler) {
        LogHelper.i(TAG, "[repeatingPreview] mSession =" + mSession + " mCamera =" +
                mCamera2Proxy + ",needConfigBuiler " + needConfigBuiler);
        if (mSession != null && mCamera2Proxy != null) {
            try {
                if (needConfigBuiler) {
                    Builder builder = doCreateAndConfigRequest(Camera2Proxy.TEMPLATE_PREVIEW);
                    mSession.setRepeatingRequest(builder.build(), mCaptureCallback, mModeHandler);
                } else {
                    mBuilder.addTarget(mPreviewSurface);
                    mSession.setRepeatingRequest(mBuilder.build(), mCaptureCallback, mModeHandler);
                }
                mCaptureSurface.setCaptureCallback(this);
            } catch (CameraAccessException | RuntimeException e) {
                LogHelper.e(TAG, "[repeatingPreview] error");
            }
        }
    }

    private Builder doCreateAndConfigRequest(int templateType) throws CameraAccessException {
        LogHelper.i(TAG, "[doCreateAndConfigRequest] mCamera2Proxy =" + mCamera2Proxy);
        Builder builder = null;
        if (mCamera2Proxy != null) {
            builder = mCamera2Proxy.createCaptureRequest(templateType);
            if (builder == null) {
                LogHelper.d(TAG, "Builder is null, ignore this configuration");
                return null;
            }
            mSettingDevice2Configurator.configCaptureRequest(builder);
            ThumbnailHelper.configPostViewRequest(builder);
            configureQuickPreview(builder);
            configureBGService(builder);
            if (Camera2Proxy.TEMPLATE_PREVIEW == templateType) {
                builder.addTarget(mPreviewSurface);
            } else if (Camera2Proxy.TEMPLATE_STILL_CAPTURE == templateType) {
                builder.addTarget(mCapturePostAlgoSurface);
                if ("off".equalsIgnoreCase(mZsdStatus)) {
                    builder.addTarget(mPreviewSurface);
                }
                if (ThumbnailHelper.isPostViewOverrideSupported()) {
                    builder.addTarget(mThumbnailSurface.getSurface());
                }
                ThumbnailHelper.setDefaultJpegThumbnailSize(builder);
                P2DoneInfo.enableP2Done(builder);
                CameraUtil.enable4CellRequest(mCameraCharacteristics, builder);
                int rotation = CameraUtil.getJpegRotationFromDeviceSpec(
                        Integer.parseInt(mCurrentCameraId), mJpegRotation, mActivity);
                HeifHelper.orientation = rotation;
                builder.set(CaptureRequest.JPEG_ORIENTATION, rotation);
                if (mICameraContext.getLocation() != null) {
                    builder.set(CaptureRequest.JPEG_GPS_LOCATION, mICameraContext.getLocation());
                }
            }

        }
        return builder;
    }

    private Builder getDefaultPreviewBuilder() throws CameraAccessException {
        if (mCamera2Proxy != null && mDefaultBuilder == null) {
            mDefaultBuilder = mCamera2Proxy.createCaptureRequest(Camera2Proxy.TEMPLATE_PREVIEW);
            ThumbnailHelper.configPostViewRequest(mDefaultBuilder);
        }
        return mDefaultBuilder;
    }

    private Size getTargetPreviewSize(double ratio) {
        Size values = null;
        try {
            CameraCharacteristics cs = mCameraManager.getCameraCharacteristics(mCurrentCameraId);
            StreamConfigurationMap streamConfigurationMap =
                    cs.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            android.util.Size previewSizes[] =
                    streamConfigurationMap.getOutputSizes(SurfaceHolder.class);
            int length = previewSizes.length;
            List<Size> sizes = new ArrayList<>(length);
            for (int i = 0; i < length; i++) {
                sizes.add(i, new Size(previewSizes[i].getWidth(), previewSizes[i].getHeight()));
            }
            values = CameraUtil.getOptimalPreviewSize(mActivity, sizes, ratio, true);
            mPreviewWidth = values.getWidth();
            mPreviewHeight = values.getHeight();
        } catch (Exception e) {
            e.printStackTrace();
            LogHelper.e(TAG, "camera process killed due to getCameraCharacteristics() error");
            //Process.killProcess(Process.myPid());
        }
        LogHelper.d(TAG, "[getTargetPreviewSize] " + mPreviewWidth + " X " + mPreviewHeight);
        return values;
    }

    private void updatePreviewSize() {
        SettingController controller = mSettingManager.getSettingController();
        String pictureSize = controller.queryValue(KEY_PICTURE_SIZE);
        LogHelper.i(TAG, "[updatePreviewSize] :" + pictureSize);
        if (pictureSize != null) {
            String[] pictureSizes = pictureSize.split("x");
            int width = Integer.parseInt(pictureSizes[0]);
            int height = Integer.parseInt(pictureSizes[1]);
            double ratio = (double) width / height;
            getTargetPreviewSize(ratio);
        }
    }

    private void updatePictureSize() {
        SettingController controller = mSettingManager.getSettingController();
        String pictureSize = controller.queryValue(KEY_PICTURE_SIZE);
        LogHelper.i(TAG, "[updatePictureSize] :" + pictureSize);
        if (pictureSize != null) {
            String[] pictureSizes = pictureSize.split("x");
            int width = Integer.parseInt(pictureSizes[0]);
            int height = Integer.parseInt(pictureSizes[1]);
            setPictureSize(new Size(width, height));
        }
    }

    @Override
    public void doCameraOpened(@Nonnull Camera2Proxy camera2proxy) {
        LogHelper.i(TAG, "[onOpened]  camera2proxy = " + camera2proxy + " preview surface = "
                + mPreviewSurface + "  mCameraState = " + mCameraState + "camera2Proxy id = "
                + camera2proxy.getId() + " mCameraId = " + mCurrentCameraId);
        try {
            if (CameraState.CAMERA_OPENING == getCameraState()
                    && camera2proxy != null && camera2proxy.getId().equals(mCurrentCameraId)) {
                mCamera2Proxy = camera2proxy;
                mFirstFrameArrived = false;
                if (mModeDeviceCallback != null) {
                    mModeDeviceCallback.onCameraOpened(mCurrentCameraId);
                }
                updateCameraState(CameraState.CAMERA_OPENED);
                ThumbnailHelper.setCameraCharacteristics(mCameraCharacteristics,
                        mActivity.getApplicationContext(), Integer.parseInt(mCurrentCameraId));
                mSettingDevice2Configurator.setCameraCharacteristics(mCameraCharacteristics);
                updatePreviewSize();
                updatePictureSize();

                if (mPreviewSizeCallback != null) {
                    mPreviewSizeCallback.onPreviewSizeReady(new Size(mPreviewWidth,
                            mPreviewHeight));
                }
                if (mNeedSubSectionInitSetting) {
                    //configureSession(true);
                } else {
                    initSettings();
                }
            }
        } catch (RuntimeException e) {
            e.printStackTrace();
        }
    }

    private void initSettings() {
        LogHelper.i(TAG, "[initSettings]");
        mSettingController.postRestriction(FaceBeautyRestriction.getRestriction()
                .getRelation("on", false));
        mSettingController.addViewEntry();
        mSettingController.refreshViewEntry();
    }

    @Override
    public void doCameraDisconnected(@Nonnull Camera2Proxy camera2proxy) {
        LogHelper.i(TAG, "[onDisconnected] camera2proxy = " + camera2proxy);
        if (mCamera2Proxy != null && mCamera2Proxy == camera2proxy) {
            CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_ERROR_SERVER_DIED);
            updateCameraState(CameraState.CAMERA_UNKNOWN);
            mCurrentCameraId = null;
        }
    }

    @Override
    public void doCameraError(@Nonnull Camera2Proxy camera2Proxy, int error) {
        LogHelper.i(TAG, "[onError] camera2proxy = " + camera2Proxy + " error = " + error);
        if ((mCamera2Proxy != null && mCamera2Proxy == camera2Proxy)
                || error == CameraUtil.CAMERA_OPEN_FAIL
                || error == CameraUtil.CAMERA_ERROR_EVICTED) {
            updateCameraState(CameraState.CAMERA_UNKNOWN);
            CameraUtil.showErrorInfoAndFinish(mActivity, error);
            updateCameraState(CameraState.CAMERA_UNKNOWN);
            mCurrentCameraId = null;
        }
    }

    /**
     * Camera session callback.
     */
    private final Camera2CaptureSessionProxy.StateCallback mSessionCallback = new
            Camera2CaptureSessionProxy.StateCallback() {

                @Override
                public void onConfigured(@Nonnull Camera2CaptureSessionProxy session) {
                    LogHelper.i(TAG, "[onConfigured],session = " + session
                            + ", mNeedFinalizeOutput:" + mNeedFinalizeOutput);
                    mDeviceLock.lock();
                    try {
                        mSession = session;
                        if (mNeedFinalizeOutput) {
                            mSession.finalizeOutputConfigurations(mOutputConfigs);
                            mNeedFinalizeOutput = false;
                            if (CameraState.CAMERA_OPENED == getCameraState()) {
                                synchronized (mSurfaceHolderSync) {
                                    if (mPreviewSurface != null) {
                                        repeatingPreview(false);
                                        configSettingsByStage2();
                                        repeatingPreview(false);
                                    }
                                }
                            }
                            return;
                        }
                        if (CameraState.CAMERA_OPENED == getCameraState()) {
                            synchronized (mSurfaceHolderSync) {
                                if (mPreviewSurface != null) {
                                    repeatingPreview(false);
                                }
                            }
                        }
                    } finally {
                        mDeviceLock.unlock();
                    }
                }

                @Override
                public void onConfigureFailed(@Nonnull Camera2CaptureSessionProxy session) {
                    LogHelper.i(TAG, "[onConfigureFailed],session = " + session);
                    if (mSession == session) {
                        mSession = null;
                    }
                }

                @Override
                public void onClosed(@Nonnull Camera2CaptureSessionProxy session) {
                    super.onClosed(session);
                    LogHelper.i(TAG, "[onClosed],session = " + session);
                    if (mSession == session) {
                        mSession = null;
                    }
                }
            };

    /**
     * Capture callback.
     */
    private final CaptureCallback mCaptureCallback = new CaptureCallback() {

        @Override
        public void onCaptureStarted(CameraCaptureSession session, CaptureRequest request, long
                timestamp, long frameNumber) {
            super.onCaptureStarted(session, request, timestamp, frameNumber);
            if (mCamera2Proxy == null || session.getDevice() != mCamera2Proxy.getCameraDevice()) {
                return;
            }
            if (CameraUtil.isStillCaptureTemplate(request)) {
                LogHelper.d(TAG, "[onCaptureStarted] capture started, frame: " + frameNumber);
                if (mIsBGServiceEnabled) {
                    mCaptureSurface.increasePictureNum();
                }
                mCaptureFrameMap.put(String.valueOf(frameNumber), Boolean.FALSE);
                mICameraContext.getSoundPlayback().play(ISoundPlayback.SHUTTER_CLICK);
            }
        }

        @Override
        public void onCaptureProgressed(CameraCaptureSession session, CaptureRequest request,
                                        CaptureResult partialResult) {
            super.onCaptureProgressed(session, request, partialResult);
            if (mCamera2Proxy == null || session.getDevice() != mCamera2Proxy.getCameraDevice()) {
                return;
            }
            if (CameraUtil.isStillCaptureTemplate(request)
                    && P2DoneInfo.checkP2DoneResult(partialResult)) {
                //p2done comes, it can do next capture
                long num = partialResult.getFrameNumber();
                if (mCaptureFrameMap.containsKey(String.valueOf(num))) {
                    mCaptureFrameMap.put(String.valueOf(num), Boolean.TRUE);
                }
                LogHelper.d(TAG, "[onCaptureProgressed] P2done comes, frame: " + num);
                updateCameraState(CameraState.CAMERA_OPENED);
                mModeDeviceCallback.onPreviewCallback(null, 0);
            }
        }

        @Override
        public void onCaptureCompleted(@Nonnull CameraCaptureSession session,
                                       @Nonnull CaptureRequest request,
                                       @Nonnull TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            if (mCamera2Proxy == null
                    || mModeDeviceCallback == null
                    || session.getDevice() != mCamera2Proxy.getCameraDevice()) {
                return;
            }
            mSettingDevice2Configurator.getRepeatingCaptureCallback().onCaptureCompleted(
                    session, request, result);
            if (CameraUtil.isStillCaptureTemplate(result)) {
                long num = result.getFrameNumber();
                if (mCaptureFrameMap.containsKey(String.valueOf(num))
                        && Boolean.FALSE == mCaptureFrameMap.get(String.valueOf(num))) {
                    mFirstFrameArrived = true;
                    updateCameraState(CameraState.CAMERA_OPENED);
                    mModeDeviceCallback.onPreviewCallback(null, 0);
                }
                mModeDeviceCallback.onStillCaptureResultDelivered(result);
                mCaptureFrameMap.remove(String.valueOf(num));

            } else if (!CameraUtil.isStillCaptureTemplate(result) && !mFirstFrameArrived) {
                mFirstFrameArrived = true;
                updateCameraState(CameraState.CAMERA_OPENED);
                mModeDeviceCallback.onPreviewCallback(null, 0);
            } else {
                //FaceDetection
                android.hardware.camera2.params.Face[] faces
                        = result.get(CaptureResult.STATISTICS_FACES);
                Rect cropRegion = result.get(CaptureResult.SCALER_CROP_REGION);
                if (faces != null && faces.length > 0) {
                    mFaceDetections = getUIRect(faces, cropRegion);
                }
            }
        }

        @Override
        public void onCaptureFailed(@Nonnull CameraCaptureSession session,
                                    @Nonnull CaptureRequest request,
                                    @Nonnull CaptureFailure failure) {
            super.onCaptureFailed(session, request, failure);
            LogHelper.e(TAG, "[onCaptureFailed], framenumber: " + failure.getFrameNumber()
                    + ", reason: " + failure.getReason() + ", sequenceId: "
                    + failure.getSequenceId() + ", isCaptured: " + failure.wasImageCaptured());
            if (mCamera2Proxy == null || session.getDevice() != mCamera2Proxy.getCameraDevice()) {
                return;
            }
            mSettingDevice2Configurator.getRepeatingCaptureCallback()
                    .onCaptureFailed(session, request, failure);
            if (mModeDeviceCallback != null && CameraUtil.isStillCaptureTemplate(request)) {
                mCaptureFrameMap.remove(String.valueOf(failure.getFrameNumber()));
                updateCameraState(CameraState.CAMERA_OPENED);
                mModeDeviceCallback.onPreviewCallback(null, 0);
            }
        }

        @Override
        public void onCaptureSequenceAborted(CameraCaptureSession session, int sequenceId) {
            super.onCaptureSequenceAborted(session, sequenceId);
            LogHelper.d(TAG, "<onCaptureSequenceAborted>");
        }

        @Override
        public void onCaptureBufferLost(CameraCaptureSession session, CaptureRequest request,
                                        Surface target, long frameNumber) {
            super.onCaptureBufferLost(session, request, target, frameNumber);
            LogHelper.d(TAG, "<onCaptureBufferLost> frameNumber: " + frameNumber);
        }
    };

    private void startPostAlgo() {
        // feature param
        mFeatureParam = new FeatureParam();
        int facing = getCameraFacingById(mCurrentCameraId);
        mFeatureParam.appendInt("postalgo.lens.facing", facing);
        if (facing == LENS_FACING_FRONT) {
            mFeatureParam.appendInt("postalgo.sensor.orientation", 270);
        } else if (facing == LENS_FACING_BACK) {
            mFeatureParam.appendInt("postalgo.sensor.orientation", 90);
        }
        // 0 open mirror 1 close mirror when facing = 0
        mFeatureParam.appendInt(MTK_POSTALGO_FACE_BEAUTY_MIRROR, 0);

        FeatureConfig[] featureConfigs = new FeatureConfig[1];

        // capture config
        FeatureConfig captureFeatureConfig = featureConfigs[0] = new FeatureConfig();

        ArrayList<Surface> captureSurfaceList = new ArrayList<>();
        captureSurfaceList.add(mCaptureSurface.getSurface());
        LogHelper.i(TAG, "[startPostAlgo] mCaptureSurface = " + mCaptureSurface);
        captureFeatureConfig.addSurface(captureSurfaceList);

        FeaturePipeConfig captureFeaturePipeConfig = new FeaturePipeConfig();
        captureFeaturePipeConfig.addFeaturePipeConfig(FeaturePipeConfig.INDEX_CAPTURE,
                new int[]{FeaturePipeConfig.INDEX_FB});
        captureFeatureConfig.addFeaturePipeConfig(captureFeaturePipeConfig);

        StreamInfo captureStreamInfo = new StreamInfo();
        LogHelper.i(TAG, "[startPostAlgo] mCaptureWidth = " + mCaptureWidth + " mCaptureHeight = "
                + mCaptureHeight);
        captureStreamInfo.addInfo(mCaptureWidth, mCaptureHeight, ImageFormat.YUV_420_888, 0);
        ArrayList<StreamInfo> captureStreamInfoList = new ArrayList<>();
        captureStreamInfoList.add(captureStreamInfo);
        captureFeatureConfig.addStreamInfo(captureStreamInfoList);
        captureFeatureConfig.addInterfaceParams(mFeatureParam);

        FeatureResult result = mICameraContext.getCamPostAlgo().start(featureConfigs,
                new ICamPostAlgoCallback.Stub() {
                    @Override
                    public void processResult(FeatureParam featureParam) throws RemoteException {

                    }

                    @Override
                    public IBinder asBinder() {
                        return this;
                    }
                });
        mCapturePostAlgoSurface = result.getStreams().elementAt(0).getmSurface();
        initMetaParams();
        LogHelper.d(TAG, "[startPostAlgo] mCapturePostAlgoSurface = " + mCapturePostAlgoSurface);
    }

    private void initMetaParams() {
        FeatureParam params = new FeatureParam();
        int value = Integer.parseInt(mDataStore.getValue(MTK_POSTALGO_FACE_BEAUTY_ENLARGE_EYE,
                "0", mDataStore.getGlobalScope()));
        LogHelper.i(TAG, "[initMetaParams] value0 : " + value);
        params.appendInt(MTK_POSTALGO_FACE_BEAUTY_ENLARGE_EYE, value / 25);
        value = Integer.parseInt(mDataStore.getValue(MTK_POSTALGO_FACE_BEAUTY_SLIM_FACE,
                "0", mDataStore.getGlobalScope()));
        LogHelper.i(TAG, "[initMetaParams] value1 : " + value);
        params.appendInt(MTK_POSTALGO_FACE_BEAUTY_SLIM_FACE, value / 25);
        value = Integer.parseInt(mDataStore.getValue(MTK_POSTALGO_FACE_BEAUTY_BRIGHT,
                "0", mDataStore.getGlobalScope()));
        LogHelper.i(TAG, "[initMetaParams] value2 : " + value);
        params.appendInt(MTK_POSTALGO_FACE_BEAUTY_BRIGHT, value / 13);
        value = Integer.parseInt(mDataStore.getValue(MTK_POSTALGO_FACE_BEAUTY_SMOOTH,
                "0", mDataStore.getGlobalScope()));
        LogHelper.i(TAG, "[initMetaParams] value3 : " + value);
        params.appendInt(MTK_POSTALGO_FACE_BEAUTY_SMOOTH, value / 13);
        value = Integer.parseInt(mDataStore.getValue(MTK_POSTALGO_FACE_BEAUTY_RUDDY,
                "0", mDataStore.getGlobalScope()));
        LogHelper.i(TAG, "[initMetaParams] value4 : " + value);
        params.appendInt(MTK_POSTALGO_FACE_BEAUTY_RUDDY, value / 13);
        params.appendInt(MTK_POSTALGO_FACE_BEAUTY_PHYSICAL_ID, Integer.parseInt(mCurrentCameraId));

        int rotation = CameraUtil.getJpegRotationFromDeviceSpec(
                Integer.parseInt(mCurrentCameraId), mJpegRotation, mActivity);
        LogHelper.i(TAG, "[configFaceDetectionMetaParams] jpegrotation = "
                + rotation);
        params.appendInt(POSTALGO_PARAMS_JPEG_ORIENTATION_KEY, rotation);
        mICameraContext.getCamPostAlgo().configParams(FeaturePipeConfig.INDEX_CAPTURE, params);
    }

    private void configFaceDetectionMetaParams() {
        mFeatureParam = new FeatureParam();
        int[] rectsArray = new int[4];

        if (mFaceDetections != null) {
            rectsArray[0] = mFaceDetections[0].left; // x value
            rectsArray[1] = mFaceDetections[0].top; // y value
            rectsArray[2] = mFaceDetections[0].right;
            rectsArray[3] = mFaceDetections[0].bottom;
            mFeatureParam.appendInt(MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION_SIZE,
                    rectsArray.length);
            LogHelper.i(TAG, "[configFaceDetectionMetaParams] rectsArray = "
                    + Arrays.toString(rectsArray));
            mFeatureParam.appendIntArray(MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION, rectsArray);
        } else {
            mFeatureParam.appendInt(MTK_POSTALGO_FACE_BEAUTY_FACE_DETECTION_SIZE,
                    0);
        }


        mICameraContext.getCamPostAlgo().configParams(FeaturePipeConfig.INDEX_CAPTURE,
                          mFeatureParam);
    }


    private void stopPostAlgo() {
        mICameraContext.getCamPostAlgo().stop();
    }

    private int getCameraFacingById(String deviceId) {
        CameraManager cameraManager = (CameraManager)
                mActivity.getSystemService(Context.CAMERA_SERVICE);
        try {
            CameraCharacteristics characteristics =
                    cameraManager.getCameraCharacteristics(deviceId);
            return characteristics.get(CameraCharacteristics.LENS_FACING);
        } catch (CameraAccessException e) {
            LogHelper.e(TAG, "[getCameraFacingById] CameraAccessException", e);
            return -1;
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, "[getCameraFacingById] IllegalArgumentException", e);
            return -1;
        }
    }

    private Rect[] getUIRect(android.hardware.camera2.params.Face[] faces, Rect cropRegion) {
        if (faces == null || faces.length == 0) {
            return null;
        }
        Rect[] uiRect = new Rect[faces.length];
        Rect rectTemp;
        for (int i = 0; i < faces.length; i++) {
            rectTemp = CoordinatesTransform.sensorToNormalizedPreview(faces[i].getBounds(),
                    mCaptureWidth,
                    mCaptureHeight, cropRegion);
            uiRect[i] = normalizedPreviewToUi(rectTemp, mCaptureWidth, mCaptureHeight, false);
        }
        return uiRect;
    }

    /**
     * Transform NormalizedPreview(-1000,1000) coordinate to UI coordinate.
     *
     * @param rect     the rect to be transformed.
     * @param w        the preview area width.
     * @param h        the preview area height.
     * @param isMirror it should be true for front camera.
     * @return the transfromed rect, Coordinate(0,0).
     */
    public static Rect normalizedPreviewToUi(Rect rect, int w, int h,
                                             boolean isMirror) {

        Matrix matrix = new Matrix();
        prepareMatrix(matrix, isMirror, w, h);
        RectF rectf = new RectF(rect);
        matrix.mapRect(rectf);
        Rect resultRect = new Rect();
        rectf.round(resultRect);
        return resultRect;
    }

    private static void prepareMatrix(Matrix matrix, boolean mirror,
                                      int viewWidth, int viewHeight) {
        // Need mirror for front camera.
        matrix.setScale(mirror ? -1 : 1, 1);
        // This is the value for android.hardware.Camera.setDisplayOrientation.
        //matrix.postRotate(displayOrientation);
        // Camera driver coordinates range from (-1000, -1000) to (1000, 1000).
        // UI coordinates range from (0, 0) to (width, height).
        matrix.postScale(viewWidth / 2000f, viewHeight / 2000f);
        matrix.postTranslate(viewWidth / 2f, viewHeight / 2f);

    }
}
