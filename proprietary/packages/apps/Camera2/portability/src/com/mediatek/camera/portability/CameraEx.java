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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.portability;

import android.hardware.Camera;
import android.hardware.Camera.AFDataCallback;
import android.hardware.Camera.AsdCallback;
import android.hardware.Camera.AutoRamaCallback;
import android.hardware.Camera.AutoRamaMoveCallback;
import android.hardware.Camera.ContinuousShotCallback;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.StereoCameraDataCallback;
import android.hardware.Camera.StereoCameraWarningCallback;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureResult;
import android.os.Build;
import android.os.Message;
import android.util.Log;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

/**
 * This class packages the APIs which are added by MTK in class {@link Camera}
 * in MTK platform.
 */
public class CameraEx {

    /**
     * Camera HAL device API version 1.0.
     */
    public static final int CAMERA_HAL_API_VERSION_1_0 = 0x100;

    private static final int API_21 = 21;
    private static final int API_22 = 22;
    private static final int API_28 = 28;

    // Command
    private static final int MTK_CAMERA_COMMAND_CANCLE_CONTINUOUS_SHOT = 0x1000000D;
    private static final int MTK_CAMERA_COMMAND_CONTINUOUS_SHOT_SPEED = 0x1000000E;
    private static final int MTK_CAMERA_COMMAND_ENABLE_DNG = 0x10000001;
    //start autorama
    private static final int MTK_CAMERA_COMMAND_START_AUTORAMA = 0x10000009;
    //stop autorama
    private static final int MTK_CAMERA_COMMAND_STOP_AUTORAMA = 0x1000000A;


    // Callback Message
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END = 0x00000006;
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONE = 0x00000016;
    private static final int MTK_CAMERA_MSG_EXT_DATA_RAW16 = 0x00000013;
    //autorama
    private static final int MTK_CAMERA_MSG_EXT_DATA_AUTORAMA = 0x00000001;
    //ASD
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_ASD = 0x00000002;
    // AF Window Results
    private static final int MTK_CAMERA_MSG_EXT_DATA_AF = 0x00000020;
    // notify for Image before compress when taking capture
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_IMAGE_UNCOMPRESSED = 0x00000017;

    // Stereo message
    //Stereo Camera JPS
    private static final int MTK_CAMERA_MSG_EXT_DATA_JPS                = 0x00000011;
    //Stereo Debug Data
    //int[0]: data type.
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_DBG         = 0x00000012;
    //Stereo Camera Depth Map Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHMAP    = 0x00000014;
    //Stereo Camera Clear Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE = 0x00000015;
    //Stereo Camera LDC Datacfghip
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_LDC         = 0x00000016;
    //Stereo Camera n3d Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_N3D         = 0x00000019;
    //Stereo Camera depth wrapper Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHWRAPPER = 0x00000020;
    // Stereo Feature: warning message
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_STEREO_WARNING   = 0x00000014;

    private static Method sCameraSetPropertyMethod =
            ReflectUtil.getMethod(Camera.class, "setProperty", String.class, String.class);
    private static Method sCameraGetPropertyMethod =
            ReflectUtil.getMethod(Camera.class, "getProperty", String.class, String.class);
    private static Method sSetUncompressedImageCallbackMethod =
            ReflectUtil.getMethod(Camera.class, "setUncompressedImageCallback",
                                                PictureCallback.class);

    private VendorDataCallback mVendorDataCallback;

    /**
     * Vendor data callback interface used to supply
     * vendor-defined data from a camera native.
     */
    public interface VendorDataCallback {
        /**
         * Called when vendor-defined data is available.
         *
         * @param message message object from native.
         */
        void onDataTaken(Message message);

        /**
         * Called when vendor-defined data is available.
         *
         * @param msgId The id of message which contains the callback info.
         * @param data  Callback data.
         * @param arg1  The first argument.
         * @param arg2  The second argument.
         */
        void onDataCallback(int msgId, byte[] data, int arg1, int arg2);
    }

    /**
     * Open Camera device.
     *
     * @param cameraId   The hardware camera to access, between 0 and
     *                   {@link #getNumberOfCameras()}-1.
     * @param halVersion The HAL API version this camera device to be opened as.
     * @return a new Camera object, connected, locked and ready for use.
     */
    public static Camera openLegacy(int cameraId, int halVersion) {
        if (Build.VERSION.SDK_INT > API_21 && Build.VERSION.SDK_INT < API_28) {
            try {
                return Camera.openLegacy(cameraId, halVersion);
            } catch (RuntimeException e) {
                // some non-mtk device may not implemented this,
                // this function call will cause RuntimeException.
                // always open legacy when api1 and throw exception.
                Log.e("CamAp_CameraEx", "[openLegacy] exception:" + e);
                throw e;
            }
        } else {
            return Camera.open(cameraId);
        }
    }

