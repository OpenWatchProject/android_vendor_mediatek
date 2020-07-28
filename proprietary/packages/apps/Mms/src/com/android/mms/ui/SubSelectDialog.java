/*
 * Copyright (C) 2008 Esmertec AG.
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

package com.android.mms.ui;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnDismissListener;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.mms.MmsApp;
import com.android.mms.R;

import java.util.ArrayList;
import java.util.List;

public class SubSelectDialog {
    public interface SubClickAndDismissListener {
        public void onDialogClick(int subId, Intent intent);

        public void onDialogDismiss();

        public void onCancelClick();
    }

    private int mSelectedSubId;
    private List<SubscriptionInfo> mSubListInfo = new ArrayList<SubscriptionInfo>();
    private List<SubscriptionInfo> mEmptySubListInfo = new ArrayList<SubscriptionInfo>();
    private SubChooseAdapter mSubAdapter;
    private Context mContext;
    private SubClickAndDismissListener mListener;
    private boolean mIsOneSubSelected = false;

    public SubSelectDialog(Context context, SubClickAndDismissListener listener) {
        mContext = context;
        mListener = listener;
    }

    private void updateSubInfoList() {
        int simCount = TelephonyManager.getDefault().getSimCount();
        mSubListInfo.clear();
        mEmptySubListInfo.clear();
        for (int slotId = 0; slotId < simCount; slotId++) {
            SubscriptionInfo subInfoRecordInOneSim = SubscriptionManager.from(
                    MmsApp.getApplication()).getActiveSubscriptionInfoForSimSlotIndex(slotId);
            if (subInfoRecordInOneSim != null) {
                mSubListInfo.add(subInfoRecordInOneSim);
            } else {
                // MR1 need fix
                // SubscriptionInfo infoRecord = new SubscriptionInfo();
                // infoRecord.getDisplayName().toString() = String.format(
                // mContext.getResources().getString(R.string.empty_sim_name),
                // slotId + 1);
                // infoRecord.getSimSlotIndex() =
                // SubscriptionManager.SIM_NOT_INSERTED;
                // mEmptySubListInfo.add(infoRecord);
            }
        }
        if (mSubListInfo == null || mSubListInfo.size() == 0) {
            return;
        }
        for (int i = 0; i < mEmptySubListInfo.size(); i++) {
            mSubListInfo.add(mEmptySubListInfo.get(i));
        }
    }

    public AlertDialog showSubSelectedDialog(boolean overridePref,
            String title, final Intent intent) {
        final int activeSimCount = getActiveSimCount(mContext);
        if (activeSimCount == 0) {
            return null;
        } else {
            if (activeSimCount > 1 && overridePref) {
                // SUB selection, always ask, show select SIM dialog even only 1
                // SIM inserted.
                updateSubInfoList();
                AlertDialog.Builder b = new AlertDialog.Builder(mContext);
                mSubAdapter = new SubChooseAdapter(mContext, mSubListInfo);

                if (title != null) {
                    b.setTitle(title);
                } else {
                    b.setTitle(mContext.getString(R.string.sim_selected_dialog_title));
                }

                b.setCancelable(true);
                b.setAdapter(mSubAdapter, new DialogInterface.OnClickListener() {
                    @SuppressWarnings("unchecked")
                    public final void onClick(DialogInterface dialog, int which) {
                        SubscriptionInfo subInfoRecord = mSubListInfo.get(which);
                        if (subInfoRecord != null) {
                            mSelectedSubId = subInfoRecord.getSubscriptionId();
                            mListener.onDialogClick(mSelectedSubId, intent);
                            dialog.dismiss();
                        }
                    }
                });
                b.setOnDismissListener(new OnDismissListener() {

                    @Override
                    public void onDismiss(DialogInterface dialog) {
                        mListener.onDialogDismiss();
                    }
                });
                AlertDialog subSelectDialog = b.create();
                subSelectDialog.show();
                return subSelectDialog;
            } else {
                mSelectedSubId = SubscriptionManager.getDefaultSmsSubscriptionId();
                mListener.onDialogClick(mSelectedSubId, intent);
                return null;
            }
        }
    }

    public int getActiveSimCount(Context context) {
        return SubscriptionManager.from(MmsApp.getApplication()).getActiveSubscriptionInfoCount();
    }

}
