/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.mediatek.voicecommand.vis;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.vis.Log;

public class VoiceTrainingEnrollmentActivity extends Activity {
    private static final String TAG = "VoiceTrainingEnrollmentActivity";
    private static final String VOICE_WAKEUP_ENABLE_CONFIRM = "Voice Wakeup Enable Confirm";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        boolean wakeupConfirm = getIntent().getBooleanExtra(VOICE_WAKEUP_ENABLE_CONFIRM, false);
        Log.d(TAG, "[onCreate] wakeupConfirm : " + wakeupConfirm);

        Intent intent = new Intent(VoiceCommandListener.VOICE_WAKEUP_ACTIVTY_ACTION);
        intent.putExtra(VOICE_WAKEUP_ENABLE_CONFIRM, wakeupConfirm);
        startActivity(intent);
        finish();
    }
}
