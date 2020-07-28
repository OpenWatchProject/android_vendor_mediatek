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

package com.mediatek.camera.common.utils;

/**
 * Immutable class for describing width and height dimensions in pixels.
 */
public class Size {
    /**
     * Create a new immutable Size instance.
     *
     * @param width The width of the size, in pixels
     * @param height The height of the size, in pixels
     */
    public Size(int width, int height) {
        mWidth = width;
        mHeight = height;
    }

    /**
     * Get the width of the size (in pixels).
     * @return width
     */
    public int getWidth() {
        return mWidth;
    }

    /**
     * Get the height of the size (in pixels).
     * @return height
     */
    public int getHeight() {
        return mHeight;
    }

    /**
     * Check if this size is equal to another size.
     * <p>
     * Two sizes are equal if and only if both their widths and heights are
     * equal.
     * </p>
     * <p>
     * A size object is never equal to any other type of object.
     * </p>
     *
     * @param obj the current object.
     * @return {@code true} if the objects were equal, {@code false} otherwise.
     */
    @Override
    public boolean equals(final Object obj) {
        if (obj == null) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        if (obj instanceof Size) {
            Size other = (Size) obj;
            return mWidth == other.mWidth && mHeight == other.mHeight;
        }
        return false;
    }

    /**
     * Return the size represented as a string with the format {@code "WxH"}.
     *
     * @return string representation of the size
     */
    @Override
    public String toString() {
        return mWidth + "x" + mHeight;
    }

    private static NumberFormatException invalidSize(String s) {
        throw new NumberFormatException("Invalid Size: \"" + s + "\"");
    }

    /**
     * Parses the specified string as a size value.
     * <p>
     * The ASCII characters {@code \}{@code u002a} ('*') and
     * {@code \}{@code u0078} ('x') are recognized as separators between
     * the width and height.</p>
     * <p>
     * For any {@code Size s}: {@code Size.parseSize(s.toString()).equals(s)}.
     * However, the method also handles sizes expressed in the
     * following forms:</p>
     * <p>
     * "<i>width</i>{@code x}<i>height</i>" or
     * "<i>width</i>{@code *}<i>height</i>" {@code => new Size(width, height)},
     * where <i>width</i> and <i>height</i> are string integers potentially
     * containing a sign, such as "-10", "+7" or "5".</p>
     *
     * <pre>{@code
     * Size.parseSize("3*+6").equals(new Size(3, 6)) == true
     * Size.parseSize("-3x-6").equals(new Size(-3, -6)) == true
     * Size.parseSize("4 by 3") => throws NumberFormatException
     * }</pre>
     *
     * @param string the string representation of a size value.
     * @return the size value represented by {@code string}.
     *
     * @throws NumberFormatException if {@code string} cannot be parsed
     * as a size value.
     */
    public static Size parseSize(String string)
            throws NumberFormatException {

        int index = string.indexOf('*');
        if (index < 0) {
            index = string.indexOf('x');
        }
        if (index < 0) {
            throw invalidSize(string);
        }
        try {
            return new Size(Integer.parseInt(string.substring(0, index)),
                    Integer.parseInt(string.substring(index + 1)));
        } catch (NumberFormatException e) {
            throw invalidSize(string);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int hashCode() {
        // assuming most sizes are <2^16, doing a rotate will give us perfect hashing
        return mHeight ^ ((mWidth << (Integer.SIZE / 2)) | (mWidth >>> (Integer.SIZE / 2)));
    }

    private final int mWidth;
    private final int mHeight;
}
