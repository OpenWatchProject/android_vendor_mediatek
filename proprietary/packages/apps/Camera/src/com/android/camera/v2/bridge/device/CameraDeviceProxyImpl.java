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
 * MediaTek Inc. (C) 2015. All rights reserved.
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
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCaptureSession.CaptureCallback;
import android.hardware.camera2.CameraCaptureSession.StateCallback;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CaptureRequest;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.platform.device.CameraDeviceProxy;

import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.HashMap;
import java.util.List;


/**
 * A CameraDeviceProxy implement by API2.
 */
public class CameraDeviceProxyImpl implements CameraDeviceProxy {
    private final Tag mTag;
    private final Activity              mActivity;
    private final CameraDevice          mCameraDevice;
    private final String                mCameraId;
    private final HandlerThread         mCameraThread;
    private final CameraHandler         mCameraHandler;
    private final Handler               mSessionStateHandler;

    private CameraCaptureSession        mCameraCaptureSession;
    private SessionStateCallback        mSessionStateCallback = new SessionStateCallback();
    private volatile boolean            mIsClosed = false;
    private volatile boolean            mIsSessionAbortCalled = false;
    private AtomicInteger mPendingSessionChangeCount = new AtomicInteger();

    private final CameraSessionCallback mCameraSessionCallback;
    private Object                      mSessionLock = new Object();

    public CameraDeviceProxyImpl(Activity activity,
            CameraDevice camera, CameraSessionCallback callback, Handler sessionHandler) {
        mActivity      = activity;
        mCameraDevice  = camera;
        mCameraId = mCameraDevice.getId();
        mTag = getTag(mCameraId);
        mCameraSessionCallback = callback;
        mSessionStateHandler = sessionHandler;
        mCameraThread = new HandlerThread("CameraDeviceProxyImpl");
        mCameraThread.start();
        mCameraHandler = new CameraHandler(mCameraThread.getLooper());
        mPendingSessionChangeCount.set(0);
    }

