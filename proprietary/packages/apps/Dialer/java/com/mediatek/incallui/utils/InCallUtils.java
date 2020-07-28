/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
package com.mediatek.incallui.utils;

import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.os.Build;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.widget.Toast;

import com.android.dialer.common.LogUtil;
import com.android.dialer.util.MoreStrings;
import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
import com.android.incallui.call.state.DialerCallState;
import com.mediatek.incallui.compat.InCallUiCompat;

/**
 * Utils class.
 */
import mediatek.telecom.MtkCall.MtkDetails;

public class InCallUtils {
  private static final String TAG = InCallUtils.class.getSimpleName();
  private static final String TELECOM_PACKAGE_NAME = "com.android.server.telecom";
  private static final String OUTGOING_FAILED_MSG_RES_ID = "outgoing_call_failed";
  public static final String ARG_CALL_ID = "call_id";
  public static final boolean MTK_IMS_SUPPORT = SystemProperties.get(
      "persist.vendor.ims_support").equals("1");
  public static final boolean MTK_VOLTE_SUPPORT = SystemProperties.get(
      "persist.vendor.volte_support").equals("1");
  /**
   * M: judge whether in CMCC/CT lab test or not
   */
  public static final boolean MTK_CMCC_VILTE_LAB_TEST = SystemProperties.get(
      "persist.vendor.vt.lab_op_code").equals("1");
  public static final boolean MTK_CT_VILTE_LAB_TEST = SystemProperties.get(
      "persist.vendor.vt.lab_op_code").equals("9");

  /// M: [ALPS03966547] Set fake camera id to test video call. @{
  public static final boolean MTK_NO_CAMERA_MODE = SystemProperties.get(
      "persist.vendor.vt.no_camera_mode").equals("1");
  /// @}

  /// M: format sensitive log. @{
  private static final String sLogNumPattern = "(?<=[\\s\\S]{2})(\\S)(?=[\\s\\S]{3})";
  private static final String sLogReplaceValue = "x";
  /// @}

  /**
   * M: show the same error message as Telecom when can't MO.
   * typically, when one call is in upgrading to video progress, someone
   * is responsible to prevent new outgoing call. Currently, we have nowhere
   * to do this except InCallUI itself.
   * TODO: the Telecom or Lower layer should be responsible to stop new outgoing call while
   * upgrading instead of InCallUI.
   *
   * @param context the ApplicationContext
   * @param call
   */
  public static void showOutgoingFailMsg(Context context, android.telecom.Call call) {
    if (context == null || call == null ||
        android.telecom.Call.STATE_RINGING == call.getState()) {
      return;
    }

    final PackageManager pm = context.getPackageManager();
    Resources telecomResources = null;
    try {
      telecomResources = pm.getResourcesForApplication(TELECOM_PACKAGE_NAME);
    } catch (PackageManager.NameNotFoundException e) {
      LogUtil.e("InCallUtils.showOutgoingFailMsg", "telecomResources not found");
    }

    if (telecomResources != null) {
      int resId = telecomResources.getIdentifier(
          OUTGOING_FAILED_MSG_RES_ID, "string", TELECOM_PACKAGE_NAME);
      String msg = telecomResources.getString(resId);
      LogUtil.e("InCallUtils.showOutgoingFailMsg", "showOutgoingFailMsg msg-->" + msg);

      if (!TextUtils.isEmpty(msg)) {
        Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
      }
    }
  }

  /**
   * [Hang Up] Hang up all/hold calls.
   * When there have more than one active call or background call and has no
   * incoming, it will be true, otherwise false.
   * @return true if can hang up all calls
   */
  public static boolean canHangupAllCalls() {
    CallList callList = CallList.getInstance();
    DialerCall call = callList.getFirstCall();
    if (call != null && !DialerCallState.isIncomingOrWaiting(call.getState())
        && callList.getActiveAndHoldCallsCount() > 1
        && InCallUiCompat.isMtkTelecomCompat()) {
      return true;
    }
    return false;
  }

  /**
   * [Hang Up] Hang up all/hold calls.
   * When there have more than one active call or background call and has no
   * incoming, it will be true, otherwise false.
    * @return true if can hang up all hold calls
  */
  public static boolean canHangupAllHoldCalls() {
    CallList callList = CallList.getInstance();
    DialerCall call = callList.getFirstCall();
    if (call != null && !DialerCallState.isIncomingOrWaiting(call.getState())
        && callList.getActiveAndHoldCallsCount() > 1
        && InCallUiCompat.isMtkTelecomCompat()) {
      return true;
    }
    return false;
  }

  /**
   * [Hang Up] Hang up all/hold calls.
   * When there has one active call and a incoming call which can be answered,
   * it will be true, otherwise false.
   * @return true if can hang up active and answer waiting call
   */
  public static boolean canHangupActiveAndAnswerWaiting() {
    CallList callList = CallList.getInstance();
    DialerCall call = callList.getFirstCall();
    if (call != null && DialerCallState.isIncomingOrWaiting(call.getState())
        && callList.getActiveCall() != null
        && !isCdmaCall(call)
        && InCallUiCompat.isMtkTelecomCompat()) {
      return true;
    }
    return false;
  }

  /**
   * [Hang Up] Hang up all/hold calls.
   * Check if the call's account has CAPABILITY_CDMA_CALL_PROVIDER.
   * @param call the dialer call
   * @return true if cdma call
   */
  public static boolean isCdmaCall(DialerCall call) {
    if (null == call) {
      return false;
    }
    return call.hasProperty(MtkDetails.MTK_PROPERTY_CDMA);
  }

  /**
   * when hold call have the ECT capable call,it will be true,otherwise false.
   */
  public static boolean canEct() {
    final DialerCall call = CallList.getInstance().getBackgroundCall();
    if (call != null && call.can(
        mediatek.telecom.MtkCall.MtkDetails.MTK_CAPABILITY_CONSULTATIVE_ECT)) {
      return true;
    }
    return false;
  }

  public static boolean canBlindEct(DialerCall call) {
    if (call != null) {
      return call.can(
          mediatek.telecom.MtkCall.MtkDetails.MTK_CAPABILITY_BLIND_OR_ASSURED_ECT);
    }
    return false;
  }

  /**
   * M: [1A1H2W]indicate is under two incoming call state or not.
   */
  public static boolean isTwoIncomingCalls() {
    return CallList.getInstance().getIncomingCall() != null
        && CallList.getInstance().getSecondaryIncomingCall() != null;
  }

  /**
   * isInTestSim.
   * @return if is in test sim mode
   */
  public static boolean isInTestSim() {
    boolean isInTestSim = false;
    isInTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1")
        || SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1")
        || SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1")
        || SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1");
    return isInTestSim;
  }

  /**
   * Format number with 'x' instead.
   */
  public static String formatSensitiveValue(String value) {
    if (value == null) {
      return null;
    }

    if (!("user".equals(Build.TYPE))) {
      return value.replaceAll(sLogNumPattern, sLogReplaceValue);
    }

    return MoreStrings.toSafeString(value);
  }
}
