/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
 */

package com.mediatek.incallui.dsda;


import android.content.res.Resources;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.telephony.PhoneNumberUtils;
import android.text.BidiFormatter;
import android.text.TextDirectionHeuristics;
import android.text.TextUtils;
import android.transition.TransitionManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnAttachStateChangeListener;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.dialer.common.Assert;
import com.android.dialer.common.LogUtil;
import com.android.incallui.R;
import com.android.incallui.incall.protocol.SecondaryInfo;

/** Shows banner UI for another incoming call */
public class DsdaFragment extends Fragment{

  private static final String ARG_INFO = "info";
  private boolean padTopInset = true;
  private int topInset;

  public static DsdaFragment newInstance(@NonNull SecondaryInfo info) {
    DsdaFragment fragment = new DsdaFragment();
    Bundle args = new Bundle();
    args.putParcelable(ARG_INFO, info);
    fragment.setArguments(args);
    return fragment;
  }

  @Nullable
  @Override
  public View onCreateView(
      LayoutInflater layoutInflater, @Nullable ViewGroup viewGroup, @Nullable Bundle bundle) {
    final View view = layoutInflater.inflate(R.layout.incall_dsda_banner, viewGroup, false);

    SecondaryInfo secondaryInfo = getArguments().getParcelable(ARG_INFO);
    secondaryInfo = Assert.isNotNull(secondaryInfo);

    ((TextView) view.findViewById(R.id.hold_contact_name))
        .setText(
            secondaryInfo.nameIsNumber()
                ? PhoneNumberUtils.createTtsSpannable(
                    BidiFormatter.getInstance()
                        .unicodeWrap(secondaryInfo.name(), TextDirectionHeuristics.LTR))
                : secondaryInfo.name());
    ((ImageView) view.findViewById(R.id.hold_phone_icon))
        .setImageResource(
            secondaryInfo.isVideoCall()
                ? R.drawable.quantum_ic_videocam_white_18
                : R.drawable.quantum_ic_call_white_18);

    TextView providerName = (TextView) view.findViewById(R.id.call_provider_name);
    if (!TextUtils.isEmpty(secondaryInfo.providerLabel())) {
      providerName.setText(
              PhoneNumberUtils.createTtsSpannable(
                BidiFormatter.getInstance()
                    .unicodeWrap(secondaryInfo.providerLabel(), TextDirectionHeuristics.LTR)));
      providerName.setTextColor(secondaryInfo.color());
    } else {
      providerName.setVisibility(View.GONE);
    }

    view.addOnAttachStateChangeListener(
        new OnAttachStateChangeListener() {
          @Override
          public void onViewAttachedToWindow(View v) {
            topInset = v.getRootWindowInsets().getSystemWindowInsetTop();
            applyInset();
          }

          @Override
          public void onViewDetachedFromWindow(View v) {}
        });
    return view;
  }

  public void setPadTopInset(boolean padTopInset) {
    this.padTopInset = padTopInset;
    applyInset();
  }

  private void applyInset() {
    if (getView() == null) {
      return;
    }

    int newPadding = padTopInset ? topInset : 0;
    if (newPadding != getView().getPaddingTop()) {
      TransitionManager.beginDelayedTransition(((ViewGroup) getView().getParent()));
      getView().setPadding(0, newPadding, 0, 0);
    }
  }
}
