/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.audioprofile;

import android.content.Context;
import android.content.res.Resources;
import android.media.AudioManager;
import android.os.Bundle;
import androidx.preference.SwitchPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.util.Log;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.R;
import com.android.settings.SettingsActivity;
import com.android.settings.SettingsPreferenceFragment;
import com.android.settings.search.BaseSearchIndexProvider;
import com.android.settings.search.Indexable;
import com.android.settings.search.SearchIndexableRaw;

import com.mediatek.settings.FeatureOption;

import java.util.ArrayList;
import java.util.List;

public class SoundEnhancement extends SettingsPreferenceFragment implements Indexable {

    private static final String TAG = "SoundEnhancement";

    private Context mContext;
    private AudioManager mAudioManager = null;

    // Sound enhancement
    private static final String KEY_MUSIC_PLUS = "music_plus";
    private static final String KEY_SOUND_ENAHCNE = "sound_enhance";
    private static final String KEY_BESLOUDNESS = "bes_loudness";
    private static final String KEY_ANC = "anc_switch";
    private static final String KEY_HIFI_MODE = "hifi_mode";

    // Audio enhance preference
    private SwitchPreference mMusicPlusPrf;
    //BesLoudness checkbox preference
    private SwitchPreference mBesLoudnessPref;
    // ANC switch preference
    private SwitchPreference mAncPref;
    // Hi-Fi mode switch preference
    private SwitchPreference mHifiModePref;

    // the keys about set/get the status of driver
    private static final String GET_MUSIC_PLUS_STATUS = "GetMusicPlusStatus";
    private static final String GET_MUSIC_PLUS_STATUS_ENABLED = "GetMusicPlusStatus=1";
    private static final String SET_MUSIC_PLUS_ENABLED = "SetMusicPlusStatus=1";
    private static final String SET_MUSIC_PLUS_DISABLED = "SetMusicPlusStatus=0";

    // the params when set/get the status of besloudness
    private static final String GET_BESLOUDNESS_STATUS = "GetBesLoudnessStatus";
    private static final String GET_BESLOUDNESS_STATUS_ENABLED = "GetBesLoudnessStatus=1";
    private static final String SET_BESLOUDNESS_ENABLED = "SetBesLoudnessStatus=1";
    private static final String SET_BESLOUDNESS_DISABLED = "SetBesLoudnessStatus=0";

    // Sound enhance category has no preference
    private static final int SOUND_PREFERENCE_NULL_COUNT = 0;

    private static final String MTK_AUDENH_SUPPORT_State = "MTK_AUDENH_SUPPORT";
    private static final String MTK_AUDENH_SUPPORT_on = "MTK_AUDENH_SUPPORT=true";
    private static final String MTK_AUDENH_SUPPORT_off = "MTK_AUDENH_SUPPORT=false";

    // ANC
    private static final String ANC_UI_STATUS_DISABLED = "ANC_UI=off";
    private static final String ANC_UI_STATUS_ENABLED = "ANC_UI=on";
    private static final String GET_ANC_UI_STATUS = "ANC_UI";

    // Hi-Fi mode
    private static final String HIFI_UI_STATUS_DISABLED = "hifi_dac=off";
    private static final String HIFI_UI_STATUS_ENABLED = "hifi_dac=on";
    private static final String GET_HIFI_UI_STATUS = "hifi_dac";

    private String mAudenhState  = null;

    /**
     * called to do the initial creation of a fragment.
     *
     * @param icicle
     */
    public void onCreate(Bundle icicle) {
        Log.d("@M_" + TAG, "onCreate");
        super.onCreate(icicle);
        mContext = getActivity();

        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        //Query the audenh state
        mAudenhState = mAudioManager.getParameters(MTK_AUDENH_SUPPORT_State);
        Log.d("@M_" + TAG, "AudENH state: " + mAudenhState);
        PreferenceScreen root = getPreferenceScreen();
        if (root != null) {
            root.removeAll();
        }
        addPreferencesFromResource(R.xml.audioprofile_sound_enhancement);

        // get the music plus preference
        mMusicPlusPrf = (SwitchPreference) findPreference(KEY_MUSIC_PLUS);
        mBesLoudnessPref = (SwitchPreference) findPreference(KEY_BESLOUDNESS);
        mAncPref = (SwitchPreference) findPreference(KEY_ANC);
        mHifiModePref = (SwitchPreference) findPreference(KEY_HIFI_MODE);

        if (!mAudenhState.equalsIgnoreCase(MTK_AUDENH_SUPPORT_on)) {
            Log.d("@M_" + TAG, "remove audio enhance preference " + mMusicPlusPrf);
            getPreferenceScreen().removePreference(mMusicPlusPrf);
        }
        if (!FeatureOption.MTK_BESLOUDNESS_SUPPORT) {
            Log.d("@M_" + TAG, "feature option is off, remove BesLoudness preference");
            getPreferenceScreen().removePreference(mBesLoudnessPref);
        }
        if (!FeatureOption.MTK_ANC_SUPPORT) {
            Log.d("@M_" + TAG, "feature option is off, remove ANC preference");
            getPreferenceScreen().removePreference(mAncPref);
        }
        if (!FeatureOption.MTK_HIFI_AUDIO_SUPPORT) {
            Log.d("@M_" + TAG, "feature option is off, remove HiFi preference");
            getPreferenceScreen().removePreference(mHifiModePref);
        }
        setHasOptionsMenu(false);
    }

