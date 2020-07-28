/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2009 The Android Open Source Project
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

import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import static com.android.camera.Util.assertError;
import com.android.camera.CameraManager.CameraProxy;

import com.mediatek.camera.util.Log;
import java.io.IOException;

/**
 * The class is used to hold an {@code android.hardware.Camera} instance.
 *
 * <p>
 * The {@code open()} and {@code release()} calls are similar to the ones in
 * {@code android.hardware.Camera}. The difference is if {@code keep()} is
 * called before {@code release()}, CameraHolder will try to hold the
 * {@code android.hardware.Camera} instance for a while, so if {@code open()} is
 * called soon after, we can avoid the cost of {@code open()} in
 * {@code android.hardware.Camera}.
 *
 * <p>
 * This is used in switching between {@code Camera} and {@code VideoCamera}
 * activities.
 */
public class CameraHolder {
    private static final String TAG = "CameraHolder";
    public static final int UNKONW_ID = -1;
    private final int mNumberOfCameras;
    private int mBackCameraId = UNKONW_ID;
    private int mFrontCameraId = UNKONW_ID;
    private final CameraInfo[] mInfo;

    private static CameraProxyWrapper sBackCamProxyWrapper;
    private static CameraProxyWrapper sFrontCamProxyWrapper;
    private static CameraManager sBackCameraManager;
    private static CameraManager sFrontCameraManager;

    // Use a singleton.
    private static CameraHolder sHolder;

    public static synchronized CameraHolder instance() {
        if (sHolder == null) {
            sHolder = new CameraHolder();
        }
        return sHolder;
    }

    private CameraHolder() {
        mNumberOfCameras = Camera.getNumberOfCameras();
        mInfo = new CameraInfo[mNumberOfCameras];
        Log.i(TAG, "mNumberOfCameras = " + mNumberOfCameras);
        for (int i = 0; i < mNumberOfCameras; i++) {
            mInfo[i] = new CameraInfo();
            Camera.getCameraInfo(i, mInfo[i]);
        }
        // get the first (smallest) back and first front camera id
        for (int i = 0; i < mNumberOfCameras; i++) {
            if (mBackCameraId == UNKONW_ID && mInfo[i].facing == CameraInfo.CAMERA_FACING_BACK) {
                mBackCameraId = i;
                sBackCameraManager = new CameraManager("BackCam");
                sBackCamProxyWrapper = new CameraProxyWrapper(mBackCameraId, sBackCameraManager);
            } else if (mFrontCameraId == UNKONW_ID
                    && mInfo[i].facing == CameraInfo.CAMERA_FACING_FRONT) {
                mFrontCameraId = i;
                sFrontCameraManager = new CameraManager("FrontCam");
                sFrontCamProxyWrapper = new CameraProxyWrapper(mFrontCameraId, sFrontCameraManager);
            }
        }
    }

    public int getNumberOfCameras() {
        return mNumberOfCameras;
    }

    public CameraInfo[] getCameraInfo() {
        Log.d(TAG, "getCameraInfo,size = " + mInfo.length);
        return mInfo;
    }

    public CameraProxy open(int cameraId) throws CameraHardwareException {
        Log.i(TAG, "CameraHolder open cameraId = " + cameraId);
        assertError(cameraId != UNKONW_ID);
        return getCameraProxyWrapper(cameraId).open();
    }

    public CameraProxy tryOpen(int cameraId) {
        return getCameraProxyWrapper(cameraId).tryOpen();
    }

    /**
     * release Camera.
     * @param isAsync if true, call native method asynchronously
     *  when exit camera.
     */
    public void release(boolean isAsync) {
        if (getCameraProxyWrapper(mBackCameraId).getCameraProxy() != null) {
            Log.i(TAG, "CameraHolder release back camera");
            getCameraProxyWrapper(mBackCameraId).release(isAsync);
        }
        if (getCameraProxyWrapper(mFrontCameraId).getCameraProxy() != null) {
            Log.i(TAG, "CameraHolder release front camera");
            getCameraProxyWrapper(mFrontCameraId).release(isAsync);
        }
    }

    public synchronized void keep(int time, int cameraId) {
        getCameraProxyWrapper(cameraId).keep(time);
    }

    public int getBackCameraId() {
        return mBackCameraId;
    }

    public int getFrontCameraId() {
        return mFrontCameraId;
    }

    public Parameters getOriginalParameters(int cameraId) {
        if (cameraId == UNKONW_ID)
            return null;
        return getCameraProxyWrapper(cameraId).getOriginalParameters();
    }

    public CameraProxy getCameraProxy(int cameraId) {
        if (cameraId == UNKONW_ID)
            return null;
        return getCameraProxyWrapper(cameraId).getCameraProxy();
    }

