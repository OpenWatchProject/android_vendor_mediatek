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

package com.mediatek.camera.feature.mode.vsdof.photo.device;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
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
import android.hardware.camera2.params.SessionConfiguration;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.hardware.camera2.params.OutputConfiguration;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Process;
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
import com.mediatek.camera.common.mode.photo.device.IDeviceController;
import com.mediatek.camera.common.mode.photo.P2DoneInfo;
import com.mediatek.camera.common.mode.photo.ThumbnailHelper;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Configurator;
import com.mediatek.camera.common.sound.ISoundPlayback;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.portability.AdvancedCamera;
import com.mediatek.camera.feature.mode.vsdof.photo.DeviceInfo;
import com.mediatek.camera.feature.mode.vsdof.photo.SdofPhotoRestriction;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;

/**
 * An implementation of {@link ISdofPhotoDeviceController} with Camera2Proxy.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
class SdofPhotoDeviceController extends Device2Controller implements
        ISdofPhotoDeviceController,
        CaptureSurface.ImageCallback,
        ISettingManager.SettingDevice2Requester {
    private static final Tag TAG = new Tag(SdofPhotoDeviceController.class.getSimpleName());
    private static final String KEY_PICTURE_SIZE = "key_picture_size";
    private static final int CAPTURE_FORMAT = ImageFormat.JPEG;
    private static final int CAPTURE_MAX_NUMBER = 2;
    private static final int WAIT_TIME = 5;
    private static final String VSDOF_KEY = "com.mediatek.multicamfeature.multiCamFeatureMode";
    private static final String STEREO_WARNING_KEY = "com.mediatek.stereofeature.stereowarning";
    private static final String DOF_LEVEL_KEY = "com.mediatek.stereofeature.doflevel";
    private static final String PREVIEW_SIZE_KEY =
            "com.mediatek.vsdoffeature.vsdofFeaturePreviewSize";
    private static final String MTK_VSDOF_FEATURE_WARNING =
            "com.mediatek.vsdoffeature.vsdofFeatureWarning";
    private static final String MTK_VSDOF_FEATURE_CAPTURE_WARNING_MSG =
            "com.mediatek.vsdoffeature.vsdofFeatureCaptureWarningMsg";
    private String mZsdStatus = "on";
    private static final int[] VSDOF_KEY_VALUE = new int[]{1};
    private static final int[] PREVIEW_SIZE_KEY_VALUE = new int[]{1080,1920};
    private static final int LEVEL_DEFAULT = 7;
    private int mCurrentLevel = LEVEL_DEFAULT;
    private static int[] CURRENT_DOFLEVEL_VALUE = new int[]{LEVEL_DEFAULT};
    private static final int DUAL_CAMERA_TOO_FAR = 1 << 31;
    private static int mVsdofWarningValue = 0;
    private static int[] DUAL_CAMERA_TOO_FAR_VALUE = new int[]{mVsdofWarningValue};

    //add for BG service
    private CaptureRequest.Key<int[]> mBGServicePrereleaseKey = null;
    private CaptureRequest.Key<int[]> mBGServiceImagereaderIdKey = null;
    private static final int[] BGSERVICE_PRERELEASE_KEY_VALUE = new int[]{1};

    private final Activity mActivity;
    private final CameraManager mCameraManager;
    private CameraCharacteristics mCameraCharacteristics;
    private CaptureRequest.Key<int[]> mVsdofKey = null;
    private CaptureRequest.Key<int[]> mWarningKey = null;
    private CaptureResult.Key<int[]> mStereoWarningKey = null;
    private CaptureResult.Key<int[]> mVsdofWarningKey = null;
    private CaptureRequest.Key<int[]> mDofLevelKey = null;
    private CaptureRequest.Key<int[]> mPreviewSizeKey = null;
    private final CaptureSurface mCaptureSurface;
    private final ICameraContext mICameraContext;
    private final Object mSurfaceHolderSync = new Object();
    private final StateCallback mDeviceCallback = new DeviceStateCallback();
    private StereoWarningCallback mStereoWarningCallback = null;
    private boolean mIsBGServiceEnabled = false;
    private BGServiceKeeper mBGServiceKeeper;

    private int mJpegRotation;
    private volatile int mPreviewWidth;
    private volatile int mPreviewHeight;
    private volatile Camera2Proxy mCamera2Proxy;
    private volatile Camera2CaptureSessionProxy mSession;

    private boolean mFirstFrameArrived = false;
    private boolean mIsPictureSizeChanged = false;

    private Lock mLockState = new ReentrantLock();
    private Lock mDeviceLock = new ReentrantLock();
    private CameraState mCameraState = CameraState.CAMERA_UNKNOWN;

    private String mCurrentCameraId;
    private Surface mPreviewSurface;
    private IDeviceController.CaptureDataCallback mCaptureDataCallback;
    private Object mSurfaceObject;
    private ISettingManager mSettingManager;
    private DeviceCallback mModeDeviceCallback;
    private SettingController mSettingController;
    private PreviewSizeCallback mPreviewSizeCallback;
    private CameraDeviceManager mCameraDeviceManager;
    private SettingDevice2Configurator mSettingDevice2Configurator;
    private static Relation sRelation = null;
    //for p2 done
    private final CaptureSurface mThumbnailSurface;
    private ConcurrentHashMap mCaptureFrameMap = new ConcurrentHashMap<String, Boolean>();
    private static final int CAPTURE_MAX_NUMBER_THUMBNAIL = 5;

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
    SdofPhotoDeviceController(@Nonnull Activity activity, @Nonnull ICameraContext context) {
        LogHelper.d(TAG, "[SdofPhotoDeviceController]");
        mActivity = activity;
        mCameraManager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        mICameraContext = context;
        mBGServiceKeeper = mICameraContext.getBGServiceKeeper();
        if (mBGServiceKeeper != null) {
            DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(
                    mActivity.getApplicationContext()).getDeviceDescriptionMap().get("0");
            if (deviceDescription != null && mBGServiceKeeper.getBGHidleService() != null) {
                mIsBGServiceEnabled = true;
                mBGServicePrereleaseKey = deviceDescription.getKeyBGServicePrerelease();
                mBGServiceImagereaderIdKey = deviceDescription.getKeyBGServiceImagereaderId();
            }
        }
        LogHelper.d(TAG, "mBGServiceKeeper = " + mBGServiceKeeper
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
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
        mThumbnailSurface = new CaptureSurface();
        mThumbnailSurface.setCaptureCallback(this);
        mThumbnailSurface.setFormat(ThumbnailHelper.FORMAT_TAG);
    }

    @Override
    public void queryCameraDeviceManager() {
        mCameraDeviceManager = mICameraContext.getDeviceManager(CameraApi.API2);
    }

    @Override
    public void openCamera(DeviceInfo info) {
        String cameraId = info.getCameraId();
        boolean sync = info.getNeedOpenCameraSync();
        LogHelper.i(TAG, "[openCamera] cameraId : " + cameraId + ",sync = " + sync);
        initSettingManager(info.getSettingManager());
        if (canOpenCamera(cameraId)) {
            try {
                mDeviceLock.tryLock(WAIT_TIME, TimeUnit.SECONDS);
                mCurrentCameraId = cameraId;
                mCameraCharacteristics = mCameraManager.getCameraCharacteristics(mCurrentCameraId);
                mVsdofKey = CameraUtil.getAvailableSessionKeys(
                        mCameraCharacteristics, VSDOF_KEY);
                mWarningKey = CameraUtil.getRequestKey(
                        mCameraCharacteristics, MTK_VSDOF_FEATURE_CAPTURE_WARNING_MSG);
                mDofLevelKey = CameraUtil.getRequestKey(
                        mCameraCharacteristics, DOF_LEVEL_KEY);
                mPreviewSizeKey = CameraUtil.getAvailableSessionKeys(
                        mCameraCharacteristics, PREVIEW_SIZE_KEY);
                initSettings();
                updateCameraState(CameraState.CAMERA_OPENING);
                doOpenCamera(sync);
            } catch (CameraOpenException e) {
                if (CameraOpenException.ExceptionType.SECURITY_EXCEPTION == e.getExceptionType()) {
                    CameraUtil.showErrorInfoAndFinish(mActivity,
                            CameraUtil.CAMERA_HARDWARE_EXCEPTION);
                }
            } catch (CameraAccessException e) {
                CameraUtil.showErrorInfoAndFinish(mActivity, CameraUtil.CAMERA_HARDWARE_EXCEPTION);
            } catch (InterruptedException e) {
                e.printStackTrace();
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
                LogHelper.e(TAG, "[openCamera] camera process killed due to" +
                        " getCameraCharacteristics() error");
                Process.killProcess(Process.myPid());
            } finally {
                mDeviceLock.unlock();
            }
        }
    }

    @Override
    public void updatePreviewSurface(Object surfaceObject) {
        LogHelper.d(TAG, "[updatePreviewSurface] surfaceHolder = " + surfaceObject + " state = "
                + mCameraState);
        synchronized (mSurfaceHolderSync) {
            mSurfaceObject = surfaceObject;
            if (surfaceObject instanceof SurfaceHolder) {
                mPreviewSurface = surfaceObject == null ? null :
                        ((SurfaceHolder) surfaceObject).getSurface();
            } else if (surfaceObject instanceof SurfaceTexture) {
                mPreviewSurface = surfaceObject == null ? null :
                        new Surface((SurfaceTexture) surfaceObject);
            }
            boolean isStateReady = CameraState.CAMERA_OPENED == mCameraState;
            if (isStateReady && mCamera2Proxy != null) {
                if (surfaceObject != null) {
                    configureSession();
                } else {
                    stopPreview();
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
        mIsPictureSizeChanged = mCaptureSurface.updatePictureInfo(size.getWidth(),
                size.getHeight(), CAPTURE_FORMAT, CAPTURE_MAX_NUMBER);
        if (mIsBGServiceEnabled) {
            mBGServiceKeeper.setBGCaptureSurface(mCaptureSurface);
        }
        double ratio = (double) size.getWidth() / size.getHeight();
        ThumbnailHelper.updateThumbnailSize(ratio);
        if (ThumbnailHelper.isPostViewSupported()) {
            mThumbnailSurface.updatePictureInfo(ThumbnailHelper.getThumbnailWidth(),
                    ThumbnailHelper.getThumbnailHeight(),
                    ThumbnailHelper.IMAGE_BUFFER_FORMAT,
                    CAPTURE_MAX_NUMBER_THUMBNAIL);
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
    }

    @Override
    public void startPreview() {
        LogHelper.i(TAG, "[startPreview]");
        configureSession();
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
                Builder builder = doCreateAndConfigStillCaptureRequest();
                mSession.capture(builder.build(), mCaptureCallback, mModeHandler);
            } catch (CameraAccessException e) {
                e.printStackTrace();
                LogHelper.e(TAG, "[takePicture] error because create build fail.");
            }
        }
    }

    @Override
    public void setZSDStatus(String value) {
        mZsdStatus = value;
    }

    @Override
    public void updateGSensorOrientation(int orientation) {
        mJpegRotation = orientation;
    }

    @Override
    public void closeCamera(boolean sync) {
        LogHelper.i(TAG, "[closeCamera] + sync = " + sync + " current state : " + mCameraState);
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
                if (!mIsBGServiceEnabled) {
                    mCaptureSurface.releaseCaptureSurface();
                } else {
                    if (mCaptureSurface.getPictureNumLeft() != 0) {
                        mCaptureSurface.releaseCaptureSurfaceLater(true);
                    } else {
                        mCaptureSurface.releaseCaptureSurface();
                    }
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                super.doCameraClosed(mCamera2Proxy);
                mDeviceLock.unlock();
            }
            recycleVariables();
        }
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
            configureSession();
        }
        return new Size(mPreviewWidth, mPreviewHeight);
    }

    @Override
    public void onPictureCallback(byte[] data,
                                  int format, String formatTag, int width, int height) {
        LogHelper.d(TAG, "<onPictureCallback> data = " + data + ", format = " + format
                + ", formatTag" + formatTag + ", width = " + width + ", height = " + height
                + ", mCaptureDataCallback = " + mCaptureDataCallback);
        //need notify preview is started, such as API2 take picture done,
        //will do start preview, the ui can enable by :onCaptureCompleted ->
        //mModeDeviceCallback.onPreviewCallback();
        mFirstFrameArrived = false;
        if (mCaptureDataCallback != null) {
            IDeviceController.DataCallbackInfo info = new IDeviceController.DataCallbackInfo();
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
        repeatingPreview();
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
    public CaptureSurface getModeSharedCaptureSurface()
            throws IllegalStateException {
        //not support now
        throw new IllegalStateException("get invalid capture surface!");
    }

    @Override
    public Surface getModeSharedPreviewSurface() throws IllegalStateException {
        //not support now
        throw new IllegalStateException("get invalid capture surface!");
    }

    @Override
    public Surface getModeSharedThumbnailSurface() throws IllegalStateException {
        //not support now
        throw new IllegalStateException("get invalid capture surface!");
    }

    @Override
    public Camera2CaptureSessionProxy getCurrentCaptureSession() {
        return mSession;
    }

    @Override
    public void requestRestartSession() {
        configureSession();
    }

    @Override
    public int getRepeatingTemplateType() {
        return Camera2Proxy.TEMPLATE_PREVIEW;
    }

    @Override
    public void setStereoWarningCallback(StereoWarningCallback callback) {
        mStereoWarningCallback = callback;
    }

    @Override
    public void setVsDofLevelParameter(int level) {
        if (mCurrentLevel != level) {
            mCurrentLevel = level;
            createAndChangeRepeatingRequest();
        }
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

    private Builder doCreateAndConfigStillCaptureRequest()
            throws CameraAccessException {
        LogHelper.i(TAG, "[doCreateAndConfigStillCaptureRequest]" +
                "mCamera2Proxy =" + mCamera2Proxy);
        CaptureRequest.Builder builder = null;
        if (mCamera2Proxy != null) {
            builder = mCamera2Proxy.createCaptureRequest(Camera2Proxy.TEMPLATE_STILL_CAPTURE);
            mSettingDevice2Configurator.configCaptureRequest(builder);
            ThumbnailHelper.configPostViewRequest(builder);
            builder.addTarget(mCaptureSurface.getSurface());
            setSpecialVendorTag(builder);
            if ("off".equalsIgnoreCase(mZsdStatus)) {
                LogHelper.d(TAG, "[takePicture] take picture with preview image.");
                builder.addTarget(mPreviewSurface);
            } else {
                LogHelper.d(TAG, "[takePicture] take picture not with preview image.");
            }
            if (ThumbnailHelper.isPostViewOverrideSupported()) {
                builder.addTarget(mThumbnailSurface.getSurface());
            }
            ThumbnailHelper.setDefaultJpegThumbnailSize(builder);
            P2DoneInfo.enableP2Done(builder);
            int rotation = CameraUtil.getJpegRotationFromDeviceSpec(
                    Integer.parseInt(mCurrentCameraId), mJpegRotation, mActivity);
            builder.set(CaptureRequest.JPEG_ORIENTATION, rotation);
            if (mICameraContext.getLocation() != null) {
                builder.set(CaptureRequest.JPEG_GPS_LOCATION, mICameraContext.getLocation());
            }
        }
        return builder;
    }

    private void setSpecialVendorTag(CaptureRequest.Builder builder) {
        if (mVsdofKey != null) {
            builder.set(mVsdofKey, VSDOF_KEY_VALUE);
            LogHelper.d(TAG, "[setSpecialVendorTag] set vsdof key.");
        }
        if (mDofLevelKey != null) {
            CURRENT_DOFLEVEL_VALUE[0] = mCurrentLevel;
            builder.set(mDofLevelKey, CURRENT_DOFLEVEL_VALUE);
            LogHelper.d(TAG, "[setSpecialVendorTag] sdoflevel " + mCurrentLevel);
        }
        if (mPreviewSizeKey != null) {
            PREVIEW_SIZE_KEY_VALUE[0] = mPreviewWidth;
            PREVIEW_SIZE_KEY_VALUE[1] = mPreviewHeight;
            builder.set(mPreviewSizeKey, PREVIEW_SIZE_KEY_VALUE);
            LogHelper.d(TAG, "[setSpecialVendorTag] set preview size " +
                    "width " + mPreviewWidth + ", height " + mPreviewHeight);
        }
        if (mWarningKey != null) {
            DUAL_CAMERA_TOO_FAR_VALUE = new int[]{mVsdofWarningValue};
            builder.set(mWarningKey, DUAL_CAMERA_TOO_FAR_VALUE);
            LogHelper.d(TAG, "[setSpecialVendorTag] set warning key to capture " +
                    DUAL_CAMERA_TOO_FAR_VALUE[0]);
        } else {
            LogHelper.d(TAG, "[setSpecialVendorTag] mWarningKey is null");
        }
        configureBGService(builder);
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

    private void initSettingManager(ISettingManager settingManager) {
        mSettingManager = settingManager;
        settingManager.updateModeDevice2Requester(this);
        mSettingDevice2Configurator = settingManager.getSettingDevice2Configurator();
        mSettingController = settingManager.getSettingController();
    }

    private void initSettings() throws CameraAccessException {
        mSettingManager.createAllSettings();
        P2DoneInfo.setCameraCharacteristics(mActivity.getApplicationContext(),
                Integer.parseInt(mCurrentCameraId));
        mSettingDevice2Configurator.setCameraCharacteristics(mCameraCharacteristics);
        SdofPhotoRestriction.setCameraCharacteristics(mCameraCharacteristics,
                mICameraContext.getDataStore());
        sRelation = SdofPhotoRestriction.getRestriction().getRelation("on", false);
        if (sRelation != null) {
            mSettingController.postRestriction(sRelation);
        }
        mSettingController.addViewEntry();
        mSettingController.refreshViewEntry();
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

    private void configureSession() {
        LogHelper.i(TAG, "[configureSession]");
        mDeviceLock.lock();
        try {
            if (mCamera2Proxy != null) {
                abortOldSession();
                List<Surface> surfaces = new LinkedList<>();
                surfaces.add(mPreviewSurface);
                surfaces.add(mCaptureSurface.getSurface());
                if (ThumbnailHelper.isPostViewSupported()) {
                    surfaces.add(mThumbnailSurface.getSurface());
                }
                mSettingDevice2Configurator.configSessionSurface(surfaces);
                Builder builder = doCreateAndConfigRequest(Camera2Proxy.TEMPLATE_PREVIEW);
                mCamera2Proxy.createCaptureSession(surfaces, mSessionCallback,
                        mModeHandler, builder);
                mIsPictureSizeChanged = false;
            }
        } catch (CameraAccessException e) {
            LogHelper.e(TAG, "[configureSession] error");
        } catch (RuntimeException e) {
            e.printStackTrace();
        } finally {
            mDeviceLock.unlock();
        }
    }

    private void abortOldSession() {
        if (mSession != null) {
            try {
                mSession.abortCaptures();
                mSession = null;
            } catch (CameraAccessException e) {
                LogHelper.e(TAG, "[abortOldSession] exception", e);
            }
        }
    }

    private void repeatingPreview() {
        LogHelper.i(TAG, "[repeatingPreview] mSession =" + mSession + " mCamera =" + mCamera2Proxy);
        if (mSession != null && mCamera2Proxy != null) {
            try {
                mFirstFrameArrived = false;
                Builder builder = doCreateAndConfigRequest(Camera2Proxy.TEMPLATE_PREVIEW);
                mCaptureSurface.setCaptureCallback(this);
                mSession.setRepeatingRequest(builder.build(), mPreviewCallback, mModeHandler);
            } catch (CameraAccessException | RuntimeException e) {
                LogHelper.e(TAG, "[repeatingPreview] error");
            }
        }
    }

    private Builder doCreateAndConfigRequest(int templateType) throws CameraAccessException {
        LogHelper.i(TAG, "[doCreateAndConfigRequest] mCamera2Proxy =" + mCamera2Proxy);
        CaptureRequest.Builder builder = null;
        if (mCamera2Proxy != null) {
            builder = mCamera2Proxy.createCaptureRequest(templateType);
            setSpecialVendorTag(builder);
            mSettingDevice2Configurator.configCaptureRequest(builder);
            ThumbnailHelper.configPostViewRequest(builder);
            builder.addTarget(mPreviewSurface);
        }
        return builder;
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
            LogHelper.e(TAG, "[getTargetPreviewSize] camera process killed due to" +
                    " getCameraCharacteristics() error");
            Process.killProcess(Process.myPid());
        }
        LogHelper.d(TAG, "[getTargetPreviewSize] " + mPreviewWidth + " X " + mPreviewHeight);
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

    @Override
    public void doCameraOpened(@Nonnull Camera2Proxy camera2proxy) {
            LogHelper.i(TAG, "[onOpened]  camera2proxy = " + camera2proxy + " preview surface = "
                    + mPreviewSurface + "  mCameraState = " + mCameraState + "camera2Proxy id = "
                    + camera2proxy.getId() + " mCameraId = " + mCurrentCameraId);
            try {
                if (CameraState.CAMERA_OPENING == getCameraState()
                        && camera2proxy != null && camera2proxy.getId().equals(mCurrentCameraId)) {
                    mCamera2Proxy = camera2proxy;
                    if (mModeDeviceCallback != null) {
                        mModeDeviceCallback.onCameraOpened(mCurrentCameraId);
                    }
                    updateCameraState(CameraState.CAMERA_OPENED);
                    ThumbnailHelper.setCameraCharacteristics(mCameraCharacteristics,
                            mActivity.getApplicationContext(), Integer.parseInt(mCurrentCameraId));
                    updatePreviewSize();
                    if (mPreviewSizeCallback != null) {
                        mPreviewSizeCallback.onPreviewSizeReady(new Size(mPreviewWidth,
                                mPreviewHeight));
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
            }
        }

    /**
     * Camera session callback.
     */
    private final Camera2CaptureSessionProxy.StateCallback mSessionCallback = new
            Camera2CaptureSessionProxy.StateCallback() {

                @Override
                public void onConfigured(@Nonnull Camera2CaptureSessionProxy session) {
                    LogHelper.i(TAG, "[onConfigured],session = " + session);
                    mDeviceLock.lock();
                    mSession = session;
                    try {
                        if (CameraState.CAMERA_OPENED == getCameraState()) {
                            synchronized (mSurfaceHolderSync) {
                                if (mPreviewSurface != null) {
                                    repeatingPreview();
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
            LogHelper.d(TAG, "[CaptureCallback.onCaptureStarted]capture started, frame: "
                    + frameNumber);
            if (CameraUtil.isStillCaptureTemplate(request) && mIsBGServiceEnabled) {
                mCaptureSurface.increasePictureNum();
            }
            mCaptureFrameMap.put(String.valueOf(frameNumber), Boolean.FALSE);
            mICameraContext.getSoundPlayback().play(ISoundPlayback.SHUTTER_CLICK);
        }

        @Override
        public void onCaptureProgressed(CameraCaptureSession session, CaptureRequest request,
                                        CaptureResult partialResult) {
            super.onCaptureProgressed(session, request, partialResult);
            if (mCamera2Proxy == null
                    || mModeDeviceCallback == null
                    || session.getDevice() != mCamera2Proxy.getCameraDevice()) {
                return;
            }
            if (P2DoneInfo.checkP2DoneResult(partialResult)) {
                //p2done comes, it can do next capture
                long num = partialResult.getFrameNumber();
                if (mCaptureFrameMap.containsKey(String.valueOf(num))) {
                    mCaptureFrameMap.put(String.valueOf(num), Boolean.TRUE);
                }
                LogHelper.d(TAG, "[CaptureCallback.onCaptureProgressed] P2done comes, frame: "
                        + num);
                updateCameraState(CameraState.CAMERA_OPENED);
                mModeDeviceCallback.onPreviewCallback(null, 0);
            }
        }

        @Override
        public void onCaptureCompleted(@Nonnull CameraCaptureSession session,
                                       @Nonnull CaptureRequest request,
                                       @Nonnull TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            LogHelper.d(TAG, "[CaptureCallback.onCaptureCompleted] mModeDeviceCallback = "
                    + mModeDeviceCallback + ", mFirstFrameArrived = " + mFirstFrameArrived);
            if (mCamera2Proxy == null
                    || mModeDeviceCallback == null
                    || session.getDevice() != mCamera2Proxy.getCameraDevice()) {
                return;
            }
            mSettingDevice2Configurator.getRepeatingCaptureCallback().onCaptureCompleted(
                    session, request, result);
            long num = result.getFrameNumber();
            if (mCaptureFrameMap.containsKey(String.valueOf(num))
                    && Boolean.FALSE == mCaptureFrameMap.get(String.valueOf(num))) {
                mFirstFrameArrived = true;
                updateCameraState(CameraState.CAMERA_OPENED);
                mModeDeviceCallback.onPreviewCallback(null, 0);
            }
            mCaptureFrameMap.remove(String.valueOf(num));
        }

        @Override
        public void onCaptureFailed(@Nonnull CameraCaptureSession session,
                                    @Nonnull CaptureRequest request,
                                    @Nonnull CaptureFailure failure) {
            super.onCaptureFailed(session, request, failure);
            LogHelper.e(TAG, "[CaptureCallback.onCaptureFailed], framenumber: "
                    + failure.getFrameNumber() + ", reason: " + failure.getReason()
                    + ", sequenceId: " + failure.getSequenceId()
                    +", isCaptured: " + failure.wasImageCaptured());
            if (mCamera2Proxy == null
                    || mModeDeviceCallback == null
                    || session.getDevice() != mCamera2Proxy.getCameraDevice()) {
                return;
            }
            mSettingDevice2Configurator.getRepeatingCaptureCallback().onCaptureFailed(
                    session, request, failure);
            mCaptureFrameMap.remove(String.valueOf(failure.getFrameNumber()));
            updateCameraState(CameraState.CAMERA_OPENED);
            mModeDeviceCallback.onPreviewCallback(null, 0);
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


    /**
     * Preview callback.
     */
    private final CaptureCallback mPreviewCallback = new CaptureCallback() {

        @Override
        public void onCaptureStarted(CameraCaptureSession session, CaptureRequest request, long
                timestamp, long frameNumber) {
            super.onCaptureStarted(session, request, timestamp, frameNumber);
        }

        @Override
        public void onCaptureCompleted(@Nonnull CameraCaptureSession session,
                @Nonnull CaptureRequest request, @Nonnull TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            LogHelper.d(TAG, "[PreviewCallback.onCaptureCompleted] mModeDeviceCallback = "
                    + mModeDeviceCallback + ", mFirstFrameArrived = " + mFirstFrameArrived);
            mSettingDevice2Configurator.getRepeatingCaptureCallback().onCaptureCompleted(
                    session, request, result);
            if (mModeDeviceCallback != null && !mFirstFrameArrived) {
                mFirstFrameArrived = true;
                mModeDeviceCallback.onPreviewCallback(null, 0);
            }
            notifyWarningKey(result);
        }

        @Override
        public void onCaptureFailed(@Nonnull CameraCaptureSession session,
                @Nonnull CaptureRequest request, @Nonnull CaptureFailure failure) {
            super.onCaptureFailed(session, request, failure);
            LogHelper.d(TAG, "[PreviewCallback.onCaptureFailed]");
            mSettingDevice2Configurator.getRepeatingCaptureCallback().onCaptureFailed(
                    session, request, failure);
        }
    };

    private void notifyWarningKey(TotalCaptureResult result) {
        if (mStereoWarningCallback == null) {
            return;
        }
        // Normal warning value(normal,too close,lens cover,too light)
        if (mStereoWarningKey == null) {
            mStereoWarningKey =
                    CameraUtil.getResultKey(mCameraCharacteristics, STEREO_WARNING_KEY);
        }
        if (mStereoWarningKey != null) {
            int[] warningVlue = result.get(mStereoWarningKey);
            LogHelper.d(TAG, "[notifyWarningKey] mStereoWarningKey value is " + warningVlue);
            if (warningVlue != null && warningVlue.length > 0) {
                mStereoWarningCallback.onWarning(warningVlue[0]);
            }
        }
        // from p1 node return(too far)
        if (mVsdofWarningKey == null) {
            mVsdofWarningKey =
                    CameraUtil.getResultKey(mCameraCharacteristics, MTK_VSDOF_FEATURE_WARNING);
        }
        if (mVsdofWarningKey != null) {
            int[] warningVlue = result.get(mVsdofWarningKey);
            LogHelper.d(TAG, "[notifyWarningKey] mVsdofWarningKey value is " + warningVlue);
            if (warningVlue != null && warningVlue.length > 0) {
                LogHelper.d(TAG, "[notifyWarningKey] onWarning too far");
                mVsdofWarningValue = warningVlue[0];
                mStereoWarningCallback.onWarning(warningVlue[0]);
                return;
            }
        }
        mVsdofWarningValue = 0;
    }
}
