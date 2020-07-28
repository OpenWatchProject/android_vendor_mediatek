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

package com.mediatek.camera.v2.mode;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.ImageFormat;
import android.graphics.RectF;
import android.hardware.camera2.CameraCaptureSession.CaptureCallback;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.TotalCaptureResult;
import android.location.Location;
import android.media.AudioManager;
import android.provider.MediaStore;
import android.util.Size;
import android.view.Surface;
import android.view.ViewGroup;

import com.android.camera.v2.app.location.LocationManager;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.mode.normal.CaptureMode;
import com.mediatek.camera.v2.mode.normal.VideoHelper;
import com.mediatek.camera.v2.mode.pip.PipMode;
import com.mediatek.camera.v2.module.ModuleListener;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.platform.ModeChangeListener;
import com.mediatek.camera.v2.platform.app.AppController;
import com.mediatek.camera.v2.platform.app.AppUi;
import com.mediatek.camera.v2.services.CameraServices;
import com.mediatek.camera.v2.services.storage.IStorageService;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.ISettingServant.ISettingChangedListener;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.stream.ICaptureStream;
import com.mediatek.camera.v2.stream.ICaptureStream.CaptureStreamCallback;
import com.mediatek.camera.v2.stream.IPreviewStream;
import com.mediatek.camera.v2.stream.IPreviewStream.PreviewStreamCallback;
import com.mediatek.camera.v2.stream.IPreviewStream.PreviewSurfaceCallback;
import com.mediatek.camera.v2.stream.IRecordStream;
import com.mediatek.camera.v2.stream.IRecordStream.RecordStreamStatus;
import com.mediatek.camera.v2.stream.ImageInfo;
import com.mediatek.camera.v2.stream.RecordStreamView;
import com.mediatek.camera.v2.stream.StreamManager;
import com.mediatek.camera.v2.util.SettingKeys;
import com.mediatek.camera.v2.util.Utils;

import junit.framework.Assert;

import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * A common abstract {@link ModeController} implementation that contains some utility
 * functions and plumbing we don't want every sub-class of {@link ModeController} to
 * duplicate. Hence all {@link ModeController} implementations should sub-class this
 * class instead.
 */
public abstract class AbstractCameraMode implements ModeController, ISettingChangedListener {
    protected final String mFeatureTag;
    private final Tag mTag;
    protected static final Byte          JPEG_QUALITY = 90; // Default JPEG encoding quality.
    // module controller
    protected final  ModuleListener      mModuleListener;
    // app resources
    protected final  AppController       mAppController;
    protected final  CameraServices      mCameraServices;
    protected final  AppUi               mAppUi;
    protected ISettingServant            mSettingServant;
    protected SettingCtrl                mSettingCtroller;
    protected boolean                    mIsCaptureIntent;
    protected VideoHelper mVideoHelper;
    protected Intent                     mIntent;
    protected Activity                   mActivity;
    protected IStorageService mStorageService;
    protected ArrayList<String>          mCaredSettingChangedKeys = new ArrayList<String>();

    // preview controller
    protected IPreviewStream mPreviewController;
    private   PreviewStreamCallback mPreviewStreamCallback;
    // capture controller
    protected ICaptureStream    mCaptureController;
    private   CaptureStreamCallback      mCaptureStreamCallback;
    // record controller
    protected IRecordStream mRecordController;
    // location
    protected LocationManager mLocationManager;
    protected Location mLocation;

    private RecordStreamView             mRecordStreamView;
    private RecordStreamStatus         mRecordStreamCallback;

    /**
     * App resources should be initialized in constructor.
     * @param app the controller at app level.
     * @param moduleListener the controller at module level for mode.
     */
    public AbstractCameraMode(AppController app, ModuleListener moduleListener) {
        Assert.assertNotNull(app);
        mAppController   = app;
        mAppUi           = app.getCameraAppUi();
        mActivity        = app.getActivity();
        mIntent          = app.getActivity().getIntent();
        mModuleListener  = moduleListener;
        mCameraServices  = app.getServices();
        mStorageService = mCameraServices.getStorageService();
        mSettingCtroller = mCameraServices.getSettingController();
        mSettingServant  = mSettingCtroller.getSettingServant(null);
        mFeatureTag      = getTagByModeId(getModeId());
        mLocationManager = app.getLocationManager();

        mTag = new Tag(AbstractCameraMode.class.getSimpleName() + "(" + mFeatureTag + ")");
        updateCaredSettingChangedKeys();
    }

