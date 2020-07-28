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

import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
//import android.net.ConnectivityManager;
//import android.net.NetworkInfo;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.android.internal.telephony.ITelephony;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import com.mediatek.common.wifi.IWifiFwkExt;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.server.wifi.op01.R;

public class WifiReselectApDialog extends AlertActivity implements DialogInterface.OnClickListener {
    private static final String TAG = "WifiReselectApDialog";
    // Used in Intent.setPackage(...), target to BroadcastReceiver runs in system
    private static String PACKAGE_NAME_WIFI_FAILOVER = "android";
    private Context mContext;
    private boolean mIsLastDataOn;
    private boolean mIsDone;
    private WifiManager mWifiManager;
    private TelephonyManager mTelephonyManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        super.setFinishOnTouchOutside(false);
        Log.d(TAG, "onCreate()");
        mContext = this;
        mWifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        createDialog();
    }

    private void createDialog() {
        final AlertController.AlertParams p = mAlertParams;
        p.mTitle = getString(R.string.confirm_title);
        p.mView = createView();
        p.mPositiveButtonText = getString(android.R.string.yes);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(android.R.string.no);
        p.mNegativeButtonListener = this;
        setupAlert();

        // Record before selection
        mIsLastDataOn = false;
        mIsDone = false;

        long lastId = Settings.System.getLong(
            this.getContentResolver(),
            MtkSettingsExt.System.LAST_SIMID_BEFORE_WIFI_DISCONNECTED,
            -1);

        if (lastId > 0) {
            mIsLastDataOn = true;
        }

        Log.d(TAG, "onCreate : mIsLastDataOn=" + mIsLastDataOn);
        // if ON, recored
        if (mIsLastDataOn) {
            onUserNegative(); // Disable firstly before user confirmed
        }
    }

    private View createView() {
        View view = getLayoutInflater().inflate(R.layout.setting_notify_dialog, null);
        TextView contentView = (TextView) view.findViewById(R.id.content);
        contentView.setText(getString(R.string.wifi_signal_found_msg));

        view.findViewById(R.id.closeReminder).setVisibility(View.GONE);;

        return view;
    }

    private void onUserPositive() {
        Log.d(TAG, "onUserPositive");
        if (mTelephonyManager != null) {
            mTelephonyManager.setDataEnabled(true);
        }
    }

    private void onUserNegative() {
        Log.d(TAG, "onUserNegative");
        if (mTelephonyManager != null) {
            mTelephonyManager.setDataEnabled(false);
        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy()");
        super.onDestroy();
        if (mIsLastDataOn && !mIsDone) {
            Log.d(TAG, "onDestroy() for dataconnection");
            mIsDone = true;
            //ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
            //NetworkInfo nwInfo = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
            if (mWifiManager.getConnectionInfo().getSupplicantState() == SupplicantState.COMPLETED) {
                onUserPositive();
            } else {
                Intent failoverIntent = new Intent("android.intent.action_WIFI_FAILOVER_GPRS_DIALOG");
                failoverIntent.setPackage(PACKAGE_NAME_WIFI_FAILOVER);
                mContext.sendBroadcastAsUser(failoverIntent, UserHandle.ALL);
            }
            mIsLastDataOn = false;
        }
    }

    @Override
    protected void onPause() {
        super.onPause();

        Log.e(TAG, "onPause entry");

        ActivityManager amgr = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        ComponentName cpname = null;
        String classname = null;

        if (amgr.getRunningTasks(1) != null && amgr.getRunningTasks(1).get(0) != null) {
            cpname = amgr.getRunningTasks(1).get(0).topActivity;
        }

        if (cpname != null) {
            classname = cpname.getClassName();
            Log.d(TAG, "ClassName:" + classname);
        } else {
            Log.e(TAG, "Component ClassName is null");
            return;
        }

        if (classname != null && classname.equals("com.mediatek.data.DataConnectionDialog")) {
            Log.e(TAG, "Reselect dialog finish because DataConnectionDialog on its top");
            finish();
        }
    }

    private void onOK() {
        Log.d(TAG, "onOK()");
        mIsDone = true;
        if (mIsLastDataOn) {
            Log.d(TAG, "onOK() for dataconnection");
            onUserPositive();
            mIsLastDataOn = false;
        }
        Intent intent = new Intent();
        intent.setAction("android.settings.WIFI_SETTINGS");
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        startActivity(intent);
        finish();
    }

    private void onCancel() {
        Log.d(TAG, "onCancel()");
        mIsDone = true;
        if (mIsLastDataOn) {
            Log.d(TAG, "onCancel() for dataconnection");
            //ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
            //NetworkInfo nwInfo = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
            if (mWifiManager.getConnectionInfo().getSupplicantState() == SupplicantState.COMPLETED) {
                Log.d(TAG, "onCancel() wifi is connecting");
                onUserPositive();
            } else {
                Log.d(TAG, "onCancel() wifi is disconnected, to open GPRS_DIALOG");
                Intent failoverIntent = new Intent("android.intent.action_WIFI_FAILOVER_GPRS_DIALOG");
                failoverIntent.setPackage(PACKAGE_NAME_WIFI_FAILOVER);
                mContext.sendBroadcastAsUser(failoverIntent, UserHandle.ALL);
            }
            mIsLastDataOn = false;
        }
        //wait for wifi framework ready
        //mWifiManager.suspendNotification(IWifiFwkExt.NOTIFY_TYPE_RESELECT);
        Intent intent = new Intent(Op01WifiUtils.ACTION_SUSPEND_NOTIFICATION_NON_SYS);
        intent.setPackage("android");
        intent.putExtra(IWifiFwkExt.EXTRA_SUSPEND_TYPE, IWifiFwkExt.NOTIFY_TYPE_RESELECT);
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
        finish();
    }

    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                onOK();
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                onCancel();
                break;
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Log.d(TAG, "onNewIntent()");
        long lastId = Settings.System.getLong(
            this.getContentResolver(),
            MtkSettingsExt.System.LAST_SIMID_BEFORE_WIFI_DISCONNECTED,
            -1);

        Log.d(TAG, "onNewIntent : lastId="
                + lastId + ", mIsLastDataOn=" + mIsLastDataOn);

        // if ON, recored
        if (lastId > 0) {
            mIsLastDataOn = true;
            onUserNegative(); // Disable firstly before user confirmed
        }
    }

}
