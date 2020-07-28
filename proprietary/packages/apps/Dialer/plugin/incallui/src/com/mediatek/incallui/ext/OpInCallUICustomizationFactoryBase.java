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

import android.content.Context;

import android.util.Log;

import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;
import com.mediatek.incallui.ext.DefaultAnswerExt;
import com.mediatek.incallui.ext.DefaultCallCardExt;
import com.mediatek.incallui.ext.DefaultInCallButtonExt;
import com.mediatek.incallui.ext.DefaultInCallExt;
import com.mediatek.incallui.ext.DefaultRCSeCallButtonExt;
import com.mediatek.incallui.ext.DefaultRCSeCallCardExt;
import com.mediatek.incallui.ext.DefaultRCSeInCallExt;
import com.mediatek.incallui.ext.DefaultStatusBarExt;
import com.mediatek.incallui.ext.DefaultVideoCallExt;
import com.mediatek.incallui.ext.DefaultVilteAutoTestHelperExt;
import com.mediatek.incallui.ext.IAnswerExt;
import com.mediatek.incallui.ext.ICallCardExt;
import com.mediatek.incallui.ext.IInCallButtonExt;
import com.mediatek.incallui.ext.IInCallExt;
import com.mediatek.incallui.ext.IRCSeCallButtonExt;
import com.mediatek.incallui.ext.IRCSeCallCardExt;
import com.mediatek.incallui.ext.IRCSeInCallExt;
import com.mediatek.incallui.ext.IStatusBarExt;
import com.mediatek.incallui.ext.IVideoCallExt;
import com.mediatek.incallui.ext.IVilteAutoTestHelperExt;

import java.util.ArrayList;

public class OpInCallUICustomizationFactoryBase {

  private static final String TAG = "OpInCallUICustomizationFactoryBase";
  private static final ArrayList<OperatorFactoryInfo> sOpFactoryInfos =
      new ArrayList<OperatorFactoryInfo>();
  private static final ArrayList<OperatorFactoryInfo> sOpRcsFactoryInfos =
      new ArrayList<OperatorFactoryInfo>();

  /// Operator owner register operator info @{
  static {
    if (isOpFactoryLoaderAvailable()) {
      sOpFactoryInfos.add(
          new OperatorFactoryInfo("OP01Dialer.apk",
              "com.mediatek.op01.incallui.Op01InCallUICustomizationFactory",
              "com.mediatek.op01.dialer",
              "OP01"));
      sOpFactoryInfos.add(
          new OperatorFactoryInfo("OP09ClibDialer.apk",
              "com.mediatek.op09clib.incallui.Op09ClibInCallUICustomizationFactory",
              "com.mediatek.op09clib.dialer",
              "OP09",
              "SEGC"));
      sOpFactoryInfos.add(
          new OperatorFactoryInfo("OP02Dialer.apk",
              "com.mediatek.op02.incallui.Op02InCallUICustomizationFactory",
              "com.mediatek.op02.dialer",
              "OP02"));
      sOpFactoryInfos.add(
          new OperatorFactoryInfo("OP07Dialer.apk",
              "com.mediatek.op07.incallui.OP07InCallUICustomizationFactory",
              "com.mediatek.op07.dialer",
              "OP07"));
      sOpFactoryInfos.add(
          new OperatorFactoryInfo("OP08Dialer.apk",
              "com.mediatek.op08.incallui.OP08InCallUICustomizationFactory",
              "com.mediatek.op08.dialer",
              "OP08"));
      sOpFactoryInfos.add(
          new OperatorFactoryInfo("OP12Dialer.apk",
              "com.mediatek.op12.incallui.OP12InCallUICustomizationFactory",
              "com.mediatek.op12.dialer",
              "OP12"));
      sOpFactoryInfos.add(
          new OperatorFactoryInfo("OP18Dialer.apk",
              "com.mediatek.op18.incallui.OP18InCallUICustomizationFactory",
              "com.mediatek.op18.dialer",
              "OP18"));
      sOpFactoryInfos.add(
          new OperatorFactoryInfo("OP112Dialer.apk",
              "com.mediatek.op112.incallui.OP112InCallUICustomizationFactory",
              "com.mediatek.op112.dialer",
              "OP112"));
    }
  }
  /// @}

