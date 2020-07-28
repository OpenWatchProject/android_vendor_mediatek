/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2018. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.bgservice;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Process;
import android.os.RemoteException;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.portability.BGService;

import java.util.ArrayList;
import java.util.List;
import java.util.NoSuchElementException;

/**
 * Camera app background service for jpeg missing,
 * save buffers in background.
 */
public class BGServiceKeeper {
    private static final Tag TAG = new Tag(BGServiceKeeper.class.getSimpleName());
    private static final int REQUEST_SUCCESS = 1;
    private static final int REQUEST_ERROR = 2;
    private final Handler mBGCaptureHandler;
    private final HandlerThread mHandlerThread;
    private BGService mBGHIDLService = null;
    private List<CaptureSurface> mCaptureSurfaceList = new ArrayList<>();

    public BGServiceKeeper(ICameraContext context) {
        mHandlerThread = new HandlerThread("BGServiceHandlerThread",
                Process.THREAD_PRIORITY_FOREGROUND);
        mHandlerThread.start();
        Looper looper = mHandlerThread.getLooper();
        mBGCaptureHandler = new Handler(looper);
        getBGHidleService();
    }

    public void setBGCaptureSurface(CaptureSurface captureSurface) {
        if (!mCaptureSurfaceList.contains(captureSurface)) {
            LogHelper.d(TAG, "add CaptureSurface:" + captureSurface);
            mCaptureSurfaceList.add(captureSurface);
        }
    }

    public Handler getBGCaptureHandler() {
        return mBGCaptureHandler;
    }

    public BGService getBGHidleService() {
        LogHelper.d(TAG, "getBGHidleService ...");
        if (mBGHIDLService == null) {
            try {
                mBGHIDLService = BGService.getService("internal/0", true);
                mBGHIDLService.linkToDeath(mBGHIDLServiceDeathRecipient, 0);
                mBGHIDLService.setEventCallback(0, mBGEventCallback);
                LogHelper.i(TAG, "getBGHidleService init..." + mBGHIDLService);
            } catch (NoSuchElementException e) {
                e.printStackTrace();
                LogHelper.e(TAG, "BGHIDLConnection NoSuchElementException ...");
            }
        }
        return mBGHIDLService;
    }

    private BGHIDLServiceDeathRecipient mBGHIDLServiceDeathRecipient
            = new BGHIDLServiceDeathRecipient();

    class BGHIDLServiceDeathRecipient implements BGService.HwBinderDeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            mBGHIDLService = null;
            boolean ret = false;
            while (!ret) {
                sleep(200);
                if (getBGHidleService() != null) {
                    LogHelper.d(TAG, "retry BGHIDLService success.");
                    ret = true;
                }
            }
        }

    }
    private BGService.IEventCallbackStub mBGEventCallback =  new BGService.IEventCallbackStub() {

        @Override
        public boolean onCompleted(int imgReaderId, int frameNumber, int status) {
            LogHelper.d(TAG, "BGEventCallback.onCompleted imgReaderId:"
                    + Integer.toHexString(imgReaderId) + ", frameNum:" + frameNumber
                    + ", status = " + status);
            synchronized (mCaptureSurfaceList) {
                // if capture surface has no imageReader,
                // should remove this capture surface out of list.
                while (!mCaptureSurfaceList.isEmpty()
                        && mCaptureSurfaceList.get(0).hasNoImageReader()) {
                    mCaptureSurfaceList.remove(0);
                }
                if (mCaptureSurfaceList.isEmpty()) {
                    return false;
                } else {
                    int size = mCaptureSurfaceList.size();
                    CaptureSurface captureSurface = null;
                    for (int i = 0; i < size; i++) {
                        CaptureSurface tempCaptureSurface = mCaptureSurfaceList.get(i);
                        if (tempCaptureSurface.hasTheImageReader(imgReaderId)) {
                            captureSurface = tempCaptureSurface;
                            break;
                        }
                    }
                    if (captureSurface == null) {
                        LogHelper.d(TAG, "BGEventCallback.onCompleted there is no" +
                                " captureSurface with the imgReader: " + imgReaderId);
                        return false;
                    }
                    if (status == REQUEST_SUCCESS) {
                        LogHelper.d(TAG, "BGEventCallback.onCompleted captureSurface = "
                                + captureSurface);
                        captureSurface.addBGEventCallbackQueue(frameNumber);
                        captureSurface.notifyImageReader();
                    } else if (status == REQUEST_ERROR) {
                        captureSurface.decreasePictureNum();
                        if (captureSurface.shouldReleaseCaptureSurface()
                                && captureSurface.getPictureNumLeft() == 0) {
                            captureSurface.releaseCaptureSurface();
                            captureSurface.releaseCaptureSurfaceLater(false);
                        }
                    }
                    LogHelper.d(TAG, "no imageReader: " + captureSurface.hasNoImageReader());
                    if (captureSurface.hasNoImageReader()) {
                        mCaptureSurfaceList.remove(0);
                        LogHelper.d(TAG, "BGEventCallback remove captureSurface");
                    }
                }
            }
            return true;
        }
    };

    private void sleep(long milliSeconds) {
        try {
            Thread.sleep(milliSeconds);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
