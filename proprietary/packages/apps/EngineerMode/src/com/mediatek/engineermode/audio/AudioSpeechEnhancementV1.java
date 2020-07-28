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
import android.content.Context;
import android.content.DialogInterface;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;


/*
 * C structure
 * struct_AUDIO_CUSTOM_WB_PARAM_STRUCT
 * {
 *    ushort speech_mode_wb_para[8][16];
 *    short  sph_wb_in_fir[6][90];
 *    short  sph_wb_out_fir[6][90];
 * }
 * sizeof() = 2416, 2 bytes alignment.
 * GET_WB_SPEECH_PARAMETER = 0X40;
 * SET_WB_SPEECH_PARAMETER = 0X41;
 * SetAudioData()&GetAudioData();
 * use MTK_WB_SPEECH_SUPPORT.
 * */

/**
 * Audio speech enhancement for AUDIO_TUNING_TOOL v1.
 *
 */
public class AudioSpeechEnhancementV1 extends Activity implements OnClickListener {
    private static final String TAG = "Audio/SpeechEnhancement1";
    private Button mBtnSet;
    private EditText mValueEdit;
    private Spinner mParaSpinner;
    private ArrayAdapter<String> mParaAdatper;
    private int mModeIndex;
    private int mParaIndex;

    private static final int VOLUME_SIZE = 22;
    private static final int COMMON_PARA_SIZE = 24;
    private static final int DATA_SIZE = 1444;
    private static final int CONSTANT_256 = 256;
    private static final int CONSTANT_32 = 32;
    private static final int CONSTANT_0XFF = 0xFF;
    private static final int WB_DATA_SIZE = 2416;
    private static final int GET_WB_SPEECH_PARAMETER = 0x40;
    private static final int SET_WB_SPEECH_PARAMETER = 0x41;
    private static final int GET_SPEECH_MAGICON_PARAMETER = 0xC0;
    private static final int SET_SPEECH_MAGICON_PARAMETER = 0xC1;
    private static final int GET_SPEECH_HAC_PARAMETER = 0xD0;
    private static final int SET_SPEECH_HAC_PARAMETER = 0xD1;
    private static final int SPEECH_DATA_SIZE = (16 + 16) * 2;
    private static final int HAC_DATA_SIZE = (16 + 16 + 45 + 45 + 90 + 90) * 2 + 15 * 3 + 1;
    private static final String VOICE_TRACKING_MODE = "Voice Tracking Mode";
    private static final String HAC_MODE = "HAC Mode";
    private static final int INDEX_VOICE_TRACKING_MODE = 9;
    private static final int INDEX_HAC_MODE = 10;
    private byte[] mData;
    private byte[] mWBdata;
    private byte[] mSpeechdata;
    private byte[] mHACdata;

    private static final int LONGEST_NUM_LENGHT = 5;
    private static final int LARGEST_NUM = 65535;
    private static final int MODE0_PARAM_NUM = 12;
    private static final int AUDIO_PARA_DIV_INDEX = 15;
    private static final int AUDIO_PARA_DIV_INDEX1 = 16;
    private static final int ACTURAL_PARAM_NUM = 32;
    private static final int DIALOG_GET_DATA_ERROR = 1;
    private static final int DIALOG_GET_WBDATA_ERROR = 2;
    private static final int DIALOG_SET_SE_SUCCESS = 3;
    private static final int DIALOG_SET_SE_ERROR = 4;
    private static final int DIALOG_SET_WB_SUCCESS = 5;
    private static final int DIALOG_SET_WB_ERROR = 6;
    private String mParamterStr = "";

    static final String AFK_WB_SPEED = "MTK_WB_SPEECH_SUPPORT";
    /**
     *   for further usage.
     */
    static boolean isAudioFeatureSupport(Context context, String featureKey) {
        boolean result = false;
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        String pairs = am.getParameters(featureKey);
        if (pairs == null) {
            Elog.d(TAG, "PARSE FAIL; parameters is null");
            return result;
        }
        String[] keyvals = pairs.split(";");
        String[] keyval = keyvals[0].split("=");
        if (keyval.length < 2) {
            Elog.d(TAG, "parse fail; invalid keyval:" + keyvals[0]);
            return result;
        }
        String val = keyval[1].trim();
        if ("true".equalsIgnoreCase(val) || "yes".equalsIgnoreCase(val)) {
            result = true;
        }
        return result;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio_speechenhancement);
        mParamterStr = getResources().getString(R.string.paramter);
        mBtnSet = (Button) findViewById(R.id.Audio_SpEnhancement_Button);
        mValueEdit = (EditText) findViewById(R.id.Audio_SpEnhancement_EditText);
        mParaSpinner =
            (Spinner) findViewById(R.id.Audio_SpEnhancement_ParaType);
        TextView valueText =
            (TextView) findViewById(R.id.Audio_SpEnhancement_TextView);
        Spinner modeSpinner =
            (Spinner) findViewById(R.id.Audio_SpEnhancement_ModeType);


