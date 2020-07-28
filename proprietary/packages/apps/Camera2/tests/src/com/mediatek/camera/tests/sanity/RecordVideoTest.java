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

package com.mediatek.camera.tests.sanity;

import android.accessibilityservice.AccessibilityServiceInfo;
import android.content.Context;
import android.os.Environment;
import android.os.SystemClock;
import android.support.test.uiautomator.UiDevice;
import android.support.test.uiautomator.UiObject;
import android.support.test.uiautomator.UiObjectNotFoundException;
import android.support.test.uiautomator.UiSelector;

import com.mediatek.camera.CameraActivity;
import com.mediatek.camera.tests.CameraTestCaseBase;
import com.mediatek.camera.tests.Log;
import com.mediatek.camera.tests.Utils;
import com.mediatek.camera.tests.Utils.Checker;

import junit.framework.Assert;

import java.io.File;
import java.io.FilenameFilter;
import java.util.Locale;

/**
 * A class that tests Camera sanity about recording video.
 */
public class RecordVideoTest extends CameraTestCaseBase<CameraActivity> {
    private static final String TAG = "RecordVideoTest";
    private static final String DCIM = Environment.getExternalStoragePublicDirectory(
            Environment.DIRECTORY_DCIM).toString();
    private static final int TIME_OUT_MS = 60000; // 60sec
    private static final long WAIT_FOR_RECORD = 15000;//15sec
    private static final int TIME_TEMP = 10000;
    private static final int SWIPE_STEPS = 20;
    private static final int SWIPE_LENS = 200;
    private Context mContext = null;
    private UiDevice mUiDevice = null;
    private UiObject mVideoShutterBtn;
    private UiObject mThumbnailBtn;
    private UiObject mSwitcher;

    private String mFilter3gp = ".3gp";
    private String mFiltermp4 = ".mp4";
    private String mVideoPath;
    private int mCntBefore = 0;

