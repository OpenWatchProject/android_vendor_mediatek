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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.modemtest;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

/**
 * Modem Test.
 */
public class ModemTestActivity extends Activity implements OnCheckedChangeListener {
    public static final String TAG = "ModemTest";
    private static final int MODEM_NONE = 0;
    private static final int MODEM_CTA = 1;
    private static final int MODEM_FTA = 2;
    private static final int MODEM_IOT = 3;
    private static final int MODEM_QUERY = 4;
    private static final int MODEM_OPERATOR = 5;
    private static final int MODEM_FACTORY = 6;
    private static final int MODEM_QUERY_CDMA = 7;
    private static final int MODEM_CDMA = 8;
    private static final int MODEM_QUERY_CLSC = 9;
    private static final int MODEM_CLSC = 10;

    private static final int REBOOT_DIALOG = 2000;
    private static final int CMD_LENGTH = 6;
    private static final int MODE_LENGTH = 3;




    private static final int ATTACH_MODE_NOT_SPECIFY = -1;
    private static final int DOCOMO_OPTION = 1 << 7;
    private static final int SOFTBANK_OPTION = 1 << 8;
    private static final int PCH_DATA_PREFER = 0;
    private static final int PCH_CALL_PREFER = 1;

    private static final int INDEX_SPIRENT = 1;
    private static final int FLAG_UNLOCK = 0x200000;
    private static final int FLAG_NOT_DETECT_CDMA_CARD = 0x100000;

    private int mCtaOption = 0;
    private int mIotOption = 0;
    private int mFtaOption = 0;
    private int mOperatorOption = 0;
    private int mCdmaOption = 0;

    private Button mNoneBtn;
    private Button mCtaBtn;
    private Button mFtaBtn;
    private Button mIotBtn;
    private Button mOperatorBtn;
    private Button mFactoryBtn;
    private Button mC2kBtn;
    private CheckBox mUnlockCheckBox;
    private CheckBox mCdmaSimSettingCheckBox;
    private TextView mTextView;

    private String[] mFtaOptionsArray;
    private String[] mIotOptionsArray;
    private String[] mOperatorOptionsArray;

    private int mCurrentMode = 0;
    private int mCurrentFlag = 0;

