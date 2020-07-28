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
package com.mediatek.camera.tests.common.normalphoto.stress;

import android.support.test.uiautomator.UiObject;
import android.support.test.uiautomator.UiSelector;

import com.mediatek.camera.tests.CameraCaptureTestCase;
import com.mediatek.camera.tests.Log;
import com.mediatek.camera.tests.Utils;
import com.mediatek.camera.tests.annotation.StressAnnotation;

/**
 * Stress test case about taking picture.
 */
public class NormalPhotoCaptureStressTestCase extends CameraCaptureTestCase {
    private static final String TAG = NormalPhotoCaptureStressTestCase.class.getSimpleName();

    private static final int CAPTURE_TIME = 50;

    @Override
    protected void setUp() throws Exception {
        Log.i(TAG, "[setUp]");
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        Log.i(TAG, "[tearDown]");
        super.tearDown();
    }

    /**
     * Repeat pause, resume activity and take picture in {@code CAPTURE_TIME}
     * times with no error.
     *
     * @throws Exception Throw exception.
     */
    @StressAnnotation
    public void testRepeatingCapture() throws Exception {
        int i = 0;
        waitCaptureStateReady();
        while (i < CAPTURE_TIME) {
            captureAndCheckSucceed();
            int sleepTime = getRandomTime();
            Log.d(TAG, "[testRepeatingCapture], time:" + i + ", sleep time:" + sleepTime);
            Thread.sleep(sleepTime);
            i++;
        }
    }

    /**
     * Repeat pause, resume activity and take picture in {@code CAPTURE_TIME}
     * times with no error.
     *
     * @throws Exception Throw exception.
     */
    @StressAnnotation
    public void testRepeatingCaptureAfterPausedAndResumed() throws Exception {
        int i = 0;
        while (i < CAPTURE_TIME) {
            pauseActivity();

            int sleepTime = getRandomTime();
            Log.d(TAG, "[testRepeatingCaptureAfterPausedAndResumed], time:" + i + "," +
                    "sleep time:" + sleepTime);
            Thread.sleep(sleepTime);
            resumeActivity();

            Thread.sleep(sleepTime);
            UiObject photoShutterUiObject = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
            photoShutterUiObject.clickAndWaitForNewWindow();
            i++;
        }
    }
}
