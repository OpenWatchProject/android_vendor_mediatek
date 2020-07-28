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
 * MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.camera.v2.stream;


/**
 * This class used for wrapper the image reader.such as the image data, image
 * width,image height,and format will be packaged to ImageInfo.
 */
public class ImageInfo {

    private byte[] mData;
    private int mWidth;
    private int mHeigth;
    private int mFormat;

    /**
     * Create a ImageInfo when get an image from imageReader.
     * @param data
     *            the byte length of image data.
     * @param widht
     *            the width of image.
     * @param height
     *            the height of the image.
     * @param format
     *            the format of the image.
     */
    public ImageInfo(
            byte[] data, int widht, int height, int format) {
        mData = data;
        mWidth = widht;
        mHeigth = height;
        mFormat = format;
    }

    /**
     * The data of the image.
     * @return the byte of the image.
     */
    public byte[] getData() {
        return mData;
    }

    /**
     * The default width of {@link Image Images}, in pixels.
     * <p>
     * The width may be overridden by the producer sending buffers to this
     * ImageReader's Surface. If so, the actual width of the images can be found
     * using {@link Image#getWidth}.
     * </p>
     *
     * @return the expected width of an Image
     */
    public int getWidth() {
        return mWidth;
    }

    /**
     * The default height of {@link Image Images}, in pixels.
     * <p>
     * The height may be overridden by the producer sending buffers to this
     * ImageReader's Surface. If so, the actual height of the images can be
     * found using {@link Image#getHeight}.
     * </p>
     * @return the expected height of an Image
     */
    public int getHeigth() {
        return mHeigth;
    }

    /**
     * The default {@link ImageFormat image format} of {@link Image Images}.
     * <p>
     * Some color formats may be overridden by the producer sending buffers to
     * this ImageReader's Surface if the default color format allows.
     * ImageReader guarantees that all {@link Image Images} acquired from
     * ImageReader (for example, with {@link #acquireNextImage}) will have a
     * "compatible" format to what was specified in {@link #newInstance}. As of
     * now, each format is only compatible to itself. The actual format of the
     * images can be found using {@link Image#getFormat}.
     * </p>
     * @return the expected format of an Image
     * @see ImageFormat
     */
    public int getFormat() {
        return mFormat;
    }
}
