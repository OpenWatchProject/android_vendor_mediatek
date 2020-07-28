/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RP_CDMA_SMS_CODEC_H__
#define __RP_CDMA_SMS_CODEC_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxObject.h"
#include "RpCdmaSmsDefs.h"
#include "RpCdmaSmsLocalReqRsp.h"
#include "RpCdmaSmsMtUrc.h"
#include "RfxMessage.h"
#include "Parcel.h"
#include "utils/String8.h"

/*****************************************************************************
 * Name Space
 *****************************************************************************/
using namespace android;

/*****************************************************************************
 * Class RpCdmaSmsCodec
 *****************************************************************************/
class RpCdmaSmsCodec : public RfxObject {
    // Required: declare this class
    RFX_DECLARE_CLASS(RpCdmaSmsCodec);

public:
    RpCdmaSmsCodec();
    // Utility APIs
    // FIXME: need a better name
    /**
     * Decode the parcel of RIL_CDMA_SMS_Message to class
     * This codes is copied from
     *     constructCdmaSms@Ril.cpp
     *
     * @param pData A parcel data of RIL_CDMA_SMS_Message for input
     * @param rilCdmaSmsMsg class of RIL_CDMA_SMS_Message for output
     *
     * @return true if success; false if failed.
     */
    bool decodeRilCdmaSmsMsg(Parcel *pData, RIL_CDMA_SMS_Message *rilCdmaSmsMsg);
    // FIXME: need a better name
    /**
     * Decode the parcel of RIL_CDMA_SMS_Message to at commands.
     * This codes is copied from
     *     sms.c@c2k-rild
     *
     * @param rilCdmaSmsMsg Class of RIL_CDMA_SMS_Message
     *
     * @return provide pdu data of at commands if success. otherwise, it will return empty string
     *     The pdu data of at comamnds would be similar as AT+C2KCMGS=pdu_len, "pdu", "number"
     */
    String8 encodeC2kcmgs(RIL_CDMA_SMS_Message *rilCdmaSmsMsg);
    // FIXME: need a better name
    /**
     * Decode the parcel of String to result.
     * encodeC2kcmgs will help to generate the AT+C2KCMGS at commands. While response back,
     * this api will help to decode the result.
     * encodeRilCdmaSmsAck will help to generate the AT+C2KCNMA at commands. While response back,
     * this api will help to decode the result.
     *
     * @message message of at commdnas from GSM Modem
     *      It could be +C2KCMGS:<msg_id> or +C2K ERROR:<error_code>, <error class>
     *      It could be OK or ERROR
     *
     * @return class of mo response
     */
    RpCdmaSmsLocalReqRsp decodelocalReq(const sp<RfxMessage>& message);
    // FIXME: need a better name
    /**
     * Decode the parcel of String to result.
     * These codes is copied from
     *    oemDispatchNewCmdaSMS@ril_sms.c/c2k-ril
     * For 3GPP2 SMS over IMS new message flow, GSM modem will notify a URC,
     * +C2KCMGS:<alpha_id>
     * <length>, <pdu_data>
     *
     * @param pUrcData A parcel data of At commdnas from GSM Modem
     *      It could be
     *      +C2KCMT:<alpha_id>, <pdu_len>
     *      <pdu_data>
     *      But the GSM rild will by-pass the <pdu_data> only
     * @param rilCdmaSmsMsg A output class of RIL_CDMA_SMS_MESSAGE
     *
     * @return RpCdmaSmsMtUrc class for result
     */
    RpCdmaSmsMtUrc decodeC2kcmtUrc(Parcel *pUrcData, RIL_CDMA_SMS_Message *rilCdmaSmsMsg);
    // FIXME: need a better name
    /**
     * Decode the parcel of String to result.
     * These codes is copied from
     *    responseCdmaSms@ril.cpp/c2k-ril
     * For 3GPP2 SMS over IMS new message flow, GSM modem will notify a URC,
     * +C2KCMGS:<alpha_id>
     * <length>, <pdu_data>
     *
     * @param pUrcData A parcel data of At commdnas from GSM Modem
     *      It could be
     *      +C2KCMT:<alpha_id>, <pdu_len>
     *      <pdu_data>
     *      But the GSM rild will by-pass the <pdu_data> only
     * @param pRilCdmaSmsMsgData A output parcel data of RIL_CDMA_SMS_MESSAGE
     *
     * @return true if success; false if failed
     */
    bool encodeRilCdmaSmsMsg(RIL_CDMA_SMS_Message *rilCdmaSmsMsg,
            Parcel *pRilCdmaSmsMsgData);
    // FIXME: need a better name
    /**
     * Decode the parcel of RIL_CDMA_SMS_Message to at commands.
     * This codes is copied from
     *     constructCdmaSms@Ril.cpp
     *
     * @param pData A parcel data of RIL_CDMA_SMS_Message for input
     * @param rilCdmaSmsAck Class of RIL_CDMA_SMS_Ack for output
     *
     * @return true if success; false if failed.
     */
    bool decodeRilCdmaSmsAckMsg(Parcel *pData, RIL_CDMA_SMS_Ack *rilCdmaSmsAck);
    // FIXME: need a better name
    /**
     * Encode RIL_CDMA_SMS_Ack to at commands.
     * This codes is copied from
     *     dispatchCdmaSmsAck@ril.cpp
     *     requesCdmatSMSAcknowledge@ril_sms.c/c2k-ril
     *
     * @param rilCdmaSmsAck Class of RIL_CDMA_SMS_Ack
     * @param replySeqId Sequence id for previous new incoming message
     * @param mtAddress previous incoming address and should encode to this pdu
     *
     * @return provide pdu data of at commands if success. otherwise, it will return empty string
     *     The pdu data of at comamnds would be similar as AT+C2KCNMA=pdu_len, "pdu"
     */
    String8 encodeRilCdmaSmsAck(RIL_CDMA_SMS_Ack *rilCdmaSmsAck, int replySeqId,
            RIL_CDMA_SMS_Address *mtAddress);

// Override
protected:
    virtual void onInit();
    void log(String8 s);
    void logEncode(String8 s);
    void logDecode(String8 s);
    int min(int a, int b);
private:
    bool mDebug;
};


