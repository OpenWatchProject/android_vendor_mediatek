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
package com.mediatek.camera.feature.setting.dualcamerazoom;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AlphaAnimation;
import android.widget.RelativeLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.widget.RotateLayout;

import java.util.List;
import java.util.Locale;

/**
 * The zoom view.
 */
public class DualZoomViewCtrl {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(DualZoomViewCtrl.class.getSimpleName());
    private static final int BASE_RATIO = 10;
    private static final int CLOSE_BAR_MARGIN_BOTTOM = 130;
    private static final int EXPAND_BAR_MARGIN_BOTTOM = 195;
    private static final float MAX_ALPHA = 1.0f;
    private static final float MIN_ALPHA = 0.0f;
    private static final int ANIMATION_DURATION = 400;
    private ViewGroup mRootViewGroup;
    private RelativeLayout mZoomView;
    private IApp mApp;
    private ObservableScrollView mScrollerView;
    private ZoomTextView mExtText;
    private RotateLayout mExtSwitchLayout;
    private MainHandler mMainHandler;
    private OnSeekBarClickListener mSeekBarClickListener;
    private ObservableScrollView.ScrollViewListener mOnScrollListener;
    private List<Integer> mSupportedRatios;
    private boolean mIsBarExpand;
    private boolean mIsBarTouched;
    private float mCurrentRatio;
    private int mCurrentX = -1;
    private int mScrollDealy = 100;
    private String mSwitchRatio = "2X";
    private float mMaxZoom = IDualZoomConfig.ZOOM_MAX_VALUE;
    private float mMinZoom = IDualZoomConfig.ZOOM_MIN_VALUE;

    //Gesture and View Control
    private static final int ZOOM_VIEW_HIDE_DELAY_TIME = 3000;
    private static final int ZOOM_VIEW_SHOW = 0;
    private static final int ZOOM_VIEW_RESET = 1;
    private static final int ZOOM_VIEW_INIT = 2;
    private static final int ZOOM_VIEW_UNINIT = 3;
    private static final int ZOOM_VIEW_ORIENTATION_CHANGED = 4;
    private static final int ZOOM_VIEW_HIDE = 5;
    private static final int ZOOM_BAR_CLOSE = 6;
    private static final int SCROLLER_VIEW_RESUME = 8;

    /**
     * The interface for switch icon click listener.
     */
    public interface OnSeekBarClickListener {
        /**
         * The switch icon touch click.
         */
        void onSingleTap();
    }

    /**
     * Init the app
     * @param app    the activity.
     */
    public void init(IApp app) {
        mApp = app;
    }

    /**
     * Init the view.
     */
    public void config() {
        if (mMainHandler == null) {
            mMainHandler = new MainHandler(mApp.getActivity().getMainLooper());
        }
        mMainHandler.obtainMessage(ZOOM_VIEW_INIT).sendToTarget();
    }

    /**
     * To destroy the zoom view.
     */
    public void unInit() {
        if (mMainHandler != null) {
            mMainHandler.sendEmptyMessage(ZOOM_VIEW_RESET);
            mMainHandler.sendEmptyMessage(ZOOM_VIEW_UNINIT);
        }
    }

    /**
     * Used to show ratio test when zooming.
     *
     * @param msg the ratio test.
     */
    public void showView(float msg) {
        if (mMainHandler != null) {
            mMainHandler.obtainMessage(ZOOM_VIEW_SHOW, msg).sendToTarget();
        }
    }

    /**
     * Used to hide ratio test when zooming.
     */
    public void hideView() {
        if (mMainHandler != null) {
            mMainHandler.sendEmptyMessage(ZOOM_VIEW_HIDE);
        }
    }

    /**
     * reset camera view after zoom done.
     */
    public void resetView() {
        if (mMainHandler != null) {
            mIsBarTouched = false;
            mMainHandler.removeMessages(ZOOM_BAR_CLOSE);
            mMainHandler.sendEmptyMessageDelayed(ZOOM_BAR_CLOSE, ZOOM_VIEW_HIDE_DELAY_TIME);
            mMainHandler.sendEmptyMessageDelayed(ZOOM_VIEW_RESET, ZOOM_VIEW_HIDE_DELAY_TIME);
        }
    }

