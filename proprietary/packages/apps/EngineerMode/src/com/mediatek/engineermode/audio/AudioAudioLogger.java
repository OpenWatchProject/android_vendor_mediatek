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
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.media.AudioSystem;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParserFactory;

/**
 * @author mtk09919 create ui from xml Time: 2015.08.13.
 */
public class AudioAudioLogger extends Activity {
    // used for Audio Logger
    private static final String TAG = "Audio/AudioLogger";

    private static final int DIALOG_ID_NO_SDCARD = 1;
    private static final int DIALOG_ID_SDCARD_BUSY = 2;
    private static final int DIALOG_ID_SHOW_GET_RET = 3;
    private static final int DIALOG_ID_SHOW_SET_RET = 4;
    private static final int DIALOG_ID_SHOW_CHECKBOX_RET = 5;
    private static final String  SET_AUDIO_COMMAND = "SetAudioCommand";
    private static final String  GET_AUDIO_COMMAND = "GetAudioCommand";
    private static final String  SET_PARAMETERS = "SetParameters";
    private static final String  GET_PARAMETERS = "GetParameters";

    private Spinner mSpGetAudioParameter = null;
    private Spinner mSpSetAudioParameter = null;
    private Spinner mSpSetAudioCommand = null;
    private Spinner mSpGetAudioCommand = null;
    private EditText mEditSetAudioCommandText = null;
    private EditText mEditGetAudioCommandText = null;
    private EditText mEditSetAudioParameterText = null;
    private EditText mEditGetAudioParameterText = null;

    private Button mBtGetAudioCommand = null;
    private Button mBtSetAudioCommand = null;
    private Button mBtGetAudioParameter = null;
    private Button mBtSetAudioParameter = null;
    private LinearLayout mCheckBoxLinearLayout = null;
    private ArrayList<CheckBox> mDumpOptionsCheck;
    private File mXmlFile = null;
    private static final String mFileName = "/vendor/etc/audio_em.xml";

    private InputStream mInputStream = null;

    private AudioLoggerXMLData mAudioLoggerXMLData = null;

    private int mAudioSetRet = 0;
    private String mAudioGetRet = null;
    private String mAudioTitle = null;

    private boolean mCategoryTitleFlag = false;

    private ArrayList<String> mAudioCheckBoxRet = null;

