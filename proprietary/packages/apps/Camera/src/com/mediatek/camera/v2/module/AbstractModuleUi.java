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

package com.mediatek.camera.v2.module;

import android.app.Activity;
import android.graphics.RectF;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.ViewGroup;

import com.android.camera.R;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.platform.module.ModuleUi;
import com.mediatek.camera.v2.platform.module.ModuleUi.GestureListener;
import com.mediatek.camera.v2.platform.module.ModuleUi.PreviewTouchedListener;
import com.mediatek.camera.v2.stream.IPreviewStream.PreviewCallback;
import com.mediatek.camera.v2.ui.CountDownView;
import com.mediatek.camera.v2.util.Utils;

import junit.framework.Assert;

import java.util.List;

public abstract class AbstractModuleUi implements ModuleUi {
    private static final Tag TAG = new Tag(AbstractModuleUi.class.getSimpleName());
    protected final Activity                     mActivity;
    protected final AbstractCameraModule         mModule;
    protected final ViewGroup                    mModuleRootView;
    protected final PreviewCallback              mPreviewCallback;

    private final ModuleGestureListener mModuleGestureListener
            = new ModuleGestureListener();
    private final ModulePreviewTouchedListener mModulePreviewTouchedListener
            = new ModulePreviewTouchedListener();

    private final CountDownView                  mCountdownView;
    private final static int RATIO_TO_MS = 1000;

    public AbstractModuleUi(Activity activity, AbstractCameraModule module,
            ViewGroup parentView, PreviewCallback callback) {
        Assert.assertNotNull(activity);
        Assert.assertNotNull(module);
        Assert.assertNotNull(parentView);
        Assert.assertNotNull(callback);

        mActivity         = activity;
        mModule           = module;
        mModuleRootView   = parentView;
        mPreviewCallback  = callback;

        activity.getLayoutInflater().inflate(R.layout.count_down_view, parentView, true);
        mCountdownView = (CountDownView) parentView.findViewById(R.id.count_down_view);
    }

    @Override
    public void onOrientationChanged(int orientation) {
        int orientationCompensation = (orientation +
                Utils.getDisplayRotation(mActivity)) % 360;
        Utils.setRotatableOrientation(mModuleRootView, orientationCompensation, true);
    }

    @Override
    public void onSurfaceAvailable(Surface surface, int width, int height) {
        mPreviewCallback.surfaceAvailable(surface, width, height);
    }

    @Override
    public boolean onSurfaceDestroyed(Surface surface) {
        mPreviewCallback.surfaceDestroyed(surface);
        return false;
    }

    /**
     * when activity rotation is changed will fired this.
     * @param isLandscape
     *            true means changed to landscape,otherwise is false.
     */
    public void onLayoutOrientationChanged(boolean isLandscape) {

    }

    @Override
    public void onSurfaceSizeChanged(Surface surface, int width, int height) {
        mPreviewCallback.surfaceSizeChanged(surface, width, height);
    }

    @Override
    public GestureListener getGestureListener() {
        return mModuleGestureListener;
    }

    @Override
    public PreviewTouchedListener getPreviewTouchedListener() {
        return mModulePreviewTouchedListener;
    }

    @Override
    public void onPreviewAreaChanged(RectF previewArea) {
        mCountdownView.onPreviewAreaChanged(previewArea);
    }

    @Override
    public void startCountdown(int sec) {
        mModule.mAppUi.showInfo(mActivity.getString(R.string.count_down_title_text), sec
                * RATIO_TO_MS);
        mCountdownView.startCountDown(sec);
    }

    @Override
    public void setCountdownFinishedListener(CountDownView.OnCountDownStatusListener listener) {
        mCountdownView.setCountDownStatusListener(listener);
    }

    @Override
    public boolean isCountingDown() {
        return mCountdownView.isCountingDown();
    }

    @Override
    public void cancelCountDown() {
        mCountdownView.cancelCountDown();
    }

    @Override
    public void updatePreviewCamera(List<String> cameraId, boolean isRemoteOpened) {

    }

    @Override
    public void updateRecordingStatus(boolean isStarted) {

    }

    /**
     * used for Cross Mount.
     * when mode is doing capture,need notify module,
     * current is capturing. when finished capture, also need update the status
     * @param isCapturing
     *            true means current is in capturing; otherwise is not
     *            capturing.
     */
    @Override
    public void updateCaptureStatus(boolean isCapturing) {

    }

    @Override
    public void updateFocusStatus(boolean isFocusing) {

    }

    @Override
    public void open(Activity activity, boolean isSecureCamera, boolean isCaptureIntent) {
        // TODO Auto-generated method stub

    }

    @Override
    public void resume() {
        // TODO Auto-generated method stub

    }

    @Override
    public void pause() {
        // TODO Auto-generated method stub

    }

    @Override
    public void close() {
        // TODO Auto-generated method stub

    }


    private class ModuleGestureListener implements GestureListener {

        @Override
        public boolean onDown(float x, float y) {
            return mModule.onDown(x, y);
        }

        @Override
        public boolean onUp() {
            return mModule.onUp();
        }

        @Override
        public boolean onScroll(float dx, float dy, float totalX, float totalY) {
            LogHelper.i(TAG, "onScroll (dx,dy)" + "(" + dx + "," + dy + ")" +
                    " totalX = " + totalX + " totalY = " + totalY);
            return mModule.onScroll(dx, dy, totalX, totalY);
        }

        @Override
        public boolean onSingleTapUp(float x, float y) {
            return mModule.onSingleTapUp(x, y);
        }

        @Override
        public boolean onSingleTapConfirmed(float x, float y) {
            return false;
        }

        @Override
        public boolean onDoubleTap(float x, float y) {
            return false;
        }

        @Override
        public boolean onScale(float focusX, float focusY, float scale) {
            return false;
        }

        @Override
        public boolean onScaleBegin(float focusX, float focusY) {
            return false;
        }

        @Override
        public boolean onLongPress(float x, float y) {
            return mModule.onLongPress(x, y);
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                float velocityY) {
            return false;
        }
    }

    private class ModulePreviewTouchedListener implements PreviewTouchedListener {
        @Override
        public boolean onPreviewTouched() {
            LogHelper.i(TAG, "onPreviewTouched");
            return false;
        }
    }
}
