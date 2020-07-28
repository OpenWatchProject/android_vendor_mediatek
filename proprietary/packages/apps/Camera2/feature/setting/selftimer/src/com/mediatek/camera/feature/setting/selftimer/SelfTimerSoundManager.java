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

import android.app.Activity;
import android.media.SoundPool;

import com.mediatek.camera.R;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;


/**
 * This class is for self timer sound manager.
 */

public class SelfTimerSoundManager {
    public static final int BEEP_ONCE = 0;
    public static final int BEEP_TWICE = 1;
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(SelfTimerSoundManager.class.getSimpleName());
    private static final int MAX_STREAM = 1;
    private static final int STREAM_SYSTEM_ENFORCED = 7;

    private SoundPool mSoundPool;
    private Activity mActvity;
    private int mBeepOnce;
    private int mBeepTwice;
    private int mBeepOnceStreamId;
    private int mBeepTwiceStreamId;

    /**
     * Construct a Capture sound instance.
     */
    SelfTimerSoundManager(IApp app) {
        mActvity = app.getActivity();
    }

    /**
     * Load the sound from the specified path.
     */
    public void load() {
        if (mSoundPool == null) {
            mSoundPool = new SoundPool(MAX_STREAM, STREAM_SYSTEM_ENFORCED, 0);
            mBeepOnce = mSoundPool.load(mActvity, R.raw.beep_once, 1);
            mBeepTwice = mSoundPool.load(mActvity, R.raw.beep_twice, 1);
        }
        LogHelper.d(TAG, "[load] mSoundPool :" + mSoundPool);
    }


    /**
     * Play a sound from a sound ID.
     * @param soundId the sound type by id.
     */
    public void play(int soundId) {
        LogHelper.d(TAG, "[play]");
        if (mSoundPool == null) {
            return;
        }
        if (soundId == BEEP_ONCE) {
            mBeepOnceStreamId = mSoundPool.play(mBeepOnce, 1.0f, 1.0f, 0, 0, 1.0f);
        } else {
            mBeepTwiceStreamId = mSoundPool.play(mBeepTwice, 1.0f, 1.0f, 0, 0, 1.0f);
        }
    }

    /**
     * Stop the sound.
     */
    public void stop() {
        LogHelper.d(TAG, "[stop]");
        if (mSoundPool != null) {
            mSoundPool.stop(mBeepOnceStreamId);
            mSoundPool.stop(mBeepTwiceStreamId);
        }
    }

    /**
     * Release the Sound resources.
     */
    public void release() {
        LogHelper.d(TAG, "[release]");
        if (mSoundPool != null) {
            mSoundPool.unload(mBeepOnce);
            mSoundPool.unload(mBeepTwice);
            mSoundPool.release();
            mSoundPool = null;
        }
    }
}
