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

import android.telephony.Rlog;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.uicc.IccUtils;

/**
 * Various methods, useful for dealing with SIM data.
 */
public class MtkIccUtils extends IccUtils {
    static final String MTK_LOG_TAG="MtkIccUtils";

    // MTK-START: SIM COMMON
    public static String
       parsePlmnToStringForEfOpl(byte[] data, int offset, int length) {

        StringBuilder ret = new StringBuilder(length * 2);
        int v;

        do {
            v = data[offset] & 0xf;
            if (v >= 0 && v <= 9)
                ret.append((char) ('0' + v));
            else if (v == 13) // wild-carding
                ret.append((char) ('d'));
            else
                break;

            v = (data[offset] >> 4) & 0xf;
            if (v >= 0 && v <= 9)
                ret.append((char) ('0' + v));
            else if (v == 13) // wild-carding
                ret.append((char) ('d'));
            else
                break;

            v = data[offset + 1] & 0xf;
            if (v >= 0 && v <= 9)
                ret.append((char) ('0' + v));
            else if (v == 13) // wild-carding
                ret.append((char) ('d'));
            else
                break;

            v = data[offset + 2] & 0xf;
            if (v >= 0 && v <= 9)
                ret.append((char) ('0' + v));
            else if (v == 13) // wild-carding
                ret.append((char) ('d'));
            else
                break;

            v = (data[offset + 2] >> 4) & 0xf;
            if (v >= 0 && v <= 9)
                ret.append((char) ('0' + v));
            else if (v == 13) // wild-carding
                ret.append((char) ('d'));
            else
                break;

            v = (data[offset + 1] >> 4) & 0xf;
            if (v >= 0 && v <= 9)
                ret.append((char) ('0' + v));
            else if (v == 13) // wild-carding
                ret.append((char) ('d'));
            else
                break;
        }   while(false);

        return ret.toString();
    }

    public static String parseLanguageIndicator(byte[] rawData, int offset, int length) {
        if (null == rawData) {
            return null;
        }

        if (rawData.length < offset + length) {
            Rlog.e(MTK_LOG_TAG, "length is invalid");
            return null;
        }

        return GsmAlphabet.gsm8BitUnpackedToString(rawData, offset, length);
    }

    /*
      * parse plmn according to spec 24008
    */
    public static String
       parsePlmnToString(byte[] data, int offset, int length) {

        StringBuilder ret = new StringBuilder(length * 2);
        int v;

        do {
            v = data[offset] & 0xf;
            if (v > 9)  break;
                ret.append((char) ('0' + v));

            v = (data[offset] >> 4) & 0xf;
            if (v > 9)  break;
                ret.append((char) ('0' + v));

            v = data[offset + 1] & 0xf;
            if (v > 9)  break;
                ret.append((char) ('0' + v));

            v = data[offset + 2] & 0xf;
            if (v > 9)  break;
                ret.append((char) ('0' + v));

            v = (data[offset + 2] >> 4) & 0xf;
            if (v > 9)  break;
                ret.append((char) ('0' + v));

            v = (data[offset + 1] >> 4) & 0xf;
            if (v > 9)  break;
                ret.append((char) ('0' + v));
        }   while(false);

        return ret.toString();
    }
// MTK-END
}