    /**
     * constructor.
     */
    public RecordVideoTest() {
        super(CameraActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Log.v(TAG, "setUp start");
        super.setUp();
        setActivityInitialTouchMode(false);
        // set up condition
        setUpCondition();
        Log.v(TAG, "setUp end");
    }

    /**
     * test record video.
     * Step 1: take main sensor begin
     * Step 2: swipe to video mode
     * Step 3: click video button and record 15s
     * Step 4: check image number
     * Step 5: take sub sensor begin
     * Step 6: switch to sub sensor
     * Step 7: go to gallery
     * @Exception throw exception
     */
    public void testRecordVideo() throws Exception {
        Log.v(TAG, "testRecordVideo start");
        // get image num before capture
        mCntBefore = getFileCntInDir(mVideoPath);
        // Step 1: take main sensor begin
        Log.v(TAG, "takeMainSensor!");
        // wait for preview idle and view ready
        Log.v(TAG, "checkViewStatus");
        assertStatusReady(mShutterStatusChecker, TIME_OUT_MS);
        assertStatusReady(mCameraStatusChecker, TIME_OUT_MS);
        // Step 2: swipe to video mode
        assertStatusReady(mSwitchVideoChecker, TIME_OUT_MS);

        // Step 3: click video button and record 15s
        Log.v(TAG, "click On Video button begin!");
        mVideoShutterBtn = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
        clickOnUiObject(mVideoShutterBtn);
        SystemClock.sleep(WAIT_FOR_RECORD);
        // stop record
        Log.v(TAG, "click On Video button end!");
        mVideoShutterBtn = mUiDevice.findObject(
                new UiSelector().resourceId(Utils.VIDEO_STOP_BUTTON_ID));
        clickOnUiObject(mVideoShutterBtn);
        // wait for preview idle and view ready
        Log.v(TAG, "checkViewStatus");
        assertStatusReady(mShutterStatusChecker, TIME_OUT_MS);
        assertStatusReady(mCameraStatusChecker, TIME_OUT_MS);
        // Step 4: check image number
        assertStatusReady(mImageNumChecker, TIME_OUT_MS);
        Log.v(TAG, "takeMainSensor end!");
        // Step 5: take sub sensor begin
        // Step 6: switch to sub sensor
        mSwitcher = mUiDevice.findObject(
                new UiSelector().resourceId(Utils.SWITCH_BUTTON_ID));
        clickOnUiObject(mSwitcher);
        Log.v(TAG, "checkViewStatus");
        assertStatusReady(mShutterStatusChecker, TIME_OUT_MS);
        assertStatusReady(mCameraStatusChecker, TIME_OUT_MS);
        Log.v(TAG, "takeSubSensor!");
        mVideoShutterBtn = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
        clickOnUiObject(mVideoShutterBtn);
        // record 10s
        SystemClock.sleep(WAIT_FOR_RECORD);
        // stop video
        mVideoShutterBtn = mUiDevice.findObject(
                new UiSelector().resourceId(Utils.VIDEO_STOP_BUTTON_ID));
        clickOnUiObject(mVideoShutterBtn);
        Log.v(TAG, "takeSubSensor end");
        // Step 7: go to gallery
        Log.v(TAG, "gotoGallery start");
        assertStatusReady(mThumbnailStatusChecker, TIME_OUT_MS);
        mThumbnailBtn = mUiDevice.findObject(
                new UiSelector().resourceId(Utils.THUMBNAIL_RES_ID));
        clickOnUiObject(mThumbnailBtn);
        Log.v(TAG, "testRecordVideo end");
    }

    private void assertStatusReady(Checker checker, int timeOutMs) {
        Assert.assertEquals(Utils.waitForTrueWithTimeOut(checker, timeOutMs), false);
    }

    private Utils.Checker mImageNumChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            int cntAfter = getFileCntInDir(mVideoPath);
            if (cntAfter >= mCntBefore + 1) {
                Log.v(TAG, "record video is Ready!!!");
                return true;
            } else {
                Log.v(TAG, "wait for record video Ready!!!");
                return false;
            }
        }
    };

    private Utils.Checker mShutterStatusChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            //TODO:wait for add interface
            SystemClock.sleep(TIME_TEMP);
            int cntAfter = getFileCntInDir(mVideoPath);
            if (cntAfter >= mCntBefore) {
                Log.v(TAG, "record video is Ready!!!");
                return true;
            } else {
                Log.v(TAG, "wait for record video Ready!!!");
                return false;
            }
        }
    };

    private Utils.Checker mThumbnailStatusChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            //TODO:wait for add interface
            SystemClock.sleep(TIME_TEMP);
            int cntAfter = getFileCntInDir(mVideoPath);
            if (cntAfter >= mCntBefore) {
                Log.v(TAG, "record video is Ready!!!");
                return true;
            } else {
                Log.v(TAG, "wait for record video Ready!!!");
                return false;
            }
        }
    };

    private Utils.Checker mCameraStatusChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            //TODO:wait for add interface
            SystemClock.sleep(TIME_TEMP);
            int cntAfter = getFileCntInDir(mVideoPath);
            if (cntAfter >= mCntBefore) {
                Log.v(TAG, "record video is Ready!!!");
                return true;
            } else {
                Log.v(TAG, "wait for record video Ready!!!");
                return false;
            }
        }
    };


    private Utils.Checker mSwitchVideoChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            for (int i = 0; i < 3; i++) {
                swipeToVideoMode();
            }
            SystemClock.sleep(TIME_TEMP);
            return true;
        }
    };

    private static class FileFilterByName implements FilenameFilter {
        private String mFilter;

        public FileFilterByName(String filter) {
            mFilter = filter;
        }

        public boolean isRightTypeFile(String file) {
            if (file.toLowerCase(Locale.ENGLISH).endsWith(mFilter)) {
                return true;
            } else {
                return false;
            }
        }

        public boolean accept(File dir, String filename) {
            return isRightTypeFile(filename);
        }
    }

    private int getFileCntInDir(String dirName) {
        int cnt = 0;
        File fdir = new File(dirName);
        if (true == fdir.isDirectory()) {
            File[] fls = fdir.listFiles(new FileFilterByName(mFilter3gp));
            for (int i = 0; i < fls.length; i++) {
                Log.i(TAG, "" + i + fls[i].getName());
                if (true == fls[i].isFile()) {
                    cnt++;
                }
            }
            fls = fdir.listFiles(new FileFilterByName(mFiltermp4));
            for (int i = 0; i < fls.length; i++) {
                Log.i(TAG, "" + i + fls[i].getName());
                if (true == fls[i].isFile()) {
                    cnt++;
                }
            }
        }
        Log.i(TAG, "cnt: " + cnt);
        return cnt;
    }

    private void setUpCondition() {
        mContext = mInstrumentation.getTargetContext();
        AccessibilityServiceInfo info = mInstrumentation.getUiAutomation().getServiceInfo();
        info.flags |= AccessibilityServiceInfo.FLAG_RETRIEVE_INTERACTIVE_WINDOWS;
        mInstrumentation.getUiAutomation().setServiceInfo(info);
        mUiDevice = UiDevice.getInstance(mInstrumentation);
        mVideoPath = DCIM + "/Camera";
    }

    private boolean clickOnUiObject(UiObject uiObject) {
        if (uiObject.exists()) {
            try {
                uiObject.clickAndWaitForNewWindow();
            } catch (UiObjectNotFoundException e) {
                Log.i(TAG, "Failed to click UiObject");
            }
            return true;
        } else {
            return false;
        }
    }

    private void swipeToVideoMode() {
        int startX = mUiDevice.getDisplayWidth() / 2;
        int startY = mUiDevice.getDisplayHeight() / 2;
        int endX = startX - SWIPE_LENS;
        int endY = startY;
        Log.i(TAG, "startX = " + startX + ", startY = " + startY
                    + ", endX = " + endX + ", endY = " + endY);
        mUiDevice.swipe(startX, startY, endX, endY, SWIPE_STEPS);
    }
}
