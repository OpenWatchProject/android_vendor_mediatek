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

package com.mediatek.camera.v2.stream.pip;

import android.app.Activity;
import android.content.Context;
import android.hardware.display.DisplayManager;
import android.util.DisplayMetrics;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SlidingDrawer;
import android.widget.SlidingDrawer.OnDrawerCloseListener;
import android.widget.SlidingDrawer.OnDrawerOpenListener;

import com.android.camera.R;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.platform.app.AppController;
import com.mediatek.camera.v2.platform.app.AppUi;
import com.mediatek.camera.v2.stream.IRecordStream.RecordStreamStatus;
import com.mediatek.camera.v2.stream.pip.IPipStream.PipStreamCallback;
import com.mediatek.camera.v2.ui.RotateImageView;
import com.mediatek.camera.v2.util.Utils;

@SuppressWarnings("deprecation")
public class PipStreamView implements IPipView {
    private static final Tag TAG = new Tag(PipStreamView.class.getSimpleName());
    private final Activity             mActivity;
    private final IPipStream  mPipController;
    private final ViewGroup            mParentViewGroup;
    private final AppUi                mAppUi;
    private final PipViewCallback      mCallback;

    private PipStreamCallback          mPipStreamCallback = new PipViewStreamCallback();
    private PipRecordStreamCallback    mPipRecordStreamCallback = new PipRecordStreamCallback();

    private ViewGroup                  mParentViewLayout;
    private ViewGroup                  mPipSettingLayout;
    private boolean                    mPipSettingShown;
    private SlidingDrawer              mSlidingDrawer;
    private ImageView                  mIndicator;
    private boolean                    mTemplateSelectViewShown = false;

    private int                        mDisplayRotation;
    private int                        mOrientationCompensation;

    private int                        mDisplayWidth;
    private int                        mDisplayHeight;
    private float                      mDensity;

    private static final int           MODE_DEFAULT_WIDTH = 100;
    private static final int           DEFAULT_VIEW_SHOW_NUM = 5;
    private static final int           PIP_TEMPLATE_NUM = 8;
    public static final int            CUBISM = 0;
    public static final int            FISHEYE = 1;
    public static final int            HEART = 2;
    public static final int            INSTANTPHOTO = 3;
    public static final int            OVALBLUR = 4;
    public static final int            POSTCARD = 5;
    public static final int            SPLIT = 6;
    public static final int            WINDOW = 7;

    private int                        mSelectedTempate = CUBISM;
    private static int                 editView = R.drawable.plus;
    private static int[]               mImageView = new int[PIP_TEMPLATE_NUM];
    private static int[]               mImageViewFocus = new int[PIP_TEMPLATE_NUM];
    private static int[]               mImageViewId = new int[PIP_TEMPLATE_NUM];
    private static int[][]             mPIPFrontView = new int[PIP_TEMPLATE_NUM][];
    private static int[]               mItemLayoutId = new int[PIP_TEMPLATE_NUM];
    private final RotateImageView[]    mModeViews = new RotateImageView[PIP_TEMPLATE_NUM];
    private final LinearLayout[]       mItemLayouts = new LinearLayout[PIP_TEMPLATE_NUM];
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

    public PipStreamView(AppController app,
            IPipStream pipController,
            PipViewCallback callback) {
        mActivity        = app.getActivity();
        mAppUi           = app.getCameraAppUi();
        mParentViewGroup = mAppUi.getModuleLayoutRoot();
        mPipController   = pipController;
        mCallback        = callback;
        getView();
    }

    @Override
    public void open() {
        LogHelper.d(TAG, "open");
        mPipController.registerPipStreamCallback(mPipStreamCallback);
        mPipController.registerPipRecordStreamCallback(mPipRecordStreamCallback);
        mDisplayRotation = Utils.getDisplayRotation(mActivity);
        ((DisplayManager) mActivity
                .getSystemService(Context.DISPLAY_SERVICE))
                .registerDisplayListener(mDisplayListener, null);
    }

    @Override
    public void close() {
        LogHelper.d(TAG, "close");
        removeView();
        mPipController.unregisterPipStreamCallback(mPipStreamCallback);
        mPipController.unregisterPipRecordStreamCallback(mPipRecordStreamCallback);
        ((DisplayManager) mActivity
                .getSystemService(Context.DISPLAY_SERVICE))
                .unregisterDisplayListener(mDisplayListener);
    }

    @Override
    public boolean onBackPressed() {
        LogHelper.d(TAG, "onBackPressed mTemplateSelectViewShown:" + mTemplateSelectViewShown);
        if (mTemplateSelectViewShown) {
            hideTemplateSelectView();
            return true;
        }
        return false;
    }

