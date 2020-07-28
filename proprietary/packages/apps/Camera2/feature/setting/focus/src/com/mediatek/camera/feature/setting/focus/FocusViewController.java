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
package com.mediatek.camera.feature.setting.focus;

import android.app.Activity;
import android.graphics.Rect;
import android.graphics.RectF;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.widget.FrameLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.widget.PreviewFrameLayout;
import com.mediatek.camera.common.widget.RotateLayout;
import com.mediatek.camera.feature.setting.focus.IFocusView.FocusViewState;
import com.mediatek.camera.feature.setting.focus.MultiZoneAfView.MultiWindow;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * The focus controller interacts with the focus UI.
 */
public class FocusViewController {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(FocusViewController.class.getSimpleName());
    private static final int TOUCH_FOCUS_VIEW_PRIORITY = 0;
    private static final int CONTINUOUS_FOCUS_VIEW_PRIORITY = 20;
    private static final int ORIENTATION_UNKNOWN = -1;
    private static final int RESET_MULTIAF_FOCUS = 0;
    private static final int COMMON_INFO_LENGTH = 12;
    private static final int AF_DATA_UNIT = 3;
    private static final int FOCUS_FRAME_DELAY = 1000;
    private FocusView mFocusView;
    private MultiZoneAfView mMultiZoneAfView;
    private MultiWindow[] mMultiAfWindows;
    private PreviewFrameLayout mFeatureRootView;
    private RotateLayout mExpandView;
    private Activity mActivity;
    private RectF mPreviewRect = new RectF();
    private Handler mHandler;
    private IApp mApp;

    /**
     * Constructor of focus view.
     *
     * @param app   The application app level controller
     * @param focus Flash instance.
     */
    public FocusViewController(final IApp app, final Focus focus) {
        mApp = app;
        mFeatureRootView = app.getAppUi().getPreviewFrameLayout();
        mActivity = app.getActivity();
        mHandler = new MainHandler(app.getActivity().getMainLooper());
        LogHelper.d(TAG, "[FocusViewController]");
    }

