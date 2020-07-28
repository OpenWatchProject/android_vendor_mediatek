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
package com.mediatek.internal.telephony.cdma;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.SQLException;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Message;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Telephony.Sms.Intents;
import android.telephony.Rlog;
import android.telephony.SmsCbMessage;

import com.android.internal.telephony.BlockChecker;
import com.android.internal.telephony.InboundSmsHandler;
import com.android.internal.telephony.InboundSmsTracker;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.ProxyController;
import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.SmsHeader;
import com.android.internal.telephony.SmsMessageBase;
import com.android.internal.telephony.SmsStorageMonitor;
import com.android.internal.telephony.cdma.CdmaInboundSmsHandler;
import com.android.internal.telephony.cdma.CdmaSMSDispatcher;
import com.android.internal.telephony.cdma.SmsMessage;
import com.android.internal.telephony.cdma.sms.SmsEnvelope;
import com.android.internal.util.HexDump;

import com.mediatek.internal.telephony.MtkInboundSmsTracker;
import com.mediatek.internal.telephony.MtkProxyController;
import com.mediatek.internal.telephony.MtkWapPushOverSms;
import com.mediatek.internal.telephony.util.MtkSmsCommonUtil;

import java.io.ByteArrayOutputStream;
import java.util.Arrays;
import java.util.List;

import mediatek.telephony.MtkTelephony;

/**
 * Sub class to enhance AOSP class CdmaInboundSmsHandler.
 */
public class MtkCdmaInboundSmsHandler extends CdmaInboundSmsHandler {
    private String mTag = "MtkCdmaInboundSmsHandler";
    private static final boolean VDBG = false;
    private static final boolean ENG = "eng".equals(Build.TYPE);
    private static final int TELESERVICE_REG_SMS_CT = 0xFDED;
    private static final int WAKE_LOCK_TIMEOUT = 500;
    private static final int RESULT_SMS_REJECT_BY_PPL = 0;
    private static final int RESULT_SMS_ACCEPT_BY_PPL = 1;

    /**
     * The constructor to contruct the Mediatek's CdmaInboundSmsHandler.
     *
     * @param context the context of the phone process
     * @param storageMonitor the object the the SmsStorageMonitor
     * @param phone the object the Phone
     * @param smsDispatcher the object of the CdmaSMSDispatcher
     */
    public MtkCdmaInboundSmsHandler(Context context, SmsStorageMonitor storageMonitor,
            Phone phone, CdmaSMSDispatcher smsDispatcher) {
        super(context, storageMonitor, phone, smsDispatcher);
        mTag = "MtkCdmaInboundSmsHandler-" + phone.getPhoneId();
    }

    @Override
    public void dispatchIntent(Intent intent, String permission, int appOp,
        Bundle opts, BroadcastReceiver resultReceiver, UserHandle user) {
        intent.putExtra("rTime", System.currentTimeMillis());
        super.dispatchIntent(intent, permission, appOp, opts, resultReceiver, user);
    }

    @Override
    protected int addTrackerToRawTableAndSendMessage(InboundSmsTracker tracker, boolean deDup) {
        MtkInboundSmsTracker t = (MtkInboundSmsTracker) tracker;
        t.setSubId(mPhone.getSubId());
        return super.addTrackerToRawTableAndSendMessage(tracker, deDup);
    }

    @Override
    protected void deleteFromRawTable(String deleteWhere, String[] deleteWhereArgs,
            int deleteType) {
        Uri uri = deleteType == DELETE_PERMANENTLY ? sRawUriPermanentDelete : sRawUri;
        if (deleteWhere == null && deleteWhereArgs == null) {
            //the contentresolver design is to delete all in table in this case.
            //in this case, means we don't want to delete anything, so return to avoid it
            loge("No rows need be deleted from raw table!");
            return;
        }
        super.deleteFromRawTable(deleteWhere, deleteWhereArgs, deleteType);
    }

    @Override
    protected int dispatchMessageRadioSpecific(SmsMessageBase smsb) {
        SmsMessage sms = (SmsMessage) smsb;
        sms = MtkSmsMessage.newMtkSmsMessage(sms);
        int ret = super.dispatchMessageRadioSpecific(sms);
        if (ret == Intents.RESULT_SMS_UNSUPPORTED) {
            int teleService = sms.getTeleService();
            if (teleService == TELESERVICE_REG_SMS_CT && sms.getPdu() != null) {
                handleAutoRegMessage(sms.getPdu());
                return Intents.RESULT_SMS_HANDLED;
            }
        }
        return ret;
    }

    private void handleAutoRegMessage(byte[] pdu) {
        ((MtkProxyController) ProxyController.getInstance()).
                getDeviceRegisterController().handleAutoRegMessage(pdu);
    }

    /**
     * Phone Privacy Lock intent handler.
     * To handle the intent that send from sms finite state machine.
     */
    private int checkPplPermission(byte[][] pdus, String format) {
        int result = RESULT_SMS_ACCEPT_BY_PPL;
        if ((is3gpp2() && (format.compareTo(SmsConstants.FORMAT_3GPP2) == 0)) ||
            (!is3gpp2() && (format.compareTo(SmsConstants.FORMAT_3GPP) == 0))) {
            if (MtkSmsCommonUtil.phonePrivacyLockCheck(pdus, format, mContext, mPhone.getSubId())
                    != PackageManager.PERMISSION_GRANTED) {
                result = RESULT_SMS_REJECT_BY_PPL;
            }
        }
        return result;
    }

    @Override
    protected String[] onModifyQueryWhereArgs(String[] whereArgs) {
        String subId = Integer.toString(mPhone.getSubId());
        return new String[]{whereArgs[0], whereArgs[1], whereArgs[2], subId};
    }

    /*
     * A hook function to stop the flow of processing message parts.
     *
     *  @param pdus the SMS pdu array
     *  @param format the SMS format
     *  @param tracker the InboundSmsTracker object
     *
     *  @return true if stop processing
     */
    @Override
    protected boolean onCheckIfStopProcessMessagePart(byte[][] pdus, String format,
            InboundSmsTracker tracker) {
        if (checkPplPermission(pdus, format) != RESULT_SMS_ACCEPT_BY_PPL) {
            log("The message was blocked by Ppl! don't prompt to user");
            deleteFromRawTable(tracker.getDeleteWhere(), tracker.getDeleteWhereArgs(),
                    DELETE_PERMANENTLY);
            return true;
        }
        return false;
    }

    /**
     * Log with debug level.
     * @param s the string to log
     */
    @Override
    protected void log(String s) {
        Rlog.d(mTag, s);
    }

    /**
     * Log with error level.
     * @param s the string to log
     */
    @Override
    protected void loge(String s) {
        Rlog.e(mTag, s);
    }

    /**
     * Log with error level.
     * @param s the string to log
     * @param e is a Throwable which logs additional information.
     */
    @Override
    protected void loge(String s, Throwable e) {
        Rlog.e(mTag, s, e);
    }
}