    /**
     * Set Property.
     *
     * @param key the given key to set.
     * @param val the value set to the key.
     */
    public static void setProperty(String key, String val) {
        if (sCameraSetPropertyMethod != null) {
            ReflectUtil.callMethodOnObject(null, sCameraSetPropertyMethod, key, val);
        }
    }

    /**
     * Get Property.
     *
     * @param key the given key to set.
     * @param val the value set to the key.
     * @return the property value.
     */
    public static String getProperty(String key, String val) {
        if (sCameraGetPropertyMethod != null) {
            return (String)ReflectUtil.callMethodOnObject(null, sCameraGetPropertyMethod, key, val);
        } else {
            return val;
        }
    }

    /**
     * Send command to camera hardware.
     *
     * @param camera  The instance of {@link Camera}.
     * @param command The command index.
     * @param arg0    The first argument.
     * @param arg1    The second argument.
     */
    public void sendCommand(Camera camera, int command, int arg0, int arg1) {
        switch (command) {
            case MTK_CAMERA_COMMAND_CANCLE_CONTINUOUS_SHOT:
                camera.cancelContinuousShot();
                break;

            case MTK_CAMERA_COMMAND_CONTINUOUS_SHOT_SPEED:
                camera.setContinuousShotSpeed(arg0);
                break;

            case MTK_CAMERA_COMMAND_ENABLE_DNG:
                enableRaw16(camera, arg0 == 1);
                break;

            case MTK_CAMERA_COMMAND_START_AUTORAMA:
                camera.startAutoRama(arg0);
                break;

            case MTK_CAMERA_COMMAND_STOP_AUTORAMA:
                camera.stopAutoRama(arg0);
                break;

            default:
                break;
        }
    }

    /**
     * Set vendor data callback to review data.
     *
     * @param camera   The instance of {@link Camera}.
     * @param msgId    The id of message which contains the callback info.
     * @param callback The instance of {@link VendorDataCallback}.
     */
    public void setVendorDataCallback(Camera camera, int msgId,
                                             VendorDataCallback callback) {
        mVendorDataCallback = callback;
        switch (msgId) {
            case MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END:
                camera.setContinuousShotCallback(new ContinuousShotCallbackImpl());
                break;

            case MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONE:
                setDngCallback(camera);
                break;

            case MTK_CAMERA_MSG_EXT_DATA_AUTORAMA:
                camera.setAutoRamaCallback(new AutoRamaCallbackImpl());
                camera.setAutoRamaMoveCallback(new AutoRamaMoveCallbackImpl());
                break;

            case MTK_CAMERA_MSG_EXT_DATA_AF:
                camera.setAFDataCallback(new AFDataCallbackImpl());
                break;

            case MTK_CAMERA_MSG_EXT_NOTIFY_ASD:
                camera.setAsdCallback(new AsdCallbackImpl());
                break;

            case MTK_CAMERA_MSG_EXT_NOTIFY_IMAGE_UNCOMPRESSED:
                if (Build.VERSION.SDK_INT > API_22 && sSetUncompressedImageCallbackMethod != null) {
                    ReflectUtil.callMethodOnObject(
                            camera,
                            sSetUncompressedImageCallbackMethod,
                            new UncompressedImageCallbackImpl());
                }
                break;

            case MTK_CAMERA_MSG_EXT_NOTIFY_STEREO_WARNING:
                camera.setStereoCameraWarningCallback(new StereoWarningCallbackImpl());
                break;
            case MTK_CAMERA_MSG_EXT_DATA_JPS:
                camera.setStereoCameraDataCallback(new StereoDatasCallbackImpl());
                break;

            default:
                break;
        }
    }

    /**
     * Continuous shot callback implementer.
     */
    private class ContinuousShotCallbackImpl implements ContinuousShotCallback {

        @Override
        public void onConinuousShotDone(int capNum) {
            mVendorDataCallback.onDataCallback(
                    MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END, null, capNum, 0);
        }
    }

