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

package com.mediatek.internal.telephony.dataconnection;

import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;
import android.net.NetworkConfig;
import android.net.NetworkRequest;
import android.os.Bundle;
import android.telephony.data.ApnSetting;
import android.telephony.Rlog;
import android.util.LocalLog;

import com.android.internal.telephony.dataconnection.ApnContext;
import com.android.internal.telephony.dataconnection.DataConnection;
import com.android.internal.telephony.dataconnection.DcTracker;
import com.android.internal.telephony.DctConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkDctConstants;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;

import java.util.ArrayList;
import mediatek.telephony.data.MtkApnSetting;

public class MtkApnContext extends ApnContext {
    private final static String SLOG_TAG = "MtkApnContext";

    // M: [OD over ePDG] start
    private ArrayList<ApnSetting> mWifiApns = null;
    // M: [OD over ePDG] end

    /**
      * To decrease the time of unused apn type.
      */
    private boolean mNeedNotify;

    private static final int NETWORK_TYPE_MCX = 1001;

    private DataConnection mDataConnectionSscMode3;

    /**
     * ApnContext constructor
     * @param phone phone object
     * @param apnType APN type (e.g. default, supl, mms, etc...)
     * @param logTag Tag for logging
     * @param config Network configuration
     * @param tracker Data call tracker
     */
    public MtkApnContext(Phone phone, String apnType, String logTag, NetworkConfig config,
            DcTracker tracker) {
        super(phone, apnType, logTag, config, tracker);
        mNeedNotify = needNotifyType(apnType);
    }

    // M: [OD over ePDG] start
    public synchronized void setWifiApns(ArrayList<ApnSetting> wifiApns) {
        mWifiApns = wifiApns;
    }

    public synchronized ArrayList<ApnSetting> getWifiApns() {
        return mWifiApns;
    }
    // M: [OD over ePDG] end

    /// SSC Mode 3 @{
    @Override
    public ApnSetting getNextApnSetting() {
        if (MtkGsmCdmaPhone.REASON_DATA_SETUP_SSC_MODE3.equals(getReason())) {
            return getApnSetting();
        }
        return super.getNextApnSetting();
    }

    public synchronized DataConnection getDataConnectionSscMode3() {
        return mDataConnectionSscMode3;
    }

    public synchronized void setDataConnectionSscMode3(DataConnection dc) {
        if (DBG) {
            log("setDataConnectionSscMode3: old dc=" + mDataConnectionSscMode3
                    + ", new dc=" + dc + ", this=" + this);
        }
        mDataConnectionSscMode3 = dc;
    }
    /// @}

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        mNeedNotify = true;
    }

    // reflection for ApnContext.getApnTypeFromNetworkType
    private static int getApnTypeFromNetworkTypeEx(int networkType) {
        switch (networkType) {
            case ConnectivityManager.TYPE_MOBILE_WAP:
                return MtkApnSetting.TYPE_WAP;
            case ConnectivityManager.TYPE_MOBILE_XCAP:
                return MtkApnSetting.TYPE_XCAP;
            case ConnectivityManager.TYPE_MOBILE_RCS:
                return MtkApnSetting.TYPE_RCS;
            case ConnectivityManager.TYPE_MOBILE_BIP:
                return MtkApnSetting.TYPE_BIP;
            case ConnectivityManager.TYPE_MOBILE_VSIM:
                return MtkApnSetting.TYPE_VSIM;
            case NETWORK_TYPE_MCX:
                return ApnSetting.TYPE_MCX;
            default:
                return ApnSetting.TYPE_NONE;
        }
    }

    // reflection for ApnContext.getApnTypeFromNetworkRequest
    private static Bundle getApnTypeFromNetworkRequestEx(NetworkCapabilities nc, int apnType,
            boolean error) {
        if (nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_WAP)) {
            if (apnType != ApnSetting.TYPE_NONE) error = true;
            apnType = MtkApnSetting.TYPE_WAP;
        }
        if (nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_XCAP)) {
            if (apnType != ApnSetting.TYPE_NONE) error = true;
            apnType = MtkApnSetting.TYPE_XCAP;
        }
        if (nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_RCS)) {
            if (apnType != ApnSetting.TYPE_NONE) error = true;
            apnType = MtkApnSetting.TYPE_RCS;
        }
        if (nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_BIP)) {
            if (apnType != ApnSetting.TYPE_NONE) error = true;
            apnType = MtkApnSetting.TYPE_BIP;
        }
        if (nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_VSIM)) {
            if (apnType != ApnSetting.TYPE_NONE) error = true;
            apnType = MtkApnSetting.TYPE_VSIM;
        }

        Bundle b = new Bundle();
        b.putInt("apnType", apnType);
        b.putBoolean("error", error);
        return b;
    }

    private boolean needNotifyType(String apnTypes) {
        if (apnTypes.equals(MtkPhoneConstants.APN_TYPE_WAP)
                || apnTypes.equals(MtkPhoneConstants.APN_TYPE_XCAP)
                || apnTypes.equals(MtkPhoneConstants.APN_TYPE_RCS)
                || apnTypes.equals(MtkPhoneConstants.APN_TYPE_BIP)
                || apnTypes.equals(MtkPhoneConstants.APN_TYPE_VSIM)
                ) {
            return false;
        }
        return true;
    }

    public boolean isNeedNotify() {
        if (DBG) {
            log("Current apn tpye:" + mApnType + " isNeedNotify" + mNeedNotify);
        }
        return mNeedNotify;
    }

    // M: [OD over ePDG] start
    @Override
    public synchronized String toString() {
        return super.toString() + " mWifiApns={" + mWifiApns + "}";
    }
    // M: [OD over ePDG] end
}
