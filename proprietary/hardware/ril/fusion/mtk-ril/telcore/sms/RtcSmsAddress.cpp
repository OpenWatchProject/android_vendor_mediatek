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
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RtcSmsAddress.h"

/*****************************************************************************
 * RtcSmsAddress
 *****************************************************************************/
RtcSmsAddress::RtcSmsAddress()
        :mTon(TON_UNKNOWN), mError(false) {
}

/*****************************************************************************
 * RtcGsmSmsAddress
 *****************************************************************************/
RtcGsmSmsAddress::RtcGsmSmsAddress(unsigned char* data, int offset, int length) {
    if ((data != NULL) && (length <= MAX_BUFFER_SIZE)) {
        memcpy(mOrigBytes, data + offset, length);
        int addressLength = mOrigBytes[OFFSET_ADDRESS_LENGTH];
        int toa = mOrigBytes[OFFSET_TOA] & 0xff;
        setTon(0x7 & (toa >> 4));
        if ((toa & 0x80) != 0x80) {
            setError();
            return;
        }
        if (isAlphanumeric()) {
            // Not support
            setError();
            return;
        } else {
            char lastByte = mOrigBytes[length - 1];

            if ((addressLength & 1) == 1) {
                // Make sure the final unused BCD digit is 0xf
                mOrigBytes[length - 1] |= 0xf0;
            }
            bool ret = calledPartyBCDFragmentToString(
                    mOrigBytes,
                    OFFSET_ADDRESS_VALUE,
                    length - OFFSET_ADDRESS_VALUE,
                    getAddr());
            if (!ret) {
                setError();
            }
            // And restore origBytes
            mOrigBytes[length - 1] = lastByte;
        }
    }else {
       setError();
    }
}

bool RtcGsmSmsAddress::bcd2Char(unsigned char bcd, String8 &addr) {
    if (bcd < 0xa) {
        addr.appendFormat("%d", bcd);
        return true;
    } else {
        return false;
    }
}

bool RtcGsmSmsAddress::calledPartyBCDFragmentToString(
        unsigned char *bcd, int offset, int length, String8 &addr) {
    for (int i = offset ; i < length + offset ; i++) {
        if (!bcd2Char((bcd[i] & 0xf), addr)) {
            return false;
        }
        unsigned char b = (bcd[i] >> 4) & 0xf;
        if (b == 0xf && i + 1 == length + offset) {
            //ignore final 0xf
            break;
        }
        if (!bcd2Char(b, addr)) {
            return false;
        }
    }
    return true;
}

/*****************************************************************************
 * RtcCdmaSmsAddress
 *****************************************************************************/
RtcCdmaSmsAddress::RtcCdmaSmsAddress(RIL_CDMA_SMS_Address &address) {
    if (address.digit_mode == RIL_CDMA_SMS_DIGIT_MODE_4_BIT) {
        CdmaSmsAddress *pAddr;
        RFX_OBJ_CREATE_EX(pAddr, CdmaSmsAddress, NULL, (address, true));
        RFX_ASSERT(pAddr != NULL);
        pAddr->getAddress(getAddr());
        RFX_OBJ_CLOSE(pAddr);
    } else {
        setError();
    }
}