    @Override
    public void onSettingChanged(Map<String, String> result) {
        final boolean needChangeSession = doSettingChanged(result);

        String pictureRatio = result.get(SettingKeys.KEY_PICTURE_RATIO);
        final String pictureSize = result.get(SettingKeys.KEY_PICTURE_SIZE);
        if (pictureRatio != null || pictureSize != null) {
            updatePreviewSize(new PreviewSurfaceCallback() {
                @Override
                public void onPreviewSufaceIsReady(boolean previewSurfaceChanged) {
                    boolean captureSurfaceChanged = updatePictureSize();
                    mModuleListener.onPreviewSurfaceReady();
                    if (needChangeSession ||
                            previewSurfaceChanged ||
                                captureSurfaceChanged) {
                        mModuleListener.requestChangeSessionOutputs(true);
                    }
                }
            });
        } else if (needChangeSession) {
            mModuleListener.requestChangeSessionOutputs(true);
        }
    }

    /**
     * Streams should be got at open, if stream not initialized,
     * {@link StreamManager} will initialized it.
     * <p>
     * Initialize resources this mode needed, these resources will be destroyed in {@link #close()}
     */
    @Override
    public void open(StreamManager streamManager,
            ViewGroup parentView, boolean isCaptureIntent) {
        mIsCaptureIntent = isCaptureIntent;
        initializeStreamControllers(streamManager);
        mRecordStreamView = new RecordStreamView(
                mAppController.getActivity(), mRecordController,
                mAppUi.getModuleLayoutRoot(), isCaptureIntent);
        mSettingServant.registerSettingChangedListener(this, mCaredSettingChangedKeys,
                ISettingChangedListener.MIDDLE_PRIORITY);
        mVideoHelper = new VideoHelper(mCameraServices, mIntent,
                mIsCaptureIntent, mSettingCtroller);
    }

    /**
     *  Exit this mode.
     *  <p>
     *  Destroy resources allocated in {@link #open(StreamManager, ViewGroup, boolean)}}
     */
    @Override
    public void close() {
        unInitializeStreamControllers();
        mRecordStreamView.close();
        mSettingServant.unRegisterSettingChangedListener(this);
    }

    /**
     *  Activity's onResume or switch mode(enter new mode) during onResume will call this.
     */
    @Override
    public void resume() {
        updatePreviewSize(new PreviewSurfaceCallback() {
            @Override
            public void onPreviewSufaceIsReady(boolean surfaceChanged) {
                boolean isNeedChaningSessionOutputs = surfaceChanged;
                isNeedChaningSessionOutputs = updatePictureSize() | isNeedChaningSessionOutputs;
                mModuleListener.onPreviewSurfaceReady();
                if (isNeedChaningSessionOutputs) {
                    mModuleListener.requestChangeSessionOutputs(true);
                }
            }
        });
        // may be ui is disable when capture, while capture do not finish when pause camera
        // so can not enable ui, force to enable ui when resume camera.
        mAppUi.setAllCommonViewEnable(true);
        mAppUi.setSwipeEnabled(true);
    }

    @Override
    public void onActivityPause() {
    }

    /**
     * Activity's onPause or switch mode(exit old mode) during onResume will call this.
     * <p>
     * Note: When onPause arrives, any capturing or recording should be interrupted.
     */
    @Override
    public void pause() {
        mCaptureController.releaseCaptureStream();
    }

    @Override
    public void onOrientationChanged(int orientation) {

    }

    @Override
    public void onPreviewVisibilityChanged(int visibility) {

    }

    @Override
    public void onPreviewAreaChanged(RectF previewArea) {

    }

    @Override
    public void onOkClick() {

    }

    @Override
    public void onCancelClick() {

    }


    @Override
    public void updatePreviewCamera(List<String> previewCameraId, List<String> hasOpenedCamera) {

    }

    @Override
    public void configuringSessionOutputs(List<Surface> sessionOutputSurfaces, String cameraId) {

    }

    @Override
    public void configuringSessionRequests(Map<RequestType, Builder> requestBuilders,
            String cameraId) {

    }

    @Override
    public void onServiceEventUpdate(int event, int value, String cameraId) {
    }

    @Override
    public ModeGestureListener getModeGestureListener() {
        return null;
    }

