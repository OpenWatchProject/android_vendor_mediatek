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

import android.net.Uri;
import android.os.AsyncResult;
import android.os.Message;
import android.os.Build;
import android.os.Bundle;
import android.os.Binder;
import android.os.UserHandle;
import android.provider.Settings;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Intents;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.telephony.Rlog;
import android.telephony.ServiceState;

import java.util.HashMap;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import static com.android.internal.telephony.IccSmsInterfaceManager.SMS_MESSAGE_PERIOD_NOT_SPECIFIED;
import static com.android.internal.telephony.IccSmsInterfaceManager.SMS_MESSAGE_PRIORITY_NOT_SPECIFIED;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.SmsDispatchersController;
import com.android.internal.telephony.SmsUsageMonitor;
import com.android.internal.telephony.SmsHeader;
import com.android.internal.telephony.SmsResponse;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.gsm.GsmInboundSmsHandler;
import com.android.internal.telephony.gsm.GsmSMSDispatcher;
import com.android.internal.telephony.SmsMessageBase;
import com.android.internal.telephony.gsm.SmsMessage;
import com.android.internal.telephony.gsm.SmsBroadcastConfigInfo;
import com.android.internal.telephony.AsyncEmergencyContactNotifier;
import com.android.internal.telephony.TelephonyComponentFactory;
import com.android.internal.telephony.util.SMSDispatcherUtil;
import android.content.Intent;
import android.content.Context;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;



import java.util.ArrayList;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;
import com.android.internal.telephony.SmsRawData;
import java.util.List;

// Mobile manager service for phone privacy lock
import com.mediatek.internal.telephony.ppl.PplSmsFilterExtension;

import com.mediatek.internal.telephony.MtkSmsHeader;
import com.mediatek.internal.telephony.gsm.MtkSmsMessage;
import com.mediatek.internal.telephony.util.MtkSMSDispatcherUtil;
import com.mediatek.internal.telephony.util.MtkSmsCommonUtil;
import com.mediatek.internal.telephony.MtkSmsDispatchersController;
import static android.Manifest.permission.SEND_SMS_NO_CONFIRMATION;
import static android.telephony.SmsManager.STATUS_ON_ICC_READ;
import static android.telephony.SmsManager.STATUS_ON_ICC_UNREAD;
import static android.telephony.SmsManager.STATUS_ON_ICC_SENT;
import static android.telephony.SmsManager.STATUS_ON_ICC_UNSENT;
import static android.telephony.SmsManager.RESULT_ERROR_GENERIC_FAILURE;
import static android.telephony.SmsManager.RESULT_ERROR_NULL_PDU;
import static android.telephony.SmsManager.RESULT_ERROR_FDN_CHECK_FAILURE;
import static android.telephony.SmsManager.RESULT_ERROR_NO_SERVICE;
import static android.telephony.SmsManager.RESULT_ERROR_RADIO_OFF;
import static mediatek.telephony.MtkSmsManager.RESULT_ERROR_SUCCESS;
import static mediatek.telephony.MtkSmsManager.RESULT_ERROR_SIM_MEM_FULL;
import static mediatek.telephony.MtkSmsManager.RESULT_ERROR_INVALID_ADDRESS;
import static mediatek.telephony.MtkSmsManager.EXTRA_PARAMS_VALIDITY_PERIOD;



public class MtkGsmSMSDispatcher extends GsmSMSDispatcher {
    private static final String TAG = "MtkGsmSMSDispatcher";
    private static final boolean ENG = "eng".equals(Build.TYPE);

    // flag of storage status
    /** For FTA test only */
    private boolean mStorageAvailable = true;

    private boolean mSuccess = true;

    // for copying text message to ICC card
    protected int messageCountNeedCopy = 0;
    protected Object mLock = new Object();

    private ThreadLocal<Integer> mEncodingType = new ThreadLocal<Integer>() {
        @Override
        protected Integer initialValue() {
            return SmsConstants.ENCODING_UNKNOWN;
        }
    };

    protected static String PDU_SIZE = "pdu_size";
    protected static String MSG_REF_NUM = "msg_ref_num";

    /* EVENT */
    /** copy text message to the ICC card */
    static final protected int EVENT_COPY_TEXT_MESSAGE_DONE = 106;
    static final protected int EVENT_ADD_DELIVER_PENDING_LIST = 107;

