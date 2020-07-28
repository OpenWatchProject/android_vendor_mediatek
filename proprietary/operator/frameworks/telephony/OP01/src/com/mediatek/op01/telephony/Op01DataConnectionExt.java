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

package com.mediatek.op01.telephony;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.NetworkRequest.Builder;
import android.net.wifi.IWifiManager;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.dataconnection.DataConnectionExt;

import vendor.mediatek.hardware.netdagent.V1_0.INetdagent;

/**
 * Implement methods to support China Mobile requirements.
 *
 * @return
 */
public class Op01DataConnectionExt extends DataConnectionExt {
    static final String TAG = "Op01DataConnectionExt";
    private Context mContext;

    public Op01DataConnectionExt(Context context) {
        super(context);
        log("constructor.");
    }

    @Override
    public boolean isDataAllowedAsOff(String apnType) {
        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_MMS)
                || TextUtils.equals(apnType, PhoneConstants.APN_TYPE_SUPL)
                || TextUtils.equals(apnType, PhoneConstants.APN_TYPE_IMS)
                || TextUtils.equals(apnType, PhoneConstants.APN_TYPE_EMERGENCY)
                || TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_XCAP)
                || TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_RCS)) {
            return true;
        }

        return false;
    }

    /**
     * For OP01 IOT test, when data connection activated, it will enable
     * firewall for non-ims pdn, which will drop unexpected dns package.
     *
     * @param apnTypes The data connection apnTypes.
     * @param ifc The interface name.
     */
    @Override
    public void onDcActivated(String[] apnTypes, String ifc) {
        if (apnTypes == null || ifc == null) {
            return;
        }

        log("onDcActivated. ifc: " + ifc);
        if (ifc.length() == 0) {
            return;
        }

        if (!hasImsApnType(apnTypes)) {
            return;
        }

        enableVolteIotFirewall(true, ifc);
    }

    /**
     * For OP01 IOT test, when data connection deactivated, it will
     * disable firewall for non-ims pdn.
     *
     * @param apnTypes The data connection apnTypes.
     * @param ifc The interface name.
     */
    @Override
    public void onDcDeactivated(String[] apnTypes, String ifc) {
        if (apnTypes == null || ifc == null) {
            return;
        }

        log("onDcDeactivated. ifc: " + ifc);
        if (ifc.length() == 0) {
            return;
        }

        if (!hasImsApnType(apnTypes)) {
            return;
        }

        enableVolteIotFirewall(false, ifc);
    }


    /**
     * To check metered apn type is decided by load type or not.
     *
     * @return true if metered apn type is decided by load type.
     */
    @Override
    public boolean isMeteredApnTypeByLoad() {
        return true;
    }


    /**
     * To check apn type is metered or not.
     *
     * @param type APN type.
     * @param isRoaming true if network in roaming state.
     * @return true if this APN type is metered.
     */
    @Override
    public boolean isMeteredApnType(String type, boolean isRoaming) {
        log("apnType=" + type + ",isRoaming=" + isRoaming);
        // Default metered apn type: [default, supl, dun, mms]
        if (TextUtils.equals(type, PhoneConstants.APN_TYPE_DEFAULT)
            || TextUtils.equals(type, PhoneConstants.APN_TYPE_DUN)
            || (isTestSim() && isRoaming && TextUtils.equals(type, PhoneConstants.APN_TYPE_MMS))) {
            return true;
        }

        return false;
    }

    /**
     *Check current sim is test sim or not.
     */
    private boolean isTestSim() {
        String simType = SystemProperties.get("vendor.gsm.sim.ril.testsim");
        if (simType == null || !simType.equals("1")) {
            simType = SystemProperties.get("vendor.gsm.sim.ril.testsim.2");
            if (simType == null || !simType.equals("1")) {
                return false;
            }
        }
       return true;
    }

    /**
     *Print radio log.
     *@param text The context needs to be printed.
     */
    @Override
    public void log(String text) {
        Rlog.d(TAG, text);
    }

    private boolean hasImsApnType(String[] apnTypes) {
        boolean ret = false;
        if (apnTypes != null) {
            for (String apnType : apnTypes) {
                if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_IMS)) {
                    log("apnType = " + apnType);
                    ret = true;
                    break;
                }
            }
        }

        return ret;
    }

    private void enableVolteIotFirewall(final boolean enable, final String ifc) {
        log("enableVolteIotFirewall,ifc:" + (ifc == null ? "null" : ifc) + "," + enable);

        String simType = SystemProperties.get("vendor.gsm.sim.ril.testsim");
        if (simType == null || !simType.equals("1")) {
            simType = SystemProperties.get("vendor.gsm.sim.ril.testsim.2");
            if (simType == null || !simType.equals("1")) {
                log("enableVolteIotFirewall, not TEST SIM");
                return;
            }
        }

        Thread thread = new Thread("enableVolteIotFirewall") {
            public void run() {
                try {
                    INetdagent agent = INetdagent.getService();
                    if (agent == null) {
                        log("agnet is null");
                        return;
                    }

                    String cmd = "";
                    if (enable) {
                        cmd = String.format("netdagent firewall set_volte_nsiot_firewall %s", ifc);
                    } else {
                        cmd = String.format("netdagent firewall clear_volte_nsiot_firewall %s", ifc);
                    }
                    log("cmd:" + cmd);
                    agent.dispatchNetdagentCmd(cmd);
                } catch (Exception e) {
                    log("enableVolteIotFirewall:" + e);
                }
            }
        };
        thread.start();
    }

}
