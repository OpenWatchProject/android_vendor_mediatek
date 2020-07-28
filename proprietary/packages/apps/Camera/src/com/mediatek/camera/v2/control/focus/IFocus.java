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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera.v2.control.focus;

import com.mediatek.camera.v2.control.IControl.FocusStateListener;
import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;

import android.app.Activity;
import android.graphics.RectF;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.view.ViewGroup;

/**
 *
 */
public interface IFocus {
    public interface AutoFocusUI {
        public void setFocusPosition(int x, int y, boolean isPassiveScan, int aFsize, int aEsize);
        public void setFocusPosition(int x, int y, boolean isPassiveScan);
        // active auto focus
        public void onFocusStarted();
        public void onFocusSucceeded();
        public void onFocusFailed();
        // passive auto focus
        public void setPassiveFocusSuccess(boolean success);
    }
    // life cycle
    public void open(Activity activity, ViewGroup parentView, boolean isCaptureIntent);
    public void resume();
    public void pause();
    public void close();
    // app resources
    public void onOrientationCompensationChanged(int orientationCompensation);
    // gesture, preview
    public void onPreviewAreaChanged(RectF previewArea);
    public void onSingleTapUp(float x, float y);

    // capture request
    public void configuringSessionRequest(
            RequestType requestType,
            CaptureRequest.Builder requestBuilder,
            CaptureType captureType,
            boolean bottomCamera);
    // capture results
    public void onPreviewCaptureStarted(CaptureRequest request, long timestamp, long frameNumber);
    public void onPreviewCaptureProgressed(CaptureRequest request, CaptureResult partialResult);
    public void onPreviewCaptureCompleted(CaptureRequest request, TotalCaptureResult result);

    /**
     * Clear the focus UI.
     */
    public void clearFocusUi();
    /**
     * Sets a listener that is called whenever the focus state of
     * the camera changes.
     * @param listener listener instance.
     */
    public void setFocusStateListener(FocusStateListener listener);

    /**
     * Clear a listener that is called whenever the focus state of
     * the camera changes.
     * @param listener listener instance.
     */
    public void clearFocusStateListener(FocusStateListener listener);

}
