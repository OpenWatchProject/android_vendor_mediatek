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

import android.annotation.Nullable;
import android.app.ActivityThread;
import android.app.PendingIntent;
import android.content.Context;
import android.net.Uri;
import android.os.Binder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Telephony.Sms.Intents;
import android.telephony.Rlog;
import android.telephony.SmsManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.SmsRawData;
import com.android.internal.telephony.IccSmsInterfaceManager;
import com.android.internal.telephony.SubscriptionController;

import java.util.List;

// MTK-START
import android.os.Bundle;
import mediatek.telephony.MtkSmsParameters;
import com.mediatek.internal.telephony.MtkIccSmsStorageStatus;
import mediatek.telephony.MtkSimSmsInsertStatus;
import static android.telephony.SmsManager.RESULT_ERROR_GENERIC_FAILURE;
// MTK-END

/**
 * UiccSmsController to provide an inter-process communication to
 * access Sms in Icc.
 */
public class MtkUiccSmsController extends IMtkSms.Stub {
    static final String LOG_TAG = "Mtk_RIL_UiccSmsController";

    protected Phone[] mPhone;

    protected MtkUiccSmsController(Phone[] phone){
        mPhone = phone;

        if (ServiceManager.getService("imtksms") == null) {
            ServiceManager.addService("imtksms", this);
        }
    }

    private void sendErrorInPendingIntent(@Nullable PendingIntent intent, int errorCode) {
        if (intent != null) {
            try {
                intent.send(errorCode);
            } catch (PendingIntent.CanceledException ex) {
            }
        }
    }

    private void sendErrorInPendingIntents(List<PendingIntent> intents, int errorCode) {
        for (PendingIntent intent : intents) {
            sendErrorInPendingIntent(intent, errorCode);
        }
    }

    /*
     * @return true if the subId is active.
     */
    private boolean isActiveSubId(int subId) {
        return SubscriptionController.getInstance().isActiveSubId(subId);
    }

     /**
     * get sms interface manager object based on subscription.
     **/
    private @Nullable MtkIccSmsInterfaceManager getIccSmsInterfaceManager(int subId) {
        if (!isActiveSubId(subId)) {
            Rlog.e(LOG_TAG, "Subscription " + subId + " is inactive.");
            return null;
        }

        int phoneId = SubscriptionController.getInstance().getPhoneId(subId) ;
        //Fixme: for multi-subscription case
        if (!SubscriptionManager.isValidPhoneId(phoneId)
                || phoneId == SubscriptionManager.DEFAULT_PHONE_INDEX) {
            phoneId = 0;
        }

        try {
            return (MtkIccSmsInterfaceManager)
                ((Phone)mPhone[(int)phoneId]).getIccSmsInterfaceManager();
        } catch (NullPointerException e) {
            Rlog.e(LOG_TAG, "Exception is :"+e.toString()+" For subscription :"+subId );
            e.printStackTrace();
            return null;
        } catch (ArrayIndexOutOfBoundsException e) {
            Rlog.e(LOG_TAG, "Exception is :"+e.toString()+" For subscription :"+subId );
            e.printStackTrace();
            return null;
        }
    }

