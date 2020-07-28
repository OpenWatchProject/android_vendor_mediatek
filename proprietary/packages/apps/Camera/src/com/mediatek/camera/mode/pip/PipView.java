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

package com.mediatek.camera.mode.pip;

import android.app.Activity;
import android.content.Context;
import android.hardware.display.DisplayManager;
import android.util.DisplayMetrics;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SlidingDrawer;
import android.widget.SlidingDrawer.OnDrawerCloseListener;
import android.widget.SlidingDrawer.OnDrawerOpenListener;

import com.android.camera.R;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.ui.CameraView;
import com.mediatek.camera.ui.RotateImageView;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;

/**
 * PIPView This Class manager the PIP view
 *
 */
public class PipView extends CameraView {
    private static final String TAG = "PipView";

    private ViewGroup mPipSettingLayout;

    private Animation mFadeIn;
    private Animation mFadeOut;

    private ICameraAppUi mICameraAppUi;

    private SlidingDrawer mSlidingDrawer;
    private ImageView mIndicator;
    private boolean mIsShowingPipSetting = false;
    private Listener mListener;
    private int mDisplayWidth;
    private int mDisplayHeight;
    private float mDensity;
    private int mModeWidth;
    private int mCurrentOrientation = -1;
    private int mDisplayRotation;
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

    public static final int TYPE_SHOW_EFFECT = 0;
    public static final int TYPE_HIDE_EFFECT = 1;
    public static final int TYPE_ORIENTATION_CHANGED = 2;

    private static int[] mImageView = new int[PIP_VIEW_NUM];
    private static int[] mImageViewFocus = new int[PIP_VIEW_NUM];
    private static int[] mImageViewId = new int[PIP_VIEW_NUM];
    private static int[][] mPIPFrontView = new int[PIP_VIEW_NUM][];
    private static int[] mItemLayoutId = new int[PIP_VIEW_NUM];
    private final RotateImageView[] mModeViews = new RotateImageView[PIP_VIEW_NUM];
    private final LinearLayout[] mItemLayouts = new LinearLayout[PIP_VIEW_NUM];
    private static int editView = R.drawable.plus;
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

    private DisplayManager.DisplayListener mDisplayListener = new DisplayManager.DisplayListener() {
        @Override
        public void onDisplayAdded(int arg0) {
            // Do nothing.
        }

        @Override
        public void onDisplayChanged(int displayId) {
            Log.d(TAG, "onDisplayChanged");
            reInflate();
        }

        @Override
        public void onDisplayRemoved(int arg0) {
            // Do nothing.
        }
    };

    public interface Listener {
        public void onUpdateEffect(int effectRearIndex, int effectFrontIndex,
                int effectFrontHighlight, int editIcon);
    }

    public PipView(Activity activity) {
        super(activity);
        Log.d(TAG, "[PipView]constructor...");
    }

    @Override
    public void init(Activity activity, ICameraAppUi cameraAppUi,
            IModuleCtrl moduleCtrl) {
        super.init(activity, cameraAppUi, moduleCtrl);
        mICameraAppUi = cameraAppUi;
    }

    @Override
    public void uninit() {
        Log.d(TAG, "[uninit]...");
        super.uninit();
        mCurrentEffect = CUBISM;
    }

    @Override
    public void show() {
        Log.d(TAG, "[show]...");
        super.show();
        if (mListener != null) {
            mListener.onUpdateEffect(mPIPFrontView[mCurrentEffect][0],
                    mPIPFrontView[mCurrentEffect][1], mPIPFrontView[mCurrentEffect][2], editView);
        }
    }

    @Override
    public void hide() {
        Log.d(TAG, "hide");
        super.hide();
        hideEffect();
    }

    @Override
    public void refresh() {
        Log.d(TAG, "refresh");
        hideEffect();
    }

    @Override
    public boolean isShowing() {
        return mIsShowingPipSetting;
    }

    @Override
    public boolean update(int type, Object... obj) {
        Log.d(TAG, "[update]type = " + type);
        switch (type) {
        case TYPE_SHOW_EFFECT:
            showEffect();
            break;
        case TYPE_HIDE_EFFECT:
            hideEffect();
            break;
        case TYPE_ORIENTATION_CHANGED:
            if (obj[0] != null) {
                mCurrentOrientation = (Integer) obj[0];
                rotatePipSettingViewItem(mCurrentOrientation);
            }
            break;
        default:
            break;
        }
        return true;
    }

