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
package com.mediatek.camera.feature.setting.picturesize;

import android.content.Context;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.WindowManager;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * Picture size helper to provide util methods.
 */
public class PictureSizeHelper {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(PictureSizeHelper.class.getSimpleName());

    public static final double RATIO_16_9 = 16d / 9;
    public static final double RATIO_5_3 = 5d / 3;
    public static final double RATIO_3_2 = 3d / 2;
    public static final double RATIO_4_3 = 4d / 3;
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
    private static List<Double> sDesiredAspectRatios = new ArrayList<>();
    private static List<String> sDesiredAspectRatiosInStr = new ArrayList<>();
    private static double sDegressiveRatio = 0;
    private static int sMaxCount = 0;

    /**
     * Compute full screen aspect ratio.
     *
     * @param context The instance of {@link Context}.
     * @return The full screen aspect ratio.
     */
    public static double findFullScreenRatio(Context context) {
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        Display display = wm.getDefaultDisplay();
        DisplayMetrics dm = new DisplayMetrics();
        display.getRealMetrics(dm);
        int width = Math.max(dm.widthPixels, dm.heightPixels);
        int height = Math.min(dm.widthPixels, dm.heightPixels);
        double displayRatio = (double) width / (double) height;

        double find = RATIO_4_3;
        for (int i = 0; i < RATIOS.length; i++) {
            double ratio = RATIOS[i];
            if (Math.abs(ratio - displayRatio) < Math.abs(find - displayRatio)) {
                find = ratio;
            }
        }
        return find;
    }

    /**
     * Set the aspect ratios want to show.
     *
     * @param desiredAspectRatios The desired aspect ratios.
     */
    public static void setDesiredAspectRatios(List<Double> desiredAspectRatios) {
        sDesiredAspectRatios.clear();
        sDesiredAspectRatiosInStr.clear();

        if (desiredAspectRatios != null) {
            sDesiredAspectRatios.addAll(desiredAspectRatios);
        }

        for (int i = 0; i < sDesiredAspectRatios.size(); i++) {
            double ratio = sDesiredAspectRatios.get(i);
            String ratioInString = null;
            for (int j = 0; j < RATIOS.length; j++) {
                if (ratio == RATIOS[j]) {
                    ratioInString = RATIOS_IN_STRING[j];
                    break;
                }
            }
            sDesiredAspectRatiosInStr.add(ratioInString);
        }
    }

    /**
     * Set picture size filter parameters.
     *
     * @param degressiveRatio Picture sizes after filter should decrease according to the input
     *                        ratio.
     * @param maxCount The max count of picture sizes under one aspect ratio.
     */
    public static void setFilterParameters(double degressiveRatio, int maxCount) {
        sDegressiveRatio = degressiveRatio;
        sMaxCount = maxCount;
    }

    /**
     * Filter picture sizes with the the filter parameters.
     *
     * @param sizes The input picture sizes.
     * @return Picture sizes after filter.
     */
    public static List<String> filterSizes(List<String> sizes) {
        // create buckets.
        Map<Double, ResolutionBucket> buckets = new HashMap<>();
        for (double ratio : sDesiredAspectRatios) {
            ResolutionBucket bucket = new ResolutionBucket();
            bucket.aspectRatio = ratio;
            buckets.put(ratio, bucket);
        }
        // classify sizes to buckets.
        for (String value : sizes) {
            Size size = valueToSize(value);
            double ratio = (double) size.width / (double) size.height;
            int index = -1;
            for (int i = 0; i < sDesiredAspectRatios.size(); i++) {
                if (Math.abs(ratio - sDesiredAspectRatios.get(i)) < ASPECT_TOLERANCE) {
                    index = i;
                    break;
                }
            }
            if (index >= 0) {
                ResolutionBucket bucket = buckets.get(sDesiredAspectRatios.get(index));
                bucket.add(size);
            }
        }
        // filter sizes from buckets.
        List<Size> sizesAfterFilter = new LinkedList<>();
        for (double ratio : sDesiredAspectRatios) {
            ResolutionBucket bucket = buckets.get(ratio);
            if (bucket.sizes.size() == 0) {
                continue;
            }
            List<Size> bucketSizes = pickUpToThree(bucket.sizes);
            for (Size size : bucketSizes) {
                int index = -1;
                for (int i = 0; i < sizesAfterFilter.size(); i++) {
                    if (area(size) >= area(sizesAfterFilter.get(i))) {
                        index = i;
                        break;
                    }
                }
                if (index == -1) {
                    index = sizesAfterFilter.size();
                }
                sizesAfterFilter.add(index, size);
            }
        }

        List<String> sizesInstr = new ArrayList<>();
        for (Size size : sizesAfterFilter) {
            sizesInstr.add(sizeToStr(size));
        }
        return sizesInstr;
    }

