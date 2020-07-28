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
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCaptureSession.CaptureCallback;
import android.hardware.camera2.CameraCaptureSession.StateCallback;
import android.hardware.camera2.CameraConstrainedHighSpeedCaptureSession;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.InputConfiguration;
import android.hardware.camera2.params.OutputConfiguration;
import android.hardware.camera2.params.SessionConfiguration;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.view.Surface;

import com.google.common.base.Preconditions;
import com.mediatek.camera.common.debug.CameraSysTrace;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.HistoryHandler;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy.SessionOperatorInfo;
import com.mediatek.camera.common.device.v2.Camera2Proxy.RequestCreatorInfo;
import com.mediatek.camera.common.device.v2.Camera2Proxy.SessionCreatorInfo;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executor;

import javax.annotation.Nonnull;

/**
 * A handler to process operations at android.hardware.camera2.
 */
@TargetApi(Build.VERSION_CODES.M)
class Camera2Handler extends HistoryHandler {
    private final Tag mTag;
    private final String mCameraId;

    private final Handler mRespondHandler;
    // This handler on which the callback should be invoked.
    private Handler mCaptureSessionProxyHandler;

    private CameraDevice mCameraDevice;
    private Camera2Proxy mCamera2Proxy;
    // Capture session, always just exist one.
    private volatile CameraCaptureSession mCameraCaptureSession;
    private volatile Map<CameraCaptureSession, Camera2CaptureSessionProxy> mSessionMap
        = new LinkedHashMap<>();
    private Camera2CaptureSessionProxy.StateCallback mSessionStateProxyCallback;
    private final IDeviceInfoListener mIDeviceInfoListener;

    /**
     * A listener that listen camera info.
     */
    public interface IDeviceInfoListener {
        /**
         * Notify when camera closed.
         */
        void onClosed();

        /**
         * Notify when camera exception happened.
         */
        void onError();
    }

    Camera2Handler(String cameraId, @Nonnull Looper looper, @Nonnull Handler respondHandler,
                    @Nonnull CameraDevice device, @Nonnull IDeviceInfoListener infoListener) {
        super(looper);
        Preconditions.checkNotNull(looper, "Construct Camera2Handler,the looper must not null");
        Preconditions.checkNotNull(respondHandler, "Construct Camera2Handler,the respondHandler " +
                "must not null");
        Preconditions.checkNotNull(device, "Construct Camera2Handler,the device must not null");
        mTag = new Tag("API2-Handler-" + cameraId);
        mCameraId = cameraId;
        mRespondHandler = respondHandler;
        mCameraDevice = device;
        mIDeviceInfoListener = infoListener;
    }

    void updateCamera2Proxy(@Nonnull Camera2Proxy proxy) {
        Preconditions.checkNotNull(proxy, "updateCamera2Proxy,the proxy must not null");
        mCamera2Proxy = proxy;
    }

    public void closeCamera() {
        mCameraDevice = null;
        mCameraCaptureSession = null;
    }

    @Override
    public void handleMessage(Message msg) {
        super.handleMessage(msg);
        int operation = msg.what;

        mMsgStartTime = SystemClock.uptimeMillis();
        printStartMsg(mTag.toString(), Camera2Actions.stringify(operation), (mMsgStartTime - msg
                .getWhen()));

        doHandleMessage(msg);

        mMsgStopTime = SystemClock.uptimeMillis();
        printStopMsg(mTag.toString(), Camera2Actions.stringify(operation), (mMsgStopTime -
                mMsgStartTime));
    }

    @Override
    protected void doHandleMessage(Message msg) {
        if (Camera2Actions.isSessionMessageType(msg.what)) {
            CameraSysTrace.onEventSystrace("Device.SessionMessage:" + msg.what, true);
            handleSessionMessage(msg);
            CameraSysTrace.onEventSystrace("Device.SessionMessage:" + msg.what, false);
        } else {
            CameraSysTrace.onEventSystrace("Device.RequestMessage:" + msg.what, true);
            handleRequestMessage(msg);
            CameraSysTrace.onEventSystrace("Device.RequestMessage:" + msg.what, false);
        }
    }

