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
package com.mediatek.camera.ui.preview;

import android.annotation.TargetApi;
import android.os.Build;
import android.view.Surface;

import com.mediatek.camera.common.gles.egl.EglCore;
import com.mediatek.camera.common.gles.egl.EglSurfaceBase;

/**
 * OpenGL thread, used to draw a black screen in surface view.
 */
@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
class GLProducerThread extends Thread {
    private GLRenderer mRenderer;
    private Object mSyncLock;
    private Surface mSurface;
    private EglCore mEglCore;
    private EglSurfaceBase mEglSurfaceBase = null;

    /**
     * OpenGL render interface.
     */
    interface GLRenderer {
        /**
         * Draw a frame.
         */
        void drawFrame();
    }

    GLProducerThread(Surface surface, GLRenderer renderer, Object syncLock) {
        mSurface = surface;
        mRenderer = renderer;
        mSyncLock = syncLock;
    }

    @Override
    public void run() {
        try {
            initGL();
            if (mRenderer != null) {
                ((GLRendererImpl) mRenderer).initGL();
            }
            if (mRenderer != null) {
                mRenderer.drawFrame();
            }
            mEglSurfaceBase.swapBuffers();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            destroyGL();
            synchronized (mSyncLock) {
                mSyncLock.notifyAll();
            }
        }
    }

    private void initGL() {
        mEglCore = new EglCore();
        mEglSurfaceBase = new EglSurfaceBase(mEglCore);
        mEglSurfaceBase.createWindowSurface(mSurface);
        mEglSurfaceBase.makeCurrent();
    }

    private void destroyGL() {
        mEglSurfaceBase.makeNothingCurrent();
        mEglSurfaceBase.releaseEglSurface();
        mEglCore.release();
    }
}
