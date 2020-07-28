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

import static com.android.internal.telephony.IccSmsInterfaceManager.SMS_MESSAGE_PERIOD_NOT_SPECIFIED;
import static com.android.internal.telephony.IccSmsInterfaceManager.SMS_MESSAGE_PRIORITY_NOT_SPECIFIED;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Build;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Intents;
import android.telephony.Rlog;
import android.telephony.SmsManager;
import android.util.Pair;

import static android.telephony.SmsManager.RESULT_ERROR_GENERIC_FAILURE;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.SmsStorageMonitor;
import com.android.internal.telephony.SmsUsageMonitor;
import com.android.internal.telephony.SmsDispatchersController;
import com.android.internal.telephony.SMSDispatcher;
import com.android.internal.telephony.SmsRawData;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.cdma.CdmaInboundSmsHandler;
import com.android.internal.telephony.cdma.CdmaSMSDispatcher;
import com.android.internal.telephony.gsm.GsmInboundSmsHandler;
import com.android.internal.telephony.gsm.GsmSMSDispatcher;
import com.mediatek.internal.telephony.MtkImsSmsDispatcher;
import com.mediatek.internal.telephony.cdma.MtkCdmaSMSDispatcher;
import com.mediatek.internal.telephony.gsm.MtkGsmSMSDispatcher;

import java.util.ArrayList;
import java.util.HashMap;

import android.os.SystemProperties;
import android.app.ActivityManager;
import android.provider.Telephony;

import android.telephony.SubscriptionManager;

import mediatek.telephony.MtkTelephony;

import java.util.Iterator;
import java.util.List;

/**
 *
 */
public class MtkSmsDispatchersController extends SmsDispatchersController {
    private static final String TAG = "MtkSmsDispatchersController";

    /**
     * Hold the wake lock for 5 seconds, which should be enough time for
     * any receiver(s) to grab its own wake lock.(SMS ready intent)
     */
    private static final int WAKE_LOCK_TIMEOUT = 500;
    /** Wake lock to ensure device stays awake while dispatching the SMS ready intent. */
    private PowerManager.WakeLock mWakeLock;

    private boolean mSmsReady = false;

    /** SMS subsystem in the modem is ready. */
    static final protected int EVENT_SMS_READY = 0;

    private static final boolean ENG = "eng".equals(Build.TYPE);

    public static final String SELECT_BY_REFERENCE = "address=? AND reference_number=? AND " +
            "count=? AND deleted=0 AND sub_id=?";

    /** Check pending SmsTracker format */
    private static final int FORMAT_NOT_MATCH = 0;
    private static final int FORMAT_IMS = 1;
    private static final int FORMAT_CS_GSM = 2;
    private static final int FORMAT_CS_CDMA = 3;

    public MtkSmsDispatchersController(Phone phone, SmsStorageMonitor storageMonitor,
            SmsUsageMonitor usageMonitor) {
        super(phone, storageMonitor, usageMonitor);
        // Register EVENT_SMS_READY
        createWakelock();
        MtkRIL ci = (MtkRIL)mCi;
        ci.registerForSmsReady(this, EVENT_SMS_READY, null);
        Rlog.d(TAG, "MtkSmsDispatchersController created");
    }

   /**
     * Send a data based SMS to a specific application port.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *  the current default SMSC
     * @param destPort the port to deliver the message to
     * @param originalPort the port to deliver the message from
     * @param data the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is successfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK<code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     */
    protected void sendData(String callingPackage, String destAddr, String scAddr, int destPort,
             int originalPort, byte[] data, PendingIntent sentIntent,
             PendingIntent deliveryIntent) {
        if (mImsSmsDispatcher.isAvailable()) {
            ((MtkImsSmsDispatcher) mImsSmsDispatcher).sendData(callingPackage, destAddr, scAddr,
                    destPort, originalPort, data, sentIntent, deliveryIntent);
        } else if (isCdmaMo()) {
            ((MtkCdmaSMSDispatcher) mCdmaDispatcher).sendData(callingPackage, destAddr, scAddr,
                    destPort, originalPort, data, sentIntent, deliveryIntent);
        } else {
            ((MtkGsmSMSDispatcher)mGsmDispatcher).sendData(callingPackage, destAddr, scAddr,
                    destPort, originalPort, data, sentIntent, deliveryIntent);
        }
    }

