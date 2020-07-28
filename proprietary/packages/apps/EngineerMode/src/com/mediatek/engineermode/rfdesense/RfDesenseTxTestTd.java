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

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

public class RfDesenseTxTestTd extends RfDesenseTxTestBase {
    public static final String TAG = "RfDesense/TxTestTd";

    public static final String KEY_TDD_BAND = "Band_TDD_3G";
    public static final String KEY_TDD_CHANNEL = "Channel_TDD_3G";
    public static final String KEY_TDD_POWER = "Power_TDD_3G";
    public static final String KEY_TDD_ANT_MODE = "Power_TDD_ant_mode";
    public static final String KEY_TDD_ANT_STATUS = "Power_TDD_ant_status";

    public static final String KEY_FDD_BAND = "Band_FDD_3G";
    public static final String KEY_FDD_CHANNEL = "Channel_FDD_3G";
    public static final String KEY_FDD_POWER = "Power_FDD_3G";
    public static final String KEY_FDD_ANT_MODE = "Power_FDD_ant_mode";
    public static final String KEY_FDD_ANT_STATUS = "Power_FDD_ant_status";

    private static int mModemType;
    private String[] mBandValues;

    private void saveState(String command) {
        SharedPreferences.Editor pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE).edit();

        if (mModemType == ModemCategory.MODEM_TD) {
            pref.putString(RfDesenseTxTest.KEY_TDSCDMA_ATCMD, command);
            pref.putInt(KEY_TDD_BAND, mBand.getSelectedItemPosition());
            pref.putString(KEY_TDD_CHANNEL, mChannel.getText());
            pref.putString(KEY_TDD_POWER, mPower.getText());
            pref.putString(KEY_TDD_ANT_MODE, mAntMode.isChecked() ? "1" : "0");
            pref.putString(KEY_TDD_ANT_STATUS, mAntStatus.getText().toString());
            String ant_str = "AT+ETXANT=1,2," + mAntStatus.getText();
            if( mAntMode.isChecked())
                pref.putString(RfDesenseTxTest.KEY_TDSCDMA_ATCMD_ANT_SWITCH, ant_str);
            else{
                ant_str = "AT+ETXANT=0,2,0";
                pref.putString(RfDesenseTxTest.KEY_TDSCDMA_ATCMD_ANT_SWITCH, ant_str);
            }
        } else {
            pref.putString(RfDesenseTxTest.KEY_WCDMA_ATCMD, command);
            pref.putInt(KEY_FDD_BAND, mBand.getSelectedItemPosition());
            pref.putString(KEY_FDD_CHANNEL, mChannel.getText());
            pref.putString(KEY_FDD_POWER, mPower.getText());
            pref.putString(KEY_FDD_ANT_MODE, mAntMode.isChecked() ? "1" : "0");
            pref.putString(KEY_FDD_ANT_STATUS, mAntStatus.getText().toString());
            String ant_str = "AT+ETXANT=1,2," + mAntStatus.getText();
            if( mAntMode.isChecked())
                pref.putString(RfDesenseTxTest.KEY_WCDMA_ATCMD_ANT_SWITCH, ant_str);
            else{
                ant_str = "AT+ETXANT=0,2,0";
                pref.putString(RfDesenseTxTest.KEY_WCDMA_ATCMD_ANT_SWITCH, ant_str);
            }
        }
        pref.apply();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        mModemType = intent.getIntExtra("mModemType", ModemCategory.MODEM_FDD);
        findViewById(R.id.afc).setVisibility(View.GONE);
        findViewById(R.id.tsc).setVisibility(View.GONE);
        findViewById(R.id.pattern).setVisibility(View.GONE);
        mAfc.editor.setVisibility(View.GONE);
        mTsc.editor.setVisibility(View.GONE);
        mPattern.setVisibility(View.GONE);

        TextView channelLabel = (TextView) findViewById(R.id.channel);
        channelLabel.setText(R.string.rf_desense_channel_3g);

