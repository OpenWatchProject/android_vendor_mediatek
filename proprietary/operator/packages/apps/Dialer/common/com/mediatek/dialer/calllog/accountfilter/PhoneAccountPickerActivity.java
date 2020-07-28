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

package com.mediatek.dialer.calllog.accountfilter;

import static android.Manifest.permission.READ_PHONE_STATE;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.text.TextUtils;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.util.Log;
import android.view.KeyEvent;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.telecom.TelecomManager;
import android.telecom.PhoneAccountHandle;

import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.dialer.common.LogUtils;
import com.mediatek.dialer.common.plugin.R;

import java.util.List;
import java.util.ArrayList;

/** M: [Call Log Account Filter] used for phone account pick @{ */
public class PhoneAccountPickerActivity extends Activity {
    public static final String TAG = "PhoneAccountPickerActivity:";
    public static final int PHONE_ACCOUNT_PICKER_DIALOG = 100;
    private AlertDialog mDialog;

    private static PhoneAccountPickerAdapter sAdapter;
    private static boolean sShowSelection = false;
    private static int sSelection = -1;

    private String mAccountPrefered = "all_account";
    private static final String KEY_SELECTION_ID = "key_selection_id";
    private boolean mIsRegistedReceiver = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);

        if (this.checkSelfPermission(READ_PHONE_STATE) != PackageManager.PERMISSION_GRANTED) {
            Log.d(TAG, "onCreate NO READ_PHONE_STATE requestPermission");
            requestPermissions(new String[] {READ_PHONE_STATE}, 0);
            finish();
            return;
        }

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_DEFAULT_VOICE_SUBSCRIPTION_CHANGED);
        intentFilter.addAction(TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE);
        intentFilter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        registerReceiver(mReceiver, intentFilter);
        mIsRegistedReceiver = true;

        // phone account Selection, select an account to filter out the Calllog
        final ArrayList<AccountItem> items = createPhoneAccountItems();

        Bundle bundle = getIntent().getExtras();
        mAccountPrefered = bundle.getString(FilterOptions.FILTER_ACCOUNT_PREFER);
        LogUtils.printSensitiveInfo(TAG, "onCreate mAccountPrefered " + mAccountPrefered);

        sAdapter = new PhoneAccountPickerAdapter(this);
        sShowSelection = true;
        sAdapter.setItemData(items);
        sAdapter.setShowSelection(sShowSelection);
        sSelection = getPreferedAccountItemIndex(items);
        Log.d(TAG, "sSelection = " + sSelection);
        int selectionId;
        if (savedInstanceState != null) {
            selectionId = savedInstanceState.getInt(KEY_SELECTION_ID);
            Log.d(TAG, "selectionId = " + selectionId);
            sSelection = selectionId;
        }
        Log.d(TAG, "onCreate() sSelection = " + sSelection);
        sAdapter.setSelection(sSelection);
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume ");

        if (sAdapter == null || sAdapter.isEmpty()) {
            finish();
            return;
        }
        showDialog(PHONE_ACCOUNT_PICKER_DIALOG);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Log.d(TAG, "onCreateDialog sSelection = " + sSelection);
        if (id != PHONE_ACCOUNT_PICKER_DIALOG) {
            return null;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(this.getResources().getString(R.string.select_account));
        if (sShowSelection) {
            builder.setSingleChoiceItems(sAdapter, sSelection,
                    new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            sSelection = which;
                            sAdapter.setSelection(sSelection);
                            sAdapter.notifyDataSetChanged();
                            Log.d(TAG,
                                    "builder.setSingleChoiceItems sSelection = "
                                            + sSelection);
                        }
                    });
        } else {
            builder.setAdapter(sAdapter, new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    final String selectedId = sAdapter.getAccountId(which);
                    Log.d(TAG, " selectedId = " + selectedId);
                    Log.d(TAG, "mAccountPrefered = " + mAccountPrefered);
                    if (!TextUtils.isEmpty(selectedId)) {
                        Log.d(TAG, "onClick set preference");
                        Intent intent = new Intent();
                        intent.putExtra(FilterOptions.SELECTED_ID, selectedId);
                        setResult(FilterOptions.ACTIVITY_RESULT_CODE, intent);
                    }
                    Log.d(TAG, "builder.setAdapter() selectedId = " + selectedId);

                    removeDialog(PHONE_ACCOUNT_PICKER_DIALOG);
                    finish();
                }
            });
        }

        builder.setNegativeButton(android.R.string.cancel,
                new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Log.d(TAG, "setNegativeButton()");
                        removeDialog(PHONE_ACCOUNT_PICKER_DIALOG);
                        finish();
                    }

                });

        // only need "ok" button when show selection
        if (sShowSelection) {
            builder.setPositiveButton(android.R.string.ok,
                    new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            final String selectedId = sAdapter
                                    .getAccountId(sSelection);
                            Log.d(TAG, "setPositiveButton() selectedId = " + selectedId);
                            LogUtils.printSensitiveInfo(TAG,
                                    "setPositiveButton() mAccountPrefer = " + mAccountPrefered);
                            if (!TextUtils.isEmpty(selectedId)) {
                                Log.d(TAG, "setPositiveButton() set preference");
                                Intent intent = new Intent();
                                intent.putExtra(FilterOptions.SELECTED_ID, selectedId);
                                setResult(FilterOptions.ACTIVITY_RESULT_CODE, intent);
                            }
                            removeDialog(PHONE_ACCOUNT_PICKER_DIALOG);
                            finish();
                        }
                    });
        }

        AlertDialog dialog = builder.create();
        dialog.setCanceledOnTouchOutside(false);
        dialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
            @Override
            public boolean onKey(DialogInterface dialog, int keyCode,
                    KeyEvent event) {
                if (keyCode == KeyEvent.KEYCODE_BACK
                        && event.getAction() == KeyEvent.ACTION_DOWN) {
                    removeDialog(PHONE_ACCOUNT_PICKER_DIALOG);
                    finish();
                    return true;
                } else {
                    return false;
                }
            }
        });
        return dialog;
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog, Bundle args) {
        if (id == PHONE_ACCOUNT_PICKER_DIALOG) {
            mDialog = (AlertDialog) dialog;
        }

        super.onPrepareDialog(id, dialog, args);
    }

    /// M: [Call Log Account Filter] @{
    private ArrayList<AccountItem> createPhoneAccountItems() {
        ArrayList<AccountItem> accountItems = new ArrayList<AccountItem>();
        // first item is "all accounts"
        accountItems.add(new AccountItem(R.string.all_accounts,
                PhoneAccountInfoHelper.FILTER_ALL_ACCOUNT_ID));

        final TelecomManager telecomManager = (TelecomManager) this.getSystemService(
                Context.TELECOM_SERVICE);
        final List<PhoneAccountHandle> accounts = telecomManager.getCallCapablePhoneAccounts();
        for (PhoneAccountHandle account : accounts) {
            accountItems.add(new AccountItem(account));
        }
        return accountItems;
    }
    /// @}

    /// M: [Call Log Account Filter] @{
    private int getPreferedAccountItemIndex(ArrayList<AccountItem> data) {
        String id = mAccountPrefered;
        LogUtils.printSensitiveInfo(TAG, "getPreferedAccountItemIndex() id = " + id);
        if (!TextUtils.isEmpty(id) && data != null) {
            for (int i = 0; i < data.size(); i++) {
                if (id.equals(data.get(i).id)) {
                    return i;
                }
            }
        }
        return 0;
    }
    /// @}

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        if (mIsRegistedReceiver) {
            unregisterReceiver(mReceiver);
            mIsRegistedReceiver = false;
        }
    }

    @Override
     protected void onSaveInstanceState(Bundle outState) {
       Log.d(TAG, "onSaveInstanceState enter");
       super.onSaveInstanceState(outState);
       outState.putInt(KEY_SELECTION_ID, sSelection);
     }
    private boolean checkAccountUpdate() {
        TelecomManager telecomManager =
            (TelecomManager) this.getSystemService(Context.TELECOM_SERVICE);

        List<PhoneAccountHandle> handles = telecomManager.getCallCapablePhoneAccounts();
        if (telecomManager.getCallCapablePhoneAccounts().size() <= 1) {
            return true;
        }
        return false;
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (TelephonyIntents.ACTION_DEFAULT_VOICE_SUBSCRIPTION_CHANGED.equals(action) ||
                TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE.equals(action) ||
                TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED.equals(action)) {
                Log.d(TAG, "BroadcastReceiver()-->action = " + action);
                if (checkAccountUpdate()) {
                    removeDialog(PHONE_ACCOUNT_PICKER_DIALOG);
                    PhoneAccountPickerActivity.this.finish();
                }
            }
        }
    };

}
