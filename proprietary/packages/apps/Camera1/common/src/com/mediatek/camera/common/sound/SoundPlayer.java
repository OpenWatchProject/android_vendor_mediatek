/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 * the prior written permission of MediaTek inc. and/or its licensor, any
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
 * NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.common.sound;

import android.content.Context;
import android.media.AudioManager;
import android.media.SoundPool;
import android.util.SparseIntArray;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;

import java.lang.reflect.Field;
import java.util.HashMap;

/**
 * Loads a plays custom sounds. For playing system-standard sounds for various
 * camera actions, please refer to {@link MediaActionSoundPlayer}.
 */
public class SoundPlayer implements SoundPool.OnLoadCompleteListener {
    private static final Tag TAG = new Tag(SoundPlayer.class.getSimpleName());
    private final Context mAppContext;
    private final SoundPool mSoundPool;
    // ID returned by load() should be non-zero.
    private static final int UNLOAD_SOUND_ID = 0;
    // Keeps a mapping from sound resource ID to sound ID.
    private final SparseIntArray mResourceToSoundId = new SparseIntArray();
    private final HashMap<Integer, Boolean> mSoundIDReadyMap = new HashMap<Integer, Boolean>();
    private int mSoundIDToPlay;
    private float mVolume;

    /**
     * Construct a new sound player.
     * @param appContext app context.
     */
    public SoundPlayer(Context appContext) {
        mAppContext = appContext;
        final int audioType = getAudioTypeForSoundPool();
        mSoundPool = new SoundPool(1 /* max streams */, audioType, 0 /* quality */);
        mSoundIDToPlay = UNLOAD_SOUND_ID;
        mSoundPool.setOnLoadCompleteListener(SoundPlayer.this);
    }

    /**
     * Load the sound from a resource.
     * @param resourceId resource id.
     */
    private void loadSound(int resourceId) {
        int soundId = mSoundPool.load(mAppContext, resourceId, 1/* priority */);
        mResourceToSoundId.put(resourceId, soundId);
    }

    /**
     * Play the sound with the given resource. The resource has to be loaded
     * before it can be played.
     *
     * @param resourceId resource id.
     * @param volume volume
     */
    public void play(int resourceId, float volume) {
        mSoundIDToPlay = mResourceToSoundId.get(resourceId, UNLOAD_SOUND_ID);
        mVolume = volume;
        if (mSoundIDToPlay == UNLOAD_SOUND_ID) {
            loadSound(resourceId);
            mSoundIDToPlay = mResourceToSoundId.get(resourceId);
        } else if (!mSoundIDReadyMap.get(mSoundIDToPlay)) {
            LogHelper.w(TAG, "sound id " + mSoundIDToPlay + " is in loading and not ready yet");
        } else {
            mSoundPool
                    .play(mSoundIDToPlay, volume, volume,
                            0 /* priority */, 0 /* loop */, 1 /* rate */);
        }
    }

    /**
     * Unload the given sound if it's not needed anymore to release memory.
     * @param resourceId resource id.
     */
    private void unloadSound(int resourceId) {
        Integer soundId = mResourceToSoundId.get(resourceId);
        if (soundId == null) {
            throw new IllegalStateException("Sound not loaded. Must call #loadSound first.");
        }
        mSoundPool.unload(soundId);
    }

    /**
     * Unload the all sound if it's not needed anymore to release memory.
     */
    public void unloadSound() {
        int resourceId = 0;
        int resourceSize = mResourceToSoundId.size();
        for (int i = 0; i < resourceSize; i++) {
            resourceId = mResourceToSoundId.keyAt(i);
            unloadSound(resourceId);
        }
        mResourceToSoundId.clear();
    }

    /**
     * Call this if you don't need the SoundPlayer anymore. All memory will be
     * released and the object cannot be re-used.
     */
    public void release() {
        mSoundPool.release();
    }

    @Override
    public void onLoadComplete(SoundPool pool, int soundID, int status) {
        if (status != 0) {
            LogHelper.e(TAG, "onLoadComplete : " + soundID + " load failed , status is " + status);
            return;
        }
        LogHelper.d(TAG, "onLoadComplete : " + soundID + " load success");
        mSoundIDReadyMap.put(soundID, true);
        if (soundID == mSoundIDToPlay) {
            mSoundIDToPlay = UNLOAD_SOUND_ID;
            mSoundPool.play(soundID, mVolume, mVolume, 0, 0, 1);
        }
    }

    private int getAudioTypeForSoundPool() {
        // STREAM_SYSTEM_ENFORCED is hidden API.
        return getIntFieldIfExists(AudioManager.class, "STREAM_SYSTEM_ENFORCED", null,
                AudioManager.STREAM_RING);
    }

    private int getIntFieldIfExists(Class<?> klass, String fieldName,
            Class<?> obj, int defaultVal) {
        try {
            Field f = klass.getDeclaredField(fieldName);
            return f.getInt(obj);
        } catch (NoSuchFieldException e) {
            return defaultVal;
        } catch (IllegalAccessException e) {
            return defaultVal;
        }
    }
}