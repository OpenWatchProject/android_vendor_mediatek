/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
package com.mediatek.settings.inputmethod;

import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ResolveInfo;
import android.media.AudioManager;
import androidx.preference.Preference;
import android.os.UserHandle;
import android.util.Log;

import com.android.settings.core.PreferenceControllerMixin;
import com.android.settingslib.core.AbstractPreferenceController;
import com.mediatek.settings.FeatureOption;

public class VoiceWakeupPreferenceController extends AbstractPreferenceController
        implements PreferenceControllerMixin {

    private static final String TAG = "VoiceWakeupPreferenceController";
    private static final String KEY_VOICE_UI = "voice_ui";
    private static final String MTK_VOW_SUPPORT_STATE = "MTK_VOW_SUPPORT";
    private static final String MTK_VOW_SUPPORT_ON = "MTK_VOW_SUPPORT=true";
    private static final String MTK_TRIGGER_SUPPORT_STATE = "MTK_VOW_2E2K_SUPPORT";
    private static final String MTK_TRIGGER_SUPPORT_ON = "MTK_VOW_2E2K_SUPPORT=true";
    private Context mContext;

    public VoiceWakeupPreferenceController(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public boolean isAvailable() {
        return isWakeupSupport(mContext) && !isTriggerSupport(mContext)
            && UserHandle.myUserId() == UserHandle.USER_OWNER;
    }

    @Override
    public String getPreferenceKey() {
        return KEY_VOICE_UI;
    }

    @Override
    public void updateState(Preference preference) {
        Intent voiceControlIntent = new Intent("com.mediatek.voicecommand.VOICE_CONTROL_SETTINGS");
        voiceControlIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        List<ResolveInfo> apps = mContext.getPackageManager().queryIntentActivities(
                voiceControlIntent, 0);
        if (apps == null || apps.size() == 0) {
            preference.setEnabled(false);
        } else {
            preference.setEnabled(true);
        }
    }

    private boolean isWakeupSupport(Context context) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (am == null) {
            Log.e(TAG, "isWakeupSupport get audio service is null");
            return false;
        }
        String state = am.getParameters(MTK_VOW_SUPPORT_STATE);
        if (state != null) {
            return state.equalsIgnoreCase(MTK_VOW_SUPPORT_ON);
        }
        return false;
    }

    /**
     * Used to check whether voice trigger feature is supported.
     * @param context android context instance
     * @return if true, the feature was supported
     */
    private boolean isTriggerSupport(Context context) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (am == null) {
            Log.e(TAG, "isTriggerSupport get audio service is null");
            return false;
        }
        String state = am.getParameters(MTK_TRIGGER_SUPPORT_STATE);
        if (state != null) {
            return state.equalsIgnoreCase(MTK_TRIGGER_SUPPORT_ON);
        }
        return false;
    }
}