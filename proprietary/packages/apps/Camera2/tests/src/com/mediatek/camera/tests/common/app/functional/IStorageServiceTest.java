/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.tests.common.app.functional;

import com.mediatek.camera.CameraActivity;
import com.mediatek.camera.common.CameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.storage.IStorageService;
import com.mediatek.camera.tests.CameraTestCaseBase;
import com.mediatek.camera.tests.Log;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * This test file used for IStorageService function test.
 */

public class IStorageServiceTest extends CameraTestCaseBase<CameraActivity> {
    private static final String TAG = IStorageServiceTest.class.getSimpleName();
    private CameraContext mCameraContext;
    private IApp mApp;
    private IStorageService mStorageService;

    /**
     * Creator for test base.
     */
    public IStorageServiceTest() {
        super(CameraActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        Log.i(TAG, "[setUp] + ");
        super.setUp();
        mApp = (CameraActivity) mActivity;
        //TODO: should modify when camera context can get from camera activity.
        mCameraContext = new CameraContext();
        mCameraContext.create(mApp, mActivity);
        mStorageService = mCameraContext.getStorageService();
        Log.i(TAG, "[setUp] - ");
    }

    /**
     * Function test for getFileDirectory().
     * It is pass, if it can write files sucessfully in the file directory returned.
     */
    public void testGetFileDirectory() {
        Log.i(TAG, "[testGetFileDirectory]+");
        // TODO: need mock a storage, and check the path from getFileDirectory is same or not.
        boolean isCaseFail = false;
        String filePath = mStorageService.getFileDirectory() + "/.txt";
        FileOutputStream out = null;
        try {
            Log.d(TAG, "save the data to SD Card");
            out = new FileOutputStream(filePath);
            out.write(new byte[1]);
            out.close();
        } catch (IOException e) {
            isCaseFail = true;
            Log.e(TAG, "Failed to write image,ex:", e);
        } finally {
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    isCaseFail = true;
                    Log.e(TAG, "IOException:", e);
                }
            }
        }
        assertFalse(isCaseFail);
        Log.i(TAG, "[testGetFileDirectory]-");
    }

    // TODO: need add other test that mock storage listener and mock broadcast.
    /**
     * Function test for registerStorageStateListener().
     * It is pass when there is no exception after register a null listener.
     */
    public void testRegisterStorageStateListener() {
        Log.i(TAG, "[testRegisterStorageStateListener]+");
        boolean isCaseFail = false;
        try {
            mStorageService.registerStorageStateListener(null);
        } catch (Exception e) {
            isCaseFail = true;
        }
        assertFalse(isCaseFail);
        Log.i(TAG, "[testRegisterStorageStateListener]-");
    }

    /**
     * Function test for unRegisterStorageStateListener().
     * It is pass when there is no exception after unRegister a null listener.
     */
    public void testUnRegisterStorageStateListener() {
        Log.i(TAG, "[testUnRegisterStorageStateListener]+");
        boolean isCaseFail = false;
        try {
            mStorageService.unRegisterStorageStateListener(null);
        } catch (Exception e) {
            isCaseFail = true;
        }
        assertFalse(isCaseFail);
        Log.i(TAG, "[testUnRegisterStorageStateListener]-");
    }
}
