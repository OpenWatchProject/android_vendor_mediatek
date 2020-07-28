/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
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

package com.mediatek.engineermode.audio;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.media.AudioSystem;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * AudioSpeechLogger for AUDIO_TUNING_TOOL v1.
 *
 */
public class AudioSpeechLoggerXV1 extends Activity {
    public static final String TAG = "Audio/SpeechLogger1";
    public static final String ENGINEER_MODE_PREFERENCE = "engineermode_audiolog_preferences";
    public static final String EPL_STATUS = "epl_status";
    public static final String ANC_STATUS = "anc_status";

    private static final String ANC_DOWN_SAMPLE = "downSample";
    private static final String ANC_NO_DOWN_SAMPLE = "noDownSample";
    private static final int DATA_SIZE = 1444;
    private static final int VM_LOG_POS = 1440;

    private static final int SET_SPEECH_VM_ENABLE = 0x60;
    private static final int SET_DUMP_SPEECH_DEBUG_INFO = 0x61;
    private static final int SET_DUMP_AP_SPEECH_EPL = 0xA0;
    private static final int GET_DUMP_AP_SPEECH_EPL = 0xA1;
    private static final int DIALOG_GET_DATA_ERROR = 0;
    private static final int DIALOG_ID_NO_SDCARD = 1;
    private static final int DIALOG_ID_SDCARD_BUSY = 2;
    private static final int CONSTANT_256 = 256;
    private static final int CONSTANT_0XFF = 0xFF;
    private static final int GET_SPEECH_ANC_SUPPORT = 0xB0;
    private static final int SET_SPEECH_ANC_LOG_STATUS = 0xB3;
    private static final int SET_SPEECH_ANC_DISABLE = 0xB4;
    private static final int ENABLE_ANC_DOWN_SAMPLE = 1;
    private static final int ENABLE_ANC_NO_DOWN_SAMPLE = 0;
    private static final int GET_SPEECH_ANC_LOG_STATUS = 0xB5;

    private CheckBox mCbSpeechLogger;
    private CheckBox mCbEplDebug;
    private CheckBox mCbCtm4Way;
    private CheckBox mCbAncLogger;
    private CheckBox mCbMagiConf;
    private RadioGroup mRgAnc;
    private RadioButton mRbEpl;
    private RadioButton mRbNormalVm;

    private byte[] mData;
    private int mVmLogState = 0;
    private boolean mForRefresh = false; // Sloved radiobutton can not checked

