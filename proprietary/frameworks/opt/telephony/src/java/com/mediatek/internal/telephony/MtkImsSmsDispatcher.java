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

import android.os.Build;
import android.os.AsyncResult;
import android.os.RemoteException;
import android.provider.Telephony.Sms.Intents;
import android.telephony.Rlog;
import android.telephony.ims.stub.ImsSmsImplBase;

import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;

import static com.android.internal.telephony.IccSmsInterfaceManager.SMS_MESSAGE_PERIOD_NOT_SPECIFIED;
import static com.android.internal.telephony.IccSmsInterfaceManager.SMS_MESSAGE_PRIORITY_NOT_SPECIFIED;

import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;
import android.telephony.ServiceState;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.SmsHeader;
import com.android.internal.telephony.SmsRawData;
import com.android.internal.telephony.SmsResponse;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.SmsDispatchersController;
import com.android.internal.telephony.SmsMessageBase;
import com.android.internal.telephony.ImsSmsDispatcher;
import com.android.internal.telephony.TelephonyComponentFactory;
import com.android.internal.telephony.util.SMSDispatcherUtil;
import com.mediatek.internal.telephony.MtkSmsHeader;
import com.mediatek.internal.telephony.util.MtkSMSDispatcherUtil;
import com.mediatek.internal.telephony.util.MtkSmsCommonUtil;
import com.mediatek.internal.telephony.ppl.PplSmsFilterExtension;

import static android.telephony.SmsManager.RESULT_ERROR_GENERIC_FAILURE;
import static android.telephony.SmsManager.RESULT_ERROR_FDN_CHECK_FAILURE;
import static android.telephony.SmsManager.RESULT_ERROR_NULL_PDU;

import java.util.HashMap;
import java.util.Map;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicBoolean;

import android.net.Uri;

/**
 * Responsible for communications with {@link com.android.ims.ImsManager} to send/receive messages
 * over IMS.
 * @hide
 */
public class MtkImsSmsDispatcher extends ImsSmsDispatcher {

    private static final String TAG = "MtkImsSmsDispacher";

    /** Mobile manager service for phone privacy lock */

    private PplSmsFilterExtension mPplSmsFilter = null;
    private static final boolean ENG = "eng".equals(Build.TYPE);
    private ThreadLocal<Integer> mEncodingType = new ThreadLocal<Integer>() {
        @Override
        protected Integer initialValue() {
            return SmsConstants.ENCODING_UNKNOWN;
        }
    };

    protected SmsMessageBase.SubmitPduBase getSubmitPdu(String scAddr,
            String destAddr, int destinationPort, int originalPort, byte[] data,
            boolean statusReportRequested) {
        return MtkSMSDispatcherUtil.getSubmitPdu(isCdmaMo(), scAddr, destAddr, destinationPort,
                originalPort, data, statusReportRequested);
    }

    protected SmsMessageBase.SubmitPduBase getSubmitPdu(String scAddr,
            String destAddr, byte[] data, byte[] smsHeader, boolean statusReportRequested) {
        return MtkSMSDispatcherUtil.getSubmitPdu(isCdmaMo(), scAddr, destAddr, data, smsHeader,
                statusReportRequested);
    }

    protected SmsMessageBase.SubmitPduBase getSubmitPdu(boolean isCdma, String scAddress,
            String destinationAddress, String message,
            boolean statusReportRequested, byte[] header, int encoding,
            int languageTable, int languageShiftTable, int validityPeriod) {
            return MtkSMSDispatcherUtil.getSubmitPdu(isCdmaMo(), scAddress, destinationAddress,
                    message, statusReportRequested, header,
                    encoding, languageTable, languageShiftTable, validityPeriod);
    }

    public MtkImsSmsDispatcher(Phone phone,
            SmsDispatchersController smsDispatchersController) {
        super(phone, smsDispatchersController);
        Rlog.d(TAG, "Created!");
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
            int originalPort, byte[] data, PendingIntent sentIntent, PendingIntent deliveryIntent) {
        if (!isCdmaMo()) {
            sendDataGsm(callingPackage, destAddr, scAddr, destPort, originalPort, data, sentIntent,
                    deliveryIntent);
        } else {
            // Don't support MO 3GPP2 SMS over IMS with original Port
            super.sendData(callingPackage, destAddr, scAddr, destPort, data, sentIntent,
                    deliveryIntent, false);
        }
    }

