/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.ims.rcsua.service.utils;

import android.content.Context;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.mediatek.ims.rcsua.service.R;

import java.util.ArrayList;

public class Utils {
    public static void setApplicationContext(Context context) {
        appContext = context;
    }

    public static Context getApplicationContext() {
        return appContext;
    }

    public static boolean sendAtCommand(final String command) {
        return !(executeCommandResponse(command) == null);
    }

    public static boolean isOP08Supported() {
        boolean status = false;

        String optr = SystemProperties.get("persist.vendor.operator.optr");
        if (optr.equalsIgnoreCase("op08")) {
            status = true;
        }
        return status;
    }

    public static boolean isOP07Supported() {
        boolean status = false;

        String optr = SystemProperties.get("persist.vendor.operator.optr");
        if (optr.equalsIgnoreCase("op07")) {
            status = true;
        }
        return status;
    }

    public static boolean isForceRoiSupport() {
        return SystemProperties.get("persist.vendor.rcs.force_roi_enabled").equals("1");
    }

    public static boolean isTestSim() {
        return (SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1")
                || SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1")
                || SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1")
                || SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1"));
    }

    public static boolean isVopsOn() {
        boolean isVopsOn = true;
        String result = executeCommandResponse("AT+CIREP?");
        if (result != null) {
            // +CIREP: <reporting>,<nwimsvops>
            // <nwimsvops>: support IMSVOPS or not
            // 0 : VoPS no support
            // 1 : VoPS support
            // +CIREP: 1,0
            try {
                String[] tokens = result.split(",|\r|\n");

                if (tokens.length >= 4) {
                    int vops = Integer.parseInt(tokens[3]);
                    isVopsOn = (vops == 1) ? true : false;
                }
            } catch (Exception e) {
            }
        }

        return isVopsOn;
    }

    public static int getSupportedCarrier(String plmn) {
        if (plmn == null || plmn.isEmpty())
            return 0;

        int mccmnc = Integer.parseInt(plmn);
        String[] opList = appContext.getResources().getStringArray(
                R.array.operator_list);

        for (String op : opList) {
            String[] plmns = op.split(",");
            int min = Integer.parseInt(plmns[0].trim());
            int max = Integer.parseInt(plmns[1].trim());
            if (mccmnc >= min && mccmnc <= max)
                return Integer.parseInt(plmns[2].trim());;
        }

        return 0;
    }

    public static boolean isSimSupported() {
        TelephonyManager telMgr = getTelephonyManager();
        int simState = telMgr.getSimState(getMainCapabilityPhoneId());
        if (simState == TelephonyManager.SIM_STATE_READY) {
            String plmn = telMgr.getSimOperator();
            if (Utils.getSupportedCarrier(plmn) > 0) {
                return true;
            }
        }

        return false;
    }

    public static boolean isSimSupported(int carrier) {
        TelephonyManager telMgr = getTelephonyManager();
        int simState = telMgr.getSimState(getMainCapabilityPhoneId());
        if (simState == TelephonyManager.SIM_STATE_READY) {
            String plmn = telMgr.getSimOperator();
            if (Utils.getSupportedCarrier(plmn) == carrier) {
                return true;
            }
        }

        return false;
    }

    public static ArrayList<Byte> arrayToList(byte[] value, int length) {
        ArrayList<Byte> al = new ArrayList<Byte>(length);
        for (int i = 0; i < length; i++) {
            al.add(value[i]);
        }
        return al;
    }

    public static byte[] listToArray(ArrayList<Byte> data) {
        byte[] ba = new byte[data.size()];
        for (int i = 0; i < data.size(); i++) {
            ba[i] = data.get(i);
        }
        return ba;
    }

    public static int getMainCapabilityPhoneId() {
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        int phoneId;
        if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID)
            phoneId = SubscriptionManager.getPhoneId(subId);
        else
            phoneId = 0;

        return phoneId;
    }

    public static void enableVopsIndication() {
        // Enable VOPS URC
        sendAtCommand("AT+CIREP=1");
    }

    public static TelephonyManager getTelephonyManager() {
        return (TelephonyManager)appContext.getSystemService(
                Context.TELEPHONY_SERVICE);
    }

    private static synchronized String executeCommandResponse(String cmd) {
        final int MAX_RESPONSE_LEN = 256 + 1;
        String response = null;
        byte[] rawResp = new byte[MAX_RESPONSE_LEN];

        TelephonyManager tm = (TelephonyManager)appContext.getSystemService(
                Context.TELEPHONY_SERVICE);
        int len = tm.invokeOemRilRequestRaw(cmd.getBytes(), rawResp);
        if (len > 0) {
            response = new String(rawResp, 0, len);
            if (response.indexOf("+CME ERROR") != -1) {
                response = null;
            }
        }

        return response;
    }

    static Context appContext;
}
