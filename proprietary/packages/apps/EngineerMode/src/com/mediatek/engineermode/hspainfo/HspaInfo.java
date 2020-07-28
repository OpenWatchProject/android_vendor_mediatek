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

package com.mediatek.engineermode.hspainfo;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

public class HspaInfo extends Activity {
    private static final String TAG = "HspaInfo";
    private static final int EVENT_HSPA_INFO = 1;
    private static final String QUERY_CMD = "AT+EHSM?";
    private static final String RESPONSE_CMD = "+EHSM: ";

    private Phone mPhone = null;
    private TextView mTextView;
    private TextView mTextView2;

    private Handler mATCmdHander = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
                case EVENT_HSPA_INFO:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception == null) {
                        handleQuery((String[]) ar.result, msg.what);
                    } else {
                        Toast.makeText(HspaInfo.this, "Send AT command failed", Toast.LENGTH_LONG);
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
        setContentView(R.layout.hspa_info);
        mTextView = (TextView) findViewById(R.id.text_view);
        mTextView2 = (TextView) findViewById(R.id.text_view2);
        mTextView2.setVisibility(View.GONE);
    }

    @Override
    protected void onResume() {
        super.onResume();
        String[] cmd = new String[2];
        cmd[0] = QUERY_CMD;
        cmd[1] = RESPONSE_CMD;
        sendATCommand(cmd,EVENT_HSPA_INFO);
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    private void handleQuery(String[] result, int queryWhat) {
        if (result != null && result.length > 0) {
            Elog.v(TAG, "Modem return: " + result[0]);
            Elog.v(TAG, "queryWhat: " + queryWhat);
            if (queryWhat == EVENT_HSPA_INFO) {
                String[] mode = result[0]
                        .substring(RESPONSE_CMD.length(), result[0].length()).split(",");
                if (mode != null && mode.length > 0) {
                    try {
                        Elog.v(TAG, "mode[0]: \"" + mode[0] + "\"");
                        updateUI(Integer.parseInt(mode[0]));
                    } catch (NumberFormatException e) {
                        Elog.e(TAG, "Modem return invalid mode: " + mode[0]);
                        mTextView.setText("Error: invalid mode: " + mode[0]);
                    }
                    return;
                }
            }
        }
        mTextView.setText("Error: invalid mode.");
    }

    private void updateUI(int mode) {
        String[] modeArray;
        if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
            modeArray = getResources().getStringArray(R.array.hspa_info_mode_array_td);
        } else {
            modeArray = getResources().getStringArray(R.array.hspa_info_mode_array_fd);
        }

        if (mode < 0 || mode >= modeArray.length) {
            Elog.e(TAG, "Modem return invalid mode: " + mode);
            mTextView.setText("Error: invalid mode: " + mode);
            return;
        }
        mTextView.setText(modeArray[mode]);
    }


    private void sendATCommand(String[] atCommand, int msg) {
        EmUtils.invokeOemRilRequestStringsEm(atCommand, mATCmdHander.obtainMessage(msg));
    }
}
