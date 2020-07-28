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

import android.telephony.PhoneNumberUtils;
import android.text.format.Time;
import android.telephony.Rlog;
import android.content.res.Resources;
import android.text.TextUtils;
import android.os.Build;

import com.android.internal.telephony.EncodeException;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.SmsAddress;
import com.android.internal.telephony.SmsHeader;
import com.android.internal.telephony.SmsMessageBase;
import com.android.internal.telephony.Sms7BitEncodingTranslator;
import com.android.internal.telephony.gsm.SmsMessage;

import com.mediatek.internal.telephony.MtkSmsHeader;

import java.io.ByteArrayOutputStream;
import java.io.UnsupportedEncodingException;
import java.text.ParseException;

import static com.android.internal.telephony.SmsConstants.MessageClass;
import static com.android.internal.telephony.SmsConstants.ENCODING_UNKNOWN;
import static com.android.internal.telephony.SmsConstants.ENCODING_7BIT;
import static com.android.internal.telephony.SmsConstants.ENCODING_8BIT;
import static com.android.internal.telephony.SmsConstants.ENCODING_16BIT;
import static com.android.internal.telephony.SmsConstants.ENCODING_KSC5601;
import static com.android.internal.telephony.SmsConstants.MAX_USER_DATA_SEPTETS;
import static com.android.internal.telephony.SmsConstants.MAX_USER_DATA_BYTES;
import static com.android.internal.telephony.SmsConstants.MAX_USER_DATA_BYTES_WITH_HEADER;

import java.util.Arrays;

// MTK-START
import static com.android.internal.telephony.SmsConstants.MAX_USER_DATA_SEPTETS_WITH_HEADER;
import android.os.SystemProperties;
// MTK-END

/**
 * A Short Message Service message.
 *
 */
public class MtkSmsMessage extends SmsMessage {
    static final String LOG_TAG = "MtkSmsMessage";
    private static final boolean ENG = "eng".equals(Build.TYPE);

    // MTK-START
    // Initial value for encoding type
    private int mEncodingType = ENCODING_UNKNOWN;

    public static final int ENCODING_7BIT_SINGLE = 11;
    public static final int ENCODING_7BIT_LOCKING = 12;
    public static final int ENCODING_7BIT_LOCKING_SINGLE = 13;

    public static final int MASK_MESSAGE_TYPE_INDICATOR     = 0x03;
    public static final int MASK_VALIDITY_PERIOD_FORMAT     = 0x18;
    public static final int MASK_USER_DATA_HEADER_INDICATOR = 0x40;

    public static final int MASK_VALIDITY_PERIOD_FORMAT_NONE = 0x00;
    public static final int MASK_VALIDITY_PERIOD_FORMAT_RELATIVE = 0x10;
    public static final int MASK_VALIDITY_PERIOD_FORMAT_ENHANCED = 0x08;
    public static final int MASK_VALIDITY_PERIOD_FORMAT_ABSOLUTE = 0x18;

    protected String mDestinationAddress;

    /** {@hide} */
    protected int relativeValidityPeriod;
    /** {@hide} */
    protected int absoluteValidityPeriod;

    /** {@hide} */
    protected int mwiType = -1;
    /** {@hide} */
    protected int mwiCount = 0;
    // MTK-END

    public static class DeliverPdu {
        public byte[] encodedScAddress; // Null if not applicable.
        public byte[] encodedMessage;

        @Override
        public String toString() {
            return "DeliverPdu: encodedScAddress = "
                    + Arrays.toString(encodedScAddress)
                    + ", encodedMessage = "
                    + Arrays.toString(encodedMessage);
        }
    }

    /**
     * Create an SmsMessage from a raw PDU.
     */
    public static MtkSmsMessage createFromPdu(byte[] pdu) {
        try {
            MtkSmsMessage msg = new MtkSmsMessage();
            msg.parsePdu(pdu);
            return msg;
        } catch (RuntimeException ex) {
            Rlog.e(LOG_TAG, "SMS PDU parsing failed: ", ex);
            return null;
        } catch (OutOfMemoryError e) {
            Rlog.e(LOG_TAG, "SMS PDU parsing failed with out of memory: ", e);
            return null;
        }
    }

    /**
     * TS 27.005 3.4.1 lines[0] and lines[1] are the two lines read from the
     * +CMT unsolicited response (PDU mode, of course)
     *  +CMT: [&lt;alpha>],<length><CR><LF><pdu>
     *
     * Only public for debugging
     *
     * {@hide}
     */
    public static MtkSmsMessage newFromCMT(String[] lines) {
        try {
            MtkSmsMessage msg = new MtkSmsMessage();
            msg.parsePdu(IccUtils.hexStringToBytes(lines[1]));
            return msg;
        } catch (RuntimeException ex) {
            Rlog.e(LOG_TAG, "SMS PDU parsing failed: ", ex);
            return null;
        }
    }

    /** @hide */
    public static MtkSmsMessage newFromCDS(byte[] pdu) {
        try {
            MtkSmsMessage msg = new MtkSmsMessage();
            msg.parsePdu(pdu);
            return msg;
        } catch (RuntimeException ex) {
            Rlog.e(LOG_TAG, "CDS SMS PDU parsing failed: ", ex);
            return null;
        }
    }

    /**
     * Create an SmsMessage from an SMS EF record.
     *
     * @param index Index of SMS record. This should be index in ArrayList
     *              returned by SmsManager.getAllMessagesFromSim + 1.
     * @param data Record data.
     * @return An SmsMessage representing the record.
     *
     * @hide
     */
    public static MtkSmsMessage createFromEfRecord(int index, byte[] data) {
        try {
            MtkSmsMessage msg = new MtkSmsMessage();

            msg.mIndexOnIcc = index;

            // First byte is status: RECEIVED_READ, RECEIVED_UNREAD, STORED_SENT,
            // or STORED_UNSENT
            // See TS 51.011 10.5.3
            if ((data[0] & 1) == 0) {
                Rlog.w(LOG_TAG,
                        "SMS parsing failed: Trying to parse a free record");
                return null;
            } else {
                msg.mStatusOnIcc = data[0] & 0x07;
            }

            int size = data.length - 1;

            // Note: Data may include trailing FF's.  That's OK; message
            // should still parse correctly.
            byte[] pdu = new byte[size];
            System.arraycopy(data, 1, pdu, 0, size);
            msg.parsePdu(pdu);
            return msg;
        } catch (RuntimeException ex) {
            Rlog.e(LOG_TAG, "SMS PDU parsing failed: ", ex);
            return null;
        }
    }

