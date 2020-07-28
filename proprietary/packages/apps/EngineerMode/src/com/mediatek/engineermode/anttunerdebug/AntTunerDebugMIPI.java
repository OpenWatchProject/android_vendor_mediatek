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

package com.mediatek.engineermode.anttunerdebug;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

public class AntTunerDebugMIPI extends Activity implements OnClickListener {
    private static final String TAG = "AntTunerDebugMIPI";

    public static final int OP_MIPI_READ = 0;
    public static final int OP_MIPI_WRITE = 1;
    private static final int OP_MIPI_SIGNALING = 3;
    private static final int OP_MIPI_NON_SIGNALING = 4;
    private static final String RESPONSE_CMD = "+ERFTX: ";

    private Spinner mSpRWType;

    private EditText mEdMipiPort;
    private EditText mEdUsid;
    private EditText mEdAddress;
    private EditText mEdData;
    private TextView mTvAddress;

    private Button mBtnMipiRead;
    private Button mBtnMipiWrite;
    private Button mbtnMipiSignaling;
    private Button mbtnMipiNonSignaling;

    private String mPort = null;
    private String mRWType = null;
    private String mUsid = null;
    private String mAddress = null;
    private String mData = null;
    private String mMipiMode = null;


    ArrayAdapter<CharSequence> adapterPattern = null;

    private void handleQuery(String[] result) {
        if (result != null && result.length > 0) {
            Elog.i(TAG, "Modem return: " + result[0]);
            String[] values = result[0]
                    .substring(RESPONSE_CMD.length(), result[0].length())
                    .trim().split(",");
            if (values != null && values.length > 0) {
                if (values[2] != null) {
                    values[2] = Integer
                            .toHexString(Integer.parseInt(values[2]));
                    mEdData.setText(values[2]);
                }
            }
        } else {
            Elog.e(TAG, "Modem return error");
        }
    }

