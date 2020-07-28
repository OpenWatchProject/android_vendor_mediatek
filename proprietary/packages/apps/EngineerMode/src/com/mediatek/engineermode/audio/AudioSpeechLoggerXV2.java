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
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

/**
 * AudioSpeechLogger for AUDIO_TUNING_TOOL v1.
 *
 */
public class AudioSpeechLoggerXV2 extends Activity  {
    public static final String TAG = "Audio/SpeechLogger2";
    private static final String PARAM_DEVIDER = "#";
    private static final String VERSION_DEVIDER = "=";
    public static final String ENGINEER_MODE_PREFERENCE = "engineermode_audiolog_preferences";
    public static final String EPL_STATUS = "epl_status";
    public static final String ANC_STATUS = "anc_status";
    private static final String CMD_GET_VMLOG_STATE = "GET_VMLOG_CONFIG";
//            "APP_GET_PARAM=AudioCommonSetting#CategoryLayer,Common#vmlog_enable";
    public static final String CMD_SET_VMLOG_STATE = "SET_VMLOG_CONFIG=";
//            "APP_SET_PARAM=AudioCommonSetting#CategoryLayer,Common#vmlog_enable#";
    private static final String CMD_GET_COMMON_PARAMETER =
            "APP_GET_PARAM=SpeechGeneral#CategoryLayer,Common#speech_common_para";
    private static final String CMD_SET_COMMON_PARAMETER =
            "APP_SET_PARAM=SpeechGeneral#CategoryLayer,Common#speech_common_para#";
    private static final String CMD_GET_DEBUG_INFO =
            "APP_GET_PARAM=SpeechGeneral#CategoryLayer,Common#debug_info";
    private static final String CMD_SET_DEBUG_INFO =
            "APP_SET_PARAM=SpeechGeneral#CategoryLayer,Common#debug_info#";
    private static final String CMD_PREFIX =
            "APP_GET_PARAM=";

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
    private static final int DIALOG_ID_DUMP_PATH_CHANGE = 10;
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
    private Button mCopy2SdButton;
    private Button mClearDumpButton;
    private TextView mCopyProgressTv;

//    private byte[] mData;
//    private int mVmLogState = 0;
    private boolean mForRefresh = false; // Sloved radiobutton can not checked

