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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera.addition.remotecamera;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningServiceInfo;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;


import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ICameraAddition.AdditionActionType;
import com.mediatek.camera.ICameraAddition.Listener;
import com.mediatek.camera.ICameraMode.ActionType;
import com.mediatek.camera.addition.CameraAddition;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.addition.remotecamera.service.MtkCameraService;
import com.mediatek.camera.addition.remotecamera.service.IMtkCameraService;
import com.mediatek.camera.util.Log;

import java.util.List;

public class RemoteCameraAddition extends CameraAddition {
    private static final String TAG = "RemoteCameraAddition";

    private boolean mIsMtkCameraApServiceLaunched = false;
    private boolean mHasNotifyParameterReady = false;

    private int mOrientation;
    private static final int MSG_ON_PREVIEW_STARTED = 0;
    private Listener mModeListener;

    private IMtkCameraService.Stub mCameraService;
    private ICameraDeviceManager mICameraDeviceManager;
    private MainHandler mMainHandler;
    private boolean mOpened = false;

    public RemoteCameraAddition(ICameraContext cameraContext) {
        super(cameraContext);
        mIsMtkCameraApServiceLaunched = isMtkCameraApServiceLaunched(mActivity);
        mICameraDeviceManager = cameraContext.getCameraDeviceManager();
        mMainHandler = new MainHandler(mActivity.getMainLooper());
    }

    @Override
    public void open() {
        if (mIsMtkCameraApServiceLaunched) {
            int cameraId = mICameraDeviceManager.getCurrentCameraId();
            ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(cameraId);
            if (cameraDevice == null) {
                Log.w(TAG, "cameraDevice is null, return.");
                return;
            }
            Parameters parameters = cameraDevice.getParameters();
            parameters.setPreviewFormat(ImageFormat.NV21);
            if (mCameraService == null) {
                bindCameraService();
            }
            mOpened = true;
        }
    }

    @Override
    public boolean isOpen() {
        return mOpened;
    }

    @Override
    public boolean isSupport() {
        Log.i(TAG, "[isSupport], mIsMtkCameraApServiceLaunched:" + mIsMtkCameraApServiceLaunched);
        return mIsMtkCameraApServiceLaunched;
    }

    @Override
    public void pause() {
        clearPreviewCallback();
        if (mCameraService != null) {
            Message msg = Message.obtain();
            msg.what = MtkCameraService.MSG_SERVER_EXIT;
            sendMessageToService(msg);
        }
    }

    @Override
    public void destory() {
        Log.d(TAG, "[destory]...");
        if (mCameraService != null) {
            unBindCameraService();
        }

    }

    @Override
    public void close() {
        clearPreviewCallback();
        mOpened = false;
    }

    @Override
    public boolean execute(ActionType type, Object... arg) {
        switch (type) {
        case ACTION_ORITATION_CHANGED:
            mOrientation = (Integer) arg[0];
            onOrientationChanged(mOrientation);
            break;
        default:
            break;
        }
        return false;
    }

    @Override
    public boolean execute(AdditionActionType type, Object... arg) {
        switch (type) {
        case ACTION_TAKEN_PICTURE:
            if (mIsMtkCameraApServiceLaunched) {
                takePicture();
                return true;
            }
            break;
        case ACTION_ON_START_PREVIEW:
            mMainHandler.sendEmptyMessage(MSG_ON_PREVIEW_STARTED);
            break;
        default:
            break;
        }
        return false;
    }


    @Override
    public void setListener(Listener listener) {
        mModeListener = listener;
    }

    private void bindCameraService() {
        mHasNotifyParameterReady = false;
        Intent intent = new Intent(mActivity, MtkCameraService.class);
        mActivity.bindService(intent, mCameraConnection, Context.BIND_AUTO_CREATE);
    }

    private void unBindCameraService() {
        mActivity.unbindService(mCameraConnection);
    }

    private ServiceConnection mCameraConnection = new ServiceConnection() {

        @Override
        public void onServiceDisconnected(ComponentName name) {
            // TODO Auto-generated method stub
            Log.d(TAG, "CameraConnection, onServiceDisconnected()");
            mCameraService = null;
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            // TODO Auto-generated method stub
            Log.d(TAG, "CameraConnection, onServiceConnected()");
            mCameraService = (IMtkCameraService.Stub) service;
        }
    };

