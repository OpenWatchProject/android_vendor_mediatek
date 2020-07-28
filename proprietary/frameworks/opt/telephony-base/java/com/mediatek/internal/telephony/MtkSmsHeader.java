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


package com.mediatek.internal.telephony;

import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.SmsHeader;
import com.android.internal.util.HexDump;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

import java.util.ArrayList;

/**
 * SMS user data header, as specified in TS 23.040 9.2.3.24.
 */
public class MtkSmsHeader extends SmsHeader {
    // MTK-START
    private static final String TAG = "SmsHeader";
    // MTK-END

    // MTK-START
    public static final int CONCATENATED_8_BIT_REFERENCE_LENGTH = 5;
    public static final int NATIONAL_LANGUAGE_SINGLE_SHIFT_LENGTH = 3;
    public static final int NATIONAL_LANGUAGE_LOCKING_SHIFT_LENGTH = 3;

    public static class NationalLanguageShift {
        public int singleShiftId = 0;
        public int lockingShiftId = 0;
    }

    public NationalLanguageShift nationalLang;
    // MTK-END

    public MtkSmsHeader() {}

    /**
     * Create structured SmsHeader object from serialized byte array representation.
     * (see TS 23.040 9.2.3.24)
     * @param data is user data header bytes
     * @return SmsHeader object
     */
    public static SmsHeader fromByteArray(byte[] data) {
        ByteArrayInputStream inStream = new ByteArrayInputStream(data);
        MtkSmsHeader smsHeader = new MtkSmsHeader();
        while (inStream.available() > 0) {
            /**
             * NOTE: as defined in the spec, ConcatRef and PortAddr
             * fields should not reoccur, but if they do the last
             * occurrence is to be used.  Also, for ConcatRef
             * elements, if the count is zero, sequence is zero, or
             * sequence is larger than count, the entire element is to
             * be ignored.
             */
            int id = inStream.read();
            int length = inStream.read();
            ConcatRef concatRef;
            PortAddrs portAddrs;
            switch (id) {
            case ELT_ID_CONCATENATED_8_BIT_REFERENCE:
                concatRef = new ConcatRef();
                concatRef.refNumber = inStream.read();
                concatRef.msgCount = inStream.read();
                concatRef.seqNumber = inStream.read();
                concatRef.isEightBits = true;
                if (concatRef.msgCount != 0 && concatRef.seqNumber != 0 &&
                        concatRef.seqNumber <= concatRef.msgCount) {
                    smsHeader.concatRef = concatRef;
                }
                break;
            case ELT_ID_CONCATENATED_16_BIT_REFERENCE:
                concatRef = new ConcatRef();
                concatRef.refNumber = (inStream.read() << 8) | inStream.read();
                concatRef.msgCount = inStream.read();
                concatRef.seqNumber = inStream.read();
                concatRef.isEightBits = false;
                if (concatRef.msgCount != 0 && concatRef.seqNumber != 0 &&
                        concatRef.seqNumber <= concatRef.msgCount) {
                    smsHeader.concatRef = concatRef;
                }
                break;
            case ELT_ID_APPLICATION_PORT_ADDRESSING_8_BIT:
                portAddrs = new PortAddrs();
                portAddrs.destPort = inStream.read();
                portAddrs.origPort = inStream.read();
                portAddrs.areEightBits = true;
                smsHeader.portAddrs = portAddrs;
                break;
            case ELT_ID_APPLICATION_PORT_ADDRESSING_16_BIT:
                portAddrs = new PortAddrs();
                portAddrs.destPort = (inStream.read() << 8) | inStream.read();
                portAddrs.origPort = (inStream.read() << 8) | inStream.read();
                portAddrs.areEightBits = false;
                smsHeader.portAddrs = portAddrs;
                break;
            case ELT_ID_NATIONAL_LANGUAGE_SINGLE_SHIFT:
                smsHeader.languageShiftTable = inStream.read();
                break;
            case ELT_ID_NATIONAL_LANGUAGE_LOCKING_SHIFT:
                smsHeader.languageTable = inStream.read();
                break;
            case ELT_ID_SPECIAL_SMS_MESSAGE_INDICATION:
                SpecialSmsMsg specialSmsMsg = new SpecialSmsMsg();
                specialSmsMsg.msgIndType = inStream.read();
                specialSmsMsg.msgCount = inStream.read();
                smsHeader.specialSmsMsgList.add(specialSmsMsg);
                break;
            default:
                MiscElt miscElt = new MiscElt();
                miscElt.id = id;
                miscElt.data = new byte[length];
                inStream.read(miscElt.data, 0, length);
                smsHeader.miscEltList.add(miscElt);
            }
        }
        return smsHeader;
    }