/*****************************************************************************
 * BitwiseOutputStream
 *****************************************************************************/
class BitwiseOutputStream {
// External Method
public:
    // Get the buffer of the stream
    //
    // RETURNS: the pointer of the buffer
    unsigned char *getBuf() {
        return m_buf;
    }

    // write some bits of the data
    //
    // RETURNS: void
    void write(
       int bits,      // [IN] the amount of data to write (gte 0, lte 8)
       uint32_t data  // [IN] the data to be write
    );

    // Get the bit length of the stream
    //
    // RETURNS: the bit length
    int getBitLen() const {
        return m_pos;
    }

    // Get the byte length of the stream
    //
    // RETURNS: the byte length
    int getByteLen() const;


// Constructor / Destructor
public:
    // Construct the stream with a start buffer size
    BitwiseOutputStream(
        int startLen    // [IN] the start buffer size of the stream
    );

    // Destructor
    ~BitwiseOutputStream() {
        delete [] m_buf;
        m_buf = NULL;
    }

// Implementations
private:
    unsigned char* m_buf;
    int m_pos;
    int m_end;
};


/*****************************************************************************
 * BitwiseInputStream
 *****************************************************************************/
class BitwiseInputStream {
// External Method
public:
    // Return the number of bit still available for reading
    //
    // RETURNS: bits available
    int available() {
        return m_end - m_pos;
    }

    // Read some data and increment the current position.
    //
    // RETURNS: the readed data
    unsigned char read(
        int bits     // [IN] bits the amount of data to read (gte 0, lte 8)
    );

    // Increment the current position and ignore contained data.
    //
    // RETURNS: None
    void skip(
        int bits    // [IN] the amount by which to increment the position
    );

// Constructor / Destructor
public:
    // Construct the stream with a buffer
    BitwiseInputStream(const unsigned char* buf, int len) :m_buf(buf), m_pos(0) {
        m_end = len << 3;
    }

// Implementations
private:
    const unsigned char* m_buf;
    int m_pos;
    int m_end;
};


/*****************************************************************************
 * CdmaSmsMessage
 *****************************************************************************/
class CdmaSmsMessage : public RfxObject {
    RFX_DECLARE_CLASS(CdmaSmsMessage);

// Const Value
public:
    static const int MESSAGE_TYPE_P2P        = 0x00;
    static const int MESSAGE_TYPE_BC         = 0x01;
    static const int MESSAGE_TYPE_ACK        = 0x02;
    static const size_t MESSAGE_TYPE_HEX_LEN = 2;

// External Method
public:
    // Convert the message type to hex string
    //
    // RETURNs: the hex string
    String8 toHexHeader() {
        return String8::format("%02x", m_type);
    }

    // Convert the message body to hex string
    //
    // RETURNS: the hex string
    String8 toHexBody();

