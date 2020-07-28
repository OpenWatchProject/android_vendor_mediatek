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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

package com.mediatek.camera.v2.stream;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.provider.MediaStore;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.android.camera.R;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.stream.IRecordStream.RecordStreamStatus;

import java.util.Locale;

public class RecordStreamView {
    private static final Tag TAG = new Tag(RecordStreamView.class.getSimpleName());
    private final IRecordStream     mRecordingController;
    private final RecordingCallback          mRecordingCallback;
    private final Activity                   mActivity;
    private final Handler                    mMainHandler;
    private final ViewGroup                  mParentViewGroup;
    private final boolean                    mIsCaptureIntent;

    private static final int                 MSG_UPDATE_RECORD_TIME = 0;

    private OnClickListener                  mPauseResumeClickListener = new OnClickListener() {
        @Override
        public void onClick(View view) {
            if (mRecording) {
                if (mRecordingPaused) {
                    mRecordingController.resumeRecord();
                    mRecordingStartTime = SystemClock.uptimeMillis() - mRecordingPausedDuration;
                    mRecordingPausedDuration = 0;
                    mRecordingPaused = false;
                    updateRecordingViewIcon();
                } else {
                    mRecordingController.pauseRecord();
                    mRecordingPausedDuration = SystemClock.uptimeMillis() - mRecordingStartTime;
                    mRecordingPaused = true;
                    updateRecordingViewIcon();
                }
            }
        }
    };
    // recording flow
    private boolean                          mRecording;
    private boolean                          mRecordingPaused;
    private int                              mShowRecordingTimeViewIndicator = 0;
    private long                             mRecordingStartTime;
    private long                             mRecordingTotalDuration;
    private long                             mRecordingPausedDuration;

    private View                             mRecordingRootView = null;
    // recording time view group
    private View                             mRecordingTimeViewGroup;
    private TextView                         mRecordingTimeView;
    private ImageView                        mPauseResumeButton;
    // recording size view group
    private View                             mRecordingSizeViewGroup;
    private SeekBar                          mRecrodingSizeSeekBar;
    private TextView                         mCurrentRecordingSizeView;
    private TextView                         mRecordingSizeTotalView;
    private long                             mLimitSize ;
    public RecordStreamView(Activity activity,
            IRecordStream recordingStream,
            ViewGroup parentViewGroup,
            boolean isCaptureIntent) {
        mActivity            = activity;
        mMainHandler         = new RecordingHandler(mActivity.getMainLooper());
        mParentViewGroup     = parentViewGroup;
        mIsCaptureIntent     = isCaptureIntent;
        mRecordingCallback   = new RecordingCallback();
        mRecordingController = recordingStream;
        mRecordingController.registerRecordingObserver(mRecordingCallback);
        mLimitSize = mActivity.getIntent().getLongExtra(MediaStore.EXTRA_SIZE_LIMIT, 0L);
    }

    public void close() {
        mRecordingController.unregisterCaptureObserver(mRecordingCallback);
    }

