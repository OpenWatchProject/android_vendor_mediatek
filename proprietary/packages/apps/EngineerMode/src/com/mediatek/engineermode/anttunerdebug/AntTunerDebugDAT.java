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
import android.telephony.TelephonyManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Spinner;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;

public class AntTunerDebugDAT extends Activity implements OnClickListener,
        OnCheckedChangeListener {
    public static final String TAG = "AntTunerDebugDAT";

    public static final int OP_DAT_READ = 0;
    public static final int OP_DAT_WRITE1 = 1;
    public static final int OP_DAT_WRITE2 = 2;
    private static final int FEATRUE_IDX = 0;
    private ArrayAdapter<String> adapterPattern = null;
    private Button mBtnDatWrite;
    private Spinner mSpDatIndex;
    private RadioGroup groupDatStatus;
    private boolean enableDat = true;

    private List<String> datIndexList = new ArrayList<String>() {{
        add("0");
        add("1");
        add("2");
        add("3");
        add("4");
        add("5");
        add("6");
        add("7");
    }};
    private int datIndex = 0;
    private String[] cmd;
    private Handler mATHandler = new Handler() {
        private String[] mReturnData = new String[2];

        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            if (msg.what == OP_DAT_READ) {
                if (ar.exception == null) {
                    mReturnData = (String[]) ar.result;
                    Elog.d(TAG, "DAT Index read successful.");
                    Elog.d(TAG, "mReturnData = " + mReturnData[0]);
                } else {
                    EmUtils.showToast("DAT Index read failed.");
                    Elog.d(TAG, "DAT Index read failed.");
                }
            } else if (msg.what == OP_DAT_WRITE1) {
                if (ar.exception == null) {
                    Elog.d(TAG, "DAT Index write successful for slot 1");
                    if (TelephonyManager.getDefault().getPhoneCount() > 1) {
                        sendAtCommand(PhoneConstants.SIM_ID_2, cmd, OP_DAT_WRITE2);
                    }
                } else {
                    EmUtils.showToast("DAT Index write failed for slot 1");
                    Elog.e(TAG, "DAT Index write failed for slot 1");
                }
            } else if (msg.what == OP_DAT_WRITE2) {
                if (ar.exception == null) {
                    Elog.d(TAG, "DAT Index write successful for slot 2");
                } else {
                    EmUtils.showToast("DAT Index write failed for slot 2");
                    Elog.e(TAG, "DAT Index write failed for slot 2");
                }
            }
        }
    };


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ant_tuner_debug_dat);

        mBtnDatWrite = (Button) findViewById(R.id.ant_tuner_debug_dat_write);
        mBtnDatWrite.setOnClickListener(this);
        mSpDatIndex = (Spinner) findViewById(R.id.ant_tuner_debug_dat_index);

        adapterPattern = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, datIndexList);
        adapterPattern
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpDatIndex.setAdapter(adapterPattern);
        mSpDatIndex.setSelection(0);
        mSpDatIndex.setOnItemSelectedListener(new SpinnerSelectedListener());
        groupDatStatus = (RadioGroup) findViewById(R.id.dat_status_rgroup);
        groupDatStatus.setOnCheckedChangeListener(this);
    }

    public String[] prepareDatWriteATCmd() {
        String[] cmd = new String[2];
        if (!enableDat) {
            cmd[0] = "AT+ERFIDX=" + FEATRUE_IDX + "," + -1;
        } else {
            cmd[0] = "AT+ERFIDX=" + FEATRUE_IDX + "," + datIndex;
        }
        cmd[1] = "";
        return cmd;
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        // TODO Auto-generated method stub
        switch (checkedId) {
            case R.id.dat_enable:
                enableDat = true;
                adapterPattern.remove("-1");
                mSpDatIndex.setSelection(datIndex);
                mSpDatIndex.setSelected(true);
                mSpDatIndex.setEnabled(true);
                break;
            case R.id.dat_disable:
                adapterPattern.insert("-1", 0);
                mSpDatIndex.setSelection(0);
                mSpDatIndex.setSelected(false);
                mSpDatIndex.setEnabled(false);
                enableDat = false;
                break;
            default:
                break;
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    public void onClick(View arg0) {
        switch (arg0.getId()) {
            case R.id.ant_tuner_debug_dat_write:
                cmd = prepareDatWriteATCmd();
                sendAtCommand(PhoneConstants.SIM_ID_1, cmd, OP_DAT_WRITE1);
                break;
            default:
                break;
        }
    }

    private void sendAtCommand(int sim_index, String[] command, int msg) {
        Elog.d(TAG, "sendAtCommand() " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(sim_index, command, mATHandler.obtainMessage(msg));
    }


    class SpinnerSelectedListener implements OnItemSelectedListener {
        public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
            datIndex = arg2;
        }

        public void onNothingSelected(AdapterView<?> arg0) {
        }
    }

}
