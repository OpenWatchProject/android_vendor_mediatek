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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.mediatek.dcfdecoder;

import static android.system.OsConstants.SEEK_SET;
import static android.system.OsConstants.SEEK_CUR;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

import android.annotation.ProductApi;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.os.MemoryFile;
import android.os.SystemProperties;
import android.system.ErrnoException;
import android.system.Os;
import android.util.Log;

import libcore.io.IoBridge;

public class DcfDecoder {
    private static final String TAG = "DRM/DcfDecoder";
    private static boolean sIsOmaDrmEnabled = false;

    /// M: header buffer size, skia will read a header to sniff available image format,
    /// try to get this header back from native when decode drm image due to some inputstream
    /// can only read one time.
    private static final int HEADER_BUFFER_SIZE = 128; // byte
    private static final int DECODE_THUMBNAIL_FLAG = 0x100; // 256
    private static final int THUMBNAIL_TARGET_SIZE = 96; // 96x96 pixel
    /**
     * Action for different decode operation, decode full image will consume rights, other
     * operations will not consume rights, and only with operation decode bound can
     * get decrypt data when rights invalid.
     */
    private static final int ACTION_DECODE_FULL_IMAGE = 0;
    private static final int ACTION_JUST_DECODE_BOUND = 1;
    private static final int ACTION_JUST_DECODE_THUMBNAIL = 2;

    static {
        sIsOmaDrmEnabled = true;
        // SystemProperties.getBoolean("ro.vendor.mtk_oma_drm_support", false);
        if (sIsOmaDrmEnabled) {
            System.loadLibrary("dcfdecoderjni");
        }
    }

    @ProductApi
    public DcfDecoder() {}

    /**
     * force decrypt dcf file
     * @param pathName complete path name for the file to be decoded.
     * @param consume True to consume rights, false not.
     * @return return clear data of the dcf file
     * @internal
     */
    @ProductApi
    public byte[] forceDecryptFile(String pathName, boolean consume) {
        if (null == pathName) {
            Log.e(TAG, "forceDecryptFile: find null file name!");
            return null;
        }
        return nativeForceDecryptFile(pathName, consume);
    }

    /// M: Add to support decode drm image file, include OMA DRM(.dcf) and CTA5(.mdup) {@
    /**
     * M: Decode drm image if need, it will be called from BitmapFactory when decode image as
     * normal file failed. We will try to decode it as drm image again.
     *
     * @param header The header return from skia which skia read to sniff it's really format.
     * @param is The input stream that holds the raw data to be decoded into a bitmap.
     * @param opts null-ok; Options that control downsampling and whether the
     *             image should be completely decoded, or just is size returned.
     * @return The decoded bitmap, or null if the image data could not be
     *         decoded, or, if opts is non-null, if opts requested only the
     *         size be returned (in opts.outWidth and opts.outHeight)
     *
     * @hide
     */
    public static Bitmap decodeDrmImageIfNeeded(byte[] header, InputStream left, Options opts) {

        Bitmap bpRtn;

        try {
            Log.d(TAG, "decodeDrmImageIfNeeded with stream left [" + left.available() + "]");

            bpRtn = decodeDrmImageIfNeeded(((FileInputStream) left).getFD(), opts);
        } catch (IOException e) {
            Log.e(TAG, "decodeDrmImageIfNeeded stream caught IOException ");
            return null;
        } catch (ClassCastException e) {
            Log.e(TAG, "decodeDrmImageIfNeeded stream caught ClassCastException ");
            return null;
        }
        return bpRtn;
    }

/*
    public static Bitmap decodeDrmImageIfNeeded(byte[] header, InputStream left, Options opts) {
        if (!sIsOmaDrmEnabled) {
            return null;
        }
        // If just decode for width and height and decode success, just return.
        if (opts != null && opts.inJustDecodeBounds && opts.outWidth > 0 && opts.outHeight > 0) {
            return null;
        }

        Log.d(TAG, "decodeDrmImageIfNeeded with stream");
        if (header == null) {
            return null;
        }
        // Read enough buffer(HEADER_BUFFER_SIZE), BitmapFactory native return buffer size may not
        // enough(M is 64 bytes and N is 30 bytes), we need read more buffer from inputstream.
        // first two byte in header is real header size. {@
        int headerSize = header[0] & 0xff;
        headerSize |= (header[1] << 8) & 0xff00;
        Log.d(TAG, "decodeDrmImageIfNeeded: [" + header[0] + "][" + header[1] + "][" + header[2]
                + "][" + header[3] + "][" + header[4] + "]");
        Log.d(TAG, "decodeDrmImageIfNeeded: headerSize = " + headerSize);
        if (headerSize < HEADER_BUFFER_SIZE) {
            byte[] headerEnough = new byte[HEADER_BUFFER_SIZE];
            System.arraycopy(header, 2, headerEnough, 0, headerSize);
            try {
                left.read(headerEnough, headerSize, HEADER_BUFFER_SIZE - headerSize);
            } catch (IOException e) {
                Log.e(TAG, "decodeDrmImageIfNeeded read header with ", e);
                return null;
            }
            header = headerEnough;
        }
        // @}
        if (!isDrmFile(header)) {
            return null;
        }
        Bitmap bm = null;
        MemoryFile ashemem = null;
        try {
            if (left instanceof FileInputStream) {
                bm = decodeDrmImage(((FileInputStream) left).getFD(), 0, opts);
            } else {
                int leftDataLen = left.available();
                ashemem = new MemoryFile("drm_image", HEADER_BUFFER_SIZE + leftDataLen);
                ashemem.writeBytes(header, 0, 0, HEADER_BUFFER_SIZE);
                byte[] leftData = new byte[leftDataLen];
                left.read(leftData);
                ashemem.writeBytes(leftData, 0, HEADER_BUFFER_SIZE, leftDataLen);
                bm = decodeDrmImage(ashemem.getFileDescriptor(), ashemem.length(), opts);
            }
        } catch (IOException e) {
            Log.e(TAG, "decodeDrmImageIfNeeded with ", e);
        } finally {
            if (ashemem != null) {
                ashemem.close();
            }
        }
        return bm;
    }
*/

