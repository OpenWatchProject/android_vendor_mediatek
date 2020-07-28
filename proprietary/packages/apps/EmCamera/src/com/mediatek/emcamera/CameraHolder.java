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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.emcamera;

import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import java.lang.reflect.Method;
import java.io.IOException;

/**
 * The class is used to hold an {@code android.hardware.Camera} instance.
 *
 * <p>
 * The {@code open()} and {@code release()} calls are similar to the ones in {@code android.hardware.Camera}. The difference
 * is if {@code keep()} is called before {@code release()}, CameraHolder will try to hold the {@code android.hardware.Camera}
 * instance for a while, so if {@code open()} is called soon after, we can avoid the cost of {@code open()} in
 * {@code android.hardware.Camera}.
 *
 * <p>
 * This is used in switching between {@code Camera} and {@code VideoCamera} activities.
 */
public class CameraHolder {
    private static final String TAG = "CameraHolder";
    private android.hardware.Camera mCameraDevice;
    private int mCameraId = -1; // current camera id
    private CameraInfo[] mInfo;

    // We store the camera parameters when we actually open the device,
    // so we can restore them in the subsequent open() requests by the user.
    // This prevents the parameters set by the Camera activity used by
    // the VideoCamera activity inadvertently.
    private Parameters mParameters;

    // Use a singleton.
    private static CameraHolder sHolder;

    public static synchronized CameraHolder instance() {
        if (sHolder == null) {
            sHolder = new CameraHolder();
        }
        return sHolder;
    }

    private CameraHolder() {
        int camerasNum = android.hardware.Camera.getNumberOfCameras();
        mInfo = new CameraInfo[camerasNum];
        for (int i = 0; i < camerasNum; i++) {
            mInfo[i] = new CameraInfo();
            android.hardware.Camera.getCameraInfo(i, mInfo[i]);
        }
    }

    public CameraInfo[] getCameraInfo() {
        return mInfo;
    }

    public synchronized android.hardware.Camera open(int cameraId) throws CameraHardwareException {
        if (mCameraDevice != null && mCameraId != cameraId) {
            mCameraDevice.release();
            mCameraDevice = null;
            mCameraId = -1;
        }
        if (mCameraDevice == null) {
            Elog.v(TAG, "open camera " + cameraId);
            mCameraDevice = android.hardware.Camera.open(cameraId);
            mCameraId = cameraId;
            if (mCameraDevice == null) {
                Elog.e(TAG, "fail to connect Camera");
                throw new CameraHardwareException("Fail to connect camera");
            }
            mParameters = mCameraDevice.getParameters();
        } else {
            try {
                mCameraDevice.reconnect();
            } catch (IOException e) {
                Elog.e(TAG, "reconnect failed.");
                throw new CameraHardwareException(e);
            }
            mCameraDevice.setParameters(mParameters);
        }
        return mCameraDevice;
    }

    public synchronized void release() {
        mCameraDevice.stopPreview();
        releaseCamera();
    }

    private synchronized void releaseCamera() {
        mCameraDevice.release();
        mCameraDevice = null;
        // We must set this to null because it has a reference to Camera.
        // Camera has references to the listeners.
        mParameters = null;
        mCameraId = -1;
    }

}
