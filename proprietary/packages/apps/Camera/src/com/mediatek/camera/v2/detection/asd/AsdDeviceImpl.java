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

package com.mediatek.camera.v2.detection.asd;

import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.detection.IDetectionCaptureObserver;
import com.mediatek.camera.v2.detection.IDetectionDevice;
import com.mediatek.camera.v2.detection.IDetectionManager.IDetectionListener;
import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.vendortag.TagMetadata;
import com.mediatek.camera.v2.vendortag.TagRequest;
import com.mediatek.camera.v2.vendortag.TagResult;

/**
 *
 * Device used to receive and process auto scene data.
 *
 */
public class AsdDeviceImpl implements IDetectionDevice {
    private static final Tag TAG = new Tag(AsdDeviceImpl.class.getSimpleName());
    private static final boolean DEBUG = true;
    // Whether auto scene mode detection had been started or not
    private boolean mIsAsdOpened = false;
    // True means FULL mode will be requested(both scene and HDR values will be
    // returned).Otherwise,SIMPLE mode will be requested(Only scene value will be returned).Can be
    // customized.
    private boolean mIsFullMode = false;

    private IDetectionListener mDetectionListener;
    private AsdCaptureObserver mAsdCaptureObserver = new AsdCaptureObserver();
    private IAsdPresenterListener mPresenterListener;

    /**
     * AsdDeviceImpl constructor.Auto scene presenter create and pass user behavior to it.
     *
     * @param additionListener
     *            Listener used to submit request.
     */
    public AsdDeviceImpl(IDetectionListener additionListener) {
        mDetectionListener = additionListener;
    }

    @Override
    public void requestStartDetection() {
        LogHelper.i(TAG, "requestStartDetection ");
        mIsAsdOpened = true;
        mDetectionListener.requestChangeCaptureRequest(false,
                mDetectionListener.getRepeatingRequestType(), CaptureType.REPEATING_REQUEST);
    }

    @Override
    public void requestStopDetection() {
        LogHelper.i(TAG, "requestStopDetection");
        mIsAsdOpened = false;
        mDetectionListener.requestChangeCaptureRequest(true,
                mDetectionListener.getRepeatingRequestType(), CaptureType.REPEATING_REQUEST);
    }

    protected void setListener(IAsdPresenterListener listerner) {
        mPresenterListener = listerner;
    }

    public IDetectionCaptureObserver getCaptureObserver() {
        return mAsdCaptureObserver;
    }

    /**
     *
     * Class used for receive and process capture request and result.
     *
     */
    private class AsdCaptureObserver implements IDetectionCaptureObserver {

        @Override
        public void configuringRequests(CaptureRequest.Builder requestBuilder,
                RequestType requestType) {
            if (DEBUG) {
                LogHelper.i(TAG, "configuringRequests mIsAsdOpened = "
                        + mIsAsdOpened + ",mIsFullMode = " + mIsFullMode);
            }
            int value = TagMetadata.MTK_FACE_FEATURE_ASD_MODE_OFF;
            if (mIsAsdOpened) {
                if (mIsFullMode) {
                    value = TagMetadata.MTK_FACE_FEATURE_ASD_MODE_FULL;
                } else {
                    value = TagMetadata.MTK_FACE_FEATURE_ASD_MODE_SIMPLE;
                }
            }
            requestBuilder.set(TagRequest.STATISTICS_ASD_MODE, value);
        }

        @Override
        public void onCaptureStarted(CaptureRequest request, long timestamp, long frameNumber) {

        }

        @Override
        public void onCaptureCompleted(CaptureRequest request, TotalCaptureResult result) {
            Integer asdRequest = request.get(TagRequest.STATISTICS_ASD_MODE);
            LogHelper.d(TAG, "onCaptureCompleted asdRequest = " + asdRequest);
            // mode[0] is the value of current auto scene and mode[1] is the value of HDR
            int[] mode = result.get(TagResult.STATISTICS_ASD_RESULT);
            if (mode == null) {
                return;
            }
            int length = mode.length;
            for (int i = 0; i < length; i++) {
                LogHelper.d(TAG, "onCaptureCompleted mode[" + i + "]= " + mode[i]);
            }
            if (mPresenterListener != null) {
                mPresenterListener.onSceneUpdate(mode[0]);
            }
        }
    }
}
