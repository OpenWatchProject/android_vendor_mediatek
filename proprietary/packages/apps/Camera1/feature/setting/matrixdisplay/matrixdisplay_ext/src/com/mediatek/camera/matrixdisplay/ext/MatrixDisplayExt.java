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
package com.mediatek.camera.matrixdisplay.ext;

import android.view.Surface;

import com.mediatek.matrixeffect.MatrixEffect;
import com.mediatek.matrixeffect.MatrixEffect.EffectsCallback;

/**
 * Matrix display extension.
 */

public class MatrixDisplayExt {
    private static MatrixDisplayExt sMatrixDisplayExt;
    private MatrixEffect mMatrixEffect;
    private EffectAvailableCallback mCallback;

    /**
     * Callback when preview buffer is handled and effect buffer is available.
     */
    public interface EffectAvailableCallback {
        /**
         * Callback when effect buffer is available at the first time.
         */
        void onEffectAvailable();
    }

    private MatrixDisplayExt() {
        mMatrixEffect = MatrixEffect.getInstance();
    }

    /**
     * Get a static matrix display ext instance.
     *
     * @return An instance of {@link MatrixDisplayExt}.
     */
    public static MatrixDisplayExt getInstance() {
        if (sMatrixDisplayExt == null) {
            sMatrixDisplayExt = new MatrixDisplayExt();
        }
        return sMatrixDisplayExt;
    }

    /**
     * Set effect available callback to be notified when first effect buffer
     * is available.
     *
     * @param callback The object of {@link EffectAvailableCallback}.
     */
    public void setCallback(EffectAvailableCallback callback) {
        mCallback = callback;
        if (mCallback == null) {
            mMatrixEffect.setCallback(null);
        } else {
            mMatrixEffect.setCallback(new EffectsCallback() {
                @Override
                public void onEffectsDone() {
                    mCallback.onEffectAvailable();
                }
            });
        }
    }

    /**
     * Initialize matrix display.
     *
     * @param previewWidth The width of preview buffer.
     * @param previewHeight The height of preview buffer.
     * @param effectNumOfPage The count of effect in one page.
     * @param format The preview buffer format.
     */
    public void initialize(int previewWidth, int previewHeight,
                           int effectNumOfPage, int format) {
        mMatrixEffect.initialize(previewWidth, previewHeight, effectNumOfPage, format);
    }

    /**
     * Set the surface.
     *
     * @param surface The surface object.
     * @param surfaceNumber The index of this surface in the surface array.
     */
    public void setSurface(Surface surface, int surfaceNumber) {
        mMatrixEffect.setSurface(surface, surfaceNumber);
    }

    /**
     * Set the size of effect buffer which draw to the surface.
     *
     * @param bufferWidth The width of effect buffer.
     * @param bufferHeight The height of effect buffer.
     * @param buffers The count of buffer block.
     */
    public void setBuffers(int bufferWidth, int bufferHeight, byte[][] buffers) {
        mMatrixEffect.setBuffers(bufferWidth, bufferHeight, buffers);
    }

    /**
     * Process the preview buffer to effect buffer.
     *
     * @param previewData The preview buffer.
     * @param effectId The ids of effects this preview buffer is processed.
     */
    public void process(byte[] previewData, int[] effectId) {
        mMatrixEffect.process(previewData, effectId);
    }

    /**
     * Release matrix display.
     */
    public void release() {
        mMatrixEffect.release();
    }
}