    private final Handler mATHandler = new Handler() {
        private String[] mReturnData = new String[2];

        @Override
        public void handleMessage(Message msg) {
            if (msg.what == OP_MIPI_READ) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Elog.d(TAG, "MIPI read successful.");
                    mReturnData = (String[]) ar.result;
                    Elog.d(TAG, "mReturnData = " + mReturnData[0]);
                    EmUtils.showToast(mReturnData[0],Toast.LENGTH_SHORT);
                    handleQuery(mReturnData);
                } else {
                    EmUtils.showToast("MIPI read failed.",Toast.LENGTH_SHORT);
                    Elog.e(TAG, "MIPI read failed.");
                }
            } else if (msg.what == OP_MIPI_WRITE) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Elog.d(TAG, "MIPI write successful.");
                } else {
                    EmUtils.showToast("MIPI write failed.",Toast.LENGTH_SHORT);
                    Elog.e(TAG, "MIPI write failed.");
                }
            } else if (msg.what == OP_MIPI_SIGNALING) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Elog.d(TAG, "MIPI Signaling successful.");
                } else {
                    EmUtils.showToast("MIPI Signaling failed.",Toast.LENGTH_SHORT);
                    Elog.e(TAG, "MIPI Signaling failed.");
                }
            } else if (msg.what == OP_MIPI_NON_SIGNALING) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Elog.d(TAG, "MIPI Non-Signaling successful.");
                } else {
                    EmUtils.showToast("MIPI Non-Signaling failed.",Toast.LENGTH_SHORT);
                    Elog.e(TAG, "MIPI Non-Signaling failed.");
                }
            }

        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ant_tuner_debug_mipi);

        mSpRWType = (Spinner) findViewById(R.id.ant_tuner_debug_mipi_rw_type);
        adapterPattern = ArrayAdapter.createFromResource(this,
                R.array.ant_tuner_debug_mipi_rw_type,
                android.R.layout.simple_spinner_item);
        adapterPattern
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpRWType.setAdapter(adapterPattern);

        mEdMipiPort = (EditText) findViewById(R.id.ant_tuner_debug_mipi_port);
        mEdUsid = (EditText) findViewById(R.id.ant_tuner_debug_mipi_usid);
        mEdAddress = (EditText) findViewById(R.id.ant_tuner_debug_mipi_address);
        mEdData = (EditText) findViewById(R.id.ant_tuner_debug_mipi_data);
        mTvAddress = (TextView) findViewById(R.id.ant_tuner_debug_mipi_address_tv);

        mBtnMipiRead = (Button) findViewById(R.id.ant_tuner_debug_mipi_read);
        mBtnMipiWrite = (Button) findViewById(R.id.ant_tuner_debug_mipi_write);
        mBtnMipiRead.setOnClickListener(this);
        mBtnMipiWrite.setOnClickListener(this);

        mbtnMipiSignaling = (Button) findViewById(R.id.mipi_signaling);
        mbtnMipiNonSignaling = (Button) findViewById(R.id.mipi_non_signaling);
        mbtnMipiSignaling.setOnClickListener(this);
        mbtnMipiNonSignaling.setOnClickListener(this);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    public void onClick(View arg0) {
        switch (arg0.getId()) {
        case R.id.ant_tuner_debug_mipi_read:
            if (valueGetAndCheck(0) == true) {
                String[] cmd = new String[2];
                mUsid = Integer.toString(Integer.parseInt(mUsid, 16));
                mAddress = Integer.toString(Integer.parseInt(mAddress, 16));
                cmd[0] = "AT+ERFTX=12,0," + mMipiMode + "," + mPort + ","
                        + mRWType + "," + mUsid + "," + mAddress;
                cmd[1] = "+ERFTX:";
                sendAtCommand(cmd, OP_MIPI_READ);
            }
            break;
        case R.id.ant_tuner_debug_mipi_write:
            if (valueGetAndCheck(1) == true) {
                String[] cmd = new String[2];
                mUsid = Integer.toString(Integer.parseInt(mUsid, 16));
                mAddress = Integer.toString(Integer.parseInt(mAddress, 16));
                mData = Integer.toString(Integer.parseInt(mData, 16));

                cmd[0] = "AT+ERFTX=12,0," + mMipiMode + "," + mPort + ","
                        + mRWType + "," + mUsid + "," + mAddress + "," + mData;
                cmd[1] = "";
                sendAtCommand(cmd, OP_MIPI_WRITE);
            }
            break;
        case R.id.mipi_signaling:
            String[] cmdSignaling = new String[2];
            cmdSignaling[0] = "AT+CFUN=1";
            cmdSignaling[1] = "";
            sendAtCommand(cmdSignaling, OP_MIPI_SIGNALING);
            break;
        case R.id.mipi_non_signaling:
            String[] cmdNonSignaling1 = new String[2];
            cmdNonSignaling1[0] = "AT+CFUN=0";
            cmdNonSignaling1[1] = "";
            sendAtCommand(cmdNonSignaling1, OP_MIPI_NON_SIGNALING);

            String[] cmdNonSignaling2 = new String[2];
            cmdNonSignaling2[0] = "AT+EGCMD=53";
            cmdNonSignaling2[1] = "";
            sendAtCommand(cmdNonSignaling2, OP_MIPI_NON_SIGNALING);
            break;
        default:
            break;
        }
    }

    private void sendAtCommand(String[] command, int msg) {
        Elog.d(TAG, "sendAtCommand() " + command[0]);
        EmUtils.showToast("sendAtCommand: " + command[0],Toast.LENGTH_SHORT);
        EmUtils.invokeOemRilRequestStringsEm(command, mATHandler.obtainMessage(msg));
    }

    private boolean valueGetAndCheck(int flag) {
        mPort = mEdMipiPort.getText().toString();
        mRWType = (mSpRWType.getSelectedItemPosition() == 0) ? "0" : "1";
        mUsid = mEdUsid.getText().toString();
        mAddress = mEdAddress.getText().toString();
        mData = mEdData.getText().toString();
        mMipiMode = (flag == 0) ? "0" : "1"; // 0:read 1:write

        if (mPort.equals("")) {
            EmUtils.showToast("MIPI port should not be empty",Toast.LENGTH_SHORT);
            return false;
        } else if (mRWType.equals("")) {
            EmUtils.showToast("RW_TYPE should not be empty",Toast.LENGTH_SHORT);
            return false;
        } else if (mUsid.equals("")) {
            EmUtils.showToast("USID should not be empty",Toast.LENGTH_SHORT);
            return false;
        } else if (mAddress.equals("")) {
            EmUtils.showToast("Address should not be empty",Toast.LENGTH_SHORT);
            return false;
        }

        try {
            int value = Integer.parseInt(mUsid, 16);
            if (value < 0 || value > 0xf) {
                EmUtils.showToast("Usid should be 0x0~0xf ",Toast.LENGTH_SHORT);
                return false;
            }
        } catch (NumberFormatException e) {
            EmUtils.showToast("Usid should be 16 HEX",Toast.LENGTH_SHORT);
            return false;
        }

        try {
            int value = Integer.parseInt(mAddress, 16);
            if (mRWType.equals("0")) {
                mTvAddress
                        .setText(getString(R.string.ant_tuner_debug_mipi_address));
                if (value < 0 || value > 0x1f) {
                    EmUtils.showToast("mAddress should be 0x0~0x1f ",Toast.LENGTH_SHORT);
                    return false;
                }
            } else if (mRWType.equals("1")) {
                mTvAddress
                        .setText(getString(R.string.ant_tuner_debug_mipi_address1));
                if (value < 0 || value > 0xff) {
                    EmUtils.showToast("mAddress should be 0x0~0xff ",Toast.LENGTH_SHORT);
                    return false;
                }
            }

        } catch (NumberFormatException e) {
            EmUtils.showToast("mAddress should be 16 HEX",Toast.LENGTH_SHORT);
            return false;
        }

        if (mMipiMode.equals("1")) {
            if (mData.equals("")) {
                EmUtils.showToast("Data should not be empty",Toast.LENGTH_SHORT);
                return false;
            } else {
                try {
                    int value = Integer.parseInt(mData, 16);
                    if (value < 0 || value > 0xff) {
                        EmUtils.showToast("Usid should be 0x0~0xff ",Toast.LENGTH_SHORT);
                        return false;
                    }
                } catch (NumberFormatException e) {
                    EmUtils.showToast("mData should be 16 HEX",Toast.LENGTH_SHORT);
                    return false;
                }
            }
        }
        return true;
    }
}
