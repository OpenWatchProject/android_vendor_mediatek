/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.android.emailcommon.utility;

import android.os.Environment;
import android.text.TextUtils;

import java.io.File;
import java.io.IOException;
import java.nio.charset.Charset;

public class Utilities {
    public static final Charset UTF_8 = Charset.forName("UTF-8");
    public static final Charset ASCII = Charset.forName("US-ASCII");

    public static final String[] EMPTY_STRINGS = new String[0];
    public static final Long[] EMPTY_LONGS = new Long[0];
    /// M: The mime type of file with .dcf extension (DRM sd file)
    public static final String MIME_DCF = "application/dcf";
    /** @} */

    /// M:To match attachment name contains invalid character or not
    private static final String INVALID_FILENAME_PATTERN = ".*[/\\\\:*?\"<>|].*";
    private static final int MAX_LENGTH = 255;

    /**
     * Interface used in {@link #createUniqueFile} instead of {@link File#createNewFile()} to make
     * it testable.
     */
    /* package */ interface NewFileCreator {
        public static final NewFileCreator DEFAULT = new NewFileCreator() {
                    @Override public boolean createNewFile(File f) throws IOException {
                        return f.createNewFile();
                    }
        };
        public boolean createNewFile(File f) throws IOException ;
    }

    /**
     * Creates a new empty file with a unique name in the given directory by appending a hyphen and
     * a number to the given filename.
     *
     * @return a new File object, or null if one could not be created
     */
    public static File createUniqueFile(File directory, String filename) throws IOException {
        return createUniqueFileInternal(NewFileCreator.DEFAULT, directory, filename);
    }

    /* package */ static File createUniqueFileInternal(NewFileCreator nfc,
            File directory, String filename) throws IOException {
        File file = new File(directory, filename);
        if (nfc.createNewFile(file)) {
            return file;
        }
        // Get the extension of the file, if any.
        int index = filename.lastIndexOf('.');
        String format;
        /// M: Modified for the filename containing '%' char
        String extension;
        if (index != -1) {
            String name = filename.substring(0, index);
            extension = filename.substring(index);
            /// M: Modified for the filename containing '%' char
            format = name + "-";
        } else {
            format = filename + "-";
            /// M: Modified for the filename containing '%' char
            extension = "";
        }

        for (int i = 2; i < Integer.MAX_VALUE; i++) {
            /// M: Modified for the filename containing '%' char
            file = new File(directory, format + i + extension);
            if (nfc.createNewFile(file)) {
                return file;
            }
        }
        return null;
    }

    public static boolean isExternalStorageMounted() {
        return Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
    }

    /**
     * M: Is the file name valid
     * @param fileName the name of file
     * @return true if the filename valid, otherwise false.
     */
    public static boolean isFileNameValid(String fileName) {
        if (TextUtils.isEmpty(fileName)) {
            return false;
        }
        return !fileName.toString().matches(INVALID_FILENAME_PATTERN);
    }

    /** M: Remove the invalid character in attahcment's name. */
    public static String removeFileNameSpecialChar(String filename) {
        if (filename == null) {
            return null;
        }
        filename = filename.replaceAll("[/\\\\:*?\"<>|]", "");
        return filename;
    }

    /** M: Dispose long filename. */
    public static String getLengthSafeFileName(String fileName) {
        if (fileName == null) {
            return null;
        }
        if (fileName.length() > MAX_LENGTH) {
            String extention = getFilenameExtension(fileName);
            int extentionLen = extention.length();
            String subString = fileName.substring(0, MAX_LENGTH - extentionLen - 1);
            fileName = subString + "." + extention;
        }
        return fileName;
    }

    /**
     * M:Extract and return filename's extension, converted to lower case, and not including the "."
     *
     * @return extension, or null if not found (or null/empty filename)
     */
    public static String getFilenameExtension(String fileName) {
        String extension = null;
        if (!TextUtils.isEmpty(fileName)) {
            int lastDot = fileName.lastIndexOf('.');
            if ((lastDot > 0) && (lastDot < fileName.length() - 1)) {
                extension = fileName.substring(lastDot + 1).toLowerCase();
            }
        }
        return extension;
    }
}
