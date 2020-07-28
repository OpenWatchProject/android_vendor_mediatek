/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#ifndef __RTC_SMS_MESSAGE_H__
#define __RTC_SMS_MESSAGE_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RtcSmsHeader.h"
#include "sms/RmcCdmaSmsConverter.h"

/*****************************************************************************
 * Class Declaration
 *****************************************************************************/
class RtcSmsAddress;
class RtcGsmSmsAddress;

/*****************************************************************************
 * External Functions
 *****************************************************************************/
extern unsigned char * convertHexStringToBytes(unsigned char *hexString, size_t dataLen);

/*****************************************************************************
 * RtcSmsMessage
 *****************************************************************************/
class RtcSmsMessage {
// Constructor / Destructor
public:
    // Constructor
    RtcSmsMessage();

    // Destructor
    virtual ~RtcSmsMessage();

// External Method
public:
    // Check if the SMS is concatenated SMS.
    //
    // RETURNS: true if the SMS is concatenated SMS
    bool isConcatSms() {
        return (mUserDataHeader != NULL) &&
                (!mUserDataHeader->isError()) &&
                (mUserDataHeader->isConcatSms());
    }

    // Get user data header pointer.
    //
    // RETURNS: User data header pointer
    RtcSmsUserDataHeader *getUserDataHeader() {
        return mUserDataHeader;
    }

    // Get the pointer of RtcSmsAddress.
    //
    // RETURNS: pointer of RtcSmsAddress
    RtcSmsAddress *getSmsAddress() {
        return mSmsAddress;
    }

    // Check if error happen or not when encoding or decoding.
    //
    // RETURNS: true for error
    bool isError() {
        return mError;
    }

    // Get the message encoding type.
    //
    // RETURNS: the encoding type
    int getEncodingType() const {
        return mEncodingType;
    }

    // Get the number of field.
    //
    // RETURNS: field number
    int getNumField() const {
        return mNumField;
    }

    // Get the user data payload.
    //
    // RETURNS: user data payload
    const Vector<unsigned char> &getUserDataPayload() const {
        return mUserDataPayload;
    }

// Constants
public:
    static const int ENCODING_UNKNOWN = 0;
    static const int ENCODING_7BIT    = 1;
    static const int ENCODING_8BIT    = 2;
    static const int ENCODING_16BIT   = 3;

// Method for sub class
protected:
    void setError(
        bool error = true
    ) {
        mError = error;
    }

    void setEncodingType(
        int encodingType
    ) {
        mEncodingType = encodingType;
    }

    void setNumberField(
        int numField
    ) {
        mNumField = numField;
    }

    void createSmsUserDataHeader(
        unsigned char *header,
        int len,
        bool is3Gpp2
    );

    void setSmsAddress(
        RtcSmsAddress *addr
    ) {
        mSmsAddress = addr;
    }

    Vector<unsigned char> &getUDP() {
        return mUserDataPayload;
    }

    void setUDP(
        Vector<unsigned char> &udp
    ) {
         mUserDataPayload = udp;
    }

// Implement
private:
    RtcSmsAddress* mSmsAddress;
    RtcSmsUserDataHeader *mUserDataHeader;
    int mEncodingType;
    int mNumField;
    bool mError;
    Vector<unsigned char> mUserDataPayload;
};

/*****************************************************************************
 * GsmSmsMessage
 *****************************************************************************/
class RtcGsmSmsMessage : public RtcSmsMessage {
// Constructor/Destructor
public:
    // Construct a message with a hex PDU array.
    RtcGsmSmsMessage(
        unsigned char *hexString,  // [IN] The Hex Pdu
        size_t dataLen             // [IN] The data length
    );

    // Construct a message with a base RtcGsmSmsMessage
    // and another Cdma SMS' user data payload.
    RtcGsmSmsMessage(
        const RtcGsmSmsMessage &message,   // [IN] the base RtcGsmSmsMessage
        int encodingType,                  // [IN] the encoding type of the CDMA SMS message
        int numFields,                     // [IN] the field number of the CDMA SMS message
        const Vector<unsigned char> &userDataPayload // [IN] user data payload of CDMA SMS message
    );

    // Destructor
    virtual ~RtcGsmSmsMessage();

// External Method
public:
    int getLengthExceptUserData() const {
        return mLenExceptUserData;
    }

    const String8 &getHexPdu() const {
        return mHexPdu;
    }

// Implement
private:
    void parse();
    void parseSmsDeliver(int firstByte);
    int parseDataCodingScheme(int dataCodingScheme);
    int getByte();

// Implement
private:
    unsigned char* mPdu;
    String8 mHexPdu;
    int mCur;
    int mLenExceptUserData;
    int mDataLen;
};

/*****************************************************************************
 * RtcCdmaSmsMessage
 *****************************************************************************/
class RtcCdmaSmsMessage : public RtcSmsMessage {
// Constructor / Destructor
public:
    // Construct with RIL message.
    RtcCdmaSmsMessage(
        RIL_CDMA_SMS_Message *pMessage // [IN] the RIL message
    );

    // Construct with one Cdma message and another GSM message's user data payload.
    RtcCdmaSmsMessage(
        const RtcCdmaSmsMessage &message,  // [IN] the base CDMA message
        int encodingType,                  // [IN] GSM message's encoding type
        int numFields,                     // [IN] GSM message's field number
        const Vector<unsigned char> &userDataPayload  // [IN] GSM message's user data payload
    );

    // Destructor
    virtual ~RtcCdmaSmsMessage() {}

// External Method
public:
    // Get the reference of RIL message.
    //
    // RETURNS: the reference of message
    const RIL_CDMA_SMS_Message &getMessage() const {
        return mMsg;
    }

// Constants
private:
    static const int TELESERVICE_WMT               = 0x1002;
    static const int TELESERVICE_WEMT              = 0x1005;

// Implement
private:
    RIL_CDMA_SMS_Message mMsg;
};
#endif /* __RTC_SMS_MESSAGE_H__ */