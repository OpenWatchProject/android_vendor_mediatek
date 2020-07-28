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

package com.mediatek.engineermode.mcfconfig;

import android.view.Gravity;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;


/**
 * A utilize class for basic file operations.
 */
public final class FileUtils {
    private static final String TAG = "McfConfig/FileUtils";
    public static final String UNIT_B = "B";
    public static final String UNIT_KB = "KB";
    public static final String UNIT_MB = "MB";
    public static final String UNIT_GB = "GB";
    public static final String UNIT_TB = "TB";
    private static final int UNIT_INTERVAL = 1024;
    private static final double ROUNDING_OFF = 0.005;
    private static final int DECIMAL_NUMBER = 100;

    public static final String SEPARATOR = "/";

    /**
     * This method gets extension of certain file.
     *
     * @param fileName name of a file
     * @return Extension of the file's name
     */
    public static String getFileExtension(String fileName) {
        if (fileName == null) {
            return null;
        }
        String extension = null;
        final int lastDot = fileName.lastIndexOf('.');
        if ((lastDot >= 0)) {
            extension = fileName.substring(lastDot + 1).toLowerCase();
        }
        return extension;
    }

    /**
     * This method gets name of certain file from its path.
     *
     * @param absolutePath the file's absolute path
     * @return name of the file
     */
    public static String getFileName(String absolutePath) {
        int sepIndex = absolutePath.lastIndexOf(SEPARATOR);
        if (sepIndex >= 0) {
            return absolutePath.substring(sepIndex + 1);
        }
        return absolutePath;

    }

    /**
     * This method gets path to directory of certain file(or folder).
     *
     * @param filePath path to certain file
     * @return path to directory of the file
     */
    public static String getFilePath(String filePath) {
        int sepIndex = filePath.lastIndexOf(SEPARATOR);
        if (sepIndex >= 0) {
            return filePath.substring(0, sepIndex);
        }
        return "";

    }

    /**
     * This method converts a size to a string
     *
     * @param size the size of a file
     * @return the string represents the size
     */
    public static String sizeToString(long size) {
        String unit = UNIT_B;
        if (size < 0) {
            Elog.d(TAG, "sizeToString(),size = " + size);
            return 0 + " " + unit;
        }
        if (size < DECIMAL_NUMBER) {
            Elog.d(TAG, "sizeToString(),size = " + size);
            return Long.toString(size) + " " + unit;
        }

        unit = UNIT_KB;
        double sizeDouble = (double) size / (double) UNIT_INTERVAL;
        if (sizeDouble > UNIT_INTERVAL) {
            sizeDouble = (double) sizeDouble / (double) UNIT_INTERVAL;
            unit = UNIT_MB;
        }
        if (sizeDouble > UNIT_INTERVAL) {
            sizeDouble = (double) sizeDouble / (double) UNIT_INTERVAL;
            unit = UNIT_GB;
        }
        if (sizeDouble > UNIT_INTERVAL) {
            sizeDouble = (double) sizeDouble / (double) UNIT_INTERVAL;
            unit = UNIT_TB;
        }

        // Add 0.005 for rounding-off.
        long sizeInt = (long) ((sizeDouble + ROUNDING_OFF) * DECIMAL_NUMBER); // strict to two
        // decimal places
        double formatedSize = ((double) sizeInt) / DECIMAL_NUMBER;
        Elog.d(TAG, "sizeToString(): " + formatedSize + unit);

        if (formatedSize == 0) {
            return "0" + " " + unit;
        } else {
            return Double.toString(formatedSize) + " " + unit;
        }
    }

    /**
    *
    * @param textView
    *            The view to be set adjust with the long string.
    */
   public static void fadeOutLongString(TextView textView) {
       if (textView == null) {
           Elog.w(TAG, "#adjustWithLongString(),the view is to be set is null");
           return;
       }
       if (!(textView instanceof TextView)) {
           Elog.w(TAG, "#adjustWithLongString(),the view instance is not right,execute failed!");
           return;
       }

       textView.setHorizontalFadingEdgeEnabled(true);
       textView.setSingleLine(true);
       textView.setGravity(Gravity.LEFT);
       textView.setGravity(Gravity.CENTER_VERTICAL);
   }
}
