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


import android.graphics.RectF;
import android.view.MotionEvent;
import android.view.View;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.IAppUiListener.ISurfaceStatusListener;
import com.mediatek.camera.common.IAppUiListener.OnPreviewAreaChangedListener;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.common.widget.PreviewFrameLayout;
import com.mediatek.camera.portability.SystemProperties;

import java.util.concurrent.CopyOnWriteArrayList;

/**
 * A manager for camera preview module, now it support surfaceview or textureview.
 */
public class PreviewManager {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(PreviewManager.class.getSimpleName());
    private static final int DEFAULT_SURFACEVIEW_VALUE = 0;
    private static final int SURFACEVIEW_ENABLED_VALUE = 1;
    private static final int DEFAULT_APP_VERSION = 2;
    private int mPreviewWidth = 0;
    private int mPreviewHeight = 0;
    private IApp mApp;
    private RectF mPreviewArea = new RectF();

    private final CopyOnWriteArrayList<OnPreviewAreaChangedListener>
                                    mPreviewAreaChangedListeners;

    private View.OnTouchListener mOnTouchListener;
    private PreviewFrameLayout mPreviewFrameLayout;

    private IController mPreviewController;

    /**
     * constructor of PreviewManager.
     * @param app The {@link IApp} implementer.
     */
    public PreviewManager(IApp app) {
        mApp = app;
        mPreviewAreaChangedListeners = new CopyOnWriteArrayList<>();
        mPreviewFrameLayout =
                (PreviewFrameLayout) mApp.getActivity().findViewById(R.id.preview_layout_container);

        int enabledValue = SystemProperties.getInt("vendor.debug.surface.enabled",
                DEFAULT_SURFACEVIEW_VALUE);

        int appVersion = SystemProperties.getInt("ro.vendor.mtk_camera_app_version",
            DEFAULT_APP_VERSION);

        LogHelper.i(TAG, "enabledValue = " + enabledValue + " appVersion " + appVersion);

        if (enabledValue == SURFACEVIEW_ENABLED_VALUE || appVersion == DEFAULT_APP_VERSION) {
            mPreviewController = new SurfaceViewController(app);
        } else {
            mPreviewController = new TextureViewController(app);
        }

        mPreviewController.setOnLayoutChangeListener(mOnLayoutChangeCallback);
        mPreviewController.setOnTouchListener(mOnTouchListenerImpl);
    }

    /**
     * update preview size.
     * This function should be called on the UI thread and listeners will be
     * notified on the UI thread.
     * @param width preview width, must > 0.
     * @param height preview height, must > 0.
     * @param listener Set preview status listener. The new listener will replace the old one.
     */
    public void updatePreviewSize(int width, int height, ISurfaceStatusListener listener) {
        LogHelper.i(TAG, "updatePreviewSize: new size (" + width + " , " + height + " )"
                             + " current size (" + mPreviewWidth + " , " + mPreviewHeight + " )");
        mPreviewWidth = width;
        mPreviewHeight = height;
        if (mPreviewController != null) {
            mPreviewController.updatePreviewSize(width, height, listener);
        }
    }

    /**
     * Clear the Status previous listener that set when update preview size.
     * @param listener Listener set to previewManager previously.
     */
    public void clearPreviewStatusListener(ISurfaceStatusListener listener) {
        mPreviewController.clearPreviewStatusListener(listener);
    }

    /**
     * Adds a listener that will get notified when the preview area changed. This
     * can be useful for UI elements or focus view to adjust themselves according
     * to the preview area change.
     * <p/>
     * Note that a listener will only be added once. A newly added listener will receive
     * a notification of current preview area immediately after being added.
     * <p/>
     * This function should be called on the UI thread and listeners will be notified
     * on the UI thread.
     *
     * @param listener the listener that will get notified of preview area change
     */
    public void registerPreviewAreaChangedListener(
            IAppUiListener.OnPreviewAreaChangedListener listener) {
        if (listener != null && !mPreviewAreaChangedListeners.contains(listener)) {
            mPreviewAreaChangedListeners.add(listener);
            if (mPreviewArea.width() != 0 || mPreviewArea.height() != 0) {
                listener.onPreviewAreaChanged(mPreviewArea,
                        new Size(mPreviewWidth, mPreviewHeight));
            }
        }
    }

    /**
     * Removes a listener that gets notified when the preview area changed.
     *
     * @param listener the listener that gets notified of preview area change
     */
    public void unregisterPreviewAreaChangedListener(
            OnPreviewAreaChangedListener listener) {
        if (listener != null && mPreviewAreaChangedListeners.contains(listener)) {
            mPreviewAreaChangedListeners.remove(listener);
        }
    }

    /**
     * Set preview module touch event enabled or not.
     * @param enabled True enabled, otherwise disabled.
     */
    public void setEnabled(final boolean enabled) {
        mPreviewController.setEnabled(enabled);
    }

    /**
     * Get current preview module view.
     * @return Preview view instance.
     */
    public View getView() {
        return mPreviewController.getView();
    }

    /**
     * When activity do pause, notify it to preview module.
     */
    public void onPause() {
        mPreviewController.onPause();
    }

    public PreviewFrameLayout getPreviewFrameLayout() {
        return mPreviewFrameLayout;
    }

    /**
     * Remove the Top preview view.
     * when switch preview size between different ratio, a new preview view will create for coming
     * preview and current preview view will bring to the top as a cover.
     * when new preview is becoming update, remove the top view.
     */
    public  void removeTopSurface() {
        if (mPreviewController != null) {
            mPreviewController.removeTopSurface();
        }
    }

    public void setOnTouchListener(View.OnTouchListener listener) {
        mOnTouchListener = listener;
    }

    private View.OnLayoutChangeListener mOnLayoutChangeCallback
                                          = new View.OnLayoutChangeListener() {
        @Override
        public void onLayoutChange(View v, int left, int top, int right, int bottom, int oldLeft,
                                   int oldTop, int oldRight, int oldBottom) {
            mPreviewArea.set(left, top, right, bottom);
            // This method can be called during layout pass. We post a Runnable so
            // that the callbacks won't happen during the layout pass.
            mPreviewFrameLayout.post(new Runnable() {
                @Override
                public void run() {
                    notifyPreviewAreaChanged();
                    mPreviewFrameLayout.setPreviewSize((int) mPreviewArea.width(),
                            (int) mPreviewArea.height());
                }
            });
        }
    };

    private View.OnTouchListener mOnTouchListenerImpl = new View.OnTouchListener() {

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (mOnTouchListener != null) {
                return mOnTouchListener.onTouch(v, event);
            } else {
                return false;
            }
        }
    };

    private void notifyPreviewAreaChanged() {
        for (OnPreviewAreaChangedListener listener :
                mPreviewAreaChangedListeners) {
            listener.onPreviewAreaChanged(mPreviewArea, new Size(mPreviewWidth, mPreviewHeight));
        }
    }
}
