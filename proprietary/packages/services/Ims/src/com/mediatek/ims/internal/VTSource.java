/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ims.internal;

import android.content.Context;
import android.graphics.PointF;
import android.graphics.Rect;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCaptureSession.StateCallback;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.params.OutputConfiguration;
import android.hardware.camera2.params.SessionConfiguration;
import android.net.Uri;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.util.Range;
import android.view.Surface;

import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.Executor;
import java.util.List;

/**
 * Provider source buffer for video call.
 * @hide
 */
public class VTSource {

    /**
     * The interface used to callback VTSource event
     */
    public interface EventCallback {
        void onError();
        void onOpenSuccess();
        void onOpenFail();
    }

    /**
     * The structure used to describe one camera sensor's resolution and install orientation.
     */
    public static final class Resolution {
        int mId;
        int mMaxWidth;
        int mMaxHeight;
        int mDegree;
        int mFacing;
        int mHal;
         @Override
        public String toString() {
            return " mId: " + mId +
                   " mMaxWidth: " + mMaxWidth +
                   " mMaxHeight: " + mMaxHeight +
                   " mDegree: " + mDegree +
                   " mFacing: " + mFacing +
                   " mHal: " + mHal;
        }
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

    public static final int VT_SRV_CALL_3G = 1;
    public static final int VT_SRV_CALL_4G = 2;
    public static final int CAMERA_HARWARE_LEVEL_1 = 1;
    public static final int CAMERA_HARWARE_LEVEL_3 = 3;
    private static final String TAG = "VT SRC";
    private static final int TIME_OUT_MS = 6500; //ms,the timeout in legacy camera is 4000ms.
    private final EventCallback mEventCallBack;
    private final CameraManager mCameraManager;
    private final int mMode;
    protected static Context sContext;
    protected static Resolution[] sCameraResolutions;
    private HandlerThread mRequestThread;
    private Handler mRequestHandler;

    private Surface mCachedPreviewSurface;
    private Surface mCachedRecordSurface;
    private boolean mNeedRecordStream;
    private boolean mIsWaitRelease = false;

    /**
    stoppreviewAndRecord be called only in case call deInit.
    So we use this flag check whether is already in call end flow.
    **/
    private boolean mStopPreviewAndRecord = false;

    private String mTAG;
    private AtomicInteger mMessageId = new AtomicInteger(0);

    /**
     * Set Context to VTSource.
     * @param context from IMS.
     * @hide
     */
    public static void setContext(Context context) {
        Log.d(TAG, "[STC] [setContext] context:" + context);
        sContext = context;
    }

    /**
     * Get current platform's all camera resolutions when boot then send to MA.
     * @return an array of all camera's resolution.
     * @hide
     */
    public static Resolution[] getAllCameraResolutions() {

        Log.d(TAG, "[STC] [getAllCameraResolutions] Start");

        if (sCameraResolutions == null) {
            ArrayList<Resolution> sensorResolutions = new ArrayList<>();
            CameraManager cameraManager =
                    (CameraManager) sContext.getSystemService(Context.CAMERA_SERVICE);
            try {
                String[] cameraIds = cameraManager.getCameraIdList();
                for (String cameraId : cameraIds) {
                    Resolution resolution = new Resolution();
                    CameraCharacteristics characteristics =
                            cameraManager.getCameraCharacteristics(cameraId);
                    Rect sensorRes = characteristics.get(
                            CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);
                    int sensorOrientation = characteristics.get(
                            CameraCharacteristics.SENSOR_ORIENTATION);
                    int facing = characteristics.get(
                            CameraCharacteristics.LENS_FACING);

                    int hal = CAMERA_HARWARE_LEVEL_3;
                    if (CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_LEGACY ==
                            characteristics.get(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL)) {
                        hal = CAMERA_HARWARE_LEVEL_1;
                    }

                    resolution.mId = Integer.valueOf(cameraId);
                    resolution.mMaxWidth = sensorRes.width();
                    resolution.mMaxHeight = sensorRes.width();
                    resolution.mDegree = sensorOrientation;
                    resolution.mFacing = facing;
                    resolution.mHal = hal;
                    Log.w(TAG, "[getAllCameraResolutions] " + resolution);
                    sensorResolutions.add(resolution);
                }
            } catch (Exception  e) {
                Log.e(TAG, "[STC] [getAllCameraResolutions] getCameraIdList with exception:"
                        + e);
            }
            if (sensorResolutions.size() > 0) {
                sCameraResolutions = new Resolution[sensorResolutions.size()];
                sCameraResolutions = sensorResolutions.toArray(sCameraResolutions);
            }
            Log.d(TAG, "[STC] [getAllCameraResolutions] resolution size:"
                    + sensorResolutions.size());
        }
        Log.d(TAG, "[STC] [getAllCameraResolutions] Finish");
        return sCameraResolutions;
    }

    /**
     * New VTSource with 3G/4G mode;
     * 4G need rotate buffer to portrait.
     * 3G no need rotate buffer, keep it the same with sensor orientation.
     * @param mode current mode.
     * @param callId  call ID.
     * @hide
     */
    public VTSource(int mode, int callId, EventCallback cb) {
        mTAG = "VT SRC - " + callId;

        Log.d(mTAG, "[INT] [VTSource] Start, mode: " + mode);

        mMode = mode;
        mEventCallBack = cb;
        mCameraManager = (CameraManager) sContext.getSystemService(Context.CAMERA_SERVICE);
        createRequestThreadAndHandler();
        Log.d(mTAG, "[INT] [VTSource] Finish");
    }

