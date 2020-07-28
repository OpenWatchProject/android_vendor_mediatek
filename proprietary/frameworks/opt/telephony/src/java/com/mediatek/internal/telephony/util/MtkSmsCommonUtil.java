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


package com.mediatek.internal.telephony.util;

import static com.android.internal.telephony.IccSmsInterfaceManager.SMS_MESSAGE_PERIOD_NOT_SPECIFIED;
import static com.android.internal.telephony.IccSmsInterfaceManager.SMS_MESSAGE_PRIORITY_NOT_SPECIFIED;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.Build;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Intents;
import android.telephony.Rlog;
import android.telephony.SmsManager;
import android.util.Pair;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.SMSDispatcher.SmsTracker;
import com.android.internal.telephony.InboundSmsTracker;
import com.android.internal.telephony.cdma.CdmaInboundSmsHandler;
import com.android.internal.telephony.cdma.CdmaSMSDispatcher;
import com.android.internal.telephony.gsm.GsmInboundSmsHandler;
import com.android.internal.telephony.gsm.GsmSMSDispatcher;

import java.util.ArrayList;
import java.util.HashMap;

import android.os.SystemProperties;
import android.app.ActivityManager;
import android.provider.Telephony;

import android.telephony.SubscriptionManager;

import com.mediatek.internal.telephony.ppl.PplSmsFilterExtension;

import mediatek.telephony.MtkTelephony;

import java.util.Iterator;
import java.util.List;

/**
 *
 */
public final class MtkSmsCommonUtil {
    private static final String TAG = "MtkSmsCommonUtil";

    private static final boolean ENG = "eng".equals(Build.TYPE);

    public static final String SELECT_BY_REFERENCE = "address=? AND reference_number=? AND " +
            "count=? AND deleted=0 AND sub_id=?";
    /**
     * Where condition to check 3GPP2 format.
     */
    public static final String SQL_3GPP2_SMS = " AND (destination_port & "
            + InboundSmsTracker.DEST_PORT_FLAG_3GPP2 + "="
            + InboundSmsTracker.DEST_PORT_FLAG_3GPP2 + ")";

    /**
     * Where condition to check 3GPP format.
     */
    public static final String SQL_3GPP_SMS = " AND (destination_port & "
            + InboundSmsTracker.DEST_PORT_FLAG_3GPP + "="
            + InboundSmsTracker.DEST_PORT_FLAG_3GPP + ")";

    public static PplSmsFilterExtension sPplSmsFilter = null;

    // Prevent instantiation.
    private MtkSmsCommonUtil() {}

    /**
     * Indicates isPrimary for ETWS..
     * @hide pending API council approval
     */
    public static final String IS_EMERGENCY_CB_PRIMARY = "isPrimary";

    private static final boolean IS_PRIVACY_PROTECTION_LOCK_SUPPORT =
            SystemProperties.get("ro.vendor.mtk_privacy_protection_lock").equals("1");

    private static final boolean IS_WAPPUSH_SUPPORT =
            SystemProperties.get("ro.vendor.mtk_wappush_support").equals("1");

    public static boolean isPrivacyLockSupport() {
        return IS_PRIVACY_PROTECTION_LOCK_SUPPORT;
    }

    public static boolean isWapPushSupport() {
        return IS_WAPPUSH_SUPPORT;
    }

    /**
     * Filter out the MO sms by phone privacy lock.
     * For mobile manager service, the native apk needs to send a special sms to server and
     * doesn't want to show to end user. But sms frameworks will help to write to database if
     * app is not default sms application.
     * Therefore, sms framework need to filter out this kind of sms and not showing to end user.
     *
     * @param context the application context
     * @param tracker the SmsTracker
     */
    public static void filterOutByPpl(Context context, SmsTracker tracker) {
        if (!MtkSmsCommonUtil.isPrivacyLockSupport()) {
            return;
        }
        // Create the instance for phone privacy lock
        if (sPplSmsFilter == null) {
            sPplSmsFilter = new PplSmsFilterExtension(context);
        }

        boolean pplResult = false;

        // Start to check phone privacy check if it does not need to write to database
        if (ENG) {
            Rlog.d(TAG, "[PPL] Phone privacy check start");
        }

        Bundle pplData = new Bundle();
        pplData.putString(sPplSmsFilter.KEY_MSG_CONTENT, tracker.mFullMessageText);
        pplData.putString(sPplSmsFilter.KEY_DST_ADDR, tracker.mDestAddress);
        pplData.putString(sPplSmsFilter.KEY_FORMAT, tracker.mFormat);
        pplData.putInt(sPplSmsFilter.KEY_SUB_ID, tracker.mSubId);
        pplData.putInt(sPplSmsFilter.KEY_SMS_TYPE, 1);

        pplResult = sPplSmsFilter.pplFilter(pplData);
        if (pplResult) {
            tracker.mPersistMessage = false;
        }

        if (ENG) {
            Rlog.d(TAG, "[PPL] Phone privacy check end, Need to filter(result) = "
                    + pplResult);
        }
    }

    /**
     * Phone Privacy Lock check if this MT sms has permission to dispatch.
     *
     * @param pdus the PDU array
     * @param format the SMS format
     * @param context the application context
     * @param subId the subscription ID
     * @return the permission check result
     */
    public static int phonePrivacyLockCheck(
            byte[][] pdus, String format, Context context, int subId) {
        int checkResult = PackageManager.PERMISSION_GRANTED;

        if (MtkSmsCommonUtil.isPrivacyLockSupport()) {
            /* CTA-level3 for phone privacy lock */
            if (checkResult == PackageManager.PERMISSION_GRANTED) {
                if (sPplSmsFilter == null) {
                    sPplSmsFilter = new PplSmsFilterExtension(context);
                }
                Bundle pplData = new Bundle();

                pplData.putSerializable(sPplSmsFilter.KEY_PDUS, pdus);
                pplData.putString(sPplSmsFilter.KEY_FORMAT, format);
                pplData.putInt(sPplSmsFilter.KEY_SUB_ID, subId);
                pplData.putInt(sPplSmsFilter.KEY_SMS_TYPE, 0);

                boolean pplResult = false;
                pplResult = sPplSmsFilter.pplFilter(pplData);
                if (ENG) {
                    Rlog.d(TAG, "[Ppl] Phone privacy check end, Need to filter(result) = "
                            + pplResult);
                }
                if (pplResult == true) {
                    checkResult = PackageManager.PERMISSION_DENIED;
                }
            }
        }

        return checkResult;
    }
}