    /**
     * Create serialized byte array representation from structured SmsHeader object.
     * (see TS 23.040 9.2.3.24)
     * @return Byte array representing the SmsHeader
     */
    public static byte[] toByteArray(SmsHeader smsHeader) {
        if (smsHeader instanceof MtkSmsHeader) {
            MtkSmsHeader smsh = (MtkSmsHeader)smsHeader;
            if ((smsh.portAddrs == null) &&
                (smsh.concatRef == null) &&
                (smsh.specialSmsMsgList.isEmpty()) &&
                // MTK-START
                (smsh.nationalLang == null) &&
                // MTK-END
                (smsh.miscEltList.isEmpty()) &&
                (smsh.languageShiftTable == 0) &&
                (smsh.languageTable == 0)) {
                return null;
            }
        }

        return smsHeader.toByteArray(smsHeader);
    }

    // MTK-START
     /**
     * Create a header for specified destination Port
     * @param destPort, a specified application port
     * @param ret byte[] containing the encoding header with destPort
     */
    public static byte[] getSubmitPduHeader(int destPort) {
        return getSubmitPduHeader(destPort, 0, 0, 0);
    }

    /**
     * Create a header for specified destination Port
     *
     * @param destPort, a specified application port
     * @param originalPort, a specified application original port
     * @param ret byte[] containing the encoding header with destPort
     */
    public static byte[] getSubmitPduHeader(int destPort, int originalPort) {
        return getSubmitPduHeader(destPort, originalPort, 0, 0, 0);
    }

    /**
     * Create a header for concatenated message
     *
     * @param destPort, a specified application port
     * @param ret byte[] containing the encoding header with destPort
     */
    public static byte[] getSubmitPduHeader(
            int refNumber, int seqNumber, int msgCount) {
        return getSubmitPduHeader(-1, refNumber, seqNumber, msgCount);
    }

    /**
     * Create a header for specified destination Port and concatenated message
     *
     * @param destPort, a specified application port
     * @param refNumber paramters for concatenated Message
     * @param seqNumber paramters for concatenated Message
     * @param msgCount paramters for concatenated Message
     * @param ret byte[] containing the encoding header with destPort and
     *            concatenatedMsgTag
     */
    public static byte[] getSubmitPduHeader(int destPort,
            int refNumber, int seqNumber, int msgCount) {
        return getSubmitPduHeaderWithLang(destPort, refNumber, seqNumber, msgCount, -1, -1);
    }

    /**
     * Create a header for specified destination Port and concatenated message
     *
     * @param destPort, a specified application port
     * @param originalPort, a specified application original port
     * @param refNumber paramters for concatenated Message
     * @param seqNumber paramters for concatenated Message
     * @param msgCount paramters for concatenated Message
     * @param ret byte[] containing the encoding header with destPort and
     *            concatenatedMsgTag
     */
    public static byte[] getSubmitPduHeader(int destPort, int originalPort,
            int refNumber, int seqNumber, int msgCount) {
        return getSubmitPduHeaderWithLang(destPort, originalPort, refNumber, seqNumber, msgCount,
                -1, -1);
    }

    /**
     * Create a header with language table
     *
     * @param ret byte[] containing the encoding header with destPort
     */
    public static byte[] getSubmitPduHeaderWithLang(int destPort, int singleShiftId,
            int lockingShiftId) {
        return getSubmitPduHeaderWithLang(destPort, 0, 0, 0, singleShiftId, lockingShiftId);
    }

    /**
     * Create a header for concatenated message with language
     *
     * @param ret byte[] containing the encoding header with destPort
     */
    public static byte[] getSubmitPduHeaderWithLang(
            int refNumber, int seqNumber, int msgCount,
            int singleShiftId, int lockingShiftId) {
        return getSubmitPduHeaderWithLang(
                -1, refNumber, seqNumber, msgCount, singleShiftId, lockingShiftId);
    }

