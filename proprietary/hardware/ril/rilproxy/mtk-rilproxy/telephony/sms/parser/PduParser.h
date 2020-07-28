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

#ifndef __PDU_PARSER_H__
#define __PDU_PARSER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <map>
#include "SmsParserUtils.h"

class SmsHeader;
class GsmSmsAddress;
class SmsMessage;
/*****************************************************************************
 * Class PduParser
 *****************************************************************************/

class PduParser {
public:
    PduParser(BYTE *pdu, int length);
    virtual ~PduParser();
    string getSCAddress();
    int getByte();
    GsmSmsAddress *getAddress();
    int constructUserData(bool hasUserDataHeader, bool dataInSeptets);
    int getUserDataLength();
    BYTE* getUserData();
    SmsHeader* getUserDataHeader();
    long getSCTimestampMillis();

private:
    BYTE *mPdu;
    int mCur;
    int mUserDataSeptetPadding;
    SmsHeader *mUserDataHeader;
    BYTE *mUserData;
    int mUserDataLength;
    int mPduLength;
};

class WappushPduParser {
public:
    WappushPduParser(BYTE* pdu, int length);
    long getValue32();
    int getDecodedDataLength();
    bool decodeContentType(int startIndex);
    bool decodeConstrainedEncoding(int startIndex);
    bool decodeIntegerValue(int startIndex);
    bool decodeShortInteger(int startIndex);
    bool decodeLongInteger(int startIndex);
    bool decodeTokenText(int startIndex);
    void decodeHeaders(int startIndex, int headerLength);
    bool decodeHeaderFieldName(int startIndex);
    bool decodeTextString(int startIndex);
    void expandWellKnownHeadersName();
    string getValueString();
    bool seekXWapApplicationId(int startIndex, int endIndex);
    bool decodeXWapApplicationId(int startIndex);
    bool decodeUintvarInteger(int startIndex);
    bool decodeValueLength(int startIndex);
    bool decodeExtensionMedia(int startIndex);
    bool decodeHeaderFieldValues(int startIndex);
    static void initWapParaMap();

public:
    static const string CONTENT_TYPE_B_PUSH_CO;
    static const int PARAMETER_ID_X_WAP_APPLICATION_ID;
    static const int PDU_TYPE_PUSH;
    static const int PDU_TYPE_CONFIRMED_PUSH;

private:
    void expandWellKnownMimeType();
    bool readContentParameters(int startIndex, int leftToRead, int accumulator);
    bool decodeNoValue(int startIndex);
    string numToHexString(int number);
    string numToString(int number);

private:
    BYTE* mWspData;
    int mDataLength;
    int mPduLength;
    long mUnsigned32bit;
    map<string, string> mContentParameters;
    map<string, string> mHeaders;
    string mStringValue;
    static const int WAP_PDU_LENGTH_QUOTE;
    static map<int, string> WELL_KNOWN_MIME_TYPES;
    static map<int, string> WELL_KNOWN_PARAMETERS;
    static map<int, string> WELL_KNOWN_HEADERS;
    static const int WAP_PDU_SHORT_LENGTH_MAX;
    static const int Q_VALUE;
};
#endif /* __PDU_PARSER_H__ */
