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

package com.android.camera.v2.bridge.device;

import android.app.Activity;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraDevice.StateCallback;
import android.hardware.camera2.CameraManager;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.Looper;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.platform.device.CameraDeviceManager;
import com.mediatek.camera.v2.platform.device.CameraDeviceProxy.CameraSessionCallback;


/**
 * The camera device manager is responsible for instantiating {@link CameraDeviceProxy}
 * instances.
 */
public class CameraDeviceManagerImpl extends CameraDeviceManager {
    private static final Tag TAG = new Tag("DeviceManagerImpl");
    private static final int OPEN_RETRY_COUNT = 1;
    private final Activity mActivity;
    private final CameraManager mCameraManager;
    private final ConditionVariable mOpenConditionVariable = new ConditionVariable();
    private int mRetryCount = 0;

    public CameraDeviceManagerImpl(CameraManager cameraManager, Activity activity) {
        mCameraManager = cameraManager;
        mActivity      = activity;
    }

    @Override
    public void open(String cameraId,
            CameraStateCallback stateCallback,
            CameraSessionCallback sessionCallback,
            Handler handler) {
        mRetryCount = 0;
        doOpenCamera(cameraId, stateCallback, sessionCallback, handler);
    }

    @Override
    public void openSync(String cameraId,
            final CameraStateCallback stateCallback,
            final CameraSessionCallback sessionCallback,
            final Handler handler) {
        if (handler.getLooper() == Looper.myLooper()) {
             throw new IllegalArgumentException("handler's looper must not be the current looper");
        }
        mOpenConditionVariable.close();
        try {
            mCameraManager.openCamera(cameraId, new StateCallback() {
                public void onOpened(CameraDevice camera) {
                    LogHelper.i(TAG, "onOpened");
                    stateCallback.onOpened(new CameraDeviceProxyImpl(
                            mActivity, camera, sessionCallback, handler));
                    mOpenConditionVariable.open();
                }
                public void onError(CameraDevice camera, int error) {
                    LogHelper.i(TAG, "onError CameraDevice:" + camera + ", error:" + error);
                    stateCallback.onError(error);
                    mOpenConditionVariable.open();
                }
                @Override
                public void onDisconnected(CameraDevice camera) {
                    LogHelper.i(TAG, "onDisconnected CameraDevice:" + camera);
                    stateCallback.onDisconnected(null);
                    mOpenConditionVariable.open();
                }
            }, handler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } finally {
            mOpenConditionVariable.block();
        }
    }

    private void doOpenCamera(final String cameraId,
            final CameraStateCallback stateCallback,
            final CameraSessionCallback sessionCallback,
            final Handler handler) {
        try {
            mCameraManager.openCamera(cameraId, new StateCallback() {
                public void onOpened(CameraDevice camera) {
                    LogHelper.i(TAG, "onOpened");
                    stateCallback.onOpened(new CameraDeviceProxyImpl(
                            mActivity, camera, sessionCallback, handler));
                }
                public void onError(CameraDevice camera, int error) {
                    LogHelper.i(TAG, "onError CameraDevice:" + camera + ", error:" + error);
                    // if camera in use, try open 1000ms later.
                    if (StateCallback.ERROR_CAMERA_IN_USE == error) {
                        if (mRetryCount < OPEN_RETRY_COUNT) {
                            mRetryCount ++;
                            try {
                                // wait some time, and try another time
                                // Camera device may be using by VT or atv.
                                Thread.sleep(1000);
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                            doOpenCamera(cameraId, stateCallback, sessionCallback, handler);
                            return;
                        }
                    }
                    stateCallback.onError(error);
                }
                @Override
                public void onDisconnected(CameraDevice camera) {
                    LogHelper.i(TAG, "onDisconnected");
                    stateCallback.onDisconnected(null);
                }
            }, handler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    @Override
    public String[] getCameraIdList() {
        String[] idList = null;
        try {
            idList = mCameraManager.getCameraIdList();
        } catch (CameraAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return idList;
    }
}