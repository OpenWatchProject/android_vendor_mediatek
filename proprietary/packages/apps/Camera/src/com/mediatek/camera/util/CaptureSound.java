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

package com.mediatek.camera.util;

import android.app.Activity;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.SoundPool;

import com.android.camera.R;

public class CaptureSound {
    private static final String TAG = "CaptureSound";

    private static int mUserCount = 0;
    private Activity mContext;
    private int mSoundId;
    private int mStreamId;

    private SoundPool mBurstSound;

    /**
     * the constructor for CaptureSound.
     * @param context camera context.
     */
    public CaptureSound(Activity context) {
        mContext = context;
    }

    public void load() {
        Log.d(TAG, "[load]mUserCount = " + mUserCount);
        mUserCount++;
        AudioAttributes attrs = new AudioAttributes.Builder()
                .setInternalLegacyStreamType(
                        AudioManager.STREAM_SYSTEM_ENFORCED).build();
        mBurstSound = new SoundPool.Builder().setMaxStreams(10)
                .setAudioAttributes(attrs).build();
        mSoundId = mBurstSound.load(mContext, R.raw.camera_shutter, 1);
    }

    public void play() {
        Log.d(TAG, "[play]mBurstSound = " + mBurstSound);
        if (mBurstSound == null) {
            // force load if user don't call load before play.
            load();
        }
        mStreamId = mBurstSound.play(mSoundId, 1.0f, 1.0f, 1, -1, 1.0f);
        if (mStreamId == 0) {
            // play failed,load and play again.
            load();
            mUserCount--;
            mStreamId = mBurstSound.play(mSoundId, 1.0f, 1.0f, 1, -1, 1.0f);
            Log.d(TAG, "[play]done mStreamId = " + mStreamId);
        }
    }

    public void stop() {
        Log.d(TAG, "[stop]mStreamId = " + mStreamId);
        if (mBurstSound != null) {
            mBurstSound.stop(mStreamId);
        }
    }

    public void release() {
        Log.d(TAG, "[release]mBurstSound = " + mBurstSound + ", user count = " + mUserCount);
        if (mBurstSound != null) {
            mUserCount--;
            mBurstSound.unload(mSoundId);
            mBurstSound.release();
            mBurstSound = null;
        }
    }
}
