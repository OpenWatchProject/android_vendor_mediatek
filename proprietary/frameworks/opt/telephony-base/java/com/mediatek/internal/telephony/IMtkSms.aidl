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

import android.app.PendingIntent;
import android.net.Uri;
import com.android.internal.telephony.SmsRawData;
import com.mediatek.internal.telephony.MtkIccSmsStorageStatus;
import mediatek.telephony.MtkSimSmsInsertStatus;
import android.os.Bundle;
import mediatek.telephony.MtkSmsParameters;

/** Interface for applications to manage MTK proprietary SMS operations.
 *
 */
interface IMtkSms {
    /**
     * Retrieves all messages currently stored on ICC based on different mode.
     * Ex. CDMA mode or GSM mode for international cards.
     *
     * @param subId subscription identity
     * @param mode the GSM mode or CDMA mode
     *
     * @return list of SmsRawData of all sms on ICC
     */
    List<SmsRawData> getAllMessagesFromIccEfByModeForSubscriber(in int subId, String callingPkg,
            int mode);

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
     */
    int copyTextMessageToIccCardForSubscriber(in int subId, String callingPkg,
            in String scAddress, in String address, in List<String> text,
            in int status, in long timestamp);

    /**
     * Send a data message with original port
     *
     * @param subId the subscription identity
     * @param destAddr destination address
     * @param scAddr the SMSC to send the message through, or NULL for the
     *  default SMSC
     * @param destPort destination port
     * @param originalPort original port
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
    void sendDataWithOriginalPortForSubscriber(in int subId, String callingPkg,
            in String destAddr, in String scAddr, in int destPort,
            in int originalPort, in byte[] data, in PendingIntent sentIntent,
            in PendingIntent deliveryIntent);

    /**
     * Judge if SMS subsystem is ready or not.
     *
     * @param subId the subscription identity
     *
     * @return true for success
     */
    boolean isSmsReadyForSubscriber(in int subId);

    /**
     * Set the memory storage status of the SMS
     * This function is used for FTA test only
     *
     * @param subId the subscription identity
     * @param status false for storage full, true for storage available
     *
     */
    void setSmsMemoryStatusForSubscriber(in int subId, boolean status);

    /**
     * Get SMS SIM Card memory's total and used number
     *
     * @param subId the subscription identity
     *
     * @return <code>MtkIccSmsStorageStatus</code> object
     *
     */
    MtkIccSmsStorageStatus getSmsSimMemoryStatusForSubscriber(in int subId, String callingPkg);

    /**
     * Send an SMS with specified encoding type.
     *
     * @param subId subscriptioni identity
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
     * @param persistMessageForNonDefaultSmsApp whether the sent message should
     *   be automatically persisted in the SMS db. It only affects messages sent
     *   by a non-default SMS app. Currently only the carrier app can set this
     *   parameter to false to skip auto message persistence.
     */
    void sendTextWithEncodingTypeForSubscriber(in int subId, String callingPkg,
            in String destAddr, in String scAddr, in String text, in int encodingType,
            in PendingIntent sentIntent, in PendingIntent deliveryIntent,
            in boolean persistMessageForNonDefaultSmsApp);

    /**
     * Send a multi-part text based SMS with specified encoding type.
     *
     * @param subId subscriptioni identity
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
     * @param persistMessageForNonDefaultSmsApp whether the sent message should
     *   be automatically persisted in the SMS db. It only affects messages sent
     *   by a non-default SMS app. Currently only the carrier app can set this
     *   parameter to false to skip auto message persistence.
     */
    void sendMultipartTextWithEncodingTypeForSubscriber(in int subId, String callingPkg,
            in String destAddr, in String scAddr, in List<String> parts, in int encodingType,
            in List<PendingIntent> sentIntents, in List<PendingIntent> deliveryIntents,
            in boolean persistMessageForNonDefaultSmsApp);

    /**
     * Copy a text SMS to the ICC.
     *
     * @param subId subscriptioni identity
     * @param scAddress Service center address
     * @param address   Destination address or original address
     * @param text      List of message text
     * @param status    message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *                  STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param timestamp Timestamp when service center receive the message
     * @return MtkSimSmsInsertStatus
     *
     */
    MtkSimSmsInsertStatus insertTextMessageToIccCardForSubscriber(in int subId,
            String callingPkg, in String scAddress, in String address,
            in List<String> text, in int status, in long timestamp);

    /**
     * Copy a raw SMS PDU to the ICC.
     *
     * @param subId subscriptioni identity
     * @param status message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *               STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param pdu the raw PDU to store
     * @param smsc encoded smsc service center
     * @return MtkSimSmsInsertStatus
     *
     */
    MtkSimSmsInsertStatus insertRawMessageToIccCardForSubscriber(in int subId,
            String callingPkg, int status, in byte[] pdu, in byte[] smsc);

