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
#include "GsmSmsAddress.h"
#include "SmsAddress.h"
#include "RfxLog.h"

#define RFX_LOG_TAG   "GsmSmsAddress"
/*****************************************************************************
 * Class GsmSmsAddress
 *****************************************************************************/
const int GsmSmsAddress::OFFSET_ADDRESS_LENGTH = 0;
const int GsmSmsAddress::OFFSET_TOA = 1;
const int GsmSmsAddress::OFFSET_ADDRESS_VALUE = 2;

GsmSmsAddress::GsmSmsAddress(BYTE *data, int length) {
    mOrigBytes = new BYTE[length];
    memcpy(mOrigBytes, data, length);

    // addressLength is the count of semi-octets, not bytes
    int addressLength = mOrigBytes[OFFSET_ADDRESS_LENGTH] & 0xff;
    int toa = mOrigBytes[OFFSET_TOA] & 0xff;
    mTon = 0x7 & (toa >> 4);
    // TOA must have its high bit set
    if ((toa & 0x80) != 0x80) {
        RFX_LOG_D(RFX_LOG_TAG, "TOA must have its high bit set");
    }
    if (isAlphanumeric()) {
        // An alphanumeric address
        int countSeptets = addressLength * 4 / 7;
    } else {
        // TS 23.040 9.1.2.5 says
        // that "the MS shall interpret reserved values as 'Unknown'
        // but shall store them exactly as received"
        BYTE lastByte = mOrigBytes[length - 1];

        if ((addressLength & 1) == 1) {
            // Make sure the final unused BCD digit is 0xf
            mOrigBytes[length - 1] |= 0xf0;
        }

        // And restore origBytes
        mOrigBytes[length - 1] = lastByte;
    }
}


GsmSmsAddress::~GsmSmsAddress() {
    if (mOrigBytes != NULL) {
        delete mOrigBytes;
        mOrigBytes = NULL;
    }
}
