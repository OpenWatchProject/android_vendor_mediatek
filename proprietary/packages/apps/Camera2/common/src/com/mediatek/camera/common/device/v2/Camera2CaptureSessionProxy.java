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
import android.hardware.camera2.CameraCaptureSession.CaptureCallback;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.OutputConfiguration;
import android.os.Handler;
import android.os.Message;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.AtomAccessor;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * A proxy for android.hardware.camera2.CameraCaptureSession.
 *
 */
public class Camera2CaptureSessionProxy implements AutoCloseable {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(Camera2CaptureSessionProxy.class
            .getSimpleName());
    private AtomAccessor mAtomAccessor = new AtomAccessor();


    /**
     * Create a camera2 capture session proxy.
     *
     * @param handler
     *            the handler is request handler which is calling handler.
     * @param proxy
     *            the camera2proxy which created.
     */
    public Camera2CaptureSessionProxy(Handler handler, Camera2Proxy proxy) {
        mRequestHandler = handler;
        mCamera2Proxy = proxy;
    }

    /**
     * Get the camera device proxy that this session is created for.
     *
     * @return the camera2proxy instance.
     */
    private Camera2Proxy mCamera2Proxy;
    private final Handler mRequestHandler;
    @Nonnull
    public Camera2Proxy getDevice() {
        return mCamera2Proxy;
    }

