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

package com.mediatek.camera.feature.setting.facedetection;

import android.annotation.TargetApi;
import android.graphics.Rect;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.CoordinatesTransform;

import junit.framework.Assert;

import java.util.ArrayList;
import java.util.List;

/**
 * It is the face implement for API2.
 */
@SuppressWarnings("deprecation")
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class FaceCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure,
        IFaceConfig {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            FaceCaptureRequestConfig.class.getSimpleName());
    private FaceDeviceCtrl.IFacePerformerMonitor mFaceMonitor;
    private OnDetectedFaceUpdateListener mOnDetectedFaceUpdateListener;
    private OnFaceValueUpdateListener mOnFaceValueUpdateListener;
    private boolean mIsRequestConfigSupported;
    private boolean mIsVendorFace3ASupported;
    private List<String> mSupportValueList = new ArrayList<String>();
    private static final int[] FACE_FORCE_FACE_3A_OFF = new int[]{0};
    private static final int[] FACE_FORCE_FACE_3A_ON = new int[]{1};
    private static final String FACE_DETECTION_FORCE_FACE_3A =
            "com.mediatek.facefeature.forceface3a";
    private CaptureRequest.Key<int[]> mFaceForce3aModesRequestKey;

    /**
     * Constructor of face parameter config in api1.
     * @param settingDevice2Requester device requester.
     */
    public FaceCaptureRequestConfig(ISettingManager.SettingDevice2Requester
                                            settingDevice2Requester) {
    }

    @Override
    public void setFaceMonitor(FaceDeviceCtrl.IFacePerformerMonitor monitor) {
        mFaceMonitor = monitor;
    }

    @Override
    public void updateImageOrientation() {

    }

    @Override
    public void resetFaceDetectionState() {}

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mIsRequestConfigSupported = isFaceDetectionSupported(characteristics);
        mFaceMonitor.setSupportedStatus(mIsRequestConfigSupported);
        if (mIsRequestConfigSupported) {
            mSupportValueList.clear();
            mSupportValueList.add(IFaceConfig.FACE_DETECTION_ON);
            mSupportValueList.add(IFaceConfig.FACE_DETECTION_OFF);
            mIsVendorFace3ASupported = isFace3ASupported(characteristics);
        }
        LogHelper.d(TAG, "[setCameraCharacteristics] mIsRequestConfigSupported = " +
                mIsRequestConfigSupported + ", mIsVendorFace3ASupported = " +
                mIsVendorFace3ASupported);
        mOnFaceValueUpdateListener.onFaceSettingValueUpdate(mIsRequestConfigSupported,
                mSupportValueList);
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        if (CameraUtil.isStillCaptureTemplate(captureBuilder)) {
            LogHelper.i(TAG, "[configCaptureRequest] capture request not has face dection.");
            return;
        }
        if (mFaceMonitor.isNeedToStart()) {
            LogHelper.i(TAG, "[configCaptureRequest] start face detection, this: " + this);
            captureBuilder.set(CaptureRequest.STATISTICS_FACE_DETECT_MODE,
                    CaptureRequest.STATISTICS_FACE_DETECT_MODE_SIMPLE);
            if (mIsVendorFace3ASupported) {
                captureBuilder.set(mFaceForce3aModesRequestKey, FACE_FORCE_FACE_3A_ON);
            }
        }
        if (mFaceMonitor.isNeedToStop()) {
            LogHelper.i(TAG, "[configCaptureRequest] stop face detection");
            captureBuilder.set(CaptureRequest.STATISTICS_FACE_DETECT_MODE,
                    CaptureRequest.STATISTICS_FACE_DETECT_MODE_OFF);
            if (mIsVendorFace3ASupported) {
                captureBuilder.set(mFaceForce3aModesRequestKey, FACE_FORCE_FACE_3A_OFF);
            }
        }
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {

    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return mPreviewCallback;
    }

    @Override
    public void sendSettingChangeRequest() {
    }

    @Override
    public void setFaceDetectionUpdateListener(OnDetectedFaceUpdateListener listener) {
        mOnDetectedFaceUpdateListener = listener;
    }

    @Override
    public void setFaceValueUpdateListener(OnFaceValueUpdateListener listener) {
        mOnFaceValueUpdateListener = listener;
    }

    public static boolean isFaceDetectionSupported(CameraCharacteristics cameraCharacteristics) {
        int faceNum = 0;
        try {
            faceNum = cameraCharacteristics
                    .get(CameraCharacteristics.STATISTICS_INFO_MAX_FACE_COUNT);
            LogHelper.d(TAG, "[isFaceDetectionSupported] faceNum = " + faceNum);
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, "[isFaceDetectionSupported] IllegalArgumentException");
        }
        return faceNum > 0;
    }

    private boolean isFace3ASupported(CameraCharacteristics cameraCharacteristics) {
        List<CaptureRequest.Key<?>> availableKeys =
                cameraCharacteristics.getAvailableCaptureRequestKeys();
        for (CaptureRequest.Key<?> key : availableKeys) {
            if (FACE_DETECTION_FORCE_FACE_3A.equals(key.getName())) {
                mFaceForce3aModesRequestKey = (CaptureRequest.Key<int[]>) key;
                return true;
            }
        }
        return false;
    }

    private CameraCaptureSession.CaptureCallback mPreviewCallback
            = new CameraCaptureSession.CaptureCallback() {

        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                                       TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            Assert.assertNotNull(result);
            android.hardware.camera2.params.Face[] faces
                    = result.get(CaptureResult.STATISTICS_FACES);
            Rect cropRegion = result.get(CaptureResult.SCALER_CROP_REGION);
            Rect[] previewRect = getPreviewRect(faces, cropRegion);
            if (mOnDetectedFaceUpdateListener != null) {
                mOnDetectedFaceUpdateListener.onDetectedFaceUpdate(getFaces(faces,
                        previewRect, cropRegion));
            }
        }
    };

    private Face[] getFaces(android.hardware.camera2.params.Face[] faces,
                            Rect[] previewRect, Rect cropRegion) {
        if (faces == null || (faces != null && faces.length == 0)) {
            return null;
        }
        Face[] faceStructures = new Face[faces.length];
        Face faceTemp = null;
        for (int i = 0; i < faces.length; i++) {
            faceTemp = new Face();
            faceTemp.id = faces[i].getId();
            faceTemp.score = faces[i].getScore();
            faceTemp.cropRegion = cropRegion;
            faceTemp.rect = previewRect[i];
            faceStructures[i] = faceTemp;
        }
        return faceStructures;
    }

    private Rect[] getPreviewRect(android.hardware.camera2.params.Face[] faces, Rect cropRegion) {
        if (faces == null || (faces != null && faces.length == 0)) {
            return null;
        }
        Rect[] previewRect = new Rect[faces.length];
        Rect rectTemp = null;
        for (int i = 0; i < faces.length; i++) {
            rectTemp = CoordinatesTransform.sensorToNormalizedPreview(faces[i].getBounds(),
                    mOnFaceValueUpdateListener.onFacePreviewSizeUpdate().getWidth(),
                    mOnFaceValueUpdateListener.onFacePreviewSizeUpdate().getHeight(), cropRegion);
            previewRect[i] = rectTemp;
        }
        return previewRect;
    }
}
