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

package com.mediatek.camera.common.device;

import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;

import java.util.concurrent.locks.ReentrantLock;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * A class used to manage Camera Devices.
 */
public abstract class CameraDeviceManager {
    /**
     * Open a connection to a camera with the given ID. If the specified camera is opened,
     * it will return with the specified CameraStateCallback.
     *
     * @param cameraId
     *            The unique identifier of the camera device to open
     * @param callback
     *            The callback which is invoked once the camera is opened
     * @param handler
     *            The handler on which the callback should be invoked, or {@code null} to use the
     *            default thread's {@link android.os.Looper looper}. the default thread is not main
     *            thread, so you can't do UI operations when callback invoked. if the callback is
     *            invoked by default thread, please don't blocking the thread, such as long time
     *            operations. if you have blocking this , you can't receive any info from
     *            framework.
     * @throws CameraOpenException
     *             if the application does not have permission to access the camera will throw
     *             security exception,or current have occur some exception when open camera. Such as
     *             camera disconnected and other unknown exception. the detail type see
     *             CameraOpenException.ExceptionType.
     */
    public abstract void openCamera(@Nonnull String cameraId,
                    @Nonnull final CameraStateCallback callback, @Nullable Handler handler)
                    throws CameraOpenException;

    /**
     * Open a connection to a camera with the given ID.
     * <p>
     * Be care do not call this function in CameraStateCallback handler thread.
     * </p>
     *
     * @param cameraId
     *            The unique identifier of the camera device to open
     * @param callback
     *            The callback which is invoked once the camera is opened
     * @param handler
     *            The handler on which the callback should be invoked, or {@code null} to use the
     *            default thread's {@link android.os.Looper looper}. the default thread is not main
     *            thread, so you can't do UI operations when callback invoked. if the callback is
     *            invoked by default thread, please don't blocking the thread, such as long time
     *            operations. if you have blocking this , you can't receive any info from
     *            framework.
     * @throws CameraOpenException
     *             if the application does not have permission to access the camera will throw
     *             security exception,or current have occur some exception when open camera. Such as
     *             camera disconnected and other unknown exception. the detail type see
     *             CameraOpenException.ExceptionType.
     */
    public abstract void openCameraSync(@Nonnull String cameraId,
                    @Nonnull final CameraStateCallback callback, @Nullable Handler handler)
                    throws CameraOpenException;

    /**
     * close camera.
     * @param cameraId .
     */
    public abstract void close(@Nonnull String cameraId);

    /**
     * close camera for sync.
     * @param cameraId .
     */
    public abstract void closeSync(@Nonnull String cameraId);
    /**
     * Recycles the resources used by this instance. CameraDeviceManager will be in
     * an unusable state after calling this.
     */
    public abstract void recycle();

    /**
     * <p>An agent used to handle one device related resource,include:</p>
     * <li>crate and destroy request and respond thread.</li>
     * <li>create request and respond handler.</li>
     * <li>Dispatch device state in specified callback & handler.</li>
     * <li>Retry open camera when camera in use.</li>
     */
    protected abstract class CameraProxyCreator {
        private static final int SDK_VERSION_FOR_LOOPER_QUIT = 18;
        protected static final int OPEN_RETRY_COUNT = 2;
        // wait some time, and try another time Camera device may be using by VT or atv.
        protected static final int RETRY_OPEN_SLEEP_TIME_MS = 1000;
        protected CameraApi mApiVersion;
        protected String mCameraId;

        protected HandlerThread mRequestThread;
        protected HandlerThread mRespondThread;

        protected ReentrantLock mThreadLock = new ReentrantLock(true);

        protected int mRetryCount = 0;

        protected CameraProxyCreator(CameraApi api, String cameraId) {
            mApiVersion = api;
            mCameraId = cameraId;
            createHandlerThreads();
        }

        protected void createHandlerThreads() {
            mThreadLock.lock();
            mRequestThread = new HandlerThread(mApiVersion + "-Request-" + mCameraId);
            mRespondThread = new HandlerThread(mApiVersion + "-Response-" + mCameraId);
            mRequestThread.start();
            mRespondThread.start();
            mThreadLock.unlock();
        }

        public void destroyHandlerThreads() {
            mThreadLock.lock();
            if (mRequestThread.isAlive()) {
                if (Build.VERSION.SDK_INT >= SDK_VERSION_FOR_LOOPER_QUIT) {
                    mRequestThread.getLooper().quitSafely();
                } else {
                    mRequestThread.getLooper().quit();
                }
            }
            if (mRespondThread.isAlive()) {
                if (Build.VERSION.SDK_INT >= SDK_VERSION_FOR_LOOPER_QUIT) {
                    mRespondThread.getLooper().quitSafely();
                } else {
                    mRespondThread.getLooper().quit();
                }
            }
            mThreadLock.unlock();
        }
    }
}