/*
 * Copyright (C) 2018 The Android Open Source Project
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

package com.mediatek.settings.network;

import android.app.Dialog;
import android.app.settings.SettingsEnums;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;

import androidx.appcompat.app.AlertDialog;

import com.android.ims.ImsManager;
import com.android.settings.R;
import com.android.settings.core.instrumentation.InstrumentedDialogFragment;

/**
 * Dialog Fragment to show dialog for "enhanced 4G LTE"
 *
 */
public class Enhanced4gLteDialogFragment extends InstrumentedDialogFragment implements
        DialogInterface.OnClickListener {

    public static final String TAG = "Enhanced4gLteDialogFragment";

    public static final int TYPE_VOLTE_UNAVAILABLE_DIALOG = 0;

    private static final String ARG_DIALOG_TYPE = "dialog_type";
    private static final String ARG_SUB_ID = "subId";

    private SubscriptionManager mSubscriptionManager;
    private ImsManager mImsManager;

    private int mType;
    private int mSubId;

    public static Enhanced4gLteDialogFragment newInstance(int type, int subId) {
        final Enhanced4gLteDialogFragment dialogFragment = new Enhanced4gLteDialogFragment();

        Bundle args = new Bundle();
        args.putInt(ARG_DIALOG_TYPE, type);
        args.putInt(ARG_SUB_ID, subId);
        dialogFragment.setArguments(args);

        return dialogFragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSubscriptionManager = getContext().getSystemService(SubscriptionManager.class);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final Bundle bundle = getArguments();
        final Context context = getContext();

        mType = bundle.getInt(ARG_DIALOG_TYPE);
        mSubId = bundle.getInt(ARG_SUB_ID);

        switch (mType) {
            case TYPE_VOLTE_UNAVAILABLE_DIALOG:
                final SubscriptionInfo subInfo =
                        mSubscriptionManager.getActiveSubscriptionInfo(mSubId);
                String displayName = "";
                if (subInfo != null) {
                    displayName = subInfo.getDisplayName().toString();
                }
                return new AlertDialog.Builder(context)
                        .setMessage(context.getString(
                                R.string.ctvolte_unavailable_message, displayName))
                        .setPositiveButton(android.R.string.ok, this)
                        .setNegativeButton(android.R.string.cancel, null)
                        .create();

            default:
                throw new IllegalArgumentException("unknown type " + mType);
        }
    }

    @Override
    public int getMetricsCategory() {
        return SettingsEnums.MOBILE_DATA_DIALOG;
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        switch (mType) {
            case TYPE_VOLTE_UNAVAILABLE_DIALOG:
                ImsManager imsMgr = ImsManager.getInstance(
                        getContext(), SubscriptionManager.getPhoneId(mSubId));
                imsMgr.setEnhanced4gLteModeSetting(true);
                break;

            default:
                throw new IllegalArgumentException("unknown type " + mType);
        }
    }

}
