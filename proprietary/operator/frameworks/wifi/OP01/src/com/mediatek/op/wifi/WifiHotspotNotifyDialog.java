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
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.CheckBox;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import com.mediatek.common.wifi.IWifiFwkExt;
import com.mediatek.server.wifi.op01.R;

public class WifiHotspotNotifyDialog extends AlertActivity implements DialogInterface.OnClickListener {
    private static final String TAG = "WifiHotspotNotifyDialog";
    private static final String PREF_REMIND_HOTSPOT = "pref_remind_hotspot";
    private static final String WIFI_DIALOG_CONTENT= "content://com.mediatek.cmcc.wifidiaog";

    private Context mContext;
    private CheckBox mCheckbox;
    private IntentFilter mIntentFilter;

    private static void log(String msg) {
        Log.d("@M_" + TAG, msg);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        super.setFinishOnTouchOutside(false);
        mContext = this;

        final AlertController.AlertParams p = mAlertParams;
        p.mTitle = getString(R.string.setting_remind_title);
        p.mView = createView();
        p.mPositiveButtonText = getString(android.R.string.ok);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(android.R.string.cancel);
        p.mNegativeButtonListener = this;
        setupAlert();
        mIntentFilter = new IntentFilter(WifiManager.WIFI_AP_STATE_CHANGED_ACTION);
        registerReceiver(mReceiver, mIntentFilter);
    }

    private View createView() {
        log("createView");
        View view = getLayoutInflater().inflate(R.layout.setting_notify_dialog, null);
        TextView contentView = (TextView) view.findViewById(R.id.content);
        contentView.setText(getString(R.string.msg_wifi_hotspot_notify));
        mCheckbox = (CheckBox) view.findViewById(R.id.closeReminder);
        return view;
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
                log("receive action: " + action);
            if (WifiManager.WIFI_AP_STATE_CHANGED_ACTION.equals(action)) {
                int state = intent.getIntExtra(
                        WifiManager.EXTRA_WIFI_AP_STATE, WifiManager.WIFI_AP_STATE_FAILED);
                if (state == WifiManager.WIFI_AP_STATE_DISABLED) {
                    finish();
                }
            }
        }
    };

    @Override
    protected void onDestroy() {
        log("onDestroy()");
        unregisterReceiver(mReceiver);
        super.onDestroy();
    }

    private void onPositive() {
        log("onOK");
        if (mCheckbox.isChecked()) {
            log("onOKgogoo");
            String[] projection = {"remind_hotspot_value"};
            String selection = "remind_hotspot_key=?";
            String[] selectionArgs = {"wifihotspotnotifydialog_remind"};
            Cursor cursor = this.getContentResolver().query(Uri.parse(WIFI_DIALOG_CONTENT),
                    projection , selection , selectionArgs , null);
            
            if (null == cursor) {
                Log.i("@M_" + TAG, "onPositive cursor == null insert wifihotspotnotifydialog");
                ContentValues values = new ContentValues();
                values.put("remind_hotspot_key", "wifihotspotnotifydialog_remind");
                values.put("remind_hotspot_value", 2);
                this.getContentResolver().insert(Uri.parse(WIFI_DIALOG_CONTENT), values);
            } else {
                if (cursor.moveToNext()) {
                    Log.i("@M_" + TAG, "onPositive cursor != null update wifihotspotnotifydialog");
                    ContentValues values = new ContentValues();
                    values.put("remind_hotspot_value", 2);
                    this.getContentResolver().update(Uri.parse(WIFI_DIALOG_CONTENT), values, selection, selectionArgs);
                } else {
                    Log.i("@M_" + TAG, "onPositive cursor != null insert wifihotspotnotifydialog");
                    ContentValues values = new ContentValues();
                    values.put("remind_hotspot_key", "wifihotspotnotifydialog_remind");
                    values.put("remind_hotspot_value", 2);
                    this.getContentResolver().insert(Uri.parse(WIFI_DIALOG_CONTENT), values);
                }
                cursor.close();

            }
        }
        finish();
    }

    private void onNegative() {
        log("onCancel");
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