    /**
     * close camera view after zoom done.
     */
    public void closeZoomView() {
        if (mMainHandler != null) {
            mIsBarTouched = false;
            mMainHandler.removeMessages(ZOOM_BAR_CLOSE);
            mMainHandler.sendEmptyMessage(ZOOM_BAR_CLOSE);
            mMainHandler.sendEmptyMessage(ZOOM_VIEW_RESET);
        }
    }

    /**
     * resume zoom view after restriction done.
     */
    public void resumeZoomView() {
        if (mMainHandler != null) {
            mIsBarTouched = false;
            mMainHandler.removeMessages(ZOOM_BAR_CLOSE);
            mMainHandler.sendEmptyMessage(SCROLLER_VIEW_RESUME);
            mMainHandler.sendEmptyMessageDelayed(ZOOM_BAR_CLOSE, ZOOM_VIEW_HIDE_DELAY_TIME);
            mMainHandler.sendEmptyMessageDelayed(ZOOM_VIEW_RESET, ZOOM_VIEW_HIDE_DELAY_TIME);
        }
    }

    /**
     * resume scroller view after restriction done.
     */
    private void resumeScrollerView() {
        if (mScrollerView != null && mScrollerView.getVisibility() != View.VISIBLE &&
                mExtSwitchLayout != null) {
            RelativeLayout.LayoutParams layoutParams
                    = (RelativeLayout.LayoutParams) mExtSwitchLayout.getLayoutParams();
            int expandBottomMargin = (int) dip2Px(mApp.getActivity(), EXPAND_BAR_MARGIN_BOTTOM);
            if (expandBottomMargin == layoutParams.bottomMargin) {
                mScrollerView.setVisibility(View.VISIBLE);
            }
        }
    }

    /**
     * when phone orientation changed, the zoom view will be updated.
     *
     * @param orientation the orientation of g-sensor.
     */
    public void onOrientationChanged(int orientation) {
        updateOrientation(orientation);
    }

    /**
     * clear the invalid view such as indicator view.
     *
     * @param enableBar true when need enable bar.
     */
    public void clearInvalidView(boolean enableBar) {
        if (mMainHandler != null) {
            mMainHandler.removeMessages(ZOOM_VIEW_RESET);
            mMainHandler.removeMessages(ZOOM_VIEW_SHOW);
        }
        if (mMainHandler != null && enableBar) {
            mIsBarTouched = true;
            mMainHandler.removeMessages(ZOOM_BAR_CLOSE);
        }
        mApp.getAppUi().setUIVisibility(IAppUi.INDICATOR, View.INVISIBLE);
    }

    /**
     * Set seek bar click listener.
     *
     * @param l the listener of switch sensor.
     */
    public void setSeekBarClickListener(OnSeekBarClickListener l) {
        mSeekBarClickListener = l;
    }

    /**
     * Set seek bar click listener.
     *
     * @param l the listener of switch sensor.
     */
    public void setOnScrollListener(ObservableScrollView.ScrollViewListener l) {
        mOnScrollListener = l;
    }

    /**
     * Set switch ratio
     * @param switchRatio
     */
    public void setSwitchRatio(int switchRatio) {
        mSwitchRatio = String.format(Locale.ENGLISH, IDualZoomConfig.PATTERN,
                Math.floor(switchRatio / BASE_RATIO) / BASE_RATIO) + "X";
    }

    /**
     * Set max zoom for slowmotion
     * @param maxZoom
     */
    public void setMaxZoom(float maxZoom) {
        mMaxZoom = maxZoom;
        LogHelper.d(TAG, "[setMaxZoom] mMaxZoom " + mMaxZoom);
    }

    /**
     * Set min zoom
     *
     * @param minZoom
     */
    public void setMinZoom(float minZoom) {
        mMinZoom = minZoom;
        LogHelper.d(TAG, "[setMinZoom] mMinZoom " + mMinZoom);
    }