    /**
     * Returns the destination address (receiver) of this SMS message in String
     * form or null if unavailable
     */
    public String getDestinationAddress() {
        if (mDestinationAddress == null) {
            return null;
        }

        return mDestinationAddress;
    }

    /**
     * Parses a SMS-STATUS-REPORT message.
     *
     * @param p A PduParser, cued past the first byte.
     * @param firstByte The first byte of the PDU, which contains MTI, etc.
     */
    @Override
    protected void parseSmsStatusReport(PduParser p, int firstByte) {
        super.parseSmsStatusReport(p, firstByte);

        mMessageBody = "";
    }

    /**
     * Parses a SMS-SUBMIT message.
     *
     * @param p A PduParser, cued past the first byte.
     * @param firstByte The first byte of the PDU, which contains MTI, etc.
     */
    @Override
    protected void parseSmsSubmit(PduParser p, int firstByte) {
        super.parseSmsSubmit(p, firstByte);

        // for SmsMessageBase, and can getDestinationAddress by AP
        if (mRecipientAddress != null) {
            mDestinationAddress = mRecipientAddress.getAddressString();
        }
    }

    /**
     * Parses the User Data of an SMS.
     *
     * @param p The current PduParser.
     * @param hasUserDataHeader Indicates whether a header is present in the
     *                          User Data.
     */
    @Override
    protected void parseUserData(PduParser p, boolean hasUserDataHeader) {
        boolean hasMessageClass = false;
        boolean userDataCompressed = false;

        int encodingType = ENCODING_UNKNOWN;

        // Look up the data encoding scheme
        if ((mDataCodingScheme & 0x80) == 0) {
            userDataCompressed = (0 != (mDataCodingScheme & 0x20));
            hasMessageClass = (0 != (mDataCodingScheme & 0x10));

            if (userDataCompressed) {
                Rlog.w(LOG_TAG, "4 - Unsupported SMS data coding scheme "
                        + "(compression) " + (mDataCodingScheme & 0xff));
            } else {
                switch ((mDataCodingScheme >> 2) & 0x3) {
                case 0: // GSM 7 bit default alphabet
                    encodingType = ENCODING_7BIT;
                    break;

                case 2: // UCS 2 (16bit)
                    encodingType = ENCODING_16BIT;
                    break;

                case 1: // 8 bit data
                    //Support decoding the user data payload as pack GSM 8-bit (a GSM alphabet
                    //string that's stored in 8-bit unpacked format) characters.
                    Resources r = Resources.getSystem();
                    if (r.getBoolean(com.android.internal.
                            R.bool.config_sms_decode_gsm_8bit_data)) {
                        encodingType = ENCODING_8BIT;
                        break;
                    }

                case 3: // reserved
                    Rlog.w(LOG_TAG, "1 - Unsupported SMS data coding scheme "
                            + (mDataCodingScheme & 0xff));
                    encodingType = ENCODING_8BIT;
                    break;
                }
            }
        } else if ((mDataCodingScheme & 0xf0) == 0xf0) {
            hasMessageClass = true;
            userDataCompressed = false;

            if (0 == (mDataCodingScheme & 0x04)) {
                // GSM 7 bit default alphabet
                encodingType = ENCODING_7BIT;
            } else {
                // 8 bit data
                encodingType = ENCODING_8BIT;
            }
        } else if ((mDataCodingScheme & 0xF0) == 0xC0
                || (mDataCodingScheme & 0xF0) == 0xD0
                || (mDataCodingScheme & 0xF0) == 0xE0) {
            // 3GPP TS 23.038 V7.0.0 (2006-03) section 4

            // 0xC0 == 7 bit, don't store
            // 0xD0 == 7 bit, store
            // 0xE0 == UCS-2, store

            if ((mDataCodingScheme & 0xF0) == 0xE0) {
                encodingType = ENCODING_16BIT;
            } else {
                encodingType = ENCODING_7BIT;
            }

            userDataCompressed = false;
            boolean active = ((mDataCodingScheme & 0x08) == 0x08);
            // bit 0x04 reserved

            // VM - If TP-UDH is present, these values will be overwritten
            if ((mDataCodingScheme & 0x03) == 0x00) {
                mIsMwi = true; /* Indicates vmail */
                mMwiSense = active;/* Indicates vmail notification set/clear */
                mMwiDontStore = ((mDataCodingScheme & 0xF0) == 0xC0);

                /* Set voice mail count based on notification bit */
                if (active == true) {
                    mVoiceMailCount = -1; // unknown number of messages waiting
                } else {
                    mVoiceMailCount = 0; // no unread messages
                }

                Rlog.w(LOG_TAG, "MWI in DCS for Vmail. DCS = "
                        + (mDataCodingScheme & 0xff) + " Dont store = "
                        + mMwiDontStore + " vmail count = " + mVoiceMailCount);

            } else {
                mIsMwi = false;
                Rlog.w(LOG_TAG, "MWI in DCS for fax/email/other: "
                        + (mDataCodingScheme & 0xff));
            }
        } else if ((mDataCodingScheme & 0xC0) == 0x80) {
            // 3GPP TS 23.038 V7.0.0 (2006-03) section 4
            // 0x80..0xBF == Reserved coding groups
            if (mDataCodingScheme == 0x84) {
                // This value used for KSC5601 by carriers in Korea.
                encodingType = ENCODING_KSC5601;
            } else {
                Rlog.w(LOG_TAG, "5 - Unsupported SMS data coding scheme "
                        + (mDataCodingScheme & 0xff));
            }
        } else {
            Rlog.w(LOG_TAG, "3 - Unsupported SMS data coding scheme "
                    + (mDataCodingScheme & 0xff));
        }

        // set both the user data and the user data header.
        int count = p.constructUserData(hasUserDataHeader,
                encodingType == ENCODING_7BIT);
        this.mUserData = p.getUserData();
        this.mUserDataHeader = p.getUserDataHeader();

        // MTK-START
        // Remember the value for the getEncodingType function
        mEncodingType = encodingType;
        // MTK-END

        /*
         * Look for voice mail indication in TP_UDH TS23.040 9.2.3.24
         * ieid = 1 (0x1) (SPECIAL_SMS_MSG_IND)
         * ieidl =2 octets
         * ieda msg_ind_type = 0x00 (voice mail; discard sms )or
         *                   = 0x80 (voice mail; store sms)
         * msg_count = 0x00 ..0xFF
         */
        if (hasUserDataHeader && (mUserDataHeader.specialSmsMsgList.size() != 0)) {
            for (SmsHeader.SpecialSmsMsg msg : mUserDataHeader.specialSmsMsgList) {
                int msgInd = msg.msgIndType & 0xff;
                /*
                 * TS 23.040 V6.8.1 Sec 9.2.3.24.2
                 * bits 1 0 : basic message indication type
                 * bits 4 3 2 : extended message indication type
                 * bits 6 5 : Profile id bit 7 storage type
                 */
                if ((msgInd == 0) || (msgInd == 0x80)) {
                    mIsMwi = true;
                    if (msgInd == 0x80) {
                        /* Store message because TP_UDH indicates so*/
                        mMwiDontStore = false;
                    } else if (mMwiDontStore == false) {
                        /* Storage bit is not set by TP_UDH
                         * Check for conflict
                         * between message storage bit in TP_UDH
                         * & DCS. The message shall be stored if either of
                         * the one indicates so.
                         * TS 23.040 V6.8.1 Sec 9.2.3.24.2
                         */
                        if (!((((mDataCodingScheme & 0xF0) == 0xD0)
                               || ((mDataCodingScheme & 0xF0) == 0xE0))
                               && ((mDataCodingScheme & 0x03) == 0x00))) {
                            /* Even DCS did not have voice mail with Storage bit
                             * 3GPP TS 23.038 V7.0.0 section 4
                             * So clear this flag*/
                            mMwiDontStore = true;
                        }
                    }

                    mVoiceMailCount = msg.msgCount & 0xff;

                    /*
                     * In the event of a conflict between message count setting
                     * and DCS then the Message Count in the TP-UDH shall
                     * override the indication in the TP-DCS. Set voice mail
                     * notification based on count in TP-UDH
                     */
                    if (mVoiceMailCount > 0)
                        mMwiSense = true;
                    else
                        mMwiSense = false;

                    Rlog.w(LOG_TAG, "MWI in TP-UDH for Vmail. Msg Ind = " + msgInd
                            + " Dont store = " + mMwiDontStore + " Vmail count = "
                            + mVoiceMailCount);

                    /*
                     * There can be only one IE for each type of message
                     * indication in TP_UDH. In the event they are duplicated
                     * last occurence will be used. Hence the for loop
                     */
                } else {
                    Rlog.w(LOG_TAG, "TP_UDH fax/email/"
                            + "extended msg/multisubscriber profile. Msg Ind = " + msgInd);
                }
            } // end of for
        } // end of if UDH

        switch (encodingType) {
        case ENCODING_UNKNOWN:
            mMessageBody = null;
            break;

        case ENCODING_8BIT:
            //Support decoding the user data payload as pack GSM 8-bit (a GSM alphabet string
            //that's stored in 8-bit unpacked format) characters.
            Resources r = Resources.getSystem();
            if (r.getBoolean(com.android.internal.
                    R.bool.config_sms_decode_gsm_8bit_data)) {
                mMessageBody = p.getUserDataGSM8bit(count);
            } else {
                mMessageBody = null;
            }
            break;

        case ENCODING_7BIT:
            mMessageBody = p.getUserDataGSM7Bit(count,
                    hasUserDataHeader ? mUserDataHeader.languageTable : 0,
                    hasUserDataHeader ? mUserDataHeader.languageShiftTable : 0);
            break;

        case ENCODING_16BIT:
            mMessageBody = p.getUserDataUCS2(count);
            break;

        case ENCODING_KSC5601:
            mMessageBody = p.getUserDataKSC5601(count);
            break;
        }

        if (mMessageBody != null) {
            parseMessageBody();
        }

        if (!hasMessageClass) {
            messageClass = MessageClass.UNKNOWN;
        } else {
            switch (mDataCodingScheme & 0x3) {
            case 0:
                messageClass = MessageClass.CLASS_0;
                break;
            case 1:
                messageClass = MessageClass.CLASS_1;
                break;
            case 2:
                messageClass = MessageClass.CLASS_2;
                break;
            case 3:
                messageClass = MessageClass.CLASS_3;
                break;
            }
        }
    }