    public MtkGsmSMSDispatcher(Phone phone, SmsDispatchersController smsDispatchersController,
            GsmInboundSmsHandler gsmInboundSmsHandler) {
        super(phone, smsDispatchersController, gsmInboundSmsHandler);
        mUiccController.unregisterForIccChanged(this);
        Integer phoneId = new Integer(mPhone.getPhoneId());
        mUiccController.registerForIccChanged(this, EVENT_ICC_CHANGED, phoneId);
        Rlog.d(TAG, "MtkGsmSMSDispatcher created");
    }

    @Override
    public void dispose() {
        super.dispose();
        mCi.unSetOnSmsStatus(this);
        mUiccController.unregisterForIccChanged(this);
    }

    @Override
    // MTK-START
    // Modification for sub class
    public String getFormat() {
    // MTK-END
        return SmsConstants.FORMAT_3GPP;
    }

    /**
     * Handles 3GPP format-specific events coming from the phone stack.
     * Other events are handled by {@link SMSDispatcher#handleMessage}.
     *
     * @param msg the message to handle
     */
    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
        case EVENT_ICC_CHANGED:
            Integer phoneId = getUiccControllerPhoneId(msg);
            if (phoneId != mPhone.getPhoneId()) {
                Rlog.d(TAG, "Wrong phone id event coming, PhoneId: " + phoneId);
                break;
            } else {
                Rlog.d(TAG, "EVENT_ICC_CHANGED, PhoneId: " + phoneId + " match exactly.");
            }
            onUpdateIccAvailability();
            break;

