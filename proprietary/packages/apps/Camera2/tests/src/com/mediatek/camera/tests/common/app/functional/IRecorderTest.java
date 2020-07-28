/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.tests.common.app.functional;

import android.media.CamcorderProfile;

import com.mediatek.camera.common.mode.video.recorder.IRecorder;
import com.mediatek.camera.common.mode.video.recorder.IRecorder.RecorderSpec;
import com.mediatek.camera.common.mode.video.recorder.NormalRecorder;
import com.mediatek.camera.tests.CameraUnitTestCaseBase;
import com.mediatek.camera.tests.Log;

import org.mockito.Mockito;

import java.io.FileDescriptor;

/**
 * This class used for test the IRecorder.java interface.
 */
public class IRecorderTest extends CameraUnitTestCaseBase  {

    private static final String TAG = IRecorderTest.class.getSimpleName();
    private final String mFilePath = "sdcard0/DCIM/Camera/CameraCorderTest";
    private final IRecorder mIRecorder;
    private RecorderSpec mRecorderSpec;

    /**
     * Construct recorder test,and create a normal recorder.
     */
    public IRecorderTest() {
        mIRecorder = new NormalRecorder();
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mRecorderSpec = new RecorderSpec();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    /**
     *Test the recorder profile is null,need have IllegalArgumentException.
     */
    public void testRecorderProfile() {
        Log.i(TAG, "[testRecorderProfile]+");
        boolean isError = false;
        try {
            mRecorderSpec.outFileDes = new FileDescriptor();
            mIRecorder.init(mRecorderSpec);
        } catch (IllegalArgumentException e) {
            isError = true;
        }
        assertTrue(isError);
        Log.i(TAG, "[testRecorderProfile]-");
    }

    /**
     * If the recorder spec don't have recorder path and descriptor,need have a
     * IllegalArgumentException.
     * so have initialize the spec just have profile,but don't have path and descriptor,
     * expect a IllegalArgumentException happened.
     */
    public void testRecorderPathAndDescriptor() {
        Log.i(TAG, "[testRecorderPathAndDescriptor]+");
        boolean isError = false;
        try {
            mRecorderSpec.profile = Mockito.mock(CamcorderProfile.class);
            mIRecorder.init(mRecorderSpec);
        } catch (IllegalArgumentException e) {
            isError = true;
        }
        assertTrue(isError);
        Log.i(TAG, "[testRecorderPathAndDescriptor]-");
    }

    /**
     *Test the recording orientation is illegal.
     *because recording orientation expected value is 0/90/180/270,
     *so if input a illegal value such as 100,need have exception occurred.
     */
    public void testRecorderOrientationIllegal() {
        Log.i(TAG, "[testRecorderOrientationIllegal]+");
        boolean isError = false;
        mRecorderSpec.profile = Mockito.mock(CamcorderProfile.class);
        mRecorderSpec.outFileDes = new FileDescriptor();
        mRecorderSpec.orientationHint = 100;
        try {
            mIRecorder.init(mRecorderSpec);
        } catch (IllegalArgumentException e) {
            isError = true;
        }
        assertTrue("orientationHint = 100", isError);
        Log.i(TAG, "[testRecorderOrientationIllegal]-");
    }

    /**
     *Test the recording orientation is 0,because camera is null ,so will be exception.
     */
    public void testRecordingOrientationZero() {
        Log.i(TAG, "[testRecordingOrientationZero]+");
        boolean isError = false;
        prepareCorrectInitializeValue();
        mRecorderSpec.orientationHint = 0;
        try {
            mIRecorder.init(mRecorderSpec);
        } catch (IllegalArgumentException e) {
            isError = true;
        }
        assertTrue("orientationHint = 0", isError);
        Log.i(TAG, "[testRecordingOrientationZero]-");
    }

    /**
     *Test the recording orientation is 90,because camera is null ,so will be exception.
     */
    public void testRecordingOrientationReverseLandscape() {
        Log.i(TAG, "[testRecordingOrientationReverseLandscape]+");
        boolean isError = false;
        prepareCorrectInitializeValue();
        mRecorderSpec.orientationHint = 90;
        try {
            mIRecorder.init(mRecorderSpec);
        } catch (IllegalArgumentException e) {
            isError = true;
        }
        assertTrue("orientationHint = 90", isError);
        Log.i(TAG, "[testRecordingOrientationReverseLandscape]-");
    }

    /**
     *Test the recording orientation is 180,because camera is null ,so will be exception.
     */
    public void testRecordingHintReversePortrait() {
        Log.i(TAG, "[testRecordingHintReversePortrait]+");
        boolean isError = false;
        prepareCorrectInitializeValue();
        mRecorderSpec.orientationHint = 180;
        try {
            mIRecorder.init(mRecorderSpec);
        } catch (IllegalArgumentException e) {
            isError = true;
        }
        assertTrue("orientationHint = 180", isError);
        Log.i(TAG, "[testRecordingHintReversePortrait]-");
    }

    /**
     *Test the recording orientation is 270,because camera is null ,so will be exception.
     */
    public void testRecordingOrientationLandScape() {
        Log.i(TAG, "[testRecordingOrientationLandScape]+");
        boolean isError = false;
        prepareCorrectInitializeValue();
        mRecorderSpec.orientationHint = 270;
        try {
            mIRecorder.init(mRecorderSpec);
        } catch (IllegalArgumentException e) {
            isError = true;
        }
        assertTrue("orientationHint = 270", isError);
        Log.i(TAG, "[testRecordingOrientationLandScape]-");
    }

    private void prepareCorrectInitializeValue() {
        mRecorderSpec.profile = Mockito.mock(CamcorderProfile.class);
        mRecorderSpec.outFileDes = new FileDescriptor();
    }
}
