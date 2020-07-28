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

package com.mediatek.camera.v2.platform.module;



import android.app.Activity;
import android.graphics.RectF;
import android.view.MotionEvent;
import android.view.Surface;

import com.mediatek.camera.v2.ui.CountDownView;

import java.util.List;

public interface ModuleUi {

    public interface GestureListener {
        /**
          * Notified when a tap occurs with the down that triggered it.
          * This will be triggered immediately for
          * every down event. All other events should be preceded by this.
          *
         * @param x touch x's position
         * @param y touch y's position
         *
         * @return true intercept onDown event, false no need intercept onDown event.
         */
        public boolean onDown(float x, float y);
        public boolean onUp();
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY);
        public boolean onScroll(float dx, float dy, float totalX, float totalY);
        public boolean onSingleTapUp(float x, float y);
        public boolean onSingleTapConfirmed(float x, float y);
        public boolean onDoubleTap(float x, float y);
        public boolean onScale(float focusX, float focusY, float scale);
        public boolean onScaleBegin(float focusX, float focusY);
        public boolean onLongPress(float x, float y);
    }

    public interface PreviewTouchedListener {
        /**
         * This gets called on any preview touch event.
         */
        public boolean onPreviewTouched();
    }

    /**
     * This listener gets notified when the actual preview frame changes due
     * to preview aspect ratio changes.
     */
    public interface PreviewAreaChangedListener {
        public void onPreviewAreaChanged(RectF previewArea);
    }

    /**
     * Notify orientation changed to Module UI.
     * @param orientation
     */
    public void onOrientationChanged(int orientation);
    /**
     * This callback notify preview surface is available.
     * @param surface preview surface
     * @param width surface's width
     * @param height surface's height
     */
    public void onSurfaceAvailable(final Surface surface, int width, int height);
    /**
     * When preview surface destroy, this callback will be called.
     * @param surface the destroyed preview surface.
     * @return
     */
    public boolean onSurfaceDestroyed(Surface surface);
    /**
     * This callback is called when surface's size changed.
     * @param surface the size changed surface.
     * @param width new width of the surface
     * @param height new height of the surface
     */
    public void onSurfaceSizeChanged(Surface surface, int width, int height);

    /**
     * The preview status listener needs to provide an
     * {@link GestureListener} in order to listen
     * to the touch events that happen on preview.
     *
     * @return a listener that listens to touch events
     */
    public GestureListener getGestureListener();
    /**
     *
     * @return a listener that listens to preview touch
     */
    public PreviewTouchedListener getPreviewTouchedListener();

    public void onPreviewAreaChanged(RectF previewArea);

    /**
     * Starts the count down timer.
     *
     * @param sec seconds to count down
     */
    public void startCountdown(int sec);
    /**
     * Sets a listener that gets notified when the count down is finished.
     */
    public void setCountdownFinishedListener(CountDownView.OnCountDownStatusListener listener);
    /**
     * Returns whether the count down is on-going.
     */
    public boolean isCountingDown();
    /**
     * Cancels the on-going count down, if any.
     */
    public void cancelCountDown();

    /**
     * when current camera have changed,will notify module UI.
     * @param previewCameraId current preview camera id list.
     * @param isRemoteOpened this used for tag remote camera is opened.
     *          true means
     */
    public void updatePreviewCamera(List<String> previewCameraId, boolean isRemoteOpened);

    /**
     * will update current recording statues.
     * @param isStarted true means current is recording ; false is stopped.
     */
    public void updateRecordingStatus(boolean isStarted);

    /**
     * used for Cross Mount.
     * when mode is doing capture,need notify module,
     * current is capturing. when finished capture, also need update the status
     * @param isCapturing
     *            true means current is in capturing; otherwise is not
     *            capturing.
     */
    public void updateCaptureStatus(boolean isCapturing);

    /**
     * When focus status change, need notify module.
     * @param isFocusing true means current is in focusing state,
     *                   otherwise is not in focusing.
     */
    public void updateFocusStatus(boolean isFocusing);

    /********************** Life cycle management **********************/

    /**
     * Initializes the module.
     *
     * @param activity
     *            The camera activity.
     * @param isSecureCamera
     *            Whether the app is in secure camera mode.
     * @param isCaptureIntent
     *            Whether the app is in capture intent mode.
     */
    public void open(Activity activity, boolean isSecureCamera, boolean isCaptureIntent);

    /**
     * Resumes the module. Always call this method whenever it's being put in
     * the foreground.
     */
    public void resume();

    /**
     * Pauses the module. Always call this method whenever it's being put in the
     * background.
     */
    public void pause();

    /**
     * Destroys the module. Always call this method to release the resources used
     * by this module.
     */
    public void close();
}
