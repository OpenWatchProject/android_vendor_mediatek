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

 package com.mediatek.android.mms.pdu;

import android.util.Log;
import java.io.UnsupportedEncodingException;
import com.google.android.mms.pdu.CharacterSets;
import java.util.HashMap;
public class MtkCharacterSets extends CharacterSets {


private MtkCharacterSets(){
super();
}
/**
     * Map an MIBEnum number to the name of the charset which this number
     * is assigned to by IANA.
     *
     * @param mibEnumValue An IANA assigned MIBEnum number.
     * @return The name string of the charset.
     */
    public static String getMimeName(int mibEnumValue){
        String name = MIBENUM_TO_NAME_MAP.get(mibEnumValue);
        if (name == null) {
            /*
             * M:Code analyze 007,add for ALPS00353101,
             * if the charset can not be found in original
             * map, try to find it in extensional map @{
             */
            name = MIBENUM_TO_NAME_MAP_EXTENDS.get(mibEnumValue);
            if (name == null) {
                return MIMENAME_UTF_8;
            }
            /// @}
        }
        return name;
    }

    /**
     * Map a well-known charset name to its assigned MIBEnum number.
     *
     * @param mimeName The charset name.
     * @return The MIBEnum number assigned by IANA for this charset.
     * @throws UnsupportedEncodingException
     */
    public static int getMibEnumValue(String mimeName)
            throws UnsupportedEncodingException {
        if(null == mimeName) {
            return -1;
        }
        /// M:Code analyze 008,add for ALPS00353101,make the string into lowercase @{
        mimeName = mimeName.toLowerCase();
        /// @}
        Integer mibEnumValue = CharacterSets.NAME_TO_MIBENUM_MAP.get(mimeName);
        if (mibEnumValue == null) {
            Log.i(LOG_TAG, "getMibEnumValue failed, mimeName is: " + mimeName);
            throw new UnsupportedEncodingException();
        }
        return mibEnumValue;
    }