    private void handleSessionMessage(Message msg) {
        //first check camera close or not
        if (isCameraClosed(Camera2Actions.stringify(msg.what))) {
            return;
        }
        try {
            switch (msg.what) {
                case Camera2Actions.CREATE_CAPTURE_SESSION:
                    createCaptureSession((SessionCreatorInfo) msg.obj);
                    break;

                case Camera2Actions.CREATE_REPROCESSABLE_CAPTURE_SESSION:
                    createReprocessingSession((SessionCreatorInfo) msg.obj);
                    break;

                case Camera2Actions.CREATE_CONSTRAINED_HIGH_SPEED_CAPTURE_SESSION:
                    createHighSpeedSession((SessionCreatorInfo) msg.obj);
                    break;

                case Camera2Actions.CREATE_CAPTURE_REQUEST:
                    RequestCreatorInfo captureRequestInfo = (RequestCreatorInfo) msg.obj;
                    CaptureRequest.Builder bdr = mCameraDevice.createCaptureRequest(msg.arg1);
                    captureRequestInfo.setCaptureRequestBuilder(bdr);
                    break;

                case Camera2Actions.CREATE_REPROCESSABLE_CAPTURE_REQUEST:
                    RequestCreatorInfo info = (RequestCreatorInfo) msg.obj;
                    CaptureRequest.Builder builder = mCameraDevice
                            .createReprocessCaptureRequest(info.mResult);
                    info.setReprocessRequestBuilder(builder);
                    break;

                default:
                    LogHelper.e(mTag, "[handleSessionMessage] Unimplemented msg: " + msg.what);
            }
        } catch (Exception e) {
            e.printStackTrace();
            dealException();
        }
    }

    private void handleRequestMessage(Message msg) {
        //If camera or session is closed, don't need to the request.
        if (isCameraClosed(Camera2Actions.stringify(msg.what))) {
            return;
        }
        if (isSessionClosed(Camera2Actions.stringify(msg.what))) {
            return;
        }
        try {
            switch (msg.what) {
                case Camera2Actions.PREPARE:
                    mCameraCaptureSession.prepare((Surface) msg.obj);
                    break;

                case Camera2Actions.CAPTURE:
                    SessionOperatorInfo captureInfo = (SessionOperatorInfo) msg.obj;
                    int[] captureNumber = captureInfo.mSessionNum;
                    captureNumber[0] = startCapture(captureInfo);
                    break;

                case Camera2Actions.CAPTURE_BURST:
                    SessionOperatorInfo burstInfo = (SessionOperatorInfo) msg.obj;
                    int[] captureBurstNumber = burstInfo.mSessionNum;
                    captureBurstNumber[0] = startBurstCapture(burstInfo);
                    break;

                case Camera2Actions.SET_REPEATING_REQUEST:
                    SessionOperatorInfo repeatingRequestInfo = (SessionOperatorInfo) msg.obj;
                    int[] repeatingRequestNumber = repeatingRequestInfo.mSessionNum;
                    repeatingRequestNumber[0] = setRepeatingRequest(repeatingRequestInfo);
                    break;

                case Camera2Actions.SET_REPEATING_BURST:
                    SessionOperatorInfo repeatingBurstInfo = (SessionOperatorInfo) msg.obj;
                    int[] repeatingBurstNumber = repeatingBurstInfo.mSessionNum;
                    repeatingBurstNumber[0] = setRepeatingBurst(repeatingBurstInfo);
                    break;

                case Camera2Actions.STOP_REPEATING:
                    mCameraCaptureSession.stopRepeating();
                    break;

                case Camera2Actions.CREATE_HIGH_SPEED_REQUEST:
                    SessionOperatorInfo highSpeedInfo = (SessionOperatorInfo) msg.obj;
                    highSpeedInfo.mResultRequest = createHighSpeedRequest(highSpeedInfo);
                    break;

                case Camera2Actions.ABORT_CAPTURES:
                    mCameraCaptureSession.abortCaptures();
                    break;

                case Camera2Actions.GET_INPUT_SURFACE:
                    Surface[] surface = (Surface[]) msg.obj;
                    surface[0] = mCameraCaptureSession.getInputSurface();
                    break;


                case Camera2Actions.FINALIZE_OUTPUTCONFIGURATIONS:
                    finalizeOutputConfigurations((List<OutputConfiguration>) msg.obj);
                    break;

                case Camera2Actions.CLOSE_SESSION:
                    mCameraCaptureSession.close();
                    mCameraCaptureSession = null;
                    break;

                case Camera2Actions.IS_REPROCESSABLE:
                    boolean[] processAble = (boolean[]) msg.obj;
                    processAble[0] = mCameraCaptureSession.isReprocessable();
                    break;

                default:
                    LogHelper.e(mTag, "[handleRequestMessage] Unimplemented msg: " + msg.what);
            }
        } catch (Exception e) {
            e.printStackTrace();
            dealException();
        }
    }