    // MTK-START
    /**
     * Get an SMS-SUBMIT PDU for a data message to a destination address &amp; port
     *
     * @param scAddress Service Centre address. null == use default
     * @param destinationAddress the address of the destination for the message
     * @param destinationPort the port to deliver the message to at the
     *        destination
     * @param originalPort the port to deliver the message from
     * @param data the dat for the message
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     */
    public static SubmitPdu getSubmitPdu(String scAddress,
            String destinationAddress, int destinationPort, int originalPort, byte[] data,
            boolean statusReportRequested) {

        byte[] smsHeaderData = MtkSmsHeader.getSubmitPduHeader(destinationPort, originalPort);
        Rlog.d(LOG_TAG, "MtkSmsMessage: get submit pdu originalPort = " + originalPort);
        if (smsHeaderData == null && originalPort != 0) {
            return null;
        }
        if (originalPort == 0) {//add for sms auto regist
            return getSubmitPdu(scAddress, destinationAddress,
                data, statusReportRequested);
        }
        return getSubmitPdu(scAddress, destinationAddress,
                data, smsHeaderData, statusReportRequested);
    }

    /**
     * Get an SMS-SUBMIT PDU for a destination address and a message
     * which are sent to a specified application port
     *
     * @param scAddress Service Centre address.  Null means use default.
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     */
    public static SubmitPdu getSubmitPdu(String scAddress,
            String destinationAddress, String message,
            int destPort, boolean statusReportRequested) {

        int language = getCurrentSysLanguage();
        int singleId = -1;
        int lockingId = -1;
        int encoding = ENCODING_UNKNOWN;
        TextEncodingDetails ted = new TextEncodingDetails();

        if (encodeStringWithSpecialLang(message, language, ted)) {
            if (ted.useLockingShift && ted.useSingleShift) {
                encoding = ENCODING_7BIT_LOCKING_SINGLE;
                singleId = lockingId = language;
            } else if (ted.useLockingShift) {
                encoding = ENCODING_7BIT_LOCKING;
                lockingId = language;
            } else if (ted.useSingleShift) {
                encoding = ENCODING_7BIT_SINGLE;
                singleId = language;
            } else {
                encoding = ENCODING_7BIT;
                language = -1;
            }
        } else {
            encoding = ENCODING_16BIT;
        }

        byte[] smsHeaderData = MtkSmsHeader.getSubmitPduHeaderWithLang(
                destPort, singleId, lockingId);

        return getSubmitPduWithLang(scAddress, destinationAddress,
                message, statusReportRequested, smsHeaderData, encoding, language, -1);
    }

