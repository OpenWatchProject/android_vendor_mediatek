/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.selftimer;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.widget.RotateImageView;
import com.mediatek.camera.common.widget.RotateStrokeTextView;

import java.util.Locale;

/**
 * This class is for self timer feature view controller.
 */

public class SelfTimerCtrl {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(SelfTimerCtrl.class.getSimpleName());
    private MainHandler mMainHandler;
    private IApp mApp;
    private IAppUi mAppUi;
    private SelfTimerSettingView mSelfTimerSettingView = new SelfTimerSettingView();
    private RotateImageView mSelfTimerIndicatorView;
    private RotateStrokeTextView mSelfTimerTextView;
    private ViewGroup mRootViewGroup;
    private View mSelfTimerView;
    private SelfTimerSoundManager mSoundManager;
    private Locale mLocale;
    private Animation mCountDownAnim;
    private volatile int mCurSelfTimerNum;
    private int mStartSelfTimerNum;
    private ISelfTimerViewListener.OnSelfTimerListener mSelfTimerListener;

    // View control
    private static final int VIEW_PRIORITY = 6;
    private static final int UPDATE_DELAY_TIME = 1000;
    private static final int INDICATOR_VIEW_INIT = 0;
    private static final int INDICATOR_VIEW_UNINIT = 1;
    private static final int SELF_TIMER_VIEW_SHOW = 2;
    private static final int SELF_TIMER_VIEW_HIDE = 3;
    private static final int SELF_TIMER_UPDATE = 4;
    private static final int SELF_TIMER_ORIENTATION_UPDATE = 5;

    /**
     * Init the view.
     * @param app The camera activity.
     */
    public void init(IApp app) {
        mApp = app;
        mAppUi = app.getAppUi();
        mMainHandler = new MainHandler(app.getActivity().getMainLooper());
        mSoundManager = new SelfTimerSoundManager(app);
        mLocale = app.getActivity().getResources().getConfiguration().locale;
        mCountDownAnim = AnimationUtils.loadAnimation(app.getActivity(), R.anim.count_down_exit);
    }

    /**
     * Used to destroy the self timer view.
     */
    public void unInit() {
        mMainHandler.sendEmptyMessage(INDICATOR_VIEW_UNINIT);
        mMainHandler.sendEmptyMessage(SELF_TIMER_VIEW_HIDE);
        mMainHandler.removeMessages(SELF_TIMER_UPDATE);
        mMainHandler.removeMessages(SELF_TIMER_ORIENTATION_UPDATE);
        mSoundManager.release();
    }

    /**
     * This is to get self timer setting view.
     * @param listener the instance of self timer listener.
     */
    public void setSelfTimerListener(ISelfTimerViewListener.OnSelfTimerListener listener) {
        mSelfTimerListener = listener;
    }

    /**
     * This is to get self timer setting view.
     * @return the instance of self timer setting View.
     */
    public SelfTimerSettingView getSelfTimerSettingView() {
        return mSelfTimerSettingView;
    }

    /**
     * This is to init res by switch, such as indicator view/loading sound.
     * @param value the switch value.
     */
    public void initResBySwitch(String value) {
        if (ISelfTimerViewListener.OFF.equals(value)) {
            mMainHandler.sendEmptyMessage(INDICATOR_VIEW_UNINIT);
            mSoundManager.release();
        } else {
            mMainHandler.obtainMessage(INDICATOR_VIEW_INIT, value).sendToTarget();
            mSoundManager.load();
        }
    }

    /**
     * This is to show self timer view.
     *
     * @param value the indicator view value.
     */
    public void showSelfTimerView(String value) {
        if (ISelfTimerViewListener.OFF.equals(value)) {
            mMainHandler.sendEmptyMessage(SELF_TIMER_VIEW_HIDE);
        } else {
            mMainHandler.obtainMessage(SELF_TIMER_VIEW_SHOW, value).sendToTarget();
        }
    }

    /**
     * when phone orientation changed, the self timer view will be updated.
     * @param orientation the orientation of g-sensor.
     */
    public void onOrientationChanged(int orientation) {
        if (mMainHandler == null) {
            return;
        }
        mMainHandler.obtainMessage(SELF_TIMER_ORIENTATION_UPDATE, orientation).sendToTarget();
    }

    /**
     * when phone orientation changed, the self timer view will be updated.
     * @return  orientation the orientation of g-sensor.
     */
    public boolean onInterrupt() {
        if (mMainHandler == null) {
            return false;
        }
        if (mCurSelfTimerNum > 0) {
            mSoundManager.stop();
            mMainHandler.sendEmptyMessage(SELF_TIMER_VIEW_HIDE);
            mSelfTimerListener.onTimerInterrupt();
            return true;
        }
        return false;
    }

