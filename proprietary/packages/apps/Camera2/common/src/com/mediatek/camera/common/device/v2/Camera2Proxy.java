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

import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.InputConfiguration;
import android.hardware.camera2.params.OutputConfiguration;
import android.os.Handler;
import android.os.Message;
import android.view.Surface;

import com.google.common.base.Preconditions;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraProxyBase;
import com.mediatek.camera.common.device.CameraStateCallback;
import com.mediatek.camera.common.utils.AtomAccessor;

import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * A proxy for the instance of android.hardware.camera2.CameraDevice.
 */
public class Camera2Proxy extends CameraProxyBase {
    private final String mCameraId;
    private final CameraDevice mCameraDevice;
    private final Camera2Handler mRequestHandler;
    private final Handler mRespondHandler;
    private final AtomicBoolean mClosing = new AtomicBoolean();
    // Lock to synchronize cross-thread access to device public interface
    // TODO: All the close logic is not complete, Need check whether can use mClosing replace it or
    // not?
    private final Object mInterfaceLock = new Object();

    private CaptureRequest.Builder mCaptureBuilder;
    private CaptureRequest.Builder mReprocessBuilder;
    private AtomAccessor mAtomAccessor = new AtomAccessor();

    /**
     * This template is usd for request.
     * This template will be set to camera framework,so the value must same as follows:
     *     public static final int TEMPLATE_MANUAL = 6;
     *     public static final int TEMPLATE_PREVIEW = 1;
     *     public static final int TEMPLATE_RECORD = 3;
     *     public static final int TEMPLATE_STILL_CAPTURE = 2;
     *     public static final int TEMPLATE_VIDEO_SNAPSHOT = 4;
     *     public static final int TEMPLATE_ZERO_SHUTTER_LAG = 5;
     */
    public static final int TEMPLATE_MANUAL = 6;
    public static final int TEMPLATE_PREVIEW = 1;
    public static final int TEMPLATE_RECORD = 3;
    public static final int TEMPLATE_STILL_CAPTURE = 2;
    public static final int TEMPLATE_VIDEO_SNAPSHOT = 4;
    public static final int TEMPLATE_ZERO_SHUTTER_LAG = 5;

    /**
     * <p>
     * Camera2 proxy constructor, will do the following things:
     * </p>
     * <li>Initialize camera handler.</li>
     *
     * @param cameraId
     *            the id of camera device.
     * @param device
     *            the device than already opened.
     * @param requestHandler
     *            the handler used to make device or session operation.
     * @param respondHandler the handler used to notify device callback and session callback.
     */
    public Camera2Proxy(@Nonnull String cameraId, @Nonnull CameraDevice device,
                    @Nonnull Camera2Handler requestHandler, @Nonnull Handler respondHandler) {
        mCameraDevice = device;
        mCameraId = cameraId;
        mRequestHandler = requestHandler;
        mRespondHandler = respondHandler;
    }

    @Nonnull
    @Override
    public String getId() {
        return mCameraId;
    }

    @Nonnull
    @Override
    public CameraApi getApiType() {
        return CameraApi.API2;
    }

    /**
     * Get respond handler.
     * @return the respond handler.
     */
    public Handler getRespondHandler() {
        return mRespondHandler;
    }
    /**
     * Get the current camera device.
     *
     * @return The current camera device which already opened.
     */
    public CameraDevice getCameraDevice() {
        return mCameraDevice;
    }

