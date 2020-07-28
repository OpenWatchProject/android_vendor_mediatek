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

package com.mediatek.simprocessor;

import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.simprocessor.SimProcessorService;
import com.mediatek.simprocessor.SimServiceUtils;
import com.mediatek.simprocessor.Log;

public class BootCmpReceiver extends BroadcastReceiver {
    private static final String TAG = "BootCmpReceiver";
    private static final int ERROR_SUB_ID = -1000;

    public void onReceive(Context context, Intent intent) {
        final String action = intent.getAction();

        // add for multi-user ALPS01964765, whether the current user is running.
        // if not , will do nothing.
        UserManager userManager = (UserManager) context.getSystemService(Context.USER_SERVICE);
        boolean isRunning = userManager.isUserRunning(new UserHandle(UserHandle.myUserId()));
        Log.d(TAG, "[onReceive] action=" + action + " isRunning: " + isRunning
                + "isUserUnlocked() = " + userManager.isUserUnlocked());
        if (!isRunning) {
            return;
        }

        /// M: Not support SIM Contacts in guest mode.
        if (UserHandle.myUserId() != UserHandle.USER_OWNER) {
            Log.i(TAG, "[onReceive], The current user isn't owner !");
            return;
        }

        if (TelephonyIntents.ACTION_PHB_STATE_CHANGED.equals(action)) {
            processPhoneBookChanged(context, intent);
        } else if (Intent.ACTION_LOCKED_BOOT_COMPLETED.equals(action)) {
            if ("1".equals(SystemProperties.get("sys.boot_completed"))) {
                processBootComplete(context);
            }
        } else if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            if (!isPhbReady()) {
                processBootComplete(context);
            } else {
                processDupSimContacts(context);
            }
        }
    }

    private boolean isPhbReady() {
        final int simCount = TelephonyManager.getDefault().getSimCount();
        for (int slotId = 0; slotId < simCount; slotId++) {
            int[] subId = SubscriptionManager.getSubId(slotId);
            if (subId != null && subId.length > 0 && SimCardUtils.isPhoneBookReady(subId[0])) {
                Log.i(TAG, "isPhbReady ready! ");
                return true;
            }
        }
        return false;
    }

    private void startSimService(Context context, int subId, int workType) {
        Intent intent = null;
        intent = new Intent(context, SimProcessorService.class);
        intent.putExtra(SimServiceUtils.SERVICE_SUBSCRIPTION_KEY, subId);
        intent.putExtra(SimServiceUtils.SERVICE_WORK_TYPE, workType);
        Log.d(TAG, "[startSimService]subId:" + subId + "|workType:" + workType);
        SimProcessorService.setServiceIsStarting(true);
        context.startService(intent);
    }

    private void processPhoneBookChanged(Context context, Intent intent) {
        Log.d(TAG, "processPhoneBookChanged");
        boolean phbReady = intent.getBooleanExtra("ready", false);
        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, ERROR_SUB_ID);
        Log.d(TAG, "[processPhoneBookChanged]phbReady:" + phbReady + "|subId:" + subId);
        if (phbReady && subId > 0) {
            startSimService(context, subId, SimServiceUtils.SERVICE_WORK_IMPORT);
        } else if (subId > 0 && !phbReady) {
            startSimService(context, subId, SimServiceUtils.SERVICE_WORK_REMOVE);
        }
    }

    /**
     * fix for [PHB Status Refatoring] ALPS01003520
     * when boot complete,remove the contacts if the card of a slot had been removed
     */
    private void processBootComplete(Context context) {
        Log.d(TAG, "processBootComplete");
        startSimService(context, SimServiceUtils.SERVICE_FORCE_REMOVE_SUB_ID,
            SimServiceUtils.SERVICE_WORK_REMOVE);
    }

    private void processDupSimContacts(Context context) {
        Log.d(TAG, "processDupSimContacts");
        startSimService(context, SimServiceUtils.SERVICE_REMOVE_DUP_SUB_ID,
            SimServiceUtils.SERVICE_WORK_REMOVE);
    }
}
