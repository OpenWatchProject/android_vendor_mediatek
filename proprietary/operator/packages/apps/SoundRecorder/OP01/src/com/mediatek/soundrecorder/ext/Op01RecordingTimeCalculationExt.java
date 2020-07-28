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

package com.mediatek.soundrecorder.ext;

import android.content.Context;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.os.storage.StorageManager;

import com.android.soundrecorder.Recorder;
import com.android.soundrecorder.RemainingTimeCalculator;
import com.android.soundrecorder.SoundRecorderService;


/**
 * Op01 implementation of IRecordingTimeCalculationExt.
 * It will keep the default behavior of SoundRecorder application.
 */
public class Op01RecordingTimeCalculationExt extends DefaultRecordingTimeCalculationExt {

    private static final String EXTRA_MAX_DURATION = "com.android.soundrecorder.maxduration";
    private static final long MAX_DURATION_NULL = -1L;
    private long mMaxDuration = MAX_DURATION_NULL;

    /**
     * Constructor of Op01 Recording Time Calculation Extension Implementation Class.
     * @param context application context
     */
    public Op01RecordingTimeCalculationExt(Context context) {
        super(context);
    }

    @Override
    public void setExtras(Bundle extras) {
        if (extras != null) {
            mMaxDuration = extras.getLong(EXTRA_MAX_DURATION, MAX_DURATION_NULL);
        } else {
            mMaxDuration = MAX_DURATION_NULL;
        }
    }

    @Override
    public MediaRecorder getMediaRecorder() {
        MediaRecorder recorder = super.getMediaRecorder();
        if (mMaxDuration != MAX_DURATION_NULL) {
            recorder.setMaxDuration((int) mMaxDuration);
        }
        return recorder;
    }

    @Override
    public RemainingTimeCalculator getRemainingTimeCalculator(
                                       StorageManager storageManager,
                                       SoundRecorderService service) {
        return new Op01RemainingTimeCalculator(storageManager, service);
    }

    /**
     * Op01 implementation of RemainingTimeCalculator.
     * Consider the max duration limitation.
     */
    public class Op01RemainingTimeCalculator extends RemainingTimeCalculator {

        private SoundRecorderService mService;

        /**
         * Constructor of Op01 implementation of RemainingTimeCalculator.
         * @param storageManager reference of StorageManager.
         * @param service reference of SoundRecorderService.
         */
        public Op01RemainingTimeCalculator(StorageManager storageManager,
                       SoundRecorderService service) {
            super(storageManager, service);
            mService = service;
        }

        @Override
        public long timeRemaining(boolean isFirstTimeGetRemainingTime,
                boolean isForStartRecording) {

            long remainingTime =
                    super.timeRemaining(isFirstTimeGetRemainingTime, isForStartRecording);
            Recorder recorder = null;
            if (mService != null) {
                recorder = mService.getRecorder();
            }

            if (mMaxDuration != MAX_DURATION_NULL && recorder != null) {
                long currentPogress = recorder.getCurrentProgress();
                long diff = mMaxDuration - currentPogress;
                if (diff > 0) {
                    diff = diff / 1000 + 1;
                }
                if (diff < 0) {
                    remainingTime = 0;
                } else {
                    remainingTime = Math.min(remainingTime, diff);
                }
            }
            return remainingTime;
        }
    }
}