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
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.R;

/**
 * RfDesenseTxTestCdma Activity.
 */
public class RfDesenseTxTestCdma extends Activity {
    public static final String TAG = "RfDesense/TxTestCdma";
    public static final String KEY_MODULATION = "Modulation_CDMA";

    public static final String KEY_1X_BAND = "Band_1x_CDMA";
    public static final String KEY_1X_CHANNEL = "Channel_1x_CDMA";
    public static final String KEY_1X_POWER = "Power_1x_CDMA";

    public static final String KEY_CDMA1X_ANT_MODE = "cdma1x_ant_mode";
    public static final String KEY_CDMA1x_ANT_STATUS = "cdma1x_ant_status";

    public static final String KEY_EVDO_BAND = "Band_evdo_CDMA";
    public static final String KEY_EVDO_CHANNEL = "Channel_evdo_CDMA";
    public static final String KEY_EVDO_POWER = "Power_evdo_CDMA";

    public static final String KEY_EVDO_ANT_MODE = "evdo_ant_mode";
    public static final String KEY_EVDO_ANT_STATUS = "evdo_ant_status";

    public static final int DEFAULT_BAND_VALUE = 0;
    public static final int DEFAULT_CHANNEL_VALUE = 384;
    public static final int DEFAULT_POWER_VALUE = 23;