    /** {@inheritDoc} */
    public void sendMultipartData(String callingPackage,
            String destAddr, String scAddr, int destPort,
            ArrayList<SmsRawData> data, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents) {
        if (!isCdmaMo()) {
            sendMultipartDataGsm(callingPackage, destAddr, scAddr, destPort, data, sentIntents,
                    deliveryIntents);
        } else {
            Rlog.d(TAG, "Don't support sendMultipartData for CDMA");
        }
    }

    /** {@inheritDoc} */
    public void sendTextWithEncodingType(String destAddr, String scAddr, String text,
            int encodingType, PendingIntent sentIntent, PendingIntent deliveryIntent,
            Uri messageUri, String callingPkg, boolean persistMessage, int priority,
            boolean expectMore, int validityPeriod) {
        if (!isCdmaMo()) {
            sendTextWithEncodingTypeGsm(destAddr, scAddr, text, encodingType,
                    sentIntent, deliveryIntent, messageUri, callingPkg, persistMessage, priority,
                    expectMore, validityPeriod);
        } else {
            // Don't support MO 3GPP2 SMS over IMS with encoding type.
            sendText(destAddr, scAddr, text, sentIntent, deliveryIntent,
                    messageUri, callingPkg, persistMessage, priority, expectMore, validityPeriod,
                    false);

        }
    }

    /** {@inheritDoc} */
    public void sendMultipartTextWithEncodingType(String destAddr, String scAddr,
            ArrayList<String> parts, int encodingType, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents, Uri messageUri, String callingPkg,
            boolean persistMessage, int priority, boolean expectMore, int validityPeriod) {
        if (!isCdmaMo()) {
            sendMultipartTextWithEncodingTypeGsm(destAddr, scAddr, parts, encodingType,
                    sentIntents, deliveryIntents, messageUri, callingPkg, persistMessage, priority,
                    expectMore, validityPeriod);
        } else {
            // Don't support MO 3GPP2 SMS over IMS with encoding type.
            sendMultipartText(destAddr, scAddr, parts, sentIntents, deliveryIntents, messageUri,
                    callingPkg, persistMessage, priority, expectMore, validityPeriod);

        }
    }

    /** {@inheritDoc} */
    protected SmsTracker getNewSubmitPduTracker(String callingPackage, String destinationAddress,
            String scAddress, String message, SmsHeader smsHeader, int encoding,
            PendingIntent sentIntent, PendingIntent deliveryIntent, boolean lastPart,
            AtomicInteger unsentPartCount, AtomicBoolean anyPartFailed, Uri messageUri,
            String fullMessageText, int priority, boolean expectMore, int validityPeriod) {
        if (ENG) {
            Rlog.d(TAG, "getNewSubmitPduTracker w/ validity");
        }

        if (isCdmaMo()) {
            return super.getNewSubmitPduTracker(callingPackage, destinationAddress, scAddress,
                    message, smsHeader, encoding, sentIntent, deliveryIntent, lastPart,
                    unsentPartCount, anyPartFailed, messageUri, fullMessageText, priority,
                    expectMore, validityPeriod);
        } else {
            return getNewSubmitPduTrackerGsm(callingPackage, destinationAddress, scAddress,
                    message, smsHeader, encoding, sentIntent, deliveryIntent, lastPart,
                    unsentPartCount, anyPartFailed, messageUri, fullMessageText, priority,
                    expectMore, validityPeriod);
        }
    }

   // GSM-START
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
    protected void sendDataGsm(String callingPackage, String destAddr, String scAddr, int destPort,
            int originalPort, byte[] data, PendingIntent sentIntent, PendingIntent deliveryIntent) {
        Rlog.d(TAG, "sendData: enter");
        SmsMessageBase.SubmitPduBase pdu = getSubmitPdu(
                scAddr, destAddr, destPort, originalPort, data, (deliveryIntent != null));
        if (pdu != null) {
            HashMap map = getSmsTrackerMap(destAddr, scAddr, destPort, data, pdu);
            SmsTracker tracker = getSmsTracker(callingPackage, map, sentIntent, deliveryIntent,
                    getFormat(), null /*messageUri*/, false /*expectMore*/,
                    null /*fullMessageText*/, false /*isText*/,
                    true /*persistMessage*/, false /*isForVvm*/);

            if (!sendSmsByCarrierApp(true /* isDataSms */, tracker)) {
                sendSubmitPdu(tracker);
            }
        } else {
            Rlog.e(TAG, "sendData(): getSubmitPdu() returned null");
        }
    }

