/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "SmsMessage.h"
#include "PduParser.h"
#include "SmsHeader.h"
#include "SmsAddress.h"
#include "GsmSmsAddress.h"
#include "SmsParserUtils.h"
#include "RfxLog.h"

#define RFX_LOG_TAG   "SmsMessage"
/*****************************************************************************
 * Class SmsMessage
 *****************************************************************************/
SmsMessage::SmsMessage() :
        mPdu(NULL),
        mScAddress(""),
        mMti(0),
        mReplyPathPresent(false),
        mProtocolIdentifier(0),
        mOriginatingAddress(NULL),
        mScTimeMillis(0),
        mDataCodingScheme(0),
        mIsMwi(false),
        mMwiSense(false),
        mMwiDontStore(false),
        mVoiceMailCount(-1),
        mUserData(NULL),
        mUserDataLength(0),
        mUserDataHeader(NULL),
        mMessageClass(CLASS_UNKNOWN),
        mEmailFrom(""),
        mEmailBody(""),
        mMessageBody(""),
        mIsEmail(false),
        mCur(0),
        mEncodingType(ENCODING_UNKNOWN) {
}

SmsMessage::~SmsMessage() {
    if (mPdu != NULL) {
        delete mPdu;
        mPdu = NULL;
    }
    if (mOriginatingAddress != NULL) {
        delete mOriginatingAddress;
        mOriginatingAddress = NULL;
    }
    if (mUserDataHeader != NULL) {
        delete mUserDataHeader;
        mUserDataHeader = NULL;
    }
    if (mUserData != NULL) {
        delete mUserData;
        mUserData = NULL;
    }
}
SmsMessage* SmsMessage::createFromPdu(BYTE *pdu, int length) {
    RFX_LOG_D(RFX_LOG_TAG, "createFromPdu begin");
    SmsMessage *smsMessage = new SmsMessage();
    smsMessage->parsePdu(pdu, length);
    RFX_LOG_D(RFX_LOG_TAG, "createFromPdu end");
    return smsMessage;
}

void SmsMessage::parsePdu(BYTE *pdu, int length) {
    mPdu = pdu;
    PduParser *parser = new PduParser(pdu, length);
    mScAddress =  parser->getSCAddress();
    if (!mScAddress.empty()) {
        // log:
    }
    int firstByte = parser->getByte();
    mMti = firstByte & 0x3;
    RFX_LOG_D(RFX_LOG_TAG, "createFromPdu mMti: %d", mMti);
    switch (mMti) {
        // TP-Message-Type-Indicator
        // 9.2.3
        case 0:
        case 3:  // GSM 03.40 9.2.3.1: MTI == 3 is Reserved.
            // This should be processed in the same way as MTI == 0 (Deliver)
            parseSmsDeliver(parser, firstByte);
            break;
        case 1:
            // parseSmsSubmit(p, firstByte);
            break;
        case 2:
            // parseSmsStatusReport(p, firstByte);
            break;
        default:
            // TODO(mkf) the rest of these
            break;
    }
    delete parser;
}

void SmsMessage::parseSmsDeliver(PduParser *parser, int firstByte) {
    mReplyPathPresent = (firstByte & 0x80) == 0x80;

    mOriginatingAddress = parser->getAddress();
    if (mOriginatingAddress != NULL) {
        // print log
    }
    // TP-Protocol-Identifier (TP-PID)
    // TS 23.040 9.2.3.9
    mProtocolIdentifier = parser->getByte();
    RFX_LOG_D(RFX_LOG_TAG, "createFromPdu mProtocolIdentifier: %d", mProtocolIdentifier);

    // TP-Data-Coding-Scheme
    // see TS 23.038
    mDataCodingScheme = parser->getByte();
    RFX_LOG_D(RFX_LOG_TAG, "createFromPdu mDataCodingScheme: %d", mDataCodingScheme);

    // to do:
    // print log
    mScTimeMillis = parser->getSCTimestampMillis();
    bool hasUserDataHeader = (firstByte & 0x40) == 0x40;
    RFX_LOG_D(RFX_LOG_TAG, "createFromPdu hasUserDataHeader: %d", hasUserDataHeader);
    parseUserData(parser, hasUserDataHeader);
    RFX_LOG_D(RFX_LOG_TAG, "createFromPdu parseSmsDeliver end");
}

