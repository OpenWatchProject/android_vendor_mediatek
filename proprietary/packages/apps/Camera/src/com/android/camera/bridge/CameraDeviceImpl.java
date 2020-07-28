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

package com.android.camera.bridge;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.AsdCallback;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.AutoFocusMoveCallback;
import android.hardware.Camera.AutoRamaCallback;
import android.hardware.Camera.AutoRamaMoveCallback;
import android.hardware.Camera.ContinuousShotCallback;
import android.hardware.Camera.DistanceInfoCallback;
import android.hardware.Camera.Face;
import android.hardware.Camera.FbOriginalCallback;
//TODO: Dng BSP build error
//import android.hardware.Camera.MetadataCallback;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.ShutterCallback;
import android.hardware.Camera.Size;
import android.hardware.Camera.StereoCameraDataCallback;
import android.hardware.Camera.StereoCameraWarningCallback;

import com.android.camera.CameraManager;

import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.StereoDataCallback;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.util.Log;

import junit.framework.Assert;

import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

/**
 * this class is used for interactive with camera manager{here is
 * cameraProxy},so can send the command to framework
 *
 */
public class CameraDeviceImpl implements ICameraDevice {
    private static final String TAG = "CameraDeviceImpl";

    private CameraManager.CameraProxy mCameraDevice;
    private ICameraDeviceExt mMyCameraDevice;

    public CameraDeviceImpl(Activity activity, ICameraDeviceExt myCameraDevice) {
        Assert.assertNotNull(activity);
        Assert.assertNotNull(myCameraDevice);

        mMyCameraDevice = myCameraDevice;
        mCameraDevice = myCameraDevice.getCameraDevice();
    }

    public int getCameraId() {
        return mMyCameraDevice.getCameraId();
    }

    @Override
    public boolean setAutoRamaCallback(PanoramaListener panoramaListener) {
        mCameraDevice.setAutoRamaCallback(panoramaListener == null ? null
                : new AutoRamaListenerImpl(panoramaListener));
        return true;
    }

    @Override
    public boolean setAutoRamaMoveCallback(PanoramaMvListener panoramaMVListener) {
        mCameraDevice.setAutoRamaMoveCallback(panoramaMVListener == null ? null
                : new AutoRamaMvListenerImpl(panoramaMVListener));
        return true;
    }

    @Override
    public boolean setAutoFocusMoveCallback(AutoFocusMvCallback autoFocusMvCallback) {
        mCameraDevice.setAutoFocusMoveCallback(autoFocusMvCallback == null ? null
                : new ContinuousFocusMovingCallback(autoFocusMvCallback));
        return true;
    }

    /**
     * set the raw image callback when capture dng.
     * @param meta the meta data callback.
     * @param raw16 the raw data callback.
     */
  //TODO: Dng BSP build error
//    public void setRawImageCallback(MetadataCallback meta, PictureCallback raw16) {
//        mCameraDevice.setRawImageCallback(meta, raw16);
//    }

    @Override
    public boolean setUncompressedImageCallback(PictureCallback pictureCallback) {
        mCameraDevice.setUncompressedImageCallback(pictureCallback);
        return true;
    }

    @Override
    public void setAsdCallback(AsdListener asdListener) {
        mCameraDevice.setAsdCallback(asdListener == null ? null : new AsdListenerImpl(asdListener));
    }

    @Override
    public boolean setcFBOrignalCallback(cFbOriginalCallback cFbOrillback) {
        mCameraDevice.setFbOriginalCallback(cFbOrillback == null ? null : new cFBCallback(
                cFbOrillback));
        return true;
    }

    @Override
    public void setStereoDataCallback(StereoDataCallback dataCallback) {
        mCameraDevice.setStereoCameraDataCallback(dataCallback == null
                ? null : new StereoCameraDataListener(dataCallback));
    }

    @Override
    public void setStereoWarningCallback(StereoWarningCallback warningCallback) {
        mCameraDevice.setStereoCameraWarningCallback(warningCallback == null ? null
                : new StereoCameraWarningListener(warningCallback));
    }

    @Override
    public void setStereoDistanceCallback(StereoDistanceCallback distanceCallback) {
        mCameraDevice.setStereoCameraDistanceCallback(distanceCallback == null ? null
                : new StereoCameraDistanceListener(distanceCallback));
    }

    @Override
    public boolean startAutoRama(int num) {
        mCameraDevice.startAutoRama(num);
        return true;
    }

    @Override
    public boolean stopAutoRama(boolean merge) {
        mCameraDevice.stopAutoRama(merge ? 1 : 0);
        return true;
    }

    @Override
    public void addCallbackBuffer(byte[] data) {
        mCameraDevice.addCallbackBuffer(data);
    }

    @Override
    public void setPreviewCallbackWithBuffer(PreviewCallback callback) {
        mCameraDevice.setPreviewCallbackWithBuffer(callback);
    }

