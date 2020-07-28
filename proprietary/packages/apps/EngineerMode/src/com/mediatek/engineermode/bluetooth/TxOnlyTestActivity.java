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
import android.app.Dialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.Arrays;


/**
 * Do BT tx mode test.
 *
 * @author mtk54040
 *
 */
public class TxOnlyTestActivity extends Activity implements
        DialogInterface.OnCancelListener {

    private static final String TAG = "BtTxOnlyTest";

    // dialog ID and MSG ID
    private static final int DLG_BT_INIT = 3;
    private static final int DLG_BT_DEINIT = 4;
    private static final int DLG_QUERY_CHANNEL = 5;

    //Msg ID
    private static final int MSG_OP_IN_PROCESS = 2;
    private static final int MSG_OP_FINISH = 0;
    private static final int MSG_OP_TX_FAIL = 4;
    private static final int MSG_UI_BT_CLOSE = 5;
    private static final int MSG_UI_BT_CLOSE_FINISHED = 6;
    private static final int MSG_OP_BT_SEND = 11;
    private static final int MSG_OP_BT_STOP = 12;

    private static final int MAP_TO_PATTERN = 0;
    private static final int MAP_TO_CHANNELS = 1;
    private static final int MAP_TO_POCKET_TYPE = 2;
    private static final int MAP_TO_FREQ = 3;
    private static final int MAP_TO_POCKET_TYPE_LEN = 4;

    private static final int BT_TEST_0 = 0;
    private static final int BT_TEST_3 = 3;
    private static final int RETURN_FAIL = -1;

    private BluetoothAdapter mAdapter;

    private Spinner mSpPattern = null;
    private Spinner mSpChannels = null;
    private Spinner mSpPktTypes = null;

    private BtTest mBtTest = null;
    private boolean mHasInit = false;
    private boolean mIniting = false;
    private int mStateBt;
    private Handler mWorkHandler = null; // used to handle the button clicked action
    private HandlerThread mWorkThread = null;

    private boolean mDoneTest = true; // used to record if the "done" button clicked
    private boolean mDumpStart = false;

    private static final char[] CHANNEL_HOP_20 = {0x00, 0x01, 0x02};
    private static final char[] SUPPORT_RESPONSE = {0x04, 0x0E, 0x04, 0x01, 0x90, 0xFD, 0x00};

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.bt_tx_only_test);
        // Initialize the UI component
        setValuesSpinner();
        mWorkThread = new HandlerThread(TAG);
        mWorkThread.start();
        Looper looper = mWorkThread.getLooper();
        mWorkHandler = new WorkHandler(looper);
        mBtTest = new BtTest();
        new FuncTask().execute();

    }

    /**
     * AsyncTask to query channels supported.
     *
     */
    private class FuncTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected void onPreExecute() {
            // TODO Auto-generated method stub
            super.onPreExecute();
            showDialog(DLG_QUERY_CHANNEL);

        }

        @Override
        protected Boolean doInBackground(Void... arg0) {
            // TODO Auto-generated method stub
            initBtTestOjbect();
            boolean result = true;
            for (char value:CHANNEL_HOP_20) {
                char[] cmd = {0x01, 0x90, 0xFD, 0x01, value};
                char[] res = mBtTest.hciCommandRun(cmd, cmd.length);
                if ((res.length != SUPPORT_RESPONSE.length)
                        || (!Arrays.equals(SUPPORT_RESPONSE, res))) {
                    result = false;
                    break;
                }
            }
            uninitBtTestOjbect();
            return result;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            // TODO Auto-generated method stub
            // for TX channels

            mSpChannels = (Spinner) findViewById(R.id.ChannelsSpinner);
            Resources res = getResources();
            ArrayList<String> arrayCh = new ArrayList<String>();
            arrayCh.addAll(Arrays.asList(res.getStringArray(R.array.bt_tx_channels)));
            if (result) {
                arrayCh.addAll(Arrays.asList(res.getStringArray(R.array.bt_tx_channels_20)));
            }

            ArrayAdapter<String> adapterChannels = new ArrayAdapter<String>(
                    TxOnlyTestActivity.this, android.R.layout.simple_spinner_item, arrayCh);
            adapterChannels
                    .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mSpChannels.setAdapter(adapterChannels);
            removeDialog(DLG_QUERY_CHANNEL);
            super.onPostExecute(result);
        }
    }

    private Handler mUiHandler = new Handler() {
        public void handleMessage(Message msg) {
            Elog.i(TAG, "receive msg of " + msg.what);
            switch (msg.what) {
            case MSG_OP_IN_PROCESS:
                showDialog(DLG_BT_INIT);
                break;
            case MSG_OP_FINISH:
                removeDialog(DLG_BT_INIT);
                break;
            case MSG_OP_TX_FAIL:
                removeDialog(DLG_BT_INIT);
                break;
            case MSG_UI_BT_CLOSE:
                showDialog(DLG_BT_DEINIT);
                break;
            case MSG_UI_BT_CLOSE_FINISHED:
                removeDialog(DLG_BT_DEINIT);
                finish();
                break;
            default:
                break;
            }
        }
    };

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
            if (msg.what == MSG_OP_BT_SEND) {
                mUiHandler.sendEmptyMessage(MSG_OP_IN_PROCESS);
                mDoneTest = false;
                // do stop
                if (mDumpStart) {
                    if (mBtTest != null) {
                        runHCIResetCmd();
                        if (RETURN_FAIL == mBtTest.doBtTest(BT_TEST_3)) { // bt deinit
                            Elog.e(TAG, "stop failed.");
                        }
                        mHasInit = false;
                    }
                }
                doSendCommandAction();
                // do start
                if (mBtTest != null && mHasInit) {
                    mDumpStart = true;
                }
                mDoneTest = true;
                mUiHandler.sendEmptyMessage(MSG_OP_FINISH);
            } else if (msg.what == MSG_OP_BT_STOP) {
                mUiHandler.sendEmptyMessage(MSG_UI_BT_CLOSE);
                runHCIResetCmd();
                // do stop
                if (RETURN_FAIL == mBtTest.doBtTest(BT_TEST_3)) {
                    Elog.e(TAG, "stop failed.");
                }
                mHasInit = false;
                mBtTest = null;
                mUiHandler.sendEmptyMessage(MSG_UI_BT_CLOSE_FINISHED);
            }
        }
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem doneItem = menu.getItem(Menu.FIRST - 1);
        if (null != doneItem) {
            if (!mDoneTest) {
                doneItem.setEnabled(false);
                menu.close();
            } else {
                doneItem.setEnabled(true);
            }
        }
        return true;
    }

    /**
     * Initialize the UI component.
     *
     *
     */
    private void setValuesSpinner() {
        // for TX pattern
        mSpPattern = (Spinner) findViewById(R.id.PatternSpinner);
        ArrayAdapter<CharSequence> adapterPattern = ArrayAdapter
                .createFromResource(this, R.array.tx_pattern,
                        android.R.layout.simple_spinner_item);
        adapterPattern
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpPattern.setAdapter(adapterPattern);



        // for TX pocket type
        mSpPktTypes = (Spinner) findViewById(R.id.PocketTypeSpinner);
        ArrayAdapter<CharSequence> adapterPocketType = ArrayAdapter
                .createFromResource(this, R.array.tx_Pocket_type,
                        android.R.layout.simple_spinner_item);
        adapterPocketType
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpPktTypes.setAdapter(adapterPocketType);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);

        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu_show, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.menu_done:
            Elog.i(TAG, "mDoneTest:" + mDoneTest);
            if (mDoneTest) {
                mWorkHandler.sendEmptyMessage(MSG_OP_BT_SEND);
            }
            return true;

        case R.id.menu_discard:
            return doRevertAction();
        default:
            break;
        }
        return false;
    }

    /**
     * Send command the user has made, and finish the activity.
     */
    private boolean doSendCommandAction() {
        getBtState();
        enableBluetooth(false);
        getValuesAndSend();
        return true;
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        // request that the service stop the query with this callback
        // mBtTestect.
        finish();
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (mAdapter == null) {
            mAdapter = BluetoothAdapter.getDefaultAdapter();
        }
        if (!BtTest.checkInitState(mAdapter, this)) {
            finish();
        }
    }


    @Override
    public void onBackPressed() {
        removeDialog(DLG_BT_INIT);
        if (mBtTest != null) {
            mWorkHandler.sendEmptyMessage(MSG_OP_BT_STOP);
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

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DLG_BT_INIT) {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage(getString(R.string.BT_init_dev));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            return dialog;
        } else if (id == DLG_BT_DEINIT) {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage(getString(R.string.BT_deinit));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            return dialog;
        } else if (id == DLG_QUERY_CHANNEL) {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage(getString(R.string.bt_query_channel));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            return dialog;
        }
        return null;
    }

    private void getBtState() {
        mStateBt = mAdapter.getState();
    }

    private void enableBluetooth(boolean enable) {
        Elog.i(TAG, "Bluetooth to enabled:" + enable);
        if (enable) {
            mAdapter.enable();
        } else {
            mAdapter.disable();
        }
    }

    /**
     * Revert any changes the user has made, and finish the activity.
     */
    private boolean doRevertAction() {
        onBackPressed();
        return true;
    }


    private void getValuesAndSend() {

        getSpinnerValue(mSpPattern, MAP_TO_PATTERN);
        getSpinnerValue(mSpChannels, MAP_TO_CHANNELS);
        getSpinnerValue(mSpPktTypes, MAP_TO_POCKET_TYPE);

        getEditBoxValue(R.id.edtFrequency, MAP_TO_FREQ);
        getEditBoxValue(R.id.edtPocketLength, MAP_TO_POCKET_TYPE_LEN);

        Elog.i(TAG, "PocketType:" + mBtTest.getPocketType() + " Frequency:" + mBtTest.getFreq());
        if (27 == mBtTest.getPocketType()) {
            if (initBtTestOjbect()) {
                handleNonModulated();
            }

        } else {
            if (RETURN_FAIL == mBtTest.doBtTest(BT_TEST_0)) {
                Elog.e(TAG, "transmit data failed.");
                if ((BluetoothAdapter.STATE_TURNING_ON == mStateBt)
                        || (BluetoothAdapter.STATE_ON == mStateBt)) {
                    enableBluetooth(true);
                }

                mUiHandler.sendEmptyMessage(MSG_OP_TX_FAIL);
                mHasInit = false;
            } else {
                mHasInit = true;
            }
        }
    }

    private void handleNonModulated() {
        /*
         * If pressing "Stop" button Tx: 01 0C 20 02 00 PP 0xPP = Filter
         * Duplicate (00 = Disable Duplicate Filtering, 01 = Enable Duplicate
         * Filtering) Rx: 04 0E 04 01 0C 20 00
         */

        /*
         * TX: 01 15 FC 01 00 RX: 04 0E 04 01 15 FC 00 TX: 01 D5 FC 01 XX (XX =
         * Channel) RX: 04 0E 04 01 D5 FC 00
         */
        int cmdLen = 5;
        char[] cmd = new char[cmdLen];

        int i = 0;
        cmd[0] = 0x01;
        cmd[1] = 0x15;
        cmd[2] = 0xFC;
        cmd[3] = 0x01;
        cmd[4] = 0x00;
        mBtTest.hciCommandRun(cmd, cmdLen);

        cmdLen = 5;
        cmd[0] = 0x01;
        cmd[1] = 0xD5;
        cmd[2] = 0xFC;
        cmd[3] = 0x01;
        cmd[4] = (char) mBtTest.getFreq();
        mBtTest.hciCommandRun(cmd, cmdLen);
    }

    // init BtTest -call init function of BtTest
    private boolean initBtTestOjbect() {
        Elog.v(TAG, "initBtTestOjbect");
        if (mIniting) {
            return false;
        }
        if (mHasInit) {
            return mHasInit;
        }

        if (mBtTest != null && !mHasInit) {
            mIniting = true;
            mHasInit = (mBtTest.init() == 0);
            Elog.i(TAG, "mBtTest init:" + mHasInit);
            mIniting = false;
        }

        return mHasInit;
    }

    private void uninitBtTestOjbect() {
        if (mHasInit) {
            mBtTest.unInit();
            mHasInit = false;
        }
    }

    // run "HCI Reset" command
    private void runHCIResetCmd() {
        /*
         * If pressing "HCI Reset" button Tx: 01 03 0C 00 Rx: 04 0E 04 01 03 0C
         * 00 After pressing "HCI Reset" button, all state will also be reset
         */
        int cmdLen = 4;
        char[] cmd = new char[cmdLen];

        int i = 0;
        cmd[0] = 0x01;
        cmd[1] = 0x03;
        cmd[2] = 0x0C;
        cmd[3] = 0x00;
        if (mBtTest == null) {
            mBtTest = new BtTest();
        }
        mBtTest.hciCommandRun(cmd, cmdLen);
    }

    private boolean getSpinnerValue(Spinner sSpinner, int flag) {
        boolean bSuccess = false;
        int index = sSpinner.getSelectedItemPosition();
        if (0 > index) {
            return bSuccess;
        }

        switch (flag) {
        case MAP_TO_PATTERN: // Pattern
            mBtTest.setPatter(index);
            break;
        case MAP_TO_CHANNELS: // Channels
            mBtTest.setChannels(index);
            break;
        case MAP_TO_POCKET_TYPE: // Pocket type
            mBtTest.setPocketType(index);
            break;
        default:
            bSuccess = false;
            break;
        }
        bSuccess = true;
        return bSuccess;
    }

    private boolean getEditBoxValue(int id, int flag) {
        boolean bSuccess = false;

        TextView text = (TextView) findViewById(id);
        String str = null;
        if (null != text) {
            str = text.getText().toString();
        }
        if ((null == str) || str.equals("")) {
            return bSuccess;
        }
        int iLen = 0;
        try {
            iLen = Integer.parseInt(str);
        } catch (NumberFormatException e) {
            Elog.e(TAG, e.getMessage());
            return bSuccess;
        }
        // frequency
        if (MAP_TO_FREQ == flag) {
            mBtTest.setFreq(iLen);
            bSuccess = true;
        } else if (MAP_TO_POCKET_TYPE_LEN == flag) {
            // pocket type length
            mBtTest.setPocketTypeLen(iLen);
            bSuccess = true;
        }
        return bSuccess;
    }
}
