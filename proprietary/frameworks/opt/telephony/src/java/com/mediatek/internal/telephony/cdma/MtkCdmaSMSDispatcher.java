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

import android.app.ActivityManager;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Build;
import android.os.Bundle;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.UserHandle;
import android.telephony.CarrierConfigManager;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SmsManager;
import android.telephony.TelephonyManager;

import static android.telephony.SmsManager.RESULT_ERROR_GENERIC_FAILURE;
import static android.telephony.SmsManager.RESULT_ERROR_NULL_PDU;
import static android.telephony.SmsManager.STATUS_ON_ICC_READ;
import static android.telephony.SmsManager.STATUS_ON_ICC_SENT;
import static android.telephony.SmsManager.STATUS_ON_ICC_UNREAD;
import static android.telephony.SmsManager.STATUS_ON_ICC_UNSENT;


import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.IccCardConstants.State;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.TelephonyComponentFactory;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.SMSDispatcher.SmsTracker;
import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.SmsDispatchersController;
import com.android.internal.telephony.SmsHeader;
import com.android.internal.telephony.SmsMessageBase;
import com.android.internal.telephony.SmsResponse;
import com.android.internal.telephony.SmsUsageMonitor;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.cdma.CdmaSMSDispatcher;
import com.android.internal.telephony.cdma.SmsMessage;
import com.android.internal.telephony.cdma.sms.UserData;
import com.android.internal.telephony.uicc.IccUtils;


import com.mediatek.internal.telephony.MtkPhoneNumberUtils;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.util.MtkSMSDispatcherUtil;
import com.mediatek.internal.telephony.util.MtkSmsCommonUtil;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Sub class to enhance the AOSP class CdmaSMSDispatcher.
 */
public class MtkCdmaSMSDispatcher extends CdmaSMSDispatcher {
    private static final String TAG = "MtkCdmaSMSDispatcher";
    private static final boolean VDBG = false;
    protected Object mLock = new Object();
    private boolean mCopied = false;
    protected boolean mSuccess = true;
    private static final boolean ENG = "eng".equals(Build.TYPE);
    private static final int RESULT_ERROR_SUCCESS = 0;
    private static final int EVENT_COPY_TEXT_MESSAGE_DONE = 106;
    private static final int RESULT_ERROR_RUIM_PLUG_OUT = 107;
    private static final int WAKE_LOCK_TIMEOUT = 500;
    private ThreadLocal<Integer> mOriginalPort = new ThreadLocal<Integer>() {
        @Override
        protected Integer initialValue() {
            return -1;
        }
    };

    private ThreadLocal<Integer> mEncodingType = new ThreadLocal<Integer>() {
        @Override
        protected Integer initialValue() {
            return android.telephony.SmsMessage.ENCODING_UNKNOWN;
        }
    };

    /**
     * Create a new Mtk Cdma SMS dispatcher.
     * @param phone the Phone to use
     * @param smsDispatchersController the SmsDispatchersController to use
     */
    public MtkCdmaSMSDispatcher(Phone phone,
            SmsDispatchersController smsDispatchersController) {
        super(phone, smsDispatchersController);
        Rlog.d(TAG, "MtkCdmaSMSDispatcher created");
    }

    /** {@inheritDoc} */
    @Override
    public void sendSms(SmsTracker tracker) {
        int ss = mPhone.getServiceState().getState();
        // if sms over IMS is not supported on data and voice is not available...
        if (!isIms() && ss != ServiceState.STATE_IN_SERVICE) {
            // MTK-START
            //tracker.onFailed(mContext, getNotInServiceError(ss), 0/*errorCode*/);
            // When CDMA card is plug out, we will power off MD3 to save power.
            // As a result, the service state will be STATE_POWER_OFF.
            // However, when GSM card is plug out, we will not power off MD1.
            // In order to keep the same behavior as the GSM.
            // We need to check the card status here.
            if (isSimAbsent()) {
                tracker.onFailed(mContext, RESULT_ERROR_GENERIC_FAILURE, 0/*errorCode*/);
            } else {
                tracker.onFailed(mContext, getNotInServiceError(ss), 0/*errorCode*/);
            }
            // MTK-END
            return;
        }
        super.sendSms(tracker);
    }

    @Override
    protected SmsTracker getSmsTracker(String callingPackage, HashMap<String, Object> data,
            PendingIntent sentIntent, PendingIntent deliveryIntent, String format,
            AtomicInteger unsentPartCount, AtomicBoolean anyPartFailed, Uri messageUri,
            SmsHeader smsHeader, boolean expectMore, String fullMessageText, boolean isText,
            boolean persistMessage, int priority, int validityPeriod, boolean isForVvm) {
        SmsTracker tracker = super.getSmsTracker(callingPackage, data, sentIntent, deliveryIntent,
                format, unsentPartCount, anyPartFailed, messageUri, smsHeader, expectMore,
                fullMessageText, isText, persistMessage, priority, validityPeriod, isForVvm);
        MtkSmsCommonUtil.filterOutByPpl(mContext, tracker);
        return tracker;
    }

