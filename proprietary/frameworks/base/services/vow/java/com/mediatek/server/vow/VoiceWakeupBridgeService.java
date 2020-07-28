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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.mediatek.server.vow;

import android.content.Context;
import android.hardware.soundtrigger.IRecognitionStatusCallback;
import android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel;
import android.hardware.soundtrigger.SoundTrigger.RecognitionConfig;
import android.util.Slog;

import com.android.server.LocalServices;
import com.android.server.SystemService;
import com.android.server.soundtrigger.SoundTriggerInternal;
import com.mediatek.vow.IVoiceWakeupBridge;

/**
 * A bridge from voice wakeup API to SoundTriggerInternal class
 * 
 * @hide
 */
public class VoiceWakeupBridgeService extends SystemService {

    private static final String TAG = "VoiceWakeupBridgeService";
    private static final String BINDER_TAG = "vow_bridge";

    private final Context mContext;
    private final VoiceWakeupBridgeStub mServiceStub;
    private SoundTriggerInternal mSoundTriggerInternal;

    public VoiceWakeupBridgeService(Context context) {
        super(context);
        mContext = context;
        mServiceStub = new VoiceWakeupBridgeStub();
    }

    @Override
    public void onStart() {
        Slog.d(TAG, "onStart");
        publishBinderService(BINDER_TAG, mServiceStub);
    }

    @Override
    public void onBootPhase(int phase) {
        Slog.d(TAG, "onBootPhase: " + phase);
        if (PHASE_SYSTEM_SERVICES_READY == phase) {
            mSoundTriggerInternal = LocalServices
                    .getService(SoundTriggerInternal.class);
        } else if (PHASE_THIRD_PARTY_APPS_CAN_START == phase) {

        }
    }

    @Override
    public void onStartUser(int userHandle) {
        Slog.d(TAG, "onStartUser: " + userHandle);
    }

    @Override
    public void onUnlockUser(int userHandle) {
        Slog.d(TAG, "onUnlockUser: " + userHandle);
    }

    @Override
    public void onSwitchUser(int userHandle) {
        Slog.d(TAG, "onSwitchUser: " + userHandle);
    }

    @Override
    public void onStopUser(int userHandle) {
        Slog.d(TAG, "onStopUser: " + userHandle);
    }

    @Override
    public void onCleanupUser(int userHandle) {
        Slog.d(TAG, "onCleanupUser: " + userHandle);
    }

    class VoiceWakeupBridgeStub extends IVoiceWakeupBridge.Stub {
        @Override
        public int startRecognition(int keyphraseId,
                KeyphraseSoundModel soundModel,
                IRecognitionStatusCallback listener,
                RecognitionConfig recognitionConfig) {
            Slog.d(TAG, "startRecognition");
            return mSoundTriggerInternal.startRecognition(keyphraseId,
                    soundModel, listener, recognitionConfig);
        }

        @Override
        public int stopRecognition(int keyphraseId,
                IRecognitionStatusCallback listener) {
            Slog.d(TAG, "stopRecognition");
            return mSoundTriggerInternal.stopRecognition(keyphraseId, listener);
        }

        @Override
        public int unloadKeyphraseModel(int keyphaseId) {
            Slog.d(TAG, "unloadKeyphraseModel");
            return mSoundTriggerInternal.unloadKeyphraseModel(keyphaseId);
        }
    }

}
