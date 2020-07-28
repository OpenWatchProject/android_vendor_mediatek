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

package com.mediatek.camera.feature.mode.dof;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.ImageFormat;
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
import android.util.Range;
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
import com.mediatek.camera.common.mode.photo.DeviceInfo;
import com.mediatek.camera.common.mode.photo.HeifHelper;
import com.mediatek.camera.common.mode.photo.P2DoneInfo;
import com.mediatek.camera.common.mode.photo.ThumbnailHelper;
import com.mediatek.camera.common.mode.photo.device.IDeviceController;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Configurator;
import com.mediatek.camera.common.sound.ISoundPlayback;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.campostalgo.FeatureConfig;
import com.mediatek.campostalgo.FeatureParam;
import com.mediatek.campostalgo.FeaturePipeConfig;
import com.mediatek.campostalgo.FeatureResult;
import com.mediatek.campostalgo.ICamPostAlgoCallback;
import com.mediatek.campostalgo.StreamInfo;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;

import static android.hardware.camera2.CameraMetadata.LENS_FACING_BACK;
import static android.hardware.camera2.CameraMetadata.LENS_FACING_FRONT;
import static com.mediatek.camera.common.utils.CameraUtil.getTurndownPreviewLevel;

/**
 * An implementation of {@link IDeviceController} with Camera2Proxy.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
class DofDevice2Controller extends Device2Controller implements
        DofDeviceController,
        CaptureSurface.ImageCallback,
        ISettingManager.SettingDevice2Requester {
    private static final Tag TAG = new Tag(DofDevice2Controller.class.getSimpleName());
    private static final String KEY_PICTURE_SIZE = "key_picture_size";
    private static final int CAPTURE_MAX_NUMBER = 5;
    private static final int WAIT_TIME = 5;
    private static final int PREVIEW_SIZE_HEIGHT = 720;
    //add for quick preview
    private static final String QUICK_PREVIEW_KEY = "com.mediatek.configure.setting.initrequest";
    public static final String MTK_POSTALGO_PORTRAIT_DOF_LEVEL = "postalgo.portrait.doflevel";
    public static final String MTK_POSTALGO_PORTRAIT_TYPE = "postalgo.portrait.type";
    public static final String MTK_POSTALGO_PORTRAIT_FACEORIENTATION
            = "postalgo.portrait.faceorientation";
    private static final String POSTALGO_PARAMS_JPEG_ORIENTATION_KEY
            = "postalgo.capture.jpegorientation";
    private static final int[] QUICK_PREVIEW_KEY_VALUE = new int[]{1};
    private CaptureRequest.Key<int[]> mQuickPreviewKey = null;
    //add for BG service
    private CaptureRequest.Key<int[]> mBGServicePrereleaseKey = null;
    private CaptureRequest.Key<int[]> mBGServiceImagereaderIdKey = null;
    private static final int[] BGSERVICE_PRERELEASE_KEY_VALUE = new int[]{1};
    private static final int LEVEL_DEFAULT = 5;
    private int mCurrentLevel = LEVEL_DEFAULT;
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
    private Surface mPreviewPostAlgoSurface,mCapturePostAlgoSurface;
    private IDeviceController.CaptureDataCallback mCaptureDataCallback;
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
    private boolean mIsStopPostAlgo = false;
    private int mFaceOrientation = 1;

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
     * @param activity the camera activity.
     */
    DofDevice2Controller(@Nonnull Activity activity, @Nonnull ICameraContext context) {
        LogHelper.d(TAG, "[PhotoDevice2Controller]");
        mActivity = activity;
        mCameraManager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        mICameraContext = context;
        mCaptureSurface = new CaptureSurface();
        mCaptureSurface.setCaptureCallback(this);
        mThumbnailSurface = new CaptureSurface();
        mThumbnailSurface.setCaptureCallback(this);
        mThumbnailSurface.setFormat(ThumbnailHelper.FORMAT_TAG);
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
    }

    @Override
    public void queryCameraDeviceManager() {
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
    }

    @Override
    public void openCamera(DofDeviceInfo info) {
        String cameraId = info.getCameraId();
        boolean sync = info.getNeedOpenCameraSync();
        LogHelper.i(TAG, "[openCamera] cameraId : " + cameraId + ",sync = " + sync);
        if (CameraUtil.isCameraFacingFront(mActivity, Integer.parseInt(cameraId))) {
            mFaceOrientation = 3;
        } else {
            mFaceOrientation = 1;
        }
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
            } catch (CameraAccessException e) {
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
        LogHelper.d(TAG, "[updatePreviewSurface] surfaceHolder = "
                + surfaceObject + " state = "
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
//                }
//                else if (onlySetSurface && mNeedSubSectionInitSetting) {
//                    mOutputConfigs.get(0).addSurface(mPreviewSurface);
//                    if (mSession != null) {
//                        mSession.finalizeOutputConfigurations(mOutputConfigs);
//                        mNeedFinalizeOutput = false;
//                        if (CameraState.CAMERA_OPENED == getCameraState()) {
//                            repeatingPreview(false);
//                            configSettingsByStage2();
//                            repeatingPreview(false);
//                        }
//                    } else {
//                        mNeedFinalizeOutput = true;
//                    }
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
    public void setVsDofLevelParameter(int level) {

            mCurrentLevel = level;
            configMetaParams(mCurrentLevel);
            LogHelper.i(TAG, "[setVsDofLevelParameter] level = " + level);
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
    public void takePicture(@Nonnull IDeviceController.CaptureDataCallback callback) {
        LogHelper.i(TAG, "[takePicture] mSession= " + mSession);
        if (mSession != null && mCamera2Proxy != null) {
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
    public void stopCapture() {

    }

    @Override
    public void setNeedWaitPictureDone(boolean needWaitPictureDone) {

    }

    @Override
    public void updateGSensorOrientation(int orientation) {
        mJpegRotation = orientation;
    }

    @Override
    public void closeCamera(boolean sync) {
        LogHelper.i(TAG, "[closeCamera] + sync = " +
                sync + " current state : " + mCameraState);
        if (CameraState.CAMERA_UNKNOWN != mCameraState) {
            try {
                mDeviceLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
                super.doCameraClosed(mCamera2Proxy);
                updateCameraState(CameraState.CAMERA_CLOSING);
                abortOldSession();
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
            IDeviceController.DataCallbackInfo  info = new IDeviceController.DataCallbackInfo();
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
    public CaptureRequest.Builder createAndConfigRequest(int templateType) {
        CaptureRequest.Builder builder = null;
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
//        Relation relation = DofRestriction.getRestriction().getRelation("on",
//                false);
//        mSettingManager.getSettingController().postRestriction(relation);
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
        LogHelper.d(TAG, "[updateCameraState] new state = " +
                state + " old =" + mCameraState);
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
    @Override
    public void closeSession() {
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
//        LogHelper.i(TAG, "[configureSession] +" + ", isFromOpen :" + isFromOpen);
//        mDeviceLock.lock();
//        mFirstFrameArrived = false;
//        try {
//            if (mCamera2Proxy != null) {
//                abortOldSession();
//                mCaptureSurface.releaseCaptureSurfaceLater(false);
//                if (isFromOpen) {
//                    mOutputConfigs = new ArrayList<>();
//                    android.util.Size previewSize = new android.util.Size(mPreviewWidth,
//                            mPreviewHeight);
//                    OutputConfiguration previewConfig = new OutputConfiguration(previewSize,
//                            SurfaceTexture.class);
//                    OutputConfiguration captureConfig
//                            = new OutputConfiguration(mCaptureSurface.getSurface());
//                    OutputConfiguration rawConfig
//                            = mSettingDevice2Configurator.getRawOutputConfiguration();
//                    mOutputConfigs.add(previewConfig);
//                    mOutputConfigs.add(captureConfig);
//                    if (rawConfig != null) {
//                        mOutputConfigs.add(rawConfig);
//                    }
//                    if (ThumbnailHelper.isPostViewSupported()) {
//                        OutputConfiguration thumbnailConfig
//                                = new OutputConfiguration(mThumbnailSurface.getSurface());
//                        mOutputConfigs.add(thumbnailConfig);
//                    }
//                    mBuilder = getDefaultPreviewBuilder();
//                    mSettingDevice2Configurator.configCaptureRequest(mBuilder);
//                    configureQuickPreview(mBuilder);
//                    configureBGService(mBuilder);
//                    mCamera2Proxy.createCaptureSession(mSessionCallback,
//                            mModeHandler, mBuilder, mOutputConfigs);
//                    mIsPictureSizeChanged = false;
//                    return;
//                }
//                List<Surface> surfaces = new LinkedList<>();
//                surfaces.add(mPreviewSurface);
//                surfaces.add(mCaptureSurface.getSurface());
//                if (ThumbnailHelper.isPostViewSupported()) {
//                    surfaces.add(mThumbnailSurface.getSurface());
//                }
//                mNeedFinalizeOutput = false;
//                mSettingDevice2Configurator.configSessionSurface(surfaces);
//                LogHelper.d(TAG, "[configureSession] surface size : " + surfaces.size());
//                mBuilder = doCreateAndConfigRequest(Camera2Proxy.TEMPLATE_PREVIEW);
//                mCamera2Proxy.createCaptureSession(surfaces, mSessionCallback,
//                        mModeHandler, mBuilder);
//                mIsPictureSizeChanged = false;
//            }
//        } catch (CameraAccessException e) {
//            LogHelper.e(TAG, "[configureSession] error");
//        } finally {
//            mDeviceLock.unlock();
//        }
        LogHelper.d(TAG, "[configureSession] + ");
        try {
            mDeviceLock.lock();
            mFirstFrameArrived = false;
            List<Surface> surfaces = new LinkedList<>();
            abortOldSession();
            mCaptureSurface.releaseCaptureSurfaceLater(false);
//          startPostAlgo(nineBlockSurface, nine_weigth, nine_height);

            startPreviewPostAlgo();

            surfaces.add(mPreviewPostAlgoSurface);
            surfaces.add(mCapturePostAlgoSurface);


            if (ThumbnailHelper.isPostViewSupported()) {
                surfaces.add(mThumbnailSurface.getSurface());
            }
            mNeedFinalizeOutput = false;
            mSettingDevice2Configurator.configSessionSurface(surfaces);
            LogHelper.d(TAG, "[configureSession] surface size : " + surfaces.size());

            if (mCamera2Proxy != null) {
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

    private void startPreviewPostAlgo() {
        LogHelper.i(TAG, "[startPreviewPostAlgo] + ");
        mIsStopPostAlgo = false;
//        if (mPreviewPostAlgoSurface != null) {
//            stopPostAlgo();
//        }
        FeatureConfig[] featureConfigs = new FeatureConfig[2];
        FeatureParam mOrientationFeatureParams = new FeatureParam();
        // feature param
        int facing = getCameraFacingById(mCurrentCameraId);
        mOrientationFeatureParams.appendInt("postalgo.lens.facing", facing);
        mOrientationFeatureParams.appendInt("postalgo.sensor.orientation",
                getSensorOrientationById(mCurrentCameraId));

        // preview config
        FeatureConfig previewFeatureConfig = featureConfigs[0] = new FeatureConfig();
        ArrayList<Surface> previewSurfaceList = new ArrayList<>();
        previewSurfaceList.add(mPreviewSurface);
        LogHelper.i(TAG, "[startPostAlgo] mPreviewSurface = " + mPreviewSurface);
        previewFeatureConfig.addSurface(previewSurfaceList);
        FeaturePipeConfig previewFeaturePipeConfig = new FeaturePipeConfig();
        previewFeaturePipeConfig.addFeaturePipeConfig(FeaturePipeConfig.INDEX_PREVIEW,
                new int[]{FeaturePipeConfig.INDEX_PORTRAIT});
        previewFeatureConfig.addFeaturePipeConfig(previewFeaturePipeConfig);
        StreamInfo previewStreamInfo = new StreamInfo();
        previewStreamInfo.addInfo(mPreviewWidth, mPreviewHeight, ImageFormat.YV12, 0);
        LogHelper.i(TAG, "[previewStreamInfo] mPreviewWidth = " + mPreviewWidth+
                "   mPreviewHeight = "+ mPreviewHeight);
        ArrayList<StreamInfo> previewStreamInfoList = new ArrayList<>();
        previewStreamInfoList.add(previewStreamInfo);
        previewFeatureConfig.addStreamInfo(previewStreamInfoList);
        previewFeatureConfig.addInterfaceParams(mOrientationFeatureParams);

        // capture config
        FeatureConfig captureFeatureConfig = featureConfigs[1] = new FeatureConfig();
        ArrayList<Surface> captureSurfaceList = new ArrayList<>();
        captureSurfaceList.add(mCaptureSurface.getSurface());
        LogHelper.i(TAG, "[startPostAlgo] mCaptureSurface = " + mCaptureSurface.getSurface());
        captureFeatureConfig.addSurface(captureSurfaceList);
        FeaturePipeConfig captureFeaturePipeConfig = new FeaturePipeConfig();
        captureFeaturePipeConfig.addFeaturePipeConfig(FeaturePipeConfig.INDEX_CAPTURE,
                new int[]{FeaturePipeConfig.INDEX_PORTRAIT});
        captureFeatureConfig.addFeaturePipeConfig(captureFeaturePipeConfig);
        StreamInfo captureStreamInfo = new StreamInfo();
        Size pictureSize = getPictureSize();
        captureStreamInfo.addInfo(pictureSize.getWidth(), pictureSize.getHeight(),
                ImageFormat.YV12, 0);
        LogHelper.i(TAG, "[captureStreamInfo] mCaptureWidth = " + pictureSize.getWidth() +
                ",mCaptureHeight = " + pictureSize.getHeight());
        ArrayList<StreamInfo> captureStreamInfoList = new ArrayList<>();
        captureStreamInfoList.add(captureStreamInfo);
        captureFeatureConfig.addStreamInfo(captureStreamInfoList);
        captureFeatureConfig.addInterfaceParams(mOrientationFeatureParams);
        LogHelper.i(TAG, "[featureConfigs] featureConfigs size = " + featureConfigs.length);

        //start for result
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
        LogHelper.i(TAG, "result.getStreams() = " + result.getStreams().size() );
        mPreviewPostAlgoSurface = result.getStreams().elementAt(0).getmSurface();
        mCapturePostAlgoSurface = result.getStreams().elementAt(1).getmSurface();
        LogHelper.i(TAG, "[startPostAlgo] mPreviewPostAlgoSurface = "
                + mPreviewPostAlgoSurface);
        LogHelper.i(TAG, "[startPostAlgo] mCapturePostAlgoSurface = "
                + mCapturePostAlgoSurface);
        LogHelper.i(TAG, "[startPreviewPostAlgo] - ");
        configParamsDefault();
    }

    private Size getPictureSize() {
        ISettingManager.SettingController controller = mSettingManager.getSettingController();
        String pictureSize = controller.queryValue(KEY_PICTURE_SIZE);
        LogHelper.i(TAG, "[getPictureSize] " + pictureSize);
        if (pictureSize != null) {
            String[] pictureSizes = pictureSize.split("x");
            int width = Integer.parseInt(pictureSizes[0]);
            int height = Integer.parseInt(pictureSizes[1]);
            return new Size(width, height);
        }
        return null;
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
        if (mSession != null) {
            try {
                mSession.abortCaptures();
                mSession.close();
                stopPostAlgo();
            } catch (CameraAccessException e) {
                LogHelper.e(TAG, "[abortOldSession] exception", e);
            }
        }
        mSession = null;
        mBuilder = null;
        mDefaultBuilder = null;
    }
    private void stopPostAlgo() {
        LogHelper.i(TAG, "[stopPostAlgo]");
        mIsStopPostAlgo = true;
        mICameraContext.getCamPostAlgo().stop();
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

//                    builder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE,getFpsRange());
                    mSession.setRepeatingRequest(builder.build(), mCaptureCallback, mModeHandler);
                } else {
                    mBuilder.addTarget(mPreviewPostAlgoSurface);
//
                    mSession.setRepeatingRequest(mBuilder.build(), mCaptureCallback, mModeHandler);
                }
                mCaptureSurface.setCaptureCallback(this);
            } catch (CameraAccessException | RuntimeException e) {
                LogHelper.e(TAG, "[repeatingPreview] error");
            }
        }
    }
    private Range<Integer> getFpsRange () {
        Range<Integer>[] fpsRange = new Range[0];
        try {
            fpsRange = mCameraManager.getCameraCharacteristics(mCurrentCameraId).get(
                    CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        if (fpsRange != null && fpsRange.length > 0) {

            for (Range<Integer> aFpsRange : fpsRange) {
                LogHelper.e(TAG, "__[getFpsRange]" + aFpsRange);
            }
        }
        LogHelper.e(TAG, "__[getFpsRange0]" + fpsRange[2]);
        return fpsRange[2];
    }
    private Builder doCreateAndConfigRequest(int templateType) throws CameraAccessException {
        LogHelper.i(TAG, "[doCreateAndConfigRequest] mCamera2Proxy =" + mCamera2Proxy);
        CaptureRequest.Builder builder = null;
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
                builder.addTarget(mPreviewPostAlgoSurface);
                builder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE,getFpsRange());
            } else if (Camera2Proxy.TEMPLATE_STILL_CAPTURE == templateType) {
                builder.addTarget(mCapturePostAlgoSurface);
                if ("off".equalsIgnoreCase(mZsdStatus)) {
                    builder.addTarget(mPreviewPostAlgoSurface);
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
                LogHelper.d(TAG, "__[PreviewSizeList] :" + "X : "+sizes.get(i).getWidth()+
                        " Y : " + sizes.get(i).getHeight());
            }
            values = CameraUtil.getOptimalPreviewSize(mActivity, sizes, ratio,
                    true);
            LogHelper.d(TAG, "__[PreviewSizeORGValues] :" + "X : "+ values.getWidth()+
                    " Y : " + values.getHeight());
            if(sizes.size() - sizes.indexOf(values) > 0 ){
                LogHelper.d(TAG, "__[getTurndownPreviewLevel] :"
                        + CameraUtil.getTurndownPreviewLevel());
                values = sizes.get(sizes.indexOf(values) + CameraUtil.getTurndownPreviewLevel());
                //Control size below 720P
                while (values.getHeight() > PREVIEW_SIZE_HEIGHT) {
                    values = sizes.get(sizes.indexOf(values) + 1);
                }

            }
            mPreviewWidth = values.getWidth();
            mPreviewHeight = values.getHeight();
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        LogHelper.d(TAG, "__[getTargetPreviewSize] " + mPreviewWidth + " X " + mPreviewHeight);
        return values;
    }

    private void updatePreviewSize() {
        ISettingManager.SettingController controller = mSettingManager.getSettingController();
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
        ISettingManager.SettingController controller = mSettingManager.getSettingController();
        String pictureSize = controller.queryValue(KEY_PICTURE_SIZE);
        LogHelper.i(TAG, "[updatePictureSize] :" + pictureSize);
        if (pictureSize != null) {
            String[] pictureSizes = pictureSize.split("x");
            int width = Integer.parseInt(pictureSizes[0]);
            int height = Integer.parseInt(pictureSizes[1]);
            setPictureSize(new Size(width, height));
        }
    }

    private void initSettings() throws CameraAccessException {
        LogHelper.i(TAG, "[initSettings]");
        mSettingController.postRestriction(DofRestriction.getRestriction().getRelation(
                "on",
                false));
        mSettingController.addViewEntry();
        mSettingController.refreshViewEntry();
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
                    configureSession(true);
                } else {
                    mSettingController.addViewEntry();
                    mSettingController.refreshViewEntry();
                }
                try {
                    initSettings();
                } catch (CameraAccessException e) {
                    e.printStackTrace();
                }
            }
        } catch (RuntimeException e) {
            e.printStackTrace();
        }
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
                mCaptureFrameMap.remove(String.valueOf(num));
            } else if (!CameraUtil.isStillCaptureTemplate(result) && !mFirstFrameArrived) {
                mFirstFrameArrived = true;
                updateCameraState(CameraState.CAMERA_OPENED);
                mModeDeviceCallback.onPreviewCallback(null, 0);
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


    private void configMetaParams(int value) {
        if (!mIsStopPostAlgo) {
            LogHelper.d(TAG, "<configMetaParamsPreview> by tyq");
            FeatureParam mPreviewParams = new FeatureParam();
            mPreviewParams.appendInt(MTK_POSTALGO_PORTRAIT_DOF_LEVEL, value);
            LogHelper.d(TAG, "<configMetaParams> MTK_POSTALGO_PORTRAIT_DOF_LEVEL: " + value);
            mPreviewParams.appendInt(MTK_POSTALGO_PORTRAIT_TYPE, 0);
            LogHelper.d(TAG, "<configMetaParams> MTK_POSTALGO_PORTRAIT_TYPE: " + 0);
            mICameraContext.getCamPostAlgo().configParams(FeaturePipeConfig.INDEX_PREVIEW,
                    mPreviewParams);
        }
    }

    private void configParamsDefault() {
        if (!mIsStopPostAlgo) {
            LogHelper.d(TAG, "<configParamsCapture> by tyq");
            int rotation = CameraUtil.getJpegRotationFromDeviceSpec(
                    Integer.parseInt(mCurrentCameraId), mJpegRotation, mActivity);
            FeatureParam captureParams = new FeatureParam();
            captureParams.appendInt(MTK_POSTALGO_PORTRAIT_TYPE, 1);
            captureParams.appendInt(POSTALGO_PARAMS_JPEG_ORIENTATION_KEY, rotation);
            captureParams.appendInt(MTK_POSTALGO_PORTRAIT_FACEORIENTATION,
                    mFaceOrientation);
            mICameraContext.getCamPostAlgo().configParams(
                    FeaturePipeConfig.INDEX_CAPTURE, captureParams);
        }
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

    private int getSensorOrientationById(String deviceId) {
        CameraManager cameraManager = (CameraManager)
                mActivity.getSystemService(Context.CAMERA_SERVICE);
        try {
            CameraCharacteristics characteristics =
                    cameraManager.getCameraCharacteristics(deviceId);
            return characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
        } catch (CameraAccessException e) {
            LogHelper.e(TAG, "[getSensorOrientationById] CameraAccessException", e);
            return -1;
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, "[getSensorOrientationById] IllegalArgumentException", e);
            return -1;
        }
    }
}
