
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.op07.browser;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.browser.ext.DefaultNetworkStateHandlerExt;
import com.mediatek.op07.browser.R;
//import com.mediatek.common.PluginImpl;
//import com.mediatek.op07.plugin.R;

/**
 * Plugin implementation for OP07.
 */
//@PluginImpl(interfaceName = "com.mediatek.browser.ext.INetworkStateHandlerExt")
public class OP07NetworkStateHandlerExt extends DefaultNetworkStateHandlerExt {

    private Context mContext = null;

    /**
     * Plugin implementation for CallfeatureSettings.
     * @param context context
     */
    public OP07NetworkStateHandlerExt(Context context) {
        Log.d("OP07NetworkStateHandlerExt", "Inside constructor!!");
        mContext = context;
    }

    @Override
    public void promptUserToEnableData(Activity activity) {
        Log.d("OP07NetworkStateHandlerExt", "Inside promptUserToEnableData()!!");
        if (mContext == null) {
            return;
        }
        ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(
                Context.CONNECTIVITY_SERVICE);
        WifiManager wifi = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        TelephonyManager tm = (TelephonyManager) mContext.getSystemService(
                Context.TELEPHONY_SERVICE);
        NetworkInfo mWifi = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI);

        if (isAirplaneModeOn()) {
            /*
            if (!wifi.isWifiEnabled()) {
                showDialog(activity, mContext.getText(R.string.open_wifi_airplane_mode));
            } */
            return;
        } else {
            if ((!tm.isNetworkRoaming()) ||  (isDataRoamingOn())) {
                /*
                if (wifi.isWifiEnabled()) {
                    if (!(tm.getDataEnabled() || mWifi.isConnected())) {
                        showDialog(activity, mContext.getText(
                            R.string.open_mobile_data));
                    }
                } else if (!tm.getDataEnabled()) {
                    showDialog(activity, mContext.getText(R.string.open_wifi_or_mobile_data));
                } */
                if (!tm.getDataEnabled()) {
                    showDialog(activity, mContext.getText(
                            R.string.open_mobile_data));
                    //showDialog(activity, R.string.open_mobile_data);
                }
            } else {
                /*
                if (wifi.isWifiEnabled()) {
                    if (!mWifi.isConnected()) {
                        if (tm.getDataEnabled()) {
                            showDialog(activity, mContext.getText(R.string.open_data_roaming));
                        } else {
                            showDialog(activity, mContext.getText(
                                    R.string.open_data_roaming_and_mobile_data));
                        }
                    }
                } else {
                    if (tm.getDataEnabled()) {
                        showDialog(activity, mContext.getText(R.string.open_data_roaming_or_wifi));
                    } else {
                        showDialog(activity, mContext.getText(
                                R.string.open_data_roaming_and_mobile_data_or_open_wifi));
                    }
                } */
                if (!tm.getDataEnabled()) {
                    showDialog(activity, mContext.getText(
                            R.string.open_data_roaming_and_mobile_data));
                    //showDialog(activity, R.string.open_data_roaming_and_mobile_data);
                } else {
                    showDialog(activity, mContext.getText(
                            R.string.open_data_roaming));
                    //showDialog(activity, R.string.open_data_roaming);
                }
            }
        }
    }

    private boolean isAirplaneModeOn() {
        return Settings.Global.getInt(
                mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }

    private boolean isDataRoamingOn() {
        return Settings.Global.getInt(
                mContext.getContentResolver(), Settings.Global.DATA_ROAMING, 0) != 0;
    }

    private void showDialog(Activity activity, CharSequence msg) {
    //private void showDialog(Activity activity, int msg) {
        final Activity a = activity;
        AlertDialog.Builder builder = new AlertDialog.Builder(activity);
        builder.setTitle(android.R.string.dialog_alert_title);
        builder.setMessage(msg);
        builder.setPositiveButton(
            mContext.getText(R.string.settings), new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                Intent intent = new Intent("android.settings.SETTINGS");
                a.startActivity(intent);
            }
        });
        builder.setNegativeButton(mContext.getText(android.R.string.cancel), null);
        AlertDialog alert = builder.create();
        alert.show();
    }
}