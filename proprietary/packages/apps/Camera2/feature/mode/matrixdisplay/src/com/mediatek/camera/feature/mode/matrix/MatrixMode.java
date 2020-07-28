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

package com.mediatek.camera.feature.mode.matrix;


import android.content.ContentValues;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.graphics.RectF;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.net.Uri;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.widget.ImageView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.CameraSysTrace;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.memory.IMemoryManager;
import com.mediatek.camera.common.memory.MemoryManagerImpl;
import com.mediatek.camera.common.mode.CameraModeBase;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.mode.photo.HeifHelper;
import com.mediatek.camera.common.mode.photo.ThumbnailHelper;
import com.mediatek.camera.common.mode.photo.device.IDeviceController;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingManagerFactory;
import com.mediatek.camera.common.storage.MediaSaver;
import com.mediatek.camera.common.utils.BitmapCreator;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

public class MatrixMode extends CameraModeBase implements IDeviceController.CaptureDataCallback,
        MatrixDeviceController.DeviceCallback,
        MatrixDeviceController.PreviewSizeCallback,
        MatrixDeviceController.MatrixAvailableCallback,
        IMemoryManager.IMemoryListener,
        MatrixDisplayViewManager.ItemClickListener,
        MatrixDisplayViewManager.ViewStateCallback,
        MatrixDisplayViewManager.EffectUpdateListener,
        IAppUiListener.OnPreviewAreaChangedListener {

    @Override
    public void onEffectUpdated(int position, int effectIndex) {

    }

    @Override
    public void onMatrixAvailable() {
        mMatrixDisplayViewManager.onEffectAvailable();
    }

    /**
     * Color effect enum value.
     */
    enum ModeEnum

    {
        NONE(0),
                ABCOLOR(1),
                COOLCOLOR(2),
                DEV(3),
                FISHEYE(4),
                FOCUS(5),
                GRY(6),
                SEPIATON(7),
                SOFTLIGHT(8);

        private int mValue = 0;

        ModeEnum( int value){
        this.mValue = value;
    }

        /**
         * Get enum value which is in integer.
         *
         * @return The enum value.
         */
        public int getValue () {
        return this.mValue;
    }

        /**
         * Get enum name which is in string.
         *
         * @return The enum name.
         */
        public String getName () {
        return this.toString();
    }
    }

    private static final LogUtil.Tag TAG = new LogUtil.Tag(MatrixMode.class.getSimpleName());
    private static final String KEY_MATRIX_DISPLAY_SHOW = "key_matrix_display_show";
    private static final String KEY_PICTURE_SIZE = "key_picture_size";
    private static final String KEY_FORMTAT = "key_format";
    private static final String KEY_DNG = "key_dng";
    private static final String JPEG_CALLBACK = "jpeg callback";
    private static final String POST_VIEW_CALLBACK = "post view callback";
    private static final long DNG_IMAGE_SIZE = 45 * 1024 * 1024;
    protected static final String PHOTO_CAPTURE_START = "start";
    protected static final String PHOTO_CAPTURE_STOP = "stop";
    protected static final String KEY_PHOTO_CAPTURE = "key_photo_capture";
    private static final String VALUE_NONE = "none";
    // preview size > destination buffer size is OK, no need to set higher resolution
    private static final int MAX_SUPPORTED_PREVIEW_SIZE[] = new int[]{960 * 540, 1280 * 720};

    protected MatrixDeviceController mIDeviceController;
    protected MatrixModeHelper mPhotoModeHelper;
    protected int mCaptureWidth;
    protected int mCaptureHeight = Integer.MAX_VALUE;
    protected volatile boolean mIsResumed = true;
    private String mCameraId;
    private int mOrientation;
    private IAppUiListener.ISurfaceStatusListener mISurfaceStatusListener
            = new SurfaceChangeListener();
    private ISettingManager mISettingManager;
    private MemoryManagerImpl mMemoryManager;
    private byte[] mPreviewData;
    private int mPreviewFormat;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private int mMatrixPreviewWidth;
    private int mMatrixPreviewHeight;
    //make sure it is in capturing to show the saving UI.
    private int mCapturingNumber = 0;
    private Object mPreviewDataSync = new Object();
    private Object mCaptureNumberSync = new Object();
    private StatusMonitor.StatusChangeListener mStatusChangeListener = new MyStatusChangeListener();
    private IMemoryManager.MemoryAction mMemoryState = IMemoryManager.MemoryAction.NORMAL;
    protected StatusMonitor.StatusResponder mPhotoStatusResponder;
    private MatrixDisplayViewManager mMatrixDisplayViewManager;
    private List<CharSequence> mEffectEntryValues;
    private List<CharSequence> mEffectEntries;
    private List<String> mSupportedEffects;
    private int mLayoutWidth;
    private int mLayoutHeight;
    private String mSelectedEffect = VALUE_NONE;
    private int mSelectEffectId = 0;
    private List<String> mSupportedPreviewSize;

    @Override
    public void onPreviewAreaChanged(RectF newPreviewArea, Size previewSize) {
        int layoutWidth = (int) (newPreviewArea.right - newPreviewArea.left);
        int layoutHeight = (int) (newPreviewArea.bottom - newPreviewArea.top);
        LogHelper.d(TAG, "[onPreviewAreaChanged], layoutWidth = " + layoutWidth
                + ", layoutHeight = " + layoutHeight);
        if (layoutWidth != mLayoutWidth || layoutHeight != mLayoutHeight) {
            mLayoutWidth = layoutWidth;
            mLayoutHeight = layoutHeight;
            if (mMatrixDisplayViewManager != null) {
                mMatrixDisplayViewManager.setLayoutSize(mLayoutWidth, mLayoutHeight);
                mMatrixDisplayViewManager.hideView(false, 0);
            }
        }
    }

    @Override
    public void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext,
                     boolean isFromLaunch) {
        super.init(app, cameraContext, isFromLaunch);
        LogHelper.d(TAG, "[init]+");
        super.init(app, cameraContext, isFromLaunch);
        mCameraId = getCameraIdByFacing(mDataStore.getValue(
                KEY_CAMERA_SWITCHER, null, mDataStore.getGlobalScope()));
        LogHelper.d(TAG, "[init] mCameraId " + mCameraId);
        // Device controller must be initialize before set preview size, because surfaceAvailable
        // may be called immediately when setPreviewSize.
        mIApp.getAppUi().applyAllUIEnabled(true);
        mIApp.getAppUi().registerOnPreviewAreaChangedListener(this);
        mIDeviceController = new MatrixDevice2Controller(mIApp.getActivity(), mICameraContext);
        mIDeviceController.setMatrixAvailableCallback(this);
        initSettingManager(mCameraId);
        initStatusMonitor();
        mMatrixDisplayViewManager = new MatrixDisplayViewManager(mIApp.getActivity());
        prepareAndOpenCamera(false, mCameraId, isFromLaunch);
        ThumbnailHelper.setApp(mIApp);
        mMemoryManager = new MemoryManagerImpl(mIApp.getActivity());
        mPhotoModeHelper = new MatrixModeHelper(cameraContext);

        // add matrix display entry view to UI
        ImageView imageView = (ImageView) mIApp.getActivity().getLayoutInflater()
                .inflate(R.layout.matrix_display_entry_view, null, false);
        imageView.setImageDrawable(mIApp.getActivity().getResources()
                .getDrawable(R.drawable.ic_matrix_display_entry));
        imageView.setVisibility(View.VISIBLE);
        mIApp.getAppUi().setEffectViewEntry(imageView);
        imageView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                enterMatrixDisplay();
            }
        });
        LogHelper.d(TAG, "[init]- ");
    }

    @Override
    public void resume(@Nonnull DeviceUsage deviceUsage) {
        super.resume(deviceUsage);
        mIsResumed = true;
        initSettingManager(mCameraId);
        initStatusMonitor();
        mMemoryManager.addListener(this);
        mMemoryManager.initStateForCapture(
                mICameraContext.getStorageService().getCaptureStorageSpace());
        mMemoryState = IMemoryManager.MemoryAction.NORMAL;
        mIDeviceController.queryCameraDeviceManager();
        prepareAndOpenCamera(false, mCameraId, false);
    }

    @Override
    public void pause(@Nullable DeviceUsage nextModeDeviceUsage) {
        LogHelper.i(TAG, "[pause]+");
        super.pause(nextModeDeviceUsage);
        mIsResumed = false;
        mMemoryManager.removeListener(this);
        mIApp.getAppUi().clearPreviewStatusListener(mISurfaceStatusListener);
        if (mNeedCloseCameraIds.size() > 0) {
            prePareAndCloseCamera(needCloseCameraSync(), mCameraId);
            recycleSettingManager(mCameraId);
        } else if (mNeedCloseSession) {
            mIDeviceController.closeSession();
        } else {
            clearAllCallbacks(mCameraId);
            mIDeviceController.stopPreview();
        }
        LogHelper.i(TAG, "[pause]-");
    }

    @Override
    public void unInit() {
        super.unInit();
        mIDeviceController.destroyDeviceController();
        mIApp.getAppUi().unregisterOnPreviewAreaChangedListener(this);
        mIApp.getAppUi().setEffectViewEntry(null);
    }

    @Override
    public DeviceUsage getDeviceUsage(@Nonnull DataStore dataStore, DeviceUsage oldDeviceUsage) {
        ArrayList<String> openedCameraIds = new ArrayList<>();
        String cameraId = getCameraIdByFacing(dataStore.getValue(
                KEY_CAMERA_SWITCHER, null, dataStore.getGlobalScope()));
        openedCameraIds.add(cameraId);
        updateModeDefinedCameraApi();
        return new DeviceUsage(DeviceUsage.DEVICE_TYPE_NORMAL, mCameraApi, openedCameraIds,
                DeviceUsage.BUFFER_FLOW_TYPE_POSTALGO);
    }

    @Override
    public boolean onCameraSelected(@Nonnull String newCameraId) {
        LogHelper.i(TAG, "[onCameraSelected] ,new id:" + newCameraId + ",current id:" + mCameraId);
        super.onCameraSelected(newCameraId);
        //first need check whether can switch camera or not.
        if (canSelectCamera(newCameraId)) {
            //trigger switch camera animation in here
            //must before mCamera = newCameraId, otherwise the animation's orientation and
            // whether need mirror is error.
            synchronized (mPreviewDataSync) {
                startSwitchCameraAnimation();
            }
            doCameraSelect(mCameraId, newCameraId);
            return true;
        } else {
            return false;
        }
    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        return true;
    }

    @Override
    protected boolean doShutterButtonClick() {
        //Storage case
        boolean storageReady = mICameraContext.getStorageService().getCaptureStorageSpace() > 0;
        boolean isDeviceReady = mIDeviceController.isReadyForCapture();
        LogHelper.i(TAG, "onShutterButtonClick, is storage ready : " + storageReady + "," +
                "isDeviceReady = " + isDeviceReady);

        if (storageReady && isDeviceReady && mIsResumed
                && mMemoryState != IMemoryManager.MemoryAction.STOP) {
            //trigger capture animation
            startCaptureAnimation();
            mPhotoStatusResponder.statusChanged(KEY_PHOTO_CAPTURE, PHOTO_CAPTURE_START);
            updateModeDeviceState(MODE_DEVICE_STATE_CAPTURING);
            disableAllUIExceptionShutter();
            mIDeviceController.updateGSensorOrientation(mIApp.getGSensorOrientation());
            mIDeviceController.takePicture(this);
        }
        return true;
    }

    @Override
    public boolean onShutterButtonLongPressed() {
        return false;
    }

    @Override
    public boolean onBackPressed() {
        LogHelper.d(TAG,
                "[onBackPressed] mMatrixDisplayViewManager.isMatrixDisplayShowing() = "
                        + mMatrixDisplayViewManager.isMatrixDisplayShowing());
        if (mMatrixDisplayViewManager.isMatrixDisplayShowing()) {
            exitMatrixDisplay();
            mIDeviceController.startPreview(false, mSelectEffectId);
            return true;
        }
        return false;
    }

    @Override
    public void onOrientationChanged(int orientation) {
        mOrientation = orientation;
        if (mMatrixDisplayViewManager != null) {
            mMatrixDisplayViewManager.setOrientation(mOrientation);
        }
    }

    @Override
    public void onDataReceived(IDeviceController.DataCallbackInfo dataCallbackInfo) {
        //when mode receive the data, need save it.
        byte[] data = dataCallbackInfo.data;
        int format = dataCallbackInfo.mBufferFormat;
        boolean needUpdateThumbnail = dataCallbackInfo.needUpdateThumbnail;
        boolean needRestartPreview = dataCallbackInfo.needRestartPreview;
        LogHelper.d(TAG, "onDataReceived, data = " + data + ",mIsResumed = " + mIsResumed +
                ",needUpdateThumbnail = " + needUpdateThumbnail + ",needRestartPreview = " +
                needRestartPreview);
        if (data != null) {
            CameraSysTrace.onEventSystrace(JPEG_CALLBACK, true);
        }

        //save file first,because save file is in other thread, so will improve the shot to shot
        //performance.
        if (data != null) {
            if (format == ImageFormat.JPEG) {
                saveData(data);
            } else if (format == HeifHelper.FORMAT_HEIF) {
                //check memory to decide whether it can take next picture.
                //if not, show saving
                ISettingManager.SettingController controller
                        = mISettingManager.getSettingController();
                String dngState = controller.queryValue(KEY_DNG);
                long saveDataSize = data.length;
                if (dngState != null && "on".equalsIgnoreCase(dngState)) {
                    saveDataSize = saveDataSize + DNG_IMAGE_SIZE;
                }
                synchronized (mCaptureNumberSync) {
                    mCapturingNumber++;
                    mMemoryManager.checkOneShotMemoryAction(saveDataSize);
                }
                HeifHelper heifHelper = new HeifHelper(mICameraContext);
                ContentValues values = heifHelper.getContentValues(dataCallbackInfo.imageWidth,
                        dataCallbackInfo.imageHeight);
                LogHelper.i(TAG, "onDataReceived,heif values =" + values.toString());
                mICameraContext.getMediaSaver().addSaveRequest(data, values, null,
                        mMediaSaverListener, HeifHelper.FORMAT_HEIF);
                //reset the switch camera to null
                synchronized (mPreviewDataSync) {
                    mPreviewData = null;
                }
            }

        }

        //update thumbnail
        if (data != null && needUpdateThumbnail && format == ImageFormat.JPEG) {
            updateThumbnail(data, dataCallbackInfo.imageWidth);
        }
        if (data != null) {
            CameraSysTrace.onEventSystrace(JPEG_CALLBACK, false);
        }
    }


    @Override
    public void onMemoryStateChanged(IMemoryManager.MemoryAction state) {

    }