    /** {@inheritDoc} */
    public void sendMultipartDataGsm(String callingPackage,
            String destAddr, String scAddr, int destPort,
            ArrayList<SmsRawData> data, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents) {
        if (data == null) {
            Rlog.e(TAG, "Cannot send multipart data when data is null!");
            return;
        }

        int refNumber = getNextConcatenatedRef() & 0x00FF;
        int msgCount = data.size();

        SmsTracker[] trackers = new SmsTracker[msgCount];

        for (int i = 0; i < msgCount; i++) {
            byte[] smsHeader = MtkSmsHeader.getSubmitPduHeader(
                    destPort, refNumber, i + 1, msgCount);   // 1-based sequence

            PendingIntent sentIntent = null;
            if (sentIntents != null && sentIntents.size() > i) {
                sentIntent = sentIntents.get(i);
            }

            PendingIntent deliveryIntent = null;
            if (deliveryIntents != null && deliveryIntents.size() > i) {
                deliveryIntent = deliveryIntents.get(i);
            }

            SmsMessageBase.SubmitPduBase pdus = getSubmitPdu(scAddr, destAddr,
                    data.get(i).getBytes() , smsHeader, deliveryIntent != null);

            HashMap map =  getSmsTrackerMap(destAddr, scAddr, destPort, data.get(i).getBytes(),
                    pdus);

            // FIXME: should use getNewSubmitPduTracker?
            trackers[i] =
                getSmsTracker(callingPackage, map, sentIntent, deliveryIntent, getFormat(),
                        null/*messageUri*/, false /*isExpectMore*/, null /*fullMessageText*/,
                        false /*isText*/, true /*persistMessage*/, false /*isForVvm*/);
        }

        if (trackers.length == 0 || trackers[0] == null) {
            Rlog.e(TAG, "Cannot send multipart data. trackers length = "
                    + trackers.length);
            return;
        }

        for (SmsTracker tracker : trackers) {
            if (tracker != null) {
                if (!sendSmsByCarrierApp(true /* isDataSms */, tracker)) {
                    sendSubmitPdu(tracker);
                }
            } else {
                Rlog.e(TAG, "Null tracker.");
            }
        }
    }

    /** {@inheritDoc} */
    public void sendTextWithEncodingTypeGsm(String destAddr, String scAddr, String text,
            int encodingType, PendingIntent sentIntent, PendingIntent deliveryIntent,
            Uri messageUri, String callingPkg, boolean persistMessage, int priority,
            boolean expectMore, int validityPeriod) {
        Rlog.d(TAG, "sendTextWithEncodingTypeGsm encoding = " + encodingType);
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
        if (encodingType == SmsConstants.ENCODING_UNKNOWN) {
            return super.onSendText(destAddr, scAddr, text,
                 sentIntent, deliveryIntent, messageUri, callingPkg, persistMessage, priority,
                 expectMore, validityPeriod);
        } else {
            TextEncodingDetails details = SMSDispatcherUtil.calculateLength(false, text, false);
            return getSubmitPdu(false, scAddr, destAddr, text,
                (deliveryIntent != null), null, encodingType, details.languageTable,
                details.languageShiftTable, validityPeriod);
        }
    }

    /** {@inheritDoc} */
    public void sendMultipartTextWithEncodingTypeGsm(String destAddr, String scAddr,
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
        // MTK-START
        encoding = onSendMultipartText(destAddr, scAddr, parts, sentIntents, deliveryIntents,
                messageUri, callingPkg, persistMessage, priority, expectMore, validityPeriod,
                encodingForParts);
        // MTK-END
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
            // MTK-START
            // Modification for sub class
            TelephonyComponentFactory telephonyComponentFactory = TelephonyComponentFactory
                    .getInstance().inject(TelephonyComponentFactory.class.getName());
            SmsHeader smsHeader = telephonyComponentFactory.makeSmsHeader();
            // MTK-END
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
        if (encodingType == SmsConstants.ENCODING_UNKNOWN) {
            encodingType = super.onSendMultipartText(destAddr, scAddr,
                    parts, sentIntents, deliveryIntents, messageUri, callingPkg,
                    persistMessage, priority, expectMore, validityPeriod, encodingForParts);
            Rlog.d(TAG, "onSendMultipartText encoding = " + encodingType);
        } else {
            int msgCount = parts.size();
            for (int i = 0; i < msgCount; ++i) {
                TextEncodingDetails details = SMSDispatcherUtil.calculateLength(false,
                        parts.get(i), false);
                if (encodingType != details.codeUnitSize &&
                    (encodingType == SmsConstants.ENCODING_UNKNOWN ||
                     encodingType == SmsConstants.ENCODING_7BIT)) {
                    Rlog.d(TAG, "[enc conflict between details[" + details.codeUnitSize
                            + "] and encoding " + encodingType);
                    details.codeUnitSize = encodingType;
                }
                encodingForParts[i] = details;
            }
        }
        return encodingType;
    }