    @Override
    protected String getPackageNameViaProcessId(String[] packageNames) {
        return MtkSMSDispatcherUtil.getPackageNameViaProcessId(mContext, packageNames);
    }

    // MTK-START
    /**
     * Send a data based SMS to a specific application port with original port.
     *
     * @param callingPackage the package name of the calling app
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *  the current default SMSC
     * @param destPort the port to deliver the message to
     * @param originalPort the original port the message from
     * @param data the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is successfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK</code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  <code>RESULT_ERROR_NO_SERVICE</code><br>.
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
    public void sendData(String callingPackage, String destAddr, String scAddr, int destPort,
            int originalPort, byte[] data, PendingIntent sentIntent,
            PendingIntent deliveryIntent) {
        mOriginalPort.set(originalPort);
        sendData(callingPackage, destAddr, scAddr, destPort, data,
                sentIntent, deliveryIntent, false);
        mOriginalPort.remove();
    }

    @Override
    protected SmsMessageBase.SubmitPduBase onSendData(
            String destAddr, String scAddr, int destPort,
            byte[] data, PendingIntent sentIntent, PendingIntent deliveryIntent) {
        if (mOriginalPort.get() == -1) {
            return super.onSendData(destAddr, scAddr, destPort, data, sentIntent, deliveryIntent);
        } else {
            return MtkSmsMessage.getSubmitPdu(
                    scAddr, destAddr, destPort, mOriginalPort.get(), data,
                    (deliveryIntent != null));
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
    public int copyTextMessageToIccCard(String scAddress, String address, List<String> text,
                    int status, long timestamp) {
        mSuccess = true;

        int msgCount = text.size();
        Rlog.d(TAG, "copyTextMessageToIccCard status = " + status + ", msgCount = " + msgCount);
        if ((status != STATUS_ON_ICC_READ &&
            status != STATUS_ON_ICC_UNREAD &&
            status != STATUS_ON_ICC_SENT &&
            status != STATUS_ON_ICC_UNSENT) || (msgCount < 1)) {
            return RESULT_ERROR_GENERIC_FAILURE;
        }
        for (int i = 0; i < msgCount; ++i) {
            if (mSuccess == false) {
                return RESULT_ERROR_GENERIC_FAILURE;
            }
            SmsMessage.SubmitPdu pdu = MtkSmsMessage.createEfPdu(address, text.get(i), timestamp);
            if (pdu != null) {
                mCi.writeSmsToRuim(status, IccUtils.bytesToHexString(pdu.encodedMessage),
                        obtainMessage(EVENT_COPY_TEXT_MESSAGE_DONE));
            } else {
                return RESULT_ERROR_GENERIC_FAILURE;
            }
            synchronized (mLock) {
                mCopied = false;
                try {
                    while (!mCopied) {
                        mLock.wait();
                    }
                } catch (InterruptedException e) {
                    return RESULT_ERROR_GENERIC_FAILURE;
                }
            }
        }
        return mSuccess ? RESULT_ERROR_SUCCESS : RESULT_ERROR_GENERIC_FAILURE;
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
     *  The result code will be <code>Activity.RESULT_OK</code> for success,
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
     * @param priority Priority level of the message
     *  Refer specification See 3GPP2 C.S0015-B, v2.0, table 4.5.9-1
     *  ---------------------------------
     *  PRIORITY      | Level of Priority
     *  ---------------------------------
     *      '00'      |     Normal
     *      '01'      |     Interactive
     *      '10'      |     Urgent
     *      '11'      |     Emergency
     *  ----------------------------------
     *  Any Other values included Negative considered as Invalid Priority Indicator of the message.
     * @param expectMore is a boolean to indicate the sending messages through same link or not.
     * @param validityPeriod Validity Period of the message in mins.
     *  Refer specification 3GPP TS 23.040 V6.8.1 section 9.2.3.12.1.
     *  Validity Period(Minimum) -> 5 mins
     *  Validity Period(Maximum) -> 635040 mins(i.e.63 weeks).
     *  Any Other values included Negative considered as Invalid Validity Period of the message.
     */
    public void sendTextWithEncodingType(String destAddr, String scAddr, String text,
            int encodingType, PendingIntent sentIntent, PendingIntent deliveryIntent,
            Uri messageUri, String callingPkg, boolean persistMessage, int priority,
            boolean expectMore, int validityPeriod) {

        Rlog.d(TAG, "sendTextWithEncodingType encoding = " + encodingType);
        mEncodingType.set(encodingType);
        sendText(destAddr, scAddr, text, sentIntent, deliveryIntent, messageUri, callingPkg,
                persistMessage, priority, expectMore, validityPeriod, false);
        mEncodingType.remove();
    }

