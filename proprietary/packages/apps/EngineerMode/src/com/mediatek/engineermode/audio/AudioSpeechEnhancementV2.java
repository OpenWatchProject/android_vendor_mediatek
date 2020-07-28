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
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


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
 * Audio speech enhancement for AUDIO_TUNING_TOOL v2.
 *
 */
public class AudioSpeechEnhancementV2 extends Activity implements OnClickListener {
    public static final String TAG = "Audio/SpeechEnhancement2";
    private static final String PARAM_DEVIDER = "#";
    private static final String VERSION_DEVIDER = "=";
    private static final String CMD_GET_COMMON_PARAMETER =
            "APP_GET_PARAM=SpeechGeneral#CategoryLayer,Common#speech_common_para";
    private static final String CMD_SET_COMMON_PARAMETER =
            "APP_SET_PARAM=SpeechGeneral#CategoryLayer,Common#speech_common_para#";
    private static final String CMD_GET_DEBUG_INFO =
            "APP_GET_PARAM=SpeechGeneral#CategoryLayer,Common#debug_info";
    private static final String CMD_SET_DEBUG_INFO =
            "APP_SET_PARAM=SpeechGeneral#CategoryLayer,Common#debug_info#";
    private static final String CMD_GET_MODE_LIST =
            "APP_GET_CATEGORY=Speech#Band";
    private static final String CMD_GET_PARAMETER_LIST =
            "APP_GET_CATEGORY=Speech#Profile";
    private static final String CMD_GET_VOLINDEX_LIST =
            "APP_GET_CATEGORY=Speech#VolIndex";
    private static final String CMD_GET_PARAMETER =
            "APP_GET_PARAM=Speech#Band,%s,Profile,%s,VolIndex,%s,Network,GSM#speech_mode_para";
    private static final String CMD_SET_PARAMETER =
            "APP_SET_PARAM=Speech#Band,%s,Profile,%s,VolIndex,%s,Network,GSM#speech_mode_para#";
    private static final String CMD_PREFIX =
            "APP_GET_PARAM=";
    private static final String CMD_GET_LIST_PREFIX =
            "APP_GET_CATEGORY=";
    private static final String CMD_SPH_PARSER_SET =
            "SPH_PARSER_SET_PARAM,PHONE_CALL,TestMode,";
    private static final int INDEX_DEBUGMODE_DEFAULT = 0;
    private static final int INDEX_COMMON = 0;
    private static final int INDEX_DEBUG = 1;
    private static final int DIALOG_GET_DATA_ERROR = 1;
    private static final int DIALOG_GET_WBDATA_ERROR = 2;
    private static final int DIALOG_SET_SE_SUCCESS = 3;
    private static final int DIALOG_SET_SE_ERROR = 4;
    private static final int DIALOG_SET_WB_SUCCESS = 5;
    private static final int DIALOG_SET_WB_ERROR = 6;
    private Spinner mModeSpinner;
    private Spinner mParaSpinner;
    private Spinner mVolumeSpinner;
    private ArrayAdapter<String> mModeAdatper;
    private ArrayAdapter<String> mParaAdatper;
    private ArrayAdapter<String> mVolumeAdatper;
    private int mModeIndex;
    private int mParaIndex;
    private int mVolumeIndex;
    private List<String> mModeEntries = new ArrayList<String>();
    private List<String> mModeEntryValues = new ArrayList<String>();
    private List<String> mParaEntries = new ArrayList<String>();
    private List<String> mParaEntryValues = new ArrayList<String>();
    private List<String> mVolumeEntries = new ArrayList<String>();
    private List<String> mVolumeEntryValues = new ArrayList<String>();
    private List<String> mParameters = new ArrayList<String>();
    private List<View> mParameterViews = new ArrayList<View>();

    private ListView mList;
    private MyAdapter mAdapter;

    private Button mBtnSet;

    private AudioManager mAudioManager;

    // Set speech parser for operators
    private Map<String, String> mParserMap = new HashMap<String, String>();
    private String[] mParserEntries;

