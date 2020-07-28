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
import com.mediatek.camera.common.device.v1.CameraDeviceManagerImpl;
import com.mediatek.camera.common.device.v2.Camera2DeviceManagerImpl;
import com.mediatek.camera.tests.CameraUnitTestCaseBase;
import com.mediatek.camera.tests.Log;
import com.mediatek.camera.tests.Utils;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

/**
 * Test CameraDeviceManagerFactory.
 */
public class CameraDeviceManagerFactoryTestCase extends CameraUnitTestCaseBase {
    private static final String TAG = CameraDeviceManagerFactoryTestCase.class.getName();

    /**
     * Test get camera device manager.
     */
    public void testGetCameraDeviceManager() {
        Log.d(TAG, "<testGetCameraDeviceManager>");
        CameraDeviceManagerFactory deviceManagerFactory =
                CameraDeviceManagerFactory.getInstance();
        // get api1 camera device manager
        CameraDeviceManager deviceManager1 = deviceManagerFactory.getCameraDeviceManager(
                mContext, CameraApi.API1);
        assertTrue("API1 can't get right camera manager!", deviceManager1.getClass().getName()
                .equals(CameraDeviceManagerImpl.class.getName()));
        // get api2 camera device manager
        CameraDeviceManager deviceManager2 = deviceManagerFactory.getCameraDeviceManager(
                mContext, CameraApi.API2);
        assertTrue("API2 can't get right camera manager!", deviceManager2.getClass().getName()
                .equals(Camera2DeviceManagerImpl.class.getName()));
        // null CameraApi
        boolean isUnsupportedException = false;
        try {
            deviceManagerFactory.getCameraDeviceManager(mContext, null);
        } catch (UnsupportedOperationException e) {
            isUnsupportedException = true;
        }
        assertTrue("Use null CameraApi, don't throw exception!", isUnsupportedException);

        // null Context, API2
        isUnsupportedException = false;
        try {
            deviceManagerFactory.getCameraDeviceManager(null, CameraApi.API2);
        } catch (UnsupportedOperationException e) {
            isUnsupportedException = true;
        }
        assertTrue("Use null context, don't throw exception", isUnsupportedException);
    }
}
