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
#include "SmsParserUtils.h"

/*****************************************************************************
 * Class RpCdmaSmsCodec
 *****************************************************************************/
const int PhoneNumberUtils::TOA_INTERNATIONAL = 0x91;
const char PhoneNumberUtils::PAUSE = ',';
const char PhoneNumberUtils::WAIT = ';';
const char PhoneNumberUtils::WILD = 'N';
const char PhoneNumberUtils::REPLACEMENT_CHAR = (char) 0xfffd;

string PhoneNumberUtils::numToString(int number) {
    char temp[128] = { 0 };
    sprintf(temp, "%d", number);
    string ret = temp;
    return ret;
}

int PhoneNumberUtils::hexCharToInt(char c) {
    if (c >= '0' && c <= '9') {
        return (c - '0');
    }
    if (c >= 'A' && c <= 'F') {
        return (c - 'A' + 10);
    }
    if (c >= 'a' && c <= 'f') {
        return (c - 'a' + 10);
    }
    return 0;
}

BYTE* PhoneNumberUtils::hexStringToBytes(string s) {
    if (s.empty()) {
        return NULL;
    }

    int sz = s.length();
    BYTE* ret = new BYTE[sz/2];

    for (int i=0 ; i <sz ; i+=2) {
        ret[i/2] = (BYTE) ((hexCharToInt(s.at(i)) << 4)
                | hexCharToInt(s.at(i+1)));
    }
    return ret;
}

string PhoneNumberUtils::stringFromBytes(BYTE* data, int offset, int byteCount) {
    char* value = NULL;;
    int length = 0;

    // We inline UTF-8, ISO-8859-1, and US-ASCII decoders for speed.
    BYTE* d = data;
    char* v = new char[byteCount];
    int idx = offset;
    int last = offset + byteCount;
    int s = 0;
outer:
    while (idx < last) {
        BYTE b0 = d[idx++];
        if ((b0 & 0x80) == 0) {
            // 0xxxxxxx
            // Range:  U-00000000 - U-0000007F
            int val = b0 & 0xff;
            v[s++] = (char) val;
        } else if (((b0 & 0xe0) == 0xc0) || ((b0 & 0xf0) == 0xe0) ||
                ((b0 & 0xf8) == 0xf0) || ((b0 & 0xfc) == 0xf8) || ((b0 & 0xfe) == 0xfc)) {
            int utfCount = 1;
            if ((b0 & 0xf0) == 0xe0) utfCount = 2;
            else if ((b0 & 0xf8) == 0xf0) utfCount = 3;
            else if ((b0 & 0xfc) == 0xf8) utfCount = 4;
            else if ((b0 & 0xfe) == 0xfc) utfCount = 5;

            // 110xxxxx (10xxxxxx)+
            // Range:  U-00000080 - U-000007FF (count == 1)
            // Range:  U-00000800 - U-0000FFFF (count == 2)
            // Range:  U-00010000 - U-001FFFFF (count == 3)
            // Range:  U-00200000 - U-03FFFFFF (count == 4)
            // Range:  U-04000000 - U-7FFFFFFF (count == 5)

            if (idx + utfCount > last) {
                v[s++] = REPLACEMENT_CHAR;
                continue;
            }

            // Extract usable bits from b0
            int val = b0 & (0x1f >> (utfCount - 1));
            for (int i = 0; i < utfCount; ++i) {
                BYTE b = d[idx++];
                if ((b & 0xc0) != 0x80) {
                    v[s++] = REPLACEMENT_CHAR;
                    idx--;  // Put the input char back
                    goto outer;
                }
                // Push new bits in from the right side
                val <<= 6;
                val |= b & 0x3f;
            }
            // Note: Java allows overlong char
            // specifications To disallow, check that val
            // is greater than or equal to the minimum
            // value for each count:
            //
            // count    min value
            // -----   ----------
            //   1           0x80
            //   2          0x800
            //   3        0x10000
            //   4       0x200000
            //   5      0x4000000

            // Allow surrogate values (0xD800 - 0xDFFF) to
            // be specified using 3-byte UTF values only
            if ((utfCount != 2) && (val >= 0xD800) && (val <= 0xDFFF)) {
                v[s++] = REPLACEMENT_CHAR;
                continue;
            }

            // Reject chars greater than the Unicode maximum of U+10FFFF.
            if (val > 0x10FFFF) {
                v[s++] = REPLACEMENT_CHAR;
                continue;
            }

            // Encode chars from U+10000 up as surrogate pairs
            if (val < 0x10000) {
                v[s++] = (char) val;
            } else {
                int x = val & 0xffff;
                int u = (val >> 16) & 0x1f;
                int w = (u - 1) & 0xffff;
                int hi = 0xd800 | (w << 6) | (x >> 10);
                int lo = 0xdc00 | (x & 0x3ff);
                v[s++] = (char) hi;
                v[s++] = (char) lo;
            }
        } else {
            // Illegal values 0x8*, 0x9*, 0xa*, 0xb*, 0xfd-0xff
            v[s++] = REPLACEMENT_CHAR;
        }
    }

    if (s == byteCount) {
        // We guessed right, so we can use our temporary array as-is.
        value = v;
        length = s;
    } else {
        // Our temporary array was too big, so reallocate and copy.
        value = new char[s];
        length = s;
        memcpy(value, v, s);
        delete[] v;
    }
    string ret(value, length);
    delete[] value;
    return ret;
}