    /**
     * Show a passive focus animation at the center of the active area.
     */
    public void showPassiveFocusAtCenter() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mFocusView == null) {
                    return;
                }
                mFeatureRootView.unRegisterView(mFocusView);
                mFeatureRootView.registerView(mFocusView, CONTINUOUS_FOCUS_VIEW_PRIORITY);
                makeSureViewOnTree();
                setFocusLocation(mFeatureRootView.getWidth() / 2, mFeatureRootView.getHeight() /
                        2);
                if (hasMultiAFData(mMultiAfWindows)) {
                    handleMultiAfWindow(true);
                    mFocusView.setFocusState(FocusViewState.STATE_PASSIVE_FOCUSING);
                } else {
                    LogHelper.d(TAG, "[showPassiveFocusAtCenter]");
                    mFocusView.startPassiveFocus();
                    mFocusView.centerFocusLocation();
                }
            }
        });
    }

    /**
     * Show an active focus animation at the given viewX and viewY position.
     * This is normally initiated by the user touching the screen at a given
     * point.
     * <p>
     *
     * @param viewX the view's x coordinate
     * @param viewY the view's y coordinate
     */
    public void showActiveFocusAt(final int viewX, final int viewY) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mFocusView == null) {
                    return;
                }
                mFeatureRootView.unRegisterView(mFocusView);
                mFeatureRootView.registerView(mFocusView, TOUCH_FOCUS_VIEW_PRIORITY);
                LogHelper.d(TAG, "[showActiveFocusAt] +");
                makeSureViewOnTree();
                setFocusLocation(viewX, viewY);
                mFocusView.setFocusLocation(viewX, viewY);
                mFocusView.startActiveFocus();
            }
        });
    }

    /**
     * Stop any currently executing focus animation.
     */
    public void stopFocusAnimations() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mFocusView == null) {
                    return;
                }
                makeSureViewOnTree();
                if (mFocusView.isPassiveFocusRunning() && hasMultiAFData(mMultiAfWindows)) {
                    handleMultiAfWindow(false);
                }
                mFocusView.stopFocusAnimations();
            }
        });
    }

    /**
     * Set multi-zone af data to focus manager.
     * @param data The data of multi-zone af.
     */
    public void setAfData(byte[] data) {
        mMultiAfWindows = getMultiWindows(data);
    }
    protected void clearAfData(){
        mMultiAfWindows = null;
    }

    protected void clearFocusUi() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mMultiZoneAfView != null) {
                    mMultiZoneAfView.clear();
                }
                if (mFocusView == null) {
                    return;
                }
                makeSureViewOnTree();
                LogHelper.d(TAG, "clearFocusUi");
                mFocusView.clearFocusUi();
            }
        });
    }

    protected void highlightFocusView() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mFocusView == null) {
                    return;
                }
                makeSureViewOnTree();
                mFocusView.highlightFocusView();
            }
        });
    }


    protected void lowlightFocusView() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mFocusView == null) {
                    return;
                }
                makeSureViewOnTree();
                mFocusView.lowlightFocusView();
            }
        });
    }

    protected void addFocusView() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // focus view may inflate by other instance or exposure view.
                mFocusView = (FocusView) mFeatureRootView.findViewById(R.id.focus_view);
                mMultiZoneAfView = (MultiZoneAfView) mFeatureRootView.
                        findViewById(R.id.multi_focus_indicator);
                if (mFocusView == null) {
                    mFocusView = (FocusView) mActivity.getLayoutInflater().inflate(R.layout
                            .focus_view, mFeatureRootView, false);
                    mFeatureRootView.addView(mFocusView);
                    LogHelper.i(TAG, "[addFocusView] mFocusView = " + mFocusView);
                }
                mFocusView.setPreviewRect(mPreviewRect);
                addMultiZoneAfView();
                int orientation = mApp.getGSensorOrientation();
                if (orientation != ORIENTATION_UNKNOWN) {
                    int compensation = CameraUtil.getDisplayRotation(mActivity);
                    orientation = orientation + compensation;
                    CameraUtil.rotateViewOrientation(mFocusView, orientation, false);
                } else {
                    LogHelper.d(TAG, "[addFocusView] unknown orientation");
                }
                setFocusLocation(mFeatureRootView.getWidth() / 2, mFeatureRootView.getHeight() /
                        2);
            }
        });
    }

    protected void removeFocusView() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mFocusView != null) {
                    mFeatureRootView.unRegisterView(mFocusView);
                    mFeatureRootView.removeView(mFocusView);
                    LogHelper.d(TAG, "[removeFocusView]");
                    mFocusView = null;
                }
                removeMultiZoneAfView();
            }
        });
    }

    private void addMultiZoneAfView() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mMultiZoneAfView == null) {
                    mMultiZoneAfView = (MultiZoneAfView) mActivity.getLayoutInflater().inflate(
                            R.layout.multi_zone_af_view, mFeatureRootView, false);
                    mMultiZoneAfView.setPreviewSize((int) mPreviewRect.width(),
                            (int) mPreviewRect.height());
                    mFeatureRootView.addView(mMultiZoneAfView);
                    mFeatureRootView.registerView(mMultiZoneAfView, CONTINUOUS_FOCUS_VIEW_PRIORITY);
                    LogHelper.d(TAG, "[addMultiZoneAfView] mMultiZoneAfView = " +
                            mMultiZoneAfView);
                }
            }
        });
    }

    private void removeMultiZoneAfView() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mMultiZoneAfView != null) {
                    mFeatureRootView.unRegisterView(mMultiZoneAfView);
                    mFeatureRootView.removeView(mMultiZoneAfView);
                    mMultiZoneAfView = null;
                }
            }
        });
    }

    protected FocusViewState getFocusState() {
        if (mFocusView == null) {
            return FocusViewState.STATE_IDLE;
        }
        makeSureViewOnTree();
        return mFocusView.getFocusState();
    }

    protected void onPreviewChanged(RectF previewRect) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mPreviewRect = previewRect;
                if (mFocusView != null) {
                    mFocusView.setPreviewRect(previewRect);
                }
                if (mMultiZoneAfView != null) {
                    mMultiZoneAfView.setPreviewSize((int) previewRect.width(),
                            (int) previewRect.height());
                }
            }
        });
    }

    protected boolean isReadyTodoFocus() {
        if (mFocusView == null) {
            LogHelper.w(TAG, "[isReadyTodoFocus]mFocusView is null");
            return false;
        }
        if (mFocusView.getWidth() == 0 || mFocusView.getHeight() == 0) {
            LogHelper.w(TAG, "[isReadyTodoFocus]width or height is 0");
            return false;
        }
        return true;
    }

    protected boolean isActiveFocusRunning() {
        if (mFocusView == null) {
            return false;
        }
        return mFocusView.isActiveFocusRunning();
    }

    protected void setOrientation(final int orientation) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mFocusView == null) {
                    return;
                }
                int compensation = CameraUtil.getDisplayRotation(mActivity);
                final int compensationOrientation = orientation + compensation;
                CameraUtil.rotateViewOrientation(mFocusView, compensationOrientation, true);
            }
        });
    }

    private void setFocusLocation(int x, int y) {
        if (mFocusView == null) {
            return;
        }
        // Use margin to set the focus indicator to the touched area.
        FrameLayout.LayoutParams p = (FrameLayout.LayoutParams) mFocusView.getLayoutParams();
        int left = 0;
        int top = 0;
        left = x - mFocusView.getWidth() / 2;
        top = y - mFocusView.getHeight() / 2;
        p.setMargins(left, top, 0, 0);
        mFocusView.requestLayout();
    }

    /**
     * Make sure focus view be added to root view when it is not null.
     */
    private void makeSureViewOnTree() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // All focus and exposure are share one View
                if (mFeatureRootView.findViewById(R.id.focus_view) == null && mFocusView != null) {
                    LogHelper.w(TAG, "[makeSureViewOnTree] mFocusView is not on view tree");
                    mFeatureRootView.addView(mFocusView);
                }
            }
        });
    }

    private void makeSureMultiZoneAfViewOnTree() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mFeatureRootView.findViewById(R.id.multi_focus_indicator) == null
                        && mMultiZoneAfView != null) {
                    LogHelper.w(TAG, "[makeSureMultiZoneAfViewOnTree]" +
                            "mMultiZoneAfView is not on view tree");
                    mFeatureRootView.addView(mMultiZoneAfView);
                }
            }
        });
    }

    private boolean hasMultiAFData(MultiWindow[] windows) {
        boolean result = windows != null && windows.length > 0;
        LogHelper.d(TAG, "hasMultiAFData result = " + result);
        return result;
    }

    /**
     * The multi zone af handler.
     */
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "[handleMessage] msg.what = " + msg.what);
            switch (msg.what) {
                case RESET_MULTIAF_FOCUS:
                    if (mMultiZoneAfView != null) {
                        mMultiZoneAfView.clear();
                        mMultiAfWindows = null;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    private void handleMultiAfWindow(boolean moving) {
        if (mMultiZoneAfView == null) {
            return;
        }
        mMultiZoneAfView.setVisibility(View.VISIBLE);
        makeSureMultiZoneAfViewOnTree();
        int length = mMultiAfWindows.length;
        LogHelper.d(TAG, "[handleMultiAfWindow] length = " + length + ", moving = " + moving);
        if (moving) {
            // Do not care the result of the windows when focusing begin
            for (int i = 0; i < length; i++) {
                mMultiAfWindows[i].mResult = 0;
            }
            mMultiZoneAfView.updateFocusWindows(mMultiAfWindows);
            mMultiZoneAfView.showWindows(true);
        } else {
            // Only the focused window should be shown.
            List<MultiWindow> list = new ArrayList<MultiWindow>();
            for (int i = 0; i < length; i++) {
                if (mMultiAfWindows[i].mResult > 0) {
                    list.add(mMultiAfWindows[i]);
                }
            }
            MultiWindow[] tempWindows = new MultiWindow[list.size()];
            for (int i = 0; i < list.size(); i++) {
                tempWindows[i] = list.get(i);
            }
            mMultiZoneAfView.updateFocusWindows(tempWindows);
            mMultiZoneAfView.showWindows(false);
            mHandler.sendEmptyMessageDelayed(RESET_MULTIAF_FOCUS, FOCUS_FRAME_DELAY);
        }
    }

    /**
     * Get multi-zone Windows from the given data.
     *
     * @param data The data of the windows. There are three information of each window,
     * the x-coordinate and y-coordinate of the window's center which stands for the window
     * position and the result which stands for whether the zone is focused or not.
     * The data structure is as below:
     *      Common information(The first COMMON_INFO_LENGTH=12 bytes):
     *              data[0]~data[2]:the total number of multi zone;
     *              data[3]~data[5]:a single focus window width;
     *              data[6]~data[8]:a single focus window height;
     *      Window information(the other bytes):
     *              data[9]~data[11]:the x-coordinate of the first window's center;
     *              data[12]~data[14]:the y-coordinate of the first window's center;
     *              data[15]~data[17]:the result of the first window.
     *      The other window information are as so on.
     *
     * @return All the multi-zone windows information.
     */
    private MultiWindow[] getMultiWindows(byte[] data) {
        LogHelper.d(TAG, "[getMultiWindows] original data size " + data.length);
        IntBuffer intBuffer = ByteBuffer.wrap(data).order(ByteOrder.nativeOrder()).asIntBuffer();
        if (intBuffer.limit() / AF_DATA_UNIT < 1) {
            LogHelper.w(TAG, "[getMultiWindows] intBuffer.limit() = " + intBuffer.limit() +
                    "the AF original data from framework is wrong.");
            return null;
        }
        // Get common information of af window
        int windowCount = intBuffer.get(0);
        int windowWidth = intBuffer.get(1);
        int windowHeight = intBuffer.get(2);
        // Get the windows buffer(git rid of common information from total data )
        IntBuffer windowBuffer = ByteBuffer
                .wrap(data, COMMON_INFO_LENGTH, data.length - COMMON_INFO_LENGTH)
                .order(ByteOrder.nativeOrder()).asIntBuffer();
        int limit = windowBuffer.limit();
        LogHelper.d(TAG, "[getMultiWindows] windowCount " + windowCount
                + " ,single window (width,height ) from native " + "(" + windowWidth + " ,"
                + windowHeight + ")");
        if (limit != windowCount * AF_DATA_UNIT) {
            LogHelper.w(TAG, "[getMultiWindows] limit = " + limit +
                    ", the window data number is not consistent with the common info");
        }
        // Get each window information
        MultiWindow[] windows = new MultiWindow[windowCount];
        for (int i = 0; i < limit; i = 3 + i) {
            // Step1:Get the original window information
            Rect position = new Rect();
            int x = windowBuffer.get(i);
            int y = windowBuffer.get(i + 1);
            int result = windowBuffer.get(i + 2);
            // Step2:Calculate the bounds of the window
            position.left = x - windowWidth / 2;
            position.top = y - windowHeight / 2;
            position.right = x + windowWidth / 2;
            position.bottom = y + windowHeight / 2;
            // Step3:Save the window information to a MultiWindow
            MultiWindow tempWindow = new MultiWindow(position, result);
            windows[i / 3] = tempWindow;
        }
        return windows;
    }
}
