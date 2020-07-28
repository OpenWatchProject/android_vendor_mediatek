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
package com.android.camera.manager;

import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SlidingDrawer;
import android.widget.SlidingDrawer.OnDrawerCloseListener;
import android.widget.SlidingDrawer.OnDrawerOpenListener;

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.R;
import com.android.camera.Util;
import com.android.camera.ui.RotateImageView;

public class PIPViewManager extends ViewManager {
    private static final String TAG = "PIPViewManager";

    private Animation mFadeIn;
    private Animation mFadeOut;

    private SlidingDrawer mSlidingDrawer;
    private ImageView mIndicator;
    private boolean mIsShowingPipSetting = false;
    private Listener mListener;
    private int mDisplayWidth;
    private int mDisplayHeight;
    private float mDensity;
    private ViewGroup mPipSettingLayout;
    private int mModeWidth;
    private int mCurrentOrientation = -1;
    private int mOrientation;
    private static final int MSG_REMOVE_VIEW = 0;

    private static final int MODE_DEFAULT_WIDTH = 100;
    private static final int PIP_VIEW_NUM = 8;
    private static final int DEFAULT_VIEW_SHOW_NUM = 5;

    public static final int CUBISM = 0;
    public static final int FISHEYE = 1;
    public static final int HEART = 2;
    public static final int INSTANTPHOTO = 3;
    public static final int OVALBLUR = 4;
    public static final int POSTCARD = 5;
    public static final int SPLIT = 6;
    public static final int WINDOW = 7;
    private int mCurrentEffect = CUBISM;

    private static int[] mImageView = new int[PIP_VIEW_NUM];
    private static int[] mImageViewFocus = new int[PIP_VIEW_NUM];
    private static int[] mImageViewId = new int[PIP_VIEW_NUM];
    private static int[][] mPIPFrontView = new int[PIP_VIEW_NUM][];
    private static int[] mItemLayoutId = new int[PIP_VIEW_NUM];
    private final RotateImageView[] mModeViews = new RotateImageView[PIP_VIEW_NUM];
    private final LinearLayout[] mItemLayouts = new LinearLayout[PIP_VIEW_NUM];
    static {
        mImageView[CUBISM] = R.drawable.effect_01;
        mImageView[FISHEYE] = R.drawable.effect_02;
        mImageView[HEART] = R.drawable.effect_03;
        mImageView[INSTANTPHOTO] = R.drawable.effect_04;
        mImageView[OVALBLUR] = R.drawable.effect_05;
        mImageView[POSTCARD] = R.drawable.effect_06;
        mImageView[SPLIT] = R.drawable.effect_07;
        mImageView[WINDOW] = R.drawable.effect_08;
    };

    static {
        mImageViewFocus[CUBISM] = R.drawable.effect_01_focus;
        mImageViewFocus[FISHEYE] = R.drawable.effect_02_focus;
        mImageViewFocus[HEART] = R.drawable.effect_03_focus;
        mImageViewFocus[INSTANTPHOTO] = R.drawable.effect_04_focus;
        mImageViewFocus[OVALBLUR] = R.drawable.effect_05_focus;
        mImageViewFocus[POSTCARD] = R.drawable.effect_06_focus;
        mImageViewFocus[SPLIT] = R.drawable.effect_07_focus;
        mImageViewFocus[WINDOW] = R.drawable.effect_08_focus;
    };
    static {
        mImageViewId[CUBISM] = R.id.pip_cubism;
        mImageViewId[FISHEYE] = R.id.pip_fisheye;
        mImageViewId[HEART] = R.id.pip_heart;
        mImageViewId[INSTANTPHOTO] = R.id.pip_instantphoto;
        mImageViewId[OVALBLUR] = R.id.pip_ovalblur;
        mImageViewId[POSTCARD] = R.id.pip_postcard;
        mImageViewId[SPLIT] = R.id.pip_split;
        mImageViewId[WINDOW] = R.id.pip_window;
    };

