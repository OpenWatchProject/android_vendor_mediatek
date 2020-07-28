/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.op18.settings;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import android.view.KeyEvent;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;


/**
 * DataSwitch confirm dialog activity.
 */
public class DataSwitchDialog extends Activity {
    private static final String TAG = "DataSwitchDialog";
    private static final int DIALOG_ID_CHANGE_DATA_CONNECTION = 1001;
    //private static final String ACTION_KEY = "switch_data_sub";

    private Context mContext;
    private Activity mActivity;
    private IntentFilter mIntentFilter;

    private int mToOpenSubId;

    private BroadcastReceiver mSubReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "mSubReceiver action = " + action);
            if (action.equals(TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE)
                || action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                int[] subids = SubscriptionManager.from(mContext).getActiveSubscriptionIdList();
                if (subids == null || subids.length <= 1) {
                    Log.d(TAG, "onReceive dealWithDataConnChanged dismiss AlertDlg");
                    finish();
                }
            } else if (TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED.equals(action)) {
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                dealDataConnectionChanged(subId);
                Log.d(TAG, "changed default data subId: " + subId);
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                finish();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        super.setFinishOnTouchOutside(false);
        mContext = this;
        mActivity = this;
        Log.d(TAG, "Switch data: " + getCallingActivity());
        Log.d(TAG, "Switch data: " + getCallingPackage());

        mToOpenSubId =
            getIntent().getIntExtra("subId", SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        Log.d(TAG, "mToOpenSubId =" + mToOpenSubId);
        mIntentFilter = new IntentFilter(TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE);
        mIntentFilter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        mIntentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        registerReceiver(mSubReceiver, mIntentFilter);

        showDialog(DIALOG_ID_CHANGE_DATA_CONNECTION);
    }

    @Override
    public Dialog onCreateDialog(int id, Bundle data) {
        switch (id) {
            case DIALOG_ID_CHANGE_DATA_CONNECTION: {
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                AlertDialog dialog = getChangeDataConnDialog(builder);
                dialog.setCanceledOnTouchOutside(false);
                dialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                        if (keyCode == KeyEvent.KEYCODE_BACK
                            && event.getAction() == KeyEvent.ACTION_DOWN) {
                            finish();
                            return true;
                        } else {
                            return false;
                        }
                    }
                });
                return dialog;
            }

            default:
                return null;
        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy()");
        unregisterReceiver(mSubReceiver);
        mToOpenSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        super.onDestroy();
    }

    private AlertDialog getChangeDataConnDialog(Builder builder) {
        Log.d(TAG, "getChangeDataConnDialog");
        SubscriptionInfo simInfo = SubscriptionManager
                    .from(mContext).getActiveSubscriptionInfo(mToOpenSubId);
        int toOpenSlotId = (simInfo.getSimSlotIndex() + 1);
        Log.d(TAG, "mToOpenSubId: " + mToOpenSubId + "toOpenSlotId: " + toOpenSlotId);
        builder.setTitle(mContext.getString(R.string.change_data_conn_title) + toOpenSlotId);
        builder.setMessage(mContext.getString(R.string.change_data_conn_alert));
        final Context context = getApplicationContext();

        builder.setPositiveButton(android.R.string.yes,
               new DialogInterface.OnClickListener() {
                      public void onClick(DialogInterface dialog, int whichButton) {
                           Log.d(TAG, "Perform On click ok");
                           //removeDialog(DIALOG_ID_CHANGE_DATA_CONNECTION);
                           Log.d(TAG, "Switch data to subId = " + mToOpenSubId);
                           Log.d(TAG, "Switch data: " + mActivity.getCallingActivity());
                           Log.d(TAG, "Switch data: " + mActivity.getCallingPackage());
                           Log.d(TAG, "Switch data = " + getCallingActivity());
                           Log.d(TAG, "Switch data = " + getCallingPackage());
                           setDefaultDataSubId(context, mToOpenSubId);
                           finish();
                           Log.d(TAG, "finish avtivity");
                       }
                   });

        builder.setNegativeButton(android.R.string.cancel,
               new DialogInterface.OnClickListener() {
                      public void onClick(DialogInterface dialog, int whichButton) {
                           Log.d(TAG, "Perform On click cancel");
                           finish();
                       }
                   });

        return builder.create();
    }

    private void dealDataConnectionChanged(int subId) {
        Log.i(TAG, "dealDataConnectionChanged: subId is " + subId +
                ", mToOpenSubId is " + mToOpenSubId);
        if (mToOpenSubId == subId) {
            Log.d(TAG, "dealDataConnectionChanged finish ");
            finish();
        }
    }

    private void setDefaultDataSubId(final Context context, final int subId) {
        if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            final SubscriptionManager subscriptionManager = SubscriptionManager.from(context);
            int curConSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            TelephonyManager tm = TelephonyManager.from(context);
            Log.i(TAG, "setDefaultDataSubId: subId is " + subId + "curConSubId is" + curConSubId);
            if (curConSubId == subId) {
                return;
            }
            boolean enable = false;
            if (tm.getDataEnabled(curConSubId) || tm.getDataEnabled(subId)) {
                Log.i(TAG, "tm.setDataEnabled(curConSubId, false) ");
                tm.setDataEnabled(curConSubId, false);
                enable = true;
            }
            subscriptionManager.setDefaultDataSubId(subId);
            if (enable) {
                Log.i(TAG, "tm.setDataEnabled(subId, true) ");
                tm.setDataEnabled(subId, true);
            }
            //Toast.makeText(context, R.string.data_switch_started, Toast.LENGTH_LONG).show();
        }
    }
}