    private void setDngCallback(Camera camera) {
        try {
            Class metadataInterfazz = Class.forName("android.hardware.Camera$MetadataCallback");
            ClassLoader loader = metadataInterfazz.getClassLoader();
            Object metadataInterfazzInstance = Proxy.newProxyInstance(
                    loader,
                    new Class<?>[]{metadataInterfazz},
                    new InvocationHandler() {
                        @Override
                        public Object invoke(Object proxy, Method method, Object[] args)
                                throws Throwable {
                            if (method.getName().equals("onMetadataReceived")) {
                                //callback
                                Log.d("CamAp_CameraEx", "onMetadataReceived");
                                Message msg = new Message();
                                msg.arg1 = MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONE;
                                msg.obj = new MessageInfo(args[0], args[1]);
                                mVendorDataCallback.onDataTaken(msg);
                                return 1;
                            }
                            return -1;
                        }
                    });

            Method setDngCallbackMethod =
                    ReflectUtil.getMethod(Camera.class, "setRaw16Callback", metadataInterfazz,
                            PictureCallback.class);
            Log.d("CamAp_CameraEx", "setDngCallback: " + setDngCallbackMethod);
            ReflectUtil.callMethodOnObject(camera, setDngCallbackMethod,
                    metadataInterfazzInstance, new DngPictureCallbackImpl());
        } catch (Exception e) {
            Log.e("CamAp_CameraEx", "metadata not defined");
        }
    }

    private void enableRaw16(Camera camera, boolean isEnabled) {
        try {
            Method enableRaw16Method =
                    ReflectUtil.getMethod(Camera.class, "enableRaw16", boolean.class);
            ReflectUtil.callMethodOnObject(camera, enableRaw16Method, isEnabled);
        } catch (Exception e) {
            Log.e("CamAp_CameraEx", "enableRaw16 not defined");
        }
    }

    /**
     * Dng picture callback implementer.
     */
    private class DngPictureCallbackImpl implements PictureCallback {

        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_RAW16, data, 0, 0);
        }
    }

    /**
     * Autorama callback implementer.
     */
    private class AutoRamaCallbackImpl implements AutoRamaCallback {

        @Override
        public void onCapture(byte[] jpegData) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_AUTORAMA, jpegData, 0, -1);
        }
    }

    /**
     * Autorama move callback implementer.
     */
    private class AutoRamaMoveCallbackImpl implements AutoRamaMoveCallback {

        @Override
        public void onFrame(int xy, int direction) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_AUTORAMA, null, xy,
                    direction);
        }

    }

    /**
     * AF data callback implementer.
     */
    private class AFDataCallbackImpl implements AFDataCallback {

        @Override
        public void onAFData(byte[] data, Camera camera) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_AF, data, -1, -1);
        }
    }

    /**
     * Auto scene detection implementer.
     */
    private class AsdCallbackImpl implements AsdCallback {

        @Override
        public void onDetected(int scene) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_ASD, null, scene,
                    0);
        }
    }

    /**
     * Uncompressed image callback implementer.
     */
    private class UncompressedImageCallbackImpl implements PictureCallback {

        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_IMAGE_UNCOMPRESSED,
                    data, 0, 0);
        }
    }

    /**
     * Stereo warning callback implementer.
     */
    private class StereoWarningCallbackImpl implements StereoCameraWarningCallback {

        @Override
        public void onWarning(int type) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_NOTIFY_STEREO_WARNING,
                    null, type, 0);
        }
    }

    /**
     * Stereo datas callback implementer.
     */
    private class StereoDatasCallbackImpl implements StereoCameraDataCallback {

        @Override
        public void onJpsCapture(byte[] jpsData) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_JPS,
                    jpsData, MTK_CAMERA_MSG_EXT_DATA_JPS, 0);
        }

        @Override
        public void onMaskCapture(byte[] maskData) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_JPS,
                    maskData, MTK_CAMERA_MSG_EXT_DATA_STEREO_DBG, 0);
        }

        @Override
        public void onDepthMapCapture(byte[] depthMapData) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_JPS,
                    depthMapData, MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHMAP, 0);
        }

        @Override
        public void onClearImageCapture(byte[] clearImageData) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_JPS,
                    clearImageData, MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE, 0);
        }

        @Override
        public void onLdcCapture(byte[] ldcData) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_JPS,
                    ldcData, MTK_CAMERA_MSG_EXT_DATA_STEREO_LDC, 0);
        }

        @Override
        public void onN3dCapture(byte[] n3dData) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_JPS,
                    n3dData, MTK_CAMERA_MSG_EXT_DATA_STEREO_N3D, 0);
        }

        @Override
        public void onDepthWrapperCapture(byte[] depthWrapper) {
            mVendorDataCallback.onDataCallback(MTK_CAMERA_MSG_EXT_DATA_JPS,
                    depthWrapper, MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHWRAPPER, 0);
        }
    }

    /**
     * This class is used for wrapper the message info.
     */
    public class MessageInfo {
        public final Object mArg1;
        public final Object mArg2;

        MessageInfo(Object arg1, Object arg2) {
            mArg1 = arg1;
            mArg2 = arg2;
        }
    }
}
