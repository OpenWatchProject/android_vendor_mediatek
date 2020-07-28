/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.android.music;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.widget.Button;
import android.widget.Toast;

/** M: use DialogFragment to show Dialog */
public class DeleteDialogFragment extends DialogFragment{

    private static final String TAG = "DeleteItems";
    private static final String KEY_SINGLE = "single";
    private static final String TRACK_ID = "track";
    private long [] mItemList = {-1};

    /**
     * M: create a instance of DeleteItems
     *
     * @param single
     *            if the number of files to be deleted is only one ?
     * @return the instance of DeleteDialogFragment
     */
    public static DeleteDialogFragment newInstance(Boolean single,
                                                   long id,
                                                   int strId,
                                                   String trackName) {
        DeleteDialogFragment frag = new DeleteDialogFragment();
        Bundle args = new Bundle();
        args.putBoolean(KEY_SINGLE, single);
        args.putLong(TRACK_ID, id);
        args.putInt(MusicUtils.DELETE_DESC_STRING_ID, strId);
        args.putString(MusicUtils.DELETE_DESC_TRACK_INFO, trackName);
        frag.setArguments(args);
        return frag;
    }

    @Override
    /**
     * M: create a dialog
     */
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        MusicLogUtils.d(TAG, "<onDELTEDialog>");
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        String alertMsg = null;
        mItemList[0] = (int) getArguments().getLong(TRACK_ID);
        MusicLogUtils.d(TAG, "Delete mList item id" + mItemList[0] +
        ", Track id = " + getArguments().getLong(TRACK_ID));
        alertMsg = String.format(getString(
                   getArguments().getInt(MusicUtils.DELETE_DESC_STRING_ID)),
                   getArguments().getString(MusicUtils.DELETE_DESC_TRACK_INFO));
        builder.setMessage(alertMsg).setPositiveButton(
                getString(R.string.delete_confirm_button_text),
                new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            int deleteNum = -1;
               deleteNum = MusicUtils.deleteTracks(getActivity()
                             .getApplicationContext(), mItemList);
               if (deleteNum != -1) {
               MusicUtils.showDeleteToast(1, getActivity().getApplicationContext());
                        }
                    }
                    }
                )
                .setNegativeButton(
                getString(R.string.cancel),
                new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                        }
                    }
                    );
        builder.setTitle(R.string.delete_item);
        builder.setIcon(android.R.drawable.ic_dialog_alert);
        Dialog dialog = builder.create();
        dialog.setCanceledOnTouchOutside(false);
        return dialog;
    }

    /**
     * M: change the buttons to disable or enable
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
                MusicLogUtils.d(TAG, " set button state to " + btn.isEnabled());
            } else {
                MusicLogUtils.d(TAG, "get button" + whichButton + " from dialog is null ");
            }
        }
    }
}

   /**
     * M: use handler to start and finish the operation.
     */
