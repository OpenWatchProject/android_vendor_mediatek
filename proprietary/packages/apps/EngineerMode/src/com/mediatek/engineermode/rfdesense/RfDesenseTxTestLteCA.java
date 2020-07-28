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
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TableLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

public class RfDesenseTxTestLteCA extends Activity implements Button.OnClickListener,
        CompoundButton.OnCheckedChangeListener, AdapterView.OnItemSelectedListener {
    public static final String TAG = "RfDesenseTxTestLteCA";
    public static final String KEY_STATE = "rf_state";
    public static final int STATE_NONE = 0;
    public static final int STATE_STARTED = 1;
    public static final int STATE_PAUSED = 2;
    private static final int START = 1;
    private static final int PAUSE = 2;
    private static final int UPDATE_BUTTON = 4;
    private static final int HINT = 1;
    private static final int SIM_CARD_INSERT = 2;

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
    private static final int POWER_MAX = 23;
    private static final int DEFAULT_DUPLEX = 1;
    private static final int DEFAULT_BAND = 2;
    private static final int DEFAULT_BAND_WIDTH = 0;
    private static final String DEFAULT_FREQ = "17475";
    private static final int DEFAULT_TDD_CONFIG = 0;
    private static final int DEFAULT_TDD_SPECIAL = 0;
    private static final String DEFAULT_VRB_START = "0";
    private static final String DEFAULT_VRB_LENGTH = "1";
    private static final int DEFAULT_MCS = 0;
    private static final String DEFAULT_POWER = "5";
    private static final int DEFAULT_MODE_SCELL1 = 0;
    private static final int DEFAULT_MODE_SCELL2 = 0;

    private static final String KEY_DUPLEX_PCELL = "duplex_pcell";
    private static final String KEY_BAND_PCELL = "band_pcell";
    private static final String KEY_BAND_WIDTH_PCELL = "bandwidth_pcell";
    private static final String KEY_FREQ_PCELL = "freq_pcell";
    private static final String KEY_TDD_CONFIG_PCELL = "tdd_config_pcell";
    private static final String KEY_TDD_SPECIAL_PCELL = "tdd_special_pcell";
    private static final String KEY_VRB_START_PCELL = "vrb_start_pcell";
    private static final String KEY_VRB_LENGTH_PCELL = "vrb_length_pcell";
    private static final String KEY_MCS_PCELL = "mcs_pcell";
    private static final String KEY_POWER_PCELL = "power_pcell";

    private static final String KEY_DUPLEX_SCELL1 = "duplex_scell1";
    private static final String KEY_BAND_SCELL1 = "band_scell1";
    private static final String KEY_BAND_WIDTH_SCELL1 = "bandwidth_scell1";
    private static final String KEY_FREQ_SCELL1 = "freq_scell1";
    private static final String KEY_TDD_CONFIG_SCELL1 = "tdd_config_scell1";
    private static final String KEY_TDD_SPECIAL_SCELL1 = "tdd_special_scell1";
    private static final String KEY_VRB_START_SCELL1 = "vrb_start_scell1";
    private static final String KEY_VRB_LENGTH_SCELL1 = "vrb_length_scell1";
    private static final String KEY_MCS_SCELL1 = "mcs_scell1";
    private static final String KEY_POWER_SCELL1 = "power_scell1";

    private static final String KEY_DUPLEX_SCELL2 = "duplex_scell2";
    private static final String KEY_BAND_SCELL2 = "band_scell2";
    private static final String KEY_BAND_WIDTH_SCELL2 = "bandwidth_scell2";
    private static final String KEY_FREQ_SCELL2 = "freq_scell2";
    private static final String KEY_TDD_CONFIG_SCELL2 = "tdd_config_scell2";
    private static final String KEY_TDD_SPECIAL_SCELL2 = "tdd_special_scell2";
    private static final String KEY_VRB_START_SCELL2 = "vrb_start_scell2";
    private static final String KEY_VRB_LENGTH_SCELL2 = "vrb_length_scell2";
    private static final String KEY_MCS_SCELL2 = "mcs_scell2";
    private static final String KEY_POWER_SCELL2 = "power_scell2";

    private static final String KEY_MODE_SCELL1 = "mode_scell1";
    private static final String KEY_MODE_SCELL2 = "mode_scell2";

    private String[] mFreqRangeArray;
    private TableLayout mTbCellConfig[] = new TableLayout[3];
    private CheckBox mRbCellSelect[] = new CheckBox[3];

    private RadioButton mFdd[] = new RadioButton[3];
    private RadioButton mTdd[] = new RadioButton[3];
    private Spinner mBand[] = new Spinner[3];
    private Spinner mBandWidth[] = new Spinner[3];
    private EditText mFreq[] = new EditText[3];
    private Spinner mTddConfig[] = new Spinner[3];
    private Spinner mTddSpecial[] = new Spinner[3];
    private EditText mVrbStart[] = new EditText[3];
    private EditText mVrbLength[] = new EditText[3];
    private Spinner mMcs[] = new Spinner[3];
    private EditText mPower[] = new EditText[3];
    private TextView mFreqRange[] = new TextView[3];
    private Button mButtonStart;
    private Button mButtonPause;

    private int mState = STATE_NONE;
    private int mCellCount = 1;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case START:
                    if (((AsyncResult) msg.obj).exception == null) {
                        EmUtils.showToast("START Command succeeded.");
                        Elog.d(TAG, "START Command succeeded.");
                    } else {
                        EmUtils.showToast("START Command failed.");
                        Elog.d(TAG, "START Command failed.");
                    }
                    mHandler.sendEmptyMessageDelayed(UPDATE_BUTTON, 1000);
                    break;
                case PAUSE:
                    if (((AsyncResult) msg.obj).exception == null) {
                        EmUtils.showToast("PAUSE Command succeeded.");
                        Elog.d(TAG, "PAUSE Command succeeded.");
                    } else {
                        EmUtils.showToast("PAUSE Command failed.");
                        Elog.d(TAG, "PAUSE Command failed.");
                    }
                    mHandler.sendEmptyMessageDelayed(UPDATE_BUTTON, 1000);
                    break;
                case UPDATE_BUTTON:
                    updateButtons();
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.rf_desense_tx_test_lte_ca);
        mFdd[0] = (RadioButton) findViewById(R.id.fdd_radio_button_pcell);
        mTdd[0] = (RadioButton) findViewById(R.id.tdd_radio_button_pcell);
        mFdd[0].setChecked(false);
        mTdd[0].setChecked(false);

        mFdd[1] = (RadioButton) findViewById(R.id.fdd_radio_button_scell1);
        mTdd[1] = (RadioButton) findViewById(R.id.tdd_radio_button_scell1);
        mFdd[1].setChecked(false);
        mTdd[1].setChecked(false);

        mFdd[2] = (RadioButton) findViewById(R.id.fdd_radio_button_scell2);
        mTdd[2] = (RadioButton) findViewById(R.id.tdd_radio_button_scell2);
        mFdd[2].setChecked(false);
        mTdd[2].setChecked(false);

        mRbCellSelect[0] = (CheckBox) findViewById(R.id.rb_rf_desense_pcell);
        mRbCellSelect[1] = (CheckBox) findViewById(R.id.rb_rf_desense_scell1);
        mRbCellSelect[2] = (CheckBox) findViewById(R.id.rb_rf_desense_scell2);
        mRbCellSelect[0].setChecked(true);
        mRbCellSelect[0].setEnabled(false);
        mRbCellSelect[1].setChecked(false);
        mRbCellSelect[1].setOnCheckedChangeListener(this);

        mRbCellSelect[2].setChecked(false);
        mRbCellSelect[2].setOnCheckedChangeListener(this);

        mTbCellConfig[0] = (TableLayout) findViewById(R.id.TableLayout_pcell);
        mTbCellConfig[1] = (TableLayout) findViewById(R.id.TableLayout_Scell1);
        mTbCellConfig[2] = (TableLayout) findViewById(R.id.TableLayout_Scell2);
        mTbCellConfig[1].setVisibility(View.GONE);
        mTbCellConfig[2].setVisibility(View.GONE);

        mBand[0] = (Spinner) findViewById(R.id.band_spinner_pcell);
        mBandWidth[0] = (Spinner) findViewById(R.id.bandwidth_spinner_pcell);
        mFreq[0] = (EditText) findViewById(R.id.freq_editor_pcell);
        mTddConfig[0] = (Spinner) findViewById(R.id.tdd_config_spinner_pcell);
        mTddSpecial[0] = (Spinner) findViewById(R.id.tdd_special_spinner_pcell);
        mVrbStart[0] = (EditText) findViewById(R.id.vrb_start_editor_pcell);
        mVrbLength[0] = (EditText) findViewById(R.id.vrb_length_editor_pcell);
        mMcs[0] = (Spinner) findViewById(R.id.mcs_spinner_pcell);
        mPower[0] = (EditText) findViewById(R.id.power_editor_pcell);
        mFreqRange[0] = (TextView) findViewById(R.id.freq_pcell);

        mBand[1] = (Spinner) findViewById(R.id.band_spinner_scell1);
        mBandWidth[1] = (Spinner) findViewById(R.id.bandwidth_spinner_scell1);
        mFreq[1] = (EditText) findViewById(R.id.freq_editor_scell1);
        mTddConfig[1] = (Spinner) findViewById(R.id.tdd_config_spinner_scell1);
        mTddSpecial[1] = (Spinner) findViewById(R.id.tdd_special_spinner_scell1);
        mVrbStart[1] = (EditText) findViewById(R.id.vrb_start_editor_scell1);
        mVrbLength[1] = (EditText) findViewById(R.id.vrb_length_editor_scell1);
        mMcs[1] = (Spinner) findViewById(R.id.mcs_spinner_scell1);
        mPower[1] = (EditText) findViewById(R.id.power_editor_scell1);
        mFreqRange[1] = (TextView) findViewById(R.id.freq_scell1);

        mBand[2] = (Spinner) findViewById(R.id.band_spinner_scell2);
        mBandWidth[2] = (Spinner) findViewById(R.id.bandwidth_spinner_scell2);
        mFreq[2] = (EditText) findViewById(R.id.freq_editor_scell2);
        mTddConfig[2] = (Spinner) findViewById(R.id.tdd_config_spinner_scell2);
        mTddSpecial[2] = (Spinner) findViewById(R.id.tdd_special_spinner_scell2);
        mVrbStart[2] = (EditText) findViewById(R.id.vrb_start_editor_scell2);
        mVrbLength[2] = (EditText) findViewById(R.id.vrb_length_editor_scell2);
        mMcs[2] = (Spinner) findViewById(R.id.mcs_spinner_scell2);
        mPower[2] = (EditText) findViewById(R.id.power_editor_scell2);
        mFreqRange[2] = (TextView) findViewById(R.id.freq_scell2);

        mButtonStart = (Button) findViewById(R.id.button_start);
        mButtonPause = (Button) findViewById(R.id.button_pause);

        ArrayAdapter[] adapter = new ArrayAdapter[3];
        for (int i = 0; i < 3; i++) {
            adapter[i] = new ArrayAdapter(this, android.R.layout
                    .simple_spinner_item);
            adapter[i].setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mBand[i].setAdapter(adapter[i]);

            adapter[i] = ArrayAdapter.createFromResource(this, R.array
                    .rf_desense_tx_test_lte_bandwidth, android.R.layout.simple_spinner_item);
            adapter[i].setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mBandWidth[i].setAdapter(adapter[i]);

            adapter[i] = ArrayAdapter.createFromResource(this, R.array.rf_desense_tx_test_lte_mcs,
                    android.R.layout.simple_spinner_item);
            adapter[i].setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mMcs[i].setAdapter(adapter[i]);

            adapter[i] = new ArrayAdapter(this, android.R.layout
                    .simple_spinner_item);
            adapter[i].setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mTddConfig[i].setAdapter(adapter[i]);

            adapter[i] = new ArrayAdapter(this, android.R.layout
                    .simple_spinner_item);
            adapter[i].setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mTddSpecial[i].setAdapter(adapter[i]);

            mFdd[i].setOnCheckedChangeListener(this);
            mTdd[i].setOnCheckedChangeListener(this);
            mBand[i].setOnItemSelectedListener(this);
        }
        mButtonStart.setOnClickListener(this);
        mButtonPause.setOnClickListener(this);
        mFreqRangeArray = getResources().getStringArray(R.array.rf_desense_tx_test_lte_freq_range);
        restoreState();
        disableAllButtons();

        if (!EmUtils.ifAirplaneModeEnabled()) {
            Elog.d(TAG, "it is in AirplaneMode");
            EmUtils.setAirplaneModeEnabled(true);
            mState = STATE_NONE;
            updateButtons();
        }
    }

    @Override
    protected void onDestroy() {
        saveState();
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Elog.d(TAG, "onResume");
        if (ModemCategory.isSimReady(-1)) {
            Elog.d(TAG, "some card insert");
            showDialog(SIM_CARD_INSERT);
        }

    }

    @Override
    public void onBackPressed() {
        if (mState == STATE_STARTED) {
            showDialog(HINT);
        } else {
            EmUtils.setAirplaneModeEnabled(false);
            super.onBackPressed();
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton v, boolean checked) {
        switch (v.getId()) {
            case R.id.rb_rf_desense_scell1:
                if (checked) {
                    mRbCellSelect[2].setEnabled(true);
                    mTbCellConfig[1].setVisibility(View.VISIBLE);
                    mCellCount = 2;
                } else {
                    mRbCellSelect[2].setEnabled(false);
                    mRbCellSelect[2].setChecked(false);
                    mTbCellConfig[1].setVisibility(View.GONE);
                    mTbCellConfig[2].setVisibility(View.GONE);
                    mCellCount = 1;
                }
                break;
            case R.id.rb_rf_desense_scell2:
                if (checked) {
                    mTbCellConfig[2].setVisibility(View.VISIBLE);
                    mCellCount = 3;
                } else {
                    mTbCellConfig[2].setVisibility(View.GONE);
                    mCellCount = 2;
                }
                break;
            case R.id.fdd_radio_button_pcell:
            case R.id.tdd_radio_button_pcell:
                onDuplexChange(0);
                break;
            case R.id.fdd_radio_button_scell1:
            case R.id.tdd_radio_button_scell1:
                onDuplexChange(1);
                break;
            case R.id.fdd_radio_button_scell2:
            case R.id.tdd_radio_button_scell2:
                onDuplexChange(2);
                break;
            default:
                break;
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.button_start:
                if (!checkValues()) {
                    break;
                }
                String command = "";

                for (int i = 0; i < mCellCount; i++) {
                    command = "AT+EGMC=1,\"lte_tx\",1,1," + mCellCount + "," + (i + 1) + ","
                            + (mTdd[i].isChecked() ? DUPLEX_TDD : DUPLEX_FDD) + ","
                            + (mBand[i].getSelectedItemPosition() + (mTdd[i].isChecked() ?
                            TDD_BAND_MIN : FDD_BAND_MIN)) + ","
                            + mBandWidth[i].getSelectedItemPosition() + ","
                            + mFreq[i].getText().toString() + ","
                            + mTddConfig[i].getSelectedItemPosition() + ","
                            + mTddSpecial[i].getSelectedItemPosition() + ","
                            + mVrbStart[i].getText().toString() + ","
                            + mVrbLength[i].getText().toString() + ","
                            + mMcs[i].getSelectedItemPosition() + ","
                            + mPower[i].getText().toString();
                    sendAtCommand(command, START);
                }
                disableAllButtons();
                mState = STATE_STARTED;
                break;
            case R.id.button_pause:
                sendAtCommand("AT+EGMC=1,\"lte_tx\",1,0", PAUSE);
                disableAllButtons();
                mState = STATE_PAUSED;
                break;
            default:
                break;
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case HINT:
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                return builder.setTitle("Hint")
                        .setMessage("Please pause the test first!")
                        .setPositiveButton("Confirm", null)
                        .create();
            case SIM_CARD_INSERT:
                AlertDialog.Builder builder2 = new AlertDialog.Builder(this);
                return builder2.setTitle("Notice")
                        .setMessage("Please pull out the sim card before test")
                        .setPositiveButton("Confirm", null)
                        .create();
            default:
                break;
        }
        return super.onCreateDialog(id);
    }

    private void onDuplexChange(int cell_index) {
        boolean tdd = mTdd[cell_index].isChecked();
        int bandMin = tdd ? TDD_BAND_MIN : FDD_BAND_MIN;
        int bandMax = tdd ? TDD_BAND_MAX : FDD_BAND_MAX;
        int configMax = tdd ? TDD_CONFIG_MAX : 0;
        int specialMax = tdd ? TDD_SPECIAL_MAX : 0;
        mTddConfig[cell_index].setEnabled(tdd);
        mTddSpecial[cell_index].setEnabled(tdd);
        Elog.i(TAG, "tdd = " + tdd);

        ArrayAdapter<CharSequence> adapter = (ArrayAdapter<CharSequence>) mBand[cell_index]
                .getAdapter();
        adapter.clear();
        for (int i = bandMin; i <= bandMax; i++) {
            adapter.add(String.valueOf(i));
        }
        adapter.notifyDataSetChanged();
        mBand[cell_index].setSelection(tdd ? 0 : DEFAULT_BAND);

        adapter = (ArrayAdapter<CharSequence>) mTddConfig[cell_index].getAdapter();
        adapter.clear();
        for (int i = 0; i <= configMax; i++) {
            adapter.add(String.valueOf(i));
        }
        adapter.notifyDataSetChanged();

        adapter = (ArrayAdapter<CharSequence>) mTddSpecial[cell_index].getAdapter();
        adapter.clear();
        for (int i = 0; i <= specialMax; i++) {
            adapter.add(String.valueOf(i));
        }
        adapter.notifyDataSetChanged();

        setDefaultValue(cell_index);
    }

    private boolean checkValues() {
        try {
            int value = Integer.parseInt(mVrbStart[0].getText().toString());
            if (value < VRB_START_MIN || value > VRB_START_MAX) {
                EmUtils.showToast("Invalid VRB Start.");
                return false;
            }
            value = Integer.parseInt(mVrbLength[0].getText().toString());
            if (value < VRB_LENGTH_MIN || value > VRB_LENGTH_MAX) {
                EmUtils.showToast("Invalid VRB Length.");
                return false;
            }
            value = Integer.parseInt(mPower[0].getText().toString());
            if (value < POWER_MIN || value > POWER_MAX) {
                EmUtils.showToast("Invalid Power Level.");
                return false;
            }
        } catch (NumberFormatException e) {
            EmUtils.showToast("Invalid Value.");
            return false;
        }
        return true;
    }

    private void saveState() {
        SharedPreferences.Editor editor = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE).edit();

        editor.putInt(KEY_DUPLEX_PCELL, mTdd[0].isChecked() ? DUPLEX_TDD : DUPLEX_FDD);
        editor.putInt(KEY_BAND_PCELL, mBand[0].getSelectedItemPosition());
        editor.putInt(KEY_BAND_WIDTH_PCELL, mBandWidth[0].getSelectedItemPosition());
        editor.putString(KEY_FREQ_PCELL, mFreq[0].getText().toString());
        editor.putInt(KEY_TDD_CONFIG_PCELL, mTddConfig[0].getSelectedItemPosition());
        editor.putInt(KEY_TDD_SPECIAL_PCELL, mTddSpecial[0].getSelectedItemPosition());
        editor.putString(KEY_VRB_START_PCELL, mVrbStart[0].getText().toString());
        editor.putString(KEY_VRB_LENGTH_PCELL, mVrbLength[0].getText().toString());
        editor.putInt(KEY_MCS_PCELL, mMcs[0].getSelectedItemPosition());
        editor.putString(KEY_POWER_PCELL, mPower[0].getText().toString());

        editor.putInt(KEY_DUPLEX_SCELL1, mTdd[1].isChecked() ? DUPLEX_TDD : DUPLEX_FDD);
        editor.putInt(KEY_BAND_SCELL1, mBand[1].getSelectedItemPosition());
        editor.putInt(KEY_BAND_WIDTH_SCELL1, mBandWidth[1].getSelectedItemPosition());
        editor.putString(KEY_FREQ_SCELL1, mFreq[1].getText().toString());
        editor.putInt(KEY_TDD_CONFIG_SCELL1, mTddConfig[1].getSelectedItemPosition());
        editor.putInt(KEY_TDD_SPECIAL_SCELL1, mTddSpecial[1].getSelectedItemPosition());
        editor.putString(KEY_VRB_START_SCELL1, mVrbStart[1].getText().toString());
        editor.putString(KEY_VRB_LENGTH_SCELL1, mVrbLength[1].getText().toString());
        editor.putInt(KEY_MCS_SCELL1, mMcs[1].getSelectedItemPosition());
        editor.putString(KEY_POWER_SCELL1, mPower[1].getText().toString());

        editor.putInt(KEY_DUPLEX_SCELL2, mTdd[2].isChecked() ? DUPLEX_TDD : DUPLEX_FDD);
        editor.putInt(KEY_BAND_SCELL2, mBand[2].getSelectedItemPosition());
        editor.putInt(KEY_BAND_WIDTH_SCELL2, mBandWidth[2].getSelectedItemPosition());
        editor.putString(KEY_FREQ_SCELL2, mFreq[2].getText().toString());
        editor.putInt(KEY_TDD_CONFIG_SCELL2, mTddConfig[2].getSelectedItemPosition());
        editor.putInt(KEY_TDD_SPECIAL_SCELL2, mTddSpecial[2].getSelectedItemPosition());
        editor.putString(KEY_VRB_START_SCELL2, mVrbStart[2].getText().toString());
        editor.putString(KEY_VRB_LENGTH_SCELL2, mVrbLength[2].getText().toString());
        editor.putInt(KEY_MCS_SCELL2, mMcs[2].getSelectedItemPosition());
        editor.putString(KEY_POWER_SCELL2, mPower[2].getText().toString());

        editor.putInt(KEY_MODE_SCELL1, mRbCellSelect[1].isChecked() ? 1 : 0);
        editor.putInt(KEY_MODE_SCELL2, mRbCellSelect[2].isChecked() ? 1 : 0);


        editor.putInt(KEY_STATE, mState);

        editor.commit();
    }

    private void restoreState() {
        SharedPreferences pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE);
        int duplex = pref.getInt(KEY_DUPLEX_PCELL, DEFAULT_DUPLEX);
        mFdd[0].setChecked(duplex == DUPLEX_FDD);
        mTdd[0].setChecked(duplex == DUPLEX_TDD);
        mBand[0].setSelection(pref.getInt(KEY_BAND_PCELL, DEFAULT_BAND));
        mBandWidth[0].setSelection(pref.getInt(KEY_BAND_WIDTH_PCELL, DEFAULT_BAND_WIDTH));
        mFreq[0].setText(pref.getString(KEY_FREQ_PCELL, DEFAULT_FREQ));
        mTddConfig[0].setSelection(pref.getInt(KEY_TDD_CONFIG_PCELL, DEFAULT_TDD_CONFIG));
        mTddSpecial[0].setSelection(pref.getInt(KEY_TDD_SPECIAL_PCELL, DEFAULT_TDD_SPECIAL));
        mVrbStart[0].setText(pref.getString(KEY_VRB_START_PCELL, DEFAULT_VRB_START));
        mVrbLength[0].setText(pref.getString(KEY_VRB_LENGTH_PCELL, DEFAULT_VRB_LENGTH));
        mMcs[0].setSelection(pref.getInt(KEY_MCS_PCELL, DEFAULT_MCS));
        mPower[0].setText(pref.getString(KEY_POWER_PCELL, DEFAULT_POWER));

        duplex = pref.getInt(KEY_DUPLEX_SCELL1, DEFAULT_DUPLEX);
        mFdd[1].setChecked(duplex == DUPLEX_FDD);
        mTdd[1].setChecked(duplex == DUPLEX_TDD);
        mBand[1].setSelection(pref.getInt(KEY_BAND_SCELL1, DEFAULT_BAND));
        mBandWidth[1].setSelection(pref.getInt(KEY_BAND_WIDTH_SCELL1, DEFAULT_BAND_WIDTH));
        mFreq[1].setText(pref.getString(KEY_FREQ_SCELL1, DEFAULT_FREQ));
        mTddConfig[1].setSelection(pref.getInt(KEY_TDD_CONFIG_SCELL1, DEFAULT_TDD_CONFIG));
        mTddSpecial[1].setSelection(pref.getInt(KEY_TDD_SPECIAL_SCELL1, DEFAULT_TDD_SPECIAL));
        mVrbStart[1].setText(pref.getString(KEY_VRB_START_SCELL1, DEFAULT_VRB_START));
        mVrbLength[1].setText(pref.getString(KEY_VRB_LENGTH_SCELL1, DEFAULT_VRB_LENGTH));
        mMcs[1].setSelection(pref.getInt(KEY_MCS_SCELL1, DEFAULT_MCS));
        mPower[1].setText(pref.getString(KEY_POWER_SCELL1, DEFAULT_POWER));

        duplex = pref.getInt(KEY_DUPLEX_SCELL2, DEFAULT_DUPLEX);
        mFdd[2].setChecked(duplex == DUPLEX_FDD);
        mTdd[2].setChecked(duplex == DUPLEX_TDD);
        mBand[2].setSelection(pref.getInt(KEY_BAND_SCELL2, DEFAULT_BAND));
        mBandWidth[2].setSelection(pref.getInt(KEY_BAND_WIDTH_SCELL2, DEFAULT_BAND_WIDTH));
        mFreq[2].setText(pref.getString(KEY_FREQ_SCELL2, DEFAULT_FREQ));
        mTddConfig[2].setSelection(pref.getInt(KEY_TDD_CONFIG_SCELL2, DEFAULT_TDD_CONFIG));
        mTddSpecial[2].setSelection(pref.getInt(KEY_TDD_SPECIAL_SCELL2, DEFAULT_TDD_SPECIAL));
        mVrbStart[2].setText(pref.getString(KEY_VRB_START_SCELL2, DEFAULT_VRB_START));
        mVrbLength[2].setText(pref.getString(KEY_VRB_LENGTH_SCELL2, DEFAULT_VRB_LENGTH));
        mMcs[2].setSelection(pref.getInt(KEY_MCS_SCELL2, DEFAULT_MCS));
        mPower[2].setText(pref.getString(KEY_POWER_SCELL2, DEFAULT_POWER));

        mRbCellSelect[1].setChecked(pref.getInt(KEY_MODE_SCELL1, DEFAULT_MODE_SCELL1) == 1);
        mRbCellSelect[2].setChecked(pref.getInt(KEY_MODE_SCELL2, DEFAULT_MODE_SCELL2) == 1);
        mState = pref.getInt(KEY_STATE, STATE_NONE);
        updateButtons();
    }

    private void setDefaultValue(int cell_index) {
        mBandWidth[cell_index].setSelection(DEFAULT_BAND_WIDTH);
        mFreq[cell_index].setText(String.valueOf(getDefaultFreq(cell_index)));
        mTddConfig[cell_index].setSelection(DEFAULT_TDD_CONFIG);
        mTddSpecial[cell_index].setSelection(DEFAULT_TDD_SPECIAL);
        mVrbStart[cell_index].setText(DEFAULT_VRB_START);
        mVrbLength[cell_index].setText(DEFAULT_VRB_LENGTH);
        mMcs[cell_index].setSelection(DEFAULT_MCS);
        mPower[cell_index].setText(DEFAULT_POWER);
    }

    private int getDefaultFreq(int cell_index) {
        int band = mBand[cell_index].getSelectedItemPosition()
                + (mTdd[cell_index].isChecked() ? TDD_BAND_MIN : FDD_BAND_MIN);
        String[] range = mFreqRangeArray[band - 1].split(",");
        try {
            int min = Integer.parseInt(range[0]);
            int max = Integer.parseInt(range[1]);
            return (min + max) / 2;
        } catch (NumberFormatException e) {
            throw new RuntimeException("Check the array resource");
        }
    }

    private void disableAllButtons() {
        mButtonStart.setEnabled(false);
        mButtonPause.setEnabled(false);
    }

    private void updateButtons() {
        mButtonStart.setEnabled(mState == STATE_NONE || mState == STATE_PAUSED);
        mButtonPause.setEnabled(mState == STATE_STARTED);
    }

    private void sendAtCommand(String str, int what) {
        String cmd[] = new String[]{str, ""};
        Elog.i(TAG, "send: " + cmd[0]);
        EmUtils.invokeOemRilRequestStringsEm(cmd, mHandler.obtainMessage(what));
    }


    public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
        if (view.getId() == R.id.band_spinner_pcell) {
            setDefaultValue(0);
        } else if (view.getId() == R.id.band_spinner_scell1) {
            setDefaultValue(1);
        } else if (view.getId() == R.id.band_spinner_scell2) {
            setDefaultValue(2);
        }
    }

    public void onNothingSelected(AdapterView<?> adapterView) {

    }
}