        // create ArrayAdapter for Spinner
        String[] modeArray = getResources().getStringArray(R.array.speech_enhance_mode);
        List<String> modeArrayList = new ArrayList<String>();
        for (int i = 0; i < modeArray.length; i++) {
            modeArrayList.add(modeArray[i]);
        }
        ArrayAdapter<String> mModeAdatper = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, modeArrayList);
        if (checkMagiStatus()) {  // mode index is 9
            mModeAdatper.add(VOICE_TRACKING_MODE);
        }
        if (checkHacStatus()) {  // mode index is 10
            mModeAdatper.add(HAC_MODE);
        }
        mModeAdatper
            .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        modeSpinner.setAdapter(mModeAdatper);
        modeSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1,
                int arg2, long arg3) {
                mModeIndex = arg2;
                if (0 == arg2) {
                    mParaAdatper.clear();
                    for (int i = 0; i < MODE0_PARAM_NUM; i++) {
                        mParaAdatper.add(mParamterStr + i);
                    }
                } else {
                    mParaAdatper.clear();
                    for (int i = 0; i < ACTURAL_PARAM_NUM; i++) {
                        mParaAdatper.add(mParamterStr + i);
                    }
                }

                mParaSpinner.setSelection(0);
                mParaIndex = 0;
                int initValue;
                String tag = arg0.getSelectedItem().toString();
                if (tag.equals(VOICE_TRACKING_MODE)) {
                    mModeIndex = INDEX_VOICE_TRACKING_MODE;
                    initValue = getSpeechData();
                } else if (tag.equals(HAC_MODE)) {
                    mModeIndex = INDEX_HAC_MODE;
                    initValue = getHacData();
                } else {
                    initValue = getAudioData();
                }
                mValueEdit.setText(String.valueOf(initValue));

            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.i(TAG, "do noting...");
            }
        });

        mBtnSet.setOnClickListener(this);
        valueText.setText(R.string.speech_enhance_text);

        // create ArrayAdapter for Spinner
        mParaAdatper =
            new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mParaAdatper
            .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < MODE0_PARAM_NUM; i++) {
            mParaAdatper.add(mParamterStr + i);
        }
        mParaSpinner.setAdapter(mParaAdatper);
        mParaSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1,
                int arg2, long arg3) {

                mParaIndex = arg2;

                int initValue;
                if (mModeIndex == INDEX_VOICE_TRACKING_MODE) {
                    initValue = getSpeechData();
                } else if (mModeIndex == INDEX_HAC_MODE) {
                    initValue = getHacData();
                } else {
                    initValue = getAudioData();
                }
                mValueEdit.setText(String.valueOf(initValue));
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.i(TAG, "do noting...");
            }
        });

        // get the current data
        mData = new byte[DATA_SIZE];
        for (int n = 0; n < DATA_SIZE; n++) {
            mData[n] = 0;
        }
        mWBdata = new byte[WB_DATA_SIZE];
        for (int n = 0; n < WB_DATA_SIZE; n++) {
            mWBdata[n] = 0;
        }
        int ret = AudioTuningJni.getEmParameter(mData, DATA_SIZE);
        if (ret != 0) {
            showDialog(DIALOG_GET_DATA_ERROR);
            Elog.i(TAG,
                "Audio_SpeechEnhancement GetEMParameter return value is : "
                    + ret);
        }

        ret =
                AudioTuningJni.getAudioData(GET_WB_SPEECH_PARAMETER, WB_DATA_SIZE,
                mWBdata);
        if (ret != 0) {
            showDialog(DIALOG_GET_WBDATA_ERROR);
            Elog.i(TAG,
                "Audio_SpeechEnhancement Get wb data return value is : " + ret);
        }

        modeSpinner.setSelection(0);
        mParaSpinner.setSelection(0);
        mModeIndex = 0;
        mParaIndex = 0;
        mValueEdit.setText(String.valueOf(getAudioData()));

    }

    /**
     * click the set button.
     *
     * @param arg0
     *            : click which view
     */
    public void onClick(View arg0) {

        if (arg0.equals(mBtnSet)) {
            int inputValue = checkEditNumber(mValueEdit, LARGEST_NUM);
            if(inputValue == -1) {
                return;
            }
            if (mModeIndex == INDEX_VOICE_TRACKING_MODE) {
                setSpeechData(inputValue);
            } else if (mModeIndex == INDEX_HAC_MODE) {
                setHacData(inputValue);
            } else {
                setAudioData(inputValue);
            }
        }

    }

    private int checkEditNumber(EditText edit, int maxValue) {
        if(edit == null || edit.getText() == null) {
            Toast.makeText(this, getString(R.string.input_null_tip), Toast.LENGTH_LONG)
            .show();
            return -1;
        }
        String editStr = edit.getText().toString();
        if (null == editStr || editStr.length() == 0) {
            Toast.makeText(this, getString(R.string.input_null_tip), Toast.LENGTH_LONG)
                    .show();
            return -1;
        }
        try {
            int value = Integer.valueOf(editStr);
            if (value > maxValue) {
                Toast.makeText(this, getString(R.string.number_arrage_tip) + maxValue,
                        Toast.LENGTH_LONG).show();
                return -1;
            }
            return value;
        } catch (NumberFormatException e) {
            Toast.makeText(this, getString(R.string.number_arrage_tip) + maxValue,
                    Toast.LENGTH_LONG).show();
            return -1;
        }
    }

    private int getAudioData() {
        if (mParaIndex > AUDIO_PARA_DIV_INDEX) {
            return getWBAudioData();
        } else {
            return getSpeechEnhanceAudioData();
        }
    }
    private int getSpeechData() {
        int high = 0;
        int low = 0;

        high = mSpeechdata[mParaIndex * 2 + 1];
        low = mSpeechdata[mParaIndex * 2];

        high = high < 0 ? high + CONSTANT_256 : high;
        low = low < 0 ? low + CONSTANT_256 : low;
        return high * CONSTANT_256 + low;
    }
    private int getHacData() {
        int high = 0;
        int low = 0;

        high = mHACdata[mParaIndex * 2 + 1];
        low = mHACdata[mParaIndex * 2];

        high = high < 0 ? high + CONSTANT_256 : high;
        low = low < 0 ? low + CONSTANT_256 : low;
        return high * CONSTANT_256 + low;
    }
    private void setHacData(int inputValue) {
        final int high = (int) (inputValue / CONSTANT_256);
        final int low = (int) (inputValue % CONSTANT_256);

        mHACdata[mParaIndex * 2] = (byte) low;
        mHACdata[mParaIndex * 2 + 1] = (byte) high;

        int result = AudioTuningJni.setAudioData(SET_SPEECH_HAC_PARAMETER, HAC_DATA_SIZE,
                mHACdata);
        if (0 == result || -38 == result) {
            showDialog(DIALOG_SET_SE_SUCCESS);
        } else {
            showDialog(DIALOG_SET_SE_ERROR);
            Elog.i(TAG,
                "Audio_SpeechEnhancement setHacData return value is : "
                    + result);
        }
    }
    private void setAudioData(int inputValue) {
        if (mParaIndex > AUDIO_PARA_DIV_INDEX) {
            setWBAudioData(inputValue);
        } else {
            setSpeechEnhanceAudioData(inputValue);
        }
    }

    private int getSpeechEnhanceAudioData() {
        int high = 0;
        int low = 0;
        if (mModeIndex == 0) {
            high = mData[VOLUME_SIZE + mParaIndex * 2 + 1];
            low = mData[VOLUME_SIZE + mParaIndex * 2];

        } else {
            high =
                mData[VOLUME_SIZE + COMMON_PARA_SIZE + (mModeIndex - 1)
                    * CONSTANT_32 + mParaIndex * 2 + 1];
            low =
                mData[VOLUME_SIZE + COMMON_PARA_SIZE + (mModeIndex - 1)
                    * CONSTANT_32 + mParaIndex * 2];
        }

        high = high < 0 ? high + CONSTANT_256 : high;
        low = low < 0 ? low + CONSTANT_256 : low;
        return high * CONSTANT_256 + low;

    }
    private void setSpeechData(int inputValue) {
        final int high = (int) (inputValue / CONSTANT_256);
        final int low = (int) (inputValue % CONSTANT_256);

        mSpeechdata[mParaIndex * 2] = (byte) low;
        mSpeechdata[mParaIndex * 2 + 1] = (byte) high;

   int result = AudioTuningJni.setAudioData(SET_SPEECH_MAGICON_PARAMETER, SPEECH_DATA_SIZE,
                mSpeechdata);
        if (0 == result || -38 == result  ) {
            showDialog(DIALOG_SET_SE_SUCCESS);
        } else {
            showDialog(DIALOG_SET_SE_ERROR);
            Elog.i(TAG,
                "Audio_SpeechEnhancement setAudioData return value is : "
                    + result);
        }
    }
    private void setSpeechEnhanceAudioData(int inputValue) {
        final int high = (int) (inputValue / CONSTANT_256);
        final int low = (int) (inputValue % CONSTANT_256);
        if (mModeIndex == 0) {
            mData[VOLUME_SIZE + mParaIndex * 2] = (byte) low;
            mData[VOLUME_SIZE + mParaIndex * 2 + 1] = (byte) high;
        } else {
            mData[VOLUME_SIZE + COMMON_PARA_SIZE + (mModeIndex - 1)
                * CONSTANT_32 + mParaIndex * 2] = (byte) low;
            mData[VOLUME_SIZE + COMMON_PARA_SIZE + (mModeIndex - 1)
                * CONSTANT_32 + mParaIndex * 2 + 1] = (byte) high;
        }

        int result = AudioTuningJni.setEmParameter(mData, DATA_SIZE);
        if (0 == result || -38 == result) {
            showDialog(DIALOG_SET_SE_SUCCESS);
        } else {
            showDialog(DIALOG_SET_SE_ERROR);
            Elog.i(TAG,
                "Audio_SpeechEnhancement SetEMParameter return value is : "
                    + result);
        }
    }

    private int getWBdata(int catalogIdx, int paraIdx) {
        int highByteTemp =
            CONSTANT_0XFF & (mWBdata[catalogIdx * CONSTANT_32 + paraIdx * 2 + 1] + CONSTANT_256);
        int highByte = CONSTANT_256 * highByteTemp;

        int lowByte =
            CONSTANT_0XFF & (mWBdata[catalogIdx * CONSTANT_32 + paraIdx * 2] + CONSTANT_256);
        Elog.v(TAG, "getWBdata mode " + catalogIdx + ", paraIdx "
            + paraIdx + "val " + (highByte + lowByte));
        return highByte + lowByte;
    }

    private void setWBdata(int catalogIdx, int paraIdx, int val) {
        mWBdata[catalogIdx * CONSTANT_32 + paraIdx * 2] =
            (byte) (val % CONSTANT_256);
        mWBdata[catalogIdx * CONSTANT_32 + paraIdx * 2 + 1] =
            (byte) (val / CONSTANT_256);
    }

    private int getWBAudioData() {
        return getWBdata(mModeIndex - 1, mParaIndex - AUDIO_PARA_DIV_INDEX1);
    }

    private void setWBAudioData(int inputval) {
        if (mParaIndex < AUDIO_PARA_DIV_INDEX1) {
            Elog.i(TAG, "Internal error. check the code.");
            return;
        }
        setWBdata(mModeIndex - 1, mParaIndex - AUDIO_PARA_DIV_INDEX1, inputval);
        int result =
            AudioTuningJni.setAudioData(SET_WB_SPEECH_PARAMETER, WB_DATA_SIZE,
                mWBdata);
        if (0 == result || -38 == result) {
            showDialog(DIALOG_SET_WB_SUCCESS);
        } else {
            showDialog(DIALOG_SET_WB_ERROR);
            Elog.i(TAG, "WB data SetAudioData return value is : "
                + result);
        }
    }

    @Override
    public Dialog onCreateDialog(int dialogId) {
        switch (dialogId) {
        case DIALOG_GET_DATA_ERROR:
            return new AlertDialog.Builder(AudioSpeechEnhancementV1.this)
                .setTitle(R.string.get_data_error_title).setMessage(
                    R.string.get_data_error_msg).setPositiveButton(
                    android.R.string.ok, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // removeDialog(DIALOG_ID_GET_DATA_ERROR);
                            AudioSpeechEnhancementV1.this.finish();
                        }

                    }).setNegativeButton(android.R.string.cancel, null)
                .create();
        case DIALOG_GET_WBDATA_ERROR:
            return new AlertDialog.Builder(AudioSpeechEnhancementV1.this)
                .setTitle(R.string.get_wbdata_error_title).setMessage(
                    R.string.get_wbdata_error_msg).setPositiveButton(
                    android.R.string.ok, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // removeDialog(DIALOG_ID_GET_DATA_ERROR);
                            AudioSpeechEnhancementV1.this.finish();
                        }

                    }).setNegativeButton(android.R.string.cancel, null)
                .create();
        case DIALOG_SET_SE_SUCCESS:
            return new AlertDialog.Builder(AudioSpeechEnhancementV1.this)
                .setTitle(R.string.set_success_title).setMessage(
                    R.string.set_speech_enhance_success).setPositiveButton(
                    android.R.string.ok, null).create();
        case DIALOG_SET_SE_ERROR:
            return new AlertDialog.Builder(AudioSpeechEnhancementV1.this)
                .setTitle(R.string.set_error_title).setMessage(
                    R.string.set_speech_enhance_failed).setPositiveButton(
                    android.R.string.ok, null).create();
        case DIALOG_SET_WB_SUCCESS:
            return new AlertDialog.Builder(AudioSpeechEnhancementV1.this)
                .setTitle(R.string.set_success_title).setMessage(
                    R.string.set_wb_success).setPositiveButton(
                    android.R.string.ok, null).create();
        case DIALOG_SET_WB_ERROR:
            return new AlertDialog.Builder(AudioSpeechEnhancementV1.this)
                .setTitle(R.string.set_error_title).setMessage(
                    R.string.set_wb_failed).setPositiveButton(
                    android.R.string.ok, null).create();
        default:
            return null;
        }
    }

    private boolean checkMagiStatus() {
        String magiSupport = AudioSystem.getParameters("GET_MAGI_CONFERENCE_SUPPORT");
        Elog.i(TAG, "Get Magi support " + magiSupport);
        String[] magiStr = magiSupport.split("=");
        if (magiStr.length >= 2 && magiStr[1].equals("1")) {
            // GET_MAGI_CONFERENCE_SUPPORT=1
            mSpeechdata = new byte[SPEECH_DATA_SIZE];
            for (int n = 0; n < SPEECH_DATA_SIZE; n++) {
                mSpeechdata[n] = 0;
            }
            int ret =
                AudioTuningJni.getAudioData(GET_SPEECH_MAGICON_PARAMETER, SPEECH_DATA_SIZE,
                mSpeechdata);
            if (ret != 0) {
                showDialog(DIALOG_GET_WBDATA_ERROR);
                Elog.i(TAG,
                    "Audio_SpeechEnhancement Get speech data return value is : " + ret);
            }
            return true;
        }
        return false;
    }

    private boolean checkHacStatus() {
        String hacSupport = AudioSystem.getParameters("GET_HAC_SUPPORT");
        Elog.i(TAG, "Get Hac support " + hacSupport);
        String[] hacStr = hacSupport.split("=");
        if (hacStr.length >= 2 && hacStr[1].equals("1")) {
            // GET_HAC_SUPPORT=1 means show the feature
            mHACdata = new byte[HAC_DATA_SIZE];
            for (int n = 0; n < HAC_DATA_SIZE; n++) {
                mHACdata[n] = 0;
            }
            int ret =
                    AudioTuningJni.getAudioData(GET_SPEECH_HAC_PARAMETER, HAC_DATA_SIZE,
                mHACdata);
            if (ret != 0) {
                showDialog(DIALOG_GET_WBDATA_ERROR);
                Elog.i(TAG,
                    "Audio_SpeechEnhancement Get hac data return value is : " + ret);
            }
            return true;
        }
        return false;
    }
}
