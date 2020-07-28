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

package com.mediatek.op.wifi;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.android.internal.telephony.ITelephony;

import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.server.wifi.op01.R;

public class DataConnectionDialog extends AlertActivity implements
        DialogInterface.OnClickListener {
    private static final String ACTION_WIFI_STATE_CHANGED = WifiManager.NETWORK_STATE_CHANGED_ACTION;
    private static final String ACTION_SS_STATE_CHANGED = "android.intent.action.SERVICE_STATE";
    private ITelephony mService;
    private boolean mIsLastDataOn;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d("DataConnectionDialog", "onReceive : " + action);
            if (ACTION_WIFI_STATE_CHANGED.equals(action)) {
                NetworkInfo info = (NetworkInfo) intent
                        .getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);

                if (info == null) {
                    finish();
                    return;
                }
                Log.d("DataConnectionDialog", "onReceive : " + action
                        + ", mIsLastDataOn: " + mIsLastDataOn
                        + ", NetworkInfo: " + info);
                if (info.getState().equals(NetworkInfo.State.CONNECTED)) {
                    // restore DATA SWITCH
                    if (mIsLastDataOn) {
                        onUserPositive();
                    }
                    finish();
                }
            } else if (ACTION_SS_STATE_CHANGED.equals(action)) {
                Log.d("DataConnectionDialog", "onReceive : " + action
                        + ", mIsLastDataOn: " + mIsLastDataOn);
                if (Settings.System.getInt(context.getContentResolver(),
                        Settings.System.AIRPLANE_MODE_ON, 0) != 0) {
                    finish();
                }
            }
        }
    };

    private void onUserPositive() {
        Log.d("DataConnectionDialog", "onUserPositive");

        TelephonyManager telMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        if (telMgr == null) {
            Log.d("DataConnectionDialog",
                    "onUserPositive : get TELEPHONY_SERVICE failed");
            return;
        }

        Log.d("DataConnectionDialog", "onUserPositive: mIsLastDataOn=" + mIsLastDataOn);
        telMgr.setDataEnabled(true);
    }

    private void onUserNegative() {
        Log.d("DataConnectionDialog", "onUserNegative");

        TelephonyManager telMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        if (telMgr != null) {
            telMgr.setDataEnabled(false);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        super.setFinishOnTouchOutside(false);

        // Set up the "dialog"
        final AlertController.AlertParams p = mAlertParams;
        p.mIconId = android.R.drawable.ic_dialog_alert;
        p.mTitle = getString(R.string.dialog_title);
        p.mView = createView();
        p.mPositiveButtonText = getString(android.R.string.ok);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(android.R.string.cancel);
        p.mNegativeButtonListener = this;
        setupAlert();

        IntentFilter intentFilter = new IntentFilter();
        //intentFilter.addAction(ACTION_IPO_SHUTDOWN);
        intentFilter.addAction(ACTION_WIFI_STATE_CHANGED);
        intentFilter.addAction(ACTION_SS_STATE_CHANGED);
        registerReceiver(mReceiver, intentFilter);

        mService = ITelephony.Stub.asInterface(ServiceManager
                .getService(Context.TELEPHONY_SERVICE));

        // Record before selection
        mIsLastDataOn = false;

        long lastId = Settings.System.getLong(
            this.getContentResolver(),
            MtkSettingsExt.System.LAST_SIMID_BEFORE_WIFI_DISCONNECTED,
            -1);

        if (lastId > 0) {
            mIsLastDataOn = true;
        }

        Log.d("DataConnectionDialog", "onCreate : mIsLastDataOn="
                + mIsLastDataOn + ", mLastDataSimId=" + lastId);
    }

    @Override
    protected void onDestroy() {
        Log.d("DataConnectionDialog", "onDestroy");

        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    private View createView() {
        View view = getLayoutInflater().inflate(R.layout.confirm_dialog, null);
        TextView contentView = (TextView) view.findViewById(R.id.content);
        if (contentView != null) {
            contentView.setText(getString(R.string.wifi_failover_gprs_content));
        }
        return view;
    }

    public void onClick(DialogInterface dialog, int which) {
        Log.d("DataConnectionDialog", "onClick which=" + which);

        switch (which) {
        case DialogInterface.BUTTON_POSITIVE:
            onUserPositive();
            finish();
            break;

        case DialogInterface.BUTTON_NEGATIVE:
            onUserNegative();
            finish();
            break;

        default:
            Log.d("DataConnectionDialog", "onClick(): which=" + which);
            break;
        }
    }
}
