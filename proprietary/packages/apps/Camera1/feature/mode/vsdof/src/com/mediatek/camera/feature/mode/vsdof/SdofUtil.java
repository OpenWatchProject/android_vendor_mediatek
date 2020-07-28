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

package com.mediatek.camera.feature.mode.vsdof;

import android.text.TextUtils;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.utils.Size;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.List;

/**
 * Collection of utility functions related to camera.
 */
public final class SdofUtil {
    private static final Tag TAG = new Tag(SdofUtil.class.getSimpleName());
    private static final int UNKNOWN = -1;

    private static final double RATIO_16_9 = 16d / 9;
    private static final double RATIO_5_3 = 5d / 3;
    private static final double RATIO_3_2 = 3d / 2;
    private static final double RATIO_4_3 = 4d / 3;
    private static final double RATIOS[] = { RATIO_16_9, RATIO_5_3, RATIO_3_2, RATIO_4_3 };

    private static final String RATIO_16_9_IN_STRING = "(16:9)";
    private static final String RATIO_5_3_IN_STRING = "(5:3)";
    private static final String RATIO_3_2_IN_STRING = "(3:2)";
    private static final String RATIO_4_3_IN_STRING = "(4:3)";
    private static final String RATIOS_IN_STRING[] = { RATIO_16_9_IN_STRING, RATIO_5_3_IN_STRING,
            RATIO_3_2_IN_STRING, RATIO_4_3_IN_STRING };

    private static final double ASPECT_TOLERANCE = 0.02;
    private static final String QVGA = "320x240"; //(4:3)
    private static final String WQVGA = "400x240"; //(5:3)
    private static final String VGA = "640x480"; //(4:3)
    private static final String WVGA = "800x480"; //(5:3)
    private static final String SVGA = "800x600"; //(4:3)

    private static DecimalFormat sFormat = new DecimalFormat("##0");

    /**
     * Build size to string.
     *
     * @param size the size want to express with string.
     * @return an string to express the size.
     */
    public static String buildSize(Size size) {
        if (size != null) {
            return "" + size.getWidth() + "x" + size.getHeight();
        } else {
            return "null";
        }
    }

    /**
     * Get size from string.
     *
     * @param sizeString the size use string express.
     * @return the size from string.
     */
    public static Size getSize(String sizeString) {
        Size size = null;
        int index = sizeString.indexOf('x');
        if (index != UNKNOWN) {
            int width = Integer.parseInt(sizeString.substring(0, index));
            int height = Integer.parseInt(sizeString.substring(index + 1));
            size = new Size(width, height);
        }
        LogHelper.d(TAG, "getSize(" + sizeString + ") return " + size);
        return size;
    }

    /**
     * Splits a comma delimited string to an ArrayList of Size.
     * @param str the picture size values
     * @return Return null if the passing string is null or the size is 0.
     */
    public static ArrayList<Size> splitSize(String str) {
        if (str == null) {
            return null;
        }

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<Size> sizeList = new ArrayList<Size>();
        for (String s : splitter) {
            Size size = strToSize(s);
            if (size != null) {
                sizeList.add(size);
            }
        }
        if (sizeList.size() == 0) {
            return null;
        }
        return sizeList;
    }

    /**
     *Sort the sizes in descending.
     * @param sizes the supported sizes.
     */
    public static void sortSizeInDescending(List<Size> sizes) {
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

    /**
     * Trans size to str.
     * @param sizes the supported sizes.
     * @return the string of supported sizes.
     */
    public static List<String> sizeToStr(List<Size> sizes) {
        List<String> sizeInStr = new ArrayList<>(sizes.size());
        for (Size size : sizes) {
            sizeInStr.add(size.getWidth() + "x" + size.getHeight());
        }
        return sizeInStr;
    }

    /**
     * Get pixels and ratio of the input picture size value.
     *
     * @param value The picture size value, such as "1920x1080";
     * @return The string contains of pixels and ratio, such as "2M(16:9)".
     */
    public static String getPixelsAndRatio(String value) {
        Size size = valueToSize(value);
        double ratio = (double) size.getWidth() / (double) size.getHeight();
        int index = -1;
        for (int i = 0; i < RATIOS.length; i++) {
            if (Math.abs(ratio - RATIOS[i]) < ASPECT_TOLERANCE) {
                index = i;
                break;
            }
        }
        if (index == -1) {
            return null;
        }

        if (size.getWidth() * size.getHeight() < 5 * 1e5) {
            if (QVGA.equals(value)) {
                return "QVGA";
            }
            if (WQVGA.equals(value)) {
                return "WQVGA";
            }
            if (VGA.equals(value)) {
                return "VGA";
            }
            if (WVGA.equals(value)) {
                return "WVGA";
            }
            if (SVGA.equals(value)) {
                return "SVGA";
            }
        }
        String ratioInString = RATIOS_IN_STRING[index];

        String pixels = sFormat.format(Math.round(size.getWidth() * size.getHeight() / 1e6));
        return  pixels + "M" + ratioInString;
    }

    private static Size valueToSize(String value) {
        int index = value.indexOf('x');
        int width = Integer.parseInt(value.substring(0, index));
        int height = Integer.parseInt(value.substring(index + 1));
        Size size = new Size(width, height);
        return size;
    }

    // Parses a string (ex: "480x320") to Size object.
    // Return null if the passing string is null.
    private static Size strToSize(String str) {
        if (str == null) {
            return null;
        }

        int pos = str.indexOf('x');
        if (pos != -1) {
            String width = str.substring(0, pos);
            String height = str.substring(pos + 1);
            return new Size(Integer.parseInt(width),
                    Integer.parseInt(height));
        }
        LogHelper.e(TAG, "Invalid size parameter string=" + str);
        return null;
    }
}
