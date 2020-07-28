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

package com.mediatek.camera.v2.stream.pip;

import com.mediatek.camera.v2.stream.IRecordStream.RecordStreamStatus;

import android.app.Activity;
import android.graphics.RectF;
import android.util.Size;
import android.view.Surface;

/**
 *  We design {@link PipStreamController} can be reused by 3rd-party application.
 *  In out application, we use PipMode instruct how to use {@link PipStreamController}.
 *  <p>
 *  Interfaces are divided into the following categories:
 *  <p>
 *  <li> Life cycle.</li>
 *  <li> App level resources.</li>
 *  <li> Pip stream status callback.</li>
 *  <li> Pip common control.</li>
 *  <li> Pip capture or vss</li>
 *  <li> Pip video recording</li>
 */
public interface IPipStream {
    /**
     * indicate one surface is belong to Main camera or Sub camera.
     * Note: "Bottom Camera" may be Main camera or Sub camera,
     * Here Main camera indicates it's camera id 0
     */
    public static final String         BOTTOM_SURFACE_KEY = "PipStreamController.Main";
    public static final String         TOP_SURFACE_KEY    = "PipStreamController.Sub";
    // Life cycle
    public void open(Activity activity);
    public void resume();
    /**
     *  Notify activity pause event happen
     *  for preview draw buffer opengl error.
     */
    public void onActivityPause();
    public void pause();
    public void close();
    // app resources
    public void onOrientationChanged(int gsensorOrientation);
    public void onPreviewAreaChanged(RectF previewArea);
    public boolean onDown(float x, float y);
    public boolean onScroll(float dx, float dy, float totalX, float totalY);
    public boolean onSingleTapUp(float x, float y);
    public boolean onLongPress(float x, float y);
    public boolean onUp();
    /**
     *  When pip stream's status changes, using {@link PipStreamCallback}
     *  to notify <item>Mode</item>
     *  or stream ui.
     */
    public void registerPipStreamCallback(PipStreamCallback callback);
    public void unregisterPipStreamCallback(PipStreamCallback callback);
    public interface PipStreamCallback {
        public void onOpened();
        public void onClosed();
        public void onPaused();
        public void onResumed();
        public void onTopGraphicTouched();
        public void onSwitchPipEventReceived();
    }
    // pip common control, it can occur among preview, capture, video recording
    public void onTemplateChanged(
            int rearResId,
            int frontResId,
            int highlightResId,
            int editBtnResId);
    public void switchingPip();
    // Pip capture or vss related
    public void setCaptureSize(
            Size bottomCaptureSize,
            Size topCaptureSize);
    // Pip video recording
    public void registerPipRecordStreamCallback(RecordStreamStatus callback);
    public void unregisterPipRecordStreamCallback(RecordStreamStatus callback);
    /**
     * Set preview surface.
     * @param surface preview surface.
     */
    public void setPreviewSurface(Surface surface);
}