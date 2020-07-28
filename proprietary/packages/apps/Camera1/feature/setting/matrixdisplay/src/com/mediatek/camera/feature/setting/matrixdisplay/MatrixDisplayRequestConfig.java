/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.matrixdisplay;

import android.graphics.ImageFormat;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * Matrix display capture request configure.
 */

public class MatrixDisplayRequestConfig implements ICameraSetting.ICaptureRequestConfigure,
        IMatrixDisplayConfig {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(MatrixDisplayRequestConfig.class.getSimpleName());

    private static final int MAX_IMAGES = 3;

    private ISettingManager.SettingDevice2Requester mDevice2Requester;
    private ValueInitializedListener mValueInitializedListener;
    private IPreviewFrameCallback mCallback;
    private ImageReader mImageReader;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private Handler mHandler;

    private String mCurrentEffect;
    private boolean mDisplayOpened = false;
    private byte[][] mPreviewBuffers = new byte[MAX_IMAGES][];
    private int mCursor = 0;

    /**
     * Color effect enum value.
     */
    enum ModeEnum {
        NONE(0),
        MONO(1),
        NEGATIVE(2),
        SOLARIZE(3),
        SEPIA(4),
        POSTERIZE(5),
        WHITEBOARD(6),
        BLACKBOARD(7),
        AQUA(8),

        NASHVILLE(11),
        HEFE(12),
        VALENCIA(13),
        XPROLL(14),
        LOFI(15),
        SIERRA(16),
        WALDEN(18);

        private int mValue = 0;
        ModeEnum(int value) {
            this.mValue = value;
        }

        /**
         * Get enum value which is in integer.
         *
         * @return The enum value.
         */
        public int getValue() {
            return this.mValue;
        }

        /**
         * Get enum name which is in string.
         *
         * @return The enum name.
         */
        public String getName() {
            return this.toString();
        }
    }

    /**
     * Listener to listen value initialized.
     */
    interface ValueInitializedListener {
        /**
         * Callback when value is initialized.
         *
         * @param supportedEffects The supported color effects.
         * @param defaultEffect The color effect default value.
         * @param supportedPreviewSizes The supported preview sizes.
         */
        void onValueInitialized(List<String> supportedEffects, String defaultEffect,
                                List<String> supportedPreviewSizes);
    }

    /**
     * Capture request configure constructor.
     *
     * @param key The key of matrix display.
     * @param device2Requester The instance of {@link ISettingManager.SettingDevice2Requester}.
     * @param listener The instance of {@link ValueInitializedListener}.
     */
    public MatrixDisplayRequestConfig(String key,
                                      ISettingManager.SettingDevice2Requester device2Requester,
                                      ValueInitializedListener listener) {
        mDevice2Requester = device2Requester;
        mValueInitializedListener = listener;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        int[] effects = characteristics.get(CameraCharacteristics.CONTROL_AVAILABLE_EFFECTS);
        List<String> supportedEffects = convertEnumToString(effects);
        String defaultEffect = ModeEnum.NONE.getName().toLowerCase();

        StreamConfigurationMap s = characteristics.get(
                CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        Size[] sizes = s.getOutputSizes(SurfaceHolder.class);
        List<Size> supportedPreviewSize = new ArrayList<Size>(sizes.length);
        for (Size size : sizes) {
            supportedPreviewSize.add(size);
        }
        sortSizeInDescending(supportedPreviewSize);
        List<String> supportedPreviewSizeInStr = sizeToStr(supportedPreviewSize);

        mValueInitializedListener.onValueInitialized(supportedEffects,
                defaultEffect, supportedPreviewSizeInStr);
    }

    @Override
    public synchronized void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        LogHelper.d(TAG, "[configCaptureRequest], mCurrentEffect:" + mCurrentEffect
                + ", mDisplayOpened:" + mDisplayOpened);
        captureBuilder.set(CaptureRequest.CONTROL_EFFECT_MODE,
                convertStringToEnum(mCurrentEffect));
        if (mDisplayOpened) {
            captureBuilder.addTarget(mImageReader.getSurface());
        }
    }

    @Override
    public synchronized void configSessionSurface(List<Surface> surfaces) {
        LogHelper.d(TAG, "[configSessionSurface], mDisplayOpened:" + mDisplayOpened);
        if (mDisplayOpened) {
            surfaces.add(mImageReader.getSurface());
        }
    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;
    }

    @Override
    public void sendSettingChangeRequest() {
        mDevice2Requester.requestRestartSession();
    }

    /**
     * Set preview frame callback to receive preview frame.
     *
     * @param callback The instance of {@link IPreviewFrameCallback}.
     */
    public void setPreviewFrameCallback(IPreviewFrameCallback callback) {
        mCallback = callback;
    }


    @Override
    public void setPreviewSize(int width, int height) {
        mPreviewWidth = width;
        mPreviewHeight = height;
    }

    @Override
    public synchronized void setDisplayStatus(boolean opened) {
        mDisplayOpened = opened;
        if (opened) {
            boolean newImageReader = false;
            if (mImageReader == null) {
                newImageReader = true;
            } else if (mImageReader.getWidth() != mPreviewWidth
                    || mImageReader.getHeight() != mPreviewHeight) {
                mImageReader.close();
                newImageReader = true;
                mHandler.getLooper().quitSafely();
            }
            if (newImageReader) {
                mImageReader = ImageReader.newInstance(mPreviewWidth,
                        mPreviewHeight, ImageFormat.YV12, MAX_IMAGES);
                HandlerThread ht = new HandlerThread("MatrixDisplay-ImageReader Handler Thread");
                ht.start();
                mHandler = new Handler(ht.getLooper());
                mImageReader.setOnImageAvailableListener(mImageAvailableListener, mHandler);
                for (int i = 0; i < MAX_IMAGES; i++) {
                    mPreviewBuffers[i] = new byte[mPreviewWidth
                            * mPreviewHeight
                            * ImageFormat.getBitsPerPixel(ImageFormat.YV12) / 8];
                }
            }
        } else {
            for (int i = 0; i < MAX_IMAGES; i++) {
                mPreviewBuffers[i] = null;
            }
            mImageReader.setOnImageAvailableListener(null,null);
            mImageReader.close();
            mImageReader = null;
            mHandler.getLooper().quitSafely();
        }
    }

    @Override
    public void setSelectedEffect(String effect) {
        mCurrentEffect = effect;
    }

    private ImageReader.OnImageAvailableListener mImageAvailableListener
            = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader imageReader) {
            byte[] data = null;
            synchronized (MatrixDisplayRequestConfig.this) {
                Image image = imageReader.acquireNextImage();
                if (image == null) {
                    LogHelper.d(TAG, "[onImageAvailable] acquireNextImage return false, return");
                    return;
                }
                Image.Plane[] planes = image.getPlanes();
                int rowStride;
                int pixelStride;
                int width = image.getWidth();
                int height = image.getHeight();
                int format = image.getFormat();
                int offset = 0;
                if (mPreviewBuffers[mCursor] == null) {
                    LogHelper.d(TAG, "[onImageAvailable] mPreviewBuffers[mCursor] is null, return");
                    image.close();
                    return;
                }
                data = mPreviewBuffers[mCursor];
                int maxRowSize = planes[0].getRowStride();
                for (int i = 0; i < planes.length; i++) {
                    if (maxRowSize < planes[i].getRowStride()) {
                        maxRowSize = planes[i].getRowStride();
                    }
                }
                byte[] rowData = new byte[maxRowSize];
                ByteBuffer buffer = null;
                for (int i = 0; i < planes.length; i++) {
                    buffer = planes[i].getBuffer();

                    rowStride = planes[i].getRowStride();
                    pixelStride = planes[i].getPixelStride();
                    // For multi-planar yuv images, assuming yuv420 with 2x2 chroma subsampling.
                    int w = (i == 0) ? width : width / 2;
                    int h = (i == 0) ? height : height / 2;
                    int bytesPerPixel = ImageFormat.getBitsPerPixel(format) / 8;
                    if (format == ImageFormat.YUV_420_888) {
                        // need to swap YUV to YVU.
                        int length = w * h;
                        if (i == 0) {
                            buffer.get(data, offset, length);
                        } else if (i == 1) {
                            buffer.get(data, offset + length, length);
                        } else if (i == 2) {
                            buffer.get(data, offset - length, length);
                        }
                        offset += length;
                    } else if (format == ImageFormat.YV12) {
                        for (int row = 0; row < h; row++) {

                            int length;
                            if (pixelStride == bytesPerPixel) {
                                // Special case: optimized read of the entire row
                                length = w * bytesPerPixel;
                                buffer.get(data, offset, length);
                                offset += length;
                            } else {
                                // Generic case: should work for any pixelStride but slower.
                                // Use intermediate buffer to avoid read byte-by-byte from
                                // DirectByteBuffer, which is very bad for performance
                                length = (w - 1) * pixelStride + bytesPerPixel;
                                buffer.get(rowData, 0, length);
                                for (int col = 0; col < w; col++) {
                                    data[offset++] = rowData[col * pixelStride];
                                }
                            }
                            // Advance buffer the remainder of the row stride
                            if (row < h - 1) {
                                buffer.position(buffer.position() + rowStride - length);
                            }
                        }
                    }

                    buffer.rewind();
                }
                image.close();
            }

            if (mCallback != null) {
                mCallback.onPreviewFrameAvailable(data);
            }
            mCursor = (mCursor++) % MAX_IMAGES;
        }
    };

    private List<String> convertEnumToString(int[] enumIndexs) {
        ModeEnum[] modes = ModeEnum.values();
        List<String> names = new ArrayList<>(enumIndexs.length);
        for (int i = 0; i < enumIndexs.length; i++) {
            int enumIndex = enumIndexs[i];
            for (ModeEnum mode : modes) {
                if (mode.getValue() == enumIndex) {
                    String name = mode.getName().replace('_', '-').toLowerCase();
                    names.add(name);
                    break;
                }
            }
        }
        return names;
    }

    private String convertEnumToString(int enumIndex) {
        String name = null;
        ModeEnum[] modes = ModeEnum.values();
        for (ModeEnum mode : modes) {
            if (mode.getValue() == enumIndex) {
                name = mode.getName().replace('_', '-').toLowerCase();
                break;
            }
        }
        return name;
    }

    private int convertStringToEnum(String value) {
        int enumIndex = 0;
        ModeEnum[] modes = ModeEnum.values();
        for (ModeEnum mode : modes) {
            String modeName = mode.getName().replace('_', '-').toLowerCase();
            if (modeName.equalsIgnoreCase(value)) {
                enumIndex = mode.getValue();
                break;
            }
        }
        return enumIndex;
    }

    private List<String> sizeToStr(List<Size> sizes) {
        List<String> sizeInStr = new ArrayList<>(sizes.size());
        for (Size size : sizes) {
            sizeInStr.add(size.getWidth() + "x" + size.getHeight());
        }
        return sizeInStr;
    }

    private void sortSizeInDescending(List<Size> sizes) {
        for (int i = 0; i < sizes.size(); i++) {
            Size maxSize = sizes.get(i);
            int maxIndex = i;
            for (int j = i + 1; j < sizes.size(); j++) {
                Size tempSize = sizes.get(j);
                if (tempSize.getWidth() * tempSize.getHeight()
                        > maxSize.getWidth() * maxSize.getHeight()) {
                    maxSize = tempSize;
                    maxIndex = j;
                }
            }
            Size firstSize = sizes.get(i);
            sizes.set(i, maxSize);
            sizes.set(maxIndex, firstSize);
        }
    }
}
