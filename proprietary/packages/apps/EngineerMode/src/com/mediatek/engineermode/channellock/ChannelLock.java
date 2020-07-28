/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.channellock;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;


public class ChannelLock extends Activity implements OnClickListener {
    private static final String TAG = "ChannelLock";
    private static final int REBOOT_DIALOG = 2000;
    private static final int MSG_CHANNEL_LOCK = 1;
    private static final int MSG_QUERY_CHANNEL_LOCK = 2;

    private Button mSetButtonReset;
    private Button mSetButtonApply;

    private RadioButton mRadioLockEnableButton;
    private RadioButton mRadioLockDisableButton;
    private RadioButton mRadioRatGSMButton;
    private RadioButton mRadioRatUMTSButton;
    private RadioButton mRadioRatLTEButton;
    private RadioButton mRadioRatNRButton;
    private RadioButton mRadioGSM1900YesButton;
    private RadioButton mRadioGSM1900NoButton;

    private RadioButton mRadioLockModeUnchanged;
    private RadioButton mRadioLockModeIdleOnly;
    private RadioButton mRadioLockModeIdleConnected;
    private int mSimType;
    private String mEMMCHLCKcommand = "AT+EMMCHLCK=";
    private EditText mTextARFCNnumber;
    private EditText mTextCELLIDNnumber;
    private final Handler mATCmdHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MSG_CHANNEL_LOCK:
                    if (ar.exception != null) {
                        Elog.i(TAG, "Failed to set Channel Lock");
                        EmUtils.showToast("Failed to set Channel Lock");
                    } else {
                        Elog.i(TAG, "set Channel Lock success");
                        EmUtils.showToast("set Channel Lock success");
                        showDialog(REBOOT_DIALOG);
                    }
                    break;
                case MSG_QUERY_CHANNEL_LOCK:
                    if (ar.exception == null) {
                        Elog.i(TAG, "Query success.");
                        String[] data = (String[]) ar.result;
                        if (null == data) {
                            EmUtils.showToast("The returned data is wrong.");
                            Elog.e(TAG, "The returned data is wrong.");
                            return;
                        }
                        handleQuery(data);

                    } else {
                        EmUtils.showToast("Query failed.");
                        Elog.d(TAG, "Query failed.");
                    }
                    break;
                default:
                    break;
            }
        }
    };


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.channel_lock);
        Intent intent = getIntent();
        mSimType = intent.getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        Elog.d(TAG, "mSimType: " + mSimType);

        //apply button
        mSetButtonApply = (Button) findViewById(R.id.channel_lock_apply_button);
        //reset button
        mSetButtonReset = (Button) findViewById(R.id.channellock_reset_button);

        mSetButtonReset.setOnClickListener(this);
        mSetButtonApply.setOnClickListener(this);

        //lock enable
        mRadioLockEnableButton = (RadioButton) findViewById(R.id.channel_lock_lock_enable_radio);
        mRadioLockDisableButton = (RadioButton) findViewById(R.id.channel_lock_lock_disable_radio);

        //RAT
        mRadioRatGSMButton = (RadioButton) findViewById(R.id.rat_gsm_radio);
        mRadioRatUMTSButton = (RadioButton) findViewById(R.id.rat_umts_radio);
        mRadioRatLTEButton = (RadioButton) findViewById(R.id.rat_lte_radio);
        mRadioRatNRButton = (RadioButton) findViewById(R.id.rat_nr_radio);

        //GSM1900
        mRadioGSM1900YesButton = (RadioButton) findViewById(R.id.gsm1900_yes_radio);
        mRadioGSM1900NoButton = (RadioButton) findViewById(R.id.gsm1900_no_radio);

        //ARFCN
        mTextARFCNnumber = (EditText) findViewById(R.id.channel_lock_ARCFN_number_text);

        //CELL ID
        mTextCELLIDNnumber = (EditText) findViewById(R.id.channel_lock_cellid_number_text);

        //LOCK Mode
        mRadioLockModeUnchanged = (RadioButton) findViewById(R.id.lock_mode_unchanged);
        mRadioLockModeIdleOnly = (RadioButton) findViewById(R.id.lock_mode_idle_only);
        mRadioLockModeIdleConnected = (RadioButton) findViewById(R.id.lock_mode_idle_connected);
    }

    @Override
    protected void onResume() {
        super.onResume();
        String[] cmd = new String[2];
        cmd[0] = "AT+EMMCHLCK?";
        cmd[1] = "+EMMCHLCK:";
        sendATCommand(cmd, MSG_QUERY_CHANNEL_LOCK);
    }

    //+EMMCHLCK=<mode>,[<act>,<band indicator>,[<number of channel>,]< arfcn >,[<arfcn>,<arfcn>,
    // ...,<arfcn>],<cell id>]
    @Override
    public void onClick(final View arg0) {
        mEMMCHLCKcommand = "AT+EMMCHLCK=";
        if (arg0 == mSetButtonReset) {
            EmUtils.setAirplaneModeEnabled(true);
            EmUtils.setAirplaneModeEnabled(false);
            EmUtils.showToast("Reset the airplane Mode succeed,please try");
        } else if (arg0 == mSetButtonApply) {       //enable
            if (mRadioLockDisableButton.isChecked()) {       //disable
                mEMMCHLCKcommand = mEMMCHLCKcommand + "0";
            } else if (mRadioLockEnableButton.isChecked()) {
                mEMMCHLCKcommand = mEMMCHLCKcommand + "1,";
                //rat
                if (mRadioRatGSMButton.isChecked()) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "0,"; //2G
                } else if (mRadioRatUMTSButton.isChecked()) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "2,"; //3G
                } else if (mRadioRatLTEButton.isChecked()) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "7,"; //4G
                } else if (mRadioRatNRButton.isChecked()) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "11,"; //5G
                } else {
                    EmUtils.showToast("ERROR in RAT");
                }

                //GSM1900
                if (mRadioGSM1900YesButton.isChecked()) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "1,";
                } else if (mRadioGSM1900NoButton.isChecked()) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "0,";
                } else {
                    EmUtils.showToast("ERROR in GSM1900");
                }

                //ARFCN
                mEMMCHLCKcommand = mEMMCHLCKcommand + mTextARFCNnumber.getText() + ",";

                //CELL ID
                mEMMCHLCKcommand = mEMMCHLCKcommand + mTextCELLIDNnumber.getText();

                //lock mode
                if (!mRadioLockModeUnchanged.isChecked()) {
                    if (mRadioLockModeIdleOnly.isChecked()) {
                        mEMMCHLCKcommand = mEMMCHLCKcommand + ",0";
                    } else if (mRadioLockModeIdleConnected.isChecked()) {
                        mEMMCHLCKcommand = mEMMCHLCKcommand + ",3";
                    }
                }

            } else {
                EmUtils.showToast("ERROR in Lock");
            }
            sendATCommand(new String[]{mEMMCHLCKcommand, ""}, MSG_CHANNEL_LOCK);
        }
    }

    private void sendATCommand(String[] atCommand, int msg) {
        Elog.d(TAG, "sendATCommand " + atCommand[0]);
        EmUtils.invokeOemRilRequestStringsEm(mSimType, atCommand, mATCmdHander.obtainMessage(msg));
    }

    private void handleQuery(String[] data) {
        Elog.v(TAG, "data = " + data[0]);
        String str = data[0].substring("+EMMCHLCK:".length()).trim();
        Elog.d(TAG, "response value:" + str);
        String[] arrayval = str.split(",");
        int lockStatus = 0;
        int ret = 0;
        int gsm1900 = 0;
        int arfcn = 0;
        int cellId = 0;
        int lockMode = 0;

        try {
            lockStatus = Integer.parseInt(arrayval[0]);
            Elog.v(TAG, "lockStatus = " + lockStatus);
            if (lockStatus != 0) {
                ret = Integer.parseInt(arrayval[1]);
                gsm1900 = Integer.parseInt(arrayval[2]);
                arfcn = Integer.parseInt(arrayval[3]);
                cellId = Integer.parseInt(arrayval[4]);
                lockMode = Integer.parseInt(arrayval[5]);
                Elog.v(TAG, "ret = " + ret);
                Elog.v(TAG, "gsm1900 = " + gsm1900);
                Elog.v(TAG, "arfcn = " + arfcn);
                Elog.v(TAG, "cellId = " + cellId);
                Elog.v(TAG, "lockMode = " + lockMode);
            }
        } catch (Exception e) {
            Elog.e(TAG, "response value parse failed," + e.getMessage());
        }

        if (lockStatus == 0) {
            mRadioLockDisableButton.setChecked(true);
        } else if (lockStatus == 1) {
            mRadioLockEnableButton.setChecked(true);

            //rat
            if (ret == 0) {
                mRadioRatGSMButton.setChecked(true);
            } else if (ret == 2) {
                mRadioRatUMTSButton.setChecked(true);
            } else if (ret == 7) {
                mRadioRatLTEButton.setChecked(true);
            }  else if (ret == 11) {
                mRadioRatNRButton.setChecked(true);
            }else {
                Elog.e(TAG, "Invalid Channel Lock RAT value");
            }

            //GSM1900
            if (gsm1900 == 1) {
                mRadioGSM1900YesButton.setChecked(true);
            } else if (gsm1900 == 0) {
                mRadioGSM1900NoButton.setChecked(true);
            } else {
                Elog.e(TAG, "Invalid Channel Lock GSM1900 value");
            }

            //ARFC
            mTextARFCNnumber.setText(String.valueOf(arfcn));

            //cellid
            if (cellId < 0 || cellId > 511) {
                Elog.e(TAG, "Invalid Cell ID value");
                EmUtils.showToast("Invalid Cell ID value");
            } else {
                mTextCELLIDNnumber.setText(String.valueOf(cellId));
            }
            //lock mode
            if (lockMode == 0) {
                mRadioLockModeIdleOnly.setChecked(true);
            } else if (lockMode == 3) {
                mRadioLockModeIdleConnected.setChecked(true);
            } else {
                Elog.e(TAG, "Invalid Channel Lock mode value");
            }
        } else {
            Elog.e(TAG, "Invalid Channel Lock value");
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder = null;
        switch (id) {
            case REBOOT_DIALOG:
                return new AlertDialog.Builder(this).setTitle("Channel lock:")
                        .setMessage("Please click Reset by Airplane Mode button to take effect!")
                        .setPositiveButton("OK", null).create();
        }
        return dialog;
    }

}