    @Override
    public void onOrientationChanged(int newOrientation) {
        mOrientationCompensation = (newOrientation + mDisplayRotation) % 360;
        rotatePipSettingViewItem(mOrientationCompensation);
    }

    @Override
    public void onCameraPicked(String cameraId) {
        if (mTemplateSelectViewShown) {
            hideTemplateSelectView();
        }
    }

    @Override
    public void onPreviewVisibleChanged(boolean visible) {
        LogHelper.d(TAG, "onPreviewVisibleChanged visible:" + visible);
        if (visible) {
            showPipView();
        } else {
            hidePipView();
            if (mTemplateSelectViewShown) {
                hideTemplateSelectView();
            }
        }
    }

    @Override
    public boolean onTouchEvent() {
        LogHelper.d(TAG, "onTouchEvent mTemplateSelectViewShown:" + mTemplateSelectViewShown);
        if (mTemplateSelectViewShown) {
            hideTemplateSelectView();
            return true;
        }
        return false;
    }

    @SuppressWarnings("deprecation")
    private void getView() {
        LogHelper.d(TAG, "[getView]+");
        mParentViewLayout = (ViewGroup) mActivity.getLayoutInflater().inflate(
                R.layout.pip_setting_v2,
                mParentViewGroup,
                true);
        mPipSettingLayout = (ViewGroup) mParentViewLayout.findViewById(R.id.pip_setting_layout);
        mIndicator = (ImageView) mParentViewLayout.findViewById(R.id.pip_indicator);
        mSlidingDrawer = (SlidingDrawer) mParentViewLayout.findViewById(R.id.drawer1);
        mSlidingDrawer.setOnDrawerCloseListener(new OnDrawerCloseListener() {
            @Override
            public void onDrawerClosed() {
                if (mDisplayRotation == 90 || mDisplayRotation == 270) {
                    mIndicator.setImageResource(R.drawable.land_open_row);
                } else {
                    mIndicator.setImageResource(R.drawable.port_close_row);
                }
                hideTemplateSelectView();
            }
        });
        mSlidingDrawer.setOnDrawerOpenListener(new OnDrawerOpenListener() {
            @Override
            public void onDrawerOpened() {
                if (mDisplayRotation == 90 || mDisplayRotation == 270) {
                    mIndicator.setImageResource(R.drawable.land_close_row);
                } else {
                    mIndicator.setImageResource(R.drawable.port_open_row);
                }
                showTemplateSelectView();
            }
        });
        DisplayMetrics metrics = mActivity.getResources().getDisplayMetrics();
        mDisplayWidth = Math.max(metrics.widthPixels, metrics.heightPixels);
        mDisplayHeight = Math.min(metrics.widthPixels, metrics.heightPixels);
        mDensity = metrics.density;

        LogHelper.d(TAG, "[getView]-");
    }

    private void showPipView() {
        LogHelper.d(TAG, "showPipView mPipSettingShown:" + mPipSettingShown);
        if (mPipSettingShown) {
            return;
        }
        if (mPipSettingLayout == null) {
            getView();
        }
        mPipSettingShown = true;
        mPipSettingLayout.setVisibility(View.VISIBLE);
    }

    private void hidePipView() {
        LogHelper.d(TAG, "hidePipView mIndicator:" + mIndicator);
        mPipSettingShown = false;
        if (mPipSettingLayout != null) {
            mPipSettingLayout.setVisibility(View.INVISIBLE);
        }
    }

    private void showTemplateSelectView() {
        LogHelper.d(TAG, "[showTemplateSelectView]+");
        if (!mTemplateSelectViewShown && mPipSettingLayout != null) {
            mAppUi.setAllCommonViewButShutterVisible(false);
            mTemplateSelectViewShown = true;
            initialEffect();
            if (mOrientationCompensation != -1) {
                rotatePipSettingViewItem(mOrientationCompensation);
            }
        }
        LogHelper.d(TAG, "[showTemplateSelectView]-");
    }

    private void hideTemplateSelectView() {
        LogHelper.d(TAG, "[hideTemplateSelectView]+ mIsShowingPipSetting = "
                + mTemplateSelectViewShown);
        if (mTemplateSelectViewShown && mPipSettingLayout != null) {
            mTemplateSelectViewShown = false;
            mSlidingDrawer.close();
            mAppUi.setAllCommonViewButShutterVisible(true);
        }
        LogHelper.d(TAG, "[hideTemplateSelectView]-");
    }

    private void setImageFocusView(int mode) {
        mSelectedTempate = mode;
        for (int i = 0; i < PIP_TEMPLATE_NUM; i++) {
            if (mModeViews[i] != null) {
                if (i == mode) {
                    mModeViews[i].setImageResource(mImageViewFocus[i]);
                } else {
                    mModeViews[i].setImageResource(mImageView[i]);
                }
            }
        }
    }

