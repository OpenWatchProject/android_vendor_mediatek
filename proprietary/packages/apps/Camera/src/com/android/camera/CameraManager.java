/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.camera;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.AsdCallback;
import android.hardware.Camera.AutoRamaCallback;
import android.hardware.Camera.AutoRamaMoveCallback;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.AutoFocusMoveCallback;
import android.hardware.Camera.ContinuousShotCallback;
import android.hardware.Camera.DistanceInfoCallback;
import android.hardware.Camera.ErrorCallback;
import android.hardware.Camera.FaceDetectionListener;
import android.hardware.Camera.FbOriginalCallback;
//TODO: Dng BSP build error
//import android.hardware.Camera.MetadataCallback;
import android.hardware.Camera.OnZoomChangeListener;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.ShutterCallback;
import android.hardware.Camera.StereoCameraDataCallback;
import android.hardware.Camera.StereoCameraWarningCallback;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.view.SurfaceHolder;

import static com.android.camera.Util.assertError;
import com.mediatek.camera.util.CameraPerformanceTracker;

import java.io.IOException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;

/**
 * The manager for handling the camera flow.
 */
public class CameraManager {
    private static final String TAG = "CameraManager";
    private String mSubTag = TAG;
    // private static CameraManager sCameraManager = new CameraManager();

    private Parameters mParameters;
    private boolean mParametersIsDirty;
    private IOException mReconnectException;

    private static final int RELEASE = 1;
    private static final int RECONNECT = 2;
    private static final int UNLOCK = 3;
    private static final int LOCK = 4;
    private static final int SET_PREVIEW_TEXTURE_ASYNC = 5;
    private static final int START_PREVIEW_ASYNC = 6;
    private static final int STOP_PREVIEW = 7;
    private static final int SET_PREVIEW_CALLBACK_WITH_BUFFER = 8;
    private static final int ADD_CALLBACK_BUFFER = 9;
    private static final int AUTO_FOCUS = 10;
    private static final int CANCEL_AUTO_FOCUS = 11;
    private static final int SET_AUTO_FOCUS_MOVE_CALLBACK = 12;
    private static final int SET_DISPLAY_ORIENTATION = 13;
    private static final int SET_ZOOM_CHANGE_LISTENER = 14;
    private static final int SET_FACE_DETECTION_LISTENER = 15;
    private static final int START_FACE_DETECTION = 16;
    private static final int STOP_FACE_DETECTION = 17;
    private static final int SET_ERROR_CALLBACK = 18;
    private static final int SET_PARAMETERS = 19;
    private static final int GET_PARAMETERS = 20;
    private static final int SET_PARAMETERS_ASYNC = 21;
    private static final int SET_FB_ORIGINAL_CALLBACK = 24;
    private static final int START_OBJECT_TRACKING = 25;
    private static final int STOP_OBJECT_TRACKING = 26;
    private static final int SET_OBJECT_TRACKING_LISTENER = 27;
    private static final int SET_ZSD_CAN_TAKE_CALLBACK = 28;
    // /M: JB migration start @{
    private static final int START_SMOOTH_ZOOM = 100;
    private static final int SET_AUTORAMA_CALLBACK = 101;
    private static final int SET_AUTORAMA_MV_CALLBACK = 102;
    private static final int START_AUTORAMA = 103;
    private static final int STOP_AUTORAMA = 104;
    private static final int SET_ASD_CALLBACK = 108;
    private static final int CANCEL_CONTINUOUS_SHOT = 112;
    private static final int SET_CONTINUOUS_SHOT_SPEED = 113;
    private static final int SET_CSHOT_DONE_CALLBACK = 115;
    // / @}
    private static final int SET_STEREO3D_MODE = 117;
    //
    private static final int SET_CONTINUOUS_SHOT_STATE = 122;
    // / @}
    private static final int SET_PREVIEW_SURFACEHOLDER_ASYNC = 124;

    private static final int SET_STEREO_CAMERA_DATA_CALLBACK = 125;
    private static final int SET_STEREO_CAMERA_WARNING_CALLBACK = 126;
    private static final int SET_STEREO_CAMERA_DISTANCE_CALLBACK = 128;
    private static final int SET_ONE_SHOT_PREVIEW_CALLBACK = 129;
    private static final int SET_RAW_IMAGE_CALLBACK = 132;
    private Handler mCameraHandler;
    private CameraProxy mCameraProxy;
    private ICamera mCamera;