    protected SmsTracker getSmsTracker(
            String callingPackage, HashMap<String, Object> data, PendingIntent sentIntent,
            PendingIntent deliveryIntent, String format, Uri messageUri, boolean expectMore,
            String fullMessageText, boolean isText, boolean persistMessage, boolean isForVvm) {
        return getSmsTracker(
                callingPackage, data, sentIntent, deliveryIntent, format, null/*unsentPartCount*/,
                null/*anyPartFailed*/, messageUri, null/*smsHeader*/, expectMore,
                fullMessageText, isText, persistMessage, SMS_MESSAGE_PRIORITY_NOT_SPECIFIED,
                SMS_MESSAGE_PERIOD_NOT_SPECIFIED, isForVvm);
    }

    protected SmsTracker getSmsTracker(
            String callingPackage, HashMap<String, Object> data, PendingIntent sentIntent,
            PendingIntent deliveryIntent, String format, AtomicInteger unsentPartCount,
            AtomicBoolean anyPartFailed, Uri messageUri, SmsHeader smsHeader,
            boolean expectMore, String fullMessageText, boolean isText, boolean persistMessage,
            int priority, int validityPeriod, boolean isForVvm) {
        SmsTracker tracker = super.getSmsTracker(callingPackage,
                data, sentIntent, deliveryIntent, format,
                unsentPartCount, anyPartFailed, messageUri, smsHeader, expectMore,
                fullMessageText, isText, persistMessage, priority,
                validityPeriod, isForVvm);
        MtkSmsCommonUtil.filterOutByPpl(mContext, tracker);
        return tracker;
    }

    /** {@inheritDoc} */


    /**
     * Send the multi-part SMS based on multipart Sms tracker
     *
     * @param tracker holds the multipart Sms tracker ready to be sent
     */
    @Override
    protected void sendMultipartSms(SmsTracker tracker) {
        ArrayList<String> parts;
        ArrayList<PendingIntent> sentIntents;

        HashMap<String, Object> map = tracker.getData();

        parts = (ArrayList<String>) map.get("parts");
        sentIntents = (ArrayList<PendingIntent>) map.get("sentIntents");

        // check if in service
        int ss = mPhone.getServiceState().getState();
        // if sms over IMS is not supported on data and voice is not available...
        // MTK-START
        // For the Wifi calling, We need to support sending SMS when radio is power off
        // and wifi calling is enabled. So we need to pass the SMS sending request to the
        // modem when radio is OFF.
        if (!isIms() && ss != ServiceState.STATE_IN_SERVICE
                && !mTelephonyManager.isWifiCallingAvailable()) {
        // MTK-END
            for (int i = 0, count = parts.size(); i < count; i++) {
                PendingIntent sentIntent = null;
                if (sentIntents != null && sentIntents.size() > i) {
                    sentIntent = sentIntents.get(i);
                }
                handleNotInService(ss, sentIntent);
            }
            return;
        }

        super.sendMultipartSms(tracker);
    }

    private SmsTracker getNewSubmitPduTrackerGsm(
            String callingPackage, String destinationAddress, String scAddress,
            String message, SmsHeader smsHeader, int encoding,
            PendingIntent sentIntent, PendingIntent deliveryIntent, boolean lastPart,
            AtomicInteger unsentPartCount, AtomicBoolean anyPartFailed, Uri messageUri,
            String fullMessageText, int priority, boolean expectMore, int validityPeriod) {
        SmsMessageBase.SubmitPduBase pdu = getSubmitPdu(false, scAddress, destinationAddress,
                    message, (deliveryIntent != null), MtkSmsHeader.toByteArray(smsHeader),
                    encoding, smsHeader.languageTable, smsHeader.languageShiftTable,
                    validityPeriod);
        if (pdu != null) {
            HashMap map =  getSmsTrackerMap(destinationAddress, scAddress,
                    message, pdu);
            return getSmsTracker(callingPackage, map, sentIntent,
                    deliveryIntent, getFormat(), unsentPartCount, anyPartFailed, messageUri,
                    smsHeader, (!lastPart || expectMore), fullMessageText, true /*isText*/,
                    true /*persistMessage*/, priority, validityPeriod, false);
        } else {
            Rlog.e(TAG, "getNewSubmitPduTrackerGsm: getSubmitPdu() returned null");
            return null;
        }
    }

    protected String getPackageNameViaProcessId(String[] packageNames) {
        return MtkSMSDispatcherUtil.getPackageNameViaProcessId(mContext, packageNames);
    }
    // GSM-END
}