    @Override
    public void setListener(Object listener) {
        mListener = (Listener) listener;
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        Log.d(TAG, "setEnabled enabled= " + enabled);
        if (enabled) {
            mSlidingDrawer.unlock();
        } else {
            mSlidingDrawer.lock();
        }
    }

    @Override
    protected View getView() {
        Log.d(TAG, "[getView]...");
        mPipSettingLayout = (ViewGroup) inflate(R.layout.pip_setting);
        mIndicator = (ImageView) mPipSettingLayout.findViewById(R.id.pip_indicator);
        mSlidingDrawer = (SlidingDrawer) mPipSettingLayout.findViewById(R.id.drawer1);
        mSlidingDrawer.setOnDrawerCloseListener(new OnDrawerCloseListener() {
            @Override
            public void onDrawerClosed() {
                mDisplayRotation = Util.getDisplayRotation(mActivity);
                if (mDisplayRotation == 90 || mDisplayRotation == 270) {
                    mIndicator.setImageResource(R.drawable.land_open_row);
                } else {
                    mIndicator.setImageResource(R.drawable.port_close_row);
                }
                hideEffect();
            }
        });
        mSlidingDrawer.setOnDrawerOpenListener(new OnDrawerOpenListener() {
            @Override
            public void onDrawerOpened() {
                mDisplayRotation = Util.getDisplayRotation(mActivity);
                if (mDisplayRotation == 90 || mDisplayRotation == 270) {
                    mIndicator.setImageResource(R.drawable.land_close_row);
                } else {
                    mIndicator.setImageResource(R.drawable.port_open_row);
                }
                showEffect();
            }
        });
        DisplayMetrics metrics = mActivity.getResources().getDisplayMetrics();
        mDisplayWidth = Math.max(metrics.widthPixels, metrics.heightPixels);
        mDisplayHeight = Math.min(metrics.widthPixels, metrics.heightPixels);
        mDensity = metrics.density;
        if (mListener != null) {
            mListener.onUpdateEffect(mPIPFrontView[mCurrentEffect][0],
                    mPIPFrontView[mCurrentEffect][1], mPIPFrontView[mCurrentEffect][2], editView);
        }
        return mPipSettingLayout;
    }

    @Override
    protected void addView(View view) {
        Log.d(TAG, "addView");
        mICameraAppUi.getNormalViewLayer().addView(view);
        ((DisplayManager) mActivity
                .getSystemService(Context.DISPLAY_SERVICE))
                .registerDisplayListener(mDisplayListener, null);
    }

    @Override
    protected void removeView() {
        Log.d(TAG, "removeView");
        if (mPipSettingLayout != null) {
            mPipSettingLayout.removeAllViewsInLayout();
        }
        super.removeView();
        removeAllLayout();
        clearListener();
        ((DisplayManager) mActivity
                .getSystemService(Context.DISPLAY_SERVICE))
                .unregisterDisplayListener(mDisplayListener);
    }

