/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.op.wifi;

import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.database.Cursor;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;
import android.widget.TextView;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import com.mediatek.common.wifi.IWifiFwkExt;
import com.mediatek.server.wifi.op01.R;

public class WifiConnectNotifyDialog extends AlertActivity
        implements DialogInterface.OnClickListener {
    private static final String TAG = "WifiConnectNotifyDialog";
    private static final String PREF_REMIND_CONNECT = "pref_remind_connect";
    private static final String WIFI_DIALOG_CONTENT= "content://com.mediatek.cmcc.wifidiaog";

    private Context mContext;
    private CheckBox mCheckbox;
    private TextView mTimeView;
    private WifiCount mWifiCount;
    private IntentFilter mIntentFilter;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (WifiManager.NETWORK_STATE_CHANGED_ACTION.equals(action)) {
                NetworkInfo info = (NetworkInfo) intent.getParcelableExtra(
                        WifiManager.EXTRA_NETWORK_INFO);
                NetworkInfo.DetailedState dState = info.getDetailedState();
                Log.d("@M_" + TAG, " receive action");
                if (dState == NetworkInfo.DetailedState.DISCONNECTED) {
                    Log.d("@M_" + TAG, " disconnected, finish dialog");
                    finish();
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        super.setFinishOnTouchOutside(false);
        mContext = this;
        final AlertController.AlertParams p = mAlertParams;
        p.mTitle = getString(R.string.wifi_cmcc_costs_title);
        Log.d("@M_" + TAG, " mTimeView = " + mTimeView);
        p.mView = createView();
        p.mPositiveButtonText = getString(android.R.string.ok);
        p.mPositiveButtonListener = this;
        setupAlert();
        mIntentFilter = new IntentFilter(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        mContext.registerReceiver(mReceiver, mIntentFilter);
    }

    private View createView() {
        Log.d("@M_" + TAG, " createView()");
        StringBuilder summary = new StringBuilder();
        summary.append(getString(R.string.wifi_cmcc_costs_msg));
        summary.append("\n");
        summary.append(String.format(getString(R.string.wifi_count_down_sec), "5"));

        View view = getLayoutInflater().inflate(R.layout.setting_notify_dialog, null);
        mTimeView = (TextView) view.findViewById(R.id.content);
        mTimeView.setText(summary.toString());
        mCheckbox = (CheckBox) view.findViewById(R.id.closeReminder);
        return view;
    }

    /* define one internal timer class */
    class WifiCount extends CountDownTimer {
        private int mTimes = 6;
        public WifiCount(long millisInFuture, long countDownInterval) {
            super(millisInFuture, countDownInterval);
        }

        @Override
        public void onFinish() {
            Log.d("@M_" + TAG, " WifiCount onFinish()");
            mWifiCount = null;
            ((AlertActivity) mContext).finish();
        }

        @Override
        public void onTick(long millisUntilFinished) {
            Log.d("@M_" + TAG, " WifiCount onTick()");
            StringBuilder summary = new StringBuilder();
            mTimes--;
            summary.append(getString(R.string.wifi_cmcc_costs_msg));
            summary.append("\n");
            summary.append(String.format(getString(R.string.wifi_count_down_sec),
                                        String.valueOf(mTimes)));
            mTimeView.setText(summary.toString());
        }
    }

    @Override
    protected void onResume() {
        Log.d("@M_" + TAG, " onResume()");
        super.onResume();
        if (mWifiCount == null) {
            mWifiCount = new WifiCount(6000, 1000);
            mWifiCount.start();
        }
    }

    @Override
    protected void onDestroy() {
        Log.d("@M_" + TAG, " onDestroy()");
        super.onDestroy();
        if (mWifiCount != null) {
            mWifiCount.cancel();
            mWifiCount = null;
        }
        mContext.unregisterReceiver(mReceiver);
    }

    private void onPositive() {
        Log.d("@M_" + TAG, " onPositive()");
        if (mCheckbox.isChecked()) {
            String[] projection = {"remind_connect_value"};
            String selection = "remind_connect_key=?";
            String[] selectionArgs = {"wificonnectnotifydialog_remind"};
            Cursor cursor = this.getContentResolver().query(Uri.parse(WIFI_DIALOG_CONTENT),
                    projection , selection , selectionArgs , null);
            
            if (null == cursor) {
                Log.i("@M_" + TAG, "onPositive cursor == null insert wificonnectnotifydialog");
                ContentValues values = new ContentValues();
                values.put("remind_connect_key", "wificonnectnotifydialog_remind");
                values.put("remind_connect_value", 2);
                this.getContentResolver().insert(Uri.parse(WIFI_DIALOG_CONTENT), values);
            } else {
                if (cursor.moveToNext()) {
                    Log.i("@M_" + TAG, "onPositive cursor != null update wificonnectnotifydialog");
                    ContentValues values = new ContentValues();
                    values.put("remind_connect_value", 2);
                    this.getContentResolver().update(Uri.parse(WIFI_DIALOG_CONTENT), values, selection, selectionArgs);
                } else {
                    Log.i("@M_" + TAG, "onPositive cursor != null insert wificonnectnotifydialog");
                    ContentValues values = new ContentValues();
                    values.put("remind_connect_key", "wificonnectnotifydialog_remind");
                    values.put("remind_connect_value", 2);
                    this.getContentResolver().insert(Uri.parse(WIFI_DIALOG_CONTENT), values);
                }
                cursor.close();
            }
        }
        finish();
    }

    private void onNegative() {
        Log.d("@M_" + TAG, " onNegative()");
        finish();
    }

    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                onPositive();
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                onNegative();
                break;
            default:
                /// do nothing.
        }
    }

}
