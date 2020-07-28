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

import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.R;

import java.util.Locale;

public class RecordingView extends ViewManager implements OnClickListener {
    private static final String TAG = "RecordingView";

    private TextView mRecordingTime;
    private ImageView mPauseResume;
    private boolean mRecordinging;
    private String mTimeText;
    private OnClickListener mListener;

    private static final int PROGRESS_MAX = 100;

    private TextView mRecordingSizeTotal;
    private TextView mRecordingSizeCurrent;
    private SeekBar mRecrodingSizeProgress;
    private View mRecordingSizeGroup;
    private long mCurrent;
    private long mTotal;
    private int mProgress;
    private int mMax = PROGRESS_MAX;

    private boolean mRecordingSizeVisible;
    private boolean mTimeVisible;
    private boolean mPauseResumeVisible;

    public RecordingView(CameraActivity context) {
        super(context);
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.recording);
        mRecordingTime = (TextView) view.findViewById(R.id.recording_time);
        mPauseResume = (ImageView) view.findViewById(R.id.btn_pause_resume);
        mPauseResume.setOnClickListener(this);

        mRecordingSizeGroup = view.findViewById(R.id.recording_size_group);
        mRecordingSizeCurrent = (TextView) view.findViewById(R.id.recording_current);
        mRecordingSizeTotal = (TextView) view.findViewById(R.id.recording_total);
        mRecrodingSizeProgress = (SeekBar) view.findViewById(R.id.recording_progress);
        if (mRecrodingSizeProgress != null) {
            mRecrodingSizeProgress.setOnTouchListener(new OnTouchListener() {
                @Override
                public boolean onTouch(View view, MotionEvent motion) {
                    // disable seek
                    return true;
                }
            });
            mRecrodingSizeProgress.setMax(mMax);
            mRecrodingSizeProgress.setProgress(mProgress);
        }
        return view;
    }

    @Override
    protected void onRefresh() {
        Log.v(TAG, "onRefresh() mCurrent=" + mCurrent + ", mTotal=" + mTotal + ", mProgress="
                + mProgress + ", mMax=" + mMax + ", mRecordinging=" + mRecordinging);
        int recordingId = R.drawable.ic_recording_indicator_pause;
        int playpauseid = R.drawable.ic_recording_play;
        if (mRecordinging) {
            recordingId = R.drawable.ic_recording_indicator_play;
            playpauseid = R.drawable.ic_recording_pause;
        }
        mPauseResume.setImageResource(playpauseid);
        mRecordingTime.setCompoundDrawablesWithIntrinsicBounds(getContext().getResources()
                .getDrawable(recordingId), null, null, null);
        mRecordingTime.setText(mTimeText);

        if (mRecordingSizeCurrent != null) {
            mRecordingSizeCurrent.setText(getFileSize(mCurrent));
        }
        if (mRecordingSizeTotal != null) {
            mRecordingSizeTotal.setText(getFileSize(mTotal));
        }
        if (mRecrodingSizeProgress != null) {
            mRecrodingSizeProgress.setMax(mMax);
            mRecrodingSizeProgress.setProgress(mProgress);
        }
        setTimeVisible(mTimeVisible);
        setPauseResumeVisible(mPauseResumeVisible);
        setRecordingSizeVisible(mRecordingSizeVisible);
    }

    @Override
    public void hide() {
        super.hide();
        setSizeProgress(0); // reset progress to 0
    }

    @Override
    public void onClick(View view) {
        Log.i(TAG, "onClick mListener = " + mListener + " view = " + view + " mPauseResume = "
                + mPauseResume);
        if (mListener != null && mPauseResume == view) {
            mListener.onClick(mPauseResume);
        }
    }

    public void setListener(OnClickListener l) {
        mListener = l;
    }

    public void setRecordingIndicator(boolean recording) {
        Log.d(TAG, "setRecordingIndicator(" + recording + ")");
        mRecordinging = recording;
        refresh();
    }

    public boolean getRecording() {
        return mRecordinging;
    }

    public void showTime(long millis, boolean showMillis) {
        mTimeText = formatTime(millis, showMillis);
        if (mRecordingTime != null) {
            mRecordingTime.setText(mTimeText);
        }
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
        Log.d(TAG, "formatTime(" + millis + ", " + showMillis + ") return " + text);
        return text;
    }

    public void setPauseResumeVisible(boolean visible) {
        Log.d(TAG, "setPauseResumeVisible(" + visible + ") mPauseResumeVisible="
                + mPauseResumeVisible);
        mPauseResumeVisible = visible;
        if (mPauseResume != null) {
            mPauseResume.setVisibility(visible ? View.VISIBLE : View.GONE);
        }
    }

    public void setTimeVisible(boolean visible) {
        Log.d(TAG, "setTimeVisible(" + visible + ") mTimeVisible=" + mTimeVisible);
        mTimeVisible = visible;
        if (mRecordingTime != null) {
            mRecordingTime.setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
        }
    }

    public void setRecordingSizeVisible(boolean visible) {
        Log.d(TAG, "setRecordingSizeVisible(" + visible + ") mRecordingSizeVisible="
                + mRecordingSizeVisible);
        mRecordingSizeVisible = visible;
        if (mRecordingSizeGroup != null) {
            mRecordingSizeGroup.setVisibility(visible ? View.VISIBLE : View.GONE);
        }
    }

    public void setCurrentSize(long size) {
        Log.d(TAG, "setCurrentSize(" + size + ")");
        mCurrent = size;
        if (mRecordingSizeCurrent != null) {
            mRecordingSizeCurrent.setText(getFileSize(mCurrent));
        }
    }

    public void setTotalSize(long size) {
        Log.d(TAG, "setTotalSize(" + size + ")");
        mTotal = size;
        if (mRecordingSizeTotal != null) {
            mRecordingSizeTotal.setText(getFileSize(mTotal));
        }
    }

    public void setMaxSize(int max) {
        Log.d(TAG, "setMaxSize(" + max + ")");
        mMax = max;
        if (mRecrodingSizeProgress != null) {
            mRecrodingSizeProgress.setMax(mMax);
        }
    }

    public void setSizeProgress(int progress) {
        Log.d(TAG, "setSizeProgress(" + progress + ")");
        mProgress = progress;
        if (mRecrodingSizeProgress != null) {
            mRecrodingSizeProgress.setProgress(mProgress);
        }
    }

    private String getFileSize(long size) {
        long kb = size / 1024;
        return kb + "K"; // Formatter.formatFileSize(getContext(), size);
    }
}
