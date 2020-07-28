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
package com.mediatek.camera.feature.setting.exposure;

import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.widget.SeekBar;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.widget.RotateLayout;

/**
 * View used to handle exposure related UI.
 */
public class ExposureView extends RotateLayout {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(ExposureView.class.getSimpleName());
    private ExposureViewChangedListener mListener;
    private VerticalSeekBar mEvSeekbar;
    private static final int PROGRESS_SCARE = 50;
    private final static int ONE_FINGER = 1;
    private static int sDeProgress = 150;
    private int mLastEv = 0;
    private static int sMinEv;
    private static int sMaxEv;
    private int mLastProgress;
    private int mOrientation;
    //the scroll distance mapping to 1 step of progress bar,can be customized.
    private static final int SCREEN_DISTANCE_MAPPING_TO_PROGRESS = 10;
    private static int sAvailableSpace = 1;
    private boolean mEvChangeStartNotified = false;

    /**
     * The constructor.
     *
     * @param context The Context the view is running in, through which it can access the current
     *                theme, resources, etc.
     * @param attrs   The attributes of the XML tag that is inflating the view.
     */
    public ExposureView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void onFinishInflate() {
        super.onFinishInflate();
        mEvSeekbar = (VerticalSeekBar) findViewById(R.id.ev_seekbar);
        mEvSeekbar.setThumb(this.getResources().getDrawable(R.drawable.ic_ev_scrubber));

    }

    /**
     * Classes implementing this interface will be called when exposure changed.
     */
    protected interface ExposureViewChangedListener {
        /**
         * Notify exposure seek bar has been changed.
         *
         * @param value The current ev seek bar value.
         */
        void onExposureViewChanged(int value);

        /**
         * Notify exposure seek bar start or stop to change exposure.
         *
         * @param start True if begin to change exposure,false if end to change.
         */
        void onTrackingTouchStatusChanged(boolean start);
    }

    /**
     * Sets or replaces a listener that is called whenever the exposureCompensation changes.
     *
     * @param listener The one which watch the exposureCompensation values.
     */
    protected void setListener(ExposureViewChangedListener listener) {
        mListener = listener;
    }

    protected void initExposureView(int[] evs) {
        int min = evs[evs.length - 1];
        int max = evs[0];
        sMinEv = min;
        sMaxEv = max;
        int seekBarMax = PROGRESS_SCARE * (sMaxEv - sMinEv);
        mEvSeekbar.setMax(seekBarMax);
        sDeProgress = PROGRESS_SCARE * (sMaxEv - sMinEv) / 2;
        sAvailableSpace = seekBarMax * SCREEN_DISTANCE_MAPPING_TO_PROGRESS;
        resetExposureView();
        mEvSeekbar.setOnSeekBarChangeListener(mEvSeekBarChangedListener);
        LogHelper.d(TAG, "[initExposureView] sDeProgress " + sDeProgress + ",max = " + mEvSeekbar
                .getMax());
    }

    protected void resetExposureView() {
        mEvSeekbar.setProgress(sDeProgress);
        mEvSeekbar.getProgressDrawable().setAlpha(0);
    }

    protected void setViewEnabled(boolean enabled) {
        mEvSeekbar.setEnabled(enabled);
    }

    protected void setOrientation(int orientation) {
        mOrientation = orientation;
    }

    protected void onTrackingTouch(boolean start) {
        if (mEvSeekBarChangedListener == null) {
            return;
        }
        if (start) {
            mEvSeekBarChangedListener.onStartTrackingTouch(mEvSeekbar);
        } else {
            mEvSeekBarChangedListener.onStopTrackingTouch(mEvSeekbar);
        }
    }

    protected void onVerticalScroll(MotionEvent event, float delta) {
        if (event.getPointerCount() == ONE_FINGER) {
            updateEvProgressbar(delta);
        }
    }

    private void updateEvProgressbar(float delta) {
        int update = extractDeltaScale(delta, mEvSeekbar);
        if (mLastProgress == update) {
            return;
        } else {
            mLastProgress = update;
        }
        mEvSeekbar.setProgressDrawable(new ColorDrawable(Color.WHITE));
        mEvSeekbar.setProgress(update);
    }

    private int extractDeltaScale(float deltaY, SeekBar seekbar) {
        int y = (int) deltaY;
        float scale;
        float progress = seekbar.getProgress();
        final int max = seekbar.getMax();
        if (mOrientation == 0 || mOrientation == 90) {
            scale = (float) (y) / (float) sAvailableSpace;
            progress += scale * max;
        }

        if (mOrientation == 180 || mOrientation == 270) {
            scale = (float) (-y) / (float) sAvailableSpace;
            progress += scale * max;
        }

        if (progress > max) {
            progress = max;
        } else if (progress < 0) {
            progress = 0;
        }
        return (int) progress;
    }


    private void onEvChanged(boolean start) {
        LogHelper.d(TAG, "[onEvChanged] " + start);
        mEvChangeStartNotified = start;
        if (start) {
            mListener.onTrackingTouchStatusChanged(true);
            mEvSeekbar.getProgressDrawable().setAlpha(1);
        } else {
            mListener.onTrackingTouchStatusChanged(false);
            mEvSeekbar.getProgressDrawable().setAlpha(0);
        }
    }

    private VerticalSeekBar.OnSeekBarChangeListener mEvSeekBarChangedListener =
            new VerticalSeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                    if (mListener != null) {
                        if (i < 0 || i > seekBar.getMax()) {
                            return;
                        }
                        int evRange = sMaxEv - sMinEv;
                        if (evRange == 0) {
                            return;
                        }
                        float ev = (i + (seekBar.getMax() * sMinEv / evRange)) /
                                (seekBar.getMax() / evRange);
                        int currentEv = Math.round(ev);
                        if (currentEv != mLastEv) {
                            LogHelper.d(TAG, "[onProgressChanged] mLastProgress " + mLastEv +
                                    ",progress = " + currentEv + ",sMaxEv = " + sMaxEv + ",sMinEv" +
                                    " " +
                                    "= " + sMinEv + ",max = " + seekBar.getMax());
                            mLastEv = currentEv;
                            mListener.onExposureViewChanged(mLastEv);
                            if (!mEvChangeStartNotified && mEvSeekbar.getProgressDrawable()
                                    .getAlpha() != 0) {
                                onEvChanged(true);
                            }
                        }
                    }
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {
                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {
                    if (mEvChangeStartNotified) {
                        onEvChanged(false);
                    }
                }
            };

}