    /**
     * See {@code CameraDevice#createCaptureSession}.
     *
     * @param outputs
     *            The new set of Surfaces that should be made available as targets for captured
     *            image data.
     * @param callback
     *            The callback to notify about the status of the new capture session.
     * @param handler
     *            The handler on which the callback should be invoked, or {@code null} to use the
     *            default thread's {@link android.os.Looper looper}. the default thread is not main
     *            thread, so you can't do UI operations when callback invoked. if the callback is
     *            invoked by default thread, please don't blocking the thread, such as long time
     *            operations. if you have blocking this , you can't receive any info from
     *            framework.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public void createCaptureSession(@Nonnull List<Surface> outputs,
                    @Nonnull Camera2CaptureSessionProxy.StateCallback callback,
                    @Nonnull Handler handler) throws CameraAccessException {
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.CREATE_CAPTURE_SESSION,
                        new SessionCreatorInfo(outputs, callback, handler, null));
        mAtomAccessor.sendAtomMessage(mRequestHandler, msg);
    }

    /**
     * Create session with parameters.
     * See {@code CameraDevice#createCaptureSession}.
     * @param outputs The new set of Surfaces that should be made available as targets for captured
     *            image data.
     * @param callback
     *            The callback to notify about the status of the new capture session.
     * @param handler
     *            The handler on which the callback should be invoked, or {@code null} to use the
     *            default thread's {@link android.os.Looper looper}. the default thread is not main
     *            thread, so you can't do UI operations when callback invoked. if the callback is
     *            invoked by default thread, please don't blocking the thread, such as long time
     *            operations. if you have blocking this , you can't receive any info from
     *            framework.
     *
     * @param builder The builder which can build request.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public void createCaptureSession(@Nonnull List<Surface> outputs,
                                         @Nonnull Camera2CaptureSessionProxy.StateCallback callback,
                                         @Nonnull Handler handler, @Nonnull Builder builder)
            throws CameraAccessException {
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.CREATE_CAPTURE_SESSION,
                new SessionCreatorInfo(outputs, callback, handler, null, builder));
        mAtomAccessor.sendAtomMessage(mRequestHandler, msg);
    }

    /**
     * Create session with parameters.
     * See {@code CameraDevice#createCaptureSession}.
     * @param outputConfigs The new set of Surfaces that should be made available as targets for captured
     *            image data.
     * @param callback
     *            The callback to notify about the status of the new capture session.
     * @param handler
     *            The handler on which the callback should be invoked, or {@code null} to use the
     *            default thread's {@link android.os.Looper looper}. the default thread is not main
     *            thread, so you can't do UI operations when callback invoked. if the callback is
     *            invoked by default thread, please don't blocking the thread, such as long time
     *            operations. if you have blocking this , you can't receive any info from
     *            framework.
     *
     * @param builder The builder which can build request.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public void createCaptureSession(@Nonnull Camera2CaptureSessionProxy.StateCallback callback,
                                     @Nonnull Handler handler, @Nonnull Builder builder,
                                     @Nonnull List<OutputConfiguration> outputConfigs)
            throws CameraAccessException {
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.CREATE_CAPTURE_SESSION,
                new SessionCreatorInfo(outputConfigs, callback, handler, builder));
        mAtomAccessor.sendAtomMessage(mRequestHandler, msg);
    }

    /**
     * See {@code CameraDevice#createReprocessableCaptureSession}.
     *
     * @param inputConfig
     *            The configuration for the input {@link Surface}
     * @param outputs
     *            The new set of Surfaces that should be made available as targets for captured
     *            image data.
     * @param callback
     *            The callback to notify about the status of the new capture session.
     * @param handler
     *            The handler on which the callback should be invoked, or {@code null} to use the
     *            default thread's {@link android.os.Looper looper}. the default thread is not main
     *            thread, so you cann't do UI operations when callback invoked. if the callback is
     *            invoked by default thread, please don't blocking the thread, such as long time
     *            operations. if you have blocking this , you cann't receive any info from
     *            framework.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public void createReprocessableCaptureSession(@Nonnull InputConfiguration inputConfig,
                    @Nonnull List<Surface> outputs,
                    @Nonnull Camera2CaptureSessionProxy.StateCallback callback,
                    @Nullable Handler handler) throws CameraAccessException {
        // Check precondition
        Preconditions.checkNotNull(inputConfig,
                        "createReprocessableCaptureSession,the inputConfig nust not null");
        Preconditions.checkNotNull(outputs,
                        "createReprocessableCaptureSession,the surface must not null");
        Preconditions.checkNotNull(callback,
                        "createReprocessableCaptureSession, the state callback must not null");

        Message msg = mRequestHandler.obtainMessage(
                Camera2Actions.CREATE_REPROCESSABLE_CAPTURE_SESSION,
                        new SessionCreatorInfo(outputs, callback, handler, inputConfig));
        mAtomAccessor.sendAtomMessage(mRequestHandler, msg);
    }

    /**
     * See {@code CameraDevice#createConstrainedHighSpeedCaptureSession}.
     *
     * @param outputs
     *            The new set of Surfaces that should be made available as targets for captured
     *            image data.
     * @param callback
     *            The callback to notify about the status of the new capture session.
     * @param handler
     *            The handler on which the callback should be invoked, or {@code null} to use the
     *            default thread's {@link android.os.Looper looper}. the default thread is not main
     *            thread, so you cann't do UI operations when callback invoked. if the callback is
     *            invoked by default thread, please don't blocking the thread, such as long time
     *            operations. if you have blocking this , you cann't receive any info from
     *            framework.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public void createConstrainedHighSpeedCaptureSession(@Nonnull List<Surface> outputs,
                    @Nonnull Camera2CaptureSessionProxy.StateCallback callback,
                    @Nullable Handler handler) throws CameraAccessException {
        // Check precondition
        Preconditions.checkNotNull(outputs,
                        "createConstrainedHighSpeedCaptureSession,the surface must not null");
        Preconditions.checkNotNull(callback,
                        "createConstrainedHighSpeedCaptureSession, the callback must not null");

        Message msg = mRequestHandler.obtainMessage(
                Camera2Actions.CREATE_CONSTRAINED_HIGH_SPEED_CAPTURE_SESSION,
                        new SessionCreatorInfo(outputs, callback, handler, null));
        mAtomAccessor.sendAtomMessage(mRequestHandler, msg);
    }

    /**
     * See {@code CameraDevice#createCaptureRequest}.
     *
     * @param templateType
     *            An enumeration selecting the use case for this request;
     * @return a CaptureRequest.Builder.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    @Nonnull
    public CaptureRequest.Builder createCaptureRequest(int templateType)
                    throws CameraAccessException {
        Message msg = mRequestHandler.obtainMessage(
                Camera2Actions.CREATE_CAPTURE_REQUEST, templateType, 0,
                        new RequestCreatorInfo(null));
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        return mCaptureBuilder;
    }

    /**
     * See {@code CameraDevice#createReprocessCaptureRequest}.
     *
     * @param inputResult
     *            The capture result of the output image or one of the output images used to
     *            generate the reprocess input image for this capture request.
     * @return a CaptureRequest.Builder.
     * @throws CameraAccessException
     *             the camera device is no longer connected or has encountered a fatal error.
     */
    @Nonnull
    public CaptureRequest.Builder createReprocessCaptureRequest(
                    @Nonnull TotalCaptureResult inputResult) throws CameraAccessException {
        Preconditions.checkNotNull(inputResult,
                        "createReprocessCaptureRequest,the total capture result must not null");
        RequestCreatorInfo info = new RequestCreatorInfo(inputResult);
        Message msg = mRequestHandler.obtainMessage(
                Camera2Actions.CREATE_REPROCESSABLE_CAPTURE_REQUEST, info);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        return mReprocessBuilder;
    }

