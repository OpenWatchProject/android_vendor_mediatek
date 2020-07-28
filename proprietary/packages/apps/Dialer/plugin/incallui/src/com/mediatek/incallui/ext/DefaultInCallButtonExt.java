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

package com.mediatek.incallui.ext;

import android.os.Bundle;
import android.util.Log;

/**
 * Default implementation for IInCallButtonExt.
 */
public class DefaultInCallButtonExt implements IInCallButtonExt {
  private static final String TAG = "DefaulInCallButtonExt";

  /**
   * Checks if the device switch feature [Digits] is supported.
   * @param the call object.
   * @return true if device switch [Digits] Feature is supported
   */
  @Override
  public boolean isDeviceSwitchSupported(Object call) {
    Log.d(TAG, "isDeviceSwitchSupported entry" );
    return false;
  }

  /**
   * Handle the call button item click.
   * @param the id of the button clicked
   * @return true if handled by plugin
   */
  @Override
  public boolean onMenuItemClick(int id) {
    Log.d(TAG, "onMenuItemClick entry, id=" + id);
    return false;
  }

  /**
   * Check if one way video call is supported
   * @param call call object
   */
  @Override
  public boolean isOneWayVideoSupportedForCall(Object call) {
    Log.d(TAG, "isOneWayVideoSupportedForCall entry");
    return false;
  }

  @Override
  public boolean isOneWayVideoSupported() {
    Log.d(TAG, "isOneWayVideoSupported false");
    return false;
  }

  /**
   * Check if should show upgrade Button.
   * @param the obj of the DialerCall instance
   * @return true if need to show upgrade button
   */
  @Override
  public boolean shouldShowUpgradeButton(Object obj) {
    Log.d(TAG, "shouldShowUpgradeButton");
    return false;
  }
}
