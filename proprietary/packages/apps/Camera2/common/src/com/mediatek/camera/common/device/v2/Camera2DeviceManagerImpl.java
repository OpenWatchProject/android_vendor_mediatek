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

package com.mediatek.camera.common.device.v2;

import android.annotation.TargetApi;
import android.app.admin.DevicePolicyManager;
import android.content.Context;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraDevice.StateCallback;
import android.hardware.camera2.CameraManager;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManager;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraOpenException;
import com.mediatek.camera.common.device.CameraOpenException.ExceptionType;
import com.mediatek.camera.common.device.CameraStateCallback;
import com.mediatek.camera.common.utils.CameraUtil;

import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * An implement of CameraDeviceManager with android.hardware.camera2.
 */
public class Camera2DeviceManagerImpl extends CameraDeviceManager {
    private static final Tag TAG = new Tag("DeviceMgr2");
    private final CameraManager mCameraManager;
    private final Context mContext;
    ConcurrentHashMap<String, Camera2ProxyCreatorImpl> mProxyCreatorMap = new ConcurrentHashMap<>();

    /**
     * CameraDeviceManager implement for camera2.
     * @param context the context used to initialize CameraManager.
     */
    public Camera2DeviceManagerImpl(Context context) {
        mContext = context;
        mCameraManager = (CameraManager) context.getSystemService(Context.CAMERA_SERVICE);
    }

    @Override
    public void openCamera(@Nonnull String cameraId, @Nonnull CameraStateCallback callback,
                    @Nullable Handler handler) throws CameraOpenException {
        LogHelper.i(TAG, "[openCamera] cameraId = " + cameraId);
        checkPreconditionsAndOpen(cameraId, callback, false);
    }

    @Override
    public void openCameraSync(@Nonnull String cameraId, @Nonnull CameraStateCallback callback,
                    @Nullable Handler handler) throws CameraOpenException {
        LogHelper.i(TAG, "[openCameraSync] cameraId = " + cameraId);
        checkPreconditionsAndOpen(cameraId, callback, true);
    }

    @Override
    public void close(@Nonnull String cameraId) {
        Camera2ProxyCreatorImpl proxyCreator = mProxyCreatorMap.get(cameraId);
        if(proxyCreator != null) {
            proxyCreator.doCloseCamera();
        }
    }

    @Override
    public void closeSync(@Nonnull String cameraId) {
        Camera2ProxyCreatorImpl proxyCreator = mProxyCreatorMap.get(cameraId);
        if(proxyCreator != null) {
            proxyCreator.doCloseCameraSync();
        }
    }

