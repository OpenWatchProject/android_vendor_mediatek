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
 */

package com.mediatek.dialer.compat;

import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;

/**
 * Compatibility utility class about TelecomManager.
 */
public class TelecomCompat {
  //For [VoLTE ConfCall] @{
  /**
   * refer to mediatek.telecom.MtkTelecomManager in O
   * Optional extra for {@link android.content.Intent#ACTION_CALL} and
   * {@link android.content.Intent#ACTION_CALL_PRIVILEGED} containing a phone
   * number {@link ArrayList} that used to launch the volte conference call.
   * The phone number in the list may be normal phone number, sip phone
   * address or IMS call phone number. This extra takes effect only when the
   * {@link #EXTRA_START_VOLTE_CONFERENCE} is true.
   * @hide
   */
  public static final String EXTRA_VOLTE_CONFERENCE_NUMBERS_O =
          "mediatek.telecom.extra.VOLTE_CONFERENCE_NUMBERS";

  /**
   * refer to mediatek.telecom.MtkTelecomManager in O
   * Optional extra for {@link android.content.Intent#ACTION_CALL} and
   * {@link android.content.Intent#ACTION_CALL_PRIVILEGED} containing an
   * boolean value that determines if it should launch a volte conference
   * call.
   * @hide
   */
  public static final String EXTRA_START_VOLTE_CONFERENCE_O =
          "mediatek.telecom.extra.EXTRA_START_VOLTE_CONFERENCE";

  /**
   * refer to com.mediatek.telecom.TelecomManagerEx in N
   * Optional extra for {@link android.content.Intent#ACTION_CALL} and
   * {@link android.content.Intent#ACTION_CALL_PRIVILEGED} containing a phone
   * number {@link ArrayList} that used to launch the volte conference call.
   * The phone number in the list may be normal phone number, sip phone
   * address or IMS call phone number. This extra takes effect only when the
   * {@link #EXTRA_VOLTE_CONF_CALL_DIAL} is true.
   * @hide
   */
  public static final String EXTRA_VOLTE_CONF_CALL_NUMBERS_N =
          "com.mediatek.volte.ConfCallNumbers";
  /**
   * refer to com.mediatek.telecom.TelecomManagerEx in N
   * Optional extra for {@link android.content.Intent#ACTION_CALL} and
   * {@link android.content.Intent#ACTION_CALL_PRIVILEGED} containing an
   * boolean value that determines if it should launch a volte conference
   * call.
   * @hide
   */
  public static final String EXTRA_VOLTE_CONF_CALL_DIAL_N = "com.mediatek.volte.ConfCallDial";
  public static final String EXTRA_VOLTE_CONF_CALL_NUMBERS;
  public static final String EXTRA_VOLTE_CONF_CALL_DIAL;
  static {
    if (VERSION.SDK_INT >= VERSION_CODES.O) {
      EXTRA_VOLTE_CONF_CALL_NUMBERS = EXTRA_VOLTE_CONFERENCE_NUMBERS_O;
      EXTRA_VOLTE_CONF_CALL_DIAL = EXTRA_START_VOLTE_CONFERENCE_O;
    } else {
      EXTRA_VOLTE_CONF_CALL_NUMBERS = EXTRA_VOLTE_CONF_CALL_NUMBERS_N;
      EXTRA_VOLTE_CONF_CALL_DIAL = EXTRA_VOLTE_CONF_CALL_DIAL_N;
    }
  }

  public class PhoneAccountCompat {
    private static final int CUSTOM_CAPABILITY_BASE = 0x8000;
    /**
     * TODO:wait PhoneAccount
     * M: refer to android.telecom.PhoneAccount.CAPABILITY_VOLTE_CONFERENCE_ENHANCED in N
     * Flag indicating that this {@code PhoneAccount} is capable of placing a volte conference
     * call at a time. This flag will be set only when the IMS service camped on the IMS
     * server and and the following features are available on the Network:
     * 1. Launch a conference with multiple participants at a time
     * 2. TBD
     * <p>
     * See {@link #getCapabilities()}
     * @hide
     */
    public static final int CAPABILITY_VOLTE_CONFERENCE_ENHANCED = CUSTOM_CAPABILITY_BASE << 2;
  }
  //@}
}
