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

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.os.RemoteException;
import android.support.test.uiautomator.UiDevice;
import android.support.test.uiautomator.UiObject;
import android.support.test.uiautomator.UiObjectNotFoundException;
import android.support.test.uiautomator.UiScrollable;
import android.support.test.uiautomator.UiSelector;
import android.test.InstrumentationTestCase;

import com.mediatek.camera.CameraActivity;

/**
 * Base class for camera UI automation test case. In {@link #setUp()} ,the environment for UI
 * automation test case is initialized and the camera application will be started. The environment
 * will be restored in {@link #tearDown()}.
 * <p>
 * UI related test case could extend this class for get common information.
 *
 * <p>
 * All sub-classes should call super {@link #setUp()} and {@link #tearDown()} to record test
 * duration. The duration format : CamAT_className: CaseDuration [methodName] = xs eg:
 * CamAT_DeviceManagerFactoryTest : CaseDuration [testAndroidTestCaseSetupProperly] = 0s
 *
 * <p>
 * If you prefer an functional test, see {@link com.mediatek.camera.tests.CameraTestCaseBase}.
 */
public abstract class CameraInstrumentationTestCaseBase extends InstrumentationTestCase {
    private static final String APP_NAME = "Camera";
    private static final String PROCESS_NAME = "com.mediatek.camera";
    private Long mStartTime;
    protected Activity mActivity;
    protected Instrumentation mInstrumentation;
    protected Context mContext;
    protected UiDevice mUiDevice;

    @Override
    protected void setUp() throws Exception {
        mStartTime = System.currentTimeMillis();
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        mUiDevice = UiDevice.getInstance(mInstrumentation);
    }

    @Override
    protected void tearDown() throws Exception {
        long duration = System.currentTimeMillis() - mStartTime;
        Log.i(Utils.formatClassName(getClass().getName()),
                Utils.formatDuration(getName(), duration));
        super.tearDown();
    }

    /**
     * Launch Camera app from home screen, if not exists,
     * try to start it in app list.
     * notice:
     * call it in sub-class when need to launch camera.
     * @throws UiObjectNotFoundException
     */
    protected void launchCameraApp() throws UiObjectNotFoundException {
        try {
            mUiDevice.wakeUp();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        mUiDevice.pressHome();
        UiObject cameraApp = mUiDevice.findObject(new UiSelector().text(APP_NAME));
        // click short cut
        if (cameraApp.exists()) {
            cameraApp.clickAndWaitForNewWindow();
            return;
        }
        // start from app list
        if (startApp(APP_NAME, PROCESS_NAME)) {
            return;
        }
        // start by intent.
        launchCameraByIntent();
    }

    private boolean startApp(String appName, String packageName) throws UiObjectNotFoundException {
        UiObject appsTab = mUiDevice.findObject(new UiSelector().text("apps"));
        if (!appsTab.exists()) {
            appsTab = mUiDevice.findObject(new UiSelector().descriptionContains("apps"));
        }
        if (!appsTab.exists()) {
            return false;
        }
        appsTab.click();
        UiScrollable appViews = new UiScrollable(new UiSelector().scrollable(true));

        // the appsViews will perform horizontal scrolls to find the app
        UiObject appsName = appViews.getChildByText(new UiSelector()
                .className(android.widget.TextView.class.getName()), appName);
        appsName.clickAndWaitForNewWindow();

        // create a selector for anything on the display and check if the
        // package name is the expected one
        UiObject packageValidation = mUiDevice.findObject(
                new UiSelector().packageName(packageName));
        assertTrue("Unable to start Apps: " + appName + "/packageName", packageValidation.exists());
        return true;
    }

    protected Activity launchCameraByIntent() {
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.setClass(mInstrumentation.getTargetContext(), CameraActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return mInstrumentation.startActivitySync(intent);
    }

    /**
     * Pause Activity.
     */
    protected void pauseActivity() {
        mInstrumentation.runOnMainSync(new Runnable() {
            @Override
            public void run() {
                Intent intent = new Intent(Intent.ACTION_MAIN);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.addCategory(Intent.CATEGORY_HOME);
                mInstrumentation.getTargetContext().startActivity(intent);
            }
        });
    }

    /**
     * Resume Activity.
     */
    protected void resumeActivity() {
        mInstrumentation.runOnMainSync(new Runnable() {
            @Override
            public void run() {
                Intent intent = new Intent(Intent.ACTION_MAIN);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.setClass(mInstrumentation.getTargetContext(), CameraActivity.class);
                mInstrumentation.getTargetContext().startActivity(intent);
            }
        });
    }
}