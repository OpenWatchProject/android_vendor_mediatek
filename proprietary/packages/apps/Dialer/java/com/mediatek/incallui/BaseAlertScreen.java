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
package com.mediatek.incallui;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.widget.EditText;

import com.android.incallui.Log;
import com.android.internal.app.AlertActivity;
import com.mediatek.incallui.utils.InCallUtils;
import com.mediatek.incallui.volte.AddMemberEditView;
import com.mediatek.incallui.volte.AddMemberScreen;

import java.util.HashMap;
import java.util.Map;

/**
 * [VoLTE Conference] BaseAlertScreen.
 */
public class BaseAlertScreen extends AlertActivity {

  private static final String TAG = "BaseAlertScreen";
  private static final int ADD_CONTACT_RESULT = 10000;
  private static final int ADD_CONFERENCE_MEMBER_RESULT = 10001;
  private boolean mWaitForResult = false;
  public Map<String, String> mContactsMap = new HashMap<String, String>();
  public EditText mTransferEditeView;
  public static AddMemberEditView mEditView;

  @Override
  protected void onPause() {
    super.onPause();
    Log.d(TAG, "onPause()... mWaitForResult: " + mWaitForResult);
    // If we are pausing while not waiting for result from startActivityForResult(),
    // finish ourself. consider case of home key pressed while we are showing.
    if (!mWaitForResult) {
      finish();
    }
    mWaitForResult = false;
  }

  /**
   * chooseFromContacts.
   * @param object Object
   */
  public void chooseFromContacts(Object object) {
    mWaitForResult = true;
    Intent intent = new Intent(Intent.ACTION_PICK, Phone.CONTENT_URI);
    /// M: fix CR:ALPS02672772,can not select sip/Ims number in contact selection activity. @{
    intent.putExtra("isCallableUri", true);
    /// @}
    if (object instanceof AddMemberScreen) {
      startActivityForResult(intent, ADD_CONFERENCE_MEMBER_RESULT);
    } else {
      startActivityForResult(intent, ADD_CONTACT_RESULT);
    }
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    Log.d(TAG, "onActivityResult, request = " + requestCode + ", ret = " + resultCode);
    if (RESULT_OK == resultCode) {
      switch (requestCode) {
        case ADD_CONTACT_RESULT:
          handleChooseContactsResult(getApplicationContext(), data, ADD_CONTACT_RESULT);
          break;
        case ADD_CONFERENCE_MEMBER_RESULT:
          handleChooseContactsResult(getApplicationContext(), data, ADD_CONFERENCE_MEMBER_RESULT);
          break;
        default:
          break;
      }
    }
  }

  /**
   * handleChooseContactsResult.
   * @param context Context
   * @param data Intent
   * @param requestCode int
   */
  public void handleChooseContactsResult(Context context, Intent data, int requestCode) {
    Uri uri = data.getData();
    // query from contacts
    String name = null;
    String number = null;
    Cursor c = context.getContentResolver().query(uri,
        new String[] { Phone.DISPLAY_NAME, Phone.NUMBER }, null, null, null);
    try {
      if (c.moveToNext()) {
        name = c.getString(0);
        number = c.getString(1);
        mContactsMap.put(number, name);
      }
    } finally {
      c.close();
    }
    Log.d(TAG, "ChooseContactsResult " + uri + ", name = " + name + ", number = " +
        InCallUtils.formatSensitiveValue(number));
    if (requestCode == ADD_CONFERENCE_MEMBER_RESULT) {
      mEditView.append(number + ",");
    }
    if (requestCode == ADD_CONTACT_RESULT) {
      mTransferEditeView.setText(number);
    }
  }

  protected boolean hasPermission(String permission) {
    return this.checkSelfPermission(permission) == PackageManager.PERMISSION_GRANTED;
  }
}
