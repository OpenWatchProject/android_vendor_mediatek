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
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.Spinner;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

/**
 * Set uart info and test relayer mode.
 *
 * @author mtk54040
 *
 */
public class BtRelayerModeActivity extends Activity implements OnClickListener {
    private static final String TAG = "BtRelayerMode";
    // dialog ID and MSG ID
    private static final int DLG_START_TEST = 0;
    //Task ID
    private static final int START_TEST = 1;
    private static final int END_TEST = 2;

    private static final int RETURN_SUCCESS = 0;
    private static final int EXIT_SUCCESS = 10;
    private static final int RESULT_FAIL = -1;
    private static final int PARA_INDEX = 0;
    // UI
    private Spinner mSpBaudrate;
    private Spinner mSpUartPort;
    private Button mBtnStart;

    private BtTest mBtTest = null;
    private boolean mStartFlag = false;
    private int mBaudrate = 9600;
    private int mPortNumber = 3;



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.bt_relayer_mode);
        // Init UI component
        mSpBaudrate = (Spinner) findViewById(R.id.spinner1);
        mSpUartPort = (Spinner) findViewById(R.id.spinner2);
        mSpUartPort
                .setOnItemSelectedListener(new OnItemSelectedListener() {
                    public void onItemSelected(AdapterView<?> arg0, View arg1,
                            int arg2, long arg3) {
                        if (arg2 == 4) {  // select usb
                            mSpBaudrate.setEnabled(false);
                        } else {
                            mSpBaudrate.setEnabled(true);
                        }
                    }
                    public void onNothingSelected(AdapterView<?> arg0) {
                    }
                });
        mBtnStart = (Button) findViewById(R.id.button1);

        mBtnStart.setOnClickListener(this);

    }

    @Override
    public void onClick(View v) {

        if (v.getId() == mBtnStart.getId()) {

            // Disable button to avoid multiple click
            mBtnStart.setEnabled(false);
            FunctionTask functionTask = new FunctionTask();
            if (mStartFlag) {
                functionTask.execute(END_TEST);
                mBtnStart.setText(R.string.BT_start);
            } else {
                try {
                    mBaudrate = Integer.parseInt(mSpBaudrate
                            .getSelectedItem().toString().trim());

                } catch (NumberFormatException e) { // detail info
                    Elog.e(TAG, e.getMessage());
                }

                Long tmpLong = mSpUartPort.getSelectedItemId();
                mPortNumber = tmpLong.intValue();
                showDialog(DLG_START_TEST);
                functionTask.execute(START_TEST);
            }
        }

    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DLG_START_TEST) {
            ProgressDialog dialog = new ProgressDialog(
                    BtRelayerModeActivity.this);
            dialog.setMessage(getString(R.string.BT_relayer_start));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);

            return dialog;
        }
        return null;
    }

    /**
     * Deal with function request.
     *
     * @author mtk54040
     *
     */
    private class FunctionTask extends AsyncTask<Integer, Void, Integer> {

        @Override
        protected Integer doInBackground(Integer... params) {
            int result = RETURN_SUCCESS;
            mBtTest = new BtTest();
            int paraValue = params[PARA_INDEX];
            Elog.i(TAG, "paraValue:" + paraValue);
            try {
                if (paraValue == START_TEST) {
                    result = EmUtils.getEmHidlService().btStartRelayer(mPortNumber, mBaudrate);
                    Elog.i(TAG, "-->relayerStart-" + mBaudrate + " uart "
                            + mPortNumber + "result 0 success,-1 fail: result= "
                            + result);
                } else if (paraValue == END_TEST) {
                    EmUtils.getEmHidlService().btStopRelayer();
                    mStartFlag = false;
                    result = EXIT_SUCCESS;
                }
            } catch (RemoteException e) {
                e.printStackTrace();
                result = RESULT_FAIL;
            }
            return result;
        }

        @Override
        protected void onPostExecute(Integer result) {
            if (result == RETURN_SUCCESS) {       // START TEST OK
                mBtnStart.setText(R.string.BT_stop);
                mStartFlag = true;
            }
            // remove dialog
            removeDialog(DLG_START_TEST);

            // Enable next click operation
            mBtnStart.setEnabled(true);

        }

    }
}