    /**
     * Update scroller view position by current level.
     */
    public void showScrollerPosition() {
        if (mMainHandler == null) {
            return;
        }
        mMainHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (mScrollerView.getScrollX() != getCurProcess(mCurrentRatio)) {
                    mScrollerView.scrollTo(getCurProcess(mCurrentRatio), 0);
                    mScrollerView.invalidate();
                }
            }
        }, mScrollDealy);
    }

    /**
     * Handler let some task execute in main thread.
     */
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case ZOOM_VIEW_SHOW:
                    show((Float) msg.obj);
                    break;
                case ZOOM_VIEW_HIDE:
                    hide();
                    break;
                case ZOOM_VIEW_RESET:
                    reset();
                    break;
                case ZOOM_VIEW_INIT:
                    initView();
                    break;
                case ZOOM_VIEW_UNINIT:
                    unInitView();
                    break;
                case ZOOM_VIEW_ORIENTATION_CHANGED:
                    updateOrientation((Integer) msg.obj);
                    break;
                case ZOOM_BAR_CLOSE:
                    closeZoomBar();
                    break;
                case SCROLLER_VIEW_RESUME:
                    resumeScrollerView();
                    break;
                default:
                    break;
            }
        }
    }

    private void show(float ratio) {
        mCurrentRatio = ratio;
        if (mExtText != null) {
            mExtText.setVisibility(View.VISIBLE);
            mExtText.setText(getPatternRatio(ratio));
        }
    }

    private void hide() {
        if (mScrollerView != null && mScrollerView.getVisibility() == View.VISIBLE) {
            mScrollerView.setVisibility(View.INVISIBLE);
        }
        if (mExtText != null && mExtText.getVisibility() == View.VISIBLE) {
            mExtText.setVisibility(View.INVISIBLE);
        }
        if (mExtSwitchLayout != null && mExtSwitchLayout.getVisibility() == View.VISIBLE) {
            mExtSwitchLayout.setVisibility(View.INVISIBLE);
        }
    }

    private void reset() {
        mApp.getAppUi().setUIVisibility(IAppUi.INDICATOR, View.VISIBLE);
    }

    private void initView() {
        if (mRootViewGroup == null || mZoomView == null) {
            mRootViewGroup = mApp.getAppUi().getModeRootView();
            mZoomView = (RelativeLayout) mApp.getActivity().getLayoutInflater()
                    .inflate(R.layout.ext_zoom_bar,
                            mRootViewGroup, false).findViewById(R.id.ext_dual_camera_root);
            mExtSwitchLayout = (RotateLayout) mZoomView.findViewById(R.id.zoom_rotate_layout);
            mExtText = (ZoomTextView) mZoomView.findViewById(R.id.ext_zoom_text_view);
            mScrollerView = (ObservableScrollView) mZoomView.findViewById(R.id.scroller);
            mScrollerView.setScrollViewListener(new ObservableScrollView.ScrollViewListener() {
                @Override
                public void onScrollChanged(ObservableScrollView scrollView, int x, int y,
                                            int oldx, int oldy) {
                    if (mOnScrollListener != null && scrollView != null) {
                        mOnScrollListener.onScrollChanged(scrollView, x, y, oldx, oldy);
                    }
                }
            });
            mScrollerView.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View view, MotionEvent motionEvent) {
                    if (mMainHandler == null) {
                        return false;
                    }
                    switch (motionEvent.getAction()) {
                        case MotionEvent.ACTION_MOVE:
                            mMainHandler.removeCallbacks(mScrollRunnable);
                            mMainHandler.removeMessages(ZOOM_BAR_CLOSE);
                            break;
                        case MotionEvent.ACTION_UP:
                            mMainHandler.post(mScrollRunnable);
                            break;
                    }
                    return false;
                }
            });

            mExtSwitchLayout.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View view, MotionEvent motionEvent) {
                    mScrollerView.dispatchTouchEvent(motionEvent);
                    return false;
                }
            });

            if (mOnScrollListener != null) {
                mExtSwitchLayout.setOnLongClickListener(mLongClickListener);
            }
            mExtSwitchLayout.setOnClickListener(mClickListener);
            mRootViewGroup.addView(mZoomView);
        }
        if (mIsBarExpand) {
            closeZoomBar();
        }
    }

    private void unInitView() {
        if (mRootViewGroup != null && mZoomView != null) {
            mRootViewGroup.removeView(mZoomView);
            mMainHandler = null;
            mZoomView = null;
        }
    }

    private void updateOrientation(int orientation) {
        if (mExtText != null) {
            mExtText.setDegrees(orientation);
            mExtText.invalidate();
        }
    }

    private View.OnLongClickListener mLongClickListener = new View.OnLongClickListener() {
        @Override
        public boolean onLongClick(View view) {
            if (mIsBarExpand) {
                closeZoomBar();
            } else {
                expandZoomBar();
                mMainHandler.sendEmptyMessageDelayed(ZOOM_BAR_CLOSE, ZOOM_VIEW_HIDE_DELAY_TIME);
            }
            return true;
        }
    };

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            mSeekBarClickListener.onSingleTap();
        }
    };

    private void expandZoomBar() {
        if (mMainHandler != null) {
            mMainHandler.removeMessages(ZOOM_BAR_CLOSE);
        }
        AlphaAnimation alphaAnimation = new AlphaAnimation(MIN_ALPHA, MAX_ALPHA);
        alphaAnimation.setDuration(ANIMATION_DURATION);
        if (mExtSwitchLayout != null) {
            mExtSwitchLayout.startAnimation(alphaAnimation);
            RelativeLayout.LayoutParams layoutParams
                    = (RelativeLayout.LayoutParams) mExtSwitchLayout.getLayoutParams();
            layoutParams.bottomMargin = (int) dip2Px(mApp.getActivity(), EXPAND_BAR_MARGIN_BOTTOM);
            mExtSwitchLayout.setLayoutParams(layoutParams);
        }
        if (mScrollerView != null && mScrollerView.getVisibility() == View.GONE) {
            mScrollerView.startAnimation(alphaAnimation);
            mScrollerView.setVisibility(View.VISIBLE);
            showScrollerPosition();
        }
        mIsBarExpand = true;
    }

    private void closeZoomBar() {
        if (mMainHandler != null) {
            mMainHandler.removeMessages(ZOOM_BAR_CLOSE);
        }
        if (mScrollerView != null && mScrollerView.getVisibility() == View.VISIBLE) {
            AlphaAnimation hideAnimation = new AlphaAnimation(MAX_ALPHA, MIN_ALPHA);
            hideAnimation.setDuration(ANIMATION_DURATION);
            mScrollerView.startAnimation(hideAnimation);
            mScrollerView.setVisibility(View.GONE);
            if (mExtSwitchLayout != null) {
                AlphaAnimation showAnimation = new AlphaAnimation(MIN_ALPHA, MAX_ALPHA);
                showAnimation.setDuration(ANIMATION_DURATION);
                mExtSwitchLayout.startAnimation(showAnimation);
                RelativeLayout.LayoutParams layoutParams
                        = (RelativeLayout.LayoutParams) mExtSwitchLayout.getLayoutParams();
                layoutParams.bottomMargin =
                        (int) dip2Px(mApp.getActivity(), CLOSE_BAR_MARGIN_BOTTOM);
                mExtSwitchLayout.setLayoutParams(layoutParams);
            }
        }
        mIsBarExpand = false;
    }

    private static float dip2Px(Context context, float dpvalue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return dpvalue * scale;
    }

    private int getCurProcess(float ratio) {
        int process = 0;
        process = (int) ((ratio - mMinZoom) *
                mScrollerView.getWidthMax() / (mMaxZoom - mMinZoom));
        return process;
    }

    private String getPatternRatio(float ratio) {
        String zoomRatio = "";
        zoomRatio = String.format(Locale.ENGLISH, IDualZoomConfig.PATTERN, ratio) + "X";
        if (zoomRatio.contains(".0X")) {
            return zoomRatio.replace(".0X", "X");
        }
        return zoomRatio;
    }

    private Runnable mScrollRunnable = new Runnable() {
        @Override
        public void run() {
            if (mMainHandler == null) {
                return;
            }
            if (mCurrentX == mScrollerView.getScrollX()) {
                LogHelper.d(TAG, "[scrollRunnable.run]");
                mMainHandler.removeCallbacks(this);
                resetView();
                return;
            }
            mCurrentX = mScrollerView.getScrollX();
            mMainHandler.postDelayed(this, mScrollDealy);
        }
    };
}
