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
#include "SmsHeader.h"
#include "RfxLog.h"

#define RFX_LOG_TAG   "SmsHeader"
/*****************************************************************************
 * Class SmsHeader
 *****************************************************************************/
const int SmsHeader::PORT_WAP_PUSH = 2948;
const int SmsHeader::PORT_SUPL = 7275;

SmsHeader::SmsHeader() :
        mLanguageShiftTable(0),
        mLanguageTable(0) {
    mPortAddrs.destPort =-1;
    mPortAddrs.origPort = -1;
    mPortAddrs.areEightBits = false;
    mConcatRef.refNumber = -1;
    mConcatRef.seqNumber = -1;
    mConcatRef.msgCount = 0;
    mConcatRef.isEightBits = false;
}

SmsHeader::~SmsHeader() {
    for (list<MiscElt>::iterator iter = mMiscEltList.begin(); iter != mMiscEltList.end(); iter++) {
        MiscElt temp = *iter;
        if (temp.data != NULL) {
            delete temp.data;
            temp.data = NULL;
        }
    }
}

SmsHeader* SmsHeader::fromByteArray(BYTE* data, int headerLength) {
    SmsHeader *smsHeader = new SmsHeader();
    int curr = 0;
    while (curr < headerLength) {
        /**
         * NOTE: as defined in the spec, ConcatRef and PortAddr
         * fields should not reoccur, but if they do the last
         * occurrence is to be used.  Also, for ConcatRef
         * elements, if the count is zero, sequence is zero, or
         * sequence is larger than count, the entire element is to
         * be ignored.
         */
        int id = data[curr++];
        int length = data[curr++];
        RFX_LOG_D(RFX_LOG_TAG, "fromByteArray header element length: %d, id: %d", id, length);
        ConcatRef concatRef;
        PortAddrs portAddrs;
        int temp1 = 0;
        int temp2 = 0;
        int temp3 = 0;
        int temp4 = 0;
        switch (id) {
            case ELT_ID_CONCATENATED_8_BIT_REFERENCE: {
                concatRef.refNumber = data[curr++];
                concatRef.msgCount = data[curr++];
                concatRef.seqNumber = data[curr++];
                concatRef.isEightBits = true;
                if (concatRef.msgCount != 0 && concatRef.seqNumber != 0 &&
                        concatRef.seqNumber <= concatRef.msgCount) {
                    smsHeader->mConcatRef = concatRef;
                }
                RFX_LOG_D(RFX_LOG_TAG, "fromByteArray concatRef.refNumber: %d,"
                        "concatRef.msgCount: %d,"
                        "concatRef.seqNumber: %d",
                        concatRef.refNumber, concatRef.msgCount, concatRef.seqNumber);
                break;
            }
            case ELT_ID_CONCATENATED_16_BIT_REFERENCE: {
                temp1 = data[curr++] << 8;
                temp2 = data[curr++];
                concatRef.refNumber = temp1 | temp2;
                concatRef.msgCount = data[curr++];
                concatRef.seqNumber = data[curr++];
                concatRef.isEightBits = false;
                if (concatRef.msgCount != 0 && concatRef.seqNumber != 0 &&
                        concatRef.seqNumber <= concatRef.msgCount) {
                    smsHeader->mConcatRef = concatRef;
                }
                RFX_LOG_D(RFX_LOG_TAG, "fromByteArray concatRef.refNumber: %d,"
                        "concatRef.msgCount: %d,"
                        "concatRef.seqNumber: %d",
                        concatRef.refNumber, concatRef.msgCount, concatRef.seqNumber);
                break;
            }
            case ELT_ID_APPLICATION_PORT_ADDRESSING_8_BIT: {
                portAddrs.destPort = data[curr++];
                portAddrs.origPort = data[curr++];
                portAddrs.areEightBits = true;
                smsHeader->mPortAddrs = portAddrs;
                RFX_LOG_D(RFX_LOG_TAG, "fromByteArray portAddrs.destPort: %d,"
                        "portAddrs.origPort: %d",
                        portAddrs.destPort, portAddrs.origPort);
                break;
            }
            case ELT_ID_APPLICATION_PORT_ADDRESSING_16_BIT: {
                temp1 = data[curr++] << 8;
                temp2 = data[curr++];
                portAddrs.destPort = temp1 | temp2;
                temp3 = data[curr++] << 8;
                temp4 = data[curr++];
                portAddrs.origPort = temp3 | temp4;
                portAddrs.areEightBits = false;
                smsHeader->mPortAddrs = portAddrs;
                RFX_LOG_D(RFX_LOG_TAG, "fromByteArray portAddrs.destPort: %d,"
                        "portAddrs.origPort: %d",
                        portAddrs.destPort, portAddrs.origPort);
                break;
            }
            case ELT_ID_NATIONAL_LANGUAGE_SINGLE_SHIFT: {
                smsHeader->mLanguageShiftTable = data[curr++];
                RFX_LOG_D(RFX_LOG_TAG, "fromByteArray mLanguageShiftTable: %d", smsHeader->mLanguageShiftTable);
                break;
            }
            case ELT_ID_NATIONAL_LANGUAGE_LOCKING_SHIFT: {
                smsHeader->mLanguageTable = data[curr++];
                RFX_LOG_D(RFX_LOG_TAG, "fromByteArray mLanguageShiftTable: %d", smsHeader->mLanguageTable);
               break;
            }
            case ELT_ID_SPECIAL_SMS_MESSAGE_INDICATION: {
                SpecialSmsMsg specialSmsMsg;;
                specialSmsMsg.msgIndType = data[curr++];
                specialSmsMsg.msgCount = data[curr++];
                smsHeader->mSpecialSmsMsgList.push_back(specialSmsMsg);
                RFX_LOG_D(RFX_LOG_TAG, "fromByteArray specialSmsMsg.msgIndType: %d,"
                        "specialSmsMsg.msgCount: %d",
                        specialSmsMsg.msgIndType, specialSmsMsg.msgCount);
                break;
            }
            default: {
                MiscElt miscElt;
                miscElt.id = id;
                miscElt.data = new BYTE[length];
                memcpy(miscElt.data, data + curr, length);
                data += length;
                smsHeader->mMiscEltList.push_back(miscElt);
                RFX_LOG_D(RFX_LOG_TAG, "fromByteArray miscElt.id: %d", miscElt.id);
                break;
            }
        }
    }
    return smsHeader;
}
