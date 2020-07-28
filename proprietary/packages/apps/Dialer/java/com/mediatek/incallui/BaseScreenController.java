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

import com.mediatek.incallui.blindect.AddTransferNumberScreen;
import com.mediatek.incallui.blindect.AddTransferNumberScreenController;
import com.mediatek.incallui.volte.AddMemberScreen;
import com.mediatek.incallui.volte.AddMemberScreenController;

/**
 * [VoLTE Conference] BaseScreenController.
 */
public class BaseScreenController {

  private AddTransferNumberScreen mAddTransferNumberDialog;
  private AddMemberScreen mAddMemberDialog;

  /**
   * showDialog.
   * @param context Context
   * @param exrtaParam String
   * @param o Object
   */
  public void showDialog(Context context, String exrtaParam, Object o) {
    Intent intent;
    if (o instanceof AddTransferNumberScreenController) {
      intent = new Intent(context, AddTransferNumberScreen.class);
      intent.putExtra("CallId", exrtaParam);
    } else {
      intent = new Intent(context, AddMemberScreen.class);
    }
    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    context.startActivity(intent);
  }

  /**
   * setDialog.
   * @param o Object
   */
  public void setDialog(Object o) {
    // If there has one "Dialog" already, dismiss it first. quick-click may cause this.
    if (o instanceof AddTransferNumberScreen) {
      if (mAddTransferNumberDialog != null) {
        mAddTransferNumberDialog.finish();
      }
      mAddTransferNumberDialog = (AddTransferNumberScreen) o;
    } else {
      if (mAddMemberDialog != null) {
        mAddMemberDialog.finish();
      }
      mAddMemberDialog = (AddMemberScreen) o;
    }
  }

  /**
   * clearDialog.
   * @param o Object
   */
  public void clearDialog(Object o) {
    if (o instanceof AddTransferNumberScreenController && mAddTransferNumberDialog != null
        && mAddTransferNumberDialog.isFinishing()) {
      mAddTransferNumberDialog = null;
    }
    if (o instanceof AddMemberScreenController && mAddMemberDialog != null
        && mAddMemberDialog.isFinishing()) {
      mAddMemberDialog = null;
    }
  }

  /**
   * dismissDialog.
   * @param o Object
   */
  public void dismissDialog(Object o) {
    if (o instanceof AddTransferNumberScreenController && mAddTransferNumberDialog != null) {
      mAddTransferNumberDialog.finish();
      mAddTransferNumberDialog = null;
    }
    if (o instanceof AddMemberScreenController && mAddMemberDialog != null) {
      mAddMemberDialog.finish();
      mAddMemberDialog = null;
    }
  }

  /**
   * isDialogShown.
   * @param o Object
   * @return if dialog show
   */
  public boolean isDialogShown(Object o) {
      if (o instanceof AddTransferNumberScreenController && mAddTransferNumberDialog != null
          && !mAddTransferNumberDialog.isFinishing()) {
        return true;
      }
      if (o instanceof AddMemberScreenController && mAddMemberDialog != null
          && !mAddMemberDialog.isFinishing()) {
        return true;
      }
    return false;
  }
}
