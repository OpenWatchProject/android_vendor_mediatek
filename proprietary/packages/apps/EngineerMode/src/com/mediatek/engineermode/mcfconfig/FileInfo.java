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

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.media.MediaFile;
import android.net.Uri;
import android.provider.MediaStore;
import android.provider.MediaStore.MediaColumns;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.mcfconfig.FileUtils;

import java.io.File;
import java.io.FileFilter;
import java.util.Arrays;
import java.util.List;

public class FileInfo {
    private static final String TAG = "McfConfig/FileInfo";


    /** File name's max length */
    public static final int FILENAME_MAX_LENGTH = 255;

    private final File mFile;
    private String mParentPath = null;
    private final String mName;
    private final String mAbsolutePath;
    private String mFileSizeStr = null;
    private final boolean mIsDir;
    private long mLastModifiedTime = -1;
    private long mSize = -1;

    /** Used in FileInfoAdapter to indicate whether the file is selected */
    private boolean mIsChecked = false;

    private String mLastAccessPath;

    /**
     * Constructor of FileInfo, which restore details of a file.
     *
     * @param file the file associate with the instance of FileInfo.
     * @throws IllegalArgumentException when the parameter file is null, will
     *             throw the Exception.
     */
    public FileInfo(File file, long size) throws IllegalArgumentException {
        if (file == null) {
            throw new IllegalArgumentException();
        }
        mFile = file;
        mName = mFile.getName();
        mAbsolutePath = mFile.getAbsolutePath();
        mLastModifiedTime = mFile.lastModified();
        mIsDir = mFile.isDirectory();
        if(size > -1) {
            mSize = size;
        } else if (!mIsDir) {
            mSize = mFile.length();
        }
    }

    /**
     * Constructor of FileInfo, which restore details of a file.
     *
     * @param absPath the absolute path of a file which associated with the
     *            instance of FileInfo.
     */
    public FileInfo(String absPath, long size) {
        if (absPath == null) {
            throw new IllegalArgumentException();
        }
        mAbsolutePath = absPath;
        mFile = new File(absPath);
        mName = FileUtils.getFileName(absPath);
        mLastModifiedTime = mFile.lastModified();
        mIsDir = mFile.isDirectory();
        if(size > -1) {
            mSize = size;
        } else if (!mIsDir) {
            mSize = mFile.length();
        }
    }

    /**
     * This method gets a file's parent path
     *
     * @return file's parent path.
     */
    public String getFileParentPath() {
        if (mParentPath == null) {
            mParentPath = FileUtils.getFilePath(mAbsolutePath);
        }
        return mParentPath;
    }

    /**
     * This method gets a file's real name.
     *
     * @return file's name on FileSystem.
     */
    public String getFileName() {

        return mName;
    }

    /**
     * This method gets the file's size(including its contains).
     *
     * @return file's size in long format.
     */
    public long getFileSize() {
        return mSize;
    }

    /**
     * This method gets transform the file's size from long to String.
     *
     * @return file's size in String format.
     */
    public String getFileSizeStr() {
        if (mFileSizeStr == null) {
            mFileSizeStr = FileUtils.sizeToString(mSize);
        }
        return mFileSizeStr;
    }

    /**
     * This method gets the file's absolute path.
     *
     * @return the file's absolute path.
     */
    public String getFileAbsolutePath() {
        return mAbsolutePath;
    }

    /**
     * This method gets the file packaged in FileInfo.
     *
     * @return the file packaged in FileInfo.
     */
    public File getFile() {
        return mFile;
    }
    /**
     * This method is used to convert file path (of the given item) to content URI
     *
     * @param context the caller's context
     * @return the content URI of the item
     */
    public Uri getItemContentUri(Context context) {
        final String[] projection = {MediaColumns._ID};
        final String where = MediaColumns.DATA + " = ?";
        Uri baseUri = MediaStore.Files.getContentUri("external");
        Cursor c = null;
        String provider = "com.android.providers.media.MediaProvider";
        Uri itemUri = null;
        context.grantUriPermission(provider, baseUri, Intent.FLAG_GRANT_READ_URI_PERMISSION);
        Elog.d(TAG, "getItemContentUri, filePath = " + mAbsolutePath+
            ", projection = "+projection+
            ", where = "+where+
            ", baseUri = "+baseUri);
        try {
            c = context.getContentResolver().query(baseUri,
                    projection,
                    where,
                    new String[]{mAbsolutePath},
                    null);
            if (c != null && c.moveToNext()) {
                int type = c.getInt(c.getColumnIndexOrThrow(MediaColumns._ID));
                if (type != 0) {
                    long id = c.getLong(c.getColumnIndexOrThrow(MediaColumns._ID));
                    Elog.d(TAG, "getItemContentUri, item id = " + id);
                    itemUri =  Uri.withAppendedPath(baseUri, String.valueOf(id));
                }
            }
        } catch (Exception e) {
            Elog.e(TAG, "getItemContentUri Exception " + e);
        } finally {
            if (c != null) {
                c.close();
            }
        }
        return itemUri;
    }
    /**
     * This method gets the file packaged in FileInfo.
     *
     * @return the file packaged in FileInfo.
     */
    public Uri getUri() {
        return Uri.fromFile(mFile);
    }

    @Override
    public int hashCode() {
        return getFileAbsolutePath().hashCode();
    }

    @Override
    public boolean equals(Object o) {
        if (super.equals(o)) {
            return true;
        } else {
            if (o instanceof FileInfo) {
                if (((FileInfo) o).getFileAbsolutePath().equals(this.getFileAbsolutePath())) {
                    return true;
                }
            }
            return false;
        }
    }

    /**
     * This method checks that weather the file is hide file, or not.
     *
     * @return true for hide file, and false for not hide file
     */
    public boolean isHideFile() {
        if (getFileName().startsWith(".")) {
            return true;
        }
        return false;
    }


}