    private void rotatePipSettingViewItem(int orientation) {
        Log.d(TAG, "rotatePipSettingViewItem (orientation) = " + orientation);
        mDisplayRotation = Util.getDisplayRotation(mActivity);
        if (mDisplayRotation == 90 || mDisplayRotation == 270) {
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

    private void initialEffect() {
        Log.d(TAG, "initialEffect()");
        initialModeViewsAndLayout();
        applyListener();
        highlightCurrentMode();
    }

    private void initialModeViewsAndLayout() {
        LinearLayout.LayoutParams mLayoutParams = null;
        mDisplayRotation = Util.getDisplayRotation(mActivity);
        for (int i = 0; i < PIP_VIEW_NUM; i++) {
            if (mModeViews[i] == null) {
                mModeViews[i] = (RotateImageView) mPipSettingLayout.findViewById(mImageViewId[i]);
            }
            if (mItemLayouts[i] == null) {
                mItemLayouts[i] = (LinearLayout) mPipSettingLayout.findViewById(mItemLayoutId[i]);
                mLayoutParams = (LinearLayout.LayoutParams) mItemLayouts[i].getLayoutParams();
                if (mDisplayRotation == 90 || mDisplayRotation == 270) {
                    mLayoutParams.setMargins(0, 0, getItemLayoutMargin(), 0);
                } else {
                    mLayoutParams.setMargins(0, getItemLayoutMargin(), 0, 0);
                }
                mItemLayouts[i].setLayoutParams(mLayoutParams);
            }
        }
        Log.d(TAG, "initialModeViewsAndLayout mOrientation = " + mDisplayRotation);
    }

    private int getItemLayoutMargin() {
        int itemLayoutMargin = 0;
        itemLayoutMargin = (int) (mDisplayWidth - mDensity * MODE_DEFAULT_WIDTH
                * DEFAULT_VIEW_SHOW_NUM - MODE_DEFAULT_WIDTH)
                / (DEFAULT_VIEW_SHOW_NUM + 1);
        Log.d(TAG, "getItemLayoutMargin itemLayoutMargin = " + itemLayoutMargin
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

    private void showEffect() {
        Log.d(TAG, "[showEffect]...");
        if (!mIsShowingPipSetting && mPipSettingLayout != null) {
            mICameraAppUi.hideAllViews();
            mIsShowingPipSetting = true;
            initialEffect();
            if (mCurrentOrientation != -1) {
                rotatePipSettingViewItem(mCurrentOrientation);
            }
        }
    }

    private void hideEffect() {
        Log.d(TAG, "[hideEffect]mIsShowingPipSetting = " + mIsShowingPipSetting);
        if (mIsShowingPipSetting && mPipSettingLayout != null) {
            mIsShowingPipSetting = false;
            mSlidingDrawer.close();
            mICameraAppUi.showAllViews();
        }
    }

    private class ViewClickListener implements View.OnClickListener {

        @Override
        public void onClick(View v) {
            Log.d(TAG, "onClick v.getId() = " + v.getId());
            if (mListener == null) {
                Log.w(TAG, "onClick mListener = null");
                return;
            }
            switch (v.getId()) {
            case R.id.pip_cubism:
                mListener.onUpdateEffect(mPIPFrontView[CUBISM][0], mPIPFrontView[CUBISM][1],
                        mPIPFrontView[CUBISM][2], editView);
                setImageFocusView(CUBISM);
                break;
            case R.id.pip_fisheye:
                mListener.onUpdateEffect(mPIPFrontView[FISHEYE][0], mPIPFrontView[FISHEYE][1],
                        mPIPFrontView[FISHEYE][2], editView);
                setImageFocusView(FISHEYE);
                break;
            case R.id.pip_heart:
                mListener.onUpdateEffect(mPIPFrontView[HEART][0], mPIPFrontView[HEART][1],
                        mPIPFrontView[HEART][2], editView);
                setImageFocusView(HEART);
                break;
            case R.id.pip_instantphoto:
                mListener.onUpdateEffect(mPIPFrontView[INSTANTPHOTO][0],
                        mPIPFrontView[INSTANTPHOTO][1], mPIPFrontView[INSTANTPHOTO][2], editView);
                setImageFocusView(INSTANTPHOTO);
                break;
            case R.id.pip_ovalblur:
                mListener.onUpdateEffect(mPIPFrontView[OVALBLUR][0], mPIPFrontView[OVALBLUR][1],
                        mPIPFrontView[OVALBLUR][2], editView);
                setImageFocusView(OVALBLUR);
                break;
            case R.id.pip_postcard:
                mListener.onUpdateEffect(mPIPFrontView[POSTCARD][0], mPIPFrontView[POSTCARD][1],
                        mPIPFrontView[POSTCARD][2], editView);
                setImageFocusView(POSTCARD);
                break;
            case R.id.pip_split:
                mListener.onUpdateEffect(mPIPFrontView[SPLIT][0], mPIPFrontView[SPLIT][1],
                        mPIPFrontView[SPLIT][2], editView);
                setImageFocusView(SPLIT);
                break;
            case R.id.pip_window:
                mListener.onUpdateEffect(mPIPFrontView[WINDOW][0], mPIPFrontView[WINDOW][1],
                        mPIPFrontView[WINDOW][2], editView);
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
        Log.d(TAG, "highlightCurrentMode()");
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

    private void removeAllLayout() {
        Log.d(TAG, "removeAllLayout()");
        mPipSettingLayout = null;
        mFadeIn = null;
        mFadeOut = null;
        for (int i = 0; i < PIP_VIEW_NUM; i++) {
            if (mItemLayouts[i] != null) {
                mItemLayouts[i].removeAllViews();
                mItemLayouts[i] = null;
            }
        }
    }
}