    private final CheckBox.OnCheckedChangeListener mCheckedListener =
        new CheckBox.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean checked) {
                SharedPreferences preferences =
                        getSharedPreferences(ENGINEER_MODE_PREFERENCE, MODE_PRIVATE);
                Editor edit = preferences.edit();
                if (buttonView.equals(mCbSpeechLogger)) {
                    onClickSpeechLogger(edit, checked);
                } else if (buttonView.equals(mCbCtm4Way)) {
                    if (checked) {
                        mData[VM_LOG_POS] |= 0x02;
                        mVmLogState |= 0x02;
                        Elog.d(TAG, "E mVmLogState " + mVmLogState);
                    } else {
                        mData[VM_LOG_POS] &= ~0x02;
                        mVmLogState &= ~0x02;
                        Elog.d(TAG, "D mVmLogState " + mVmLogState);
                    }
                    int index = AudioTuningJni.setEmParameter(mData, DATA_SIZE);
                    if (index != 0 && index != -38) {
                        Elog.i(TAG, "set CTM4WAY parameter failed");
                        Toast.makeText(AudioSpeechLoggerXV1.this,
                                R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                    }
                } else if (buttonView.equals(mCbEplDebug)) {
                    if (checked) {
                        Elog.d(TAG, "mCKEPLDebug checked");
                        int ret = AudioTuningJni.setAudioCommand(SET_DUMP_AP_SPEECH_EPL, 1);
                    } else {
                        Elog.d(TAG, "mCKEPLDebug Unchecked");
                        int ret = AudioTuningJni.setAudioCommand(SET_DUMP_AP_SPEECH_EPL, 0);
                    }
                } else if (buttonView.equals(mRbEpl)) {
                    if (checked) {
                        Elog.d(TAG, "mCKBEPL checked");
                        int ret = AudioTuningJni.setAudioCommand(SET_SPEECH_VM_ENABLE, 1);
                        AudioTuningJni.getEmParameter(mData, DATA_SIZE);
                        if (ret == -1) {
                            Elog.i(TAG, "set mCKBEPL parameter failed");
                            Toast.makeText(AudioSpeechLoggerXV1.this,
                                    R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                        }
                        edit.putInt(EPL_STATUS, 1);
                        edit.commit();
                    } else {
                        Elog.d(TAG, "mCKBEPL unchecked");
                    }
                } else if (buttonView.equals(mRbNormalVm)) {
                    if (checked) {
                        Elog.d(TAG, "mCKBNormalVm checked");
                        if (mForRefresh) {
                            mForRefresh = false;
                        } else {
                            Elog.d(TAG, "mCKBNormalVm checked ok");
                            int ret = AudioTuningJni.setAudioCommand(SET_SPEECH_VM_ENABLE, 0);
                            AudioTuningJni.getEmParameter(mData, DATA_SIZE);
                            if (ret == -1) {
                                Elog.i(TAG, "set mCKBNormalVm parameter failed");
                                Toast.makeText(AudioSpeechLoggerXV1.this,
                                        R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                            }
                            edit.putInt(EPL_STATUS, 0);
                            edit.commit();
                        }
                    } else {
                        Elog.d(TAG, "mCKBNormalVm unchecked");
                    }
                } else if (buttonView.equals(mCbAncLogger)) {
                    for (int i = 0; i < mRgAnc.getChildCount(); i++) {
                        mRgAnc.getChildAt(i).setEnabled(checked);
                    }
                    if (checked) {
                        Elog.d(TAG, "mCbSpeechLogger checked");
                        int ret = AudioTuningJni.setAudioCommand(
                                SET_SPEECH_ANC_LOG_STATUS, ENABLE_ANC_DOWN_SAMPLE);
                        mRgAnc.check(R.id.Audio_AncLogger_Down);

                    } else {
                        Elog.d(TAG, "mCbSpeechLogger unchecked");
                        int ret = AudioTuningJni.setAudioCommand(SET_SPEECH_ANC_DISABLE, 0);
                    }
                    edit.putString(ANC_STATUS, ANC_DOWN_SAMPLE);
                    edit.commit();

                } else if (buttonView.equals(mCbMagiConf)) {
                    if (checked) {
                        Elog.d(TAG, "mCbMagiConf checked");
                        int ret = AudioSystem.setParameters("SET_MAGI_CONFERENCE_ENABLE=1");
                    } else {
                        Elog.d(TAG, "mCbMagiConf Unchecked");
                        int ret = AudioSystem.setParameters("SET_MAGI_CONFERENCE_ENABLE=0");
                    }
                }
            }
        };
    private final RadioGroup.OnCheckedChangeListener mRgCheckedListener =
        new RadioGroup.OnCheckedChangeListener() {
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if (group.equals(mRgAnc)) {
                    SharedPreferences preferences =
                        getSharedPreferences(ENGINEER_MODE_PREFERENCE, MODE_PRIVATE);
                    Editor edit = preferences.edit();
                    if (R.id.Audio_AncLogger_Down == checkedId) {
                        Elog.d(TAG, "Audio_AncLogger_Down checked");
                        int ret = AudioTuningJni.setAudioCommand(
                                SET_SPEECH_ANC_LOG_STATUS, ENABLE_ANC_DOWN_SAMPLE);
                        edit.putString(ANC_STATUS, ANC_DOWN_SAMPLE);
                        edit.commit();
                    } else if (R.id.Audio_AncLogger_NoDown == checkedId) {
                        Elog.d(TAG, "Audio_SpeechLogger_NoDown checked");
                        int ret = AudioTuningJni.setAudioCommand(
                                SET_SPEECH_ANC_LOG_STATUS, ENABLE_ANC_NO_DOWN_SAMPLE);
                        edit.putString(ANC_STATUS, ANC_NO_DOWN_SAMPLE);
                        edit.commit();
                    }

                }

            }
    };
    private void onClickSpeechLogger(Editor edit, boolean checked) {
        if (checked) {
            Elog.d(TAG, "mCbSpeechLogger checked");
            mRbEpl.setEnabled(true);
            mRbNormalVm.setEnabled(true);
            mForRefresh = true;
            mRbNormalVm.setChecked(true);
            mRbEpl.setChecked(true);
            mData[VM_LOG_POS] |= 0x01;
            int index = AudioTuningJni.setEmParameter(mData, DATA_SIZE);
            if (index != 0 && index != -38) {
                Elog.i(TAG, "set mAutoVM parameter failed");
                Toast.makeText(this, R.string.set_failed_tip, Toast.LENGTH_LONG).show();
            }
        } else {
            Elog.d(TAG, "mCbSpeechLogger unchecked");
            if (mRbEpl.isChecked()) {
                mRbEpl.setChecked(false);
            }
            if (mRbNormalVm.isChecked()) {
                mRbNormalVm.setChecked(false);
            }

            mRbEpl.setEnabled(false);
            mRbNormalVm.setEnabled(false);
            AudioTuningJni.getEmParameter(mData, DATA_SIZE);
            edit.putInt(EPL_STATUS, 0);
            edit.commit();
            mData[VM_LOG_POS] &= ~0x01;
            int index = AudioTuningJni.setEmParameter(mData, DATA_SIZE);
            if (index != 0 && index != -38) {
                Elog.i(TAG, "set mAutoVM parameter failed");
                Toast.makeText(this, R.string.set_failed_tip, Toast.LENGTH_LONG).show();
            }
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio_speechloggerx);

        mCbSpeechLogger = (CheckBox) findViewById(R.id.Audio_SpeechLogger_Enable);
        mCbEplDebug = (CheckBox) findViewById(R.id.Audio_EPLDebug_Enable);
        mCbCtm4Way = (CheckBox) findViewById(R.id.Audio_CTM4WAYLogger_Enable);
        mCbAncLogger = (CheckBox) findViewById(R.id.Audio_AncLogger_Enable);
        mCbMagiConf = (CheckBox) findViewById(R.id.Audio_MagiConference_Enable);
        mRbEpl = (RadioButton) findViewById(R.id.Audio_SpeechLogger_EPL);
        mRbNormalVm = (RadioButton) findViewById(R.id.Audio_SpeechLogger_Normalvm);
        mRgAnc = (RadioGroup) findViewById(R.id.RadioGroup2);
        Button dumpSpeechInfo = (Button) findViewById(R.id.Dump_Speech_DbgInfo);
        View spliteView = (View) this.findViewById(R.id.Audio_View1);
        TextView ctm4WayText = (TextView) findViewById(R.id.Audio_CTM4WAYLogger_EnableText);
        View spliteView3 = (View) this.findViewById(R.id.Audio_View3);
        TextView ancText = (TextView) findViewById(R.id.Audio_AncLogger_FileText);
        if (!AudioTuningJni.isFeatureSupported("MTK_TTY_SUPPORT")) {
            mCbCtm4Way.setVisibility(View.GONE);
            ctm4WayText.setVisibility(View.GONE);
            spliteView.setVisibility(View.GONE);
        }
        int ancSupport = AudioTuningJni.getAudioCommand(GET_SPEECH_ANC_SUPPORT);
        if (ancSupport == 0) {
            ancText.setVisibility(View.GONE);
            mCbAncLogger.setVisibility(View.GONE);
            mRgAnc.setVisibility(View.GONE);
            spliteView3.setVisibility(View.GONE);
        } else {
            initAncStatus();
        }
        String magiSupport = AudioSystem.getParameters("GET_MAGI_CONFERENCE_SUPPORT");
        Elog.i(TAG, "Get Magi support " + magiSupport);
        String[] magiStr = magiSupport.split("=");
        if (false) { // force to hiden MAGI_CONFERENCE_SUPPORT
        //if (magiStr.length >= 2 && magiStr[1].equals("1")) { // GET_MAGI_CONFERENCE_SUPPORT=1
           String magiEnable = AudioSystem.getParameters("GET_MAGI_CONFERENCE_ENABLE");
           Elog.i(TAG, "Get Magi Enable " + magiEnable);
           String[] strEnable = magiEnable.split("=");
           if (strEnable.length >= 2 && strEnable[1].equals("1")) {
               mCbMagiConf.setChecked(true);
           } else {
               mCbMagiConf.setChecked(false);
           }
        } else {
            findViewById(R.id.Audio_View2).setVisibility(View.GONE);
            findViewById(R.id.Audio_MagiConference_EnableText).setVisibility(View.GONE);
            mCbMagiConf.setVisibility(View.GONE);
        }
        checkStatus();

        mCbSpeechLogger.setOnCheckedChangeListener(mCheckedListener);
        mCbEplDebug.setOnCheckedChangeListener(mCheckedListener);
        mCbCtm4Way.setOnCheckedChangeListener(mCheckedListener);
        mCbAncLogger.setOnCheckedChangeListener(mCheckedListener);
        mCbMagiConf.setOnCheckedChangeListener(mCheckedListener);
        mRbEpl.setOnCheckedChangeListener(mCheckedListener);
        mRbNormalVm.setOnCheckedChangeListener(mCheckedListener);
        mRgAnc.setOnCheckedChangeListener(mRgCheckedListener);
        dumpSpeechInfo.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                Elog.d(TAG, "On Click mDumpSpeechInfo button.");
                int ret = AudioTuningJni.setAudioCommand(SET_DUMP_SPEECH_DEBUG_INFO, 1);
                if (ret == -1) {
                    Elog.i(TAG, "set mDumpSpeechInfo parameter failed");
                    Toast.makeText(AudioSpeechLoggerXV1.this,
                            R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(AudioSpeechLoggerXV1.this,
                            R.string.set_success_tip, Toast.LENGTH_LONG).show();
                }
            }
        });
    }

    private void checkStatus() {
        final SharedPreferences preferences =
                getSharedPreferences(ENGINEER_MODE_PREFERENCE, MODE_PRIVATE);
        final int eplStatus = preferences.getInt(EPL_STATUS, 1);

        mData = new byte[DATA_SIZE];
        int ret = AudioTuningJni.getEmParameter(mData, DATA_SIZE);
        if (ret != 0) {
            showDialog(DIALOG_GET_DATA_ERROR);
            Elog.i(TAG, "Audio_SpeechLogger GetEMParameter return value is : " + ret);
        }

        mVmLogState = shortToInt(mData[VM_LOG_POS], mData[VM_LOG_POS + 1]);
        Elog.i(TAG, "Audio_SpeechLogger GetEMParameter return value is : " + mVmLogState);

        if ((mVmLogState & 0x01) == 0) {
            mCbSpeechLogger.setChecked(false);
            mRbEpl.setEnabled(false);
            mRbNormalVm.setEnabled(false);
            mRbEpl.setChecked(false);
            mRbNormalVm.setChecked(false);
        } else {
            mCbSpeechLogger.setChecked(true);
            mRbEpl.setEnabled(true);
            mRbNormalVm.setEnabled(true);
            if (eplStatus == 1) {
                mRbEpl.setChecked(true);
            } else {
                mRbNormalVm.setChecked(true);
            }
        }

        if ((mVmLogState & 0x02) == 0) {
            mCbCtm4Way.setChecked(false);
        } else {
            mCbCtm4Way.setChecked(true);
        }
        int epl = AudioTuningJni.getAudioCommand(GET_DUMP_AP_SPEECH_EPL);
        Elog.i(TAG, "Get EPL setting: " + epl);
        if (epl == 1) {
            mCbEplDebug.setChecked(true);
        } else {
            mCbEplDebug.setChecked(false);
        }
    }
    private void initAncStatus() {
        final SharedPreferences preferences =
                getSharedPreferences(ENGINEER_MODE_PREFERENCE, MODE_PRIVATE);
        int anc = AudioTuningJni.getAudioCommand(GET_SPEECH_ANC_LOG_STATUS);
        Elog.i(TAG, "Get ANC setting: " + anc);
        if (anc == 1) {
            mCbAncLogger.setChecked(true);
            final String ancStatus = preferences.getString(ANC_STATUS, ANC_DOWN_SAMPLE);
            Elog.i(TAG, "Get ANC status: " + ancStatus);
            if (ancStatus.equals(ANC_DOWN_SAMPLE)) {
                mRgAnc.check(R.id.Audio_AncLogger_Down);
            } else {
                mRgAnc.check(R.id.Audio_AncLogger_NoDown);
            }
        } else {
            mCbAncLogger.setChecked(false);
            for (int i = 0; i < mRgAnc.getChildCount(); i++) {
                mRgAnc.getChildAt(i).setEnabled(false);
            }
        }
    }





    private int shortToInt(byte low, byte high) {
        int temp = CONSTANT_0XFF & (high + CONSTANT_256);
        int highByte = CONSTANT_256 * temp;
        int lowByte = CONSTANT_0XFF & (low + CONSTANT_256);
        return highByte + lowByte;
    }

    @Override
    public Dialog onCreateDialog(int id) {
        switch (id) {
        case DIALOG_GET_DATA_ERROR:
            return new AlertDialog.Builder(this)
                    .setTitle(R.string.get_data_error_title)
                    .setMessage(R.string.get_data_error_msg)
                    .setPositiveButton(android.R.string.ok,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    AudioSpeechLoggerXV1.this.finish();
                                }
                            }).create();
        case DIALOG_ID_NO_SDCARD:
            return new AlertDialog.Builder(this)
                    .setTitle(R.string.no_sdcard_title)
                    .setMessage(R.string.no_sdcard_msg)
                    .setPositiveButton(android.R.string.ok, null).create();
        case DIALOG_ID_SDCARD_BUSY:
            return new AlertDialog.Builder(this)
                    .setTitle(R.string.sdcard_busy_title)
                    .setMessage(R.string.sdcard_busy_msg)
                    .setPositiveButton(android.R.string.ok, null).create();
        default:
            return super.onCreateDialog(id);
        }
    }
}
