/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.internal.telephony;

import android.content.Context;
import android.os.SystemProperties;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.provider.MtkSettingsExt;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;
import java.util.List;

public class MtkDefaultSmsSimSettings {
    private static final String TAG = "MTKDefaultSmsSimSettings";
    public static final int ASK_USER_SUB_ID = (int)MtkSettingsExt.System.DEFAULT_SIM_SETTING_ALWAYS_ASK; //-2;

    public static void setSmsTalkDefaultSim(List<SubscriptionInfo> subInfos, Context context) {
        int oldSmsDefaultSIM = SubscriptionManager.getDefaultSmsSubscriptionId();
        Log.i(TAG, "oldSmsDefaultSIM" + oldSmsDefaultSIM);

        if (subInfos == null) {
            Log.i(TAG, "subInfos == null, return");
            //SubscriptionManager.from(context).setDefaultSmsSubId(SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        } else {
            Log.i(TAG, "subInfos size = " + subInfos.size());
            if (subInfos.size() > 1) {
               if (isoldDefaultSMSSubIdActive(subInfos)) {
                  Log.i(TAG, "subInfos size > 1 & old available, set to :"
                          + oldSmsDefaultSIM);
                  //SubscriptionManager.from(context).setDefaultSmsSubId(oldSmsDefaultSIM);
              } else {
                  String optr = SystemProperties.get("persist.vendor.operator.optr");
                  Log.d(TAG, "optr = " + optr);
                  /* P0 migration*/
                  if ("OP01".equals(optr)) {
                      /*Log.i(TAG, "subInfos size > 1, set to : AUTO");
                      SubscriptionManager.from(context)
                          .setDefaultSmsSubId((int) MtkSettingsExt.System.SMS_SIM_SETTING_AUTO);*/
                      Log.i(TAG, "subInfos size > 1, set to : ASK_USER_SUB_ID");
                      SubscriptionManager.from(context).setDefaultSmsSubId(ASK_USER_SUB_ID);
                  } else if ("OP09".equals(optr)
                          && "SEGDEFAULT".equals(SystemProperties
                                  .get("persist.vendor.operator.seg"))) {
                      //only op09-A use this setting, Op09-C use common setting
                      SubscriptionInfo subInfo = SubscriptionManager.from(context)
                             .getActiveSubscriptionInfoForSimSlotIndex(0);
                      int firstSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
                      if (subInfo != null) {
                          firstSubId = subInfo.getSubscriptionId();
                      }
                      SubscriptionManager.from(context).setDefaultSmsSubId(firstSubId);
                      Log.i(TAG, "subInfos size > 1, set to " + firstSubId);
                  }  else if ("OP07".equals(optr)) {
                      int mainPhoneId = getMainCapabilityPhoneId();
                      Log.d(TAG, "Main slot = " + mainPhoneId);

                      SubscriptionInfo subInfo = SubscriptionManager.from(context)
                             .getActiveSubscriptionInfoForSimSlotIndex(mainPhoneId);
                      int mainSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

                      if (subInfo != null) {
                          mainSubId = subInfo.getSubscriptionId();
                          Log.d(TAG, "Get subId from subInfo = " + mainSubId);
                      }

                      Log.d(TAG, "subInfos size > 1, set to " + mainSubId);
                      SubscriptionManager.from(context).setDefaultSmsSubId(mainSubId);
                  } else {
                      Log.i(TAG, "subInfos size > 1, set to : ASK_USER_SUB_ID");
                      //SubscriptionManager.from(context).setDefaultSmsSubId(ASK_USER_SUB_ID);
                  }
              }
            } else if (subInfos.size() == 1) {
              Log.i(TAG, "sub size = 1,segment = " + SystemProperties
                      .get("persist.vendor.operator.seg"));
              if ("OP09".equals(SystemProperties.get("persist.vendor.operator.optr"))
                      && "SEGDEFAULT".equals(SystemProperties.get("persist.vendor.operator.seg"))) {
                  //only op09-A use this setting, Op09-C use common setting
                  int defaultSubId = subInfos.get(0).getSubscriptionId();
                  SubscriptionManager.from(context).setDefaultSmsSubId(defaultSubId);
                  Log.i(TAG, "subInfos size = 1, set to " + defaultSubId);
              } else if ("OP01".equals(SystemProperties.get("persist.vendor.operator.optr"))) {
                  int defaultSubId = subInfos.get(0).getSubscriptionId();
                  SubscriptionManager.from(context).setDefaultSmsSubId(defaultSubId);
                  Log.i(TAG, "subInfos size = 1, set to " + defaultSubId);
              }
              //SubscriptionManager.from(context).setDefaultSmsSubId(subInfos.get(0).getSubscriptionId());
           } else {
              Log.i(TAG, "setSmsTalkDefaultSim SIM not insert");
              //SubscriptionManager.from(context).setDefaultSmsSubId(SubscriptionManager.INVALID_SUBSCRIPTION_ID);
           }
       }
    }

    private static boolean isoldDefaultSMSSubIdActive(List<SubscriptionInfo> subInfos) {
       int oldSmsDefaultSIM = SubscriptionManager.getDefaultSmsSubscriptionId();

        for (SubscriptionInfo subInfo : subInfos) {
            if (subInfo.getSubscriptionId() == oldSmsDefaultSIM) {
                return true;
            }
        }
        if ("OP01".equals(SystemProperties.get("persist.vendor.operator.optr"))) {
            if (oldSmsDefaultSIM == ASK_USER_SUB_ID/* ||
                       oldSmsDefaultSIM == (int) MtkSettingsExt.System.SMS_SIM_SETTING_AUTO*/) {
                return true;
            }
        }
       return false;
    }

    private static int getMainCapabilityPhoneId() {
        int phoneId = 0;
        phoneId = SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        Log.d(TAG, "getMainCapabilityPhoneId " + phoneId);
        return phoneId;
    }
}
