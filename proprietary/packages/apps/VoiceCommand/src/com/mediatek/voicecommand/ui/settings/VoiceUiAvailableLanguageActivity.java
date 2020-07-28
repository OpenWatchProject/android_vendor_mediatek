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

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.util.Log;

/**
 * An activity to show voice ui language hierarchy of preferences to the user.
 * 
 */
public class VoiceUiAvailableLanguageActivity extends PreferenceActivity {
    private static final String TAG = "VoiceUiAvailableLanguageActivity";

    private RadioButtonPreference mLastSelectedPref;
    private int mDefaultLanguage = 0;
    private String[] mAvailableLangs;
    private ConfigurationManager mVoiceConfigMgr;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "[onCreate]...");
        addPreferencesFromResource(R.xml.voice_ui_available_langs);
        mVoiceConfigMgr = ConfigurationManager.getInstance(this);
        mAvailableLangs = mVoiceConfigMgr.getLanguageList();
        mDefaultLanguage = mVoiceConfigMgr.getCurrentLanguage();
        if (mAvailableLangs == null) {
            Log.e(TAG, "[onCreate]mAvailableLangs is null,return!");
            return;
        }
        Log.i(TAG, "[onCreate]voice ui deafult language: " + mAvailableLangs[mDefaultLanguage]
                + ", mAvailableLangs: " + mAvailableLangs.toString());

        for (int j = 0; j < mAvailableLangs.length; j++) {
            RadioButtonPreference pref = new RadioButtonPreference(this, mAvailableLangs[j], "");
            pref.setKey(Integer.toString(j));
            if (mDefaultLanguage == j) {
                pref.setChecked(true);
                mLastSelectedPref = pref;
            }
            if (getPreferenceScreen() != null) {
            getPreferenceScreen().addPreference(pref);
        }
    }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference instanceof RadioButtonPreference) {
            selectLanguage((RadioButtonPreference) preference);
            Log.d(TAG, "[onPreferenceTreeClick]default language changed to "
                    + mAvailableLangs[mDefaultLanguage]);
            finish();
        }
        return true;
    }

    private void selectLanguage(RadioButtonPreference preference) {
        if (mLastSelectedPref != null) {
            if (mLastSelectedPref == preference) {
                Log.i(TAG, "[selectLanguage]mLastSelectedPref==preference.");
                return;
            }
            mLastSelectedPref.setChecked(false);
        }
        mDefaultLanguage = Integer.parseInt(preference.getKey().toString());
        Log.d(TAG, "[selectLanguage]set default language to " + mAvailableLangs[mDefaultLanguage]);
        mVoiceConfigMgr.setCurrentLanguage(mDefaultLanguage);
        preference.setChecked(true);
        mLastSelectedPref = preference;
    }

}
