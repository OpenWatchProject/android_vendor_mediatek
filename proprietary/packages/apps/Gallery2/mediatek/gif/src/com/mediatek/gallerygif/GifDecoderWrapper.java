/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.gallerygif;

import android.graphics.Bitmap;

import com.bumptech.glide.gifdecoder.GifDecoder;
import com.bumptech.glide.gifdecoder.GifHeaderParser;
import com.bumptech.glide.gifdecoder.GifHeader;
import com.mediatek.gallerybasic.util.DebugUtils;
import com.mediatek.gallerybasic.util.Log;
import com.mediatek.gallerybasic.util.Utils;
import com.mediatek.galleryportable.SystemPropertyUtils;

import java.io.ByteArrayOutputStream;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.BufferUnderflowException;

/**
 * Wrap glide gif decoder.
 */
public class GifDecoderWrapper {
    private static final String TAG = "MtkGallery2/GifDecoderWrapper";
    public static final int INVALID_VALUE = -1;
    // The minimum delay time of every frame is 10ms
    public static final int MINIMUM_DURATION = 10;

    private static final int BUFFER_SIZE = 16 * 1024;
    private static final boolean ENABLE_DEBUG = SystemPropertyUtils.get("gifdecoder.debug")
            .equals("1");
    private GifDecoder mGifDecoder;
    private final static long MAX_GIF_FRAME_PIXEL_SIZE = (long) (1.5f * 1024 * 1024); // 1.5MB

    private GifDecoderWrapper(GifDecoder gifDecoder) {
        mGifDecoder = gifDecoder;
    }

