/*
 *  Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation
 * ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.audio;

import android.app.Activity;
import android.content.Intent;
import android.media.AudioSystem;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

/** The Audio activity which is a entry of Audio debug functions. */
public class Audio extends Activity implements OnItemClickListener {
    /**
     * . CURRENT_MODE is in order to distinguish the headset, normal and loud
     * speaker mode
     */
    public static final String CURRENT_MODE = "CurrentMode";
    public static final String ENHANCE_MODE = "is_enhance";
    public static final String AUDIO_VERSION_COMMAND = "GET_AUDIO_VOLUME_VERSION";
    public static final String AUDIO_VERSION_1 = "GET_AUDIO_VOLUME_VERSION=1";
    private static final String AUDIO_TURNING_VER = "ro.vendor.mtk_audio_tuning_tool_ver";
    private static final String AUDIO_TURNING_VER_V1 = "V1";
    private static final String AUDIO_TURNING_VER_V2_1 = "V2.1";
    private static final String AUDIO_TURNING_VER_V2_2 = "V2.2";
    /** Log Tag. */
    public static final String TAG = "Audio";
    /** Used for start NORMAL MODE activity. */
    private static final String NORMAL_MODE_NAME = "Normal Mode";
    /** Used for start HEADSET MODE activity. */
    private static final String HEADSET_MODE_NAME = "Headset Mode";
    /** Used for start LOUDSPEAKER MODE activity. */
    private static final String LOUDSPEAKER_MODE_NAME = "LoudSpeaker Mode";
    /** Used for start HEADSET LOUDSPEAKER MODE activity. */
    private static final String HEADSET_LOUDSPEAKER_MODE_NAME = "Headset_LoudSpeaker Mode";
    /** Used for start volume activity. */
    private static final String VOLUME_NAME = "Volume";
    /** Used for start SPEECH ENHANCE activity. */
    private static final String SPEECH_ENHANCE_NAME = "Speech Enhancement";
    /** Used for start DEBUG INFO activity. */
    private static final String DEBUG_INFO_NAME = "Debug Info";
    private static final String DEBUG_SESSION_NAME = "Debug Session";
    /** Used for start SPEECH LOGGER activity. */
    private static final String SPEECH_LOGGER_NAME = "Speech Logger";
    /** Used for start AUDIO LOGGER activity. */
    private static final String AUDIO_LOGGER_NAME = "Audio Logger";
    private boolean mIsVolumeEnhancementMode = true;
    private ListView mAudioListView;
    private ArrayList<String> mListData;

    /**
     * Audio tuning version.
     *
     */
    enum AudioTuningVer {
        UNKNOWN,
        VER_1,
        VER_2_1,
        VER_2_2
    };

