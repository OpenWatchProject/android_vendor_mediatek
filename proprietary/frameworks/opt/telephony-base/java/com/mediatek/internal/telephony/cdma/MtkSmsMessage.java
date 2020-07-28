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

import android.content.res.Resources;
import android.telephony.Rlog;
import android.telephony.SmsCbLocation;
import android.telephony.SmsCbMessage;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;
import com.android.internal.telephony.SmsAddress;
import com.android.internal.telephony.SmsHeader;
import com.android.internal.telephony.cdma.SmsMessage;
import com.android.internal.telephony.cdma.sms.BearerData;
import com.android.internal.telephony.cdma.sms.CdmaSmsAddress;
import com.android.internal.telephony.cdma.sms.SmsEnvelope;
import com.android.internal.telephony.cdma.sms.UserData;
import com.android.internal.util.HexDump;

import com.mediatek.internal.telephony.MtkPhoneNumberUtils;
import com.mediatek.internal.telephony.cdma.pluscode.IPlusCodeUtils;
import com.mediatek.internal.telephony.cdma.pluscode.PlusCodeProcessor;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;

/**
 * The sub class to enhance the AOSP SmsMessage class.
 */
public class MtkSmsMessage extends SmsMessage {

    private static final String LOG_TAG = "MtkCdmaSmsMessage";
    private static  final String LOGGABLE_TAG = "CDMA:SMS";
    private static final int RETURN_ACK     = 1;
    private static IPlusCodeUtils sPlusCodeUtils = PlusCodeProcessor.getPlusCodeUtils();

    /**
     * Create an SmsMessage from an SMS EF record.
     *
     * @param index Index of SMS record. This should be index in ArrayList
     *              returned by RuimSmsInterfaceManager.getAllMessagesFromIcc + 1.
     * @param data Record data.
     * @return An SmsMessage representing the record.
     *
     * @hide
     */
    public static SmsMessage createFromEfRecord(int index, byte[] data) {
        try {
            SmsMessage msg = new MtkSmsMessage();

            msg.mIndexOnIcc = index;

            // First byte is status: RECEIVED_READ, RECEIVED_UNREAD, STORED_SENT,
            // or STORED_UNSENT
            // See 3GPP2 C.S0023 3.4.27
            if ((data[0] & 1) == 0) {
                Rlog.w(LOG_TAG, "SMS parsing failed: Trying to parse a free record");
                return null;
            } else {
                msg.mStatusOnIcc = data[0] & 0x07;
            }

            // Second byte is the MSG_LEN, length of the message
            // See 3GPP2 C.S0023 3.4.27
            int size = data[1] & 0xFF;

            // Note: Data may include trailing FF's.  That's OK; message
            // should still parse correctly.
            byte[] pdu = new byte[size];
            System.arraycopy(data, 2, pdu, 0, size);
            // the message has to be parsed before it can be displayed
            // see gsm.SmsMessage
            msg.parsePduFromEfRecord(pdu);
            return msg;
        } catch (RuntimeException ex) {
            Rlog.e(LOG_TAG, "SMS PDU parsing failed: ", ex);
            return null;
        }

    }

    @Override
    public String getOriginatingAddress() {
        replaceIddNddWithPluscode(mOriginatingAddress);
        return super.getOriginatingAddress();
    }

    @Override
    protected BearerData onDecodeBroadcastSms() {
        return MtkBearerData.decode(mEnvelope.bearerData, mEnvelope.serviceCategory);
    }

    /**
     * Get an SMS-SUBMIT PDU for a data message to a destination address and port.
     *
     * @param scAddr Service Centre address. null == use default
     * @param destAddr the address of the destination for the message
     * @param destPort the port to deliver the message to at the
     *        destination
     * @param originalPort the port to deliver the message from
     * @param data the data for the message
     * @param statusReportRequested Indicates whether a report is requested
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     *
     * @hide
     */
    public static SubmitPdu getSubmitPdu(String scAddr, String destAddr, int destPort,
            int originalPort, byte[] data, boolean statusReportRequested) {

         /**
         * TODO(cleanup): this is not a general-purpose SMS creation
         * method, but rather something specialized to messages
         * containing OCTET encoded (meaning non-human-readable) user
         * data.  The name should reflect that, and not just overload.
         */

        SmsHeader.PortAddrs portAddrs = new SmsHeader.PortAddrs();
        portAddrs.destPort = destPort;
        portAddrs.origPort = originalPort;
        portAddrs.areEightBits = false;

        SmsHeader smsHeader = new SmsHeader();
        smsHeader.portAddrs = portAddrs;

        UserData uData = new UserData();
        // This is just a workaround: for cdma sms auto register, CT network
        // will not response if the pdu with a header, so we assumed that the
        // originalPort 0 meaning nothing.
        if (originalPort == 0) {
            uData.userDataHeader = null;
            Rlog.d(LOG_TAG, "getSubmitPdu(with dest&original port), clear the header.");
        } else {
            uData.userDataHeader = smsHeader;
        }
        uData.msgEncoding = UserData.ENCODING_OCTET;
        uData.msgEncodingSet = true;
        uData.payload = data;

        return privateGetSubmitPdu(destAddr, statusReportRequested, uData);
    }