    AudioManager mAudioManager;
    private static final String CUST_XML_PARAM = "GET_CUST_XML_ENABLE";
    private static final String CUST_XML_SET_SUPPORT_PARAM = "SET_CUST_XML_ENABLE=1";
    private static final String CUST_XML_SET_UNSUPPORT_PARAM = "SET_CUST_XML_ENABLE=0";
    private static final String RESULT_SUPPORT = "GET_CUST_XML_ENABLE=1";
    private static final String RESULT_UNSUPPORT = "GET_CUST_XML_ENABLE=0";

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
                        mAudioManager.setParameters(CMD_SET_VMLOG_STATE + "2");
                    } else {
                        mAudioManager.setParameters(CMD_SET_VMLOG_STATE + "0");
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
                        if (!FeatureSupport.isEngLoad()) {
                            String check = mAudioManager.getParameters(CUST_XML_PARAM);
                            if ((check != null) && (RESULT_SUPPORT.equals(check))) {
                                Elog.d(TAG, "get CUST_XML_PARAM = 1");
                            } else {
                                Elog.d(TAG, "set CUST_XML_PARAM = 1");
                                mAudioManager.setParameters(CUST_XML_SET_SUPPORT_PARAM );
                                AudioTuningJni.CustXmlEnableChanged(1);
                            }
                        }
                        String ret = getParameters(CMD_GET_DEBUG_INFO);
                        Elog.i(TAG, "getParameters " + ret);
                        if (ret != null) {
                            String[] entries = ret.split(",");
                            if (entries.length >= 1) {
                                entries[0] = "0x3";
                                String cmd = CMD_SET_DEBUG_INFO;
                                for (int i = 0; i < entries.length; i++) {
                                    cmd += entries[i] + ",";
                                }
                                cmd = cmd.substring(0, cmd.length() - 1);
                                Elog.i(TAG, "setParameters " + cmd);
                                setParameters(cmd);
                            } else {
                                Elog.i(TAG, "set mCKBEPL parameter failed");
                                Toast.makeText(AudioSpeechLoggerXV2.this,
                                        R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                            }
                        } else {
                            Elog.i(TAG, "set mCKBEPL parameter failed");
                            Toast.makeText(AudioSpeechLoggerXV2.this,
                                    R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                        }

                        ret = getParameters(CMD_GET_COMMON_PARAMETER);
                        Elog.i(TAG, "getParameters " + ret);
                        if (ret != null) {
                            String[] entries = ret.split(",");
                            if (entries.length >= 1) {
                                entries[0] = "0x6";
                                String cmd = CMD_SET_COMMON_PARAMETER;
                                for (int i = 0; i < entries.length; i++) {
                                    cmd += entries[i] + ",";
                                }
                                cmd = cmd.substring(0, cmd.length() - 1);
                                Elog.i(TAG, "setParameters " + cmd);
                                setParameters(cmd);
                            } else {
                                Elog.i(TAG, "set mCKBEPL parameter failed");
                                Toast.makeText(AudioSpeechLoggerXV2.this,
                                        R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                            }
                        } else {
                            Elog.i(TAG, "set mCKBEPL parameter failed");
                            Toast.makeText(AudioSpeechLoggerXV2.this,
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
                        if (!FeatureSupport.isEngLoad()) {
                            String check = mAudioManager.getParameters(CUST_XML_PARAM);
                            if ((check != null) && (RESULT_SUPPORT.equals(check))) {
                                Elog.d(TAG, "get CUST_XML_PARAM = 1");
                            } else {
                                Elog.d(TAG, "set CUST_XML_PARAM = 1");
                                mAudioManager.setParameters(CUST_XML_SET_SUPPORT_PARAM );
                                AudioTuningJni.CustXmlEnableChanged(1);
                            }
                        }
                        if (mForRefresh) {
                            mForRefresh = false;
                        } else {
                            Elog.d(TAG, "mCKBNormalVm checked ok");
                            String ret = getParameters(CMD_GET_DEBUG_INFO);
                            Elog.i(TAG, "getParameters " + ret);
                            if (ret != null) {
                                String[] entries = ret.split(",");
                                if (entries.length >= 1) {
                                    if (!"0x0".equals(entries[0])) {
                                        entries[0] = "0x0";
                                        String cmd = CMD_SET_DEBUG_INFO;
                                        for (int i = 0; i < entries.length; i++) {
                                            cmd += entries[i] + ",";
                                        }
                                        cmd = cmd.substring(0, cmd.length() - 1);
                                        Elog.i(TAG, "setParameters " + cmd);
                                        setParameters(cmd);
                                    }
                                } else {
                                    Elog.i(TAG, "set mCKBEPL parameter failed");
                                    Toast.makeText(AudioSpeechLoggerXV2.this,
                                            R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                                }
                            } else {
                                Elog.i(TAG, "set mCKBEPL parameter failed");
                                Toast.makeText(AudioSpeechLoggerXV2.this,
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
            mAudioManager.setParameters(CMD_SET_VMLOG_STATE + "1");
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
            mAudioManager.setParameters(CMD_SET_VMLOG_STATE + "0");
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio_speechloggerx);

        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
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
                    Toast.makeText(AudioSpeechLoggerXV2.this,
                            R.string.set_failed_tip, Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(AudioSpeechLoggerXV2.this,
                            R.string.set_success_tip, Toast.LENGTH_LONG).show();
                }
            }
        });
        showDialog(DIALOG_ID_DUMP_PATH_CHANGE);

        // Add for Copy Audio HAL Dump To SDCard feature
        Intent intent = new Intent();
        intent.setClass(this, CopyHalDumpService.class);
        startForegroundService(intent);
        initCopyDumpUi();
        // End or Copy Audio HAL Dump To SDCard feature
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private void checkStatus() {
        final SharedPreferences preferences =
                getSharedPreferences(ENGINEER_MODE_PREFERENCE, MODE_PRIVATE);
        final int eplStatus = preferences.getInt(EPL_STATUS, 1);
        String vmLogState = mAudioManager.getParameters(CMD_GET_VMLOG_STATE);
        Elog.i(TAG, "return value is : " + vmLogState);
        if (vmLogState != null && vmLogState.length() > CMD_GET_VMLOG_STATE.length() + 1) {
            vmLogState = vmLogState.substring(CMD_GET_VMLOG_STATE.length() + 1);
        }

        if (!"1".equals(vmLogState)) {
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
            if (!FeatureSupport.isEngLoad()) {
                String check = mAudioManager.getParameters(CUST_XML_PARAM);
                if ((check != null) && (RESULT_SUPPORT.equals(check))) {
                    Elog.d(TAG, "get CUST_XML_PARAM = 1");
                } else {
                    Elog.d(TAG, "get CUST_XML_PARAM = 0");
                    mCbSpeechLogger.setChecked(false);
                    mRbEpl.setEnabled(false);
                    mRbNormalVm.setEnabled(false);
                    mRbEpl.setChecked(false);
                    mRbNormalVm.setChecked(false);
                }
            }
        }
        mCbCtm4Way.setOnCheckedChangeListener(null);
        if (!"2".equals(vmLogState)) {
            mCbCtm4Way.setChecked(false);
        } else {
            mCbCtm4Way.setChecked(true);
        }
        mCbCtm4Way.setOnCheckedChangeListener(mCheckedListener);
        int epl = AudioTuningJni.getAudioCommand(GET_DUMP_AP_SPEECH_EPL);
        Elog.i(TAG, "Get EPL setting: " + epl);
        mCbEplDebug.setOnCheckedChangeListener(null);
        if (epl == 1) {
            mCbEplDebug.setChecked(true);
        } else {
            mCbEplDebug.setChecked(false);
        }
        mCbEplDebug.setOnCheckedChangeListener(mCheckedListener);
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
                                    AudioSpeechLoggerXV2.this.finish();
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
        case DIALOG_ID_DUMP_PATH_CHANGE:
            return new AlertDialog.Builder(this)
                    .setTitle(R.string.audio_warning_title)
                    .setMessage(R.string.audio_dump_path_change_msg)
                    .setPositiveButton(android.R.string.ok, null).create();
        default:
            return super.onCreateDialog(id);
        }
    }
    private String getParameters(String command) {
        if (Audio.AudioTuningVer.VER_2_2 != Audio.getAudioTuningVer()) {
            String result = mAudioManager.getParameters(command);
            if (result != null && result.length() > CMD_PREFIX.length()) {
                result = result.substring(CMD_PREFIX.length());
            }
            Elog.i(TAG, "getParameters return " + result);
            return result;
        } else {
        int index = command.indexOf(VERSION_DEVIDER);
        command = command.substring(index + 1);
        String[] params = command.split(PARAM_DEVIDER);
        String result = null;
        if (params.length == 3) {
            result = AudioTuningJni.getParams(params[0], params[1], params[2]);
        } else if (params.length == 2) {
            result = AudioTuningJni.getCategory(params[0], params[1]);
        } else {
            Elog.i(TAG, "error parameter");
        }
        Elog.i(TAG, "getParameters " + command + " return " + result);
        return result;
    }

    }
    private void setParameters(String command) {
        if (Audio.AudioTuningVer.VER_2_2 != Audio.getAudioTuningVer()) {
            mAudioManager.setParameters(command);
        } else {
        int index = command.indexOf(VERSION_DEVIDER);
        command = command.substring(index + 1);
        String[] params = command.split(PARAM_DEVIDER);
        if (params.length == 4) {
            AudioTuningJni.setParams(params[0], params[1], params[2], params[3]);
            AudioTuningJni.saveToWork(params[0]);
        }
    }
        Elog.i(TAG, "setParameters " + command);
    }

    /*********************************************************************************************
     *  Bellow is for Copy Audio HAL Dump To SDCard feature
     *********************************************************************************************
     */
    private boolean mBound;
    private CopyHalDumpService mService;

    @Override
    protected void onStart() {
        super.onStart();
        // Bind to LocalService
        Intent intent = new Intent(this, CopyHalDumpService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        super.onStop();
        // Unbind from the service
        if (mBound) {
            unbindService(mConnection);
            mBound = false;
        }
    }

    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            // We've bound to LocalService, cast the IBinder and get
            // LocalService instance
            Elog.d(TAG, "onServiceConnected" );
            CopyHalDumpService.LocalBinder binder = (CopyHalDumpService.LocalBinder) service;
            mService = binder.getService(mHandler);
            mBound = true;
            checkCopyDumpStatus();
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            Elog.d(TAG, "onServiceDisconnected" );
            mService.removeUiHandler();
            mBound = false;
        }
    };

    private void initCopyDumpUi () {
        mCopy2SdButton = (Button) findViewById(R.id.Audio_move_to_sdcard);
        mClearDumpButton = (Button) findViewById(R.id.Audio_clear_dump);
        mCopy2SdButton.setOnClickListener(mCopyAudioDumpButtonListener);
        mClearDumpButton.setOnClickListener(mCopyAudioDumpButtonListener);
        mCopy2SdButton.setEnabled(false);
        mClearDumpButton.setEnabled(false);
        mCopyProgressTv = (TextView) findViewById(R.id.Audio_copy_progress);
    }

private void checkCopyDumpStatus () {
        switch (mService.getCopyDumpStatus()) {
            case COPY_HAL_DUMP:
                Elog.v(TAG, "COPY_HAL_DUMP" );
                mCopy2SdButton.setText(R.string.Audio_stop_moving);
                mCopy2SdButton.setEnabled(true);
                mClearDumpButton.setEnabled(false);
                break;
            case DELETE_HAL_DUMP:
                Elog.v(TAG, "DELETE_HAL_DUMP" );
                mCopy2SdButton.setEnabled(false);
                mClearDumpButton.setEnabled(false);
                break;
            case COPY_DUMP_DONE:
                Elog.v(TAG, "COPY_DUMP_DONE" );
                mCopy2SdButton.setEnabled(true);
                mClearDumpButton.setEnabled(true);
                mCopyProgressTv.setText(R.string.Audio_copy_dump_done);
                break;
            case DELETE_DUMP_DONE:
                Elog.v(TAG, "DELETE_DUMP_DONE" );
                mCopy2SdButton.setEnabled(true);
                mClearDumpButton.setEnabled(true);
                mCopyProgressTv.setText(R.string.Audio_delete_dump_done);
                break;
            default:
                Elog.v(TAG, "DEFAULT" );
                mCopy2SdButton.setEnabled(true);
                mClearDumpButton.setEnabled(true);
                break;
        }
    }

    Button.OnClickListener mCopyAudioDumpButtonListener = new Button.OnClickListener() {
        @Override
        public void onClick(View v) {
            if(!mBound) {
                Elog.w(TAG, "onClick CopyAudioDumpButtons should connect Service first");
            }
            switch (v.getId()) {
                case R.id.Audio_move_to_sdcard:
                    mCopy2SdButton.setEnabled(false);
                    if(mClearDumpButton.isEnabled()) {
                        Elog.v(TAG, "copy hal dump");
                        mCopy2SdButton.setText(R.string.Audio_stop_moving);
                        AudioTuningJni.copyAudioHalDumpFilesToSdcard(mService);
                        mClearDumpButton.setEnabled(false);
                    } else {
                        Elog.v(TAG, "stop copying hal dump");
                        mCopy2SdButton.setText(R.string.Audio_move_to_sdcard);
                        mCopyProgressTv.setText(R.string.Audio_stop_moving_text);
                        AudioTuningJni.cancleCopyAudioHalDumpFile();
                        mClearDumpButton.setEnabled(true);
                    }
                    try {
                        Thread.sleep(200);
                    } catch (InterruptedException e) {

                    }
                    mCopy2SdButton.setEnabled(true);
                    break;
                case R.id.Audio_clear_dump:
                    Elog.v(TAG, "clear hal dump");
                    mCopy2SdButton.setEnabled(false);
                    mClearDumpButton.setEnabled(false);
                    AudioTuningJni.delAudioHalDumpFiles(mService);
                    break;
                default:
                    break;
            }
        }
    };

    public Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            super.handleMessage(msg);
            switch (msg.what){
                case CopyHalDumpService.MSG_COPY_PROGRESS_UPDATE:
                    mCopyProgressTv.setText((String) msg.obj);
                break;
                case CopyHalDumpService.MSG_DELETE_PROGRESS_UPDATE:
                    mCopyProgressTv.setText((String) msg.obj);
                    break;
                case CopyHalDumpService.MSG_COPY_DONE:
                    mCopyProgressTv.setText(R.string.Audio_copy_dump_done);
                    mCopy2SdButton.setText(R.string.Audio_move_to_sdcard);
                    mClearDumpButton.setEnabled(true);
                    break;
                case CopyHalDumpService.MSG_DELETE_DONE:
                    mCopyProgressTv.setText(R.string.Audio_delete_dump_done);
                    mCopy2SdButton.setEnabled(true);
                    mClearDumpButton.setEnabled(true);
                    break;
                default:
                    break;
            }

        }
    };
}