    /**
     * Get an SMS-SUBMIT PDU for a data message with data header
     * to a destination address
     *
     * @param scAddress Service Centre address. null == use default
     * @param destinationAddress the address of the destination for the message
     * @param data the data for the message
     * @param header the pdu header for the message
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     */
    public static SubmitPdu getSubmitPdu(String scAddress,
            String destinationAddress, byte[] data, byte[] smsHeaderData,
            boolean statusReportRequested) {

        if ((data.length + smsHeaderData.length + 1) > MAX_USER_DATA_BYTES) {
            Rlog.e(LOG_TAG, "SMS data message may only contain "
                    + (MAX_USER_DATA_BYTES - smsHeaderData.length - 1) + " bytes");
            return null;
        }

        SubmitPdu ret = new SubmitPdu();
        ByteArrayOutputStream bo = getSubmitPduHead(
                scAddress, destinationAddress, (byte) 0x41, // MTI = SMS-SUBMIT,
                                                            // TP-UDHI = true
                statusReportRequested, ret);

        // TP-Data-Coding-Scheme
        // No class, 8 bit data
        bo.write(0x04);

        // (no TP-Validity-Period)

        // Total size
        bo.write(data.length + smsHeaderData.length + 1);

        // User data header
        bo.write(smsHeaderData.length);
        bo.write(smsHeaderData, 0, smsHeaderData.length);

        // User data
        bo.write(data, 0, data.length);

        ret.encodedMessage = bo.toByteArray();
        return ret;
    }

   /**
     * Get an SMS-SUBMIT PDU for a sms auto regist data message without data header
     * to a destination address
     *
     * @param scAddress Service Centre address. null == use default
     * @param destinationAddress the address of the destination for the message
     * @param data the data for the message
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     */
    public static SubmitPdu getSubmitPdu(String scAddress,
            String destinationAddress, byte[] data,
            boolean statusReportRequested) {
        Rlog.d(LOG_TAG, "get SubmitPdu for auto regist data.length = "
                + data.length + " bytes");
        if ((data.length + 1) > MAX_USER_DATA_BYTES) {
            Rlog.e(LOG_TAG, "data length is too long,SMS data.length = "
                    + data.length + " bytes");
            return null;
        }

        SubmitPdu ret = new SubmitPdu();
        ByteArrayOutputStream bo = getSubmitPduHead(
                scAddress, destinationAddress, (byte) 0x01, // MTI = SMS-SUBMIT
                statusReportRequested, ret);

        // TP-Data-Coding-Scheme
        // No class, 8 bit data
        bo.write(0x04);

        // Total size
        bo.write(data.length);

        // User data
        bo.write(data, 0, data.length);

        ret.encodedMessage = bo.toByteArray();
        return ret;
    }