    @Override
    public void prepareSurfaceBeforeOpenCamera() {
        LogHelper.i(mTag, "[prepareSurfaceBeforeOpenCamera]+");
        updatePreviewSize(new PreviewSurfaceCallback() {
            @Override
            public void onPreviewSufaceIsReady(boolean surfaceChanged) {
                boolean isNeedChaningSessionOutputs = surfaceChanged;
                isNeedChaningSessionOutputs = updatePictureSize() | isNeedChaningSessionOutputs;
                mModuleListener.onPreviewSurfaceReady();
                if (isNeedChaningSessionOutputs) {
                    mModuleListener.requestChangeSessionOutputs(true);
                }
            }
        });
        LogHelper.i(mTag, "[prepareSurfaceBeforeOpenCamera]-");
    }

    @Override
    public void onFirstFrameAvailable() {
        if (mPreviewController != null) {
            mPreviewController.onFirstFrameAvailable();
        }
    }

    @Override
    public void onPreviewCaptureStarted(CaptureRequest request, long timestamp,
            long frameNumber) {

    }

    @Override
    public void onPreviewCaptureCompleted(CaptureRequest request,
            TotalCaptureResult result) {
    }

    @Override
    public CaptureCallback getCaptureCallback() {
        return null;
    }

    @Override
    public boolean onBackPressed() {
        return false;
    }

    @Override
    public boolean switchCamera(String cameraId) {
        return false;
    }

    @Override
    public void onPlay() {

    }

    @Override
    public void onRetake() {

    }

    protected boolean doSettingChanged(Map<String, String> result) {
        return false;
    }

    /**
     * Cared setting items should be added in sub-class's constructors.
     */
    protected void updateCaredSettingChangedKeys() {
        addCaredSettingChangedKeys(SettingKeys.KEY_PICTURE_RATIO);
        addCaredSettingChangedKeys(SettingKeys.KEY_PICTURE_SIZE);
    }

    protected void addCaredSettingChangedKeys(String key) {
        if (key != null && !mCaredSettingChangedKeys.contains(key)) {
            mCaredSettingChangedKeys.add(key);
        }
    }

    protected abstract int getModeId();

    protected PreviewStreamCallback getPreviewStreamCallback() {
        if (mPreviewStreamCallback == null) {
            mPreviewStreamCallback = new PreviewStreamCallback() {
                @Override
                public void onFirstFrameAvailable() {
                    mAppController.onPreviewStarted();
                }
            };
        }
        return mPreviewStreamCallback;
    }

    protected CaptureStreamCallback getCaptureStreamCallback() {
        if (mCaptureStreamCallback == null) {
            mCaptureStreamCallback = new CaptureStreamCallback() {
                @Override
                public void onCaptureCompleted(ImageInfo imageInfo) {
                    Assert.assertNotNull(imageInfo);
                    LogHelper.i(mTag, "onCaptureCompleted");
                    saveJpegInPath(imageInfo.getData(), "/sdcard/DCIM/Camera/test.jpeg");
                }
            };
        }
        return mCaptureStreamCallback;
    }

    protected RecordStreamStatus getRecordStreamCallback() {
        return null;
    }

    /**
     * get the required capture format, default value is ImageForamt.JPEG
     * @return
     */
    protected int getCaptureFormat() {
        return ImageFormat.JPEG;
    }


    protected Size getPreviewSize() {
        return mSettingServant.getPreviewSize();
    }
    /**
     * Default storage is ejected.
     */
    public void onMediaEjected() {
    }
    /**
     * Get current picture size from settings.
     * @return the picture size.
     */
    protected Size getCaptureSize() {
        LogHelper.i(mTag, "[getCaptureSize]+");
        String currentPictureSize = mSettingServant.getSettingValue(SettingKeys.KEY_PICTURE_SIZE);
        Size size = Utils.getSize(currentPictureSize);
        LogHelper.i(mTag, "[getCaptureSize]-");
        return size;
    }

    /**
     * when set preview size, sub-class can use this method do their preview size change action.
     * @return true indicate preview surface changes, need configure new session outputs
     */
    protected boolean changingModePreviewSize() {
        return false;
    }

    /**
     * when set picture size, sub-class can use this method do their picture size change action.
     * @return true indicate picture surface changes, need configure new session outputs
     */
    protected boolean changingModePictureSize() {
        return false;
    }

