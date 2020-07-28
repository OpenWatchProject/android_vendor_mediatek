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

package com.mediatek.camera.v2.control;

import java.util.Map;

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
public interface IControl {
    /**
     * Auto focus system status; 1:1 mapping from camera2 AF_STATE.
     * <ul>
     * <li>{@link #INACTIVE}</li>
     * <li>{@link #ACTIVE_SCAN}</li>
     * <li>{@link #ACTIVE_FOCUSED}</li>
     * <li>{@link #ACTIVE_UNFOCUSED}</li>
     * <li>{@link #PASSIVE_SCAN}</li>
     * <li>{@link #PASSIVE_FOCUSED}</li>
     * <li>{@link #PASSIVE_UNFOCUSED}</li>
     * </ul>
     */
    public static enum AutoFocusState {
        /** Indicates AF system is inactive for some reason (could be an error). */
        INACTIVE,
        /** Indicates active scan in progress. */
        ACTIVE_SCAN,
        /** Indicates active scan success (in focus). */
        ACTIVE_FOCUSED,
        /** Indicates active scan failure (not in focus). */
        ACTIVE_UNFOCUSED,
        /** Indicates passive scan in progress. */
        PASSIVE_SCAN,
        /** Indicates passive scan success (in focus). */
        PASSIVE_FOCUSED,
        /** Indicates passive scan failure (not in focus). */
        PASSIVE_UNFOCUSED
    }

    public interface IAaaListener {
        public RequestType getRepeatingRequestType();
        public void requestChangeCaptureRequets(boolean sync,
                RequestType requestType, CaptureType captureType);
    }
    public interface IAaaController {
        public void aePreTriggerAndCapture();
    }

    /**
     * Classes implementing this interface will be called when the state of the
     * focus changes. Guaranteed not to stay stuck in scanning state past some
     * reasonable timeout even if Camera API is stuck.
     */
    public static interface FocusStateListener {
        /**
         * Called when state of auto focus system changes.
         * @param state Current auto focus state.
         */
        public void onFocusStatusUpdate(AutoFocusState state);
    }

    // life cycle
    public void open(Activity activity, ViewGroup parentView, boolean isCaptureIntent);
    public void resume();
    public void pause();
    public void close();
    // app resources
    /**
     * Notify g-sensor's orientation changed.
     * @param orientation new orientation
     */
    public void onOrientationChanged(int orientation);
    // gesture, preview
    public void onPreviewAreaChanged(RectF previewArea);
    public void onSingleTapUp(float x, float y);
    // capture requests
    public void configuringSessionRequests(
            Map<RequestType, CaptureRequest.Builder> requestBuilders,
            CaptureType captureType,
            boolean bottomCamera);
    // capture results
    public void onPreviewCaptureStarted(CaptureRequest request, long timestamp, long frameNumber);
    public void onPreviewCaptureProgressed(CaptureRequest request, CaptureResult partialResult);
    public void onPreviewCaptureCompleted(CaptureRequest request, TotalCaptureResult result);
    // capture callback

    /**
     * Clear the focus ui.
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
