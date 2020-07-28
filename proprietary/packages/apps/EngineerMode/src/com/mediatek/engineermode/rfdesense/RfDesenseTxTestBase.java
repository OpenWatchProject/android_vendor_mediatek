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

package com.mediatek.engineermode.rfdesense;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;


public class RfDesenseTxTestBase extends Activity {
    public static final String TAG = "RfDesense/TxTestBase";

    protected static final int START = 1;
    protected static final int PAUSE = 2;
    protected static final int UPDATE_BUTTON = 4;
    protected static final int HINT = 5;

    protected static final int UPDATE_DELAY = 1000;

    protected static final int CHANNEL_DEFAULT = 0;
    protected static final int CHANNEL_MIN = 1;
    protected static final int CHANNEL_MAX = 2;
    protected static final int CHANNEL_MIN2 = 3;
    protected static final int CHANNEL_MAX2 = 4;
    protected static final int POWER_DEFAULT = 5;
    protected static final int POWER_MIN = 6;
    protected static final int POWER_MAX = 7;

    protected static final int STATE_NONE = 0;
    protected static final int STATE_STARTED = 1;
    protected static final int STATE_PAUSED = 2;

    protected Spinner mBand;
    // protected RadioGroup mModulation;
    protected Editor mChannel = new Editor();
    protected Editor mPower = new Editor();
    protected Editor mAfc = new Editor();
    protected Editor mTsc = new Editor();
    protected Spinner mPattern;
    protected CheckBox mAntMode;
    protected EditText mAntStatus;
    protected Button mButtonSet;
    protected Toast mToast = null;
    protected TextView mDbm;
    protected Phone mPhone;
    protected int mCurrentBand = -1;
    private String[] mBandArray;
    private String[] mPatternArray;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.rf_desense_tx_test_gsm);

        mBand = (Spinner) findViewById(R.id.band_spinner);
        mChannel.editor = (EditText) findViewById(R.id.channel_editor);
        mPower.editor = (EditText) findViewById(R.id.power_editor);
        mAfc.editor = (EditText) findViewById(R.id.afc_editor);
        mTsc.editor = (EditText) findViewById(R.id.tsc_editor);
        mPattern = (Spinner) findViewById(R.id.pattern_spinner);

        mAntMode = (CheckBox) findViewById(R.id.rf_ant_mode);
        mAntStatus = (EditText) findViewById(R.id.rf_ant_status);

        mButtonSet = (Button) findViewById(R.id.button_set);

        mDbm = (TextView) findViewById(R.id.powerDbm);

        mBandArray = getResources().getStringArray(R.array.rf_desense_tx_test_gsm_band_values);
        mPatternArray = getResources().getStringArray(
                R.array.rf_desense_tx_test_gsm_pattern_values);

        Button.OnClickListener listener = new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                switch (v.getId()) {
                    case R.id.button_set:
                        if (!checkValues()) {
                            break;
                        }
                        String band = mBandArray[mBand.getSelectedItemPosition()];
                        String channel = mChannel.getText();
                        String power = mPower.getText();
                        String afc = mAfc.getText();
                        String tsc = mTsc.getText();
                        String pattern = mPatternArray[mPattern.getSelectedItemPosition()];
                        String command = "AT+ERFTX=2,1," + channel + "," + afc + "," + band + ","
                                + tsc + "," + power + "," + pattern;
                        saveState(command);
                        showToast("Set param suecceed!");
                        break;

                    default:
                        break;
                }
            }
        };

        mButtonSet.setOnClickListener(listener);

        AdapterView.OnItemSelectedListener bandAdapter = new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (arg2 != mCurrentBand) {
                    mCurrentBand = arg2;
                    updateLimits();
                    setDefaultValues();
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        };

        mBand.setOnItemSelectedListener(bandAdapter);
    }

    private void saveState(String command) {
        SharedPreferences.Editor pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE).edit();
        pref.putInt(RfDesenseTxTestGsm.KEY_BAND, mBand.getSelectedItemPosition());
        pref.putString(RfDesenseTxTestGsm.KEY_CHANNEL, mChannel.getText());
        pref.putString(RfDesenseTxTestGsm.KEY_POWER, mPower.getText());
        pref.putString(RfDesenseTxTestGsm.KEY_AFC, mAfc.getText());
        pref.putString(RfDesenseTxTestGsm.KEY_TSC, mTsc.getText());
        pref.putInt(RfDesenseTxTestGsm.KEY_PATTERN, mPattern.getSelectedItemPosition());
        pref.putString(RfDesenseTxTestGsm.KEY_ANT_MODE, mAntMode.isChecked() ? "1" : "0");
        pref.putString(RfDesenseTxTestGsm.KEY_ANT_STATUS, mAntStatus.getText().toString());
        pref.putString(RfDesenseTxTest.KEY_GSM_ATCMD, command);
        String ant_str = "AT+ETXANT=1,1," + mAntStatus.getText();
        if (mAntMode.isChecked())
            pref.putString(RfDesenseTxTest.KEY_GSM_ATCMD_ANT_SWITCH, ant_str);
        else {
            ant_str = "AT+ETXANT=0,1,0";
            pref.putString(RfDesenseTxTest.KEY_GSM_ATCMD_ANT_SWITCH, ant_str);
        }
        Elog.i(TAG, "GSM at command = " + command);
        Elog.i(TAG, "GSM at command ant status = " + ant_str);
        pref.apply();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    protected void updateLimits() {
    }

    protected boolean checkValues() {
        Editor[] editors = new Editor[]{mChannel, mPower, mAfc, mTsc};
        String[] toast = new String[]{"Channel", "TX Power", "AFC", "TSC"};

        for (int i = 0; i < 4; i++) {
            Editor editor = editors[i];
            if (editor.editor.getVisibility() == View.VISIBLE && !editor.check()) {
                String text = "Invalid " + toast[i] + ". Valid range: "
                        + editor.getValidRange();
                Toast.makeText(this, text, Toast.LENGTH_SHORT).show();
                return false;
            }
        }

        return true;
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_LONG);
        mToast.show();
    }

    protected void setDefaultValues() {
        mChannel.setToDefault();
        mPower.setToDefault();
        mAfc.setToDefault();
        mTsc.setToDefault();
        mAntMode.setChecked(RfDesenseTxTestLte.DEFAULT_ANT_MODE.equals("1"));
        mAntStatus.setText(RfDesenseTxTestLte.DEFAULT_ANT_STATUS);
    }

    // Some help methods to make code line shorter
    protected static class Editor {
        public EditText editor;
        public String defaultValue = null;
        public int min;
        public int max;
        public int min2;
        public int max2;
        public int step = 1;

        public void set(String arg0, String arg1, String arg2, String arg3, String arg4) {
            defaultValue = arg0;
            min = Integer.parseInt(arg1);
            max = Integer.parseInt(arg2);
            min2 = Integer.parseInt(arg3);
            max2 = Integer.parseInt(arg4);
        }

        public void set(String arg0, String arg1, String arg2) {
            defaultValue = arg0;
            min = Integer.parseInt(arg1);
            max = Integer.parseInt(arg2);
            min2 = Integer.parseInt(arg1);
            max2 = Integer.parseInt(arg2);
        }

        public String getText() {
            return editor.getText().toString();
        }

        public void setText(String text) {
            editor.setText(text);
        }

        public void setToDefault() {
            editor.setText(defaultValue);
        }

        public boolean check() {
            try {
                int value = Integer.parseInt(getText());

                //Elog.i(TAG, "value = " + value);
                //Elog.i(TAG, "limits = [" + min + "," + max + "],[" + min2 + "," + max2 + "]");
                if ((value < min || value > max) && (value < min2 || value > max2)) {
                    return false;
                }
                if (step != 1 && (value - min) % step != 0) {
                    return false;
                }
            } catch (NumberFormatException e) {
                return false;
            }

            return true;
        }

        public String getValidRange() {
            String text = "[" + min + "," + max + "]";
            if (min != min2) {
                text += ", [" + min2 + "," + max2 + "]";
            }
            if (step != 1) {
                text += ", step " + step;
            }
            return text;
        }
    }
}
