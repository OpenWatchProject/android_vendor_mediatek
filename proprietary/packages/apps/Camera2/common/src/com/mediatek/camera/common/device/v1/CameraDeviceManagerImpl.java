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

package com.mediatek.camera.common.device.v1;

import android.app.admin.DevicePolicyManager;
import android.content.Context;
import android.hardware.Camera;
import android.hardware.Camera.ErrorCallback;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManager;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraOpenException;
import com.mediatek.camera.common.device.CameraOpenException.ExceptionType;
import com.mediatek.camera.common.device.CameraStateCallback;
import com.mediatek.camera.common.loader.FeatureLoader;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.portability.CameraEx;

import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * An implement of CameraDeviceManager with android.hardware.camera.Camera.
 */
public class CameraDeviceManagerImpl extends CameraDeviceManager {
    private static final Tag TAG = new Tag("DeviceMgr");
    private ConcurrentHashMap<String, CameraProxyCreatorImpl> mCameraProxyCreatorList
            = new ConcurrentHashMap<>();
    private final Context mContext;

    /**
     * Construct a camera device manager impl.
     * @param context camera context.
     */
    public CameraDeviceManagerImpl(Context context) {
        mContext = context;
    }

    @Override
    public synchronized void openCamera(@Nonnull String cameraId,
                                        @Nullable CameraStateCallback callback,
                                        @Nullable Handler handler) throws CameraOpenException {
        LogHelper.d(TAG, "[openCamera] ");
        checkPreconditionsAndOpen(cameraId, callback, handler, false);
    }

    @Override
    public synchronized void openCameraSync(@Nonnull String cameraId,
                                            @Nonnull CameraStateCallback callback,
                                            @Nullable Handler handler) throws CameraOpenException {
        LogHelper.d(TAG, "[openCameraSync]");
        checkPreconditionsAndOpen(cameraId, callback, handler, true);
    }

    @Override
    public void closeSync(@Nonnull String cameraId) {

    }

    @Override
    public void close(@Nonnull String cameraId) {

    }

    private void checkPreconditionsAndOpen(String cameraId,
                                           CameraStateCallback callback, Handler handler,
                                           boolean isSync) throws CameraOpenException {
        checkDevicePolicy();
        LogHelper.d(TAG, "[checkPreconditions]" +
                " mProxyCreatorMap size = " + mCameraProxyCreatorList.size());
        CameraProxyCreatorImpl proxyCreator = mCameraProxyCreatorList.get(cameraId);
        if (proxyCreator == null) {
            LogHelper.i(TAG, "[checkPreconditions] add new id = " + cameraId);
            proxyCreator = new CameraProxyCreatorImpl(cameraId);
            mCameraProxyCreatorList.put(cameraId, proxyCreator);
        }
        if (isSync) {
            proxyCreator.doOpenCameraSync(callback);
        } else {
            proxyCreator.doOpenCamera(callback);
        }
    }

