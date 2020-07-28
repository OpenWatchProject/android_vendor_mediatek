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

package com.mediatek.engineermode.bluetooth;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Do bluetooth no singal rx test.
 *
 * @author mtk54040
 *
 */
public class NoSigRxTestActivity extends Activity implements
        DialogInterface.OnCancelListener, OnClickListener {
    private static final String TAG = "BtNoSigRxTest";

    private static final int OP_BT_SEND = 0;
    private static final int OP_BT_STOP = 1;

    private static final int MSG_UI_BT_CLOSE = 5;
    private static final int MSG_UI_BT_CLOSE_FINISHED = 6;

    private static final int MSG_OP_IN_PROCESS = 8;
    private static final int MSG_OP_FINISH = 9;
    private static final int MSG_OP_RX_FAIL = 10;
    private static final int MSG_OP_ADDR_DEFAULT = 11;
    private static final int MSG_OP_TEST_OK_STEP1 = 12;
    private static final int MSG_OP_TEST_OK_STEP2 = 13;

    private static final int DLG_RX_FAIL = 21;
    private static final int DLG_RX_TEST = 22;
    private static final int DLG_BT_STOP = 23;

    private static final int TEST_STATUS_BEGIN = 100;
    private static final int TEST_STATUS_RESULT = 101;

    private BluetoothAdapter mBtAdapter;

    private Spinner mSpPattern;
    private Spinner mSpPocketType;
    private EditText mEdFreq;
    private EditText mEdAddr;

    private Button mBtnStartTest;

    private TextView mTvPackCnt;
    private TextView mTvPackErrRate;
    private TextView mTvRxByteCnt;
    private TextView mTvBitErrRate;

    private int[] mResult = null; //
    private int mTestStatus = TEST_STATUS_BEGIN;
    private int mStateBt;

    // used to record if the button clicked
    private boolean mDoneFinished = true;
    private boolean mDumpStart = false;
    // used to handle the button clicked action
    private Handler mWorkHandler = null;
    private HandlerThread mWorkThread = null;

    private BtTest mBtTest = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.rx_nosig_test);
        setValuesSpinner();
        mEdFreq = (EditText) findViewById(R.id.NSRX_editFrequency);
        mEdAddr = (EditText) findViewById(R.id.NSRX_editTesterAddr);
        mBtnStartTest = (Button) findViewById(R.id.NSRX_StartTest);
        mTvPackCnt = (TextView) findViewById(R.id.NSRX_StrPackCnt);
        mTvPackErrRate = (TextView) findViewById(R.id.NSRX_StrPackErrRate);
        mTvRxByteCnt = (TextView) findViewById(R.id.NSRX_StrPackByteCnt);
        mTvBitErrRate = (TextView) findViewById(R.id.NSRX_StrBitErrRate);

        mBtnStartTest.setOnClickListener(this);

        mWorkThread = new HandlerThread(TAG);
        mWorkThread.start();

        Looper looper = mWorkThread.getLooper();
        mWorkHandler = new WorkHandler(looper);

    }

    private Handler mUiHandler = new Handler() {
        public void handleMessage(Message msg) {
            Elog.i(TAG, "receive msg of " + msg.what);
            switch (msg.what) {
            case MSG_OP_IN_PROCESS:
                showDialog(DLG_RX_TEST);
                break;
            case MSG_OP_FINISH:
                dismissDialog(DLG_RX_TEST);
                break;
            case MSG_OP_RX_FAIL:
                showDialog(DLG_RX_FAIL);
                break;
            case MSG_OP_ADDR_DEFAULT:
                mEdAddr.setText("A5F0C3");
                break;
            case MSG_OP_TEST_OK_STEP1:
                mTestStatus = TEST_STATUS_RESULT;
                mBtnStartTest.setText("End Test");
                break;
            case MSG_OP_TEST_OK_STEP2:
                mTvPackCnt.setText(String.valueOf(mResult[0]));
                mTvPackErrRate.setText(String.format("%.2f",
                        mResult[1] / 100.0)
                        + "%");
                mTvRxByteCnt.setText(String.valueOf(mResult[2]));
                mTvBitErrRate.setText(String.format("%.2f",
                        mResult[3] / 100.0)
                        + "%");
                mTestStatus = TEST_STATUS_BEGIN;
                mBtnStartTest.setText("Start");
                break;
            case MSG_UI_BT_CLOSE:
                showDialog(DLG_BT_STOP);
                break;
            case MSG_UI_BT_CLOSE_FINISHED:
                removeDialog(DLG_BT_STOP);
                finish();
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DLG_RX_FAIL) {
            AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                    false).setTitle(R.string.Error)
                    .setMessage(R.string.BT_no_sig_rx_fail)
                    .setPositiveButton(R.string.OK,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog,
                                        int which) {
                                }
                            }).create();
            return dialog;
        } else if (id == DLG_RX_TEST) {
            ProgressDialog dialog = new ProgressDialog(NoSigRxTestActivity.this);
            dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
            dialog.setMessage(getString(R.string.bt_rx_ongoing));
            dialog.setTitle(R.string.BTRxTitle);
            dialog.setCancelable(false);
            return dialog;
        } else if (id == DLG_BT_STOP) {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage(getString(R.string.BT_deinit));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            return dialog;
        }
        return null;
    }

    @Override
    public void onBackPressed() {
        if (mBtTest != null) {
            mWorkHandler.sendEmptyMessage(OP_BT_STOP);
        } else {
            super.onBackPressed();
        }
    }

    @Override
    protected void onDestroy() {
        if (mWorkThread != null) {
            mWorkThread.quit();
        }
        super.onDestroy();
    }

    /**
     * Handler for message.
     *
     */
    private final class WorkHandler extends Handler {
        private WorkHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg.what == OP_BT_SEND) {
                mUiHandler.sendEmptyMessage(MSG_OP_IN_PROCESS);
                mDoneFinished = false;
                doSendCommandAction();
                mDoneFinished = true;
                mUiHandler.sendEmptyMessage(MSG_OP_FINISH);
            } else if (msg.what == OP_BT_STOP) {
                mUiHandler.sendEmptyMessage(MSG_UI_BT_CLOSE);
                // do stop
                if (mDumpStart) {
                    if (mBtTest != null) {
                        mBtTest.noSigRxTestResult();
                        mDumpStart = false;
                    }
                }
                mBtTest = null;
                mUiHandler.sendEmptyMessage(MSG_UI_BT_CLOSE_FINISHED);
            }
        }
    }

    protected void setValuesSpinner() {
        // for TX pattern
        mSpPattern = (Spinner) findViewById(R.id.NSRX_PatternSpinner);
        ArrayAdapter<CharSequence> adapterPattern = ArrayAdapter
                .createFromResource(this, R.array.nsrx_pattern,
                        android.R.layout.simple_spinner_item);
        adapterPattern
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpPattern.setAdapter(adapterPattern);

        // for TX pocket type
        mSpPocketType = (Spinner) findViewById(R.id.NSRX_PocketTypeSpinner);
        ArrayAdapter<CharSequence> adapterPocketType = ArrayAdapter
                .createFromResource(this, R.array.nsrx_Pocket_type,
                        android.R.layout.simple_spinner_item);
        adapterPocketType
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpPocketType.setAdapter(adapterPocketType);
    }

    @Override
    public void onClick(View arg0) {
        Elog.i(TAG, "mDoneFinished:" + mDoneFinished);
        if (mDoneFinished) {
            mWorkHandler.sendEmptyMessage(OP_BT_SEND);
        }
    }

    /**
     * Send command the user has made, and finish the activity.
     */
    private boolean doSendCommandAction() {
        if (mTestStatus == TEST_STATUS_BEGIN) {
            getBtState();
            enableBluetooth(false);
            getValuesAndSend();

        } else if (mTestStatus == TEST_STATUS_RESULT) {
            getResult();
        }

        return true;
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        // request that the service stop the query with this callback object.
        finish();
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (mBtAdapter == null) {
            mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        }
        if (!BtTest.checkInitState(mBtAdapter, this)) {
            finish();
        }

    }

    private void getBtState() {
        mStateBt = mBtAdapter.getState();
    }

    private void enableBluetooth(boolean enable) {
        Elog.i(TAG, "Bluetooth to enabled:" + enable);
        if (enable) {
            mBtAdapter.enable();
        } else {
            mBtAdapter.disable();
        }
    }

    private void getValuesAndSend() {
        mBtTest = null;
        int nPatternIdx = mSpPattern.getSelectedItemPosition();
        int nPocketTypeIdx = mSpPocketType.getSelectedItemPosition();
        int nFreq = 0;
        int nAddress = 0;
        try {
            nFreq = Integer.valueOf(mEdFreq.getText().toString());
            long longAdd = Long.valueOf(mEdAddr.getText().toString(), 16);
            nAddress = (int) longAdd;
            if (nFreq < 0 || nFreq > 78) {
                mUiHandler.sendEmptyMessage(MSG_OP_RX_FAIL);
                return;
            }
            if (nAddress == 0) {
                nAddress = 0xA5F0C3;
                mUiHandler.sendEmptyMessage(MSG_OP_ADDR_DEFAULT);
            }
        } catch (NumberFormatException e) {
            Elog.e(TAG, e.getMessage());
            return;
        }

        mBtTest = new BtTest();
        // send command to....
        boolean rc = mBtTest.noSigRxTestStart(nPatternIdx, nPocketTypeIdx,
                nFreq, nAddress);
        if (rc) {
            mDumpStart = true;
            mUiHandler.sendEmptyMessage(MSG_OP_TEST_OK_STEP1);
        } else {
            Elog.e(TAG, "no signal rx test failed.");
            if ((BluetoothAdapter.STATE_TURNING_ON == mStateBt)
                    || (BluetoothAdapter.STATE_ON == mStateBt)) {
                enableBluetooth(true);
            }
            mUiHandler.sendEmptyMessage(MSG_OP_RX_FAIL);
        }
    }

    private void getResult() {
        if (mBtTest == null) {
            return;
        }

        mResult = mBtTest.noSigRxTestResult();

        if (mResult == null) {
            Elog.e(TAG, "no signal rx test failed.");
            if ((BluetoothAdapter.STATE_TURNING_ON == mStateBt)
                    || (BluetoothAdapter.STATE_ON == mStateBt)) {
                enableBluetooth(true);
            }
            mUiHandler.sendEmptyMessage(MSG_OP_RX_FAIL);
        } else {
            mDumpStart = false;
            mUiHandler.sendEmptyMessage(MSG_OP_TEST_OK_STEP2);
        }

    }

}
