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
package com.android.soundrecorder;

import android.media.MediaPlayer;

import java.io.File;
import java.io.IOException;

/**
 * M: we split player from recorder, the player is only responsible for play back.
 */
public class Player implements MediaPlayer.OnCompletionListener, MediaPlayer.OnErrorListener {

    private static final String TAG = "SR/Player";
    private MediaPlayer mPlayer = null;
    private String mCurrentFilePath = null;
    private PlayerListener mListener = null;

    /**
     * Player Interface.
     */
    public interface PlayerListener {
        /**
         * Error Callback.
         *
         * @param player the instance of Player
         * @param errorCode the error code
         */
        void onError(Player player, int errorCode);

        /**
         * State Change Callback.
         *
         * @param player the instance of Player
         * @param stateCode the state code
         */
        void onStateChanged(Player player, int stateCode);
    }

    /**
     * M: Constructor of player, only PlayListener needed.
     * @param listener the listener of player
     */
    public Player(PlayerListener listener) {
        mListener = listener;
    }

    @Override
    public void onCompletion(MediaPlayer player) {
        LogUtils.i(TAG, "<onCompletion>");
        stopPlayback();
    }

    @Override
    public boolean onError(MediaPlayer player, int errorType, int extraCode) {
        mListener.onError(this, ErrorHandle.ERROR_PLAYING_FAILED);
        return false;
    }

    /**
     * M: set the path of audio file which will play, used by
     * SoundRecorderService.
     *
     * @param filePath file path info
     */
    public void setCurrentFilePath(String filePath) {
        mCurrentFilePath = filePath;
    }

    /**
     * M: start play the audio file which is set in setCurrentFilePath.
     * @return the result of play back, success or fail
     */
    public boolean startPlayback() {
        if (null == mCurrentFilePath) {
            return false;
        }

        File file = new File(mCurrentFilePath);
        if (!file.exists()) {
            mListener.onError(this, ErrorHandle.ERROR_FILE_DELETED_WHEN_PLAY);
            return false;
        }

        synchronized (this) {
            if (null == mPlayer) {
                mPlayer = new MediaPlayer();
                try {
                    mPlayer.setDataSource(mCurrentFilePath);
                    mPlayer.setOnCompletionListener(this);
                    mPlayer.prepare();
                    mPlayer.start();
                    LogUtils.i(TAG, "<startPlayback> The length of recording file is "
                            + mPlayer.getDuration());
                    setState(SoundRecorderService.STATE_PLAYING);
                } catch (IllegalStateException e) {
                    return handleException(e);
                } catch (IOException e) {
                    return handleException(e);
                }
            }
        }
        return true;
    }

    /**
     * M: Handle the Exception when call the function of MediaPlayer.
     * @param exception exception info
     * @return false
     */
    public boolean handleException(Exception exception) {
        LogUtils.i(TAG, "<handleException>");
        exception.printStackTrace();
        if (mPlayer != null) {
            mPlayer.release();
            mPlayer = null;
        }
        mListener.onError(this, ErrorHandle.ERROR_PLAYING_FAILED);
        return false;
    }

    /**
     * M: pause play the audio file which is set in setCurrentFilePath.
     *
     * @return pause result
     */
    public boolean pausePlayback() {
        if (null == mPlayer) {
            return false;
        }
        try {
            mPlayer.pause();
            setState(SoundRecorderService.STATE_PAUSE_PLAYING);
        } catch (IllegalStateException e) {
            return handleException(e);
        }
        return true;
    }

    /**
     * M: goon play the audio file which is set in setCurrentFilePath.
     *
     * @return resume result
     */
    public boolean goonPlayback() {
        if (null == mPlayer) {
            return false;
        }

        try {
            mPlayer.start();
            setState(SoundRecorderService.STATE_PLAYING);
        } catch (IllegalStateException e) {
            return handleException(e);
        }
        return true;
    }

    /**
     * M: stop play the audio file which is set in setCurrentFilePath.
     *
     * @return stop result
     */
    public boolean stopPlayback() {
        // we were not in playback
        synchronized (this) {
            if (null == mPlayer) {
                return false;
            }
            try {
                mPlayer.stop();
                setState(SoundRecorderService.STATE_IDLE);
            } catch (IllegalStateException e) {
                return handleException(e);
            }
            mPlayer.release();
            mPlayer = null;
        }
        return true;
    }

    /**
     * M: reset Player to initial state.
     */
    public void reset() {
        synchronized (this) {
            if (null != mPlayer) {
                mPlayer.stop();
                mPlayer.release();
                mPlayer = null;
            }
        }
        mCurrentFilePath = null;
    }

    /**
     * M: get the current position of audio which is playing.
     * @return the current position in millseconds
     */
    public int getCurrentProgress() {
        if (null != mPlayer) {
            return mPlayer.getCurrentPosition();
        }
        return 0;
    }

    /**
     * M: get the duration of audio file.
     * @return the duration in millseconds
     */
    public int getFileDuration() {
        if (null != mPlayer) {
            return mPlayer.getDuration();
        }
        return 0;
    }

    private void setState(int state) {
        mListener.onStateChanged(this, state);
    }
}