    private static final String CUST_XML_PARAM = "GET_CUST_XML_ENABLE";
    private static final String CUST_XML_SET_SUPPORT_PARAM = "SET_CUST_XML_ENABLE=1";
    private static final String CUST_XML_SET_UNSUPPORT_PARAM = "SET_CUST_XML_ENABLE=0";
    private static final String RESULT_SUPPORT = "GET_CUST_XML_ENABLE=1";
    private static final String RESULT_UNSUPPORT = "GET_CUST_XML_ENABLE=0";
    /**
     * Adapter for ListView.
     */
    private class MyAdapter extends ArrayAdapter<String> {
        /**
         * Default constructor.
         *
         * @param activity
         *              the context
         */
        public MyAdapter(Context activity) {
            super(activity, 0);
        }

        /**
         * ViewHolder.
         */
        private class ViewHolder {
            public TextView label;
            public EditText editor;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            final int pos = position;
            final ViewHolder holder;
            LayoutInflater inflater = AudioSpeechEnhancementV2.this.getLayoutInflater();
            View view = mParameterViews.get(position);
            if (view == null) {
                view = inflater.inflate(R.layout.audio_speechenhancement_entry, null);
                holder = new ViewHolder();
                holder.label = (TextView) view.findViewById(R.id.label);
                holder.editor = (EditText) view.findViewById(R.id.editor);
                view.setTag(holder);
                mParameterViews.set(position, view);
            } else {
                holder = (ViewHolder) view.getTag();
            }
            holder.editor.addTextChangedListener(new TextWatcher() {
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                }
                public void afterTextChanged(Editable s) {
                    if (pos < mParameters.size()) {
                        if(holder.editor.getText() != null) {
                            mParameters.set(pos, holder.editor.getText().toString());
                        }
                    }
                }
            });
            holder.label.setText("Index " + pos);
            holder.editor.setText(String.valueOf(getItem(position)));
            return view;
        }
    }

    private String getParameters(String command) {
        if (Audio.AudioTuningVer.VER_2_2 != Audio.getAudioTuningVer()) {
            String ret = mAudioManager.getParameters(command);
            Elog.i(TAG, "getParameters " + command + " return " + ret);
            int prefixLength = CMD_PREFIX.length();
            if (CMD_GET_MODE_LIST.equals(command) || CMD_GET_PARAMETER_LIST.equals(command)
                    || CMD_GET_VOLINDEX_LIST.equals(command) ) {
                prefixLength = CMD_GET_LIST_PREFIX.length();
            }
            if (ret != null && ret.length() > prefixLength) {
                ret = ret.substring(prefixLength);
            }
            Elog.i(TAG, "getParameters return " + ret);
            return ret;
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

    private void handleParameters(String data) {
        if (data == null) {
            return;
        }
        String[] entries = data.split(",");

        mParameters.clear();
        mParameterViews.clear();
        for (int i = 0; i < entries.length; i++) {
            String entry = entries[i];
            if (entry != null && entry.length() > 2) {
                try {
                    entry = Long.valueOf(entry.substring(2), 16).toString();
                } catch (NumberFormatException e) {
                    entry = "ERROR";
                }
            } else {
                entry = "ERROR";
            }
            mParameters.add(entry);
            mParameterViews.add(null);
        }

        mAdapter.clear();
        mAdapter.addAll(mParameters);
    }

    private boolean handleModeList(String data) {
        if (data == null) {
            return false;
        }
        String[] values = data.split(",");
        if (values.length <= 0 || values.length % 2 != 0) {
            return false;
        }

        mModeEntries.clear();
        mModeEntryValues.clear();
        mModeEntries.add("Common Parameter");
        mModeEntries.add("Debug Info");
        mModeEntryValues.add("");
        mModeEntryValues.add("");
        for (int i = 0; i < values.length; i++) {
            if (i % 2 == 0) {
                mModeEntryValues.add(values[i]);
            } else {
                mModeEntries.add(values[i]);
            }
        }
        mModeAdatper = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, mModeEntries);
        mModeAdatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mModeSpinner.setAdapter(mModeAdatper);
        return true;
    }

    private boolean handleParameterList(String data) {
        if (data == null) {
            return false;
        }
        String[] values = data.split(",");
        if (values.length <= 0 || values.length % 2 != 0) {
            return false;
        }

        mParaEntries.clear();
        mParaEntryValues.clear();
        for (int i = 0; i < values.length; i++) {
            if (i % 2 == 0) {
                mParaEntryValues.add(values[i]);
            } else {
                mParaEntries.add(values[i]);
            }
        }
        mParaAdatper = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, mParaEntries);
        mParaAdatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mParaSpinner.setAdapter(mParaAdatper);
        return true;
    }
    private boolean handleVolindexList(String data) {
        if (data == null) {
            return false;
        }
        String[] values = data.split(",");
        if (values.length <= 0 || values.length % 2 != 0) {
            return false;
        }

        mVolumeEntries.clear();
        mVolumeEntryValues.clear();
        for (int i = 0; i < values.length; i++) {
            if (i % 2 == 0) {
                mVolumeEntryValues.add(values[i]);
            } else {
                mVolumeEntries.add(values[i]);
            }
        }
        mVolumeAdatper = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, mVolumeEntries);
        mVolumeAdatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mVolumeSpinner.setAdapter(mVolumeAdatper);
        return true;
    }
    private void initComponents() {
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mModeSpinner = (Spinner) findViewById(R.id.Audio_SpEnhancement_ModeType);
        mParaSpinner = (Spinner) findViewById(R.id.Audio_SpEnhancement_ParaType);
        mVolumeSpinner =  (Spinner) findViewById(R.id.Audio_SpEnhancement_Vol_spin);
        mList = (ListView) findViewById(R.id.Audio_SpEnhancement_ListView);
        mAdapter = new MyAdapter(this);
        mList.setAdapter(mAdapter);
        mBtnSet = (Button) findViewById(R.id.Audio_SpEnhancement_Button);
        mBtnSet.setOnClickListener(this);

        String modes1 = getParameters(CMD_GET_MODE_LIST);
        if (!handleModeList(modes1)) {
            Toast.makeText(this, "Wrong format", Toast.LENGTH_SHORT).show();
        }

        String modes2 = getParameters(CMD_GET_VOLINDEX_LIST);
        if (!handleVolindexList(modes2)) {
            Toast.makeText(this, "Wrong format", Toast.LENGTH_SHORT).show();
        }
        mVolumeSpinner.setVisibility(View.GONE);

        mModeSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                mModeIndex = arg2;
                switch (mModeIndex) {
                case INDEX_COMMON:
                    handleParameters(getParameters(CMD_GET_COMMON_PARAMETER));
                    mParaSpinner.setVisibility(View.GONE);
                    mVolumeSpinner.setVisibility(View.GONE);
                    break;
                case INDEX_DEBUG:
                    handleParameters(getParameters(CMD_GET_DEBUG_INFO));
                    mParaSpinner.setVisibility(View.GONE);
                    mVolumeSpinner.setVisibility(View.GONE);
                    break;
                default:
                    if (!handleParameterList(getParameters(CMD_GET_PARAMETER_LIST))) {
                        Toast.makeText(AudioSpeechEnhancementV2.this,
                                "Wrong format", Toast.LENGTH_SHORT).show();
                        mParaSpinner.setVisibility(View.GONE);
                        mVolumeSpinner.setVisibility(View.GONE);
                    } else {
                        mParaSpinner.setVisibility(View.VISIBLE);
                        mVolumeSpinner.setVisibility(View.VISIBLE);
                    }
                    break;
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.i(TAG, "do noting...");
            }
        });

        mParaSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                mParaIndex = arg2;
                String cmd = String.format(CMD_GET_PARAMETER,
                        mModeEntryValues.get(mModeIndex), mParaEntryValues.get(mParaIndex),
                            mVolumeEntryValues.get(mVolumeIndex));
                Elog.d(TAG, "cmd= " + cmd);
                handleParameters(getParameters(cmd));
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.i(TAG, "do noting...");
            }
        });
        mVolumeSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                mVolumeIndex = arg2;
                String cmd = String.format(CMD_GET_PARAMETER,
                   mModeEntryValues.get(mModeIndex), mParaEntryValues.get(mParaIndex),
                             mVolumeEntryValues.get(mVolumeIndex));
                Elog.d(TAG, "cmd= " + cmd);
                handleParameters(getParameters(cmd));
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Elog.i(TAG, "do noting...");
            }
        });
    }
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio_speechenhancement_new);
        initComponents();
        mParserEntries = getResources().getStringArray(R.array.speech_enhance_parser);
        for (int i = 0; i < mParserEntries.length; ++i) {
            mParserMap.put(mParserEntries[i], String.valueOf(i));
        }
    }

    private void setDebugInfoIndex10Param() {
        Elog.v(TAG, "setDebugInfoIndex10Param");
        try {
            int index = Integer.parseInt(mParameters.get(10));
            if(index >= 0 && index < mParserEntries.length) {
                String cmd = mParserEntries[index];
                mAudioManager.setParameters(CMD_SPH_PARSER_SET + cmd);
                Elog.i(TAG, "setDebugInfoIndex10Param " + cmd);
            }
        } catch (NumberFormatException e) {
            Toast.makeText(this, "Wrong format", Toast.LENGTH_SHORT).show();
            return;
        }
    }
    /**
     * click the set button.
     *
     * @param arg0
     *            : click which view
     */
    public void onClick(View arg0) {
        if (arg0.equals(mBtnSet)) {
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
            String cmd = "";
            switch (mModeIndex) {
            case INDEX_COMMON:
                cmd = CMD_SET_COMMON_PARAMETER;
                break;
            case INDEX_DEBUG:
                setDebugInfoIndex10Param();
                cmd = CMD_SET_DEBUG_INFO;
                break;
            default:
                if (mModeEntryValues.size() <= 0 || mParaEntryValues.size() <= 0) {
                    return;
                }
                cmd = String.format(CMD_SET_PARAMETER,
                        mModeEntryValues.get(mModeIndex), mParaEntryValues.get(mParaIndex),
                           mVolumeEntryValues.get(mVolumeIndex));
                break;
            }
            for (String p : mParameters) {
                try {
                    cmd += "0x" + Long.toString(Long.parseLong(p), 16) + ",";
                } catch (NumberFormatException e) {
                    Toast.makeText(this, "Wrong format", Toast.LENGTH_SHORT).show();
                    return;
                }
            }
            cmd = cmd.substring(0, cmd.length() - 1);
            setParameters(cmd);
            Toast.makeText(this, "Set parameter done", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public Dialog onCreateDialog(int dialogId) {
        switch (dialogId) {
        case DIALOG_GET_DATA_ERROR:
            return new AlertDialog.Builder(AudioSpeechEnhancementV2.this)
                .setTitle(R.string.get_data_error_title).setMessage(
                    R.string.get_data_error_msg).setPositiveButton(
                    android.R.string.ok, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // removeDialog(DIALOG_ID_GET_DATA_ERROR);
                            AudioSpeechEnhancementV2.this.finish();
                        }

                    }).setNegativeButton(android.R.string.cancel, null)
                .create();
        case DIALOG_GET_WBDATA_ERROR:
            return new AlertDialog.Builder(AudioSpeechEnhancementV2.this)
                .setTitle(R.string.get_wbdata_error_title).setMessage(
                    R.string.get_wbdata_error_msg).setPositiveButton(
                    android.R.string.ok, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // removeDialog(DIALOG_ID_GET_DATA_ERROR);
                            AudioSpeechEnhancementV2.this.finish();
                        }

                    }).setNegativeButton(android.R.string.cancel, null)
                .create();
        case DIALOG_SET_SE_SUCCESS:
            return new AlertDialog.Builder(AudioSpeechEnhancementV2.this)
                .setTitle(R.string.set_success_title).setMessage(
                    R.string.set_speech_enhance_success).setPositiveButton(
                    android.R.string.ok, null).create();
        case DIALOG_SET_SE_ERROR:
            return new AlertDialog.Builder(AudioSpeechEnhancementV2.this)
                .setTitle(R.string.set_error_title).setMessage(
                    R.string.set_speech_enhance_failed).setPositiveButton(
                    android.R.string.ok, null).create();
        case DIALOG_SET_WB_SUCCESS:
            return new AlertDialog.Builder(AudioSpeechEnhancementV2.this)
                .setTitle(R.string.set_success_title).setMessage(
                    R.string.set_wb_success).setPositiveButton(
                    android.R.string.ok, null).create();
        case DIALOG_SET_WB_ERROR:
            return new AlertDialog.Builder(AudioSpeechEnhancementV2.this)
                .setTitle(R.string.set_error_title).setMessage(
                    R.string.set_wb_failed).setPositiveButton(
                    android.R.string.ok, null).create();
        default:
            return null;
        }
    }
}