    private final OnClickListener mAudioButtonClickListener = new OnClickListener() {

        @SuppressWarnings("deprecation")
        @Override
        public void onClick(View arg0) {
            // TODO Auto-generated method stub
            int ret = -1;
            String str = null;
            int cmdID = 0;
            int cmdParameter = 0;
            if (!checkSDCardIsAvaliable()) {
                return;
            }

            switch (arg0.getId()) {
            case R.id.Audio_SetAudioCommand:
                str = mEditSetAudioCommandText.getText().toString();
                String[] cmdStr = str.replaceAll("\\s*", "").split(",");
                try {
                    cmdID = Integer.decode(cmdStr[0]).intValue();
                    cmdParameter = Integer.decode(cmdStr[1]).intValue();
                    ret = AudioTuningJni.setAudioCommand(cmdID, cmdParameter);
                    Elog.d(TAG, "Audio_SetAudioCommand=" + str + "," + "ret= " + ret);
                    mAudioSetRet = ret;
                    mAudioTitle = str;
                    Toast.makeText(AudioAudioLogger.this, String.valueOf(mAudioSetRet),
                            Toast.LENGTH_SHORT).show();
                    showDialog(DIALOG_ID_SHOW_SET_RET);
                } catch (NumberFormatException e) {
                    Toast.makeText(AudioAudioLogger.this, getString(R.string.number_arrage_tip,
                            Integer.MAX_VALUE), Toast.LENGTH_SHORT).show();
                } catch(ArrayIndexOutOfBoundsException exception) {
                    Toast.makeText(AudioAudioLogger.this, getString(R.string.set_failed_tip),
                            Toast.LENGTH_SHORT).show();
                }
                break;
            case R.id.Audio_GetAudioCommand:
                str = mEditGetAudioCommandText.getText().toString();
                Elog.d(TAG, "Audio_GetAudioCommand=" + str);
                try {
                    cmdID = Integer.decode(str).intValue();
                    mAudioGetRet = String.valueOf(AudioTuningJni.getAudioCommand(cmdID));
                    Elog.d(TAG, "Audio_GetAudioCommand=" + str + "," + "ret= "
                            + mAudioGetRet);
                    mAudioTitle = str;
                    Toast.makeText(AudioAudioLogger.this, mAudioGetRet, Toast.LENGTH_SHORT)
                            .show();
                    showDialog(DIALOG_ID_SHOW_GET_RET);
                } catch (NumberFormatException e) {
                    Toast.makeText(AudioAudioLogger.this, getString(R.string.number_arrage_tip,
                            Integer.MAX_VALUE), Toast.LENGTH_SHORT).show();
                }
                break;
            case R.id.Audio_SetAudioParameter:
                str = mEditSetAudioParameterText.getText().toString();
                ret = AudioSystem.setParameters(str);
                Elog.d(TAG, "Audio_SetAudioParameter=" + str + "," + "ret= " + ret);
                mAudioSetRet = ret;
                mAudioTitle = str;
                Toast.makeText(AudioAudioLogger.this, String.valueOf(mAudioSetRet),
                        Toast.LENGTH_SHORT).show();
                showDialog(DIALOG_ID_SHOW_SET_RET);
                break;
            case R.id.Audio_GetAudioParameter:
                str = mEditGetAudioParameterText.getText().toString();
                mAudioGetRet = AudioSystem.getParameters(str);
                Elog.d(TAG, "Audio_GetAudioParameter=" + str + "," + "ret="
                        + mAudioGetRet);
                mAudioTitle = str;
                Toast.makeText(AudioAudioLogger.this, mAudioGetRet, Toast.LENGTH_SHORT)
                        .show();
                showDialog(DIALOG_ID_SHOW_GET_RET);
                break;
            default:
                break;
            }
        }
    };
    private final OnItemSelectedListener mSpinnerListener = new OnItemSelectedListener() {
        public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
            Elog.d(TAG, "id:" + parent.getId());
            switch (parent.getId()) {
            case R.id.Audio_SetAudioCommandSpinner:
                mEditSetAudioCommandText.setText(parent.getItemAtPosition(position)
                        .toString());
                break;
            case R.id.Audio_GetAudioCommandSpinner:
                mEditGetAudioCommandText.setText(parent.getItemAtPosition(position)
                        .toString());
                break;
            case R.id.Audio_GetAudioParameterSpinner:
                mEditGetAudioParameterText.setText(parent.getItemAtPosition(position)
                        .toString());
                break;
            case R.id.Audio_SetAudioParameterSpinner:
                mEditSetAudioParameterText.setText(parent.getItemAtPosition(position)
                        .toString());
                break;
            default:
                break;
            }
        }