    private void updatePreferenceHierarchy() {
        // update music plus state
        if (mAudenhState.equalsIgnoreCase(MTK_AUDENH_SUPPORT_on)) {
            String state = mAudioManager.getParameters(GET_MUSIC_PLUS_STATUS);
            Log.d("@M_" + TAG, "get the state: " + state);
            boolean isChecked = false;
            if (state != null) {
                isChecked = state.equals(GET_MUSIC_PLUS_STATUS_ENABLED) ? true
                        : false;
            }
            mMusicPlusPrf.setChecked(isChecked);
        }

        //update Besloudness preference state
        if (FeatureOption.MTK_BESLOUDNESS_SUPPORT) {
            String state = mAudioManager.getParameters(GET_BESLOUDNESS_STATUS);
            Log.d("@M_" + TAG, "get besloudness state: " + state);
            mBesLoudnessPref.setChecked(GET_BESLOUDNESS_STATUS_ENABLED.equals(state));
        }

        if (FeatureOption.MTK_ANC_SUPPORT) {
            String state = mAudioManager.getParameters(GET_ANC_UI_STATUS);
            Log.d("@M_" + TAG, "ANC state: " + state);
            boolean checkedStatus = ANC_UI_STATUS_ENABLED.equals(state);
            mAncPref.setChecked(checkedStatus);
        }

        if (FeatureOption.MTK_HIFI_AUDIO_SUPPORT) {
            String state = mAudioManager.getParameters(GET_HIFI_UI_STATUS);
            Log.d("@M_" + TAG, "HiFi state: " + state);
            boolean checkedStatus = HIFI_UI_STATUS_ENABLED.equals(state);
            mHifiModePref.setChecked(checkedStatus);
        }
    }

    /**
     * called when the fragment is visible to the user Need to update summary
     * and active profile, register for the profile change.
     */
    public void onResume() {
        Log.d("@M_" + TAG, "onResume");
        super.onResume();
        updatePreferenceHierarchy();
    }

    /**
     * Click the preference and enter into the EditProfile.
     *
     * @param preference
     *            the clicked preference
     * @return set success or fail
     */
    public boolean onPreferenceTreeClick(Preference preference) {

        // click the music plus checkbox
        if (mAudenhState.equalsIgnoreCase(MTK_AUDENH_SUPPORT_on)) {
            if (mMusicPlusPrf == preference) {
                boolean enabled = ((SwitchPreference) preference).isChecked();
                String cmdStr = enabled ? SET_MUSIC_PLUS_ENABLED : SET_MUSIC_PLUS_DISABLED;
                Log.d("@M_" + TAG, " set command about music plus: " + cmdStr);
                mAudioManager.setParameters(cmdStr);
            }
        }

        if (FeatureOption.MTK_BESLOUDNESS_SUPPORT) {
            if (mBesLoudnessPref == preference) {
                boolean enabled = ((SwitchPreference) preference).isChecked();
                String cmdStr = enabled ? SET_BESLOUDNESS_ENABLED : SET_BESLOUDNESS_DISABLED;
                Log.d("@M_" + TAG, " set command about besloudness: " + cmdStr);
                mAudioManager.setParameters(cmdStr);
            }
        }

        if (FeatureOption.MTK_ANC_SUPPORT) {
            if (mAncPref == preference) {
                boolean enabled = ((SwitchPreference) preference).isChecked();
                String cmdStr = enabled ? ANC_UI_STATUS_ENABLED : ANC_UI_STATUS_DISABLED;
                Log.d("@M_" + TAG, " set command about besloudness: " + cmdStr);
                mAudioManager.setParameters(cmdStr);
            }
        }

        if (FeatureOption.MTK_HIFI_AUDIO_SUPPORT) {
            if (mHifiModePref == preference) {
                boolean enabled = ((SwitchPreference) preference).isChecked();
                String cmdStr = enabled ? HIFI_UI_STATUS_ENABLED : HIFI_UI_STATUS_DISABLED;
                Log.d("@M_" + TAG, " set command about hifi mode: " + cmdStr);
                mAudioManager.setParameters(cmdStr);
            }
        }

        return super.onPreferenceTreeClick(preference);
    }

    public static final SearchIndexProvider SEARCH_INDEX_DATA_PROVIDER =
    new BaseSearchIndexProvider() {
        @Override
        public List<SearchIndexableRaw> getRawDataToIndex(Context context, boolean enabled) {
            final List<SearchIndexableRaw> result = new ArrayList<SearchIndexableRaw>();
            final Resources res = context.getResources();

            SearchIndexableRaw data = new SearchIndexableRaw(context);
            data.title = res.getString(R.string.sound_enhancement_title);
            data.screenTitle = res.getString(R.string.sound_enhancement_title);
            data.keywords = res.getString(R.string.sound_enhancement_title);
            result.add(data);

            return result;
        }
    };

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.SOUND;
    }
}

