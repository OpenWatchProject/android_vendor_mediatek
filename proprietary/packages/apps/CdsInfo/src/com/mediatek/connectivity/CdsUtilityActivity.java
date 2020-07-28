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

/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.connectivity;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.ConnectivityManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.net.Authenticator;
import java.net.CookieHandler;
import java.net.CookieManager;
import java.net.CookiePolicy;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.PasswordAuthentication;
import java.net.SocketException;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.List;
import java.util.Map;
import java.util.StringTokenizer;

/**
 * Class for network utiltiy tools.
 *
 */
public class CdsUtilityActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CDSINFO/CdsUtilityActivity";

    private static final String PINGSTRING = "ping -c 5 www.google.com";
    private static final String PINGV6STRING = "ping6 -c 5 www.google.com";

    private ProgressDialog mDialog = null;

    private Context mContext;
    private ProgressThread mProgressThread = null;
    private ArrayAdapter<String> mAutoCompleteAdapter;
    private AutoCompleteTextView mCmdLineList;

    private TextView mOutputScreen = null;
    private Toast mToast;

    private int mCmdOption = 0;
    private static int sPingType;
    private static int sPingCount;
    private static int sPingInterval;
    private static String sPingHost;
    private static String sPingErrMsg;

    private CdsPingService.ServiceBinder mCdsPingService;

    private static final String[] WEBSITES = new String[] {
            "www.google.com"};
    private static final String[] CMDTYPESTRING = new String[] {
        "PING", "PING IPV6"
    };

    private static final String PING_HOST_NAME = "www.google.com";
    private static final String PING = "ping";
    private static final String PING6 = "ping6";

    private static final int PING_OPTION         = 0x1002;
    private static final int PINGV6_OPTION       = 0x1003;

    private static final int BASE          = PING_OPTION;

    private static final int MSG_UPDATE_UI = 0x3001;
    private static final int MSG_RESET_UI =  0x3002;

    private static int checkPingCommandFormat(String pingCmd) {
        String host = PING_HOST_NAME;
        int i = 0;
        int count = 0;
        int interval = 1;

        try {
            StringTokenizer st = new StringTokenizer(pingCmd);
            String token = "";
            sPingErrMsg = "";

            while (st.hasMoreTokens()) {
                token = st.nextToken().trim();
                if (i == 0) {
                   if (!token.equalsIgnoreCase(PING) && !token.equalsIgnoreCase(PING6)) {
                       sPingErrMsg = "Please input command with ping or ping6\n";
                       return -1;
                   }
                } else if (token.equalsIgnoreCase("-t")) {
                   token = st.nextToken().trim();
                   interval = Integer.parseInt(token);
                   if (interval > 10 || interval < 0) {
                       sPingErrMsg = "The ping interval should be 1 ~ 10 seconds\n";
                       return -1;
                   }
                } else if (token.equalsIgnoreCase("-c")) {
                   token = st.nextToken().trim();
                   count = Integer.parseInt(token);
                   if (count < 0 || count > 65536) {
                       sPingErrMsg = "The ping count is 1 ~ 65536\n";
                       return -1;
                   }
                } else {
                   host = token;
                }
                i++;
            }
        } catch (Exception e) {
            e.printStackTrace();
            return -1;
        }

        Log.d(TAG, "Ping command:" + host + " c:" + count + " t:" + interval + "\n");
        sPingHost = host;
        sPingCount = count;
        sPingInterval = interval;
        return 0;
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.cds_network_tool);

        mContext = this.getBaseContext();

        if (mContext == null) {
            Log.e(TAG, "Could not get Conext of this activity");
        }

        mAutoCompleteAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_dropdown_item_1line, WEBSITES);

        mCmdLineList = (AutoCompleteTextView) findViewById(R.id.cmdLine);
        mCmdLineList.setThreshold(3);
        mCmdLineList.setAdapter(mAutoCompleteAdapter);

        mOutputScreen = (TextView) findViewById(R.id.outputText);

        Spinner spinner = (Spinner) findViewById(R.id.cmdSpinnner);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, CMDTYPESTRING);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        spinner.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> arg0, View arg1,
            int position, long arg3) {
                position += BASE;
                if (position == PING_OPTION) {
                    mCmdLineList.setText(PINGSTRING);
                } else if (position == PINGV6_OPTION) {
                    mCmdLineList.setText(PINGV6STRING);
                }
                mCmdOption = position;
                mCmdLineList.requestFocus();
                mCmdLineList.setSelection(mCmdLineList.getText().length());
            }
            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });

        Button button = (Button) findViewById(R.id.runBtn);
        button.setOnClickListener(this);
        button = (Button) findViewById(R.id.stopBtn);
        button.setOnClickListener(this);
        mToast = Toast.makeText(this, null, Toast.LENGTH_SHORT);

        Intent serviceIntent = new Intent(CdsUtilityActivity.this, CdsPingService.class);
        startService(serviceIntent);
        bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);

        sPingType = 0;
        sPingCount = 5;
        sPingInterval = 1;
        sPingHost = PING_HOST_NAME;

        Log.i(TAG, "CdsUtilityActivity is started");
    }

    @Override
    protected void onResume() {
        super.onResume();

    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();

    }

    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();

    }

    /*
     * onClick function for button.
     */
    public void onClick(View v) {
        int buttonId = v.getId();

        Log.d(TAG, "button id:" + buttonId);

        switch (buttonId) {
        case R.id.runBtn:
            handleRunCmd();
            break;
        case R.id.stopBtn:
            handleStopCmd();
            break;
        default:
            break;
        }
    }

    private void handleStopCmd() {
        Log.d(TAG, "handleStopCmd");
        mCdsPingService.finish();
        mOutputScreen.setText("");
    }

    private void handleRunCmd() {

        String cmdStr = mCmdLineList.getText().toString();
        Log.d(TAG, "" + cmdStr);

        if (cmdStr == null || cmdStr.length() == 0) {
            mToast.setText("Please input command");
            mToast.show();
            return;
        } else {
            if (checkPingCommandFormat(cmdStr) < 0) {
                if (sPingErrMsg.length() > 0) {
                    mToast.setText(sPingErrMsg);
                } else {
                    mToast.setText("Input command format is wrong\n");
                }
                mToast.show();
                return;
            }
        }

        if (mCmdOption == PING_OPTION || mCmdOption == PINGV6_OPTION) {
            Log.i(TAG, "Run PING/RUN command");
            if (mHandler.hasMessages(MSG_UPDATE_UI)) {
                mHandler.removeMessages(MSG_UPDATE_UI);
            }
            mHandler.obtainMessage(MSG_RESET_UI).sendToTarget();
            new Thread(new Runnable() {
                public void run() {
                    mProgressThread = new ProgressThread(mHandler);
                    try {
                        mProgressThread.start();
                        mCdsPingService.execCommand(sPingHost, sPingCount,
                            sPingInterval, mCmdOption - PING_OPTION);
                    } catch (Exception e) {
                        e.printStackTrace();
                    } finally {
                        mProgressThread.setState(ProgressThread.STATE_DONE);
                    }
                }
            }).start();
        }
    };

    // Define the Handler that receives messages from the thread and update the
    // progress
    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            String output = "";
            switch (msg.what) {
            case MSG_UPDATE_UI:
                try {
                    output = mCdsPingService.getOutput();
                    mOutputScreen.setText(output);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
            case MSG_RESET_UI:
                handleStopCmd();
                break;
            default:
                break;
            }
        }
    };

    /** Nested class that performs screen update. */
    private class ProgressThread extends Thread {
        Handler mHandler = null;
        private final static int STATE_DONE = 0;
        private final static int STATE_RUNNING = 1;
        private int mState = 0;

        ProgressThread(Handler h) {
            this.mHandler = h;
        }

        public void run() {
            setState(STATE_RUNNING);

            while (STATE_RUNNING == mState) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    Log.e(TAG, "Thread Interrupted");
                }

                if (!mHandler.hasMessages(MSG_UPDATE_UI)) {
                    Message msg = mHandler.obtainMessage();
                    msg.what = MSG_UPDATE_UI;
                    mHandler.sendMessage(msg);
                }
            }
        }

        /**
        * sets the current state for the thread, used to stop the thread.
        */
        public void setState(int state) {
            mState = state;
        }
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceDisconnected(ComponentName name) {
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mCdsPingService = (CdsPingService.ServiceBinder) service;
            Log.d(TAG, "CDS Shell service is connected");
        }
    };
}
