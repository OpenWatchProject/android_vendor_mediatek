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
package com.mediatek.contacts.widget;

import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.os.Bundle;
import android.view.WindowManager;

import com.android.contacts.R;

import com.mediatek.contacts.util.Log;

public class SimpleProgressDialogFragment extends DialogFragment {
    private static String TAG = "SimpleProgressDialogFragment";

    private static final String DIALOG_TAG = "progress_dialog";

    private static SimpleProgressDialogFragment getInstance(FragmentManager fm) {
        SimpleProgressDialogFragment dialog = getExistDialogFragment(fm);
        Log.i(TAG, "[getInstance]dialog:" + dialog);
        if (dialog == null) {
            dialog = new SimpleProgressDialogFragment();
            Log.i(TAG, "[getInstance]create new dialog " + dialog + " in " + fm);
        }
        return dialog;
    }

    private static SimpleProgressDialogFragment getExistDialogFragment(FragmentManager fm) {
        return (SimpleProgressDialogFragment) fm.findFragmentByTag(DIALOG_TAG);
    }

    public static void show(FragmentManager fm) {
        Log.d(TAG, "[show]show dialog for " + fm);
        SimpleProgressDialogFragment dialog = getInstance(fm);
        if (dialog.isAdded()) {
            Log.d(TAG, "[show]dialog is already shown: " + dialog);
        } else {
            Log.d(TAG, "[show]dialog created and shown: " + dialog);
            // M:fix Fragment IllegalStateException when show dialog and no use
            // dialog.show function@{
            FragmentTransaction ft = fm.beginTransaction();
            ft.add(dialog, DIALOG_TAG);
            ft.commitAllowingStateLoss();
            //@}

           // dialog.show(fm, DIALOG_TAG);
            dialog.setCancelable(false);
        }
    }

    public static void dismiss(FragmentManager fm) {
        Log.d(TAG, "[dismiss]dismiss dialog for " + fm);

        if (fm == null) {
            return;
        }

        SimpleProgressDialogFragment dialog = getExistDialogFragment(fm);
        if (dialog == null) {
            Log.w(TAG, "[dismiss]dialog never shown before, no need dismiss");
            return;
        }
        if (dialog.isAdded()) {
            Log.d(TAG, "[dismiss]force dismiss dialog: " + dialog);
            dialog.dismissAllowingStateLoss();
        } else {
            Log.d(TAG, "[dismiss]dialog not added, dismiss failed: " + dialog);
        }
    }

    public SimpleProgressDialogFragment() {
    }

    @Override
    public String toString() {
        return String.valueOf(hashCode());
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Log.d(TAG, "[onCreateDialog]");
        ProgressDialog dialog = new ProgressDialog(getActivity());
        dialog.setMessage(getActivity().getString(R.string.please_wait));
        return dialog;
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "[onResume]");
        getDialog().getWindow().clearFlags(WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);
    }
}
