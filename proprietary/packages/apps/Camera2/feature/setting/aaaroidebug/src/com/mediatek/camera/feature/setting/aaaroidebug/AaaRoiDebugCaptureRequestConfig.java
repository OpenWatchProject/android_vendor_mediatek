/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.feature.setting.aaaroidebug;

import android.graphics.Rect;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.portability.SystemProperties;

import junit.framework.Assert;

import java.util.List;

/**
 * This class used to get 3a rang from capture result, and notify to listeners.
 */
public class AaaRoiDebugCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(AaaRoiDebugCaptureRequestConfig.class
            .getSimpleName());
    private static final String DEBUG_KEY_AE_RANGE_RESULT = "com.mediatek.3afeature.aeroi";
    private static final String DEBUG_KEY_AF_RANGE_RESULT = "com.mediatek.3afeature.afroi";
    private static final String DEBUG_KEY_AWB_RANGE_RESULT = "com.mediatek.3afeature.awbroi";

    private CaptureResult.Key<int[]> mAeRangeResultKey;
    private CaptureResult.Key<int[]> mAfRangeResultKey;
    private CaptureResult.Key<int[]> mAwbRangeResultKey;

    private DebugInfoListener mDebugInfoListener;

    private static final String LOG_ROIS_PROP = "vendor.mtk.camera.app.3a.debug.log";
    private static boolean sIsLogRois = SystemProperties.getInt(LOG_ROIS_PROP, 0) == 1;

    /**
     * Listen the 3a roi range update.
     */
    public interface DebugInfoListener {
        /**
         * Called when 3a roi rang update.
         *
         * @param aeRois     all AE range
         * @param afRois     all AF range
         * @param awbRois    all AWB rang
         * @param cropRegion Crop region in capture result
         */
        void onRangeUpdate(Rect[] aeRois, Rect[] afRois, Rect[] awbRois, Rect cropRegion);
    }

    public void setDebugInfoListener(DebugInfoListener listener) {
        mDebugInfoListener = listener;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        List<CaptureResult.Key<?>> availableResultKeys
                = characteristics.getAvailableCaptureResultKeys();
        for (int i = 0; i < availableResultKeys.size(); i++) {
            CaptureResult.Key<?> key = availableResultKeys.get(i);
            if (DEBUG_KEY_AE_RANGE_RESULT.equals(key.getName())) {
                mAeRangeResultKey = (CaptureResult.Key<int[]>) key;
                continue;
            }
            if (DEBUG_KEY_AF_RANGE_RESULT.equals(key.getName())) {
                mAfRangeResultKey = (CaptureResult.Key<int[]>) key;
                continue;
            }
            if (DEBUG_KEY_AWB_RANGE_RESULT.equals(key.getName())) {
                mAwbRangeResultKey = (CaptureResult.Key<int[]>) key;
                continue;
            }
        }
        LogHelper.d(TAG, "[setCameraCharacteristics] mAeRangeResultKey = "
                + mAeRangeResultKey + ", mAfRangeResultKey = " + mAfRangeResultKey
                + ", mAwbRangeResultKey = " + mAwbRangeResultKey);

    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {

    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {

    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return mPreviewCallback;
    }

    @Override
    public void sendSettingChangeRequest() {

    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    private CameraCaptureSession.CaptureCallback mPreviewCallback
            = new CameraCaptureSession.CaptureCallback() {

        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                                       TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);

            if (mAeRangeResultKey == null
                    && mAfRangeResultKey == null
                    && mAwbRangeResultKey == null) {
                return;
            }

            Assert.assertNotNull(result);

            Rect[] aeRois = null;
            Rect[] afRois = null;
            Rect[] awbRois = null;

            List<CaptureResult.Key<?>> keyList = result.getKeys();
            for (CaptureResult.Key<?> key : keyList) {
                if (mAeRangeResultKey != null
                        && key.getName().equals(DEBUG_KEY_AE_RANGE_RESULT)) {
                    aeRois = covertRoiToRectArray(result.get(mAeRangeResultKey));
                }
                if (mAfRangeResultKey != null
                        && key.getName().equals(DEBUG_KEY_AF_RANGE_RESULT)) {
                    afRois = covertRoiToRectArray(result.get(mAfRangeResultKey));
                }
                if (mAwbRangeResultKey != null
                        && key.getName().equals(DEBUG_KEY_AWB_RANGE_RESULT)) {
                    awbRois = covertRoiToRectArray(result.get(mAwbRangeResultKey));
                }
            }

            if (sIsLogRois) {
                LogHelper.d(TAG, "[onCaptureCompleted] aeRois = "
                        + covertRectArrayToString(aeRois));
                LogHelper.d(TAG, "[onCaptureCompleted] afRois = "
                        + covertRectArrayToString(afRois));
                LogHelper.d(TAG, "[onCaptureCompleted] awbRois = "
                        + covertRectArrayToString(awbRois));
            }

            if (mDebugInfoListener != null) {
                mDebugInfoListener.onRangeUpdate(aeRois, afRois, awbRois,
                        result.get(CaptureResult.SCALER_CROP_REGION));
            }
        }
    };

    private String covertRectArrayToString(Rect[] rect) {
        if (rect == null || rect.length == 0) {
            return "null";
        }

        String res = "";
        for (Rect r : rect) {
            if (!res.equals("")) {
                res += ",";
            }
            if (r == null) {
                res += "null";
            } else {
                res += r;
            }
        }
        return res;
    }

    /*
     * [Int array structure]
     * roi[0] = Type, reserve
     * roi[1] = Number of roi
     * roi[2+ n*5 + 0] = left of n roi
     * roi[2+ n*5 + 1] = top of n roi
     * roi[2+ n*5 + 2] = right of n roi
     * roi[2+ n*5 + 3] = bottom of n roi
     * roi[2+ n*5 + 4] = result of n roi
     * n >= 0
     * n = Number of roi - 1
     */
    private Rect[] covertRoiToRectArray(int[] roi) {
        if (roi == null || roi.length < 7 || roi.length != roi[1] * 5 + 2) {
            return null;
        }

        int numberOfRoi = roi[1];
        Rect[] res = new Rect[numberOfRoi];

        for (int i = 0; i < numberOfRoi; i++) {
            res[i] = new Rect(
                    roi[2 + i * 5],
                    roi[2 + i * 5 + 1],
                    roi[2 + i * 5 + 2],
                    roi[2 + i * 5 + 3]);
        }
        return res;
    }
}
