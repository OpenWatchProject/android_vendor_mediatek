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

import android.content.Context;
import android.os.ServiceManager;
import android.telephony.Rlog;
import android.text.TextUtils;

/**
 * SubscriptionController to provide an inter-process communication to
 * access Sms in Icc.
 *
 * Any setters which take subId, slotId or phoneId as a parameter will throw an exception if the
 * parameter equals the corresponding INVALID_XXX_ID or DEFAULT_XXX_ID.
 *
 * All getters will lookup the corresponding default if the parameter is DEFAULT_XXX_ID. Ie calling
 * getPhoneId(DEFAULT_SUB_ID) will return the same as getPhoneId(getDefaultSubId()).
 *
 * Finally, any getters which perform the mapping between subscriptions, slots and phones will
 * return the corresponding INVALID_XXX_ID if the parameter is INVALID_XXX_ID. All other getters
 * will fail and return the appropriate error value. Ie calling getSlotId(INVALID_SUBSCRIPTION_ID)
 * will return INVALID_SLOT_ID and calling getSubInfoForSubscriber(INVALID_SUBSCRIPTION_ID)
 * will return null.
 *
 */
public class MtkSubscriptionControllerEx extends IMtkSub.Stub {
    private static final String LOG_TAG = "MtkSubscriptionControllerEx";
    private static final boolean ENGDEBUG = TextUtils.equals(android.os.Build.TYPE, "eng");
    private static final boolean DBG = true;
    private static MtkSubscriptionControllerEx sInstance = null;

    private Context mContext;

    protected MtkSubscriptionControllerEx(Context c) {
        mContext = c;
        if(ServiceManager.getService("isubstub") == null) {
                ServiceManager.addService("isubstub", this);
            if (DBG) {
                Rlog.d(LOG_TAG, "[MtkSubscriptionControllerEx] init by Context, this = " + this);
            }
        }

        if (DBG) Rlog.d(LOG_TAG, "[MtkSubscriptionControllerEx] init by Context");
    }

    protected static void MtkInitStub(Context c) {
        synchronized (MtkSubscriptionControllerEx.class) {
            if (sInstance == null) {
                sInstance = new MtkSubscriptionControllerEx(c);
                if (DBG) Rlog.d(LOG_TAG, "[MtkSubscriptionControllerEx] sInstance = " + sInstance);
            } else {
                Rlog.w(LOG_TAG, "init() called multiple times!  sInstance = " + sInstance);
            }
        }
    }

    // FIXME: getSubInfo and getSubInfoForIccId seem could be removed.
    // It could be replace by AOSP API. It seems only for simple usage.
    /**
     * Get the SubscriptionInfo with the subId key.
     * @param subId The unique SubscriptionInfo key in database
     * @param callingPackage operation package name
     * @return SubscriptionInfo, maybe null if not found
     * @hide
     */
    @Override
    public MtkSubscriptionInfo getSubInfo(String callingPackage, int subId) {
        return MtkSubscriptionController.getMtkInstance()
                  .getSubscriptionInfo(callingPackage, subId);
    }

    /**
     * Get the SubscriptionInfo associated with the iccId.
     * @param callingPackage operation package name
     * @param iccId the IccId of SIM card
     * @return SubscriptionInfo, maybe null if not found
     * @hide
     */
    @Override
    public MtkSubscriptionInfo getSubInfoForIccId(String callingPackage, String iccId) {
        return MtkSubscriptionController.getMtkInstance()
                .getSubscriptionInfoForIccId(callingPackage, iccId);
    }

    /**
     * Get subId associated with the slotId.
     * @param phoneId the specified phoneId
     * @return subId as a positive integer
     * INVALID_SUBSCRIPTION_ID if an invalid phone index
     * @hide
     */
    @Override
    public int getSubIdUsingPhoneId(int phoneId) {
        return MtkSubscriptionController.getMtkInstance().getSubIdUsingPhoneId(phoneId);
    }

    /* Sets the default subscription. If only one sub is active that
     * sub is set as default subId. If two or more  sub's are active
     * the first sub is set as default subscription
     * @hide
     */
    @Override
    public void setDefaultFallbackSubId(int subId, int subscriptionType) {
        MtkSubscriptionController.getMtkInstance().setDefaultFallbackSubId(subId,
                subscriptionType);
    }

    /**
     * Set deafult data sub ID without invoking capability switch.
     * @param subId the default data sub ID
     * @hide
     */
    @Override
    public void setDefaultDataSubIdWithoutCapabilitySwitch(int subId) {
        MtkSubscriptionController.getMtkInstance()
                .setDefaultDataSubIdWithoutCapabilitySwitch(subId);
    }
}
