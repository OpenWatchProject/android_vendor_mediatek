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

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ListActivity;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

/**
 * Show Bluetooth test modules.
 *
 * @author mtk54040
 *
 */
public class BtList extends ListActivity {
    private static final String TAG = "BTList";

    // dialog ID and MSG ID
    private static final int DLG_CHECK_BLE = 2;
    private static final int DLG_NOT_SUPPORT = 3;
    private static final int RENTURN_SUCCESS = 0;

    private BluetoothAdapter mBtAdapter;

    // flags
    private BleSupportState mBleSupport = BleSupportState.BLE_NONE; // mHasBleSupport
    private boolean mComboSupport = false;

    private BtTest mBtTest = null;
    private ArrayList<String> mModuleList = null;
    /**
     * BLE support type.
     *
     */
    enum BleSupportState {
        BLE_NONE,
        BLE_NORMAL,
        BLE_ENHANCED
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!FeatureSupport.isEmBTSupport()) {
            showDialog(DLG_NOT_SUPPORT);
            return;
        }
        setContentView(R.layout.btlist);

        mModuleList = new ArrayList<String>();

        mModuleList.add(getString(R.string.BT_tx_only_Title));

        ArrayAdapter<String> moduleAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mModuleList);
        setListAdapter(moduleAdapter);
        mModuleList.clear();
        if (mBtAdapter == null) {
            mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        }

        if (!BtTest.checkInitState(mBtAdapter, this)) {
            finish();
        } else {
            showDialog(DLG_CHECK_BLE);
            FunctionTask functionTask = new FunctionTask();
            functionTask.execute();
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {

        if (id == DLG_CHECK_BLE) {
            ProgressDialog dialog = new ProgressDialog(this);

            dialog.setMessage(getString(R.string.BT_init_dev));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            return dialog;
        } else if (id == DLG_NOT_SUPPORT) {
            return new AlertDialog.Builder(this).setTitle(R.string.Bluetooth)
                    .setCancelable(false)
                    .setMessage(getString(R.string.bt_not_support))
                    .setPositiveButton(R.string.dialog_ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                }).create();
        }
        return null;
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {

        String moduleTitle = mModuleList.get(position);
        if (moduleTitle == null) {
            return;
        }
        if (moduleTitle.equals(
                getString(R.string.BT_tx_only_Title))) {
            startActivity(new Intent(BtList.this, TxOnlyTestActivity.class));
        } else if (moduleTitle.equals(
                getString(R.string.BTNSRXTitle))) {
            startActivity(new Intent(BtList.this, NoSigRxTestActivity.class));
        } else if (moduleTitle.equals(
                getString(R.string.BTTMTitle))) {
            startActivity(new Intent(BtList.this, TestModeActivity.class));

        } else if (moduleTitle.equals(
                getString(R.string.BT_RelayerModeTitle))) {
            startActivity(new Intent(BtList.this,
                    BtRelayerModeActivity.class));
        } else if (moduleTitle.equals(getString(R.string.BT_ble_test_mode_Title))) {
            startActivity(new Intent(BtList.this, BleTestMode.class));
        } else if (moduleTitle.equals(getString(R.string.BT_ble_enhanced_test_mode_Title))) {
            startActivity(new Intent(BtList.this, BleEnhancedTestMode.class));
        }
    }

    /**
     * AsyncTask to query feature support status.
     *
     */
    private class FunctionTask extends AsyncTask<Void, Void, Integer> {

        @Override
        protected Integer doInBackground(Void... params) {
            mBtTest = new BtTest();
            if (mBtTest.isBLEEnhancedSupport()) {
                mBleSupport = BleSupportState.BLE_ENHANCED;
            } else if (mBtTest.isBLESupport() == 1) {
                mBleSupport = BleSupportState.BLE_NORMAL;
            }

            if (mBtTest.isComboSupport() == 1) {
                mComboSupport = true;
            } else {
                mComboSupport = false;
            }
            Elog.i(TAG, "BLE supported ? " + mBleSupport);


            return RENTURN_SUCCESS;
        }

        @Override
        protected void onPostExecute(Integer result) {
            mModuleList = new ArrayList<String>();

            mModuleList.add(getString(R.string.BT_tx_only_Title));


            mModuleList.add(getString(R.string.BTNSRXTitle));

            mModuleList.add(getString(R.string.BTTMTitle));

            if (mBleSupport == BleSupportState.BLE_ENHANCED) {
                mModuleList.add(getString(R.string.BT_ble_enhanced_test_mode_Title));
            } else if (mBleSupport == BleSupportState.BLE_NORMAL) {
                mModuleList.add(getString(R.string.BT_ble_test_mode_Title));
            }

            if (mComboSupport) {
                mModuleList.add(getString(R.string.BT_RelayerModeTitle));
            }

            ArrayAdapter<String> moduleAdapter = new ArrayAdapter<String>(
                    BtList.this, android.R.layout.simple_list_item_1,
                    mModuleList);
            BtList.this.setListAdapter(moduleAdapter);


            removeDialog(DLG_CHECK_BLE);

            super.onPostExecute(result);
        }

    }
}
