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

package com.mediatek.camera.v2.stream.pip;

import android.app.Activity;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.media.CamcorderProfile;
import android.util.Size;
import android.view.Surface;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.stream.ICaptureStream;
import com.mediatek.camera.v2.stream.IPreviewStream;
import com.mediatek.camera.v2.stream.IRecordStream;
import com.mediatek.camera.v2.stream.pip.IPipGesture.GestureCallback;
import com.mediatek.camera.v2.stream.pip.pipwrapping.RendererManager;
import com.mediatek.camera.v2.stream.pip.pipwrapping.RendererManager.RendererCallback;

import java.io.FileDescriptor;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;

public class PipStream implements IPreviewStream, ICaptureStream, IRecordStream, IPipStream {
    private static final Tag TAG = new Tag(PipStream.class.getSimpleName());
    // next stream controllers
    private final IPreviewStream mNextPreviewStream;
    private final ICaptureStream mNextCaptureStream;
    private final IRecordStream mNextRecordStream;
    // status callback list
    private final CopyOnWriteArrayList<PipStreamCallback>
             mPipCallbacks = new CopyOnWriteArrayList<PipStreamCallback>();
    // renderer and gesture
    private RendererManager mRendererManager;
    private PipRendererCallback                  mPipRendererCallback = new PipRendererCallback();
    private IPipGesture                 mPipGestureController;
    private PipGestureCallback                   mPipGestureCallback = new PipGestureCallback();
    private int                                  mGsensorOrientation;
    // preview or video recording stream
    private int                                  mPreviewWidth = -1;
    private int                                  mPreviewHeight = -1;
    private Surface                              mOutputPreviewSurface;
    private Surface                              mInputPreviewBottomSurface = null;
    private Surface                              mInputPreviewTopSurface = null;
    // capture or vss stream
    private Surface                              mOutputCaptureSurface;
    private Surface                              mInputPictureBottomSurface = null;
    private Surface                              mInputPictureTopSurface = null;

    public PipStream(IPreviewStream nextPreviewStream,
                     ICaptureStream nextCaptureStream,
                     IRecordStream  nextRecordStream) {
        LogHelper.i(TAG, "PipStream constructor");
        mNextPreviewStream = nextPreviewStream;
        mNextCaptureStream = nextCaptureStream;
        mNextRecordStream  = nextRecordStream;
    }

    /*******************************pip   Stream   Controller**********************/

    @Override
    public void registerPipStreamCallback(PipStreamCallback callback) {
        if (callback != null && !mPipCallbacks.contains(callback)) {
            mPipCallbacks.add(callback);
        }
    }

    @Override
    public void unregisterPipStreamCallback(PipStreamCallback callback) {
        if (callback != null && mPipCallbacks.contains(callback)) {
            mPipCallbacks.remove(callback);
        }
    }

    @Override
    public void open(Activity activity) {
        LogHelper.d(TAG, "[open]+");
        mRendererManager = new RendererManager(activity, mPipRendererCallback);
        mRendererManager.init();
        mPipGestureController = new PipGestureImpl(activity, mPipGestureCallback);
        mPipGestureController.open();

        for (PipStreamCallback callback :mPipCallbacks) {
            callback.onOpened();
        }
        LogHelper.d(TAG, "[open]-");
    }

    @Override
    public void resume() {
        LogHelper.d(TAG, "[resume]+");
        // press home key and enter pip again, should init renderer manager again
        resetPipStreamStatus();
        // mRendererManager.init();
        for (PipStreamCallback callback :mPipCallbacks) {
            callback.onResumed();
        }
        mRendererManager.onActivityResume();
        LogHelper.d(TAG, "[resume]-");
    }

    @Override
    public void onActivityPause() {
        LogHelper.d(TAG, "[onActivityPause]+");
        mRendererManager.onActivityPause();
        LogHelper.d(TAG, "[onActivityPause]-");
    }

    @Override
    public void pause() {
        LogHelper.d(TAG, "[pause]+");
        // mRendererManager.unInit();
        for (PipStreamCallback callback :mPipCallbacks) {
            callback.onPaused();
        }
        LogHelper.d(TAG, "[pause]-");
    }

