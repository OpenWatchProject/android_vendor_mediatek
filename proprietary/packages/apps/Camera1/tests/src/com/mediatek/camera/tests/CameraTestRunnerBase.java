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

import android.os.Environment;
import android.support.test.uiautomator.UiDevice;
import android.test.AndroidTestRunner;
import android.test.InstrumentationTestRunner;
import android.test.InstrumentationTestSuite;

import junit.framework.AssertionFailedError;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestListener;
import junit.framework.TestSuite;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Basic class for camera test runner. New test runner could extends this basic class.
 * All sub-classes should define own {@link #getAllTests()}
 * and {@link #getLoader()} to run test case.
 * <p>
 * You could get all tests by test suite added.
 *
 */

public class CameraTestRunnerBase extends InstrumentationTestRunner {
    private static final String TAG = CameraTestRunnerBase.class.getSimpleName();
    private static final String DCIM = Environment
            .getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM)
            .toString();
    private static final String FILE_PATH = DCIM + "/Camera/.CameraTest/";

    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new InstrumentationTestSuite(this);
        return suite;
    }

    @Override
    public ClassLoader getLoader() {
        return CameraTestRunnerBase.class.getClassLoader();
    }

    @Override
    protected AndroidTestRunner getAndroidTestRunner() {
        AndroidTestRunner testRunner = super.getAndroidTestRunner();
        testRunner.addTestListener(new TestListener() {
            @Override
            public void startTest(Test test) {
            }

            @Override
            public void endTest(Test test) {
            }

            @Override
            public void addFailure(Test test, AssertionFailedError e) {
                SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd_hh_mm_ss");
                Date date = new Date();
                String time = sdf.format(date);
                /** If case fail, save the Screenshot for analyze */
                File saveFile = new File(FILE_PATH,
                        ((TestCase) test).getName() + "_" + time + ".png");
                try {
                    saveFile.createNewFile();
                    // take Screenshot
                    UiDevice.getInstance().takeScreenshot(saveFile);
                } catch (IOException e1) {
                    e1.printStackTrace();
                }
            }

            @Override
            public void addError(Test test, Throwable t) {
            }
        });
        return testRunner;
    }
}