    private CameraProxyWrapper getCameraProxyWrapper(int cameraId) {
        if (cameraId == mBackCameraId) {
            if (sBackCamProxyWrapper == null) {
                sBackCameraManager = new CameraManager("BackCam");
                sBackCamProxyWrapper = new CameraProxyWrapper(cameraId, sBackCameraManager);
            }
            return sBackCamProxyWrapper;
        } else {
            if (sFrontCamProxyWrapper == null) {
                sFrontCameraManager = new CameraManager("FrontCam");
                sFrontCamProxyWrapper = new CameraProxyWrapper(cameraId, sFrontCameraManager);
            }
            return sFrontCamProxyWrapper;
        }
    }

    private class CameraProxyWrapper {
        private int mCameraId = CameraHolder.UNKONW_ID; // current camera id
        private CameraProxy mCameraProxy;
        private CameraManager mCameraManager;
        // We store the camera parameters when we actually open the device,
        // so we can restore them in the subsequent open() requests by the user.
        // This prevents the parameters set by the Camera activity used by
        // the VideoCamera activity inadvertently.
        private Parameters mParameters;
        private boolean mCameraOpened; // true if camera is opened
        private long mKeepBeforeTime; // Keep the Camera before this time.
        private final Handler mHandler;

        private static final int RELEASE_CAMERA = 1;

        private class MyHandler extends Handler {
            MyHandler(Looper looper) {
                super(looper);
            }

            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case RELEASE_CAMERA:
                    synchronized (CameraHolder.this) {
                        // In 'CameraHolder.open', the 'RELEASE_CAMERA' message
                        // will be removed if it is found in the queue. However,
                        // there is a chance that this message has been handled
                        // before being removed. So, we need to add a check
                        // here:
                        if (!mCameraOpened) {
                            release(false);
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }

        private CameraProxyWrapper(int cameraId, CameraManager manager) {
            Log.d(TAG, "[CameraProxyWrapper]constructor, cameraId = " + cameraId);
            mCameraId = cameraId;
            mCameraManager = manager;
            HandlerThread ht = new HandlerThread(cameraId + "'s CameraHolder ");
            ht.start();
            mHandler = new MyHandler(ht.getLooper());
        }

        public synchronized CameraProxy open() throws CameraHardwareException {
            Log.d(TAG, "CameraProxyWrapper open mCameraOpened = " + mCameraOpened + " mCameraId = "
                    + mCameraId);
            assertError(!mCameraOpened);
            if (mCameraProxy == null) {
                try {
                    Log.i(TAG, "open camera " + mCameraId);
                    mCameraProxy = mCameraManager.cameraOpen(mCameraId);
                } catch (RuntimeException e) {
                    Log.e(TAG, "fail to connect Camera", e);
                    throw new CameraHardwareException(e);
                }
                mParameters = mCameraProxy.getParameters();
            } else {
                try {
                    mCameraProxy.reconnect();
                } catch (IOException e) {
                    Log.e(TAG, "reconnect failed.");
                    throw new CameraHardwareException(e);
                }
                mCameraProxy.setParameters(mParameters);
            }
            mCameraOpened = true;
            mHandler.removeMessages(RELEASE_CAMERA);
            mKeepBeforeTime = 0;
            Log.d(TAG, "open camera " + mCameraId + " end" + " mCameraProxy = " + mCameraProxy);
            return mCameraProxy;
        }

        /**
         * Tries to open the hardware camera. If the camera is being used or
         * unavailable then return {@code null}.
         */
        public CameraProxy tryOpen() {
            try {
                return !mCameraOpened ? open() : null;
            } catch (CameraHardwareException e) {
                // QA will always consider JE as issue, so..
                // In eng build, we throw the exception so that test tool
                // can detect it and report it
                // if ("eng".equals(Build.TYPE)) {
                // throw new RuntimeException(e);
                // }
                return null;
            }
        }

        public CameraProxy getCameraProxy() {
            return mCameraProxy;
        }

        /**
         * release Camera.
         * @param isAsync call native method asynchronously for exit performance.
         */
        public synchronized void release(boolean isAsync) {
            Log.i(TAG, "release");
            assertError(mCameraProxy != null);

            long now = System.currentTimeMillis();
            if (now < mKeepBeforeTime) {
                if (mCameraOpened) {
                    mCameraOpened = false;
                    mCameraProxy.stopPreview();
                }
                mHandler.sendEmptyMessageDelayed(RELEASE_CAMERA, mKeepBeforeTime - now);
                return;
            }
            mCameraOpened = false;
            if (isAsync) {
                mCameraProxy.releaseAsync();
            } else {
               mCameraProxy.release();
            }
            mCameraProxy = null;
            // We must set this to null because it has a reference to Camera.
            // Camera has references to the listeners.
            mParameters = null;
        }

        public synchronized void keep(int time) {
            // We allow mCameraOpened in either state for the convenience of the
            // calling activity. The activity may not have a chance to call
            // open()
            // before the user switches to another activity.
            mKeepBeforeTime = System.currentTimeMillis() + time;
        }

        public Parameters getOriginalParameters() {
            if (mParameters == null) {
                throw new IllegalArgumentException();
            }
            return mParameters;
        }
    }

}