    /**
     * Get an SMS-SUBMIT PDU for a destination address and a message using the
     * specified encoding.
     *
     * @param scAddress Service Centre address.  Null means use default.
     * @param encoding Encoding defined by constants in android.telephony.SmsMessage.ENCODING_*
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     * @hide
     */
    public static SubmitPdu getSubmitPduWithLang(String scAddress,
            String destinationAddress, String message,
            boolean statusReportRequested, byte[] header, int encoding, int language,
            int validityPeriod) {
        Rlog.d(LOG_TAG, "SmsMessage: get submit pdu with Lang");
        // Perform null parameter checks.
        if (message == null || destinationAddress == null) {
            return null;
        }

        SubmitPdu ret = new SubmitPdu();

        int validityPeriodFormat = VALIDITY_PERIOD_FORMAT_NONE;
        int relativeValidityPeriod = INVALID_VALIDITY_PERIOD;

        // TP-Validity-Period-Format (TP-VPF) in 3GPP TS 23.040 V6.8.1 section 9.2.3.3
        //bit 4:3 = 10 - TP-VP field present - relative format
        if((relativeValidityPeriod = getRelativeValidityPeriod(validityPeriod)) >= 0) {
            validityPeriodFormat = VALIDITY_PERIOD_FORMAT_RELATIVE;
        }

        byte mtiByte = (byte)(0x01 | (validityPeriodFormat << 0x03) |
                (header != null ? 0x40 : 0x00));

        ByteArrayOutputStream bo = getSubmitPduHead(
                scAddress, destinationAddress, mtiByte,
                statusReportRequested, ret);
        // User Data (and length)
        byte[] userData;
        if (encoding == ENCODING_UNKNOWN) {
            // First, try encoding it with the GSM alphabet
            encoding = ENCODING_7BIT;
        }
        try {
            Rlog.d(LOG_TAG, "Get SubmitPdu with Lang " + encoding + " " + language);
            if (encoding == ENCODING_7BIT) {
                //userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header);
                userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header, 0, 0);
            } else if (language > 0 && encoding != ENCODING_16BIT) {
                if (encoding == ENCODING_7BIT_LOCKING) {
                    //userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header,
                    //        -1, language);
                    userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header, 0,
                            language);
                } else if (encoding == ENCODING_7BIT_SINGLE) {
                    //userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header,
                    //        language, -1);
                    userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header,
                            language, 0);
                } else if (encoding == ENCODING_7BIT_LOCKING_SINGLE) {
                    userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header,
                            language, language);
                } else {
                    //userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header);
                    userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header, 0, 0);
                }
                encoding = ENCODING_7BIT;
            } else { // assume UCS-2
                try {
                    userData = encodeUCS2(message, header);
                } catch (UnsupportedEncodingException uex) {
                    Rlog.e(LOG_TAG,
                            "Implausible UnsupportedEncodingException ",
                            uex);
                    return null;
                } catch (EncodeException ucs2Ex) {
                    Rlog.e(LOG_TAG,
                            "Implausible EncodeException ",
                            ucs2Ex);
                    return null;
                }
            }
        } catch (EncodeException ex) {
            // Encoding to the 7-bit alphabet failed. Let's see if we can
            // send it as a UCS-2 encoded message
            try {
                userData = encodeUCS2(message, header);
                encoding = ENCODING_16BIT;
            } catch (UnsupportedEncodingException uex) {
                Rlog.e(LOG_TAG,
                        "Implausible UnsupportedEncodingException ",
                        uex);
                return null;
            } catch (EncodeException ucs2Ex) {
                Rlog.e(LOG_TAG,
                        "Implausible EncodeException ",
                        ucs2Ex);
                return null;
            }
        }

        if (encoding == ENCODING_7BIT) {
            if ((0xff & userData[0]) > MAX_USER_DATA_SEPTETS) {
                // Message too long
                return null;
            }
            // TP-Data-Coding-Scheme
            // Default encoding, uncompressed
            // To test writing messages to the SIM card, change this value 0x00
            // to 0x12, which means "bits 1 and 0 contain message class, and the
            // class is 2". Note that this takes effect for the sender. In other
            // words, messages sent by the phone with this change will end up on
            // the receiver's SIM card. You can then send messages to yourself
            // (on a phone with this change) and they'll end up on the SIM card.
            bo.write(0x00);
        } else { // assume UCS-2
            if ((0xff & userData[0]) > MAX_USER_DATA_BYTES) {
                // Message too long
                return null;
            }
            // TP-Data-Coding-Scheme
            // Class 3, UCS-2 encoding, uncompressed

            // modified by mtk80611
            // bo.write(0x0b);
            bo.write(0x08);
            // modified by mtk80611
        }


        if (validityPeriodFormat == VALIDITY_PERIOD_FORMAT_RELATIVE) {
            // ( TP-Validity-Period - relative format)
            bo.write(relativeValidityPeriod);
        }

        bo.write(userData, 0, userData.length);
        ret.encodedMessage = bo.toByteArray();
        return ret;
    }

    public static DeliverPdu getDeliverPduWithLang(String scAddress, String originalAddress,
            String message, byte[] header, long timestamp, int encoding, int language) {
        Rlog.d(LOG_TAG, "SmsMessage: get deliver pdu");

        if (message == null || originalAddress == null) {
            return null;
        }

        DeliverPdu ret = new DeliverPdu();

        Rlog.d(LOG_TAG, "SmsMessage: UDHI = " + (header != null));
        byte mtiByte = (byte) (0x00 | (header != null ? 0x40 : 0x00));

        ByteArrayOutputStream bo = getDeliverPduHead(scAddress, originalAddress, mtiByte, ret);

        // encode User Data (and length)
        byte[] userData;
        if (encoding == ENCODING_UNKNOWN) {
            // First, try encoding it with the GSM alphabet
            encoding = ENCODING_7BIT;
        }
        try {
            Rlog.d(LOG_TAG, "Get SubmitPdu with Lang " + encoding + " " + language);
            if (encoding == ENCODING_7BIT) {
                //userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header);
                userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header, 0, 0);
            } else if (language > 0 && encoding != ENCODING_16BIT) {
                if (encoding == ENCODING_7BIT_LOCKING) {
                    //userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header, -1,
                    //        language);
                    userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header, 0,
                            language);
                } else if (encoding == ENCODING_7BIT_SINGLE) {
                    //userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header,
                    //        language, -1);
                    userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header,
                            language, 0);
                } else if (encoding == ENCODING_7BIT_LOCKING_SINGLE) {
                    userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header,
                            language, language);
                } else {
                    //userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header);
                    userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header, 0, 0);
                }
                encoding = ENCODING_7BIT;
            } else { // assume UCS-2
                try {
                    userData = encodeUCS2(message, header);
                } catch (UnsupportedEncodingException uex) {
                    Rlog.e(LOG_TAG,
                            "Implausible UnsupportedEncodingException ",
                            uex);
                    return null;
                } catch (EncodeException ucs2Ex) {
                    Rlog.e(LOG_TAG,
                            "Implausible EncodeException ",
                            ucs2Ex);
                    return null;
                }
            }
        } catch (EncodeException ex) {
            // Encoding to the 7-bit alphabet failed. Let's see if we can
            // send it as a UCS-2 encoded message
            try {
                userData = encodeUCS2(message, header);
                encoding = ENCODING_16BIT;
            } catch (UnsupportedEncodingException uex) {
                Rlog.e(LOG_TAG,
                        "Implausible UnsupportedEncodingException ",
                        uex);
                return null;
            } catch (EncodeException ucs2Ex) {
                Rlog.e(LOG_TAG,
                        "Implausible EncodeException ",
                        ucs2Ex);
                return null;
            }
        }

        if (userData != null && (0xff & userData[0]) > MAX_USER_DATA_SEPTETS) {
            Rlog.d(LOG_TAG, "SmsMessage: message is too long");
            return null;
        }

        // write dcs type
        if (encoding == ENCODING_7BIT) {
            bo.write(0x00);
        } else { // assume UCS-2
            bo.write(0x08);
        }

        // write timestamp
        // Because we can't get invalid timestamp which indicate the message arrival service center,
        // we just write 7 0x00 into the pdu

        byte[] scts = parseSCTimestamp(timestamp);
        if (scts != null) {
            bo.write(scts, 0, scts.length);
        } else {
            for (int i = 0; i < 7; ++i) {
                bo.write(0x00);
            }
        }

        bo.write(userData, 0, userData.length);
        ret.encodedMessage = bo.toByteArray();

        return ret;
    }

    static private byte[] parseSCTimestamp(long millis) {
        Time t = new Time(Time.TIMEZONE_UTC);
        t.set(millis);

        byte[] scts = new byte[7];
        scts[0] = intToGsmBCDByte(t.year);
        scts[1] = intToGsmBCDByte(t.month + 1);
        scts[2] = intToGsmBCDByte(t.monthDay);
        scts[3] = intToGsmBCDByte(t.hour);
        scts[4] = intToGsmBCDByte(t.minute);
        scts[5] = intToGsmBCDByte(t.second);
        scts[6] = intToGsmBCDByte(0);

        return scts;
    }

    static private byte intToGsmBCDByte(int value) {
        if (value < 0) {
            Rlog.d(LOG_TAG, "[time invalid value: " + value);
            return (byte) 0;
        }
        value %= 100;
        Rlog.d(LOG_TAG, "[time value: " + value);

        // byte b = (byte)(((value / 10) << 4) + (value % 10));
        byte b = (byte) (((value / 10) & 0x0f) | (((value % 10) << 4) & 0xf0));
        Rlog.d(LOG_TAG, "[time bcd value: " + b);
        return b;
    }

    private static ByteArrayOutputStream getDeliverPduHead(
            String scAddress, String originalAddress, byte mtiByte, DeliverPdu ret) {

        ByteArrayOutputStream bo = new ByteArrayOutputStream(
                MAX_USER_DATA_BYTES + 40);

        if (scAddress == null) {
            ret.encodedScAddress = null;
        } else {
            ret.encodedScAddress = PhoneNumberUtils.networkPortionToCalledPartyBCDWithLength(
                    scAddress);
        }

        // write mti byte
        bo.write(mtiByte);

        // write original bytes
        byte[] oaBytes;
        oaBytes = PhoneNumberUtils.networkPortionToCalledPartyBCD(originalAddress);

        if (oaBytes != null) {
            // originalAddress is phone number
            bo.write((oaBytes.length - 1) * 2
                    - ((oaBytes[oaBytes.length - 1] & 0xf0) == 0xf0 ? 1 : 0));
            bo.write(oaBytes, 0, oaBytes.length);
        } else {
            try {
                oaBytes = GsmAlphabet.stringToGsm7BitPacked(originalAddress);
            } catch (EncodeException ex) {
                Rlog.d(LOG_TAG, "ex:" + ex);
            }

            if (oaBytes != null) {
                // originalAddress is alphanumeric
                bo.write((oaBytes.length - 1) * 2);
                Rlog.d(LOG_TAG, "oaBytes length = " + oaBytes.length);

                bo.write((SmsAddress.TON_ALPHANUMERIC << 4) | 0x80);
                // The first byte of oaBytes is data length, thus copy from index 1.
                bo.write(oaBytes, 1, oaBytes.length - 1);
            } else {
                Rlog.d(LOG_TAG, "write a empty address for deliver pdu");
                bo.write(0);
                bo.write(PhoneNumberUtils.TOA_International);
            }
        }

        // write PID
        bo.write(0);

        return bo;
    }

    private static boolean encodeStringWithSpecialLang(
            CharSequence msgBody,
            int language,
            TextEncodingDetails ted) {

        int septets;

        //1st, try default GSM
        //septets = GsmAlphabet.countGsmSeptetsWithTable(
        //        msgBody, -1, -1);
        septets = GsmAlphabet.countGsmSeptetsUsingTables(
                msgBody, true, 0, 0);
        if (septets != -1) {

            ted.codeUnitCount = septets;
            if (septets > MAX_USER_DATA_SEPTETS) {
                ted.msgCount = (septets / MAX_USER_DATA_SEPTETS_WITH_HEADER) + 1;
                ted.codeUnitsRemaining = MAX_USER_DATA_SEPTETS_WITH_HEADER
                        - (septets % MAX_USER_DATA_SEPTETS_WITH_HEADER);
            } else {
                ted.msgCount = 1;
                ted.codeUnitsRemaining = MAX_USER_DATA_SEPTETS - septets;
            }
            ted.codeUnitSize = ENCODING_7BIT;
            ted.shiftLangId = -1;
            Rlog.d(LOG_TAG, "Try Default: " + language + " " + ted);
            return true;
        }

        //2nd, try locking shift
        //septets = GsmAlphabet.countGsmSeptetsWithTable(
        //        msgBody, -1, language);
        septets = GsmAlphabet.countGsmSeptetsUsingTables(
                msgBody, true, 0, language);
        if (septets != -1) {

            int headerElt[] = {SmsHeader.ELT_ID_NATIONAL_LANGUAGE_LOCKING_SHIFT, 0xffff};
            int maxLength = computeRemainUserDataLength(true, headerElt);

            ted.codeUnitCount = septets;
            if (septets > maxLength) {
                headerElt[1] = SmsHeader.ELT_ID_CONCATENATED_8_BIT_REFERENCE;
                maxLength = computeRemainUserDataLength(true, headerElt);

                ted.msgCount = (septets / maxLength) + 1;
                ted.codeUnitsRemaining = maxLength - (septets % maxLength);
            } else {
                ted.msgCount = 1;
                ted.codeUnitsRemaining = maxLength - septets;
            }
            ted.codeUnitSize = ENCODING_7BIT;
            ted.useLockingShift = true;
            ted.shiftLangId = language;
            Rlog.d(LOG_TAG, "Try Locking Shift: " + language + " " + ted);
            return true;
        }

        //3rd, try single shift
        //septets = GsmAlphabet.countGsmSeptetsWithTable(
        //        msgBody, language, -1);
        septets = GsmAlphabet.countGsmSeptetsUsingTables(
                msgBody, true, language, 0);
        if (septets != -1) {

            int headerElt[] = {SmsHeader.ELT_ID_NATIONAL_LANGUAGE_SINGLE_SHIFT, 0xffff};
            int maxLength = computeRemainUserDataLength(true, headerElt);

            ted.codeUnitCount = septets;
            if (septets > maxLength) {
                headerElt[1] = SmsHeader.ELT_ID_CONCATENATED_8_BIT_REFERENCE;
                maxLength = computeRemainUserDataLength(true, headerElt);

                ted.msgCount = (septets / maxLength) + 1;
                ted.codeUnitsRemaining = maxLength - (septets % maxLength);
            } else {
                ted.msgCount = 1;
                ted.codeUnitsRemaining = maxLength - septets;
            }
            ted.codeUnitSize = ENCODING_7BIT;
            ted.useSingleShift = true;
            ted.shiftLangId = language;
            Rlog.d(LOG_TAG, "Try Single Shift: " + language + " " + ted);
            return true;
        }

        //4th, try locking and single shift
        //septets = GsmAlphabet.countGsmSeptetsWithTable(
        //        msgBody, language, language);
        septets = GsmAlphabet.countGsmSeptetsUsingTables(
                msgBody, true, language, language);
        if (septets != -1) {
            int headerElt[] = {
                    SmsHeader.ELT_ID_NATIONAL_LANGUAGE_LOCKING_SHIFT,
                    SmsHeader.ELT_ID_NATIONAL_LANGUAGE_SINGLE_SHIFT,
                    0xffff};
            int maxLength = computeRemainUserDataLength(true, headerElt);

            ted.codeUnitCount = septets;
            if (septets > maxLength) {
                headerElt[2] = SmsHeader.ELT_ID_CONCATENATED_8_BIT_REFERENCE;
                maxLength = computeRemainUserDataLength(true, headerElt);

                ted.msgCount = (septets / maxLength) + 1;
                ted.codeUnitsRemaining = maxLength - (septets % maxLength);
            } else {
                ted.msgCount = 1;
                ted.codeUnitsRemaining = maxLength - septets;
            }
            ted.codeUnitSize = ENCODING_7BIT;
            ted.useLockingShift = true;
            ted.useSingleShift = true;
            ted.shiftLangId = language;
            Rlog.d(LOG_TAG, "Try Locking & Single Shift: " + language + " " + ted);
            return true;
        }

        Rlog.d(LOG_TAG, "Use UCS2" + language + " " + ted);
        return false;
    }

    private static int getCurrentSysLanguage() {
        int ret;
        String language;

        language = SystemProperties.get("persist.sys.language", null);
        if (language == null) {
            language = SystemProperties.get("ro.product.locale.language", null);
        }

        if (language.equals("tr")) {
            // ret = GsmAlphabet.SHIFT_ID_TURKISH;
            ret = -1;
        } else {
            ret = -1;
        }

        return ret;
    }

    public static int computeRemainUserDataLength(boolean inSeptets, int headerElt[]) {
        int headerBytes = 0;
        int count;
        for (int i = 0; i < headerElt.length; i++) {
            switch (headerElt[i]) {
                case SmsHeader.ELT_ID_CONCATENATED_8_BIT_REFERENCE:
                    headerBytes += MtkSmsHeader.CONCATENATED_8_BIT_REFERENCE_LENGTH;
                    break;
                case SmsHeader.ELT_ID_NATIONAL_LANGUAGE_SINGLE_SHIFT:
                    headerBytes += MtkSmsHeader.NATIONAL_LANGUAGE_SINGLE_SHIFT_LENGTH;
                    break;
                case SmsHeader.ELT_ID_NATIONAL_LANGUAGE_LOCKING_SHIFT:
                    headerBytes += MtkSmsHeader.NATIONAL_LANGUAGE_LOCKING_SHIFT_LENGTH;
                    break;
                default:
                    break;
            }
        }

        if (headerBytes != 0) {
            headerBytes++; // header length
        }

        count = MAX_USER_DATA_BYTES - headerBytes;
        if (inSeptets) {
            count = count * 8 / 7;
        }

        //Log.d(LOG_TAG, "computeRemainUserDataLength: inSeptets: "+ inSeptets +
        //        " , max: "+ count + " header:" + headerBytes);

        return count;
    }

    // MTK-START [ALPS00094531] Orange feature SMS Encoding Type Setting by mtk80589 in 2011.11.22
    /**
     * Calculate the number of septets needed to encode the message.
     *
     * @param msgBody the message to encode
     * @param use7bitOnly ignore (but still count) illegal characters if true
     * @param encodingType text encoding type(7-bit, 16-bit or automatic)
     * @return TextEncodingDetails
     */
    public static TextEncodingDetails calculateLength(CharSequence msgBody,
            boolean use7bitOnly, int encodingType) {
        CharSequence newMsgBody = null;
        Resources r = Resources.getSystem();
        if (r.getBoolean(com.android.internal.R.bool.config_sms_force_7bit_encoding)) {
            newMsgBody  = Sms7BitEncodingTranslator.translate(msgBody, false);
        }
        if (TextUtils.isEmpty(newMsgBody)) {
            newMsgBody = msgBody;
        }
        TextEncodingDetails ted = GsmAlphabet.countGsmSeptets(newMsgBody, use7bitOnly);

        if (encodingType == ENCODING_16BIT) {
            Rlog.d(LOG_TAG, "input mode is unicode");
            ted = null;
        }
        if (ted == null) {
            Rlog.d(LOG_TAG, "7-bit encoding fail");
            return SmsMessageBase.calcUnicodeEncodingDetails(newMsgBody);
        }
        return ted;
    }
    // MTK-END [ALPS00094531] Orange feature SMS Encoding Type Setting by mtk80589 in 2011.11.22

    /**
     * Get an SMS-SUBMIT PDU for a destination address and a message using the
     * specified encoding.
     *
     * @param scAddress Service Centre address.  Null means use default.
     * @param encoding Encoding defined by constants in android.telephony.SmsMessage.ENCODING_*
     * @param languageTable
     * @param languageShiftTable
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     * @hide
     */
    public static SubmitPdu getSubmitPdu(String scAddress,
            String destinationAddress, String message,
            boolean statusReportRequested, byte[] header, int encoding,
            int languageTable, int languageShiftTable, int validityPeriod) {

        // Perform null parameter checks.
        if (message == null || destinationAddress == null) {
            return null;
        }

        if (encoding == ENCODING_UNKNOWN) {
            // Find the best encoding to use
            TextEncodingDetails ted = calculateLength(message, false);
            encoding = ted.codeUnitSize;
            languageTable = ted.languageTable;
            languageShiftTable = ted.languageShiftTable;

            if (encoding == ENCODING_7BIT && (languageTable != 0 || languageShiftTable != 0)) {
                if (header != null) {
                    SmsHeader smsHeader = SmsHeader.fromByteArray(header);
                    if (smsHeader.languageTable != languageTable
                            || smsHeader.languageShiftTable != languageShiftTable) {
                        Rlog.w(LOG_TAG, "Updating language table in SMS header: "
                                + smsHeader.languageTable + " -> " + languageTable + ", "
                                + smsHeader.languageShiftTable + " -> " + languageShiftTable);
                        smsHeader.languageTable = languageTable;
                        smsHeader.languageShiftTable = languageShiftTable;
                        header = SmsHeader.toByteArray(smsHeader);
                    }
                } else {
                    MtkSmsHeader smsHeader = (MtkSmsHeader)makeSmsHeader();
                    smsHeader.languageTable = languageTable;
                    smsHeader.languageShiftTable = languageShiftTable;
                    header = SmsHeader.toByteArray(smsHeader);
                }
            }
        }

        SubmitPdu ret = new SubmitPdu();

        int validityPeriodFormat = VALIDITY_PERIOD_FORMAT_NONE;
        int relativeValidityPeriod = INVALID_VALIDITY_PERIOD;

        // TP-Validity-Period-Format (TP-VPF) in 3GPP TS 23.040 V6.8.1 section 9.2.3.3
        //bit 4:3 = 10 - TP-VP field present - relative format
        if((relativeValidityPeriod = getRelativeValidityPeriod(validityPeriod)) >= 0) {
            validityPeriodFormat = VALIDITY_PERIOD_FORMAT_RELATIVE;
        }

        byte mtiByte = (byte)(0x01 | (validityPeriodFormat << 0x03) |
                (header != null ? 0x40 : 0x00));

        ByteArrayOutputStream bo = getSubmitPduHead(
                scAddress, destinationAddress, mtiByte,
                statusReportRequested, ret);

        // User Data (and length)
        byte[] userData;
        try {
            if (encoding == ENCODING_7BIT) {
                userData = GsmAlphabet.stringToGsm7BitPackedWithHeader(message, header,
                        languageTable, languageShiftTable);
            } else { // assume UCS-2
                try {
                    userData = encodeUCS2(message, header);
                } catch (UnsupportedEncodingException uex) {
                    Rlog.e(LOG_TAG,
                            "Implausible UnsupportedEncodingException ",
                            uex);
                    return null;
                } catch (EncodeException ucs2Ex) {
                    Rlog.e(LOG_TAG,
                            "Implausible EncodeException ",
                            ucs2Ex);
                    return null;
                }
            }
        } catch (EncodeException ex) {
            // Encoding to the 7-bit alphabet failed. Let's see if we can
            // send it as a UCS-2 encoded message
            try {
                userData = encodeUCS2(message, header);
                encoding = ENCODING_16BIT;
            } catch (UnsupportedEncodingException uex) {
                Rlog.e(LOG_TAG,
                        "Implausible UnsupportedEncodingException ",
                        uex);
                return null;
            } catch (EncodeException ucs2Ex) {
                Rlog.e(LOG_TAG,
                        "Implausible EncodeException ",
                        ucs2Ex);
                return null;
            }
        }

        if (encoding == ENCODING_7BIT) {
            if ((0xff & userData[0]) > MAX_USER_DATA_SEPTETS) {
                // Message too long
                // MTK-START [mtk04070][111223][ALPS00106134]Merge to ICS 4.0.3
                Rlog.e(LOG_TAG, "Message too long (" + (0xff & userData[0]) + " septets)");
                // MTK-END [mtk04070][111223][ALPS00106134]Merge to ICS 4.0.3
                return null;
            }
            // TP-Data-Coding-Scheme
            // Default encoding, uncompressed
            // To test writing messages to the SIM card, change this value 0x00
            // to 0x12, which means "bits 1 and 0 contain message class, and the
            // class is 2". Note that this takes effect for the sender. In other
            // words, messages sent by the phone with this change will end up on
            // the receiver's SIM card. You can then send messages to yourself
            // (on a phone with this change) and they'll end up on the SIM card.
            bo.write(0x00);
        } else { // assume UCS-2
            if ((0xff & userData[0]) > MAX_USER_DATA_BYTES) {
                // Message too long
                // MTK-START [mtk04070][111223][ALPS00106134]Merge to ICS 4.0.3
                Rlog.e(LOG_TAG, "Message too long (" + (0xff & userData[0]) + " bytes)");
                // MTK-END [mtk04070][111223][ALPS00106134]Merge to ICS 4.0.3
                return null;
            }
            // TP-Data-Coding-Scheme
            // UCS-2 encoding, uncompressed
            bo.write(0x08);
        }

        if (validityPeriod >= 0 && validityPeriod <= 255) {
            Rlog.d(LOG_TAG, "write validity period into pdu: " + validityPeriod);
            bo.write(validityPeriod);
        }


        if (validityPeriodFormat == VALIDITY_PERIOD_FORMAT_RELATIVE) {
            // ( TP-Validity-Period - relative format)
            bo.write(relativeValidityPeriod);
        }

        bo.write(userData, 0, userData.length);
        ret.encodedMessage = bo.toByteArray();
        return ret;
    }
    /**
     * Get the current encoding type, ex. 7 bit or UCS2.
     * App can use the encoding type to check if all Google emoticons content. It has bug while
     * the emoticons codec cross 2 segments, it will have the broken content and shows to end user.
     * Therefore, app can check if the encoding type is equal to 16bit. If it is equal to 16 bit,
     * app is able to merge all content and display as correct emoticons.
     *
     * @return ENCODING_UNKNOWN, ENCODING_7BIT, ENCODING_8BIT or ENCODING_16BIT
     */
    public int getEncodingType() {
        return mEncodingType;
    }
}