    @Override
    public String getParameter(String key) {
        return mMyCameraDevice.getParameters().get(key);
    }

    @Override
    public void setParameter(String key, String value) {
        mMyCameraDevice.getParametersExt().set(key, value);
    }

    @Override
    public Parameters getParameters() {
        return mMyCameraDevice.getParametersExt();
    }

    public List<Size> getSupportedPreviewSizes() {
        return mMyCameraDevice.getParametersExt().getSupportedPreviewSizes();
    }

    public Size getPreviewSize() {
        return mMyCameraDevice.getParameters().getPreviewSize();
    }

    public void setPreviewSize(int width, int height) {
        mMyCameraDevice.getParametersExt().setPreviewSize(width, height);
    }

    public List<Integer> getPIPFrameRateZSDOn() {
        return mMyCameraDevice.getParametersExt().getPIPFrameRateZSDOn();
    }

    public List<Integer> getPIPFrameRateZSDOff() {
        return mMyCameraDevice.getParametersExt().getPIPFrameRateZSDOff();
    }

    public boolean isDynamicFrameRateSupported() {
        return mMyCameraDevice.getParametersExt().isDynamicFrameRateSupported();
    }

    public void enableRecordingSound(String value) {
        mMyCameraDevice.getParametersExt().enableRecordingSound(value);
    }

    public void setDynamicFrameRate(boolean toggle) {
        mMyCameraDevice.getParametersExt().setDynamicFrameRate(toggle);
    }

    @Override
    public void applyParameters() {
        mMyCameraDevice.applyParametersToServer();
    }

    @Override
    public void fetchParametersFromServer() {
        mMyCameraDevice.fetchParametersFromServer();
    }

    private class AutoRamaListenerImpl implements AutoRamaCallback {
        private PanoramaListener mListener;

        AutoRamaListenerImpl(PanoramaListener listener) {
            mListener = listener;
        }

        @Override
        public void onCapture(byte[] jpegData) {
            if (mListener != null) {
                mListener.onCapture(jpegData);
            }
        }
    }

    private class AsdListenerImpl implements AsdCallback {
        private AsdListener mListener;

        AsdListenerImpl(AsdListener listener) {
            mListener = listener;
        }

        @Override
        public void onDetected(int xy) {
            Log.d(TAG, "[onDetecte]xy:" + xy);
            if (mListener != null) {
                mListener.onDeviceCallback(xy);
            }
        }
    }

    private class ContinuousFocusMovingCallback implements AutoFocusMoveCallback {
        private AutoFocusMvCallback mListener;

        ContinuousFocusMovingCallback(AutoFocusMvCallback listener) {
            mListener = listener;
        }

        @Override
        public void onAutoFocusMoving(boolean moving, android.hardware.Camera camera) {
            Log.i(TAG, "[onAutoFocusMoving]moving = " + moving);
            if (mListener != null) {
                mListener.onAutoFocusMoving(moving, camera);
            }
        }
    }

    private class StereoCameraDataListener implements StereoCameraDataCallback {
        private StereoDataCallback mListener;

        public StereoCameraDataListener(StereoDataCallback listener) {
            mListener = listener;
        }
        @Override
        public void onJpsCapture(byte[] data) {
            Log.d(TAG, "[onJpsCapture]data:" + data);
            if (mListener != null) {
                mListener.onJpsCapture(data);
            }
        }
        @Override
        public void onMaskCapture(byte[] data) {
            Log.d(TAG, "[onMaskCapture]data:" + data);
            if (mListener != null) {
                mListener.onMaskCapture(data);
            }
        }

        @Override
        public void onDepthMapCapture(byte[] data) {
            Log.d(TAG, "[onDepthMapCapture]data:" + data);
            if (mListener != null) {
                mListener.onDepthMapCapture(data);
            }
        }
        @Override
        public void onClearImageCapture(byte[] data) {
            Log.d(TAG, "[onClearImageCapture]data:" + data);
            if (mListener != null) {
                mListener.onClearImageCapture(data);
            }
        }
        @Override
        public void onLdcCapture(byte[] data) {
            Log.d(TAG, "[onLdcCapture]data:" + data);
            if (mListener != null) {
                mListener.onLdcCapture(data);
            }
        }
        @Override
        public void onN3dCapture(byte[] data) {
            Log.d(TAG, "[onN3dCapture]data:" + data);
            if (mListener != null) {
                mListener.onN3dCapture(data);
            }
        }
        @Override
        public void onDepthWrapperCapture(byte[] data) {
            Log.d(TAG, "[onDepthWrapperCapture]data:" + data);
            if (mListener != null) {
                mListener.onDepthWrapperCapture(data);
            }
        }
    }

    private class StereoCameraWarningListener implements StereoCameraWarningCallback {
        private StereoWarningCallback mListener;

        public StereoCameraWarningListener(StereoWarningCallback listener) {
            mListener = listener;
        }
        @Override
        public void onWarning(int type) {
            Log.d(TAG, "[onWarning]type:" + type);
            if (mListener != null) {
                mListener.onWarning(type);
            }
        }
    }

