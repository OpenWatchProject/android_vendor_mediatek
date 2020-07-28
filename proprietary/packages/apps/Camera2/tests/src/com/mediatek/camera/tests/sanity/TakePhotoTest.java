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
 * A class that tests Camera sanity about taking photo.
 */
public class TakePhotoTest extends CameraTestCaseBase<CameraActivity> {
    private static final String TAG = "TakePhotoTest";
    private static final String DCIM = Environment.getExternalStoragePublicDirectory(
            Environment.DIRECTORY_DCIM).toString();
    private static final int TIME_OUT_MS = 60000; // 60sec
    private static final int TIME_TEMP = 10000;
    private Context mContext = null;
    private UiDevice mUiDevice = null;
    private UiObject mPhotoShutterBtn;
    private UiObject mThumbnailBtn;
    private UiObject mSwitcher;

    private String filter = ".jpg";
    private String mPhotoPath;
    private int mCntBefore = 0;

    /**
     * constructor.
     */
    public TakePhotoTest() {
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
     * test take photo.
     * Step 1: take main sensor begin
     * Step 2: click photo button
     * Step 3: check capture image number
     * Step 4: take sub sensor begin
     * Step 5: switch to sub sensor
     * Step 6: go to gallery
     * @Exception throw exception
     */
    public void testTakePhoto() throws Exception {
        Log.v(TAG, "testTakephoto start");
        // get image num before capture
        mCntBefore = getFileCntInDir(mPhotoPath, filter);
        // Step 1: Take main sensor capture
        Log.v(TAG, "takeMainSensor!");
        // wait for preview idle and view ready
        Log.v(TAG, "checkViewStatus");
        assertStatusReady(mShutterStatusChecker, TIME_OUT_MS);
        assertStatusReady(mCameraStatusChecker, TIME_OUT_MS);
        // Step 2: click shutter button
        Log.v(TAG, "click On Photo button!");
        mPhotoShutterBtn = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
        clickOnUiObject(mPhotoShutterBtn);
        // wait for preview idle and view ready
        Log.v(TAG, "checkViewStatus");
        assertStatusReady(mShutterStatusChecker, TIME_OUT_MS);
        assertStatusReady(mCameraStatusChecker, TIME_OUT_MS);
        Log.v(TAG, "takeMainSensor end!");
        // Step 3: check capture image number
        assertStatusReady(mImageNumChecker, TIME_OUT_MS);
        // Step 4: take sub sensor begin
        Log.v(TAG, "takeSubSendor begin");
        // Step 5: switch to sub sensor
        mSwitcher = mUiDevice.findObject(
                new UiSelector().resourceId(Utils.SWITCH_BUTTON_ID));
        clickOnUiObject(mSwitcher);
        // wait for preview idle and view ready
        Log.v(TAG, "checkViewStatus");
        assertStatusReady(mShutterStatusChecker, TIME_OUT_MS);
        assertStatusReady(mCameraStatusChecker, TIME_OUT_MS);
        Log.v(TAG, "click On Photo button!");
        mPhotoShutterBtn = new UiObject(Utils.SHUTTER_BUTTON_SELECTOR);
        clickOnUiObject(mPhotoShutterBtn);
        Log.v(TAG, "takeSubSendor end");
        // Step 6: go to gallery
        Log.v(TAG, "gotoGallery start");
        assertStatusReady(mThumbnailStatusChecker, TIME_OUT_MS);
        mThumbnailBtn = mUiDevice.findObject(
                new UiSelector().resourceId(Utils.THUMBNAIL_RES_ID));
        clickOnUiObject(mThumbnailBtn);
        Log.v(TAG, "testTakephoto end");
    }

    private void assertStatusReady(Checker checker, int timeOutMs) {
        Assert.assertEquals(Utils.waitForTrueWithTimeOut(checker, timeOutMs), false);
    }

    private Utils.Checker mImageNumChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            int cntAfter = getFileCntInDir(mPhotoPath, filter);
            if (cntAfter >= mCntBefore + 1) {
                Log.v(TAG, "capture image is Ready!!!");
                return true;
            } else {
                Log.v(TAG, "wait for capture image Ready!!!");
                return false;
            }
        }
    };

    private Utils.Checker mShutterStatusChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            //TODO:wait for add interface
            SystemClock.sleep(TIME_TEMP);
            int cntAfter = getFileCntInDir(mPhotoPath, filter);
            if (cntAfter >= mCntBefore) {
                Log.v(TAG, "capture image is Ready!!!");
                return true;
            } else {
                Log.v(TAG, "wait for capture image Ready!!!");
                return false;
            }
        }
    };

    private Utils.Checker mThumbnailStatusChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            //TODO:wait for add interface
            SystemClock.sleep(TIME_TEMP);
            int cntAfter = getFileCntInDir(mPhotoPath, filter);
            if (cntAfter >= mCntBefore) {
                Log.v(TAG, "capture image is Ready!!!");
                return true;
            } else {
                Log.v(TAG, "wait for thumbnail Ready!!!");
                return false;
            }
        }
    };

    private Utils.Checker mCameraStatusChecker = new Utils.Checker() {
        @Override
        public boolean check() {
            //TODO:wait for add interface
            SystemClock.sleep(TIME_TEMP);
            int cntAfter = getFileCntInDir(mPhotoPath, filter);
            if (cntAfter >= mCntBefore) {
                Log.v(TAG, "capture image is Ready!!!");
                return true;
            } else {
                Log.v(TAG, "wait for capture image Ready!!!");
                return false;
            }
        }
    };

    /**
     * File filter class by tag.
     */
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

    private int getFileCntInDir(String dirName, String filter) {
        int cnt = 0;
        File fdir = new File(dirName);
        if (true == fdir.isDirectory()) {
            File[] fls = fdir.listFiles(new FileFilterByName(filter));
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
        mPhotoPath = DCIM + "/Camera";
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
}