    /**
     * Get an SMS-SUBMIT PDU for a data message to a destination address &amp; port.
     *
     * @param destAddr the address of the destination for the message
     * @param userData the data for the message
     * @param statusReportRequested Indicates whether a report is requested for this message.
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     */
    public static SubmitPdu getSubmitPdu(String destAddr, UserData userData,
            boolean statusReportRequested) {
        return privateGetPdu(destAddr, statusReportRequested, userData, 0, -1, -1);
    }

    /**
     * Create an PDU for an SMS record. M: via added.
     *
     * @param destinationAddress sms address in SMS record.
     * @param message sms content.
     * @param timeStamp sms time.
     * @return SubmitPdu.
     * @hide
     */
    public static SubmitPdu createEfPdu(String destinationAddress, String message,
            long timeStamp) {
        if (destinationAddress == null || message == null) {
            return null;
        }

        UserData uData = new UserData();
        uData.payloadStr = message;
        uData.userDataHeader = null;
        /**
         * Firstly, 3GPP2 have no timezone info for timestamp in PDU,
         * the 3GPP2 SMS timestamp just is a 6-byte-field(3GPP2 C.S0015-B, v2, 4.5.4,
         * 3GPP is 7-byte-field, and the extra byte for timezone info). So, it is difficul
         * to give out the UTC timestamp for CDMA framework.
         *
         * For some reason, the SMS APP want to get a timestamp in
         * UTC(see the attach mail in HANDROID#1489), so we supported a
         * compromise parsing solution basing on the current network situation
         * (current CDMA network SMS always is local timezone, and the Google
         * original action is also fit to this.
         * {@link com.android.internal.telephony.cdma.sms.BearerData#TimeStamp})
         *
         * The other hand, when copy a SMS to UIM card, and the SMS APP transfered
         * an UTC timestamp to the encode method, relative to the parsing action,
         * we change it to a local timestamp here, let them dont conflict with each other.
         *
         * {@link com.android.internal.telephony.cdma.SmsMessage#parseSms()}
         *
        */
        // make sure just do this for available timestamp
        if (timeStamp > 0) {
            long scTimeMillis = timeStamp /*+ (getTimeZoneOffset() * 15 * 60 * 1000)*/;
            Rlog.d(LOG_TAG, "createEfPdu, input timeStamp = " + timeStamp
                    + ", out scTimeMillis = " + scTimeMillis);
            // make a value assignment back to reduce next code modify
            timeStamp = scTimeMillis;
        } else {
            Rlog.d(LOG_TAG, "createEfPdu, input timeStamp = " + timeStamp
                    + ", dont assign time zone to this invalid value");
        }
        // VIA modify end, HANDROID#1489

        return privateGetPdu(destinationAddress, false, uData, timeStamp, -1, -1);
    }

