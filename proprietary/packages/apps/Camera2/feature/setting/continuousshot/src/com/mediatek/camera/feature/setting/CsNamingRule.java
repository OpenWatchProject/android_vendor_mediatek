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

package com.mediatek.camera.feature.setting;

import android.content.ContentValues;
import android.provider.MediaStore;

import com.mediatek.camera.common.utils.CameraUtil;

import java.sql.Date;
import java.text.SimpleDateFormat;

/**
 * this class is used for continuous shot naming.
 */

class CsNamingRule {
    private static final String IMAGE_FORMAT = "'IMG'_yyyyMMdd_HHmmss";
    private static final String FILE_NAME_CONNECTOR = "_";
    private static final String FILE_NAME_SUFFIX = "CS";
    private static final String FILE_FORMAT_SUFFIX = ".jpg";

    private final ImageFileName mImageFileName;

    /**
     * Create a image file name for continuous shot name.
     */
    CsNamingRule() {
        mImageFileName = new ImageFileName(IMAGE_FORMAT);
    }

    /**
     * create a content values from data.
     *
     * @param data          the resource file.
     * @param fileDirectory file directory.
     * @param pictureWidth  the width of content values.
     * @param pictureHeight the height of content values.
     * @param shutterTime shutter time.
     * @param count current continuous shot number.
     * @return the content values from the data.
     */
    ContentValues createContentValues(byte[] data, String fileDirectory, int
            pictureWidth, int pictureHeight, long shutterTime, int count) {
        ContentValues values = new ContentValues();
        String title = mImageFileName.generateTitle(shutterTime) +
                FILE_NAME_CONNECTOR + count + FILE_NAME_SUFFIX;
        int orientation = CameraUtil.getOrientationFromExif(data);

        String mime = "image/jpeg";
        String fileName = mImageFileName.generateTitle(shutterTime) +
                FILE_NAME_CONNECTOR + count + FILE_NAME_SUFFIX + FILE_FORMAT_SUFFIX;
        String path = fileDirectory + '/' + fileName;

        values.put(MediaStore.Images.ImageColumns.DATE_TAKEN, System.currentTimeMillis());
        values.put(MediaStore.Images.ImageColumns.TITLE, title);
        values.put(MediaStore.Images.ImageColumns.DISPLAY_NAME, fileName);
        values.put(MediaStore.Images.ImageColumns.MIME_TYPE, mime);
        values.put(MediaStore.Images.ImageColumns.WIDTH, pictureWidth);
        values.put(MediaStore.Images.ImageColumns.HEIGHT, pictureHeight);
        values.put(MediaStore.Images.ImageColumns.ORIENTATION, orientation);
        values.put(MediaStore.Images.ImageColumns.DATA, path);

        return values;
    }

    /**
     * Used for create image file name.
     */
    private class ImageFileName {
        private final SimpleDateFormat mSimpleDateFormat;

        public ImageFileName(String format) {
            mSimpleDateFormat = new SimpleDateFormat(format);
        }

        public String generateTitle(long dateTaken) {
            Date date = new Date(dateTaken);
            return mSimpleDateFormat.format(date);
        }
    }
}
