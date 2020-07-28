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

import android.annotation.ProductApi;
import android.telephony.Rlog;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.telephony.SubscriptionManager;

import com.mediatek.internal.telephony.IMtkSub;

/**
 * SubscriptionManager is the application interface to SubscriptionController
 * and provides information about the current Telephony Subscriptions.
 * * <p>
 * You do not instantiate this class directly; instead, you retrieve
 * a reference to an instance through {@link #from}.
 * <p>
 * All SDK public methods require android.Manifest.permission.READ_PHONE_STATE.
 */
public class MtkSubscriptionManager {
    private static final String LOG_TAG = "MtkSubscriptionManager";
    private static final boolean DBG = false;
    private static final boolean VDBG = false;

    public static final int EXTRA_VALUE_NEW_SIM = 1;
    public static final int EXTRA_VALUE_REMOVE_SIM = 2;
    public static final int EXTRA_VALUE_REPOSITION_SIM = 3;
    public static final int EXTRA_VALUE_NOCHANGE = 4;

    public static final String INTENT_KEY_DETECT_STATUS = "simDetectStatus";
    public static final String INTENT_KEY_SIM_COUNT = "simCount";
    public static final String INTENT_KEY_PROP_KEY = "simPropKey";

    public MtkSubscriptionManager() {
    }

    // FIXME: getSubInfo and getSubInfoForIccId seem could be removed.
    // It could be replace by AOSP API. It seems only for simple usage.
    /**
     * Get the SubscriptionInfo with the subId key.
     * Note!!! Please using AOSP API first if AOSP can cover
     * @param callingPackage operation package name
     * @param subId The unique SubscriptionInfo key in database
     * @return SubscriptionInfo, maybe null if not found
     */
    @ProductApi
    public static MtkSubscriptionInfo getSubInfo(String callingPackage, int subId) {
        if (VDBG) {
            Rlog.d(LOG_TAG, "[getSubInfo]+ subId=" + subId);
        }

        if (!SubscriptionManager.isValidSubscriptionId(subId)) {
            Rlog.d(LOG_TAG, "[getSubInfo]- invalid subId, subId = " + subId);
            return null;
        }

        MtkSubscriptionInfo subInfo = null;

        try {
            IMtkSub iSub = IMtkSub.Stub.asInterface(ServiceManager.getService("isubstub"));
            if (iSub != null) {
                subInfo = iSub.getSubInfo(callingPackage, subId);
            }
        } catch (RemoteException ex) {
            // ignore it
        }

        return subInfo;
    }

    /**
     * Get the SubscriptionInfo associated with the iccId.
     * Note!!! Please using AOSP API first if AOSP can cover
     * @param callingPackage operation package name
     * @param iccId the IccId of SIM card
     * @return SubscriptionInfo, maybe null if not found
     */
    public static MtkSubscriptionInfo getSubInfoForIccId(String callingPackage, String iccId) {
        if (VDBG) {
            Rlog.d(LOG_TAG, "[getSubInfoForIccId]+ iccId=" + iccId);
        }

        if (iccId == null) {
            Rlog.d(LOG_TAG, "[getSubInfoForIccId]- null iccid");
            return null;
        }

        MtkSubscriptionInfo result = null;

        try {
            IMtkSub iSub = IMtkSub.Stub.asInterface(ServiceManager.getService("isubstub"));
            if (iSub != null) {
                result = iSub.getSubInfoForIccId(callingPackage, iccId);
            }
        } catch (RemoteException ex) {
            // ignore it
        }

        return result;
    }

    /**
     * Get subId associated with the slotId.
     * @param phoneId the specified phoneId
     * @return subId as a positive integer
     * INVALID_SUBSCRIPTION_ID if an invalid phone index
     */
    @ProductApi
    public static int getSubIdUsingPhoneId(int phoneId) {
        if (VDBG) Rlog.d(LOG_TAG, "[getSubIdUsingPhoneId]+ phoneId:" + phoneId);

        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

        try {
            IMtkSub iSub = IMtkSub.Stub.asInterface(ServiceManager.getService("isubstub"));
            if (iSub != null) {
                subId = iSub.getSubIdUsingPhoneId(phoneId);
            }
        } catch (RemoteException ex) {
            // ignore it
        }

        return subId;
    }

    /**
     * Set subId as default SubId.
     * @param subId the specified subId
     */
    public static void setDefaultSubId(int subId) {
        if (VDBG) {
            Rlog.d(LOG_TAG, "setDefaultSubId sub id = " + subId);
        }
        if (subId <= 0) {
            printStackTrace("setDefaultSubId subId 0");
        }

        try {
            IMtkSub iSub = IMtkSub.Stub.asInterface(ServiceManager.getService("isubstub"));
            if (iSub != null) {
                iSub.setDefaultFallbackSubId(subId,
                        SubscriptionManager.SUBSCRIPTION_TYPE_LOCAL_SIM);
            }
        } catch (RemoteException ex) {
            // ignore it
        }
    }

    /**
     * Set deafult data sub ID without invoking capability switch.
     * @param subId the default data sub ID
     */
    public static void setDefaultDataSubIdWithoutCapabilitySwitch(int subId) {
        if (VDBG) {
            Rlog.d(LOG_TAG, "setDefaultDataSubIdWithoutCapabilitySwitch sub id = " + subId);
        }

        if (subId <= 0) {
            printStackTrace(
                    "setDefaultDataSubIdWithoutCapabilitySwitch subId 0");
        }

        try {
            IMtkSub iSub = IMtkSub.Stub.asInterface(ServiceManager.getService("isubstub"));
            if (iSub != null) {
                iSub.setDefaultDataSubIdWithoutCapabilitySwitch(subId);
            }
        } catch (RemoteException ex) {
            // ignore it
        }
    }

    /**
     * Print stack trace
     */
    private static void printStackTrace(String msg) {
        RuntimeException re = new RuntimeException();
        Rlog.d(LOG_TAG, "StackTrace - " + msg);
        StackTraceElement[] st = re.getStackTrace();
        for (StackTraceElement ste : st) {
            Rlog.d(LOG_TAG, ste.toString());
        }
    }
}

