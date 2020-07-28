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

import android.os.Binder;
import android.os.Parcel;
import android.content.res.Resources;
import android.text.TextUtils;

import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;

import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;
import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.SmsMessageBase;
import com.android.internal.telephony.SmsMessageBase.SubmitPduBase;
import com.android.internal.telephony.Sms7BitEncodingTranslator;

import java.lang.Math;
import java.util.ArrayList;
import java.util.Arrays;

import static android.telephony.TelephonyManager.PHONE_TYPE_CDMA;

// MTK-START
import com.android.internal.telephony.SmsHeader;
// MTK-END

/**
 * A Short Message Service message.
 * @see android.provider.Telephony.Sms.Intents#getMessagesFromIntent
 */
public class MtkSmsMessage extends SmsMessage {
    private static final String LOG_TAG = "MtkSmsMessage";

    // MTK-START
    /*
     * Detail description:
     * Constants for message waiting indication type
     */
    /** Message Waiting Indication
     *  @hide
     */
    public static final int MWI_VOICEMAIL = 0;
    /** Message Waiting Indication
     *  @hide
     */
    public static final int MWI_FAX = 1;
    /** Message Waiting Indication
     *  @hide
     */
    public static final int MWI_EMAIL = 2;
    /** Message Waiting Indication
     *  @hide
     */
    public static final int MWI_OTHER = 3;
    /** Message Waiting Indication
     *  @hide
     */
    public static final int MWI_VIDEO = 7;
    // MTK-END


    /** Keep the format of the SmsMessage.
     *
     * @hide
     */
    private String mFormat;

    private MtkSmsMessage(SmsMessageBase smb) {
        super(smb);
    }

    /**
     * Create an SmsMessage from a raw PDU. Guess format based on Voice
     * technology first, if it fails use other format.
     * All applications which handle
     * incoming SMS messages by processing the {@code SMS_RECEIVED_ACTION} broadcast
     * intent <b>must</b> now pass the new {@code format} String extra from the intent
     * into the new method {@code createFromPdu(byte[], String)} which takes an
     * extra format parameter. This is required in order to correctly decode the PDU on
     * devices that require support for both 3GPP and 3GPP2 formats at the same time,
     * such as dual-mode GSM/CDMA and CDMA/LTE phones.
     * @deprecated Use {@link #createFromPdu(byte[], String)} instead.
     */
    @Deprecated
    public static MtkSmsMessage createFromPdu(byte[] pdu) {
         MtkSmsMessage message = null;

        // cdma(3gpp2) vs gsm(3gpp) format info was not given,
        // guess from active voice phone type
        int activePhone = TelephonyManager.getDefault().getCurrentPhoneType();
        String format = (PHONE_TYPE_CDMA == activePhone) ?
                SmsConstants.FORMAT_3GPP2 : SmsConstants.FORMAT_3GPP;
        message = createFromPdu(pdu, format);

        if (null == message || null == message.mWrappedSmsMessage) {
            // decoding pdu failed based on activePhone type, must be other format
            format = (PHONE_TYPE_CDMA == activePhone) ?
                    SmsConstants.FORMAT_3GPP : SmsConstants.FORMAT_3GPP2;
            message = createFromPdu(pdu, format);
        }
        return message;
    }

    /**
     * Create an SmsMessage from a raw PDU with the specified message format. The
     * message format is passed in the
     * {@link android.provider.Telephony.Sms.Intents#SMS_RECEIVED_ACTION} as the {@code format}
     * String extra, and will be either "3gpp" for GSM/UMTS/LTE messages in 3GPP format
     * or "3gpp2" for CDMA/LTE messages in 3GPP2 format.
     *
     * @param pdu the message PDU from the
     * {@link android.provider.Telephony.Sms.Intents#SMS_RECEIVED_ACTION} intent
     * @param format the format extra from the
     * {@link android.provider.Telephony.Sms.Intents#SMS_RECEIVED_ACTION} intent
     */
    public static MtkSmsMessage createFromPdu(byte[] pdu, String format) {
        SmsMessageBase wrappedMessage;

        if (SmsConstants.FORMAT_3GPP2.equals(format)) {
            wrappedMessage = com.android.internal.telephony.cdma.SmsMessage.createFromPdu(pdu);
            wrappedMessage = com.mediatek.internal.telephony.cdma.MtkSmsMessage.newMtkSmsMessage(
                    (com.android.internal.telephony.cdma.SmsMessage) wrappedMessage);
        } else if (SmsConstants.FORMAT_3GPP.equals(format)) {
            wrappedMessage = com.mediatek.internal.telephony.gsm.MtkSmsMessage.createFromPdu(pdu);
        } else {
            Rlog.e(LOG_TAG, "createFromPdu(): unsupported message format " + format);
            return null;
        }

        if (wrappedMessage != null) {
            MtkSmsMessage msg = new MtkSmsMessage(wrappedMessage);
            msg.mFormat = format;
            return msg;
        } else {
            Rlog.e(LOG_TAG, "createFromPdu(): wrappedMessage is null");
            return null;
        }
    }