    @Override
    public void requestChangeCaptureRequets(boolean sync,
            RequestType requestType, CaptureType captureType) {
        LogHelper.d(mTag, "[requestChangeCaptureRequets]+ requestType:" + requestType +
                 " captureType:" + captureType + " sync:" + sync +
                 " mIsClosed:" + mIsClosed +
                 " mCameraCaptureSession:" + mCameraCaptureSession +
                 " mIsSessionAbortCalled:" + mIsSessionAbortCalled);
        if (mIsClosed) {
            return;
        }
        synchronized (mSessionLock) {
            if (mIsSessionAbortCalled) {
                try {
                    LogHelper.d(mTag, "waiting for new session ready...");
                    mSessionLock.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
        HashMap<String, Object> args = new HashMap<String, Object>();
        args.put("requestType", requestType);
        args.put("captureType", captureType);
        mCameraHandler.obtainMessage(REQUEST_CHANGE_REQUEST, args).sendToTarget();
        if (sync) {
            waitMessageProcessDone();
        }
        LogHelper.d(mTag, "[requestChangeCaptureRequets]-");
    }

    @Override
    public void requestChangeSessionOutputs(boolean sync) {
        LogHelper.d(mTag, "[requestChangeSessionOutputs]+ mIsClosed:" + mIsClosed);
        if (mIsClosed) {
            return;
        }
        mPendingSessionChangeCount.incrementAndGet();
        mCameraHandler.removeMessages(REQUEST_CHANGE_REQUEST);
        mCameraHandler.sendEmptyMessage(REQUEST_CHANGE_SESSION_OUTPUTS);
        if (sync) {
            waitMessageProcessDone();
        }
        LogHelper.d(mTag, "[requestChangeSessionOutputs]- mIsClosed:" + mIsClosed);
    }

    @Override
    public String getCameraId() {
        return mCameraId;
    }

    @Override
    public void close() {
        LogHelper.d(mTag, "[close]+ mIsClosed:" + mIsClosed);
        if (mIsClosed) {
            return;
        }
        mCameraHandler.removeMessages(REQUEST_CHANGE_REQUEST);
        mCameraHandler.removeMessages(REQUEST_CHANGE_SESSION_OUTPUTS);
        mCameraHandler.sendEmptyMessage(CLOSE);
        waitMessageProcessDone();
        mCameraThread.quitSafely();
        LogHelper.d(mTag, "[close]-");
    }

    private boolean waitMessageProcessDone() {
        int cameraThreadId = mCameraThread.getThreadId();
        int currentThreadId = (int) Thread.currentThread().getId();
        if (cameraThreadId == currentThreadId) {
            LogHelper.d(mTag, "ignore waitDone.");
            return false;
        }
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
            mCameraHandler.post(unlockRunnable);
            try {
                waitDoneLock.wait();
            } catch (InterruptedException ex) {
                LogHelper.i(mTag, "waitDone interrupted");
                return false;
            }
        }
        return true;
    }

    private class CameraHandler extends Handler {
        public CameraHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(mTag, "handleMessage: what = " + msg.what);
            switch (msg.what) {
            case REQUEST_CHANGE_SESSION_OUTPUTS:
                try {
                    synchronized (mSessionLock) {
                        if (mIsSessionAbortCalled || mIsClosed) {
                            LogHelper.d(mTag, "ignore configure session," +
                                     " mIsSessionAbortCalled:" + mIsSessionAbortCalled
                                    + " DeviceClosed:" + mIsClosed);
                            return;
                        }

                        if (mCameraCaptureSession != null) {
                            mCameraCaptureSession.abortCaptures();
                            mIsSessionAbortCalled = true;
                        }
                        List<Surface> outputSurfaces = new ArrayList<Surface>();
                        mCameraSessionCallback.configuringSessionOutputs(outputSurfaces);
                            if (outputSurfaces.size() != 0) {
                                mCameraDevice.createCaptureSession(outputSurfaces,
                                                mSessionStateCallback, mCameraHandler);
                            }
                        }
                } catch (CameraAccessException e) {
                    e.printStackTrace();
                    return;
                }
                break;
            case REQUEST_CHANGE_REQUEST:
                synchronized (mSessionLock) {
                    if (mIsSessionAbortCalled || mCameraCaptureSession == null || mIsClosed) {
                        LogHelper.d(mTag, "ignore request change, mIsSessionAbortCalled:"
                                + mIsSessionAbortCalled
                                + " mCameraCaptureSession:" + mCameraCaptureSession
                                + " DeviceClosed:" + mIsClosed);
                        return;
                    }
                    @SuppressWarnings("unchecked")
                    HashMap<String, Object> args = (HashMap<String, Object>) msg.obj;
                    RequestType requestType = (RequestType) args.get("requestType");
                    CaptureType captureType = (CaptureType) args.get("captureType");
                    CaptureRequest.Builder builder =  createCaptureRequests(requestType);
                    // configuring capture requests
                    CaptureCallback captureCallback =
                            mCameraSessionCallback.configuringSessionRequests(builder,
                                    requestType, captureType);
                    // submit capture requests
                    submitCaptureRequests(builder.build(),
                            requestType, captureType, captureCallback);
                    LogHelper.d(mTag, "request change done");
                }
                break;
            case CLOSE:
                if (!mIsClosed) {
                    mCameraDevice.close();
                    mIsClosed = true;
                }
                break;
            default:
                break;
            }
        }
    }

    private CaptureRequest.Builder createCaptureRequests(RequestType requestType) {
        CaptureRequest.Builder builder = null;
        try {
            switch (requestType) {
            case PREVIEW:
                builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                break;
            case STILL_CAPTURE:
                builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
                break;
            case RECORDING:
                builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
                break;
            case VIDEO_SNAP_SHOT:
                builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_VIDEO_SNAPSHOT);
                break;
            default:
                builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                break;
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        return builder;
    }

    private void submitCaptureRequests(CaptureRequest request,
            RequestType requestType,
            CaptureType captureType,
            CaptureCallback captureCallback) {
        LogHelper.d(mTag, "[submitCaptureRequests] captureType = " + captureType);
        //TODO this is a dirty workaround
        //Consider a better way to handle Surafce's life cycle between creator and it's user.
        if (request.getTargets().isEmpty() || mPendingSessionChangeCount.get() > 0) {
            LogHelper.e(mTag, "Skip submitCaptureRequests,pending session count:"
                    + mPendingSessionChangeCount.get());
            return;
        }
        try {
            switch (captureType) {
            case REPEATING_REQUEST:
                mCameraCaptureSession.setRepeatingRequest(request, captureCallback, mCameraHandler);
                break;
            case CAPTURE:
                mCameraCaptureSession.capture(request, captureCallback, mCameraHandler);
                break;
            default:
                break;
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private class SessionStateCallback extends StateCallback {
        @Override
        public void onActive(CameraCaptureSession session) {
            LogHelper.d(mTag, "onActive");
            if (session == mCameraCaptureSession) {
                mSessionStateHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mCameraSessionCallback.onSessionActive();
                    }
                });
            }
        }

        @Override
        public void onClosed(CameraCaptureSession session) {
            LogHelper.d(mTag, "onClosed, session = " + session);
            if (session == mCameraCaptureSession) {
                mCameraCaptureSession = null;
            }
        }

        @Override
        public void onConfigureFailed(CameraCaptureSession session) {
            LogHelper.d(mTag, "onConfigureFailed, session = " + session);
            if (session == mCameraCaptureSession) {
                mCameraCaptureSession = null;
            }
            mPendingSessionChangeCount.decrementAndGet();
            session.close();
        }

        @Override
        public void onConfigured(CameraCaptureSession session) {
            LogHelper.d(mTag, "onConfigured session:" + session);
            synchronized (mSessionLock) {
                mCameraCaptureSession = session;
                mIsSessionAbortCalled = false;
                mPendingSessionChangeCount.decrementAndGet();
                mSessionLock.notifyAll();
            }
            mSessionStateHandler.post(new Runnable() {
                @Override
                public void run() {
                    mCameraSessionCallback.onSessionConfigured();
                }
            });
        }

        @Override
        public void onReady(CameraCaptureSession session) {
            LogHelper.d(mTag, "onReady,session = " + session);
        }
    }

    private Tag getTag(String cameraId) {
        return new Tag("CameraDevice(" + cameraId + ")");
    }
}