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

package com.mediatek.engineermode.modemfilter;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

/**
 * MD EM Filter
 */
public class ModemFilterActivity extends Activity implements OnCheckedChangeListener {
    private static final String TAG = "ModemFilter";
    private static final String MODEM_FILTER_SHAREPRE= "telephony_modem_filter_settings";
    private static final int MSG_ENABLE_MD_FILTER = 1;
    private static final int MSG_DISABLE_MD_FILTER = 2;
    private static final String CMD_ENABLE_MD_FILTER = "AT+EINFO=8,4294967295,0,0";
    private static final String CMD_DISABLE_MD_FILTER = "AT+EINFO=8,4294967295,1,0";

    private Phone mPhone = null;
    private CheckBox mEnableMDFilter;
    private boolean mEnableMDFilterCheckedCancel = false;
    private int mSimType;

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
            case MSG_ENABLE_MD_FILTER:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    Elog.e(TAG, ar.exception.getMessage());
                    Toast.makeText(ModemFilterActivity.this, "Failed to enable MD filter",
                        Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(ModemFilterActivity.this, "Enable MD filter success",
                        Toast.LENGTH_SHORT).show();
                }
                break;
            case MSG_DISABLE_MD_FILTER:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    Elog.e(TAG, ar.exception.getMessage());
                    Toast.makeText(ModemFilterActivity.this, "Failed to disable MD filter",
                        Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(ModemFilterActivity.this, "Disable MD filter success",
                        Toast.LENGTH_SHORT).show();
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
        setContentView(R.layout.md_em_filter);
        mEnableMDFilter = (CheckBox) findViewById(R.id.md_em_filter);
        mSimType = getIntent().getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        Elog.d(TAG, "onCreate mSimType: " + mSimType);
        final SharedPreferences modemFilterSh = getSharedPreferences(MODEM_FILTER_SHAREPRE,
            android.content.Context.MODE_PRIVATE);
        boolean enableMDFilterChecked = false;
        if (mSimType == PhoneConstants.SIM_ID_2) {
            enableMDFilterChecked = modemFilterSh.getBoolean(
                getString(R.string.enable_md_filter_sim2), false);
        } else {
            enableMDFilterChecked = modemFilterSh.getBoolean(
                getString(R.string.enable_md_filter_sim1), false);
        }

        Elog.d(TAG, "onCreate enableMDFilterChecked " + enableMDFilterChecked);
        if (enableMDFilterChecked) {
            mEnableMDFilter.setChecked(true);
        } else {
            mEnableMDFilter.setChecked(false);
        }

        mEnableMDFilter.setOnCheckedChangeListener(this);


    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onCheckedChanged(CompoundButton view, boolean isChecked) {
        if (view == mEnableMDFilter) {
            if (isChecked) {
                showAlertDlg("MD will enable all EM types, it may cause more power consumption!");
            } else {
                if (!mEnableMDFilterCheckedCancel) {
                    Elog.d(TAG, "sendAtCommand disable");
                    writeSharedPreference(false);
                    sendAtCommand(new String[] {CMD_DISABLE_MD_FILTER, ""}, MSG_DISABLE_MD_FILTER);
                }
            }
        }

    }

    private void sendAtCommand(String[] command, int msg) {
        Elog.d(TAG, "sendAtCommand() " + command[0]);

        EmUtils.invokeOemRilRequestStringsEm(mSimType,command, mHandler.obtainMessage(msg));
    }

    private void showAlertDlg(String message) {
        AlertDialog.Builder dialog = new AlertDialog.Builder(this);
        dialog.setTitle(android.R.string.dialog_alert_title);
        dialog.setMessage(message);
        dialog.setIcon(android.R.drawable.ic_dialog_alert);
        dialog.setCancelable(false);
        dialog.setPositiveButton(android.R.string.yes, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                 mEnableMDFilterCheckedCancel = false;
                 writeSharedPreference(true);
                 sendAtCommand(new String[] {CMD_ENABLE_MD_FILTER, ""}, MSG_ENABLE_MD_FILTER);
                 dialog.dismiss();
            }
        });

       dialog.setNegativeButton(android.R.string.no, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                 mEnableMDFilterCheckedCancel = true;
                 mEnableMDFilter.setChecked(false);
                 writeSharedPreference(false);
                 dialog.dismiss();
            }
        });
        dialog.show();
    }

    private void writeSharedPreference(boolean flag) {
        final SharedPreferences modemFilterSh = getSharedPreferences(
                   MODEM_FILTER_SHAREPRE, android.content.Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = modemFilterSh.edit();
        if (mSimType == PhoneConstants.SIM_ID_2) {
           editor.putBoolean(getString(R.string.enable_md_filter_sim2), flag);
        } else {
           editor.putBoolean(getString(R.string.enable_md_filter_sim1), flag);
        }
        editor.commit();
    }

}
