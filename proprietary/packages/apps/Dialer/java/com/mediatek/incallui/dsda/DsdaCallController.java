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

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.View;
import android.view.View.OnClickListener;

import com.android.dialer.common.Assert;
import com.android.dialer.common.LogUtil;
import com.android.incallui.incall.protocol.InCallScreenDelegate;
import com.android.incallui.incall.protocol.SecondaryInfo;

/** Manages DSDA two incoming banner. */
public class DsdaCallController implements OnClickListener {

  @NonNull private InCallScreenDelegate inCallScreenDelegate;

  @NonNull private View dsdaCallBar;

  private boolean isVisible;

  private boolean isEnabled;

  @Nullable private SecondaryInfo secondaryInfo;

  public DsdaCallController(
      @NonNull View dsdaBanner,
      @NonNull InCallScreenDelegate inCallScreenDelegate) {
    this.dsdaCallBar = Assert.isNotNull(dsdaBanner);
    this.dsdaCallBar.setOnClickListener(this);
    this.inCallScreenDelegate = Assert.isNotNull(inCallScreenDelegate);
  }

  public void setEnabled(boolean isEnabled) {
    this.isEnabled = isEnabled;
    updateButtonState();
  }

  public void setVisible(boolean isVisible) {
    this.isVisible = isVisible;
    updateButtonState();
  }

  public void setOnScreen() {
    isVisible = hasSecondaryInfo();
    updateButtonState();
  }

  public void setSecondaryInfo(@Nullable SecondaryInfo secondaryInfo) {
    this.secondaryInfo = secondaryInfo;
    isVisible = hasSecondaryInfo();
  }

  private boolean hasSecondaryInfo() {
    return secondaryInfo != null && secondaryInfo.shouldShow();
  }

  public void updateButtonState() {
    dsdaCallBar.setEnabled(isEnabled);
    dsdaCallBar.setVisibility(isVisible ? View.VISIBLE : View.INVISIBLE);
  }

  @Override
  public void onClick(View view) {
    LogUtil.d("AnswerFragment.onClick", "DsdaCallController");
    inCallScreenDelegate.onSecondaryInfoClicked();
  }
}
