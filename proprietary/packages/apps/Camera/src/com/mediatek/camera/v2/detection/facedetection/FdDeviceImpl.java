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
 * MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.camera.v2.detection.facedetection;

import android.graphics.Point;
import android.graphics.Rect;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.Face;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.detection.IDetectionCaptureObserver;
import com.mediatek.camera.v2.detection.IDetectionDevice;
import com.mediatek.camera.v2.detection.IDetectionManager.IDetectionListener;
import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.vendortag.TagMetadata;
import com.mediatek.camera.v2.vendortag.TagRequest;

import junit.framework.Assert;

/**
 * Face detection device which used to interact with capture device.
 */
public class FdDeviceImpl implements IDetectionDevice {
    private static final Tag TAG = new Tag(FdDeviceImpl.class.getSimpleName());
    private IFdPresenterListener mListener;
    private IDetectionListener mDetectionListener;

    private FdCaptureObserver mCaptureObserver = new FdCaptureObserver();
    private boolean mIsFdRequested = false;
    private boolean mIsForceFace3aSupported = false;
    private static final int ARRAY_LENGTH = 3;

    /**
     * Constructor of face detection device.
     *
     * @param detectionListener
     *            Listener used for get capture callback from detection manager.
     */
    public FdDeviceImpl(IDetectionListener detectionListener) {
        mDetectionListener = detectionListener;
    }

    protected IDetectionCaptureObserver getCaptureObserver() {
        return mCaptureObserver;
    }

    @Override
    public void requestStartDetection() {
        LogHelper.i(TAG, "startFaceDetection");
        mIsFdRequested = true;
        mDetectionListener.requestChangeCaptureRequest(false,
                mDetectionListener.getRepeatingRequestType(), CaptureType.REPEATING_REQUEST);
    }

    @Override
    public void requestStopDetection() {
        LogHelper.i(TAG, "stopFaceDetection");
        mIsFdRequested = false;
        mDetectionListener.requestChangeCaptureRequest(false,
                mDetectionListener.getRepeatingRequestType(), CaptureType.REPEATING_REQUEST);
    }

    public void setListener(IFdPresenterListener listener) {
        mListener = listener;
    }

    protected void setForceFace3a(boolean isForceFace3aSupported) {
        mIsForceFace3aSupported = isForceFace3aSupported;
    }

    /**
     * Class used for get capture callback from detection manager.
     */
    private class FdCaptureObserver implements IDetectionCaptureObserver {

        @Override
        public void configuringRequests(CaptureRequest.Builder requestBuilder,
                RequestType requestType) {
            int fdMode = mIsFdRequested ? CaptureRequest.STATISTICS_FACE_DETECT_MODE_SIMPLE
                    : CaptureRequest.STATISTICS_FACE_DETECT_MODE_OFF;
            requestBuilder.set(CaptureRequest.STATISTICS_FACE_DETECT_MODE, fdMode);
            int forceFace3aMode = TagMetadata.MTK_FORCE_FACE_3A_OFF;
            if (mIsForceFace3aSupported) {
                forceFace3aMode = mIsFdRequested ? TagMetadata.MTK_FORCE_FACE_3A_ON
                        : TagMetadata.MTK_FORCE_FACE_3A_OFF;
                requestBuilder.set(TagRequest.STATISTICS_FORCE_FACE_3A, forceFace3aMode);
            }
            LogHelper.i(TAG, "configuringRequests done,fdMode = " + fdMode + " ,forceFace3a = "
                    + forceFace3aMode);
        }

        @Override
        public void onCaptureStarted(CaptureRequest request, long timestamp, long frameNumber) {

        }

        @Override
        public void onCaptureCompleted(CaptureRequest request, TotalCaptureResult result) {
            Assert.assertNotNull(result);
            Face[] faces = result.get(CaptureResult.STATISTICS_FACES);
            int length = faces.length;
            int[] ids = result.get(CaptureResult.STATISTICS_FACE_IDS);
            Rect[] rectangles = result.get(CaptureResult.STATISTICS_FACE_RECTANGLES);
            byte[] scores = result.get(CaptureResult.STATISTICS_FACE_SCORES);
            Point[][] pointsInfo = new Point[length][ARRAY_LENGTH];
            Rect cropRegion = result.get(CaptureResult.SCALER_CROP_REGION);
            LogHelper.i(TAG, "[onCaptureCompleted] faces's length = " + length);
            if (length != 0) {
                for (int i = 0; i < length; i++) {
                    pointsInfo[i][0] = faces[i].getLeftEyePosition();
                    pointsInfo[i][1] = faces[i].getRightEyePosition();
                    pointsInfo[i][2] = faces[i].getMouthPosition();
                }
            }
            // Notify face presenter to update UI.
            mListener.onFaceDetected(ids, rectangles, scores, pointsInfo, cropRegion);
        }
    }
}
