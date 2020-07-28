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

package com.mediatek.camera.common.mode.photo;

import android.content.Context;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.mode.CameraApiHelper;

/**
 * For p2done callback check.
 */

public class P2DoneInfo {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(P2DoneInfo.class.getSimpleName());
    private static final String P2_KEY_SUPPORT_MODES =
            "com.mediatek.control.capture.early.notification.support";
    private static final String P2_KEY_NOTIFICATION_TRIGGER =
            "com.mediatek.control.capture.early.notification.trigger";
    private static final String P2_KEY_NOTIFICATION_RESULT =
            "com.mediatek.control.capture.next.ready";
    private static final int[] P2DONE_SUPPORT = new int[]{1};
    private static boolean mIsSupport = false;
    private static CaptureRequest.Key<int[]> mKeyP2NotificationRequest;
    private static CaptureResult.Key<int[]> mKeyP2NotificationResult;

    /**
     * when open camera, check whether it supports postview callback.
     * @param context The context.
     * @param cameraId The camera id.
     */
    public static void setCameraCharacteristics(Context context, int cameraId) {
        DeviceDescription deviceDescription = CameraApiHelper.getDeviceSpec(context)
                .getDeviceDescriptionMap().get(String.valueOf(cameraId));
        if (deviceDescription != null) {
            mIsSupport = deviceDescription.isSpeedUpSupport();
        }

        LogHelper.d(TAG, "[setCameraCharacteristics], mIsSupport = " + mIsSupport);
        if (!mIsSupport) {
            return;
        }

        if (deviceDescription != null) {
            mKeyP2NotificationRequest = deviceDescription.getKeyP2NotificationRequestMode();
            mKeyP2NotificationResult = deviceDescription.getKeyP2NotificationResult();
        }

    }

    public static boolean isSupported() {
        return mIsSupport;
    }

    public static boolean enableP2Done(CaptureRequest.Builder builder) {
        if (mIsSupport) {
            builder.set(mKeyP2NotificationRequest, P2DONE_SUPPORT);
            return true;
        }
        return false;
    }

    public static boolean checkP2DoneResult(CaptureResult result) {
        if (mIsSupport) {
            int re[] = result.get(mKeyP2NotificationResult);
            if (re != null && re[0] == P2DONE_SUPPORT[0]) {
                return true;
            }
        }
        return false;
    }
}
