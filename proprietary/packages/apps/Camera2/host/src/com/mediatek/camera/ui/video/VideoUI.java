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
package com.mediatek.camera.ui.video;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SeekBar;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.mode.video.videoui.IVideoUI;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.widget.RotateStrokeTextView;
import com.mediatek.camera.common.widget.ScaleAnimationButton;
import com.mediatek.camera.common.widget.StrokeTextView;

import java.util.Locale;

/**
 * Video recording ui used to show recording time,vss button and pause resume button and maybe
 * recording progress.
 * User can through combination to implement new UI.
 */
public class VideoUI implements IVideoUI {

    private static final Tag TAG = new Tag(VideoUI.class.getSimpleName());

    private static final int PERCENTAGE_FOR_PROGRESS = 100;
    private static final int COMPUTER_NUMBER_SYSTEM = 1024;
    private static final int MSG_UPDATE_RECORD_TIME = 0;
    private static final int ONE_SECOND_TO_MS = 1000;
    private static final int HALF_SECOND_TO_MS = 500;
    private static final int ORIENTATION_270 = 270;
    private static final int ORIENTATION_180 = 180;
    private static final int ORIENTATION_90 = 90;
    private static final int ORIENTATION_0 = 0;

    private static final int SHOW_HINT_TIME = 3000;
    private static final int ONE_HOUR_TO_S = 3600;
    private static final int TIME_BASE = 60;

    private static final int DEFAULT_FPS = 30;

    private final Handler mMainHandler;

    private int mShowRecordingTimeViewIndicator = 0;
    private long mRecordingPausedDuration = 0;
    private long mRecordingTotalDuration = 0;
    private long mRecordingStartTime = 0;

    private boolean mIsInRecording = false;
    private View mRecordingRootView = null;

    private VideoUIState mUIState = VideoUIState.STATE_PREVIEW;
    private RotateStrokeTextView mCurrentRecordingSizeView;
    private RotateStrokeTextView mRecordingSizeTotalView;
    private SeekBar mRecordingSizeSeekBar;
    private View mRecordingSizeViewGroup;
    private View mRecordingTimeViewGroup;
    private ScaleAnimationButton mPauseResumeButton;
    private StrokeTextView mRecordingTimeView;
    private StrokeTextView mRecordingFpsView;
    private ViewGroup mParentViewGroup;
    private ImageView mRecordingIndicator;
    private ScaleAnimationButton mStopButton;
    private ScaleAnimationButton mVssButton;
    private ScaleAnimationButton mBurstButton;
    private Activity mActivity;
    private UISpec mUISpec;
    private IApp mApp;
    private IAppUi.HintInfo mVideoErrorHint;

    /**
     * VideoUI used for recording ui and other feature
     * can override it and show it self ui.
     * @param app the camera app
     * @param parentViewGroup camera root view
     */
    public VideoUI(IApp app, ViewGroup parentViewGroup) {
        mApp = app;
        mActivity = app.getActivity();
        mParentViewGroup = parentViewGroup;
        mMainHandler = new RecordingHandler(mActivity.getMainLooper());
        mVideoErrorHint = new IAppUi.HintInfo();
        int id = app.getActivity().getResources().getIdentifier("hint_text_background",
                "drawable", app.getActivity().getPackageName());
        mVideoErrorHint.mBackground = app.getActivity().getDrawable(id);
        mVideoErrorHint.mType = IAppUi.HintType.TYPE_AUTO_HIDE;
        mVideoErrorHint.mDelayTime = SHOW_HINT_TIME;
    }

    @Override
    public void initVideoUI(UISpec spec) {
        mUISpec = spec;
    }

    @Override
    public void updateUIState(VideoUIState state) {
        LogHelper.e(TAG, "[updateUIState] mUIState = " + mUIState + " new state = " + state);
        switch (state) {
            case STATE_PREVIEW:
                doUpdateUI(state);
                break;
            case STATE_PRE_RECORDING:
                if (mUIState == VideoUIState.STATE_PREVIEW) {
                    doUpdateUI(state);
                }
                break;
            case STATE_RECORDING:
                if (mUIState == VideoUIState.STATE_PRE_RECORDING) {
                    doUpdateUI(state);
                }
                break;
            case STATE_PAUSE_RECORDING:
                if (mUIState == VideoUIState.STATE_RECORDING) {
                    doUpdateUI(state);
                }
                break;
            case STATE_RESUME_RECORDING:
                if (mUIState == VideoUIState.STATE_PAUSE_RECORDING) {
                    doUpdateUI(state);
                    enableBurst();
                    break;
                } else if (mUIState == VideoUIState.STATE_BURST_RECORDING) {
                    doUpdateUI(state);
                    break;
                }
                break;
            case STATE_BURST_RECORDING:
                if (mUIState == VideoUIState.STATE_RECORDING) {
                    doUpdateUI(state);
                }
                break;
            case STATE_RESUME_RECORDING_AFTER_BURST:
                if (mUIState == VideoUIState.STATE_BURST_RECORDING) {
                    doUpdateUI(state);
                }
                break;
            default:
                break;
        }

    }