    // Used to retain a copy of Parameters for setting parameters.
    private Parameters mParamsToSet;

    // public static CameraManager instance() {
    // return sCameraManager;
    // }

    private boolean mFaceDetectionRunning = false;

    private boolean mIsCameraInError = false;
    private ErrorCallbackWrapper mErrorCallbackWrapper;
    /*
     * the constructor for CameraManager
     */
    public CameraManager(String subTag) {
        Log.d(TAG, "[CameraManager]constructor,subTag = " + subTag);
        mSubTag = mSubTag + "/" + subTag;
        HandlerThread ht = new HandlerThread("Camera Handler" + subTag + " Thread");
        ht.start();
        mCameraHandler = new CameraHandler(ht.getLooper());
    }

    private static String getMsgLabel(int msg) {
        switch (msg) {
        case RELEASE:
            return "[release] ";
        case RECONNECT:
            return "[reconnect] ";
        case UNLOCK:
            return "[unlock] ";
        case LOCK:
            return "[lock] ";
        case SET_PREVIEW_TEXTURE_ASYNC:
            return "[setPreviewTexture] ";
        case START_PREVIEW_ASYNC:
            return "[startPreviewAsync] ";
        case STOP_PREVIEW:
            return "[stopPreview] ";
        case SET_PREVIEW_CALLBACK_WITH_BUFFER:
            return "[setPreviewCallbackWithBuffer] ";
        case ADD_CALLBACK_BUFFER:
            return "[addCallbackBuffer] ";
        case SET_RAW_IMAGE_CALLBACK:
            return "[addRawCallback] ";
        case AUTO_FOCUS:
            return "[autoFocus] ";
        case CANCEL_AUTO_FOCUS:
            return "[cancelAutoFocus] ";
        case SET_AUTO_FOCUS_MOVE_CALLBACK:
            return "[setAutoFocusMoveCallback] ";
        case SET_DISPLAY_ORIENTATION:
            return "[setDisplayOrientation] ";
        case SET_ZOOM_CHANGE_LISTENER:
            return "[setZoomChangeListener] ";
        case SET_FACE_DETECTION_LISTENER:
            return "[setFaceDetectionListener] ";
        case START_FACE_DETECTION:
            return "[startFaceDetection] ";
        case STOP_FACE_DETECTION:
            return "[stopFaceDetection] ";
        case SET_ERROR_CALLBACK:
            return "[setErrorCallback] ";
        case SET_PARAMETERS:
            return "[setParameters] ";
        case GET_PARAMETERS:
            return "[getParameters] ";
        case SET_PARAMETERS_ASYNC:
            return "[setParametersAsync] ";
        case SET_FB_ORIGINAL_CALLBACK:
            return "[setFbOriginalCallback] ";
        case START_SMOOTH_ZOOM:
            return "[startSmoothZoom] ";
        case SET_AUTORAMA_CALLBACK:
            return "[setAutoRamaCallback] ";
        case SET_AUTORAMA_MV_CALLBACK:
            return "[setAutoramraMVCallback] ";
        case START_AUTORAMA:
            return "[startAutoRama] ";
        case STOP_AUTORAMA:
            return "[stopAutoRama] ";
        case SET_ASD_CALLBACK:
            return "[setAsdCallback] ";
        case CANCEL_CONTINUOUS_SHOT:
            return "[cancelContinuousShot] ";
        case SET_CONTINUOUS_SHOT_SPEED:
            return "[setContinuousShotSpeed] ";
        case SET_CSHOT_DONE_CALLBACK:
            return "[setContinuousShotCallback] ";
        case SET_STEREO3D_MODE:
            return "[setStereo3DMode] ";
        case SET_CONTINUOUS_SHOT_STATE:
            return "[setContinousShotState] ";
        case SET_PREVIEW_SURFACEHOLDER_ASYNC:
            return "[setPreviewSurfaceHolderAsync] ";
        default:
            break;
        }
        return "unknown message msg id = " + msg;
    }

