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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.camera.feature.mode.vsdof.photo.mmsdkcallback;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.media.Image;
import android.media.ImageReader;
import android.media.ImageReader.OnImageAvailableListener;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.feature.mode.vsdof.photo.device.ISdofPhotoDeviceController;
import com.mediatek.camera.portability.SystemProperties;

import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * A mmsdk callback data handler for receiving stereo Jps/Clear image/
 * Mask/Ldc/DepthMap.
 */
public class MmsdkCallbackData {
    private static final Tag TAG = new Tag(MmsdkCallbackData.class.getSimpleName());
    private static int sIsMmsdkCallbackEnable = SystemProperties
            .getInt("camera.callback.enable", 0);
    // Stereo Camera JPS
    private static final int MTK_CAMERA_MSG_EXT_DATA_JPS = 0x00000011;
    // Stereo Debug Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_DBG = 0x00000012;
    // Stereo Camera Depth Map Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHMAP = 0x00000014;
    // Stereo Camera Clear Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE = 0x00000015;
    // Stereo Camera LDC Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_LDC = 0x00000016;
    // Stereo Camera n3d Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_N3D = 0x00000019;
    // Stereo Camera Depth wrapper
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHWRAPPER = 0x00000020;
    private static final int CAMERA_MSG_COMPRESSED_IMAGE = 0x100;
    public static final String KEY_STEREO_LDC_SIZE = "stereo-ldc-size";
    public static final String KEY_STEREO_N3D_SIZE = "stereo-n3d-size";
    public static final String KEY_STEREO_EXTRA_SIZE = "stereo-extra-size";
    public static final String KEY_STEREO_DEPTH_SIZE = "stereo-depth-size";
    public static final String KEY_PICTURE_SIZE = "picture-size";
    private static final String DEFAULT_CALLBACK_LIST = "ci,bi,mdb,mbm,mdw,ldc";
    private Camera.PictureCallback mPictureCallback;
    private CameraProxy.VendorDataCallback mExStereoDataCallback;
    private HashMap<String, String> mParamMap;
    private ImageReader mBokehImageReader;
    private ImageReader mClearImageReader;
    private ImageReader mMaskImageReader;
    private ImageReader mLdcImageReader;
    private ImageReader mN3dImageReader;
    private ImageReader mDepthMapReader;
    private ImageReader mDepthWrapperImageReader;

    // handler
    private HandlerThread mBufferHandlerThread;

    /**
     * Constructor of mmsdk callback data.
     */
    public MmsdkCallbackData() {
        LogHelper.d(TAG, "MmsdkCallbackData constructor" + this);
    }

    /**
     * Set jpeg listener.
     *
     * @param pictureCallback
     *            the picture callback.
     */
    public void setJpegListener(Camera.PictureCallback pictureCallback) {
        mPictureCallback = pictureCallback;
    }

    /**
     * Set ex stereo data callback.
     *
     * @param dataCallback
     *            the ex stereo data callback.
     */
    public void setExStereoDataCallback(CameraProxy.VendorDataCallback dataCallback) {
        mExStereoDataCallback = dataCallback;
    }

    /**
     * Release imageReader.
     */
    public void release() {
        close();
        if (mBufferHandlerThread != null && mBufferHandlerThread.isAlive()) {
            mBufferHandlerThread.quit();
            mBufferHandlerThread = null;
        }
    }