    /**
     * A callback objects for receiving updates about the state of a camera2 device proxy.
     * <p>
     * A callback instance must be provided to the {@link CameraDeviceManager#openCamera} or
     * {@link CameraDeviceManager#openCameraSync} method to open a camera2 device proxy.
     * </p>
     *
     * @see CameraDeviceManager#openCamera
     * @see CameraDeviceManager#openCameraSync
     */
    public static abstract class StateCallback extends CameraStateCallback {

        /**
         * The method called when a camera2 device has finished opening.
         * <p>
         * At this point, the camera2 device is ready to use, and
         * {@link CameraDevice#createCaptureSession} can be called to set up the first capture
         * session.
         * </p>
         *
         * @param camera2proxy
         *            the camera2 device proxy that has become opened
         */
        public abstract void onOpened(@Nonnull Camera2Proxy camera2proxy); // Must implement

        /**
         * The method called when a camera2 device proxy has been closed with
         * {@link Camera2Proxy#close}.
         * <p>
         * Any attempt to call methods on this CameraProxy in the future will throw a
         * {@link IllegalStateException}.
         * </p>
         * <p>
         * The default implementation of this method does nothing.
         * </p>
         *
         * @param camera2Proxy
         *            the camera2 device that has become closed
         */
        public void onClosed(@Nonnull Camera2Proxy camera2Proxy) {
        }