    private class RecordingHandler extends Handler {
        public RecordingHandler(Looper looper) {
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

    private class RecordingCallback implements RecordStreamStatus {
        @Override
        public void onRecordingStarted(boolean canUseExtra) {
            LogHelper.i(TAG, "onRecordingStarted canUseExtra = " + canUseExtra);
            mRecording = true;
            mRecordingPaused = false;
            mRecordingPausedDuration = 0;
            mRecordingTotalDuration = 0;
            show(canUseExtra);
            if (!canUseExtra) {
                mRecordingStartTime = SystemClock.uptimeMillis();
                updateRecordingTime();
            }
        }

        @Override
        public void onRecordingStoped() {
            LogHelper.i(TAG, "onRecordingStoped");
            mRecording = false;
            mMainHandler.removeMessages(MSG_UPDATE_RECORD_TIME);
            hide();
        }

        @Override
        public void onInfo(int what, int extra) {
            if (what == IRecordStream.MEDIA_RECORDER_INFO_START_TIMER) {
                mRecordingStartTime = SystemClock.uptimeMillis();
                updateRecordingTime();
            }
            if (what == IRecordStream.MEDIA_RECORDER_INFO_RECORDING_SIZE) {
                if (0 < mLimitSize) {
                    int progress = (int) (extra * 100 / mLimitSize);
                    LogHelper.i(TAG, "extra = " + extra + " : mLimitSize = "
                            + mLimitSize + "  : progress = " + progress);
                    if (100 >= progress) {
                        mCurrentRecordingSizeView.setText(formatFileSize(extra));
                        mRecrodingSizeSeekBar.setProgress(progress);
                    }
                }
            }
        }

        @Override
        public void onError(int what, int extra) {
            mRecording = false;
            mMainHandler.removeMessages(MSG_UPDATE_RECORD_TIME);
            hide();
        }
    }

    private View getView() {
        LogHelper.i(TAG, "getView");
        // inflate recoding view layout
        View viewLayout = mActivity.getLayoutInflater().inflate(R.layout.recording_ext_v2,
                mParentViewGroup, true);

        View rootView = viewLayout.findViewById(R.id.recording_root_group);

        // initialize recording time group
        mRecordingTimeViewGroup = viewLayout.findViewById(R.id.recording_time_group);
        mRecordingTimeView = (TextView) viewLayout.findViewById(R.id.recording_time);
        mPauseResumeButton = (ImageView) viewLayout.findViewById(R.id.btn_pause_resume);
        mPauseResumeButton.setOnClickListener(mPauseResumeClickListener);

        // initialize recording time size group
        mRecordingSizeViewGroup = (ViewGroup) viewLayout.findViewById(R.id.recording_size_group);
        mCurrentRecordingSizeView = (TextView) viewLayout.findViewById(R.id.recording_current);
        mRecrodingSizeSeekBar = (SeekBar) viewLayout.findViewById(R.id.recording_progress);
        mRecordingSizeTotalView = (TextView) viewLayout.findViewById(R.id.recording_total);
        mRecrodingSizeSeekBar.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motion) {
                // disable seek bar
                return true;
            }
        });
        //TODO set seek bar's setMax and setProgress ????
        return rootView;
    }

    private void show(boolean canUseExtra) {
        LogHelper.i(TAG, "show");
        if (mRecordingRootView == null) {
            mRecordingRootView = getView();
        }
        updateRecordingViewIcon();
        mRecordingRootView.setVisibility(View.VISIBLE);
        /**Recording View Group**/
        mRecordingTimeViewGroup.setVisibility(View.VISIBLE);
        mRecordingTimeView.setText(formatTime(0L, false));
        mRecordingTimeView.setVisibility(View.VISIBLE);
        mPauseResumeButton.setVisibility(View.VISIBLE);

        /**Recording Size Group**/
        if (mIsCaptureIntent && (mLimitSize > 0) && canUseExtra) {
            mCurrentRecordingSizeView.setText("0");
            mRecrodingSizeSeekBar.setProgress(0);
            mRecordingSizeTotalView.setText(formatFileSize(mLimitSize));
            mRecordingSizeViewGroup.setVisibility(View.VISIBLE);
        } else {
            mRecordingSizeViewGroup.setVisibility(View.GONE);
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
        mPauseResumeButton.setVisibility(View.INVISIBLE);
        /**Recording Size Group**/
        mRecordingSizeViewGroup.setVisibility(View.INVISIBLE);

        mParentViewGroup.removeView(mRecordingRootView);
        mRecordingRootView = null;
    }

    private void updateRecordingViewIcon() {
        int recordingId = R.drawable.ic_recording_indicator_play;
        int playpauseId = R.drawable.ic_recording_pause;
        if (mRecordingPaused) {
            recordingId = R.drawable.ic_recording_indicator_pause;
            playpauseId = R.drawable.ic_recording_play;
        }
        mRecordingTimeView.setCompoundDrawablesWithIntrinsicBounds(mActivity.getResources()
                .getDrawable(recordingId), null, null, null);
        mPauseResumeButton.setImageResource(playpauseId);
    }

    private void showTime(long millis, boolean showMillis) {
        String timeText = formatTime(millis, showMillis);
        if (mRecordingTimeView != null) {
            mRecordingTimeView.setText(timeText);
        }
    }

    private void updateRecordingTime() {
        if (!mRecording) {
            return;
        }
        long now = SystemClock.uptimeMillis();
        mRecordingTotalDuration = now - mRecordingStartTime;
        if (mRecordingPaused) {
            mRecordingTotalDuration = mRecordingPausedDuration;
        }
        long targetNextUpdateDelay = 1000;
        showTime(mRecordingTotalDuration, false);
        mShowRecordingTimeViewIndicator = 1 - mShowRecordingTimeViewIndicator;
        if (mRecordingPaused && 1 == mShowRecordingTimeViewIndicator) {
            mRecordingTimeView.setVisibility(View.INVISIBLE);
        } else {
            mRecordingTimeView.setVisibility(View.VISIBLE);
        }
        long actualNextUpdateDelay = 500;
        if (!mRecordingPaused) {
            actualNextUpdateDelay = targetNextUpdateDelay
                    - (mRecordingTotalDuration % targetNextUpdateDelay);
        }
        LogHelper.d(TAG, "[updateRecordingTime()],actualNextUpdateDelay = "
                + actualNextUpdateDelay);
        mMainHandler.sendEmptyMessageDelayed(MSG_UPDATE_RECORD_TIME, actualNextUpdateDelay);
    }

    private String formatFileSize(long size) {
        long kb = size / 1024;
        return kb + "K"; // Formatter.formatFileSize(getContext(), size);
    }

    private String formatTime(long millis, boolean showMillis) {
        final int totalSeconds = (int) millis / 1000;
        final int millionSeconds = (int) (millis % 1000) / 10;
        final int seconds = totalSeconds % 60;
        final int minutes = (totalSeconds / 60) % 60;
        final int hours = totalSeconds / 3600;
        String text = null;
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
}