    /**
     *  update preview stream's preview size.
     *  <p>
     *  Note: if preview size changes, notify app level preview change.
     */
    protected boolean updatePreviewSize(PreviewSurfaceCallback surfaceCallback) {
        LogHelper.i(mTag, "[updatePreviewSize]+");
        Size previewSize = getPreviewSize();
        if (previewSize == null) {
            LogHelper.i(mTag, "why preview size is nulll?");
            return false;
        }

        if (surfaceCallback != null) {
            mPreviewController.setOneShotPreviewSurfaceCallback(surfaceCallback);
        }
        boolean previewSizeChanged = mPreviewController.updatePreviewSize(previewSize);
        previewSizeChanged = changingModePreviewSize() || previewSizeChanged;

        if (previewSizeChanged) {
            // tell preview view surface size changed
            mAppController.updatePreviewSize(previewSize.getWidth(), previewSize.getHeight());
        }
        LogHelper.i(mTag, "[updatePreviewSize]- previewSizeChanged:" + previewSizeChanged +
                " preview size: " + previewSize.getWidth() + " x " + previewSize.getHeight());
        return previewSizeChanged;
    }

    protected boolean updatePictureSize() {
        LogHelper.i(mTag, "[updatePictureSize]+");
        int format = getCaptureFormat();
        Size pictureSize = getCaptureSize();
        if (pictureSize == null) {
            LogHelper.i(mTag, "why picture size is nulll?");
            return false;
        }
        LogHelper.i(mTag, "[updatePictureSize]- pictureSize = " +
                pictureSize.getWidth() + " x " + pictureSize.getHeight()
                + " format = " + format);
        boolean pictureSizeChange = mCaptureController.updateCaptureSize(pictureSize, format);
        mCaptureController.setCaptureStreamCallback(getCaptureStreamCallback());
        pictureSizeChange = changingModePictureSize() || pictureSizeChange;
        if (pictureSizeChange && !MediaStore.ACTION_VIDEO_CAPTURE.equals(mIntent.getAction())) {
            String pictureFormat = pictureSize.getWidth() + "x" +
                    pictureSize.getHeight() + "-superfine";
            mAppUi.showLeftCounts(Utils.getImageSize(pictureFormat), true);
        }
        return pictureSizeChange;
    }

    protected void pauseAudioPlayback() {
        LogHelper.i(mTag, "[pauseAudioPlayback]");
        AudioManager am = (AudioManager) mActivity.getSystemService(Context.AUDIO_SERVICE);
        if (am != null) {
            am.requestAudioFocus(null, AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN);
        }
    }

    protected void releaseAudioFocus() {
        LogHelper.i(mTag, "[releaseAudioFocus]");
        AudioManager am = (AudioManager) mActivity.getSystemService(Context.AUDIO_SERVICE);
        if (am != null) {
            am.abandonAudioFocus(null);
        }
    }

    private String getTagByModeId(int modeId) {
        String tagName = null;
        switch (modeId) {
        case ModeChangeListener.MODE_CAPTURE:
            tagName = CaptureMode.class.getSimpleName();
            break;
        case ModeChangeListener.MODE_PIP:
            tagName = PipMode.class.getSimpleName();
            break;
        default:
            break;
        }
        return tagName;
    }

    private void initializeStreamControllers(StreamManager streamManager) {
        mPreviewController = streamManager.getPreviewController(getModeId());
        mPreviewController.setPreviewStreamCallback(getPreviewStreamCallback());

        mCaptureController = streamManager.getCaptureController(getModeId());

        mRecordController = streamManager.getRecordController(getModeId());
        mRecordController.registerRecordingObserver(getRecordStreamCallback());
    }

    private void unInitializeStreamControllers() {
        if (mPreviewController != null) {
            mPreviewController.setPreviewStreamCallback(null);
        }
        if (mCaptureController != null) {
            mCaptureController.setCaptureStreamCallback(null);
        }
        if (mRecordController != null) {
            mRecordController.unregisterCaptureObserver(getRecordStreamCallback());
        }
    }

    private void saveJpegInPath(byte[] jepgdata, String path) {
        LogHelper.i(mTag, "[saveJpegInPath]+ path = " + path);
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(path);
            out.write(jepgdata);
            out.close();
        } catch (IOException e) {
            LogHelper.e(mTag, "[saveJpegInPath]Failed to write image,exception:", e);
        } finally {
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    LogHelper.e(mTag, "[saveJpegInPath]ioexception:", e);
                }
            }
            LogHelper.i(mTag, "[saveJpegInPath]-");
        }
    }
}