    private boolean mFirstEntry = true;
    private final Handler mATCmdHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            boolean rebootFlag = false;
            switch (msg.what) {
                case MODEM_NONE:
                case MODEM_CTA:
                case MODEM_FTA:
                case MODEM_IOT:
                case MODEM_OPERATOR:
                case MODEM_FACTORY:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        EmUtils.showToast(getToastString(msg.what) + " AT cmd success.");
                        rebootFlag = true;
                    } else {
                        EmUtils.showToast(getToastString(msg.what) + " AT cmd failed.");
                    }
                    break;
                case MODEM_QUERY:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        Elog.d(TAG, "Query success.");
                        String[] data = (String[]) ar.result;
                        handleQuery(data);
                        if (ModemCategory.isCdma()) {
                            queryCdmaOption();
                            queryUnlockOption();
                        }
                    } else {
                        EmUtils.showToast(" Query failed.");
                    }
                    break;
                case MODEM_QUERY_CDMA:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        Elog.i(TAG, "Query success.");
                        String[] data = (String[]) ar.result;
                        handleQueryCdma(data);
                    } else {
                        EmUtils.showToast(" Query failed.");
                    }
                    break;
                case MODEM_CDMA:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        EmUtils.showToast("AT cmd success.");
                        rebootFlag = true;
                    } else {
                        EmUtils.showToast("AT cmd failed.");
                    }
                    break;
                case MODEM_QUERY_CLSC:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        Elog.i(TAG, "Query success.");
                        String[] data = (String[]) ar.result;
                        handleQueryUnlock(data);
                    } else {
                        EmUtils.showToast("Query failed.");
                    }
                    break;
                case MODEM_CLSC:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        EmUtils.showToast("AT cmd success.");
                    } else {
                        EmUtils.showToast("AT cmd failed.");
                    }
                    break;
                default:
                    break;
            }
            if (rebootFlag) {
                Elog.i(TAG, "disableAllButton.");
                disableAllButton();
            }
        }
    };

    private String getToastString(int what) {
        switch (what) {
            case MODEM_NONE:
                return "MODEM_NONE";
            case MODEM_CTA:
                return "MODEM_CTA";
            case MODEM_FTA:
                return "MODEM_FTA";
            case MODEM_IOT:
                return "MODEM_IOT";
            case MODEM_OPERATOR:
                return "MODEM_OPERATOR";
            case MODEM_FACTORY:
                return "MODEM_FACTORY";
            default:
                return "";
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.modem_test_activity_6589);

        String[] cmd = new String[2];
        cmd[0] = "AT+EPCT?";
        cmd[1] = "+EPCT:";

        View.OnClickListener listener = new View.OnClickListener() {

            public void onClick(View v) {
                switch (v.getId()) {
                    case R.id.modem_test_none_btn:
                        sendATCommad("0", 0, MODEM_NONE);
                        if (mCurrentMode == MODEM_FTA) {
                            setGprsTransferType(PCH_CALL_PREFER);
                        }
                        break;
                    case R.id.modem_test_cta_btn:
                        sendATCommad("1", 0, MODEM_CTA);
                        if (mCurrentMode == MODEM_FTA) {
                            setGprsTransferType(PCH_CALL_PREFER);
                        }
                        break;
                    case R.id.modem_test_fta_btn:
                        showDialog(MODEM_FTA);
                        break;
                    case R.id.modem_test_iot_btn:
                        showDialog(MODEM_IOT);
                        break;
                    case R.id.modem_test_operator_btn:
                        showDialog(MODEM_OPERATOR);
                        break;
                    case R.id.modem_test_factory_btn:
                        sendATCommad("5", 0, MODEM_FACTORY);
                        if (mCurrentMode == MODEM_FTA) {
                            setGprsTransferType(PCH_CALL_PREFER);
                        }
                        break;
                    case R.id.modem_test_c2k_btn:
                        showDialog(MODEM_CDMA);
                    default:
                        break;
                }
            }
        };
        mTextView = (TextView) findViewById(R.id.modem_test_textview);
        mNoneBtn = (Button) findViewById(R.id.modem_test_none_btn);
        mFactoryBtn = (Button) findViewById(R.id.modem_test_factory_btn);
        mFactoryBtn.setOnClickListener(listener);
        mCtaBtn = (Button) findViewById(R.id.modem_test_cta_btn);
        mCtaBtn.setOnClickListener(listener);

        mFtaBtn = (Button) findViewById(R.id.modem_test_fta_btn);
        mIotBtn = (Button) findViewById(R.id.modem_test_iot_btn);
        mOperatorBtn = (Button) findViewById(R.id.modem_test_operator_btn);
        mC2kBtn = (Button) findViewById(R.id.modem_test_c2k_btn);
        mUnlockCheckBox = (CheckBox) findViewById(R.id.modem_test_unlock);

        mCdmaSimSettingCheckBox = (CheckBox) findViewById(R.id.modem_test_cdma_sim_setting);
        mNoneBtn.setOnClickListener(listener);
        mFtaBtn.setOnClickListener(listener);
        mIotBtn.setOnClickListener(listener);
        mOperatorBtn.setOnClickListener(listener);
        mC2kBtn.setOnClickListener(listener);
        mUnlockCheckBox.setOnCheckedChangeListener(this);

        mCdmaSimSettingCheckBox.setOnCheckedChangeListener(this);
        mTextView.setText("The current mode is unknown");
        mFtaOptionsArray = getResources().getStringArray(
                R.array.modem_test_fta_options);
        mIotOptionsArray = getResources().getStringArray(
                R.array.modem_test_iot_options_6589);
        mOperatorOptionsArray = getResources().getStringArray(
                R.array.modem_test_operator_options_6589);

        if (!ModemCategory.isCdma()) {
            mC2kBtn.setVisibility(View.GONE);
            mUnlockCheckBox.setVisibility(View.GONE);
        }

        EmUtils.invokeOemRilRequestStringsEm(cmd, mATCmdHander.obtainMessage(MODEM_QUERY));
    }

    @Override
    protected void onResume() {
        super.onResume();
        mCtaOption = 0;
        mIotOption = 0;
        mFtaOption = 0;
        mOperatorOption = 0;
        mCdmaOption = 0;
    }


    private void sendATCommad(String str, int flag, int message) {
        String[] cmd = new String[2];
        mCurrentFlag = (mCurrentFlag & 0xFF0000) | flag;
        cmd[0] = "AT+EPCT=" + str + "," + mCurrentFlag;
        cmd[1] = "";
        Elog.v(TAG, "cmd = " + cmd[0]);
        EmUtils.invokeOemRilRequestStringsEm(cmd, mATCmdHander.obtainMessage(message));
    }

    private void handleQuery(String[] data) {
        if (null == data) {
            EmUtils.showToast("The returned data is wrong.");
            return;
        } else {
            Elog.i(TAG, "data length is " + data.length);
            int i = 0;
            for (String str : data) {
                if (str != null) {
                    Elog.i(TAG, "data[" + i + "] is : " + str);
                }
                i++;
            }
        }
        if ((data.length > 0) && (data[0] != null) && (data[0].length() > CMD_LENGTH)) {
            String mode = data[0].substring(CMD_LENGTH + 1, data[0].length());
            Elog.i(TAG, "mode is " + mode);
            if (mode.length() >= MODE_LENGTH) {
                String subMode = mode.substring(0, 1);
                String subCtaMode = mode.substring(2, mode.length());
                Elog.i(TAG, "subMode is " + subMode);
                Elog.i(TAG, "subCtaMode is " + subCtaMode);
                mCurrentMode = Integer.parseInt(subMode);
                mCurrentFlag = Integer.parseInt(subCtaMode);
                handleQueryCdmaSimSetting(mCurrentFlag);
                if ("0".equals(subMode)) {
                    mTextView.setText("The current mode is none");
                } else if ("1".equals(subMode)) {
                    mTextView.setText("The current mode is Integrity Off");
                } else if ("2".equals(subMode)) {
                    mTextView.setText("The current mode is FTA:");
                    try {
                        int ftaLength = mFtaOptionsArray.length;
                        Elog.i(TAG, "ftaLength is " + ftaLength);
                        int val = Integer.valueOf(subCtaMode).intValue();
                        Elog.i(TAG, "val is " + val);
                        String text = "The current mode is FTA: ";
                        for (int j = 0; j < ftaLength; j++) {
                            Elog.i(TAG, "j ==== " + j);
                            Elog.i(TAG, "(val & (1 << j)) is "
                                    + (val & (1 << j)));
                            if ((val & (1 << j)) != 0) {
                                text = text + mFtaOptionsArray[j] + ",";
                            }
                        }
                        // Drop the last ","
                        text = text.substring(0, text.length() - 1);
                        mTextView.setText(text);
                    } catch (NumberFormatException e) {
                        e.printStackTrace();
                        Elog.i(TAG, "Exception when transfer subFtaMode");
                    }
                } else if ("3".equals(subMode)) {
                    mTextView.setText("The current mode is IOT:");
                    try {
                        int iotLength = mIotOptionsArray.length;
                        Elog.i(TAG, "iotLength is " + iotLength);
                        int val = Integer.valueOf(subCtaMode).intValue();
                        Elog.i(TAG, "val is " + val);
                        String text = "The current mode is IOT: ";
                        for (int j = 0; j < iotLength - 1; j++) {
                            Elog.i(TAG, "j ==== " + j);
                            Elog.i(TAG, "(val & (1 << j)) is "
                                    + (val & (1 << j)));
                            if ((val & (1 << j)) != 0) {
                                text = text + mIotOptionsArray[j + 1] + ",";
                            }
                        }
                        // Drop the last ","
                        text = text.substring(0, text.length() - 1);
                        mTextView.setText(text);
                    } catch (NumberFormatException e) {
                        e.printStackTrace();
                        Elog.i(TAG, "Exception when transfer subIotMode");
                    }
                } else if ("4".equals(subMode)) {
                    mTextView.setText("The current mode is Operator.");
                    try {
                        int operatorLength = mOperatorOptionsArray.length;
                        Elog.i(TAG, "operatorLength is " + operatorLength);
                        int val = Integer.valueOf(subCtaMode).intValue();
                        Elog.i(TAG, "val is " + val);
                        String text = "The current mode is Operator: ";
                        for (int j = 0; j < operatorLength; j++) {
                            Elog.i(TAG, "j ==== " + j);
                            Elog.i(TAG, "(val & (1 << j)) is "
                                    + (val & (1 << j)));
                            if ((val & (1 << j)) != 0) {
                                text = text + mOperatorOptionsArray[j]
                                        + ",";
                            }
                        }
                        // Drop the last ","
                        text = text.substring(0, text.length() - 1);
                        mTextView.setText(text);
                    } catch (NumberFormatException e) {
                        e.printStackTrace();
                        Elog.i(TAG, "Exception when transfer subFtaMode");
                    }
                } else if ("5".equals(subMode)) {
                    mTextView.setText("The current mode is Factory.");
                }
            } else {
                Elog.i(TAG, "mode len is " + mode.length());
            }
        } else {
            Elog.i(TAG, "The data returned is not right.");
        }
    }

    private void disableAllButton() {
        mNoneBtn.setEnabled(false);
        mFactoryBtn.setEnabled(false);
        mCtaBtn.setEnabled(false);
        mFtaBtn.setEnabled(false);
        mIotBtn.setEnabled(false);
        mOperatorBtn.setEnabled(false);
        mC2kBtn.setEnabled(false);
        mUnlockCheckBox.setEnabled(false);
        showDialog(REBOOT_DIALOG);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case REBOOT_DIALOG:
                return new AlertDialog.Builder(ModemTestActivity.this).setTitle(
                        "MODEM TEST").setMessage("Please reboot the phone!")
                        .setPositiveButton("OK", null).create();
            case MODEM_CTA:
                return new AlertDialog.Builder(ModemTestActivity.this).setTitle(
                        "MODEM TEST").setMultiChoiceItems(
                        R.array.modem_test_cta_options,
                        new boolean[]{false, false, false, false, false, false,
                                false, false, false},
                        new DialogInterface.OnMultiChoiceClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int whichButton, boolean isChecked) {

                            /* User clicked on a check box do some stuff */
                                Elog.v(TAG, "whichButton = " + whichButton);
                                Elog.v(TAG, "isChecked = " + isChecked);
                                if (isChecked) {
                                    mCtaOption = mCtaOption + (1 << whichButton);
                                } else {
                                    mCtaOption = mCtaOption - (1 << whichButton);
                                }
                                Elog.v(TAG, "mCtaOption = " + mCtaOption);
                            }
                        }).setPositiveButton("Send",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int whichButton) {

                            /* User clicked Yes so do some stuff */
                                sendATCommad("1", mCtaOption, MODEM_CTA);
                                if (mCurrentMode == MODEM_FTA) {
                                    setGprsTransferType(PCH_CALL_PREFER);
                                }
                            }
                        }).setNegativeButton("Cancel",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int whichButton) {

                            /* User clicked No so do some stuff */
                                mCtaOption = 0;
                            }
                        }).create();
            case MODEM_FTA:
                return new AlertDialog.Builder(ModemTestActivity.this)
                        .setTitle("MODEM TEST").setMultiChoiceItems(R.array.modem_test_fta_options,
                                new boolean[]{false, false, false, false, false,
                                        false, false, false, false},
                                new DialogInterface.OnMultiChoiceClickListener() {
                                    public void onClick(DialogInterface dialog,
                                                        int whichButton, boolean isChecked) {

                                    /* User clicked on a check box do some stuff */
                                        Elog.v(TAG, "whichButton = " + whichButton);
                                        Elog.v(TAG, "isChecked = " + isChecked);
                                        if (isChecked) {
                                            mFtaOption = mFtaOption
                                                    + (1 << whichButton);
                                        } else {
                                            mFtaOption = mFtaOption
                                                    - (1 << whichButton);
                                        }
                                        Elog.v(TAG, "mFtaOption = " + mFtaOption);
                                    }
                                }).setPositiveButton("Send",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog,
                                                        int whichButton) {

                                    /* User clicked Yes so do some stuff */
                                        sendATCommad("2", mFtaOption, MODEM_FTA);

                                        setGprsTransferType(PCH_DATA_PREFER);
                                    }
                                }).setNegativeButton("Cancel",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog,
                                                        int whichButton) {

                                    /* User clicked No so do some stuff */
                                        mFtaOption = 0;
                                    }
                                }).create();
            case MODEM_IOT:
                return new AlertDialog.Builder(ModemTestActivity.this).setTitle(
                        "MODEM TEST").setMultiChoiceItems(
                        R.array.modem_test_iot_options_6589,
                        new boolean[]{false, false, false, false, false, false,
                                false, false, false},
                        new DialogInterface.OnMultiChoiceClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int whichButton, boolean isChecked) {

                            /* User clicked on a check box do some stuff */
                                Elog.v(TAG, "whichButton = " + whichButton);
                                Elog.v(TAG, "isChecked = " + isChecked);
                                if (whichButton > 0) {
                                    if (isChecked) {
                                        mIotOption = mIotOption + (1 << (whichButton - 1));
                                    } else {
                                        mIotOption = mIotOption - (1 << (whichButton - 1));
                                    }
                                }
                                Elog.v(TAG, "mIotOption = " + mIotOption);
                            }
                        }).setPositiveButton("Send",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int whichButton) {

                            /* User clicked Yes so do some stuff */
                                sendATCommad("3", mIotOption, MODEM_IOT);
                                if (mCurrentMode == MODEM_FTA) {
                                    setGprsTransferType(PCH_CALL_PREFER);
                                }
                            }
                        }).setNegativeButton("Cancel",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int whichButton) {

                            /* User clicked No so do some stuff */

                                mIotOption = 0;
                            }
                        }).create();
            case MODEM_OPERATOR:
                return new AlertDialog.Builder(ModemTestActivity.this).setTitle(
                        "MODEM TEST").setMultiChoiceItems(
                        R.array.modem_test_operator_options_6589,
                        new boolean[]{false, false, false, false, false, false,
                                false, false, false, false, false, false, false, false,false},
                        new DialogInterface.OnMultiChoiceClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int whichButton, boolean isChecked) {

                            /* User clicked on a check box do some stuff */
                                Elog.v(TAG, "whichButton = " + whichButton);
                                Elog.v(TAG, "isChecked = " + isChecked);
                                if (isChecked) {
                                    mOperatorOption = mOperatorOption
                                            + (1 << whichButton);
                                } else {
                                    mOperatorOption = mOperatorOption
                                            - (1 << whichButton);
                                }
                                Elog.v(TAG, "mOperatorOption = " + mOperatorOption);
                            }
                        }).setPositiveButton("Send",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int whichButton) {

                            /* User clicked Yes so do some stuff */
                                attachOrDetachGprs();
                                sendATCommad("4", mOperatorOption, MODEM_OPERATOR);
                                if (mCurrentMode == MODEM_FTA) {
                                    setGprsTransferType(PCH_CALL_PREFER);
                                }
                            }
                        }).setNegativeButton("Cancel",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int whichButton) {

                            /* User clicked No so do some stuff */
                                mOperatorOption = 0;
                            }
                        }).create();
            case MODEM_CDMA:
                return new AlertDialog.Builder(ModemTestActivity.this)
                        .setTitle("MODEM TEST").setSingleChoiceItems(
                                R.array.modem_test_c2k_options, -1,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog,
                                                        int whichButton) {
                                    /* User clicked on a check box do some stuff */
                                        Elog.v(TAG, "whichButton = " + whichButton);
                                        mCdmaOption = whichButton;
                                        Elog.v(TAG, "mCdmaOption = " + mCdmaOption);
                                    }
                                }).setPositiveButton("Send",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog,
                                                        int whichButton) {
                                    /* User clicked Yes so do some stuff */
                                        setCdmaOption();
                                    }
                                }).setNegativeButton("Cancel",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog,
                                                        int whichButton) {
                                    /* User clicked No so do some stuff */
                                        mCdmaOption = 0;
                                    }
                                }).create();
            default:
                break;
        }
        return null;
    }

    private void attachOrDetachGprs() {
        if ((mOperatorOption & DOCOMO_OPTION) != 0 || (mOperatorOption & SOFTBANK_OPTION) != 0) {
            Elog.v(TAG, "Attach GPRS for DoCoMo/Softband");
            SystemProperties.set("persist.vendor.radio.gprs.attach.type", "1");
            String[] cmdStr = {"AT+EGTYPE=1,1", ""};
            EmUtils.invokeOemRilRequestStringsEm(cmdStr, null);
        } else {
            Elog.v(TAG, "Dettach GPRS for DoCoMo/Softband");
            SystemProperties.set("persist.vendor.radio.gprs.attach.type", "0");
            String[] cmdStr = {"AT+EGTYPE=0,1", ""};
            EmUtils.invokeOemRilRequestStringsEm(cmdStr, null);
        }
    }


    private void setGprsTransferType(int type) {
        String property = (type == PCH_DATA_PREFER ? "1" : "0");
        Elog.v(TAG, "Change persist.radio.gprs.prefer to " + property);
        try {
            EmUtils.getEmHidlService().setPreferGprsMode(property);
        } catch (Exception e) {
            e.printStackTrace();
            Elog.e(TAG, "set property failed ...");
        }
        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            EmUtils.invokeOemRilRequestStringsEm(i, new String[]{"AT+EGTP=" + type, ""}, null);
            EmUtils.invokeOemRilRequestStringsEm(i, new String[]{"AT+EMPPCH=" + type, ""}, null);
        }
    }

    private void queryUnlockOption() {
        // TODO: command
        String[] cmdOri = new String[3];
        cmdOri[0] = "AT+ECLSC?";
        cmdOri[1] = "+ECLSC:";
        cmdOri[2] = "DESTRILD:C2K";
        String[] cmd = ModemCategory.getCdmaCmdArr(cmdOri);

        EmUtils.invokeOemRilRequestStringsEm(true, cmd, mATCmdHander
                .obtainMessage(MODEM_QUERY_CLSC));
    }

    private void setUnlockOption(boolean unlock) {
        mCurrentFlag = unlock ? (mCurrentFlag | FLAG_UNLOCK) : (mCurrentFlag & ~FLAG_UNLOCK);

        EmUtils.invokeOemRilRequestStringsEm(new String[]{"AT+EPCT=" + mCurrentMode + "," +
                        mCurrentFlag, ""},
                mATCmdHander.obtainMessage(MODEM_CLSC));


        EmUtils.invokeOemRilRequestStringsEm(true,
                ModemCategory.getCdmaCmdArr(new String[]{"AT+ECLSC=" + (unlock ? "1" : "0"), "",
                        "DESTRILD:C2K"}),
                mATCmdHander.obtainMessage(MODEM_CLSC));
        EmUtils.invokeOemRilRequestStringsEm(true,
                ModemCategory.getCdmaCmdArr(new String[]{"AT+RFSSYNC", "", "DESTRILD:C2K"}),
                null);
    }


    private void queryCdmaOption() {
        String[] cmdOri = new String[3];
        cmdOri[0] = "AT+ECTM?";
        cmdOri[1] = "+ECTM:";
        cmdOri[2] = "DESTRILD:C2K";
        String[] cmd = ModemCategory.getCdmaCmdArr(cmdOri);
        EmUtils.invokeOemRilRequestStringsEm(true,
                cmd, mATCmdHander.obtainMessage(MODEM_QUERY_CDMA));
    }

    private void setCdmaOption() {
        if (mCdmaOption == INDEX_SPIRENT) {
            sendATCommandCdma("\"SPIRENT\"", MODEM_CDMA);
        } else {
            sendATCommandCdma("\"NONE\"", MODEM_CDMA);
        }
    }

    private void handleQueryCdmaSimSetting(int flag) {
        if ((flag & FLAG_NOT_DETECT_CDMA_CARD) != 0) {
            Elog.d(TAG, "handleQueryCdmaSimSetting set check true");
            mFirstEntry = true;
            mCdmaSimSettingCheckBox.setChecked(true);
        } else {
            Elog.d(TAG, "handleQueryCdmaSimSetting set check false");
            mFirstEntry = false;
            mCdmaSimSettingCheckBox.setChecked(false);
        }
    }

    private void setCdmaSimSettingOption(boolean check) {
        mCurrentFlag = check ? (mCurrentFlag | FLAG_NOT_DETECT_CDMA_CARD) :
                (mCurrentFlag & (~FLAG_NOT_DETECT_CDMA_CARD));
        EmUtils.invokeOemRilRequestStringsEm(
                new String[]{"AT+EPCT=" + mCurrentMode + "," + mCurrentFlag, ""},
                mATCmdHander.obtainMessage(MODEM_CLSC));
    }

    private void sendATCommandCdma(String str, int message) {
        String[] cmdOri = new String[3];
        cmdOri[0] = "AT+ECTM=" + str;
        cmdOri[1] = "";
        cmdOri[2] = "DESTRILD:C2K";
        String[] cmd = ModemCategory.getCdmaCmdArr(cmdOri);
        EmUtils.invokeOemRilRequestStringsEm(true, cmd, mATCmdHander.obtainMessage(message));
        cmd[0] = "AT+RFSSYNC";
        EmUtils.invokeOemRilRequestStringsEm(true, cmd, null);
    }

    private void handleQueryUnlock(String[] data) {
        if (null == data || data.length <= 0) {
            Toast.makeText(ModemTestActivity.this,
                    "The returned data is wrong.", Toast.LENGTH_SHORT)
                    .show();
            return;
        }
        if (data[0].length() > "+ECLSC:".length()) {
            String str = data[0].substring("+ECLSC:".length()).trim();
            Elog.i(TAG, "unlock setting is " + str);
            int value = Integer.parseInt(str);
            mUnlockCheckBox.setOnCheckedChangeListener(null);
            mUnlockCheckBox.setChecked(value == 1);
            mUnlockCheckBox.setOnCheckedChangeListener(this);
        } else {
            Elog.i(TAG, "The data returned is not right.");
        }
    }

    private void handleQueryCdma(String[] data) {
        if (null == data) {
            Toast.makeText(ModemTestActivity.this,
                    "The returned data is wrong.", Toast.LENGTH_SHORT)
                    .show();
            return;
        } else {
            Elog.i(TAG, "data length is " + data.length);
            int i = 0;
            for (String str : data) {
                if (str != null) {
                    Elog.i(TAG, "data[" + i + "] is : " + str);
                }
                i++;
            }
        }
        if (data[0].length() > CMD_LENGTH) {
            String mode = data[0].substring(CMD_LENGTH, data[0].length()).trim();
            mode = mode.substring(1, mode.length() - 1);
            Elog.i(TAG, "mode is " + mode);
            String text = mTextView.getText().toString();
            text += "\nThe current C2K mode is " + mode;
            mTextView.setText(text);
        } else {
            Elog.i(TAG, "The data returned is not right.");
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (buttonView.getId() == R.id.modem_test_unlock) {
            setUnlockOption(isChecked);
        }

        if (buttonView.getId() == R.id.modem_test_cdma_sim_setting) {
            setCdmaSimSettingOption(isChecked);
            if (mFirstEntry == true) {
                mFirstEntry = false;
                return;
            }
            showDialog(REBOOT_DIALOG);
        }
    }
}
