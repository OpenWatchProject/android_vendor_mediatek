/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 */
package com.mediatek.vcalendar.valuetype;

import android.text.TextUtils;

import com.mediatek.vcalendar.utils.LogUtil;
import com.mediatek.vcalendar.utils.StringUtil;

import org.apache.james.mime4j.decoder.QuotedPrintableInputStream;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;

/**
 * "Character Set".
 */
public final class Charset {
    private static final String TAG = "Charset";

    public static final String UTF8 = "UTF-8";

    public static final String GB18030 = "GB18030";

    static char[] sList = new char[] { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    static StringBuffer sBuf = new StringBuffer();

    private Charset() {
    }

    /**
     * Encode the string with the default char set UTF8
     *
     * @param src
     *            the source string
     * @return the encoded string
     */
    public static String encoding(String src) {
        return encoding(src, UTF8);
    }

    /**
     * Encode the string with the given char set
     *
     * @param src
     *            the source string
     * @param chset
     *            the char set
     * @return the encoded string
     */
    public static String encoding(String src, String chset) {
        if (StringUtil.isNullOrEmpty(src) || StringUtil.isNullOrEmpty(chset)) {
            return null;
        }

        sBuf.setLength(0);

        try {
            byte[] bb = src.getBytes(chset);
            for (int i = 0; i < bb.length; i++) {
                // buf.append(String.format("=%02X", bb[i]));
                sBuf.append('=');
                sBuf.append(sList[(bb[i] >> 4) & 0xF]);
                sBuf.append(sList[bb[i] & 0xF]);
            }
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }

        return new String(sBuf);
    }

    /**
     * Encode string use Quoted-Printable.
     *
     * @param str
     *            the source string
     * @param chset
     *            character set
     * @return encoded string
     */
    public static String encodeQuotedPrintable(final String str, String chset) {
        if (TextUtils.isEmpty(str)) {
            return "";
        }

        final StringBuilder builder = new StringBuilder();
        int index = 0;
        int lineCount = 0;
        byte[] strArray = null;

        try {
            strArray = str.getBytes(chset);
        } catch (UnsupportedEncodingException e) {
            LogUtil.e(TAG, "encodeQuotedPrintable(): Charset " + chset
                    + " cannot be used. " + "Try default charset");
            strArray = str.getBytes();
        }
        while (index < strArray.length) {
            builder.append(String.format("=%02X", strArray[index]));
            index += 1;
            lineCount += 3;

            if (lineCount >= 67) {
                // Specification requires CRLF must be inserted before the
                // length of the line
                // becomes more than 76.
                // Assuming that the next character is a multi-byte character,
                // it will become
                // 6 bytes.
                // 76 - 6 - 3 = 67
                builder.append("=\r\n");
                lineCount = 0;
            }
        }

        return builder.toString();
    }

    /**
     * Decode the string with the default char set UTF8
     *
     * @param src
     *            the encoded string
     * @return the decoded string
     */
    public static String decoding(String src) {
        return decoding(src, UTF8);
    }

    /**
     * Decode the string with the given char set
     *
     * @param src
     *            the encoded string
     * @param chset
     *            the char set
     * @return the decoded string
     */
    public static String decoding(String src, String chset) {
        if (StringUtil.isNullOrEmpty(src) || StringUtil.isNullOrEmpty(chset)) {
            return null;
        }

        if (src.indexOf("=") != 0) {
            return null;
        }

        int len = src.length();
        if (len % 3 != 0) {
            return null;
        }

        String tmp = src.replaceAll("=", "");
        len = tmp.length();
        if (len % 2 != 0) {
            return null;
        }

        byte[] bb = hexStringToByteArray(tmp);

        try {
            return new String(bb, chset);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }

        return null;
    }

    private static byte[] hexStringToByteArray(String hexString) {
        int length = hexString.length();
        byte buffer[] = new byte[length / 2];
        for (int i = 0; i < length; i += 2) {
            buffer[i / 2] = (byte) (toByte(hexString.charAt(i)) << 4
                    | toByte(hexString.charAt(i + 1)));
        }

        return buffer;
    }

    private static int toByte(char c) {
        if (c >= '0' && c <= '9') {
            return c - 48;
        }
        if (c >= 'A' && c <= 'F') {
            return (c - 65) + 10;
        }
        if (c >= 'a' && c <= 'f') {
            return (c - 97) + 10;
        } else {
            throw new RuntimeException((new StringBuilder())
                    .append("Invalid hex char '").append(c).append("'")
                    .toString());
        }
    }

    /**
     * Decode an encoded word encoded with the 'Q' encoding (described in RFC
     * 2047) found in a header field body.
     *
     * @param encodedWord
     *            the encoded word to decode.
     * @param charset
     *            the Java charset to use.
     * @return the decoded string; or null if error
     */
    public static String decodeQuotedPrintable(String encodedWord,
            String charset) {
        /*
         * Replace _ with =20
         */
        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < encodedWord.length(); i++) {
            char c = encodedWord.charAt(i);
            if (c == '_') {
                sb.append("=20");
            } else {
                sb.append(c);
            }
        }
        String decodedStr;
        try {
            decodedStr = new String(decodeBaseQuotedPrintable(sb.toString()),
                    charset);
            return decodedStr;
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * Decodes an encoded word encoded with the 'Q' encoding (described in RFC
     * 2047) found in a header field body.
     *
     * @param s
     *            the encoded string to decode.
     * @return the decoded string; or null if error
     */
    public static byte[] decodeBaseQuotedPrintable(String s) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        try {
            byte[] bytes = s.getBytes("US-ASCII");

            QuotedPrintableInputStream is = new
                    QuotedPrintableInputStream(new ByteArrayInputStream(bytes));

            int b = 0;
            while ((b = is.read()) != -1) {
                baos.write(b);
            }
            is.close();
        } catch (IOException e) {
            /*
             * This should never happen!
             */
            LogUtil.i(TAG, "decodeBaseQuotedPrintable(): Charset--error.");
        }

        return baos.toByteArray();
    }

}
