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

import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.InputConfiguration;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.view.Surface;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;
import com.mediatek.camera.common.device.v2.Camera2Proxy;
import com.mediatek.camera.tests.CameraUnitTestCaseBase;
import com.mediatek.camera.tests.Log;
import com.mediatek.camera.tests.Utils;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

/**
 * Test camera2 proxy.
 */
public class Camera2ProxyTestCase extends CameraUnitTestCaseBase {
    private static final String TAG = Camera2ProxyTestCase.class.getSimpleName();

    private static final String BACK_CAMERA_ID = "0";
    private Camera2Proxy mCamera2Proxy = null;
    private Handler mRespondHandler;
    protected HandlerThread mRequestThread;
    private CameraCaptureSession.StateCallback mSessionStateCallback;

    /**
     * Test get correct camera id.
     */
    public void testGetId() {
        CameraDevice device = mock(CameraDevice.class);
        createCameraProxy(device);

        String cameraId = mCamera2Proxy.getId();
        assertTrue("exception, get wrong camera id!", cameraId.equals(BACK_CAMERA_ID));
    }

    /**
     * Test create capture session.
     */
    public void testCreateCaptureSession() {
        CameraDevice device = mock(CameraDevice.class);
        createCameraProxy(device);
        Camera2CaptureSessionProxy.StateCallback callback =
                new Camera2CaptureSessionProxy.StateCallback() {
            @Override
            public void onConfigured(Camera2CaptureSessionProxy session) {
            }

            @Override
            public void onConfigureFailed(Camera2CaptureSessionProxy session) {
            }
        };
        Handler handler = mock(Handler.class);
        List<Surface> surfaceList = mock(ArrayList.class);
        try {
            mCamera2Proxy.createCaptureSession(surfaceList, callback, handler);
        } catch (CameraAccessException e) {
            Log.d(TAG, "camera proxy createCaptureSession:" + e);
        }
        try {
            verify(device, timeout(Utils.TIME_OUT_MS).times(1)).createCaptureSession(surfaceList,
                    mSessionStateCallback, mRespondHandler);
        } catch (CameraAccessException e) {
            Log.d(TAG, "mock device createCaptureSession:" + e);
        }
    }

