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
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;


public class RfDesenseTxTestNR extends Activity implements Button.OnClickListener,
        CompoundButton.OnCheckedChangeListener {
    public static final String TAG = "RfDesense/TxTestNr";
    public static final int DEFAULT_TX_MODE = 0;
    public static final int DEFAULT_BAND_NR = 0;
    public static final int DEFAULT_BAND_WIDTH = 0;
    public static final int DEFAULT_SCS_CONFIG = 0;
    public static final int DEFAULT_MCS = 0;
    public static final String DEFAULT_VRB_START = "0";
    public static final String DEFAULT_VRB_LENGTH = "1";
    public static final String DEFAULT_NR_FREQ = "17475";
    public static final String DEFAULT_ANT_MODE = "0";
    public static final String DEFAULT_ANT_STATUS = "0";
    public static final String DEFAULT_POWER = "23";
    public static final String DEFAULT_TDD_SLOT_CONFIG = "1";
    public static final String KEY_NR_BAND = "band_nr";
    public static final String KEY_NR_BAND_WIDTH = "bandwidth_nr";
    public static final String KEY_NR_FREQ = "freq_nr";
    public static final String KEY_NR_VRB_START = "vrb_start_nr";
    public static final String KEY_NR_VRB_LENGTH = "vrb_length_nr";
    public static final String KEY_NR_MCS = "mcs_nr";
    public static final String KEY_NR_SCS = "scs_nr";
    public static final String KEY_NR_ANT_MODE = "ant_mode_nr";
    public static final String KEY_NR_ANT_STATUS = "ant_status_nr";
    public static final String KEY_NR_POWER = "power_nr";
    public static final String KEY_TX_MODE = "tx_mode";
    public static final String KEY_TDD_SLOT_CONFIG = "tdd_slot_config";
    public static final int[] mBandMapping = {1, 3, 7, 8, 20, 28, 38, 41, 77, 78, 79};
    public static final int[] mBandWidthMapping = {5000, 10000, 15000, 20000, 25000, 30000,
            35000, 40000, 45000, 50000, 55000, 60000, 65000, 70000, 75000, 80000, 85000, 90000,
            95000, 100000};

    private static final int VRB_START_MIN = 0;
    private static final int VRB_START_MAX = 272;
    private static final int VRB_LENGTH_MIN = 0;
    private static final int VRB_LENGTH_MAX = 273;
    private static final int POWER_MIN = -50;
    private static final int POWER_MAX_PUSCH = 23;
    private static final int POWER_MAX_TONE = 26;
    private static final int TDD_SLOT_CONFIG_MIN = 1;
    private static final int TDD_SLOT_CONFIG_MAX = 44;
    private static int mPowerMax = POWER_MAX_PUSCH;
    private RadioButton mToneTX;
    private RadioButton mPuschTX;
    private Spinner mBand;
    private Spinner mBandWidth;
    private Spinner mScsConfig;
    private Spinner mMcs;
    private EditText mFreq;
    private EditText mVrbStart;
    private EditText mVrbLength;
    private EditText mPower;
    private TextView mPowerView;
    private EditText mTddSlotConfig;
    private EditText mAntStatus;
    private CheckBox mAntMode;
    private Button mButtonSet;
    private int mCurrentBandIndex = 1;
    private String[] mFreqRangeArray;
    private String atcmd = "";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.rf_desense_tx_test_nr);

        mToneTX = (RadioButton) findViewById(R.id.tone_radio_button);
        mPuschTX = (RadioButton) findViewById(R.id.pusch_radio_button);

        mBand = (Spinner) findViewById(R.id.band_spinner);
        mBandWidth = (Spinner) findViewById(R.id.bandwidth_spinner);
        mFreq = (EditText) findViewById(R.id.freq_editor);

        mScsConfig = (Spinner) findViewById(R.id.scs_config_spinner);
        mTddSlotConfig = (EditText) findViewById(R.id.tddSlotConfig_editor);

        mVrbStart = (EditText) findViewById(R.id.vrb_start_editor);
        mVrbLength = (EditText) findViewById(R.id.vrb_length_editor);
        mMcs = (Spinner) findViewById(R.id.mcs_spinner);
        mPower = (EditText) findViewById(R.id.power_editor);
        mPowerView = (TextView) findViewById(R.id.power_view);
        mAntMode = (CheckBox) findViewById(R.id.rf_ant_mode);
        mAntStatus = (EditText) findViewById(R.id.rf_ant_status);
        mAntMode.setVisibility(View.GONE);
        mAntStatus.setVisibility(View.GONE);

        mButtonSet = (Button) findViewById(R.id.button_set);

        mToneTX.setOnCheckedChangeListener(this);
        mPuschTX.setOnCheckedChangeListener(this);
        mButtonSet.setOnClickListener(this);

        ArrayAdapter<CharSequence> adapter =
                new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item);

        adapter = ArrayAdapter.createFromResource(this, R.array.rf_desense_tx_test_nr_freq_range,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBand.setAdapter(adapter);

        adapter = ArrayAdapter.createFromResource(this,
                R.array.rf_desense_tx_test_nr_bandwidth,
                android.R.layout.simple_spinner_item);
        mBandWidth.setAdapter(adapter);

        adapter = ArrayAdapter.createFromResource(this, R.array.rf_desense_tx_test_nr_mcs,
                android.R.layout.simple_spinner_item);
        mMcs.setAdapter(adapter);

        adapter = ArrayAdapter.createFromResource(this, R.array.rf_desense_tx_test_nr_scs,
                android.R.layout.simple_spinner_item);
        mScsConfig.setAdapter(adapter);

        mFreqRangeArray = getResources().getStringArray(
                R.array.rf_desense_tx_test_nr_freq_range);

        mBand.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (arg2 != mCurrentBandIndex) {
                    mCurrentBandIndex = arg2;
                    mFreq.setText(String.valueOf(getDefaultFreq()));
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });
        restoreState();
        onTxModeChange();
    }

    private Long getDefaultFreq() {
        int index = mBand.getSelectedItemPosition();
        String[] range = mFreqRangeArray[index].split(",");
        try {
            Long min = Long.parseLong(range[1]);
            Long max = Long.parseLong(range[2]);
            return (min + max) / 2;
        } catch (NumberFormatException e) {
            throw new RuntimeException("Check the array resource");
        }
    }

    private void onTxModeChange() {
        if (mToneTX.isChecked()) {
            mBandWidth.setEnabled(false);
            mScsConfig.setEnabled(false);
            mMcs.setEnabled(false);
            mVrbStart.setEnabled(false);
            mVrbLength.setEnabled(false);
            mTddSlotConfig.setEnabled(false);
            mPowerMax = POWER_MAX_TONE;
            mPowerView.setText(R.string.rf_desense_nr_power_tone);
        } else {
            mBandWidth.setEnabled(true);
            mScsConfig.setEnabled(true);
            mMcs.setEnabled(true);
            mVrbStart.setEnabled(true);
            mVrbLength.setEnabled(true);
            mTddSlotConfig.setEnabled(true);
            mPowerMax = POWER_MAX_PUSCH;
            mPowerView.setText(R.string.rf_desense_nr_power_pusch);
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton v, boolean checked) {
        switch (v.getId()) {
            case R.id.tone_radio_button:
            case R.id.pusch_radio_button:
                onTxModeChange();
                break;
            default:
                break;
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.button_set:
                if (!checkValues()) {
                    break;
                }
                int txMode = mToneTX.isChecked() ? 0 : 1;
                int band = mBandMapping[mBand.getSelectedItemPosition()];
                int bandWidth = mBandWidthMapping[mBandWidth.getSelectedItemPosition()];
                String freq = mFreq.getText().toString();

                int ScsConfig = mScsConfig.getSelectedItemPosition();
                String vrbStart = mVrbStart.getText().toString();
                String vrbLength = mVrbLength.getText().toString();
                int mcs = mMcs.getSelectedItemPosition();
                String power = mPower.getText().toString();
                String tddSlotConfig = mTddSlotConfig.getText().toString();
                String antStatus = mAntStatus.getText().toString();
                int antMode = mAntMode.isChecked() ? 1 : 0;
                if (txMode == 0) {
                    atcmd = "AT+EGMC=1,\"NrForcedTx\",2,";
                    atcmd += band + "," + freq + "," + power;
                } else {
                    atcmd = "AT+EGMC=1,\"NrForcedTx\",1,";
                    atcmd += band + "," + bandWidth + "," + freq + "," + "," + ScsConfig + "," +
                            vrbStart + "," + vrbLength + "," +
                            mcs + "," + power + "," + tddSlotConfig;
                }
                saveState(atcmd);
                EmUtils.showToast("Set param suecceed");
                Elog.d(TAG, "command = " + atcmd);
                break;
            default:
                break;
        }
    }

    private boolean checkValues() {
        try {
            int value = Integer.parseInt(mVrbStart.getText().toString());
            if (value < VRB_START_MIN || value > VRB_START_MAX) {
                EmUtils.showToast("Invalid VRB Start.");
                return false;
            }
            value = Integer.parseInt(mVrbLength.getText().toString());
            if (value < VRB_LENGTH_MIN || value > VRB_LENGTH_MAX) {
                EmUtils.showToast("Invalid VRB Length.");
                return false;
            }
            value = Integer.parseInt(mPower.getText().toString());
            if (value < POWER_MIN || value > mPowerMax) {
                EmUtils.showToast("Invalid Power Level.");
                return false;
            }
            value = Integer.parseInt(mTddSlotConfig.getText().toString());
            if (value < TDD_SLOT_CONFIG_MIN || value > TDD_SLOT_CONFIG_MAX) {
                EmUtils.showToast("Invalid tdd Slot Config.");
                return false;
            }
        } catch (NumberFormatException e) {
            EmUtils.showToast("Invalid Value.");
            return false;
        }
        return true;
    }

    private void saveState(String command) {
        SharedPreferences.Editor editor = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE).edit();
        editor.putInt(KEY_TX_MODE, mToneTX.isChecked() ? 0 : 1);
        editor.putString(RfDesenseTxTest.KEY_NR_ATCMD, command);
        editor.putInt(KEY_NR_BAND, mBand.getSelectedItemPosition());
        editor.putInt(KEY_NR_BAND_WIDTH, mBandWidth.getSelectedItemPosition());
        editor.putString(KEY_NR_FREQ, mFreq.getText().toString());
        editor.putString(KEY_NR_VRB_START, mVrbStart.getText().toString());
        editor.putString(KEY_NR_VRB_LENGTH, mVrbLength.getText().toString());
        editor.putInt(KEY_NR_MCS, mMcs.getSelectedItemPosition());
        editor.putInt(KEY_NR_SCS, mScsConfig.getSelectedItemPosition());
        editor.putString(KEY_NR_POWER, mPower.getText().toString());
        editor.putString(KEY_TDD_SLOT_CONFIG, mTddSlotConfig.getText().toString());
        editor.putString(KEY_NR_ANT_MODE, mAntMode.isChecked() ? "1" : "0");
        editor.putString(KEY_NR_ANT_STATUS, mAntStatus.getText().toString());
        editor.apply();
    }

    private void restoreState() {
        SharedPreferences pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE);
        int checked = pref.getInt(KEY_TX_MODE, DEFAULT_TX_MODE);
        mToneTX.setChecked(checked == 0);
        mPuschTX.setChecked(checked == 1);
        mCurrentBandIndex = pref.getInt(KEY_NR_BAND, DEFAULT_BAND_NR);
        mBand.setSelection(pref.getInt(KEY_NR_BAND, DEFAULT_BAND_NR));
        mBandWidth.setSelection(pref.getInt(KEY_NR_BAND_WIDTH, DEFAULT_BAND_WIDTH));
        mFreq.setText(pref.getString(KEY_NR_FREQ, DEFAULT_NR_FREQ));

        mVrbStart.setText(pref.getString(KEY_NR_VRB_START, DEFAULT_VRB_START));
        mVrbLength.setText(pref.getString(KEY_NR_VRB_LENGTH, DEFAULT_VRB_LENGTH));
        mMcs.setSelection(pref.getInt(KEY_NR_MCS, DEFAULT_MCS));
        mScsConfig.setSelection(pref.getInt(KEY_NR_SCS, DEFAULT_SCS_CONFIG));
        mPower.setText(pref.getString(KEY_NR_POWER, DEFAULT_POWER));
        mTddSlotConfig.setText(pref.getString(KEY_TDD_SLOT_CONFIG, DEFAULT_TDD_SLOT_CONFIG));
        mAntMode.setChecked(pref.getString(KEY_NR_ANT_MODE, DEFAULT_ANT_MODE).equals("1"));
        mAntStatus.setText(pref.getString(KEY_NR_ANT_STATUS, DEFAULT_ANT_STATUS));
    }

}
