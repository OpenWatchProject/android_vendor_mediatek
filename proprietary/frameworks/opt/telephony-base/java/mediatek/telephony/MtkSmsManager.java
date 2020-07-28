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
package mediatek.telephony;

import android.app.ActivityThread;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.Rlog;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.SubscriptionManager;
import mediatek.telephony.MtkSimSmsInsertStatus;
import mediatek.telephony.MtkSmsParameters;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.util.ArrayMap;
import android.text.TextUtils;


import com.android.internal.telephony.ISms;
import com.android.internal.telephony.SmsRawData;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SmsConstants;

import com.mediatek.internal.telephony.IMtkSms;
import com.mediatek.internal.telephony.MtkIccSmsStorageStatus;
import mediatek.telephony.MtkSmsMessage;



import java.util.Map;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;


/**
 * Manages MTK proprietary SMS operations.
 */
public final class MtkSmsManager {

    private static final String TAG = "MtkSmsManager";

    private static final int DEFAULT_SUBSCRIPTION_ID = -1002;

    /** Singleton object constructed during class initialization. */
    private static final MtkSmsManager sInstance = new MtkSmsManager(DEFAULT_SUBSCRIPTION_ID);
    private static final Object sLockObject = new Object();

    private static final Map<Integer, MtkSmsManager> sSubInstances =
            new ArrayMap<Integer, MtkSmsManager>();

    /** A concrete subscription id, or the pseudo DEFAULT_SUBSCRIPTION_ID */
    private int mSubId;

    /*
     * Forwarded constants from SimDialogActivity.
     */
    private static String DIALOG_TYPE_KEY = "dialog_type";
    private static final int SMS_PICK = 2;

    /**
     * Sucessful error code.
     *
     * @hide
     */
    static public final int RESULT_ERROR_SUCCESS = 0;
    /**
     * Failed because sim memory is full.
     *
     * @internal
     * @hide
     */
    static public final int RESULT_ERROR_SIM_MEM_FULL = 7;
    /** @hide */
    static public final int RESULT_ERROR_INVALID_ADDRESS = 8;

    // for SMS validity period feature
    /**
     * Support to change the validity period.
     * Extra parameter on bundle for validity period.
     *
     * @internal
     * @hide
     */
    public static final String EXTRA_PARAMS_VALIDITY_PERIOD = "validity_period";

    /** @hide */
    public static final String EXTRA_PARAMS_ENCODING_TYPE = "encoding_type";

    /**
     * Support to change the validity period.
     * The value of no duration.
     *
     * @internal
     * @hide
     */
    public static final int VALIDITY_PERIOD_NO_DURATION = -1;

    /**
     * Support to change the validity period.
     * The value of one hour.
     *
     * @internal
     * @hide
     */
    public static final int VALIDITY_PERIOD_ONE_HOUR = 11; // (VP + 1) * 5 = 60 Mins

    /**
     * Support to change the validity period.
     * The value of six hours.
     *
     * @internal
     * @hide
     */
    public static final int VALIDITY_PERIOD_SIX_HOURS = 71; // (VP + 1) * 5 = 6 * 60 Mins

    /**
     * Support to change the validity period.
     * The value of twelve hours.
     *
     * @internal
     * @hide
     */
    public static final int VALIDITY_PERIOD_TWELVE_HOURS = 143; // (VP + 1) * 5 = 12 * 60 Mins

    /**
     * Support to change the validity period.
     * The value of one day.
     *
     * @internal
     * @hide
     */
    public static final int VALIDITY_PERIOD_ONE_DAY = 167; // 12 + (VP - 143) * 30 Mins = 24 Hours

    /**
     * Support to change the validity period.
     * The value of maximum duration and use the network setting.
     *
     * @internal
     * @hide
     */
    public static final int VALIDITY_PERIOD_MAX_DURATION = 255; // (VP - 192) Weeks

    /**
     * Get the SmsManager associated with the default subscription id. The instance will always be
     * associated with the default subscription id, even if the default subscription id is changed.
     *
     * @return the SmsManager associated with the default subscription id
     */
    public static MtkSmsManager getDefault() {
        return sInstance;
    }

    /**
     * Get the the instance of the SmsManager associated with a particular subscription id
     *
     * @param subId an SMS subscription id, typically accessed using
     *   {@link android.telephony.SubscriptionManager}
     * @return the instance of the SmsManager associated with subId
     */
    public static MtkSmsManager getSmsManagerForSubscriptionId(int subId) {
        // TODO(shri): Add javadoc link once SubscriptionManager is made public api
        synchronized(sLockObject) {
            MtkSmsManager smsManager = sSubInstances.get(subId);
            if (smsManager == null) {
                smsManager = new MtkSmsManager(subId);
                sSubInstances.put(subId, smsManager);
            }
            return smsManager;
        }
    }

    private MtkSmsManager(int subId) {
        mSubId = subId;
    }

    // Overwrite sendTextMessage, sendTextMessageWithoutPersisting and sendTextMessageInternal
    // in order to support empty contents

    /**
     * Send a text based SMS.
     *
     * <p class="note"><strong>Note:</strong> Using this method requires that your app has the
     * {@link android.Manifest.permission#SEND_SMS} permission.</p>
     *
     * <p class="note"><strong>Note:</strong> Beginning with Android 4.4 (API level 19), if
     * <em>and only if</em> an app is not selected as the default SMS app, the system automatically
     * writes messages sent using this method to the SMS Provider (the default SMS app is always
     * responsible for writing its sent messages to the SMS Provider). For information about
     * how to behave as the default SMS app, see {@link android.provider.Telephony}.</p>
     *
     *
     * @param destinationAddress the address to send the message to
     * @param scAddress is the service center address or null to use
     *  the current default SMSC
     * @param text the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is successfully sent, or failed.
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
     *
     * @throws IllegalArgumentException if destinationAddress or text are empty
     */
    public void sendTextMessage(
            String destinationAddress, String scAddress, String text,
            PendingIntent sentIntent, PendingIntent deliveryIntent) {
        sendTextMessageInternal(destinationAddress, scAddress, text,
            sentIntent, deliveryIntent, true /* persistMessageForCarrierApp*/);
    }