    /**
     * Test create reprocessable capture session.
     */
    public void testCreateReprocessableCaptureSession() {
        if (!isMNewerSdkVersion()) {
            Log.d(TAG, "<testCreateReprocessableCaptureSession> current sdk version too old!");
            return;
        }
        CameraDevice device = mock(CameraDevice.class);
        createCameraProxy(device);

        InputConfiguration config = new InputConfiguration(0, 0, 0);
        List<Surface> outputs = mock(ArrayList.class);
        Camera2CaptureSessionProxy.StateCallback callback =
                mock(Camera2CaptureSessionProxy.StateCallback.class);
        Handler handler = mock(Handler.class);

        try {
            mCamera2Proxy.createReprocessableCaptureSession(config, outputs, callback, handler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        try {
            verify(device, timeout(Utils.TIME_OUT_MS).times(1)).createReprocessableCaptureSession(
                    config, outputs, mSessionStateCallback, mRespondHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Test create constrained high speed capture session.
     */
    public void testCreateConstrainedHighSpeedCaptureSession() {
        if (!isMNewerSdkVersion()) {
            Log.d(TAG, "<testCreateConstrainedHighSpeedCaptureSession>" +
                    "current sdk version too old!");
            return;
        }
        CameraDevice device = mock(CameraDevice.class);
        createCameraProxy(device);
        List<Surface> outputs = mock(ArrayList.class);
        Camera2CaptureSessionProxy.StateCallback callback =
                new Camera2CaptureSessionProxy.StateCallback() {
            @Override
            public void onConfigured(Camera2CaptureSessionProxy session) {
            }

            @Override
            public void onConfigureFailed(Camera2CaptureSessionProxy session) {
            }
        };
        Handler handler = mock(Handler.class);

        try {
            mCamera2Proxy.createConstrainedHighSpeedCaptureSession(outputs, callback, handler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        try {
            verify(device, timeout(Utils.TIME_OUT_MS).times(1))
                    .createConstrainedHighSpeedCaptureSession(outputs, mSessionStateCallback,
                            mRespondHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Test create capture request.
     */
    public void testCreateCaptureRequest() {
        CameraDevice device = mock(CameraDevice.class);
        createCameraProxy(device);

        try {
            when(device.createCaptureRequest(0)).thenReturn(null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        CaptureRequest.Builder res = null;
        try {
            res = mCamera2Proxy.createCaptureRequest(0);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        try {
            verify(device).createCaptureRequest(0);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        assertTrue(res == null);
    }

    /**
     * Test create reprocess capture request.
     */
    public void testCreateReprocessCaptureRequest() {
        if (!isMNewerSdkVersion()) {
            Log.d(TAG, "<testCreateReprocessCaptureRequest>" + "current sdk version too old!");
            return;
        }
        CameraDevice device = mock(CameraDevice.class);
        createCameraProxy(device);
        try {
            when(device.createReprocessCaptureRequest(null)).thenReturn(null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        boolean isNullPointerException = false;
        CaptureRequest.Builder res = null;
        try {
            res = mCamera2Proxy.createReprocessCaptureRequest(null);
        } catch (CameraAccessException e) {
            Log.d(TAG, "createReprocessCaptureRequest:" + e);
        } catch (NullPointerException e) {
            isNullPointerException = true;
        }
        assertTrue(isNullPointerException);
    }

    /**
     * Test close camera2 proxy.
     */
    public void testClose() {
        CameraDevice device = mock(CameraDevice.class);
        createCameraProxy(device);

        mCamera2Proxy.close();
        verify(device).close();
    }

    private void createCameraProxy(CameraDevice device) {
        mRequestThread = new HandlerThread(CameraApi.API2 + "-Request-" + BACK_CAMERA_ID);
        mRequestThread.start();
        mRespondHandler = new Handler(mRequestThread.getLooper());
        try {
            Class handlerClass = Class
                    .forName("com.mediatek.camera.common.device.v2.Camera2Handler");
            try {
                Constructor handlerCon = handlerClass.getDeclaredConstructor(String.class,
                        Looper.class, Handler.class, CameraDevice.class);
                handlerCon.setAccessible(true);
                try {
                    Object camera2Handler = handlerCon.newInstance(BACK_CAMERA_ID,
                            mRequestThread.getLooper(), mRespondHandler, device);
                    Log.d(TAG, "create Camera2Handler");
                    Constructor proxyCon = Camera2Proxy.class.getDeclaredConstructor(String.class,
                            CameraDevice.class, camera2Handler.getClass(), Handler.class);
                    proxyCon.setAccessible(true);
                    mCamera2Proxy = (Camera2Proxy) proxyCon.newInstance(BACK_CAMERA_ID, device,
                            camera2Handler, mRespondHandler);
                    Log.d(TAG, "create Camera2Proxy");

                    Method updateCamera2Proxy = handlerClass.getDeclaredMethod(
                            "updateCamera2Proxy", Camera2Proxy.class);
                    updateCamera2Proxy.setAccessible(true);
                    updateCamera2Proxy.invoke(camera2Handler, mCamera2Proxy);
                    Log.d(TAG, "invoke Camera2Proxy.updateCamera2Proxy");
                    try {
                        Field field = handlerClass.getDeclaredField("mSessionStateCallback");
                        field.setAccessible(true);
                        mSessionStateCallback = (CameraCaptureSession.StateCallback) field
                                .get(camera2Handler);
                        Log.d(TAG, "set Camera2Handler mSessionStateCallback");
                    } catch (NoSuchFieldException e) {
                        Log.d(TAG, "getDeclaredField:" + e);
                    }
                } catch (InstantiationException e) {
                    Log.d(TAG, "newInstance:" + e);
                } catch (IllegalAccessException e) {
                    Log.d(TAG, "newInstance:" + e);
                } catch (IllegalArgumentException e) {
                    Log.d(TAG, "newInstance:" + e);
                } catch (InvocationTargetException e) {
                    Log.d(TAG, "newInstance:" + e);
                }
            } catch (NoSuchMethodException e) {
                Log.d(TAG, "getConstructor:" + e);
            } catch (SecurityException e) {
                Log.d(TAG, "getConstructor:" + e);
            }
        } catch (ClassNotFoundException e) {
            Log.d(TAG, "Class.forName:" + e);
        }
    }

    private boolean isMNewerSdkVersion () {
        return android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M;
    }
}