    // Get the message type
    //
    // RETURN: get the message type
    int getType() const {
        return m_type;
    }

    // Get the error code
    //
    // RETURNS: the error code
    int getErr() const {
        return m_error;
    }

    // Check if point to point message
    //
    // RETURNS: true if point to point message
    bool isP2pSms() {
        return getType() == MESSAGE_TYPE_P2P;
    }

    // Check if broadcast message
    //
    // RETURNS: true if broadcast message
    bool isBroadcastSms() {
        return getType() == MESSAGE_TYPE_BC;
    }

// Constructor / Destructor
public:
    // Default Constructor
    CdmaSmsMessage() : m_type(MESSAGE_TYPE_P2P), m_error(0) {
    }

    // Constructor for construct the message by type
    CdmaSmsMessage(
        const int type    // [IN] Construct the message by type
    );

    // Constructor for construct the message by hexPdu from modem
    CdmaSmsMessage(
        const char* hexPdu, // [IN] type part of the hexPdu from modem
        int len             // [IN] length of the type part in hexPdu
    );

    // Destructor
    virtual ~CdmaSmsMessage() {
    }

// Implementation
private:
    int m_type;
    int m_error;
};


/*****************************************************************************
 * RmcTlvNode
 *****************************************************************************/
class RmcTlvNode: public RfxObject {
    RFX_DECLARE_CLASS(RmcTlvNode);

// External Method
public:
    // Convert the tag and length part to the hex string
    //
    // RETRURNS: the hex string
    String8 toHexHeader() {
        return String8::format("%02x%02x", m_tag, m_length);
    }

    // Convert the value part to the hex string
    //
    // RETURNS: the hex string
    String8 toHexBody();

// Constructor / Destructor
public:
    // Default Constructor
    RmcTlvNode() : m_tag(-1), m_length(-1), m_error(0) {
    }

    // Constructor for construct the node by tag and length
    RmcTlvNode(
        int tag,    // [IN] the tag of the TLV node
        int length  // [IN] the length of the TLV node
    );

    // Constructor for construct the node by the hexPdu from modem
    RmcTlvNode(
        const char* hexPdu, // [IN] A part of the hexPdu which corresponse to a TLV mode
        int len             // [IN] The length of the hexPdu
    );

// Implementation
protected:
    int m_tag;
    int m_length;
    int m_error;
    Vector<unsigned char> m_value;

    static char char2Int(char c);
    void hex2Bin(const char *src, int len);
};

/*****************************************************************************
 * TeleServiceId
 *****************************************************************************/
class TeleServiceId : public RmcTlvNode{
    RFX_DECLARE_CLASS(TeleServiceId);

// Const Value
public:
    static const int TELESERVICE_ID     = 0x00;
    static const int TELESERVICE_ID_LEN = 0x02;
    static const int CT_WAP_PUSH_TID    = 0xFDEA;

// External Method
public:
    // Get the teleservicd id
    //
    // RETURNS: the teleservice id
    int getTeleServiceId();

    // Check if the message is WAP push message from CT
    //
    // RETURNS: true if wap push message from CT
    bool isCtWapPush() {
        return (getTeleServiceId() == CT_WAP_PUSH_TID);
    }

// Constructor / Destructor
public:
    // Default Constructor
    TeleServiceId() {
    }

    // Constructor for construct the teleservice id by the id from RILJ
    TeleServiceId(
        int id     // [IN] the teleservice id from RILJ
    );

    // Constructor for construct the teleservice if by the hexPdu from modem
    TeleServiceId(
        const char* hexPdu, // [IN] the teleservice part of the hexPdu from modem
        int len             // [IN] the length of the hexPdu
    );

    virtual ~TeleServiceId() {
    }
};


/*****************************************************************************
 * ServiceCategory
 *****************************************************************************/
class ServiceCategory : public RmcTlvNode{
    RFX_DECLARE_CLASS(ServiceCategory);

// Const Value
public:
    static const int SERVICE_CATEGORY     = 0x01;
    static const int SERVICE_CATEGORY_LEN = 0x02;

// External Method
public:
    // Get the service category
    //
    // RETURNS: the service category
    int getServiceCategory();

// Constructor / Destructor
public:
    // Default Constructor
    ServiceCategory() {
    }

    // Constructor for construct the service category by the hexPdu from modem
    ServiceCategory(
        const char* hexPdu, // [IN] the service category part of the hexPdu from the modem
        int len             // [IN] the length of the hexPdu
    );

    // Destructor
    virtual ~ServiceCategory() {
    }
};


