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

import android.os.Bundle;
import android.text.TextUtils;

import com.android.incallui.Log;
import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
import com.android.incallui.call.state.DialerCallState;
import com.mediatek.incallui.utils.InCallUtils;

import mediatek.telecom.MtkTelecomManager;

/**
 * [VoLTE Conference] InCallUIVolteUtils.
 */
public class InCallUIVolteUtils {

  private static final String LOG_TAG = "InCallUIVolteUtils";
  private static final int INVALID_RES_ID = -1;

  public static boolean isVolteSupport() {
    return InCallUtils.MTK_IMS_SUPPORT && InCallUtils.MTK_VOLTE_SUPPORT;
  }

  //-------------For VoLTE normal call switch to ECC------------------
  public static boolean isVolteMarkedEcc(final android.telecom.Call.Details details) {
    boolean result = false;
    if (isVolteSupport() && details != null) {
      Bundle bundle = details.getExtras();
      if (bundle != null
          && bundle.containsKey(MtkTelecomManager.EXTRA_VOLTE_MARKED_AS_EMERGENCY)) {
        Object value = bundle.get(MtkTelecomManager.EXTRA_VOLTE_MARKED_AS_EMERGENCY);
        if (value instanceof Boolean) {
          result = (Boolean) value;
        }
      }
    }
    return result;
  }

  public static boolean isVolteMarkedEccChanged(final android.telecom.Call.Details oldDetails,
      final android.telecom.Call.Details newDetails) {
    boolean result = false;
    boolean isVolteMarkedEccOld = isVolteMarkedEcc(oldDetails);
    boolean isVolteMarkedEccNew = isVolteMarkedEcc(newDetails);
    result = !isVolteMarkedEccOld && isVolteMarkedEccNew;
    return result;
  }

  //-------------For VoLTE PAU field------------------
  public static String getVoltePauField(final android.telecom.Call.Details details) {
    String result = "";
    if (isVolteSupport() && details != null) {
      Bundle bundle = details.getExtras();
      if (bundle != null) {
        result = bundle.getString(MtkTelecomManager.EXTRA_VOLTE_PAU, "");
      }
    }
    return result;
  }

  public static String getPhoneNumber(final android.telecom.Call.Details details) {
    String result = "";
    if (details != null) {
      if (details.getGatewayInfo() != null) {
        result = details.getGatewayInfo().getOriginalAddress().getSchemeSpecificPart();
      } else {
        result = details.getHandle() == null ? null : details.getHandle().getSchemeSpecificPart();
      }
    }
    if (result == null) {
      result = "";
    }
    return result;
  }

  public static boolean isPhoneNumberChanged(final android.telecom.Call.Details oldDetails,
      final android.telecom.Call.Details newDetails) {
    boolean result = false;
    String numberOld = getPhoneNumber(oldDetails);
    String numberNew = getPhoneNumber(newDetails);
    result = !TextUtils.equals(numberOld, numberNew);
    if (result) {
      log("number changed from " + InCallUtils.formatSensitiveValue(numberOld) + " to " +
          InCallUtils.formatSensitiveValue(numberNew));
    }
    return result;
  }

  private static void log(String msg) {
    Log.d(LOG_TAG, msg);
  }

  /**
   * Check if a call is an incoming VoLTE conference call.
   * @param call the call to be checked.
   * @return true if yes.
   */
  public static boolean isIncomingVolteConferenceCall(DialerCall call) {
    return call != null
        && DialerCallState.isIncomingOrWaiting(call.getState())
        && call.isConferenceCall()
        && call.hasProperty(mediatek.telecom.MtkCall.MtkDetails.MTK_PROPERTY_VOLTE);
  }

  /**
   * M: check incoming call conference call or not.
   * @return if it is incoming VoLTE conference call
   */
  public static boolean isIncomingVolteConferenceCall() {
    DialerCall call = CallList.getInstance().getIncomingCall();
    return InCallUIVolteUtils.isIncomingVolteConferenceCall(call);
  }
}
