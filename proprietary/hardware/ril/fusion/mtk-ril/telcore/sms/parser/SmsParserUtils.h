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

#ifndef __SMS_PARSER_UTILS_H__
#define __SMS_PARSER_UTILS_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string>
using namespace std;

typedef unsigned char BYTE;

typedef enum {
    ENCODING_UNKNOWN = 0,
    ENCODING_7BIT,
    ENCODING_8BIT,
    ENCODING_16BIT,
    ENCODING_KSC5601
} SmsDataCodingScheme;

typedef enum {
    CLASS_UNKNOWN = 0,
    CLASS_0,
    CLASS_1,
    CLASS_2,
    CLASS_3
} MessageClass;

/*****************************************************************************
 * Class SmsMessage
 *****************************************************************************/
class PhoneNumberUtils {
public:
    static string prependPlusToNumber(string number);
    static string numToString(int number);
    static BYTE* hexStringToBytes(string s);
    static string stringFromBytes(BYTE* data, int offset, int byteCount);

private:
    static string internalCalledPartyBCDFragmentToString(BYTE *bytes, int offset, int length);
    static char bcdToChar(BYTE b);
    static int hexCharToInt(char c);

private:
    static const int TOA_INTERNATIONAL;
    static const char REPLACEMENT_CHAR;
    static const char PAUSE;
    static const char WAIT;
    static const char WILD;
};

class GsmAlphabet  {
public:
    static string gsm7BitPackedToString(BYTE* pdu, int offset, int lengthSeptets);
    static string gsm7BitPackedToString(BYTE* pdu, int offset, int lengthSeptets,
            int numPaddingBits, int languageTable, int shiftTable);
    static string gsm8BitUnpackedToString(BYTE* data, int offset, int length);
public:
    static const BYTE GSM_EXTENDED_ESCAPE;

private:
    static const string *sLanguageTables;
    static const string *sLanguageShiftTables;
};
#endif /* __SMS_PARSER_UTILS_H__ */