    @Override
    protected SmsMessageBase.SubmitPduBase onSendText(
            String destAddr, String scAddr, String text,
            PendingIntent sentIntent, PendingIntent deliveryIntent, Uri messageUri,
            String callingPkg, boolean persistMessage, int priority,
            boolean expectMore, int validityPeriod) {
         int encodingType = mEncodingType.get();
         if ((encodingType == android.telephony.SmsMessage.ENCODING_7BIT)
                 || (encodingType == android.telephony.SmsMessage.ENCODING_8BIT)
                 || (encodingType == android.telephony.SmsMessage.ENCODING_16BIT)) {
             return MtkSmsMessage.getSubmitPdu(scAddr, destAddr, text,
                (deliveryIntent != null), null, encodingType, validityPeriod, priority, true);
         }
         return super.onSendText(destAddr, scAddr, text,
                 sentIntent, deliveryIntent, messageUri, callingPkg, persistMessage, priority,
                 expectMore, validityPeriod);
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
     *   The result code will be <code>Activity.RESULT_OK</code> for success,
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
     * @param priority Priority level of the message
     *  Refer specification See 3GPP2 C.S0015-B, v2.0, table 4.5.9-1
     *  ---------------------------------
     *  PRIORITY      | Level of Priority
     *  ---------------------------------
     *      '00'      |     Normal
     *      '01'      |     Interactive
     *      '10'      |     Urgent
     *      '11'      |     Emergency
     *  ----------------------------------
     *  Any Other values included Negative considered as Invalid Priority Indicator of the message.
     * @param expectMore is a boolean to indicate the sending messages through same link or not.
     * @param validityPeriod Validity Period of the message in mins.
     *  Refer specification 3GPP TS 23.040 V6.8.1 section 9.2.3.12.1.
     *  Validity Period(Minimum) -> 5 mins
     *  Validity Period(Maximum) -> 635040 mins(i.e.63 weeks).
     *  Any Other values included Negative considered as Invalid Validity Period of the message.
     */
    public void sendMultipartTextWithEncodingType(String destAddr, String scAddr,
            ArrayList<String> parts, int encodingType, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents, Uri messageUri, String callingPkg,
            boolean persistMessage, int priority, boolean expectMore, int validityPeriod) {
        Rlog.d(TAG, "sendMultipartTextWithEncodingType encoding = " + encodingType);
        mEncodingType.set(encodingType);
        sendMultipartText(destAddr, scAddr, parts, sentIntents, deliveryIntents, messageUri,
                callingPkg, persistMessage, priority, expectMore, validityPeriod);
        mEncodingType.remove();
    }


    @Override
    public void sendMultipartText(String destAddr, String scAddr,
            ArrayList<String> parts, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents, Uri messageUri, String callingPkg,
            boolean persistMessage, int priority, boolean expectMore, int validityPeriod) {
        final String fullMessageText = getMultipartMessageText(parts);
        int refNumber = getNextConcatenatedRef() & 0x00FF;
        int encoding = SmsConstants.ENCODING_UNKNOWN;
        int msgCount = parts.size();
        if (msgCount < 1) {
            triggerSentIntentForFailure(sentIntents);
            return;
        }

        TextEncodingDetails[] encodingForParts = new TextEncodingDetails[msgCount];
        encoding = onSendMultipartText(destAddr, scAddr, parts, sentIntents, deliveryIntents,
                messageUri, callingPkg, persistMessage, priority, expectMore, validityPeriod,
                encodingForParts);
        SmsTracker[] trackers = new SmsTracker[msgCount];

        // States to track at the message level (for all parts)
        final AtomicInteger unsentPartCount = new AtomicInteger(msgCount);
        final AtomicBoolean anyPartFailed = new AtomicBoolean(false);

        for (int i = 0; i < msgCount; i++) {
            SmsHeader.ConcatRef concatRef = new SmsHeader.ConcatRef();
            concatRef.refNumber = refNumber;
            concatRef.seqNumber = i + 1;  // 1-based sequence
            concatRef.msgCount = msgCount;
            // TODO: We currently set this to true since our messaging app will never
            // send more than 255 parts (it converts the message to MMS well before that).
            // However, we should support 3rd party messaging apps that might need 16-bit
            // references
            // Note:  It's not sufficient to just flip this bit to true; it will have
            // ripple effects (several calculations assume 8-bit ref).
            concatRef.isEightBits = true;
            TelephonyComponentFactory telephonyComponentFactory = TelephonyComponentFactory
                    .getInstance().inject(TelephonyComponentFactory.class.getName());
            SmsHeader smsHeader = telephonyComponentFactory.makeSmsHeader();
            smsHeader.concatRef = concatRef;

            // Set the national language tables for 3GPP 7-bit encoding, if enabled.
            if (encoding == SmsConstants.ENCODING_7BIT) {
                smsHeader.languageTable = encodingForParts[i].languageTable;
                smsHeader.languageShiftTable = encodingForParts[i].languageShiftTable;
            }

            PendingIntent sentIntent = null;
            if (sentIntents != null && sentIntents.size() > i) {
                sentIntent = sentIntents.get(i);
            }

            PendingIntent deliveryIntent = null;
            if (deliveryIntents != null && deliveryIntents.size() > i) {
                deliveryIntent = deliveryIntents.get(i);
            }

            trackers[i] =
                    getNewSubmitPduTracker(callingPkg, destAddr, scAddr, parts.get(i), smsHeader,
                            encoding, sentIntent, deliveryIntent, (i == (msgCount - 1)),
                            unsentPartCount, anyPartFailed, messageUri,
                            fullMessageText, priority, expectMore, validityPeriod);
            if (trackers[i] == null) {
                triggerSentIntentForFailure(sentIntents);
                return;
            }
        }

        String carrierPackage = getCarrierAppPackageName();
        if (carrierPackage != null) {
            Rlog.d(TAG, "Found carrier package.");
            MultipartSmsSender smsSender = new MultipartSmsSender(parts, trackers);
            smsSender.sendSmsByCarrierApp(carrierPackage,
                    new MultipartSmsSenderCallback(smsSender));
        } else {
            Rlog.v(TAG, "No carrier package.");
            for (SmsTracker tracker : trackers) {
                sendSubmitPdu(tracker);
            }
        }
    }

    @Override
    protected int onSendMultipartText(String destAddr, String scAddr,
                ArrayList<String> parts, ArrayList<PendingIntent> sentIntents,
                ArrayList<PendingIntent> deliveryIntents, Uri messageUri, String callingPkg,
                boolean persistMessage, int priority, boolean expectMore, int validityPeriod,
                TextEncodingDetails[] encodingForParts) {
        int encodingType = mEncodingType.get();
        if ((encodingType == android.telephony.SmsMessage.ENCODING_7BIT)
                || (encodingType == android.telephony.SmsMessage.ENCODING_8BIT)
                || (encodingType == android.telephony.SmsMessage.ENCODING_16BIT)) {
            int msgCount = parts.size();
            for (int i = 0; i < msgCount; i++) {
                TextEncodingDetails details =
                        MtkSmsMessage.calculateLength(parts.get(i), false, encodingType);
                details.codeUnitSize = encodingType;
                encodingForParts[i] = details;
            }
        } else {
            encodingType = super.onSendMultipartText(destAddr, scAddr,
                    parts, sentIntents, deliveryIntents, messageUri, callingPkg,
                    persistMessage, priority, expectMore, validityPeriod, encodingForParts);
        }
        return encodingType;
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case EVENT_COPY_TEXT_MESSAGE_DONE:
            {
                AsyncResult ar;
                ar = (AsyncResult) msg.obj;
                synchronized (mLock) {
                    mSuccess = (ar.exception == null);
                    mCopied = true;
                    mLock.notifyAll();
                }
                break;
            }

            default:
                super.handleMessage(msg);
        }
    }

