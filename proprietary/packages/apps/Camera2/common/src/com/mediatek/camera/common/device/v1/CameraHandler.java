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

import android.content.Context;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.AutoFocusMoveCallback;
import android.hardware.Camera.FaceDetectionListener;
import android.hardware.Camera.OnZoomChangeListener;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.os.Build;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.support.annotation.RequiresApi;
import android.view.Display;
import android.view.SurfaceHolder;
import android.view.WindowManager;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.HistoryHandler;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.portability.CameraEx;

import java.io.IOException;
import java.util.HashMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;

/**
 * A handler to process operations at android.hardware.camera.Camera in request thread.
 */
@SuppressWarnings("deprecation")
public class CameraHandler extends HistoryHandler {
    private final Tag mTag;
    private Parameters mOriginalParameters;
    private final IDeviceInfoListener mIDeviceInfoListener;
    private Camera mCamera;
    private Context mContext;
    private String mCameraId;
    private Lock mLockMap = new ReentrantLock();
    private HashMap<Integer, CameraProxy.VendorDataCallback> mVendorCallbackMap
            = new HashMap<>();
    private CameraEx mCameraEx;
    private static final int MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONE = 0x00000016;
    private static final int MTK_CAMERA_MSG_EXT_DATA_RAW16 = 0x00000013;
    private static final String KEY_DISP_ROT_SUPPORTED = "disp-rot-supported";
    private static final String KEY_PANEL_SIZE = "panel-size";
    private final Object mDeviceStateSync = new Object();
    private boolean mFaceDetectionRunning = false;

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

    CameraHandler(@Nonnull Context context,
                  @Nonnull String cameraId,
                  @Nonnull Looper looper,
                  @Nonnull Camera camera,
                  @Nonnull IDeviceInfoListener infoListener) {
        super(looper);
        mCamera = camera;
        mContext = context;
        mCameraId = cameraId;
        mCameraEx = new CameraEx();
        mIDeviceInfoListener = infoListener;
        mTag = new Tag("API1-Handler-" + cameraId);
    }

    /**
     * Get original parameters no need a new instance.
     * @return the original parameters.
     */
    public Parameters getOriginalParameters() {
        return mOriginalParameters;
    }

    /**
     * Notify when device occur error.
     * @param error error type.
     */
    public void notifyDeviceError(int error) {
        synchronized (mDeviceStateSync) {
            mCamera = null;
        }
    }

    @Override
    public void handleMessage(Message msg) {
        super.handleMessage(msg);
        int operation = msg.what;

        mMsgStartTime = SystemClock.uptimeMillis();
        printStartMsg(
                mTag.toString(),
                CameraActions.stringify(operation),
                (mMsgStartTime - msg.getWhen()));
        synchronized (mDeviceStateSync) {
            if (mCamera == null) {
                printStopMsg(mTag.toString(), "camera is closed ,ignore this :" + msg.what, 0);
                return;
            }
            try {
                doHandleMessage(msg);
            } catch (RuntimeException e) {
                if (msg.what != CameraActions.CLOSE) {
                    mLockMap.lock();
                    try {
                        mCamera.release();
                        mCamera = null;
                        mCameraEx = null;
                        mVendorCallbackMap.clear();
                    } catch (Exception ex) {
                        LogHelper.e(mTag, "Fail to release the camera.");
                    } finally {
                        mLockMap.unlock();
                    }
                }
                mIDeviceInfoListener.onError();
            }
        }

        mMsgStopTime = SystemClock.uptimeMillis();
        printStopMsg(mTag.toString(),
                CameraActions.stringify(operation),
                (mMsgStopTime - mMsgStartTime));
    }

