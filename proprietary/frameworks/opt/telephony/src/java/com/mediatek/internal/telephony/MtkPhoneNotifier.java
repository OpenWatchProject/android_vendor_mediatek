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

import java.util.Arrays;

import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.AccessNetworkConstants;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.text.TextUtils;

import com.android.internal.telephony.DefaultPhoneNotifier;
import com.android.internal.telephony.ITelephonyRegistry;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.internal.telephony.dataconnection.MtkDcTracker;
import com.mediatek.internal.telephony.IMtkTelephonyRegistryEx;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkTelephonyRegistryEx;

import mediatek.telephony.MtkServiceState;

/**
 * broadcast intents
 */
public class MtkPhoneNotifier extends DefaultPhoneNotifier {
    private static final String LOG_TAG = "MtkPhoneNotifr";
    private static final boolean DBG = false; // STOPSHIP if true

    protected IMtkTelephonyRegistryEx mMtkRegistry;

    // TelephonyRegistry will drop the notify with this mFakeSub and valid phone id.
    private final int mFakeSub = Integer.MAX_VALUE - 1;

    public MtkPhoneNotifier() {
        super();
        Rlog.d(LOG_TAG, "constructor");
        MtkTelephonyRegistryEx.init();
        mMtkRegistry = IMtkTelephonyRegistryEx.Stub.asInterface(ServiceManager.getService(
                "telephony.mtkregistry"));
    }

    private boolean checkSubIdPhoneId(Phone s) {
        if (s.getSubId() < 0) {
            // this SIM is removed.
            int defaultFallbackSubId = // for debug info
                    MtkSubscriptionController.getMtkInstance().getDefaultFallbackSubId();
            int defaultFallbackPhoneId = // for debug info
                    MtkSubscriptionController.getMtkInstance().getPhoneId(defaultFallbackSubId);
            int subIds[] =
                    MtkSubscriptionController.getMtkInstance().getActiveSubIdList(true);
            Rlog.d(LOG_TAG, "checkSubIdPhoneId defaultFallbackSubId:" + defaultFallbackSubId
                    + " defaultFallbackPhoneId:" + defaultFallbackPhoneId
                    + " sender's SubId:" + s.getSubId()
                    + " activeSubIds: " + Arrays.toString(subIds));
            // sub is invalid, do check whether there is other sims
            // return true for doing broadcast when there is no sim
            // return false for using Fake subId to broadcast to no receiver.
            return subIds.length == 0 ? true : false;
        } else {
            // sub is valid
            return true;
        }
    }

    @Override
    public void notifyServiceState(Phone sender) {
        if (checkSubIdPhoneId(sender)) {
            super.notifyServiceState(sender);
        } else {
            ServiceState ss = sender.getServiceState();
            int phoneId = sender.getPhoneId();
            // Fake Sub leads to no receiver but clear the cache in TelephonyRegistry
            int subId = mFakeSub;

            if (ss == null) {
                ss = new MtkServiceState();
                ss.setStateOutOfService();
            }

            Rlog.d(LOG_TAG, "MtkPhoneNotifier notifyServiceState"
                    + " phoneId:" + phoneId
                    + " fake subId: " + subId
                    + " ServiceState: " + ss);

            try {
                if (mRegistry != null) {
                    mRegistry.notifyServiceStateForPhoneId(phoneId, subId, ss);
                }
            } catch (RemoteException ex) {
                // system process is dead
            }
        }
    }

    @Override
    public void notifySignalStrength(Phone sender) {
        if (checkSubIdPhoneId(sender)) {
            super.notifySignalStrength(sender);
        } else {
            int phoneId = sender.getPhoneId();
            // Fake Sub leads to no receiver but clear the cache in TelephonyRegistry
            int subId = mFakeSub;

            Rlog.d(LOG_TAG, "MtkPhoneNotifier notifySignalStrength"
                    + " phoneId:" + phoneId
                    + " fake subId: " + subId
                    + " signal: " + sender.getSignalStrength());

            try {
                if (mRegistry != null) {
                    mRegistry.notifySignalStrengthForPhoneId(phoneId, subId,
                            sender.getSignalStrength());
                }
            } catch (RemoteException ex) {
                // system process is dead
            }
        }
    }

