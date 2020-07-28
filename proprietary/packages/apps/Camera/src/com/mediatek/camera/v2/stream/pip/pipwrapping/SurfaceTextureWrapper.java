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

package com.mediatek.camera.v2.stream.pip.pipwrapping;

import android.graphics.SurfaceTexture;
import android.graphics.SurfaceTexture.OnFrameAvailableListener;
import android.os.Handler;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import junit.framework.Assert;

/**
 * A wrapper for surface texture.
 */
public class SurfaceTextureWrapper {
    private static final Tag TAG = new Tag(SurfaceTextureWrapper.class.getSimpleName());
    private SurfaceTexture        mSurfaceTexture    = null;
    private int                   mWidth             = -1;
    private int                   mHeight            = -1;
    private int                   mTextureId         = -12345;
    private float[]               mSTTransformMatrix = new float[16];
    private long                  mSTTimeStamp       = 0L;

    public SurfaceTextureWrapper() {

    }

    public SurfaceTextureWrapper(int surfaceTexId) {
        Assert.assertTrue(surfaceTexId >= 0);
        mTextureId = surfaceTexId;
        mSurfaceTexture = new SurfaceTexture(surfaceTexId);
    }

    public void setOnFrameAvailableListener(OnFrameAvailableListener frameListener,
                        Handler handler) {
        if (mSurfaceTexture == null) {
            throw new IllegalStateException("SurfaceTexure not created," +
                    " pls call setDefaultBufferSize" +
                    " or use SurfaceTextureWrapper(int surfaceTexId) firstly!");
        }
        mSurfaceTexture.setOnFrameAvailableListener(frameListener, handler);
    }

    public void setDefaultBufferSize(int width, int height) {
        Assert.assertTrue(width > 0);
        Assert.assertTrue(height > 0);
        int tempWidth = Math.max(width, height);
        int tempHeight = Math.min(width, height);
        if (mWidth == tempWidth && mHeight == tempHeight && mSurfaceTexture != null) {
            LogHelper.i(TAG, "skip setDefaultBufferSize w = " + tempWidth + " h = " + tempHeight);
            return;
        }
        mWidth = tempWidth;
        mHeight = tempHeight;
        if (mSurfaceTexture == null) {
            if (mTextureId < 0) {
                mTextureId = GLUtil.generateTextureIds(1)[0];
                GLUtil.bindPreviewTexure(mTextureId);
            }
            mSurfaceTexture = new SurfaceTexture(mTextureId);
        }
        mSurfaceTexture.setDefaultBufferSize(tempWidth, tempHeight);
    }

    public int getWidth() {
        return mWidth;
    }

    public int getHeight() {
        return mHeight;
    }

    public int getTextureId() {
        return mTextureId;
    }

    public SurfaceTexture getSurfaceTexture() {
        return mSurfaceTexture;
    }

    public float[] getBufferTransformMatrix() {
        return mSTTransformMatrix;
    }

    public long getBufferTimeStamp() {
        return mSTTimeStamp;
    }

    //Note: this method must be called in GL Thread.
    public void updateTexImage() {
        if (mSurfaceTexture != null) {
            mSurfaceTexture.updateTexImage();
            mSTTimeStamp = mSurfaceTexture.getTimestamp();
            mSurfaceTexture.getTransformMatrix(mSTTransformMatrix);
        }
    }

    public void resetSTStatus() {
        mSTTimeStamp       = 0L;
        mSTTransformMatrix = new float[16];
    }

    public void release() {
        resetSTStatus();
        mWidth = 0;
        mHeight = 0;
        if (mTextureId >= 0) {
            GLUtil.deleteTextures(new int[]{mTextureId});
            mTextureId = -12345;
        }
        if (mSurfaceTexture != null) {
            mSurfaceTexture.setOnFrameAvailableListener(null);
            mSurfaceTexture.release();
            mSurfaceTexture = null;
        }
    }
}