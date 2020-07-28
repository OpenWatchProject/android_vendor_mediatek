/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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
package com.mediatek.rcse.service.binder;

import android.os.Parcel;
import android.os.Parcelable;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;

import java.util.Date;

/**
 * This class is for IPC FileStruct.
 */
public class FileStructForBinder implements Parcelable {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "FileStructForBinder";
    /**
     * The file path.
     */
    public String mFilePath = null;
    /**
     * The thumbnail.
     */
    public String mThumbnail = null;
    /**
     * The file name.
     */
    public String mFileName = null;
    /**
     * The file size.
     */
    public long mFileSize = -1;
    /**
     * The file transfer tag.
     */
    public String mFileTransferTag = null;
    /**
     * The date.
     */
    public Date mDate;

    public int isReload = 0;

    /* (non-Javadoc)
     * @see java.lang.Object#toString()
     */
    @Override
    public String toString() {
        return TAG + "file path is " + mFilePath + " file name is "
                + mFileName + " size is " + mFileSize
                + " FileTransferTag is " + mFileTransferTag
                + " date is " + mDate;
    }
    /**
     * Instantiates a new file struct for binder.
     *
     * @param source the source
     */
    public FileStructForBinder(Parcel source) {
        Logger.d(TAG, "FileStructForBinder() entry! source = "
                + source);
        mFilePath = source.readString();
        mFileName = source.readString();
        mFileSize = source.readLong();
        mFileTransferTag = source.readString();
        mThumbnail = source.readString();
        mDate = new Date(source.readLong());
         isReload = source.readInt();
        Logger.d(TAG, "readfromparcel(), mFilePath = " + mFilePath + " mName = " + mFileName + " mSize = " + mFileSize
                + " mFileTransferTag = " + mFileTransferTag + " mDate = " + mDate + "mThumbnail =" + mThumbnail + "isreload =" + isReload);
              
    }
    /* (non-Javadoc)
     * @see android.os.Parcelable#writeToParcel(android.os.Parcel, int)
     */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        Logger.d(TAG, "writeToParcel(), mFilePath = " + mFilePath
                + " mName = " + mFileName + " mSize = " + mFileSize
                + " mFileTransferTag = " + mFileTransferTag
                + " mDate = " + mDate + "thumbnial =" + mThumbnail);
        dest.writeString(mFilePath);
        dest.writeString(mFileName);
        dest.writeLong(mFileSize);
        dest.writeString(mFileTransferTag);
        dest.writeString(mThumbnail);
        dest.writeLong(mDate.getTime());
        dest.writeInt(isReload);
    }
    /**
     * Instantiates a new file struct for binder.
     *
     * @param fileStruct the file struct
     */
    public FileStructForBinder(FileStruct fileStruct) {
        Logger.d(TAG, "FileStructForBinder() entry! fileStruct = "
                + fileStruct);
        mFilePath = fileStruct.mFilePath;
        mFileName = fileStruct.mName;
        mFileSize = fileStruct.mSize;
        TagTranslater.saveTag(fileStruct.mFileTransferTag);
        mFileTransferTag = fileStruct.mFileTransferTag.toString();
        mDate = fileStruct.mDate;
        mThumbnail = fileStruct.mThumbnail;
                isReload = fileStruct.mReload;
        if (mThumbnail == null) {
            mThumbnail = "";
        }
        if (mFilePath == null) {
            mFilePath = "";
        }
    }
    /**
     * Instantiates a new file struct for binder.
     *
     * @param path the path
     * @param name the name
     * @param size the size
     * @param tag the tag
     * @param fileDate the file date
     */
    public FileStructForBinder(String path, String name, long size,
            Object tag, Date fileDate) {
        mFilePath = path;
        mFileName = name;
        mFileSize = size;
        TagTranslater.saveTag(tag);
        mFileTransferTag = tag.toString();
    }
    /* (non-Javadoc)
     * @see android.os.Parcelable#describeContents()
     */
    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * Parcelable creator.
     */
    public static final Parcelable.Creator<FileStructForBinder> CREATOR =
            new Parcelable.Creator<FileStructForBinder>() {
        @Override
        public FileStructForBinder createFromParcel(Parcel source) {
            return new FileStructForBinder(source);
        }
        @Override
        public FileStructForBinder[] newArray(int size) {
            return new FileStructForBinder[size];
        }
    };
}