    /**
     * TS 27.005 3.4.1 lines[0] and lines[1] are the two lines read from the
     * +CMT unsolicited response (PDU mode, of course)
     *  +CMT: [&lt;alpha>],<length><CR><LF><pdu>
     *
     * Only public for debugging and for RIL
     *
     * {@hide}
     */
    public static MtkSmsMessage newFromCMT(String[] lines) {
        // received SMS in 3GPP format
        SmsMessageBase wrappedMessage =
                com.mediatek.internal.telephony.gsm.MtkSmsMessage.newFromCMT(lines);

        if (wrappedMessage != null) {
            MtkSmsMessage msg = new MtkSmsMessage(wrappedMessage);
            msg.mFormat = SmsConstants.FORMAT_3GPP;
            return msg;
        } else {
            Rlog.e(LOG_TAG, "newFromCMT(): wrappedMessage is null");
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
        SmsMessageBase wrappedMessage = null;

        if (isCdmaVoice()) {
            wrappedMessage = com.mediatek.internal.telephony.cdma.MtkSmsMessage.createFromEfRecord(
                    index, data);
        } else {
            wrappedMessage = com.mediatek.internal.telephony.gsm.MtkSmsMessage.createFromEfRecord(
                    index, data);
        }

        if (wrappedMessage != null) {
            MtkSmsMessage msg = new MtkSmsMessage(wrappedMessage);
            if (isCdmaVoice()) {
                msg.mFormat = SmsConstants.FORMAT_3GPP2;
            } else {
                msg.mFormat = SmsConstants.FORMAT_3GPP;
            }
            return msg;
        } else {
            Rlog.e(LOG_TAG, "createFromEfRecord(): wrappedMessage is null");
            return null;
        }
    }

    /**
     * Get the TP-Layer-Length for the given SMS-SUBMIT PDU Basically, the
     * length in bytes (not hex chars) less the SMSC header
     *
     * FIXME: This method is only used by a CTS test case that isn't run on CDMA devices.
     * We should probably deprecate it and remove the obsolete test case.
     */
    public static int getTPLayerLengthForPDU(String pdu) {
        if (isCdmaVoice()) {
            return com.android.internal.telephony.cdma.SmsMessage.getTPLayerLengthForPDU(pdu);
        } else {
            return com.mediatek.internal.telephony.gsm.MtkSmsMessage.getTPLayerLengthForPDU(pdu);
        }
    }

    /*
     * TODO(cleanup): It would make some sense if the result of
     * preprocessing a message to determine the proper encoding (i.e.
     * the resulting data structure from calculateLength) could be
     * passed as an argument to the actual final encoding function.
     * This would better ensure that the logic behind size calculation
     * actually matched the encoding.
     */

    /**
     * Calculates the number of SMS's required to encode the message body and
     * the number of characters remaining until the next message.
     *
     * @param msgBody the message to encode
     * @param use7bitOnly if true, characters that are not part of the
     *         radio-specific 7-bit encoding are counted as single
     *         space chars.  If false, and if the messageBody contains
     *         non-7-bit encodable characters, length is calculated
     *         using a 16-bit encoding.
     * @return an int[4] with int[0] being the number of SMS's
     *         required, int[1] the number of code units used, and
     *         int[2] is the number of code units remaining until the
     *         next message. int[3] is an indicator of the encoding
     *         code unit size (see the ENCODING_* definitions in SmsConstants)
     */
    public static int[] calculateLength(CharSequence msgBody, boolean use7bitOnly) {
        // this function is for MO SMS
        TextEncodingDetails ted = (useCdmaFormatForMoSms()) ?
            com.android.internal.telephony.cdma.SmsMessage.calculateLength(msgBody, use7bitOnly,
                    true) :
            com.mediatek.internal.telephony.gsm.MtkSmsMessage.calculateLength(msgBody, use7bitOnly);
        int ret[] = new int[4];
        ret[0] = ted.msgCount;
        ret[1] = ted.codeUnitCount;
        ret[2] = ted.codeUnitsRemaining;
        ret[3] = ted.codeUnitSize;
        return ret;
    }

    /**
     * Divide a message text into several fragments, none bigger than
     * the maximum SMS message text size.
     *
     * @param text text, must not be null.
     * @return an <code>ArrayList</code> of strings that, in order,
     *   comprise the original msg text
     *
     * @hide
     */
    public static ArrayList<String> fragmentText(String text) {
        // This function is for MO SMS
        final boolean isCdma = useCdmaFormatForMoSms();
        TextEncodingDetails ted = isCdma ?
            com.android.internal.telephony.cdma.SmsMessage.calculateLength(text, false, true) :
            com.mediatek.internal.telephony.gsm.MtkSmsMessage.calculateLength(text, false);

        // TODO(cleanup): The code here could be rolled into the logic
        // below cleanly if these MAX_* constants were defined more
        // flexibly...

        int limit;
        if (ted.codeUnitSize == SmsConstants.ENCODING_7BIT) {
            int udhLength;
            if (ted.languageTable != 0 && ted.languageShiftTable != 0) {
                udhLength = GsmAlphabet.UDH_SEPTET_COST_TWO_SHIFT_TABLES;
            } else if (ted.languageTable != 0 || ted.languageShiftTable != 0) {
                udhLength = GsmAlphabet.UDH_SEPTET_COST_ONE_SHIFT_TABLE;
            } else {
                udhLength = 0;
            }

            if (ted.msgCount > 1) {
                udhLength += GsmAlphabet.UDH_SEPTET_COST_CONCATENATED_MESSAGE;
            }

            if (udhLength != 0) {
                udhLength += GsmAlphabet.UDH_SEPTET_COST_LENGTH;
            }

            limit = SmsConstants.MAX_USER_DATA_SEPTETS - udhLength;
        } else {
            if (ted.msgCount > 1) {
                limit = SmsConstants.MAX_USER_DATA_BYTES_WITH_HEADER;
                // If EMS is not supported, break down EMS into single segment SMS
                // and add page info " x/y".
                // In the case of UCS2 encoding, we need 8 bytes for this,
                // but we only have 6 bytes from UDH, so truncate the limit for
                // each segment by 2 bytes (1 char).
                // Make sure total number of segments is less than 10.
                if (!hasEmsSupport() && ted.msgCount < 10) {
                    limit -= 2;
                }
            } else {
                limit = SmsConstants.MAX_USER_DATA_BYTES;
            }
        }

        String newMsgBody = null;
        Resources r = Resources.getSystem();
        if (r.getBoolean(com.android.internal.R.bool.config_sms_force_7bit_encoding)) {
            newMsgBody  = Sms7BitEncodingTranslator.translate(text, isCdma);
        }
        if (TextUtils.isEmpty(newMsgBody)) {
            newMsgBody = text;
        }
        int pos = 0;  // Index in code units.
        int textLen = newMsgBody.length();
        ArrayList<String> result = new ArrayList<String>(ted.msgCount);
        while (pos < textLen) {
            int nextPos = 0;  // Counts code units.
            if (ted.codeUnitSize == SmsConstants.ENCODING_7BIT) {
                if (useCdmaFormatForMoSms() && ted.msgCount == 1) {
                    // For a singleton CDMA message, the encoding must be ASCII...
                    nextPos = pos + Math.min(limit, textLen - pos);
                } else {
                    // For multi-segment messages, CDMA 7bit equals GSM 7bit encoding (EMS mode).
                    nextPos = GsmAlphabet.findGsmSeptetLimitIndex(newMsgBody, pos, limit,
                            ted.languageTable, ted.languageShiftTable);
                }
            } else {  // Assume unicode.
                nextPos = SmsMessageBase.findNextUnicodePosition(pos, limit, newMsgBody);
            }
            if ((nextPos <= pos) || (nextPos > textLen)) {
                Rlog.e(LOG_TAG, "fragmentText failed (" + pos + " >= " + nextPos + " or " +
                          nextPos + " >= " + textLen + ")");
                break;
            }
            result.add(newMsgBody.substring(pos, nextPos));
            pos = nextPos;
        }
        return result;
    }

    /**
     * Calculates the number of SMS's required to encode the message body and
     * the number of characters remaining until the next message, given the
     * current encoding.
     *
     * @param messageBody the message to encode
     * @param use7bitOnly if true, characters that are not part of the radio
     *         specific (GSM / CDMA) alphabet encoding are converted to as a
     *         single space characters. If false, a messageBody containing
     *         non-GSM or non-CDMA alphabet characters are encoded using
     *         16-bit encoding.
     * @return an int[4] with int[0] being the number of SMS's required, int[1]
     *         the number of code units used, and int[2] is the number of code
     *         units remaining until the next message. int[3] is the encoding
     *         type that should be used for the message.
     */
    public static int[] calculateLength(String messageBody, boolean use7bitOnly) {
        return calculateLength((CharSequence)messageBody, use7bitOnly);
    }

    /*
     * TODO(cleanup): It looks like there is now no useful reason why
     * apps should generate pdus themselves using these routines,
     * instead of handing the raw data to SMSDispatcher (and thereby
     * have the phone process do the encoding).  Moreover, CDMA now
     * has shared state (in the form of the msgId system property)
     * which can only be modified by the phone process, and hence
     * makes the output of these routines incorrect.  Since they now
     * serve no purpose, they should probably just return null
     * directly, and be deprecated.  Going further in that direction,
     * the above parsers of serialized pdu data should probably also
     * be gotten rid of, hiding all but the necessarily visible
     * structured data from client apps.  A possible concern with
     * doing this is that apps may be using these routines to generate
     * pdus that are then sent elsewhere, some network server, for
     * example, and that always returning null would thereby break
     * otherwise useful apps.
     */

    /**
     * Get an SMS-SUBMIT PDU for a destination address and a message.
     * This method will not attempt to use any GSM national language 7 bit encodings.
     *
     * @param scAddress Service Centre address.  Null means use default.
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     */
    public static SubmitPdu getSubmitPdu(String scAddress,
            String destinationAddress, String message, boolean statusReportRequested) {
        SubmitPduBase spb;

        if (useCdmaFormatForMoSms()) {
            spb = com.android.internal.telephony.cdma.SmsMessage.getSubmitPdu(scAddress,
                    destinationAddress, message, statusReportRequested, null);
        } else {
            spb = com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddress,
                    destinationAddress, message, statusReportRequested);
        }

        return new SubmitPdu(spb);
    }

