/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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

/****************************************************************
 * Licensed to the Apache Software Foundation (ASF) under one   *
 * or more contributor license agreements.  See the NOTICE file *
 * distributed with this work for additional information        *
 * regarding copyright ownership.  The ASF licenses this file   *
 * to you under the Apache License, Version 2.0 (the            *
 * "License"); you may not use this file except in compliance   *
 * with the License.  You may obtain a copy of the License at   *
 *                                                              *
 *   http://www.apache.org/licenses/LICENSE-2.0                 *
 *                                                              *
 * Unless required by applicable law or agreed to in writing,   *
 * software distributed under the License is distributed on an  *
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY       *
 * KIND, either express or implied.  See the License for the    *
 * specific language governing permissions and limitations      *
 * under the License.                                           *
 ****************************************************************/

package org.apache.james.mime4j.decoder;

//BEGIN android-changed: Stubbing out logging
import org.apache.james.mime4j.Log;
import org.apache.james.mime4j.LogFactory;
//END android-changed
import org.apache.james.mime4j.util.CharsetUtil;

import com.android.emailcommon.utility.CharsetUtilities;
import com.android.mail.utils.LogUtils;
//import com.ibm.icu.text.CharsetDetector;
//import com.ibm.icu.text.CharsetMatch;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Static methods for decoding strings, byte arrays and encoded words.
 *
 *
 * @version $Id: DecoderUtil.java,v 1.3 2005/02/07 15:33:59 ntherning Exp $
 */
public class DecoderUtil {
    private static Log log = LogFactory.getLog(DecoderUtil.class);
    private static final String DECODED_REGEX = "(=\\?)([A-Za-z0-9_-]*)\\?(?i)[b,q]\\?([^?])+(\\?=)";
    private static final String BASE64_SEP = "=";