    /**
     * <p>
     * Pre-allocate all buffers for an output Surface.
     * </p>
     *
     * @param surface
     *            the surface to be pre-allocated.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public void prepare(@Nonnull Surface surface) throws CameraAccessException {
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.PREPARE, surface);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * <p>
     * Submit a request for an image to be captured by the camera device.
     * </p>
     *
     * @param request
     *            the settings for this capture
     * @param listener
     *            The callback object to notify once this request has been processed. If null, no
     *            metadata will be produced for this capture, although image data will still be
     *            produced.
     * @param handler
     *            the handler on which the listener should be invoked, or {@code null} to use the
     *            current thread's {@link android.os.Looper looper}, if current thread have no
     *            looper will occur IllegalArgumentException.
     * @return int A unique capture sequence ID used by
     *         {@link CaptureCallback#onCaptureSequenceCompleted}.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public int capture(@Nonnull CaptureRequest request, @Nullable CaptureCallback listener,
                    @Nullable Handler handler) throws CameraAccessException {
        int[] captureNum = new int[1];
        List<CaptureRequest> requestList = new ArrayList<>();
        requestList.add(request);
        SessionOperatorInfo info = new SessionOperatorInfo(requestList, listener, handler,
                        captureNum);
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.CAPTURE, info);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        return captureNum[0];
    }

    /**
     * Submit a list of requests to be captured in sequence as a burst. The burst will be captured
     * in the minimum amount of time possible, and will not be interleaved with requests submitted
     * by other capture or repeat calls.
     *
     * @param requests
     *            the list of settings for this burst capture
     * @param listener
     *            The callback object to notify each time one of the requests in the burst has been
     *            processed. If null, no metadata will be produced for any requests in this burst,
     *            although image data will still be produced.
     * @param handler
     *            the handler on which the listener should be invoked, or {@code null} to use the
     *            current thread's {@link android.os.Looper looper}, if current thread have no
     *            looper will occur IllegalArgumentException.
     * @return int A unique capture sequence ID used by
     *         {@link CaptureCallback#onCaptureSequenceCompleted}.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public int captureBurst(@Nonnull List<CaptureRequest> requests,
                    @Nullable CaptureCallback listener, @Nullable Handler handler)
                    throws CameraAccessException {
        int[] burstNum = new int[1];
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.CAPTURE_BURST,
                        new SessionOperatorInfo(requests, listener, handler, burstNum));
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        return burstNum[0];
    }

    /**
     * Request endlessly repeating capture of images by this capture session.
     *
     * @param request
     *            the request to repeat indefinitely
     * @param listener
     *            The callback object to notify every time the request finishes processing. If null,
     *            no metadata will be produced for this stream of requests, although image data will
     *            still be produced.
     * @param handler
     *            the handler on which the listener should be invoked, or {@code null} to use the
     *            current thread's {@link android.os.Looper looper}, if current thread have no
     *            looper will occur IllegalArgumentException.
     * @return int A unique capture sequence ID used by
     *         {@link CaptureCallback#onCaptureSequenceCompleted}.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public int setRepeatingRequest(@Nonnull CaptureRequest request,
                    @Nullable CaptureCallback listener, @Nullable Handler handler)
                    throws CameraAccessException {
        int[] repeatingNumber = new int[1];
        List<CaptureRequest> requestList = new ArrayList<>();
        requestList.add(request);
        SessionOperatorInfo info = new SessionOperatorInfo(requestList, listener, handler,
                        repeatingNumber);
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.SET_REPEATING_REQUEST, info);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        return repeatingNumber[0];
    }

    /**
     * <p>
     * Request endlessly repeating capture of a sequence of images by this capture session.
     * </p>
     *
     * @param requests
     *            the list of requests to cycle through indefinitely
     * @param listener
     *            The callback object to notify each time one of the requests in the repeating
     *            bursts has finished processing. If null, no metadata will be produced for this
     *            stream of requests, although image data will still be produced.
     * @param handler
     *            the handler on which the listener should be invoked, or {@code null} to use the
     *            current thread's {@link android.os.Looper looper}, if current thread have no
     *            looper will occur IllegalArgumentException.
     * @return int A unique capture sequence ID used by
     *         {@link CaptureCallback#onCaptureSequenceCompleted}.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public int setRepeatingBurst(@Nonnull List<CaptureRequest> requests,
                    @Nullable CaptureCallback listener, @Nullable Handler handler)
                    throws CameraAccessException {
        int[] burstNum = new int[1];
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.SET_REPEATING_BURST,
                        new SessionOperatorInfo(requests, listener, handler, burstNum));
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        return burstNum[0];
    }

    /**
     * A constrained high speed capture session for a {@link CameraDevice}, used for capturing high
     * speed images from the {@link CameraDevice} for high speed video recording use case.
     * <p>
     * A CameraHighSpeedCaptureSession is created by providing a set of target output surfaces to
     * {@link CameraDevice#createConstrainedHighSpeedCaptureSession}, Once created, the session is
     * active until a new session is created by the camera device, or the camera device is closed.
     * </p>
     * <p>
     * An active high speed capture session is a specialized capture session that is only
     * targeted at
     * high speed video recording (>=120fps) use case if the camera device supports high speed video
     * capability (i.e., {@link CameraCharacteristics#REQUEST_AVAILABLE_CAPABILITIES} contains
     * {@link CameraMetadata#REQUEST_AVAILABLE_CAPABILITIES_CONSTRAINED_HIGH_SPEED_VIDEO}). It only
     * accepts request lists created via {@link #createHighSpeedRequestList}, and the request
     * list can
     * only be submitted to this session via
     * {@link CameraCaptureSession#captureBurst captureBurst}, or
     * {@link CameraCaptureSession#setRepeatingBurst setRepeatingBurst}. See
     * {@link CameraDevice#createConstrainedHighSpeedCaptureSession} for more details of the
     * limitations.
     * </p>
     * <p>
     * Creating a session is an expensive operation and can take several hundred milliseconds,
     * since it
     * requires configuring the camera device's internal pipelines and allocating memory buffers for
     * sending images to the desired targets. Therefore the setup is done asynchronously, and
     * {@link CameraDevice#createConstrainedHighSpeedCaptureSession} will send the ready-to-use
     * CameraCaptureSession to the provided listener's
     * {@link CameraCaptureSession.StateCallback#onConfigured} callback. If configuration cannot be
     * completed, then the {@link CameraCaptureSession.StateCallback#onConfigureFailed} is
     * called, and
     * the session will not become active.
     * </p>
     * <p>
     * Any capture requests (repeating or non-repeating) submitted before the session is ready
     * will be
     * queued up and will begin capture once the session becomes ready. In case the session
     * cannot be
     * configured and
     * {@link CameraCaptureSession.StateCallback#onConfigureFailed onConfigureFailed} is
     * called, all queued capture requests are discarded.  </p>
     * -->
     * <p>
     * If a new session is created by the camera device, then the previous session is closed, and
     * its
     * associated {@link CameraCaptureSession.StateCallback#onClosed onClosed} callback will be
     * invoked. All of the session methods will throw an IllegalStateException if called once the
     * session is closed.
     * </p>
     * <p>
     * A closed session clears any repeating requests (as if {@link #stopRepeating} had been
     * called),
     * but will still complete all of its in-progress capture requests as normal, before a newly
     * created
     * session takes over and reconfigures the camera device.
     * </p>
     * @param request The high speed capture request that will be used to generate the high speed
     *                request list.
     * @return A unmodifiable CaptureRequest list that is suitable for constrained high speed
     *         capture.
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public List<CaptureRequest> createHighSpeedRequestList(CaptureRequest request) throws
            CameraAccessException {
        List<CaptureRequest> resultRequest = new ArrayList<>();
        List<CaptureRequest> captureRequest = new ArrayList<>();
        captureRequest.add(request);
        SessionOperatorInfo info = new SessionOperatorInfo(captureRequest, null, null,
                resultRequest);
        Message msg = mRequestHandler.obtainMessage(
                Camera2Actions.CREATE_HIGH_SPEED_REQUEST, info);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        resultRequest.addAll(info.mResultRequest);
        return resultRequest;
    }
    /**
     * <p>
     * Cancel any ongoing repeating capture set by either
     * </p>
     * .
     *
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public void stopRepeating() throws CameraAccessException {
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.STOP_REPEATING);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Discard all captures currently pending and in-progress as fast as possible.
     *
     * @throws CameraAccessException
     *             if the camera device is no longer connected or has encountered a fatal error
     */
    public void abortCaptures() throws CameraAccessException {
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.ABORT_CAPTURES);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Return if the application can submit reprocess capture requests with this camera capture
     * session.
     *
     * @return {@code true} if the application can submit reprocess capture requests with this
     *         camera capture session. {@code false} otherwise.
     */
    public boolean isReprocessable() {
        boolean[] processable = new boolean[1];
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.IS_REPROCESSABLE, processable);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        return processable[0];
    }

    /**
     * Get the input Surface associated with a reprocessable capture session.
     *
     * @return The {@link Surface} where reprocessing capture requests get the input images from. If
     *         this is not a reprocess capture session, {@code null} will be returned.
     */
    @Nullable
    public Surface getInputSurface() {
        Surface[] surface = new Surface[1];
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.GET_INPUT_SURFACE, surface);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        return surface[0];
    }

    /**
     * finalize output configurations.
     */
    public void finalizeOutputConfigurations(List<OutputConfiguration> outputConfigurations) {
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.FINALIZE_OUTPUTCONFIGURATIONS, outputConfigurations);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Close this capture session asynchronously.
     */
    @Override
    public void close() {
        Message msg = mRequestHandler.obtainMessage(Camera2Actions.CLOSE_SESSION);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * A wrapper for android.hardware.camera2.CameraCaptureSession.StateCallback.
     */
    public static abstract class StateCallback {
        /**
         * See {@code CameraCaptureSession.StateCallback#onConfigured}.
         *
         * @param session
         *            the session returned by {@link Camera2Proxy#createCaptureSession}
         */
        public abstract void onConfigured(@Nonnull Camera2CaptureSessionProxy session);

        /**
         * See {@code CameraCaptureSession.StateCallback#onConfigureFailed}.
         *
         * @param session
         *            the session returned by {@link Camera2Proxy#createCaptureSession}
         */
        public abstract void onConfigureFailed(@Nonnull Camera2CaptureSessionProxy session);

        /**
         * See {@code CameraCaptureSession.StateCallback#onReady}.
         *
         * @param session
         *            the session returned by {@link Camera2Proxy#createCaptureSession}
         */
        public void onReady(@Nonnull Camera2CaptureSessionProxy session) {
            // default empty implementation
        }

        /**
         * See {@code CameraCaptureSession.StateCallback#onActive}.
         *
         * @param session
         *            the session returned by {@link Camera2Proxy#createCaptureSession}
         */
        public void onActive(@Nonnull Camera2CaptureSessionProxy session) {
            // default empty implementation
        }

        /**
         * See {@code CameraCaptureSession.StateCallback#onClosed}.
         *
         * @param session
         *            the session returned by {@link Camera2Proxy#createCaptureSession}
         */
        public void onClosed(@Nonnull Camera2CaptureSessionProxy session) {
            // default empty implementation
        }

        /**
         * See {@code CameraCaptureSession.StateCallback#onSurfacePrepared}.
         *
         * @param session
         *            the session returned by {@link Camera2Proxy#createCaptureSession}
         * @param surface
         *            the Surface that was used with the {@link #prepare} call.
         */
        public void onSurfacePrepared(@Nonnull Camera2CaptureSessionProxy session,
                        @Nonnull Surface surface) {
            // default empty implementation
        }
    }

    /**
     * This class is used for wrapper the session parameters. Because handler just can transmit one
     * object parameters, but we need transmit more than one parameters,so we can package the infos
     * into it.
     */
    class SessionOperatorInfo {
        List<CaptureRequest> mCaptureRequest;
        CaptureCallback mCaptureCallback;
        Handler mHandler;
        int[] mSessionNum;
        volatile List<CaptureRequest> mResultRequest;

        SessionOperatorInfo(List<CaptureRequest> captureRequests, CaptureCallback captureCallback,
                        Handler handler, int[] number) {
            mCaptureRequest = captureRequests;
            mCaptureCallback = captureCallback;
            mHandler = handler;
            mSessionNum = number;
        }

        SessionOperatorInfo(List<CaptureRequest> captureRequests, CaptureCallback captureCallback,
                Handler handler, List<CaptureRequest> requests) {
            mCaptureRequest = captureRequests;
            mCaptureCallback = captureCallback;
            mHandler = handler;
            mResultRequest = requests;
        }
    }
}