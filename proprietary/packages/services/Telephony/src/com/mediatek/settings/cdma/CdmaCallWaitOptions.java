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

package com.mediatek.settings.cdma;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.SystemProperties;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import com.android.internal.telephony.Phone;
import com.android.phone.PhoneUtils;
import com.android.phone.R;

import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;

/**
 * CDMA Call Wait Option. Remove CNIR and move CW option to cdma call option.
 */
public class CdmaCallWaitOptions {
    private static final String LOG_TAG = "Settings/CdmaCallWaitOptions";

    /**
     * Should be unique. Take VoicemailDialogUtil for ref.
     * The dialog IDs used in CallFeaturesSetting need refactory.
     */
    public static final int CW_MODIFY_DIALOG = 1000;
    private Phone mPhone;
    private Context mContext;
    private static final String[] CW_HEADERS = {"*74", "*740"}; //cw enable and disable

    /**
     * Constructor.
     *
     * @param context the context for the dialog.
     * @param phone the current phone.
     */
    public CdmaCallWaitOptions(Context context, Phone phone) {
        mContext = context;
        mPhone = phone;
    }

    /**
     * Create the call wait setting dialog.
     *
     * @return the created dialog object.
     */
    public Dialog createDialog() {
        final Dialog dialog = new Dialog(mContext, R.style.CWDialogTheme);
        dialog.setContentView(R.layout.mtk_cdma_cf_dialog);
        dialog.setTitle(R.string.labelCW);

        final RadioGroup radioGroup = (RadioGroup) dialog.findViewById(R.id.group);

        final TextView textView = (TextView) dialog.findViewById(R.id.dialog_sum);
        if (textView != null) {
            textView.setVisibility(View.GONE);
        } else {
            Log.d(LOG_TAG, "--------------[text view is null]---------------");
        }

        EditText editText = (EditText) dialog.findViewById(R.id.EditNumber);
        if (editText != null) {
            editText.setVisibility(View.GONE);
        }

        ImageButton addContactBtn = (ImageButton) dialog.findViewById(R.id.select_contact);
        if (addContactBtn != null) {
            addContactBtn.setVisibility(View.GONE);
        }

        Button dialogSaveBtn = (Button) dialog.findViewById(R.id.save);
        if (dialogSaveBtn != null) {
            dialogSaveBtn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (radioGroup.getCheckedRadioButtonId() == -1) {
                        dialog.dismiss();
                        return;
                    }
                    int select = radioGroup.getCheckedRadioButtonId() == R.id.enable ? 0 : 1;
                    String cw = CW_HEADERS[select];
                    dialog.dismiss();
                    setCallWait(cw);
                }
            });
        }

        Button dialogCancelBtn = (Button) dialog.findViewById(R.id.cancel);
        if (dialogCancelBtn != null) {
                dialogCancelBtn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    dialog.dismiss();
                }
            });
        }
        return dialog;
    }

    /**
     * Create the call wait setting dialog.
     *
     * @param cw the string need to pass to dailer.
     */
    private void setCallWait(String cw) {
        Log.d(LOG_TAG, "[setCallWait][cw = " + cw + "], mPhone = " + mPhone);
        if (mPhone == null ||
            cw == null || cw.isEmpty()) {
            return;
        }

        //Show error dialog instead of make CW call in no c2k version.
        if (RatConfiguration.isC2kSupported()) {
            Intent intent = new Intent(Intent.ACTION_CALL);
            intent.setData(Uri.parse("tel:" + cw));
            PhoneAccountHandle phoneAccountHandle = PhoneUtils.makePstnPhoneAccountHandle(mPhone);

            intent.putExtra(TelecomManager.EXTRA_PHONE_ACCOUNT_HANDLE, phoneAccountHandle);
            mContext.startActivity(intent);
        } else {
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setTitle(R.string.error_updating_title);
            builder.setMessage(R.string.exception_error);
            builder.setCancelable(false);
            builder.setPositiveButton(R.string.close_dialog, null);
            AlertDialog dialog = builder.create();
            dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
            dialog.show();
        }
    }
}