    @Override
    protected void handleSendComplete(AsyncResult ar) {
        SmsTracker tracker = (SmsTracker) ar.userObj;
        if (ar.exception != null) {
            if (ar.result != null) {
                int errorCode = ((SmsResponse) ar.result).mErrorCode;
                if (errorCode == RESULT_ERROR_RUIM_PLUG_OUT) {
                    Rlog.d(TAG, "RUIM card is plug out");
                    tracker.onFailed(mContext, RESULT_ERROR_GENERIC_FAILURE, errorCode);
                    return;
                }
                int ss = mPhone.getServiceState().getState();
                if (!isIms() && (ss != ServiceState.STATE_IN_SERVICE) && isSimAbsent()) {
                    tracker.onFailed(mContext, RESULT_ERROR_GENERIC_FAILURE, errorCode);
                    return;
                }
            }
        }
        super.handleSendComplete(ar);
    }

    private boolean isSimAbsent() {
        IccCard card = PhoneFactory.getPhone(mPhone.getPhoneId()).getIccCard();
        State state;;
        if (card == null) {
            state = IccCardConstants.State.UNKNOWN;
        } else {
            state = card.getState();
        }
        boolean ret =  (state == IccCardConstants.State.ABSENT ||
                state == IccCardConstants.State.NOT_READY);
        Rlog.d(TAG, "isSimAbsent state = " + state + " ret=" + ret);
        return ret;
    }
}
