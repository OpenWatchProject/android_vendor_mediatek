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
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Do BT test mode.
 *
 * @author mtk54040
 *
 */
public class TestModeActivity extends Activity implements
        DialogInterface.OnCancelListener {


    private static final String TAG = "BtTestMode";

    private static final int DLG_BT_INIT = 2;
    private static final int DLG_BT_DEINIT = 3;
    private static final int BT_TEST_1 = 1;
    private static final int BT_TEST_2 = 2;
    private static final int RETURN_FAIL = -1;
    private static final int DEFAULT_INT = 9;

    private static final int MSG_UI_TEST_SUCCESS = 5;
    private static final int MSG_UI_TEST_FAIL = 6;
    private static final int MSG_UI_TEST_STOP_SUCCESS = 7;
    private static final int MSG_UI_EXIT = 8;

    private static final int MSG_OP_TEST_START = 11;
    private static final int MSG_OP_TEST_STOP = 12;
    private static final int MSG_OP_DO_TEST_STOP = 13;
    private static final int MSG_OP_EXIT = 14;
    private static final int POWER_INVALID = -1;

    private BluetoothAdapter mAdapter;
    private EditText mEtPower;
    private RadioGroup mRgPowerLevel;
    private Button mBtnEnableTm;
    private Button mBtnDisableTm;
    private View mLayoutPower;

    private BtTest mBtTest;
    private WorkHandler mWorkHandler = null;
    private HandlerThread mWorkThread = null;

    private final Handler mUiHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {

            Elog.i(TAG, "receive msg of " + msg.what);
            switch (msg.what) {
            case MSG_UI_TEST_SUCCESS:
                removeDialog(DLG_BT_INIT);
                break;
            case MSG_UI_TEST_FAIL:
                Toast.makeText(getApplicationContext(),
                        R.string.BT_data_fail,
                        Toast.LENGTH_SHORT).show();
                break;
            case MSG_UI_TEST_STOP_SUCCESS:
                removeDialog(DLG_BT_DEINIT);
                break;
            case MSG_UI_EXIT:
                finish();
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.bt_test_mode);
        initComponent();
        if (mAdapter == null) {
            mAdapter = BluetoothAdapter.getDefaultAdapter();
        }
        if (!BtTest.checkInitState(mAdapter, this)) {
            finish();
        } else {
            mWorkThread = new HandlerThread(TAG);
            mWorkThread.start();

            Looper looper = mWorkThread.getLooper();
            mWorkHandler = new WorkHandler(looper);
        }
    }

    private void initComponent() {
        mLayoutPower = findViewById(R.id.bt_tm_power_ll);
        mEtPower = (EditText) findViewById(R.id.bt_tm_power_et);
        mRgPowerLevel = findViewById(R.id.bt_tm_power_rg);
        mRgPowerLevel.setOnCheckedChangeListener(new OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                // TODO Auto-generated method stub
                mLayoutPower.setVisibility((checkedId == R.id.bt_tm_power_cus_rb)
                        ? View.VISIBLE : View.GONE);
            }

        });
        mBtnEnableTm = findViewById(R.id.bt_tm_enable);
        mBtnEnableTm.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub

                // TODO Auto-generated method stub

                int power = POWER_INVALID;
                if (mRgPowerLevel.getCheckedRadioButtonId() == R.id.bt_tm_power_cus_rb) {
                    power = getPowerInput();
                    if (power == POWER_INVALID) {
                        Toast.makeText(TestModeActivity.this,
                                getString(R.string.bt_tm_power_invalid),
                                Toast.LENGTH_LONG).show();
                        return;
                    }

                }
                enableUi(false);
                showDialog(DLG_BT_INIT);
                Message msg = mWorkHandler.obtainMessage(MSG_OP_TEST_START);
                msg.arg1 = power;
                mWorkHandler.sendMessage(msg);
            }
        });
        mBtnDisableTm = findViewById(R.id.bt_tm_disable);
        mBtnDisableTm.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub

                // TODO Auto-generated method stub
                enableUi(true);
                showDialog(DLG_BT_DEINIT);
                mWorkHandler.sendEmptyMessage(MSG_OP_TEST_STOP);
            }
        });
        enableUi(true);

    }
    private void enableUi(boolean enabled) {
        int count = mRgPowerLevel.getChildCount();
        for (int k = 0; k < count; k++) {
            mRgPowerLevel.getChildAt(k).setEnabled(enabled);
        }
        mEtPower.setEnabled(enabled);
        mBtnEnableTm.setEnabled(enabled);
        mBtnDisableTm.setEnabled(!enabled);
    }

    private int getPowerInput() {
        int power = POWER_INVALID;
        String val = mEtPower.getText().toString();
        if ((val != null) && (val.length() == 1)) {
            try {
                power = Integer.parseInt(val);

            } catch (NumberFormatException e) {
                e.printStackTrace();
                return POWER_INVALID;
            }
        }
        return power;

    }
    /**
     *  implemented for DialogInterface.OnCancelListener.
     *
     *  @param dialog : dialog interface
     */
    public void onCancel(DialogInterface dialog) {
        // request that the service stop the query with this callback object.
        finish();
    }

    @Override
    public void onBackPressed() {
        if (mWorkHandler != null && mBtTest != null) {
            mWorkHandler.sendEmptyMessage(MSG_OP_EXIT);
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
        Dialog returnDialog = null;
        if (id == DLG_BT_INIT) {
            ProgressDialog dialog = new ProgressDialog(TestModeActivity.this);
            dialog.setMessage(getString(R.string.BT_init_dev));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            dialog.setOnCancelListener(this);
            returnDialog = dialog;
        } else if (id == DLG_BT_DEINIT) {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage(getString(R.string.BT_deinit));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            returnDialog = dialog;
        }

        return returnDialog;
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

            if (msg.what == MSG_OP_TEST_START) {
                mBtTest = new BtTest();
                mBtTest.setPower(msg.arg1);
                Elog.i(TAG, "power set " + msg.arg1);
                if (RETURN_FAIL == mBtTest.doBtTest(BT_TEST_1)) {
                    Elog.e(TAG, "transmit data failed.");
                    mUiHandler.sendEmptyMessage(MSG_UI_TEST_FAIL);
                } else {
                    mUiHandler.sendEmptyMessage(MSG_UI_TEST_SUCCESS);
                }

                mBtTest.pollingStart();

            } else if (msg.what == MSG_OP_TEST_STOP || msg.what == MSG_OP_EXIT) {
                if (mBtTest != null) {
                    mBtTest.pollingStop();
                    runHCIResetCmd();
                    if (RETURN_FAIL == mBtTest.doBtTest(BT_TEST_2)) {
                        Elog.e(TAG, "transmit data failed 1.");
                    }
                    mBtTest = null;
                }
                if (msg.what == MSG_OP_TEST_STOP) {
                    mUiHandler.sendEmptyMessage(MSG_UI_TEST_STOP_SUCCESS);
                } else if (msg.what == MSG_OP_EXIT) {
                    mUiHandler.sendEmptyMessage(MSG_UI_EXIT);
                }
            }
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
}
