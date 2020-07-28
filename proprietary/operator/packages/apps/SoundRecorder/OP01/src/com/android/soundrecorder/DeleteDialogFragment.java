/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.android.soundrecorder;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.widget.Button;

/** M: use DialogFragment to show Dialog. */
public class DeleteDialogFragment extends DialogFragment
        implements DialogInterface.OnClickListener {

    private static final String TAG = "SR/DeleteDialogFragment";
    private static final String KEY_SINGLE = "single";
    private DialogInterface.OnClickListener mClickListener = null;

    /**
     * M: create a instance of DeleteDialogFragment.
     *
     * @param single
     *            if the number of files to be deleted is only one ?
     * @return the instance of DeleteDialogFragment
     */
    public static DeleteDialogFragment newInstance(Boolean single) {
        DeleteDialogFragment frag = new DeleteDialogFragment();
        Bundle args = new Bundle();
        args.putBoolean(KEY_SINGLE, single);
        frag.setArguments(args);
        return frag;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        LogUtils.i(TAG, "<onCreateDialog>");
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        String alertMsg = null;
        if (getArguments().getBoolean(KEY_SINGLE)) {
            alertMsg = getString(R.string.alert_delete_single);
        } else {
            alertMsg = getString(R.string.alert_delete_multiple);
        }

        builder.setTitle(R.string.delete).setIcon(android.R.drawable.ic_dialog_alert).setMessage(
                alertMsg).setPositiveButton(getString(R.string.ok), this).setNegativeButton(
                getString(R.string.cancel), null);
        Dialog dialog = builder.create();
        dialog.setCanceledOnTouchOutside(false);
        return dialog;
    }

    @Override
    public void onClick(DialogInterface arg0, int arg1) {
        if (null != mClickListener) {
            mClickListener.onClick(arg0, arg1);
        }
    }

    /**
     * M: set listener of OK button.
     *
     * @param listener the listener to be set
     */
    public void setOnClickListener(DialogInterface.OnClickListener listener) {
        mClickListener = listener;
    }

    /**
     * M: update the message of dialog, single/multi file/files to be deleted.
     *
     * @param single
     *            if single file to be deleted
     */
    public void setSingle(boolean single) {
        AlertDialog dialog = (AlertDialog) getDialog();
        if (null != dialog) {
            if (single) {
                dialog.setMessage(getString(R.string.alert_delete_single));
            } else {
                dialog.setMessage(getString(R.string.alert_delete_multiple));
            }
        }
    }
    /**
     * M: change the buttons to disable or enable.
     *
     * @param whichButton to be setting state
     * @param isEnable whether enable button or disable
     */
    public void setButton(int whichButton, boolean isEnable) {
        AlertDialog dialog = (AlertDialog) getDialog();
        if (null != dialog) {
            Button btn = dialog.getButton(whichButton);
            if (btn != null) {
                btn.setEnabled(isEnable);
                LogUtils.d(TAG, " set button state to " + btn.isEnabled());
            } else {
                LogUtils.d(TAG, "get button" + whichButton + " from dialog is null ");
            }
        }
    }
}