    /**
     * Retrieves all messages currently stored on ICC based on different mode.
     * Ex. CDMA mode or GSM mode for international cards.
     * @param subId the subId id.
     * @param callingPackage the calling packages
     * @param mode the GSM mode or CDMA mode
     *
     * @return list of SmsRawData of all sms on ICC
     */
    public List<SmsRawData> getAllMessagesFromIccEfByModeForSubscriber(int subId,
            String callingPackage, int mode) {
        // MTK-START
        if (!isSmsReadyForSubscriber(subId)) {
            Rlog.e(LOG_TAG, "getAllMessagesFromIccEf SMS not ready");
            return null;
        }
        // MTK-END

        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.getAllMessagesFromIccEfByMode(callingPackage, mode);
        } else {
            Rlog.e(LOG_TAG, "getAllMessagesFromIccEfByModeForSubscriber iccSmsIntMgr is null for" +
                          " Subscription: " + subId);
        }
        return null;
    }

    /**
     * Copy a text SMS to the ICC.
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
     * @param scAddress Service center address
     * @param address   Destination address or original address
     * @param text      List of message text
     * @param status    message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *                  STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param timestamp Timestamp when service center receive the message
     * @return success or not
     *
     */
    public int copyTextMessageToIccCardForSubscriber(int subId, String callingPackage,
            String scAddress, String address, List<String> text, int status, long timestamp) {
        int result = RESULT_ERROR_GENERIC_FAILURE;
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            result = iccSmsIntMgr.copyTextMessageToIccCard(callingPackage, scAddress, address, text,
                    status, timestamp);
        } else {
            Rlog.e(LOG_TAG, "sendStoredMultipartText iccSmsIntMgr is null for subscription: "
                    + subId);
        }

        return result;
    }

    /**
     * Send a data message with original port
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
     * @param destAddr the destination address
     * @param scAddr the SMSC to send the message through, or NULL for the
     *  default SMSC
     * @param destPort destination port
     * @param originalPort origianl sender port
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
     */
    public void sendDataWithOriginalPortForSubscriber(int subId, String callingPackage,
            String destAddr, String scAddr, int destPort, int originalPort, byte[] data,
            PendingIntent sentIntent, PendingIntent deliveryIntent) {
        sendDataInternal(subId, callingPackage, destAddr,
                scAddr, destPort, originalPort, data, sentIntent, deliveryIntent, true);
    }

    /**
     * Send a data message with original port without permission check.
     *
     * @param subId subscription identity
     * @param destAddr the destination address
     * @param scAddr the SMSC to send the message through, or NULL for the
     *  default SMSC
     * @param destPort destination port
     * @param originalPort origianl sender port
     * @param data the body of the message to send
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
     *  is NULL the caller will be checked against all unknown applicaitons,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     */
    public void sendData(int subId,
            String destAddr, String scAddr, int destPort, int originalPort, byte[] data,
            PendingIntent sentIntent, PendingIntent deliveryIntent) {
        sendDataInternal(subId, ActivityThread.currentPackageName(), destAddr,
                scAddr, destPort, originalPort, data, sentIntent, deliveryIntent, false);
    }

    private void sendDataInternal(int subId, String callingPackage,
            String destAddr, String scAddr, int destPort, int originalPort, byte[] data,
            PendingIntent sentIntent, PendingIntent deliveryIntent, boolean checkPermission) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            iccSmsIntMgr.sendDataWithOriginalPort(callingPackage, destAddr, scAddr, destPort,
                    originalPort, data, sentIntent, deliveryIntent, checkPermission);
        } else {
            Rlog.e(LOG_TAG, "sendDataInternal iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }
    }

    /**
     * Judge if SMS subsystem is ready or not
     *
     * @param subId subscription identity
     *
     * @return true for success
     */
    public boolean isSmsReadyForSubscriber(int subId) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.isSmsReady();
        } else {
            Rlog.e(LOG_TAG, "isSmsReady iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return false;
    }

    /**
     * Set the memory storage status of the SMS.
     * This function is used for FTA test only
     *
     * @param subId subscription identity
     * @param status false for storage full, true for storage available
     *
     */
    public void setSmsMemoryStatusForSubscriber(int subId, boolean status) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            iccSmsIntMgr.setSmsMemoryStatus(status);
        } else {
            Rlog.e(LOG_TAG, "setSmsMemoryStatus iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }
    }

    /**
     * Get SMS SIM Card memory's total and used number
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
     *
     * @return <code>MtkIccSmsStorageStatus</code> object
     */
    public MtkIccSmsStorageStatus getSmsSimMemoryStatusForSubscriber(int subId,
            String callingPackage) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.getSmsSimMemoryStatus(callingPackage);
        } else {
            Rlog.e(LOG_TAG, "setSmsMemoryStatus iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return null;
    }

    /**
     * Send an SMS with specified encoding type.
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
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
     * @param persistMessageForNonDefaultSmsApp whether to save the sent message into SMS DB for a
     *   non-default SMS app.
     */
    public void sendTextWithEncodingTypeForSubscriber(int subId, String callingPackage,
            String destAddr, String scAddr, String text, int encodingType, PendingIntent sentIntent,
            PendingIntent deliveryIntent, boolean persistMessageForNonDefaultSmsApp) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            iccSmsIntMgr.sendTextWithEncodingType(callingPackage, destAddr, scAddr, text,
                    encodingType, sentIntent, deliveryIntent, persistMessageForNonDefaultSmsApp);
        } else {
            Rlog.e(LOG_TAG, "sendTextWithEncodingTypeForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);

            sendErrorInPendingIntent(sentIntent, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
        }
    }

    /**
     * Send a multi-part text based SMS with specified encoding type.
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
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
     * @param persistMessageForNonDefaultSmsApp whether to save the sent message into SMS DB for a
     *   non-default SMS app.
     */
    public void sendMultipartTextWithEncodingTypeForSubscriber(int subId, String callingPackage,
            String destAddr, String scAddr, List<String> parts, int encodingType,
            List<PendingIntent> sentIntents, List<PendingIntent> deliveryIntents,
            boolean persistMessageForNonDefaultSmsApp) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            iccSmsIntMgr.sendMultipartTextWithEncodingType(callingPackage, destAddr, scAddr, parts,
                    encodingType, sentIntents, deliveryIntents, persistMessageForNonDefaultSmsApp);
        } else {
            Rlog.e(LOG_TAG, "sendMultipartTextWithEncodingTypeForSubscriber iccSmsIntMgr is null"
                     + " for subscription: " + subId);

            sendErrorInPendingIntents(sentIntents, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
        }
    }

    /**
     * Copy a text SMS to the ICC.
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
     * @param scAddress Service center address
     * @param address   Destination address or original address
     * @param text      List of message text
     * @param status    message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *                  STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param timestamp Timestamp when service center receive the message
     * @return MtkSimSmsInsertStatus
     *
     */
    public MtkSimSmsInsertStatus insertTextMessageToIccCardForSubscriber(int subId,
            String callingPackage, String scAddress, String address, List<String> text, int status,
            long timestamp) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            iccSmsIntMgr.insertTextMessageToIccCard(callingPackage, scAddress, address, text,
                    status, timestamp);
        } else {
            Rlog.e(LOG_TAG, "sendMultipartTextWithEncodingTypeForSubscriber iccSmsIntMgr is null"
                    + " for subscription: " + subId);
        }

        return null;
    }

    /**
     * Copy a raw SMS PDU to the ICC.
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
     * @param status message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *               STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param pdu the raw PDU to store
     * @param smsc encoded smsc service center
     * @return MtkSimSmsInsertStatus
     *
     */
    public MtkSimSmsInsertStatus insertRawMessageToIccCardForSubscriber(int subId,
            String callingPackage, int status, byte[] pdu, byte[] smsc) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);

        MtkSimSmsInsertStatus ret = null;
        if (iccSmsIntMgr != null) {
            ret = iccSmsIntMgr.insertRawMessageToIccCard(callingPackage, status, pdu, smsc);
        } else {
            Rlog.e(LOG_TAG, "insertRawMessageToIccCardForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return ret;
    }

    /**
     * Send an SMS with specified encoding type.
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
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
     * @param persistMessageForNonDefaultSmsApp whether to save the sent message into SMS DB for a
     *   non-default SMS app.
     */
    public void sendTextWithExtraParamsForSubscriber(int subId, String callingPackage,
            String destAddr, String scAddr, String text, Bundle extraParams,
            PendingIntent sentIntent, PendingIntent deliveryIntent,
            boolean persistMessageForNonDefaultSmsApp) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            iccSmsIntMgr.sendTextWithExtraParams(callingPackage, destAddr, scAddr, text,
                    extraParams, sentIntent, deliveryIntent, persistMessageForNonDefaultSmsApp);
        } else {
            Rlog.e(LOG_TAG, "sendTextWithExtraParamsForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);

            sendErrorInPendingIntent(sentIntent, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
        }
    }

    /**
     * Send a multi-part text based SMS with specified encoding type.
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
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
     * @param persistMessageForNonDefaultSmsApp whether to save the sent message into SMS DB for a
     *   non-default SMS app.
     */
    public void sendMultipartTextWithExtraParamsForSubscriber(int subId, String callingPackage,
            String destAddr, String scAddr, List<String> parts, Bundle extraParams,
            List<PendingIntent> sentIntents, List<PendingIntent> deliveryIntents,
            boolean persistMessageForNonDefaultSmsApp) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            iccSmsIntMgr.sendMultipartTextWithExtraParams(callingPackage, destAddr, scAddr, parts,
                    extraParams, sentIntents, deliveryIntents, persistMessageForNonDefaultSmsApp);
        } else {
            Rlog.e(LOG_TAG, "sendTextWithExtraParamsForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);

            sendErrorInPendingIntents(sentIntents, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
        }
    }

    /**
     * Get sms parameters from EFsmsp.
     * Such as the validity period & its format, Protocol identifier and decode char set value.
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
     *
     * @return sms parameter stored on EF like valid period
     */
    public MtkSmsParameters getSmsParametersForSubscriber(int subId, String callingPackage) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.getSmsParameters(callingPackage);
        } else {
            Rlog.e(LOG_TAG, "getSmsParametersForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return null;
    }

    /**
     * Save sms parameters into EFsmsp.
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
     * @param params sms EFsmsp values
     *
     * @return true if set completed; false if set failed
     */
    public boolean setSmsParametersForSubscriber(int subId, String callingPackage,
            MtkSmsParameters params) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.setSmsParameters(callingPackage, params);
        } else {
            Rlog.e(LOG_TAG, "setSmsParametersForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return false;
    }

    /**
     * Retrieves message currently stored on ICC by index.
     *
     * @param subId subscription identity
     * @param callingPackage the calling packages
     * @param index the index of sms save in EFsms
     *
     * @return SmsRawData of sms on ICC
     */
    public SmsRawData getMessageFromIccEfForSubscriber(int subId, String callingPackage,
            int index) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.getMessageFromIccEf(callingPackage, index);
        } else {
            Rlog.e(LOG_TAG, "getMessageFromIccEfForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return null;
    }

    /**
     * Query the activation status of cell broadcast.
     *
     * @param subId subscription identity
     *
     * @return true if activate; false if inactivate.
     */
    public boolean queryCellBroadcastSmsActivationForSubscriber(int subId) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.queryCellBroadcastSmsActivation();
        } else {
            Rlog.e(LOG_TAG, "setCellBroadcastSmsConfigForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return false;
    }

    /**
     * Activate or deactivate cell broadcast SMS.
     *
     * @param subId subscription identity
     * @param activate 0 = activate, 1 = deactivate
     *
     * @return true if activate successfully; false if activate failed
     */
    public boolean activateCellBroadcastSmsForSubscriber(int subId, boolean activate) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.activateCellBroadcastSms(activate);
        } else {
            Rlog.e(LOG_TAG, "activateCellBroadcastSmsForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return false;
    }

    /**
     * Remove specified channel and serial of cb message.
     *
     * @param subId subscription identity
     * @param channelId removed channel id
     * @param serialId removed serial id
     *
     * @return true process successfully; false process failed.
     *
     */
    public boolean removeCellBroadcastMsgForSubscriber(int subId, int channelId, int serialId) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.removeCellBroadcastMsg(channelId, serialId);
        } else {
            Rlog.e(LOG_TAG, "removeCellBroadcastMsg iccSmsIntMgr is null for subscription: "
                    + subId);
        }

        return false;
    }

    /**
     * Set the specified (Earthquake and Tsunami Warning System) mode to modem.
     *
     * @param subId subscription identity
     * @param mode a bit mask value. bit0: enable ETWS. bit1: enable receiving ETWS with security
     *         check. bit2: enable receiving test purpose ETWS
     *
     * @return true process successfully; false process failed.
     */
    public boolean setEtwsConfigForSubscriber(int subId, int mode) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.setEtwsConfig(mode);
        } else {
            Rlog.e(LOG_TAG, "setEtwsConfigForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return false;
    }

    /**
     * Query cell broadcast channel configuration.
     *
     * @param subId subscription identity
     *
     * @return channel configuration. e.g. "1,3,100-123".
     */
    public String getCellBroadcastRangesForSubscriber(int subId) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.getCellBroadcastRanges();
        } else {
            Rlog.e(LOG_TAG, "getCellBroadcastRangesForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return "";
    }

    /**
     * Set cell broadcast language configuration
     *
     * @param subId subscription identity
     * @param lang denotes language configuration. e.g. "1,7,5-29"
     *
     */
    public boolean setCellBroadcastLangsForSubscriber(int subId, String lang) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.setCellBroadcastLangs(lang);
        } else {
            Rlog.e(LOG_TAG, "setCellBroadcastLangsForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return false;
    }

    /**
     * Get cell broadcast language configuration
     *
     * @return language configuration. e.g. "1,2,3,32"
     *
     */
    public String getCellBroadcastLangsForSubscriber(int subId) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.getCellBroadcastLangs();
        } else {
            Rlog.e(LOG_TAG, "getCellBroadcastLangsForSubscriber iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return "";
    }

    /**
     * Get service center address
     *
     * @param subId subscription identity
     *
     * @return service message center address
     */
    public String getScAddressForSubscriber(int subId) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.getScAddress();
        } else {
            Rlog.e(LOG_TAG, "getScAddress iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return null;
    }

    /**
     * Get service center address
     *
     * @param subId subscription identity
     *
     * @return service message center address and error code
     */
    public Bundle getScAddressWithErrorCodeForSubscriber(int subId) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.getScAddressWithErrorCode();
        } else {
            Rlog.e(LOG_TAG, "getScAddressWithErrorCode iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return null;
    }

    /**
     * Set service message center address
     *
     * @param subId subscription identity
     * @param address service message center addressto be set
     *
     * @return true for success, false for failure
     */
    public boolean setScAddressForSubscriber(int subId, String address) {
        MtkIccSmsInterfaceManager iccSmsIntMgr = getIccSmsInterfaceManager(subId);
        if (iccSmsIntMgr != null) {
            return iccSmsIntMgr.setScAddress(address);
        } else {
            Rlog.e(LOG_TAG, "setScAddress iccSmsIntMgr is null for" +
                    "subscription: " + subId);
        }

        return false;
    }
}
