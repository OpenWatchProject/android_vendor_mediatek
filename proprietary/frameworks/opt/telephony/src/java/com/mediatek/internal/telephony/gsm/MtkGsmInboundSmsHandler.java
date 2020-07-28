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


package com.mediatek.internal.telephony.gsm;

import android.app.Activity;
import android.app.AppOpsManager;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.ContentUris;
import android.content.Intent;
import android.content.ComponentName;
import android.content.ContentValues;
import android.content.pm.PackageManager;
import android.os.Message;
import android.os.SystemProperties;
import android.os.PowerManager;
import android.os.UserManager;
import android.os.Build;
import android.os.Bundle;
import android.os.UserHandle;
import android.database.Cursor;
import android.database.SQLException;
import android.net.Uri;
import android.provider.Telephony.Sms.Intents;
import android.provider.Telephony;
import android.service.carrier.ICarrierMessagingService;
import android.service.carrier.CarrierMessagingService;

import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.SmsManager;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.InboundSmsHandler;
import com.android.internal.telephony.InboundSmsTracker;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.ProxyController;
import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.SmsMessageBase;
import com.android.internal.telephony.SmsStorageMonitor;
import com.android.internal.telephony.SmsHeader;
import com.android.internal.telephony.TelephonyComponentFactory;
import com.android.internal.telephony.BlockChecker;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SmsApplication;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UsimServiceTable;
import com.android.internal.telephony.gsm.GsmInboundSmsHandler;
import com.android.internal.telephony.gsm.SmsMessage;

import com.mediatek.internal.telephony.MtkInboundSmsTracker;
import com.mediatek.internal.telephony.MtkProxyController;
import com.mediatek.internal.telephony.MtkWapPushOverSms;
import com.mediatek.internal.telephony.MtkSmsDispatchersController;
import com.mediatek.internal.telephony.util.MtkSmsCommonUtil;
import com.mediatek.internal.telephony.OpTelephonyCustomizationFactoryBase;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;
import com.mediatek.internal.telephony.ppl.PplSmsFilterExtension;

import mediatek.telephony.MtkTelephony;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.android.internal.util.HexDump;


/**
 * This class broadcasts incoming SMS messages to interested apps after storing them in
 * the SmsProvider "raw" table and ACKing them to the SMSC. After each message has been
 */
public class MtkGsmInboundSmsHandler extends GsmInboundSmsHandler {
    private String mTag = "MtkGsmInboundSmsHandler";
    private static final int RESULT_SMS_REJECT_BY_PPL = 0;
    private static final int RESULT_SMS_ACCEPT_BY_PPL = 1;

    /**
     * Create a new GSM inbound SMS handler.
     */
    public MtkGsmInboundSmsHandler(Context context, SmsStorageMonitor storageMonitor,
            Phone phone) {
        super(context, storageMonitor, phone);
        mTag = "MtkGsmInboundSmsHandler-" + phone.getPhoneId();
        if (DBG) log("created InboundSmsHandler from MtkGsmInboundSmsHandler");
    }

    /**
     * Wait for state machine to enter startup state. We can't send any messages until then.
     */
    public static MtkGsmInboundSmsHandler makeInboundSmsHandler(Context context,
            SmsStorageMonitor storageMonitor, Phone phone) {
        MtkGsmInboundSmsHandler handler = new MtkGsmInboundSmsHandler(
                context, storageMonitor, phone);
        handler.start();
        return handler;
    }

    /**
     * Handle type zero, SMS-PP data download, and 3GPP/CPHS MWI type SMS. Normal SMS messages
     * are handled by {@link #dispatchNormalMessage} in parent class.
     *
     * @param smsb the SmsMessageBase object from the RIL
     * @return a result code from {@link android.provider.Telephony.Sms.Intents},
     *  or {@link Activity#RESULT_OK} for delayed acknowledgment to SMSC
     */
    @Override
    protected int dispatchMessageRadioSpecific(SmsMessageBase smsb) {
        SmsMessage sms = (SmsMessage) smsb;

        if (sms.getDisplayOriginatingAddress().equals("10659401")) {
            log("handleAutoRegMessage.");
            handleAutoRegMessage(sms.getPdu());
            return Intents.RESULT_SMS_HANDLED;
        }

        return super.dispatchMessageRadioSpecific(smsb);
    }

    private void handleAutoRegMessage(byte[] pdu) {
        ((MtkProxyController) ProxyController.getInstance()).getDeviceRegisterController().
                handleAutoRegMessage(mPhone.getSubId(), SmsConstants.FORMAT_3GPP, pdu);
    }

    @Override
    protected int addTrackerToRawTableAndSendMessage(InboundSmsTracker tracker, boolean deDup) {
        MtkInboundSmsTracker t = (MtkInboundSmsTracker) tracker;
        t.setSubId(mPhone.getSubId());
        return super.addTrackerToRawTableAndSendMessage(tracker, deDup);
    }

    @Override
    public void dispatchIntent(Intent intent, String permission, int appOp,
            Bundle opts, BroadcastReceiver resultReceiver, UserHandle user) {
        intent.putExtra("rTime", System.currentTimeMillis());
        super.dispatchIntent(intent, permission, appOp, opts, resultReceiver, user);
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

    @Override
    protected android.telephony.SmsMessage onCreateSmsMessage(byte[] pdu, String format) {
        return mediatek.telephony.MtkSmsMessage.createFromPdu(pdu, SmsConstants.FORMAT_3GPP);
    }

    @Override
    protected int onDispatchWapPdu(byte[][] smsPdus,
            byte[] pdu, BroadcastReceiver receiver, String address) {
        if (MtkSmsCommonUtil.isWapPushSupport()) {
            log("dispatch wap push pdu with addr & sc addr");
            Bundle bundle = new Bundle();
            mediatek.telephony.MtkSmsMessage sms =
                    mediatek.telephony.MtkSmsMessage.createFromPdu(
                            smsPdus[0], SmsConstants.FORMAT_3GPP);
            if (sms != null) {
                bundle.putString(MtkTelephony.WapPush.ADDR, sms.getOriginatingAddress());
                String sca = sms.getServiceCenterAddress();
                if (sca == null) {
                    /* null for app is not a item, it needs to transfer to empty string */
                    sca = "";
                }
                bundle.putString(MtkTelephony.WapPush.SERVICE_ADDR, sca);
            }
            return ((MtkWapPushOverSms) mWapPush).dispatchWapPdu(pdu,
                    receiver, this, bundle);
        } else {
            return super.onDispatchWapPdu(smsPdus, pdu, receiver, address);
        }
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
