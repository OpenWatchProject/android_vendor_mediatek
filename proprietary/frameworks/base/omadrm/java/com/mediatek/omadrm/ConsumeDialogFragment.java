/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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
package com.mediatek.omadrm;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;

/** M: use DialogFragment to show Dialog */
public class ConsumeDialogFragment extends DialogFragment
        implements DialogInterface.OnClickListener {

    private static final String TAG = "OmaDrmUtils/ConsumeDialogFragment";
    private DialogInterface.OnClickListener mClickListener = null;
    private static StringBuilder mAlertMsg;

    /**
     * M: create a instance of ConsumeDialogFragment
     *
     * @param single
     *            if the number of files to be deleted is only one ?
     * @return the instance of ConsumeDialogFragment
     */
    public static ConsumeDialogFragment newInstance(StringBuilder alertMsg) {
        mAlertMsg = alertMsg;
        ConsumeDialogFragment frag = new ConsumeDialogFragment();
        return frag;
    }

    @Override
    /**
     * M: create a dialog
     */
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setIcon(android.R.drawable.ic_dialog_info);
        builder.setTitle(com.mediatek.internal.R.string.drm_consume_title);
        builder.setPositiveButton(android.R.string.ok, this);
        builder.setNegativeButton(android.R.string.cancel, this);
        builder.setMessage(mAlertMsg);
        Dialog dialog = builder.create();
        return dialog;
    }

    @Override
    public void onPause() {
        Log.d(TAG, "showConsumerDialog: dismiss dialog when activity paused");
        this.dismissAllowingStateLoss(); // dismiss dialog here when activity pause
        super.onPause();
    }

    /**
     * M: the process of click OK button on dialog
     */
    @Override
    public void onClick(DialogInterface arg0, int arg1) {
        if (null != mClickListener) {
            mClickListener.onClick(arg0, arg1);
        }
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        Log.d(TAG, "showConsumerDialog onDismiss dialog: " + dialog);
        if (null != mClickListener) {
            mClickListener.onClick(dialog, DialogInterface.BUTTON_NEGATIVE);
        }
        super.onDismiss(dialog);
    }
    /**
     * M: set listener of OK button
     *
     * @param listener the listener to be set
     */
    public void setOnClickListener(DialogInterface.OnClickListener listener) {
        mClickListener = listener;
    }

}