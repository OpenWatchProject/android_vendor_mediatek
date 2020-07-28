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

package com.mediatek.op06.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

import com.mediatek.ims.config.ImsConfigContract;
import com.mediatek.ims.internal.MtkImsConfig;
import mediatek.telephony.MtkCarrierConfigManager;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


/**
 * Class containing util apis for WifiCalling.
 */
public class WifiCallingUtils {

    private static final String TAG = "OP06WifiCallingUtils";
    private static final String ACTION_WIFI_ONLY_MODE_CHANGED
            = "android.intent.action.ACTION_WIFI_ONLY_MODE";
    private static final String EXTRA_WIFI_ONLY_MODE_CHANGED = "state";

    private static final Pattern fqdnPattern = Pattern
    .compile("(?=^.{1,254}$)(^(?:(?!\\d+\\.|-)[a-zA-Z0-9_\\-]{1,63}(?<!-)\\.?)+(?:[a-zA-Z]{2,})$)",
        Pattern.CASE_INSENSITIVE);

    /**
     * @param context Context
     * @param phoneId phoneId
     * @return
     * Customize boolean whether provisioned or not
     */
    public static boolean isWifiCallingProvisioned(Context context, int phoneId) {
        boolean wifiCallingProvisioned = false;
        ImsConfig imsConfig = null;
        ImsManager imsManager = ImsManager.getInstance(context, phoneId);
        if (imsManager != null) {
            try {
                imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    int value = imsConfig.getProvisionedValue(
                            ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED);
                    if (value == 1) {
                        wifiCallingProvisioned = true;
                    }
                }
            } catch (ImsException e) {
                Log.e(TAG, "WFC settings not updated, ImsConfig null");
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "WFC settings not updated, ImsManager null");
        }
        Log.d(TAG, "Wfc provisioned:" + wifiCallingProvisioned);
        return wifiCallingProvisioned;
    }

    /**
     * @param context Context
     * @param phoneId phoneId
     * @return String
     * Returns FQDN from ImsConfig
     */
    public static String getFQDN(Context context, int phoneId) {
        String wfcFqdn = null;
        ImsConfig imsConfig = null;
        ImsManager imsManager = ImsManager.getInstance(context, phoneId);
        if (imsManager != null) {
            try {
                imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    wfcFqdn = imsConfig.getProvisionedStringValue(
                            MtkImsConfig.ConfigConstants.EPDG_ADDRESS);
                }
            } catch (ImsException e) {
                Log.e(TAG, "WFC settings not updated, ImsConfig null");
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "WFC settings not updated, ImsManager null");
        }
        Log.d(TAG, "Wfc FQDN:" + wfcFqdn);
        return wfcFqdn;
    }

    /**
     * Get Wfc mode summary.
     * @param wfcMode wfcMode
     * @return int wfcMode
     */
    public static int getWfcModeSummary(int wfcMode) {
        int resId = com.android.internal.R.string.wifi_calling_off_summary;
            switch (wfcMode) {
            case ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY:
                resId = com.android.internal.R.string.wfc_mode_wifi_only_summary;
                break;
            case ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED:
                resId = com.android.internal.R.string.wfc_mode_cellular_preferred_summary;
                break;
            case ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED:
                resId = com.android.internal.R.string.wfc_mode_wifi_preferred_summary;
                break;
            default:
                Log.e("@M_" + TAG, "Unexpected WFC mode value: " + wfcMode);
        }
        return resId;
    }

    /**
     * @param context Context
     * @param receiver broacast reciever to register
     * @return
     * Registers the broadcast receiver
     */
    public static void registerReceiver(Context context, BroadcastReceiver receiver) {
        IntentFilter filter = new IntentFilter(ImsConfigContract.ACTION_CONFIG_LOADED);
        filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        context.registerReceiver(receiver, filter);
    }

    /**
     * @param context Context
     * @param receiver broacast reciever to unregister
     * @return
     * Unregisters the broadcast receiver
     */
    public static void unRegisterReceiver(Context context, BroadcastReceiver receiver) {
        context.unregisterReceiver(receiver);
    }

 /**
     * is IPv6 Address.
     *
     * @param ipAddress address
     * @return true if IPv6
     */
    public static boolean isIPv6(String ipAddress) {
        InetAddress addr;
        boolean isIPv6 = false;
        try {
            addr = InetAddress.getByName(ipAddress);
            isIPv6 = addr instanceof Inet6Address;
            Log.d("@M_" + TAG, "isIPv6:" + isIPv6);
        } catch (UnknownHostException e) {
            Log.d("@M_" + TAG, "UnknownHostException:" + e);
            return false;
        }
        return isIPv6;
    }

    /**
     * is IPv4 Address.
     *
     * @param ipAddress address
     * @return true if IPv4
     */
    public static boolean isIPv4(String ipAddress) {
        InetAddress addr;
        boolean isIPv4 = false;
        try {
            addr = InetAddress.getByName(ipAddress);
            isIPv4 = addr instanceof Inet4Address;
            Log.d("@M_" + TAG, "isIPv4:" + isIPv4);
        } catch (UnknownHostException e) {
            Log.d("@M_" + TAG, "UnknownHostException:" + e);
            return false;
        }
        return isIPv4;
    }

        /**
     * is valid FQDN. String validation only.
     *
     * @param fqdn fqdn
     * @return true if valid fqdn
     */
    public static boolean isValidFqdn(String fqdn) {
        boolean isvalid = false;
        try {
            Matcher matcher = fqdnPattern.matcher(fqdn);
            isvalid = matcher.matches();
        } catch (Exception e) {
            Log.d("@M_" + TAG, "Exception:" + e);
            isvalid = false;
        }
        return isvalid;
    }

    /**
     * Is Ims Provisioning feature supported or not.
     * @param context context
     * @param subId sybId
     * @return true if feature supported
     */
    public static boolean isImsProvSupported(Context context, int subId) {
        boolean isFeatureSupported = false;
        CarrierConfigManager configMgr = (CarrierConfigManager) context
                .getSystemService(Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle b = configMgr.getConfigForSubId(subId);
        if (b != null) {
            isFeatureSupported = b.getBoolean(MtkCarrierConfigManager
                                  .MTK_KEY_WFC_PROV_SUPPORTED_BOOL);
        }
        Log.d(TAG, "isFeatureSupported:" + isFeatureSupported);
        return isFeatureSupported;
    }
}