    private void initialModeViewsAndLayout() {
        LinearLayout.LayoutParams mLayoutParams = null;
        for (int i = 0; i < PIP_TEMPLATE_NUM; i++) {
            if (mModeViews[i] == null) {
                mModeViews[i] = (RotateImageView) mParentViewLayout.findViewById(mImageViewId[i]);
            }
            if (mItemLayouts[i] == null) {
                mItemLayouts[i] = (LinearLayout) mParentViewLayout.findViewById(mItemLayoutId[i]);
                mLayoutParams = (LinearLayout.LayoutParams) mItemLayouts[i].getLayoutParams();
                if (mDisplayRotation == 90 || mDisplayRotation == 270) {
                    mLayoutParams.setMargins(0, 0, getItemLayoutMargin(), 0);
                } else {
                    mLayoutParams.setMargins(0, getItemLayoutMargin(), 0, 0);
                }
                mItemLayouts[i].setLayoutParams(mLayoutParams);
            }
        }
        LogHelper.d(TAG, "initialModeViewsAndLayout mOrientation = " + mDisplayRotation);
    }

    private int getItemLayoutMargin() {
        int itemLayoutMargin = 0;
        itemLayoutMargin = (int) (mDisplayWidth - mDensity * MODE_DEFAULT_WIDTH
                * DEFAULT_VIEW_SHOW_NUM - MODE_DEFAULT_WIDTH)
                / (DEFAULT_VIEW_SHOW_NUM + 1);
        LogHelper.d(TAG, "getItemLayoutMargin itemLayoutMargin = " + itemLayoutMargin
                + "mDisplayWidth = " + mDisplayWidth);
        return itemLayoutMargin;
    }

    private void applyListener() {
        for (int i = 0; i < PIP_TEMPLATE_NUM; i++) {
            if (mModeViews[i] != null) {
                mModeViews[i].setOnClickListener(new ViewClickListener());
            }
        }
    }

    private void clearListener() {
        for (int i = 0; i < PIP_TEMPLATE_NUM; i++) {
            if (mModeViews[i] != null) {
                mModeViews[i].setOnClickListener(null);
                mModeViews[i] = null;
            }
        }
    }

    private void highlightCurrentMode() {
        LogHelper.d(TAG, "highlightCurrentMode()");
        for (int i = 0; i < PIP_TEMPLATE_NUM; i++) {
            if (mModeViews[i] != null) {
                if (i == mSelectedTempate) {
                    mModeViews[i].setImageResource(mImageViewFocus[i]);
                } else {
                    mModeViews[i].setImageResource(mImageView[i]);
                }
            }
        }
    }

    private void initialEffect() {
        LogHelper.d(TAG, "initialEffect()");
        initialModeViewsAndLayout();
        applyListener();
        highlightCurrentMode();
    }

    private void rotatePipSettingViewItem(int orientation) {
        if (mDisplayRotation == 90 || mDisplayRotation == 270) {
            for (int i = 0; i < PIP_TEMPLATE_NUM; i++) {
                if (mModeViews[i] != null && mModeViews[i].isShown()) {
                    Utils.setRotatableOrientation(mModeViews[i], orientation, false);
                }
            }
        } else {
            for (int i = 0; i < PIP_TEMPLATE_NUM; i++) {
                if (mModeViews[i] != null && mModeViews[i].isShown()) {
                    Utils.setRotatableOrientation(mModeViews[i], orientation + 180, false);
                }
            }
        }
    }

    private void removeAllLayout() {
        LogHelper.d(TAG, "removeAllLayout()");
        mPipSettingLayout = null;
        for (int i = 0; i < PIP_TEMPLATE_NUM; i++) {
            if (mItemLayouts[i] != null) {
                mItemLayouts[i].removeAllViews();
                mItemLayouts[i] = null;
            }
        }
    }

    private void removeView() {
        LogHelper.d(TAG, "removeView");
        if (mPipSettingLayout != null) {
            mPipSettingLayout.removeAllViewsInLayout();
            mParentViewGroup.removeView(mPipSettingLayout);
            mPipSettingLayout = null;
        }
        clearListener();
        removeAllLayout();
    }

    private void reInflate() {
        LogHelper.d(TAG, "reInflate");
        boolean showing = mPipSettingShown;
        removeView();
        hidePipView();
        if (showing) {
            showPipView();
        }
    }