    /**
     * Get an SMS-SUBMIT PDU for a data message to a destination address &amp; port.
     * This method will not attempt to use any GSM national language 7 bit encodings.
     *
     * @param scAddress Service Centre address. null == use default
     * @param destinationAddress the address of the destination for the message
     * @param destinationPort the port to deliver the message to at the
     *        destination
     * @param data the data for the message
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     */
    public static SubmitPdu getSubmitPdu(String scAddress,
            String destinationAddress, short destinationPort, byte[] data,
            boolean statusReportRequested) {
        SubmitPduBase spb;

        if (useCdmaFormatForMoSms()) {
            spb = com.android.internal.telephony.cdma.SmsMessage.getSubmitPdu(scAddress,
                    destinationAddress, destinationPort, data, statusReportRequested);
        } else {
            spb = com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddress,
                    destinationAddress, destinationPort, data, statusReportRequested);
        }

        return new SubmitPdu(spb);
    }

    // MTK-START
    /** This method returns the reference to a specific
     *  SmsMessage object, which is used for accessing its static methods.
     * @return Specific SmsMessage.
     *
     * @hide
     */
    private static final SmsMessageBase getSmsFacility() {
        if (isCdmaVoice()) {
            return new com.mediatek.internal.telephony.cdma.MtkSmsMessage();
        } else {
            return new com.mediatek.internal.telephony.gsm.MtkSmsMessage();
        }
    }

    /**
     * Constructor
     *
     * @hide
     */
    public MtkSmsMessage() {
        this(getSmsFacility());
        if (isCdmaVoice()) {
            mFormat = SmsConstants.FORMAT_3GPP2;
        } else {
            mFormat = SmsConstants.FORMAT_3GPP;
        }
    }

    /** @hide */
    public static MtkSmsMessage newFromCDS(byte[] pdu) {
        // received SMS delivery report in 3GPP format
        SmsMessageBase wrappedMessage =
                com.mediatek.internal.telephony.gsm.MtkSmsMessage.newFromCDS(pdu);
        MtkSmsMessage msg = new MtkSmsMessage(wrappedMessage);
        msg.mFormat = SmsConstants.FORMAT_3GPP;
        return msg;
    }

    /**
     * Get an SMS-SUBMIT PDU for a destination address and a message
     *
     * @param scAddress Service Centre address.  Null means use default.
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     * @hide
     */
    public static SubmitPdu getSubmitPdu(String scAddress,
            String destinationAddress, String message,
            boolean statusReportRequested, byte[] header) {
        SubmitPduBase spb;
        if (useCdmaFormatForMoSms()) {
            spb = com.android.internal.telephony.cdma.SmsMessage.getSubmitPdu(scAddress,
                    destinationAddress, message, statusReportRequested,
                    SmsHeader.fromByteArray(header));
        } else {
            spb = com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddress,
                    destinationAddress, message, statusReportRequested, header);
        }

        return new SubmitPdu(spb);
    }

    /**
     * Get an SMS-SUBMIT PDU for a data message to a destination address with original port
     *
     * @param scAddress Service Centre address. null == use default
     * @param destinationAddress the address of the destination for the message
     * @param destinationPort the port to deliver the message to at the
     *        destination
     * @param originalPort the port to deliver the message from
     * @param data the data for the message
     * @return a <code>SubmitPdu</code> containing the encoded SC
     *         address, if applicable, and the encoded message.
     *         Returns null on encode error.
     *
     * @hide
     */
    public static SubmitPdu getSubmitPdu(String scAddress, String destinationAddress,
                short destinationPort, short originalPort, byte[] data,
                boolean statusReportRequested) {
        Rlog.d(LOG_TAG, "[xj android.telephony.SmsMessage getSubmitPdu");
        SubmitPduBase spb = null;
        if (useCdmaFormatForMoSms()) {
            spb = com.mediatek.internal.telephony.cdma.MtkSmsMessage.getSubmitPdu(scAddress,
                    destinationAddress, destinationPort, data, statusReportRequested);
        } else {
            spb = com.mediatek.internal.telephony.gsm.MtkSmsMessage.getSubmitPdu(scAddress,
                    destinationAddress, destinationPort, originalPort, data, statusReportRequested);
        }

        if (spb != null) {
            return new SubmitPdu(spb);
        }
        return null;
    }

    /**
     * @return the destination address (receiver) of this SMS message in String
     * form or null if unavailable
     *
     * @hide
     * @internal
     */
    public String getDestinationAddress() {
        if (SmsConstants.FORMAT_3GPP2.equals(mFormat)) {
            return ((com.mediatek.internal.telephony.cdma.MtkSmsMessage) mWrappedSmsMessage).
                    getDestinationAddress();
        } else {
            return ((com.mediatek.internal.telephony.gsm.MtkSmsMessage)mWrappedSmsMessage).
                    getDestinationAddress();
        }
    }

    /**
     * Return the user data header (UDH).
     *
     * @hide
     * @internal
     */
    public SmsHeader getUserDataHeader() {
        return mWrappedSmsMessage.getUserDataHeader();
    }

    /**
     * Returns the smsc raw data from the pdu
     *
     * @return the raw smsc for the message.
     *
     * @hide
     * @internal
     */
    public byte[] getSmsc() {
        Rlog.d(LOG_TAG, "getSmsc");
        byte[] pdu = this.getPdu();

        if (isCdma()) {
            Rlog.d(LOG_TAG, "getSmsc with CDMA and return null");
            return null;
        }

        if (pdu == null) {
            Rlog.d(LOG_TAG, "pdu is null");
            return null;
        }

        int smscLen = (pdu[0] & 0xff) + 1;
        byte[] smsc = new byte[smscLen];

        try {
            System.arraycopy(pdu, 0, smsc, 0, smsc.length);
            return smsc;
        } catch (ArrayIndexOutOfBoundsException e) {
            Rlog.e(LOG_TAG, "Out of boudns");
            return null;
        }
    }

    /**
     * Returns the tpdu from the pdu
     *
     * @return the tpdu for the message.
     *
     * @hide
     * @internal
     */
    public byte[] getTpdu() {
        Rlog.d(LOG_TAG, "getTpdu");
        byte[] pdu = this.getPdu();

        if (isCdma()) {
            Rlog.d(LOG_TAG, "getSmsc with CDMA and return null");
            return pdu;
        }

        if (pdu == null) {
            Rlog.d(LOG_TAG, "pdu is null");
            return null;
        }

        int smscLen = (pdu[0] & 0xff) + 1;
        int tpduLen = pdu.length - smscLen;
        byte[] tpdu = new byte[tpduLen];

        try {
            System.arraycopy(pdu, smscLen, tpdu, 0, tpdu.length);
            return tpdu;
        } catch (ArrayIndexOutOfBoundsException e) {
            Rlog.e(LOG_TAG, "Out of boudns");
            return null;
        }
    }

    /**
     * Calculates the number of SMS's required to encode the message body and
     * the number of characters remaining until the next message.
     *
     * @param msgBody the message to encode
     * @param use7bitOnly if true, characters that are not part of the
     *         radio-specific 7-bit encoding are counted as single
     *         space chars.  If false, and if the messageBody contains
     *         non-7-bit encodable characters, length is calculated
     *         using a 16-bit encoding.
     * @param encodingType text encoding type(7-bit, 16-bit or automatic)
     * @return an int[4] with int[0] being the number of SMS's
     *         required, int[1] the number of code units used, and
     *         int[2] is the number of code units remaining until the
     *         next message. int[3] is an indicator of the encoding
     *         code unit size (see the ENCODING_* definitions in this
     *         class).
     * @hide
     * @internal
     */
    public static int[] calculateLength(CharSequence msgBody, boolean use7bitOnly,
            int encodingType) {
        TextEncodingDetails ted = (useCdmaFormatForMoSms()) ?
                com.mediatek.internal.telephony.cdma.MtkSmsMessage.calculateLength(
                msgBody, use7bitOnly, encodingType) :
                com.mediatek.internal.telephony.gsm.MtkSmsMessage.calculateLength(
                msgBody, use7bitOnly, encodingType);
        int ret[] = new int[4];
        ret[0] = ted.msgCount;
        ret[1] = ted.codeUnitCount;
        ret[2] = ted.codeUnitsRemaining;
        ret[3] = ted.codeUnitSize;
        return ret;
    }

    /**
     * Divide a message text into several fragments, none bigger than
     * the maximum SMS message text size.
     *
     * @param text text, must not be null.
     * @param encodingType text encoding type(7-bit, 16-bit or automatic)
     * @return an <code>ArrayList</code> of strings that, in order,
     *   comprise the original msg text
     *
     * @hide
     */
    public static ArrayList<String> fragmentText(String text, int encodingType) {
        final boolean isCdma = useCdmaFormatForMoSms();
        TextEncodingDetails ted = isCdma ?
                com.mediatek.internal.telephony.cdma.MtkSmsMessage.calculateLength(
                text, false, encodingType) :
                com.mediatek.internal.telephony.gsm.MtkSmsMessage.calculateLength(
                text, false, encodingType);

        // TODO(cleanup): The code here could be rolled into the logic
        // below cleanly if these MAX_* constants were defined more
        // flexibly...

        int limit;
        if (ted.codeUnitSize == SmsConstants.ENCODING_7BIT) {
            int udhLength;
            if (ted.languageTable != 0 && ted.languageShiftTable != 0) {
                udhLength = GsmAlphabet.UDH_SEPTET_COST_TWO_SHIFT_TABLES;
            } else if (ted.languageTable != 0 || ted.languageShiftTable != 0) {
                udhLength = GsmAlphabet.UDH_SEPTET_COST_ONE_SHIFT_TABLE;
            } else {
                udhLength = 0;
            }

            if (ted.msgCount > 1) {
                udhLength += GsmAlphabet.UDH_SEPTET_COST_CONCATENATED_MESSAGE;
            }

            if (udhLength != 0) {
                udhLength += GsmAlphabet.UDH_SEPTET_COST_LENGTH;
            }

            limit = SmsConstants.MAX_USER_DATA_SEPTETS - udhLength;
        } else {
            if (ted.msgCount > 1) {
                limit = SmsConstants.MAX_USER_DATA_BYTES_WITH_HEADER;
                // If EMS is not supported, break down EMS into single segment SMS
                // and add page info " x/y".
                // In the case of UCS2 encoding, we need 8 bytes for this,
                // but we only have 6 bytes from UDH, so truncate the limit for
                // each segment by 2 bytes (1 char).
                // Make sure total number of segments is less than 10.
                if (!hasEmsSupport() && ted.msgCount < 10) {
                    limit -= 2;
                }
            } else {
                limit = SmsConstants.MAX_USER_DATA_BYTES;
            }
        }

        String newMsgBody = null;
        Resources r = Resources.getSystem();
        if (r.getBoolean(com.android.internal.R.bool.config_sms_force_7bit_encoding)) {
            newMsgBody  = Sms7BitEncodingTranslator.translate(text, isCdma);
        }
        if (TextUtils.isEmpty(newMsgBody)) {
            newMsgBody = text;
        }
        int pos = 0;  // Index in code units.
        int textLen = newMsgBody.length();
        ArrayList<String> result = new ArrayList<String>(ted.msgCount);
        while (pos < textLen) {
            int nextPos = 0;  // Counts code units.
            if (ted.codeUnitSize == SmsConstants.ENCODING_7BIT) {
                if (useCdmaFormatForMoSms() && ted.msgCount == 1) {
                    // For a singleton CDMA message, the encoding must be ASCII...
                    nextPos = pos + Math.min(limit, textLen - pos);
                } else {
                    // For multi-segment messages, CDMA 7bit equals GSM 7bit encoding (EMS mode).
                    nextPos = GsmAlphabet.findGsmSeptetLimitIndex(newMsgBody, pos, limit,
                            ted.languageTable, ted.languageShiftTable);
                }
            } else {  // Assume unicode.
                nextPos = SmsMessageBase.findNextUnicodePosition(pos, limit, newMsgBody);
            }
            if ((nextPos <= pos) || (nextPos > textLen)) {
                Rlog.e(LOG_TAG, "fragmentText failed (" + pos + " >= " + nextPos + " or " +
                          nextPos + " >= " + textLen + ")");
                break;
            }
            result.add(newMsgBody.substring(pos, nextPos));
            pos = nextPos;
        }
        return result;
    }

    /**
     * Divide a message text into several fragments, none bigger than
     * the maximum SMS message text size using TextEncodingDetails.
     *
     * @param subId subscription identity
     * @param text text, must not be null.
     * @param ted text encodingdetails
     *
     * @return an <code>ArrayList</code> of strings that, in order,
     *   comprise the original msg text
     *
     * @hide
     */
    public ArrayList<String> fragmentTextUsingTed(int subId, String text,
            TextEncodingDetails ted) {
        boolean useCdmaFormat = false;

        if (!SmsManager.getSmsManagerForSubscriptionId(subId).isImsSmsSupported()) {
            // use Voice technology to determine SMS format.
            useCdmaFormat = (TelephonyManager.getDefault().getCurrentPhoneType()
                    == TelephonyManager.PHONE_TYPE_CDMA);
        } else {
            // IMS is registered with SMS support, check the SMS format supported
            useCdmaFormat = (SmsConstants.FORMAT_3GPP2.
                    equals(SmsManager.getSmsManagerForSubscriptionId(subId).getImsSmsFormat()));
        }

        int limit;
        if (ted.codeUnitSize == SmsConstants.ENCODING_7BIT) {
            int udhLength;
            if (ted.languageTable != 0 && ted.languageShiftTable != 0) {
                udhLength = GsmAlphabet.UDH_SEPTET_COST_TWO_SHIFT_TABLES;
            } else if (ted.languageTable != 0 || ted.languageShiftTable != 0) {
                udhLength = GsmAlphabet.UDH_SEPTET_COST_ONE_SHIFT_TABLE;
            } else {
                udhLength = 0;
            }

            if (ted.msgCount > 1) {
                udhLength += GsmAlphabet.UDH_SEPTET_COST_CONCATENATED_MESSAGE;
            }

            if (udhLength != 0) {
                udhLength += GsmAlphabet.UDH_SEPTET_COST_LENGTH;
            }

            limit = SmsConstants.MAX_USER_DATA_SEPTETS - udhLength;
        } else {
            if (ted.msgCount > 1) {
                limit = SmsConstants.MAX_USER_DATA_BYTES_WITH_HEADER;
                // If EMS is not supported, break down EMS into single segment SMS
                // and add page info " x/y".
                // In the case of UCS2 encoding, we need 8 bytes for this,
                // but we only have 6 bytes from UDH, so truncate the limit for
                // each segment by 2 bytes (1 char).
                // Make sure total number of segments is less than 10.
                if (!hasEmsSupport() && ted.msgCount < 10) {
                    limit -= 2;
                }
            } else {
                limit = SmsConstants.MAX_USER_DATA_BYTES;
            }
        }

        String newMsgBody = null;
        Resources r = Resources.getSystem();
        if (r.getBoolean(com.android.internal.R.bool.config_sms_force_7bit_encoding)) {
            newMsgBody  = Sms7BitEncodingTranslator.translate(text, useCdmaFormat);
        }
        if (TextUtils.isEmpty(newMsgBody)) {
            newMsgBody = text;
        }
        int pos = 0;  // Index in code units.
        int textLen = newMsgBody.length();
        ArrayList<String> result = new ArrayList<String>(ted.msgCount);
        while (pos < textLen) {
            int nextPos = 0;  // Counts code units.
            if (ted.codeUnitSize == SmsConstants.ENCODING_7BIT) {
                if (useCdmaFormat && ted.msgCount == 1) {
                    // For a singleton CDMA message, the encoding must be ASCII...
                    nextPos = pos + Math.min(limit, textLen - pos);
                } else {
                    // For multi-segment messages, CDMA 7bit equals GSM 7bit encoding (EMS mode).
                    nextPos = GsmAlphabet.findGsmSeptetLimitIndex(newMsgBody, pos, limit,
                            ted.languageTable, ted.languageShiftTable);
                }
            } else {  // Assume unicode.
                nextPos = SmsMessageBase.findNextUnicodePosition(pos, limit, newMsgBody);
            }
            if ((nextPos <= pos) || (nextPos > textLen)) {
                Rlog.e(LOG_TAG, "fragmentText failed (" + pos + " >= " + nextPos + " or " +
                          nextPos + " >= " + textLen + ")");
                break;
            }
            result.add(newMsgBody.substring(pos, nextPos));
            pos = nextPos;
        }
        return result;
    }

    /**
     * create SmsMessage by format
     * @hide
     */
    public static MtkSmsMessage createFromEfRecord(int index, byte[] data, String format) {
        SmsMessageBase wrappedMessage = null;
        MtkSmsMessage msg = null;

        Rlog.d(LOG_TAG, "createFromEfRecord(): format " + format);
        if (FORMAT_3GPP2.equals(format)) {
            wrappedMessage = com.mediatek.internal.telephony.cdma.MtkSmsMessage.createFromEfRecord(
                    index, data);
        } else if (FORMAT_3GPP.equals(format)) {
            wrappedMessage = com.mediatek.internal.telephony.gsm.MtkSmsMessage.createFromEfRecord(
                    index, data);
        } else {
            Rlog.e(LOG_TAG, "createFromEfRecord(): unsupported message format " + format);
            return null;
        }

        if (wrappedMessage != null) {
            msg = new MtkSmsMessage(wrappedMessage);
            msg.mFormat = format;
        }
        return msg;
    }

    /**
     * Determines whether or not to current phone type is cdma. This function is similar with
     * isCdmaVoice. Only one difference is this is the non-static function and can use mSubId.
     *
     * @return true if current phone type is cdma, false otherwise.
     */
    private boolean isCdma() {
        int activePhone = TelephonyManager.getDefault().getCurrentPhoneType(mSubId);
        return (PHONE_TYPE_CDMA == activePhone);
    }

    /**
     * Get the current encoding type, ex. 7 bit or UCS2.
     * App can use the encoding type to check if all Google emoticons content. It has bug while
     * the emoticons codec cross 2 segments, it will have the broken content and shows to end user.
     * Therefore, app can check if the encoding type is equal to 16bit. If it is equal to 16 bit,
     * app is able to merge all content and display as correct emoticons.
     *
     * @return ENCODING_UNKNOWN, ENCODING_7BIT, ENCODING_8BIT or ENCODING_16BIT
     * @hide
     */
    public int getEncodingType() {
        if (SmsConstants.FORMAT_3GPP2.equals(mFormat)) {
            return ENCODING_UNKNOWN;
        } else {
            return ((com.mediatek.internal.telephony.gsm.MtkSmsMessage)mWrappedSmsMessage).
                getEncodingType();
        }
    }

    /**
     * Determines whether or not to use CDMA format for MO SMS.
     * If SMS over IMS is supported, then format is based on IMS SMS format,
     * otherwise format is based on current phone type.
     *
     * @return true if Cdma format should be used for MO SMS, false otherwise.
     */
    // MTK-START
    // Modification for sub class
    /** {@hide} */
    protected static boolean useCdmaFormatForMoSms() {
    // MTK-END
        if (!MtkSmsManager.getDefault().isImsSmsSupported()) {
            // use Voice technology to determine SMS format.
            return isCdmaVoice();
        }
        // IMS is registered with SMS support, check the SMS format supported
        return (SmsConstants.FORMAT_3GPP2.equals(MtkSmsManager.getDefault().getImsSmsFormat()));
    }
    // MTK-END
}
