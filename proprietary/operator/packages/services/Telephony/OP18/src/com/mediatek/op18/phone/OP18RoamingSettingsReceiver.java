/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.op18.phone;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;


import java.util.List;




/**
 * Receiver Class to receive intents related to Roaming settings.
 */
public class OP18RoamingSettingsReceiver extends BroadcastReceiver {

    private static final String TAG = "OP18RoamingSettingsReceiver";
    private static final String DATA_ROAMING_INIT = "DATA_ROAMING_INIT";
    private Context mContext;
    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context.getApplicationContext();
        String action = intent.getAction();
        if (action != null &&
                "android.intent.action.SIM_STATE_CHANGED".equals(action)) {
                String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            Log.d(TAG, "ICC key state " + stateExtra);
            if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra)) {
                setDataRoamingEnabled();
            }
        }
    }


    /**
     * Enbale data roaming for first time.
     */
    public void setDataRoamingEnabled() {
        List<SubscriptionInfo> simList = SubscriptionManager.from(mContext).
                getActiveSubscriptionInfoList();
        if (simList == null) {
            return;
        }
        int simCount = simList.size();
        try {
            // Inspite of checking simCount check Sim count from TelephonyManger
            // to know whether the hardware supports single Sim or not.
            if (TelephonyManager.getDefault().getSimCount() == 1) {
                if (android.provider.Settings.Global.getInt(mContext.getContentResolver(),
                            DATA_ROAMING_INIT , 0) == 1) {
                    Log.d(TAG, "simCount 1 and value already 1");
                    return;
                } else {
                    Log.d(TAG, "simCount 1 and update roaming value");
                    android.provider.Settings.Global.putInt(mContext.getContentResolver(),
                        DATA_ROAMING_INIT, 1);
                }
                Phone phone = PhoneFactory.getDefaultPhone();
                Log.d(TAG, "simCount 1 and update roaming value");
                android.provider.Settings.Global.putInt(phone.getContext().getContentResolver(),
                        Settings.Global.DATA_ROAMING, 1);
            } else {
                for (int index = 0; index < simCount; index ++) {
                    int subId = simList.get(index).getSubscriptionId();
                    if (android.provider.Settings.Global.getInt(mContext.getContentResolver(),
                            DATA_ROAMING_INIT + subId, 0) == 1) {
                        Log.d(TAG, "simCount not 1 subId " + subId + "and value already 1");
                        return;
                    } else {
                        Log.d(TAG, "simCount not 1 subId " + subId + "and update roaming value");
                        android.provider.Settings.Global.putInt(mContext.getContentResolver(),
                            DATA_ROAMING_INIT + subId, 1);
                    }
                    Phone phone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(subId));
                    Log.d(TAG, "update for subId " + subId + "and update roaming value");
                    android.provider.Settings.Global.putInt(phone.getContext().getContentResolver(),
                        Settings.Global.DATA_ROAMING + subId, 1);
                }
            }
        } catch (Exception snfe) {
            Log.e(TAG, "getDataOnRoamingEnabled: SettingNofFoundException snfe=" + snfe);
        }
    }

}