    /**
     * Create GifDecoderWrapper with file path.
     * @param filePath
     *            gif path
     * @return GifDecoderWrapper
     */
    public static GifDecoderWrapper createGifDecoderWrapper(String filePath) {
        if (filePath == null) {
            return null;
        }
        FileInputStream inputStream = null;
        try {
            Log.d(TAG, "<createGifDecoderWrapper> filePath: " + filePath);
            inputStream = new FileInputStream(filePath);
            byte[] buffer = inputStreamToBytes(inputStream);
            if (buffer == null || buffer.length <= 0) {
                return null;
            }
            GifDecoder gifDecoder = new GifDecoder(new GalleryBitmapProvider());
            GifHeader header = new GifHeaderParser().setData(buffer).parseHeader();
            long gifFrameSize = (long)header.getWidth() * (long)header.getHeight();
            if (gifFrameSize > MAX_GIF_FRAME_PIXEL_SIZE) {
                Log.d(TAG, "<createGifDecoderWrapper> Gif size larger than MAX_GIF_FRAME_PIXEL_SIZE");
                return null;
            }
            gifDecoder.setData(header, buffer);
            return new GifDecoderWrapper(gifDecoder);
        } catch (FileNotFoundException e) {
            Log.e(TAG, "<createGifDecoderWrapper> FileNotFoundException", e);
            return null;
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "<createGifDecoderWrapper> IllegalArgumentException", e);
            return null;
        } catch (BufferUnderflowException e) {
            Log.e(TAG, "<createGifDecoderWrapper> BufferUnderflowException", e);
            return null;
        } finally {
            Utils.closeSilently(inputStream);
        }
    }

    /**
     * Create GifDecoderWrapper from buffer.
     * @param buffer
     *            gif buffer
     * @param offset
     *            buffer offset
     * @param length
     *            buffer length
     * @return GifDecoderWrapper
     */
    public static GifDecoderWrapper createGifDecoderWrapper(byte[] buffer, int offset, int length) {
        if (buffer == null || buffer.length <= 0) {
            return null;
        }
        try {
            Log.d(TAG, "<createGifDecoderWrapper> buffer: " + buffer);
            GifDecoder gifDecoder = new GifDecoder(new GalleryBitmapProvider());
            GifHeader header = new GifHeaderParser().setData(buffer).parseHeader();
            long gifFrameSize = (long)header.getWidth() * (long)header.getHeight();
            if (gifFrameSize > MAX_GIF_FRAME_PIXEL_SIZE) {
                Log.d(TAG, "<createGifDecoderWrapper> Gif size larger than MAX_GIF_FRAME_PIXEL_SIZE");
                return null;
            }
            gifDecoder.setData(header, buffer);
            return new GifDecoderWrapper(gifDecoder);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "<createGifDecoderWrapper> IllegalArgumentException", e);
            return null;
        } catch (BufferUnderflowException e) {
            Log.e(TAG, "<createGifDecoderWrapper> BufferUnderflowException", e);
            return null;
        }
    }

    /**
     * Create GifDecoderWrapper from stream.
     * @param is
     *            input stream
     * @return GifDecoderWrapper
     */
    public static GifDecoderWrapper createGifDecoderWrapper(InputStream is) {
        if (is == null) {
            return null;
        }
        byte[] buffer = inputStreamToBytes(is);
        if (buffer == null || buffer.length <= 0) {
            return null;
        }
        try {
            Log.d(TAG, "<createGifDecoderWrapper> is: " + is);
            GifDecoder gifDecoder = new GifDecoder(new GalleryBitmapProvider());
            GifHeader header = new GifHeaderParser().setData(buffer).parseHeader();
            long gifFrameSize = (long)header.getWidth() * (long)header.getHeight();
            if (gifFrameSize > MAX_GIF_FRAME_PIXEL_SIZE) {
                Log.d(TAG, "<createGifDecoderWrapper> Gif size larger than MAX_GIF_FRAME_PIXEL_SIZE");
                return null;
            }
            gifDecoder.setData(header, buffer);
            return new GifDecoderWrapper(gifDecoder);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "<createGifDecoderWrapper> IllegalArgumentException", e);
            return null;
        } catch (BufferUnderflowException e) {
            Log.e(TAG, "<createGifDecoderWrapper> BufferUnderflowException", e);
            return null;
        }
    }

    /**
     * Create GifDecoderWrapper from file descriptor.
     * @param fd
     *            file descriptor
     * @return GifDecoderWrapper
     */
    public static GifDecoderWrapper createGifDecoderWrapper(FileDescriptor fd) {
        if (fd == null) {
            return null;
        }
        FileInputStream is = null;
        try {
            is = new FileInputStream(fd);
            byte[] buffer = inputStreamToBytes(is);
            if (buffer == null || buffer.length <= 0) {
                return null;
            }
            Log.d(TAG, "<createGifDecoderWrapper> fd: " + fd);
            GifDecoder gifDecoder = new GifDecoder(new GalleryBitmapProvider());
            GifHeader header = new GifHeaderParser().setData(buffer).parseHeader();
            long gifFrameSize = (long)header.getWidth() * (long)header.getHeight();
            if (gifFrameSize > MAX_GIF_FRAME_PIXEL_SIZE) {
                Log.d(TAG, "<createGifDecoderWrapper> Gif size larger than MAX_GIF_FRAME_PIXEL_SIZE");
                return null;
            }
            gifDecoder.setData(header, buffer);
            return new GifDecoderWrapper(gifDecoder);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "<createGifDecoderWrapper> IllegalArgumentException", e);
            return null;
        } catch (BufferUnderflowException e) {
            Log.e(TAG, "<createGifDecoderWrapper> BufferUnderflowException", e);
            return null;
        } finally {
            Utils.closeSilently(is);
        }
    }

    /**
     * Close GifDecoderWrapper.
     */
    public void close() {
        if (null == mGifDecoder) {
            return;
        }
        Log.d(TAG, "<close>");
        mGifDecoder.clear();
        mGifDecoder = null;
    }

    /**
     * Get gif width.
     * @return gif width
     */
    public int getWidth() {
        if (null == mGifDecoder) {
            return INVALID_VALUE;
        }
        return mGifDecoder.getWidth();
    }

    /**
     * Get gif height.
     * @return gif height
     */
    public int getHeight() {
        if (null == mGifDecoder) {
            return INVALID_VALUE;
        }
        return mGifDecoder.getHeight();
    }

    /**
     * Get gif total frame count.
     * @return total frame count
     */
    public int getTotalFrameCount() {
        if (null == mGifDecoder) {
            return INVALID_VALUE;
        }
        return mGifDecoder.getFrameCount();
    }

    /**
     * Get frame duration.
     * @param frameIndex
     *            frame index
     * @return frame duration
     */
    public int getFrameDuration(int frameIndex) {
        if (null == mGifDecoder) {
            return INVALID_VALUE;
        }
        // for some special gif, the delay time is too small,
        // if that's the case, set the allowed minimum delay time as 1/100s(10ms)
        int duration = Math.max(mGifDecoder.getDelay(frameIndex), MINIMUM_DURATION);
        return duration;
    }

    /**
     * Get gif frame.
     * @param frameIndex
     *            frame index
     * @return frame bitmap
     */
    public Bitmap getFrameBitmap(int frameIndex) {
        if (null == mGifDecoder) {
            return null;
        }
        mGifDecoder.advance();
        Bitmap frameBitmap = null;
        try {
            frameBitmap = mGifDecoder.getNextFrame();
        } catch (IllegalArgumentException e) {
            Log.d(TAG, "<getFrameBitmap> IllegalArgumentException:"  + e);
            return null;
        }
        if (ENABLE_DEBUG) {
            DebugUtils.dumpBitmap(frameBitmap, frameIndex + "");
        }
        return frameBitmap;
    }

    private static byte[] inputStreamToBytes(InputStream is) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream(BUFFER_SIZE);
        try {
            int count;
            byte[] data = new byte[BUFFER_SIZE];
            while ((count = is.read(data)) != -1) {
                buffer.write(data, 0, count);
            }
            buffer.flush();
            return buffer.toByteArray();
        } catch (IOException e) {
            Log.d(TAG, "<inputStreamToBytes>", e);
            return null;
        }
    }

    /**
     * Implement GifDecoder.BitmapProvider.
     */
    private static class GalleryBitmapProvider implements GifDecoder.BitmapProvider {
        @Override
        public Bitmap obtain(int width, int height, Bitmap.Config config) {
            return null;
        }

        @Override
        public void release(Bitmap bitmap) {

        }
    }
}