    static {
        mItemLayoutId[CUBISM] = R.id.item_layout1;
        mItemLayoutId[FISHEYE] = R.id.item_layout2;
        mItemLayoutId[HEART] = R.id.item_layout3;
        mItemLayoutId[INSTANTPHOTO] = R.id.item_layout4;
        mItemLayoutId[OVALBLUR] = R.id.item_layout5;
        mItemLayoutId[POSTCARD] = R.id.item_layout6;
        mItemLayoutId[SPLIT] = R.id.item_layout7;
        mItemLayoutId[WINDOW] = R.id.item_layout8;
    };
    static {
        mPIPFrontView[CUBISM] = new int[] { R.drawable.rear_01, R.drawable.front_01,
                R.drawable.front_01_focus };
        mPIPFrontView[FISHEYE] = new int[] { R.drawable.rear_02, R.drawable.front_02,
                R.drawable.front_02_focus };
        mPIPFrontView[HEART] = new int[] { R.drawable.rear_03, R.drawable.front_03,
                R.drawable.front_03_focus };
        mPIPFrontView[INSTANTPHOTO] = new int[] { R.drawable.rear_04, R.drawable.front_04,
                R.drawable.front_04_focus };
        mPIPFrontView[OVALBLUR] = new int[] { R.drawable.rear_05, R.drawable.front_05,
                R.drawable.front_05_focus };
        mPIPFrontView[POSTCARD] = new int[] { R.drawable.rear_06, R.drawable.front_06,
                R.drawable.front_06_focus };
        mPIPFrontView[SPLIT] = new int[] { R.drawable.rear_07, R.drawable.front_07,
                R.drawable.front_07_focus };
        mPIPFrontView[WINDOW] = new int[] { R.drawable.rear_08, R.drawable.front_08,
                R.drawable.front_08_focus };
    };

    public interface Listener {
        public void onUpdateEffect(int effectRearIndex, int effectFrontIndex,
                int effectFrontHighlight);
    }

    public PIPViewManager(CameraActivity context) {
        super(context);
    }

