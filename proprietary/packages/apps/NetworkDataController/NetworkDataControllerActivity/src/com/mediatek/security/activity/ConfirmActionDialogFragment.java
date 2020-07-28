/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.security;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;

import com.mediatek.security.PermControlUtils;
import com.mediatek.security.Log;
import com.mediatek.security.R;
import com.mediatek.security.datamanager.CheckedPermRecord;

public final class ConfirmActionDialogFragment extends DialogFragment {
    public static final String ARG_ACTION = "ACTION";

    public static interface OnActionConfirmedListener {
        public void onActionConfirmed(boolean confirmed);
    }

    public static ConfirmActionDialogFragment newInstance(
            @NonNull ArrayList<CheckedPermRecord> records) {
        Bundle arguments = new Bundle();
        arguments.putParcelableArrayList(ARG_ACTION, records);
        ConfirmActionDialogFragment fragment = new ConfirmActionDialogFragment();
        fragment.setArguments(arguments);
        return fragment;
    }

    @Override
    public Dialog onCreateDialog(Bundle bundle) {
        LayoutInflater inflater = LayoutInflater.from(getContext());
        final TextView view = (TextView )inflater.inflate(R.layout.dialog_title,
                null);
        view.setText(R.string.warning_network_title);
        return new AlertDialog.Builder(getContext())
                .setCustomTitle(view)
                .setMessage(getWarningMessage())
                .setNegativeButton(R.string.cancel, (dialog, which) -> {
                    Activity activity = getActivity();
                    if (activity instanceof OnActionConfirmedListener) {
                        ((OnActionConfirmedListener) activity)
                                .onActionConfirmed(false);
                    }
                })
                .setPositiveButton(R.string.ok, (dialog, which) -> {
                    Activity activity = getActivity();
                    if (activity instanceof OnActionConfirmedListener) {
                        ((OnActionConfirmedListener) activity)
                                .onActionConfirmed(true);
                    }
                }).create();
    }

    private String getWarningMessage() {
        String[] array = getContext().getResources().getStringArray(
                R.array.application_warnings);
        HashMap<String, String> warningMap = new HashMap<String, String>();
        for (String str : array) {
            String[] splitResult = str.split("\\|", 2);
            warningMap.put(splitResult[0], splitResult[1]);
        }

        array = getContext().getResources()
                .getStringArray(R.array.uid_packages);
        HashMap<Integer, String> uidMap = new HashMap<Integer, String>();
        for (String str : array) {
            String[] splitResult = str.split("\\|", 2);
            uidMap.put(Integer.valueOf(splitResult[0]), splitResult[1]);
        }

        final ArrayList<CheckedPermRecord> records = getArguments()
                .getParcelableArrayList(ARG_ACTION);
        StringBuilder builder = new StringBuilder();
        boolean isLoadOthers = false;
        for (CheckedPermRecord record : records) {
            String pkgName;
            String[] pkgNames = record.splitPkgName();
            if (pkgNames.length == 1) {
                pkgName = pkgNames[0];
            } else {
                pkgName = uidMap.get(record.mUid);
                if (pkgName == null) {
                    pkgName = pkgNames[0];
                }
            }
            String warningMessage = warningMap.get(pkgName);
            if (warningMessage == null && isLoadOthers == false) {
                warningMessage = getContext().getString(
                        R.string.warning_network);
                isLoadOthers = true;
            }
            if (warningMessage != null) {
                builder.append(" * ");
                builder.append(warningMessage);
                builder.append("\n");
            }
        }
        return builder.toString();
    }
}