/*****************************************************************************
 * CdmaSmsAddress
 *****************************************************************************/
class CdmaSmsAddress : public RmcTlvNode{
    RFX_DECLARE_CLASS(CdmaSmsAddress);

public:
    static const int ORIG_ADDRESS = 0x02;
    static const int DEST_ADDRESS = 0x04;

// External Method
public:
    // Get the address
    //
    // return true if get the address successfully
    bool getAddress(
        RIL_CDMA_SMS_Address &addr // [OUT] the address
    );

    // Get the string format address
    //
    // return true if get the address successfully
    bool getAddress(
        String8 &addr   // [OUT] the address
    );

// Constructor / Destructor
public:
    // Default Constructor
    CdmaSmsAddress() {
    }

    // Constructor for construct the address by address from RILJ
    CdmaSmsAddress(
        RIL_CDMA_SMS_Address &addr, // [IN] the address
        bool isMo                   // [IN] MO or MT
    );

    // Constructor for construct the address by the hexPdu from modem
    CdmaSmsAddress(
        const char* hexPdu,    // the address part of the hexPdu from the modem
        int len                // the length of the hexPdu
    );

    // Destructor
    virtual ~CdmaSmsAddress() {
    }

private:
    String8 m_number;
    static unsigned char dtmf2Char(unsigned char dtmf);
};


/*****************************************************************************
 * CdmaSmsSubAddress
 *****************************************************************************/
class CdmaSmsSubAddress : public RmcTlvNode {
    RFX_DECLARE_CLASS(CdmaSmsSubAddress);

// Const Value
public:
    static const int ORIG_SUB_ADDR = 0x03;
    static const int DEST_SUB_ADDR = 0x05;

// External Method
public:
    // Get the sub address
    //
    // true if get the address successfully
    bool getSubAddr(
        RIL_CDMA_SMS_Subaddress &subAddr // [OUT] the sub address
    );

// Constructor / Destructor
public:
    // Default Constructor
    CdmaSmsSubAddress() {
    }

    // Constructor for construct the sub address by the hexPdu from the modem
    CdmaSmsSubAddress(
        const char* hexPdu,  // [IN] the sub address part of the hexPdu from the modem
        int len              // [IN] the length of the hexPdu
    );

    // Destructor
    virtual ~CdmaSmsSubAddress() {
    }
};

/*****************************************************************************
 * BearerReplyOpt
 *****************************************************************************/
class BearerReplyOpt: public RmcTlvNode {
    RFX_DECLARE_CLASS(BearerReplyOpt);

// Const Value
public:
    static const int BEARER_REPLY_OPT     = 0x06;
    static const int BEARER_REPLY_OPT_LEN = 0x01;

// External Method
public:
    // Get the reply sequence number
    //
    // The reply sequence number
    int getReplySeq();

// Constructor / Destructor
public:
    // Default Constructor
    //
    // Constructor for construct the bearer reply option for MO SMS
    BearerReplyOpt();

    // Constructor for construct the bearer reply option by the hexPdu from modem
    BearerReplyOpt(
        const char* hexPdu, // [IN] Bearer reply option part of the hexPdu from modem
        int len             // [IN] The length of the hexPdu
    );

    // Destructor
    virtual ~BearerReplyOpt() {
    }
};

/*****************************************************************************
 * CauseCode
 *****************************************************************************/
class CauseCode: public RmcTlvNode {
    RFX_DECLARE_CLASS(CauseCode);

// Const Value
public:
    static const int CAUSE_CODE              = 0x07;
    static const int CAUSE_CODE_NO_ERR_LEN   = 0x01;
    static const int CAUSE_CODE_ERR_LEN      = 0x02;
    static const int NO_ERR                  = 0x00;
    static const int TEMPORARY_ERR           = 0x02;
    static const int PERMANENT_ERR           = 0x03;
    static const int RIL_CDMA_SMS_TEMP_ERROR = 2;

// Constructor / Desctructor
public:
    // Default Constructor
    CauseCode() {
    }

    // Constructor for construct Cause Code by the ack message from RILJ
    CauseCode(
        RIL_CDMA_SMS_Ack &ack,   // [IN] the ack message from RILJ
        int replySeqNo           // [IN] the reply sequence number from the MT message
    );

    // Constructor for construct the Cause Code by the hexPdu from modem
    CauseCode(
        const char* hexPdu,    // [IN] the cause code part of hexPdu from modem
        int len                // [IN] the length of the hexPdu
    );

    // Destructor
    virtual ~CauseCode() {
    }
};

