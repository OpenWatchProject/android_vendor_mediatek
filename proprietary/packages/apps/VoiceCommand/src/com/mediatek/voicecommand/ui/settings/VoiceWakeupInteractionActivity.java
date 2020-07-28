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

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Toast;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.cfg.VoiceWakeupInfo;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * An transition activity to show Voice Wakeup Activity.
 *
 */
public class VoiceWakeupInteractionActivity extends Activity {
    private static final String TAG = "VoiceWakeupInteractionActivity";
    private static final String VOICE_WAKEUP_ENABLE_CONFIRM = "Voice Wakeup Enable Confirm";
    private ConfigurationManager mVoiceConfigMgr;
    private int mWakeupMode = 1;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mVoiceConfigMgr = ConfigurationManager.getInstance(this);
        if (mVoiceConfigMgr == null) {
            Log.e(TAG, "[onCreate]ConfigurationManager is null");
            finish();
            return;
        }

        mWakeupMode = mVoiceConfigMgr.getWakeupMode();
        boolean wakeupConfirm = getIntent().getBooleanExtra(VOICE_WAKEUP_ENABLE_CONFIRM, false);
        Log.d(TAG, "[onCreate] wakeupMode : " + mWakeupMode + ", wakeupConfirm : " + wakeupConfirm);

        // Voice Input UI Radio Button checked.
        if (wakeupConfirm) {
            if (mWakeupMode == VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT) {
                startVoiceWakeupAnyoneRecord();
            } else if (mWakeupMode == VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT) {
                startVoiceWakeupCommand();
            }
        // Voice Input UI Settings Button checked.
        } else {
            if (mWakeupMode == VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT) {
                startVoiceWakeupAnyone();
            } else if (mWakeupMode == VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT) {
                startVoiceWakeupCommand();
            }
        }
        finish();
    }

    /**
     * Start Voice Wakeup Anyone Training Activity.
     */
    private void startVoiceWakeupAnyone() {
        VoiceWakeupInfo[] wakeupInfos = mVoiceConfigMgr.getCurrentWakeupInfo(mWakeupMode);
        if (wakeupInfos == null) {
            Log.w(TAG, "[startVoiceWakeupAnyone]wakeupInfo is null,return!");
            return;
        }
        // Now we need only one app to launch in wakeup anyone mode
        VoiceWakeupInfo wakeupInfo = wakeupInfos[mVoiceConfigMgr.getActiveCommandId()];
        List<String> keywordList = new ArrayList<String>();
        List<String> packageList = new ArrayList<String>();
        List<String> classList = new ArrayList<String>();
        for (int i = 0; i < wakeupInfos.length; i++) {
            VoiceWakeupInfo wakeupInfoItem = wakeupInfos[i];
            if (wakeupInfoItem != null) {
                keywordList.add(String.join(", ", wakeupInfoItem.mKeyWord));
                packageList.add(wakeupInfoItem.mPackageName);
                classList.add(wakeupInfoItem.mClassName);
            }
        }
        Intent intent = new Intent("com.mediatek.vow.VOW_NO_SPEAKER_ID");
        intent.putExtra(VoiceUiSettingsActivity.KEY_COMMAND_ID, wakeupInfo.mID);
        intent.putExtra(VoiceUiSettingsActivity.KEY_COMMAND_PACKAGENAME,
            packageList.toArray(new String[packageList.size()]));
        intent.putExtra(VoiceUiSettingsActivity.KEY_COMMAND_CLASSNAME,
            classList.toArray(new String[classList.size()]));
        intent.putExtra(VoiceUiSettingsActivity.KEY_COMMAND_KEYWORD,
            keywordList.toArray(new String[keywordList.size()]));
        Log.d(TAG, "[startVoiceWakeupAnyone]wakeupInfo mID:" + wakeupInfo.mID + ", mPackageName"
            + wakeupInfo.mPackageName + ", mClassName" + wakeupInfo.mClassName + ", mKeyWord:"
            + Arrays.toString(wakeupInfo.mKeyWord) + ", count:" + packageList.size());
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startNewActivity(intent);
    }

    /**
     * Start Voice Wakeup Anyone Training Record Activity.
     */
    private void startVoiceWakeupAnyoneRecord() {
        VoiceWakeupInfo[] wakeupInfos = mVoiceConfigMgr.getCurrentWakeupInfo(mWakeupMode);
        if (wakeupInfos == null) {
            Log.w(TAG, "[startVoiceWakeupAnyoneRecord]wakeupInfo is null,return!");
            return;
        }
        // Now we need only one app to launch in wakeup anyone mode
        VoiceWakeupInfo wakeupInfo = wakeupInfos[mVoiceConfigMgr.getActiveCommandId()];
        ComponentName component = new ComponentName(wakeupInfo.mPackageName, wakeupInfo.mClassName);
        String componetStr = component.flattenToShortString();

        Intent intent = new Intent("com.mediatek.voicewakeup.VOW_COMMAND_RECORD");
        intent.putExtra(VoiceUiSettingsActivity.KEY_COMMAND_ID, wakeupInfo.mID);
        intent.putExtra(VoiceUiSettingsActivity.KEY_COMMAND_VALUE, componetStr);
        intent.putExtra(VoiceUiSettingsActivity.KEY_COMMAND_TYPE,
                VoiceUiSettingsActivity.COMMAND_TYPE_RECORD);
        intent.putExtra(VoiceUiSettingsActivity.KEY_COMMAND_MODE, mWakeupMode);
        // Now we need only one keyword
        intent.putExtra(VoiceUiSettingsActivity.KEY_COMMAND_KEYWORD,
            String.join(",", wakeupInfo.mKeyWord));
        Log.d(TAG,
                "[startVoiceWakeupAnyoneRecord]wakeupInfo mID:" + wakeupInfo.mID + ", componetStr:"
                        + componetStr + ", commandType:"
                        + VoiceUiSettingsActivity.COMMAND_TYPE_RECORD + ", mKeyWord:"
                        + Arrays.toString(wakeupInfo.mKeyWord));
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startNewActivity(intent);
    }

    /**
     * Start Voice Wakeup Command Training Activity.
     */
    private void startVoiceWakeupCommand() {
        VoiceWakeupInfo[] wakeupInfos = mVoiceConfigMgr.getCurrentWakeupInfo(mWakeupMode);
        if (wakeupInfos == null) {
            Log.w(TAG, "[startVoiceWakeupCommand]wakeupInfo is null, return!");
            return;
        }

        List<String> commandList = new ArrayList<String>();
        for (int i = 0; i < wakeupInfos.length; i++) {
            VoiceWakeupInfo wakeupInfo = wakeupInfos[i];
            if (wakeupInfo != null) {
                commandList.add(wakeupInfo.mPackageName + "/" + wakeupInfo.mClassName);
            }
        }
        Intent intent = new Intent("com.mediatek.vow.VOW_WITH_SPEAKER_ID");
        intent.putExtra(VoiceUiSettingsActivity.KEY_COMMAND_LISTS,
                commandList.toArray(new String[commandList.size()]));
        Log.d(TAG, "[startVoiceWakeupCommand]commandList:" + commandList);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        startNewActivity(intent);
    }

    private void startNewActivity(Intent intent) {
        try {
            startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Log.e(TAG, "[startNewActivity]exception:" + e);
            Toast.makeText(VoiceWakeupInteractionActivity.this, R.string.voice_wakeup_missing,
                    Toast.LENGTH_SHORT).show();
        }
    }
}
