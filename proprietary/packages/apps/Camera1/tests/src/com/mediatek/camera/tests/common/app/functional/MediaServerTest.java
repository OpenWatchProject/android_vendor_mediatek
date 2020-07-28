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
package com.mediatek.camera.tests.common.app.functional;

import android.content.ContentValues;
import android.media.CamcorderProfile;
import android.net.Uri;
import android.provider.MediaStore;

import com.mediatek.camera.CameraActivity;
import com.mediatek.camera.common.CameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.storage.MediaSaver;
import com.mediatek.camera.tests.CameraTestCaseBase;
import com.mediatek.camera.tests.Log;
import com.mediatek.camera.tests.Utils;
import com.mediatek.camera.tests.Utils.Checker;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * This test file used for MediaServer to function test.
 */

public class MediaServerTest extends CameraTestCaseBase<CameraActivity> {
    private static final String TAG = MediaServerTest.class.getSimpleName();
    private static final int ADD_REQUEST_NUMBER = 20;
    private static final int BACK_CAMERA_ID = 0;
    private static final int LOW_QUALITY_ID = 0;
    private boolean mIsFileSaved = false;

    private CameraContext mCameraContext;
    private CamcorderProfile mProfile;
    private MediaSaver mMediaSaver;
    private IApp mApp;
    private Uri mUri;

    private String mFileName;
    private String mFilePath;
    private String mTitle;
    private long mDateTaken;