    @Override
    public void notifyDataConnection(Phone sender, String apnType,
            PhoneConstants.DataState state) {
        if (sender.getActiveApnHost(apnType) == null &&
                !(PhoneConstants.APN_TYPE_DEFAULT.equals(apnType)
                || PhoneConstants.APN_TYPE_EMERGENCY.equals(apnType))) {
            return;
        }

         super.notifyDataConnection(sender, apnType, state);
    }

    public void notifyMtkServiceState(Phone sender, MtkServiceState mss) {
        ServiceState ss = (ServiceState) mss;
        int phoneId = sender.getPhoneId();
        // Fake Sub leads to no receiver but clear the cache in TelephonyRegistry
        int subId = mFakeSub;

        if (ss == null) {
            ss = new MtkServiceState();
            ss.setStateOutOfService();
        }

        Rlog.d(LOG_TAG, "MtkPhoneNotifier notifyMtkServiceState"
                + " phoneId:" + phoneId
                + " fake subId: " + subId
                + " ServiceState: " + ss);

        try {
            if (mRegistry != null) {
                mRegistry.notifyServiceStateForPhoneId(phoneId, subId, ss);
            }
        } catch (RemoteException ex) {
            // system process is dead
        }
    }

    public void notifyMtkSignalStrength(Phone sender, SignalStrength ss) {
        int phoneId = sender.getPhoneId();
        // Fake Sub leads to no receiver but clear the cache in TelephonyRegistry
        int subId = mFakeSub;

        Rlog.d(LOG_TAG, "MtkPhoneNotifier notifyMtkSignalStrength"
                + " phoneId:" + phoneId
                + " fake subId: " + subId
                + " signal: " + ss);

        try {
            if (mRegistry != null) {
                mRegistry.notifySignalStrengthForPhoneId(phoneId, subId,
                        ss);
            }
        } catch (RemoteException ex) {
            // system process is dead
        }
    }

    @Override
    protected int mtkGetDataNetworkType(TelephonyManager telephony, Phone sender,
            String apnType, PhoneConstants.DataState state, int subId) {
        /// M: Data icon performance enhancement @{
        if (apnType.equals(PhoneConstants.APN_TYPE_DEFAULT)
                && state == PhoneConstants.DataState.CONNECTED) {
            MtkDcTracker dct = (MtkDcTracker)
                    sender.getDcTracker(AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
            MtkServiceState turboSS = dct.getTurboSS();
            if (turboSS != null) {
                Rlog.d(LOG_TAG, "mtkGetDataNetworkType: get turbo SS");
                return turboSS.getDataNetworkType();
            }
        }
        /// @}
        return super.mtkGetDataNetworkType(telephony, sender, apnType, state, subId);
    }

    private boolean mIsOp20 = "OP20".equals(SystemProperties.get(
                "persist.vendor.operator.optr", ""));

    /**
     * Anchor method of doNotifyDataConnection
     */
    @Override
    protected String mtkConvertDunToDefault(String apnType, Phone sender, ServiceState ss) {
        if (!mIsOp20)
            return apnType;
        if (sender.getActiveApnHost(apnType) == null ||
                sender.getActiveApnHost(apnType).toLowerCase().contains("pam"))
            return apnType;
        if (!TextUtils.equals(apnType, ApnSetting.getApnTypeString(ApnSetting.TYPE_DUN)))
            return apnType;
        if (sender.getDataConnectionState() == PhoneConstants.DataState.CONNECTED)
            return apnType;

        int rilRat = ServiceState.networkTypeToRilRadioTechnology(ss.getDataNetworkType());
        if (rilRat == ServiceState.RIL_RADIO_TECHNOLOGY_IS95A ||
                rilRat == ServiceState.RIL_RADIO_TECHNOLOGY_IS95B  ||
                rilRat == ServiceState.RIL_RADIO_TECHNOLOGY_1xRTT  ||
                rilRat == ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_0 ||
                rilRat == ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_A ||
                rilRat == ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_B) {
            apnType = ApnSetting.getApnTypeString(ApnSetting.TYPE_DEFAULT);
        }
        return apnType;
    }
}