    @Override
    public void close() {
        LogHelper.d(TAG, "[close]+");
        mRendererManager.unInit();
        if (mPipGestureController != null) {
            mPipGestureController.release();
            mPipGestureController = null;
        }
        for (PipStreamCallback callback :mPipCallbacks) {
            callback.onClosed();
        }
        LogHelper.d(TAG, "[close]-");
    }

    @Override
    public void onTemplateChanged(int rearResId, int frontResId,
            int highlightResId, int editBtnResId) {
        if (mRendererManager != null) {
            mRendererManager.updateEffectTemplates(rearResId,
                    frontResId, highlightResId, editBtnResId);
        }
    }

    @Override
    public void switchingPip() {
        if (mRendererManager != null) {
            mRendererManager.switchPIP();
        }
    }

    @Override
    public void setCaptureSize(Size bottomCaptureSize, Size topCaptureSize) {
        if (bottomCaptureSize != null && topCaptureSize != null) {
            mInputPictureBottomSurface = null;
            mInputPictureTopSurface = null;
            mRendererManager.setPictureSize(bottomCaptureSize, topCaptureSize);
        }
    }

    @Override
    public void onOrientationChanged(int gsensorOrientation) {
        mGsensorOrientation = gsensorOrientation;
        if (mRendererManager != null && mPipGestureController != null) {
            mRendererManager.updateGSensorOrientation(gsensorOrientation);
            mRendererManager.updateTopGraphic(
                    mPipGestureController.getTopGraphicRect(mGsensorOrientation));
        }
    }

    @Override
    public void onPreviewAreaChanged(RectF previewArea) {
        if (mPipGestureController != null) {
            mPipGestureController.onPreviewAreaChanged(previewArea);
        }
    }

    @Override
    public boolean onDown(float x, float y) {
        boolean interceptEvent = false;
        if (mPipGestureController != null && mRendererManager != null) {
            interceptEvent = mPipGestureController.onDown(x, y);
            mRendererManager.updateTopGraphic(
                    mPipGestureController.getTopGraphicRect(mGsensorOrientation));
        }
        return interceptEvent;
    }

