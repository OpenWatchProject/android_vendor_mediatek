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

package com.mediatek.engineermode.fastdormancy;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;
/**
 * Description: A helper tool to test fast dormancy.
 *
 * @author mtk54043
 *
 */
public class FastDormancy extends Activity implements OnClickListener {
    private static final String TAG = "FastDormancy";

    private static final String STR_PROPERTY_FD_SCREEN_ON_TIMER =
            "persist.vendor.radio.fd.counter";
    private static final String STR_PROPERTY_FD_SCREEN_ON_R8_TIMER =
            "persist.vendor.radio.fd.r8.counter";
    private static final String STR_PROPERTY_FD_SCREEN_OFF_TIMER =
            "persist.vendor.radio.fd.off.counter";
    private static final String STR_PROPERTY_FD_SCREEN_OFF_R8_TIMER =
            "persist.vendor.radio.fd.off.r8.counter";

    private static final String DEFAULT_FD_SCREEN_ON_TIMER = "150";
    private static final String DEFAULT_FD_SCREEN_ON_R8_TIMER = "150";
    private static final String DEFAULT_FD_SCREEN_OFF_TIMER = "50";
    private static final String DEFAULT_FD_SCREEN_OFF_R8_TIMER = "50";

    private enum FdTimerType {
        SCREEN_OFF_LEGACY_FD,
        SCREEN_ON_LEGACY_FD,
        SCREEN_OFF_R8_FD,
        SCREEN_ON_R8_FD,
        SUPPORT_TIMER_TYPES
    }

    private String mTimerValue[] = {"50", "150", "50", "150"};

    private static final int COUNT = 6;
    private static final int COUNT_R8 = 4;
    private static final int INDEX_OFF_LE = 0;
    private static final int INDEX_ON_LE = 1;
    private static final int INDEX_OFF_R8 = 2;
    private static final int INDEX_ON_R8 = 3;
    private static final int INDEX_ON_AP = 4;
    private static final int INDEX_OFF_AP = 5;
    private static final int MSG_SET_TIME = 101;
    private static final int MSG_SEND_FD = 102;
    private static final int DIALOG_SET_FAILED = 201;
    private static final int DIALOG_SEND_FD = 202;