     /**
     * Create a header for specified destination Port,
     * concatenated message, and shift tables
     * @param destPort, a specified application port
     * @param refNumber paramters for concatenated Message
     * @param seqNumber paramters for concatenated Message
     * @param msgCount paramters for concatenated Message
     * @param singleShiftId paramters for singleShift table ID
     * @param lockingShiftId paramters for lockingShift table ID
     * @param ret byte[] containing the encoding header with destPort and
     *            concatenatedMsgTag
     */
    public static byte[] getSubmitPduHeaderWithLang(int destPort,
            int refNumber, int seqNumber, int msgCount,
            int singleShiftId, int lockingShiftId) {
        MtkSmsHeader smsHeader = new MtkSmsHeader();

        if (destPort >= 0) {
            SmsHeader.PortAddrs portAddrs = new SmsHeader.PortAddrs();
            portAddrs.destPort = destPort;
            portAddrs.origPort = 0;
            portAddrs.areEightBits = false;

            smsHeader.portAddrs = portAddrs;
        }

        if (msgCount > 0) {
            SmsHeader.ConcatRef concatRef = new SmsHeader.ConcatRef();
            concatRef.refNumber = refNumber;
            concatRef.seqNumber = seqNumber; // 1-based sequence
            concatRef.msgCount = msgCount;

            // TODO: We currently set this to true since our messaging app will never
            // send more than 255 parts (it converts the message to MMS well before that).
            // However, we should support 3rd party messaging apps that might need 16-bit
            // references
            // Note:  It's not sufficient to just flip this bit to true; it will have
            // ripple effects (several calculations assume 8-bit ref).
            concatRef.isEightBits = true;

            smsHeader.concatRef = concatRef;
        }

        if (singleShiftId > 0 || lockingShiftId > 0) {
            smsHeader.nationalLang = new MtkSmsHeader.NationalLanguageShift();
            smsHeader.nationalLang.singleShiftId = singleShiftId;
            smsHeader.nationalLang.lockingShiftId = lockingShiftId;
        }

        return SmsHeader.toByteArray(smsHeader);
    }

    /**
     * Create a header for specified destination Port,
     * concatenated message, and shift tables
     * @param destPort, a specified application port
     * @param originalPort, a specified application original port
     * @param refNumber paramters for concatenated Message
     * @param seqNumber paramters for concatenated Message
     * @param msgCount paramters for concatenated Message
     * @param singleShiftId paramters for singleShift table ID
     * @param lockingShiftId paramters for lockingShift table ID
     * @param ret byte[] containing the encoding header with destPort and
     *            concatenatedMsgTag
     */
    public static byte[] getSubmitPduHeaderWithLang(int destPort, int originalPort,
            int refNumber, int seqNumber, int msgCount,
            int singleShiftId, int lockingShiftId) {
        MtkSmsHeader smsHeader = new MtkSmsHeader();

        if (destPort >= 0) {
            SmsHeader.PortAddrs portAddrs = new SmsHeader.PortAddrs();
            portAddrs.destPort = destPort;
            portAddrs.origPort = originalPort;
            portAddrs.areEightBits = false;

            smsHeader.portAddrs = portAddrs;
        }

        if (msgCount > 0) {
            SmsHeader.ConcatRef concatRef = new SmsHeader.ConcatRef();
            concatRef.refNumber = refNumber;
            concatRef.seqNumber = seqNumber; // 1-based sequence
            concatRef.msgCount = msgCount;

            // TODO: We currently set this to true since our messaging app will never
            // send more than 255 parts (it converts the message to MMS well before that).
            // However, we should support 3rd party messaging apps that might need 16-bit
            // references
            // Note:  It's not sufficient to just flip this bit to true; it will have
            // ripple effects (several calculations assume 8-bit ref).
            concatRef.isEightBits = true;

            smsHeader.concatRef = concatRef;
        }

        if (singleShiftId > 0 || lockingShiftId > 0) {
            smsHeader.nationalLang = new MtkSmsHeader.NationalLanguageShift();
            smsHeader.nationalLang.singleShiftId = singleShiftId;
            smsHeader.nationalLang.lockingShiftId = lockingShiftId;
        }

        return SmsHeader.toByteArray(smsHeader);
    }
    // MTK-END
}
