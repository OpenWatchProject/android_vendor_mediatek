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
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;


public class RfDesenseTxTestLte extends Activity implements Button.OnClickListener,
        CompoundButton.OnCheckedChangeListener {
    public static final int DEFAULT_MODULATION_MODE = 1;
    public static final int DEFAULT_BAND_FDD = 2;
    public static final int DEFAULT_BAND_TDD = 5;
    public static final int DEFAULT_BAND_WIDTH = 3;
    public static final String DEFAULT_FDD_FREQ = "17475";
    public static final String DEFAULT_TDD_FREQ = "25950";
    public static final int DEFAULT_TDD_CONFIG = 0;
    public static final int DEFAULT_TDD_SPECIAL = 0;
    public static final String DEFAULT_VRB_START = "0";
    public static final String DEFAULT_VRB_LENGTH = "1";
    public static final int DEFAULT_MCS = 0;
    public static final String DEFAULT_ANT_MODE = "0";
    public static final String DEFAULT_ANT_STATUS = "0";
    public static final String DEFAULT_POWER = "23";
    public static final String KEY_FDD_BAND = "band_fdd";
    public static final String KEY_FDD_BAND_WIDTH = "bandwidth_fdd";
    public static final String KEY_FDD_FREQ = "freq_fdd";
    public static final String KEY_FDD_VRB_START = "vrb_start_fdd";
    public static final String KEY_FDD_VRB_LENGTH = "vrb_length_fdd";
    public static final String KEY_FDD_MCS = "mcs_fdd";
    public static final String KEY_FDD_ANT_MODE = "ant_mode_fdd";
    public static final String KEY_FDD_ANT_STATUS = "ant_status_fdd";
    public static final String KEY_FDD_POWER = "power_fdd";
    public static final String KEY_TDD_BAND = "band_tdd";
    public static final String KEY_TDD_BAND_WIDTH = "bandwidth_tdd";
    public static final String KEY_TDD_FREQ = "freq_tdd";
    public static final String KEY_TDD_CONFIG = "tdd_config";
    public static final String KEY_TDD_SPECIAL = "tdd_special";
    public static final String KEY_TDD_VRB_START = "vrb_start_tdd";
    public static final String KEY_TDD_VRB_LENGTH = "vrb_length_tdd";
    public static final String KEY_TDD_MCS = "mcs_tdd";
    public static final String KEY_TDD_POWER = "power_tdd";
    public static final String KEY_TDD_ANT_MODE = "ant_mode_tdd";
    public static final String KEY_TDD_ANT_STATUS = "ant_status_tdd";

    public static final String KEY_MODULATION_MODE = "modulation_mode";
    private static final String TAG = "RfDesense/TxTestLte";
    private static final int DUPLEX_TDD = 0;
    private static final int DUPLEX_FDD = 1;
    private static final int FDD_BAND_MIN = 1;
    private static final int FDD_BAND_MAX = 31;
    private static final int TDD_BAND_MIN = 33;
    private static final int TDD_BAND_MAX = 44;
    private static final int TDD_CONFIG_MAX = 6;
    private static final int TDD_SPECIAL_MAX = 9;
    private static final int VRB_START_MIN = 0;
    private static final int VRB_START_MAX = 99;
    private static final int VRB_LENGTH_MIN = 1;
    private static final int VRB_LENGTH_MAX = 100;
    private static final int POWER_MIN = -50;
    private static final int POWER_MAX = 27;
    private static int mModemType;
    protected CheckBox mAntMode;
    protected EditText mAntStatus;
    String[] mFreqRangeArray;
    private RadioButton mFdd;
    private RadioButton mTdd;
    private Spinner mBand;
    private Spinner mBandWidth;
    private EditText mFreq;
    private Spinner mTddConfig;
    private Spinner mTddSpecial;
    private EditText mVrbStart;
    private EditText mVrbLength;
    private Spinner mMcs;
    private EditText mPower;
    private TextView mFreqRange;
    private Button mButtonSet;
    private Toast mToast = null;
    private int mCurrentBand = 0;

    private TextView mModulationMode;
    private RadioButton mSingleTone;
    private RadioButton mModulationSignal;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.rf_desense_tx_test_lte);
        Intent intent = getIntent();
        mModemType = intent.getIntExtra("mModemType", ModemCategory.MODEM_FDD);

        mModulationMode = (TextView) findViewById(R.id.rf_desense_modulation_mode);
        mSingleTone = (RadioButton) findViewById(R.id.single_radio_button);
        mModulationSignal = (RadioButton) findViewById(R.id.modulatio_radio_button);
        if (!FeatureSupport.is93Modem()) {
            mModulationMode.setVisibility(View.GONE);
            mSingleTone.setVisibility(View.GONE);
            mModulationSignal.setVisibility(View.GONE);
        }
        mFdd = (RadioButton) findViewById(R.id.fdd_radio_button);
        mTdd = (RadioButton) findViewById(R.id.tdd_radio_button);
        mFdd.setChecked(false);
        mTdd.setChecked(false);
        mBand = (Spinner) findViewById(R.id.band_spinner);
        mBandWidth = (Spinner) findViewById(R.id.bandwidth_spinner);
        mFreq = (EditText) findViewById(R.id.freq_editor);
        mTddConfig = (Spinner) findViewById(R.id.tdd_config_spinner);
        mTddSpecial = (Spinner) findViewById(R.id.tdd_special_spinner);
        mVrbStart = (EditText) findViewById(R.id.vrb_start_editor);
        mVrbLength = (EditText) findViewById(R.id.vrb_length_editor);
        mMcs = (Spinner) findViewById(R.id.mcs_spinner);
        mPower = (EditText) findViewById(R.id.power_editor);
        mFreqRange = (TextView) findViewById(R.id.freq);
        mAntMode = (CheckBox) findViewById(R.id.rf_ant_mode);
        mAntStatus = (EditText) findViewById(R.id.rf_ant_status);
        mButtonSet = (Button) findViewById(R.id.button_set);

        ArrayAdapter<CharSequence> adapter =
                new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBand.setAdapter(adapter);

        adapter = ArrayAdapter.createFromResource(this,
                R.array.rf_desense_tx_test_lte_bandwidth,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBandWidth.setAdapter(adapter);

        adapter = ArrayAdapter.createFromResource(this, R.array.rf_desense_tx_test_lte_mcs,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mMcs.setAdapter(adapter);

        adapter = new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mTddConfig.setAdapter(adapter);

        adapter = new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mTddSpecial.setAdapter(adapter);

        mFdd.setOnCheckedChangeListener(this);
        mTdd.setOnCheckedChangeListener(this);
        mButtonSet.setOnClickListener(this);

        mBand.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (arg2 != mCurrentBand) {
                    mCurrentBand = arg2;
                    setDefaultValue();
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });

        mFreqRangeArray = getResources().getStringArray(
                R.array.rf_desense_tx_test_lte_freq_range);


        if (mModemType == ModemCategory.MODEM_TD) {
            mTdd.setChecked(true);
        } else {
            mFdd.setChecked(true);
        }

        restoreState();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onCheckedChanged(CompoundButton v, boolean checked) {
        switch (v.getId()) {
            case R.id.fdd_radio_button:
            case R.id.tdd_radio_button:
                onDuplexChange();
                break;
            default:
                break;
        }
    }

    int calcExtentedIndex(int SelectedItemPosition) {
        int index;
        if (SelectedItemPosition == 31) {     //band 66
            index = 34;
        } else if (SelectedItemPosition == 32) {// band 71
            index = 38;
        } else {
            index = 0;
        }
        return index;
    }


    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.button_set:
                if (!checkValues()) {
                    break;
                }
                String atcmd = "";
                if (FeatureSupport.is93Modem() && mSingleTone.isChecked()) {
                    atcmd = "AT+ERFTX=6,0,2,";
                } else {
                    atcmd = "AT+ERFTX=6,0,1,";
                }
                String command = atcmd
                        + (mBand.getSelectedItemPosition()
                        + (mTdd.isChecked() ? TDD_BAND_MIN : FDD_BAND_MIN)
                        + calcExtentedIndex(mBand.getSelectedItemPosition())) + ","
                        + mBandWidth.getSelectedItemPosition() + ","
                        + mFreq.getText().toString() + ","
                        + (mTdd.isChecked() ? DUPLEX_TDD : DUPLEX_FDD) + ","
                        + mTddConfig.getSelectedItemPosition() + ","
                        + mTddSpecial.getSelectedItemPosition() + ","
                        + mVrbStart.getText().toString() + ","
                        + mVrbLength.getText().toString() + ","
                        + mMcs.getSelectedItemPosition() + ","
                        + mPower.getText().toString();
                if (mAntMode.isChecked() && FeatureSupport.is93Modem()) {
                    command += ",1," + mAntStatus.getText().toString();
                }
                saveState(command);
                showToast("Set param suecceed, " + command);
                Elog.d(TAG, "command = " + command);
                break;
            default:
                break;
        }
    }

    private void onDuplexChange() {
        boolean tdd = mTdd.isChecked();
        int bandMin = tdd ? TDD_BAND_MIN : FDD_BAND_MIN;
        int bandMax = tdd ? TDD_BAND_MAX : FDD_BAND_MAX;
        int configMax = tdd ? TDD_CONFIG_MAX : 0;
        int specialMax = tdd ? TDD_SPECIAL_MAX : 0;
        mTddConfig.setEnabled(tdd);
        mTddSpecial.setEnabled(tdd);
        Elog.i(TAG, "tdd = " + tdd);

        ArrayAdapter<CharSequence> adapter = (ArrayAdapter<CharSequence>) mBand.getAdapter();
        adapter.clear();
        for (int i = bandMin; i <= bandMax; i++) {
            adapter.add(String.valueOf(i));
        }
        if (!tdd) {
            adapter.add("66");
            adapter.add("71");
        }
        adapter.notifyDataSetChanged();
        mBand.setSelection(tdd ? DEFAULT_BAND_TDD : DEFAULT_BAND_FDD);

        adapter = (ArrayAdapter<CharSequence>) mTddConfig.getAdapter();
        adapter.clear();
        for (int i = 0; i <= configMax; i++) {
            adapter.add(String.valueOf(i));
        }
        adapter.notifyDataSetChanged();

        adapter = (ArrayAdapter<CharSequence>) mTddSpecial.getAdapter();
        adapter.clear();
        for (int i = 0; i <= specialMax; i++) {
            adapter.add(String.valueOf(i));
        }
        adapter.notifyDataSetChanged();

        setDefaultValue();
    }

    private boolean checkValues() {
        try {
            int value = Integer.parseInt(mVrbStart.getText().toString());
            if (value < VRB_START_MIN || value > VRB_START_MAX) {
                showToast("Invalid VRB Start.");
                return false;
            }
            value = Integer.parseInt(mVrbLength.getText().toString());
            if (value < VRB_LENGTH_MIN || value > VRB_LENGTH_MAX) {
                showToast("Invalid VRB Length.");
                return false;
            }
            value = Integer.parseInt(mPower.getText().toString());
            if (value < POWER_MIN || value > POWER_MAX) {
                showToast("Invalid Power Level.");
                return false;
            }
        } catch (NumberFormatException e) {
            showToast("Invalid Value.");
            return false;
        }
        return true;
    }

    private void saveState(String command) {
        SharedPreferences.Editor editor = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE).edit();
        editor.putInt(KEY_MODULATION_MODE, mSingleTone.isChecked() ? 0 : 1);
        if (mModemType == ModemCategory.MODEM_TD) {
            editor.putString(RfDesenseTxTest.KEY_LTE_TDD_ATCMD, command);
            editor.putInt(KEY_TDD_BAND, mBand.getSelectedItemPosition());
            editor.putInt(KEY_TDD_BAND_WIDTH, mBandWidth.getSelectedItemPosition());
            editor.putString(KEY_TDD_FREQ, mFreq.getText().toString());
            editor.putInt(KEY_TDD_CONFIG, mTddConfig.getSelectedItemPosition());
            editor.putInt(KEY_TDD_SPECIAL, mTddSpecial.getSelectedItemPosition());
            editor.putString(KEY_TDD_VRB_START, mVrbStart.getText().toString());
            editor.putString(KEY_TDD_VRB_LENGTH, mVrbLength.getText().toString());
            editor.putInt(KEY_TDD_MCS, mMcs.getSelectedItemPosition());
            editor.putString(KEY_TDD_POWER, mPower.getText().toString());
            editor.putString(KEY_TDD_ANT_MODE, mAntMode.isChecked() ? "1" : "0");
            editor.putString(KEY_TDD_ANT_STATUS, mAntStatus.getText().toString());
        } else {
            editor.putString(RfDesenseTxTest.KEY_LTE_FDD_ATCMD, command);
            editor.putInt(KEY_FDD_BAND, mBand.getSelectedItemPosition());
            editor.putInt(KEY_FDD_BAND_WIDTH, mBandWidth.getSelectedItemPosition());
            editor.putString(KEY_FDD_FREQ, mFreq.getText().toString());
            editor.putString(KEY_FDD_VRB_START, mVrbStart.getText().toString());
            editor.putString(KEY_FDD_VRB_LENGTH, mVrbLength.getText().toString());
            editor.putInt(KEY_FDD_MCS, mMcs.getSelectedItemPosition());
            editor.putString(KEY_FDD_POWER, mPower.getText().toString());
            editor.putString(KEY_FDD_ANT_MODE, mAntMode.isChecked() ? "1" : "0");
            editor.putString(KEY_FDD_ANT_STATUS, mAntStatus.getText().toString());
        }
        editor.apply();
    }

    private void restoreState() {
        SharedPreferences pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE);

        int checked = pref.getInt(KEY_MODULATION_MODE, DEFAULT_MODULATION_MODE);
        mSingleTone.setChecked(checked == 0);
        mModulationSignal.setChecked(checked == 1);

        if (mModemType == ModemCategory.MODEM_TD) {
            mCurrentBand = pref.getInt(KEY_TDD_BAND, DEFAULT_BAND_TDD);
            mBand.setSelection(pref.getInt(KEY_TDD_BAND, DEFAULT_BAND_TDD));
            mBandWidth.setSelection(pref.getInt(KEY_TDD_BAND_WIDTH, DEFAULT_BAND_WIDTH));
            mFreq.setText(pref.getString(KEY_TDD_FREQ, DEFAULT_TDD_FREQ));

            mTddConfig.setSelection(pref.getInt(KEY_TDD_CONFIG, DEFAULT_TDD_CONFIG));
            mTddSpecial.setSelection(pref.getInt(KEY_TDD_SPECIAL, DEFAULT_TDD_SPECIAL));

            mVrbStart.setText(pref.getString(KEY_TDD_VRB_START, DEFAULT_VRB_START));
            mVrbLength.setText(pref.getString(KEY_TDD_VRB_LENGTH, DEFAULT_VRB_LENGTH));
            mMcs.setSelection(pref.getInt(KEY_TDD_MCS, DEFAULT_MCS));
            mPower.setText(pref.getString(KEY_TDD_POWER, DEFAULT_POWER));

            mAntMode.setChecked(pref.getString(KEY_TDD_ANT_MODE, DEFAULT_ANT_MODE).equals("1"));
            mAntStatus.setText(pref.getString(KEY_TDD_ANT_STATUS, DEFAULT_ANT_STATUS));
        } else {
            mCurrentBand = pref.getInt(KEY_FDD_BAND, DEFAULT_BAND_FDD);
            mBand.setSelection(pref.getInt(KEY_FDD_BAND, DEFAULT_BAND_FDD));
            mBandWidth.setSelection(pref.getInt(KEY_FDD_BAND_WIDTH, DEFAULT_BAND_WIDTH));
            mFreq.setText(pref.getString(KEY_FDD_FREQ, DEFAULT_FDD_FREQ));

            mVrbStart.setText(pref.getString(KEY_FDD_VRB_START, DEFAULT_VRB_START));
            mVrbLength.setText(pref.getString(KEY_FDD_VRB_LENGTH, DEFAULT_VRB_LENGTH));
            mMcs.setSelection(pref.getInt(KEY_FDD_MCS, DEFAULT_MCS));
            mPower.setText(pref.getString(KEY_FDD_POWER, DEFAULT_POWER));
            mAntMode.setChecked(pref.getString(KEY_FDD_ANT_MODE, DEFAULT_ANT_MODE).equals("1"));
            mAntStatus.setText(pref.getString(KEY_FDD_ANT_STATUS, DEFAULT_ANT_STATUS));
        }

    }

    private void setDefaultValue() {
        mBandWidth.setSelection(DEFAULT_BAND_WIDTH);
        mFreq.setText(String.valueOf(getDefaultFreq()));
        mTddConfig.setSelection(DEFAULT_TDD_CONFIG);
        mTddSpecial.setSelection(DEFAULT_TDD_SPECIAL);
        mVrbStart.setText(DEFAULT_VRB_START);
        mVrbLength.setText(DEFAULT_VRB_LENGTH);
        mMcs.setSelection(DEFAULT_MCS);
        mPower.setText(DEFAULT_POWER);

        mAntMode.setChecked(DEFAULT_ANT_MODE.equals("1"));
        mAntStatus.setText(DEFAULT_ANT_STATUS);
    }

    private int getDefaultFreq() {
        int index = mBand.getSelectedItemPosition()
                + (mTdd.isChecked() ? TDD_BAND_MIN : FDD_BAND_MIN);

        if (mBand.getSelectedItemPosition() == 32) { //band 71
            index = 29;
        }

        String[] range = mFreqRangeArray[index - 1].split(",");
        try {
            int min = Integer.parseInt(range[0]);
            int max = Integer.parseInt(range[1]);
            return (min + max) / 2;
        } catch (NumberFormatException e) {
            throw new RuntimeException("Check the array resource");
        }
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

}