   /**
     * Send a multi-part data based SMS.
     *
     * @param destinationAddress the address to send the message to
     * @param scAddress is the service center address or null to use
     *   the current default SMSC
     * @param data an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param destPort the port to deliver the message to
     * @param data an array of data messages in order,
     *   comprise the original message
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     *   The result code will be <code>Activity.RESULT_OK<code> for success,
     *   or one of these errors:
     *   <code>RESULT_ERROR_GENERIC_FAILURE</code>
     *   <code>RESULT_ERROR_RADIO_OFF</code>
     *   <code>RESULT_ERROR_NULL_PDU</code>.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     */
    protected void sendMultipartData(
            String callingPackage, String destAddr, String scAddr, int destPort,
            ArrayList<SmsRawData> data, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents) {
        if (mImsSmsDispatcher.isAvailable()) {
            ((MtkImsSmsDispatcher) mImsSmsDispatcher).sendMultipartData(callingPackage,
                    destAddr, scAddr, destPort, data, sentIntents, deliveryIntents);
        } else if (isCdmaMo()) {
        } else {
            ((MtkGsmSMSDispatcher)mGsmDispatcher).sendMultipartData(callingPackage,
                    destAddr, scAddr, destPort, data, sentIntents, deliveryIntents);
        }
    }

    /**
     * Copy a text SMS to the ICC.
     *
     * @param scAddress Service center address
     * @param address   Destination address or original address
     * @param text      List of message text
     * @param status    message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *                  STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param timestamp Timestamp when service center receive the message
     * @return success or not
     *
     */
    public int copyTextMessageToIccCard(String scAddress, String address,
            List<String> text, int status, long timestamp) {
        if (mPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            return ((MtkCdmaSMSDispatcher) mCdmaDispatcher).copyTextMessageToIccCard(scAddress,
                    address, text, status, timestamp);
        } else {
            return ((MtkGsmSMSDispatcher)mGsmDispatcher).copyTextMessageToIccCard(
                    scAddress, address, text, status, timestamp);
        }
    }

    /**
     * Send an SMS with specified encoding type.
     *
     * @param destAddr the address to send the message to
     * @param scAddr the SMSC to send the message through, or NULL for the
     *  default SMSC
     * @param text the body of the message to send
     * @param encodingType the encoding type of content of message(GSM 7-bit, Unicode or Automatic)
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is sucessfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK<code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     * @param messageUri optional URI of the message if it is already stored in the system
     * @param callingPkg the calling package name
     * @param persistMessage whether to save the sent message into SMS DB for a
     *   non-default SMS app.
     */
    protected void sendTextWithEncodingType(String destAddr, String scAddr,
            String text, int encodingType, PendingIntent sentIntent,
            PendingIntent deliveryIntent, Uri messageUri, String callingPkg,
            boolean persistMessage, int priority, boolean expectMore, int validityPeriod) {
        if (mImsSmsDispatcher.isAvailable()) {
            ((MtkImsSmsDispatcher) mImsSmsDispatcher).sendTextWithEncodingType(destAddr, scAddr,
                    text, encodingType, sentIntent, deliveryIntent, messageUri, callingPkg,
                    persistMessage, priority, expectMore, validityPeriod);
        } else if (isCdmaMo()) {
            ((MtkCdmaSMSDispatcher) mCdmaDispatcher).sendTextWithEncodingType(destAddr,
                    scAddr, text, encodingType, sentIntent, deliveryIntent, messageUri, callingPkg,
                    persistMessage, priority, expectMore, validityPeriod);
        } else {
            ((MtkGsmSMSDispatcher)mGsmDispatcher).sendTextWithEncodingType(destAddr, scAddr, text,
                    encodingType, sentIntent, deliveryIntent, messageUri, callingPkg,
                    persistMessage, priority, expectMore, validityPeriod);
        }
    }