void SmsMessage::parseUserData(PduParser *parser, bool hasUserDataHeader) {
    bool hasMessageClass = false;
    bool userDataCompressed = false;

    int encodingType = ENCODING_UNKNOWN;
    // Look up the data encoding scheme
    if ((mDataCodingScheme & 0x80) == 0) {
        userDataCompressed = (0 != (mDataCodingScheme & 0x20));
        hasMessageClass = (0 != (mDataCodingScheme & 0x10));
        RFX_LOG_D(RFX_LOG_TAG, "parseUserData userDataCompressed: %d", userDataCompressed);
        RFX_LOG_D(RFX_LOG_TAG, "parseUserData hasMessageClass: %d", hasMessageClass);
        if (userDataCompressed) {
            // print log
        } else {
            switch ((mDataCodingScheme >> 2) & 0x3) {
            case 0:  // GSM 7 bit default alphabet
                encodingType = ENCODING_7BIT;
                break;

            case 2:  // UCS 2 (16bit)
                encodingType = ENCODING_16BIT;
                break;

            case 1:  // 8 bit data
            case 3:  // reserved
                // Print log
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
        bool active = ((mDataCodingScheme & 0x08) == 0x08);
        // bit 0x04 reserved

        // VM - If TP-UDH is present, these values will be overwritten
        if ((mDataCodingScheme & 0x03) == 0x00) {
            mIsMwi = true;  /* Indicates vmail */
            mMwiSense = active;  /* Indicates vmail notification set/clear */
            mMwiDontStore = ((mDataCodingScheme & 0xF0) == 0xC0);

            /* Set voice mail count based on notification bit */
            if (active == true) {
                mVoiceMailCount = -1;  // unknown number of messages waiting
            } else {
                mVoiceMailCount = 0;  // no unread messages
            }
        } else {
            mIsMwi = false;
        }
    } else if ((mDataCodingScheme & 0xC0) == 0x80) {
        // 3GPP TS 23.038 V7.0.0 (2006-03) section 4
        // 0x80..0xBF == Reserved coding groups
        if (mDataCodingScheme == 0x84) {
            // This value used for KSC5601 by carriers in Korea.
            encodingType = ENCODING_KSC5601;
        } else {
            // Print log
        }
    } else {
        // Print log
    }
    RFX_LOG_D(RFX_LOG_TAG, "parseUserData encodingType: %d", encodingType);
    // set both the user data and the user data header.
    int count = parser->constructUserData(hasUserDataHeader,
            encodingType == ENCODING_7BIT);
    mUserData = parser->getUserData();
    mUserDataLength = parser->getUserDataLength();
    RFX_LOG_D(RFX_LOG_TAG, "parseUserData mUserDataLength: %d", mUserDataLength);
    mUserDataHeader = parser->getUserDataHeader();

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
    if (hasUserDataHeader && (mUserDataHeader->mSpecialSmsMsgList.size() != 0)) {
        for (list<SpecialSmsMsg>::iterator iter = mUserDataHeader->mSpecialSmsMsgList.begin();
                iter != mUserDataHeader->mSpecialSmsMsgList.end(); iter++) {
            SpecialSmsMsg msg = *iter;
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
                if (mVoiceMailCount > 0) {
                    mMwiSense = true;
                } else {
                    mMwiSense = false;
                }
            } else {
                // Print log
            }
        }  // end of for
    }  // end of if UDH
    switch (encodingType) {
    case ENCODING_UNKNOWN:
        // mMessageBody = NULL;
        break;

    case ENCODING_8BIT:
        // Support decoding the user data payload as pack GSM 8-bit (a GSM alphabet string
        // that's stored in 8-bit unpacked format) characters.
        // mMessageBody = parser.getUserDataGSM8bit(count);
        break;

    case ENCODING_7BIT:
        // mMessageBody = parser.getUserDataGSM7Bit(count,
                // hasUserDataHeader ? mUserDataHeader.mLanguageTable : 0,
                // hasUserDataHeader ? mUserDataHeader.mLanguageShiftTable : 0);
        break;

    case ENCODING_16BIT:
        // mMessageBody = parser.getUserDataUCS2(count);
        break;

    case ENCODING_KSC5601:
        // mMessageBody = parser.getUserDataKSC5601(count);
        // mMessageBody = NULL;
        break;
    }

    // if (VDBG) Rlog.v(LOG_TAG, "SMS message body (raw): '" + mMessageBody + "'");

    if (!mMessageBody.empty()) {
        parseMessageBody();
    }

    if (!hasMessageClass) {
        mMessageClass = CLASS_UNKNOWN;
    } else {
        switch (mDataCodingScheme & 0x3) {
        case 0:
            mMessageClass = CLASS_0;
            break;
        case 1:
            mMessageClass = CLASS_1;
            break;
        case 2:
            mMessageClass = CLASS_2;
            break;
        case 3:
            mMessageClass = CLASS_3;
            break;
        }
    }
    RFX_LOG_D(RFX_LOG_TAG, "parseUserData mMessageClass: %d", mMessageClass);
}

void SmsMessage::parseMessageBody() {
    if (mOriginatingAddress != NULL && mOriginatingAddress->couldBeEmailGateway() &&
            !isReplace()) {
        extractEmailAddressFromMessageBody();
    }
}
bool SmsMessage::isReplace() {
    return (mProtocolIdentifier & 0xc0) == 0x40
            && (mProtocolIdentifier & 0x3f) > 0
            && (mProtocolIdentifier & 0x3f) < 8;
}
BYTE* SmsMessage::getUserData() {
    return mUserData;
}

int SmsMessage::getUserDataLength() {
    return mUserDataLength;
}

int SmsMessage::getRefNumber() {
    if (mUserDataHeader != NULL) {
        return (mUserDataHeader->mConcatRef).refNumber;
    }
    return -1;
}

int SmsMessage::getSeqNumber() {
    if (mUserDataHeader != NULL) {
        return (mUserDataHeader->mConcatRef).seqNumber;
    }
    return -1;
}

int SmsMessage::getTotalSegnumber() {
    if (mUserDataHeader != NULL) {
        return (mUserDataHeader->mConcatRef).msgCount;
    }
    return -1;
}

void SmsMessage::extractEmailAddressFromMessageBody() {
    /* Some carriers may use " /" delimiter as below
     *
     * 1. [x@y][ ]/[subject][ ]/[body]
     * -or-
     * 2. [x@y][ ]/[body]
     */
    // String[] parts = mMessageBody.split("( /)|( )", 2);
    // if (parts.length < 2) return;
    // mEmailFrom = parts[0];
    // mEmailBody = parts[1];
    // mIsEmail = Telephony.Mms.isEmailAddress(mEmailFrom);
}

bool SmsMessage::isWapush() {
    if (mUserDataHeader != NULL) {
        return (mUserDataHeader->mPortAddrs).destPort == SmsHeader::PORT_WAP_PUSH;
    }
    return false;
}

bool SmsMessage::isSmsForSUPL() {
    if (mUserDataHeader != NULL) {
        return (mUserDataHeader->mPortAddrs).destPort == SmsHeader::PORT_SUPL;
    }
    return false;
}

bool SmsMessage::isConcentratedSms() {
    return (mUserDataHeader != NULL) &&
            ((mUserDataHeader->mConcatRef).msgCount != 0) &&
            ((mUserDataHeader->mConcatRef).seqNumber != -1);
}

const string WappushMessage::MIME_TYPE_SUPL = "application/vnd.omaloc-supl-init";
const string WappushMessage::XWAP_APP_ID_SUPL = "x-oma-application:ulp.ua";
const string WappushMessage::MIME_TYPE_SUPL_HEX = "786";
const string WappushMessage::XWAP_APP_ID_SUPL_HEX = "16";

WappushMessage::WappushMessage(BYTE *pdu, int length) :
        mPduLength(length),
        mUserData(NULL),
        mUserDataLength(0) {
    mTransactionId = -1;
    mPduType = -1;
    mWapAppId = -1;
    mContentType = "";
    mPdu = new BYTE[length];
    memcpy(mPdu, pdu, length);
}

WappushMessage::~WappushMessage() {
    if (mPdu != NULL) {
        delete[] mPdu;
        mPdu = NULL;
    }
    if (mUserData != NULL && mUserData != mPdu) {
        delete mUserData;
        mUserData = NULL;
    }
}

void WappushMessage::parsePdu() {
    int index = 0;
    mTransactionId = mPdu[index++] & 0xFF;
    mPduType = mPdu[index++] & 0xFF;
    RFX_LOG_D(RFX_LOG_TAG, "parsePdu mTransactionId: %d, mPduType: %d", mTransactionId, mPduType);
    if ((mPduType != WappushPduParser::PDU_TYPE_PUSH) &&
            (mPduType != WappushPduParser::PDU_TYPE_CONFIRMED_PUSH)) {
        return;
    }
    WappushPduParser *pduDecoder = new WappushPduParser(mPdu, mPduLength);
    /**
     * Parse HeaderLen(unsigned integer).
     * From wap-230-wsp-20010705-a section 8.1.2
     * The maximum size of a uintvar is 32 bits.
     * So it will be encoded in no more than 5 octets.
     */
    if (pduDecoder->decodeUintvarInteger(index) == false) {
        delete pduDecoder;
        return;
    }
    int headerLength = (int) pduDecoder->getValue32();
    index += pduDecoder->getDecodedDataLength();
    RFX_LOG_D(RFX_LOG_TAG, "parsePdu headerLength: %d, index: %d", headerLength, index);
    int headerStartIndex = index;
    /**
     * Parse Content-Type.
     * From wap-230-wsp-20010705-a section 8.4.2.24
     *
     * Content-type-value = Constrained-media | Content-general-form
     * Content-general-form = Value-length Media-type
     * Media-type = (Well-known-media | Extension-Media) *(Parameter)
     * Value-length = Short-length | (Length-quote Length)
     * Short-length = <Any octet 0-30>   (octet <= WAP_PDU_SHORT_LENGTH_MAX)
     * Length-quote = <Octet 31>         (WAP_PDU_LENGTH_QUOTE)
     * Length = Uintvar-integer
     */
    if (pduDecoder->decodeContentType(index) == false) {
        RFX_LOG_D(RFX_LOG_TAG, "parsePdu received PDU Header Content-Type error.");
        delete pduDecoder;
        return;
    }
    string mimeType = pduDecoder->getValueString();
    long binaryContentType = pduDecoder->getValue32();
    index += pduDecoder->getDecodedDataLength();
    RFX_LOG_D(RFX_LOG_TAG, "parsePdu mimeType: %s, binaryContentType: %d, index: %d",
            mimeType.c_str(), (int)binaryContentType, index);
    BYTE* header = new BYTE[headerLength];
    memcpy(header, mPdu + headerStartIndex, headerLength);
    pduDecoder->decodeHeaders(index, headerLength - index + headerStartIndex);

    if (!mimeType.empty() && (mimeType.compare(WappushPduParser::CONTENT_TYPE_B_PUSH_CO) == 0)) {
        mUserData = mPdu;
        mUserDataLength = mPduLength;
    } else {
        int dataIndex = headerStartIndex + headerLength;
        mUserData = new BYTE[mPduLength - dataIndex];
        memcpy(mUserData, mPdu + dataIndex, mPduLength - dataIndex);
        mUserDataLength = mPduLength - dataIndex;
    }
    RFX_LOG_D(RFX_LOG_TAG, "parsePdu mUserDataLength: %d", mUserDataLength);
    if (pduDecoder->seekXWapApplicationId(index, index + headerLength - 1)) {
        index = (int) pduDecoder->getValue32();
        pduDecoder->decodeXWapApplicationId(index);
        string wapAppId = pduDecoder->getValueString();
        if (wapAppId.empty()) {
            wapAppId = PhoneNumberUtils::numToString((int) pduDecoder->getValue32());
        }
        mWapAppId = wapAppId;

        mContentType = ((mimeType.empty()) ?
                PhoneNumberUtils::numToString(binaryContentType) : mimeType);
        RFX_LOG_D(RFX_LOG_TAG, "parsePdu mWapAppId: %s, mContentType: %s",
                mWapAppId.c_str(), mContentType.c_str());
    }
    delete[] header;
    delete pduDecoder;
}

bool WappushMessage::isWapushForSUPL() {
    if ((mContentType.compare(MIME_TYPE_SUPL) == 0 || mContentType.compare(MIME_TYPE_SUPL_HEX) == 0) &&
            (mWapAppId.compare(XWAP_APP_ID_SUPL) == 0 ||
            mWapAppId.compare(XWAP_APP_ID_SUPL_HEX) == 0)) {
        return true;
    }
    return false;
}

BYTE* WappushMessage::getUserData() {
    return mUserData;
}

int WappushMessage::getUserDataLength() {
    return mUserDataLength;
}
