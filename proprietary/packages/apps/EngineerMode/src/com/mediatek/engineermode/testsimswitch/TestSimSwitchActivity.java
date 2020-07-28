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

package com.mediatek.engineermode.testsimswitch;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

public class TestSimSwitchActivity extends Activity {
    private static final String TAG = "testsimswitch";
    private static final String PROP_TESTSIM_CARDTYPE = "persist.vendor.radio.testsim.cardtype";
    private static final int REBOOT = 0;
    private RadioButton mRadioBtnDefault;
    private RadioButton mRadioBtnCdma;
    private RadioButton mRadioBtnGsm;
    private String mTestSimType = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.testsimswitch);
        mRadioBtnDefault = (RadioButton) findViewById(R.id.simtype_default_radio);
        mRadioBtnCdma = (RadioButton) findViewById(R.id.simtype_cdma_radio);
        mRadioBtnGsm = (RadioButton) findViewById(R.id.simtype_gsm_radio);

        Button buttonSet = (Button) findViewById(R.id.simtype_set_button);

        buttonSet.setOnClickListener(new View.OnClickListener() {

            public void onClick(View v) {
                if (mRadioBtnDefault.isChecked()) {
                    mTestSimType = "0";
                } else if (mRadioBtnCdma.isChecked()) {
                    mTestSimType = "1";
                } else if (mRadioBtnGsm.isChecked()) {
                    mTestSimType = "2";
                } else {
                    mTestSimType = "";
                }
                try {
                    EmUtils.getEmHidlService().setTestSimCardType(mTestSimType);
                } catch (Exception e) {
                    e.printStackTrace();
                    Elog.e(TAG, "set property failed ...");
                }
                Elog.d(TAG, "set persist.radio.testsim.cardtype to " + mTestSimType);
                showDialog(REBOOT);
            }
        });
    }


    @Override
    protected void onResume() {
        super.onResume();
        queryTestSimType();
    }

    // get the IA APN for default data sub
    private void queryTestSimType() {
        mTestSimType = SystemProperties.get(PROP_TESTSIM_CARDTYPE, "0");
        Elog.d(TAG, "get persist.radio.testsim.cardtype is " + mTestSimType);
        if (mTestSimType.equals("0")) {
            mRadioBtnDefault.setChecked(true);
        } else if (mTestSimType.equals("1")) {
            mRadioBtnCdma.setChecked(true);
        } else if (mTestSimType.equals("2")) {
            mRadioBtnGsm.setChecked(true);
        } else {
            mRadioBtnDefault.setChecked(true);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case REBOOT:
                return new AlertDialog.Builder(this).setTitle("Hint")
                        .setMessage("Please reboot phone to apply new setting!")
                        .setPositiveButton(android.R.string.ok, new OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {

                            }
                        })
                        .create();
        }
        return super.onCreateDialog(id);
    }

}
