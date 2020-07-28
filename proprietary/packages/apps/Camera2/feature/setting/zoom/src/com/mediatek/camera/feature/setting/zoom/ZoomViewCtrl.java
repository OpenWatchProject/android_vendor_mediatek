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
package com.mediatek.camera.feature.setting.zoom;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.widget.RotateLayout;

/**
 * The zoom view.
 */
public class ZoomViewCtrl {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(ZoomViewCtrl.class.getSimpleName());
    private ViewGroup mRootViewGroup;
    private RotateLayout mZoomView;
    private IApp mApp;
    private TextView mTextView;
    private MainHandler mMainHandler;
    private IAppUi.HintInfo mZoomIndicatorHint;
    //Gesture and View Control
    private static final int ZOOM_TEXT_MARGION_VERTICAL_REVERSE = 120;
    private static final int ZOOM_TEXT_MARGION_VERTICAL = 40;
    private static final int ZOOM_TEXT_MARGION_VERTICAL_TABLET = 100;
    private static final int ZOOM_TEXT_MARGION_HORIZON = 2;
    private static final int ZOOM_VIEW_HIDE_DELAY_TIME = 3000;
    private static final int ZOOM_VIEW_SHOW = 0;
    private static final int ZOOM_VIEW_RESET = 1;
    private static final int ZOOM_VIEW_INIT = 2;
    private static final int ZOOM_VIEW_UNINIT = 3;
    private static final int ZOOM_VIEW_ORIENTATION_CHANGED = 4;
    private static final int ZOOM_VIEW_HIDE = 5;

    /**
     * Init the view.
     * @param app the activity.
     */
    public void init(IApp app) {
        LogHelper.d(TAG, "[init]");
        mApp = app;
        mMainHandler = new MainHandler(app.getActivity().getMainLooper());
        mMainHandler.sendEmptyMessage(ZOOM_VIEW_INIT);
    }

    /**
     * To destroy the zoom view.
     */
    public void unInit() {
        mMainHandler.sendEmptyMessage(ZOOM_VIEW_RESET);
        mMainHandler.sendEmptyMessage(ZOOM_VIEW_UNINIT);
    }

    /**
     * Used to show ratio test when zooming.
     * @param msg the ratio test.
     */
    public void showView(String msg) {
        mMainHandler.obtainMessage(ZOOM_VIEW_SHOW, msg).sendToTarget();
    }

    /**
     * Used to hide ratio test when zooming.
     */
    public void hideView() {
        mMainHandler.sendEmptyMessage(ZOOM_VIEW_HIDE);
    }

    /**
     * reset camera view after zoom done.
     */
    public void resetView() {
        mMainHandler.sendEmptyMessageDelayed(ZOOM_VIEW_RESET, ZOOM_VIEW_HIDE_DELAY_TIME);
    }

    /**
     * when phone orientation changed, the zoom view will be updated.
     * @param orientation the orientation of g-sensor.
     */
    public void onOrientationChanged(int orientation) {
        if (mMainHandler == null) {
            return;
        }
        mMainHandler.obtainMessage(ZOOM_VIEW_ORIENTATION_CHANGED, orientation).sendToTarget();
    }

    /**
     * clear the invalid view such as indicator view.
     */
    public void clearInvalidView() {
        mMainHandler.removeMessages(ZOOM_VIEW_RESET);
        mMainHandler.removeMessages(ZOOM_VIEW_SHOW);
        mApp.getAppUi().setUIVisibility(IAppUi.INDICATOR, View.INVISIBLE);
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
                    show((String) msg.obj);
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
                default:
                    break;
            }
        }
    }

    private void show(String msg) {
        if (mZoomView == null) {
            return;
        }
        mZoomIndicatorHint.mHintText = msg;
        mApp.getAppUi().showScreenHint(mZoomIndicatorHint);
    }

    private void hide() {
        if (mZoomView == null) {
            return;
        }
        mApp.getAppUi().hideScreenHint(mZoomIndicatorHint);
    }

    private void reset() {
        if (mZoomView == null) {
            return;
        }
        mApp.getAppUi().hideScreenHint(mZoomIndicatorHint);
        mApp.getAppUi().setUIVisibility(IAppUi.INDICATOR, View.VISIBLE);
    }

    private void initView() {
        mRootViewGroup = mApp.getAppUi().getModeRootView();
        mZoomView = (RotateLayout) mApp.getActivity().getLayoutInflater()
                .inflate(R.layout.zoom_view,
                mRootViewGroup, false).findViewById(R.id.zoom_rotate_layout);
        mTextView = (TextView) mZoomView.findViewById(R.id.zoom_ratio);
        mRootViewGroup.addView(mZoomView);

        mZoomIndicatorHint = new IAppUi.HintInfo();
        mZoomIndicatorHint.mType = IAppUi.HintType.TYPE_AUTO_HIDE;
        mZoomIndicatorHint.mDelayTime = ZOOM_VIEW_HIDE_DELAY_TIME;
    }

    private void unInitView() {
        mRootViewGroup.removeView(mZoomView);
        mZoomView = null;
    }

    private void updateOrientation(int orientation) {
        RelativeLayout.LayoutParams params =
                (RelativeLayout.LayoutParams) mTextView.getLayoutParams();
        switch (orientation) {
            case 0:
                if (CameraUtil.isTablet()) {
                    params.setMargins(params.leftMargin, dpToPixel(ZOOM_TEXT_MARGION_VERTICAL_TABLET),
                            params.rightMargin, params.bottomMargin);
                } else {
                    params.setMargins(params.leftMargin, dpToPixel(ZOOM_TEXT_MARGION_VERTICAL),
                            params.rightMargin, params.bottomMargin);
                }
                break;
            case 180:
                params.setMargins(params.leftMargin, dpToPixel(ZOOM_TEXT_MARGION_VERTICAL_REVERSE),
                        params.rightMargin, params.bottomMargin);
                break;
            case 90:
            case 270:
                params.setMargins(params.leftMargin, dpToPixel(ZOOM_TEXT_MARGION_HORIZON),
                        params.rightMargin, params.bottomMargin);
                break;
            default:
                break;
        }
        mTextView.setLayoutParams(params);
        CameraUtil.rotateRotateLayoutChildView(mApp.getActivity(), mZoomView, orientation, true);
    }

    private int dpToPixel(int dp) {
        float scale = mApp.getActivity().getResources().getDisplayMetrics().density;
        return (int) (dp * scale + 0.5f);
    }
}