    /**
     * M: Decode drm image if need, it will be called from BitmapFactory when decode image as
     * normal file failed. We will try to decode it as drm image again.
     *
     * @param fd The file descriptor containing the bitmap data to decode
     * @param opts null-ok; Options that control downsampling and whether the
     *             image should be completely decoded, or just is size returned.
     * @return The decoded bitmap, or null if the image data could not be
     *         decoded, or, if opts is non-null, if opts requested only the
     *         size be returned (in opts.outWidth and opts.outHeight)
     *
     * @hide
     */
    public static Bitmap decodeDrmImageIfNeeded(FileDescriptor fd, Options opts) {
        if (!sIsOmaDrmEnabled) {
            return null;
        }
        // If just decode for width and height and decode success, just return.
        if (opts != null && opts.inJustDecodeBounds && opts.outWidth > 0 && opts.outHeight > 0) {
            return null;
        }
        Log.d(TAG, "decodeDrmImageIfNeeded with fd");
        // Because CTS will check file offset after decode, we need store old offset of fd
        // and restore after finishing decode.
        long offset = -1;
        try {
            offset = Os.lseek(fd, 0, SEEK_CUR);  // get current offset
//            Os.lseek(fd, 0, SEEK_SET); // seek to beginning to read buffer
//            byte[] header = new byte[HEADER_BUFFER_SIZE];
//            int readSize = IoBridge.read(fd, header, 0, HEADER_BUFFER_SIZE);
//          Log.e(TAG, "decodeDrmImageIfNeeded readheader [" + header + "]");

//            if (readSize == HEADER_BUFFER_SIZE && isDrmFile(header)) {
                return decodeDrmImage(fd, 0, opts);
//            }
        } catch (ErrnoException errno) {
            Log.e(TAG, "decodeDrmImageIfNeeded seek fd to beginning with ", errno);
            // catch (IOException e) {
            // Log.e(TAG, "decodeDrmImageIfNeeded get header with ", e);
        // }
        } finally {
            if (offset != -1) {
                try {
                    Os.lseek(fd, offset, SEEK_SET);
                } catch (ErrnoException errno1) {
                    Log.e(TAG, "decodeDrmImageIfNeeded seek fd to initial offset with ", errno1);
                }
            }
        }
        return null;
    }

    /**
     * M: Decode drm image if need, it will be called from BitmapFactory when decode image as
     * normal file failed. We will try to decode it as drm image again.
     *
     * @param data byte array of compressed image data
     * @param opts null-ok; Options that control downsampling and whether the
     *             image should be completely decoded, or just is size returned.
     * @return The decoded bitmap, or null if the image data could not be
     *         decoded, or, if opts is non-null, if opts requested only the
     *         size be returned (in opts.outWidth and opts.outHeight)
     *
     * @hide
     */
    public static Bitmap decodeDrmImageIfNeeded(byte[] data, Options opts) {
        if (!sIsOmaDrmEnabled) {
            return null;
        }
        // If just decode for width and height and decode success, just return.
        if (opts != null && opts.inJustDecodeBounds && opts.outWidth > 0 && opts.outHeight > 0) {
            return null;
        }

        Log.d(TAG, "decodeDrmImageIfNeeded with data");
        if (!isDrmFile(data)) {
            return null;
        }
        Bitmap bm = null;
        MemoryFile ashemem = null;
        try {
            ashemem = new MemoryFile("drm_image", data.length);
            ashemem.writeBytes(data, 0, 0, data.length);
            bm = decodeDrmImage(ashemem.getFileDescriptor(), ashemem.length(), opts);
        } catch (IOException e) {
            Log.e(TAG, "decodeDrmImageIfNeeded with ", e);
        } finally {
            if (ashemem != null) {
                ashemem.close();
            }
        }
        return bm;
    }