    private MediaSaver.MediaSaverListener mMediaSaverListener =
            new MediaSaver.MediaSaverListener() {
        @Override
        public void onFileSaved(Uri uri) {
            mIsFileSaved = true;
            mUri = uri;
        }
    };
    /**
     * Creator for test base.
     */
    public MediaServerTest() {
        super(CameraActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Log.i(TAG, "[setUp] + ");
        super.setUp();
        mApp = (CameraActivity) mActivity;
        //TODO: maybe camera context will get from camera activity.
        mCameraContext = new CameraContext();
        mCameraContext.create(mApp, mActivity);
        mMediaSaver = mCameraContext.getMediaSaver();
        mProfile = CamcorderProfile.get(BACK_CAMERA_ID, LOW_QUALITY_ID);
        Log.i(TAG, "[setUp] - ");
    }
    /**
     * Function test for add AddSaveRequest().
     * only set listener not null.
     */
    public void testAddSaveRequestForOnlyListenerNotNull1() {
        Log.i(TAG, "[testAddSaveRequestForOnlyListenerNotNull 1] + ");
        mIsFileSaved = false;
        mMediaSaver.addSaveRequest(null, null, mMediaSaverListener);
        Utils.waitForTrueWithTimeOut(mChecker, Utils.TIME_OUT_MS);
        assertFalse(mIsFileSaved);
        Log.i(TAG, "[testAddSaveRequestForOnlyListenerNotNull 1] - ");
    }
    /**
     * Function test for add AddSaveRequest().
     * only set listener not null.
     */
    public void testAddSaveRequestForOnlyListenerNotNull2() {
        Log.i(TAG, "[testAddSaveRequestForOnlyListenerNotNull 2] + ");
        mIsFileSaved = false;
        mMediaSaver.addSaveRequest(null, null, null, mMediaSaverListener);
        Utils.waitForTrueWithTimeOut(mChecker, Utils.TIME_OUT_MS);
        assertFalse(mIsFileSaved);
        Log.i(TAG, "[testAddSaveRequestForOnlyListenerNotNull 2] - ");
    }
    /**
     * Function test for add AddSaveRequest().
     * set all parameter are null.
     */
    public void testAddSaveRequestForAllParameterIsNull() {
        Log.i(TAG, "[testAddSaveRequestForAllParameterIsNull] + ");
        boolean isCaseFail = false;
        try {
            mMediaSaver.addSaveRequest(null, null, null, null);
            Utils.waitForTrueWithTimeOut(mChecker, Utils.TIME_OUT_MS);
        } catch (Exception e) {
            isCaseFail = true;
        }
        assertFalse(isCaseFail);
        Log.i(TAG, "[testAddSaveRequestForAllParameterIsNull] - ");
    }
    /**
     * Function test for add AddSaveRequest().
     * only set byte not null.
     */
    public void testAddSaveRequestForOnlyByteNotNull() {
        Log.i(TAG, "[testAddSaveRequestForOnlyByteNotNull] + ");
        boolean isCaseFail = false;
        try {
            mMediaSaver.addSaveRequest(new byte[1], null, null, null);
            Utils.waitForTrueWithTimeOut(mChecker, Utils.TIME_OUT_MS);
        } catch (Exception e) {
            isCaseFail = true;
        }
        assertFalse(isCaseFail);
        Log.i(TAG, "[testAddSaveRequestForOnlyByteNotNull] - ");
    }
    /**
     * Function test for add AddSaveRequest().
     * only set photo values not null.
     */
    public void testAddSaveRequestForOnlyPhotoValueNotNull() {
        Log.i(TAG, "[testAddSaveRequestForOnlyPhotoValueNotNull] + ");
        boolean isCaseFail = false;
        initializeCommonInfo(false);
        try {
            mMediaSaver.addSaveRequest(null, createPhotoValues(), null, null);
            Utils.waitForTrueWithTimeOut(mChecker, Utils.TIME_OUT_MS);
        } catch (Exception e) {
            isCaseFail = true;
        }
        assertFalse(isCaseFail);
        Log.i(TAG, "[testAddSaveRequestForOnlyPhotoValueNotNull] - ");
    }
    /**
     * Function test for add AddSaveRequest().
     * only set video value not null.
     */
    public void testAddSaveRequestForOnlyVideoValueNotNull() {
        Log.i(TAG, "[testAddSaveRequestForOnlyVideoValueNotNull] + ");
        initializeCommonInfo(true);
        boolean isCaseFail = false;
        try {
            mMediaSaver.addSaveRequest(null, createVideoValues(), null, null);
            Utils.waitForTrueWithTimeOut(mChecker, Utils.TIME_OUT_MS);
        } catch (Exception e) {
            isCaseFail = true;
        }
        assertFalse(isCaseFail);
        Log.i(TAG, "[testAddSaveRequestForOnlyVideoValueNotNull] - ");
    }
    /**
     * Function test for add AddSaveRequest().
     * only set file path not null.
     */
    public void testAddSaveRequestForOnlyFilePathNotNull() {
        Log.i(TAG, "[testAddSaveRequestForOnlyFilePathNotNull] + ");
        initializeCommonInfo(true);
        boolean isCaseFail = false;
        try {
            mMediaSaver.addSaveRequest(null, null, getTempPath(), null);
            Utils.waitForTrueWithTimeOut(mChecker, Utils.TIME_OUT_MS);
        } catch (Exception e) {
            isCaseFail = true;
        }
        assertFalse(isCaseFail);
        Log.i(TAG, "[testAddSaveRequestForOnlyFilePathNotNull] - ");
    }
    /**
     * Function test for add AddSaveRequest().
     * set all parameters not null.
     */
    public void testAddSaveRequestForVideo() {
        Log.i(TAG, "[testAddSaveRequestForVideo] + ");
        boolean isCaseFail = false;
        mIsFileSaved = false;
        mUri = null;
        initializeCommonInfo(true);
        try {
            mMediaSaver.addSaveRequest(createVideoValues(), getTempPath(), mMediaSaverListener);
            Utils.waitForTrueWithTimeOut(mChecker, Utils.TIME_OUT_MS);
        } catch (Exception e) {
            isCaseFail = true;
        }
        assertFalse(isCaseFail);
        assertTrue(mIsFileSaved);
        assertNotNull(mUri);
        Log.i(TAG, "[testAddSaveRequestForVideo] - ");
    }
    /**
     * Function test for add AddSaveRequest().
     * set all parameters not null for photo.
     */
    public void testAddSaveRequestForPhoto() {
        Log.i(TAG, "[testAddSaveRequestForPhoto] + ");
        boolean isCaseFail = false;
        mIsFileSaved = false;
        mUri = null;
        initializeCommonInfo(false);
        try {
            mMediaSaver.addSaveRequest(new byte[1], createPhotoValues(), null, mMediaSaverListener);
            Utils.waitForTrueWithTimeOut(mChecker, Utils.TIME_OUT_MS);
        } catch (Exception e) {
            isCaseFail = true;
        }
        assertFalse(isCaseFail);
        assertTrue(mIsFileSaved);
        assertNotNull(mUri);
        Log.i(TAG, "[testAddSaveRequestForPhoto] - ");
    }
    /**
     * Function test for add getPendingRequestNumber().
     */
    public void testGetPendingRequestNumber() {
        Log.i(TAG, "[testGetPendingRequestNumber] + ");
        for (int i = 0; i <= ADD_REQUEST_NUMBER; i++) {
            mMediaSaver.addSaveRequest(new byte[i], createPhotoValues(), null, mMediaSaverListener);
        }
        assertTrue(mMediaSaver.getPendingRequestNumber() >= 0);
        Log.i(TAG, "[testGetPendingRequestNumber] - ");
    }

    private ContentValues createVideoValues() {
        ContentValues values = new ContentValues();
        values.put(MediaStore.Video.Media.TITLE, mTitle);
        values.put(MediaStore.Video.Media.DISPLAY_NAME, mFileName);
        values.put(MediaStore.Video.Media.DATE_TAKEN, mDateTaken);
        values.put(MediaStore.MediaColumns.DATE_MODIFIED, mDateTaken / 1000);
        values.put(MediaStore.Video.Media.MIME_TYPE, "video/3gpp");
        values.put(MediaStore.Video.Media.DATA, mFilePath);
        values.put(MediaStore.Video.Media.WIDTH, mProfile.videoFrameWidth);
        values.put(MediaStore.Video.Media.HEIGHT, mProfile.videoFrameHeight);
        values.put(MediaStore.Video.Media.RESOLUTION,
                Integer.toString(mProfile.videoFrameWidth) + "x"
                        + Integer.toString(mProfile.videoFrameHeight));
        return values;
    }

    private ContentValues createPhotoValues() {
        ContentValues values = new ContentValues();
        String mime = "image/jpeg";
        values.put(MediaStore.Images.ImageColumns.DATE_TAKEN, mDateTaken);
        values.put(MediaStore.Images.ImageColumns.TITLE, mTitle);
        values.put(MediaStore.Images.ImageColumns.DISPLAY_NAME, mFileName);
        values.put(MediaStore.Images.ImageColumns.MIME_TYPE, mime);
        values.put(MediaStore.Images.ImageColumns.WIDTH, 1280);
        values.put(MediaStore.Images.ImageColumns.HEIGHT, 720);

        values.put(MediaStore.Images.ImageColumns.ORIENTATION, 90);
        values.put(MediaStore.Images.ImageColumns.DATA, mFilePath);

        return values;
    }

    private void initializeCommonInfo(boolean isVideo) {
        mDateTaken = System.currentTimeMillis();
        mTitle = createFileTitle(isVideo, mDateTaken);
        mFileName = createFileName(isVideo, mTitle);
        mFilePath = mCameraContext.getStorageService().getFileDirectory() + '/' + mFileName;
    }

    private String createFileTitle(boolean isVideo, long dateTaken) {
        SimpleDateFormat format;
        Date date = new Date(dateTaken);
        if (isVideo) {
            format = new SimpleDateFormat("'VID'_yyyyMMdd_HHmmss");
        } else {
            format = new  SimpleDateFormat("'IMG'_yyyyMMdd_HHmmss");
        }
        return format.format(date);
    }


    private String createFileName(boolean isVideo, String  title) {
        String fileName = title + ".jpg";
        if (isVideo) {
            fileName = title + ".3gp";
        }
        return fileName;
    }

    private String getTempPath() {
        return mCameraContext.getStorageService().getFileDirectory() +
                '/' + "videorecorder" + ".3gp" + ".tmp";
    }

    private Checker mChecker =  new Checker() {
        @Override
        public boolean check() {
            return mMediaSaver.getPendingRequestNumber() == 0;
        }
    };
}