    /**
     * Creates BearerData and Envelope from parameters for a Submit SMS. via add
     * a new parameter - timeStamp.
     *
     * @return byte stream for SubmitPdu.
     */
    private static SubmitPdu privateGetPdu(String destAddrStr,
            boolean statusReportRequested, UserData userData, long timeStamp,
            int validityPeriod, int priority) {
        /**
         * TODO(cleanup): give this function a more meaningful name.
         */

        /**
         * TODO(cleanup): Make returning null from the getSubmitPdu
         * variations meaningful -- clean up the error feedback
         * mechanism, and avoid null pointer exceptions.
         */

        /**
         * North America Plus Code :
         * Convert + code to 011 and dial out for international SMS
         */
        CdmaSmsAddress destAddr = CdmaSmsAddress.parse(
                MtkPhoneNumberUtils.cdmaCheckAndProcessPlusCodeForSms(destAddrStr));
        if (destAddr == null) {
            return null;
        }
        if (destAddr.numberOfDigits > CdmaSmsAddress.SMS_ADDRESS_MAX) {
            Rlog.d(LOG_TAG, "number of digit exceeds the SMS_ADDRESS_MAX");
            return null;
        }
        BearerData bearerData = new BearerData();
        bearerData.messageType = BearerData.MESSAGE_TYPE_SUBMIT;

        bearerData.messageId = getNextMessageId();

        bearerData.deliveryAckReq = statusReportRequested;
        bearerData.userAckReq = false;
        bearerData.readAckReq = false;
        bearerData.reportReq = false;

        bearerData.userData = userData;

        if (timeStamp > 0) {
            //Assume the time has been convert to UTC time.
            bearerData.msgCenterTimeStamp = new BearerData.TimeStamp();
            bearerData.msgCenterTimeStamp.set(timeStamp);
        }

        // add validity Period, -1 for no
        if (validityPeriod >= 0) {
            bearerData.validityPeriodRelativeSet = true;
            bearerData.validityPeriodRelative = validityPeriod;
        } else {
            bearerData.validityPeriodRelativeSet = false;
        }

        // add priority, -1 for not set
        if (priority >= 0) {
            bearerData.priorityIndicatorSet = true;
            bearerData.priority = priority;
        } else {
            bearerData.priorityIndicatorSet = false;
        }

        byte[] encodedBearerData = MtkBearerData.encode(bearerData);
        if (Rlog.isLoggable(LOGGABLE_TAG, Log.VERBOSE)) {
            Rlog.d(LOG_TAG, "MO (encoded) BearerData = " + bearerData);
            if (encodedBearerData != null) {
                Rlog.d(LOG_TAG,
                        "MO raw BearerData = '" + HexDump.toHexString(encodedBearerData) + "'");
            }
        }
        if (encodedBearerData == null) {
            return null;
        }
        int teleservice = bearerData.hasUserDataHeader ?
                SmsEnvelope.TELESERVICE_WEMT : SmsEnvelope.TELESERVICE_WMT;


        SmsEnvelope envelope = new SmsEnvelope();
        envelope.messageType = SmsEnvelope.MESSAGE_TYPE_POINT_TO_POINT;
        envelope.teleService = teleservice;
        envelope.destAddress = destAddr;
        envelope.bearerReply = RETURN_ACK;
        envelope.bearerData = encodedBearerData;

        /**
         * TODO(cleanup): envelope looks to be a pointless class, get
         * rid of it.  Also -- most of the envelope fields set here
         * are ignored, why?
         */

        try {
            /**
             * TODO(cleanup): reference a spec and get rid of the ugly comments
             */
            ByteArrayOutputStream baos = new ByteArrayOutputStream(100);
            DataOutputStream dos = new DataOutputStream(baos);
            dos.writeInt(envelope.teleService);
            dos.writeInt(0); //servicePresent
            dos.writeInt(0); //serviceCategory
            dos.write(destAddr.digitMode);
            dos.write(destAddr.numberMode);
            dos.write(destAddr.ton); // number_type
            dos.write(destAddr.numberPlan);
            dos.write(destAddr.numberOfDigits);
            dos.write(destAddr.origBytes, 0, destAddr.origBytes.length); // digits
            // Subaddress is not supported.
            dos.write(0); //subaddressType
            dos.write(0); //subaddr_odd
            dos.write(0); //subaddr_nbr_of_digits
            dos.write(encodedBearerData.length);
            dos.write(encodedBearerData, 0, encodedBearerData.length);
            dos.close();

            SubmitPdu pdu = new SubmitPdu();
            pdu.encodedMessage = baos.toByteArray();
            pdu.encodedScAddress = null;
            return pdu;
        } catch (IOException ex) {
            Rlog.e(LOG_TAG, "creating SubmitPdu failed: " + ex);
        }
        return null;
    }

    /**
     * Get an SMS-SUBMIT PDU for a destination address and a message using the
     * specified encoding and validity period.
     *
     * @param scAddress Service Centre address.  Null means use default.
     * @param destAddrStr           Address of the recipient.
     * @param message               String representation of the message payload.
     * @param statusReportRequested Indicates whether a report is requested for this message.
     * @param smsHeader             Array containing the data for the User Data Header, preceded
     *                              by the Element Identifiers.
     * @param encodingtype Encoding defined by constants in android.telephony.SmsMessage.ENCODING_*
     * @param validityPeriod The sms`s validity period
     * @param priority The sms`s priority
     * @param use7BitAscii using 7BIT_ASCII or GSM_7BIT_ALPHABETwhen 7bit encoding is used
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     * @hide
     */
    public static SubmitPdu getSubmitPdu(String scAddress,
            String destAddrStr, String message, boolean statusReportRequested,
            SmsHeader smsHeader, int encodingtype, int validityPeriod, int priority,
            boolean use7BitAscii) {

        if (destAddrStr == null || message == null) {
            Log.e(LOG_TAG, "getSubmitPdu, null sms text or destination address. do nothing.");
            return null;
        }

        if (destAddrStr.isEmpty()) {
            Log.e(LOG_TAG, "getSubmitPdu, destination address is empty. do nothing.");
            return null;
        }

        if (message.isEmpty()) {
            Log.e(LOG_TAG, "getSubmitPdu, message text is empty. do nothing.");
            return null;
        }

        if ((validityPeriod > 244) && (validityPeriod <= 255)) {
            validityPeriod = 244;
        }

        UserData uData = new UserData();
        uData.payloadStr = message;
        uData.userDataHeader = smsHeader;

        // fix one reasonable encoding type, avoiding the sending failure
        if (encodingtype == android.telephony.SmsMessage.ENCODING_7BIT) {
            uData.msgEncoding = use7BitAscii ? UserData.ENCODING_7BIT_ASCII
                    : UserData.ENCODING_GSM_7BIT_ALPHABET;
        } else if (encodingtype == android.telephony.SmsMessage.ENCODING_8BIT) {
            uData.msgEncoding = UserData.ENCODING_OCTET;
        } else {
            uData.msgEncoding = UserData.ENCODING_UNICODE_16;
        }
        uData.msgEncodingSet = true;

        return privateGetPdu(destAddrStr,
                statusReportRequested, uData, 0, validityPeriod, priority);
    }

