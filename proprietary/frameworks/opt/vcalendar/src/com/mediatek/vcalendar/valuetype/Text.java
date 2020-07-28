/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.vcalendar.valuetype;

import com.mediatek.vcalendar.parameter.Encoding;
import com.mediatek.vcalendar.utils.LogUtil;

/**
 * supply the method to code or decode the text value of properties.
 *
 */

public final class Text {
    private static final String TAG = "Text";

    private Text() {
    }

    /**
     * Decode the encoded string
     *
     * @param srcString
     *            the encoded string
     * @param charset
     *            the char set
     * @param encoding
     *            the encoding method
     * @return the decoded string
     */
    public static String decode(String srcString, String charset,
            String encoding) {
        String text = srcString;

        if (encoding.equals(Encoding.QUOTED_PRINTABLE)) {
            text = Charset.decodeQuotedPrintable(srcString, charset);
        }
        if (encoding.equals(Encoding.BASE64)) {
            LogUtil.e(TAG, "decode: we do not support the BASE64 text decode");
        }

        return text;
    }

    /**
     * Decode the encoded string with default char set UTF8
     *
     * @param srcString
     *            the encoded string
     * @param encoding
     *            the encoding method
     * @return the decoded string
     */
    public static String decode(String srcString, String encoding) {
        return Text.decode(srcString, Charset.UTF8, encoding);
    }

    /**
     * Encoding the string with the charSet
     *
     * @param src
     *            the source string
     * @param charset
     *            char set
     * @param encoding
     *            the encoded method, e.g.Quoted-Printable .
     * @return the encoded string
     */
    public static String encoding(String src, String charset, String encoding) {
        String text = src;

        if (encoding.equals(Encoding.QUOTED_PRINTABLE)) {
            text = Charset.encodeQuotedPrintable(src, charset);
        }
        if (encoding.equals(Encoding.BASE64)) {
            LogUtil.e(TAG, "encoding(): not support the BASE64 text decode");
        }

        return text;
    }

    /**
     * Encoding the string with the default char set UTF8
     *
     * @param src
     *            the source string
     * @param encoding
     *            the encoded method, e.g.Quoted-Printable .
     * @return the encoded string
     */
    public static String encoding(String src, String encoding) {
        return encoding(src, Charset.UTF8, encoding);
    }
}