    private static final int CHANNEL_DEFAULT = 0;
    private static final int CHANNEL_MIN = 1;
    private static final int CHANNEL_MAX = 2;
    private static final int CHANNEL_MIN2 = 3;
    private static final int CHANNEL_MAX2 = 4;
    private static final int POWER_DEFAULT = 5;
    private static final int POWER_MIN = 6;
    private static final int POWER_MAX = 7;
    private static int mModemType;
    protected Toast mToast = null;
    protected CheckBox mAntMode;
    protected EditText mAntStatus;
    private Spinner mBand;
    private RadioGroup mModulation;
    private EditText mChannel;
    private EditText mPower;
    private Button mButtonSet;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.rf_desense_tx_test_cdma);
        Intent intent = getIntent();
        mModemType = intent.getIntExtra("mModemType", RfDesenseTxTest.CDMD_MODE_EVDO);
        mBand = (Spinner) findViewById(R.id.band_spinner);
        mModulation = (RadioGroup) findViewById(R.id.modulation_radio_group);
        mChannel = (EditText) findViewById(R.id.channel_editor);
        mPower = (EditText) findViewById(R.id.power_editor);
        mAntMode = (CheckBox) findViewById(R.id.rf_ant_mode);
        mAntStatus = (EditText) findViewById(R.id.rf_ant_status);
        mButtonSet = (Button) findViewById(R.id.button_set);

        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.rf_desense_tx_test_cdma_band,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBand.setAdapter(adapter);

        final String[] bandValues =
                getResources().getStringArray(R.array.rf_desense_tx_test_cdma_band_values);

        Button.OnClickListener listener = new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                switch (v.getId()) {
                    case R.id.button_set:
                        if (!checkValues()) {
                            break;
                        }
                        String band = bandValues[mBand.getSelectedItemPosition()];
                        long modulation = mModulation.getCheckedRadioButtonId();
                        String channel = mChannel.getText().toString();
                        String power = mPower.getText().toString();
                        String command = "";
                        int tx_power = 0;
                        if ((power != null) && !("".equals(power))) {
                            tx_power = Integer.valueOf(power) + 60;
                        }

                        if (mModemType == RfDesenseTxTest.CDMD_MODE_1X) {
                            command = "AT+ECRFTX=1," + channel + "," + band + ","
                                    + tx_power
                                    + ","
                                    + (modulation == R.id.modulation_1x ? 0 : 1);
                        } else {
                            command = "AT+ERFTX=13,4," + channel + "," + band + ","
                                    + tx_power;
                        }

                        saveState(command);
                        showToast("Set param suecceed!");
                        break;
                    default:
                        break;
                }
            }
        };
        mButtonSet.setOnClickListener(listener);
        restoreState();
    }

    private void saveState(String command) {
        SharedPreferences.Editor pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE).edit();
        if (mModemType == RfDesenseTxTest.CDMD_MODE_1X) {
            pref.putInt(KEY_1X_BAND, mBand.getSelectedItemPosition());
            pref.putString(KEY_1X_CHANNEL, mChannel.getText().toString());
            pref.putString(KEY_1X_POWER, mPower.getText().toString());
            pref.putString(KEY_CDMA1X_ANT_MODE, mAntMode.isChecked() ? "1" : "0");
            pref.putString(KEY_CDMA1x_ANT_STATUS, mAntStatus.getText().toString());
            pref.putString(RfDesenseTxTest.KEY_CDMA_1X_ATCMD, command);
            String ant_str = "AT+ETXANT=1,4," + mAntStatus.getText();
            if( mAntMode.isChecked())
                pref.putString(RfDesenseTxTest.KEY_CDMA1X_ATCMD_ANT_SWITCH, ant_str);
            else{
                ant_str = "AT+ETXANT=0,4,0";
                pref.putString(RfDesenseTxTest.KEY_CDMA1X_ATCMD_ANT_SWITCH, ant_str);
            }
        } else {
            pref.putInt(KEY_EVDO_BAND, mBand.getSelectedItemPosition());
            pref.putString(KEY_EVDO_CHANNEL, mChannel.getText().toString());
            pref.putString(KEY_EVDO_POWER, mPower.getText().toString());
            pref.putString(RfDesenseTxTest.KEY_CDMA_EVDO_ATCMD, command);
            pref.putString(KEY_EVDO_ANT_MODE, mAntMode.isChecked() ? "1" : "0");
            pref.putString(KEY_EVDO_ANT_STATUS, mAntStatus.getText().toString());
            String ant_str = "AT+ETXANT=1,4," + mAntStatus.getText();
            if( mAntMode.isChecked())
                pref.putString(RfDesenseTxTest.KEY_EVDO_ATCMD_ANT_SWITCH, ant_str);
            else{
                ant_str = "AT+ETXANT=0,4,0";
                pref.putString(RfDesenseTxTest.KEY_EVDO_ATCMD_ANT_SWITCH,ant_str);
            }
        }
        pref.apply();
    }

    private void restoreState() {
        SharedPreferences pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE);
        if (mModemType == RfDesenseTxTest.CDMD_MODE_1X) {
            mModulation.check(R.id.modulation_1x);
            mBand.setSelection(pref.getInt(KEY_1X_BAND, DEFAULT_BAND_VALUE));
            mChannel.setText(pref.getString(KEY_1X_CHANNEL, DEFAULT_CHANNEL_VALUE + ""));
            mPower.setText(pref.getString(KEY_1X_POWER, DEFAULT_POWER_VALUE + ""));
            mAntMode.setChecked(pref.getString(KEY_CDMA1X_ANT_MODE, "0").equals("1"));
            mAntStatus.setText(pref.getString(KEY_CDMA1x_ANT_STATUS, "0"));
        } else {
            mModulation.check(R.id.modulation_evdo);
            mBand.setSelection(pref.getInt(KEY_EVDO_BAND, DEFAULT_BAND_VALUE));
            mChannel.setText(pref.getString(KEY_EVDO_CHANNEL, DEFAULT_CHANNEL_VALUE + ""));
            mPower.setText(pref.getString(KEY_EVDO_POWER, DEFAULT_POWER_VALUE + ""));
            mAntMode.setChecked(pref.getString(KEY_EVDO_ANT_MODE, "0").equals("1"));
            mAntStatus.setText(pref.getString(KEY_EVDO_ANT_STATUS, "0"));
        }

    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    protected boolean checkValues() {
        return true;
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_LONG);
        mToast.show();
    }
}