    /**
     * Calculate the number of septets needed to encode the message.
     *
     * @param messageBody the message to encode
     * @param use7bitOnly ignore (but still count) illegal characters if true
     * @param encodingType the encoding type of content of message(GSM 7-bit, Unicode or Automatic)
     * @return TextEncodingDetails
     */
    public static TextEncodingDetails calculateLength(CharSequence messageBody,
            boolean use7bitOnly, int encodingType) {
        Resources r = Resources.getSystem();
        if (r.getBoolean(com.android.internal.R.bool.config_sms_force_7bit_encoding)) {
            Rlog.d(LOG_TAG, "here use BearerData.calcTextEncodingDetails, but divide in parent " +
                    "class will use Sms7BitEncodingTranslator.translate(messageBody) returned " +
                    "string instead again in this case, Caution!!");
            Rlog.d(LOG_TAG, "search calculateLengthCDMA for help!", new Throwable());
        }
        return MtkBearerData.calcTextEncodingDetails(messageBody, use7bitOnly, encodingType);
    }

    /**
     * Create a MtkSmsMessage from a AOSP SmsMessage.
     *
     * @param sms the AOSP SmsMessage
     *
     * @return MtkSmsMessage
     */
    public static MtkSmsMessage newMtkSmsMessage(SmsMessage sms) {
        if (sms == null) {
            return null;
        }
        MtkSmsMessage mtkSms = new MtkSmsMessage();
        mtkSms.mScAddress = sms.getServiceCenterAddress();
        mtkSms.mOriginatingAddress = sms.mOriginatingAddress;
        mtkSms.mMessageBody = sms.getMessageBody();
        mtkSms.mPseudoSubject = sms.getPseudoSubject();
        mtkSms.mEmailFrom = sms.getEmailFrom();
        mtkSms.mEmailBody = sms.getEmailBody();
        mtkSms.mIsEmail = sms.isEmail();
        mtkSms.mScTimeMillis = sms.getTimestampMillis();
        mtkSms.mPdu = sms.getPdu();
        mtkSms.mUserData = sms.getUserData();
        mtkSms.mUserDataHeader = sms.getUserDataHeader();
        mtkSms.mIsMwi = false;
        mtkSms.mMwiSense = false;
        mtkSms.mMwiDontStore = false;
        mtkSms.mStatusOnIcc = sms.getStatusOnIcc();
        mtkSms.mIndexOnIcc = sms.getIndexOnIcc();
        mtkSms.mMessageRef = sms.mMessageRef;
        mtkSms.status = (sms.getStatus()) >> 16;
        mtkSms.mEnvelope = sms.mEnvelope;
        mtkSms.mBearerData = sms.mBearerData;
        return mtkSms;
    }

    private static String handlePlusCodeInternal(int ton, String number) {
        String ret = sPlusCodeUtils.removeIddNddAddPlusCodeForSms(number);
        if (TextUtils.isEmpty(ret)) {
            return null;
        }
        if (ton == SmsAddress.TON_INTERNATIONAL && number.charAt(0) != '+') {
            ret = "+" + ret;
        }
        Rlog.d(LOG_TAG, "handlePlusCodeInternal, after handled, the address = " +
                Rlog.pii(LOG_TAG, ret));
        return ret;
    }


    private static void replaceIddNddWithPluscode(SmsAddress addr) {
        String orignalStr = new String(addr.origBytes);
        String number = handlePlusCodeInternal(addr.ton, orignalStr);
        if (TextUtils.isEmpty(number)) {
            return;
        }
        boolean changed = !number.equals(orignalStr);
        if (changed) {
            addr.origBytes = number.getBytes();
            addr.address = number;
        }
    }

    /**
     * Returns the destination address (receiver) of this SMS message in String.
     * form or null if unavailable
     *
     * @return the destination address for MO message
     */
    public String getDestinationAddress() {
        return getOriginatingAddress();
    }
}