/*****************************************************************************
 * BearerData
 *****************************************************************************/
class BearerData : public RmcTlvNode{
    RFX_DECLARE_CLASS(BearerData);

// Const Value
public:
    static const int BEARER_DATA = 0x08;
    static const int MESSAGE_TYPE_DELIVERY_ACK   = 0x04;
    static const int SUBPARAM_MESSAGE_IDENTIFIER               = 0x00;
    static const int SUBPARAM_USER_DATA                        = 0x01;
    static const int SUBPARAM_REPLY_OPTION                     = 0x0A;

// External Method
public:
    // Get the binary data of the bearer data
    //
    // RETURNS: true if get the data successfully, otherwise false
    bool getBearerData(
        bool isCtWap,               // [IN] if WAP push message of CT
        unsigned char *bearerData,  // [OUT] the bearer data
        int *len                    // [OUT] the length of the bearer data
    );

    // Get the message Id
    //
    // RETURNS: the message Id
    int getMessageId() const {
        return m_messageId;
    }

    // Get the message type
    //
    // RETURNS: the message type
    int getMessageType() const {
         return m_messageType;
    }

    // Check if status report
    //
    // RETURNS: true if statue report
    bool isStatusReport() {
        return m_messageType == MESSAGE_TYPE_DELIVERY_ACK;
    }

    // decode the message id
    void decodeMessageId();

    // deocde reply option
    void decodeReplyOpt();

    // Check if need status report
    //
    // RETURNS: true if status report is needed
    bool isNeedStatusReport() {
        return m_needStatusReport;
    }

// Constructor / Desctructor
public:
    // Default constructor
    BearerData() :
            RmcTlvNode(BEARER_DATA, -1),
            m_messageId(-1),
            m_messageType(-1),
            m_needStatusReport(false) {
    }

    // Constructor for construct the Bearer Data from the structure of RILJ
    BearerData(
        unsigned char *data, // [IN] the bearer data
        int len              // [IN] the length of the bearer data
    );

    // Constructor for construct the Bearer Data from the hexPdu from modem
    BearerData(
        const char* hexPdu, // [IN] the bearer data part of hex Pdu from modem
        int len             // [IN] the length of the hexPdu
    );

    // Destructor
    virtual ~BearerData() {
    }

// Implementation
private:
    int specialProcessForCtWapPush(unsigned char* targetArray);
    static int removeZero(unsigned char* bytePdu, unsigned short len);

private:
    int m_messageId;
    int m_messageType;
    int m_needStatusReport;
};


/*****************************************************************************
 * Class RpCdmaSmsConverter
 *****************************************************************************/
/*
 * This class is used to convert the CDMA SMS from structure to PDU or
 * from PDU to structure
 */
class RpCdmaSmsConverter {
// External Method
public:
    /*
     * Convert point to point sms message to hex pdu
     *
     * RETURNS: true if successfully convert, otherwise false
     */
    static bool toHexPdu(
        RIL_CDMA_SMS_Message &message,    // [IN] The normal SMS from RILJ
        String8 &hexPdu,                  // [OUT] The hex pdu for modem
        String8 &address,                 // [OUT] The string of the address
        bool needBearerReplyOpt = false    // [IN] need BearerReplyOpt or not
    );

    /*
     * Convert the ack sms message to hex pdu
     *
     * RETURNS: true if successfully convert, otherwise false
     */
    static bool toHexPdu(
        RIL_CDMA_SMS_Ack* pAck,
        int replySeqNo,
        RIL_CDMA_SMS_Address* pAddress,
        String8 &hexPdu
    );

    /*
     * Convert the hex PDU to Point to Point/Broadcast Message
     *
     * RETURNS: true if successfully convert, otherwise false
     */
    static bool toMessage(
        RIL_CDMA_SMS_Message *pMessage,  // [OUT] The message to RILJ
        String8 &hexPdu,                 // [IN]  The hex pdu from modem
        int *pReplySeqNo                 // [OUT] The reply sequence number for the ack messsage
    );

    /*
     * Check the message if status report message.
     *
     * RETURNS: true if status report message.
     */
    static bool isStatusReport(
        RIL_CDMA_SMS_Message *pMessage,  // [IN] The SMS message
        int *messageId                   // [OUT] the message ID of this message
    );

    /*
     * Check the message if need status report
     */
    static bool isNeedStatusReport(
        RIL_CDMA_SMS_Message *pMessage   // [IN] The SMS message
    );
};


#endif /* __RP_CDMA_SMS_CODEC_H__ */