        case EVENT_COPY_TEXT_MESSAGE_DONE:
        {
            AsyncResult ar;
            ar = (AsyncResult) msg.obj;
            synchronized (mLock) {
                mSuccess = (ar.exception == null);

                if (mSuccess == true) {
                    Rlog.d(TAG, "[copyText success to copy one");
                    messageCountNeedCopy -= 1;
                } else {
                    Rlog.d(TAG, "[copyText fail to copy one");
                    messageCountNeedCopy = 0;
                }

                mLock.notifyAll();
            }
            break;
        }
        case EVENT_ADD_DELIVER_PENDING_LIST:
        {
            SmsTracker tracker = (SmsTracker) msg.obj;
            // Expecting a status report.  Add it to the list.
            Rlog.d(TAG, "EVENT_ADD_DELIVER_PENDING_LIST mMessageRef=" + tracker.mMessageRef);
            deliveryPendingList.add(tracker);
            break;
        }
        default:
            super.handleMessage(msg);
        }
    }


    // MTK-START, support validity for operator feature
    /** {@inheritDoc} */
    protected SmsTracker getNewSubmitPduTracker(String callingPackage, String destinationAddress,
            String scAddress, String message, SmsHeader smsHeader, int encoding,
            PendingIntent sentIntent, PendingIntent deliveryIntent, boolean lastPart,
            AtomicInteger unsentPartCount, AtomicBoolean anyPartFailed, Uri messageUri,
            String fullMessageText, int priority, boolean expectMore, int validityPeriod) {
        if (ENG) {
            Rlog.d(TAG, "getNewSubmitPduTracker w/ validity");
        }
        MtkSmsMessage.SubmitPdu pdu = MtkSmsMessage.getSubmitPdu(scAddress, destinationAddress,
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
            Rlog.e(TAG, "GsmSMSDispatcher.getNewSubmitPduTracker(): getSubmitPdu() returned null");
            return null;
        }
    }
    // MTK-END

    /** {@inheritDoc} */

    // MTK-START
    /**
     * Get the correct phone id to identify the event sent from which slot.
     *
     * @param msg message event
     *
     * @return integer value of phone id
     *
     */
    private Integer getUiccControllerPhoneId(Message msg) {
        AsyncResult ar;
        Integer phoneId = new Integer(SubscriptionManager.INVALID_PHONE_INDEX);

        ar = (AsyncResult) msg.obj;
        if (ar != null && ar.result instanceof Integer) {
            phoneId = (Integer) ar.result;
        }
        return phoneId;
    }

    /** {@inheritDoc} */
    public void sendData(String callingPackage, String destAddr, String scAddr, int destPort,
            int originalPort, byte[] data, PendingIntent sentIntent, PendingIntent deliveryIntent) {
        Rlog.d(TAG, "MtkGsmSmsDispatcher.sendData: enter");

        MtkSmsMessage.SubmitPdu pdu = MtkSmsMessage.getSubmitPdu(
                scAddr, destAddr, destPort, originalPort, data, (deliveryIntent != null));
        if (pdu != null) {
            HashMap map = getSmsTrackerMap(destAddr, scAddr, destPort, data, pdu);
            SmsTracker tracker = getSmsTracker(callingPackage, map, sentIntent, deliveryIntent,
                    getFormat(), null /*messageUri*/, false /*isExpectMore*/,
                    null /*fullMessageText*/, false /*isText*/, true /*persistMessage*/,
                    false /*isForVvm*/);

            if (!sendSmsByCarrierApp(true /* isDataSms */, tracker)) {
                sendSubmitPdu(tracker);
            }
        } else {
            Rlog.e(TAG, "GsmSMSDispatcher.sendData(): getSubmitPdu() returned null");
        }
    }

    /** {@inheritDoc} */
    public void sendMultipartData(
            String callingPackage, String destAddr, String scAddr, int destPort,
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

            MtkSmsMessage.SubmitPdu pdus = MtkSmsMessage.getSubmitPdu(scAddr, destAddr,
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

    public int copyTextMessageToIccCard(String scAddress, String address, List<String> text,
            int status, long timestamp) {
        Rlog.d(TAG, "GsmSMSDispatcher: copy text message to icc card");

        if (checkPhoneNumber(scAddress) == false) {
            Rlog.d(TAG, "[copyText invalid sc address");
            scAddress = null;
        }

        mSuccess = true;

        boolean isDeliverPdu = true;

        int msgCount = text.size();
        // we should check the available storage of SIM here,
        // but now we suppose it always be true
        if (true) {
            Rlog.d(TAG, "[copyText storage available");
        } else {
            Rlog.d(TAG, "[copyText storage unavailable");
            return RESULT_ERROR_SIM_MEM_FULL;
        }

        if (status == STATUS_ON_ICC_READ || status == STATUS_ON_ICC_UNREAD) {
            Rlog.d(TAG, "[copyText to encode deliver pdu");
            isDeliverPdu = true;
        } else if (status == STATUS_ON_ICC_SENT || status == STATUS_ON_ICC_UNSENT) {
            isDeliverPdu = false;
            Rlog.d(TAG, "[copyText to encode submit pdu");
        } else {
            Rlog.d(TAG, "[copyText invalid status, default is deliver pdu");
            // isDeliverPdu = true;
            return RESULT_ERROR_GENERIC_FAILURE;
        }

        if (isDeliverPdu == false && checkPhoneNumber(address) == false) {
            Rlog.d(TAG, "[copyText invalid dest address");
            return RESULT_ERROR_INVALID_ADDRESS;
        }

        Rlog.d(TAG, "[copyText msgCount " + msgCount);
        if (msgCount > 1) {
            Rlog.d(TAG, "[copyText multi-part message");
        } else if (msgCount == 1) {
            Rlog.d(TAG, "[copyText single-part message");
        } else {
            Rlog.d(TAG, "[copyText invalid message count");
            return RESULT_ERROR_GENERIC_FAILURE;
        }

        int refNumber = getNextConcatenatedRef() & 0x00FF;
        int encoding = SmsConstants.ENCODING_UNKNOWN;
        TextEncodingDetails details[] = new TextEncodingDetails[msgCount];
        for (int i = 0; i < msgCount; i++) {
            details[i] = MtkSmsMessage.calculateLength(text.get(i), false);
            if (encoding != details[i].codeUnitSize &&
                (encoding == SmsConstants.ENCODING_UNKNOWN ||
                 encoding == SmsConstants.ENCODING_7BIT)) {
                encoding = details[i].codeUnitSize;
            }
        }

        for (int i = 0; i < msgCount; ++i) {
            if (mSuccess == false) {
                Rlog.d(TAG, "[copyText Exception happened when copy message");
                return RESULT_ERROR_GENERIC_FAILURE;
            }
            int singleShiftId = -1;
            int lockingShiftId = -1;
            int language = details[i].shiftLangId;
            int encoding_method = encoding;

            if (encoding == SmsConstants.ENCODING_7BIT) {
                Rlog.d(TAG, "Detail: " + i + " ted" + details[i]);
                if (details[i].useLockingShift && details[i].useSingleShift) {
                    singleShiftId = language;
                    lockingShiftId = language;
                    encoding_method = MtkSmsMessage.ENCODING_7BIT_LOCKING_SINGLE;
                } else if (details[i].useLockingShift) {
                    lockingShiftId = language;
                    encoding_method = MtkSmsMessage.ENCODING_7BIT_LOCKING;
                } else if (details[i].useSingleShift) {
                    singleShiftId = language;
                    encoding_method = MtkSmsMessage.ENCODING_7BIT_SINGLE;
                }
            }

            byte[] smsHeader = null;
            if (msgCount > 1) {
                Rlog.d(TAG, "[copyText get pdu header for multi-part message");
                // 1-based sequence
                smsHeader = MtkSmsHeader.getSubmitPduHeaderWithLang(
                        -1, refNumber, i + 1, msgCount, singleShiftId, lockingShiftId);
            }

            if (isDeliverPdu) {
                MtkSmsMessage.DeliverPdu pdu = MtkSmsMessage.getDeliverPduWithLang(
                    scAddress, address, text.get(i), smsHeader, timestamp, encoding, language);

                if (pdu != null) {
                    Rlog.d(TAG, "[copyText write deliver pdu into SIM");
                    mCi.writeSmsToSim(status, IccUtils.bytesToHexString(pdu.encodedScAddress),
                            IccUtils.bytesToHexString(pdu.encodedMessage),
                            obtainMessage(EVENT_COPY_TEXT_MESSAGE_DONE));

                    synchronized (mLock) {
                        try {
                            Rlog.d(TAG, "[copyText wait until the message be wrote in SIM");
                            mLock.wait();
                        } catch (InterruptedException e) {
                            Rlog.d(TAG, "Fail to copy text message into SIM");
                            return RESULT_ERROR_GENERIC_FAILURE;
                        }
                    }
                }
            } else {
                MtkSmsMessage.SubmitPdu pdu = MtkSmsMessage.getSubmitPduWithLang(scAddress, address,
                          text.get(i), false, smsHeader, encoding_method, language,
                          SMS_MESSAGE_PRIORITY_NOT_SPECIFIED);

                if (pdu != null) {
                    Rlog.d(TAG, "[copyText write submit pdu into SIM");
                    mCi.writeSmsToSim(status, IccUtils.bytesToHexString(pdu.encodedScAddress),
                            IccUtils.bytesToHexString(pdu.encodedMessage),
                            obtainMessage(EVENT_COPY_TEXT_MESSAGE_DONE));

                    synchronized (mLock) {
                        try {
                            Rlog.d(TAG, "[copyText wait until the message be wrote in SIM");
                            mLock.wait();
                        } catch (InterruptedException e) {
                            Rlog.d(TAG, "fail to copy text message into SIM");
                            return RESULT_ERROR_GENERIC_FAILURE;
                        }
                    }
                }
            }

            Rlog.d(TAG, "[copyText thread is waked up");
        }

        if (mSuccess == true) {
            Rlog.d(TAG, "[copyText all messages have been copied into SIM");
            return RESULT_ERROR_SUCCESS;
        }

        Rlog.d(TAG, "[copyText copy failed");
        return RESULT_ERROR_GENERIC_FAILURE;
    }

    private boolean isValidSmsAddress(String address) {
        String encodedAddress = PhoneNumberUtils.extractNetworkPortion(address);

        return (encodedAddress == null) ||
                (encodedAddress.length() == address.length());
    }

    private boolean checkPhoneNumber(final char c) {
        return (c >= '0' && c <= '9') || (c == '*') || (c == '+')
                || (c == '#') || (c == 'N') || (c == ' ') || (c == '-');
    }

    private boolean checkPhoneNumber(final String address) {
        if (address == null) {
            return true;
        }

        Rlog.d(TAG, "checkPhoneNumber");
        for (int i = 0, n = address.length(); i < n; ++i) {
            if (checkPhoneNumber(address.charAt(i))) {
                continue;
            } else {
                return false;
            }
        }

        return true;
    }

    /** {@inheritDoc} */
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
        if (encodingType == SmsConstants.ENCODING_UNKNOWN) {
            return super.onSendText(destAddr, scAddr, text,
                 sentIntent, deliveryIntent, messageUri, callingPkg, persistMessage, priority,
                 expectMore, validityPeriod);
        } else {
            TextEncodingDetails details = SMSDispatcherUtil.calculateLength(false, text, false);
            return SmsMessage.getSubmitPdu(
                scAddr, destAddr, text, (deliveryIntent != null), null, encodingType,
                details.languageTable, details.languageShiftTable, validityPeriod);
        }
    }

    /** {@inheritDoc} */
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

    /**
     * Called when IccSmsInterfaceManager update SIM card fail due to SIM_FULL.
     */
    public void handleIccFull() {
        // broadcast SIM_FULL intent
        mGsmInboundSmsHandler.mStorageMonitor.handleIccFull();
    }

    /**
     * Called when a CB activation result is received.
     *
     * @param ar AsyncResult passed into the message handler.
     */
    public void handleQueryCbActivation(AsyncResult ar) {

        Boolean result = null;

        if (ar.exception == null) {
            ArrayList<SmsBroadcastConfigInfo> list =
                    (ArrayList<SmsBroadcastConfigInfo>) ar.result;

            if (list.size() == 0) {
                result = new Boolean(false);
            } else {
                SmsBroadcastConfigInfo cbConfig = list.get(0);
                Rlog.d(TAG, "cbConfig: " + cbConfig.toString());

                if (cbConfig.getFromCodeScheme() == -1 &&
                    cbConfig.getToCodeScheme() == -1 &&
                    cbConfig.getFromServiceId() == -1 &&
                    cbConfig.getToServiceId() == -1 &&
                    cbConfig.isSelected() == false) {

                    result = new Boolean(false);
                } else {
                    result = new Boolean(true);
                }
            }
        }

        Rlog.d(TAG, "queryCbActivation: " + result);
        AsyncResult.forMessage((Message) ar.userObj, result, ar.exception);
        ((Message) ar.userObj).sendToTarget();
    }

    /**
     * Set the memory storage status of the SMS
     * This function is used for FTA test only
     *
     * @param status false for storage full, true for storage available
     *
     */
    public void setSmsMemoryStatus(boolean status) {
        if (status != mStorageAvailable) {
            mStorageAvailable = status;
            mCi.reportSmsMemoryStatus(status, null);
        }
    }

    public boolean isSmsReady() {
        MtkSmsDispatchersController pSmsDispatcherctrl =
                (MtkSmsDispatchersController)mSmsDispatchersController;
        return pSmsDispatcherctrl.isSmsReady();
    }

    protected String getPackageNameViaProcessId(String[] packageNames) {
        return MtkSMSDispatcherUtil.getPackageNameViaProcessId(mContext, packageNames);
    }

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

    protected SmsTracker getSmsTracker(String callingPackage, HashMap<String, Object> data,
            PendingIntent sentIntent, PendingIntent deliveryIntent, String format,
            AtomicInteger unsentPartCount, AtomicBoolean anyPartFailed, Uri messageUri,
            SmsHeader smsHeader,boolean isExpectMore, String fullMessageText, boolean isText,
            boolean persistMessage, boolean isForVvm) {
        SmsTracker tracker = super.getSmsTracker(callingPackage, data, sentIntent, deliveryIntent,
                format, unsentPartCount, anyPartFailed, messageUri, smsHeader, isExpectMore,
                fullMessageText, isText, persistMessage, SMS_MESSAGE_PRIORITY_NOT_SPECIFIED,
                SMS_MESSAGE_PERIOD_NOT_SPECIFIED, isForVvm);
        MtkSmsCommonUtil.filterOutByPpl(mContext, tracker);
        return tracker;
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

    public void addToGsmDeliverPendingList(SmsTracker tracker) {
        if (tracker.mDeliveryIntent != null) {
            Rlog.d(TAG, "addToGsmDeliverPendingList sendMessage");
            sendMessage(obtainMessage(EVENT_ADD_DELIVER_PENDING_LIST, tracker));
        }
    }
}
