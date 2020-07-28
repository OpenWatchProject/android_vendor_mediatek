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
package com.mediatek.incallui.video;

import android.os.SystemProperties;
import android.view.Gravity;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.android.incallui.Log;
import com.android.incallui.video.impl.VideoCallFragment;

/**
 * M: add a TextView to the video call screen to display some debug message.
 * Enable it by turning on related option in EngineerMode.
 */
public class VideoDebugInfo {
  private static final String ENGINEER_MODE_OPTION = "persist.vendor.vt.RTPInfo";
  private VideoCallFragment mFragment;
  private RelativeLayout mParentLayout;
  private TextView mDebugInfoView;
  private long lossRate ;

  public void setFragment(VideoCallFragment fragmentShowInfo) {
    mFragment = fragmentShowInfo;
  }

  public void setParent(RelativeLayout parent) {
    mParentLayout = parent;
  }
  /**
   * M: Checking the feature option and the parameters to determine whether or not to
   * display the debug message in the video call screen.
   *
  */
  private void checkDisplay() {
    if (!isFeatureOn()) {
        return;
    }
    if(mFragment == null || mParentLayout == null){
        return;
    }
    addToScreen();
    periodicUpdate();
  }

  /**
     * @return The system property set by EngineerMode.
     */
  public static boolean isFeatureOn() {
    return "1".equals(SystemProperties.get(ENGINEER_MODE_OPTION, "-1"));
  }

  private void addToScreen() {
    if (mDebugInfoView == null) {
      mDebugInfoView = new TextView(mFragment.getContext());

      RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(
        RelativeLayout.LayoutParams.WRAP_CONTENT,
        RelativeLayout.LayoutParams.WRAP_CONTENT);
      lp.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
      lp.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
      lp.setMargins(40, 30, 40, 30);

      mDebugInfoView.setLayoutParams(lp);
      mDebugInfoView.setBackgroundColor(android.graphics.Color.RED);
      mDebugInfoView.setTextSize(15);
      logd("[addToScreen]add Text to screen");
      mParentLayout.addView(mDebugInfoView);
    }
  }

  private void periodicUpdate() {
    if (mFragment.isDetached()) {
      logd("[periodicUpdate]end updating");
      return;
    }
    if (mDebugInfoView == null) {
      loge("[periodicUpdate]mDebugInfoView == null");
      return;
    }
    if (!isFeatureOn()) {
      logd("[periodicUpdate]feature option turned off");
      return;
    }
    logd("periodicUpdate loss package is -->"+ Long.toString(lossRate));
    mDebugInfoView.setText(Long.toString(lossRate));
  }

  private static void loge(String msg) {
    Log.e(VideoDebugInfo.class.getSimpleName(), msg);
  }

  private static void logd(String msg) {
    Log.d(VideoDebugInfo.class.getSimpleName(), msg);
  }

  public void onCallDataUsageChange(long dataUsage) {
    if (dataUsage <= 0) {
      logd("onCallDataUsageChange the loss package is -->"+ Long.toString(dataUsage));
      lossRate = -1 * dataUsage;
      checkDisplay();
    }
  }
}