//    @Override
//    public void onDataReceived(IDeviceController.DataCallbackInfo dataCallbackInfo) {
//
//    }

    @Override
    public void onPostViewCallback(byte[] data) {
        LogHelper.d(TAG, "[onPostViewCallback] data = " + data + ",mIsResumed = " + mIsResumed);
        CameraSysTrace.onEventSystrace(POST_VIEW_CALLBACK, true);
        if (data != null && mIsResumed) {
            //will update the thumbnail
            int rotation = CameraUtil.getJpegRotationFromDeviceSpec(Integer.parseInt(mCameraId),
                    mIApp.getGSensorOrientation(), mIApp.getActivity());
            Bitmap bitmap = BitmapCreator.createBitmapFromYuv(data,
                    ThumbnailHelper.POST_VIEW_FORMAT,
                    ThumbnailHelper.getThumbnailWidth(),
                    ThumbnailHelper.getThumbnailHeight(),
                    mIApp.getAppUi().getThumbnailViewWidth(),
                    rotation);
            mIApp.getAppUi().updateThumbnail(bitmap);
        }
        CameraSysTrace.onEventSystrace(POST_VIEW_CALLBACK, false);
    }


    @Override
    protected ISettingManager getSettingManager() {
        return mISettingManager;
    }

    @Override
    public void onCameraOpened(String cameraId) {
        updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
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
        if (!mIsResumed) {
            return;
        }
        synchronized (mPreviewDataSync) {
            //Notify preview started.
//            if (!mIsMatrixDisplayShow) {
//                mIApp.getAppUi().applyAllUIEnabled(true);
//            }
            mIApp.getAppUi().applyAllUIEnabled(true);
            mIApp.getAppUi().onPreviewStarted(mCameraId);
            if (mPreviewData == null) {
                mIApp.getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        LogHelper.d(TAG, "[stopAllAnimations] run");
                        //means preview is started,
                        // so need notify switch camera animation need stop.
                        stopSwitchCameraAnimation();
                        //need notify change mode animation need stop if is doing change mode.
                        stopChangeModeAnimation();
                        //stop the capture animation if is doing capturing.
                        stopCaptureAnimation();
                    }
                });
            }
            updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);

            mPreviewData = data;
            mPreviewFormat = format;
        }
    }

    private void enterMatrixDisplay() {
        mIDeviceController.closeSession();
        initializeViewManager();
        mIApp.getAppUi().applyAllUIVisibility(View.INVISIBLE);
        mMatrixDisplayViewManager.setSelectedEffect(mSelectedEffect);
//        mMatrixDisplayViewManager.setMirror(CameraUtil.isCameraFacingFront(
//                mIApp.getActivity(), Integer.parseInt(mCameraId)));
        mMatrixDisplayViewManager.showView();
    }

    private void exitMatrixDisplay() {
        mIApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
        mIApp.getAppUi().onPreviewStarted(mCameraId);
        mMatrixDisplayViewManager.hideView(true, 100);
        mIDeviceController.stopPreview();
        mIDeviceController.closeSession();
    }

    private void initializeViewManager() {
        mMatrixDisplayViewManager.setViewStateCallback(this);
        mMatrixDisplayViewManager.setItemClickListener(this);
        mMatrixDisplayViewManager.setEffectUpdateListener(this);
        mMatrixDisplayViewManager.setSurfaceAvailableListener(
                new MatrixDisplayViewManager.SurfaceAvailableListener() {
            @Override
            public void onSurfaceAvailable(Surface[] surface, int width, int height, int position) {
                computeSuitableMatrixPreviewSize();
                mIDeviceController.setSurfaceInfo(
                        surface, mMatrixPreviewWidth, mMatrixPreviewHeight);
                mIDeviceController.startPreview(true, mSelectEffectId);
                LogHelper.d(TAG, "[SurfaceAvailableListener] width = " + width
                        + ", height = " + height
                        + ", surface.length =" + surface.length);
            }
        });
        setEffectValue(new int[]{0, 1, 2, 3, 4, 5, 6, 7, 8});
        initEffectEntriesAndEntryValues(mSupportedEffects);
        mMatrixDisplayViewManager.setEffectEntriesAndEntryValues(
                mEffectEntries, mEffectEntryValues);
        mMatrixDisplayViewManager.setLayoutSize(mLayoutWidth, mLayoutHeight);
        mMatrixDisplayViewManager.setOrientation(mOrientation);
//        mMatrixDisplayViewManager.setDisplayOrientation(getDisplayOrientation());
    }

    private int getDisplayOrientation() {
        int cameraId = Integer.valueOf(mCameraId);
        int displayRotation = CameraUtil.getDisplayRotation(mIApp.getActivity());
        int displayOrientation = CameraUtil.getDisplayOrientation(displayRotation,
                cameraId, mIApp.getActivity());
        return displayOrientation;
    }

    @Override
    public void onPreviewSizeReady(Size previewSize) {
        LogHelper.d(TAG, "[onPreviewSizeReady] previewSize: " + previewSize.toString());
        updatePictureSizeAndPreviewSize(previewSize);
    }

    @Override
    public void onViewCreated() {

    }

    @Override
    public void onViewScrollOut() {
        exitMatrixDisplay();
        mIDeviceController.startPreview(false, mSelectEffectId);
    }

    @Override
    public void onViewHidden() {

    }

    @Override
    public void onViewDestroyed() {

    }

    private void setEffectsValueParameters() {

    }

    @Override
    public boolean onItemClicked(int effect) {
        LogHelper.d(TAG, "[onItemClicked] effect = " + effect);
        if (mMatrixDisplayViewManager.isMatrixDisplayShowing() == false) {
            return false;
        }
        mSelectEffectId = effect;
        mSelectedEffect = mSupportedEffects.get(mSelectEffectId);
        exitMatrixDisplay();
        mIDeviceController.startPreview(false, mSelectEffectId);
        return false;
    }


    /**
     * surface changed listener.
     */
    private class SurfaceChangeListener implements IAppUiListener.ISurfaceStatusListener {

        @Override
        public void surfaceAvailable(Object surfaceObject, int width, int height) {
            LogHelper.d(TAG, "surfaceAvailable,device controller = " + mIDeviceController
                    + ",w = " + width + ",h = " + height);
            if (mModeHandler != null) {
                mModeHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mIDeviceController != null && mIsResumed) {
                            mIDeviceController.updatePreviewSurface(surfaceObject);
                        }
                    }
                });
            }
        }

        @Override
        public void surfaceChanged(Object surfaceObject, int width, int height) {
            LogHelper.d(TAG, "surfaceChanged,device controller = " + mIDeviceController
                    + ",w = " + width + ",h = " + height);
            if (mModeHandler != null) {
                mModeHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mIDeviceController != null && mIsResumed) {
                            mIDeviceController.updatePreviewSurface(surfaceObject);
                        }
                    }
                });
            }
        }

        @Override
        public void surfaceDestroyed(Object surfaceObject, int width, int height) {

        }
    }

    private class MyStatusChangeListener implements StatusMonitor.StatusChangeListener {
        @Override
        public void onStatusChanged(String key, String value) {
            LogHelper.d(TAG, "[onStatusChanged] key = " + key + ",value = " + value);
            if (KEY_PICTURE_SIZE.equalsIgnoreCase(key)) {
                String[] sizes = value.split("x");
                mCaptureWidth = Integer.parseInt(sizes[0]);
                mCaptureHeight = Integer.parseInt(sizes[1]);
                mIDeviceController.setPictureSize(new Size(mCaptureWidth, mCaptureHeight));
                Size previewSize = mIDeviceController.getPreviewSize((double) mCaptureWidth /
                        mCaptureHeight);
                int width = previewSize.getWidth();
                int height = previewSize.getHeight();
                if (width != mPreviewWidth || height != mPreviewHeight) {
                    onPreviewSizeChanged(width, height);
                }
            }
        }
    }

    private void onPreviewSizeChanged(int width, int height) {
        //Need reset the preview data to null if the preview size is changed.
        synchronized (mPreviewDataSync) {
            mPreviewData = null;
        }
        mPreviewWidth = width;
        mPreviewHeight = height;

        mIApp.getAppUi().setPreviewSize(mPreviewWidth, mPreviewHeight, mISurfaceStatusListener);
    }

    private void initSettingManager(String cameraId) {
        SettingManagerFactory smf = mICameraContext.getSettingManagerFactory();
        mISettingManager = smf.getInstance(
                cameraId,
                getModeKey(),
                ModeType.PHOTO,
                mCameraApi);
    }

    private void initStatusMonitor() {
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        mPhotoStatusResponder = statusMonitor.getStatusResponder(KEY_PHOTO_CAPTURE);
    }

    private void prepareAndOpenCamera(boolean needOpenCameraSync, String cameraId,
                                      boolean needFastStartPreview) {
        mCameraId = cameraId;
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        statusMonitor.registerValueChangedListener(KEY_PICTURE_SIZE, mStatusChangeListener);
        statusMonitor.registerValueChangedListener(KEY_FORMTAT, mStatusChangeListener);
        statusMonitor.registerValueChangedListener(KEY_MATRIX_DISPLAY_SHOW, mStatusChangeListener);

        //before open camera, prepare the device callback and size changed callback.
        mIDeviceController.setDeviceCallback(
                (MatrixDeviceController.DeviceCallback) MatrixMode.this);
        mIDeviceController.setPreviewSizeReadyCallback(MatrixMode.this);
        //prepare device info.
        MatrixDeviceInfo info = new MatrixDeviceInfo();
        info.setCameraId(mCameraId);
        info.setSettingManager(mISettingManager);
        mIDeviceController.openCamera(info);

        initSupportedPreviewSize();
    }

    private void prepareAndOpenMatrixCamera(boolean needOpenCameraSync, String cameraId,
                                            boolean needFastStartPreview) {
        mCameraId = cameraId;
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(mCameraId);
        statusMonitor.registerValueChangedListener(KEY_PICTURE_SIZE, mStatusChangeListener);
        statusMonitor.registerValueChangedListener(KEY_FORMTAT, mStatusChangeListener);
        statusMonitor.registerValueChangedListener(KEY_MATRIX_DISPLAY_SHOW, mStatusChangeListener);

        //before open camera, prepare the device callback and size changed callback.
        mIDeviceController.setDeviceCallback(
                (MatrixDeviceController.DeviceCallback) MatrixMode.this);
        mIDeviceController.setPreviewSizeReadyCallback(MatrixMode.this);
        //prepare device info.
        MatrixDeviceInfo info = new MatrixDeviceInfo();
        info.setCameraId(mCameraId);
        info.setSettingManager(mISettingManager);
        mIDeviceController.openCamera(info);
//        enterMatrixDisplay();

    }

    private void prePareAndCloseCamera(boolean needSync, String cameraId) {
        clearAllCallbacks(cameraId);
        mIDeviceController.closeCamera(needSync);
        //reset the preview size and preview data.
        mPreviewData = null;
        mPreviewWidth = 0;
        mPreviewHeight = 0;
    }

    private void clearAllCallbacks(String cameraId) {
        mIDeviceController.setPreviewSizeReadyCallback(null);
        StatusMonitor statusMonitor = mICameraContext.getStatusMonitor(cameraId);
        statusMonitor.unregisterValueChangedListener(KEY_PICTURE_SIZE, mStatusChangeListener);
        statusMonitor.unregisterValueChangedListener(KEY_FORMTAT, mStatusChangeListener);
        statusMonitor.unregisterValueChangedListener(
                KEY_MATRIX_DISPLAY_SHOW, mStatusChangeListener);
    }

    private void recycleSettingManager(String cameraId) {
        mICameraContext.getSettingManagerFactory().recycle(cameraId);
    }

    private boolean canSelectCamera(@Nonnull String newCameraId) {
        boolean value = true;

        if (newCameraId == null || mCameraId.equalsIgnoreCase(newCameraId)) {
            value = false;
        }
        LogHelper.d(TAG, "[canSelectCamera] +: " + value);
        return value;
    }

    private void startSwitchCameraAnimation() {
        // Prepare the animation data.
        IAppUi.AnimationData data = prepareAnimationData(mPreviewData, mPreviewWidth,
                mPreviewHeight, mPreviewFormat);
        // Trigger animation start.
        mIApp.getAppUi().animationStart(IAppUi.AnimationType.TYPE_SWITCH_CAMERA, data);
    }

    private IAppUi.AnimationData prepareAnimationData(
            byte[] data, int width, int height, int format) {
        // Prepare the animation data.
        IAppUi.AnimationData animationData = new IAppUi.AnimationData();
        animationData.mData = data;
        animationData.mWidth = width;
        animationData.mHeight = height;
        animationData.mFormat = format;
        animationData.mOrientation = mPhotoModeHelper.getCameraInfoOrientation(mCameraId,
                mIApp.getActivity());
        animationData.mIsMirror = mPhotoModeHelper.isMirror(mCameraId, mIApp.getActivity());
        return animationData;
    }

    private void doCameraSelect(String oldCamera, String newCamera) {
        mIApp.getAppUi().applyAllUIEnabled(false);
        mIApp.getAppUi().onCameraSelected(newCamera);
        prePareAndCloseCamera(true, oldCamera);
        recycleSettingManager(oldCamera);
        initSettingManager(newCamera);
        prepareAndOpenCamera(false, newCamera, true);
    }

    private void startCaptureAnimation() {
        mIApp.getAppUi().animationStart(IAppUi.AnimationType.TYPE_CAPTURE, null);
    }

    private void disableAllUIExceptionShutter() {
        mIApp.getAppUi().applyAllUIEnabled(false);
        mIApp.getAppUi().setUIEnabled(IAppUi.SHUTTER_BUTTON, true);
        mIApp.getAppUi().setUIEnabled(IAppUi.SHUTTER_TEXT, false);

    }

    private void saveData(byte[] data) {
        if (data != null) {
            //check memory to decide whether it can take next picture.
            //if not, show saving
            ISettingManager.SettingController controller = mISettingManager.getSettingController();
            String dngState = controller.queryValue(KEY_DNG);
            long saveDataSize = data.length;
            if (dngState != null && "on".equalsIgnoreCase(dngState)) {
                saveDataSize = saveDataSize + DNG_IMAGE_SIZE;
            }
            synchronized (mCaptureNumberSync) {
                mCapturingNumber++;
                mMemoryManager.checkOneShotMemoryAction(saveDataSize);
            }
            String fileDirectory = mICameraContext.getStorageService().getFileDirectory();
            Size exifSize = CameraUtil.getSizeFromExif(data);
            ContentValues contentValues = mPhotoModeHelper.createContentValues(data,
                    fileDirectory, exifSize.getWidth(), exifSize.getHeight());
            mICameraContext.getMediaSaver().addSaveRequest(data, contentValues, null,
                    mMediaSaverListener);
            //reset the switch camera to null
            synchronized (mPreviewDataSync) {
                mPreviewData = null;
            }
        }
    }

    private MediaSaver.MediaSaverListener mMediaSaverListener
            = new MediaSaver.MediaSaverListener() {

        @Override
        public void onFileSaved(Uri uri) {
            mIApp.notifyNewMedia(uri, true);
            synchronized (mCaptureNumberSync) {
                mCapturingNumber--;
                if (mCapturingNumber == 0) {
                    mMemoryState = IMemoryManager.MemoryAction.NORMAL;
                    mIApp.getAppUi().hideSavingDialog();
                    mIApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
                }
            }
            LogHelper.d(TAG, "[onFileSaved] uri = " + uri + ", mCapturingNumber = "
                    + mCapturingNumber);
        }
    };

    private void updateThumbnail(byte[] data, int imageWidth) {
        Bitmap bitmap = BitmapCreator.createBitmapFromJpegWithoutExif(data, imageWidth,
                mIApp.getAppUi().getThumbnailViewWidth());
        mIApp.getAppUi().updateThumbnail(bitmap);
    }

    private void stopSwitchCameraAnimation() {
        mIApp.getAppUi().animationEnd(IAppUi.AnimationType.TYPE_SWITCH_CAMERA);
    }

    private void stopChangeModeAnimation() {
        mIApp.getAppUi().animationEnd(IAppUi.AnimationType.TYPE_SWITCH_MODE);
    }

    private void stopCaptureAnimation() {
        mIApp.getAppUi().animationEnd(IAppUi.AnimationType.TYPE_CAPTURE);
    }

    private void updatePictureSizeAndPreviewSize(Size previewSize) {
        ISettingManager.SettingController controller = mISettingManager.getSettingController();
        String size = controller.queryValue(KEY_PICTURE_SIZE);
        if (size != null && mIsResumed) {
            String[] pictureSizes = size.split("x");
            mCaptureWidth = Integer.parseInt(pictureSizes[0]);
            mCaptureHeight = Integer.parseInt(pictureSizes[1]);
            mIDeviceController.setPictureSize(new Size(mCaptureWidth, mCaptureHeight));
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

    public void setEffectValue(int[] effects) {
        List<String> supportedEffects = convertEnumToString(effects);
        mSupportedEffects = supportedEffects;
    }

    private List<String> convertEnumToString(int[] enumIndexs) {
        ModeEnum[] modes = ModeEnum.values();
        List<String> names = new ArrayList<>(enumIndexs.length);
        for (int i = 0; i < enumIndexs.length; i++) {
            int enumIndex = enumIndexs[i];
            for (ModeEnum mode : modes) {
                if (mode.getValue() == enumIndex) {
                    String name = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
                    names.add(name);
                    break;
                }
            }
        }
        return names;
    }

    private void initEffectEntriesAndEntryValues(List<String> supportedEffects) {
        mEffectEntryValues = new ArrayList<>();
        mEffectEntries = new ArrayList<>();

        String[] originalEffectEntries = mIApp.getActivity().getResources()
                .getStringArray(R.array.pref_camera_coloreffect_entries);
        String[] originalEffectEntryValues = mIApp.getActivity().getResources()
                .getStringArray(R.array.pref_camera_coloreffect_entryvalues);

        for (int i = 0; i < originalEffectEntryValues.length; i++) {
            String effect = originalEffectEntryValues[i];
            for (int j = 0; j < supportedEffects.size(); j++) {
                if (effect.equals(supportedEffects.get(j))) {
                    mEffectEntryValues.add(effect);
                    mEffectEntries.add(originalEffectEntries[i]);
                    break;
                }
            }
        }
    }

    private void computeSuitableMatrixPreviewSize() {
        double refAspectRatio = (double) Math.max(mLayoutWidth, mLayoutHeight)
                / (double) Math.min(mLayoutWidth, mLayoutHeight);

        int selectedWidth = 0;
        int selectedHeight = 0;
        for (int j = 0; j < MAX_SUPPORTED_PREVIEW_SIZE.length; j++) {
            for (int i = 0; i < mSupportedPreviewSize.size(); i++) {
                String size = mSupportedPreviewSize.get(i);
                int index = size.indexOf('x');
                int width = Integer.parseInt(size.substring(0, index));
                int height = Integer.parseInt(size.substring(index + 1));
                double ratio = (double) width / (double) height;

                if (width % 32 != 0) {
                    continue;
                }
                if (width * height > MAX_SUPPORTED_PREVIEW_SIZE[j]) {
                    continue;
                }
                if (Math.abs(refAspectRatio - ratio) > 0.02) {
                    continue;
                }
                if (width * height > selectedWidth * selectedHeight) {
                    selectedWidth = width;
                    selectedHeight = height;
                }
            }
            if (selectedWidth != 0 && selectedHeight != 0) {
                break;
            }
        }
        mMatrixPreviewWidth = selectedWidth;
        mMatrixPreviewHeight = selectedHeight;
        LogHelper.d(TAG, "[computeSuitableMatrixPreviewSize] mMatrixPreviewWidth = "
                        + mMatrixPreviewWidth + ", mMatrixPreviewHeight:" + mMatrixPreviewHeight);
    }

    private void initSupportedPreviewSize() {
        CameraManager cameraManager = (CameraManager)
                mIApp.getActivity().getSystemService(Context.CAMERA_SERVICE);
        try {
            CameraCharacteristics characteristics =
                    cameraManager.getCameraCharacteristics(mCameraId);
            StreamConfigurationMap s = characteristics.get(
                    CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            android.util.Size[] sizes = s.getOutputSizes(SurfaceHolder.class);
            List<android.util.Size> supportedPreviewSize =
                    new ArrayList<android.util.Size>(sizes.length);
            for (android.util.Size size : sizes) {
                supportedPreviewSize.add(size);
            }
            sortSizeInDescending(supportedPreviewSize);
            mSupportedPreviewSize = sizeToStr(supportedPreviewSize);
        } catch (CameraAccessException e) {
            LogHelper.e(TAG, "[initSupportedPreviewSize] CameraAccessException", e);
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, "[initSupportedPreviewSize] IllegalArgumentException", e);
        }
    }

    private List<String> sizeToStr(List<android.util.Size> sizes) {
        List<String> sizeInStr = new ArrayList<>(sizes.size());
        for (android.util.Size size : sizes) {
            sizeInStr.add(size.getWidth() + "x" + size.getHeight());
        }
        return sizeInStr;
    }

    private void sortSizeInDescending(List<android.util.Size> sizes) {
        for (int i = 0; i < sizes.size(); i++) {
            android.util.Size maxSize = sizes.get(i);
            int maxIndex = i;
            for (int j = i + 1; j < sizes.size(); j++) {
                android.util.Size tempSize = sizes.get(j);
                if (tempSize.getWidth() * tempSize.getHeight()
                        > maxSize.getWidth() * maxSize.getHeight()) {
                    maxSize = tempSize;
                    maxIndex = j;
                }
            }
            android.util.Size firstSize = sizes.get(i);
            sizes.set(i, maxSize);
            sizes.set(maxIndex, firstSize);
        }
    }
}
