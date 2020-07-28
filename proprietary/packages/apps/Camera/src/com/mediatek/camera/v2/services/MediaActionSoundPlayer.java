/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.camera.v2.services;

import android.annotation.TargetApi;
import android.media.MediaActionSound;
import android.os.Build;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

/**
 * This class controls to play system-standard sounds.
 *
 */
@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
class MediaActionSoundPlayer {

    private static final Tag TAG = new Tag(MediaActionSoundPlayer.class.getSimpleName());
    private MediaActionSound mSound;

    /**
     * Load the system-standard sounds in constructor.
     */
    protected MediaActionSoundPlayer() {
        mSound = new MediaActionSound();
        mSound.load(MediaActionSound.START_VIDEO_RECORDING);
        mSound.load(MediaActionSound.STOP_VIDEO_RECORDING);
        mSound.load(MediaActionSound.FOCUS_COMPLETE);
        mSound.load(MediaActionSound.SHUTTER_CLICK);
    }

    /**
     * Play the system-standard sound with a specail sound action.
     *
     * @param action The action of the sound.
     */
    protected synchronized void play(int action) {
        switch (action) {
            case ISoundPlayback.FOCUS_COMPLETE:
                mSound.play(MediaActionSound.FOCUS_COMPLETE);
                break;
            case ISoundPlayback.START_VIDEO_RECORDING:
                mSound.play(MediaActionSound.START_VIDEO_RECORDING);
                break;
            case ISoundPlayback.STOP_VIDEO_RECORDING:
                mSound.play(MediaActionSound.STOP_VIDEO_RECORDING);
                break;
            case ISoundPlayback.SHUTTER_CLICK:
                mSound.play(MediaActionSound.SHUTTER_CLICK);
                break;
            default:
                LogHelper.w(TAG, "Unrecognized action:" + action);
        }
    }

    /**
     * Call this if you don't need the MediaActionSoundPlayer anymore.
     */
    protected void release() {
        if (mSound != null) {
            mSound.release();
            mSound = null;
        }
    }
}