    @Override
    public synchronized void recycle() {
        LogHelper.i(TAG, "[recycle]");
        Iterator iterator = mCameraProxyCreatorList.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry e = (Map.Entry) iterator.next();
            CameraProxyCreatorImpl cameraProxyCreator = (CameraProxyCreatorImpl) e.getValue();
            cameraProxyCreator.destroyHandlerThreads();
        }
        mCameraProxyCreatorList.clear();
    }

    private void checkDevicePolicy() throws CameraOpenException {
        DevicePolicyManager dpm = (DevicePolicyManager) mContext
                .getSystemService(Context.DEVICE_POLICY_SERVICE);
        if (dpm.getCameraDisabled(null)) {
            throw new CameraOpenException(ExceptionType.SECURITY_EXCEPTION);
        }
    }

    /**
     * Camera proxy creator implement.
     */
    @SuppressWarnings("deprecation")
    private class CameraProxyCreatorImpl extends CameraProxyCreator {
        private final Handler mRespondHandler;
        private final Tag mHandlerTag;
        private final Tag mRespondTag;

        /**
         * Create camera proxy creator by camera id.
         * @param cameraId the open camera id.
         */
        CameraProxyCreatorImpl(String cameraId) {
            super(CameraApi.API1, cameraId);
            mThreadLock.lock();
            mRespondHandler = new RespondCameraHandler(mRespondThread.getLooper());
            mThreadLock.unlock();
            mHandlerTag = new Tag("API1-Handler-" + cameraId);
            mRespondTag = new Tag("API1-Respond-" + cameraId);
        }

        private void doOpenCamera(CameraStateCallback callback) {
            mRespondHandler.obtainMessage(
                    RespondCameraHandler.MSG_OPEN_CAMERA, callback).sendToTarget();
        }

        private void doOpenCameraSync(CameraStateCallback callback) throws CameraOpenException {
            mRespondHandler.obtainMessage(
                    RespondCameraHandler.MSG_OPEN_CAMERA, callback).sendToTarget();
            waitDone();
        }

        /**
         * Camera device respond handler.
         */
        private class RespondCameraHandler extends Handler {
            static final int MSG_OPEN_CAMERA = 0;
            static final int MSG_NOTIFY_ON_CLOSED = 1;
            private CameraHandler mRequestHandler;
            private Camera mCamera;
            private CameraProxy mCameraProxy;
            private CameraProxy.StateCallback mOpenStateCallback;
            private DummyCameraStateCallback mDummyCameraStateCallback
                    = new DummyCameraStateCallback();
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
                        LogHelper.d(TAG, "handle open camera msg.");
                        CameraProxy.StateCallback stateCallback =
                                (CameraProxy.StateCallback) msg.obj;
                        mOpenStateCallback =
                                stateCallback == null ? mDummyCameraStateCallback : stateCallback;
                        if (mCameraProxy == null) {
                            LogHelper.i(mHandlerTag, "[openCamera]+");
                            long startTime = SystemClock.uptimeMillis();
                            FeatureLoader.notifySettingBeforeOpenCamera(mContext, mCameraId,
                                    CameraApi.API1);
                            retryOpenCamera();
                            LogHelper.i(mHandlerTag, "[openCamera]-, executing time = " +
                                    (SystemClock.uptimeMillis() - startTime) + "ms.");
                            if (mCamera == null) {
                                LogHelper.w(mHandlerTag, "[openCamera] result with exception!");
                                return;
                            }
                            mThreadLock.lock();
                            // if looper of request thread is null, not create CameraHandler
                            // any more, and enter onError logic
                            if (mRequestThread.getLooper() != null) {
                                mRequestHandler = new CameraHandler(
                                        mContext,
                                        mCameraId,
                                        mRequestThread.getLooper(),
                                        mCamera,
                                        mDeviceInfoListener);
                            } else {
                                LogHelper.w(mHandlerTag,
                                        "[openCamera] mRequestThread.getLooper() is null" +
                                                ", mRequestThread.isAlive() = " +
                                                mRequestThread.isAlive() +
                                                ", mCameraProxyCreatorList.size() = "
                                                + mCameraProxyCreatorList.size() +
                                                ", mOpenStateCallback.onError, return");
                                mOpenStateCallback.onError(new CameraProxy(
                                        mCameraId, mRequestHandler, mCamera),
                                        CameraUtil.CAMERA_OPEN_FAIL);
                                mThreadLock.unlock();
                                return;
                            }
                            mThreadLock.unlock();
                            mCamera.setErrorCallback(mCameraErrorCallback);
                            mRequestHandler.sendEmptyMessage(CameraActions.INIT_PARAMETERS);
                            waitDone();
                            mCameraProxy = new CameraProxy(mCameraId, mRequestHandler, mCamera);
                        }
                        boolean getParamFail = mRequestHandler.getOriginalParameters() == null;
                        if (getParamFail && mOpenStateCallback != mDummyCameraStateCallback) {
                            LogHelper.e(TAG, "get parameters fail after open camera so return");
                            mOpenStateCallback.onError(mCameraProxy, CameraUtil.CAMERA_OPEN_FAIL);
                            return;
                        }
                        mOpenStateCallback.onOpened(mCameraProxy);
                        mRetryCount = 0;
                        break;
                    case MSG_NOTIFY_ON_CLOSED:
                        LogHelper.i(TAG, "onClose");
                        mOpenStateCallback.onClosed(mCameraProxy);
                        mCamera = null;
                        mCameraProxy = null;
                        break;
                    default:
                        break;
                }
            }

            private void waitDone() {
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
                    mRequestHandler.post(unlockRunnable);
                    try {
                        waitDoneLock.wait();
                    } catch (InterruptedException ex) {
                        LogHelper.e(TAG, "waitDone interrupted");
                    }
                }
            }

            private ErrorCallback mCameraErrorCallback = new ErrorCallback() {
                @Override
                public void onError(int error, Camera camera) {
                    LogHelper.i(mRespondTag, "[onError] error:" + error);
                    mRequestHandler.notifyDeviceError(error);
                    mOpenStateCallback.onError(mCameraProxy, error);
                    mCameraProxy = null;
                    mCamera = null;
                }
            };

            private CameraHandler.IDeviceInfoListener mDeviceInfoListener =
                    new CameraHandler.IDeviceInfoListener() {
                        @Override
                        public void onClosed() {
                            LogHelper.i(mRespondTag, "[onClosed]");
                            mRespondHandler.obtainMessage(MSG_NOTIFY_ON_CLOSED).sendToTarget();
                        }

                        @Override
                        public void onError() {
                            LogHelper.i(TAG, "[onError]");
                            mOpenStateCallback.onClosed(mCameraProxy);
                            mOpenStateCallback.onError(mCameraProxy,
                                            CameraUtil.CAMERA_ERROR_UNKNOWN);
                            mCameraProxy = null;
                            mCamera = null;
                        }
                    };

            private void retryOpenCamera() {
                try {
                    mCamera = CameraEx.openLegacy(Integer.parseInt(mCameraId),
                            CameraEx.CAMERA_HAL_API_VERSION_1_0);
                } catch (RuntimeException e) {
                    LogHelper.e(mHandlerTag, "[retryOpenCamera] error: " + e.getMessage());
                    if (isNeedRetryOpen()) {
                        mOpenStateCallback.onRetry();
                        LogHelper.e(mHandlerTag, "[retryOpenCamera] retry time: " + mRetryCount);
                        retryOpenCamera();
                        return;
                    }
                    mOpenStateCallback.onError(new CameraProxy(mCameraId, mRequestHandler, mCamera),
                            CameraUtil.CAMERA_OPEN_FAIL);
                }
            }

            private boolean isNeedRetryOpen() {
                if (mRetryCount < OPEN_RETRY_COUNT) {
                    mRetryCount++;
                    return true;
                }
                return false;
            }

            private class DummyCameraStateCallback extends CameraProxy.StateCallback {
                @Override
                public void onOpened(@Nonnull CameraProxy cameraProxy) {
                }

                @Override
                public void onClosed(@Nonnull CameraProxy cameraProxy) {
                }

                @Override
                public void onDisconnected(@Nonnull CameraProxy cameraProxy) {
                }

                @Override
                public void onError(@Nonnull CameraProxy cameraProxy, int error) {
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
                mRespondHandler.post(unlockRunnable);
                try {
                    waitDoneLock.wait();
                } catch (InterruptedException ex) {
                    LogHelper.e(mHandlerTag, "waitDone interrupted");
                    return false;
                }
            }
            return true;
        }
    }
}