    private class CameraHandler extends Handler {
        private ErrorCallback mErrorCallback;
        CameraHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(final Message msg) {
            long now = SystemClock.uptimeMillis();
            if (mCamera == null || mIsCameraInError) {
                Log.e(mSubTag, "[handleMessage] with abnormal: device = " + mCamera
                        + ", error = " + mIsCameraInError);
                return;
            }
            if (ADD_CALLBACK_BUFFER != msg.what) {
                Log.i(mSubTag, "[handleMessage]msg.what = " + getMsgLabel(msg.what)
                        + " + pending time = " + (now - msg.getWhen()) + "ms.");
            }
            long current = now;
            try {
                switch (msg.what) {
                case RELEASE:
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_CAMERA_RELEASE,
                            CameraPerformanceTracker.ISBEGIN);
                    mCamera.release();
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_CAMERA_RELEASE,
                            CameraPerformanceTracker.ISEND);
                    mCamera = null;
                    mCameraProxy = null;
                    mFaceDetectionRunning = false;
                    return;

                case RECONNECT:
                    mReconnectException = null;
                    try {
                        mCamera.reconnect();
                    } catch (IOException ex) {
                        mReconnectException = ex;
                    }
                    mFaceDetectionRunning = false;
                    return;

                case UNLOCK:
                    mCamera.unlock();
                    return;

                case LOCK:
                    mCamera.lock();
                    return;

                case SET_PREVIEW_TEXTURE_ASYNC:
                    try {
                        mCamera.setPreviewTexture((SurfaceTexture) msg.obj);
                    } catch (IOException e) {
                        Log.e(mSubTag, "[handleMessage] IOException. ");
                        throw new RuntimeException(e);
                    }
                    return; // no need to call mSig.open()

                case START_PREVIEW_ASYNC:
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_CAMERA_START_PREVIEW,
                            CameraPerformanceTracker.ISBEGIN);
                    mCamera.startPreview();
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_CAMERA_START_PREVIEW,
                            CameraPerformanceTracker.ISEND);
                    return; // no need to call mSig.open()

                case STOP_PREVIEW:
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_CAMERA_STOP_PREVIEW,
                            CameraPerformanceTracker.ISBEGIN);
                    if (mFaceDetectionRunning) {
                        mCamera.stopFaceDetection();
                        mFaceDetectionRunning = false;
                        Log.w(TAG, "Please call stopFaceDetecton firstly before stop preview!");
                    }
                    mCamera.stopPreview();
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_CAMERA_STOP_PREVIEW,
                            CameraPerformanceTracker.ISEND);
                    return;

                case SET_PREVIEW_CALLBACK_WITH_BUFFER:
                    mCamera.setPreviewCallbackWithBuffer((PreviewCallback) msg.obj);
                    return;

                  //TODO: Dng BSP build error
