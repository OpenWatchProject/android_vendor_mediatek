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

package com.mediatek.camera.feature.mode.vsdof.photo;

import android.content.ContentValues;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.view.OrientationEventListener;

import com.mediatek.camera.CameraAppService;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener.ISurfaceStatusListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.mode.CameraModeBase;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingManagerFactory;
import com.mediatek.camera.common.storage.MediaSaver.MediaSaverListener;
import com.mediatek.camera.common.utils.BitmapCreator;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.feature.mode.vsdof.photo.device.DeviceControllerFactory;
import com.mediatek.camera.feature.mode.vsdof.photo.device.ISdofPhotoDeviceController;
import com.mediatek.camera.feature.mode.vsdof.photo.view.SdofPictureSizeSettingView;
import com.mediatek.camera.feature.mode.vsdof.view.SdofViewCtrl;
import com.mediatek.camera.portability.CameraEx;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentLinkedQueue;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * Sdof photo mode implementation.
 */
public class SdofPhotoMode extends CameraModeBase implements
        ISdofPhotoDeviceController.DeviceCallback, ISdofPhotoDeviceController.PreviewSizeCallback {
    private static final Tag TAG = new Tag(SdofPhotoMode.class.getSimpleName());
    private static final String PROPERTY_KEY_CLIENT_APP_MODE = "vendor.mtk.client.appmode";
    private static final String APP_MODE_NAME_MTK_DUAL_CAMERA = "MtkStereo";
    private static final String KEY_STEREO_PICTURE_SIZE = "key_stereo_picture_size";
    private static final String KEY_PICTURE_SIZE = "key_picture_size";
    private static final int POST_VIEW_FORMAT = ImageFormat.NV21;
    private StereoDataSynth mStereoDataSynth = new StereoDataSynth();

    protected ISdofPhotoDeviceController mISdofPhotoDeviceController;
    protected SdofPhotoHelper mSdofPhotoHelper;
    protected int mCaptureWidth;
    // make sure the picture size ratio = mCaptureWidth / mCaptureHeight not to NAN.
    protected int mCaptureHeight = Integer.MAX_VALUE;
    //the reason is if surface is ready, let it to set to device controller, otherwise
    //if surface is ready but activity is not into resume ,will found the preview
    //can not start preview.
    private volatile boolean mIsResumed = true;
    private String mCameraId;
    private boolean mIsMmsdkCallbackSupported = false;
    private boolean mIsPostViewCallbackSupported = false;

    private ISurfaceStatusListener mISurfaceStatusListener = new SurfaceChangeListener();
    private ISettingManager mISettingManager;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private HandlerThread mAnimationHandlerThread;
    private Handler mAnimationHandler;
    private List<String> mSupportSizes;
    private ConcurrentLinkedQueue<Uri> mUriQueue = new ConcurrentLinkedQueue<>();
    private int mOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;
    private SdofPictureSizeSettingView mSdofPictureSizeSettingView;
    private SdofViewCtrl mSdofViewCtrl = new SdofViewCtrl();

    @Override
    public void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext,
                     boolean isFromLaunch) {
        LogHelper.d(TAG, "[init]+");
        super.init(app, cameraContext, isFromLaunch);
        startPreWarmService();
        mIApp.getAppUi().applyAllUIEnabledImmediately(false);
        mSdofPhotoHelper = new SdofPhotoHelper(cameraContext);
        createAnimationHandler();
        mCameraId = getCameraIdByFacing(mDataStore.getValue(
                KEY_CAMERA_SWITCHER, null, mDataStore.getGlobalScope()));

        // Device controller must be initialize before set preview size, because surfaceAvailable
        // may be called immediately when setPreviewSize.
        DeviceControllerFactory deviceControllerFactory = new DeviceControllerFactory();
        mISdofPhotoDeviceController
                = deviceControllerFactory.createDeviceController(app.getActivity(),
                mCameraApi, mICameraContext);
        initSettingManager(mCameraId);
        prepareAndOpenCamera(false, mCameraId, false);
        mSdofViewCtrl.setViewChangeListener(mViewChangeListener);
        mSdofViewCtrl.init(app);
        mSdofPictureSizeSettingView = new SdofPictureSizeSettingView(
                mPictureSizeChangeListener,
                app.getActivity(), cameraContext.getDataStore());
        mStereoDataSynth.addCompoundJpegListener(mCompoundJpegListener);
        mStereoDataSynth.setDeviceCtrl(mISdofPhotoDeviceController);
        LogHelper.d(TAG, "[init]- ");
    }

    @Override
    public void resume(@Nonnull DeviceUsage deviceUsage) {
        LogHelper.i(TAG, "[resume]+");
        super.resume(deviceUsage);
        mIsResumed = true;
        initSettingManager(mCameraId);
        mISdofPhotoDeviceController.queryCameraDeviceManager();
        prepareAndOpenCamera(false, mCameraId, false);

        //enable all the ui when resume done.
        mIApp.getAppUi().applyAllUIEnabled(true);
        LogHelper.d(TAG, "[resume]-");
    }

    @Override
    public void pause(@Nullable DeviceUsage nextModeDeviceUsage) {
        LogHelper.i(TAG, "[pause]+");
        super.pause(nextModeDeviceUsage);
        boolean needCloseCamera =
                mNeedCloseCameraIds == null ? true : mNeedCloseCameraIds.size() > 0;
        mIsResumed = false;
        //clear the surface listener
        mIApp.getAppUi().clearPreviewStatusListener(mISurfaceStatusListener);
        mSdofViewCtrl.unInit();
        if (needCloseCamera) {
            prePareAndCloseCamera(true);
            recycleSettingManager(mCameraId);
        } else {
            //clear the all callbacks.
            clearAllCallbacks();
            //do stop preview
            mISdofPhotoDeviceController.stopPreview();
        }
        LogHelper.d(TAG, "[pause]-");
    }

    @Override
    public void unInit() {
        LogHelper.i(TAG, "[unInit]+");
        super.unInit();
        mIApp.getAppUi().removeSettingView(mSdofPictureSizeSettingView);
        destroyAnimationHandler();
        mISdofPhotoDeviceController.destroyDeviceController();
        LogHelper.d(TAG, "[unInit]-");
    }

    @Override
    public void onOrientationChanged(int orientation) {
        mSdofViewCtrl.onOrientationChanged(orientation);
    }

    @Override
    public DeviceUsage getDeviceUsage(@Nonnull DataStore dataStore, DeviceUsage oldDeviceUsage) {
        ArrayList<String> openedCameraIds = new ArrayList<>();
        String cameraId = getCameraIdByFacing(dataStore.getValue(
                KEY_CAMERA_SWITCHER, null, dataStore.getGlobalScope()));
        openedCameraIds.add(cameraId);
        updateModeDefinedCameraApi();
        return new DeviceUsage(DeviceUsage.DEVICE_TYPE_STEREO_VSDOF, mCameraApi, openedCameraIds);
    }

    @Override
    public boolean isModeIdle() {
        return mISdofPhotoDeviceController.isDeviceCtrlIdle();
    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        return true;
    }

    @Override
    public boolean onShutterButtonClick() {
        //Storage case
        boolean storageReady = mICameraContext.getStorageService().getCaptureStorageSpace() > 0;
        boolean isDeviceReady = mISdofPhotoDeviceController.isReadyForCapture();
        LogHelper.i(TAG, "onShutterButtonClick, is storage ready : " + storageReady + "," +
                "isDeviceReady = " + isDeviceReady);

        if (storageReady && isDeviceReady) {
            //trigger capture animation
            startCaptureAnimation();
            updateModeDeviceState(MODE_DEVICE_STATE_CAPTURING);
            mIApp.getAppUi().applyAllUIEnabled(false);
            mOrientation = mIApp.getGSensorOrientation();
            mISdofPhotoDeviceController.updateGSensorOrientation(mOrientation);
            mISdofPhotoDeviceController.takePicture(mCaptureEnhanceCallback);
        }
        return true;
    }

    @Override
    public boolean onShutterButtonLongPressed() {
        return false;
    }

    @Override
    protected ISettingManager getSettingManager() {
        return mISettingManager;
    }

    @Override
    public void onCameraOpened(String cameraId) {
        LogHelper.d(TAG, "[onCameraOpened]");
        mISdofPhotoDeviceController.setStereoWarningCallback(mWarningCallback);
        Relation relation = SdofPhotoRestriction.getRestriction().getRelation("on", false);
        String pictureSizeId = mISettingManager.getSettingController()
                .queryValue(KEY_PICTURE_SIZE);
        relation.addBody(KEY_PICTURE_SIZE, pictureSizeId, pictureSizeId);
        mISettingManager.getSettingController().postRestriction(relation);

        updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
    }

    @Override
    public void isVsdofSupported(boolean isSupport, String callbackBufferList) {
        if (mIsResumed) {
            mSdofViewCtrl.showView();
            mSdofViewCtrl.onOrientationChanged(mIApp.getGSensorOrientation());
            mStereoDataSynth.setCallbackInfo(callbackBufferList);
        }
    }

    @Override
    public void beforeCloseCamera() {
        updateModeDeviceState(MODE_DEVICE_STATE_CLOSED);
    }

    @Override
    public void afterStopPreview() {
        updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
    }

    @Override
    public void onPreviewCallback(byte[] data, int format) {
        // Because we want use the one preview frame for doing switch camera animation
        // so will dismiss the later frames.
        // The switch camera data will be restore to null when camera close done.
        if (!mIsResumed) {
            return;
        }
        //Notify preview started.
        mIApp.getAppUi().applyAllUIEnabled(true);
        mIApp.getAppUi().onPreviewStarted(mCameraId);
        stopAllAnimations();
        updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);
    }

    @Override
    public void onPreviewSizeReady(Size previewSize, List<String> pictureSizes) {
        LogHelper.i(TAG, "[onPreviewSizeReady] previewSize: " + previewSize.toString());
        mSupportSizes = pictureSizes;
        // update video quality view
        mSdofPictureSizeSettingView.setEntryValues(pictureSizes);
        mSdofPictureSizeSettingView.setDefaultValue(pictureSizes.get(0));

        mIApp.getAppUi().addSettingView(mSdofPictureSizeSettingView);
        updatePictureSizeAndPreviewSize(previewSize);
    }

    private final SdofPictureSizeSettingView.Listener mPictureSizeChangeListener
            = new SdofPictureSizeSettingView.Listener() {
        @Override
        public void onSizeChanged(String newSize) {
            String[] sizes = newSize.split("x");
            mCaptureWidth = Integer.parseInt(sizes[0]);
            mCaptureHeight = Integer.parseInt(sizes[1]);
            mISdofPhotoDeviceController.setPictureSize(new Size(mCaptureWidth, mCaptureHeight));
            Size previewSize = mISdofPhotoDeviceController.getPreviewSize((double) mCaptureWidth
                    / mCaptureHeight);
            int width = previewSize.getWidth();
            int height = previewSize.getHeight();
            if (width != mPreviewWidth || height != mPreviewHeight) {
                onPreviewSizeChanged(width, height);
            }
        }
    };

    private StereoDataSynth.CompoundJpegListener mCompoundJpegListener
            = new StereoDataSynth.CompoundJpegListener() {
        @Override
        public void onXmpCompoundJpeg(byte[] xmpJpeg, long captureTime) {
            LogHelper.i(TAG, "save by xmp jpeg  MmsdkCallbackSupported : "
                    + mIsMmsdkCallbackSupported);
            if (xmpJpeg == null) {
                LogHelper.e(TAG, "[onXmpCompoundJpeg] xmp jpeg is null!");
                return;
            }
            if (mIsMmsdkCallbackSupported) {
                Size exifSize = CameraUtil.getSizeFromExif(xmpJpeg);
                ContentValues contentValues = mSdofPhotoHelper.createContentValues(xmpJpeg,
                        exifSize.getWidth(), exifSize.getHeight(), captureTime, false);
                synchronized (mUriQueue) {
                    if (!mUriQueue.isEmpty()) {
                        mICameraContext.getMediaSaver().updateSaveRequest(xmpJpeg,
                                contentValues,
                                null,
                                mUriQueue.poll());
                    }
                }
            } else {
                saveFile(xmpJpeg, captureTime,
                        true);
            }
        }
    };

    private ISdofPhotoDeviceController.CaptureEnhanceCallback mCaptureEnhanceCallback
            = new ISdofPhotoDeviceController.CaptureEnhanceCallback() {
        @Override
        public void onP2DoneCallback() {
            //when mode receive the data, need save it.
            LogHelper.d(TAG, "[onP2DoneCallback]");
            //Notify preview started.
            mIApp.getAppUi().applyAllUIEnabled(true);
            mIApp.getAppUi().onPreviewStarted(mCameraId);
            updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);
        }

        @Override
        public void onPostViewCallback(byte[] data, long captureTime,
                boolean isMmsdkCallbackSupported) {
            LogHelper.d(TAG, "[onPostViewCallback] data = " + data + ",captureTime:" +
                      captureTime);
            if (data != null) {
                mIsPostViewCallbackSupported = true;
                mIsMmsdkCallbackSupported = isMmsdkCallbackSupported;
                //will update the thumbnail
                int rotation = CameraUtil.getJpegRotationFromDeviceSpec(Integer.parseInt(mCameraId),
                        mOrientation, mIApp.getActivity());
                Size postViewSize = mISdofPhotoDeviceController.getPostViewSize();
                Bitmap bitmap = null;
                byte[] thumbnailJpeg = null;
                if (rotation == 0 || rotation == 180) {
                    bitmap = BitmapCreator.createBitmapFromYuv(data, POST_VIEW_FORMAT,
                            postViewSize.getWidth(), postViewSize.getHeight(),
                            mIApp.getAppUi().getThumbnailViewWidth(),
                            0);
                    if (isMmsdkCallbackSupported) {
                        thumbnailJpeg = BitmapCreator.covertYuvDataToJpeg(data, POST_VIEW_FORMAT,
                                postViewSize.getWidth(), postViewSize.getHeight());
                    }
                } else {
                    bitmap = BitmapCreator.createBitmapFromYuv(data, POST_VIEW_FORMAT,
                            postViewSize.getHeight(), postViewSize.getWidth(),
                            mIApp.getAppUi().getThumbnailViewWidth(),
                            0);
                    if (isMmsdkCallbackSupported) {
                        thumbnailJpeg = BitmapCreator.covertYuvDataToJpeg(data, POST_VIEW_FORMAT,
                                postViewSize.getHeight(), postViewSize.getWidth());
                    }
                }

                // need to insert DB when thumbnail ready for gallery show.
                if (isMmsdkCallbackSupported) {
                    ContentValues contentValues = mSdofPhotoHelper.createThumbnailContentValues(
                            mCaptureWidth, mCaptureHeight,
                            captureTime);
                    mICameraContext.getMediaSaver().addSaveRequest(thumbnailJpeg, contentValues,
                            null,
                            mMediaSaverListener);
                }
                mIApp.getAppUi().updateThumbnail(bitmap);
            }
        }

        @Override
        public void onCaptureDone() {
            LogHelper.d(TAG, "[onCaptureDone] mUriQueue is empty: " + mUriQueue.isEmpty());
            if (!mUriQueue.isEmpty()) {
                mUriQueue.clear();
            }
        }
    };

    private void saveFile(byte[] jpegData, long time, boolean isStereo) {
        LogHelper.i(TAG, "[saveFile] isStereo " + isStereo);
        Size exifSize = CameraUtil.getSizeFromExif(jpegData);
        ContentValues contentValues = mSdofPhotoHelper.createContentValues(jpegData,
                exifSize.getWidth(), exifSize.getHeight(), time, false);
        mICameraContext.getMediaSaver().addSaveRequest(jpegData, contentValues, null,
                mMediaSaverListener);
        if (!mIsPostViewCallbackSupported) {
            //update thumbnail
            Bitmap bitmap = BitmapCreator.createBitmapFromJpeg(jpegData, mIApp.getAppUi()
                    .getThumbnailViewWidth());
            mIApp.getAppUi().updateThumbnail(bitmap);
        }
    }

    private void onPreviewSizeChanged(int width, int height) {
        mPreviewWidth = width;
        mPreviewHeight = height;
        mIApp.getAppUi().setPreviewSize(mPreviewWidth, mPreviewHeight, mISurfaceStatusListener);
    }

    private void prepareAndOpenCamera(boolean needOpenCameraSync, String cameraId,
                                      boolean needFastStartPreview) {
        mCameraId = cameraId;

        //before open camera, prepare the preview callback and size changed callback.
        mISdofPhotoDeviceController.setDeviceCallback(this);
        mISdofPhotoDeviceController.setPreviewSizeReadyCallback(this);
        //prepare device info.
        DeviceInfo info = new DeviceInfo();
        info.setCameraId(mCameraId);
        info.setSettingManager(mISettingManager);
        info.setNeedOpenCameraSync(needOpenCameraSync);

        LogHelper.i(TAG, "[setProperty] stereo camera mode");
        CameraEx.setProperty(PROPERTY_KEY_CLIENT_APP_MODE,
                APP_MODE_NAME_MTK_DUAL_CAMERA);
        mISdofPhotoDeviceController.openCamera(info);
    }

    private void prePareAndCloseCamera(boolean needSync) {
        clearAllCallbacks();
        mISdofPhotoDeviceController.closeCamera(needSync);
        mPreviewWidth = 0;
        mPreviewHeight = 0;
    }

    private void clearAllCallbacks() {
        mISdofPhotoDeviceController.setPreviewSizeReadyCallback(null);
    }

    private void initSettingManager(String cameraId) {
        SettingManagerFactory smf = mICameraContext.getSettingManagerFactory();
        mISettingManager = smf.getInstance(
                cameraId,
                getModeKey(),
                ModeType.PHOTO,
                mCameraApi);
    }

    private void recycleSettingManager(String cameraId) {
        mICameraContext.getSettingManagerFactory().recycle(cameraId);
    }

    private void createAnimationHandler() {
        mAnimationHandlerThread = new HandlerThread("Animation_handler");
        mAnimationHandlerThread.start();
        mAnimationHandler = new Handler(mAnimationHandlerThread.getLooper());
    }

    private void destroyAnimationHandler() {
        if (mAnimationHandlerThread.isAlive()) {
            mAnimationHandlerThread.quit();
            mAnimationHandler = null;
        }
    }

    private MediaSaverListener mMediaSaverListener = new MediaSaverListener() {

        @Override
        public void onFileSaved(Uri uri) {
            LogHelper.d(TAG, "[onFileSaved] uri = " + uri);
            if (uri == null) {
                return;
            }
            if (mIsMmsdkCallbackSupported) {
                synchronized (mUriQueue) {
                    mUriQueue.add(uri);
                }
            }
            mIApp.notifyNewMedia(uri, true);
        }
    };

    private ISdofPhotoDeviceController.StereoWarningCallback mWarningCallback
            = new ISdofPhotoDeviceController.StereoWarningCallback() {
        @Override
        public void onWarning(int type) {
            LogHelper.i(TAG, "[StereoWarningCallback onWarning] " + type);
            switch (type) {
                case SdofViewCtrl.DUAL_CAMERA_LOW_LIGHT:
                    break;
                case SdofViewCtrl.DUAL_CAMERA_READY:
                    break;
                case SdofViewCtrl.DUAL_CAMERA_TOO_CLOSE:
                    break;
                case SdofViewCtrl.DUAL_CAMERA_LENS_COVERED:
                    break;
                default:
                    LogHelper.w(TAG, "Warning message don't need to show");
                    break;
            }
            mSdofViewCtrl.showWarningView(type);
        }
    };

    private void stopAllAnimations() {
        LogHelper.d(TAG, "[stopAllAnimations]");
        if (mAnimationHandler == null) {
            return;
        }
        //clear the old one.
        mAnimationHandler.removeCallbacksAndMessages(null);
        mAnimationHandler.post(new Runnable() {
            @Override
            public void run() {
                LogHelper.d(TAG, "[stopAllAnimations] run");
                //stop the capture animation if is doing capturing.
                stopCaptureAnimation();
            }
        });
    }

    private void startCaptureAnimation() {
        mIApp.getAppUi().animationStart(IAppUi.AnimationType.TYPE_CAPTURE, null);
    }

    private void stopCaptureAnimation() {
        mIApp.getAppUi().animationEnd(IAppUi.AnimationType.TYPE_CAPTURE);
    }

    private void updatePictureSizeAndPreviewSize(Size previewSize) {
        String pictureSize = mICameraContext.getDataStore().getValue(
                KEY_STEREO_PICTURE_SIZE,
                mSupportSizes.get(0),
                mICameraContext.getDataStore().getGlobalScope());
        if (pictureSize != null && mIsResumed) {
            String[] pictureSizes = pictureSize.split("x");
            mCaptureWidth = Integer.parseInt(pictureSizes[0]);
            mCaptureHeight = Integer.parseInt(pictureSizes[1]);
            mISdofPhotoDeviceController.setPictureSize(new Size(mCaptureWidth, mCaptureHeight));
            int width = previewSize.getWidth();
            int height = previewSize.getHeight();
            LogHelper.d(TAG, "[updatePictureSizeAndPreviewSize] picture size: " + mCaptureWidth +
                    " X" + mCaptureHeight + ",current preview size:" + mPreviewWidth + " X " +
                    mPreviewHeight + ",new value :" + width + " X " + height);
            if (width != mPreviewWidth || height != mPreviewHeight) {
                onPreviewSizeChanged(width, height);
            }
        }
    }

    /**
     * surface changed listener.
     */
    private class SurfaceChangeListener implements ISurfaceStatusListener {

        @Override
        public void surfaceAvailable(Object surfaceObject, int width, int height) {
            LogHelper.d(TAG, "surfaceAvailable,device controller = " + mISdofPhotoDeviceController
                    + ",w = " + width + ",h = " + height);
            if (mISdofPhotoDeviceController != null && mIsResumed) {
                mISdofPhotoDeviceController.updatePreviewSurface(surfaceObject);
            }
        }

        @Override
        public void surfaceChanged(Object surfaceObject, int width, int height) {
            LogHelper.d(TAG, "surfaceChanged, device controller = " + mISdofPhotoDeviceController
                    + ",w = " + width + ",h = " + height);
            if (mISdofPhotoDeviceController != null && mIsResumed) {
                mISdofPhotoDeviceController.updatePreviewSurface(surfaceObject);
            }
        }

        @Override
        public void surfaceDestroyed(Object surfaceObject, int width, int height) {
            LogHelper.d(TAG, "surfaceDestroyed,device controller = " + mISdofPhotoDeviceController);
        }
    }

    private SdofViewCtrl.ViewChangeListener mViewChangeListener
            = new SdofViewCtrl.ViewChangeListener() {
        @Override
        public void onVsDofLevelChanged(String level) {
            mISdofPhotoDeviceController.setVsDofLevelParameter(level);
        }

        @Override
        public void onTouchPositionChanged(String value) {
        }
    };

    private void startPreWarmService() {
        if (!CameraUtil.isServiceRun(mIApp.getActivity().getApplicationContext(),
                "com.mediatek.camera.CameraAppService")) {
            Intent appServiceIntent = new Intent(mIApp.getActivity().getApplicationContext(),
                    CameraAppService.class);
            CameraUtil.startService(mIApp.getActivity().getApplicationContext(), appServiceIntent);
        }
    }
}
