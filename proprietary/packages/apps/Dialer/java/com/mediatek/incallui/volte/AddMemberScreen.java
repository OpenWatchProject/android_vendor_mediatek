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
package com.mediatek.incallui.volte;

import android.content.DialogInterface;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.PhoneLookup;
import android.telephony.PhoneNumberUtils;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.util.Rfc822Tokenizer;
import android.view.View;
import android.widget.ImageButton;
import android.widget.Toast;

import com.android.dialer.util.MoreStrings;
import com.android.incallui.Log;
import com.android.incallui.R;
import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
import com.android.incallui.call.TelecomAdapter;
import com.android.internal.app.AlertController;
import com.mediatek.incallui.BaseAlertScreen;
import com.mediatek.incallui.utils.InCallUtils;
import com.mediatek.incallui.volte.AddMemberEditView.AddMemberEditViewAdatper;

import java.util.ArrayList;
import java.util.List;

/**
 * [VoLTE Conference] AddMemberScreen.
 */
public class AddMemberScreen extends BaseAlertScreen implements DialogInterface.OnClickListener {

  private static ImageButton mChooseContactsView;
  public static final String ADD_CONFERENCE_MEMBER_DIALOG = "add conference_member";
  private static final String LOG_TAG = "VoLteConfAddMemberScreen";
  private int mConferenceId = -1;
  private boolean mIsTestMode;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    log("onCreate...");
    super.onCreate(savedInstanceState);
    /// M: ALPS03889138 Add permission check for AddMemberScreen  @{
    if (!hasPermission(android.Manifest.permission.READ_CONTACTS)) {
      Toast.makeText(AddMemberScreen.this,
          R.string.missing_required_permission,
          Toast.LENGTH_SHORT).show();
      finish();
      return;
    }
    /// @}

    mIsTestMode = InCallUtils.isInTestSim();
    final AlertController.AlertParams p = mAlertParams;
    p.mView = createView();
    p.mTitle = getResources().getString(R.string.volte_add_conference_member_title);
    p.mPositiveButtonText = getString(com.android.internal.R.string.ok);
    p.mNegativeButtonText = getString(com.android.internal.R.string.cancel);
    p.mPositiveButtonListener =  this;
    p.mNegativeButtonListener = this;
    AddMemberScreenController.getInstance().setAddMemberScreen(this);
    setupAlert();
  }

  @Override
  protected void onPause() {
    super.onPause();
  }

  @Override
  protected void onDestroy() {
    log("onDestroy...");
    mContactsMap.clear();
    AddMemberScreenController.getInstance().clearAddMemberScreen();
    super.onDestroy();
  }

  private View createView() {
    View view = getLayoutInflater().inflate(R.layout.mtk_volte_add_conference_member, null);

    mEditView = (AddMemberEditView) view.findViewById(R.id.memeber_editor);
    mChooseContactsView = (ImageButton) view.findViewById(R.id.choose_contacts);
    if (mIsTestMode) {
      mEditView.setHint("");
      mChooseContactsView.setVisibility(View.GONE);
      return view;
    }
    mEditView.setAdapter(new AddMemberEditViewAdatper(this));
    mEditView.setTokenizer(new Rfc822Tokenizer());
    mEditView.requestFocus();

    mChooseContactsView.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        if (!hasPermission(android.Manifest.permission.READ_CONTACTS)) {
          Toast.makeText(AddMemberScreen.this,
              R.string.missing_required_permission,
              Toast.LENGTH_SHORT).show();
          return;
        }
        chooseFromContacts(AddMemberScreen.this);
      }
    });
    return view;
  }

  private void processAddConferenceMemberAction() {
    ArrayList<String> numberList = getInputNumbers();
    if (numberList == null || numberList.isEmpty()) {
      log("[processAddConferenceMemberAction]empty input");
      return;
    }

    String conferenceCallId = AddMemberScreenController.getInstance().getConferenceCallId();
    DialerCall conferenceCall = CallList.getInstance().getCallById(conferenceCallId);
    if (conferenceCall == null
        || !conferenceCall.can(
                mediatek.telecom.MtkCall.MtkDetails.MTK_CAPABILITY_INVITE_PARTICIPANTS)) {
      log("processAddConferenceMemberAction()...can not find a VoLTE conference.");
      return;
    }

    /// M: [log optimize]
    Log.op(conferenceCall, Log.CcOpAction.INVITE, "invite " + numberList.size() + " members");
    TelecomAdapter.getInstance().inviteConferenceParticipants(conferenceCallId, numberList);
  }

  private void log(String msg) {
    Log.d(LOG_TAG, msg);
  }

  @Override
  public void onClick(DialogInterface dialog, int which) {
    // TODO Auto-generated method stub
    if (DialogInterface.BUTTON_POSITIVE == which) {
      processAddConferenceMemberAction();
      finish();
    } else if (DialogInterface.BUTTON_NEGATIVE == which) {
      finish();
    }
  }

  private ArrayList<String> getInputNumbers() {
    Spanned sp = mEditView.getText();
    String inputString = sp.toString();
    inputString = inputString.trim();
    log("getNumbers, numbers = " + InCallUtils.formatSensitiveValue(inputString));
    ArrayList<String> numbers = new ArrayList<String>();
    ArrayList<String> formatNumbers = new ArrayList<String>();

    int i = 0;
    int breakIndex  = -1;
    int cursor = inputString.length();
    String number = "";

    while (i < cursor) {
      char c = inputString.charAt(i);
      if (c == ',' || (!mIsTestMode && c == ';')) {
        if (breakIndex + 1 < i) {
          number = inputString.substring(breakIndex + 1, i);
          if (!TextUtils.isEmpty(number)) {
            numbers.add(number);
          }
        }
        breakIndex = i;
      }
      if (c == '<') {
        breakIndex = i - 1;
      }
      i++;
    }
    if (breakIndex + 1 < cursor) {
      number = inputString.substring(breakIndex + 1, cursor);
      if (!TextUtils.isEmpty(number)) {
        numbers.add(number);
      }
    }

    for (String str : numbers) {
      if (!mIsTestMode) {
        str = PhoneNumberUtils.stripSeparators(str);
      }
      formatNumbers.add(str);
    }
    log("[getInputNumbers]all input numbers: ");
    dumpNumberList(formatNumbers);
    return formatNumbers;
  }

  private String getContactsName(String number) {
    String ret = number;
    if (mContactsMap.containsKey(number)) {
      log("getContactsName, find in map ~~");
      ret = mContactsMap.get(number);
    } else {
      String normalizedNumber = PhoneNumberUtils.normalizeNumber(number);
      if (!TextUtils.isEmpty(normalizedNumber)) {
        Cursor c = getContentResolver().query(
            Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, normalizedNumber),
                new String[] { Phone.DISPLAY_NAME }, null, null, null);
        try {
          if (c != null && c.moveToFirst()) {
            ret = c.getString(0);
            mContactsMap.put(number, ret);
          }
        } finally {
          // M: fix CR:ALPS02360492,dereference after null check.
          if (c != null) {
            c.close();
          }
        }
      }
    }
    log("getContactsName for " + InCallUtils.formatSensitiveValue(number) +
        ", name =" + MoreStrings.toSafeString(ret));
    return ret;
  }

  private void dumpNumberList(List<String> list) {
    log("--------dump NumberList begin-------");
    log("list.size = " + list.size());
    for (int i = 0; i < list.size(); i++) {
      log("index / number: " + i + " / " + InCallUtils.formatSensitiveValue(list.get(i)));
    }
    log("--------dump NumberList end-------");
  }
}
