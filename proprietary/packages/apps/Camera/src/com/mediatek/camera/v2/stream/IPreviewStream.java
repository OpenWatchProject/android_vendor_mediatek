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

package com.mediatek.camera.v2.stream;

import java.util.Map;

import android.util.Size;
import android.view.Surface;

/**
 *
 */
public interface IPreviewStream {
    public static String               PREVIEW_SURFACE_KEY = "PreviewStream.Surface";
    /**
     *  This callback is used by 3rd party to notify preview surface status is changed.
     */
    public interface PreviewCallback {
        /**
         * This callback notify preview surface is available.
         * @param surface preview surface
         * @param width surface's width
         * @param height surface's height
         */
        public void surfaceAvailable(final Surface surface, int width, int height);
        /**
         * When preview surface destroy, this callback will be called.
         * @param surface the destroyed preview surface.
         */
        public void surfaceDestroyed(Surface surface);
        /**
         * This callback is called when surface's size changed.
         * @param surface the size changed surface.
         * @param width new width of the surface
         * @param height new height of the surface
         */
        public void surfaceSizeChanged(Surface surface, int width, int height);
    }

    /**
     *  This callback is used by PreviewStream to notify first frame is available.
     */
    public interface PreviewStreamCallback {
        /**
         * notify first preview frame ready
         */
        public void onFirstFrameAvailable();
    }

    public interface PreviewSurfaceCallback {
        public void onPreviewSufaceIsReady(boolean surfaceChanged);
    }

    /**
     * Set preview size.
     * @param size the desired preview size
     * @return true, need reconfigure surface again.
     */
    public boolean updatePreviewSize(Size size);
    /**
     * Get preview stream's input surface
     * <p>
     * Note: be more careful call this method in UI thread.
     * @return the preview surface map, the key indicates the surface's usage.
     */
    public Map<String, Surface> getPreviewInputSurfaces();

    /**
     *
     * @param callback
     */
    public void setPreviewStreamCallback(PreviewStreamCallback callback);

    /**
      * Installs a callback to be invoked for the next preview. After one invocation,
      * the callback is cleared.
      */
    public void setOneShotPreviewSurfaceCallback(PreviewSurfaceCallback surfaceCallback);

    /**
     * Set preview callback to preview stream, preview stream can notify surface
     * status through this callback(example: surface destory).
     * @param previewCallback preview callback
     */
    public void setPreviewCallback(PreviewCallback previewCallback);

    public void onFirstFrameAvailable();

    public void releasePreviewStream();
}