        public void onNothingSelected(AdapterView<?> parent) {
        }

    };

    /** mCheck or mUncheck checkbox items. */
    private final CheckBox.OnCheckedChangeListener mCheckedListener
                                = new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            int ret = -1;
            int mCheckBoxID = buttonView.getId();
            String mCheckBoxName = (String) buttonView.getText();
            String cmdSend = null;
            int cmdID = mCheckBoxID >> 8;
            int cmdParameter = mCheckBoxID & 0xff;
            if (mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mCategoryTitle
                    .equals(mCheckBoxName)) {
                mCategoryTitleFlag = true;
                for (int i = 0; i < mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mCmdName
                        .size(); i++) {
                    CheckBox tempCheck = (CheckBox) findViewById(mCheckBoxID + i + 1);
                    boolean isboxChecked = buttonView.isChecked();
                    tempCheck.setChecked(isboxChecked);
                }
                if(mAudioCheckBoxRet != null && !mAudioCheckBoxRet.isEmpty()) {
                    Elog.d(TAG, "CategoryTitle is checked, show dialog");
                    mAudioTitle = mCheckBoxName;
                    if(isChecked && ("Output".equals(mAudioTitle) ||
                            "Input".equals(mAudioTitle) ||
                            "USB".equals(mAudioTitle))) {
                        Toast.makeText(AudioAudioLogger.this, R.string.audio_dump_path_change_msg,
                                Toast.LENGTH_LONG).show();
                    }
                    showDialog(DIALOG_ID_SHOW_CHECKBOX_RET);
                }
                mCategoryTitleFlag = false;
            } else if (mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mCmdName.get(
                    cmdParameter - 1).equals(mCheckBoxName)) {

                if (mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mType.get(
                        cmdParameter - 1).equals(SET_AUDIO_COMMAND)) {
                    int mCmd = Integer.decode(
                            mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mCmd
                                    .get(cmdParameter - 1)).intValue();
                    int mCheck = Integer.decode(
                            mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mCheck
                                    .get(cmdParameter - 1)).intValue();
                    int mUncheck = Integer.decode(
                            mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mUncheck
                                    .get(cmdParameter - 1)).intValue();
                    cmdSend = Integer.toHexString(mCmd)
                            + ", "
                            + (isChecked ? String.valueOf(mCheck) : String
                                    .valueOf(mUncheck));
                    Elog.d(TAG, "cmdSend: " + cmdSend);
                    ret = AudioTuningJni
                            .setAudioCommand(mCmd, isChecked ? mCheck : mUncheck);
                    mAudioCheckBoxRet.add("Set: 0x" + cmdSend + " Ret: "
                            + String.valueOf(ret));
                } else if (mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mType.get(
                        cmdParameter - 1).equals(SET_PARAMETERS)) {
                    String mCmd = mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mCmd
                            .get(cmdParameter - 1);
                    String mCheck = mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mCheck
                            .get(cmdParameter - 1);
                    String mUncheck = mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mUncheck
                            .get(cmdParameter - 1);
                    cmdSend = mCmd + "=" + (isChecked ? mCheck : mUncheck);
                    Elog.d(TAG, "cmdSend: " + cmdSend);
                    ret = AudioSystem.setParameters(cmdSend);
                    mAudioCheckBoxRet.add("Set: " + cmdSend + "   Ret: "
                            + String.valueOf(ret));
                } else if (mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mType.get(
                        cmdParameter - 1).equals(GET_AUDIO_COMMAND)) {
                    String str = mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mCmd
                            .get(cmdParameter - 1);
                    int mCmd = Integer.decode(str).intValue();
                    cmdSend = str;
                    Elog.d(TAG, "cmdSend: " + cmdSend);
                    ret = AudioTuningJni.getAudioCommand(mCmd);
                    mAudioCheckBoxRet.add("Get: " + cmdSend + "  Ret: "
                            + String.valueOf(ret));
                } else if (mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mType.get(
                        cmdParameter - 1).equals(GET_PARAMETERS)) {
                    String str = mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mCmd
                            .get(cmdParameter - 1);
                    cmdSend = str;
                    String retString = AudioSystem.getParameters(str);
                    Elog.d(TAG, "cmdSend: " + cmdSend);
                    mAudioCheckBoxRet.add("Get: " + cmdSend + "   Ret: " + retString);
                }
                if (mCategoryTitleFlag == false) {
                    Elog.d(TAG, "mCmdName is checked, show dialog");
                    mAudioTitle = mAudioLoggerXMLData.mAudioDumpOperation.get(cmdID).mCategoryTitle;
                    if(isChecked && ("Output".equals(mAudioTitle) ||
                            "Input".equals(mAudioTitle) ||
                            "USB".equals(mAudioTitle))) {
                        Toast.makeText(AudioAudioLogger.this, R.string.audio_dump_path_change_msg,
                                Toast.LENGTH_LONG).show();
                    }
                    showDialog(DIALOG_ID_SHOW_CHECKBOX_RET);
                }
            }
        }
    };

    /**
     * @param audioSpn.
     *            Spinner
     * @param mAudioLoggerXMLData
     *            AudioLoggerXMLData
     * @return true is OK
     */
    private boolean initmSpSetAudioCommand(Spinner audioSpn,
            AudioLoggerXMLData mAudioLoggerXMLData) {
        List<String> audioList = mAudioLoggerXMLData.mAudioCommandSetOperation;
        int mAudioCount = audioList.size();
        if (mAudioCount > 0) {
            ArrayAdapter<String> audioAdatper = new ArrayAdapter<String>(this,
                    android.R.layout.simple_spinner_item, audioList);
            audioAdatper
                    .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            audioSpn.setAdapter(audioAdatper);
            return true;
        } else {
            Elog.d(TAG, "init audio spinner fail; mmAudioCount:" + mAudioCount);
            return false;
        }

    }

    /**
     * @param audioSpn.
     *            Spinner
     * @param mAudioLoggerXMLData
     *            mAudioLoggerXMLData
     * @return true is OK
     */
    private boolean initmSpGetAudioCommand(Spinner audioSpn,
            AudioLoggerXMLData mAudioLoggerXMLData) {

        List<String> audioList = mAudioLoggerXMLData.mAudioCommandGetOperation;
        int mAudioCount = audioList.size();

        if (mAudioCount > 0) {
            ArrayAdapter<String> audioAdatper = new ArrayAdapter<String>(this,
                    android.R.layout.simple_spinner_item, audioList);
            audioAdatper
                    .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            audioSpn.setAdapter(audioAdatper);
            return true;
        } else {
            Elog.d(TAG, "init audio spinner fail; mmAudioCount:" + mAudioCount);
            return false;
        }

    }

    /**
     * @param audioSpn.
     *            Spinner
     * @param mAudioLoggerXMLData
     *            AudioLoggerXMLData
     * @return true is OK
     */
    private boolean initmSpSetAudioParameter(Spinner audioSpn,
            AudioLoggerXMLData mAudioLoggerXMLData) {

        List<String> audioList = mAudioLoggerXMLData.mParametersSetOperationItems;
        int mAudioCount = audioList.size();

        if (mAudioCount > 0) {
            ArrayAdapter<String> audioAdatper = new ArrayAdapter<String>(this,
                    android.R.layout.simple_spinner_item, audioList);
            audioAdatper
                    .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            audioSpn.setAdapter(audioAdatper);
            return true;
        } else {
            Elog.d(TAG, "init audio spinner fail; mAudioCount:" + mAudioCount);
            return false;
        }
    }

    /**
     * @param audioSpn.
     *            Spinner
     * @param mAudioLoggerXMLData
     *            AudioLoggerXMLData
     * @return true is OK
     */
    private boolean initmSpGetAudioParameter(Spinner audioSpn,
            AudioLoggerXMLData mAudioLoggerXMLData) {

        List<String> audioList = mAudioLoggerXMLData.mParametersGetOperationItems;
        int mAudioCount = audioList.size();
        if (mAudioCount > 0) {
            ArrayAdapter<String> audioAdatper = new ArrayAdapter<String>(this,
                    android.R.layout.simple_spinner_item, audioList);
            audioAdatper
                    .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            audioSpn.setAdapter(audioAdatper);
            return true;
        } else {
            Elog.d(TAG, "init audio spinner fail; mAudioCount:" + mAudioCount);
            return false;
        }

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio_audiologger);
        mAudioLoggerXMLData = new AudioLoggerXMLData();
        mAudioCheckBoxRet = new ArrayList<String>();

        mXmlFile = new File(mFileName);
        if (!mXmlFile.exists()) {
            Elog.d(TAG, "mFileName:" + mFileName + "is not exists");
            Toast.makeText(AudioAudioLogger.this, mFileName + "is not exists",
                    Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        try {
            mInputStream = new FileInputStream(mXmlFile);
        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        try {
            parseXMLWithSAX(mInputStream, mAudioLoggerXMLData);
        } catch (SAXException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (ParserConfigurationException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        Elog.d(TAG, "parseXMLWithSAX pass!");

        mSpSetAudioCommand = (Spinner) this
                .findViewById(R.id.Audio_SetAudioCommandSpinner);
        mSpGetAudioCommand = (Spinner) this
                .findViewById(R.id.Audio_GetAudioCommandSpinner);
        mSpSetAudioParameter = (Spinner) this
                .findViewById(R.id.Audio_SetAudioParameterSpinner);
        mSpGetAudioParameter = (Spinner) this
                .findViewById(R.id.Audio_GetAudioParameterSpinner);

        mEditSetAudioCommandText = (EditText) this.findViewById(R.id.SetAudioCommandText);
        mEditGetAudioCommandText = (EditText) this.findViewById(R.id.GetAudioCommandText);

        mEditSetAudioParameterText = (EditText) this
                .findViewById(R.id.SetAudioParameterText);
        mEditGetAudioParameterText = (EditText) this
                .findViewById(R.id.GetAudioParameterText);

        mBtGetAudioCommand = (Button) this.findViewById(R.id.Audio_GetAudioCommand);
        mBtSetAudioCommand = (Button) this.findViewById(R.id.Audio_SetAudioCommand);

        mBtSetAudioParameter = (Button) this.findViewById(R.id.Audio_SetAudioParameter);
        mBtGetAudioParameter = (Button) this.findViewById(R.id.Audio_GetAudioParameter);

        mCheckBoxLinearLayout = (LinearLayout) this.findViewById(R.id.LinearLayoutCheck);

        mDumpOptionsCheck = new ArrayList<CheckBox>();

        for (int i = 0; i < mAudioLoggerXMLData.mAudioDumpOperation.size(); i++) {
            String title = mAudioLoggerXMLData.mAudioDumpOperation.get(i).mCategoryTitle;
            Elog.d(TAG, "title:" + i + " : " + title);

            CheckBox checkBox = new CheckBox(this);
            checkBox.setText(title);
            checkBox.setId((i) << 8);
            mDumpOptionsCheck.add(checkBox);
            mCheckBoxLinearLayout.addView(checkBox);
            boolean isAllChecked = true;
            CheckBox tempCheckBox = null;
            for (int j = 0; j < mAudioLoggerXMLData.mAudioDumpOperation.get(i).mCmdName
                    .size(); j++) {
                String mCmd = mAudioLoggerXMLData.mAudioDumpOperation.get(i).mCmd.get(j);
                String mCmdName = mAudioLoggerXMLData.mAudioDumpOperation.get(i).mCmdName
                        .get(j);
                String mType = mAudioLoggerXMLData.mAudioDumpOperation.get(i).mType.get(j);

                tempCheckBox = new CheckBox(this);
                tempCheckBox.setText(mCmdName);
                tempCheckBox.setX(30);
                tempCheckBox.setId(((i) << 8) + (j + 1));
                if ( mType.equals(SET_AUDIO_COMMAND) ) {
                    int cmdID = Integer.decode(mCmd).intValue() + 1;
                    mAudioGetRet = String.valueOf(AudioTuningJni.getAudioCommand(cmdID));
                    if ( !mAudioGetRet.isEmpty() ) {
                        Elog.d(TAG, "Audio_GetAudioCommand:" + mCmd + ","
                            + "ret= " + mAudioGetRet);
                        tempCheckBox.setChecked( mAudioGetRet.equals("1") ?  true : false );
                    }
                }
                else if (mType.equals(SET_PARAMETERS)) {
                    mAudioGetRet = AudioSystem.getParameters(mCmd);
                    if ( !mAudioGetRet.isEmpty() ) {
                        String[] strs = mAudioGetRet.split("=");
                        Elog.d(TAG, "Audio_GetAudioParameter:" + mCmd + ","
                                + "ret=" + mAudioGetRet);
                        tempCheckBox.setChecked( strs[1].equals("1") ?  true : false );
                    }
                }
                mDumpOptionsCheck.add(tempCheckBox);
                mCheckBoxLinearLayout.addView(tempCheckBox);
                Elog.d(TAG, "mAudioDumpOperation,mCmd:" + mCmd);
                Elog.d(TAG, "mAudioDumpOperation,mCmd name:" + mCmdName);
                isAllChecked &= tempCheckBox.isChecked();
            }
            checkBox.setChecked(isAllChecked);
        }

        for (int i = 0; i < mDumpOptionsCheck.size(); i++) {
            mDumpOptionsCheck.get(i).setOnCheckedChangeListener(mCheckedListener);
        }
        mSpSetAudioCommand.setOnItemSelectedListener(mSpinnerListener);
        mSpGetAudioCommand.setOnItemSelectedListener(mSpinnerListener);
        mSpSetAudioParameter.setOnItemSelectedListener(mSpinnerListener);
        mSpGetAudioParameter.setOnItemSelectedListener(mSpinnerListener);

        mBtSetAudioCommand.setOnClickListener(mAudioButtonClickListener);
        mBtGetAudioCommand.setOnClickListener(mAudioButtonClickListener);
        mBtSetAudioParameter.setOnClickListener(mAudioButtonClickListener);
        mBtGetAudioParameter.setOnClickListener(mAudioButtonClickListener);

        if (false == initmSpSetAudioCommand(mSpSetAudioCommand, mAudioLoggerXMLData)) {
            Toast.makeText(this, "mSpSetAudioCommand spinner fail", Toast.LENGTH_SHORT)
                    .show();
            finish();
            return;
        }
        if (false == initmSpGetAudioCommand(mSpGetAudioCommand, mAudioLoggerXMLData)) {
            Toast.makeText(this, "mSpSetAudioCommand spinner fail", Toast.LENGTH_SHORT)
                    .show();
            finish();
            return;
        }
        if (false == initmSpSetAudioParameter(mSpSetAudioParameter, mAudioLoggerXMLData)) {
            Toast.makeText(this, "mSpSetAudioCommand spinner fail", Toast.LENGTH_SHORT)
                    .show();
            finish();
            return;
        }
        if (false == initmSpGetAudioParameter(mSpGetAudioParameter, mAudioLoggerXMLData)) {
            Toast.makeText(this, "mSpSetAudioCommand spinner fail", Toast.LENGTH_SHORT)
                    .show();
            finish();
            return;
        }

    }

    /**
     * mCheck the sdcard is available.
     *
     * @return is sdcard avilable.
     */

    private Boolean checkSDCardIsAvaliable() {
        final String state = Environment.getExternalStorageState();
        Elog.i(Audio.TAG, "Environment.getExternalStorageState() is : " + state);
        if (state.equals(Environment.MEDIA_REMOVED)) {
            showDialog(DIALOG_ID_NO_SDCARD);
            return false;
        }

        if (state.equals(Environment.MEDIA_SHARED)) {
            showDialog(DIALOG_ID_SDCARD_BUSY);
            return false;
        }
        return true;
    }

    @Override
    public Dialog onCreateDialog(int dialogId) {
        switch (dialogId) {
        case DIALOG_ID_NO_SDCARD:
            return new AlertDialog.Builder(this).setTitle(R.string.no_sdcard_title)
                    .setMessage(R.string.no_sdcard_msg)
                    .setPositiveButton(android.R.string.ok, null).create();
        case DIALOG_ID_SDCARD_BUSY:
            return new AlertDialog.Builder(this).setTitle(R.string.sdcard_busy_title)
                    .setMessage(R.string.sdcard_busy_msg)
                    .setPositiveButton(android.R.string.ok, null).create();
        case DIALOG_ID_SHOW_GET_RET:
            return new AlertDialog.Builder(this)
                    .setTitle(mAudioTitle)
                    .setMessage("ret : " + mAudioGetRet)
                    .setPositiveButton(android.R.string.ok,
                            new android.content.DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface arg0, int arg1) {
                                    removeDialog(DIALOG_ID_SHOW_GET_RET);
                                }
                            }).create();
        case DIALOG_ID_SHOW_SET_RET:
            return new AlertDialog.Builder(this)
                    .setTitle(mAudioTitle)
                    .setMessage("ret : " + String.valueOf(mAudioSetRet))
                    .setPositiveButton(android.R.string.ok,
                            new android.content.DialogInterface.OnClickListener() {

                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    // TODO Auto-generated method stub
                                    removeDialog(DIALOG_ID_SHOW_SET_RET);
                                }
                            }).create();

        case DIALOG_ID_SHOW_CHECKBOX_RET:
            CharSequence[] cs = mAudioCheckBoxRet
                    .toArray(new CharSequence[mAudioCheckBoxRet.size()]);

            return new AlertDialog.Builder(this)
                    .setTitle(mAudioTitle)
                    .setItems(cs, null)
                    .setPositiveButton(android.R.string.ok,
                            new android.content.DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    // TODO Auto-generated method stub
                                    mAudioCheckBoxRet.clear();
                                    removeDialog(DIALOG_ID_SHOW_CHECKBOX_RET);
                                }
                            }).create();
        default:
            return null;

        }

    }

    /**
     * @param xmlData.
     *            InputStream
     * @param mAudioLoggerXMLData
     * @throws SAXException
     * @throws ParserConfigurationException
     * @throws IOException
     */
    private void parseXMLWithSAX(InputStream xmlData,
            AudioLoggerXMLData mAudioLoggerXMLData) throws SAXException,
            ParserConfigurationException, IOException {

        SAXParserFactory factory = SAXParserFactory.newInstance();

        XMLReader xmlReader = factory.newSAXParser().getXMLReader();

        ContentHandler handler = new ContentHandler(mAudioLoggerXMLData);

        xmlReader.setContentHandler(handler);

        xmlReader.parse(new InputSource(xmlData));
    }

}