        if (mModemType == ModemCategory.MODEM_TD) {
            mBandValues = getResources().getStringArray(
                    R.array.rf_desense_tx_test_td_band_values);
        } else {
            mBandValues = getResources().getStringArray(
                    R.array.rf_desense_tx_test_fd_band_values);
        }

        Button.OnClickListener listener = new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                switch (v.getId()) {
                    case R.id.button_set:
                        if (!checkValues()) {
                            break;
                        }

                        String band = mBandValues[mBand.getSelectedItemPosition()];
                        String channel = mChannel.getText();
                        String power = mPower.getText();
                        String command = "AT+ERFTX=0,0," + band + "," + channel + "," + power;
                        saveState(command);
                        showToast("Set param suecceed!");
                        break;

                    default:
                        break;
                }
            }
        };

        mButtonSet.setOnClickListener(listener);

        ArrayAdapter<CharSequence> adapter;
        if (mModemType == ModemCategory.MODEM_TD) {
            adapter = ArrayAdapter.createFromResource(this,
                    R.array.rf_desense_tx_test_td_band,
                    android.R.layout.simple_spinner_item);
        } else {
            adapter = ArrayAdapter.createFromResource(this,
                    R.array.rf_desense_tx_test_fd_band,
                    android.R.layout.simple_spinner_item);
        }
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBand.setAdapter(adapter);

        restoreState();
    }

    private void restoreState() {
        SharedPreferences pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE);

        if (mModemType == ModemCategory.MODEM_TD) {
            mCurrentBand = pref.getInt(KEY_TDD_BAND, 0);
            mBand.setSelection(mCurrentBand);

            updateLimits();
            mChannel.setText(pref.getString(KEY_TDD_CHANNEL, mChannel.defaultValue));
            mPower.setText(pref.getString(KEY_TDD_POWER, mPower.defaultValue));

            mAntMode.setChecked(pref.getString(KEY_TDD_ANT_MODE, RfDesenseTxTestLte.DEFAULT_ANT_MODE)
                    .equals("1"));
            mAntStatus.setText(pref.getString(KEY_TDD_ANT_STATUS, RfDesenseTxTestLte.DEFAULT_ANT_STATUS));


        } else {
            mCurrentBand = pref.getInt(KEY_FDD_BAND, 0);
            mBand.setSelection(mCurrentBand);
            updateLimits();
            mChannel.setText(pref.getString(KEY_FDD_CHANNEL, mChannel.defaultValue));
            mPower.setText(pref.getString(KEY_FDD_POWER, mPower.defaultValue));
            mAntMode.setChecked(pref.getString(KEY_FDD_ANT_MODE, RfDesenseTxTestLte.DEFAULT_ANT_MODE)
                    .equals("1"));
            mAntStatus.setText(pref.getString(KEY_FDD_ANT_STATUS, RfDesenseTxTestLte.DEFAULT_ANT_STATUS));
        }

    }

    @Override
    protected void updateLimits() {
        int band = mBand.getSelectedItemPosition();
        String[] limits;

        if (mModemType == ModemCategory.MODEM_TD) {
            limits = getResources().getStringArray(
                    R.array.rf_desense_tx_test_td_limits)[band].split(",");
        } else {
            limits = getResources().getStringArray(
                    R.array.rf_desense_tx_test_fd_limits)[band].split(",");
        }
        int min = 0;
        int max = 0;
        String defaults = "";
        try {
            min = Integer.valueOf(limits[CHANNEL_MIN]);
            max = Integer.valueOf(limits[CHANNEL_MAX]);
            defaults = (min + max) / 2 + "";
        } catch (Exception e) {
            e.getMessage();
        }

        mChannel.set(defaults, limits[CHANNEL_MIN], limits[CHANNEL_MAX],
                limits[CHANNEL_MIN2], limits[CHANNEL_MAX2]);
        mPower.set(limits[POWER_DEFAULT], limits[POWER_MIN], limits[POWER_MAX]);
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_LONG);
        mToast.show();
    }
}