    /**
     * main thread for view behavior.
     */
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case INDICATOR_VIEW_INIT:
                    initIndicatorView((String) msg.obj);
                    break;
                case INDICATOR_VIEW_UNINIT:
                    unInitIndicatorView();
                    break;
                case SELF_TIMER_VIEW_SHOW:
                    startSelfTimer((String) msg.obj);
                    break;
                case SELF_TIMER_VIEW_HIDE:
                    stopSelfTimer();
                    break;
                case SELF_TIMER_UPDATE:
                    updateSelfTimer();
                    break;
                case SELF_TIMER_ORIENTATION_UPDATE:
                    updateOrientation((Integer) msg.obj);
                    break;
                default:
                    break;
            }
        }
    }

    private void initIndicatorView(String value) {
        if (mSelfTimerIndicatorView == null) {
            mSelfTimerIndicatorView
                    = (RotateImageView) mApp.getActivity().getLayoutInflater().inflate(
                    R.layout.self_timer_indicator, null);
        }
        if (ISelfTimerViewListener.TWO_SECONDS.equals(value)) {
            mSelfTimerIndicatorView.setImageResource(R.drawable.ic_selftimer_indicator_2);
        } else {
            mSelfTimerIndicatorView.setImageResource(R.drawable.ic_selftimer_indicator_10);
        }
        mAppUi.addToIndicatorView(mSelfTimerIndicatorView, VIEW_PRIORITY);
    }

    private void unInitIndicatorView() {
        if (mSelfTimerIndicatorView != null) {
            mAppUi.removeFromIndicatorView(mSelfTimerIndicatorView);
        }
    }

    private void updateOrientation(int orientation) {
        if (mSelfTimerTextView != null) {
            CameraUtil.rotateRotateLayoutChildView(mApp.getActivity(), mSelfTimerView,
                    orientation, true);
        }
    }

    private void startSelfTimer(String value) {
        mRootViewGroup = mApp.getAppUi().getModeRootView();
        mSelfTimerView = mApp.getActivity().getLayoutInflater().inflate(R.layout.self_timer_view,
                mRootViewGroup, true);
        mSelfTimerTextView
                = (RotateStrokeTextView) mSelfTimerView.findViewById(R.id.self_timer_num);
        CameraUtil.rotateRotateLayoutChildView(mApp.getActivity(), mSelfTimerView,
                mApp.getGSensorOrientation(), false);
        mSelfTimerTextView.setVisibility(View.VISIBLE);
        mStartSelfTimerNum = Integer.parseInt(value);
        mCurSelfTimerNum = mStartSelfTimerNum + 1;
        mSelfTimerListener.onTimerStart();
        mMainHandler.sendEmptyMessage(SELF_TIMER_UPDATE);
    }

    private void stopSelfTimer() {
        if (mSelfTimerView != null) {
            mSelfTimerTextView.setVisibility(View.INVISIBLE);
            mMainHandler.removeMessages(SELF_TIMER_UPDATE);
            mRootViewGroup.removeView(mSelfTimerView);
            mSelfTimerListener.onTimerInterrupt();
            mStartSelfTimerNum = 0;
            mCurSelfTimerNum = 0;
        }
    }

    private void updateSelfTimer() {
        mCurSelfTimerNum--;
        LogHelper.d(TAG, "[updateSelfTimer] mCurSelfTimerNum: " + mCurSelfTimerNum);
        if (mCurSelfTimerNum > 0) {
            String localizedValue = String.format(mLocale, "%d", mCurSelfTimerNum);
            mSelfTimerTextView.setText(localizedValue);
            // Fade-out animation
            mCountDownAnim.reset();
            mSelfTimerTextView.clearAnimation();
            mSelfTimerTextView.startAnimation(mCountDownAnim);
            if (mCurSelfTimerNum == 1) {
                mSoundManager.play(SelfTimerSoundManager.BEEP_TWICE);
            } else if (mCurSelfTimerNum <= 3) {
                mSoundManager.play(SelfTimerSoundManager.BEEP_ONCE);
            }
            mMainHandler.sendEmptyMessageDelayed(SELF_TIMER_UPDATE, UPDATE_DELAY_TIME);
        } else {
            mSoundManager.stop();
            mMainHandler.sendEmptyMessage(SELF_TIMER_VIEW_HIDE);
            mSelfTimerListener.onTimerDone();
        }
    }
}
