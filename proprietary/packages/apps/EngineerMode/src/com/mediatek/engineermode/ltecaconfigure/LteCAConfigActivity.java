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

package com.mediatek.engineermode.ltecaconfigure;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

/**
 * Description: To set LTE CA status.
 */
public class LteCAConfigActivity extends Activity {
    private static final String TAG = "LteCAConfigActivity";

    private static final String SET_CA_CMD = "AT+ECASW=";
    private static final String QUERY_CA_CMD = "AT+ECASW?";
    private static final String FORE_CMD = "+ECASW:";

    private static final int MSG_QUERY_CA_CMD = 101;
    private static final int MSG_SET_CA_CMD = 102;

    private RadioButton mRadioBtnOn;
    private RadioButton mRadioBtnOff;
    private Toast mToast = null;

    private final Handler mCommandHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Elog.i(TAG, "Receive msg from modem");
            AsyncResult asyncResult = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MSG_QUERY_CA_CMD:

                    if (asyncResult != null && asyncResult.exception == null
                            && asyncResult.result != null) {
                        final String[] result = (String[]) asyncResult.result;
                        Elog.d(TAG, "Query lte CA status succeed,result = " + result[0]);
                        int mode = parseCurrentLteCAMode(result[0]);
                        Elog.d(TAG, "mode = " + mode);
                        if (mode == 0) {
                            mRadioBtnOff.setChecked(true);
                        } else {
                            mRadioBtnOn.setChecked(true);
                        }
                    } else {
                        showToast("Query lte CA status failed.");
                        Elog.d(TAG, "Query lte CA status failed.");
                    }
                    break;

                case MSG_SET_CA_CMD:
                    if (null == asyncResult.exception) {
                        showToast("set LTE CA Succeed!");
                        Elog.d(TAG, "set LTE CA Succeed!");
                    } else {
                        showToast("set LTE CA failed!");
                        Elog.d(TAG, "set LTE CA failed!");
                    }
                    break;

                default:
                    break;
            }
        }
    };

    private int parseCurrentLteCAMode(String data) {
        int mode = -1;
        try {
            mode = Integer.valueOf(data.substring(FORE_CMD.length()).trim());
        } catch (NumberFormatException e) {
            Elog.e(TAG, "Wrong current mode format: " + data);
        }
        return mode;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.lte_ca_config_activity);

        mRadioBtnOn = (RadioButton) findViewById(R.id.lte_ca_on_radio);
        mRadioBtnOff = (RadioButton) findViewById(R.id.lte_ca_off_radio);
        Button buttonOk = (Button) findViewById(R.id.lte_ca_set_button);


        queryCAStatus();

        buttonOk.setOnClickListener(new View.OnClickListener() {

            public void onClick(View v) {
                if (mRadioBtnOn.isChecked()) {
                    sendAtCommand("1");
                    Elog.i(TAG, "Set LTE CA Status : on");
                } else if (mRadioBtnOff.isChecked()) {
                    sendAtCommand("0");
                    Elog.i(TAG, "Set LTE CA Status : off");
                }
            }
        });
    }

    private void queryCAStatus() {
        sendCommand(new String[]{QUERY_CA_CMD, FORE_CMD}, MSG_QUERY_CA_CMD);
    }

    private void sendAtCommand(String cmd) {
        sendCommand(new String[]{SET_CA_CMD + cmd, ""}, MSG_SET_CA_CMD);
    }

    private void sendCommand(String[] command, int msg) {
        Elog.d(TAG, "sendCommand " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(command, mCommandHander.obtainMessage(msg));
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}
