/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.ims;

import android.telephony.Rlog;
import java.io.IOException;
import java.util.Arrays;
import com.mediatek.ims.ImsServiceCallTracker;

/**
 * For RTT text trnascoding between UTF8 and Unicode.
 * MD will pass UTF8 in URC +ERTTSTR and to show in UI, the text should be transcoding to
 * Unicode. And MD may not always send a completed UTF8, so RttTextEncoder need to store
 * the mRemaining. And wait for next URC to complete the UTF8 and transcoding to Unicode.
 */
public class RttTextEncoder {
    private static final String LOG_TAG = "RttTextEncoder";

    private final byte B_10000000 = 128 - 256;
    private final byte B_11000000 = 192 - 256;
    private final byte B_11100000 = 224 - 256;
    private final byte B_11110000 = 240 - 256;
    private final byte B_00011100 = 28;
    private final byte B_00000011 = 3;
    private final byte B_00000111 = 7;
    private final byte B_00111111 = 63;
    private final byte B_00001111 = 15;
    private final byte B_00111100 = 60;
    private final byte B_00110000 = 48;

    private String printBytes(byte[] bytes) {
        String ret = "";
        for (byte b: bytes) {
            ret += Integer.toString((b & 0xff) , 16) + " ";
        }
        return ret;
    }

    private byte[] mRemaining = new byte[]{};

    /** Convert from UTF8 bytes to UNICODE character */
    private char[] toUCS2(byte[] utf8Bytes) {
        CharList charList = new CharList();
        byte b2 = 0, b3 = 0, b4 = 0;
        int ub1 = 0, ub2 = 0, ub3 = 0;

        utf8Bytes = appendByteArray(mRemaining, utf8Bytes);
        clearRemaining();

        for (int i = 0; i < utf8Bytes.length; i++) {
            try{
                byte b = utf8Bytes[i];
                if (isNotHead(b)) {
                    // start with 10xxxxxx, skip it.
                    continue;
                } else if (b > 0) {
                    // 1 byte, ASCII
                    charList.add((char) b);
                } else if ((b & B_11110000) == B_11110000) {
                    // UCS-4 is used for emoji icons
                    if (checkIsRemaining(i+1, utf8Bytes)) break;
                    b2 = utf8Bytes[i+1];
                    if (!isNotHead(b2)) continue;
                    i++;

                    if (checkIsRemaining(i+1, utf8Bytes)) break;
                    b3 = utf8Bytes[i+1];
                    if (!isNotHead(b3)) continue;
                    i++;

                    if (checkIsRemaining(i+1, utf8Bytes)) break;
                    b4 = utf8Bytes[i+1];
                    if (!isNotHead(b4)) continue;
                    i++;

                    ub1 = ((b & B_00000111) << 2) + ((b2 & B_00110000) >> 4);
                    ub2 = ((b2 & B_00001111) << 4) + ((b3 & B_00111100) >> 2);
                    ub3 = ((b3 & B_00000011) << 6) + ((b4 & B_00111111));
                    charList.add(makeChar(ub1, ub2, ub3));

                    clearRemaining();
                } else if ((b & B_11100000) == B_11100000) {
                    // 3 bytes
                    if (checkIsRemaining(i+1, utf8Bytes)) break;
                    b2 = utf8Bytes[i+1];
                    if (!isNotHead(b2)) continue;
                    i++;

                    if (checkIsRemaining(i+1, utf8Bytes)) break;
                    b3 = utf8Bytes[i+1];
                    if (!isNotHead(b3)) continue;
                    i++;

                    ub1 = ((b & B_00001111) << 4) + ((b2 & B_00111100) >> 2);
                    ub2 = ((b2 & B_00000011) << 6) + ((b3 & B_00111111));
                    charList.add(makeChar(ub1, ub2));

                    clearRemaining();
                } else {
                    // 2 bytes
                    if (checkIsRemaining(i+1, utf8Bytes)) break;
                    b2 = utf8Bytes[i+1];
                    if (!isNotHead(b2)) continue;
                    i++;

                    ub1 = (b & B_00011100) >> 2;
                    ub2 = ((b & B_00000011) << 6) + (b2 & B_00111111);
                    charList.add(makeChar(ub1, ub2));
                    clearRemaining();
                }
            } catch (IndexOutOfBoundsException e) {
                Rlog.e(LOG_TAG, "toUCS2: " + e);
                break;
            }
        }
        return charList.toArray();
    }
    private boolean isNotHead(byte b) {
        return (b & B_11000000) == B_10000000;
    }
    private int makeChar(int b1, int b2) {
        return ((b1 << 8) + b2);
    }
    private int makeChar(int b1, int b2, int b3) {
        return ((b1 << 16) + (b2 << 8) + b3);
    }