    private void sendTextMessageInternal(String destinationAddress, String scAddress,
            String text, PendingIntent sentIntent, PendingIntent deliveryIntent,
            boolean persistMessageForCarrierApp) {
        if (TextUtils.isEmpty(destinationAddress)) {
            throw new IllegalArgumentException("Invalid destinationAddress");
        }

        try {
            ISms iccISms = getISmsService();
            iccISms.sendTextForSubscriber(getSubscriptionId(), ActivityThread.currentPackageName(),
                    destinationAddress,
                    scAddress, text, sentIntent, deliveryIntent,
                    persistMessageForCarrierApp);
        } catch (RemoteException ex) {
            // ignore it
            // MTK-START
            Rlog.d(TAG, "sendTextMessage, RemoteException!");
            // MTK-END
        }
    }

    /**
     * Send a text based SMS without writing it into the SMS Provider.
     *
     * <p>Only the carrier app can call this method.</p>
     *
     * @see #sendTextMessage(String, String, String, PendingIntent, PendingIntent)
     * @hide
     */
    public void sendTextMessageWithoutPersisting(
            String destinationAddress, String scAddress, String text,
            PendingIntent sentIntent, PendingIntent deliveryIntent) {
        sendTextMessageInternal(destinationAddress, scAddress, text,
            sentIntent, deliveryIntent, false /* persistMessageForCarrierApp*/);
    }

    /**
     * Retrieves all messages currently stored on ICC.
     * ICC (Integrated Circuit Card) is the card of the device.
     * For example, this can be the SIM or USIM for GSM.
     *
     * @return <code>ArrayList</code> of <code>SmsMessage</code> objects
     *
     * {@hide}
     */
    public ArrayList<MtkSmsMessage> getAllMessagesFromIcc() {
        Rlog.d(TAG, "getAllMessagesFromIcc");
        List<SmsRawData> records = null;

        try {
            ISms iccISms = getISmsService();
            if (iccISms != null) {
                records = iccISms.getAllMessagesFromIccEfForSubscriber(
                        getSubscriptionId(),
                        ActivityThread.currentPackageName());
            }
        } catch (RemoteException ex) {
            // ignore it
            Rlog.d(TAG, "getAllMessagesFromIcc, RemoteException!");
        }

        return createMessageListFromRawRecords(records);
    }

    /**
     * Create a list of <code>SmsMessage</code>s from a list of RawSmsData
     * records returned by <code>getAllMessagesFromIcc()</code>
     *
     * @param records SMS EF records, returned by
     *   <code>getAllMessagesFromIcc</code>
     * @return <code>ArrayList</code> of <code>SmsMessage</code> objects.
     */
    // MTK-START
    private ArrayList<MtkSmsMessage> createMessageListFromRawRecords(List<SmsRawData> records) {
    // MTK-END
        ArrayList<MtkSmsMessage> messages = new ArrayList<MtkSmsMessage>();
        // MTK-START
        Rlog.d(TAG, "createMessageListFromRawRecords");
        // MTK-END
        if (records != null) {
            int count = records.size();
            for (int i = 0; i < count; i++) {
                SmsRawData data = records.get(i);
                // List contains all records, including "free" records (null)
                if (data != null) {
                    // MTK-START
                    Context context = ActivityThread.currentApplication().getApplicationContext();
                    int activePhone = TelephonyManager.from(context).getCurrentPhoneType(mSubId);
                    String phoneType = (PhoneConstants.PHONE_TYPE_CDMA == activePhone)
                            ? SmsConstants.FORMAT_3GPP2 : SmsConstants.FORMAT_3GPP;
                    Rlog.d(TAG, "phoneType: " + phoneType);
                    MtkSmsMessage sms = MtkSmsMessage.createFromEfRecord(i + 1, data.getBytes(),
                            phoneType);
                    // MTK-END
                    if (sms != null) {
                        messages.add(sms);
                    }
                }
            }
            // MTK-START
            Rlog.d(TAG, "actual sms count is " + count);
            // MTK-END
        // MTK-START
        } else {
            Rlog.d(TAG, "fail to parse SIM sms, records is null");
        }
        // MTK-END

        return messages;
    }