        /**
         * The method called when a camera2 device is no longer available for use.
         * <p>
         * This callback may be called instead of {@link #onOpened} if opening the camera fails.
         * </p>
         *
         * @param camera2proxy
         *            the device2 device that has been disconnected
         */
        public abstract void onDisconnected(@Nonnull Camera2Proxy camera2proxy);

        /**
         * The method called when a camera2 device has encountered a serious error.
         * <p>
         * This callback may be called instead of {@link #onOpened} if opening the camera fails.
         * </p>
         *
         * @param camera2Proxy
         *            The device reporting the error
         * @param error
         *            The error code, one of the {@code CameraDevice.StateCallback.ERROR_*} values.
         */
        public abstract void onError(@Nonnull Camera2Proxy camera2Proxy, int error);
    }

    /**
     * This class is used for wrapper the session info, such as surface,handler and callback.
     */
    final class SessionCreatorInfo {
        final List<Surface> mSurfaces;
        final Camera2CaptureSessionProxy.StateCallback mCallback;
        final Handler mHandler;
        final InputConfiguration mInputConfiguration;
        final Builder mBuilder;
        final List<OutputConfiguration> mOutputConfigs;

        SessionCreatorInfo(List<Surface> surfaces,
                        Camera2CaptureSessionProxy.StateCallback callback, Handler handler,
                        InputConfiguration configuration) {
            mSurfaces = surfaces;
            mCallback = callback;
            mHandler = handler;
            mInputConfiguration = configuration;
            mBuilder = null;
            mOutputConfigs = null;
        }

        SessionCreatorInfo(List<Surface> surfaces,
                           Camera2CaptureSessionProxy.StateCallback callback, Handler handler,
                           InputConfiguration configuration, Builder builder) {
            mSurfaces = surfaces;
            mCallback = callback;
            mHandler = handler;
            mInputConfiguration = configuration;
            mBuilder = builder;
            mOutputConfigs = null;
        }

        SessionCreatorInfo(List<OutputConfiguration> outputConfigurations,
                           Camera2CaptureSessionProxy.StateCallback callback, Handler handler,
                           Builder builder) {
            mSurfaces = null;
            mCallback = callback;
            mHandler = handler;
            mInputConfiguration = null;
            mBuilder = builder;
            mOutputConfigs = outputConfigurations;
        }
    }

    /**
     * This class is used for wrapper the create reprocess Capture Request.
     */
    final class RequestCreatorInfo {
        TotalCaptureResult mResult;

        RequestCreatorInfo(TotalCaptureResult result) {
            mResult = result;
        }

        void setCaptureRequestBuilder(CaptureRequest.Builder builder) {
            mCaptureBuilder = builder;
        }

        void setReprocessRequestBuilder(CaptureRequest.Builder builder) {
            mReprocessBuilder = builder;
        }
    }

    /** Whether the camera device has started to close.(may not yet have finished). */
    private boolean isClosed() {
        return mClosing.get();
    }
}