    @Override
    public void updateOrientation(int orientation) {
        LogHelper.d(TAG, "[updateOrientation] orientation = " + orientation
                     + " mUIState = " + mUIState);
        if (mUIState == VideoUIState.STATE_PREVIEW) {
            return;
        }
        switch (orientation) {
            case ORIENTATION_0:
            case ORIENTATION_90:
            case ORIENTATION_180:
            case ORIENTATION_270:
                CameraUtil.rotateRotateLayoutChildView(mActivity, mRecordingRootView,
                        orientation, true);
                break;
            default:
                LogHelper.e(TAG, "error orientation = " + orientation);
        }
    }

    @Override
    public void updateRecordingSize(long size) {
        LogHelper.d(TAG, "[updateRecordingSize] mUIState = " + mUIState);
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mUIState == VideoUIState.STATE_PREVIEW) {
                    return;
                }
                if (size < 0 || size > mUISpec.recordingTotalSize) {
                    LogHelper.e(TAG, "[updateRecordingSize] size = " + size);
                    return;
                  //  throw new IllegalArgumentException();
                }
                int progress =
                        (int) (size * PERCENTAGE_FOR_PROGRESS / mUISpec.recordingTotalSize);
                if (PERCENTAGE_FOR_PROGRESS >= progress) {
                    mCurrentRecordingSizeView.setText(formatFileSize(size));
                    mRecordingSizeSeekBar.setProgress(progress);
                }
            }
        });
    }

    @Override
    public void enableBurst() {
        LogHelper.d(TAG, "[enableBurst]");
        if (mUISpec.isSupportedBurst == false) {
            LogHelper.d(TAG, "[enableBurst] not support burst, return");
            return;
        }
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mBurstButton.setEnabled(true);
            }
        });
    }

    @Override
    public void unInitVideoUI() {
        mParentViewGroup.removeView(mRecordingRootView);
        mRecordingRootView = null;
        mUISpec = null;
    }

    /**
     * this function used to show info when recording.
     *
     * @param infoId the info id which tell implement show which info.
     */
    @Override
    public void showInfo(int infoId) {
        LogHelper.i(TAG, "[showInfo] infoId = " + infoId);
        switch (infoId) {
            case VIDEO_BAD_PERFORMANCE_AUTO_STOP:
                mVideoErrorHint.mHintText = mActivity.getString(
                        R.string.video_bad_performance_auto_stop);
                mApp.getAppUi().showScreenHint(mVideoErrorHint);
                break;
            case VIDEO_RECORDING_NOT_AVAILABLE:
                mVideoErrorHint.mHintText = mActivity.getString(
                        R.string.video_reach_size_limit);
                mApp.getAppUi().showScreenHint(mVideoErrorHint);
                break;
            case VIDEO_RECORDING_ERROR:
                mVideoErrorHint.mHintText = mActivity.getString(
                        R.string.video_recording_error);
                mApp.getAppUi().showScreenHint(mVideoErrorHint);
                break;
            case REACH_SIZE_LIMIT:
                mVideoErrorHint.mHintText = mActivity.getString(
                        R.string.video_reach_size_limit);
                mApp.getAppUi().showScreenHint(mVideoErrorHint);
                break;
                default:
                    break;
        }
    }

    private void doUpdateUI(VideoUIState state) {
        mUIState = state;
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                updateUI();
            }
        });
    }

    /**
     * use to handle some thing.
     */
    private class RecordingHandler extends Handler {
        /**
         * the construction method.
         */
        RecordingHandler(Looper looper) {
            super(looper);
        }
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_UPDATE_RECORD_TIME:
                    updateRecordingTime();
                    break;
                default:
                    break;
            }
        }
    }

    private void updateRecordingTime() {
        if (!mIsInRecording) {
            return;
        }
        mRecordingTotalDuration = SystemClock.uptimeMillis() - mRecordingStartTime;
        if (VideoUIState.STATE_PAUSE_RECORDING == mUIState) {
            mRecordingTotalDuration = mRecordingPausedDuration;
        }
        long targetNextUpdateDelay = ONE_SECOND_TO_MS;
        showTime(mRecordingTotalDuration, false);
        mShowRecordingTimeViewIndicator = 1 - mShowRecordingTimeViewIndicator;
        if (VideoUIState.STATE_PAUSE_RECORDING == mUIState &&
                1 == mShowRecordingTimeViewIndicator) {
            mRecordingTimeViewGroup.setVisibility(View.INVISIBLE);
        } else {
            mRecordingTimeViewGroup.setVisibility(View.VISIBLE);
        }
        long actualNextUpdateDelay = HALF_SECOND_TO_MS;
        if (VideoUIState.STATE_PAUSE_RECORDING != mUIState) {
            actualNextUpdateDelay = targetNextUpdateDelay
                    - (mRecordingTotalDuration % targetNextUpdateDelay);
        }
        mMainHandler.sendEmptyMessageDelayed(MSG_UPDATE_RECORD_TIME, actualNextUpdateDelay);
    }

    private void showTime(long millis, boolean showMillis) {
        String timeText = formatTime(millis, showMillis);
        if (mRecordingTimeView != null) {
            mRecordingTimeView.setText(timeText);
        }
        if (mRecordingFpsView != null && mUISpec.isSupportedBurst) {
            if (mUIState == VideoUIState.STATE_BURST_RECORDING) {
                mRecordingFpsView.setText(formatFps(mUISpec.burstFps));
            } else {
                mRecordingFpsView.setText(formatFps(mUISpec.normalFps));
            }
        }
    }

    private String formatFps(int fps) {
        return "(" + fps + "fps)";
    }

    private String formatTime(long millis, boolean showMillis) {
        final int totalSeconds = (int) millis / ONE_SECOND_TO_MS;
        final int millionSeconds = (int) (millis % ONE_SECOND_TO_MS) / 10;
        final int seconds = totalSeconds % TIME_BASE;
        final int minutes = (totalSeconds / TIME_BASE) % TIME_BASE;
        final int hours = totalSeconds / ONE_HOUR_TO_S;
        String text;
        if (showMillis) {
            if (hours > 0) {
                text = String.format(Locale.ENGLISH, "%d:%02d:%02d.%02d", hours, minutes, seconds,
                        millionSeconds);
            } else {
                text = String.format(Locale.ENGLISH, "%02d:%02d.%02d", minutes, seconds,
                        millionSeconds);
            }
        } else {
            if (hours > 0) {
                text = String.format(Locale.ENGLISH, "%d:%02d:%02d", hours, minutes, seconds);
            } else {
                text = String.format(Locale.ENGLISH, "%02d:%02d", minutes, seconds);
            }
        }
        return text;
    }

    private void updateUI() {
        switch (mUIState) {
            case STATE_PREVIEW:
                mIsInRecording = false;
                mMainHandler.removeMessages(MSG_UPDATE_RECORD_TIME);
                hide();
                break;
            case STATE_PRE_RECORDING:
                mRecordingStartTime = SystemClock.uptimeMillis();
                mRecordingPausedDuration = 0;
                show();
                break;
            case STATE_RECORDING:
                mIsInRecording = true;
                mRecordingStartTime = SystemClock.uptimeMillis();
                updateRecordingTime();
                updateRecordingViewIcon();
                break;
            case STATE_PAUSE_RECORDING:
                mRecordingPausedDuration = SystemClock.uptimeMillis() - mRecordingStartTime;
                updateRecordingViewIcon();
                break;
            case STATE_RESUME_RECORDING:
                mRecordingStartTime = SystemClock.uptimeMillis() - mRecordingPausedDuration;
                mRecordingPausedDuration = 0;
                mUIState = VideoUIState.STATE_RECORDING;
                updateRecordingViewIcon();
                break;
            case STATE_BURST_RECORDING:
                updateRecordingViewIcon();
                break;
            case STATE_RESUME_RECORDING_AFTER_BURST:
                mUIState = VideoUIState.STATE_RECORDING;
                updateRecordingViewIcon();
                break;
            default:
                break;
        }
    }

    private void hide() {
        if (mRecordingRootView == null) {
            return;
        }
        mMainHandler.removeMessages(MSG_UPDATE_RECORD_TIME);
        mRecordingRootView.setVisibility(View.INVISIBLE);
        /**Recording View Group**/
        mRecordingTimeViewGroup.setVisibility(View.INVISIBLE);
        mRecordingTimeView.setVisibility(View.INVISIBLE);
        mRecordingFpsView.setVisibility(View.INVISIBLE);
        mPauseResumeButton.setVisibility(View.INVISIBLE);
        /**Recording Size Group**/
        mRecordingSizeViewGroup.setVisibility(View.INVISIBLE);

        mParentViewGroup.removeView(mRecordingRootView);
        mRecordingRootView = null;
    }

    private void show() {
        LogHelper.d(TAG, "[show] + mRecordingRootView = " + mRecordingRootView);
        if (mRecordingRootView == null) {
            mRecordingRootView = getView();
        }
        updateRecordingViewIcon();
        mRecordingRootView.setVisibility(View.VISIBLE);
        /**Recording View Group**/
        mRecordingTimeViewGroup.setVisibility(View.VISIBLE);
        mRecordingTimeView.setText(formatTime(0L, false));
        mRecordingTimeView.setVisibility(View.VISIBLE);
        if (mUISpec.isSupportedBurst) {
            mRecordingFpsView.setText(formatFps(DEFAULT_FPS));
            mRecordingFpsView.setVisibility(View.VISIBLE);
        }
        if (mUISpec.isSupportedPause) {
            mPauseResumeButton.setVisibility(View.VISIBLE);
        } else {
            mPauseResumeButton.setVisibility(View.GONE);
        }
        if (mUISpec.isSupportedVss) {
            mVssButton.setVisibility(View.VISIBLE);
        } else {
            mVssButton.setVisibility(View.GONE);
        }
        if (mUISpec.isSupportedBurst) {
            mBurstButton.setVisibility(View.VISIBLE);
        } else {
            mBurstButton.setVisibility(View.GONE);
        }
        /**Recording Size Group**/
        if (mUISpec.recordingTotalSize > 0) {
            mCurrentRecordingSizeView.setText("0");
            mRecordingSizeSeekBar.setProgress(0);
            mRecordingSizeTotalView.setText(formatFileSize(mUISpec.recordingTotalSize));
            mRecordingSizeViewGroup.setVisibility(View.VISIBLE);
        } else {
            mRecordingSizeViewGroup.setVisibility(View.GONE);
        }
        LogHelper.d(TAG, "[show] - ");
    }

    private String formatFileSize(long size) {
        long kb = size / COMPUTER_NUMBER_SYSTEM;
        return kb + "K"; // Formatter.formatFileSize(getContext(), size);
    }

    private View getView() {
        LogHelper.d(TAG, "[getView] +");
        // inflate recoding view layout
        View viewLayout = mActivity.getLayoutInflater().inflate(R.layout.recording,
                mParentViewGroup, true);
        View rootView = viewLayout.findViewById(R.id.recording_root_group);

        // initialize recording time group
        mRecordingTimeViewGroup = viewLayout.findViewById(R.id.recording_time_group);
        mRecordingTimeView = (StrokeTextView) viewLayout.findViewById(R.id.recording_time);
        mRecordingFpsView = (StrokeTextView) viewLayout.findViewById(R.id.recording_fps);
        mRecordingIndicator = (ImageView) viewLayout.findViewById(R.id.recording_indicator);
        mPauseResumeButton = (ScaleAnimationButton) viewLayout.findViewById(R.id.btn_pause_resume);
        mPauseResumeButton.setOnClickListener(mUISpec.pauseResumeListener);
        mStopButton = (ScaleAnimationButton) viewLayout.findViewById(R.id.video_stop_shutter);
        mStopButton.setOnClickListener(mUISpec.stopListener);
        mVssButton = (ScaleAnimationButton) viewLayout.findViewById(R.id.btn_vss);
        mVssButton.setOnClickListener(mUISpec.vssListener);
        mBurstButton = (ScaleAnimationButton) viewLayout.findViewById(R.id.btn_burst);
        mBurstButton.setOnClickListener(mUISpec.burstListener);
        // initialize recording time size group
        mRecordingSizeViewGroup = viewLayout.findViewById(R.id.recording_size_group);
        mCurrentRecordingSizeView = (RotateStrokeTextView) viewLayout.findViewById(
                                                           R.id.recording_current);
        mRecordingSizeSeekBar = (SeekBar) viewLayout.findViewById(R.id.recording_progress);
        mRecordingSizeTotalView =
                (RotateStrokeTextView) viewLayout.findViewById(R.id.recording_total);
        mRecordingSizeSeekBar.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motion) {
                // disable seek bar
                return true;
            }
        });
        LogHelper.d(TAG, "[getView] - ");
        return rootView;
    }

    private void updateRecordingViewIcon() {
        int recordingId = R.drawable.ic_recording_indicator;
        int playPauseId = R.drawable.ic_pause_recording;
        if (mUIState == VideoUIState.STATE_PAUSE_RECORDING) {
            recordingId = R.drawable.ic_pause_indicator;
            playPauseId = R.drawable.ic_resume_recording;
        }
        mRecordingIndicator.setImageResource(recordingId);
        mPauseResumeButton.setImageResource(playPauseId);

        if (mUISpec.isSupportedBurst) {
            if (mUIState == VideoUIState.STATE_PAUSE_RECORDING) {
                mBurstButton.setEnabled(false);
            } else if (mUIState == VideoUIState.STATE_BURST_RECORDING) {
                mPauseResumeButton.setEnabled(false);
                mStopButton.setEnabled(false);
                mBurstButton.setEnabled(false);
            } else if (mUIState == VideoUIState.STATE_RECORDING) {
                mPauseResumeButton.setEnabled(true);
                mStopButton.setEnabled(true);
            }
        }
    }
}

