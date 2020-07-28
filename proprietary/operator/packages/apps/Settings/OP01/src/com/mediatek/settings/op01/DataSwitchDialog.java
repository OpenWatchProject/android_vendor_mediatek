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

package com.mediatek.settings.op01;

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
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

//DataSwitch confirm
public class DataSwitchDialog extends Activity {
    private static final String TAG = "DataSwitchDialog";
    private static final int DIALOG_ID_CHANGE_DATA_CONNECTION = 1001;
    private static final int VOLTE_DIALOG = 0x01;
    private static final int RCS_DIALOG = 0x02;

    private Context mContext;
    private Activity mActivity;
    private IntentFilter mIntentFilter;

    private int mToOpenSubId;

    private BroadcastReceiver mSubReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d("@M_" + TAG, "mSubReceiver action = " + action);
            if (action.equals(TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE)
                || action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                int[] subids = SubscriptionManager.from(mContext).getActiveSubscriptionIdList();
                if (subids == null || subids.length <= 1) {
                    Log.d("@M_" + TAG, "mSubReceiver dismiss AlertDlg");
                    finish();
                }
            } else if (TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED.equals(action)) {
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                dealDataConnectionChanged(subId);
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

        mToOpenSubId =
            getIntent().getIntExtra("subId", SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        Log.d("@M_" + TAG, "mToOpenSubId =" + mToOpenSubId);
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
        Log.d("@M_" + TAG, "onDestroy()");
        unregisterReceiver(mSubReceiver);
        mToOpenSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        super.onDestroy();
    }

    private AlertDialog getChangeDataConnDialog(Builder builder) {
        Log.d("@M_" + TAG, "getChangeDataConnDialog");
        //builder.setTitle(mContext.getString(R.string.change_data_conn_title));
        int showType =
            getIntent().getIntExtra("showType", VOLTE_DIALOG);
        if (showType == VOLTE_DIALOG) {
            builder.setMessage(mContext.getString(R.string.change_data_conn_alert_volte));
        } else if (showType == RCS_DIALOG) {
            builder.setMessage(mContext.getString(R.string.change_data_conn_alert_rcs));
        } else {
            builder.setMessage(mContext.getString(R.string.change_data_conn_alert));
        }

        final Context context = getApplicationContext();
        builder.setPositiveButton(android.R.string.yes,
               new DialogInterface.OnClickListener() {
                      public void onClick(DialogInterface dialog, int whichButton) {
                           Log.d("@M_" + TAG, "Click ok, Switch data to subId = " + mToOpenSubId);
                           setDefaultDataSubId(context, mToOpenSubId);
                           finish();
                       }
                   });

        builder.setNegativeButton(android.R.string.cancel,
               new DialogInterface.OnClickListener() {
                      public void onClick(DialogInterface dialog, int whichButton) {
                           Log.d("@M_" + TAG, "Perform On click cancel");
                           finish();
                       }
                   });

        return builder.create();
    }

    private void dealDataConnectionChanged(int subId) {
        Log.i("@M_" + TAG, "dealDataConnectionChanged: subId is " + subId +
                ", mToOpenSubId is " + mToOpenSubId);
        if (mToOpenSubId == subId) {
            Log.d("@M_" + TAG, "dealDataConnectionChanged finish ");
            finish();
        }
    }

    private void setDefaultDataSubId(final Context context, final int subId) {
        if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            final SubscriptionManager subscriptionManager = SubscriptionManager.from(context);
            int curConSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            TelephonyManager tm = TelephonyManager.from(context);
            Log.i("@M_" + TAG, "setDefaultDataSubId: subId is " + subId + "curConSubId is"
                    + curConSubId);
            if (curConSubId == subId) {
                return;
            }
            subscriptionManager.setDefaultDataSubId(subId);
            Toast.makeText(context, R.string.data_switch_started, Toast.LENGTH_LONG).show();
        }
    }
}