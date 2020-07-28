/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 *     the prior written permission of MediaTek inc. and/or its licensor, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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

package com.mediatek.camera.common.gles;

import javax.annotation.Nonnull;

/**
 *
 */
public class CameraGLThread {
    /**
     * Simultaneously draws to two surface with draw twice.
     */
    public static final int RENDER_MODE_DRAW_TWICE = 0;
    /**
     * Simultaneously draws to two surface with draw offscreen and blit twice.
     */
    public static final int RENDER_MODE_FBO = 1;
    /**
     * Simultaneously draws to two surface with draw onscreen and blit framebuffer.
     */
    public static final int RENDER_MODE_BLIT_FRAMEBUFFER = 2;
    private Renderer mRenderer;
    private int mRenderMode = RENDER_MODE_DRAW_TWICE;

    /**
     * Frame renderer interface, implementation can have its own render logical.
     */
    public interface Renderer {
        /**
         * Called when the rendering thread starts.
         */
        void onGLThreadStarted();

        /**
         * Called to draw the current frame.
         */
        void onDrawFrame();

        /**
         * prepare to swap frame to first buffer.
         */
        void prepareFirstSwapBuffer();

        /**
         * prepare to swap frame to second buffer.
         */
        void prepareSecondSwapBuffer();
    }

    /**
     * Set the rendering mode.
     *
     * @param renderMode one of the RENDER_MODE_X constants.
     */
    public void setRenderMode(int renderMode) {
        mRenderMode = renderMode;
    }

    /**
     * Set the renderer associated with this thread.
     *
     * @param renderer the renderer to use to perform OpenGL ES drawing.
     */
    public void setRenderer(@Nonnull Renderer renderer) {
        mRenderer = renderer;
    }

    /**
     * Request that the renderer render a frame.
     */
    public void requestRender() {
    }
}