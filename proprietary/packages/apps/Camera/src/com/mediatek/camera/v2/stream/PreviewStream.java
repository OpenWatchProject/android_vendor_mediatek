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

import android.os.ConditionVariable;
import android.util.Size;
import android.view.Surface;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import junit.framework.Assert;

import java.util.HashMap;
import java.util.Map;

/**
 * A stream used for preview display.
 * <p>
 * preview stream is controlled by {@link PreviewStreamController} and reported status by
 * {@link PreviewStreamCallback}.
 * Any client uses this stream should consider how to use {@link PreviewStreamController}
 * and {@link PreviewStreamCallback}.
 */
public class PreviewStream implements IPreviewStream, IPreviewStream.PreviewCallback {
    private static final Tag TAG = new Tag(PreviewStream.class.getSimpleName());
    private static final int BLOCKING_TIME_OUT_MS = 2500;
    private PreviewStreamCallback       mStreamCallback;
    private PreviewSurfaceCallback      mSurfaceCallback;
    private ConditionVariable           mSurfaceReadySync = new ConditionVariable();
    private Surface                     mPreviewSurface;
    private int                         mPreviewWidth;
    private int                         mPreviewHeight;
    private PreviewCallback             mPreviewCallback;

    public PreviewStream() {

    }

    @Override
    public boolean updatePreviewSize(Size size) {
        LogHelper.i(TAG, "[updatePreviewSize]+ size:" + size.getWidth() + " x " + size.getHeight());
        Assert.assertNotNull(size);
        int width = size.getWidth();
        int height = size.getHeight();
        if (width == mPreviewWidth && height == mPreviewHeight && mPreviewSurface != null) {
            if (mSurfaceCallback != null) {
                mSurfaceCallback.onPreviewSufaceIsReady(false);
                mSurfaceCallback = null;
            }
            LogHelper.i(TAG, "[updatePreviewSize]- with the same preview size");
            return false;
        }
        // when preview size changed, should be updated with a new surface
        mPreviewSurface = null;
        // close condition variable to wait new surface really comes
        mSurfaceReadySync.close();
        mPreviewWidth = width;
        mPreviewHeight = height;
        LogHelper.i(TAG, "[updatePreviewSize]-");
        return true;
    }

    @Override
    public Map<String, Surface> getPreviewInputSurfaces() {
        LogHelper.i(TAG, "[getPreviewInputSurfaces] +");
        Map<String, Surface> surfaceMap = new HashMap<String, Surface>();
        if (mPreviewSurface == null) {
            //if resume and quickly pause activity.surface maybe not ready forever.
            //in this case, main thread maybe blocked by the surface.
            mSurfaceReadySync.block(BLOCKING_TIME_OUT_MS);
        }
        surfaceMap.put(PREVIEW_SURFACE_KEY, mPreviewSurface);
        LogHelper.i(TAG, "[getPreviewInputSurfaces]- mPreviewSurface:" + mPreviewSurface);
        return surfaceMap;
    }

    @Override
    public void setPreviewStreamCallback(PreviewStreamCallback callback) {
        mStreamCallback = callback;
    }

    @Override
    public void setOneShotPreviewSurfaceCallback(PreviewSurfaceCallback surfaceCallback) {
        mSurfaceCallback = surfaceCallback;
    }

    @Override
    public void setPreviewCallback(PreviewCallback previewCallback) {
        mPreviewCallback = previewCallback;
    }

    @Override
    public void onFirstFrameAvailable() {
        LogHelper.i(TAG, "onFirstFrameAvailable mStreamCallback:" + mStreamCallback);
        if (mStreamCallback != null) {
            mStreamCallback.onFirstFrameAvailable();
        }
    }

    @Override
    public void surfaceAvailable(Surface surface, int width, int height) {
        LogHelper.i(TAG, "surfaceAvailable surface = " + surface
                + " width = " + width
                + " height = " + height);
        if (width == mPreviewWidth && height == mPreviewHeight) {
            mPreviewSurface = surface;
            mSurfaceReadySync.open();

            if (mSurfaceCallback != null) {
                mSurfaceCallback.onPreviewSufaceIsReady(true);
                mSurfaceCallback = null;
            }
            if (mPreviewCallback != null) {
                mPreviewCallback.surfaceAvailable(surface, width, height);
            }
        }
    }

    @Override
    public void surfaceSizeChanged(Surface surface, int width, int height) {
        LogHelper.i(TAG, "surfaceSizeChanged surface = " + surface
                + " width = " + width
                + " height = " + height);
        if (width == mPreviewWidth && height == mPreviewHeight) {
            mPreviewSurface = surface;
            mSurfaceReadySync.open();
            if (mSurfaceCallback != null) {
                mSurfaceCallback.onPreviewSufaceIsReady(true);
                mSurfaceCallback = null;
            }
            if (mPreviewCallback != null) {
                mPreviewCallback.surfaceSizeChanged(surface, width, height);
            }
        } else if (mPreviewSurface == null && mPreviewWidth == 0 && mPreviewHeight == 0) {
            mPreviewSurface = surface;
            mPreviewWidth = width;
            mPreviewHeight = height;
            if (mPreviewCallback != null) {
                mPreviewCallback.surfaceSizeChanged(surface, width, height);
            }
        }
    }

    @Override
    public void surfaceDestroyed(Surface surface) {
        LogHelper.i(TAG, "surfaceDestroyed surface = " + surface + ",mPreviewSurface:"
                + mPreviewSurface + ",mPreviewCallback:" + mPreviewCallback);
        // destroy current surface
        if (surface == mPreviewSurface) {
            if (mPreviewCallback != null) {
                mPreviewCallback.surfaceDestroyed(surface);
            }
            mPreviewSurface  = null;
            mPreviewWidth    = 0;
            mPreviewHeight   = 0;
        }
    }

    @Override
    public void releasePreviewStream() {
    }
}
