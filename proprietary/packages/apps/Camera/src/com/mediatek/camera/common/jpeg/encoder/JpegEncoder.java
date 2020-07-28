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

package com.mediatek.camera.common.jpeg.encoder;

import android.content.Context;
import android.view.Surface;

/**
 * An abstract class of jpeg encoder, it may encode jpeg by software
 * or hardware.
 */
public abstract class JpegEncoder {
    /**
     *  The callback used to notify jpeg arrives.
     */
    public interface JpegCallback {
        /**
         * Notify jpeg arrives.
         * @param jpegData the byte array of jpeg data.
         */
        void onJpegAvailable(byte[] jpegData);
    }
    /**
     * Judge whether support Effect HAL or not.
     * @param context the context.
     * @return true if support;otherwise is false.
     */
    public static boolean isHwEncoderSupported(Context context) {
        return true;
    }

    /**
     * New an instance if JpegEncoder, HwJpegEncodeImpl or SwJpegEncodeImp.
     * @param context the context used JpegEncoder.
     * @param useHwEncoder choose whether use hardware jpeg encoder.
     * @return an instance of JpegEncoder.
     */
    public static JpegEncoder newInstance(Context context, boolean useHwEncoder) {
        if (useHwEncoder) {
            return new HwJpegEncodeImpl(context);
        } else {
            return new SwJpegEncodeImpl();
        }
    }

    /**
     * Get the supported jpeg encode formats.
     * @return an int array of supported formats.
     */
    public abstract int[] getSupportedInputFormats();

    /**
     * Config input surface used to pass raw data to Jpeg Encoder.
     * @param outputSurface the surface used to receive jpeg data.
     * @param width jpeg's width
     * @param height jpeg's height
     * @param format the used raw buffer format.
     * @return an input surface to receive raw data.
     */
    public Surface configInputSurface(Surface outputSurface,
            int width, int height, int format) {
        return null;
    }

    /**
     * Config input surface used to pass raw data to Jpeg Encoder.
     *
     * @param jpegCallback the callback used to receive jpeg data.
     * @param width jpeg's width
     * @param height jpeg's height
     * @param format the used raw buffer format.
     * @return an input surface to receive raw data.
     */
    public Surface configInputSurface(JpegCallback jpegCallback,
            int width, int height, int format) {
        return null;
    }

    /**
     * start encode.
     */
    public abstract void startEncode();

    /**
     * start encoder, and release encoder when encode done.
     */
    public abstract void startEncodeAndReleaseWhenDown();

    /**
     * release encode.
     */
    public abstract void release();
}