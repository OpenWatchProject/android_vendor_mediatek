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
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Do BT BLE test mode.
 * @author mtk54040
 */

public class BleTestMode extends Activity implements OnClickListener {

    private BluetoothAdapter mBtAdapter = null;

    private static final String TAG = "BleTestMode";

    // views in this activity
    // button
    private Button mBtnStart = null;
    private Button mBtnStop = null;

    private TextView mTvResult = null;
    private String mResultStr = "R:";

    // Radio Button
    // BLE test mode Tx/Rx RadioGroup
    private RadioButton mRbTx = null;
    private RadioButton mRbRx = null;
    // BLE test mode Hopping/Single RadioGroup
    private RadioButton mRbHopping = null;
    private RadioButton mRbSingle = null;
    private static final int CHANNEL_NUM = 40;
    // Checkbox
    private CheckBox mCbContinune = null;

    // Spinner
    private Spinner mSpChannel = null;
    private Spinner mSpPattern = null;

    // spinner value
    private byte mChannelValue = 0x00;
    private byte mPatternValue = 0x00;

    // btn values
    private boolean mTxTest = true;

    private static final int RETURN_SUCCESS = 0;

    // jni layer object
    private BtTest mBtTest = null;

    // BtTest object init and start test flag
    private boolean mBtInited = false;
    private boolean mTestStarted = false;

    private boolean mIniting = false;
    // Dialog ID
    private static final int DLG_CHECK_STOP = 2;
    private static final int DLG_CHECK_BT_DEVEICE = 3;
    // Message ID
    private static final int MSG_TEST_START = 11;
    private static final int MSG_TEST_STOP = 12;
    // execute result
    private static final int TEST_SUCCESS = 13;
    private static final int TEST_FAILED = 14;
    private static final int STOP_FINISH = 15;
    // activity exit message ID
    private static final int MSG_ACTIVITY_EXIT = 20;

    private WorkHandler mWorkHandler = null;
    private HandlerThread mWorkThread = null;

    @Override
    protected void onCreate(Bundle onSavedInstanceState) {
        super.onCreate(onSavedInstanceState);
        setContentView(R.layout.ble_test_mode);

        // Initialize UI component
        mBtnStart = (Button) findViewById(R.id.BLEStart);
        mBtnStop = (Button) findViewById(R.id.BLEStop);
        mTvResult = (TextView) findViewById(R.id.BLEResult_Text);

        mRbTx = (RadioButton) findViewById(R.id.BLETestModeTx);
        mRbRx = (RadioButton) findViewById(R.id.BLETestModeRx);

        mRbHopping = (RadioButton) findViewById(R.id.BLEHopping);
        mRbSingle = (RadioButton) findViewById(R.id.BLESingle);

        mCbContinune = (CheckBox) findViewById(R.id.BLEContiniousTx);

        mSpChannel = (Spinner) findViewById(R.id.BLEChannelSpinner);
        mSpPattern = (Spinner) findViewById(R.id.BLEPatternSpinner);

        mBtnStart.setOnClickListener(this);
        mBtnStop.setOnClickListener(this);

        mRbTx.setChecked(true);
        mTxTest = true;

        mRbTx.setOnClickListener(this);
        mRbRx.setOnClickListener(this);

        mRbSingle.setChecked(true);
        mRbSingle.setOnClickListener(this);
        mRbHopping.setOnClickListener(this);

        // Fill "channel  spinner" content and action handler set
        ArrayAdapter<String> mSpnChannelAdapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_spinner_item);
        mSpnChannelAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        for (int i = 0; i < CHANNEL_NUM; i++) {
            mSpnChannelAdapter
                    .add(getString(R.string.BT_ble_test_channnel) + i);
        }

