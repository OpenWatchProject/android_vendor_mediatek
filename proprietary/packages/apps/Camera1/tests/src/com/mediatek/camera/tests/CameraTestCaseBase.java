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
import android.test.ActivityInstrumentationTestCase2;

/**
 * Basic class for camera functional test case. New functional tests could extends this basic class.
 * The activity under test will be created using the system infrastructure (by calling
 * getActivity()) and all the subClasses should manipulate the Activity directly.
 * <p>
 * The activity under test will be created in {@link #setUp()} and finished in {@link #tearDown()}.
 * <p>
 * All sub-classes should call super {@link #setUp()} and {@link #tearDown()} to record test
 * duration. The duration format : CamAT_className: CaseDuration [methodName] = xs eg:
 * CamAT_DeviceManagerFactoryTest : CaseDuration [testAndroidTestCaseSetupProperly] = 0s
 * <p>
 * Other options supported by this test case include:
 * <ul>
 * <li>You can run any test method on the UI thread (see {@link android.test.UiThreadTest}).</li>
 * <li>
 * You can inject custom Intents into your Activity (see {@link #setActivityIntent(Intent)}).</li>
 * </ul>
 * <p>
 * If you prefer an UI test,see {@link com.mediatek.camera.tests.CameraInstrumentationTestCaseBase}.
 *
 * @param <T>
 *            The activity which been test.
 */

public abstract class CameraTestCaseBase<T extends Activity> extends
        ActivityInstrumentationTestCase2<T> {
    private Long mStartTime;
    protected Activity mActivity;
    protected Instrumentation mInstrumentation;

    /**
     * Creator for test base.
     *
     * @param activityClass
     *            The activity to test. This must be a class in the instrumentation targetPackage
     *            specified in the AndroidManifest.xml
     */
    public CameraTestCaseBase(Class<T> activityClass) {
        super(activityClass);
    }

    @Override
    protected void setUp() throws Exception {
        mStartTime = System.currentTimeMillis();
        mInstrumentation = getInstrumentation();
        mActivity = getActivity();
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        long duration = System.currentTimeMillis() - mStartTime;
        Log.i(Utils.formatClassName(getClass().getName()),
                Utils.formatDuration(getName(), duration));
    }
}