    protected Handler mMainHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_REMOVE_VIEW:
                // If we removeView and addView frequently, drawing cache may be
                // wrong.
                // Here avoid do this action frequently to workaround that
                // issue.
                if (mSlidingDrawer != null && mSlidingDrawer.getParent() != null) {
                    getContext().removeView(mSlidingDrawer, VIEW_LAYER_SETTING);
                }
                break;
            default:
                break;
            }
        };
    };

    @Override
    protected View getView() {
        // View view = inflate(R.layout.pip_setting,
        // ViewManager.VIEW_LAYER_SETTING);
        mPipSettingLayout = (ViewGroup) getContext().inflate(R.layout.pip_setting,
                ViewManager.VIEW_LAYER_SETTING);
        mIndicator = (ImageView) mPipSettingLayout.findViewById(R.id.pip_indicator);
        mSlidingDrawer = (SlidingDrawer) mPipSettingLayout.findViewById(R.id.drawer1);
        mSlidingDrawer.setOnDrawerCloseListener(new OnDrawerCloseListener() {
            @Override
            public void onDrawerClosed() {
                mOrientation = getContext().getDisplayRotation();
                if (mOrientation == 90 || mOrientation == 270) {
                    mIndicator.setImageResource(R.drawable.land_open_row);
                } else {
                    mIndicator.setImageResource(R.drawable.port_close_row);
                }
                collapse(true);
            }
        });
        mSlidingDrawer.setOnDrawerOpenListener(new OnDrawerOpenListener() {
            @Override
            public void onDrawerOpened() {
                mOrientation = getContext().getDisplayRotation();
                if (mOrientation == 90 || mOrientation == 270) {
                    mIndicator.setImageResource(R.drawable.land_close_row);
                } else {
                    mIndicator.setImageResource(R.drawable.port_open_row);
                }
                showEffect();
            }
        });
        if (mSlidingDrawer.getParent() == null) {
            getContext().addView(mSlidingDrawer, VIEW_LAYER_SETTING);
        }
        DisplayMetrics metrics = getContext().getResources().getDisplayMetrics();
        mDisplayWidth = Math.max(metrics.widthPixels, metrics.heightPixels);
        mDisplayHeight = Math.min(metrics.widthPixels, metrics.heightPixels);
        mDensity = metrics.density;
        if (mListener != null) {
            mListener.onUpdateEffect(mPIPFrontView[mCurrentEffect][0],
                    mPIPFrontView[mCurrentEffect][1], mPIPFrontView[mCurrentEffect][2]);
        }
        return mPipSettingLayout;
    }

    @Override
    public void onOrientationChanged(int orientation) {
        mCurrentOrientation = orientation;
        rotatePipSettingViewItem(orientation);
    }

    private void rotatePipSettingViewItem(int orientation) {
        Log.i(TAG, "rotatePipSettingViewItem (orientation) = " + orientation);
        mOrientation = getContext().getDisplayRotation();
        if (mOrientation == 90 || mOrientation == 270) {
            for (int i = 0; i < PIP_VIEW_NUM; i++) {
                if (mModeViews[i] != null && mModeViews[i].isShown()) {
                    Util.setOrientation(mModeViews[i], orientation, false);
                }
            }
        } else {
            for (int i = 0; i < PIP_VIEW_NUM; i++) {
                if (mModeViews[i] != null && mModeViews[i].isShown()) {
                    Util.setOrientation(mModeViews[i], orientation + 180, false);
                }
            }
        }
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    public void showEffect() {
        Log.i(TAG, "showEffect... start");
        if (getContext().isFullScreen()) {
            if (!mIsShowingPipSetting && getContext().getCameraAppUI().isNormalViewState()
                    && mPipSettingLayout != null) {
                mMainHandler.removeMessages(MSG_REMOVE_VIEW);
                mIsShowingPipSetting = true;
                initialEffect();
                if (mCurrentOrientation != -1) {
                    rotatePipSettingViewItem(mCurrentOrientation);
                }
            }
        }
        Log.i(TAG, "showEffect... end");
    }

    private void initialEffect() {
        Log.i(TAG, "initialEffect()");
        initialModeViewsAndLayout();
        applyListener();
        highlightCurrentMode();
    }

    private void initialModeViewsAndLayout() {
        LinearLayout.LayoutParams mLayoutParams = null;
        mOrientation = getContext().getDisplayRotation();
        for (int i = 0; i < PIP_VIEW_NUM; i++) {
            if (mModeViews[i] == null) {
                mModeViews[i] = (RotateImageView) mPipSettingLayout.findViewById(mImageViewId[i]);
            }
            if (mItemLayouts[i] == null) {
                mItemLayouts[i] = (LinearLayout) mPipSettingLayout.findViewById(mItemLayoutId[i]);
                mLayoutParams = (LinearLayout.LayoutParams) mItemLayouts[i].getLayoutParams();
                if (mOrientation == 90 || mOrientation == 270) {
                    mLayoutParams.setMargins(0, 0, getItemLayoutMargin(), 0);
                } else {
                    mLayoutParams.setMargins(0, getItemLayoutMargin(), 0, 0);
                }
                mItemLayouts[i].setLayoutParams(mLayoutParams);
            }
        }
        Log.i(TAG, "initialModeViewsAndLayout mOrientation = " + mOrientation);
    }

    private int getItemLayoutMargin() {
        int itemLayoutMargin = 0;
        itemLayoutMargin = (int) (mDisplayWidth - mDensity * MODE_DEFAULT_WIDTH
                * DEFAULT_VIEW_SHOW_NUM - MODE_DEFAULT_WIDTH)
                / (DEFAULT_VIEW_SHOW_NUM + 1);
        Log.i(TAG, "getItemLayoutMargin itemLayoutMargin = " + itemLayoutMargin
                + "mDisplayWidth = " + mDisplayWidth);
        return itemLayoutMargin;
    }

    private void applyListener() {
        for (int i = 0; i < PIP_VIEW_NUM; i++) {
            if (mModeViews[i] != null) {
                mModeViews[i].setOnClickListener(new ViewClickListener());
            }
        }
    }

    private class ViewClickListener implements View.OnClickListener {

        @Override
        public void onClick(View v) {
            Log.i(TAG, "onClick v.getId() = " + v.getId());
            if (mListener == null) {
                Log.i(TAG, "onClick mListener = null");
                return;
            }
            switch (v.getId()) {
            case R.id.pip_cubism:
                mListener.onUpdateEffect(mPIPFrontView[CUBISM][0], mPIPFrontView[CUBISM][1],
                        mPIPFrontView[CUBISM][2]);
                setImageFocusView(CUBISM);
                break;
            case R.id.pip_fisheye:
                mListener.onUpdateEffect(mPIPFrontView[FISHEYE][0], mPIPFrontView[FISHEYE][1],
                        mPIPFrontView[FISHEYE][2]);
                setImageFocusView(FISHEYE);
                break;
            case R.id.pip_heart:
                mListener.onUpdateEffect(mPIPFrontView[HEART][0], mPIPFrontView[HEART][1],
                        mPIPFrontView[HEART][2]);
                setImageFocusView(HEART);
                break;
            case R.id.pip_instantphoto:
                mListener.onUpdateEffect(mPIPFrontView[INSTANTPHOTO][0],
                        mPIPFrontView[INSTANTPHOTO][1], mPIPFrontView[INSTANTPHOTO][2]);
                setImageFocusView(INSTANTPHOTO);
                break;
            case R.id.pip_ovalblur:
                mListener.onUpdateEffect(mPIPFrontView[OVALBLUR][0], mPIPFrontView[OVALBLUR][1],
                        mPIPFrontView[OVALBLUR][2]);
                setImageFocusView(OVALBLUR);
                break;
            case R.id.pip_postcard:
                mListener.onUpdateEffect(mPIPFrontView[POSTCARD][0], mPIPFrontView[POSTCARD][1],
                        mPIPFrontView[POSTCARD][2]);
                setImageFocusView(POSTCARD);
                break;
            case R.id.pip_split:
                mListener.onUpdateEffect(mPIPFrontView[SPLIT][0], mPIPFrontView[SPLIT][1],
                        mPIPFrontView[SPLIT][2]);
                setImageFocusView(SPLIT);
                break;
            case R.id.pip_window:
                mListener.onUpdateEffect(mPIPFrontView[WINDOW][0], mPIPFrontView[WINDOW][1],
                        mPIPFrontView[WINDOW][2]);
                setImageFocusView(WINDOW);
                break;
            default:
                break;
            }

        }
    }

    private void setImageFocusView(int mode) {
        mCurrentEffect = mode;
        for (int i = 0; i < PIP_VIEW_NUM; i++) {
            if (mModeViews[i] != null) {
                if (i == mode) {
                    mModeViews[i].setImageResource(mImageViewFocus[i]);
                } else {
                    mModeViews[i].setImageResource(mImageView[i]);
                }
            }
        }
    }

    private void clearListener() {
        for (int i = 0; i < PIP_VIEW_NUM; i++) {
            if (mModeViews[i] != null) {
                mModeViews[i].setOnClickListener(null);
                mModeViews[i] = null;
            }
        }
    }

    private void highlightCurrentMode() {
        Log.i(TAG, "highlightCurrentMode()");
        for (int i = 0; i < PIP_VIEW_NUM; i++) {
            if (mModeViews[i] != null) {
                if (i == mCurrentEffect) {
                    mModeViews[i].setImageResource(mImageViewFocus[i]);
                } else {
                    mModeViews[i].setImageResource(mImageView[i]);
                }
            }
        }
    }

    public void hideEffect() {
        Log.i(TAG, "hideEffect() mIsShowingPipSetting = " + mIsShowingPipSetting);
        if (mIsShowingPipSetting && mPipSettingLayout != null) {
            mMainHandler.removeMessages(MSG_REMOVE_VIEW);
            mIsShowingPipSetting = false;
            mSlidingDrawer.close();
            getContext().getCameraAppUI().restoreViewState();
        }
    }

    private void removeAllLayout() {
        Log.i(TAG, "removeAllLayout()");
        mMainHandler.sendEmptyMessage(MSG_REMOVE_VIEW);
        mPipSettingLayout = null;
        mFadeIn = null;
        mFadeOut = null;
        for (int i = 0; i < PIP_VIEW_NUM; i++) {
            if (mItemLayouts[i] != null) {
                mItemLayouts[i] = null;
            }
        }
    }

    @Override
    public void onRelease() {
        Log.i(TAG, "onRelease");
        super.onRelease();
        removeAllLayout();
        clearListener();
        if (!getContext().isDualCameraDeviceEnable()) {
            mCurrentEffect = CUBISM;
        }
    }

    @Override
    public boolean collapse(boolean force) {
        boolean collapsechild = false;
        if (mIsShowingPipSetting) {
            hideEffect();
            collapsechild = true;
        }
        Log.d(TAG, "collapse(" + force + ") mIsShowingPipSetting=" + mIsShowingPipSetting
                + ", return " + collapsechild);

        return collapsechild;
    }

    public SlidingDrawer getSlideDrawer() {
        return mSlidingDrawer;
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        Log.i(TAG, "setEnabled enabled= " + enabled);
        if (enabled) {
            mSlidingDrawer.unlock();
        } else {
            mSlidingDrawer.lock();
        }
    }

    public boolean isPipSettingShowing() {
        return mIsShowingPipSetting;
    }
    @Override
    public void show() {
        super.show();
        Log.i(TAG, "show()");
        if (mListener != null) {
            mListener.onUpdateEffect(mPIPFrontView[mCurrentEffect][0],
                    mPIPFrontView[mCurrentEffect][1], mPIPFrontView[mCurrentEffect][2]);
        }
    }
}
