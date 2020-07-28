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

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.AutoFocusMoveCallback;
import android.hardware.Camera.FaceDetectionListener;
import android.hardware.Camera.OnZoomChangeListener;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.ShutterCallback;
import android.os.Message;
import android.view.SurfaceHolder;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.CameraProxyBase;
import com.mediatek.camera.common.device.CameraStateCallback;
import com.mediatek.camera.common.utils.AtomAccessor;

import java.io.IOException;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * A proxy for the instance of android.hardware.camera.Camera.
 */
@SuppressWarnings({"deprecation", "unused" })
public class CameraProxy extends CameraProxyBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(CameraProxy.class.getSimpleName());
    private String mCameraId = null;
    private CameraHandler mRequestHandler = null;
    private Camera mCamera = null;
    private Object mWaitDoneObject = new Object();
    private AtomAccessor mAtomAccessor = new AtomAccessor();

    /**
     * <p>Camera proxy constructor, will do the following things:</p>
     * <li>Initialize camera handler.</li>
     * @param cameraId the id of camera device.
     * @param requestHandler the handler used to make device or session operation.
     * @param camera the device than already opened.
     */
    public CameraProxy(@Nonnull String cameraId, @Nonnull CameraHandler requestHandler,
            @Nonnull Camera camera) {
        mCameraId = cameraId;
        mRequestHandler = requestHandler;
        mCamera = camera;
    }

    @Nonnull
    @Override
    public String getId() {
        return mCameraId;
    }

    @Nonnull
    @Override
    public CameraApi getApiType() {
        return CameraApi.API1;
    }

    /**
     * <p>Get the android.hardware.camera.Camera instance.</p>
     * <p>This will be null, if open camera fails.</p>
     *
     * @return camera instance.
     */
    @Nullable
    public Camera getCamera() {
        return mCamera;
    }

    /**
     * Sets the surface to be used for live preview.
     *
     * @param holder containing the Surface on which to place the preview,
     *     or null to remove the preview surface
     * @throws IOException if the method fails (for example, if the surface
     *     is unavailable or unsuitable).
     */
    public void setPreviewDisplay(SurfaceHolder holder) throws IOException {
        Message msg = mRequestHandler.obtainMessage(CameraActions.SET_PREVIEW_DISPLAY, holder);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Sets the {@link SurfaceTexture} to be used for live preview.
     *
     * @param surfaceTexture the {@link SurfaceTexture} to which the preview
     *     images are to be sent or null to remove the current preview surface
     * @throws IOException if the method fails (for example, if the surface
     *     texture is unavailable or unsuitable).
     */
    public void setPreviewTexture(SurfaceTexture surfaceTexture) throws IOException {
        Message msg = mRequestHandler.obtainMessage(
                CameraActions.SET_PREVIEW_TEXTURE, surfaceTexture);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Starts capturing and drawing preview frames to the screen.
     *
     * <p>Note: this is synchronous function call.</p>
     */
    public void startPreview() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.START_PREVIEW);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Starts capturing and drawing preview frames to the screen.
     *
     * <p>Note: this is asynchronous function call.</p>
     */
    public void startPreviewAsync() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.START_PREVIEW);
        mAtomAccessor.sendAtomMessage(mRequestHandler, msg);
    }

    /**
     * Stops capturing and drawing preview frames to the surface, and
     * resets the camera for a future call to {@link #startPreview()}.
     *
     * <p>Note: this is synchronous function call.</p>
     *
     */
    public void stopPreview() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.STOP_PREVIEW);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Stops capturing and drawing preview frames to the surface, and
     * resets the camera for a future call to {@link #startPreview()}.
     *
     * <p>Note: this is asynchronous function call.</p>
     *
     */
    public void stopPreviewAsync() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.STOP_PREVIEW);
        mAtomAccessor.sendAtomMessage(mRequestHandler, msg);
    }

    /**
     * Installs a callback to be invoked for every preview frame in addition
     * to displaying them on the screen.
     *
     * @param cb a callback object that receives a copy of each preview frame,
     *     or null to stop receiving callbacks.
     */
    public void setPreviewCallback(PreviewCallback cb) {
        Message msg = mRequestHandler.obtainMessage(CameraActions.SET_PREVIEW_CALLBACK, cb);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Installs a callback to be invoked for the next preview frame in
     * addition to displaying it on the screen.
     *
     * @param cb a callback object that receives a copy of the next preview frame,
     *     or null to stop receiving callbacks.
     */
    public void setOneShotPreviewCallback(PreviewCallback cb) {
        Message msg = mRequestHandler.obtainMessage(
                CameraActions.SET_ONE_SHOT_PREVIEW_CALLBACK, cb);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Installs a callback to be invoked for every preview frame, using
     * buffers supplied with {@link #addCallbackBuffer(byte[])}, in addition to
     * displaying them on the screen.
     *
     * @param cb a callback object that receives a copy of the preview frame,
     *     or null to stop receiving callbacks and clear the buffer queue.
     */
    public void setPreviewCallbackWithBuffer(PreviewCallback cb) {
        Message msg =
                mRequestHandler.obtainMessage(CameraActions.SET_PREVIEW_CALLBACK_WITH_BUFFER, cb);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Adds a pre-allocated buffer to the preview callback buffer queue.
     *
     * @param callbackBuffer he buffer to add to the queue. The size of the
     *   buffer must match the values described above.
     */
    public void addCallbackBuffer(byte[] callbackBuffer) {
        Message msg =
                mRequestHandler.obtainMessage(CameraActions.ADD_CALLBACK_BUFFER, callbackBuffer);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Starts camera auto-focus and registers a callback function to run when
     * the camera is focused.
     *
     * @param cb the callback to run
     */
    public void autoFocus(AutoFocusCallback cb) {
        Message msg = mRequestHandler.obtainMessage(CameraActions.AUTO_FOCUS, cb);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Cancels any auto-focus function in progress.
     */
    public void cancelAutoFocus() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.CANCEL_AUTO_FOCUS);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
      * Sets camera auto-focus move callback.
      *
      * @param cb the callback to run
      */
    public void setAutoFocusMoveCallback(AutoFocusMoveCallback cb) {
        Message msg = mRequestHandler.obtainMessage(CameraActions.SET_AUTO_FOCUS_MOVE_CALLBACK, cb);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Triggers an image capture. The camera service will initiate
     * a series of callbacks to the application as the image capture progresses.
     *
     * <p>Note: this is synchronous function call.</p>
     *
     * @param shutter   the callback for image capture moment, or null
     * @param raw       the callback for raw (uncompressed) image data, or null
     * @param postView  callback with post view image data, may be null
     * @param jpeg      the callback for JPEG image data, or null
     */
    public void takePicture(ShutterCallback shutter, PictureCallback raw,
                        PictureCallback postView, PictureCallback jpeg) {
        Message msg = mRequestHandler.obtainMessage(CameraActions.TAKE_PICTURE,
                new CaptureCallbackGroup(shutter, raw, postView, jpeg));
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Triggers an image capture. The camera service will initiate
     * a series of callbacks to the application as the image capture progresses.
     *
     * <p>Note: this is asynchronous function call.</p>
     *
     * @param shutter   the callback for image capture moment, or null
     * @param raw       the callback for raw (uncompressed) image data, or null
     * @param postView  callback with post view image data, may be null
     * @param jpeg      the callback for JPEG image data, or null
     */
    public void takePictureAsync(ShutterCallback shutter, PictureCallback raw,
                        PictureCallback postView, PictureCallback jpeg) {
        Message msg = mRequestHandler.obtainMessage(CameraActions.TAKE_PICTURE,
                new CaptureCallbackGroup(shutter, raw, postView, jpeg));
        mAtomAccessor.sendAtomMessage(mRequestHandler, msg);
    }

    /**
     * Zooms to the requested value smoothly.
     *
     * @param value zoom value.
     */
    public void startSmoothZoom(int value) {
        Message msg = mRequestHandler.obtainMessage(CameraActions.START_SMOOTH_ZOOM, value);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Stops the smooth zoom.
     */
    public void stopSmoothZoom() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.STOP_SMOOTH_ZOOM);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Set the clockwise rotation of preview display in degrees.
     *
     * @param degrees the angle that the picture will be rotated clockwise.
     */
    public void setDisplayOrientation(int degrees) {
        Message msg = mRequestHandler.obtainMessage(CameraActions.SET_DISPLAY_ORIENTATION, degrees);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Enable or disable the default shutter sound when taking a picture.
     *
     * @param enabled enabled whether the camera should play the system shutter sound
     *                when {@link #takePicture takePicture} is called.
     * @return {@code true} if the shutter sound state was successfully
     *         changed. {@code false} if the shutter sound state could not be
     *         changed.
     */
    public boolean enableShutterSound(boolean enabled) {
        boolean[] isEnabled = new boolean[1];
        Message msg = mRequestHandler.obtainMessage(CameraActions.ENABLE_SHUTTER_SOUND, isEnabled);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        return isEnabled[0];
    }

    /**
     * Registers a listener to be notified when the zoom value is updated by the
     * camera driver during smooth zoom.
     *
     * @param listener the listener to notify
     */
    public void setZoomChangeListener(OnZoomChangeListener listener) {
        Message msg =
                mRequestHandler.obtainMessage(CameraActions.SET_ZOOM_CHANGE_LISTENER, listener);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Registers a listener to be notified about the faces detected in the
     * preview frame.
     *
     * @param listener the listener to notify
     */
    public void setFaceDetectionListener(FaceDetectionListener listener) {
        Message msg =
                mRequestHandler.obtainMessage(CameraActions.SET_FACE_DETECTION_LISTENER, listener);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Starts the face detection. This should be called after preview is started.
     */
    public void startFaceDetection() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.START_FACE_DETECTION);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Stops the face detection.
     */
    public void stopFaceDetection() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.STOP_FACE_DETECTION);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Changes the settings for this Camera service.
     *
     * @param params the Parameters to use for this Camera service.
     */
    public void setParameters(Parameters params) {
        Message msg = mRequestHandler.obtainMessage(CameraActions.SET_PARAMETERS, params);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Get the original parameters.
     *
     * @param needNewInstance whether need a new instance of Parameters.
     * @return an new instance of parameters.
     */
    public Parameters getOriginalParameters(boolean needNewInstance) {
        if (needNewInstance) {
            Parameters[] params = new Parameters[1];
            Message msg = mRequestHandler.obtainMessage(CameraActions.GET_ORIGINAL_PARAMETERS,
                    needNewInstance ? 1 : 0 , 0, params);
            boolean isFromNative = mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
            if (isFromNative && params[0] != null) {
                return params[0];
            } else {
                return mRequestHandler.getOriginalParameters();
            }
        }
        return mRequestHandler.getOriginalParameters();
    }

    /**
     * Returns the current settings for this Camera service.
     *
     * @return the parameters
     */
    public Parameters getParameters() {
        Parameters[] params = new Parameters[1];
        Message msg = mRequestHandler.obtainMessage(CameraActions.GET_PARAMETERS, params);
        boolean isFromNative = mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
        if (isFromNative && params[0] != null) {
            return params[0];
        } else {
            return mRequestHandler.getOriginalParameters();
        }
    }

    /**
     * Send command to camera native.
     *
     * @param command the indicator of the command.
     * @param arg1 the first argument of the command.
     * @param arg2 the second argument of the command.
     */
    public void sendCommand(int command, int arg1, int arg2) {
        Message msg = mRequestHandler.obtainMessage(CameraActions.SEND_COMMAND,
                new CommandInfo(command, arg1, arg2));
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Installs a callback to be invoked for vendor-defined data.
     *
     * @param msgId the indicator of callback.
     * @param cb the callback to run.
     */
    public void setVendorDataCallback(int msgId, VendorDataCallback cb) {
        Message msg = mRequestHandler.obtainMessage(CameraActions.SET_VENDOR_DATA_CALLBACK,
                new VendCallbackInfo(msgId, cb));
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Re-locks the camera to prevent other processes from accessing it.
     * @param needCallToNative whether need to call camera device's lock
     */
    public void lock(boolean needCallToNative) {
        if (needCallToNative) {
            Message msg = mRequestHandler.obtainMessage(CameraActions.LOCK);
            mAtomAccessor.sendAtomMessageAtFrontOfQueue(mRequestHandler, msg);
        }
        mResumeLockRunnable.run();
    }

    /**
     * Unlocks the camera to allow another process to access it.
     *
     */
    public void unlock() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.UNLOCK);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg, mWaitLockRunnable);
    }

    /**
     * Reconnects to the camera service after another process used it.
     *
     * @throws IOException if a connection cannot be re-established (for
     *     example, if the camera is still in use by another process).
     */
    public void reconnect() throws IOException {
        Message msg = mRequestHandler.obtainMessage(CameraActions.RECONNECT);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }

    /**
     * Disconnects and releases the Camera object resources.
     *
     * <p>Note: this is synchronous function call.</p>
     */
    public void close() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.CLOSE);
        mAtomAccessor.sendAtomMessageAndWait(mRequestHandler, msg);
    }


    /**
     * Disconnects and releases the Camera object resources.
     *
     * <p>Note: this is asynchronous function call.</p>
     */
    public void closeAsync() {
        Message msg = mRequestHandler.obtainMessage(CameraActions.CLOSE);
        mAtomAccessor.sendAtomMessage(mRequestHandler, msg);
    }

    /**
     * Vendor data callback interface used to supply
     * vendor-defined data from a camera native.
     */
    public interface VendorDataCallback {
        /**
         * Called when vendor-defined data is available.
         *
         * @param message message object from native.
         */
        void onDataTaken(Message message);

        /**
         * Called when vendor-defined data is available.
         *
         * @param msgId The id of message which contains the callback info.
         * @param data Callback data.
         * @param arg1 The first argument.
         * @param arg2 The second argument.
         */
        void onDataCallback(int msgId, byte[] data, int arg1, int arg2);
    }

    /**
     * A callback objects for receiving updates about the state of a camera device proxy.
     *
     * <p>A callback instance must be provided to the CameraDeviceManager#openCamera
     * or CameraDeviceManager#openCameraSync method to
     * open a camera device proxy.</p>
     *
     */
    public static abstract class StateCallback extends CameraStateCallback {
        /**
         * The method called when a camera device has finished opening.
         *
         * @param cameraProxy the camera device proxy that has become opened
         */
        public abstract void onOpened(@Nonnull CameraProxy cameraProxy);

        /**
         * The method called when a camera device proxy has been closed with
         * {@link CameraProxy#close}.
         *
         * <p>Any attempt to call methods on this CameraProxy in the
         * future will throw a {@link IllegalStateException}.</p>
         *
         * <p>The default implementation of this method does nothing.</p>
         *
         * @param cameraProxy the camera device that has become closed
         *
         */
        public abstract void onClosed(@Nonnull CameraProxy cameraProxy);

        /**
         * The method called when a camera device is no longer available for
         * use.
         *
         * @param cameraProxy the device device that has been disconnected
         */
        public abstract void onDisconnected(@Nonnull CameraProxy cameraProxy);

        /**
         * The method called when a camera device has encountered a serious error.
         *
         * <p>This callback may be called instead of {@link #onOpened}
         * if opening the camera fails.</p>
         *
         * @param cameraProxy The device reporting the error.
         * @param error See {@code Camera#CAMERA_ERROR_UNKNOWN},
         *                  {@code Camera#CAMERA_ERROR_EVICTED},
         *                  {@code Camera#CAMERA_ERROR_SERVER_DIED}
         */
        public abstract void onError(@Nonnull CameraProxy cameraProxy, int error);
    }

    /**
     * This class is used for wrapper the capture callback,
     * such as ShutterCallback,PictureCallback.
     */
    final class CaptureCallbackGroup {
        final ShutterCallback mShutterCallback;
        final PictureCallback mRawCallback;
        final PictureCallback mPostViewCallback;
        final PictureCallback mJpegCallback;

        CaptureCallbackGroup(ShutterCallback shutterCallback,
                PictureCallback raw, PictureCallback postView,
                PictureCallback jpeg) {
            mShutterCallback = shutterCallback;
            mRawCallback = raw;
            mPostViewCallback = postView;
            mJpegCallback = jpeg;
        }
    }

    /**
     * This class is used for wrapper the command info,
     * such as ShutterCallback,PictureCallback.
     */
    final class CommandInfo {
        final int mCommand;
        final int mArg1;
        final int mArg2;

        CommandInfo(int command, int arg1, int arg2) {
            mCommand = command;
            mArg1 = arg1;
            mArg2 = arg2;
        }
    }

    /**
     * This class is used for wrapper the command info,
     * such as ShutterCallback,PictureCallback.
     */
    final class VendCallbackInfo {
        final int mMsgId;
        final VendorDataCallback mArg1;

        VendCallbackInfo(int msgId, VendorDataCallback callback) {
            mMsgId = msgId;
            mArg1 = callback;
        }
    }

    /**
     * Runnable used to pending camera handler thread to do nothing until lock camera is run.
     */
    final Runnable mWaitLockRunnable = new Runnable() {
        @Override
        public void run() {
            synchronized (mWaitDoneObject) {
                try {
                    LogHelper.d(TAG, "[waitLockRunnable] wait +");
                    mWaitDoneObject.wait();
                    LogHelper.d(TAG, "[waitLockRunnable] wait -");
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    };
    /**
     * Notify that thread which is waiting lock camera,and after this notify camera handler.
     * can handle messages which are pended in handler queue
     */
    final Runnable mResumeLockRunnable = new Runnable() {
        @Override
        public void run() {
            synchronized (mWaitDoneObject) {
                mWaitDoneObject.notifyAll();
                LogHelper.d(TAG, "[resumeLockRunnable] notifyAll ");
            }
        }
    };
}
