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
package com.mediatek.voicecommand.ui.settings;

import android.content.Intent;
import android.media.AudioManager;
import android.media.SoundPool;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;

import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.util.Log;

import java.util.HashMap;

/**
 * An activity to show voice control command play hierarchy of preferences to
 * the user.
 * 
 */
public class VoiceUiCommandPlayActivity extends PreferenceActivity {
    private static final String TAG = "VoiceUiCommandPlayActivity";

    private static final String KEY_VOICE_UI_FOR_COMMAND_PLAY = "command_play";
    private static final String KEY_VOICE_UI_FOR_COMMAND_CATEGORY = "voice_ui_command";

    private String mProcessKey;
    String[] mCommands;

    private PreferenceCategory mVoiceUiCommandCategory;
    private ConfigurationManager mVoiceConfigMgr;
    private SoundPool mSoundPool;
    private HashMap<String, Integer> mSoundIdMap = new HashMap<String, Integer>();

    @SuppressWarnings("deprecation")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.voice_ui_available_preference);

        Intent intent = getIntent();
        if (intent == null) {
            Log.e(TAG, "[onCreate]Intent is null!");
            finish();
            return;
        }
        mVoiceConfigMgr = ConfigurationManager.getInstance(this);
        mProcessKey = (String) intent
                .getCharSequenceExtra(VoiceUiSettingsActivity.KEY_VOICE_UI_FOR_PLAY_COMMAND);
        int processID = mVoiceConfigMgr.getProcessID(mProcessKey);
        int commandTitleId = VoiceUiResUtil.getCommandTitleResourceId(processID);
        if (commandTitleId != 0) {
            setTitle(VoiceUiResUtil.getCommandTitleResourceId(processID));
        } else {
            setTitle("Error");
        }

        if (mVoiceConfigMgr == null) {//TODO: moved after line 86?
            Log.e(TAG, "[onCreate]ConfigurationManager is null");
            finish();
            return;
        }
        mCommands = mVoiceConfigMgr.getKeyWordForSettings(mProcessKey);
        if (mCommands == null) {
            Log.e(TAG, "[onCreate]mCommands is null");
            finish();
            return;
        }

        CommandPlayPreference titlePref = (CommandPlayPreference) findPreference(KEY_VOICE_UI_FOR_COMMAND_PLAY);
        titlePref.setShowTitle(fetchSummary(processID));
        titlePref.setSelectable(false);
        titlePref.setOrder(0);

        //ALPS02317449 AudioManager.STREAM_MUSIC changed to AudioManager.STREAM_SYSTEM_ENFORCED
        //to sync with marshmallow.
        mSoundPool = new SoundPool(1, AudioManager.STREAM_SYSTEM_ENFORCED, 0);
        mVoiceUiCommandCategory = (PreferenceCategory) findPreference(KEY_VOICE_UI_FOR_COMMAND_CATEGORY);
        mVoiceUiCommandCategory.setTitle(R.string.voice_ui_commands);
        for (int i = 0; i < mCommands.length; i++) {
            Preference pref = new Preference(this);
            pref.setLayoutResource(R.layout.voice_ui_preference_image);
            pref.setTitle(mCommands[i]);
            String path = mVoiceConfigMgr.getCommandPath(mProcessKey);
            path = path + i + ".ogg";
            mSoundIdMap.put(mCommands[i], mSoundPool.load(path, 1));
            pref.setKey(mCommands[i]);
            pref.setOrder(i + 1);
            mVoiceUiCommandCategory.addPreference(pref);
        }
    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "[onDestroy]...");
        if (mSoundPool != null) {
            mSoundPool.release();
        }
        super.onDestroy();
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "[onPause]...");
        mSoundPool.autoPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "[onResume]...");
        mSoundPool.autoResume();
        super.onResume();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        String key = preference.getKey();
        if ((key != null) && (mSoundIdMap.containsKey(key)) && (mSoundPool != null)) {
            mSoundPool.play(mSoundIdMap.get(key), 1, 1, 0, 0, 1);
        } else {
            Log.e(TAG, "[onPreferenceTreeClick] path is null! ");
        }

        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    /**
     * Fetch summary according process ID.
     * 
     * @param processID
     *            process id in voiceprocess.xml
     * @return summary title
     */
    private String fetchSummary(int processID) {
        StringBuilder keywords = new StringBuilder();
        String lastWord = "\"" + mCommands[mCommands.length - 1] + "\"";

        for (int i = 0; i < mCommands.length - 1; i++) {
            keywords.append("\"").append(mCommands[i]).append("\"");
            if (i != mCommands.length - 2) {
                keywords.append(",");
            }
        }

        int resId = VoiceUiResUtil.getSummaryResourceId(processID);
        if (resId == 0) {
            Log.e(TAG, "[fetchSummary]resId is 0!");
            return new String("Error");
        }
        String summary = getString(resId, keywords.toString(), lastWord);
        Log.d(TAG, "[fetchSummary]summary = " + summary);
        
        return summary;
    }
}