    private void createCaptureSession(SessionCreatorInfo info) throws CameraAccessException {
        if (mCameraCaptureSession != null) {
            mCameraCaptureSession = null;
        }
        List<Surface> surfaces = info.mSurfaces;
        mSessionStateProxyCallback = info.mCallback;
        mCaptureSessionProxyHandler = info.mHandler;
        if (info.mBuilder == null) {
            LogHelper.i(mTag, "[createCaptureSession] mBuilder is null");
            mCameraDevice.createCaptureSession(surfaces, mSessionStateCallback, mRespondHandler);
            return;
        }
        if (info.mOutputConfigs != null) {
            SessionConfiguration sessionConfigByOutput = new SessionConfiguration(
                    SessionConfiguration.SESSION_REGULAR, info.mOutputConfigs,
                    new HandlerExecutor(mRespondHandler), mSessionStateCallback);
            LogHelper.i(mTag, "[createCaptureSession] with mOutputConfigs");
            sessionConfigByOutput.setSessionParameters(info.mBuilder.build());
            mCameraDevice.createCaptureSession(sessionConfigByOutput);
            return;
        }
        List<OutputConfiguration> outConfigurations = new ArrayList<>(surfaces.size());
        for (Surface surface : surfaces) {
            outConfigurations.add(new OutputConfiguration(surface));
        }
        SessionConfiguration sessionConfig = new SessionConfiguration(
                SessionConfiguration.SESSION_REGULAR, outConfigurations,
                new HandlerExecutor(mRespondHandler), mSessionStateCallback);
        LogHelper.i(mTag, "[createCaptureSession] with parameters");
        sessionConfig.setSessionParameters(info.mBuilder.build());
        mCameraDevice.createCaptureSession(sessionConfig);
    }

    private void createReprocessingSession(SessionCreatorInfo info) throws CameraAccessException {
        if (mCameraCaptureSession != null) {
            mCameraCaptureSession = null;
        }
        InputConfiguration inputConfiguration = info.mInputConfiguration;
        List<Surface> outPutSurfaces = info.mSurfaces;
        mSessionStateProxyCallback = info.mCallback;
        mCaptureSessionProxyHandler = info.mHandler;
        mCameraDevice.createReprocessableCaptureSession(inputConfiguration, outPutSurfaces,
                        mSessionStateCallback, mRespondHandler);
    }

    private void createHighSpeedSession(SessionCreatorInfo info) throws CameraAccessException {
        if (mCameraCaptureSession != null) {
            mCameraCaptureSession = null;
        }
        List<Surface> highSurface = info.mSurfaces;
        mSessionStateProxyCallback = info.mCallback;
        mCaptureSessionProxyHandler = info.mHandler;
        mCameraDevice.createConstrainedHighSpeedCaptureSession(highSurface, mSessionStateCallback,
                        mRespondHandler);
    }

    private int startCapture(SessionOperatorInfo info) throws CameraAccessException {
        CaptureRequest request = info.mCaptureRequest.get(0);
        CaptureCallback callback = info.mCaptureCallback;
        Handler handler = info.mHandler;
        return mCameraCaptureSession.capture(request, callback, handler);
    }

    private int startBurstCapture(SessionOperatorInfo info) throws CameraAccessException {
        List<CaptureRequest> request = info.mCaptureRequest;
        CaptureCallback callback = info.mCaptureCallback;
        Handler handler = info.mHandler;
        return mCameraCaptureSession.captureBurst(request, callback, handler);
    }

    private int setRepeatingRequest(SessionOperatorInfo info) throws CameraAccessException {
        CaptureRequest request = info.mCaptureRequest.get(0);
        CaptureCallback callback = info.mCaptureCallback;
        Handler handler = info.mHandler;
        return mCameraCaptureSession.setRepeatingRequest(request, callback, handler);
    }

    private int setRepeatingBurst(SessionOperatorInfo info) throws CameraAccessException {
        List<CaptureRequest> request = info.mCaptureRequest;
        CaptureCallback callback = info.mCaptureCallback;
        Handler handler = info.mHandler;
        return mCameraCaptureSession.setRepeatingBurst(request, callback, handler);
    }

    private List<CaptureRequest> createHighSpeedRequest(SessionOperatorInfo info) throws
            CameraAccessException {
        CaptureRequest request = info.mCaptureRequest.get(0);
        return ((CameraConstrainedHighSpeedCaptureSession) mCameraCaptureSession)
                .createHighSpeedRequestList(request);
    }

    private void finalizeOutputConfigurations(List<OutputConfiguration> outputConfigurations)
            throws CameraAccessException {
        mCameraCaptureSession.finalizeOutputConfigurations(outputConfigurations);
    }

    private static class HandlerExecutor implements Executor {
        private final Handler mHandler;

        public HandlerExecutor(Handler handler) {
            mHandler = handler;
        }