    /**
     * M: Decode drm image, we will decrypt it first and decode as a bitmap. Each time to decode
     * drm file will consume rights, when consume all rights, it could not decode drm image and
     * will return null. Only when decode drm image as thumbnail will not consume rights and will
     * return a 96x96 bitmap as thumbnail. Please set a flag in options as below to mark decode
     * as thumbnail:<br>
     * <p>opts.inSampleSize |= 0x100; // mark decode as thumbnail<br>
     *
     * @param fd The file descriptor containing the bitmap data to decode
     * @param opts null-ok; Options that control downsampling and whether the
     *             image should be completely decoded, or just is size returned.
     * @return The decoded bitmap, or null if the image data could not be
     *         decoded, or, if opts is non-null, if opts requested only the
     *         size be returned (in opts.outWidth and opts.outHeight)
     */
    private static Bitmap decodeDrmImage(FileDescriptor fd, int size, Options opts) {
        Bitmap bm = null;
        int action = ACTION_DECODE_FULL_IMAGE;
        // Only when in below case don't need consume rights
        // 1. only get width and height(inJustDecodeBounds is true)
        // 2. mark a flag in inSampleSize(0x100) to decode as thumbnail
        // 3. TODO decode low quality of high quality image, inSampleSize will be set bigger than
        // 1, and need limit output bitmap scale. ***don't add it now, need do more test***
        if (opts != null) {
            if (opts.inJustDecodeBounds) {
                action = ACTION_JUST_DECODE_BOUND;
            } else if ((opts.inSampleSize & DECODE_THUMBNAIL_FLAG) > 0) {
                action = ACTION_JUST_DECODE_THUMBNAIL;
            }
        }

        byte[] clearData = nativeDecryptDcfFile(fd, size, action);
        if (clearData == null) {
            Log.e(TAG, "decodeDrmImage native decrypt failed ");
            return null;
        }
        // If decode as if decode as thumbnail, we need scale output bitmap to 96x96
        if (action == ACTION_JUST_DECODE_THUMBNAIL) {
            BitmapFactory.Options thumbnailOpts = new BitmapFactory.Options();
            thumbnailOpts.inJustDecodeBounds = true;
            BitmapFactory.decodeByteArray(clearData, 0, clearData.length, thumbnailOpts);
            // Calculate available inSampleSize for thumbnail, use min sample size to scale
            int height = thumbnailOpts.outHeight;
            int width = thumbnailOpts.outWidth;
            int sampleSizeWidth = width / THUMBNAIL_TARGET_SIZE;
            int sampleSizeHeight = height / THUMBNAIL_TARGET_SIZE;
            opts.inSampleSize = Math.min(sampleSizeWidth, sampleSizeHeight);
        }
        bm = BitmapFactory.decodeByteArray(clearData, 0, clearData.length, opts);
        return bm;
    }

    /**
     * M: check the give file header whether is a drm file, include OMA DRM and CTA5.
     *
     * @param header file header with 64 bytes
     * @return if it is drm file, return true, otherwise false.
     *
     * @hide
     */
    private static boolean isDrmFile(byte[] header) {
        if (header == null || header.length < HEADER_BUFFER_SIZE) {
            return false;
        }
        // 1. CTA5
        // Structure: CTA5MM(8byte)
        final int ctaMagicLen = 8; // byte
        final String ctaMagic = "CTA5";
        String magic = new String(header, 0, ctaMagicLen);
        if (magic != null && magic.startsWith(ctaMagic)) {
            Log.d(TAG, "isDrmFile: this is a cta5 file: " + magic);
            return true;
        }
        // 2. OMA DRM
        // Structure: version(1), content type length(1), content uri lenght(1),
        //            content type(<128), content uri(<128)
        int version = header[0];
        if (version != 1) {
            Log.d(TAG, "isDrmFile: version is not dcf version 1, no oma drm file");
            return false;
        }
        int contentTypeLen = header[1];
        int contentUriLen = header[2];
        if (contentTypeLen <= 0 || (contentTypeLen + 3) > HEADER_BUFFER_SIZE
                || contentUriLen <= 0 || contentUriLen > HEADER_BUFFER_SIZE) {
            Log.d(TAG, "isDrmFile: content type or uri len invalid, not oma drm file, contentType["
                    + contentTypeLen + "] contentUri[" + contentUriLen + "]");
            return false;
        }
        String contentType = new String(header, 3, contentTypeLen);
        if (contentType == null || !contentType.contains("/")) {
            Log.d(TAG, "isDrmFile: content type not right, not oma drm file");
            return false;
        }
        Log.d(TAG, "this is a oma drm file: " + contentType);
        return true;
    }
/// @}

    // native method
    private native byte[] nativeForceDecryptFile(String pathName, boolean consume);
    private static native byte[] nativeDecryptDcfFile(FileDescriptor fd, int size, int action);
}
