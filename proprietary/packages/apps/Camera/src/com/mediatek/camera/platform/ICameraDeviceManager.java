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
package com.mediatek.camera.platform;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Face;
//TODO: Dng BSP build error
//import android.hardware.Camera.MetadataCallback;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.ShutterCallback;
import android.hardware.Camera.Size;

import java.util.List;

public interface ICameraDeviceManager {

    public void onCameraCloseDone();

    public int getNumberOfCameras();

    //TODO need two getCameraInfo
    public CameraInfo getCameraInfo(int cameraId);

    public CameraInfo[] getCameraInfo();

    public int getBackCameraId();

    public int getFrontCameraId();

    public int getCurrentCameraId();

    public ICameraDevice getCameraDevice(int CameraId);

    public interface ICameraDevice {
        public static final int CALLBACK_ONSHUTTER = 101;
        public static final int CALLBACK_RAWPICTURE = 102;
        public static final int CALLBACK_POSTVIEW = 103;
        public static final int CALLBACK_JPEG = 104;

        // TODO phase out
        public interface Listener {
            public void onDeviceCallback(Object... obj);
        }

        public interface AutoFocusExtCallback {
            public void onAutoFocus(boolean focused);
        }

        public interface AsdListener {
            public void onDeviceCallback(int xy);
        }

        public interface PanoramaListener {
            public void onCapture(byte[] jpegData);
        }

        public interface PanoramaMvListener {
            public void onFrame(int xy, int direction);
        }

        public interface AutoFocusMvCallback {
            void onAutoFocusMoving(boolean start, Camera camera);
        }

        public interface ContinuousShotListener {
            public void onConinuousShotDone(int capNum);
        }

        public interface OtListener {
            public void onObjectTracking(Face face, Camera camera);
        }

        /**
         * this interface is used for cFB callback
         */
        public interface cFbOriginalCallback {
            public void onOriginalCallback(byte[] data);
        }
        public interface StereoDataCallback {
            public void onJpsCapture(byte[] data);
            public void onMaskCapture(byte[] data);
            public void onDepthMapCapture(byte[] data);
            public void onClearImageCapture(byte[] data);
            /**
             * set the ldc callback when capture sDof.
             * @param data the ldc data callback.
             */
            public void onLdcCapture(byte[] data);
            /**
             * set the N3d debug callback when capture sDof.
             * @param data the n3d data callback.
             */
            public void onN3dCapture(byte[] data);
            /**
             * set the depth wrapper callback when capture sDof.
             * @param data the n3d data callback.
             */
            public void onDepthWrapperCapture(byte[] data);
        }

        public interface StereoWarningCallback {
            public void onWarning(int type);
        }

        public interface StereoDistanceCallback {
            public void onInfo(String info);
        }

        public void setStereoDataCallback(StereoDataCallback dataCallback);

        public void setStereoWarningCallback(StereoWarningCallback warningCallback);

        public void setStereoDistanceCallback(StereoDistanceCallback distanceCallback);

        public void startPreview();

        public void stopPreview();

        /**
         * set the raw image callback when capture dng.
         * @param meta the meta data callback.
         * @param raw16 the raw data callback.
         */
        //TODO: Dng BSP build error
//        public void setRawImageCallback(MetadataCallback meta, PictureCallback raw16);

        public boolean setAutoRamaCallback(PanoramaListener panoramaCallback);

        public boolean setAutoRamaMoveCallback(PanoramaMvListener panoramaMVCallback);

        public boolean setAutoFocusMoveCallback(AutoFocusMvCallback autoFocusMvCallback);

        public boolean setUncompressedImageCallback(PictureCallback pictureCallback);

        public boolean setcFBOrignalCallback(cFbOriginalCallback cfbCallback);

        public boolean startAutoRama(int num);

        public boolean stopAutoRama(boolean merge);

        public void setAsdCallback(AsdListener AsdCallback);

        public void setContinuousShotSpeed(int speed);

        public void cancelContinuousShot();

        public void setContinuousShotCallback(ContinuousShotListener csDoneCallback);

        /**
         * get a special key's parameter
         *
         * @param type
         *            which type of parameter need want to get
         * @return the type of parameter's value
         */
        public String getParameter(String key);

        public int getCameraId();

        /**
         * get the current camera's parameters
         *
         * @return all the parameters of current camera
         */
        public Parameters getParameters();

        public List<Size> getSupportedPreviewSizes();

        public void setPreviewSize(int width, int height);

        public Size getPreviewSize();

        public void setParameter(String key, String value);

        /**
         * set the parameters to camera client
         *
         * @param parameters
         *            which need to apply
         */
        public void applyParameters();

        public void fetchParametersFromServer();
        public List<Integer> getPIPFrameRateZSDOn();

        public List<Integer> getPIPFrameRateZSDOff();

        public void setDynamicFrameRate(boolean toggle);

        public boolean isDynamicFrameRateSupported();

        public void enableRecordingSound(String value);

        public Camera getCamera();

        public void takePicture(ShutterCallback shutterCallback,
                PictureCallback rawPictureCallback, PictureCallback postViewPictureCallback,
                PictureCallback jpegPictureCallback);

        public void takePictureAsync(ShutterCallback shutterCallback,
                PictureCallback rawPictureCallback, PictureCallback postViewPictureCallback,
                PictureCallback jpegPictureCallback);

        void unlock();

        public void lock();

        public void cancelAutoFocus();

        public void autoFocus(AutoFocusCallback autoFocusCallback);

        public void setPreviewTexture(final SurfaceTexture surfaceTexture);

        public void setDisplayOrientation(int degree);

        public void lockParametersRun(Runnable runnable);

        public void addCallbackBuffer(byte[] data);

        public void setPreviewCallbackWithBuffer(PreviewCallback callback);
    }
}
