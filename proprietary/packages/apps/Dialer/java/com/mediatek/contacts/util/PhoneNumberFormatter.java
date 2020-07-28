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

package com.mediatek.contacts.util;

import android.content.Context;
import android.os.AsyncTask;
import android.telephony.PhoneNumberFormattingTextWatcher;
import android.widget.TextView;

import com.android.dialer.location.GeoUtil;

public final class PhoneNumberFormatter {
  private PhoneNumberFormatter() {}

  /**
   * Load {@link TextWatcherLoadAsyncTask} in a worker thread and set it to a {@link TextView}.
   */
  private static class TextWatcherLoadAsyncTask extends
      AsyncTask<Void, Void, PhoneNumberFormattingTextWatcher> {
    private final String mCountryCode;
    private final TextView mTextView;
    private final boolean mFormatAfterWatcherSet;

    public TextWatcherLoadAsyncTask(
        String countryCode, TextView textView, boolean formatAfterWatcherSet) {
      mCountryCode = countryCode;
      mTextView = textView;
      mFormatAfterWatcherSet = formatAfterWatcherSet;
    }

    @Override
    protected PhoneNumberFormattingTextWatcher doInBackground(Void... params) {
      //return PhoneNumberFormattingTextWatcherCompat.newInstance(mCountryCode);
      ///M: @{
      return new PhoneNumberFormattingTextWatcher(mCountryCode);
      ///@}
    }

    @Override
    protected void onPostExecute(PhoneNumberFormattingTextWatcher watcher) {
      if (watcher == null || isCancelled()) {
        return; // May happen if we cancel the task.
      }

      // Setting a text changed listener is safe even after the view is detached.
      mTextView.addTextChangedListener(watcher);

      // Forcing formatting the existing phone number
      if (mFormatAfterWatcherSet && mTextView.getEditableText() != null) {
        watcher.afterTextChanged(mTextView.getEditableText());
      }
    }
  }

  /**
   * Delay-set {@link PhoneNumberFormattingTextWatcher} to a {@link TextView}.
   */
  public static final void setPhoneNumberFormattingTextWatcher(Context context,
      TextView textView) {
    setPhoneNumberFormattingTextWatcher(context, textView,
        /* formatAfterWatcherSet =*/ false);
  }

  /**
   * Delay-sets {@link PhoneNumberFormattingTextWatcher} to a {@link TextView}
   * and formats the number immediately if formatAfterWaterSet is true.
   * In some cases, formatting before user editing might cause unwanted results
   * (e.g. the editor thinks the user changed the content, and would save
   * when closed even when the user didn't make other changes.)
   */
  public static final void setPhoneNumberFormattingTextWatcher(
      Context context, TextView textView, boolean formatAfterWatcherSet) {
    new TextWatcherLoadAsyncTask(GeoUtil.getCurrentCountryIso(context),
        textView, formatAfterWatcherSet)
        .executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, (Void[]) null);
  }
}