    private DisplayManager.DisplayListener mDisplayListener = new DisplayManager.DisplayListener() {
        @Override
        public void onDisplayAdded(int arg0) {
            // Do nothing.
        }
        @Override
        public void onDisplayChanged(int displayId) {
            LogHelper.d(TAG, "[onDisplayChanged]+ mDisplayRotation:" + mDisplayRotation);
            mDisplayRotation = Utils.getDisplayRotation(mActivity);
            //Don't need to inflate view when Camera pause,
            //or there will be multi-instance of Pip view
            if (mPipSettingLayout != null
                    && mPipSettingLayout.getVisibility() == View.VISIBLE) {
                reInflate();
            }
            LogHelper.d(TAG, "[onDisplayChanged]- mDisplayRotation:" + mDisplayRotation);
        }
        @Override
        public void onDisplayRemoved(int arg0) {
            // Do nothing.
        }
    };

    private class ViewClickListener implements View.OnClickListener {

        @Override
        public void onClick(View v) {
            LogHelper.d(TAG, "onClick v.getId() = " + v.getId());
            switch (v.getId()) {
            case R.id.pip_cubism:
                mCallback.onTemplateChanged(mPIPFrontView[CUBISM][0], mPIPFrontView[CUBISM][1],
                        mPIPFrontView[CUBISM][2], editView);
                setImageFocusView(CUBISM);
                break;
            case R.id.pip_fisheye:
                mCallback.onTemplateChanged(mPIPFrontView[FISHEYE][0], mPIPFrontView[FISHEYE][1],
                        mPIPFrontView[FISHEYE][2], editView);
                setImageFocusView(FISHEYE);
                break;
            case R.id.pip_heart:
                mCallback.onTemplateChanged(mPIPFrontView[HEART][0], mPIPFrontView[HEART][1],
                        mPIPFrontView[HEART][2], editView);
                setImageFocusView(HEART);
                break;
            case R.id.pip_instantphoto:
                mCallback.onTemplateChanged(mPIPFrontView[INSTANTPHOTO][0],
                        mPIPFrontView[INSTANTPHOTO][1], mPIPFrontView[INSTANTPHOTO][2], editView);
                setImageFocusView(INSTANTPHOTO);
                break;
            case R.id.pip_ovalblur:
                mCallback.onTemplateChanged(mPIPFrontView[OVALBLUR][0], mPIPFrontView[OVALBLUR][1],
                        mPIPFrontView[OVALBLUR][2], editView);
                setImageFocusView(OVALBLUR);
                break;
            case R.id.pip_postcard:
                mCallback.onTemplateChanged(mPIPFrontView[POSTCARD][0], mPIPFrontView[POSTCARD][1],
                        mPIPFrontView[POSTCARD][2], editView);
                setImageFocusView(POSTCARD);
                break;
            case R.id.pip_split:
                mCallback.onTemplateChanged(mPIPFrontView[SPLIT][0], mPIPFrontView[SPLIT][1],
                        mPIPFrontView[SPLIT][2], editView);
                setImageFocusView(SPLIT);
                break;
            case R.id.pip_window:
                mCallback.onTemplateChanged(mPIPFrontView[WINDOW][0], mPIPFrontView[WINDOW][1],
                        mPIPFrontView[WINDOW][2], editView);
                setImageFocusView(WINDOW);
                break;
            default:
                break;
            }

        }
    }

    private class PipViewStreamCallback implements PipStreamCallback {
        @Override
        public void onOpened() {
            LogHelper.d(TAG, "onOpened");
            showPipView();
            mCallback.onTemplateChanged(
                    mPIPFrontView[mSelectedTempate][0],
                    mPIPFrontView[mSelectedTempate][1],
                    mPIPFrontView[mSelectedTempate][2],
                    editView);
        }
        @Override
        public void onClosed() {
            hidePipView();
        }
        @Override
        public void onPaused() {
            if (mTemplateSelectViewShown && mPipSettingLayout != null) {
                mTemplateSelectViewShown = false;
                mSlidingDrawer.close();
                mAppUi.showThumbnailManagerUi();
            }
            hidePipView();
        }
        @Override
        public void onResumed() {
            showPipView();
        }
        @Override
        public void onTopGraphicTouched() {
            if (mTemplateSelectViewShown) {
                hideTemplateSelectView();
            }
        }
        @Override
        public void onSwitchPipEventReceived() {
            if (mTemplateSelectViewShown) {
                hideTemplateSelectView();
            }
        }
    }

    private class PipRecordStreamCallback implements RecordStreamStatus {
        @Override
        public void onRecordingStarted(boolean canUseExtra) {
            hidePipView();
            if (mTemplateSelectViewShown && mPipSettingLayout != null) {
                mTemplateSelectViewShown = false;
                mSlidingDrawer.close();
                mAppUi.showThumbnailManagerUi();
            }
        }

        @Override
        public void onRecordingStoped() {
            showPipView();
        }

        @Override
        public void onInfo(int what, int extra) {

        }

        @Override
        public void onError(int what, int extra) {

        }
    }
}