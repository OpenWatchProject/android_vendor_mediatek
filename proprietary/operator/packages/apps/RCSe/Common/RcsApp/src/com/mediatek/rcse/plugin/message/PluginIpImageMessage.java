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
package com.mediatek.rcse.plugin.message;

import android.content.ContentResolver;
import android.database.Cursor;
import android.provider.MediaStore;
import android.provider.MediaStore.Images.Thumbnails;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.binder.FileStructForBinder;

import com.mediatek.rcse.service.MediatekFactory;

/**
 * Provide text message.
 */
public class PluginIpImageMessage extends IpImageMessage {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "PluginIpImageMessage";
    /**
     * The m progress.
     */
    private long mProgress = 0;
    /**
     * The m status.
     */
    private int mStatus = 11;
    /**
     * The m rcs status.
     */
    private int mRcsStatus = 0;
    /**
     * The m transfer tag.
     */
    private String mTransferTag;
    /**
     * The m file name.
     */
    private String mFileName;
    /**
     * The thumbnail.
     */
    private String mThumbnail = null;

    /**
     * Instantiates a new plugin ip image message.
     *
     * @param fileStruct the file struct
     * @param remote the remote
     */
    public PluginIpImageMessage(FileStructForBinder fileStruct, String remote) {
        Logger.d(TAG, "PluginIpImageMessage(), fileStruct = " + fileStruct
                + " remote = " + remote);
        setSimId((int) PluginUtils.DUMMY_SIM_ID);
        setPath(fileStruct.mFilePath);
        setSize((int) (fileStruct.mFileSize / PluginUtils.SIZE_K));
        mFileName = fileStruct.mFileName;
        setType(IpMessageConsts.IpMessageType.PICTURE);
        setFrom(remote);
        setTo(remote);
        setmMessageId(fileStruct.mFileTransferTag);
        mThumbnail = fileStruct.mThumbnail;
        mTransferTag = fileStruct.mFileTransferTag;
        analysisAttribute();
    }
    /**
     * Analysis attribute.
     */
    private void analysisAttribute() {
        try {
        long imageId = -1;
        String whereClause = MediaStore.Images.Media.DISPLAY_NAME + "='"
                + mFileName + "'";
        ContentResolver cr = MediatekFactory.getApplicationContext()
                .getContentResolver();
        Cursor cursor = null;
        try {
            cursor = cr.query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                    new String[] { MediaStore.Images.Media._ID,
                            MediaStore.Images.Media.WIDTH,
                            MediaStore.Images.Media.HEIGHT }, whereClause,
                    null, null);
            if (cursor != null && cursor.getCount() != 0) {
                cursor.moveToFirst();
                imageId = cursor.getLong(cursor
                        .getColumnIndex(MediaStore.Images.Media._ID));
                int width = cursor.getInt(cursor
                        .getColumnIndex(MediaStore.Images.Media.WIDTH));
                int hight = cursor.getInt(cursor
                        .getColumnIndex(MediaStore.Images.Media.HEIGHT));
                setWidthInPixel(width);
                setHeightInPixel(hight);
            } else {
                Logger.w(TAG, "analysisAttribute(), cursor is null!");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        if (imageId != -1) {
            whereClause = Thumbnails.IMAGE_ID + "='" + imageId + "'";
            try {
                cursor = cr.query(Thumbnails.EXTERNAL_CONTENT_URI,
                        new String[] { Thumbnails.DATA }, whereClause, null,
                        null);
                if (cursor != null && cursor.getCount() != 0) {
                    cursor.moveToFirst();
                    String thumbnailPath = cursor.getString(cursor
                            .getColumnIndex(Thumbnails.DATA));
                    if (thumbnailPath == null || thumbnailPath.equals("")) {
                        thumbnailPath = mThumbnail;
                    }
                    setThumbPath(thumbnailPath);
                } else {
                    Logger.w(TAG, "analysisAttribute(), cursor is null!");
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                    cursor = null;
                }
            }
        } else {
            if (mThumbnail != null) {
                setThumbPath(mThumbnail);
            }
            Logger.w(TAG, "analysisAttribute(), have no this image!");
        }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    /**
     * Set file progress.
     *
     * @param progress the new progress
     */
    public void setProgress(long progress) {
        Logger.d(TAG, "setProgress(), progress = " + progress);
        mProgress = progress;
    }
    /**
     * Get file progress.
     *
     * @return progress
     */
    public int getProgress() {
        Logger.d(TAG, "getProgress()");
        return (int) mProgress;
    }
    /**
     * Set file status.
     *
     * @param status the new status
     */
    public void setStatus(int status) {
        Logger.v(TAG, "SetStatus():" + status);
        mStatus = status;
        super.setStatus(status);
    }
    /**
     * Get file status.
     *
     * @return status
     */
    public int getStatus() {
        Logger.v(TAG, "getStatus():" + mStatus);
        return mStatus;
    }
    /**
     * Set rcse status.
     *
     * @param status the new rcs status
     */
    public void setRcsStatus(int status) {
        Logger.d(TAG, "setRcsStatus()");
        mRcsStatus = status;
    }
    /**
     * Get rcse status.
     *
     * @return status
     */
    public int getRcsStatus() {
        Logger.d(TAG, "getRcsStatus()");
        return mRcsStatus;
    }
    /**
     * Set file transfer tag.
     *
     * @param tag the new tag
     */
    public void setTag(String tag) {
        Logger.d(TAG, "setTag()");
        mTransferTag = tag;
    }
    /**
     * Get file transfer tag.
     *
     * @return tag
     */
    public String getTag() {
        Logger.d(TAG, "getTag()");
        return mTransferTag;
    }
    /**
     * Get file transfer name.
     *
     * @return name
     */
    public String getName() {
        Logger.d(TAG, "getName()");
        return mFileName;
    }
}