    /**
     * Send a multi-part text based SMS with specified encoding type.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *   the current default SMSC
     * @param parts an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param encodingType the encoding type of content of message(GSM 7-bit, Unicode or Automatic)
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     *   The result code will be <code>Activity.RESULT_OK<code> for success,
     *   or one of these errors:
     *   <code>RESULT_ERROR_GENERIC_FAILURE</code>
     *   <code>RESULT_ERROR_RADIO_OFF</code>
     *   <code>RESULT_ERROR_NULL_PDU</code>.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     * @param messageUri optional URI of the message if it is already stored in the system
     * @param callingPkg the calling package name
     * @param persistMessage whether to save the sent message into SMS DB for a
     *   non-default SMS app.
     */
    protected void sendMultipartTextWithEncodingType(String destAddr, String scAddr,
            ArrayList<String> parts, int encodingType, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents, Uri messageUri, String callingPkg,
            boolean persistMessage, int priority, boolean expectMore, int validityPeriod) {
        if (mImsSmsDispatcher.isAvailable()) {
            ((MtkImsSmsDispatcher) mImsSmsDispatcher).sendMultipartTextWithEncodingType(
                    destAddr, scAddr, parts, encodingType, sentIntents, deliveryIntents,
                    messageUri, callingPkg, persistMessage, priority, expectMore, validityPeriod);
        } else if (isCdmaMo()) {
            ((MtkCdmaSMSDispatcher) mCdmaDispatcher).sendMultipartTextWithEncodingType(destAddr,
                    scAddr, parts, encodingType, sentIntents, deliveryIntents, messageUri,
                    callingPkg, persistMessage, priority, expectMore, validityPeriod);
        } else {
            ((MtkGsmSMSDispatcher)mGsmDispatcher).sendMultipartTextWithEncodingType(
                    destAddr, scAddr, parts, encodingType, sentIntents, deliveryIntents,
                    messageUri, callingPkg, persistMessage, priority, expectMore, validityPeriod);
        }
    }

    /**
     * Called when SimSmsInterfaceManager update SIM card fail due to SIM_FULL.
     */
    protected void handleIccFull() {
        // broadcast SIM_FULL intent
        if (isCdmaMo()) {
        } else  {
            ((MtkGsmSMSDispatcher)mGsmDispatcher).handleIccFull();
        }
    }

    /**
     * Set the memory storage status of the SMS
     * This function is used for FTA test only
     *
     * @param status false for storage full, true for storage available
     *
     */
    protected void setSmsMemoryStatus(boolean status) {
        if (isCdmaMo()) {
        } else {
            ((MtkGsmSMSDispatcher)mGsmDispatcher).setSmsMemoryStatus(status);
        }
    }

    public boolean isSmsReady() {
        return mSmsReady;
    }

    /**
     * @param msg the message to handle
     */
    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
        case EVENT_SMS_READY: {
            Rlog.d(TAG, "SMS is ready, Phone: " + mPhone.getPhoneId());
            mSmsReady = true;

            notifySmsReady(mSmsReady);
            break;
        }
        default:
            super.handleMessage(msg);
        }
    }

    /**
     * create wake lock for sms ready intent.
     */
    private void createWakelock() {
        PowerManager pm = (PowerManager) mPhone.getContext().getSystemService(
                Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "SmsCommonEventHelp");
        mWakeLock.setReferenceCounted(true);
    }

    private void notifySmsReady(boolean isReady) {
        // broadcast SMS_STATE_CHANGED_ACTION intent
        Intent intent = new Intent(MtkTelephony.SMS_STATE_CHANGED_ACTION);
        intent.putExtra("ready", isReady);
        SubscriptionManager.putPhoneIdAndSubIdExtra(intent, mPhone.getPhoneId());
        // Allow registered broadcast receivers to get this intent even
        // when they are in the background.
        intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
        intent.setComponent(null);
        mWakeLock.acquire(WAKE_LOCK_TIMEOUT);
        mPhone.getContext().sendBroadcast(intent);
    }

    int isFormatMatch(SMSDispatcher.SmsTracker tracker, Phone phone) {
        if (ENG) {
            Rlog.d(TAG, "isFormatMatch, isIms " + isIms() + ", ims sms format "
                    + getImsSmsFormat() + ", tracker format " + tracker.mFormat
                    + ", Phone type " + phone.getPhoneType());
        }
        // Check IMS status first
        if (mImsSmsDispatcher.isAvailable()
                && tracker.mFormat.equals(mImsSmsDispatcher.getFormat())) {
            return FORMAT_IMS;
        }
        // Then check CS
        if (tracker.mFormat.equals(SmsConstants.FORMAT_3GPP2) &&
            phone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            return FORMAT_CS_CDMA;
        }
        if (tracker.mFormat.equals(SmsConstants.FORMAT_3GPP) &&
            phone.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM) {
            return FORMAT_CS_GSM;
        }
        return FORMAT_NOT_MATCH;
    }

    public void addToGsmDeliverPendingList(SMSDispatcher.SmsTracker tracker) {
        ((MtkGsmSMSDispatcher) mGsmDispatcher).addToGsmDeliverPendingList(tracker);
    }
}