    @Override
    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        boolean interceptEvent = false;
        if (mPipGestureController != null && mRendererManager != null) {
            interceptEvent = mPipGestureController.onScroll(dx, dy, totalX, totalY);
            mRendererManager.updateTopGraphic(
                    mPipGestureController.getTopGraphicRect(mGsensorOrientation));
        }
        return interceptEvent;
    }

    @Override
    public boolean onSingleTapUp(float x, float y) {
        if (mPipGestureController != null) {
            return mPipGestureController.onSingleTapUp(x, y);
        }
        return false;
    }

    @Override
    public boolean onLongPress(float x, float y) {
        if (mPipGestureController != null) {
            return mPipGestureController.onLongPress(x, y);
        }
        return false;
    }

    @Override
    public boolean onUp() {
        boolean interceptEvent = false;
        if (mPipGestureController != null && mRendererManager != null) {
            interceptEvent =  mPipGestureController.onUp();
            mRendererManager.updateTopGraphic(
                    mPipGestureController.getTopGraphicRect(mGsensorOrientation));
        }
        return interceptEvent;
    }

    /*******************************Preview Stream Controller**********************/

    @Override
    public boolean updatePreviewSize(Size size) {
        LogHelper.d(TAG, "[updatePreviewSize]+");

        if (!mNextPreviewStream.updatePreviewSize(size) &&
                mPreviewWidth > 0 && mPreviewHeight > 0) {
            LogHelper.i(TAG, "[updatePreviewSize]- not change preview size.");
            return false;
        }
        mPreviewWidth = size.getWidth();
        mPreviewHeight = size.getHeight();
        mInputPreviewBottomSurface   = null;
        mInputPreviewTopSurface      = null;
        mOutputPreviewSurface        = null;

        mRendererManager.setPreviewSize(size);
        if (mPipGestureController != null) {
            mPipGestureController.setPreviewSize(size);
            mRendererManager.updateTopGraphic(
                    mPipGestureController.getTopGraphicRect(mGsensorOrientation));
        }
        LogHelper.d(TAG, "[updatePreviewSize]- preview size changed.");
        return true;
    }

    @Override
    public synchronized Map<String, Surface> getPreviewInputSurfaces() {
        LogHelper.d(TAG, "[getPreviewInputSurfaces]+");
        Map<String, Surface> previewInputSurfaces = new HashMap<String, Surface>();
        if (mInputPreviewBottomSurface == null
                || mInputPreviewTopSurface == null) {
            SurfaceTexture mainCamPvSt = mRendererManager.getMainCamPvSt();
            SurfaceTexture subCamPvSt = mRendererManager.getSubCamPvSt();
            if (mainCamPvSt == null || mainCamPvSt == null) {
                LogHelper.i(TAG, "[getPreviewInputSurfaces] null surface texture");
                return previewInputSurfaces;
            }
            mInputPreviewBottomSurface = new Surface(mainCamPvSt);
            mInputPreviewTopSurface =  new Surface(subCamPvSt);
            // mOutputPreviewSurface =
            //         mNextPreviewStream.getPreviewInputSurfaces().get(PREVIEW_SURFACE_KEY);
            // mRendererManager.setPreviewSurface(mOutputPreviewSurface);
        }
        previewInputSurfaces.put(BOTTOM_SURFACE_KEY, mInputPreviewBottomSurface);
        previewInputSurfaces.put(TOP_SURFACE_KEY, mInputPreviewTopSurface);

        LogHelper.d(TAG, "[getPreviewInputSurfaces]-");
        return previewInputSurfaces;
    }

    @Override
    public void setPreviewStreamCallback(PreviewStreamCallback callback) {
        mNextPreviewStream.setPreviewStreamCallback(callback);
    }

    @Override
    public void setOneShotPreviewSurfaceCallback(PreviewSurfaceCallback surfaceCallback) {
        mNextPreviewStream.setOneShotPreviewSurfaceCallback(surfaceCallback);
    }

    @Override
    public void setPreviewCallback(IPreviewStream.PreviewCallback previewCallback) {
         mNextPreviewStream.setPreviewCallback(previewCallback);
    }

    @Override
    public void onFirstFrameAvailable() {
         // Do nothing.
        // pip display surface receive frame from pip renderer result.
    }

    @Override
    public void releasePreviewStream() {
        mNextPreviewStream.releasePreviewStream();
    }

    /*******************************Capture Stream Controller *********************/

    @Override
    public void setCaptureStreamCallback(CaptureStreamCallback callback) {
        mNextCaptureStream.setCaptureStreamCallback(callback);
    }

    @Override
    public boolean updateCaptureSize(Size pictureSize, int pictureFormat) {
        mNextCaptureStream.updateCaptureSize(pictureSize, pictureFormat);
        return false;
    }

    @Override
    public Map<String, Surface> getCaptureInputSurface() {
        Map<String, Surface> captureInputSurfaces = new HashMap<String, Surface>();

        if (mInputPictureBottomSurface == null || mInputPictureTopSurface == null
                || mOutputCaptureSurface == null) {
            mInputPictureBottomSurface = new Surface(mRendererManager.getMainCamCapSt());
            mInputPictureTopSurface =  new Surface(mRendererManager.getSubCamCapSt());
        }
        mOutputCaptureSurface =
                mNextCaptureStream.getCaptureInputSurface().get(CAPUTRE_SURFACE_KEY);
        mRendererManager.setCaptureOutputSurface(mOutputCaptureSurface);

        captureInputSurfaces.put(BOTTOM_SURFACE_KEY, mInputPictureBottomSurface);
        captureInputSurfaces.put(TOP_SURFACE_KEY, mInputPictureTopSurface);

        return captureInputSurfaces;
    }

    @Override
    public void releaseCaptureStream() {
        mNextCaptureStream.releaseCaptureStream();
    }

    /*******************************Record  Stream Controller *********************/

    @Override
    public void registerPipRecordStreamCallback(RecordStreamStatus callback) {
        mNextRecordStream.registerRecordingObserver(callback);
    }

    @Override
    public void unregisterPipRecordStreamCallback(RecordStreamStatus callback) {
        mNextRecordStream.unregisterCaptureObserver(callback);
    }

    @Override
    public void registerRecordingObserver(RecordStreamStatus status) {
        mNextRecordStream.registerRecordingObserver(status);
    }

    @Override
    public void unregisterCaptureObserver(RecordStreamStatus status) {
        mNextRecordStream.unregisterCaptureObserver(status);
    }

    @Override
    public void setRecordingProfile(CamcorderProfile profile) {
        mNextRecordStream.setRecordingProfile(profile);
    }

    @Override
    public void setMaxDuration(int max_duration_ms) {
        mNextRecordStream.setMaxDuration(max_duration_ms);
    }

    @Override
    public void setMaxFileSize(long max_filesize_bytes) {
        mNextRecordStream.setMaxFileSize(max_filesize_bytes);
    }

    @Override
    public void setOutputFile(String path) {
        mNextRecordStream.setOutputFile(path);
    }

    @Override
    public void setOutputFile(FileDescriptor fd) {
        mNextRecordStream.setOutputFile(fd);
    }

    @Override
    public void setCaptureRate(double fps) {
        mNextRecordStream.setCaptureRate(fps);
    }

    @Override
    public void setLocation(float latitude, float longitude) {
        mNextRecordStream.setLocation(latitude, longitude);
    }

    @Override
    public void setOrientationHint(int degrees) {
        mNextRecordStream.setOrientationHint(degrees);
    }

    @Override
    public void setMediaRecorderParameters(List<String> paramters) {
        mNextRecordStream.setMediaRecorderParameters(paramters);
    }

    @Override
    public void enalbeAudioRecording(boolean enable_audio) {
        mNextRecordStream.enalbeAudioRecording(enable_audio);
    }

    @Override
    public void setAudioSource(int audio_source) {
        mNextRecordStream.setAudioSource(audio_source);
    }

    @Override
    public void setHDRecordMode(String mode) {
     //   mNextRecordStream.setHDRecordMode(mode);
    }

    @Override
    public void setVideoSource(int video_source) {
        mNextRecordStream.setVideoSource(video_source);
    }

    @Override
    public void prepareRecord() {
        mNextRecordStream.prepareRecord();
        mRendererManager.prepareRecording();
    }

    @Override
    public void startRecord() {
        mRendererManager.startRecording();
        mNextRecordStream.startRecord();
    }

    @Override
    public void pauseRecord() {
        mNextRecordStream.pauseRecord();
    }

    @Override
    public void resumeRecord() {
        mNextRecordStream.resumeRecord();
    }

    @Override
    public void stopRecord(boolean need_save_video) {
        mRendererManager.stopRecording();
        mNextRecordStream.stopRecord(need_save_video);
    }

    @Override
    public boolean deleteVideoFile() {
        return mNextRecordStream.deleteVideoFile();
    }

    @Override
    public Surface getRecordInputSurface() {
        Surface recordOutputSurface = mNextRecordStream.getRecordInputSurface();
        mRendererManager.setRecordingSurface(recordOutputSurface);
        return recordOutputSurface;
    }

    @Override
    public void releaseRecordStream() {
        mNextRecordStream.releaseRecordStream();
    }

    @Override
    public void setPreviewSurface(Surface surface) {
        mRendererManager.setPreviewSurface(surface);
    }

    private class PipRendererCallback implements RendererCallback {
        @Override
        public void onFristFrameAvailable(long timestamp) {
            mNextPreviewStream.onFirstFrameAvailable();
        }
    }

    private class PipGestureCallback implements GestureCallback {
        @Override
        public void onTopGraphicTouched() {
            for (PipStreamCallback callback :mPipCallbacks) {
                callback.onTopGraphicTouched();
            }
        }

        @Override
        public void onTopGraphicSingleTapUp() {
            for (PipStreamCallback callback :mPipCallbacks) {
                callback.onSwitchPipEventReceived();
            }
        }
    }

    private void resetPipStreamStatus() {
        mOutputPreviewSurface = null;
        mOutputCaptureSurface = null;

        mInputPreviewBottomSurface = null;
        mInputPreviewTopSurface = null;

        mInputPictureBottomSurface = null;
        mInputPictureTopSurface = null;

        mPreviewWidth = -1;
        mPreviewHeight = -1;
    }
}