    /**
     * Get pixels and ratio of the input picture size value.
     *
     * @param value The picture size value, such as "1920x1080";
     * @return The string contains of pixels and ratio, such as "2M(16:9)".
     */
    public static String getPixelsAndRatio(String value) {
        Size size = valueToSize(value);
        double ratio = (double) size.width / (double) size.height;
        int index = -1;
        for (int i = 0; i < sDesiredAspectRatios.size(); i++) {
            if (Math.abs(ratio - sDesiredAspectRatios.get(i)) < ASPECT_TOLERANCE) {
                index = i;
                break;
            }
        }
        if (index == -1) {
            return null;
        }

        if (size.width * size.height < 5 * 1e5) {
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
        String ratioInString = sDesiredAspectRatiosInStr.get(index);

        String pixels = sFormat.format(Math.round(size.width * size.height / 1e6));
        return  pixels + "M" + ratioInString;
    }

    /**
     * Get the aspect ratio of input picture size value.
     *
     * @param value The input picture size value.
     * @return The aspect ratio of input picture size value.
     */
    public static double getStandardAspectRatio(String value) {
        Size size = valueToSize(value);
        double ratio = (double) size.width / (double) size.height;
        for (int i = 0; i < sDesiredAspectRatios.size(); i++) {
            double standardRatio = sDesiredAspectRatios.get(i);
            if (Math.abs(ratio - standardRatio) < ASPECT_TOLERANCE) {
                return standardRatio;
            }
        }
        return ratio;
    }

    private static List<Size> pickUpToThree(List<Size> sizes) {
        if (sDegressiveRatio == 0 || sMaxCount == 0) {
            return sizes;
        }
        List<Size> result = new ArrayList<>();
        Size largest = sizes.get(0);
        result.add(largest);
        Size lastSize = largest;
        for (Size size : sizes) {
            double targetArea = Math.pow(sDegressiveRatio, result.size()) * area(largest);
            if (area(size) < targetArea) {
                // This candidate is smaller than half the mega pixels of the
                // last one. Let's see whether the previous size, or this size
                // is closer to the desired target.
                if (!result.contains(lastSize)
                        && (area(lastSize) - targetArea < targetArea - area(size))) {
                    result.add(lastSize);
                } else {
                    result.add(size);
                }
            }
            lastSize = size;
            if (result.size() == sMaxCount) {
                break;
            }
        }

        // If we have less than three, we can add the smallest size.
        if (result.size() < sMaxCount && !result.contains(lastSize)) {
            result.add(lastSize);
        }
        return result;
    }

    private static Size valueToSize(String value) {
        int index = value.indexOf('x');
        int width = Integer.parseInt(value.substring(0, index));
        int height = Integer.parseInt(value.substring(index + 1));
        Size size = new Size();
        size.width = width;
        size.height = height;
        return size;
    }

    private static String sizeToStr(Size size) {
        return size.width + "x" + size.height;
    }

    private static int area(Size size) {
        if (size == null) {
            return 0;
        }
        return size.width * size.height;
    }

    /**
     * Size struct.
     */
    private static class Size {
        /**
         * Width of size.
         */
        public int width;

        /**
         * Height of size.
         */
        public int height;
    }

    /**
     * Resolution bucket.
     */
    private static class ResolutionBucket {
        /**
         * The aspect ration of picture sizes in this bucket.
         */
        public double aspectRatio;
        /**
         * This is a sorted list of sizes, going from largest to smallest.
         */
        public List<Size> sizes = new LinkedList<>();

        /**
         * Use this to add a new resolution to this bucket. It will insert it
         * into the sizes array and update appropriate members.
         *
         * @param size the new size to be added
         */
        public void add(Size size) {
            sizes.add(size);
        }
    }
}
