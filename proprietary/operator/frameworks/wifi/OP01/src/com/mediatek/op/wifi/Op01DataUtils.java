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

package com.mediatek.op.wifi;

import android.content.Context;
import android.content.Intent;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.provider.MtkSettingsExt;

public class Op01DataUtils {

    private static String TAG = "Op01DataUtils";

    public static boolean isDataAvailable(Context context) {
        try {
            ITelephony phone = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
            TelephonyManager tm =
                    (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
            if (phone == null || !phone.isRadioOn(context.getPackageName()) || tm == null) {
                return false;
            }

            boolean isSim1Insert = tm.hasIccCard(PhoneConstants.SIM_ID_1);
            boolean isSim2Insert = false;
            if (tm.getDefault().getPhoneCount() >= 2) {
                isSim2Insert = tm.hasIccCard(PhoneConstants.SIM_ID_2);
            }
            if (!isSim1Insert && !isSim2Insert) {
                return false;
            }
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    public static boolean isPsDataAvailable(Context context) {
        // Check SIM ready
        TelephonyManager telMgr = (TelephonyManager) context.getSystemService(
                Context.TELEPHONY_SERVICE);
        if (telMgr == null) {
            log("TelephonyManager is null");
            return false;
        }

        boolean isSIMReady = false;
        int n = telMgr.getSimCount();
        for (int i = 0; i < n; i++) {
            if (telMgr.getSimState(i) == TelephonyManager.SIM_STATE_READY) {
                isSIMReady = true;
                break;
            }
        }

        log("isSIMReady: " + isSIMReady);
        if (!isSIMReady) {
            return false;
        }

        // check radio on
        ITelephony iTel = ITelephony.Stub.asInterface(
                ServiceManager.getService(Context.TELEPHONY_SERVICE));
        if (iTel == null) {
            log("ITelephony is null");
            return false;
        }

        SubscriptionManager subMgr = SubscriptionManager.from(context);
        if (subMgr == null) {
            log("SubscriptionManager is null");
            return false;
        }

        int[] subIdList = subMgr.getActiveSubscriptionIdList();
        n = 0;
        if (subIdList != null) {
            n = subIdList.length;
        }

        boolean isRadioOn = false;
        for (int i = 0; i < n; i++) {
            try {
                isRadioOn = iTel.isRadioOnForSubscriber(
                        subIdList[i],
                        context.getPackageName());
                if (isRadioOn) {
                    break;
                }
            } catch (RemoteException e) {
                log("isRadioOnForSubscriber RemoteException");
                isRadioOn = false;
            }
        }
        if (!isRadioOn) {
            log("All sub Radio OFF");
            return false;
        }

        // Check flight mode
        int airplanMode = Settings.System.getInt(
                context.getContentResolver(),
                Settings.System.AIRPLANE_MODE_ON, 0);
        log("airplanMode:" + airplanMode);
        if (airplanMode == 1) {
            return false;
        }

        return true;
    }

    public static void turnOffDataConnection(Context context) {
        TelephonyManager tm =
                (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        if (tm != null) {
            tm.setDataEnabled(false);
        }
    }

    public static void recordLastDataStatus(Context context) {
        TelephonyManager tm =
                (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        // Remember last status on(1) or off(-1)
        int lastId = tm.getDataEnabled() ? 1 : -1;
        Settings.System.putLong(context.getContentResolver(),
                MtkSettingsExt.System.LAST_SIMID_BEFORE_WIFI_DISCONNECTED,
                lastId);
        log("recordLastDataStatus lastId " + lastId);
    }

    public static void showDataDialog(Context context) {
        log("showDataDialog (package as android)");
        Intent intent = new Intent(Op01WifiUtils.ACTION_WIFI_FAILOVER_GPRS_DIALOG);
        intent.setPackage(Op01WifiUtils.PACKAGE_ANDROID);
        context.sendBroadcastAsUser(intent, UserHandle.ALL);
    }

    private static void log(String message) {
        Op01WifiUtils.log(TAG, message);
    }
}