    public VTSource() {
        mMode = VT_SRV_CALL_4G;
        mEventCallBack = null;
        mCameraManager = null;
    }

    /**
     * Set replace picture path.
     * @param uri the replaced picture's uri.
     * @hide
     */
    public void setReplacePicture(Uri uri) {
        Log.d(mTAG, "[INT] [setReplacePicture] uri:" + uri);
    }

    /**
     * open camera, if another camera is running, switch camera.
     * @param cameraId indicate which camera to be opened.
     * @hide
     */
    public void open(String cameraId) {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [open] Start, id : " + cameraId + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [open] Fail" + " [" + messageId + "]");
            return;
        }

        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_OPEN_CAMERA, messageId, 0, cameraId)
                .sendToTarget();

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [open] Finish" + " [" + messageId + "]");
    }

    /**
     * Close current opened camera.
     * @hide
     */
    public void close() {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [close] Start" + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [close] Fail" + " [" + messageId + "]");
            return;
        }

        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_CLOSE_CAMERA, messageId, 0).sendToTarget();

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [close] Finish" + " [" + messageId + "]");
    }

    public void restart() {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [restart] Start" + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [restart] Fail" + " [" + messageId + "]");
            return;
        }

        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_RESTART_CAMERA, messageId, 0).sendToTarget();

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [restart] Finish" + " [" + messageId + "]");
    }

    /**
     * Release resource when do not use it.
     * @hide
     */
    public void release() {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [release] Start" + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [release] Fail" + " [" + messageId + "]");
            return;
        }

        mIsWaitRelease = true;

        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_RELEASE, messageId, 0).sendToTarget();

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }
        mRequestHandler.removeCallbacksAndMessages(null);
        mRequestThread.quitSafely();
        mRequestThread = null;

        Log.d(mTAG, "[INT] [release] Finish" + " [" + messageId + "]");
    }

    /**
     * Set BufferQueueProducer to VTSource to put image data.
     * @param surface the surface used to receive record buffer.
     * @hide
     */
    public void setRecordSurface(Surface surface) {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [setRecordSurface] Start, surface:" + surface + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [setRecordSurface] Fail" + " [" + messageId + "]");
            return;
        }

        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_UPDATE_RECORD_SURFACE, messageId, 0, surface)
                .sendToTarget();

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [setRecordSurface] Finish" + " [" + messageId + "]");
    }

    /**
     * Update preview surface, if surface is null, do stop preview and clear cached preview surface.
     * @param surface the surface used to receive preview buffer.
     * @hide
     */
    public void setPreviewSurface(Surface surface) {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [setPreviewSurface] Start, surface:" + surface + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [setPreviewSurface] Fail");
            return;
        }

        if (surface == null) {
            mRequestHandler.obtainMessage(
                    DeviceHandler.MSG_STOP_PREVIEW, messageId, 0).sendToTarget();

        } else {

            if (mCachedPreviewSurface != null) {
                // Ex: surface:Surface(name=android.graphics.SurfaceTexture@f1fbca5)/@0x7d5913f
                // @SurfaceTexture hash code/@Surface hash code
                String[] oriSurfaceToken = mCachedPreviewSurface.toString().split("@");
                String[] newSurfaceToken = surface.toString().split("@");

                Log.d(mTAG, "[INT] [setPreviewSurface] oriSurfaceToken[1]:" + oriSurfaceToken[1] +
                                                    ", newSurfaceToken[1]:" + newSurfaceToken[1]);

                // Here we only compare surfaceTexture hash code
                if (newSurfaceToken[1].equals(oriSurfaceToken[1])) {
                    Log.d(mTAG, "[INT] [setPreviewSurface] surface not changed, ignore!" + " [" + messageId + "]");
                    return;
                }
            }

            mRequestHandler.obtainMessage(
                    DeviceHandler.MSG_START_PREVIEW, messageId, 0, surface)
                    .sendToTarget();
        }

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [setPreviewSurface] Finish" + " [" + messageId + "]");
    }

    /**
     * Perform zoom by specified zoom value.
     * @param zoomValue the wanted zoom value.
     * @hide
     */
    public void setZoom(float zoomValue) {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [setZoom] Start" + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [setZoom] Fail" + " [" + messageId + "]");
            return;
        }

        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_PERFORM_ZOOM, messageId, 0, zoomValue)
                .sendToTarget();

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [setZoom] Finish" + " [" + messageId + "]");
    }

    /**
     * Get current using camera's characteristics.
     * @return an instance of camera's characteristics, if camera closed we return null.
     */
    public CameraCharacteristics getCameraCharacteristics() {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [getCameraCharacteristics] Start" + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [getCameraCharacteristics] Fail" + " [" + messageId + "]");
            return null;
        }

        CameraCharacteristics[] characteristicses = new CameraCharacteristics[1];
        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_GET_CAMERA_CHARACTERISTICS, messageId, 0, characteristicses)
                .sendToTarget();

        if (waitDone(mRequestHandler)) {

            Log.d(mTAG, "[INT] [getCameraCharacteristics] Finish" + " [" + messageId + "]");
            return characteristicses[0];
        } else {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [getCameraCharacteristics] Finish (null)" + " [" + messageId + "]");
        return null;
    }

    /**
     * Start preview and recording.
     * @hide
     */
    public void startRecording() {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [startRecording] Start" + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [startRecording] Fail" + " [" + messageId + "]");
            return;
        }

        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_START_RECORDING, messageId, 0).sendToTarget();

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [startRecording] Finish" + " [" + messageId + "]");
    }

    /**
     * Stop recording.
     * @hide
     */
    public void stopRecording() {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [stopRecording] Start" + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [stopRecording] Fail" + " [" + messageId + "]");
            return;
        }

        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_STOP_RECORDING, messageId, 0).sendToTarget();

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [stopRecording] Finish" + " [" + messageId + "]");
    }

    /**
     * Stop preview and recording.
     * @hide
     */
    public void stopRecordingAndPreview() {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [stopRecordingAndPreview] Start" + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [stopRecordingAndPreview] Fail" + " [" + messageId + "]");
            return;
        }

        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_STOP_RECORDING_PREVIEW, messageId, 0).sendToTarget();

        mStopPreviewAndRecord = true;

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [stopRecordingAndPreview] Finish" + " [" + messageId + "]");
    }

    /**
     * If RJIL NW, replace output with picture data else drop camera data.
     * @hide
     */
    public void hideMe() {
        Log.d(mTAG, "[INT] [hideMe]");
    }

    /**
     * If RJIL NW, resume the camera output else stop dropping camera data.
     * @hide
     */
    public void showMe() {
        Log.d(mTAG, "[INT] [showMe]");
    }

    /**
     * Set device orientation.
     * @hide
     */
    public void setDeviceOrientation(int degree) {
        int messageId = mMessageId.incrementAndGet();
        Log.d(mTAG, "[INT] [setDeviceOrientation] Start, degree : " + degree
                + " [" + messageId + "]");

        if (IsHandlerThreadUnavailable()) {
            Log.d(mTAG, "[INT] [setDeviceOrientation] Fail");
            return;
        }

        mRequestHandler.obtainMessage(
                DeviceHandler.MSG_DEVICE_ORIENTATION, messageId, 0, degree)
                .sendToTarget();

        boolean ret = waitDone(mRequestHandler);
        if (!ret) {
            mEventCallBack.onError();
        }

        Log.d(mTAG, "[INT] [setDeviceOrientation] Finish" + " [" + messageId + "]");
    }

    private void createRequestThreadAndHandler() {
        if (mRequestThread == null) {
            mRequestThread = new HandlerThread("VTSource-Request");
            mRequestThread.start();
            mRequestHandler = new DeviceHandler(mRequestThread.getLooper(),
                    mMode == VT_SRV_CALL_4G, mEventCallBack);
        }
    }

    private boolean IsHandlerThreadUnavailable() {
        if (mRequestThread == null || mIsWaitRelease) {
            Log.d(mTAG, "Thread = null:" + (mRequestThread == null) + ", mIsWaitRelease:" + mIsWaitRelease);
            return true;
        } else {
            return false;
        }
    }

    /**
     * The handler used to process device operation.
     */
    private class DeviceHandler extends Handler {
        public static final int MSG_OPEN_CAMERA = 0;
        public static final int MSG_START_PREVIEW = 1;
        public static final int MSG_STOP_PREVIEW = 2;
        public static final int MSG_UPDATE_RECORD_SURFACE = 3;
        public static final int MSG_START_RECORDING = 4;
        public static final int MSG_STOP_RECORDING = 5;
        public static final int MSG_SUBMIT_REQUEST = 6;
        public static final int MSG_PERFORM_ZOOM = 7;
        public static final int MSG_GET_CAMERA_CHARACTERISTICS = 8;
        public static final int MSG_CLOSE_CAMERA = 9;
        public static final int MSG_RELEASE = 10;
        public static final int MSG_DEVICE_ORIENTATION = 11;
        public static final int MSG_RESTART_CAMERA = 12;
        public static final int MSG_STOP_RECORDING_PREVIEW = 13;

        private static final int MAX_RETRY_OPEN_CAMERA_COUNT = 25;

        private HandlerThread mRespondThread;
        private CameraDevice mCameraDevice;
        private String mCameraId;
        private int mRetryCount;
        private int mDeviceDegree;
        private boolean mNeedRestart = false;
        private ConditionVariable mDeviceConditionVariable = new ConditionVariable();
        private CameraCharacteristics mCameraCharacteristics;
        private EventCallback mEventCallBack;

        private boolean mNeedPortraitBuffer;
        private float mZoomValue = 1.0f;
        private boolean mHasAddTarget = false;
        private CameraCaptureSession mCameraCaptureSession;
        private ConditionVariable mSessionConditionVariable = new ConditionVariable();
        private List<Surface> mSessionUsedSurfaceList = new ArrayList<>();
        private List<OutputConfiguration> mOutputConfigurations = new ArrayList<>();

        DeviceHandler(Looper looper, boolean needPortraitBuffer, EventCallback cb) {
            super(looper);
            mNeedPortraitBuffer = needPortraitBuffer;
            mRespondThread = new HandlerThread("VTSource-Respond");
            mRespondThread.start();
            mDeviceDegree = 0;
            mEventCallBack = cb;
        }

        @Override
        public void handleMessage(Message msg) {

            if (mRespondThread == null) {
                Log.w(mTAG, "[handleMessage] mRespondThread null, ignore message!!");
                return;
            }

            switch (msg.what) {
                case MSG_OPEN_CAMERA:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_OPEN_CAMERA" + " [" + msg.arg1 + "]");

                    String cameraId = (String) msg.obj;
                    if (mCameraDevice != null && mCameraDevice.getId().equals(cameraId)) {
                        Log.w(mTAG, "open existing camera, ignore open!!!");
                        return;
                    }
                    // close camera if camera is running, update camera characteristics
                    prepareForOpenCamera(cameraId);
                    mRetryCount = 0;
                    mStopPreviewAndRecord = false;
                    mDeviceConditionVariable.close();
                    doOpenCamera(mCameraId);
                    mDeviceConditionVariable.block();
                    break;

                case MSG_START_PREVIEW: {

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_START_PREVIEW" + " [" + msg.arg1 + "]");

                    Surface newSurface = (Surface) msg.obj;
                    if (mCameraDevice == null || newSurface == null || !newSurface.isValid()) {
                        Log.w(mTAG, "[HDR] [handleMessage] start preview with status error, device:"
                                + mCameraDevice + ", new surface:" + newSurface);
                        if (newSurface != null && newSurface.isValid()) {
                            Log.d(mTAG, "[HDR] [handleMessage] Camera closed, "
                                    + "store the surface for use later.");
                            mCachedPreviewSurface = newSurface;
                        }
                        return;
                    }
                    if (newSurface.equals(mCachedPreviewSurface)) {
                        closeSession();
                    }
                    mCachedPreviewSurface = newSurface;
                    createSession();
                    break;
                }

                case MSG_SUBMIT_REQUEST:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_SUBMIT_REQUEST" + " [" + msg.arg1 + "]");

                    if (mCameraDevice == null || mCameraCaptureSession == null) {
                        Log.w(mTAG, "[HDR] [handleMessage] submitRepeatingRequest illegal state"
                                + ", ignore!");
                        return;
                    }
                    submitRepeatingRequest();
                    break;

                case MSG_STOP_PREVIEW:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_STOP_PREVIEW" + " [" + msg.arg1 + "]");

                    mCachedPreviewSurface = null;
                    closeSession();
                    createSession();
                    break;

                case MSG_UPDATE_RECORD_SURFACE: {

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_UPDATE_RECORD_SURFACE"
                            + " [" + msg.arg1 + "]");

                    Surface newSurface = (Surface) msg.obj;
                    if (newSurface == null && mCachedRecordSurface == null) {
                        return;
                    }

                    mCachedRecordSurface = newSurface;
                    closeSession();
                    createSession();
                    break;
                }

                case MSG_START_RECORDING:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_START_RECORDING"
                            + " [" + msg.arg1 + "]");

                    if (mCameraDevice == null ||
                            mCameraCaptureSession == null || mNeedRecordStream) {
                        Log.w(mTAG, "[HDR] [handleMessage] start recording status error, device:"
                                            + mCameraDevice
                                            + ", session:" + mCameraCaptureSession
                                            + ", record status:" + mNeedRecordStream);

                        mNeedRecordStream = true;
                        return;
                    }
                    mNeedRecordStream = true;
                    submitRepeatingRequest();
                    break;

                case MSG_STOP_RECORDING:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_STOP_RECORDING" + " [" + msg.arg1 + "]");

                    if (mNeedRecordStream) {
                        mNeedRecordStream = false;
                        closeSession();
                        createSession();
                    }
                    break;

                case MSG_STOP_RECORDING_PREVIEW:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_STOP_RECORDING_PREVIEW" +
                            " [" + msg.arg1 + "]");

                    if (mNeedRecordStream) {
                        mNeedRecordStream = false;
                        closeSession();
                    }
                    break;

                case MSG_PERFORM_ZOOM:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_PERFORM_ZOOM" + " [" + msg.arg1 + "]");

                    if (mCameraDevice == null || mCameraCaptureSession == null) {
                        Log.w(mTAG, "[HDR] [handleMessage] perform zoom with"
                                + " null device or session!!!");

                        return;
                    }
                    mZoomValue = (float) msg.obj;
                    submitRepeatingRequest();
                    break;

                case MSG_GET_CAMERA_CHARACTERISTICS:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_GET_CAMERA_CHARACTERISTICS"
                            + " [" + msg.arg1 + "]");

                    CameraCharacteristics[] characteristicses = (CameraCharacteristics[]) msg.obj;
                    characteristicses[0] = mCameraCharacteristics;
                    break;

                case MSG_CLOSE_CAMERA:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_CLOSE_CAMERA" + " [" + msg.arg1 + "]");

                    // mNeedRecordStream = false;
                    mCameraCaptureSession = null;
                    mZoomValue = 1.0f;
                    doCloseCamera();
                    break;

                case MSG_RELEASE:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_RELEASE" + " [" + msg.arg1 + "]");

                    // mNeedRecordStream = false;
                    mCameraCaptureSession = null;
                    mZoomValue = 1.0f;
                    doCloseCamera();
                    mSessionUsedSurfaceList.clear();
                    mOutputConfigurations.clear();
                    mCachedRecordSurface = null;
                    mCachedPreviewSurface = null;
                    mRespondThread.quitSafely();
                    break;

                case MSG_DEVICE_ORIENTATION:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_DEVICE_ORIENTATION"
                            + " [" + msg.arg1 + "]");

                    if(mDeviceDegree != (int) msg.obj) {
                        Log.d(mTAG, "[HDR] [handleMessage] Change device orientation from "
                                + mDeviceDegree + "to " + (int) msg.obj);
                        mDeviceDegree = (int) msg.obj;
                    }
                    break;

                case MSG_RESTART_CAMERA:

                    Log.d(mTAG, "[HDR] [handleMessage] MSG_RESTART_CAMERA" + " [" + msg.arg1 + "]");

                    closeSession();
                    mNeedRestart = true;
                    doCloseCamera();
                    break;

                default:
                    Log.d(mTAG, "[HDR] [handleMessage] what:" + msg.what + " [" + msg.arg1 + "]");
                    break;
            }
        }

        private void createSession() {

            Log.d(mTAG, "[HDR] [createSession] Start");

            if (mCameraDevice == null) {
                Log.w(mTAG, "[HDR] [createSession] mCameraDevice is null !!!");
                return;
            }
            boolean ret = prepareOutputConfiguration();
            if (mSessionUsedSurfaceList.size() <= 0 || !ret) {
                Log.w(mTAG, "[HDR] [createSession] Session surface list size <=0 "
                        + "or prepareOutputConfiguration fail");
                return;
            }

            SessionConfiguration sessionConfigByOutput = new SessionConfiguration(
                    SessionConfiguration.SESSION_REGULAR,
                    mOutputConfigurations,
                    new HandlerExecutor(new Handler(mRespondThread.getLooper())),
                    mSessionCallback);
            Log.d(mTAG, "[HDR] [createSession] Create sessionConfig");

            Builder requestBuilder = makeRequestBuilder();
            if (null == requestBuilder) {
                Log.w(mTAG, "[HDR] [createSession] requestBuilder == null");
                mEventCallBack.onError();
                return;
            }
            sessionConfigByOutput.setSessionParameters(requestBuilder.build());

            mSessionConditionVariable.close();
            try {
                mCameraDevice.createCaptureSession(sessionConfigByOutput);
            } catch (Exception e) {

                Log.e(mTAG, "[HDR] [createSession] create preview session with exception:"
                        + e);
                mEventCallBack.onError();
                return;
            }
            mSessionConditionVariable.block();

            Log.d(mTAG, "[HDR] [createSession] Finish");

        }

        private int getSessionRotationIndex(int rotation) {

            Log.d(mTAG, "[HDR] [getSessionRotationIndex] rotation = "+ rotation);

            switch (rotation) {
                case 0:
                    return 0;
                case 90:
                    return 1;
                case 180:
                    return 2;
                case 270:
                    return 3;
                default:
                    return 0;
            }
        }

        private void closeSession() {

            Log.d(mTAG, "[HDR] [closeSession] Start");

            if (mCameraCaptureSession != null) {
                try {
                    mCameraCaptureSession.abortCaptures();
                    mCameraCaptureSession.close();
                    mCameraCaptureSession = null;
                } catch (CameraAccessException e) {
                    Log.e(TAG, "[HDR] [closeSession] exception", e);
                } catch (IllegalStateException e) {
                    Log.e(TAG, "[HDR] [closeSession] exception", e);
                    e.printStackTrace();
                }

            } else {
                Log.d(mTAG, "[HDR] [closeSession] mCameraCaptureSession = NULL");
            }

            Log.d(mTAG, "[HDR] [closeSession] Finish");
        }

        private Rect calculateCropRegionByZoomValue(float zoomValue) {

            Log.d(mTAG, "[HDR] [calculateCropRegionByZoomValue] Start, zoomValue = "+ zoomValue);

            PointF center = new PointF(0.5f, 0.5f); // center only crop
            float maxZoom = mCameraCharacteristics.get(
                    CameraCharacteristics.SCALER_AVAILABLE_MAX_DIGITAL_ZOOM);
            Rect sensorArraySize = mCameraCharacteristics.get(
                    CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);

            Log.d(mTAG, "[HDR] [calculateCropRegionByZoomValue] Finish");

            return getCropRegionForZoom(
                    zoomValue,
                    center,
                    maxZoom,
                    sensorArraySize);
        }

        private Range calculateAeFpsRange() {

            Log.d(mTAG, "[HDR] [calculateAeFpsRange] Start");

            if (ImsVTProviderUtil.isVideoQualityTestMode()) {
                Range <Integer> preSetFps = new Range(30, 30);
                Log.d(mTAG, "[HDR] [calculateAeFpsRange] for VQ test, Range = [" +
                        preSetFps.getLower() + ", " + preSetFps.getUpper() + "], Finish");
                return preSetFps;
            }

            Range<Integer>[] availableFpsRange = mCameraCharacteristics.get(
                    CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);

            // Pick FPS range with highest max value, tiebreak on lower min value
            // Expect to set [5,30]
            Range<Integer> bestRange = availableFpsRange[0];
            for (Range<Integer> r : availableFpsRange) {
                if (bestRange.getUpper() < r.getUpper()) {
                    bestRange = r;
                } else if (bestRange.getUpper() == r.getUpper() &&
                    bestRange.getLower() > r.getLower()) {
                    bestRange = r;
                }
            }

            Log.d(mTAG, "[HDR] [calculateAeFpsRange] Finish, Range = [" +
                    bestRange.getLower() + ", " + bestRange.getUpper() + "]");

            return bestRange;
        }

        private Builder makeRequestBuilder() {

            Log.d(mTAG, "[HDR] [makeRequestBuilder] Start");

            mHasAddTarget = false;
            Builder builder = null;

            try {
                builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);

                Rect cropRegion = calculateCropRegionByZoomValue(mZoomValue);
                builder.set(CaptureRequest.SCALER_CROP_REGION, cropRegion);

                Range aeFps = calculateAeFpsRange();
                builder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, aeFps);

                if (ImsVTProviderUtil.is512mbProject() || ImsVTProviderUtil.isVideoQualityTestMode()) {
                    Log.d(mTAG, "[HDR] [makeRequestBuilder] 512MB project or VQtest," +
                            "turn off face detection");
                } else {
                    Log.d(mTAG, "[HDR] [makeRequestBuilder] Turn on face detection");
                    builder.set(CaptureRequest.CONTROL_MODE, CaptureRequest.CONTROL_MODE_USE_SCENE_MODE);
                    builder.set(CaptureRequest.CONTROL_SCENE_MODE, CaptureRequest.CONTROL_SCENE_MODE_FACE_PRIORITY);
                }

                Log.d(mTAG, "[HDR] [makeRequestBuilder] Add target "
                        + "mNeedRecordStream = " + mNeedRecordStream
                        + ", mCachedRecordSurface = " + mCachedRecordSurface
                        + ", mCachedPreviewSurface = " + mCachedPreviewSurface);

                if (mNeedRecordStream && mCachedRecordSurface != null &&
                        mSessionUsedSurfaceList.contains(mCachedRecordSurface)) {
                    builder.addTarget(mCachedRecordSurface);
                    mHasAddTarget = true;
                }

                if (mCachedPreviewSurface != null &&
                        mSessionUsedSurfaceList.contains(mCachedPreviewSurface)) {
                    builder.addTarget(mCachedPreviewSurface);
                    mHasAddTarget = true;
                }

                //for op01 and op09 VQ test, set focus and
                if (ImsVTProviderUtil.isVideoQualityTestMode()) {
                    Log.d(mTAG, "[HDR] [makeRequestBuilder]" +
                        " set CONTINUOUS_PICTURE");
                    builder.set(CaptureRequest.CONTROL_AF_MODE,
                            CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                }

                builder.set(CaptureRequest.CONTROL_VIDEO_STABILIZATION_MODE,
                        CaptureRequest.CONTROL_VIDEO_STABILIZATION_MODE_OFF);

            } catch  (Exception e) {

                Log.d(mTAG, "[HDR] [makeRequestBuilder] exception: " + e);
                e.printStackTrace();
                mEventCallBack.onError();
            }

            Log.d(mTAG, "[HDR] [makeRequestBuilder] Finish");

            return builder;
        }

        private void submitRepeatingRequest() {

            Log.d(mTAG, "[HDR] [submitRepeatingRequest] Start");

            if (mCameraDevice == null || mCameraCaptureSession == null) {
                Log.w(mTAG, "submitRepeatingRequest illegal state, ignore!");
                return;
            }

            Builder requestBuilder = makeRequestBuilder();
            if (null == requestBuilder) {
                Log.w(mTAG, "submitRepeatingRequest requestBuilder == null");
                mEventCallBack.onError();
                return;
            }

            try {
                if (mHasAddTarget) {
                    mCameraCaptureSession.setRepeatingRequest(
                            requestBuilder.build(),
                            null,
                            new Handler(mRespondThread.getLooper()));
                }

                //for op01 and op09 VQ tes, trigger focus
                if (ImsVTProviderUtil.isVideoQualityTestMode()) {
                    Log.d(mTAG, "[HDR] [submitRepeatingRequest] trigger set focus once");
                    requestBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER,
                            CaptureRequest.CONTROL_AF_TRIGGER_START);
                    mCameraCaptureSession.capture(
                        requestBuilder.build(),
                        null,
                        new Handler(mRespondThread.getLooper()));
                }
            } catch (Exception e) {

                Log.d(mTAG, "[HDR] [submitRepeatingRequest] exception: " + e);
                e.printStackTrace();
                mEventCallBack.onError();
            }

            Log.d(mTAG, "[HDR] [submitRepeatingRequest] Finish");
        }

        private void prepareForOpenCamera(String cameraId) {

            Log.d(mTAG, "[HDR] [prepareForOpenCamera] Start, cameraId = "+ cameraId);

            if (mCameraId != null && !mCameraId.equals(cameraId)) {
                closeSession();
                doCloseCamera();
            }
            mCameraId = cameraId;
            try {
                mCameraCharacteristics = mCameraManager.getCameraCharacteristics(mCameraId);
            } catch (Exception e) {
                Log.e(mTAG, "[HDR] [prepareForOpenCamera] before open camera "
                        + "getCameraCharacteristics access exception: " + e);
                mEventCallBack.onError();
            }

            Log.d(mTAG, "[HDR] [prepareForOpenCamera] Finish");
        }

        private void doCloseCamera() {

            Log.d(mTAG, "[HDR] [doCloseCamera] Start");

            if (mCameraDevice != null) {
                mCameraDevice.close();
                mCameraDevice = null;

            } else {
                Log.d(mTAG, "[HDR] [doCloseCamera] mCameraDevice = NULL");
            }

            Log.d(mTAG, "[HDR] [doCloseCamera] Finish");
        }

        private boolean prepareOutputConfiguration() {

            Log.d(mTAG, "[HDR] [prepareOutputConfiguration] Start");

            mSessionUsedSurfaceList.clear();
            mOutputConfigurations.clear();

            if (mCachedPreviewSurface != null) {

                Log.d(mTAG, "[HDR] [prepareOutputConfiguration][Preview]");

                mSessionUsedSurfaceList.add(mCachedPreviewSurface);

                /* Not rotate preview by camera HAL3, current AOSP codes have FOV issue
                int bufferRotation = 0;
                if (mNeedPortraitBuffer) {
                    //rotation buffer to portrait
                    bufferRotation = getCameraRotation(mDeviceDegree, mCameraCharacteristics);
                }

                int rotationIndex = getSessionRotationIndex(bufferRotation);

                Log.d(mTAG, "[HDR] [prepareOutputConfiguration] prepareOutputConfiguration, "
                        + "set bufferRotation:" + bufferRotation
                        + ", rotationIndex: " + rotationIndex);
                */

                try {
                    mOutputConfigurations.add(new OutputConfiguration(mCachedPreviewSurface));

                } catch (Exception ex) {
                    Log.e(mTAG, "[HDR] [prepareOutputConfiguration][Preview] "
                            + "new OutputConfiguration with exception: " + ex);
                    mSessionUsedSurfaceList.remove(mCachedPreviewSurface);
                    mCachedPreviewSurface = null;
                    mEventCallBack.onError();

                    Log.d(mTAG, "[HDR] [prepareOutputConfiguration] Finish");

                    return false;
                }
            }

            if (mCachedRecordSurface != null) {

                Log.d(mTAG, "[HDR] [prepareOutputConfiguration][Record]");

                mSessionUsedSurfaceList.add(mCachedRecordSurface);
                try {
                    mOutputConfigurations.add(new OutputConfiguration(mCachedRecordSurface));

                } catch (Exception ex) {
                    Log.e(mTAG, "[HDR] [prepareOutputConfiguration][Record] "
                            + "new OutputConfiguration with exception: " + ex);
                    mSessionUsedSurfaceList.remove(mCachedRecordSurface);
                    mCachedRecordSurface = null;
                    mEventCallBack.onError();

                    Log.d(mTAG, "[HDR] [prepareOutputConfiguration] Finish");

                    return false;
                }
            }

            Log.d(mTAG, "[HDR] [prepareOutputConfiguration] Finish");

            return true;
        }

        private void doOpenCamera(String cameraId) {

            Log.d(mTAG, "[HDR] [doOpenCamera] Start");

            try {
                mCameraManager.openCamera(cameraId,
                        mDeviceCallback,
                        new Handler(mRespondThread.getLooper()));
            } catch (Exception e) {
                Log.i(mTAG, "[HDR] [doOpenCamera] open camera with access exception:" + e);
                mDeviceConditionVariable.open();
                mEventCallBack.onError();
            }

            Log.d(mTAG, "[HDR] [doOpenCamera] Finish");
        }

        private int getCameraRotation(int degrees, CameraCharacteristics characteristics) {

            Log.d(mTAG, "[HDR] [getCameraRotation] Start");

            int facing = characteristics.get(CameraCharacteristics.LENS_FACING);
            int orientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
            Log.d(mTAG, "[HDR] [getCameraRotation] degrees: " + degrees
                    + ", facing: " + facing + ", orientation: " + orientation);

            int result;
            if (facing == CameraCharacteristics.LENS_FACING_FRONT) {
                switch (degrees) {
                    case 0:
                        result = 0;
                        break;
                    case 90:
                        result = 270;
                        break;
                    case 180:
                        result = 180;
                        break;
                    case 270:
                        result = 90;
                        break;
                    default:
                        result = 0;
                        break;
                }

                /* Original logic
                result = (orientation + degrees) % 360;
                result = (360 - result) % 360; // compensate the mirror
                result = (result + 270) % 360;
                */
            } else { // back-facing
                switch (degrees) {
                    case 0:
                        result = 0;
                        break;
                    case 90:
                        result = 90;
                        break;
                    case 180:
                        result = 180;
                        break;
                    case 270:
                        result = 270;
                        break;
                    default:
                        result = 0;
                        break;
                }

                /* Original logic
                result = (orientation - degrees + 360) % 360;

                if (degrees == 0 || degrees == 180) {
                    result = (result + 270) % 360;
                } else {
                    result = (result + 90) % 360;
                }
                */
            }
            Log.d(mTAG, "[HDR] [getCameraRotation] Fisnish, Final angle = "+ result);

            return result;
        }

        private Rect getCropRegionForZoom(float zoomFactor, final PointF center,
                                                final float maxZoom, final Rect activeArray) {

            Log.d(mTAG, "[HDR] [getCropRegionForZoom] Start, zoomFactor = " + zoomFactor
                    + ", center = " + center
                    + ", maxZoom = " + maxZoom
                    + ", activeArray = " + activeArray);

            if (zoomFactor < 1.0) {
                throw new IllegalArgumentException(
                        "zoom factor " + zoomFactor + " should be >= 1.0");
            }
            if (center.x > 1.0 || center.x < 0) {
                throw new IllegalArgumentException("center.x " + center.x
                        + " should be in range of [0, 1.0]");
            }
            if (center.y > 1.0 || center.y < 0) {
                throw new IllegalArgumentException("center.y " + center.y
                        + " should be in range of [0, 1.0]");
            }
            if (maxZoom < 1.0) {
                throw new IllegalArgumentException(
                        "max zoom factor " + maxZoom + " should be >= 1.0");
            }
            if (activeArray == null) {
                throw new IllegalArgumentException("activeArray must not be null");
            }

            float minCenterLength = Math.min(Math.min(center.x, 1.0f - center.x),
                    Math.min(center.y, 1.0f - center.y));
            float minEffectiveZoom =  0.5f / minCenterLength;
            if (minEffectiveZoom > maxZoom) {
                throw new IllegalArgumentException("Requested center " + center.toString() +
                        " has minimal zoomable factor " + minEffectiveZoom + ", which exceeds max"
                        + " zoom factor " + maxZoom);
            }

            if (zoomFactor < minEffectiveZoom) {
                Log.w(mTAG, "Requested zoomFactor " + zoomFactor + " > minimal zoomable factor "
                        + minEffectiveZoom + ". It will be overwritten by " + minEffectiveZoom);
                zoomFactor = minEffectiveZoom;
            }

            int cropCenterX = (int) (activeArray.width() * center.x);
            int cropCenterY = (int) (activeArray.height() * center.y);
            int cropWidth = (int) (activeArray.width() / zoomFactor);
            int cropHeight = (int) (activeArray.height() / zoomFactor);

            Log.d(mTAG, "[HDR] [getCropRegionForZoom] Finish");

            return new Rect(
                /*left*/cropCenterX - cropWidth / 2,
                /*top*/cropCenterY - cropHeight / 2,
                /*right*/ cropCenterX + cropWidth / 2 - 1,
                /*bottom*/cropCenterY + cropHeight / 2 - 1);
        }

        private CameraDevice.StateCallback mDeviceCallback = new CameraDevice.StateCallback() {
            @Override
            public void onError(CameraDevice cameraDevice, int error) {

                Log.e(mTAG, "[HDR] [onError] error:" + error);
                if (mRetryCount < MAX_RETRY_OPEN_CAMERA_COUNT &&
                        mStopPreviewAndRecord == false &&
                       (error == CameraDevice.StateCallback.ERROR_CAMERA_IN_USE ||
                        error == CameraDevice.StateCallback.ERROR_MAX_CAMERAS_IN_USE)) {
                    //if preview and recorded both stopped, means call end, skip retry
                    mRetryCount++;
                    try {
                        Thread.sleep(400);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    doOpenCamera(mCameraId);
                    return;
                }
                mDeviceConditionVariable.open();
                mEventCallBack.onError();
                mEventCallBack.onOpenFail();
            }

            @Override
            public void onDisconnected(CameraDevice cameraDevice) {
                Log.e(mTAG, "[HDR] [onDisconnected] cameraDevice:" + cameraDevice);
                if(mCameraDevice != null) {
                    mCameraDevice.close();
                    mCameraDevice = null;
                }
                mDeviceConditionVariable.open();
            }

            @Override
            public void onOpened(CameraDevice cameraDevice) {
                Log.d(mTAG, "[HDR] [onOpened]");
                mCameraDevice = cameraDevice;
                if (mCachedPreviewSurface != null) {
                    obtainMessage(MSG_START_PREVIEW, mMessageId.incrementAndGet(), 0,
                            mCachedPreviewSurface).sendToTarget();
                    Log.d(mTAG, "[HDR] [onOpened] Send message to handler ["
                            + mMessageId.get() + "]");
                }
                mDeviceConditionVariable.open();
                mEventCallBack.onOpenSuccess();
            }

            @Override
            public void onClosed(CameraDevice cameraDevice) {
                Log.d(mTAG, "[HDR] [onClosed]");
                super.onClosed(cameraDevice);
                if (mNeedRestart) {
                    obtainMessage(MSG_OPEN_CAMERA, mMessageId.incrementAndGet(), 0, mCameraId)
                            .sendToTarget();
                    Log.d(mTAG, "[HDR] [onClosed] Send message to handler ["
                            + mMessageId.get() + "]");
                    mNeedRestart = false;
                }
                mDeviceConditionVariable.open();
            }
        };
        private StateCallback mSessionCallback = new StateCallback() {
            @Override
            public void onConfigured(CameraCaptureSession cameraCaptureSession) {
                Log.d(mTAG, "[onConfigured]");
                mCameraCaptureSession = cameraCaptureSession;
                obtainMessage(MSG_SUBMIT_REQUEST, mMessageId.incrementAndGet(), 0).sendToTarget();
                Log.d(mTAG, "[onConfigured] Send message to handler [" + mMessageId.get() + "]");
                mSessionConditionVariable.open();
            }

            @Override
            public void onConfigureFailed(CameraCaptureSession cameraCaptureSession) {
                Log.d(mTAG, "[onConfigureFailed]");
                mSessionConditionVariable.open();
                mEventCallBack.onError();
            }
        };
    }

    /**
     * Wait for the message is processed by post a runnable.
     * @param handler the post notify wait done handler.
     * @return whether wait done success.
     */
    private boolean waitDone(Handler handler) {
        if (handler == null) {
            return false;
        }
        final ConditionVariable waitDoneCondition = new ConditionVariable();
        final Runnable unlockRunnable = new Runnable() {
            @Override
            public void run() {
                synchronized (waitDoneCondition) {
                    waitDoneCondition.open();
                }
            }
        };
        synchronized (waitDoneCondition) {
            if (handler.post(unlockRunnable)) {
                waitDoneCondition.block();
                return true;
            }
        }
        return true;
    }
}
