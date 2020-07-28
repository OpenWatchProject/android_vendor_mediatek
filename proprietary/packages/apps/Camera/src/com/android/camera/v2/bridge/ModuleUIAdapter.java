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

package com.android.camera.v2.bridge;

import junit.framework.Assert;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;

import com.android.camera.v2.ui.PreviewStatusListener;
import com.mediatek.camera.v2.platform.module.ModuleUi;
import com.mediatek.camera.v2.platform.module.ModuleUi.GestureListener;
import com.mediatek.camera.v2.platform.module.ModuleUi.PreviewTouchedListener;

/**
 *  This adapter is used to adapt preview status change from App level to Module level
 */
public class ModuleUIAdapter implements PreviewStatusListener {

    private final ModuleUi                 mModuleUi;
    private AppGestureListener             mAppGestureListener;
    private GestureListener                mModuleGestureListener;
    private AppOnPreviewTouchedListener    mAppPreviewTouchedListener;
    private PreviewTouchedListener         mModulePreviewTouchedListener;

    public ModuleUIAdapter(ModuleUi moduleUi) {
        Assert.assertNotNull(moduleUi);
        mModuleUi = moduleUi;
    }

    @Override
    public void surfaceAvailable(Surface surface, int width, int height) {
        mModuleUi.onSurfaceAvailable(surface, width, height);
    }

    @Override
    public void surfaceDestroyed(Surface surface) {
        mModuleUi.onSurfaceDestroyed(surface);
    }

    @Override
    public void surfaceSizeChanged(Surface surface, int width, int height) {
        mModuleUi.onSurfaceSizeChanged(surface, width, height);
    }

    @Override
    public OnGestureListener getGestureListener() {
        if (mAppGestureListener == null) {
            mAppGestureListener = new AppGestureListener();
        }
        mModuleGestureListener = mModuleUi.getGestureListener();
        return mAppGestureListener;
    }

    @Override
    public OnPreviewTouchedListener getTouchListener() {
        if (mAppPreviewTouchedListener == null) {
            mAppPreviewTouchedListener = new AppOnPreviewTouchedListener();
        }
        mModulePreviewTouchedListener = mModuleUi.getPreviewTouchedListener();
        return mAppPreviewTouchedListener;
    }

    @Override
    public void onPreviewLayoutChanged(View v, int left, int top, int right,
            int bottom, int oldLeft, int oldTop, int oldRight, int oldBottom) {

    }

    private class AppGestureListener implements OnGestureListener {

        @Override
        public boolean onDown(float x, float y) {
            return mModuleGestureListener.onDown(x, y);
        }

        @Override
        public boolean onUp() {
            return mModuleGestureListener.onUp();
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                float velocityY) {
            return mModuleGestureListener.onFling(e1, e2, velocityX, velocityY);
        }

        @Override
        public boolean onScroll(float dx, float dy, float totalX, float totalY) {
            return mModuleGestureListener.onScroll(dx, dy, totalX, totalY);
        }

        @Override
        public boolean onSingleTapUp(float x, float y) {
            return mModuleGestureListener.onSingleTapUp(x, y);
        }

        @Override
        public boolean onSingleTapConfirmed(float x, float y) {
            return mModuleGestureListener.onSingleTapConfirmed(x, y);
        }

        @Override
        public boolean onDoubleTap(float x, float y) {
            return mModuleGestureListener.onDoubleTap(x, y);
        }

        @Override
        public boolean onScale(float focusX, float focusY, float scale) {
            return mModuleGestureListener.onScale(focusX, focusY, scale);
        }

        @Override
        public boolean onScaleBegin(float focusX, float focusY) {
            return mModuleGestureListener.onScaleBegin(focusX, focusY);
        }

        @Override
        public boolean onLongPress(float x, float y) {
            return mModuleGestureListener.onLongPress(x, y);
        }
    }

    public ModuleUi getModuleUi() {
        return mModuleUi;
    }

    private class AppOnPreviewTouchedListener implements OnPreviewTouchedListener {
        @Override
        public boolean onPreviewTouched() {
            if (mModulePreviewTouchedListener != null) {
                return mModulePreviewTouchedListener.onPreviewTouched();
            }
            return false;
        }
    }
}
