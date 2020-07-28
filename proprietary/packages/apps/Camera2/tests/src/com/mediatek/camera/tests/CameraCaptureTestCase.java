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
package com.mediatek.camera.tests;

import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.ConditionVariable;
import android.os.Environment;
import android.os.Handler;
import android.os.SystemClock;
import android.provider.MediaStore;
import android.support.test.uiautomator.UiObject;
import android.support.test.uiautomator.UiSelector;

import java.io.File;
import java.util.List;

/**
 * Base case for take picture.
 */
public class CameraCaptureTestCase extends CameraInstrumentationTestCaseBase {
    private static final String TAG = CameraCaptureTestCase.class.getSimpleName();

    protected static final String DCIM_CAMERA_FOLDER_ABSOLUTE_PATH =
            Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DCIM).toString() + "/Camera";

    private static final String[] PROJECTION = new String[] {
            MediaStore.Images.ImageColumns._ID, MediaStore.Images.ImageColumns.DATE_TAKEN,
            MediaStore.Images.ImageColumns.TITLE
    };
    private static final String ORDER_CLAUSE = MediaStore.Images.ImageColumns.DATE_TAKEN + " DESC";
    private static final int TIME_TEMP = 10000;
    private static final int PROJECTION_TITLE = 2;
    private static final int PROJECTION_DATE_TAKEN = 1;
    private static final int PROJECTION_ID = 0;
    private static final int UPPER_RANDOM_TIME = 5 * 1000;
    private static final int LOWER_RANDOM_TIME = 100;

    private ConditionVariable mCondition = new ConditionVariable();
    private long mCaptureDateTaken;

    @Override
    protected void setUp() throws Exception {
        Log.i(TAG, "[setUp]");
        super.setUp();
        mActivity = launchCameraByIntent();
        mInstrumentation.waitForIdleSync();
        deleteCameraFolder();
    }

    @Override
    protected void tearDown() throws Exception {
        Log.i(TAG, "[tearDown]");
        closeCameraActivity();
        super.tearDown();
    }

    /**
     * Take picture and check capture succeed.
     *
     * @throws Exception Throw exception.
     */
    protected void captureAndCheckSucceed() throws Exception {
        ContentObserver observer = createContentObserver();
        registerContentObserver(observer);
        mCondition.close();
        mCaptureDateTaken = System.currentTimeMillis();
        Log.d(TAG, "[CaptureAndCheckSucceed], data taken:" + mCaptureDateTaken);
        UiObject photoShutterUiObject = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
        photoShutterUiObject.clickAndWaitForNewWindow();

        boolean blocked = mCondition.block(Utils.TIME_OUT_MS);
        assertTrue("check capture failed.", blocked);
        unRegisterContentObserver(observer);
    }

    /**
     * Return a random time, its value scope:[100, 5000].
     *
     * @return Return a random time, its scope:[100, 5000].
     */
    protected int getRandomTime() {
        int sleepTime = (int) (Math.random() * UPPER_RANDOM_TIME + LOWER_RANDOM_TIME)
                % UPPER_RANDOM_TIME;
        return sleepTime;
    }

    /**
     * To clear camera image.
     */
    protected void deleteCameraFolder() {
        File imageDir = new File(DCIM_CAMERA_FOLDER_ABSOLUTE_PATH);
        Log.d(TAG, "[deleteCameraFolder], imageDir: " + imageDir);
        clearCameraImages(imageDir);
    }

    /**
     * Wait to the state that can take picture.
     */
    protected void waitCaptureStateReady() {
        SystemClock.sleep(TIME_TEMP);
    }

    private void clearCameraImages(File imageDir) {
        if (imageDir == null) {
            return;
        }
        if (imageDir.isDirectory()) {
            File[] dirChildren = imageDir.listFiles();
            if (dirChildren == null || dirChildren.length == 0) {
                imageDir.delete();
                return;
            }
            for (int i = 0; i < dirChildren.length; i++) {
                clearCameraImages(dirChildren[i]);
            }
            imageDir.delete();
        } else {
            imageDir.delete();
        }
    }

    private void closeCameraActivity() {
        mActivity.finish();
    }

    private ContentObserver createContentObserver() {
        ContentObserver observer = new ContentObserver(
                new Handler(mActivity.getMainLooper())) {
            @Override
            public void onChange(boolean selfChange, Uri uri) {
                super.onChange(selfChange, uri);
                boolean normal = checkUri(uri);
                if (normal) {
                    mCondition.open();
                }
            }
        };
        return observer;
    }

    private void registerContentObserver(ContentObserver observer) {
        mActivity.getContentResolver().registerContentObserver(
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                true,
                observer);
    }

    private void unRegisterContentObserver(ContentObserver observer) {
        mActivity.getContentResolver().unregisterContentObserver(observer);
    }

    /**
     * Check the data taken of changed uri in data base is later then capture take taken.
     */
    private boolean checkUri(Uri uri) {
        Log.d(TAG, "[checkUri], uri:" + uri.getPath());

        Cursor cursor = mActivity.getContentResolver().query(
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                PROJECTION,
                null,
                null,
                ORDER_CLAUSE
        );
        String dateTaken = null;
        if (cursor != null && cursor.moveToNext()) {
            dateTaken = cursor.getString(PROJECTION_DATE_TAKEN);
            Log.d(TAG, "[checkUri], dateTaken:" + dateTaken + ", id:" + cursor.getString
                    (PROJECTION_ID) + ", title:" + cursor.getString(PROJECTION_TITLE));
            cursor.close();
        }
        if (dateTaken == null) {
            return false;
        }
        return Long.parseLong(dateTaken) > mCaptureDateTaken;
    }
}