  static {
    if (isOpFactoryLoaderAvailable()) {
      sOpRcsFactoryInfos.add(
          new OperatorFactoryInfo("RCSPhone.apk",
              "com.mediatek.rcs.incallui.ext.Op01InCallUICustomizationFactory",
              "com.mediatek.rcs.incallui.ext",
              "OP01"));
    }
  }

  static OpInCallUICustomizationFactoryBase sFactory = null;
  public static synchronized OpInCallUICustomizationFactoryBase getOpFactory(Context context) {
    if (sFactory == null && isOpFactoryLoaderAvailable()) {
      sFactory = (OpInCallUICustomizationFactoryBase)
          OperatorCustomizationFactoryLoader.loadFactory(context, sOpFactoryInfos);
    }
    if (sFactory == null) {
      sFactory = new OpInCallUICustomizationFactoryBase();
    }
    return sFactory;
  }

  static OpInCallUICustomizationFactoryBase sRcsFactory = null;
  public static synchronized OpInCallUICustomizationFactoryBase getRcsFactory(Context context) {
    if (sRcsFactory == null && isOpFactoryLoaderAvailable()) {
      sRcsFactory = (OpInCallUICustomizationFactoryBase)
          OperatorCustomizationFactoryLoader.loadFactory(context, sOpRcsFactoryInfos);
    }
    if (sRcsFactory == null) {
      sRcsFactory = new OpInCallUICustomizationFactoryBase();
    }
    return sFactory;
  }

  /**
   * For portable.
   * @return
   */
  private static boolean isOpFactoryLoaderAvailable() {
    try {
      Class.forName("com.mediatek.common.util.OperatorCustomizationFactoryLoader");
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
      return false;
    }
    return true;
  }

  /// Operator owner should override these methods @{
  public IRCSeCallButtonExt getRCSeCallButtonExt(Context context) {
    Log.d(TAG, "return DefaultRCSeCallButtonExt");
    return new DefaultRCSeCallButtonExt();
  }

  public IRCSeCallCardExt getRCSeCallCardExt(Context context) {
    Log.d(TAG, "return DefaultRCSeCallCardExt");
    return new DefaultRCSeCallCardExt();
  }

  public IRCSeInCallExt getRCSeInCallExt(Context context) {
    Log.d(TAG, "return DefaultRCSeInCallExt");
    return new DefaultRCSeInCallExt();
  }

  public IInCallExt getInCallExt() {
    Log.d(TAG, "return DefaultInCallExt");
    return new DefaultInCallExt();
  }

  public IAnswerExt getAnswerExt() {
    Log.d(TAG, "return DefaultAnswerExt");
    return new DefaultAnswerExt();
  }

  public ICallCardExt getCallCardExt() {
    Log.d(TAG, "return DefaultCallCardExt");
    return new DefaultCallCardExt();
  }

  public IVideoCallExt getVideoCallExt() {
    Log.d(TAG, "return DefaultVideoCallExt");
    return new DefaultVideoCallExt();
  }

  public IInCallButtonExt getInCallButtonExt() {
    Log.d(TAG, "return DefaultInCallButtonExt");
    return new DefaultInCallButtonExt();
  }

  public IStatusBarExt getStatusBarExt() {
    Log.d(TAG, "return DefaultStatusBarExt");
    return new DefaultStatusBarExt();
  }

  public IVilteAutoTestHelperExt getVilteAutoTestHelperExt() {
    Log.d(TAG, "return DefaultVilteAutoTestHelperExt");
    return new DefaultVilteAutoTestHelperExt();
  }
  /// @}
}
