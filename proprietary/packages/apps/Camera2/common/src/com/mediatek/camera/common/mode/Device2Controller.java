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
 *     MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.camera.common.mode;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.mediatek.camera.common.device.v2.Camera2Proxy;

import javax.annotation.Nonnull;

/**
 * Use to control device and handle device call back message.
 */
public abstract class Device2Controller {
    private static final int MSG_DEVICE_ON_CAMERA_OPENED = 0;
    private static final int MSG_DEVICE_ON_CAMERA_CLOSED = 1;
    private static final int MSG_DEVICE_ON_CAMERA_DISCONNECTED = 2;
    private static final int MSG_DEVICE_ON_CAMERA_ERROR = 3;
    protected final ModeHandler mModeHandler;


    /**
     * Create mode handler, use current thread looper.
     */
    public Device2Controller() {
        mModeHandler = new ModeHandler(Looper.myLooper());
    }

    /**
     * Mode handler run in mode thread.
     */
    private class ModeHandler extends Handler {

        public ModeHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_DEVICE_ON_CAMERA_OPENED:
                    doCameraOpened((Camera2Proxy) msg.obj);
                    break;

                case MSG_DEVICE_ON_CAMERA_CLOSED:
                    doCameraClosed((Camera2Proxy) msg.obj);
                    break;

                case MSG_DEVICE_ON_CAMERA_DISCONNECTED:
                    doCameraDisconnected((Camera2Proxy) msg.obj);
                    break;

                case MSG_DEVICE_ON_CAMERA_ERROR:
                    doCameraError((Camera2Proxy) msg.obj, msg.arg1);
                    break;

                default:
                    break;
            }
        }
    }

    /**
     * Use to receive device state callback.
     */
    public class DeviceStateCallback extends Camera2Proxy.StateCallback {

        @Override
        public void onOpened(@Nonnull Camera2Proxy camera2proxy) {
            mModeHandler.obtainMessage(MSG_DEVICE_ON_CAMERA_OPENED,
                    camera2proxy).sendToTarget();
        }

        @Override
        public void onClosed(@Nonnull Camera2Proxy camera2Proxy) {
            mModeHandler.obtainMessage(MSG_DEVICE_ON_CAMERA_CLOSED,
                    camera2Proxy).sendToTarget();
        }

        @Override
        public void onDisconnected(@Nonnull Camera2Proxy camera2proxy) {
            mModeHandler.obtainMessage(MSG_DEVICE_ON_CAMERA_DISCONNECTED,
                    camera2proxy).sendToTarget();
        }

        @Override
        public void onError(@Nonnull Camera2Proxy camera2Proxy, int error) {
            mModeHandler.obtainMessage(MSG_DEVICE_ON_CAMERA_ERROR, error, 0,
                    camera2Proxy).sendToTarget();
        }

    }

    protected abstract void doCameraOpened(Camera2Proxy camera2proxy);

    protected abstract void doCameraDisconnected(Camera2Proxy camera2proxy);

    protected abstract void doCameraError(Camera2Proxy camera2Proxy, int error);

    protected void doCameraClosed(Camera2Proxy camera2proxy) {
        mModeHandler.removeMessages(MSG_DEVICE_ON_CAMERA_OPENED);
    }
}
