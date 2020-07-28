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


package com.mediatek.engineermode.spc;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

public class SendSpc extends Activity {
    private static final String TAG = "SendSpc";
    private static final int EVENT_AT_CMD_DONE = 1003;
    private static final String INFO_TITLE = "Info.";
    private static final String NA = "N/A";
    private static final String[] CMDLINES = new String[]{"AT+CSPC="};

    private Button mAtBtnCmd;
    private ArrayAdapter<String> mAutoCompleteAdapter;
    private AutoCompleteTextView mCmdLineList;

    private Context mContext;
    private boolean mUserMode = false;

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;

            switch (msg.what) {

                case EVENT_AT_CMD_DONE:
                    ar = (AsyncResult) msg.obj;
                    handleAtCmdResponse(ar);
                    break;
                default:
                    break;

            }
        }
    };
    OnClickListener mSendATCommad = new OnClickListener() {
        public void onClick(View v) {
            String atCmdLine = "";
            String aTCmd[] = new String[3];
            atCmdLine = mCmdLineList.getText().toString();
            Elog.v(TAG, "Execute AT command:" + atCmdLine);
            aTCmd[0] = atCmdLine;
            aTCmd[1] = "";
            aTCmd[2] = "DESTRILD:C2K";
            String[] cmd_s = ModemCategory.getCdmaCmdArr(aTCmd);
            Elog.d(TAG, "mSendATCommad: " + cmd_s[0] + ",cmd_s.length = " + cmd_s.length);
            EmUtils.invokeOemRilRequestStringsEm(true, cmd_s, mHandler.obtainMessage
                    (EVENT_AT_CMD_DONE));
        }
    };

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Intent intent = getIntent();
        mContext = this.getBaseContext();
        setContentView(R.layout.spc_send);

        mAutoCompleteAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_dropdown_item_1line, CMDLINES);

        mCmdLineList = (AutoCompleteTextView) findViewById(R.id.AtComLine);
        mCmdLineList.setThreshold(3);
        mCmdLineList.setAdapter(mAutoCompleteAdapter);
        mCmdLineList.setText("AT+");
        mAtBtnCmd = (Button) findViewById(R.id.Submit);
        mAtBtnCmd.setOnClickListener(mSendATCommad);
        //mUserMode = SystemProperties.get("ro.build.type").equals("user");
    }

    @Override
    protected void onResume() {
        super.onResume();

    }

    @Override
    public void onPause() {
        super.onPause();

        Elog.i(TAG, "[RadioInfo] onPause: unregister phone & data intents");
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        return true;
    }

    void handleAtCmdResponse(AsyncResult ar) {
        if (ar.exception != null) {
            Elog.i(TAG, "The response of command is failed");
            showInfo("AT command is failed to send");
        } else {
            try {
                String[] rawData = (String[]) ar.result;
                Elog.i(TAG, "ar.result:" + rawData);
                Elog.i(TAG, "The resopnse is " + rawData[0]);
                showInfo("AT command is sent:" + rawData[0]);
            } catch (NullPointerException e) {
                showInfo("Something is wrong");
                e.printStackTrace();
            }
        }
    }

    private void showInfo(String info) {
        if (isFinishing()) {
            return;
        }
        AlertDialog.Builder infoDialog = new AlertDialog.Builder(this);
        infoDialog.setTitle(INFO_TITLE);
        infoDialog.setMessage(info);
        infoDialog.setIcon(android.R.drawable.ic_dialog_alert);
        infoDialog.show();
    }
}