    private static AudioTuningVer sAudioTuningVer = AudioTuningVer.UNKNOWN;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio);
        initAudioTunVer(); //Get audio tuning version
        initVolumeModeVer(); //Get if volume support HEADSET_LOUDSPEAKER_MODE

        initListItem(); //Init listview based on the information above
    }

    private static void initAudioTunVer() {
        String value = SystemProperties.get(AUDIO_TURNING_VER);
//        String value = ChipSupport.getAudioTuningVersion();
        Elog.d(TAG, "initAudioTurnVer:" + value);
        sAudioTuningVer = AudioTuningVer.VER_1;

        if (value != null) {
            if (AUDIO_TURNING_VER_V2_1.equalsIgnoreCase(value)) {
                sAudioTuningVer = AudioTuningVer.VER_2_1;
            }

            if (AUDIO_TURNING_VER_V2_2.equalsIgnoreCase(value)) {
                sAudioTuningVer = AudioTuningVer.VER_2_2;
                boolean result = AudioTuningJni.registerXmlChangedCallback();
                if ( result == true )
                    Elog.d(TAG, "registerXmlChangedCallback OK!");
                else
                    Elog.d(TAG, "registerXmlChangedCallback failed!");
            }
        }
        Elog.d(TAG, "sAudioTuningVer:" + sAudioTuningVer);
    }
    private void initVolumeModeVer() {
        String version = AudioSystem.getParameters(AUDIO_VERSION_COMMAND);
        Elog.d(TAG, "initVolumeModeVer:" + version);
        if (!AUDIO_VERSION_1.equals(version)) {
            mIsVolumeEnhancementMode = false;
        }
    }
    private void initListItem() {

        mListData = new ArrayList<String>();
        if (sAudioTuningVer != AudioTuningVer.VER_2_2) {
            mListData.add(NORMAL_MODE_NAME);
            mListData.add(HEADSET_MODE_NAME);
            mListData.add(LOUDSPEAKER_MODE_NAME);
            if (mIsVolumeEnhancementMode) {
                mListData.add(HEADSET_LOUDSPEAKER_MODE_NAME);
            }
        } else {
            mListData.add(VOLUME_NAME);
        }
        mListData.add(SPEECH_ENHANCE_NAME);
        if (sAudioTuningVer == AudioTuningVer.VER_1) {
            mListData.add(DEBUG_INFO_NAME);
        }
        mListData.add(DEBUG_SESSION_NAME);
        mListData.add(SPEECH_LOGGER_NAME);
        mListData.add(AUDIO_LOGGER_NAME);

        mAudioListView = (ListView) findViewById(R.id.ListView_Audio);
        final ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mListData);
        mAudioListView.setAdapter(adapter);
        mAudioListView.setOnItemClickListener(this);

    }

    /**
     * Click List view items to start different activity.
     *
     * @param arg0
     *            : View adapter.
     * @param arg1
     *            : Selected view.
     * @param arg2
     *            : Selected view's position.
     * @param arg3
     *            : Selected view's id
     */
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        String itemName = mListData.get(arg2);
        final Intent intent = new Intent();
        if (NORMAL_MODE_NAME.equalsIgnoreCase(itemName)
                || HEADSET_MODE_NAME.equalsIgnoreCase(itemName)
                || LOUDSPEAKER_MODE_NAME.equalsIgnoreCase(itemName)
                || HEADSET_LOUDSPEAKER_MODE_NAME.equalsIgnoreCase(itemName)) {
            intent.setClass(this, AudioModeSetting.class);
            intent.putExtra(CURRENT_MODE, arg2);
            intent.putExtra(ENHANCE_MODE, mIsVolumeEnhancementMode);
        } else if (VOLUME_NAME.equalsIgnoreCase(itemName)) {
            intent.setClass(this, AudioVolume.class);
        } else if (SPEECH_ENHANCE_NAME.equalsIgnoreCase(itemName)) {
            if (sAudioTuningVer == AudioTuningVer.VER_1) {
                intent.setClass(this, AudioSpeechEnhancementV1.class);

            } else {
                intent.setClass(this, AudioSpeechEnhancementV2.class);
            }
        } else if (DEBUG_INFO_NAME.equalsIgnoreCase(itemName)) {
            intent.setClass(this, AudioDebugInfo.class);
        } else if (DEBUG_SESSION_NAME.equalsIgnoreCase(itemName)) {
            intent.setClass(this, AudioDebugSession.class);
        } else if (SPEECH_LOGGER_NAME.equalsIgnoreCase(itemName)) {
            if (sAudioTuningVer == AudioTuningVer.VER_1) {
                intent.setClass(this, AudioSpeechLoggerXV1.class);
            } else {
                intent.setClass(this, AudioSpeechLoggerXV2.class);
            }
        } else if (AUDIO_LOGGER_NAME.equalsIgnoreCase(itemName)) {
            intent.setClass(this, AudioAudioLogger.class);
        } else {
            return;
        }
        this.startActivity(intent);
    }

    static AudioTuningVer getAudioTuningVer() {
        if(AudioTuningVer.UNKNOWN == sAudioTuningVer) {
            initAudioTunVer();
        }
        return sAudioTuningVer;
    }

}
