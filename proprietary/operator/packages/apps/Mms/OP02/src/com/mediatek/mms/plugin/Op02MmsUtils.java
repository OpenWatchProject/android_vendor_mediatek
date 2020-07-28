/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.mms.plugin;

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.provider.Telephony;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.ISms;

import java.util.List;
import com.mediatek.internal.telephony.IMtkSms;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * Utils Class.
 */
public class Op02MmsUtils {
    private static final String TAG = "Op02MmsUtils";
    private static final String MMS_APP_PACKAGE = "com.android.mms";

    /**
     * Check System default Mms enable status.
     */
    public static boolean isSmsEnabled(Context context) {
        String defaultSmsApplication = Telephony.Sms.getDefaultSmsPackage(context);
        if (defaultSmsApplication != null && defaultSmsApplication.equals(MMS_APP_PACKAGE)) {
            return true;
        }
        return false;
    }

    /**
     * Get subscription id count.
     * @param context the context
     * @return the id count.
     */
    public static int getSimCount(Context context) {
        int count = getEnabledSubCount(context);
        return count;
    }

    /**
     * Get first active subscription id.
     * @param context the context
     * @return the first id count or 0.
     */
    public static int getActiveSubId(Context context) {
        List<SubscriptionInfo> list =
                SubscriptionManager.from(context).getActiveSubscriptionInfoList();
        if (list != null) {
            int subId = list.get(0).getSubscriptionId();
            Log.d(TAG, "getActiveSubId subid:" + subId);
            return subId;
        } else {
            Log.d(TAG, "getActiveSubId return 0");
            return 0;
        }
    }

    /**
     * Check sim card eixstence.
     * @param context the context
     */
    public static boolean isSimInserted(Context context) {
        int count = getSimCount(context);
        if (count > 0) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Check airplane status.
     */
    public static boolean isAirplaneOn(Context context) {
        boolean airplaneOn = Settings.System.getInt(context.getContentResolver(),
                                        Settings.System.AIRPLANE_MODE_ON, 0) == 1;
        if (airplaneOn) {
            Log.d(TAG, "airplane is On");
            return true;
        }
        return false;
    }

    /**
     * @param context The current context.
     * @return Whether the sms is ready.
     */
    public static boolean isSmsReady(Context context) {
        Log.d(TAG, "isSmsReady");
        IMtkSms smsManager = IMtkSms.Stub.asInterface(ServiceManager.getService("imtksms"));
        if (smsManager == null) {
            Log.d(TAG, "smsManager is null");
            return false;
        }

        boolean smsReady = false;
        List<SubscriptionInfo> subInfoList =
                SubscriptionManager.from(context).getActiveSubscriptionInfoList();

        if (subInfoList == null || subInfoList.size() == 0) {
            return smsReady;
        }

        for (SubscriptionInfo subInfoRecord : subInfoList) {
            try {
                smsReady = smsManager.isSmsReadyForSubscriber(subInfoRecord.getSubscriptionId());
                if (smsReady) {
                    break;
                }
            } catch (RemoteException e) {
                Log.d(TAG, "isSmsReady failed to get sms state for sub "
                        + subInfoRecord.getSubscriptionId());
                smsReady = false;
            }
        }

        Log.d(TAG, "isSmsReady" + smsReady);
        return smsReady;
    }

    private static int getEnabledSubCount(Context context) {
        Log.d(TAG, "getEnabledSubCount");
        List<SubscriptionInfo> subInfoList = SubscriptionManager.from(context).getActiveSubscriptionInfoList();
        int subCount = (subInfoList != null && !subInfoList.isEmpty()) ? subInfoList.size() : 0;
        boolean simOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        Log.d(TAG, "getEnabledSubCount->simOnOffEnabled = " + simOnOffEnabled);

        if (simOnOffEnabled == false) {
            return subCount;
        }

        Log.d(TAG, "getEnabledSubCount->subInfoList = " + subInfoList);
        Log.d(TAG, "getEnabledSubCount->subCount = " + subCount);
        if (subInfoList == null || subCount == 0) {
            return subCount;
        }

        int enabledSubCount = 0;
        for (int i = 0; i < subCount; i++) {
            SubscriptionInfo subInfo = subInfoList.get(i);
            if (subInfo != null) {
                int slotId = subInfo.getSimSlotIndex();
                Log.d(TAG, "getEnabledSubCount->slotId = " + slotId);
                int slotState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
                Log.d(TAG, "getEnabledSubCount->slotState = " + slotState);
                if (slotState == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                    enabledSubCount++;
                }
            }
        }
        Log.d(TAG, "getEnabledSubCount->enabledSubCount = " + enabledSubCount);
        return enabledSubCount;
    }
}
