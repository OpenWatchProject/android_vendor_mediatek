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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.android.camera.bridge;

import junit.framework.Assert;
import android.location.Location;

import com.android.camera.FileSaver;
import com.android.camera.FileSaver.FileSaverListener;
import com.android.camera.SaveRequest;
import com.android.camera.Storage;

import com.mediatek.camera.util.Log;
import com.mediatek.camera.platform.IFileSaver;

public class FileSaverImpl implements IFileSaver {
    private static final String TAG = "FileSaverImpl";

    FILE_TYPE mFileType;
    private OnFileSavedListener mListener;
    private FileSaver mFileSaver;
    private SaveRequest mSaveRequest;
    private SaveRequest mRawSaveRequest;
    private SaveRequest mVideoSaveRequest;

    public FileSaverImpl(FileSaver fileSaver) {
        Assert.assertNotNull(fileSaver);
        mFileSaver = fileSaver;
    }

    @Override
    public void init(FILE_TYPE fileType, int outputFileFormat, String resolution, int rotation) {
        Log.d(TAG, "[initFileSaver]fileType= " + fileType + ",resolution = " + resolution
                + ",rotation = " + rotation);
        mFileType = fileType;
        switch (fileType) {
        case VIDEO:
        case PIPVIDEO:
        case SLOWMOTION:
            // rotation need check
            mVideoSaveRequest = mFileSaver.prepareVideoRequest(Storage.FILE_TYPE_VIDEO,
                    outputFileFormat, resolution, rotation);
            break;

        case PANORAMA:
            mSaveRequest = mFileSaver.preparePhotoRequest(Storage.FILE_TYPE_PANO,
                    Storage.PICTURE_TYPE_JPG);
            break;

        case RAW:
            mRawSaveRequest = mFileSaver.preparePhotoRequest(Storage.FILE_TYPE_PHOTO,
                    Storage.PICTURE_TYPE_RAW);
            break;

        default:
            // normal is photo save request
            mSaveRequest = mFileSaver.preparePhotoRequest(Storage.FILE_TYPE_PHOTO,
                    Storage.PICTURE_TYPE_JPG);
            break;
        }
    }

    @Override
    public SaveRequest getVideoSaveRequest() {
        return mVideoSaveRequest;
    }

    @Override
    public void uninit() {
        // do nothing
    }

    @Override
    public void setRawFlagEnabled(boolean isEnable) {
        mFileSaver.enableRawFlag(isEnable);
    }

    /**
     * save dng image, it need init the save request first.
     * @param photoData the dng data.
     * @param width the width of dng.
     * @param height the height of dng.
     * @param fileName the dng file name.
     * @param date the dng image captured data.
     * @param location the image captured location, may be null.
     * @param tag add for Refocus image database.
     * @param listener the listener after saving the dng.
     * @return the save result, true or false.
     */
    public boolean saveRawFile(byte[] photoData, int width, int height,
            String fileName, long date, Location location, int tag,
            OnFileSavedListener listener) {
        Log.d(TAG, "[saveRawFile]title =" + fileName);
        if (null == mRawSaveRequest || null == photoData) {
            Log.w(TAG, "[savePhotoFile]fail,mRawSaveRequest = " + mRawSaveRequest);
            return false;
        }
        mListener = listener;
        mRawSaveRequest.setData(photoData);
        mRawSaveRequest.setSize(width, height);
        mRawSaveRequest.setFileName(fileName);
        mRawSaveRequest.updateDataTaken(date);
        mRawSaveRequest.setIgnoreThumbnail(true);
        mRawSaveRequest.setLocation(location);
        mRawSaveRequest.setListener(mFileSaverListener);
        mRawSaveRequest.addRequest();
        //set raw request to null for speed up GC
        mRawSaveRequest = null;
        return true;
    }

    @Override
    public boolean savePhotoFile(byte[] photoData, String fileName, long date,
            Location location, int tag, OnFileSavedListener listener) {
        Log.d(TAG, "[savePhotoFile]title =" + fileName);
        if (null == mSaveRequest || null == photoData) {
            Log.w(TAG, "[savePhotoFile]fail,mSaveRequest = " + mSaveRequest);
            return false;
        }

        mListener = listener;
        if (mSaveRequest.getDataSize() > 0) {
            Log.d(TAG, "[savePhotoFile]Current SaveRequest is used, copy new one!");
            mSaveRequest = mFileSaver.copyPhotoRequest(mSaveRequest);
        }
        mSaveRequest.setData(photoData);
        mSaveRequest.setFileName(fileName);
        mSaveRequest.setTag(tag);
        mSaveRequest.updateDataTaken(date);
        mSaveRequest.setLocation(location);
        mSaveRequest.setListener(mFileSaverListener);
        mSaveRequest.addRequest();

        return true;
    }

    @Override
    public boolean saveVideoFile(Location location, String tempPath, long duration, int tag,
            OnFileSavedListener listener) {
        Log.d(TAG, "[saveVideoFile]tempPath =" + tempPath + ",duration =" + duration + ",tag ="
                + tag);
        if (null == mVideoSaveRequest || null == tempPath || null == listener) {
            Log.w(TAG, "[saveVideoFile]fail, you should need to call init.so retrun!");
            return false;
        }

        mListener = listener;
        mVideoSaveRequest.setLocation(location);
        mVideoSaveRequest.setTempPath(tempPath);
        mVideoSaveRequest.setDuration(duration);
       if (mFileType == FILE_TYPE.SLOWMOTION) {
            mVideoSaveRequest.setSlowMotionSpeed(tag);
        } else {
            mVideoSaveRequest.setSlowMotionSpeed(0);
        }
        mVideoSaveRequest.setListener(mFileSaverListener);
        mVideoSaveRequest.addRequest();

        return true;
    }

    @Override
    public long getWaitingDataSize() {
        return mFileSaver.getWaitingDataSize();
    }

    @Override
    public void waitDone() {
        mFileSaver.waitDone();
    }

    @Override
    public boolean isEnoughSpace() {
        return 1 <= Storage.getLeftSpace();
    }

    @Override
    public long getAvailableSpace() {
        return Storage.getAvailableSpace();
    }

    private final FileSaverListener mFileSaverListener = new FileSaverListener() {
        @Override
        public void onFileSaved(SaveRequest request) {
            if (mListener != null) {
                mListener.onFileSaved(request.getUri());
            }
        }
    };
}
