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
package com.mediatek.incallui.blindect;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.telephony.PhoneNumberFormattingTextWatcher;
import android.telephony.PhoneNumberUtils;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.ImageButton;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.android.incallui.Log;
import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
import com.android.incallui.call.TelecomAdapter;
import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.mediatek.contacts.util.PhoneNumberFormatter;
import com.mediatek.incallui.BaseAlertScreen;

import mediatek.telecom.MtkTelecomManager;

public class AddTransferNumberScreen extends BaseAlertScreen implements
    DialogInterface.OnClickListener, TextWatcher {

  private static final String TAG = "AddTransferNumberScreen";
  private ImageButton mChooseContact;
  private Button mTransferButton;
  private String mCallId;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    Log.d(TAG, "onCreate...");
    super.onCreate(savedInstanceState);
    mCallId = getIntent().getExtras().getString("CallId");
    final AlertController.AlertParams alertView = mAlertParams;
    alertView.mView = createView();
    alertView.mTitle = getResources().getString(R.string.add_transfer_call_number);
    alertView.mPositiveButtonText = getString(R.string.menu_ect);
    alertView.mPositiveButtonListener = this;
    alertView.mNegativeButtonText = getString(com.android.internal.R.string.cancel);
    alertView.mNegativeButtonListener = this;
    AddTransferNumberScreenController.getInstance().setAddTransferNumberDialog(this);
    setupAlert();
    mTransferButton = (Button) mAlert.getButton(DialogInterface.BUTTON_POSITIVE);
    if (mTransferButton != null) {
      mTransferButton.setEnabled(false);
    }
  }

  public View createView() {
    View view = getLayoutInflater().inflate(R.layout.mtk_add_transfer_number, null);
    mChooseContact = (ImageButton) view.findViewById(R.id.contact_icon);
    mChooseContact.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        // ALPS03684731 add toast to indicate permission
        if (!hasPermission(android.Manifest.permission.READ_CONTACTS)) {
            Toast.makeText(AddTransferNumberScreen.this,
            R.string.missing_required_permission,
                Toast.LENGTH_SHORT).show();
          return;
        }
        // TODO Auto-generated method stub
        chooseFromContacts(AddTransferNumberScreen.this);
      }
    });
    mTransferEditeView = (EditText) view.findViewById(R.id.add_transfer_call_number);
    mTransferEditeView.addTextChangedListener(new PhoneNumberFormattingTextWatcher());
    mTransferEditeView.addTextChangedListener(this);
    PhoneNumberFormatter.setPhoneNumberFormattingTextWatcher(this, mTransferEditeView);
    // show keyboard
    InputMethodManager inputMethodManager = (InputMethodManager)getSystemService
        (Context.INPUT_METHOD_SERVICE);
    inputMethodManager.showSoftInput(mTransferEditeView, InputMethodManager.SHOW_IMPLICIT);
    return view;
  }

  @Override
  public void beforeTextChanged(CharSequence s, int start, int count, int after) {
    // no-op
  }

  @Override
  public void onTextChanged(CharSequence text, int start, int before, int count) {
    if (mTransferButton != null) {
      mTransferButton.setEnabled(!TextUtils.isEmpty(PhoneNumberUtils.stripSeparators(text
          .toString())));
    }
  }

  @Override
  public void afterTextChanged(Editable s) {
    // no-op
    mTransferEditeView.setSelection(s.length());
  }

  @Override
  protected void onDestroy() {
    AddTransferNumberScreenController.getInstance().clearAddTransferNumberDialog();
    super.onDestroy();
  }

  @Override
  public void onClick(DialogInterface dialog, int which) {
    // TODO Auto-generated method stub
    if (DialogInterface.BUTTON_POSITIVE == which) {
      sendTransferNumber(PhoneNumberUtils.stripSeparators(mTransferEditeView.getText()
          .toString()));
      finish();
    } else if (DialogInterface.BUTTON_NEGATIVE == which) {
      finish();
    }
  }

  public void sendTransferNumber(String number) {
    DialerCall call = CallList.getInstance().getCallById(mCallId);
    Log.d(TAG, "sendTransferNumber->number = " + number + "===callId = " + mCallId
        +"====call = " + call);
    if (call != null
        && call.can(
            mediatek.telecom.MtkCall.MtkDetails.MTK_CAPABILITY_BLIND_OR_ASSURED_ECT)) {
      TelecomAdapter.getInstance().blindExplicitCallTransfer(
          call.getTelecomCall().getDetails().getTelecomCallId(), number,
          MtkTelecomManager.ECT_TYPE_AUTO);
    }
  }
}