    /**
     * Send an SMS with specified encoding type.
     *
     * @param subId subscriptioni identity
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
     * @param persistMessageForNonDefaultSmsApp whether the sent message should
     *   be automatically persisted in the SMS db. It only affects messages sent
     *   by a non-default SMS app. Currently only the carrier app can set this
     *   parameter to false to skip auto message persistence.
     */
    void sendTextWithExtraParamsForSubscriber(in int subId, String callingPkg,
            in String destAddr, in String scAddr, in String text,
            in Bundle extraParams, in PendingIntent sentIntent,
            in PendingIntent deliveryIntent, in boolean persistMessageForNonDefaultSmsApp);

    /**
     * Send a multi-part text based SMS with specified encoding type.
     *
     * @param subId subscriptioni identity
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
     * @param persistMessageForNonDefaultSmsApp whether the sent message should
     *   be automatically persisted in the SMS db. It only affects messages sent
     *   by a non-default SMS app. Currently only the carrier app can set this
     *   parameter to false to skip auto message persistence.
     */
    void sendMultipartTextWithExtraParamsForSubscriber(in int subId, String callingPkg,
            in String destAddr, in String scAddr, in List<String> parts,
            in Bundle extraParams, in List<PendingIntent> sentIntents,
            in List<PendingIntent> deliveryIntents,
            in boolean persistMessageForNonDefaultSmsApp);

   /**
    * Get sms parameters from EFsmsp, such as the validity period & its format,
    * protocol identifier and decode char set value
    *
    * @param subId subscriptioni identity
    */
    MtkSmsParameters getSmsParametersForSubscriber(in int subId, String callingPkg);

   /**
    * Save sms parameters into EFsmsp
    *
    * @param subId subscriptioni identity
    */
    boolean setSmsParametersForSubscriber(in int subId, String callingPkg,
            in MtkSmsParameters params);

    /**
     * Retrieves message currently stored on ICC by index.
     *
     * @param subId subscriptioni identity
     * @param index the index of sms save in EFsms
     *
     * @return SmsRawData of sms on ICC
     */
    SmsRawData getMessageFromIccEfForSubscriber(in int subId, String callingPkg, in int index);

    /**
     * Query the activation status of cell broadcast.
     *
     * @param subId subscriptioni identity
     *
     * @return true if activate; false if inactivate.
     */
    boolean queryCellBroadcastSmsActivationForSubscriber(in int subId);

    /**
     * Activate or deactivate cell broadcast SMS.
     *
     * @param subId subscriptioni identity
     * @param activate 0 = activate, 1 = deactivate
     *
     * @return true if activate successfully; false if activate failed
     */
    boolean activateCellBroadcastSmsForSubscriber(in int subId, in boolean activate);

    /**
     * Remove specified channel and serial of cb message.
     *
     * @param channelId removed channel id
     * @param serialId removed serial id
     *
     * @return true process successfully; false process failed.
     */
    boolean removeCellBroadcastMsgForSubscriber(in int subId, in int channelId,
            in int serialId);

    /**
     * Set the ETWS(Earthquake and Tsunami Warning System) config to modem
     *
     * @param subId subscriptioni identity
     * @param mode the etws mode
     *
     * @return true if set successfully; false if set failed
     */
    boolean setEtwsConfigForSubscriber(in int subId, in int mode);

    /**
     * Query cell broadcast channel configuration.
     *
     * @param subId subscription identity
     *
     * @return channel configuration. e.g. "1,3,100-123".
     */
    String getCellBroadcastRangesForSubscriber(int subId);

    /**
     * Set cell broadcast language configuration
     *
     * @param subId subscription identity
     * @param lang denotes language configuration. e.g. "1,7,5-29"
     *
     * @return true if set successfully; false if set failed
     */
    boolean setCellBroadcastLangsForSubscriber(int subId, String lang);

    /**
     * Get cell broadcast language configuration
     *
     * @param subId subscription identity
     * @return language configuration. e.g. "1,2,3,32"
     *
     * @return true if set successfully; false if set failed
     */
    String getCellBroadcastLangsForSubscriber(int subId);

    /**
     * Get service center address
     *
     * @param subId subscription identity
     *
     * @return service message center address
     */
    String getScAddressForSubscriber(int subId);

    /**
     * Get service center address
     *
     * @param subId subscription identity
     *
     * @return service message center address and error code
     */
    Bundle getScAddressWithErrorCodeForSubscriber(int subId);

    /**
     * Set service message center address
     *
     * @param subId subscription identity
     * @param address service message center addressto be set
     *
     * @return true for success, false for failure
     */
    boolean setScAddressForSubscriber(int subId, String address);
}