    /**
     * Retrieves all messages currently stored on ICC based on different mode.
     * Ex. CDMA mode or GSM mode for international cards.
     *
     * @param subId subscription identity
     * @param mode the GSM mode or CDMA mode
     *
     * @return <code>ArrayList</code> of <code>SmsMessage</code> objects
     * @hide
     */
    public ArrayList<MtkSmsMessage> getAllMessagesFromIccEfByMode(int mode) {
        Rlog.d(TAG, "getAllMessagesFromIcc, mode=" + mode);

        List<SmsRawData> records = null;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                records = iccISms.getAllMessagesFromIccEfByModeForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName(), mode);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException!");
        }

        int sz = 0;
        if (records != null) {
            sz = records.size();
        }
        for (int i = 0; i < sz; ++i) {
            byte[] data = null;
            SmsRawData record = records.get(i);
            if (record == null) {
                continue;
            } else {
                data = record.getBytes();
            }
            int index = i + 1;
            if ((data[0] & 0xff) == SmsManager.STATUS_ON_ICC_UNREAD) {
                Rlog.d(TAG, "index[" + index + "] is STATUS_ON_ICC_READ");
                SmsManager manager = SmsManager.getSmsManagerForSubscriptionId(mSubId);
                boolean ret = manager.updateMessageOnIcc(index,
                        SmsManager.STATUS_ON_ICC_READ, data);
                if (ret) {
                    Rlog.d(TAG, "update index[" + index + "] to STATUS_ON_ICC_READ");
                } else {
                    Rlog.d(TAG, "fail to update message status");
                }
            }
        }

        return createMessageListFromRawRecordsByMode(getSubscriptionId(), records, mode);
    }

    /**
     * Copy a text SMS to the ICC.
     *
     * @param subId subscription identity
     * @param scAddress Service center address
     * @param address   Destination address or original address
     * @param text      List of message text
     * @param status    message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *                  STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param timestamp Timestamp when service center receive the message
     * @return success or not
     *
     * @internal
     * @hide
     */
    public int copyTextMessageToIccCard(String scAddress, String address, List<String> text,
            int status, long timestamp) {
        Rlog.d(TAG, "copyTextMessageToIccCard");
        int result = SmsManager.RESULT_ERROR_GENERIC_FAILURE;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                result = iccISms.copyTextMessageToIccCardForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName(), scAddress, address, text, status,
                        timestamp);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException!");
        }

        return result;
    }


    /**
     * Send a data based SMS to a specific application port.
     *
     * @param destinationAddress the address to send the message to
     * @param scAddress is the service center address or null to use
     *  the current default SMSC
     * @param destinationPort the port to deliver the message to
     * @param originalPort the port to deliver the message from
     * @param data the body of the message to send
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
     *  is NULL the caller will be checked against all unknown applicaitons,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     *
     * @throws IllegalArgumentException if destinationAddress or data are empty
     *
     * @hide
     */
    public void sendDataMessage(String destinationAddress, String scAddress, short destinationPort,
            short originalPort, byte[] data, PendingIntent sentIntent,
            PendingIntent deliveryIntent) {
        Rlog.d(TAG, "sendDataMessage");
        if (TextUtils.isEmpty(destinationAddress)) {
            throw new IllegalArgumentException("Invalid destinationAddress");
        }

        if (!isValidParameters(destinationAddress, "send_data", sentIntent)) {
            return;
        }

        if (data == null || data.length == 0) {
            throw new IllegalArgumentException("Invalid message data");
        }

        // MTK-START
        //Context context = ActivityThread.currentApplication().getApplicationContext();
        //if (mDataOnlySmsFwkExt != null && mDataOnlySmsFwkExt.is4GDataOnlyMode(
        //        sentIntent, getSubscriptionId(), context)) {
        //    Rlog.d(TAG, "is4GDataOnlyMode");
        //    return;
        //}
        // MTK-END

        try {
            IMtkSms iccISms = getIMtkSmsServiceOrThrow();
            if (iccISms != null) {
                iccISms.sendDataWithOriginalPortForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName(), destinationAddress, scAddress,
                        destinationPort & 0xFFFF, originalPort & 0xFFFF, data, sentIntent,
                        deliveryIntent);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException!");
        }

    }

    /**
     * Send a text based SMS.
     *
     * @param subId subscription identity
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *  the current default SMSC
     * @param text the body of the message to send
     * @param encodingType the encoding type of message(gsm 7-bit, unicode or automatic)
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
     *
     * @throws IllegalArgumentException if destinationAddress or text are empty
     * @hide
     */
    public void sendTextMessageWithEncodingType(String destAddr, String scAddr, String text,
            int encodingType, PendingIntent sentIntent, PendingIntent deliveryIntent) {
        Rlog.d(TAG, "sendTextMessageWithEncodingType, encoding=" + encodingType);
        if (TextUtils.isEmpty(destAddr)) {
            throw new IllegalArgumentException("Invalid destinationAddress");
        }

        if (!isValidParameters(destAddr, text, sentIntent)) {
            Rlog.d(TAG, "the parameters are invalid");
            return;
        }

        // MTK-START
        //Context context = ActivityThread.currentApplication().getApplicationContext();
        //if (mDataOnlySmsFwkExt != null && mDataOnlySmsFwkExt.is4GDataOnlyMode(
        //        sentIntent, getSubscriptionId(), context)) {
        //    Rlog.d(TAG, "is4GDataOnlyMode");
        //    return;
        //}
        // MTK-END

        try {
            IMtkSms iccISms = getIMtkSmsServiceOrThrow();
            if (iccISms != null) {
                iccISms.sendTextWithEncodingTypeForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName(), destAddr, scAddr, text, encodingType,
                        sentIntent, deliveryIntent, true /*persistMessageForCarrierApp*/);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }
    }

    /**
     * Send a multi-part text based SMS.  The callee should have already
     * divided the message into correctly sized parts by calling
     * <code>divideMessage</code>.
     *
     * @param subId subscription identity
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *   the current default SMSC
     * @param parts an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param encodingType the encoding type of message(gsm 7-bit, unicode or automatic)
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     *   The result code will be <code>Activity.RESULT_OK<code> for success,
     *   or one of these errors:<br>
     *   <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *   <code>RESULT_ERROR_RADIO_OFF</code><br>
     *   <code>RESULT_ERROR_NULL_PDU</code><br>
     *   For <code>RESULT_ERROR_GENERIC_FAILURE</code> each sentIntent may include
     *   the extra "errorCode" containing a radio technology specific value,
     *   generally only useful for troubleshooting.<br>
     *   The per-application based SMS control checks sentIntent. If sentIntent
     *   is NULL the caller will be checked against all unknown applicaitons,
     *   which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     *
     * @throws IllegalArgumentException if destinationAddress or data are empty
     *
     * @internal
     * @hide
     */
    public void sendMultipartTextMessageWithEncodingType(String destAddr, String scAddr,
            ArrayList<String> parts, int encodingType, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents) {
        Rlog.d(TAG, "sendMultipartTextMessageWithEncodingType, encoding=" + encodingType);
        if (TextUtils.isEmpty(destAddr)) {
            throw new IllegalArgumentException("Invalid destinationAddress");
        }

        if (!isValidParameters(destAddr, parts, sentIntents)) {
            Rlog.d(TAG, "invalid parameters for multipart message");
            return;
        }
        // MTK-START
        //Context context = ActivityThread.currentApplication().getApplicationContext();
        //if (mDataOnlySmsFwkExt != null && mDataOnlySmsFwkExt.is4GDataOnlyMode(
        //        sentIntents, getSubscriptionId(), context)) {
        //    Rlog.d(TAG, "is4GDataOnlyMode");
        //    return;
        //}
        // MTK-END

        if (parts != null && parts.size() > 1) {
            try {
                IMtkSms iccISms = getIMtkSmsServiceOrThrow();
                if (iccISms != null) {
                    iccISms.sendMultipartTextWithEncodingTypeForSubscriber(getSubscriptionId(),
                            ActivityThread.currentPackageName(), destAddr, scAddr, parts,
                            encodingType, sentIntents, deliveryIntents,
                            true /*persistMessageForCarrierApp*/);
                }
            } catch (RemoteException ex) {
                Rlog.d(TAG, "RemoteException");
            }
        } else {
            PendingIntent sentIntent = null;
            PendingIntent deliveryIntent = null;
            if (sentIntents != null && sentIntents.size() > 0) {
                sentIntent = sentIntents.get(0);
            }
            Rlog.d(TAG, "get sentIntent: " + sentIntent);
            if (deliveryIntents != null && deliveryIntents.size() > 0) {
                deliveryIntent = deliveryIntents.get(0);
            }
            Rlog.d(TAG, "send single message");
            if (parts != null) {
                Rlog.d(TAG, "parts.size = " + parts.size());
            }
            String text = (parts == null || parts.size() == 0) ? "" : parts.get(0);
            Rlog.d(TAG, "pass encoding type " + encodingType);
            sendTextMessageWithEncodingType(destAddr, scAddr, text, encodingType, sentIntent,
                    deliveryIntent);
        }
    }

    /**
     * Divide a message text into several fragments, none bigger than
     * the maximum SMS message size.
     *
     * @param text the original message.  Must not be null.
     * @param encodingType text encoding type(7-bit, 16-bit or automatic)
     * @return an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     *
     * @internal
     * @hide
     */
    public ArrayList<String> divideMessage(String text, int encodingType) {
        Rlog.d(TAG, "divideMessage, encoding = " + encodingType);
        ArrayList<String> ret = MtkSmsMessage.fragmentText(text, encodingType);
        Rlog.d(TAG, "divideMessage: size = " + ret.size());
        return ret;
    }

    /**
     * insert a text SMS to the ICC.
     *
     * @param subId subscription identity
     * @param scAddress Service center address
     * @param address   Destination address or original address
     * @param text      List of message text
     * @param status    message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *                  STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param timestamp Timestamp when service center receive the message
     * @return MtkSimSmsInsertStatus
     * @hide
     */
    public MtkSimSmsInsertStatus insertTextMessageToIccCard(String scAddress, String address,
            List<String> text, int status, long timestamp) {
        Rlog.d(TAG, "insertTextMessageToIccCard");
        MtkSimSmsInsertStatus ret = null;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                ret = iccISms.insertTextMessageToIccCardForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName(), scAddress, address, text, status,
                        timestamp);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        Rlog.d(TAG, (ret != null) ? "insert Text " + ret.indexInIcc : "insert Text null");
        return ret;

    }

    /**
     * Copy a raw SMS PDU to the ICC.
     *
     * @param subId subscription identity
     * @param status message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *               STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param pdu the raw PDU to store
     * @param smsc encoded smsc service center
     * @return MtkSimSmsInsertStatus
     * @hide
     */
    public MtkSimSmsInsertStatus insertRawMessageToIccCard(int status, byte[] pdu, byte[] smsc) {
        Rlog.d(TAG, "insertRawMessageToIccCard");
        MtkSimSmsInsertStatus ret = null;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                ret = iccISms.insertRawMessageToIccCardForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName(), status, pdu, smsc);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        Rlog.d(TAG, (ret != null) ? "insert Raw " + ret.indexInIcc : "insert Raw null");
        return ret;
    }

    /**
     * Send an SMS with specified encoding type.
     *
     * @param subId subscription identity
     * @param destAddr the address to send the message to
     * @param scAddr the SMSC to send the message through, or NULL for the
     *  default SMSC
     * @param text the body of the message to send
     * @param extraParams extra parameters, such as validity period, encoding type
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is sucessfully sent, or failed.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     * @hide
     */
    // Deprecated
    // Please use SmsMananger.sendTextMessage already support valid periot
    public void sendTextMessageWithExtraParams(String destAddr, String scAddr, String text,
            Bundle extraParams, PendingIntent sentIntent, PendingIntent deliveryIntent) {
        Rlog.d(TAG, "sendTextMessageWithExtraParams");
        if (TextUtils.isEmpty(destAddr)) {
            throw new IllegalArgumentException("Invalid destinationAddress");
        }

        if (!isValidParameters(destAddr, text, sentIntent)) {
            return;
        }

        if (extraParams == null) {
            Rlog.d(TAG, "bundle is null");
            return;
        }

        // MTK-START
        //Context context = ActivityThread.currentApplication().getApplicationContext();
        //if (mDataOnlySmsFwkExt != null && mDataOnlySmsFwkExt.is4GDataOnlyMode(
        //        sentIntent, getSubscriptionId(), context)) {
        //    Rlog.d(TAG, "is4GDataOnlyMode");
        //    return;
        //}
        // MTK-END

        try {
            IMtkSms iccISms = getIMtkSmsServiceOrThrow();
            if (iccISms != null) {
                iccISms.sendTextWithExtraParamsForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName(), destAddr, scAddr, text, extraParams,
                        sentIntent, deliveryIntent, true /*persistMessageForCarrierApp*/);
            }
        } catch (RemoteException e) {
            Rlog.d(TAG, "RemoteException");
        }

    }

    /**
     * Send a multi-part text based SMS with specified encoding type.
     *
     * @param subId subscription identity
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *   the current default SMSC
     * @param parts an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param extraParams extra parameters, such as validity period, encoding type
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     *
     * @internal
     * @hide
     */
    // Deprecated
    // SmsMananger.sendMultipartTextMessage already support valid periot
    public void sendMultipartTextMessageWithExtraParams(String destAddr, String scAddr,
            ArrayList<String> parts, Bundle extraParams, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents) {
        Rlog.d(TAG, "sendMultipartTextMessageWithExtraParams");
        if (TextUtils.isEmpty(destAddr)) {
            throw new IllegalArgumentException("Invalid destinationAddress");
        }

        if (!isValidParameters(destAddr, parts, sentIntents)) {
            return;
        }

        if (extraParams == null) {
            Rlog.d(TAG, "bundle is null");
            return;
        }
        // MTK-START
        //Context context = ActivityThread.currentApplication().getApplicationContext();
        //if (mDataOnlySmsFwkExt != null && mDataOnlySmsFwkExt.is4GDataOnlyMode(
        //        sentIntents, getSubscriptionId(), context)) {
        //    Rlog.d(TAG, "is4GDataOnlyMode");
        //    return;
        //}
        // MTK-END

        if (parts != null && parts.size() > 1) {
            try {
                IMtkSms iccISms = getIMtkSmsServiceOrThrow();
                if (iccISms != null) {
                    iccISms.sendMultipartTextWithExtraParamsForSubscriber(getSubscriptionId(),
                            ActivityThread.currentPackageName(), destAddr, scAddr, parts,
                            extraParams, sentIntents, deliveryIntents,
                            true /*persistMessageForCarrierApp*/);
                }
            } catch (RemoteException e) {
                Rlog.d(TAG, "RemoteException");
            }
        } else {
            PendingIntent sentIntent = null;
            PendingIntent deliveryIntent = null;
            if (sentIntents != null && sentIntents.size() > 0) {
                sentIntent = sentIntents.get(0);
            }
            if (deliveryIntents != null && deliveryIntents.size() > 0) {
                deliveryIntent = deliveryIntents.get(0);
            }

            String text = (parts == null || parts.size() == 0) ? "" : parts.get(0);
            sendTextMessageWithExtraParams(destAddr, scAddr, text, extraParams, sentIntent,
                    deliveryIntent);
        }
    }

    /**
     * Get SMS paramter from icc cards
     *
     * @return <code>MtkSmsParameters</code> object for sms sim card settings.
     *
     * @hide
     */
    public MtkSmsParameters getSmsParameters() {
        Rlog.d(TAG, "getSmsParameters");

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                return iccISms.getSmsParametersForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName());
            } else {
                return null;
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        Rlog.d(TAG, "fail to get MtkSmsParameters");
        return null;

    }

    /**
     * Set sms paramter icc cards.
     *
     * @param params <code>MtkSmsParameters</code>.
     *
     * @return true set complete; false set failed.
     *
     * @hide
     */
    public boolean setSmsParameters(MtkSmsParameters params) {
        Rlog.d(TAG, "setSmsParameters");

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                return iccISms.setSmsParametersForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName(), params);
            } else {
                return false;
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        return false;

    }

    /**
     * Copy SMS to Icc cards.
     *
     * @param smsc service message centers address
     * @param pdu sms pdu
     * @param status sms status
     *
     * @return copied index on Icc cards
     *
     * @hide
     */
    public int copySmsToIcc(byte[] smsc, byte[] pdu, int status) {
        Rlog.d(TAG, "copySmsToIcc");

        MtkSimSmsInsertStatus smsStatus = insertRawMessageToIccCard(status, pdu, smsc);
        if (smsStatus == null) {
            return -1;
        }
        int[] index = smsStatus.getIndex();

        if (index != null && index.length > 0) {
            return index[0];
        }

        return -1;
    }

    /**
     * Update sms status on icc card.
     *
     * @param index updated index of sms on icc card
     * @param read read status
     *
     * @return true updated successful; false updated failed.
     *
     * @hide
     */
    public boolean updateSmsOnSimReadStatus(int index, boolean read) {
        Rlog.d(TAG, "updateSmsOnSimReadStatus");
        SmsRawData record = null;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                record = iccISms.getMessageFromIccEfForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName(), index);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        if (record != null) {
            byte[] rawData = record.getBytes();
            int status = rawData[0] & 0xff;
            Rlog.d(TAG, "sms status is " + status);
            if (status != SmsManager.STATUS_ON_ICC_UNREAD &&
                    status != SmsManager.STATUS_ON_ICC_READ) {
                Rlog.d(TAG, "non-delivery sms " + status);
                return false;
            } else {
                if ((status == SmsManager.STATUS_ON_ICC_UNREAD && read == false)
                        || (status == SmsManager.STATUS_ON_ICC_READ && read == true)) {
                    Rlog.d(TAG, "no need to update status");
                    return true;
                } else {
                    Rlog.d(TAG, "update sms status as " + read);
                    int newStatus = ((read == true) ? SmsManager.STATUS_ON_ICC_READ
                            : SmsManager.STATUS_ON_ICC_UNREAD);
                    SmsManager manager = SmsManager.getSmsManagerForSubscriptionId(mSubId);
                    boolean success = manager.updateMessageOnIcc(index,
                            SmsManager.STATUS_ON_ICC_READ, rawData);
                    return success;
                }
            }
        } // end if(record != null)

        Rlog.d(TAG, "record is null");

        return false;
    }

    /**
     * Set the memory storage status of the SMS.
     * This function is used for FTA test only.
     *
     * @param subId subscription identity
     * @param status false for storage full, true for storage available
     *
     * @internal
     * @hide
     */
    public void setSmsMemoryStatus(boolean status) {
        Rlog.d(TAG, "setSmsMemoryStatus");

        try {
            IMtkSms iccISms = getIMtkSmsServiceOrThrow();
            if (iccISms != null) {
                iccISms.setSmsMemoryStatusForSubscriber(getSubscriptionId(), status);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }
    }

    /**
     * Get SMS SIM Card memory's total and used number.
     *
     * @param subId subscription identity
     *
     * @return <code>MtkIccSmsStorageStatus</code> object
     *
     * @internal
     * @hide
     */
    public MtkIccSmsStorageStatus getSmsSimMemoryStatus() {
        Rlog.d(TAG, "getSmsSimMemoryStatus");

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                return iccISms.getSmsSimMemoryStatusForSubscriber(getSubscriptionId(),
                        ActivityThread.currentPackageName());
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        return null;
    }

    // Utilities functions
    /**
     * Judge if the destination address is a valid SMS address or not, and if
     * the text is null or not
     *
     * @destinationAddress the destination address to which the message be sent
     * @text the content of shorm message
     * @sentIntent will be broadcast if the address or the text is invalid
     * @return true for valid parameters
     */
    private static boolean isValidParameters(String destinationAddress, String text,
            PendingIntent sentIntent) {
        ArrayList<PendingIntent> sentIntents =
                new ArrayList<PendingIntent>();
        ArrayList<String> parts =
                new ArrayList<String>();

        sentIntents.add(sentIntent);
        parts.add(text);

        // if (TextUtils.isEmpty(text)) {
        // throw new IllegalArgumentException("Invalid message body");
        // }

        return isValidParameters(destinationAddress, parts, sentIntents);
    }

    /**
     * Judges if the destination address is a valid SMS address or not, and if
     * the text is null or not.
     *
     * @param destinationAddress The destination address to which the message be sent
     * @param parts The content of shorm message
     * @param sentIntent will be broadcast if the address or the text is invalid
     * @return True for valid parameters
     */
    private static boolean isValidParameters(String destinationAddress, ArrayList<String> parts,
            ArrayList<PendingIntent> sentIntents) {
        if (parts == null || parts.size() == 0) {
            return true;
        }

        if (!isValidSmsDestinationAddress(destinationAddress)) {
            for (int i = 0; i < sentIntents.size(); i++) {
                PendingIntent sentIntent = sentIntents.get(i);
                if (sentIntent != null) {
                    try {
                        sentIntent.send(SmsManager.RESULT_ERROR_GENERIC_FAILURE);
                    } catch (CanceledException ex) { }
                }
            }

            Rlog.d(TAG, "Invalid destinationAddress");
            return false;
        }

        if (TextUtils.isEmpty(destinationAddress)) {
            throw new IllegalArgumentException("Invalid destinationAddress");
        }
        if (parts == null || parts.size() < 1) {
            throw new IllegalArgumentException("Invalid message body");
        }

        return true;
    }

    /**
     * judge if the input destination address is a valid SMS address or not
     *
     * @param da the input destination address
     * @return true for success
     *
     */
    private static boolean isValidSmsDestinationAddress(String da) {
        String encodeAddress = PhoneNumberUtils.extractNetworkPortion(da);
        if (encodeAddress == null)
            return true;

        return !(encodeAddress.isEmpty());
    }

    /**
     * Create a list of <code>SmsMessage</code>s from a list of RawSmsData
     * records returned by <code>getAllMessagesFromIcc()</code>.
     *
     * @param subId subscription identity
     * @param records SMS EF records, returned by
     *            <code>getAllMessagesFromIcc</code>
     *
     * @return <code>ArrayList</code> of <code>SmsMessage</code> objects.
     */
    private static ArrayList<MtkSmsMessage> createMessageListFromRawRecordsByMode(int subId,
            List<SmsRawData> records, int mode) {
        Rlog.d(TAG, "createMessageListFromRawRecordsByMode");

        ArrayList<MtkSmsMessage> msg = null;
        if (records != null) {
            int count = records.size();
            msg = new ArrayList<MtkSmsMessage>();

            for (int i = 0; i < count; i++) {
                SmsRawData data = records.get(i);

                if (data != null) {
                    MtkSmsMessage singleSms =
                            createFromEfRecordByMode(subId, i + 1, data.getBytes(), mode);
                    if (singleSms != null) {
                        msg.add(singleSms);
                    }
                }
            }
            Rlog.d(TAG, "actual sms count is " + msg.size());
        } else {
            Rlog.d(TAG, "fail to parse SIM sms, records is null");
        }

        return msg;
    }

    /**
     * Create an SmsMessage from an SMS EF record.
     *
     * @param index Index of SMS record. This should be index in ArrayList
     *              returned by SmsManager.getAllMessagesFromSim + 1.
     * @param data Record data.
     * @param slotId SIM card the user would like to access
     * @return An SmsMessage representing the record.
     *
     */
    private static MtkSmsMessage createFromEfRecordByMode(int subId, int index, byte[] data,
            int mode) {
        MtkSmsMessage sms = null;

        if (mode == PhoneConstants.PHONE_TYPE_CDMA) {
            //sms = SmsMessage.createFromEfRecord(index, data, SmsConstants.FORMAT_3GPP2);
        } else {
            sms = MtkSmsMessage.createFromEfRecord(index, data, SmsConstants.FORMAT_3GPP);
        }

        if (sms != null) {
            sms.setSubId(subId);
        }

        return sms;
    }

    /**
     * Get the associated subscription id. If the instance was returned by {@link #getDefault()},
     * then this method may return different values at different points in time (if the user
     * changes the default subscription id). It will return < 0 if the default subscription id
     * cannot be determined.
     *
     * Additionally, to support legacy applications that are not multi-SIM aware,
     * if the following are true:
     *     - We are using a multi-SIM device
     *     - A default SMS SIM has not been selected
     *     - At least one SIM subscription is available
     * then ask the user to set the default SMS SIM.
     *
     * @return associated subscription id
     */
    public int getSubscriptionId() {
        final int subId = (mSubId == DEFAULT_SUBSCRIPTION_ID)
                ? SmsManager.getDefaultSmsSubscriptionId() : mSubId;
        boolean isSmsSimPickActivityNeeded = false;
        final Context context = ActivityThread.currentApplication().getApplicationContext();
        try {
            ISms iccISms = getISmsService();
            if (iccISms != null) {
                isSmsSimPickActivityNeeded = iccISms.isSmsSimPickActivityNeeded(subId);
            }
        } catch (RemoteException ex) {
            Rlog.e(TAG, "Exception in getSubscriptionId");
        }

        // MTK-START
        // Mark since MTK have another SIM Card selection logic
        isSmsSimPickActivityNeeded = false;
        // MTK-END
        if (isSmsSimPickActivityNeeded) {
            Rlog.d(TAG, "getSubscriptionId isSmsSimPickActivityNeeded is true");
            // ask the user for a default SMS SIM.
            Intent intent = new Intent();
            intent.setClassName("com.android.settings",
                    "com.android.settings.sim.SimDialogActivity");
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra(DIALOG_TYPE_KEY, SMS_PICK);
            try {
                context.startActivity(intent);
            } catch (ActivityNotFoundException anfe) {
                // If Settings is not installed, only log the error as we do not want to break
                // legacy applications.
                Rlog.e(TAG, "Unable to launch Settings application.");
            }
        }

        return subId;
    }

    /**
     * Returns the IMtkSms service, or throws an UnsupportedOperationException if
     * the service does not exist.
     */
    private static IMtkSms getIMtkSmsServiceOrThrow() {
        IMtkSms iccISms = getIMtkSmsService();
        if (iccISms == null) {
            throw new UnsupportedOperationException("SmsEx is not supported");
        }
        return iccISms;
    }

    private static IMtkSms getIMtkSmsService() {
        return IMtkSms.Stub.asInterface(ServiceManager.getService("imtksms"));
    }

    /**
     * Returns the ISms service, or throws an UnsupportedOperationException if
     * the service does not exist.
     */
    private static ISms getISmsServiceOrThrow() {
        ISms iccISms = getISmsService();
        if (iccISms == null) {
            throw new UnsupportedOperationException("Sms is not supported");
        }
        return iccISms;
    }

    private static ISms getISmsService() {
        return ISms.Stub.asInterface(ServiceManager.getService("isms"));
    }

    /**
     * Query if cell broadcast activation.
     *
     * @return true activatd; false deactivated.
     */
    public boolean queryCellBroadcastSmsActivation() {
        Rlog.d(TAG, "queryCellBroadcastSmsActivation");
        Rlog.d(TAG, "subId=" + getSubscriptionId());
        boolean result = false;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                result = iccISms.queryCellBroadcastSmsActivationForSubscriber(
                        getSubscriptionId());
            } else {
                Rlog.d(TAG, "fail to get sms service");
                result = false;
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException!");
        }

        return result;
    }

    /**
     * To activate the cell broadcast.
     *
     * @param activate true activation; false de-activation.
     *
     * @return true process successfully; false process failed.
     */
    public boolean activateCellBroadcastSms(boolean activate) {
        Rlog.d(TAG, "activateCellBroadcastSms activate : " + activate + ", sub = " +
                getSubscriptionId());
        boolean result = false;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                result = iccISms.activateCellBroadcastSmsForSubscriber(getSubscriptionId(),
                        activate);
            } else {
                Rlog.d(TAG, "fail to get sms service, maybe phone is initializing");
                result = false;
            }
        } catch (RemoteException e) {
            Rlog.d(TAG, "fail to activate CB");
            result = false;
        }

        return result;
    }

    /**
     * Remove specified channel and serial of cb message.
     *
     * @param channelId removed channel id
     * @param serialId removed serial id
     *
     * @return true process successfully; false process failed.
     */
    public boolean removeCellBroadcastMsg(int channelId, int serialId) {
        Rlog.d(TAG, "RemoveCellBroadcastMsg, subId=" + getSubscriptionId());
        boolean result = false;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                result = iccISms.removeCellBroadcastMsgForSubscriber(getSubscriptionId(),
                        channelId, serialId);
            } else {
                Rlog.d(TAG, "fail to get sms service");
                result = false;
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoveCellBroadcastMsg, RemoteException!");
        }

        return result;
    }

    /**
     * Query cell broadcast channel configuration
     *
     * @return channel configuration. e.g. "1,3,100-123"
     */
    public String getCellBroadcastRanges() {
        Rlog.d(TAG, "getCellBroadcastRanges, subId=" + getSubscriptionId());
        String configs = "";

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                configs = iccISms.getCellBroadcastRangesForSubscriber(getSubscriptionId());
            } else {
                Rlog.d(TAG, "fail to get sms service");
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        return configs;
    }

    /**
     * Set cell broadcast language configuration
     *
     * @param lang denotes language configuration. e.g. "1,7,5-29"
     */
    public boolean setCellBroadcastLang(String lang) {
        Rlog.d(TAG, "setCellBroadcastLang, subId=" + getSubscriptionId());
        boolean result = false;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                result = iccISms.setCellBroadcastLangsForSubscriber(getSubscriptionId(), lang);
            } else {
                Rlog.d(TAG, "fail to get sms service");
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        return result;
    }

    /**
     * Get cell broadcast language configuration
     *
     * @return language configuration. e.g. "1,2,3,32"
     */
    public String getCellBroadcastLang() {
        Rlog.d(TAG, "getCellBroadcastLang, subId=" + getSubscriptionId());
        String langs = "";

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                langs = iccISms.getCellBroadcastLangsForSubscriber(getSubscriptionId());
            } else {
                Rlog.d(TAG, "fail to get sms service");
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        return langs;
    }

    /**
     * Set Earthquake and Tsunami Warning System config to modem.
     *
     * @param mode ETWS config mode.
     *
     * @return true set ETWS config successful; false set ETWS config failed.
     */
    public boolean setEtwsConfig(int mode) {
        Rlog.d(TAG, "setEtwsConfig, mode=" + mode);
        boolean ret = false;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                ret = iccISms.setEtwsConfigForSubscriber(getSubscriptionId(), mode);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        return ret;
    }

    /**
     * Get SC address bundle key: result.
     */
    public static final String GET_SC_ADDRESS_KEY_RESULT = "errorCode";

    /**
     * Get SC address bundle key: scAddress.
     */
    public static final String GET_SC_ADDRESS_KEY_ADDRESS = "scAddress";

    /**
     * Error Code: success.
     * Now, it is only used by {@link #getScAddressWithErroCode(subId)}.
     */
    public static final byte ERROR_CODE_NO_ERROR = 0x00;

    /**
     * Error Code: generic error.
     * Now, it is only used by {@link #getScAddressWithErroCode(subId)}.
     */
    public static final byte ERROR_CODE_GENERIC_ERROR = 0x01;

    /**
     * Error Code: no sms center number support
     * Now, it is only used by {@link #getScAddressWithErroCode(subId)}.
     */
    public static final byte ERROR_CODE_NO_SUPPORT_SC_ADDR = 0x02;

    /**
     * Get service center address
     *
     * @param subId subscripiton identity
     *
     * @return Current service center address
     */
    public String getScAddress() {
        Rlog.d(TAG, "getScAddress");
        String address = null;
        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                address = iccISms.getScAddressForSubscriber(getSubscriptionId());
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }
        return address;
    }

    /**
     * Get service center address with error code.
     *
     * @param subId subscripiton identity
     *
     * @return Current service center address and error code by Bundle
     * The error code will be
     *     {@link #ERROR_CODE_NO_ERROR}
     *     {@link #ERROR_CODE_GENERIC_ERROR}
     */
    public Bundle getScAddressWithErroCode() {
        Rlog.d(TAG, "getScAddressWithErroCode");
        Bundle response = null;
        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                response = iccISms.getScAddressWithErrorCodeForSubscriber(getSubscriptionId());
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }
        return response;
    }

    /**
     * Set service center address
     *
     * @param subId subscripiton identity
     * @param address Address to be set
     *
     * @return True for success, false for failure
     */
    public boolean setScAddress(String address) {
        Rlog.d(TAG, "setScAddress");
        boolean ret = false;

        try {
            IMtkSms iccISms = getIMtkSmsService();
            if (iccISms != null) {
                ret = iccISms.setScAddressForSubscriber(getSubscriptionId(), address);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "RemoteException");
        }

        return ret;
    }

    /**
     * SMS over IMS is supported if IMS is registered and SMS is supported
     * on IMS. Because MtkSmsMessage.useCdmaFormatForMoSms will use the API and
     * we would like to call getSubscriptionId() here in order to prevent from SimDialogActivity
     * popup.
     *
     * @return true if SMS over IMS is supported, false otherwise
     *
     * @see #getImsSmsFormat()
     */
    public boolean isImsSmsSupported() {
        boolean boSupported = false;
        try {
            ISms iccISms = getISmsService();
            if (iccISms != null) {
                boSupported = iccISms.isImsSmsSupportedForSubscriber(getSubscriptionId());
                Rlog.d(TAG, "isImsSmsSupported " + boSupported);
            }
        } catch (RemoteException ex) {
            // ignore it
        }
        return boSupported;
    }

    /**
     * Gets SMS format supported on IMS.  SMS over IMS format is
     * either 3GPP or 3GPP2. Because MtkSmsMessage.useCdmaFormatForMoSms will use the API and
     * we would like to call getSubscriptionId() here in order to prevent from SimDialogActivity
     * popup.
     *
     * @return SmsMessage.FORMAT_3GPP, SmsMessage.FORMAT_3GPP2 or SmsMessage.FORMAT_UNKNOWN
     *
     * @see #isImsSmsSupported()
     */
    public String getImsSmsFormat() {
        String format = com.android.internal.telephony.SmsConstants.FORMAT_UNKNOWN;
        try {
            ISms iccISms = getISmsService();
            if (iccISms != null) {
                format = iccISms.getImsSmsFormatForSubscriber(getSubscriptionId());
                Rlog.d(TAG, "getImsSmsFormat " + format);
            }
        } catch (RemoteException ex) {
            // ignore it
        }
        return format;
    }

    /**
     * Divide a message text into several fragments, none bigger than
     * the maximum SMS message size.
     *
     * @param text the original message.  Must not be null.
     * @return an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     *
     * @throws IllegalArgumentException if text is null
     */
    public ArrayList<String> divideMessage(String text) {
        if (null == text) {
            throw new IllegalArgumentException("text is null");
        }
        return MtkSmsMessage.fragmentText(text);
    }

    /**
     * Send a data based SMS to a specific application port.
     *
     * <p class="note"><strong>Note:</strong> Using this method requires that your app has the
     * {@link android.Manifest.permission#SEND_SMS} permission.</p>
     *
     * @param destinationAddress the address to send the message to
     * @param scAddress is the service center address or null to use
     *  the current default SMSC
     * @param destinationPort the port to deliver the message to
     * @param data the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is successfully sent, or failed.
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
     *
     * @throws IllegalArgumentException if destinationAddress or data are empty
     */
    public void sendDataMessage(
            String destinationAddress, String scAddress, short destinationPort,
            byte[] data, PendingIntent sentIntent, PendingIntent deliveryIntent) {
        if (TextUtils.isEmpty(destinationAddress)) {
            throw new IllegalArgumentException("Invalid destinationAddress");
        }

        if (data == null || data.length == 0) {
            throw new IllegalArgumentException("Invalid message data");
        }

        try {
            ISms iccISms = getISmsServiceOrThrow();
            iccISms.sendDataForSubscriber(getSubscriptionId(), ActivityThread.currentPackageName(),
                    destinationAddress, scAddress, destinationPort & 0xFFFF,
                    data, sentIntent, deliveryIntent);
        } catch (RemoteException ex) {
            // ignore it
        }
    }

    /**
     * A variant of {@link SmsManager#sendDataMessage} that allows self to be the caller. This is
     * for internal use only.
     *
     * @hide
     */
    public void sendDataMessageWithSelfPermissions(
            String destinationAddress, String scAddress, short destinationPort,
            byte[] data, PendingIntent sentIntent, PendingIntent deliveryIntent) {
        if (TextUtils.isEmpty(destinationAddress)) {
            throw new IllegalArgumentException("Invalid destinationAddress");
        }

        if (data == null || data.length == 0) {
            throw new IllegalArgumentException("Invalid message data");
        }

        try {
            ISms iccISms = getISmsServiceOrThrow();
            iccISms.sendDataForSubscriberWithSelfPermissions(getSubscriptionId(),
                    ActivityThread.currentPackageName(), destinationAddress, scAddress,
                    destinationPort & 0xFFFF, data, sentIntent, deliveryIntent);
        } catch (RemoteException ex) {
            // ignore it
        }
    }

    public static boolean checkSimPickActivityNeeded(boolean needed) {
        return false;
    }
}
