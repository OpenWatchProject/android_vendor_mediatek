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

import android.content.Context;
import android.util.AttributeSet;

import com.android.mtkex.chips.BaseRecipientAdapter;
import com.android.mtkex.chips.MTKRecipientEditTextView;

/**
 * [VoLTE Conference] AddMemberEditView.
 */
public class AddMemberEditView extends MTKRecipientEditTextView {

  private static final int AUTO_SEARCH_THRESHOLD_LENGTH = 1;

  public AddMemberEditView(Context context, AttributeSet attrs) {
    super(context, attrs);
    super.setValidator(new NumberValidator());
    //set search address threshold length as 1
    setThreshold(AUTO_SEARCH_THRESHOLD_LENGTH);
  }

  /** A noop validator that does not munge invalid texts and claims any number is valid */
  private class NumberValidator implements Validator {
    public CharSequence fixText(CharSequence invalidText) {
      return invalidText;
    }

    public boolean isValid(CharSequence text) {
      return true;
    }
  }

  public static class AddMemberEditViewAdatper extends BaseRecipientAdapter {
    private static final int DEFAULT_PREFERRED_MAX_RESULT_COUNT = 10;

    public AddMemberEditViewAdatper(Context context) {
      // The Chips UI is email-centric by default. By setting QUERY_TYPE_PHONE, the chips UI
      // will operate with phone numbers instead of emails.
      super(context, DEFAULT_PREFERRED_MAX_RESULT_COUNT, QUERY_TYPE_PHONE);
      // setShowDuplicateResults(true);
    }
  }
}