    private void sendMessageToService(Message msg) {
        if (mCameraService != null) {
            try {
                if (!mHasNotifyParameterReady) {
                    Message parameterMessage = Message.obtain();
                    int cameraId = mICameraDeviceManager.getCurrentCameraId();
                    ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(cameraId);
                    parameterMessage.what = MtkCameraService.MSG_PARAMETERS_READY;
                    parameterMessage.obj = cameraDevice.getParameters();
                    mCameraService.sendMessage(parameterMessage);
                    mHasNotifyParameterReady = true;
                }
                Log.d(TAG, "msg:" + msg.what);
                mCameraService.sendMessage(msg);
            } catch (Exception e) {
                // TODO: handle exception
                Log.e(TAG, "sendMessageToService exception");
            }
        }
    }

    private boolean isMtkCameraApServiceLaunched(Context context) {
        ActivityManager activityManager = (ActivityManager) context
                .getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningServiceInfo> serviceList = activityManager
                .getRunningServices(Integer.MAX_VALUE);
        if (!(serviceList.size() > 0)) {
            return false;
        }
        for (int i = 0; i < serviceList.size(); i++) {
            RunningServiceInfo serviceInfo = serviceList.get(i);
            ComponentName servcieName = serviceInfo.service;
            if ("com.mediatek.camera.addition.remotecamera.service.MtkCameraService"
                    .equals(servcieName.getClassName())) {
                Log.d(TAG, "isMtkCameraApServiceLaunched true");
                return true;
            }
        }
        return false;
    }

    private int getBufferSize() {
        int cameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(cameraId);
        Size size = cameraDevice.getParameters().getPreviewSize();
        int imageFormat = cameraDevice.getParameters().getPreviewFormat();
        return size.width * size.height * ImageFormat.getBitsPerPixel(imageFormat) / 8;
    }

    private void setPreviewCallback() {
        int cameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(cameraId);
        if (cameraDevice == null) {
            Log.w(TAG, "[setPreviewCallback], cameraDevice is null");
            return;
        }
        int bufferSize = getBufferSize();
        Log.d(TAG, "[setPreviewCallback], bufferSize:" + bufferSize);
        for (int i = 0; i < 3; i++) {
            byte[] buffer = new byte[bufferSize];
            cameraDevice.addCallbackBuffer(buffer);
        }
        cameraDevice.setPreviewCallbackWithBuffer(mPreviewCallback);
    }

    private void clearPreviewCallback() {
        Log.d(TAG, "[clearPreviewCallback]...");
        int cameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(cameraId);
        if (cameraDevice == null) {
            Log.w(TAG, "[clearPreviewCallback], cameraDevice is null");
            return;
        }
        cameraDevice.setPreviewCallbackWithBuffer(null);
    }

    private void onOrientationChanged(int orientation) {
        Log.d(TAG, "[onOrientationChanged], orientation:" + orientation);
        Message msg = new Message();
        msg.what = MtkCameraService.MSG_ORIENTATION_CHANGED;
        msg.arg1 = orientation;
        sendMessageToService(msg);
    }

    private void takePicture() {
        int cameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(cameraId);
        cameraDevice.takePicture(null, null, null, mJpegPictureCallback);
        mICameraAppUi.setViewState(ViewState.VIEW_STATE_CAPTURE);
    }

    private final PictureCallback mJpegPictureCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] jpegData, Camera camera) {
            Log.d(TAG, "[mJpegPictureCallback]onPictureTaken, jpegData:" + jpegData);
            if (mModeListener != null) {
                mModeListener.restartPreview(true);
            }
            if (jpegData == null) {
                return;
            }
            long dateTaken = System.currentTimeMillis();
            mIFileSaver.savePhotoFile(jpegData, null, dateTaken, mIModuleCtrl.getLocation(),
                    0, null);
            Message msg = Message.obtain();
            msg.what = MtkCameraService.MSG_CAPTURE_DATA;
            msg.obj = jpegData;
            sendMessageToService(msg);
        }
    };

    private PreviewCallback mPreviewCallback = new PreviewCallback() {

        @Override
        public void onPreviewFrame(byte[] data, android.hardware.Camera camera) {
            int cameraId = mICameraDeviceManager.getCurrentCameraId();
            ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(cameraId);

            Message msg = Message.obtain();
            msg.what = MtkCameraService.MSG_PREVIEW_FRAME_DATA;
            msg.obj = data;
            msg.arg1 = getDisplayOrientation(-mOrientation, cameraId);;
            sendMessageToService(msg);

            if (cameraDevice != null) {
                cameraDevice.addCallbackBuffer(data);
            }
        }
    };

    private int getDisplayOrientation(int degrees, int cameraId) {
        // See android.hardware.Camera.setDisplayOrientation for
        // documentation.
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;  // compensate the mirror
        } else {  // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }
        return result;
    }

    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_ON_PREVIEW_STARTED:
                if (mIsMtkCameraApServiceLaunched) {
                    setPreviewCallback();
                }
                break;
            default:
                break;
            }
        }
    }
}