    /**
     * Get surfaces according to parameter.
     * @param bufferSizeMap bufferSizeMap
     * @param paramMap paramMap
     * @return the surface created by image reader.
     */
    public List<Surface> getSurfacesBySize(HashMap<String, String> bufferSizeMap,
                                           HashMap<String, String> paramMap, String callbackList) {
        mParamMap = paramMap;
        List<Surface> surfaces = new ArrayList<Surface>();
        int pictureSize = Integer.parseInt(bufferSizeMap.get(KEY_PICTURE_SIZE));
        int maskSize = 0;
        int ldcSize = 0;
        int n3dSize = 0;
        int depthSize = 0;
        close();
        if (mBufferHandlerThread == null) {
            mBufferHandlerThread = new HandlerThread("ImageBufferListener");
            mBufferHandlerThread.start();
        }
        Looper looper = mBufferHandlerThread.getLooper();
        if (looper == null) {
            throw new RuntimeException("why looper is null?");
        }
        Handler imageHandler = new Handler(looper);
        if (callbackList.contains(ISdofPhotoDeviceController.CLEAR_IMAGE)) {
            // add clean image surface
            mClearImageReader = ImageReader.newInstance(pictureSize, 1, ImageFormat.JPEG, 1);
            mClearImageReader.setOnImageAvailableListener(mCleanImageListener, imageHandler);
            surfaces.add(mClearImageReader.getSurface());
        }
        if (callbackList.contains(ISdofPhotoDeviceController.BOKEH_IMAGE)) {
            // add bokeh surface
            mBokehImageReader = ImageReader.newInstance(pictureSize, 1, ImageFormat.JPEG, 1);
            mBokehImageReader.setOnImageAvailableListener(mBokehListener, imageHandler);
            surfaces.add(mBokehImageReader.getSurface());
        }
        if (callbackList.contains(ISdofPhotoDeviceController.MTK_BOKEH_DEPTH)) {
            depthSize = Integer.parseInt(bufferSizeMap.get(KEY_STEREO_DEPTH_SIZE));
            // add depth map surface
            mDepthMapReader = ImageReader.newInstance(depthSize, 1,
                    ImageFormat.JPEG, 1);
            mDepthMapReader.setOnImageAvailableListener(mDepthMapListener, imageHandler);
            surfaces.add(mDepthMapReader.getSurface());
        }
        if (callbackList.contains(ISdofPhotoDeviceController.MTK_DEBUG_BUFFER)) {
            n3dSize = Integer.parseInt(bufferSizeMap.get(KEY_STEREO_N3D_SIZE));
            // add n3d surface
            mN3dImageReader = ImageReader.newInstance(n3dSize, 1, ImageFormat.JPEG, 1);
            mN3dImageReader.setOnImageAvailableListener(mN3dListener, imageHandler);
            surfaces.add(mN3dImageReader.getSurface());
        }
        if (callbackList.contains(ISdofPhotoDeviceController.MTK_BOKEH_META)) {
            maskSize = Integer.parseInt(bufferSizeMap.get(KEY_STEREO_EXTRA_SIZE));
            // add mask surface
            mMaskImageReader = ImageReader.newInstance(maskSize, 1, ImageFormat.JPEG, 1);
            mMaskImageReader.setOnImageAvailableListener(mMaskListener, imageHandler);
            surfaces.add(mMaskImageReader.getSurface());
        }
        if (callbackList.contains(ISdofPhotoDeviceController.MTK_DEPTH_WRAPPER)) {
            depthSize = Integer.parseInt(bufferSizeMap.get(KEY_STEREO_DEPTH_SIZE));
            // add depth wrapper surface
            mDepthWrapperImageReader = ImageReader.newInstance(depthSize, 1,
                    ImageFormat.JPEG, 1);
            mDepthWrapperImageReader.setOnImageAvailableListener(mDepthWrapperListener,
                    imageHandler);
            surfaces.add(mDepthWrapperImageReader.getSurface());
        }
        if (callbackList.contains(ISdofPhotoDeviceController.LDC)) {
            ldcSize = Integer.parseInt(bufferSizeMap.get(KEY_STEREO_LDC_SIZE));
            // add ldc surface
            mLdcImageReader = ImageReader.newInstance(ldcSize, 1, ImageFormat.JPEG, 1);
            mLdcImageReader.setOnImageAvailableListener(mLdcListener, imageHandler);
            surfaces.add(mLdcImageReader.getSurface());
        }
        LogHelper.d(TAG, "ldc size : " + ldcSize / 1024 + "n3d size: " + n3dSize / 1024
                + "depth map size: " + depthSize / 1024 + "extra size: " + maskSize / 1024
                + "depth wrapper size: " + depthSize / 1024
                + "picture size: " + pictureSize / 1024);
        return surfaces;
    }

    private void close() {
        LogHelper.d(TAG, "[close]");
        if (mBokehImageReader != null) {
            mBokehImageReader.close();
            mBokehImageReader = null;
        }
        if (mClearImageReader != null) {
            mClearImageReader.close();
            mClearImageReader = null;
        }
        if (mMaskImageReader != null) {
            mMaskImageReader.close();
            mMaskImageReader = null;
        }
        if (mLdcImageReader != null) {
            mLdcImageReader.close();
            mLdcImageReader = null;
        }
        if (mN3dImageReader != null) {
            mN3dImageReader.close();
            mN3dImageReader = null;
        }
        if (mDepthMapReader != null) {
            mDepthMapReader.close();
            mDepthMapReader = null;
        }
        if (mDepthWrapperImageReader != null) {
            mDepthWrapperImageReader.close();
            mDepthWrapperImageReader = null;
        }
    }