//                case SET_RAW_IMAGE_CALLBACK:
//                    RawImageCallback cb = (RawImageCallback) msg.obj;
//                    mCamera.setRawImageCallback(cb.metaCallback, cb.raw16Callback);
//                    return;

                case ADD_CALLBACK_BUFFER:
                    mCamera.addCallbackBuffer((byte[]) msg.obj);
                    return;

                case AUTO_FOCUS:
                    mCamera.autoFocus((AutoFocusCallback) msg.obj);
                    return;

                case CANCEL_AUTO_FOCUS:
                    mCamera.cancelAutoFocus();
                    return;

                case SET_AUTO_FOCUS_MOVE_CALLBACK:
                    mCamera.setAutoFocusMoveCallback((AutoFocusMoveCallback) msg.obj);
                    return;

                case SET_ZSD_CAN_TAKE_CALLBACK:
                    mCamera.setUncompressedImageCallback((PictureCallback) msg.obj);
                    return;

                case SET_DISPLAY_ORIENTATION:
                    mCamera.setDisplayOrientation(msg.arg1);
                    return;

                case SET_ZOOM_CHANGE_LISTENER:
                    mCamera.setZoomChangeListener((OnZoomChangeListener) msg.obj);
                    return;

                case SET_FACE_DETECTION_LISTENER:
                    mCamera.setFaceDetectionListener((FaceDetectionListener) msg.obj);
                    return;

                case START_FACE_DETECTION:
                    if (!mFaceDetectionRunning) {
                        mCamera.startFaceDetection();
                        mFaceDetectionRunning = true;
                    } else {
                        Log.w(TAG, "face detection is running, don't need to start it again!");
                    }
                    return;

                case STOP_FACE_DETECTION:
                    if (mFaceDetectionRunning) {
                        mCamera.stopFaceDetection();
                        mFaceDetectionRunning = false;
                    } else {
                        Log.w(TAG, "face detection is stopped, don't need to stop it again!");
                    }
                    return;

                case SET_ERROR_CALLBACK:
                    mErrorCallback = (ErrorCallback) msg.obj;
                    mCamera.setErrorCallback((ErrorCallback) msg.obj);
                    return;

                case SET_PARAMETERS:
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_SET_PARAMETERS,
                            CameraPerformanceTracker.ISBEGIN);
                    mParametersIsDirty = true;
                    mParamsToSet.unflatten((String) msg.obj);
                    mCamera.setParameters(mParamsToSet);
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_SET_PARAMETERS,
                            CameraPerformanceTracker.ISEND);
                    return;

                case GET_PARAMETERS:
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_GET_PARAMETERS,
                            CameraPerformanceTracker.ISBEGIN);
                    if (mParametersIsDirty) {
                        mParameters = mCamera.getParameters();
                        mParametersIsDirty = false;
                    }
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_GET_PARAMETERS,
                            CameraPerformanceTracker.ISEND);
                    return;

                case SET_PARAMETERS_ASYNC:
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_SET_PARAMETERS,
                            CameraPerformanceTracker.ISBEGIN);
                    mParametersIsDirty = true;
                    mParamsToSet.unflatten((String) msg.obj);
                    mCamera.setParameters(mParamsToSet);
                    CameraPerformanceTracker.onEvent(TAG,
                            CameraPerformanceTracker.NAME_SET_PARAMETERS,
                            CameraPerformanceTracker.ISEND);
                    return; // no need to call mSig.open()

                case START_SMOOTH_ZOOM:
                    mCamera.startSmoothZoom(msg.arg1);
                    return;
                case SET_AUTORAMA_CALLBACK:
                    mCamera.setAutoRamaCallback((AutoRamaCallback) msg.obj);
                    return;
                case SET_AUTORAMA_MV_CALLBACK:
                    mCamera.setAutoRamaMoveCallback((AutoRamaMoveCallback) msg.obj);
                    return;
                case START_AUTORAMA:
                    mCamera.startAutoRama(msg.arg1);
                    return;
                case STOP_AUTORAMA:
                    mCamera.stopAutoRama(msg.arg1);
                    return;
                case SET_ASD_CALLBACK:
                    mCamera.setAsdCallback((AsdCallback) msg.obj);
                    return;
                case CANCEL_CONTINUOUS_SHOT:
                    mCamera.cancelContinuousShot();
                    return;
                case SET_CONTINUOUS_SHOT_SPEED:
                    mCamera.setContinuousShotSpeed(msg.arg1);
                    return;
                case SET_CSHOT_DONE_CALLBACK:
                    mCamera.setContinuousShotCallback((ContinuousShotCallback) msg.obj);
                    break;
                case SET_FB_ORIGINAL_CALLBACK:
                    mCamera.setFbOriginalCallback((FbOriginalCallback) msg.obj);
                    return;
                case SET_STEREO_CAMERA_DATA_CALLBACK:
                    mCamera.setDataCallback((StereoCameraDataCallback) msg.obj);
                    return;
                case SET_STEREO_CAMERA_WARNING_CALLBACK:
                    mCamera.setWarningCallback((StereoCameraWarningCallback) msg.obj);
                    return;
                case SET_STEREO_CAMERA_DISTANCE_CALLBACK:
                    mCamera.setDistanceInfoCallback((DistanceInfoCallback) msg.obj);
                    return;
                case SET_PREVIEW_SURFACEHOLDER_ASYNC:
                    try {
                        mCamera.setPreviewDisplay((SurfaceHolder) msg.obj);
                    } catch (IOException e) {
                        throw new RuntimeException(e);
                    }
                    break; // no need to call mSig.open()
                case SET_ONE_SHOT_PREVIEW_CALLBACK:
                    mCamera.setOneShotPreviewCallback((PreviewCallback) msg.obj);
                    break;
                default:
                    throw new RuntimeException("Invalid CameraProxy message=" + msg.what);
                }
            } catch (RuntimeException e) {
                if (msg.what != RELEASE && mCamera != null) {
                    try {
                        Log.e(TAG, "[handleMessgae]release the camera.");
                        mCamera.release();
                    } catch (Exception ex) {
                        Log.e(TAG, "Fail to release the camera.");
                    }
                    mCamera = null;
                    mCameraProxy = null;
                }
                if (!mIsCameraInError) {
                    if (mErrorCallback != null) {
                        mErrorCallback.onError(Camera.CAMERA_ERROR_UNKNOWN, null);
                    }
                }
            } finally {
                if (ADD_CALLBACK_BUFFER != msg.what) {
                    Log.i(mSubTag, "[handleMessage]msg.what = " + getMsgLabel(msg.what)
                            + " - run time = " + (SystemClock.uptimeMillis() - current) + "ms.");
                }
            }
        }
    }

    /**
     * Open camera with camera id.
     * @param cameraId the specified camera id.
     * @return ICamera instance used to control camera.
     */
    public static ICamera openCamera(int cameraId) {
            Camera camera = null;
            // choose legacy mode in order to enter cam hal 1.0
            camera = Camera.openLegacy(cameraId, Camera.CAMERA_HAL_API_VERSION_1_0);
            if (null == camera) {
                Log.e(TAG, "openCamera:got null hardware camera!");
                return null;
            }
            // wrap it with ICamera
            return new AndroidCamera(camera);
    }

    // Open camera synchronously. This method is invoked in the context of a
    // background thread.
    CameraProxy cameraOpen(int cameraId) {
        // Cannot open camera in mCameraHandler, otherwise all camera events
        // will be routed to mCameraHandler looper, which in turn will call
        // event handler like Camera.onFaceDetection, which in turn will modify
        // UI and cause exception like this:
        // CalledFromWrongThreadException: Only the original thread that created
        // a view hierarchy can touch its views.
        mIsCameraInError = false;
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_OPEN,
                CameraPerformanceTracker.ISBEGIN);
        mCamera = openCamera(cameraId);
        Log.d(mSubTag, "openCamera cameraId = " + cameraId + " camera device = " + mCamera);
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_OPEN,
                CameraPerformanceTracker.ISEND);
        if (mCamera != null) {
            mParametersIsDirty = true;
            if (mParamsToSet == null) {
                mParamsToSet = mCamera.getParameters();
            }
            mCameraProxy = new CameraProxy();
            return mCameraProxy;
        } else {
            return null;
        }
    }

    public class CameraProxy {
        private CameraProxy() {
            assertError(mCamera != null);
        }

        public ICamera getCamera() {
            return mCamera;
        }

        public void release() {
            mCameraHandler.sendEmptyMessage(RELEASE);
            waitDone();
        }

        /**
         * release Camera asynchronously.
         */
        public void releaseAsync() {
            Log.d(TAG, "releaseAsync");
            Message msg = Message.obtain();
            msg.what = RELEASE;
            mCameraHandler.sendMessageAtFrontOfQueue(msg);
        }

        public void reconnect() throws IOException {
            mCameraHandler.sendEmptyMessage(RECONNECT);
            waitDone();
            if (mReconnectException != null) {
                throw mReconnectException;
            }
        }

        public void unlock() {
            mCameraHandler.sendEmptyMessage(UNLOCK);
            waitDone();
        }

        public void lock() {
            mCameraHandler.sendEmptyMessage(LOCK);
            waitDone();
        }

        public void setPreviewTextureAsync(final SurfaceTexture surfaceTexture) {
            mCameraHandler.obtainMessage(SET_PREVIEW_TEXTURE_ASYNC, surfaceTexture).sendToTarget();
        }

        public void startPreviewAsync() {
            mCameraHandler.sendEmptyMessage(START_PREVIEW_ASYNC);
            waitDone();
        }

        public void stopPreview() {
            mCameraHandler.sendEmptyMessage(STOP_PREVIEW);
            waitDone();
        }

        public void setPreviewCallbackWithBuffer(final PreviewCallback cb) {
            mCameraHandler.obtainMessage(SET_PREVIEW_CALLBACK_WITH_BUFFER, cb).sendToTarget();
        }

        /**
         * add callback buffer.
         * @param callbackBuffer the buffer to be set to jni.
         */
        public void addCallbackBuffer(byte[] callbackBuffer) {
            mCameraHandler.obtainMessage(ADD_CALLBACK_BUFFER, callbackBuffer).sendToTarget();
        }

        /**
         * set raw callback for dng capture.
         * @param meta the callback for meta data.
         * @param raw16 the callback for image data.
         */
      //TODO: Dng BSP build error
//        public void setRawImageCallback(MetadataCallback meta, PictureCallback raw16) {
//            RawImageCallback callback = new RawImageCallback(meta, raw16);
//            mCameraHandler.obtainMessage(SET_RAW_IMAGE_CALLBACK, callback).sendToTarget();
//        }

        public void autoFocus(AutoFocusCallback cb) {
            mCameraHandler.obtainMessage(AUTO_FOCUS, cb).sendToTarget();
        }

        public void cancelAutoFocus() {
            mCameraHandler.removeMessages(AUTO_FOCUS);
            mCameraHandler.sendEmptyMessage(CANCEL_AUTO_FOCUS);
        }

        public void setAutoFocusMoveCallback(AutoFocusMoveCallback cb) {
            mCameraHandler.obtainMessage(SET_AUTO_FOCUS_MOVE_CALLBACK, cb).sendToTarget();
        }

        public void setUncompressedImageCallback(PictureCallback cb) {
            mCameraHandler.obtainMessage(SET_ZSD_CAN_TAKE_CALLBACK, cb).sendToTarget();
        }

        public void takePictureAsync(final ShutterCallback shutter, final PictureCallback raw,
                final PictureCallback postview, final PictureCallback jpeg) {
            mCameraHandler.post(new Runnable() {
                @Override
                public void run() {
                    Log.d(mSubTag, " takePictureAsync begin, mIsCameraInError:" + mIsCameraInError);
                    if (mCamera != null && !mIsCameraInError) {
                        mCamera.takePicture(shutter, raw, postview, jpeg);
                        mFaceDetectionRunning = false;
                    }
                }
            });
        }

        public void takePicture(final ShutterCallback shutter, final PictureCallback raw,
                final PictureCallback postview, final PictureCallback jpeg) {
            // Too many parameters, so use post for simplicity
            mCameraHandler.post(new Runnable() {
                @Override
                public void run() {
                    Log.d(mSubTag, " takePicture begin, mIsCameraInError:" + mIsCameraInError);
                    if (mCamera != null && !mIsCameraInError) {
                        mCamera.takePicture(shutter, raw, postview, jpeg);
                        mFaceDetectionRunning = false;
                    }
                }
            });
            waitDone();
        }

        public void setDisplayOrientation(int degrees) {
            mCameraHandler.obtainMessage(SET_DISPLAY_ORIENTATION, degrees, 0).sendToTarget();
        }

        public void setZoomChangeListener(OnZoomChangeListener listener) {
            mCameraHandler.obtainMessage(SET_ZOOM_CHANGE_LISTENER, listener).sendToTarget();
        }

        public void setFaceDetectionListener(FaceDetectionListener listener) {
            mCameraHandler.obtainMessage(SET_FACE_DETECTION_LISTENER, listener).sendToTarget();
            waitDone();
        }

        public void startFaceDetection() {
            mCameraHandler.sendEmptyMessage(START_FACE_DETECTION);
        }

        public void stopFaceDetection() {
            mCameraHandler.sendEmptyMessage(STOP_FACE_DETECTION);
        }

        /**
         * Set error callback to device.
         * @param cb receive onError message.
         */
        public void setErrorCallback(ErrorCallback cb) {
            mErrorCallbackWrapper = cb == null ? null : new ErrorCallbackWrapper(cb);
            mCameraHandler.obtainMessage(SET_ERROR_CALLBACK, mErrorCallbackWrapper).sendToTarget();
        }

        public void setParameters(Parameters params) {
            if (params == null) {
                Log.v(TAG, "null parameters in setParameters()");
                return;
            }
            mCameraHandler.obtainMessage(SET_PARAMETERS, params.flatten()).sendToTarget();
        }

        public void setParametersAsync(Parameters params) {
            mCameraHandler.removeMessages(SET_PARAMETERS_ASYNC);
            if (params == null) {
                Log.v(TAG, "null parameters in setParameters()");
                return;
            }
            mCameraHandler.obtainMessage(SET_PARAMETERS_ASYNC, params.flatten()).sendToTarget();
        }

        public void setParametersAsync(final Parameters params, final int zoomValue) {
            // Too many parameters, so use post for simplicity
            synchronized (CameraProxy.this) {
                if (mAsyncRunnable != null) {
                    mCameraHandler.removeCallbacks(mAsyncRunnable);
                }
                mAsyncRunnable = new Runnable() {
                    @Override
                    public void run() {
                        Log.d(TAG, "mAsyncRunnable.run(" + zoomValue + ") this=" + mAsyncRunnable
                                + ", mCamera=" + mCamera);
                        if (mCamera != null && mCameraProxy != null) {
                            if (!mCameraProxy.tryLockParametersRun(new Runnable() {
                                @Override
                                public void run() {
                                    CameraPerformanceTracker.onEvent(TAG,
                                            CameraPerformanceTracker.NAME_SET_PARAMETERS,
                                            CameraPerformanceTracker.ISBEGIN);
                                    // Here we use zoom value instead of
                                    // parameters for that:
                                    // parameters may be different from
                                    // current parameters.
                                    if (mCamera != null && params != null) {
                                        params.setZoom(zoomValue);
                                        mCamera.setParameters(params);
                                    }
                                    CameraPerformanceTracker.onEvent(TAG,
                                            CameraPerformanceTracker.NAME_SET_PARAMETERS,
                                            CameraPerformanceTracker.ISEND);
                                }
                            })) {
                                // Second async may changed the runnable,
                                // here we sync the new runnable and post it
                                // again.
                                synchronized (CameraProxy.this) {
                                    if (mAsyncRunnable != null) {
                                        mCameraHandler.removeCallbacks(mAsyncRunnable);
                                    }
                                    mCameraHandler.post(mAsyncRunnable);
                                    Log.d(TAG, "mAsyncRunnable.post " + mAsyncRunnable);
                                }
                            }
                        }
                    }
                };
                mCameraHandler.post(mAsyncRunnable);
                Log.d(TAG, "setParametersAsync(" + zoomValue + ") and mAsyncRunnable = "
                        + mAsyncRunnable);
            }
        }

        public Parameters getParameters() {
            mCameraHandler.sendEmptyMessage(GET_PARAMETERS);
            waitDone();
            return mParameters;
        }

        public boolean isFaceDetectionRunning() {
            return mFaceDetectionRunning;
        }

        // /M: JB migration start @{
        public void startSmoothZoom(int zoomValue) {
            mCameraHandler.obtainMessage(START_SMOOTH_ZOOM, zoomValue, 0).sendToTarget();
            waitDone();
        }

        public void setAutoRamaCallback(AutoRamaCallback autoRamaCallback) {
            mCameraHandler.obtainMessage(SET_AUTORAMA_CALLBACK, autoRamaCallback).sendToTarget();
            waitDone();
        }

        public void setAutoRamaMoveCallback(AutoRamaMoveCallback autoRamaMoveCallback) {
            mCameraHandler.obtainMessage(SET_AUTORAMA_MV_CALLBACK, autoRamaMoveCallback)
                    .sendToTarget();
            waitDone();
        }

        public void setFbOriginalCallback(FbOriginalCallback fbOriginalCallback) {
            mCameraHandler.obtainMessage(SET_FB_ORIGINAL_CALLBACK, fbOriginalCallback)
                    .sendToTarget();
            waitDone();
        }

        public void startAutoRama(int num) {
            mCameraHandler.obtainMessage(START_AUTORAMA, num, 0).sendToTarget();
            waitDone();
        }

        public void stopAutoRama(int isMerge) {
            mCameraHandler.obtainMessage(STOP_AUTORAMA, isMerge, 0).sendToTarget();
            waitDone();
        }

        public void setAsdCallback(AsdCallback asdCallback) {
            mCameraHandler.obtainMessage(SET_ASD_CALLBACK, asdCallback).sendToTarget();
            waitDone();
        }

        /**
         * Set Stereo Data Callback.
         * Need setData callback when stereo capture
         * @param dataCallback need to set.
         */
        public void setStereoCameraDataCallback(StereoCameraDataCallback dataCallback) {
            mCameraHandler.obtainMessage(SET_STEREO_CAMERA_DATA_CALLBACK, dataCallback)
                    .sendToTarget();
            waitDone();
        }

        public void setStereoCameraWarningCallback(StereoCameraWarningCallback warningCallback) {
            mCameraHandler.obtainMessage(SET_STEREO_CAMERA_WARNING_CALLBACK, warningCallback)
                    .sendToTarget();
            waitDone();
        }

        public void setStereoCameraDistanceCallback(DistanceInfoCallback distanceCallback) {
            mCameraHandler.obtainMessage(SET_STEREO_CAMERA_DISTANCE_CALLBACK, distanceCallback)
                    .sendToTarget();
            waitDone();
        }

        public void cancelContinuousShot() {
            mCameraHandler.sendEmptyMessage(CANCEL_CONTINUOUS_SHOT);
            waitDone();
        }

        public void setContinuousShotSpeed(int speed) {
            mCameraHandler.obtainMessage(SET_CONTINUOUS_SHOT_SPEED, speed, 0).sendToTarget();
            waitDone();
        }

        public void setContinuousShotCallback(ContinuousShotCallback callback) {
            mCameraHandler.obtainMessage(SET_CSHOT_DONE_CALLBACK, callback).sendToTarget();
            waitDone();
        }

        public void setPreviewDisplayAsync(SurfaceHolder holder) {
            mCameraHandler.obtainMessage(SET_PREVIEW_SURFACEHOLDER_ASYNC, holder).sendToTarget();
            waitDone();
        }

        public void setOneShotPreviewCallback(PreviewCallback cb) {
            mCameraHandler.obtainMessage(SET_ONE_SHOT_PREVIEW_CALLBACK, cb).sendToTarget();
        }

        // /M: lock parameter for ConcurrentModificationException. @{
        private Runnable mAsyncRunnable;
        private static final int ENGINE_ACCESS_MAX_TIMEOUT_MS = 500;
        private ReentrantLock mLock = new ReentrantLock();

        /**
         * Lock parameters.
         * @throws InterruptedException If lock failed, throw interrupted exception.
         */
        public void lockParameters() throws InterruptedException {
            mLock.lock();
        }

        /**
         * Unlock parameters.
         */
        public void unlockParameters() {
            mLock.unlock();
        }

        private boolean tryLockParameters(long timeoutMs) throws InterruptedException {
            boolean acquireSem = mLock.tryLock(timeoutMs, TimeUnit.MILLISECONDS);
            Log.d(TAG, "try lock: grabbed lock status " + acquireSem);

            return acquireSem;
        }

        public void lockParametersRun(Runnable runnable) {
            boolean lockedParameters = false;
            try {
                lockParameters();
                lockedParameters = true;
                runnable.run();
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParametersRun() not successfull.", ex);
            } finally {
                if (lockedParameters) {
                    unlockParameters();
                }
            }
        }

        public boolean tryLockParametersRun(Runnable runnable) {
            boolean lockedParameters = false;
            try {
                lockedParameters = tryLockParameters(ENGINE_ACCESS_MAX_TIMEOUT_MS);
                if (lockedParameters) {
                    runnable.run();
                }
            } catch (InterruptedException ex) {
                Log.e(TAG, "tryLockParametersRun() not successfull.", ex);
            } finally {
                if (lockedParameters) {
                    unlockParameters();
                }
            }
            Log.d(TAG, "tryLockParametersRun(" + runnable + ") return " + lockedParameters);
            return lockedParameters;
        }
        // / @}
    }

    // return false if cancelled.
    public boolean waitDone() {
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
                Log.v(TAG, "waitDone interrupted");
                return false;
            }
        }
        return true;
    }

    /**
     * Be used to send in message. It has two callbacks.
     * One for meta data, and the other is for image callback.
     */
  //TODO: Dng BSP build error
//    private class RawImageCallback {
//        public MetadataCallback metaCallback;
//        public PictureCallback raw16Callback;
//        RawImageCallback(MetadataCallback meta, PictureCallback raw16) {
//            this.metaCallback = meta;
//            this.raw16Callback = raw16;
//        }
//    }
    /**
     * An ErrorCallback wrapper used to update error status when onError happens.
     * <p> When onError happens, use this status to prevent any operations on
     * camera device.</p>
     *
     */
    private class ErrorCallbackWrapper implements ErrorCallback {
        private final ErrorCallback mErrorCallback;
        public ErrorCallbackWrapper(ErrorCallback errorCallback) {
            mErrorCallback = errorCallback;
        }

        @Override
        public void onError(int error, Camera camera) {
            mIsCameraInError = true;
            mErrorCallback.onError(error, camera);
            if (error == Camera.CAMERA_ERROR_SERVER_DIED) {
                mCameraHandler.sendEmptyMessage(RELEASE);
            }
        }
    }
}