    private EditText[] mFastDormancyEdit;
    private int mFdMdEnableMode;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.fastdormancy);
        findViewById(R.id.fd_btn_set).setOnClickListener(this);
        findViewById(R.id.fd_btn_send).setOnClickListener(this);
        findViewById(R.id.fd_btn_config).setOnClickListener(this);

        mFastDormancyEdit = new EditText[COUNT];
        mFastDormancyEdit[INDEX_OFF_LE] = (EditText) findViewById(R.id.fd_edit_screen_off_legacy);
        mFastDormancyEdit[INDEX_ON_LE] = (EditText) findViewById(R.id.fd_edit_screen_on_legacy);
        mFastDormancyEdit[INDEX_OFF_R8] = (EditText) findViewById(R.id.fd_edit_screen_off_r8fd);
        mFastDormancyEdit[INDEX_ON_R8] = (EditText) findViewById(R.id.fd_edit_screen_on_r8fd);
        mFastDormancyEdit[INDEX_ON_AP] = (EditText) findViewById(R.id.fd_edit_screen_on_ap);
        mFastDormancyEdit[INDEX_OFF_AP] = (EditText) findViewById(R.id.fd_edit_screen_off_ap);

    }

    private void init() {
        mFdMdEnableMode = Integer.parseInt(SystemProperties.get("vendor.ril.fd.mode", "0"));
        initFdTimer();
    }

    @Override
    protected void onResume() {
        super.onResume();
        init();

        if (mFdMdEnableMode == 1) {
            for (int i = 0; i < COUNT_R8; i++) {
                mFastDormancyEdit[i].setText(mTimerValue[i]);
            }
        } else {
            String str1 = SystemProperties.get(STR_PROPERTY_FD_SCREEN_ON_TIMER, "20");
            String str2 = SystemProperties.get(STR_PROPERTY_FD_SCREEN_OFF_TIMER, "20");
            mFastDormancyEdit[INDEX_ON_AP].setText(str1);
            mFastDormancyEdit[INDEX_OFF_AP].setText(str2);

            findViewById(R.id.fd_screen_on_r8fd_layout).setVisibility(View.GONE);
            findViewById(R.id.fd_screen_off_r8fd_layout).setVisibility(View.GONE);
            findViewById(R.id.fd_screen_on_legacy_layout).setVisibility(View.GONE);
            findViewById(R.id.fd_screen_off_legacy_layout).setVisibility(View.GONE);
            findViewById(R.id.fd_ap_sol_on_layout).setVisibility(View.VISIBLE);
            findViewById(R.id.fd_ap_sol_off_layout).setVisibility(View.VISIBLE);
        }

    }

    @Override
    protected void onDestroy() {
        Elog.i(TAG, "onDestroy(), removeMessages");
        mResponseHander.removeMessages(MSG_SET_TIME);
        mResponseHander.removeMessages(MSG_SEND_FD);
        super.onDestroy();
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
        case R.id.fd_btn_set:
            if (mFdMdEnableMode == 1) { //modem solution
                for (int i = 0; i < COUNT_R8; i++) {
                    mTimerValue[i] = mFastDormancyEdit[i].getText().toString().trim();
                    try {
                        Integer.valueOf(mTimerValue[i]);
                    } catch (NumberFormatException e) {
                        Elog.e(TAG, "NumberFormatException");
                        Toast.makeText(this, "Your input number must be a int type!",
                                Toast.LENGTH_SHORT).show();
                        return;
                    }
                    Elog.i(TAG, "mTimerValue[" + i + "] = " + mTimerValue[i]);
                    String command[] = {"STATUS_SYNC",
                            "EM_FASTDORMANCY_SYNC:2," + i + "," + mTimerValue[i]};
                    sendAtCommand(command,MSG_SEND_FD);
                }
            } else {
                //Get scri data counter for screen on
                boolean isSuccess = true;
                String str1 = mFastDormancyEdit[INDEX_ON_AP].getText().toString().trim();
                int val1 = Integer.parseInt(str1);
                if (val1 < 5 || val1 > 3600) {
                    str1 = "20";
                    isSuccess = false;
                }

                //Get scri data counter for screen off
                String str2 = mFastDormancyEdit[INDEX_OFF_AP].getText().toString().trim();
                int val2 = Integer.parseInt(str2);
                if (val2 < 5 || val2 > 3600) {
                    str2 = "20";
                    isSuccess = false;
                }

                if (isSuccess) {
                    Toast.makeText(this, "Success!", Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(this, "Please set value between 5~3600! Restore to default: 20",
                            Toast.LENGTH_LONG).show();
                }
            }


            break;
        case R.id.fd_btn_send:
            String command[] = {"AT+ESCRI", ""};
            sendAtCommand(command,MSG_SEND_FD);
            break;
        case R.id.fd_btn_config:
            startActivity(new Intent(this, ConfigFD.class));
            break;
        default:
            break;
        }
    }

    private Handler mResponseHander = new Handler() {
        public void handleMessage(Message msg) {
            Elog.i(TAG, "Receive msg from modem");
            AsyncResult ar;
            if (msg.what == MSG_SET_TIME) {
                Elog.i(TAG, "Receive MSG_SET_TIME");
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(FastDormancy.this, "Success!", Toast.LENGTH_LONG).show();
                } else {
                    showDialog(DIALOG_SET_FAILED);
                }
            } else if (msg.what == MSG_SEND_FD) {
                Elog.i(TAG, "Receive MSG_SEND_FD");
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Toast.makeText(FastDormancy.this, "Success!", Toast.LENGTH_LONG).show();
                } else {
                    showDialog(DIALOG_SET_FAILED);
                }
            }
        }
    };

    @Override
    protected Dialog onCreateDialog(int id) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        if (id == DIALOG_SET_FAILED) {
            return builder.setTitle("Warnning!").setMessage("Failed to set FD parameter.")
                    .setPositiveButton("OK", null).create();
        } else if (id == DIALOG_SEND_FD) {
            return builder.setTitle("Warnning!").setMessage("Failed to send FD.")
                    .setPositiveButton("OK", null).create();
        } else {
            Elog.i(TAG, "Should no be here.");
        }
        return null;
    }

    private void initFdTimer() {
        String timerStr[] = new String[4];
        timerStr[0] = SystemProperties.get(STR_PROPERTY_FD_SCREEN_OFF_TIMER,
                DEFAULT_FD_SCREEN_OFF_TIMER);
        mTimerValue[FdTimerType.SCREEN_OFF_LEGACY_FD.ordinal()] = Integer
                .toString((int) (Double.parseDouble(timerStr[0])));
        timerStr[1] = SystemProperties.get(STR_PROPERTY_FD_SCREEN_ON_TIMER,
                DEFAULT_FD_SCREEN_ON_TIMER);
        mTimerValue[FdTimerType.SCREEN_ON_LEGACY_FD.ordinal()] = Integer
                .toString((int) (Double.parseDouble(timerStr[1])));
        timerStr[2] = SystemProperties.get(STR_PROPERTY_FD_SCREEN_OFF_R8_TIMER,
                DEFAULT_FD_SCREEN_OFF_R8_TIMER);
        mTimerValue[FdTimerType.SCREEN_OFF_R8_FD.ordinal()] = Integer
                .toString((int) (Double.parseDouble(timerStr[2])));
        timerStr[3] = SystemProperties.get(STR_PROPERTY_FD_SCREEN_ON_R8_TIMER,
                DEFAULT_FD_SCREEN_ON_R8_TIMER);
        mTimerValue[FdTimerType.SCREEN_ON_R8_FD.ordinal()] = Integer
                .toString((int) (Double.parseDouble(timerStr[3])));
        Elog.i(TAG, "initFdTimer: timers = " + mTimerValue[0] + ", " + mTimerValue[1]
                + ", " + mTimerValue[2] + ", " + mTimerValue[3]);
    }
    private void sendAtCommand(String[] command, int msg) {
        Elog.i(TAG, "sendAtCommand() " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(command, mResponseHander.obtainMessage(msg));
    }
}