    private OnImageAvailableListener mBokehListener = new OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            if (reader != null) {
                Image image = reader.acquireNextImage();
                byte[] jpegData = acquireBytesAndClose(image);
                LogHelper.d(TAG, "[onImageAvailable] bokeh image : " + jpegData +
                        ", length = " + jpegData.length + ", reader = " + reader);
                if (sIsMmsdkCallbackEnable > 0) {
                    saveJpeg(jpegData, "/sdcard/bokeh");
                }
                if (jpegData != null) {
                    mPictureCallback.onPictureTaken(jpegData, null);
                }
            }
        }
    };

    private OnImageAvailableListener mMaskListener = new OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            if (reader != null) {
                Image image = reader.acquireNextImage();
                byte[] jpegData = acquireBytesAndClose(image);
                LogHelper.d(TAG, "[onImageAvailable] mask buffer : " + jpegData +
                        ", length = " + jpegData.length + ", reader = " + reader);
                if (sIsMmsdkCallbackEnable > 0) {
                    saveJpeg(jpegData, "/sdcard/mask");
                }
                if (jpegData != null) {
                    mExStereoDataCallback.onDataCallback(0, jpegData,
                            MTK_CAMERA_MSG_EXT_DATA_STEREO_DBG, 0);
                }
            }
        }
    };

    private OnImageAvailableListener mLdcListener = new OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            if (reader != null) {
                Image image = reader.acquireNextImage();
                byte[] jpegData = acquireBytesAndClose(image);
                LogHelper.d(TAG, "[onImageAvailable] ldc buffer : " + jpegData +
                        ", length = " + jpegData.length + ", reader = " + reader);
                if (sIsMmsdkCallbackEnable > 0) {
                    saveJpeg(jpegData, "/sdcard/ldc");
                }
                if (jpegData != null) {
                    mExStereoDataCallback.onDataCallback(0, jpegData,
                            MTK_CAMERA_MSG_EXT_DATA_STEREO_LDC, 0);
                }
            }
        }
    };

    private OnImageAvailableListener mN3dListener = new OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            if (reader != null) {
                Image image = reader.acquireNextImage();
                byte[] jpegData = acquireBytesAndClose(image);
                LogHelper.d(TAG, "[onImageAvailable] n3d buffer : " + jpegData +
                        ", length = " + jpegData.length + ", reader = " + reader);
                if (sIsMmsdkCallbackEnable > 0) {
                    saveJpeg(jpegData, "/sdcard/n3d");
                }
                if (jpegData != null) {
                    mExStereoDataCallback.onDataCallback(0, jpegData,
                            MTK_CAMERA_MSG_EXT_DATA_STEREO_N3D, 0);
                }
            }
        }
    };

    private OnImageAvailableListener mDepthMapListener = new OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            if (reader != null) {
                Image image = reader.acquireNextImage();
                byte[] jpegData = acquireBytesAndClose(image);
                LogHelper.d(TAG, "[onImageAvailable] depth map  : " + jpegData +
                        ", length = " + jpegData.length + ", reader = " + reader);
                if (sIsMmsdkCallbackEnable > 0) {
                    saveJpeg(jpegData, "/sdcard/depthmap");
                }
                if (jpegData != null) {
                    mExStereoDataCallback.onDataCallback(0, jpegData,
                            MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHMAP, 0);
                }
            }
        }
    };

    private OnImageAvailableListener mDepthWrapperListener = new OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            if (reader != null) {
                Image image = reader.acquireNextImage();
                byte[] jpegData = acquireBytesAndClose(image);
                LogHelper.d(TAG, "[onImageAvailable] depth buffer : " + jpegData +
                        ", length = " + jpegData.length + ", reader = " + reader);
                if (sIsMmsdkCallbackEnable > 0) {
                    saveJpeg(jpegData, "/sdcard/depthwrapper");
                }
                if (jpegData != null) {
                    mExStereoDataCallback.onDataCallback(0, jpegData,
                            MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHWRAPPER, 0);
                }
            }
        }
    };

    private OnImageAvailableListener mCleanImageListener = new OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            if (reader != null) {
                Image image = reader.acquireNextImage();
                byte[] jpegData = acquireBytesAndClose(image);
                LogHelper.d(TAG, "[onImageAvailable] clear image : " + jpegData +
                        ", length = " + jpegData.length + ", reader = " + reader);
                if (sIsMmsdkCallbackEnable > 0) {
                    saveJpeg(jpegData, "/sdcard/clearImage");
                }
                if (jpegData != null) {
                    mExStereoDataCallback.onDataCallback(0, jpegData,
                            MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE, 0);
                }
            }
        }
    };

    private byte[] acquireBytesAndClose(Image image) {
        ByteBuffer buffer;
        Image.Plane plane0 = image.getPlanes()[0];
        buffer = plane0.getBuffer();
        byte[] imageBytes = new byte[buffer.remaining()];
        buffer.get(imageBytes);
        buffer.rewind();
        image.close();
        return imageBytes;
    }

    private void saveJpeg(byte[] jpegData, String path) {
        LogHelper.d(TAG, "[saveJpeg]path = " + path);
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(path);
            out.write(jpegData);
            out.close();
        } catch (IOException e) {
            LogHelper.e(TAG, "[saveJpeg]Failed to write image,exception:", e);
        } finally {
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    LogHelper.e(TAG, "[saveJpeg], io exception:", e);
                }
            }
        }
    }
}