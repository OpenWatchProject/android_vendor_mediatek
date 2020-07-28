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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.android.camera.bridge;

import com.android.camera.CameraHolder;

import android.app.Activity;
import android.hardware.Camera.CameraInfo;

import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.util.Log;

import junit.framework.Assert;

public class CameraDeviceManagerImpl implements ICameraDeviceManager {
    private static final String TAG = "CameraDeviceManagerImpl";

    public static final int UNKNOWN = -1;

    // private static final int OPEN_RETRY_COUNT = 2;

    private CameraHolder mCameraHolder;
    private Activity mActivity;
    private CameraDeviceCtrl mCameraDeviceCtrl;
    private ICameraDevice mICurCameraDevice = null;
    private ICameraDevice mITopCameraDevice = null;

    public CameraDeviceManagerImpl(Activity activity, CameraDeviceCtrl cameraDeviceCtrl) {
        Assert.assertNotNull(activity);
        Assert.assertNotNull(cameraDeviceCtrl);
        mActivity = activity;
        mCameraDeviceCtrl = cameraDeviceCtrl;
        mCameraHolder = cameraDeviceCtrl.getCameraHolder();
    }

    @Override
    public void onCameraCloseDone() {
        mICurCameraDevice = null;
        mITopCameraDevice = null;
    }

    @Override
    public int getBackCameraId() {
        return mCameraHolder.getBackCameraId();
        // return mCameraDeviceCtrl.getCurCameraDevice().getCameraId();
    }

    @Override
    public int getFrontCameraId() {
        return mCameraHolder.getFrontCameraId();
        // return mCameraDeviceCtrl.getTopCameraDevice().getCameraId();
    }

    @Override
    public int getCurrentCameraId() {
        return mCameraDeviceCtrl.getCameraId();
    }

    @Override
    public CameraInfo getCameraInfo(int cameraId) {
        return mCameraHolder.getCameraInfo()[cameraId];
    }

    @Override
    public CameraInfo[] getCameraInfo() {
        return mCameraHolder.getCameraInfo();
    }

    @Override
    public int getNumberOfCameras() {
        return mCameraHolder.getNumberOfCameras();
    }

    @Override
    public ICameraDevice getCameraDevice(int cameraId) {
        if (UNKNOWN == cameraId) {
            Log.w(TAG, "[getCameraDevice] cameraId is UNKNOWN,return!");
            return null;
        }

        ICameraDeviceExt curCameraDevice = mCameraDeviceCtrl.getCurCameraDevice();
        ICameraDeviceExt topCameraDevice = mCameraDeviceCtrl.getTopCameraDevice();
        int curCameraId = curCameraDevice.getCameraId();
        int topCameraId = topCameraDevice.getCameraId();

        if (mICurCameraDevice == null || curCameraId != mICurCameraDevice.getCameraId()) {
            Log.d(TAG, "[getCameraDevice] new mICurCameraDevice.curCameraId:" + curCameraId);
            mICurCameraDevice = new CameraDeviceImpl(mActivity, curCameraDevice);
        }
        if (mITopCameraDevice == null || topCameraId != mITopCameraDevice.getCameraId()) {
            Log.d(TAG, "[getCameraDevice] new mITopCameraDevice.topCameraId:" + topCameraId);
            mITopCameraDevice = new CameraDeviceImpl(mActivity, topCameraDevice);
        }

        if (mICurCameraDevice.getCameraId() == cameraId) {
            return mICurCameraDevice;
        } else if (mITopCameraDevice.getCameraId() == cameraId) {
            return mITopCameraDevice;
        } else {
            Log.w(TAG, "[getCameraDevice]return null,cameraId = " + cameraId
                    + ",mICurCameraDevice.getCameraId() = " + mICurCameraDevice.getCameraId()
                    + ",mITopCameraDevice.getCameraId() = " + mITopCameraDevice.getCameraId());
            return null;
        }
    }
}
