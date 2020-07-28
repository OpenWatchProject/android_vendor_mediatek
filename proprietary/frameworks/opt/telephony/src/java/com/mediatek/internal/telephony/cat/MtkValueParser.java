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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.cat.Duration.TimeUnit;
import com.android.internal.telephony.uicc.IccUtils;

import com.android.internal.telephony.cat.ResultException;
import com.android.internal.telephony.cat.ResultCode;
import com.android.internal.telephony.cat.ComprehensionTlv;
import com.android.internal.telephony.cat.Item;
import com.android.internal.telephony.cat.CatService;

import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

abstract class MtkValueParser {
    /**
     * Retrieves Item information from the COMPREHENSION-TLV object.
     *
     * @param ctlv A Text Attribute COMPREHENSION-TLV object
     * @return An Item
     * @throws ResultException
     */
    static Item retrieveItem(ComprehensionTlv ctlv) throws ResultException {
        Item item = null;

        byte[] rawValue = ctlv.getRawValue();
        int valueIndex = ctlv.getValueIndex();
        int length = ctlv.getLength();

        if (length != 0) {
            int textLen = length - 1;

            try {
                int id = rawValue[valueIndex] & 0xff;
                textLen = removeInvalidCharInItemTextString(rawValue, valueIndex, textLen);
                String text = IccUtils.adnStringFieldToString(rawValue, valueIndex + 1, textLen);
                item = new Item(id, text);
            } catch (IndexOutOfBoundsException e) {
                // throw new
                // ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
                MtkCatLog.d("ValueParser", "retrieveItem fail");
            }
        }

        return item;
    }

    static int removeInvalidCharInItemTextString(byte[] rawValue, int valueIndex, int textLen) {
        Boolean isucs2 = false;
        int len = textLen;
        // CatLog.d("ValueParser",
        // "Try to remove invalid raw data 0xf0, valueIndex: "
        // + valueIndex + ", textLen: " + textLen);
        if (textLen >= 1 && rawValue[valueIndex + 1] == (byte) 0x80 || textLen >= 3
                && rawValue[valueIndex + 1] == (byte) 0x81 || textLen >= 4
                && rawValue[valueIndex + 1] == (byte) 0x82) {
            /* The text string format is UCS2 */
            isucs2 = true;
        }
        // CatLog.d("ValueParser", "Is the text string format UCS2? " + isucs2);
        if (!isucs2 && textLen > 0) {
            /* Remove invalid char only when it is not UCS2 format */
            for (int i = textLen; i > 0; i -= 1) {
                if (rawValue[valueIndex + i] == (byte) 0xF0) {
                    // CatLog.d("ValueParser", "find invalid raw data 0xf0");
                    len -= 1;
                } else {
                    break;
                }
            }
        }
        // CatLog.d("ValueParser", "new textLen: " + len);
        return len;
    }

    /**
     * Retrieves alpha identifier from an Alpha Identifier COMPREHENSION-TLV
     * object.
     *
     * @param ctlv An Alpha Identifier COMPREHENSION-TLV object
     * @return String corresponding to the alpha identifier
     * @throws ResultException
     */
    static String retrieveAlphaId(ComprehensionTlv ctlv) throws ResultException {

        if (ctlv != null) {
            byte[] rawValue = ctlv.getRawValue();
            int valueIndex = ctlv.getValueIndex();
            int length = ctlv.getLength();
            if (length != 0) {
                try {
                    return IccUtils.adnStringFieldToString(rawValue, valueIndex, length);
                } catch (IndexOutOfBoundsException e) {
                    throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
                }
            } else {
                MtkCatLog.d("ValueParser", "Alpha Id length=" + length);
                return "";
            }
        } else {
            /*
             * Per 3GPP specification 102.223, if the alpha identifier is not
             * provided by the UICC, the terminal MAY give information to the
             * user noAlphaUsrCnf defines if you need to show user confirmation
             * or not
             */
            boolean noAlphaUsrCnf = false;
            Resources resource = Resources.getSystem();
            try {
                noAlphaUsrCnf = resource
                        .getBoolean(com.android.internal.R.bool.config_stkNoAlphaUsrCnf);
            } catch (NotFoundException e) {
                noAlphaUsrCnf = false;
            }
            return (noAlphaUsrCnf ? null : CatService.STK_DEFAULT);
        }
    }

    static byte[] retrieveNextActionIndicator(ComprehensionTlv ctlv) throws ResultException {
        byte[] nai;
        byte[] rawValue = ctlv.getRawValue();
        int valueIndex = ctlv.getValueIndex();
        int length = ctlv.getLength();

        nai = new byte[length];
        try {
            for (int index = 0; index < length;) {
                nai[index++] = rawValue[valueIndex++];
            }
        } catch (IndexOutOfBoundsException e) {
            throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        }
        return nai;
    }

    static int retrieveTarget(ComprehensionTlv ctlv) throws ResultException {
        byte[] rawValue = ctlv.getRawValue();
        int valueIndex = ctlv.getValueIndex();
        int target = 0;

        try {
            target = rawValue[valueIndex] & 0xff;
        } catch (IndexOutOfBoundsException e) {
            throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        }
        return target;
    }
}