        @Override
        public void execute(Runnable runCmd) {
            mHandler.post(runCmd);
        }
    }
    private boolean isCameraClosed(String msg) {
        boolean closed = mCameraDevice == null;
        if (closed) {
            LogHelper.e(mTag, "camera is closed,can not call : " + msg);
        }
        return closed;
    }

    private boolean isSessionClosed(String msg) {
        boolean isClosed = mCameraCaptureSession == null;
        if (isClosed) {
            LogHelper.e(mTag, "session is closed,can not call : " + msg);
        }
        return isClosed;
    }

    private void dealException() {
        mIDeviceInfoListener.onError();
        if (mCameraDevice != null) {
            mCameraDevice.close();
            mCameraDevice = null;
        }
        generateHistoryString(Integer.parseInt(mCameraId));
    }

    // when old session is later than new session callback. how to do?
    // -->Done, use a map to keep it.
    private final StateCallback mSessionStateCallback = new StateCallback() {

        @Override
        public void onConfigured(final CameraCaptureSession session) {
            LogHelper.i(mTag, "[onConfigured], session = " + session);
            createSessionAndProxy(session);
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    mSessionStateProxyCallback.onConfigured(mSessionMap.get(session));
                }
            };
            postSessionRunnable(runnable);
        }

        @Override
        public void onConfigureFailed(final CameraCaptureSession session) {
            LogHelper.i(mTag, "[onConfigureFailed] session = " + session);
            createSessionAndProxy(session);
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    mSessionStateProxyCallback.onConfigureFailed(mSessionMap.get(session));
                }
            };
            postSessionRunnable(runnable);
        }

        @Override
        public void onReady(final CameraCaptureSession session) {
            super.onReady(session);
            LogHelper.i(mTag, "[onReady] mCameraCaptureSession = " + mCameraCaptureSession
                            + ",ready session = " + session);
            final Camera2CaptureSessionProxy proxy = mSessionMap.get(session);
            if (proxy != null) {
                Runnable runnable = new Runnable() {
                    @Override
                    public void run() {
                        mSessionStateProxyCallback.onReady(proxy);
                    }
                };
                postSessionRunnable(runnable);
            }
        }

        @Override
        public void onActive(final CameraCaptureSession session) {
            super.onActive(session);
            LogHelper.i(mTag, "[onActive] mCameraCaptureSession = " + mCameraCaptureSession
                            + ",active session = " + session);
            final Camera2CaptureSessionProxy proxy = mSessionMap.get(session);
            if (proxy != null) {
                Runnable runnable = new Runnable() {
                    @Override
                    public void run() {
                        mSessionStateProxyCallback.onActive(proxy);
                    }
                };
                postSessionRunnable(runnable);
            }
        }

        @Override
        public void onClosed(final CameraCaptureSession session) {
            super.onClosed(session);
            LogHelper.i(mTag, "[onClosed] mCameraCaptureSession = " + mCameraCaptureSession
                            + ",closed session = " + session);
            final Camera2CaptureSessionProxy proxy = mSessionMap.get(session);
            if (proxy != null) {
                Runnable runnable = new Runnable() {
                    @Override
                    public void run() {
                        mSessionStateProxyCallback.onClosed(proxy);
                        mSessionMap.remove(session);
                    }
                };
                postSessionRunnable(runnable);
            }
        }

        @Override
        public void onSurfacePrepared(final CameraCaptureSession session, final Surface surface) {
            super.onSurfacePrepared(session, surface);
            LogHelper.i(mTag, "[onSurfacePrepared] mCameraCaptureSession = " + mCameraCaptureSession
                            + ",prepared session = " + session);
            final Camera2CaptureSessionProxy proxy = mSessionMap.get(session);
            if (proxy != null) {
                Runnable runnable = new Runnable() {
                    @Override
                    public void run() {
                        mSessionStateProxyCallback.onSurfacePrepared(proxy, surface);
                    }
                };
                postSessionRunnable(runnable);
            }
        }

        private void createSessionAndProxy(CameraCaptureSession session) {
            // If session not in the mSessionMap,means need add the session to the map,and also need
            // create a new proxy.
            if (!mSessionMap.containsKey(session)) {
                mSessionMap.put(session, new Camera2CaptureSessionProxy(Camera2Handler.this,
                                mCamera2Proxy));
                mCameraCaptureSession = session;
            }
        }

        private void postSessionRunnable(Runnable runnable) {
            if (mCaptureSessionProxyHandler != null) {
                mCaptureSessionProxyHandler.post(runnable);
            } else {
                // User maybe not specify the handler on which to return the action,such as
                // onSessionConfigured(). so use the default respond handler
                LogHelper.i(mTag, "[postSessionRunnable]use the respond handler");
                mRespondHandler.post(runnable);
            }
        }
    };
}