        mSpChannel.setAdapter(mSpnChannelAdapter);
        mSpChannel.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                Elog.i(TAG, "mSpChannel item " + arg2);
                mChannelValue = (byte) arg2;
            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });

        // Fill "pattern  " content and action handler set
        ArrayAdapter<String[]> mSpnPatternAdapter = new ArrayAdapter<String[]>(
                this, android.R.layout.simple_spinner_item);
        mSpnPatternAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpnPatternAdapter.add(getResources().getStringArray(
                R.array.bt_ble_test_pattern));

        mSpPattern.setOnItemSelectedListener(new OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                Elog.i(TAG, "mSpPattern item " + arg2);
                mPatternValue = (byte) arg2;
            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });

        setViewState(false);

        mWorkThread = new HandlerThread(TAG);
        mWorkThread.start();

        Looper looper = mWorkThread.getLooper();
        mWorkHandler = new WorkHandler(looper);

    }

    // UI thread's handler
    private Handler mUiHandler = new Handler() {
        public void handleMessage(Message msg) {
            Elog.i(TAG, "receive msg of " + msg.what);
            mTvResult.setText(mResultStr);
            switch (msg.what) {
            case TEST_SUCCESS:
                mTestStarted = true;
                break;
            case TEST_FAILED:
                // here we can give some notification
                mTestStarted = false;
                setViewState(false);
                break;
            case STOP_FINISH:
                mTestStarted = false;
                setViewState(false);
                removeDialog(DLG_CHECK_STOP);
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected void onResume() {
        super.onResume();
        if (mBtAdapter == null) {
            mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        }

        if (mBtAdapter == null) {
            showDialog(DLG_CHECK_BT_DEVEICE);
        } else {
            if (!BtTest.checkInitState(mBtAdapter, this)) {
                finish();
            }
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DLG_CHECK_STOP) {
            ProgressDialog dialog = new ProgressDialog(this);

            dialog.setMessage(getString(R.string.BT_init_dev));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);

            return dialog;
        } else if (id == DLG_CHECK_BT_DEVEICE) {
            AlertDialog dialog = new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.Error)
                .setMessage(getString(R.string.BT_no_dev)) // put in strings.xml
                .setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int which) {
                            finish();
                        }
                    }).create();
            return dialog;
        }

        return null;
    }

    @Override
    protected void onDestroy() {
        mWorkHandler.sendEmptyMessage(MSG_ACTIVITY_EXIT);
        if (mWorkThread != null) {
            mWorkThread.quitSafely();
        }
        super.onDestroy();

    }

    /**
     * If pressing "Start" button Tx: 01 1E 20 03 XX 25 YY //HCI LE Transmitter
     * Test CMD XX is based on Channel selection YY is based on Pattern
     * selection Rx: 04 0E 04 01 1E 20 00 //HCI Command Complete Event.
     */
    private boolean handleTxTestStart() {

        int cmdLen = 7;
        char[] cmd = new char[cmdLen];
        int i = 0;
        cmd[0] = 0x01;
        cmd[1] = 0x1E;
        cmd[2] = 0x20;
        cmd[3] = 0x03;
        cmd[4] = (char) mChannelValue;
        cmd[5] = 0x25;
        cmd[6] = (char) mPatternValue;

        mBtTest.hciCommandRun(cmd, cmdLen);

        return true;
    }

    /* BLE test mode test */

    /**
     * If pressing "Start" button Tx: 01 1D 20 01 ZZ //HCI LE Receiver Test CMD
     * ZZ is based on Channel selection Rx: 04 0E 04 01 1D 20 00
     * //HCI Command Complete Event.
     */
    private boolean handleRxTestStart() {
        Elog.v(TAG, "handleRxTestStart");
        char[] cmd = new char[5];
        cmd[0] = 0x01;
        cmd[1] = 0x1D;
        cmd[2] = 0x20;
        cmd[3] = 0x01;
        cmd[4] = (char) mChannelValue;
        mBtTest.hciCommandRun(cmd, cmd.length);
        return true;
    }

    /**
     * If pressing "Stop" button Tx: 01 1F 20 00 //HCI LE Test End CMD For Rx,
     * we have two cases of HCI Command Complete Event Case A) Rx: 04 0E 0A 01
     * 1F 20 00 BB AA ?? ?? ?? ?? Case B) Rx: 04 0E 06 01 1F 20 00 BB AA ??
     * means do not care Packet Count = 0xAABB
     */
    private void handleTestStop() {
        Elog.v(TAG, "handleRxTestStop");
        char[] response = sendTestStopCmd();
        if (response != null) {
            // Response format: 04 0e 0a/06 01 1f 20 00 BB AA 00 00...
            // packet count = 0xAABB
            if (!mTxTest) {
                mResultStr = String.format("***Packet Count: %d",
                        (long) response[8] * 256 + (long) response[7]);
            }

        }
    }

    private char[] sendTestStopCmd() {
        char[] cmd = {0x01, 0x1F, 0x20, 0x00};
        return mBtTest.hciCommandRun(cmd, cmd.length);
    }

    /**
     * Do test after push "start" button.
     *
     * @return 0 success
     */
    private boolean handleStartBtnClick() {
        Elog.v(TAG, "handleStartBtnClick");
        if (!initBtTestOjbect()) {
            return false;
        }
        if (mTxTest) {
            return handleTxTestStart();
        } else {
            return handleRxTestStart();
        }
    }


    /**
     * stop test.
     */
    private void handleStopBtnClick() {
        Elog.v(TAG, "handleStopBtnClick");
        handleTestStop();
        mTestStarted = false;
        uninitBtTestOjbect();
    }

    @Override
    public void onClick(View v) {

        if (v.equals(mBtnStart)) {
            setViewState(true);
            mWorkHandler.sendEmptyMessage(MSG_TEST_START);

        } else if (v.equals(mBtnStop)) {
            // Rx test button is clicked
            mBtnStop.setEnabled(false);
            showDialog(DLG_CHECK_STOP);
            mWorkHandler.sendEmptyMessage(MSG_TEST_STOP);
        } else if (v.equals(mRbRx)) {
            // Rx test button is clicked
            mTxTest = false;

        } else if (v.equals(mRbTx)) {
            // Rx test button is clicked
            mTxTest = true;

        }
    }

    /**
     * set view to state (true/false --> enable(stop btn pressed) /
     * disable(start btn pressed)).
     *
     * @param state
     */
    private void setViewState(boolean state) {

        mRbTx.setEnabled(!state);
        mRbRx.setEnabled(!state);
        mRbHopping.setEnabled(false);
        mRbSingle.setEnabled(!state);
        mCbContinune.setEnabled(!state);
        mSpChannel.setEnabled(!state);
        mSpPattern.setEnabled(!state);

        mBtnStart.setEnabled(!state);
        mBtnStop.setEnabled(state);
    }

    /**
     * Init BtTest -call init function of BtTest.
     *
     * @return true if success.
     */
    private boolean initBtTestOjbect() {
        Elog.v(TAG, "initBtTestOjbect");
        if (mIniting) {
            return false;
        }
        if (mBtInited) {
            return mBtInited;
        }
        if (mBtTest == null) {
            mBtTest = new BtTest();
        }
        if (mBtTest != null && !mBtInited) {
            mIniting = true;
            mBtInited = (mBtTest.init() == RETURN_SUCCESS);
            Elog.i(TAG, "mBtTest init:" + mBtInited);
            mIniting = false;
        }

        return mBtInited;
    }

    /**
     * Clear BtTest object -call deInit function of BtTest.
     *
     * @return true if success.
     */
    private boolean uninitBtTestOjbect() {
        Elog.v(TAG, "uninitBtTestOjbect");
        if (mBtTest.unInit() != RETURN_SUCCESS) {
            Elog.e(TAG, "mBT un-initialization failed");
        }
        mBtTest = null;
        mBtInited = false;
        mTestStarted = false;
        return true;
    }

    /**
     * Deal with function request.
     *
     * @author mtk54040
     *
     */
    private final class WorkHandler extends Handler {

        /**
         * @param looper
         */
        private WorkHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_TEST_START:
                if (handleStartBtnClick()) {
                    mUiHandler.sendEmptyMessage(TEST_SUCCESS);
                }
                if (!mBtInited) {
                    mUiHandler.sendEmptyMessage(TEST_FAILED);
                }
                break;
            case MSG_TEST_STOP:
                if (mBtTest != null && mBtInited) {
                    handleStopBtnClick();
                }
                mUiHandler.sendEmptyMessage(STOP_FINISH);
                break;
            case MSG_ACTIVITY_EXIT:
                if (mBtTest != null && mBtInited) {
                    sendTestStopCmd();
                    uninitBtTestOjbect();
                }

                break;
            default:
                break;
            }
        }
    }
}
