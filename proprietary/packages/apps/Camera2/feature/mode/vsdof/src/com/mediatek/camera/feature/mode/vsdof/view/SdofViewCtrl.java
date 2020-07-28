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
package com.mediatek.camera.feature.mode.vsdof.view;

import java.util.List;
import java.util.Arrays;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.hardware.camera2.CameraCharacteristics;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.widget.RotateLayout;

/**
 * The sdof view manager.
 */
public class SdofViewCtrl {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SdofViewCtrl.class.getSimpleName());
    // Stereo Photo warning message
    public static final int DUAL_CAMERA_LENS_COVERED = 1;
    public static final int DUAL_CAMERA_LOW_LIGHT = 2;
    public static final int DUAL_CAMERA_TOO_CLOSE = 4;
    public static final int DUAL_CAMERA_TOO_FAR = 1 << 31;
    public static final int DUAL_CAMERA_READY = 0;
    //Gesture and View Control
    private static final int SHOW_INFO_LENGTH_LONG = 5 * 1000;
    private static final int SDOF_BAR_MARGION_VERTICAL = 130;
    private static final int SDOF_BAR_MARGION_HORIZON = 40;
    private static final int SDOF_VIEW_HIDE_DELAY_TIME = 3000;
    private static final int SDOF_TEXT_VIEW_SHOW = 0;
    private static final int SDOF_VIEW_RESET = 1;
    private static final int SDOF_VIEW_INIT_AND_SHOW = 2;
    private static final int SDOF_VIEW_UNINIT = 3;
    private static final int SDOF_VIEW_ORIENTATION_CHANGED = 4;
    private static final int SDOF_TEXT_VIEW_HIDE = 5;
    private static final int SDOF_WARNING_VIEW_SHOW = 6;
    private static final long PROGRESS_DELAY_TIME = 50;
    private static final int LEVEL_DEFAULT = 7;

    private ViewGroup mRootViewGroup;
    private RotateLayout mSdofLayout;
    private RelativeLayout mSdofView;
    private IApp mApp;
    private TextView mTextView;
    private MainHandler mMainHandler;
    private ViewChangeListener mViewChangeListener;
    private SeekBar mDofBar;
    private long mProcessTime;
    private int mLevel = LEVEL_DEFAULT;
    private IAppUi.HintInfo mGuideHint;

    private static final String VSDOF_LEVEL_KEY = "com.mediatek.stereofeature.supporteddoflevel";
    // default dof level 15 values.
    private int mDofLevel = 15;
    private int mProgress = 50;
    private static final int PROGRESS_MAX = 100;

    /**
     * Init the view.
     * @param app the activity.
     */
    public void init(IApp app) {
        mApp = app;
        mGuideHint = new IAppUi.HintInfo();
        int id = mApp.getActivity().getResources().getIdentifier("hint_text_background",
                "drawable", mApp.getActivity().getPackageName());
        mGuideHint.mBackground = mApp.getActivity().getDrawable(id);
        mGuideHint.mType = IAppUi.HintType.TYPE_AUTO_HIDE;
        mGuideHint.mDelayTime = SHOW_INFO_LENGTH_LONG;
        mMainHandler = new MainHandler(app.getActivity().getMainLooper());
        CameraCharacteristics cs = CameraUtil.getCameraCharacteristics(mApp.getActivity(),
                CameraUtil.getLogicalCameraId());
        if (cs != null) {
            int[] dofLevel = CameraUtil.getStaticKeyResult(cs, VSDOF_LEVEL_KEY);
            if (dofLevel == null || dofLevel.length == 0 ||
                    dofLevel[0] == 0 || dofLevel[0] == 1) {
                return;
            }
            LogHelper.i(TAG, "[init] dofLevel value " + dofLevel[0]);
            if (dofLevel[0] != mDofLevel) {
                mDofLevel = dofLevel[0];
            }
        }
    }


    /**
     * show the view.
     */
    public void showView() {
        if (mMainHandler != null) {
            mMainHandler.sendEmptyMessage(SDOF_VIEW_INIT_AND_SHOW);
            mViewChangeListener.onVsDofLevelChanged(mLevel);
        }
    }

    /**
     * To destroy the zoom view.
     */
    public void unInit() {
        if (mMainHandler != null) {
            mMainHandler.sendEmptyMessage(SDOF_VIEW_RESET);
            mMainHandler.sendEmptyMessage(SDOF_VIEW_UNINIT);
            mMainHandler.removeMessages(SDOF_WARNING_VIEW_SHOW);
        }
    }

    /**
     * when phone orientation changed, the zoom view will be updated.
     * @param orientation the orientation of g-sensor.
     */
    public void onOrientationChanged(int orientation) {
        if (mMainHandler != null) {
            mMainHandler.obtainMessage(SDOF_VIEW_ORIENTATION_CHANGED, orientation).sendToTarget();
        }
    }

    /**
     * Set dof bar view change listener.
     * @param listener the view change listener.
     */
    public void setViewChangeListener(ViewChangeListener listener) {
        mViewChangeListener = listener;
    }

    /**
     * show warning info view.
     *
     * @param type The warning info.
     */
    public void showWarningView(int type) {
        if (mMainHandler != null) {
            mMainHandler.obtainMessage(SDOF_WARNING_VIEW_SHOW, type).sendToTarget();
        }
    }

    /**
     * This listener used for update info with mode.
     */
    public interface ViewChangeListener {
        /**
         * This method used for notify mode dof level.
         * @param level Video shallow dof level
         */
        public void onVsDofLevelChanged(int level);

        /**
         * This method used for notify mode touch position.
         * @param value touch position.
         */
        public void onTouchPositionChanged(int value);
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
                case SDOF_TEXT_VIEW_SHOW:
                    mTextView.setVisibility(View.VISIBLE);
                    break;
                case SDOF_TEXT_VIEW_HIDE:
                    mTextView.setVisibility(View.INVISIBLE);
                    break;
                case SDOF_VIEW_INIT_AND_SHOW:
                    initView();
                    break;
                case SDOF_VIEW_UNINIT:
                    unInitView();
                    break;
                case SDOF_VIEW_ORIENTATION_CHANGED:
                    updateOrientation((Integer) msg.obj);
                    break;
                case SDOF_WARNING_VIEW_SHOW:
                    showGuideView((Integer) msg.obj);
                    break;
                default:
                    break;
            }
        }
    }

    private void initView() {
        mRootViewGroup = mApp.getAppUi().getModeRootView();
        mSdofLayout = (RotateLayout) mApp.getActivity().getLayoutInflater()
                .inflate(R.layout.sdof_view,
                mRootViewGroup, false).findViewById(R.id.sdof_rotate_layout);
        mSdofView = (RelativeLayout) mSdofLayout.findViewById(R.id.sdof_bottom_controls);
        mTextView = (TextView) mSdofLayout.findViewById(R.id.dof_text_view);
        mDofBar = (SeekBar) mSdofLayout.findViewById(R.id.sdof_bar);
        mDofBar.setVisibility(View.VISIBLE);
        mProgress = LEVEL_DEFAULT * PROGRESS_MAX / (mDofLevel -1);
        mDofBar.setProgress(mProgress);
        mDofBar.setOnSeekBarChangeListener(mChangeListener);
        mRootViewGroup.addView(mSdofLayout);
    }

    private void unInitView() {
        mLevel = LEVEL_DEFAULT;
        mProgress = LEVEL_DEFAULT * PROGRESS_MAX / (mDofLevel -1);
        if (mRootViewGroup != null) {
            mRootViewGroup.removeView(mSdofLayout);
        }
        mSdofLayout = null;
        mSdofView = null;
        mApp.getAppUi().hideScreenHint(mGuideHint);
    }

    private void showGuideView(int type) {
        int guideId = 0;
        switch (type) {
            case DUAL_CAMERA_LOW_LIGHT:
                guideId = R.string.dual_camera_lowlight_toast;
                break;
            case DUAL_CAMERA_READY:
                guideId = 0;
                break;
            case DUAL_CAMERA_TOO_CLOSE:
                guideId = R.string.dual_camera_too_close_toast;
                break;
            case DUAL_CAMERA_LENS_COVERED:
                guideId = R.string.dual_camera_lens_covered_toast;
                break;
            case DUAL_CAMERA_TOO_FAR:
                guideId = R.string.dual_camera_too_far_toast;
            default:
                break;
        }
        if (guideId != 0) {
            mGuideHint.mHintText = mApp.getActivity().getString(guideId);
            mApp.getAppUi().showScreenHint(mGuideHint);
        }
    }

    private void updateOrientation(int orientation) {
        if (mSdofView == null) {
            LogHelper.w(TAG, "[updateOrientation] view is null!");
            return;
        }
        RelativeLayout.LayoutParams params =
                (RelativeLayout.LayoutParams) mSdofView.getLayoutParams();

        switch (orientation) {
            case 0:
            case 180:
                params.setMargins(params.leftMargin, params.topMargin,
                        params.rightMargin, dpToPixel(SDOF_BAR_MARGION_VERTICAL));
                break;
            case 90:
            case 270:
                params.setMargins(params.leftMargin, params.topMargin,
                        params.rightMargin, dpToPixel(SDOF_BAR_MARGION_HORIZON));
                break;
            default:
                break;
        }
        mSdofView.setLayoutParams(params);
        CameraUtil.rotateRotateLayoutChildView(mApp.getActivity(), mSdofLayout, orientation, true);
    }

    private int dpToPixel(int dp) {
        float scale = mApp.getActivity().getResources().getDisplayMetrics().density;
        return (int) (dp * scale + 0.5f);
    }

    private SeekBar.OnSeekBarChangeListener mChangeListener
            = new SeekBar.OnSeekBarChangeListener() {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
            long now = System.currentTimeMillis();
            int wantToLevel = Math.round(progress * (mDofLevel - 1) / PROGRESS_MAX);
            if (now - mProcessTime >= PROGRESS_DELAY_TIME) {
                if (wantToLevel != mLevel) {
                    mLevel = wantToLevel;
                    LogHelper.d(TAG, "onProgressChanged level = " + mLevel);
                    mViewChangeListener.onVsDofLevelChanged(mLevel);
                }
            }
            mProcessTime = System.currentTimeMillis();
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
            /*mMainHandler.removeMessages(SDOF_TEXT_VIEW_HIDE);
            mMainHandler.sendEmptyMessage(SDOF_TEXT_VIEW_SHOW);*/
        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
            mProgress = seekBar.getProgress();
            mLevel = Math.round(mProgress * (mDofLevel - 1) / PROGRESS_MAX);
            LogHelper.i(TAG, "onStopTrackingTouch level = " + mLevel);
            mViewChangeListener.onVsDofLevelChanged(mLevel);
            //mMainHandler.sendEmptyMessageDelayed(SDOF_TEXT_VIEW_HIDE, SDOF_VIEW_HIDE_DELAY_TIME);
        }
    };
}
