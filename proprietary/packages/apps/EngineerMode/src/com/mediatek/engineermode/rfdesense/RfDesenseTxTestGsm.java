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

import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class RfDesenseTxTestGsm extends RfDesenseTxTestBase {
    public static final String TAG = "RfDesense/TxTestGsm";

    public static final String KEY_BAND = "Band_2G";
    public static final String KEY_MODULATION = "Bodulation_2G";
    public static final String KEY_CHANNEL = "Channel_2G";
    public static final String KEY_POWER = "Power_2G";
    public static final String KEY_AFC = "AFC_2G";
    public static final String KEY_TSC = "TSC_2G";
    public static final String KEY_PATTERN = "Pattern_2G";
    public static final String KEY_ANT_MODE= "gsm_ant_mode";
    public static final String KEY_ANT_STATUS = "gsm_ant_status";

    private void restoreState() {
        SharedPreferences pref = getSharedPreferences(RfDesenseTxTest.PREF_FILE,
                MODE_PRIVATE);
        mCurrentBand = pref.getInt(KEY_BAND, 0);
        mBand.setSelection(mCurrentBand);
        updateLimits();
        mChannel.setText(pref.getString(KEY_CHANNEL, mChannel.defaultValue));
        mPower.setText(pref.getString(KEY_POWER, mPower.defaultValue));
        mAfc.setText(pref.getString(KEY_AFC, mAfc.defaultValue));
        mTsc.setText(pref.getString(KEY_TSC, mTsc.defaultValue));
        mPattern.setSelection(pref.getInt(KEY_PATTERN, 0));
        mAntMode.setChecked(pref.getString(KEY_ANT_MODE, RfDesenseTxTestLte.DEFAULT_ANT_MODE)
                .equals("1"));
        mAntStatus.setText(pref.getString(KEY_ANT_STATUS, RfDesenseTxTestLte.DEFAULT_ANT_STATUS));
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        ArrayAdapter<CharSequence> adapter_band = ArrayAdapter.createFromResource(this,
                R.array.rf_desense_tx_test_gsm_band,
                android.R.layout.simple_spinner_item);
        adapter_band.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBand.setAdapter(adapter_band);

        ArrayAdapter<CharSequence> adapter_pattern = ArrayAdapter.createFromResource(this,
                R.array.rf_desense_tx_test_gsm_pattern,
                android.R.layout.simple_spinner_item);
        adapter_pattern.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mPattern.setAdapter(adapter_pattern);

        restoreState();
        Elog.i(TAG, "restoreState GSM" );
        mDbm.setVisibility(View.GONE);
    }

    protected void updateLimits() {
        int band = mBand.getSelectedItemPosition();
        String[] limits = getResources().getStringArray(
                R.array.rf_desense_tx_test_gsm_gmsk_limits)[band].split(",");

        mChannel.set(limits[CHANNEL_DEFAULT], limits[CHANNEL_MIN], limits[CHANNEL_MAX],
                limits[CHANNEL_MIN2], limits[CHANNEL_MAX2]);
        mPower.set(limits[POWER_DEFAULT], limits[POWER_MIN], limits[POWER_MAX]);
        mPower.step = 1;
        mAfc.set("4100", "0", "8191");
        mTsc.set("0", "0", "7");
    }
}
