/*
 * Copyright (C) 2013 The Android Open Source Project
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

import com.mediatek.camera.v2.platform.app.AppContext;
import com.mediatek.camera.v2.platform.app.AppController;
import com.mediatek.camera.v2.services.storage.IStorageService;
import com.mediatek.camera.v2.services.storage.StorageServiceImpl;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.util.Utils;

/**
 * Functionality available to all modules and services.
 */
public class CameraAppContext implements AppContext, CameraServices {

    private final AppController mAppController;
    private FileSaver mMediaSaver;
    private SettingCtrl mSettingCtrl;
    private IStorageService mStorageService;
    private SoundPlaybackImpl mSoundPlayback;

    public CameraAppContext(AppController app) {
        mAppController = app;
        Utils.initialize(app.getActivity());
    }

    @Override
    public void onCreate() {
        mSoundPlayback = new SoundPlaybackImpl(mAppController.getActivity());
        mStorageService = new StorageServiceImpl(mAppController.getActivity());
    }

    @Override
    public void onResume() {
        StorageServiceImpl.getStorageMonitor().registerIntentFilter();
    }

    @Override
    public void onPause() {
        mSoundPlayback.pause();
        StorageServiceImpl.getStorageMonitor().unregisterIntentFilter();
    }

    @Override
    public void onDestroy() {
        mSoundPlayback.release();
    }

    @Override
    public FileSaver getMediaSaver() {
        if (mMediaSaver == null) {
            mMediaSaver = new FileSaverImpl();
        }
        return mMediaSaver;
    }

    @Override
    public IStorageService getStorageService() {
        if (mStorageService == null) {
            mStorageService = new StorageServiceImpl(mAppController.getActivity());
        }
        return mStorageService;
    }
    @Override
    public SettingCtrl getSettingController() {
        if (mSettingCtrl == null) {
            mSettingCtrl = new SettingCtrl(mAppController.getActivity());
        }
        return mSettingCtrl;
    }

    @Override
    public ISoundPlayback getSoundPlayback() {
        return mSoundPlayback;
    }
}