    private boolean checkIsRemaining(int index, byte[] utf8Bytes) {
        addRemaining(utf8Bytes[index-1]);
        if (index >= utf8Bytes.length) {
            return true;
        } else return false;
    }

    private void addRemaining(byte b) {
        mRemaining = appendByteArray(mRemaining, new byte[] {b});
    }

    private void clearRemaining() {
        mRemaining = new byte[]{};
    }

    private byte[] appendByteArray(byte[] a, byte[] b) {
        if (a.length == 0) return b;
        if (b.length == 0) return a;
        byte[] byteArray = new byte[a.length + b.length];
        System.arraycopy(a, 0, byteArray, 0, a.length);
        System.arraycopy(b, 0, byteArray, a.length, b.length);
        return byteArray;
    }

    private class CharList {
        private char[] data = null;
        private int used = 0;
        public void add(int c) {
            if (data == null) {
                data = new char[16];
            } else if (used >= data.length) {
                char[] temp = new char[data.length * 2];
                System.arraycopy(data, 0, temp, 0, used);
                data = temp;
            }
            char[] tmp = Character.toChars(c);
            for (int i = 0 ; i < tmp.length ; i++) {
                data[used++] = tmp[i];
            }
        }
        public char[] toArray() {
            char[] chars = new char[used];
            System.arraycopy(data, 0, chars, 0, used);
            return chars;
        }
    }
    public RttTextEncoder() {
    }
    public String getUnicodeFromUTF8(String utf8) {
        String decodeText = null;
        try {
            String text = new String(utf8);
            int length = text.length();
            if (length <= 0) return null;
            byte[] data = new byte[length / 2];
            for (int i = 0; i < length; i += 2) {
                data[i / 2] = (byte) ((Character.digit(text.charAt(i), 16) << 4)
                        + Character.digit(text.charAt(i+1), 16));
            }
            char[] c = toUCS2(data);
            decodeText = new String(c);
            Rlog.d(LOG_TAG, "Decode len = " + sensitiveEncode(String.valueOf(decodeText.length()))
                    + ", textMessage = " + sensitiveEncode(printBytes(decodeText.getBytes()))
                    + ", remain len: " + sensitiveEncode(String.valueOf(mRemaining.length))
                    + ", " + sensitiveEncode(printBytes(mRemaining)));
        } catch (Exception e) {
            Rlog.e(LOG_TAG, "handleRttTextReceivedIndication:exception " + e);
            return null;
        }

        byte[] bom = new byte[]{(byte)0xEF, (byte)0xBB, (byte)0xBF};
        String BOM = null;
        try {
            BOM = new String(bom, "utf-8");
        } catch (IOException e) {
            Rlog.e(LOG_TAG, "Exception when transcode bom to string, " + e);
        }
        if (decodeText.length() == 1 && decodeText.equals(BOM) && mRemaining.length == 0) {
            Rlog.d(LOG_TAG, "found BOM, ignore it");
            return null;
        }
        return decodeText;
    }

    private String sensitiveEncode(String msg) {
        return ImsServiceCallTracker.sensitiveEncode(msg);
    }
}
