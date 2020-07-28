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

package com.mediatek.engineermode.amrwb;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;


/**
 * Set AMR-WB on/off.
 */
public class AmrWbActivity extends Activity implements OnCheckedChangeListener {
    private static final String TAG = "AmrWbActivity";
    private static final int MSG_QUERY_GSM = 1;
    private static final int MSG_QUERY_UMTS = 2;
    private static final int MSG_SET = 3;
    private static final int ID_GSM = ModemCategory.isLteSupport() ? 59 : 8;
    private static final int ID_UMTS = ModemCategory.isLteSupport() ? 60 : 9;
    private static final String CMD_QUERY_GSM = "AT+ESBP=3," + ID_GSM;
    private static final String CMD_QUERY_UMTS = "AT+ESBP=3," + ID_UMTS;
    private static final String CMD_QUERY = "+ESBP:";
    private static final String CMD_SET_GSM = "AT+ESBP=1," + ID_GSM + ",";
    private static final String CMD_SET_UMTS = "AT+ESBP=1," + ID_UMTS + ",";

    private CheckBox mGsmAmrWb;
    private CheckBox mUmtsAmrWb;
    private boolean mGsmChecked = false;
    private boolean mUmtsChecked = false;

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
                case MSG_QUERY_GSM:
                    ar = (AsyncResult) msg.obj;
                    mGsmChecked = parseData(ar);
                    mGsmAmrWb.setChecked(mGsmChecked);
                    break;
                case MSG_QUERY_UMTS:
                    ar = (AsyncResult) msg.obj;
                    mUmtsChecked = parseData(ar);
                    mUmtsAmrWb.setChecked(mUmtsChecked);
                    break;
                case MSG_SET:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        Elog.e(TAG, ar.exception.getMessage());
                        EmUtils.showToast("Failed to set", Toast.LENGTH_SHORT);
                        Elog.w(TAG, "Failed to set");
                    } else {
                        EmUtils.showToast("Succeed to set", Toast.LENGTH_SHORT);
                        Elog.i(TAG, "Succeed to set");
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
        setContentView(R.layout.amr_wb);
        mGsmAmrWb = (CheckBox) findViewById(R.id.amr_wb_gsm);
        mUmtsAmrWb = (CheckBox) findViewById(R.id.amr_wb_umts);
        mGsmAmrWb.setOnCheckedChangeListener(this);
        mUmtsAmrWb.setOnCheckedChangeListener(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        sendAtCommand(new String[]{CMD_QUERY_GSM, CMD_QUERY}, MSG_QUERY_GSM);
        sendAtCommand(new String[]{CMD_QUERY_UMTS, CMD_QUERY}, MSG_QUERY_UMTS);
    }

    @Override
    public void onCheckedChanged(CompoundButton view, boolean isChecked) {
        if (view == mGsmAmrWb) {
            if (mGsmChecked != isChecked) {
                mGsmChecked = isChecked;
                sendAtCommand(new String[]{CMD_SET_GSM + (isChecked ? 1 : 0), ""}, MSG_SET);
            }
        } else if (view == mUmtsAmrWb) {
            if (mUmtsChecked != isChecked) {
                mUmtsChecked = isChecked;
                sendAtCommand(new String[]{CMD_SET_UMTS + (isChecked ? 1 : 0), ""}, MSG_SET);
            }
        }
    }

    private void sendAtCommand(String[] command, int msg) {
        Elog.i(TAG, "sendAtCommand() " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(command, mHandler.obtainMessage(msg));
    }

    private boolean parseData(AsyncResult ar) {
        if (ar.exception == null) {
            if (ar.result != null && ar.result instanceof String[]) {
                String data[] = (String[]) ar.result;
                if (data.length > 0 && data[0].length() > CMD_QUERY.length()) {
                    Elog.v(TAG, "data[0] is : " + data[0]);
                    try {
                        int flag = Integer.valueOf(data[0].substring(CMD_QUERY.length()).trim());
                        return flag == 1;
                    } catch (NumberFormatException e) {
                        Elog.e(TAG, e.getMessage());
                    }
                }
            }
        } else {
            Elog.e(TAG, ar.exception.getMessage());
        }
        EmUtils.showToast("Failed to query current settings", Toast.LENGTH_SHORT);
        return false;
    }
}