    /**
     * Decodes a string containing quoted-printable encoded data.
     *
     * @param s the string to decode.
     * @return the decoded bytes.
     */
    public static byte[] decodeBaseQuotedPrintable(String s) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try {
            byte[] bytes = s.getBytes("US-ASCII");
            QuotedPrintableInputStream is = new QuotedPrintableInputStream(
                                               new ByteArrayInputStream(bytes));
            int b = 0;
            while ((b = is.read()) != -1) {
                baos.write(b);
            }
        } catch (IOException e) {
            /*
             * This should never happen!
             */
            log.error(e);
        }
        return baos.toByteArray();
    }

    /**
     * Decodes a string containing base64 encoded data.
     *
     * @param s the string to decode.
     * @return the decoded bytes.
     */
    public static byte[] decodeBase64(String s) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try {
            byte[] bytes = s.getBytes("US-ASCII");
            Base64InputStream is = new Base64InputStream(
                                        new ByteArrayInputStream(bytes));
            int b = 0;
            while ((b = is.read()) != -1) {
                baos.write(b);
            }
        } catch (IOException e) {
            /*
             * This should never happen!
             */
            log.error(e);
        }
        return baos.toByteArray();
    }

    /**
     * Decodes an encoded word encoded with the 'B' encoding (described in
     * RFC 2047) found in a header field body.
     *
     * @param encodedWord the encoded word to decode.
     * @param charset the Java charset to use.
     * @return the decoded string.
     * @throws UnsupportedEncodingException if the given Java charset isn't
     *         supported.
     */
    public static String decodeB(String encodedWord, String charset)
            throws UnsupportedEncodingException {

        return new String(decodeBase64(encodedWord), charset);
    }

    /**
     * Decodes an encoded word encoded with the 'Q' encoding (described in
     * RFC 2047) found in a header field body.
     *
     * @param encodedWord the encoded word to decode.
     * @param charset the Java charset to use.
     * @return the decoded string.
     * @throws UnsupportedEncodingException if the given Java charset isn't
     *         supported.
     */
    public static String decodeQ(String encodedWord, String charset)
            throws UnsupportedEncodingException {
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
        return new String(decodeBaseQuotedPrintable(sb.toString()), charset);
    }

    public static String decodeEncodedWords(String body) {
        return decodeEncodedWordsProcess(body);
    }

    public static boolean isAllAscii(InputStream in) {
        int result = 0;
        try {
            while ((result = in.read()) != -1) {
                if ((0x0080 & result) != 0) {
                    return false;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return true;
    }

//    public static String emailCharsetDetect(InputStream in) {
//        String properCharset = null;
//        // use ICU lib to detect charset.
//        CharsetDetector cd = new CharsetDetector();
//        try {
//            cd.setText(in);
//        } catch (IOException e) {
//            e.printStackTrace();
//        }
//         CharsetMatch[] cm = cd.detectAll();
//         if (cm != null && cm.length > 0) {
//             // print chraset information.
//             for(CharsetMatch match:cm){
//                 LogUtils.d(LogUtils.TAG, "Charset Detect Result: " + match.getName());
//             }
//             // If the most possible charset was "UTF-8", we do nothing.
//             if(cm[0].getName().equals("UTF-8")) {
//                 return cm[0].getName();
//             }
//
//            properCharset = cm[0].getName();
//            return properCharset;
//         }else {
//             return null;
//         }
//    }

    /**
     * Decodes a string containing encoded words as defined by RFC 2047.
     * Encoded words in have the form
     * =?charset?enc?Encoded word?= where enc is either 'Q' or 'q' for
     * quoted-printable and 'B' or 'b' for Base64.
     *
     * Mediatek: Implement new version
     * 1. Match all substrings by regular-expression
     * 2. Decode each substrings or combine them first
     * 3. This version could resolved most of the situations but still few kinds of missing which I don not know
     * @param body the string to decode.
     * @return the decoded string.
     */
    public static String decodeEncodedWordsProcess(String body) {

        // ANDROID:  Most strings will not include "=?" so a quick test can prevent unneeded
        // object creation.  This could also be handled via lazy creation of the StringBuilder.
        if (body.indexOf("=?") == -1) {
            String resultString = body;
            String properCharset = null;
            byte[] bytesOfUnkown = bytesFromUnknownString(body);
            boolean isAscii = isAllAscii(new ByteArrayInputStream(bytesOfUnkown));
            if (isAscii) {
                return resultString;
            }
            // detect charset.
            properCharset = CharsetUtilities.emailCharsetDetect(new ByteArrayInputStream(bytesOfUnkown));
            LogUtils.d(LogUtils.TAG, "decodeEncodedWordsProcess: Charset Detect Result : properCharset = "
                    + properCharset);
            if (properCharset != null) {
                try {
                    resultString = new String(bytesOfUnkown, properCharset);
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                }
            }
            return resultString;
        }
        body = body.replace("\t", " ");

        String decodeReg = DECODED_REGEX;

        Pattern p = Pattern.compile(decodeReg);
        Matcher m = p.matcher(body);
        StringBuilder encodedText = new StringBuilder();
        while (m.find()) {
            encodedText.append(m.group().replaceAll("\\s*", ""));
            encodedText.append(" ");
        }
        /**
         * M: Enhance the body's disposed process in order to avoid losing
         * undecoded part which is in the body's end. Body is mixed with decode
         * part and undecoded part,and contains "".
         */
        String[] unencodeParts = body.split(decodeReg);
        int unencodeLen = unencodeParts.length;
        String firstUnencodePart = unencodeLen > 0 ? unencodeParts[0] : "";
        String lastUnencodePart = "";
        if (unencodeLen > 1 && body.endsWith(unencodeParts[unencodeLen - 1])) {
            lastUnencodePart = unencodeParts[unencodeLen - 1];
        }
        return firstUnencodePart
                + decodeEncodedWord(encodedText.toString(), 0, encodedText.length())
                + lastUnencodePart;
        /** @} */
    }

    private static String decodeEncodedWord(String body, int begin, int end) {
        // split the content with space then join the sub-parts together into one string.
        // then decode it.
        String [] ms = body.split(" ");
        StringBuilder sb = new StringBuilder();
        StringBuilder sbQP = new StringBuilder();
        StringBuilder sbBase64 = new StringBuilder();
        String charset = null;
        String encoding = null;
        int textPiece = 0;
        for (String s : ms) {
            textPiece++;
            int b = 0;
            int e = s.length();
            int qm1 = s.indexOf('?', b + 2);
            // "add qm1==-1, and return body" if string is not include, it will not parse, and return original string.
            if (qm1 == e - 2 || qm1 == -1)
                return body;

            int qm2 = s.indexOf('?', qm1 + 1);
            if (qm2 == e - 2)
                return body;

            String mimeCharset = s.substring(b + 2, qm1);
            encoding = s.substring(qm1 + 1, qm2);
            String encodedText = s.substring(qm2 + 1, e - 2);
            //sb.append(encodedText);

            charset = CharsetUtil.toJavaCharset(mimeCharset);
            if (charset == null) {
                if (log.isWarnEnabled()) {
                    log.warn("MIME charset '" + mimeCharset + "' in encoded word '"
                            + s.substring(b, e) + "' doesn't have a "
                            + "corresponding Java charset");
                }
                return null;
            } else if (charset.equalsIgnoreCase("GB18030")) {
                log.warn("Current JDK doesn't support decoding of charset '" + charset + "',use GBK!");
                charset = "GBK";

            } else if (!CharsetUtil.isDecodingSupported(charset)) {
                if (log.isWarnEnabled()) {
                    log.warn("Current JDK doesn't support decoding of charset '"
                            + charset + "' (MIME charset '" + mimeCharset
                            + "' in encoded word '" + body.substring(b, e)
                            + "')");
                }
                return null;
            }
        //}
            //String encodedText = sb.toString();

            if (encodedText.length() == 0) {
                if (log.isWarnEnabled()) {
                    log.warn("Warning: Unknown encoding in encoded word '"
                            + body.substring(begin, end) + "'");
                }
                return null;
            }

            try {
                if ("Q".equalsIgnoreCase(encoding)) {
                    /** M: If Base64 decode is pending, decode that first, and clear the sbBase64
                     * buffer.
                     * Use "sbQP" but not "sb" to avoid 2 problem:
                     * 1. duplicate decoding.
                     * 2. leave out some QP string which should be decoded. This can happen when
                     * the first line is "Q", but the others are "B", so the first line will be
                     * forgotten, now we decode sbQP first when decode Base64. @{ */
                    if (sbBase64.length() > 0) {
                        sb.append(DecoderUtil.decodeB(sbBase64.toString(), charset));
                        sbBase64 = new StringBuilder();
                    }
                    sbQP.append(encodedText);
                    if (textPiece >= ms.length) {
                        sb.append(DecoderUtil.decodeQ(sbQP.toString(), charset));
                    }
                    /** @} */
                    //return DecoderUtil.decodeQ(encodedText, charset);
                } else if ("B".equalsIgnoreCase(encoding)) {
                    /** M: Base64 decoding may have 2 scenarios:
                     * 1. Each encoded line comes from complete words
                     * 2. The encoded line is not complete by its own
                     * The default implementation is decoding by lines which only match the case 1.
                     * So decoding by multiple lines just in case.
                     *
                     * Other, if QP decode is pending, decode that first, and clear the sbQP
                     * buffer. @{ */
                    if (sbQP.length() > 0) {
                        sb.append(DecoderUtil.decodeQ(sbQP.toString(), charset));
                        sbQP = new StringBuilder();
                    }
                    sbBase64.append(encodedText);
                    if (encodedText.endsWith(BASE64_SEP)
                            && textPiece < ms.length) {
                        sb.append(DecoderUtil.decodeB(sbBase64.toString(), charset));
                        sbBase64 = new StringBuilder();
                    }
                    if (textPiece >= ms.length) {
                        sb.append(DecoderUtil.decodeB(sbBase64.toString(), charset));
                    }
                    /** @} */
                } else {
                    if (log.isWarnEnabled()) {
                        log.warn("Warning: Unknown encoding in encoded word '"
                                + body.substring(begin, end) + "'");
                    }
                    return null;
                }
            } catch (UnsupportedEncodingException ex) {
                // should not happen because of isDecodingSupported check above
                if (log.isWarnEnabled()) {
                    log.warn("Unsupported encoding in encoded word '"
                            + body.substring(begin, end) + "'", ex);
                }
                return null;
            } catch (RuntimeException ex) {
                if (log.isWarnEnabled()) {
                    log.warn("Could not decode encoded word '"
                            + body.substring(begin, end) + "'", ex);
                }
                return null;
            }
        }
        return sb.toString();
    }

    public static byte[] bytesFromUnknownString(String in) {
        byte[] b = new byte[in.length()];
            for (int i = 0; i < in.length(); i++) {
                b[i] = (byte) in.charAt(i);
            }
        return b;
    }
}
