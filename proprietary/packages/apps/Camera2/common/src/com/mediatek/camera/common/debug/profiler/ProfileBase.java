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
 * MediaTek Inc. (C) 2016. All rights reserved.
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


package com.mediatek.camera.common.debug.profiler;

/**
 * Basic profiler that will compute start, end and "time since last event"
 * values and pass them along to the subclass.
 */
public abstract class ProfileBase implements IPerformanceProfile {
    private long mStartNanos;
    private long mLastMark;
    protected LogFormatter mFormatter;

    /** Create a new profile and init log format.
     * @param name the function name that to record log.
     */
    public ProfileBase(String name) {
        mFormatter = new LogFormatter(name);
    }

    /**
     * Start, or restart the timers associated with instance.
     * @return the performance profile.
     */
    @Override
    public final IPerformanceProfile start() {
        mStartNanos = System.nanoTime();
        mLastMark = mStartNanos;
        onStart();

        return this;
    }

    /**
     * Mark an empty event at the current time.
     */
    @Override
    public final void mark() {
        long time = System.nanoTime();
        onMark(getTotalMillis(time), getTimeFromLastMillis(time));
        mLastMark = time;
    }

    /**
     * Mark something at the current time.
     * @param reason the msg to log.
     */
    @Override
    public final void mark(String reason) {
        long time = System.nanoTime();
        onMark(getTotalMillis(time), getTimeFromLastMillis(time), reason);
        mLastMark = time;
    }

    /**
     * Stop the profile.
     */
    @Override
    public final void stop() {
        long time = System.nanoTime();
        onStop(getTotalMillis(time), getTimeFromLastMillis(time));
        mLastMark = time;
    }

    /**
     * Stop the profile for a given reason.
     * @param reason the msg to log.
     */
    @Override
    public final void stop(String reason) {
        long time = System.nanoTime();
        onStop(getTotalMillis(time), getTimeFromLastMillis(time), reason);
        mLastMark = time;
    }

    /**
     * Called when start() is called.
     */
    protected void onStart() { }

    /**
     * Called when mark() is called with computed total and time.
     */
    protected void onMark(double totalMillis, double lastMillis) { }

    /**
     * Called when mark() is called with computed total and time.
     * since last event values in milliseconds.
     */
    protected void onMark(double totalMillis, double lastMillis, String reason) { }

    /**
     * Called when stop() is called with computed total and time.
     * since last event values in milliseconds.
     */
    protected void onStop(double totalMillis, double lastMillis) { }

    /**
     * Called when stop() is called with computed total and time
     * since last event values in milliseconds. Inclues the stop reason.
     */
    protected void onStop(double totalMillis, double lastMillis, String reason) { }

    private double getTotalMillis(long timeNanos) {
        return nanoToMillis(timeNanos - mStartNanos);
    }

    private double getTimeFromLastMillis(long timeNanos) {
        return nanoToMillis(timeNanos - mLastMark);
    }

    private double nanoToMillis(long timeNanos) {
        return (double) (timeNanos) / 1000000.0;
    }
}
