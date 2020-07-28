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

package com.android.camera.bridge;

import java.util.Locale;

import com.android.camera.R;
import com.android.camera.manager.ViewManager;

import android.app.Activity;
import android.media.AudioManager;
import android.media.SoundPool;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.TextView;

import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ISelfTimeManager;
import com.mediatek.camera.util.Log;

public class SelfTimerManager extends ViewManager implements ISelfTimeManager {
    private static final String TAG = "SelfTimerManager";

    private static final int MAX_DELEY_TIME = 10000; // sec
    // self timer related fields
    private static final int STATE_SELF_TIMER_IDLE = 0;
    private static final int STATE_SELF_TIMER_COUNTING = 1;
    private static final int STATE_SELF_TIMER_SNAP = 2;
    private static final int MSG_SELFTIMER_TIMEOUT = 9;

    private final Handler mHandler;
    private TextView mRemainingSecondsView;
    private Animation mCountDownAnim;
    private SoundPool mSoundPool;
    private Activity mContext;
    private ICameraAppUi mCameraUi;
    private SelfTimerListener mSelfTimerListener;

    private boolean mIsLowStorageTag = false;
    private boolean mNeedPlaySound = true;

    //private long mTimeSelfTimerStart;
    private int mSelfTimerDuration;
    private int mSelfTimerState;
    private int mBeepTwice;
    private int mBeepOnce;
    private int mRemainingSecs;

    public interface SelfTimerListener {
        void onTimerStart();

        void onTimerTimeout();

        void onTimerStop();
    }

    public SelfTimerManager(Activity context, ICameraAppUi cameraUi) {
        super((com.android.camera.CameraActivity) context);
        Log.i(TAG, "[SelfTimerManager] constractor begin");
        mContext = context;
        mCameraUi = cameraUi;
        // Use STREAM_MUSIC type so that the self timer sound will be play only out of speaker when
        // with a headset.
        mSoundPool = new SoundPool(1, AudioManager.STREAM_SYSTEM_ENFORCED, 0);
        mBeepOnce = mSoundPool.load(mContext, R.raw.beep_once, 1);
        mBeepTwice = mSoundPool.load(mContext, R.raw.beep_twice, 1);
        mHandler = new Handler(mContext.getMainLooper()) {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case MSG_SELFTIMER_TIMEOUT:
                    selfTimerTimeout(mRemainingSecs - 1);
                    break;
                default:
                    break;
                }
            }
        };
        Log.i(TAG, "[SelfTimerManager] constractor end");
    }

    @Override
    public boolean isSelfTimerEnabled() {
        return mSelfTimerDuration > 0;
    }

    @Override
    public void setSelfTimerDuration(String timeDelay) {
        int delay = Integer.valueOf(timeDelay);
        if (delay < 0 || delay > MAX_DELEY_TIME) {
            throw new RuntimeException("invalid self timer delay");
        }
        mSelfTimerDuration = delay;
    }

    @Override
    public void needPlaySound(boolean play) {
        mNeedPlaySound = play;
    }

    @Override
    public boolean startSelfTimer() {
        Log.w(TAG, "[startSelfTimer]mSelfTimerState = " + mSelfTimerState
                + ",mSelfTimerDuration = " + mSelfTimerDuration);
        if (mSelfTimerDuration > 0 && mSelfTimerState == STATE_SELF_TIMER_IDLE) {
            selfTimerStart();
            return true;
        } else if (mSelfTimerState == STATE_SELF_TIMER_COUNTING) {
            return true;
        }
        return false;
    }

    public void releaseSelfTimer() {
        mSoundPool.release();
        mSoundPool = null;
        mHandler.removeCallbacksAndMessages(null);
    }

    public synchronized void setTimerListener(SelfTimerListener listener) {
        mSelfTimerListener = listener;
    }

    public void setLowStorage() {
        Log.i(TAG, "[setLowStorage]...");
        mCameraUi.showRemaining();
        stopSelfTimer();
    }

    public synchronized void stopSelfTimer() {
        if (mSelfTimerState != STATE_SELF_TIMER_IDLE) {
            mHandler.removeMessages(MSG_SELFTIMER_TIMEOUT);
            mSelfTimerState = STATE_SELF_TIMER_IDLE;
            hideTimerView();
            if (mSelfTimerListener != null) {
                mSelfTimerListener.onTimerStop();
            }
        }
    }

    @Override
    public boolean isSelfTimerCounting() {
        return mSelfTimerState == STATE_SELF_TIMER_COUNTING;
    }

    @Override
    protected View getView() {
        mCountDownAnim = AnimationUtils.loadAnimation(mContext, R.anim.count_down_exit);
        View view = inflate(R.layout.count_down_to_capture);
        mRemainingSecondsView = (TextView) view.findViewById(R.id.remaining_seconds);
        return view;
    }

    private synchronized void selfTimerStart() {
        if (mSelfTimerState != STATE_SELF_TIMER_IDLE || mHandler.hasMessages(MSG_SELFTIMER_TIMEOUT)
                || mIsLowStorageTag) {
            Log.w(TAG, "[selfTimerStart]mSelfTimerState = " + mSelfTimerState
                    + ",mIsLowStorageTag = " + mIsLowStorageTag);
            return;
        }
        // mTimeSelfTimerStart = System.currentTimeMillis();
        mSelfTimerListener.onTimerStart();
        mSelfTimerState = STATE_SELF_TIMER_COUNTING;
        showTimerView();
        Log.i(TAG, "SelfTimer start");
        selfTimerTimeout(mSelfTimerDuration / 1000);
    }

    private void showTimerView() {
        show();
        mCameraUi.hideAllViews();
        mRemainingSecondsView.setVisibility(View.VISIBLE);
        mCameraUi.showInfo(mContext.getString(R.string.count_down_title_text), mSelfTimerDuration);
    }

    private synchronized void selfTimerTimeout(int newVal) {
        Log.i(TAG, "selfTimerTimeout: newVal = " + newVal);
        mRemainingSecs = newVal;
        if (newVal <= 0) {
            // Countdown has finished
            hideTimerView();
            mSelfTimerState = STATE_SELF_TIMER_SNAP;
            if (mSelfTimerListener != null) {
                Log.i(TAG, "onTimerTimeout");
                mSelfTimerListener.onTimerTimeout();
            }
            mSelfTimerState = STATE_SELF_TIMER_IDLE;
        } else {
            Locale locale = mContext.getResources().getConfiguration().locale;
            String localizedValue = String.format(locale, "%d", newVal);
            mRemainingSecondsView.setText(localizedValue);
            // Fade-out animation
            mCountDownAnim.reset();
            mRemainingSecondsView.clearAnimation();
            mRemainingSecondsView.startAnimation(mCountDownAnim);

            // Play sound effect for the last 3 seconds of the countdown
            if (mNeedPlaySound) {
                if (newVal == 1) {
                    mSoundPool.play(mBeepTwice, 1.0f, 1.0f, 0, 0, 1.0f);
                } else if (newVal <= 3) {
                    mSoundPool.play(mBeepOnce, 1.0f, 1.0f, 0, 0, 1.0f);
                }
            }

            // Schedule the next remainingSecondsChanged() call in 1 second
            mHandler.sendEmptyMessageDelayed(MSG_SELFTIMER_TIMEOUT, 1000);
        }
    }

    private void hideTimerView() {
        mRemainingSecondsView.setVisibility(View.INVISIBLE);
        hide();
        mCameraUi.showAllViews();
        mCameraUi.dismissInfo();
    }
}