    private class StereoCameraDistanceListener implements DistanceInfoCallback {
        private StereoDistanceCallback mListener;

        public StereoCameraDistanceListener(StereoDistanceCallback listener) {
            mListener = listener;
        }
        @Override
        public void onInfo(String info) {
            Log.d(TAG, "[onInfo]info:" + info);
            if (mListener != null) {
                mListener.onInfo(info);
            }
        }
    }

    private class AutoRamaMvListenerImpl implements AutoRamaMoveCallback {
        private PanoramaMvListener mListener;

        AutoRamaMvListenerImpl(PanoramaMvListener listener) {
            mListener = listener;
        }

        @Override
        public void onFrame(int xy, int direction) {
            Log.d(TAG, "[onFrame]xy:" + xy + ",direction:" + direction);
            if (mListener != null) {
                mListener.onFrame(xy, direction);
            }
        }
    }

    @Override
    public Camera getCamera() {
        return mCameraDevice.getCamera().getInstance();
    }

    @Override
    public void lock() {
        mCameraDevice.lock();
    }

    @Override
    public void unlock() {
        mCameraDevice.unlock();
    }

    @Override
    public void startPreview() {
        mCameraDevice.startPreviewAsync();
    }

    @Override
    public void stopPreview() {
        mCameraDevice.stopPreview();
    }

    @Override
    public void autoFocus(AutoFocusCallback autoFocusCallback) {
        mCameraDevice.autoFocus(autoFocusCallback);
    }

    @Override
    public void takePicture(ShutterCallback shutterCallback, PictureCallback rawPictureCallback,
            PictureCallback postViewPictureCallback, PictureCallback jpegPictureCallback) {
        mCameraDevice.takePicture(shutterCallback, rawPictureCallback, postViewPictureCallback,
                jpegPictureCallback);
    }

    @Override
    public void takePictureAsync(ShutterCallback shutterCallback,
            PictureCallback rawPictureCallback, PictureCallback postViewPictureCallback,
            PictureCallback jpegPictureCallback) {
        mCameraDevice.takePictureAsync(shutterCallback, rawPictureCallback,
                postViewPictureCallback, jpegPictureCallback);
    }

    @Override
    public void cancelAutoFocus() {
        mCameraDevice.cancelAutoFocus();
    }

    @Override
    public void setPreviewTexture(final SurfaceTexture surfaceTexture) {
        mCameraDevice.setPreviewTextureAsync(surfaceTexture);
    }

    @Override
    public void setDisplayOrientation(int degrees) {
        mCameraDevice.setDisplayOrientation(degrees);
    }

    @Override
    public void lockParametersRun(Runnable runnable) {
        boolean lockedParameters = false;
        try {
            lockParameters();
            lockedParameters = true;
            runnable.run();
        } catch (InterruptedException ex) {
            Log.e(TAG, "lockParametersRun() not successfull.", ex);
        } finally {
            if (lockedParameters) {
                unlockParameters();
            }
        }
    }

    private ReentrantLock mLock = new ReentrantLock();

    private void lockParameters() throws InterruptedException {
        Log.d(TAG, "lockParameters: grabbing lock", new Throwable());
        mLock.lock();
        Log.d(TAG, "lockParameters: grabbed lock");
    }

    private void unlockParameters() {
        Log.d(TAG, "lockParameters: releasing lock");
        mLock.unlock();
    }

    @Override
    public void setContinuousShotSpeed(int speed) {
        mCameraDevice.setContinuousShotSpeed(speed);
    }

    @Override
    public void cancelContinuousShot() {
        mCameraDevice.cancelContinuousShot();
    }

    @Override
    public void setContinuousShotCallback(ContinuousShotListener csDoneCallback) {
        ContinuousShotListenerImpl mListener = null;
        if (csDoneCallback != null) {
            mListener = new ContinuousShotListenerImpl(csDoneCallback);
        }
        mCameraDevice.setContinuousShotCallback(mListener);
    }

    private class ContinuousShotListenerImpl implements ContinuousShotCallback {
        private ContinuousShotListener mCallback;

        public ContinuousShotListenerImpl(ContinuousShotListener csDoneCallback) {
            mCallback = csDoneCallback;
        }

        @Override
        public void onConinuousShotDone(int capNum) {
            if (mCallback != null) {
                mCallback.onConinuousShotDone(capNum);
            }
        }
    }

    private class cFBCallback implements FbOriginalCallback {

        cFbOriginalCallback mFbOriginalCallback;
        public cFBCallback(cFbOriginalCallback callback) {
            mFbOriginalCallback = callback;
        }
        @Override
        public void onCapture(byte[] originJpegData) {
            if (mFbOriginalCallback != null) {
                mFbOriginalCallback.onOriginalCallback(originJpegData);
            }
        }

    }
}