    @Override
    protected void doHandleMessage(Message msg) {
        switch (msg.what) {
            case CameraActions.CLOSE:
                mLockMap.lock();
                try {
                    mCamera.release();
                    mCamera = null;
                    mCameraEx = null;
                    mFaceDetectionRunning = false;
                    mIDeviceInfoListener.onClosed();
                    mVendorCallbackMap.clear();
                } finally {
                    mLockMap.unlock();
                }
                break;

            case CameraActions.RECONNECT:
                try {
                    mCamera.reconnect();
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
                break;

            case CameraActions.UNLOCK:
                mCamera.unlock();
                break;

            case CameraActions.LOCK:
                mCamera.lock();
                break;

            case CameraActions.INIT_PARAMETERS:
                try {
                    mOriginalParameters = mCamera.getParameters();
                    CameraApiHelper.getDeviceSpec(
                            mContext).getDeviceDescriptionMap()
                            .get(mCameraId)
                            .setParameters(mOriginalParameters);
                    setPanelSizeToNative(mContext);
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
                break;

            case CameraActions.SET_PREVIEW_TEXTURE:
                try {
                    mCamera.setPreviewTexture((SurfaceTexture) msg.obj);
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
                break;

            case CameraActions.START_PREVIEW:
                mCamera.startPreview();
                break;

            case CameraActions.STOP_PREVIEW:
                mCamera.stopPreview();
                mFaceDetectionRunning = false;
                break;

            case CameraActions.SET_PREVIEW_CALLBACK_WITH_BUFFER:
                mCamera.setPreviewCallbackWithBuffer((PreviewCallback) msg.obj);
                break;

            case CameraActions.ADD_CALLBACK_BUFFER:
                mCamera.addCallbackBuffer((byte[]) msg.obj);
                break;

            case CameraActions.SET_PREVIEW_DISPLAY:
                try {
                    mCamera.setPreviewDisplay((SurfaceHolder) msg.obj);
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
                break;

            case CameraActions.SET_PREVIEW_CALLBACK:
                mCamera.setPreviewCallback((PreviewCallback) msg.obj);
                break;

            case CameraActions.SET_ONE_SHOT_PREVIEW_CALLBACK:
                mCamera.setOneShotPreviewCallback((PreviewCallback) msg.obj);
                break;

            case CameraActions.SET_PARAMETERS:
                mCamera.setParameters((Parameters) msg.obj);
                break;

            case CameraActions.GET_PARAMETERS:
                Parameters[] params = (Parameters[]) msg.obj;
                params[0] = mCamera.getParameters();
                break;

            case CameraActions.GET_ORIGINAL_PARAMETERS:
                Parameters[] originalParameters = (Parameters[]) msg.obj;
                if (msg.arg1 == 1) {
                    originalParameters[0] = mCamera.getParameters();
                    originalParameters[0].unflatten(mOriginalParameters.flatten());
                    return;
                }
                originalParameters[0] = mOriginalParameters;
                break;

            case CameraActions.AUTO_FOCUS:
                mCamera.autoFocus((AutoFocusCallback) msg.obj);
                break;

            case CameraActions.CANCEL_AUTO_FOCUS:
                mCamera.cancelAutoFocus();
                break;

            case CameraActions.SET_AUTO_FOCUS_MOVE_CALLBACK:
                mCamera.setAutoFocusMoveCallback((AutoFocusMoveCallback) msg.obj);
                break;

            case CameraActions.SET_ZOOM_CHANGE_LISTENER:
                mCamera.setZoomChangeListener((OnZoomChangeListener) msg.obj);
                break;

            case CameraActions.START_SMOOTH_ZOOM:
                mCamera.startSmoothZoom((Integer) msg.obj);
                break;

            case CameraActions.STOP_SMOOTH_ZOOM:
                mCamera.stopSmoothZoom();
                break;

            case CameraActions.SET_FACE_DETECTION_LISTENER:
                mCamera.setFaceDetectionListener((FaceDetectionListener) msg.obj);
                break;

            case CameraActions.START_FACE_DETECTION:
                if (mFaceDetectionRunning) {
                    LogHelper.w(mTag, "Face detection is already running");
                    return;
                }
                mCamera.startFaceDetection();
                mFaceDetectionRunning = true;
                break;

            case CameraActions.STOP_FACE_DETECTION:
                mCamera.stopFaceDetection();
                mFaceDetectionRunning = false;
                break;

            case CameraActions.ENABLE_SHUTTER_SOUND:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
                    boolean[] isEnabled = (boolean[]) msg.obj;
                    isEnabled[0] = mCamera.enableShutterSound(isEnabled[0]);
                }
                break;

            case CameraActions.SET_DISPLAY_ORIENTATION:
                mCamera.setDisplayOrientation((Integer) msg.obj);
                break;

            case CameraActions.TAKE_PICTURE:
                takePicture((CameraProxy.CaptureCallbackGroup) msg.obj);
                mFaceDetectionRunning = false;
                break;

            case CameraActions.SEND_COMMAND:
                sendCommand((CameraProxy.CommandInfo) msg.obj);
                break;

            case CameraActions.SET_VENDOR_DATA_CALLBACK:
                setVendorDataCallback((CameraProxy.VendCallbackInfo) msg.obj);
                break;
            default:
                throw new RuntimeException("Unimplemented msg:" + msg.what);
        }
    }

    private void takePicture(CameraProxy.CaptureCallbackGroup callbackGroup) {
        mCamera.takePicture(callbackGroup.mShutterCallback, callbackGroup.mRawCallback,
                callbackGroup.mPostViewCallback, callbackGroup.mJpegCallback);
    }

    private void sendCommand(CameraProxy.CommandInfo commandInfo) {
        mCameraEx.sendCommand(mCamera, commandInfo.mCommand,
                commandInfo.mArg1, commandInfo.mArg2);
    }

    private void setVendorDataCallback(CameraProxy.VendCallbackInfo vendCallbackInfo) {
        mLockMap.lock();
        try {
            if (vendCallbackInfo.mArg1 == null) {
                if (mVendorCallbackMap.containsKey(vendCallbackInfo.mMsgId)) {
                    mVendorCallbackMap.remove(vendCallbackInfo.mMsgId);
                }
            } else {
                mVendorCallbackMap.put(vendCallbackInfo.mMsgId, vendCallbackInfo.mArg1);
            }
        } finally {
            mLockMap.unlock();
        }
        mCameraEx.setVendorDataCallback(mCamera,
                vendCallbackInfo.mMsgId, mVendorExDataCallback);
    }

    private CameraEx.VendorDataCallback mVendorExDataCallback = new CameraEx.VendorDataCallback() {

        @Override
        public void onDataTaken(Message message) {
            mLockMap.lock();
            try {
                if (mVendorCallbackMap.containsKey(message.arg1)) {
                    mVendorCallbackMap.get(message.arg1).onDataTaken(message);
                }
            } finally {
                mLockMap.unlock();
            }
        }

        @Override
        public void onDataCallback(int msgId, byte[] bytes, int i1, int i2) {
            mLockMap.lock();
            try {
                if (mVendorCallbackMap.containsKey(msgId)) {
                    mVendorCallbackMap.get(msgId).onDataCallback(msgId, bytes, i1, i2);
                }
                if (msgId == MTK_CAMERA_MSG_EXT_DATA_RAW16) {
                    if (mVendorCallbackMap.containsKey(MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONE)) {
                        mVendorCallbackMap.get(MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONE)
                                .onDataCallback(msgId, bytes, i1, i2);
                    }
                }
            } finally {
                mLockMap.unlock();
            }
        }
    };

    private void setPanelSizeToNative(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            String displayRotSupported = mOriginalParameters.get(KEY_DISP_ROT_SUPPORTED);
            if (displayRotSupported == null || "false".equals(displayRotSupported)) {
                LogHelper.i(mTag, "isDisplayRotateSupported: false.");
                return;
            }
            try {
                mOriginalParameters.set(KEY_PANEL_SIZE, getPanelSizeStr(context));
                mCamera.setParameters(mOriginalParameters);
                mOriginalParameters = mCamera.getParameters();
            } catch (RuntimeException e) {
                e.printStackTrace();
            }

        }
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
    private static String getPanelSizeStr(Context context) {
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        Display display = wm.getDefaultDisplay();
        Point point = new Point();
        display.getRealSize(point);
        int panelHeight = Math.min(point.x, point.y);
        int panelWidth = Math.max(point.x, point.y);
        return "" + panelWidth + "x" + panelHeight;
    }
}