    /// new variable and methods
    /// M:Code analyze 001,add for ALPS00353101,add log tag @{
    private static final String LOG_TAG = "CharacterSets";
    /// @}
    /// M:Code analyze 002,add for ALPS00353101  extends charsets and reference:
    /// http://www.iana.org/assignments/character-sets/ @{
    public static final int BIG5_HKSCS = 0x0835; //2101
    public static final int BOCU_1 = 0x03FC; //1020
    public static final int CESU_8 = 0x03F8; //1016
    public static final int CP864 = 0x0803; //2051
    public static final int EUC_JP = 0x12; //18
    public static final int EUC_KR = 0x26; //38
    public static final int GB18030 = 0x72; //114
    public static final int GBK = 0x71; //113
    public static final int HZ_GB_2312 = 0x0825; //2085
    public static final int GB_2312 = 0x07E9; //2025
    public static final int ISO_2022_CN = 0x68; //104
    public static final int ISO_2022_CN_EXT = 0x69; //105
    public static final int ISO_2022_JP = 0x27; //39
    public static final int ISO_2022_KR = 0x25; //37
    public static final int ISO_8859_10 = 0x0D; //13
    public static final int ISO_8859_13 = 0x6D; //109
    public static final int ISO_8859_14 = 0x6E; //110
    public static final int ISO_8859_15 = 0x6F; //111
    public static final int ISO_8859_16 = 0x70; //112
    public static final int KOI8_R = 0x0824; //2084
    public static final int KOI8_U = 0x0828; //2088
    public static final int MACINTOSH = 0x07EB; //2027
    public static final int SCSU = 0x03F3; //1011
    public static final int TIS_620 = 0x08D3; //2259
    public static final int UTF_16BE = 0x03F5; //1013
    public static final int UTF_16LE = 0x03F6; //1014
    public static final int UTF_32 = 0x03F9; //1017
    public static final int UTF_32BE = 0x03FA; //1018
    public static final int UTF_32LE = 0x03FB; //1019
    public static final int UTF_7 = 0x03F4; //1012
    public static final int WINDOWS_1250 = 0x08CA; //2250
    public static final int WINDOWS_1251 = 0x08CB; //2251
    public static final int WINDOWS_1252 = 0x08CC; //2252
    public static final int WINDOWS_1253 = 0x08CD; //2253
    public static final int WINDOWS_1254 = 0x08CE; //2254
    public static final int WINDOWS_1255 = 0x08CF; //2255
    public static final int WINDOWS_1256 = 0x08D0; //2256
    public static final int WINDOWS_1257 = 0x08D1; //2257
    public static final int WINDOWS_1258 = 0x08D2; //2258
    /// @}
    /// M:Code analyze 003,add for ALPS00353101, Extend array of MIB enum numbers @{
    private static final int[] MIBENUM_NUMBERS_EXTENDS = {
       BIG5_HKSCS,
       BOCU_1,
       CESU_8,
       CP864,
       EUC_JP,
       EUC_KR,
       GB18030,
       GBK,
       HZ_GB_2312,
       GB_2312,
       ISO_2022_CN,
       ISO_2022_CN_EXT,
       ISO_2022_JP,
       ISO_2022_KR,
       ISO_8859_10,
       ISO_8859_13,
       ISO_8859_14,
       ISO_8859_15,
       ISO_8859_16,
       KOI8_R,
       KOI8_U,
       MACINTOSH,
       SCSU,
       TIS_620,
       UTF_16BE,
       UTF_16LE,
       UTF_32,
       UTF_32BE,
       UTF_32LE,
       UTF_7,
       WINDOWS_1250,
       WINDOWS_1251,
       WINDOWS_1252,
       WINDOWS_1253,
       WINDOWS_1254,
       WINDOWS_1255,
       WINDOWS_1256,
       WINDOWS_1257,
       WINDOWS_1258,
    };
    /// @}
    /// M:Code analyze 004,add for ALPS00353101, extends charsets and reference:
    /// http://www.iana.org/assignments/character-sets/ @{
    public static final String MIMENAME_BIG5_HKSCS = "Big5-HKSCS";
    public static final String MIMENAME_BOCU_1 = "BOCU-1";
    public static final String MIMENAME_CESU_8 = "CESU-8";
    public static final String MIMENAME_CP864 = "cp864";
    public static final String MIMENAME_EUC_JP = "EUC-JP";
    public static final String MIMENAME_EUC_KR = "EUC-KR";
    public static final String MIMENAME_GB18030 = "GB18030";
    public static final String MIMENAME_GBK = "GBK";
    public static final String MIMENAME_HZ_GB_2312 = "HZ-GB-2312";
    public static final String MIMENAME_GB_2312 = "GB2312";
    public static final String MIMENAME_ISO_2022_CN = "ISO-2022-CN";
    public static final String MIMENAME_ISO_2022_CN_EXT = "ISO-2022-CN-EXT";
    public static final String MIMENAME_ISO_2022_JP = "ISO-2022-JP";
    public static final String MIMENAME_ISO_2022_KR = "ISO-2022-KR";
    public static final String MIMENAME_ISO_8859_10 = "ISO-8859-10";
    public static final String MIMENAME_ISO_8859_13 = "ISO-8859-13";
    public static final String MIMENAME_ISO_8859_14 = "ISO-8859-14";
    public static final String MIMENAME_ISO_8859_15 = "ISO-8859-15";
    public static final String MIMENAME_ISO_8859_16 = "ISO-8859-16";
    public static final String MIMENAME_KOI8_R = "KOI8-R";
    public static final String MIMENAME_KOI8_U = "KOI8-U";
    public static final String MIMENAME_MACINTOSH = "macintosh";
    public static final String MIMENAME_SCSU = "SCSU";
    public static final String MIMENAME_TIS_620 = "TIS-620";
    public static final String MIMENAME_UTF_16BE = "UTF-16BE";
    public static final String MIMENAME_UTF_16LE = "UTF-16LE";
    public static final String MIMENAME_UTF_32 = "UTF-32";
    public static final String MIMENAME_UTF_32BE = "UTF-32BE";
    public static final String MIMENAME_UTF_32LE = "UTF-32LE";
    public static final String MIMENAME_UTF_7 = "UTF-7";
    public static final String MIMENAME_WINDOWS_1250 = "windows-1250";
    public static final String MIMENAME_WINDOWS_1251 = "windows-1251";
    public static final String MIMENAME_WINDOWS_1252 = "windows-1252";
    public static final String MIMENAME_WINDOWS_1253 = "windows-1253";
    public static final String MIMENAME_WINDOWS_1254 = "windows-1254";
    public static final String MIMENAME_WINDOWS_1255 = "windows-1255";
    public static final String MIMENAME_WINDOWS_1256 = "windows-1256";
    public static final String MIMENAME_WINDOWS_1257 = "windows-1257";
    public static final String MIMENAME_WINDOWS_1258 = "windows-1258";
    /// @}
    /// M:Code analyze 005,add for ALPS00353101, extends MIMINAME @}
    private static final String[] MIME_NAMES_EXTENDS = {
        MIMENAME_BIG5_HKSCS,
        MIMENAME_BOCU_1,
        MIMENAME_CESU_8,
        MIMENAME_CP864,
        MIMENAME_EUC_JP,
        MIMENAME_EUC_KR,
        MIMENAME_GB18030,
        MIMENAME_GBK,
        MIMENAME_HZ_GB_2312,
        MIMENAME_GB_2312,
        MIMENAME_ISO_2022_CN,
        MIMENAME_ISO_2022_CN_EXT,
        MIMENAME_ISO_2022_JP,
        MIMENAME_ISO_2022_KR,
        MIMENAME_ISO_8859_10,
        MIMENAME_ISO_8859_13,
        MIMENAME_ISO_8859_14,
        MIMENAME_ISO_8859_15,
        MIMENAME_ISO_8859_16,
        MIMENAME_KOI8_R,
        MIMENAME_KOI8_U,
        MIMENAME_MACINTOSH,
        MIMENAME_SCSU,
        MIMENAME_TIS_620,
        MIMENAME_UTF_16BE,
        MIMENAME_UTF_16LE,
        MIMENAME_UTF_32,
        MIMENAME_UTF_32BE,
        MIMENAME_UTF_32LE,
        MIMENAME_UTF_7,
        MIMENAME_WINDOWS_1250,
        MIMENAME_WINDOWS_1251,
        MIMENAME_WINDOWS_1252,
        MIMENAME_WINDOWS_1253,
        MIMENAME_WINDOWS_1254,
        MIMENAME_WINDOWS_1255,
        MIMENAME_WINDOWS_1256,
        MIMENAME_WINDOWS_1257,
        MIMENAME_WINDOWS_1258,
        };
    /// @}
    /// M:Code analyze 006,add for ALPS00353101,create a new hash map for extending encoding @{
    protected static final HashMap<Integer, String> MIBENUM_TO_NAME_MAP_EXTENDS;
    /*Mtk Change*/
      protected static final HashMap<String, Integer> NAME_TO_MIBENUM_MAP_EXTENDS;
    /// @}

    static {
        /// M:Code analyze 006,add for ALPS00353101,create a new hash map for extending encoding @{
        MIBENUM_TO_NAME_MAP_EXTENDS = new HashMap<Integer, String>();
        NAME_TO_MIBENUM_MAP_EXTENDS = new HashMap<String, Integer>();
        /// @}

        /// M:Code analyze 006,add for ALPS00353101,create a new hash map for extending encoding,
        /// set values into the new hash map @{
        assert (MIBENUM_NUMBERS_EXTENDS.length == MIME_NAMES_EXTENDS.length);
        int len = MIBENUM_NUMBERS_EXTENDS.length - 1;
        for (int i = 0; i <= len; i++) {
            MIBENUM_TO_NAME_MAP_EXTENDS.put(MIBENUM_NUMBERS_EXTENDS[i], MIME_NAMES_EXTENDS[i]);
            NAME_TO_MIBENUM_MAP_EXTENDS.put(MIME_NAMES_EXTENDS[i], MIBENUM_NUMBERS_EXTENDS[i]);
        }
        /// @}
    }

}