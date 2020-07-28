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
package com.mediatek.camera.tests.common.device;

import com.mediatek.camera.common.device.CameraDeviceManager;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraOpenException;
import com.mediatek.camera.common.device.v2.Camera2Proxy;
import com.mediatek.camera.tests.CameraUnitTestCaseBase;
import com.mediatek.camera.tests.Log;

import static org.mockito.Mockito.mock;

/**
 * Test CameraDeviceManager.
 */
public class CameraDeviceManagerTestCase extends CameraUnitTestCaseBase {
    private static final String TAG = CameraDeviceManagerTestCase.class.getName();

    private static final String BACK_CAMERA_ID = "0";
    private static final String FRONT_CAMERA_ID = "1";

    /**
     * Test camera API2 open camera.
     */
    public void testApi2OpenCamera() {
        CameraDeviceManagerFactory deviceManagerFactory = CameraDeviceManagerFactory.getInstance();
        CameraDeviceManager deviceManager = deviceManagerFactory.getCameraDeviceManager(
                mContext, CameraApi.API2);
        checkIllegalParameter(deviceManager, false);
        checkIllegalParameter(deviceManager, true);
    }

    private void checkIllegalParameter(CameraDeviceManager deviceManager, boolean isSync) {
        Camera2Proxy.StateCallback callback = mock(Camera2Proxy.StateCallback.class);

        boolean isNullPointerException = false;
        try {
            if (isSync) {
                deviceManager.openCameraSync(null, callback, null);
            } else {
                deviceManager.openCamera(null, callback, null);
            }
        } catch (CameraOpenException e) {
            Log.d(TAG, "<testApi2OpenCamera> exception:" + e.getExceptionType());
        } catch (NullPointerException e) {
            Log.d(TAG, "<testApi2OpenCamera> exception:" + e);
            isNullPointerException = true;
        }
        assertTrue("openCamera,null camera id don't throw NullPointerException",
                isNullPointerException);

        isNullPointerException = false;
        try {
            if (isSync) {
                deviceManager.openCameraSync(BACK_CAMERA_ID, null, null);
            } else {
                deviceManager.openCamera(BACK_CAMERA_ID, null, null);
            }

        } catch (CameraOpenException e) {
            Log.d(TAG, "<testApi2OpenCamera> exception:" + e.getExceptionType());
        } catch (NullPointerException e) {
            Log.d(TAG, "<testApi2OpenCamera> exception:" + e);
            isNullPointerException = true;
        }
        assertTrue("openCamera, null callback don't throw NullPointerException",
                isNullPointerException);
    }
}
