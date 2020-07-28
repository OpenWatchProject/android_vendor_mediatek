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

package com.mediatek.camera.tests.common.app.functional;

import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.hardware.Camera;

import com.mediatek.camera.DisableCameraReceiver;
import com.mediatek.camera.tests.CameraUnitTestCaseBase;
import com.mediatek.camera.tests.Log;

import java.util.List;

/**
 * Test class for DisableCameraReceiver.
 */
public class DisableCameraReceiverTest extends CameraUnitTestCaseBase {
    private static final String TAG = DisableCameraReceiverTest.class.getSimpleName();
    private static final String BOOTUP = "android.intent.action.BOOT_COMPLETED";
    private DisableCameraReceiver mDisableCameraReceiver;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mDisableCameraReceiver = new DisableCameraReceiver();
    }

    /**
     * Test power on broad cast with no exception.
     *
     * @throws Exception test with exception.
     */
    public void testBootCompleteBroadcast() throws Exception {
        Intent bootUpIntent = new Intent(BOOTUP);
        mDisableCameraReceiver.onReceive(getContext(), bootUpIntent);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            Log.e(TAG, "testBootCompleteBroadcast with InterruptedException!!!");
        }
    }

    /**
     * If current device has no camera device, should not find mediatek camera package.
     * or else can find mediatek camera pacakge.
     *
     * @throws Exception test with exception.
     */
    public void testCameraPackageByCameraNumber() throws Exception {
        int cameraNumber = Camera.getNumberOfCameras();

        if (cameraNumber > 0) {
            assertTrue("Has camera device, but can't find" +
                    "com.mediatek.camera package!!!", hasMediatekCameraPackage());
        } else {
            assertFalse("Has no camera device, but can find" +
                    "com.mediatek.camera package!!!", hasMediatekCameraPackage());
        }
    }

    private boolean hasMediatekCameraPackage() {
        boolean hasCameraPackage = false;
        PackageManager packageManager = getContext().getPackageManager();
        List<PackageInfo> packageInfoList = packageManager.getInstalledPackages(0);
        for (PackageInfo info: packageInfoList) {
            String packageName = info.packageName;
            if (packageName != null && packageName.equalsIgnoreCase("com.mediatek.camera")) {
                hasCameraPackage = true;
                break;
            }
        }
        return hasCameraPackage;
    }
}