    @Override
    public void recycle() {
        LogHelper.i(TAG, "[recycle]");
        Iterator iterator = mProxyCreatorMap.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry e = (Map.Entry) iterator.next();
            Camera2ProxyCreatorImpl cameraProxyCreator = (Camera2ProxyCreatorImpl) e.getValue();
            cameraProxyCreator.destroyHandlerThreads();
        }
        mProxyCreatorMap.clear();
    }

    private void checkPreconditionsAndOpen(String cameraId, CameraStateCallback callback,
                    boolean isSync) throws CameraOpenException {
        checkDevicePolicy();
        LogHelper.d(TAG, "[checkPreconditions] mProxyCreatorMap size = " + mProxyCreatorMap.size());
        Camera2ProxyCreatorImpl proxyCreator = mProxyCreatorMap.get(cameraId);
        if (proxyCreator == null) {
            proxyCreator = new Camera2ProxyCreatorImpl(cameraId);
            mProxyCreatorMap.put(cameraId, proxyCreator);
        }
        if (isSync) {
            proxyCreator.doOpenCameraSync(callback);
        } else {
            proxyCreator.doOpenCamera(callback);
        }
    }

    private void checkDevicePolicy() throws CameraOpenException {
        DevicePolicyManager dpm = (DevicePolicyManager) mContext
                .getSystemService(Context.DEVICE_POLICY_SERVICE);
        if (dpm.getCameraDisabled(null)) {
            throw new CameraOpenException(ExceptionType.SECURITY_EXCEPTION);
        }
    }

    /**
     * Camera2 proxy creator implement.
     */
    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private class Camera2ProxyCreatorImpl extends CameraProxyCreator {
        private final Handler mRespondHandler;
        private final RespondCameraHandler mOpenHandler;
        private final Tag mHandlerTag;
        private final Tag mRespondTag;

        Camera2ProxyCreatorImpl(String cameraId) {
            super(CameraApi.API2, cameraId);
            mThreadLock.lock();
            mRespondHandler = new RespondCameraHandler(mRespondThread.getLooper());
            mOpenHandler = new RespondCameraHandler(mRequestThread.getLooper());
            mThreadLock.unlock();
            mHandlerTag = new Tag("API2-De-Handler-" + cameraId);
            mRespondTag = new Tag("API2-De-Respond-" + cameraId);
        }

        private void doOpenCamera(CameraStateCallback callback) {
            mOpenHandler.removeMessages(RespondCameraHandler.MSG_CLOSE_CAMERA);
            mOpenHandler.obtainMessage(
                    RespondCameraHandler.MSG_OPEN_CAMERA, callback).sendToTarget();
        }

        private void doOpenCameraSync(CameraStateCallback callback) {
            mOpenHandler.removeMessages(RespondCameraHandler.MSG_CLOSE_CAMERA);
            mOpenHandler.obtainMessage(
                    RespondCameraHandler.MSG_OPEN_CAMERA, callback).sendToTarget();
            waitDone();
        }

        private void doCloseCamera() {
            mOpenHandler.clearOpenOperation();
            mOpenHandler.obtainMessage(RespondCameraHandler.MSG_CLOSE_CAMERA).sendToTarget();
        }

        private void doCloseCameraSync() {
            mOpenHandler.clearOpenOperation();
            mOpenHandler.obtainMessage(RespondCameraHandler.MSG_CLOSE_CAMERA).sendToTarget();
            waitDone();
        }
        /**
         * Camera device respond handler.
         */
        private class RespondCameraHandler extends Handler {
            static final int MSG_OPEN_CAMERA = 0;
            static final int MSG_CLOSE_CAMERA = 1;
            private Camera2Handler mRequestHandler;
            private Camera2Proxy mCamera2Proxy;
            private CameraDevice mCameraDevice;
            private Object mOpenStateCallbackSync = new Object();
            private volatile Camera2Proxy.StateCallback mOpenStateCallback;
            private Object mOpenLock = new Object();
            private boolean mIsOnOpenCallback = false;
            /**
             * Create camera device respond handler.
             * @param looper the respond looper.
             */
            RespondCameraHandler(Looper looper) {
                super(looper);
            }

            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                switch (msg.what) {
                    case MSG_OPEN_CAMERA:
                        LogHelper.d(TAG, "handle open camera msg +");
                        Camera2Proxy.StateCallback stateCallback
                                = (Camera2Proxy.StateCallback) msg.obj;
                        mOpenStateCallback = stateCallback
                                == null ? new DummyCameraStateCallback() : stateCallback;
                        if (mCamera2Proxy == null) {
                            openCamera();
                        }
                        synchronized (mOpenStateCallbackSync) {
                            if (mCameraDevice != null && mOpenStateCallback != null) {
                                mOpenStateCallback.onOpened(mCamera2Proxy);
                            }
                        }

                        LogHelper.d(TAG, "handle open camera msg -");
                        break;

                    case MSG_CLOSE_CAMERA:
                        LogHelper.d(TAG, "handle close camera msg +");
                        if(mCamera2Proxy != null && mCameraDevice != null) {
                            //mCamera2Proxy.close will set camera device to null in camera2handler
                            // this operation will prevent any other operation after camera closed.
                            mRequestHandler.closeCamera();
                            mCameraDevice.close();
                            mCameraDevice = null;
                            mCamera2Proxy = null;
                        }
                        LogHelper.d(TAG, "handle close camera msg -");
                        break;
                    default:
                        break;
                }
            }

            private void openCamera() {
                try {
                    mIsOnOpenCallback = false;
                    mCameraManager.openCamera(mCameraId, mStateCallback, mRespondHandler);
                    waitStateCallback();
                } catch (Exception e) {
                    LogHelper.e(mHandlerTag, "[openCamera] error:" + e.getMessage());
                    doOpenException();
                }
            }

            private void doOpenException() {
                if (isNeedRetryOpen()) {
                    try {
                        Thread.sleep(RETRY_OPEN_SLEEP_TIME_MS);
                    } catch (InterruptedException ee) {
                        ee.printStackTrace();
                    }
                    LogHelper.e(mHandlerTag, "[doOpenException] retry time: " + mRetryCount);
                    openCamera();
                    return;
                }
                LogHelper.w(mHandlerTag, "[doOpenException] result with exception!");
                mOpenStateCallback.onError(new Camera2Proxy(mCameraId, mCameraDevice,
                        mRequestHandler, mRespondHandler), CameraUtil.CAMERA_OPEN_FAIL);
            }

            private boolean isNeedRetryOpen() {
                if (mRetryCount < OPEN_RETRY_COUNT) {
                    mRetryCount++;
                    return true;
                }
                return false;
            }

            private Camera2Handler.IDeviceInfoListener mDeviceInfoListener =
                    new Camera2Handler.IDeviceInfoListener() {
                        @Override
                        public void onClosed() {
                        }

                        @Override
                        public void onError() {
                            LogHelper.i(mRespondTag, "[onError]");
                            mStateCallback.onError(
                                    mCameraDevice, StateCallback.ERROR_CAMERA_IN_USE);
                        }
                    };

            private final StateCallback mStateCallback = new StateCallback() {
                @Override
                public void onOpened(CameraDevice camera) {
                    LogHelper.i(mRespondTag, "[onOpened] camera = " + camera);
                    mRetryCount = 0;
                    createHandlerAndProxy(camera);
                    notifyStateCallback();
                }

                @Override
                public void onClosed(CameraDevice camera) {
                    super.onClosed(camera);
                    if (mCameraDevice != null && mCameraDevice == camera) {
                        LogHelper.d(mRespondTag, "[onClosed] camera = " + camera);
                        mOpenStateCallback.onClosed(mCamera2Proxy);
                        mCameraDevice = null;
                        mCamera2Proxy = null;
                    }
                }

                @Override
                public void onError(CameraDevice camera, final int error) {
                    LogHelper.e(mRespondTag, "[onError] camera = " + camera + " error = " + error);
                    notifyStateCallback();
                    mOpenStateCallback.onError(mCamera2Proxy, error);
                    if (mRequestHandler != null) {
                        mRequestHandler.closeCamera();
                    }
                    if (mCameraDevice != null) {
                        mCameraDevice.close();
                    }
                    mCameraDevice = null;
                    mCamera2Proxy = null;
                }

                @Override
                public void onDisconnected(CameraDevice camera) {
                    LogHelper.d(mRespondTag, "[onDisconnected] camera = " + camera);
                    if (mCameraDevice != null && mCameraDevice == camera) {
                        mOpenStateCallback.onDisconnected(mCamera2Proxy);
                        mCameraDevice = null;
                        mCamera2Proxy = null;
                    }

                }
            };

            private void createHandlerAndProxy(CameraDevice camera) {
                if (camera != mCameraDevice) {
                    mCameraDevice = camera;
                    mThreadLock.lock();
                    mRequestHandler = new Camera2Handler(mCameraId, mRequestThread.getLooper(),
                            mRespondHandler, camera, mDeviceInfoListener);
                    mThreadLock.unlock();
                    mCamera2Proxy = new Camera2Proxy(
                            mCameraId, camera, mRequestHandler, mRespondHandler);
                    mRequestHandler.updateCamera2Proxy(mCamera2Proxy);
                }
            }

            public void clearOpenOperation() {
                synchronized (mOpenStateCallbackSync) {
                    if (mOpenStateCallback != null) {
                        mOpenStateCallback = new DummyCameraStateCallback();
                    }
                }
                removeMessages(MSG_OPEN_CAMERA);
            }

            private void waitStateCallback() throws InterruptedException {
                if (!mIsOnOpenCallback) {
                    synchronized (mOpenLock) {
                        if (!mIsOnOpenCallback) {
                            mOpenLock.wait();
                        }
                    }
                }
            }

            private void notifyStateCallback() {
                if (!mIsOnOpenCallback) {
                    synchronized (mOpenLock) {
                        if (!mIsOnOpenCallback) {
                            mIsOnOpenCallback = true;
                            mOpenLock.notifyAll();
                        }
                    }
                }
            }

        }


        private boolean waitDone() {
            final Object waitDoneLock = new Object();
            final Runnable unlockRunnable = new Runnable() {
                @Override
                public void run() {
                    synchronized (waitDoneLock) {
                        waitDoneLock.notifyAll();
                    }
                }
            };

            synchronized (waitDoneLock) {
                mOpenHandler.post(unlockRunnable);
                try {
                    waitDoneLock.wait();
                } catch (InterruptedException ex) {
                    LogHelper.e(mHandlerTag, "waitDone interrupted");
                    return false;
                }
            }
            return true;
        }

        /**
         * Dummy state call back.
         */
        private class DummyCameraStateCallback extends Camera2Proxy.StateCallback {
            @Override
            public void onOpened(@Nonnull Camera2Proxy camera2proxy) {
            }

            @Override
            public void onDisconnected(@Nonnull Camera2Proxy camera2proxy) {
            }

            @Override
            public void onError(@Nonnull Camera2Proxy camera2Proxy, int error) {

            }
        }

    }

}

