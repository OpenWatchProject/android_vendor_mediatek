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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

package com.mediatek.internal.telephony.cdma.pluscode;

import android.telephony.Rlog;
import android.util.SparseIntArray;

import java.util.ArrayList;
import java.util.List;

/**
 * The utilities class of telephony plus code function.
 * @hide
 */
public class TelephonyPlusCode {

    static final String LOG_TAG = "TelephonyPlusCode";

    public static final String PROPERTY_TIME_LTMOFFSET = "vendor.cdma.operator.ltmoffset";
    public static final String PROPERTY_ICC_CDMA_OPERATOR_MCC = "vendor.cdma.icc.operator.mcc";

    private static List<SidMccMnc> sSidMccMncList = null;

    /**
     * only contains the countries that China Telecom supports international roaming.
     */
    public static final MccIddNddSid[] MCC_IDD_NDD_SID_MAP = {
        new MccIddNddSid(302, "1"  , 16384, 18431, "011"  , "1"), //canada
        new MccIddNddSid(310, "1"  , 1    , 2175 , "011"  , "1"), //usa
        new MccIddNddSid(311, "1"  , 2304 , 7679 , "011"  , "1"), //usa
        new MccIddNddSid(312, "1"  , 0    , 0    , "011"  , "1"), //usa
        new MccIddNddSid(313, "1"  , 0    , 0    , "011"  , "1"), //usa
        new MccIddNddSid(314, "1"  , 0    , 0    , "011"  , "1"), //usa
        new MccIddNddSid(315, "1"  , 0    , 0    , "011"  , "1"), //usa
        new MccIddNddSid(316, "1"  , 0    , 0    , "011"  , "1"), //usa
        new MccIddNddSid(334, "52" , 24576, 25075, "00"   , "01"), //mexico
        new MccIddNddSid(334, "52" , 25100, 25124, "00"   , "01"), //mexico
        new MccIddNddSid(404, "91" , 14464, 14847, "00"   , "0"), //India
        new MccIddNddSid(405, "91" , 14464, 14847, "00"   , "0"), //India
        new MccIddNddSid(425, "972", 8448 , 8479 , "00"   , "0"), //Israel
        new MccIddNddSid(428, "976", 15520, 15551, "002"  , "0"), //Mongolia
        new MccIddNddSid(440, "81" , 12288, 13311, "010"  , "0"), //Japan
        new MccIddNddSid(450, "82" , 2176 , 2303 , "00700", "0"), //Korea(South)
        new MccIddNddSid(452, "84" , 13312, 13439, "00"   , "0"), //Vietnam
        new MccIddNddSid(454, "852", 10640, 10655, "001"  , ""), //Hong Kong
        new MccIddNddSid(455, "853", 11296, 11311, "00"   , "0"), //Macao/Macau
        new MccIddNddSid(460, "86" , 13568, 14335, "00"   , "0"), //china
        new MccIddNddSid(460, "86" , 25600, 26111, "00"   , "0"), //china
        new MccIddNddSid(466, "886", 13504, 13535, "005"  , "0"), //Taiwan
        new MccIddNddSid(470, "880", 13472, 13503, "00"   , "0"), //Bangladesh
        new MccIddNddSid(510, "62" , 10496, 10623, "001"  , "0"), //Indonesia
    };

    public static final MccSidLtmOff[] MCC_SID_LTM_OFF_MAP = {
        new MccSidLtmOff(310, 1    , -20, -10),   new MccSidLtmOff(404, 1    , 11, 11),
        new MccSidLtmOff(310, 7    , -20, -10),   new MccSidLtmOff(404, 7    , 11, 11),

        new MccSidLtmOff(310, 13   , -20, -10),   new MccSidLtmOff(454, 13   , 16, 16),

        new MccSidLtmOff(310, 1111 , -20, -10),   new MccSidLtmOff(450, 1111 , 18, 18),
        new MccSidLtmOff(310, 1112 , -20, -10),   new MccSidLtmOff(450, 1112 , 18, 18),
        new MccSidLtmOff(310, 1113 , -20, -10),   new MccSidLtmOff(450, 1113 , 18, 18),
        new MccSidLtmOff(310, 1700 , -20, -10),   new MccSidLtmOff(450, 1700 , 18, 18),
        new MccSidLtmOff(310, 2177 , -20, -10),   new MccSidLtmOff(450, 2177 , 18, 18),
        new MccSidLtmOff(310, 2179 , -20, -10),   new MccSidLtmOff(450, 2179 , 18, 18),
        new MccSidLtmOff(310, 2181 , -20, -10),   new MccSidLtmOff(450, 2181 , 18, 18),
        new MccSidLtmOff(310, 2183 , -20, -10),   new MccSidLtmOff(450, 2183 , 18, 18),
        new MccSidLtmOff(310, 2185 , -20, -10),   new MccSidLtmOff(450, 2185 , 18, 18),
        new MccSidLtmOff(310, 2187 , -20, -10),   new MccSidLtmOff(450, 2187 , 18, 18),
        new MccSidLtmOff(310, 2189 , -20, -10),   new MccSidLtmOff(450, 2189 , 18, 18),
        new MccSidLtmOff(310, 2191 , -20, -10),   new MccSidLtmOff(450, 2191 , 18, 18),
        new MccSidLtmOff(310, 2193 , -20, -10),   new MccSidLtmOff(450, 2193 , 18, 18),
        new MccSidLtmOff(310, 2195 , -20, -10),   new MccSidLtmOff(450, 2195 , 18, 18),
        new MccSidLtmOff(310, 2197 , -20, -10),   new MccSidLtmOff(450, 2197 , 18, 18),
        new MccSidLtmOff(310, 2199 , -20, -10),   new MccSidLtmOff(450, 2199 , 18, 18),

        new MccSidLtmOff(310, 2201 , -20, -10),   new MccSidLtmOff(450, 2201 , 18, 18),
        new MccSidLtmOff(310, 2203 , -20, -10),   new MccSidLtmOff(450, 2203 , 18, 18),
        new MccSidLtmOff(310, 2205 , -20, -10),   new MccSidLtmOff(450, 2205 , 18, 18),
        new MccSidLtmOff(310, 2207 , -20, -10),   new MccSidLtmOff(450, 2207 , 18, 18),
        new MccSidLtmOff(310, 2209 , -20, -10),   new MccSidLtmOff(450, 2209 , 18, 18),
        new MccSidLtmOff(310, 2211 , -20, -10),   new MccSidLtmOff(450, 2211 , 18, 18),
        new MccSidLtmOff(310, 2213 , -20, -10),   new MccSidLtmOff(450, 2213 , 18, 18),
        new MccSidLtmOff(310, 2215 , -20, -10),   new MccSidLtmOff(450, 2215 , 18, 18),
        new MccSidLtmOff(310, 2217 , -20, -10),   new MccSidLtmOff(450, 2217 , 18, 18),
        new MccSidLtmOff(310, 2219 , -20, -10),   new MccSidLtmOff(450, 2219 , 18, 18),
        new MccSidLtmOff(310, 2221 , -20, -10),   new MccSidLtmOff(450, 2221 , 18, 18),
        new MccSidLtmOff(310, 2223 , -20, -10),   new MccSidLtmOff(450, 2223 , 18, 18),
        new MccSidLtmOff(310, 2225 , -20, -10),   new MccSidLtmOff(450, 2225 , 18, 18),
        new MccSidLtmOff(310, 2227 , -20, -10),   new MccSidLtmOff(450, 2227 , 18, 18),
        new MccSidLtmOff(310, 2229 , -20, -10),   new MccSidLtmOff(450, 2229 , 18, 18),
        new MccSidLtmOff(310, 2231 , -20, -10),   new MccSidLtmOff(450, 2231 , 18, 18),
        new MccSidLtmOff(310, 2233 , -20, -10),   new MccSidLtmOff(450, 2233 , 18, 18),
        new MccSidLtmOff(310, 2235 , -20, -10),   new MccSidLtmOff(450, 2235 , 18, 18),
        new MccSidLtmOff(310, 2237 , -20, -10),   new MccSidLtmOff(450, 2237 , 18, 18),
        new MccSidLtmOff(310, 2239 , -20, -10),   new MccSidLtmOff(450, 2239 , 18, 18),
        new MccSidLtmOff(310, 2241 , -20, -10),   new MccSidLtmOff(450, 2241 , 18, 18),
        new MccSidLtmOff(310, 2243 , -20, -10),   new MccSidLtmOff(450, 2243 , 18, 18),
        new MccSidLtmOff(310, 2301 , -20, -10),   new MccSidLtmOff(450, 2301 , 18, 18),
        new MccSidLtmOff(310, 2303 , -20, -10),   new MccSidLtmOff(450, 2303 , 18, 18),
        new MccSidLtmOff(310, 2369 , -20, -10),   new MccSidLtmOff(450, 2369 , 18, 18),
        new MccSidLtmOff(310, 2370 , -20, -10),   new MccSidLtmOff(450, 2370 , 18, 18),
        new MccSidLtmOff(310, 2371 , -20, -10),   new MccSidLtmOff(450, 2371 , 18, 18),

        new MccSidLtmOff(450, 2222 , 18 ,  18),   new MccSidLtmOff(404, 2222 , 11, 11),

        new MccSidLtmOff(440, 12461, 18 ,  18),   new MccSidLtmOff(470, 12461, 12, 12),
        new MccSidLtmOff(440, 12463, 18 ,  18),   new MccSidLtmOff(470, 12463, 12, 12),
        new MccSidLtmOff(440, 12464, 18 ,  18),   new MccSidLtmOff(470, 12464, 12, 12),
    };

    public static final SparseIntArray MOBILE_NUMBER_SPEC_MAP = new SparseIntArray();
    static {
        MOBILE_NUMBER_SPEC_MAP.put(13, 86);
        MOBILE_NUMBER_SPEC_MAP.put(15, 86);
        MOBILE_NUMBER_SPEC_MAP.put(18, 86);
        MOBILE_NUMBER_SPEC_MAP.put(99, 91);
        MOBILE_NUMBER_SPEC_MAP.put(17, 86);
    }

    public static final String[] IDD_MAP_2_CHAR = new String[] {
        "00",
    };

    public static final String[] IDD_MAP_3_CHAR = new String[] {
        "000", "001", "002", "005", "009", "010", "011", "020", "810"
    };

    public static final String[] IDD_MAP_4_CHAR = new String[] {
        "0011", "0015",
    };

    public static final String[] IDD_MAP_5_CHAR = new String[] {
        "00700", "17700"
    };

    /**
     * @return the SidMccMnc list.
     */
    public static List<SidMccMnc> getSidMccMncList() {
        Rlog.d(LOG_TAG, "[getSidMccMncList] getSidMccMncList = " + (sSidMccMncList == null));

        if (sSidMccMncList == null) {
            initSidMccMncList();
        }

        return sSidMccMncList;
    }

    private static void initSidMccMncList() {
        Rlog.d(LOG_TAG, "[InitSidMccMncList] InitSidMccMncList");

        sSidMccMncList = new ArrayList<SidMccMnc>();

        add(2, 310010);      //Verizon
        add(3, 310010);      //Verizon
        add(4, 310010);      //Verizon
        add(5, 310730);      //U.S.Cellular
        add(6, 310010);      //Verizon
        add(8, 310010);      //Verizon
        add(10, 310070);     //Cingular
        add(12, 310010);      //Verizon
        add(15, 310010);      //Verizon
        add(17, 310010);      //Verizon
        add(18, 310010);      //Verizon
        add(20, 310010);      //Verizon
        add(21, 310010);      //Verizon
        add(22, 310010);      //Verizon
        add(26, 310010);      //Verizon
        add(28, 310010);      //Verizon
        add(30, 310010);      //Verizon
        add(32, 310010);      //Verizon
        add(40, 310010);      //Verizon
        add(41, 310010);      //Verizon
        add(42, 310500);       //Alltel
        add(44, 310070);      //Cingular
        add(45, 310500);      //Alltel
        add(46, 310070);      //Cingular
        add(48, 310010);      //Verizon
        add(51, 310010);      //Verizon
        add(53, 310500);       //Alltel
        add(54, 310500);       //Alltel
        add(56, 310010);      //Verizon
        add(57, 310500);       //Alltel
        add(58, 310010);      //Verizon
        add(59, 310010);      //Verizon
        add(60, 310010);      //Verizon
        add(64, 310010);      //Verizon
        add(65, 310010);      //Verizon
        add(69, 310010);      //Verizon
        add(73, 310010);      //Verizon
        add(74, 310500);      //Alltel
        add(78, 310010);      //Verizon
        add(79, 310500);      //Alltel
        add(80, 310010);      //Verizon
        add(81, 310070);      //Cingular
        add(83, 310500);       //Alltel
        add(84, 310500);       //Alltel
        add(85, 310500);       //Alltel
        add(81, 310070);      //Cingular
        add(83, 310500);      //Alltel
        add(84, 310500);      //Alltel
        add(85, 310500);      //Alltel
        add(86, 310010);      //Verizon
        add(92, 310010);      //Verizon
        add(93, 310010);      //Verizon
        add(94, 310010);      //Verizon
        add(95, 310010);      //Verizon
        add(96, 310010);      //Verizon
        add(97, 310500);      //Alltel
        add(100, 310500);      //Alltel
        add(106, 310070);      //Cingular
        add(110, 310010);      //Verizon
        add(112, 310010);      //Verizon
        add(113, 310010);      //Verizon
        add(114, 310500);      //Alltel
        add(116, 310500);      //Alltel
        add(119, 310010);      //Verizon
        add(120, 310500);      //Alltel
        add(126, 310500);      //Alltel
        add(127, 310010);      //Verizon
        add(130, 310500);      //Alltel
        add(133, 310010);      //Verizon
        add(137, 310010);      //Verizon
        add(138, 310070);      //Cingular
        add(139, 310010);      //Verizon
        add(140, 310010);      //Verizon
        add(142, 310500);      //Alltel
        add(143, 310010);       //Verizon
        add(144, 310500);       //Alltel
        add(150, 310010);       //Verizon
        add(152, 310500);       //Alltel
        add(154, 310010);      //Verizon
        add(156, 310500);      //Alltel
        add(162, 310010);       //Verizon
        add(163, 310010);       //Verizon
        add(165, 310010);       //Verizon
        add(166, 310730);       //U.S.Cellular
        add(169, 310500);        //Alltel
        add(179, 310010);        //Verizon
        add(180, 310010);        //Verizon
        add(181, 310010);        //Verizon
        add(182, 310500);        //Alltel
        add(186, 310010);        //Verizon
        add(188, 310500);        //Alltel
        add(189, 310010);        //Verizon
        add(190, 310010);        //Verizon
        add(204, 310500);        //Alltel
        add(205, 310500);        //Alltel
        add(208, 310500);        //Alltel
        add(212, 310500);        //Alltel
        add(214, 310010);        //Verizon
        add(215, 310070);         //Cingular
        add(216, 310500);         //Alltel
        add(220, 310500);         //Alltel
        add(222, 310010);        //Verizon
        add(224, 310010);        //Verizon
        add(226, 310010);        //Verizon
        add(228, 310010);        //Verizon
        add(229, 310070);        //Cingular
        add(234, 310050);        //ACS Wireless
        add(240, 310500);        //Alltel
        add(241, 310010);        //Verizon
        add(244, 310500);        //Alltel
        add(249, 311130);        //Amarillo Cellular
        add(250, 310010);        //Verizon
        add(256, 310500);        //Alltel
        add(260, 310500);        //Alltel
        add(262, 310010);         //Verizon
        add(264, 0);             //Cellular South
        add(266, 310010);         //Verizon
        add(272, 310010);         //Verizon
        add(276, 310010);         //Verizon
        add(277, 310030);         //CENT USA
        add(281, 310500);         //Alltel
        add(284, 310010);         //Verizon
        add(285, 310500);         //Alltel
        add(286, 310010);         //Verizon
        add(294, 310010);         //Verizon
        add(298, 310730);         //U.S.Cellular
        add(299, 310010);         //Verizon
        add(300, 310010);         //Verizon
        add(302, 310500);         //Alltel
        add(312, 310500);         //Alltel
        add(316, 310010);          //Verizon
        add(318, 310500);          //Alltel
        add(319, 0);              //Midwest Wireless
        add(323, 310010);         //Verizon
        add(324, 0);              //Pioneer/Enid Cellular
        add(328, 310010);         //Verizon
        add(329, 310010);         //Verizon
        add(330, 310010);         //Verizon
        add(340, 310500);         //Alltel
        add(341, 0);              //Dobson Cellular Systems
        add(342, 310500);         //Alltel
        add(348, 310500);         //Alltel
        add(349, 310010);         //Verizon
        add(350, 310500);          //Alltel
        add(359, 310070);          //Cingular
        add(361, 310070);          //Cingular Wireless
        add(362, 42502);           //Cellcom Israel
        add(364, 310730);          //U.S.Cellular
        add(368, 310500);          //Alltel
        add(370, 310500);          //Alltel
        add(371, 310500);          //Alltel
        add(374, 310500);          //Alltel
        add(376, 310500);          //Alltel
        add(377, 310010);          //Verizon
        add(384, 310730);          //U.S.Cellular
        add(386, 310500);          //Alltel
        add(392, 310500);          //Alltel
        add(396, 310500);          //Alltel
        add(400, 310070);          //Cingular
        add(403, 310030);          //CENT USA
        add(404, 310010);          //Verizon
        add(414, 310070);          //Cingular
        add(416, 310500);          //Alltel
        add(418, 310500);          //Alltel
        add(424, 310500);          //Alltel
        add(426, 310070);          //Cingular
        add(428, 310010);          //Verizon
        add(436, 310730);          //U.S.Cellular
        add(440, 310500);          //Alltel
        add(443, 310010);          //Verizon
        add(444, 310500);          //Alltel
        add(447, 310010);          //Verizon
        add(448, 310500);          //Alltel
        add(451, 310500);          //Alltel
        add(463, 310070);          //Cingular Wireless
        add(478, 310500);          //Alltel
        add(486, 310010);          //Verizon
        add(487, 310500);          //Alltel
        add(498, 310010);          //Verizon
        add(502, 310010);          //Verizon
        add(506, 310010);          //Verizon
        add(510, 310180);          //West Central Wireless
        add(511, 310500);          //Alltel
        add(520, 310500);          //Alltel
        add(528, 310010);          //Verizon
        add(529, 310500);          //Alltel
        add(530, 310010);          //Verizon
        add(532, 310010);          //Verizon
        add(539, 310010);          //Verizon
        add(544, 310500);          //Alltel
        add(546, 310500);          //Alltel
        add(550, 310500);          //Alltel
        add(555, 310500);          //Alltel
        add(574, 310730);          //U.S.Cellular
        add(578, 310500);          //Alltel
        add(579, 310070);          //Cingular Wireless
        add(580, 310730);          //U.S.Cellular
        add(587, 310070);          //Cingular Wireless
        add(607, 310070);          //Cingular
        add(1015, 310010);         //Verizon
        add(1018, 310050);         //ACS Wireless
        add(1022, 310050);         //ACS Wireless
        add(1024, 310350);         //Mohave Cellular
        add(1026, 310010);         //Verizon
        add(1027, 310320);         //Smith Bagley
        add(1028, 310010);         //Verizon
        add(1029, 310500);         //Alltel
        add(1030, 310010);         //Verizon
        add(1034, 310010);         //Verizon
        add(1038, 310500);         //Alltel
        add(1055, 310070);         //Cingular
        add(1058, 310500);         //Alltel
        add(1060, 310010);         //Verizon
        add(1064, 311590);         //Golden State Cellular
        add(1069, 310500);         //Alltel
        add(1083, 310010);         //Verizon
        add(1086, 310010);         //Verizon
        add(1088, 310010);         //Verizon
        add(1093, 310500);         //Alltel
        add(1101, 310500);         //Alltel
        add(1124, 310500);         //Alltel
        add(1129, 310010);         //Verizon
        add(1139, 310010);         //Verizon
        add(1148, 310500);         //Alltel
        add(1151, 310010);         //Verizon
        add(1153, 310010);         //Verizon
        add(1155, 310070);         //Cingular Wireless
        add(1164, 310010);         //Verizon
        add(1165, 310500);         //Alltel
        add(1173, 310500);         //Alltel
        add(1174, 310010);         //Verizon
        add(1180, 310010);         //Verizon
        add(1189, 310010);         //Verizon
        add(1192, 310500);         //Alltel
        add(1200, 310730);         //U.S.Cellular
        add(1211, 310730);         //U.S.Cellular
        add(1212, 311430);         //Cellular 29 Plus
        add(1213, 310730);         //U.S.Cellular
        add(1215, 310730);         //U.S.Cellular
        add(1216, 0);              //Midwest Wireless
        add(1220, 310010);         //Verizon
        add(1232, 0);              //Midwest Wireless
        add(1234, 0);              //Midwest Wireless
        add(1236, 0);              //Midwest Wireless
        add(1255, 310890);         //Rural Cellular
        add(1258, 310500);         //Alltel
        add(1267, 310890);         //Rural Cellular
        add(1271, 310500);         //Alltel
        add(1272, 310730);         //U.S.Cellular
        add(1280, 311440);         //Bluegrass Cellular
        add(1290, 0);              //Appalachian Wireless
        add(1317, 310730);         //U.S.Cellular
        add(1320, 310730);         //U.S.Cellular
        add(1332, 310500);         //Alltel
        add(1333, 0);              //Dobson Cellular Systems
        add(1335, 0);              //Dobson Cellular Systems
        add(1336, 310500);         //Alltel
        add(1337, 0);              //Dobson Cellular Systems
        add(1338, 310500);         //Alltel
        add(1341, 310030);         //CENT USA
        add(1345, 310030);         //CENT USA
        add(1350, 311050);         //Thumb Cellular
        add(1367, 310500);         //Alltel
        add(1369, 310500);         //Alltel
        add(1370, 0);              //??
        add(1372, 0);              //Midwest Wireless
        add(1375, 310500);         //Alltel
        add(1382, 0);              //Cellular South
        add(1383, 310500);         //Alltel
        add(1385, 310500);         //Alltel
        add(1393, 310500);         //Alltel
        add(1394, 0);              //Cellular South
        add(1396, 311420);         //Northwest Missouri Cellular
        add(1399, 310730);         //U.S.Cellular
        add(1400, 310500);         //Alltel
        add(1403, 310730);         //U.S.Cellular
        add(1406, 310730);         //U.S.Cellular
        add(1408, 310010);         //Verizon
        add(1411, 310730);         //U.S.Cellular
        add(1419, 310730);         //U.S.Cellular
        add(1423, 310730);         //U.S.Cellular
        add(1425, 310730);         //U.S.Cellular
        add(1434, 310010);         //Verizon
        add(1441, 310500);         //Alltel
        add(1453, 311030);         //Indigo Wireless
        add(1465, 310730);         //U.S.Cellular
        add(1466, 310500);         //Alltel
        add(1473, 310500);         //Alltel
        add(1484, 310730);         //U.S.Cellular
        add(1493, 310500);         //Alltel
        add(1496, 310100);         //Plateau Wireless
        add(1499, 310500);         //Alltel
        add(1500, 310100);         //Plateau Wireless
        add(1504, 310100);         //Plateau Wireless
        add(1506, 310010);         //Verizon
        add(1508, 310010);         //Verizon
        add(1516, 310010);         //Verizon
        add(1522, 310130);         //Carolina West Wireless
        add(1528, 310500);         //Alltel
        add(1530, 310500);         //Alltel
        add(1532, 310500);         //Alltel
        add(1534, 310500);         //Alltel
        add(1536, 310500);         //Alltel
        add(1538, 310500);         //Alltel
        add(1540, 310500);         //Alltel
        add(1541, 310730);         //U.S.Cellular
        add(1542, 310500);         //Alltel
        add(1544, 310500);         //Alltel
        add(1546, 310500);         //Alltel
        add(1548, 310010);         //Verizon
        add(1559, 310010);         //Verizon
        add(1567, 310010);         //Verizon
        add(1574, 310500);         //Alltel
        add(1590, 0);              //Dobson Cellular Systems
        add(1595, 310730);         //U.S.Cellular
        add(1598, 311080);         //Pine Cellular
        add(1607, 310730);         //U.S.Cellular
        add(1608, 0);              //Ramcell
        add(1609, 310890);         //Rural Cellular
        add(1610, 310730);         //U.S.Cellular
        add(1640, 310500);         //Alltel
        add(1643, 310730);         //U.S.Cellular
        add(1645, 0);              //Triton
        add(1650, 310500);         //Alltel
        add(1652, 310500);         //Alltel
        add(1661, 310500);         //Alltel
        add(1692, 310950);         //XIT
        add(1696, 310100);         //Plateau Wireless
        add(1703, 310500);         //Alltel
        add(1739, 310500);         //Alltel
        add(1740, 310010);         //Verizon
        add(1741, 310500);         //Alltel
        add(1742, 310020);         //UnionTel
        add(1748, 310010);         //Verizon
        add(1749, 310010);         //Verizon
        add(1759, 310500);         //Alltel
        add(1776, 310010);         //Verizon
        add(1779, 310730);         //U.S.Cellular
        add(1784, 310730);         //U.S.Cellular
        add(1794, 310730);         //U.S.Cellular
        add(1802, 310730);         //U.S.Cellular
        add(1812, 0);              //Midwest Wireless
        add(1818, 310500);         //Alltel
        add(1823, 310500);         //Alltel
        add(1825, 310500);         //Alltel
        add(1826, 310010);         //Verizon
        add(1827, 310010);         //Verizon
        add(1828, 310020);         //UnionTel
        add(1830, 310010);         //Verizon
        add(1868, 310980);         //AT&amp;T
        add(1892, 310860);         //Five Star Wireless
        add(1902, 310500);         //Alltel
        add(1912, 310010);         //Verizon
        add(1922, 311150);         //Wilkes Cellular
        add(1932, 311000);         //Mid-Tex Cellular
        add(1949, 311040);         //Commnet
        add(1970, 310540);         //Oklahoma Western Telephone
        add(1976, 0);              //Brazos Celllular
        add(1989, 310500);         //Alltel
        add(1996, 0);              //Cellular South
        add(2038, 310500);         //Alltel
        add(2058, 310010);         //Verizon
        add(2115, 310010);         //Verizon
        add(2119, 310010);         //Verizon
        add(2129, 310890);         //Rural Cellular
        add(2141, 310730);         //U.S.Cellular
        add(3000, 311040);         //Commnet
        add(3034, 311040);         //Commnet
        add(3066, 310010);         //Verizon
        add(3076, 310020);         //UnionTel
        add(3226, 310500);         //Alltel
        add(3462, 0);              //Custer Telephone Cooperative
        add(4103, 310120);         //Sprint
        add(4106, 310120);         //Sprint
        add(4107, 310120);         //Sprint
        add(4110, 310500);         //Alltel
        add(4119, 310010);         //Verizon
        add(4120, 310120);         //Sprint
        add(4121, 310120);         //Sprint
        add(4124, 310120);         //Sprint
        add(4126, 310120);         //Sprint
        add(4132, 310120);         //Sprint
        add(4135, 310120);         //Sprint
        add(4138, 310010);         //Verizon
        add(4139, 310120);         //Sprint
        add(4144, 310120);         //Sprint
        add(4145, 310120);         //Sprint
        add(4148, 310120);         //Sprint
        add(4151, 310120);         //Sprint
        add(4152, 310010);         //Verizon
        add(4153, 310120);         //Sprint
        add(4154, 310010);         //Verizon
        add(4155, 310120);         //Sprint
        add(4157, 310120);         //Sprint
        add(4159, 310120);         //Sprint
        add(4160, 310010);         //Verizon
        add(4162, 310120);         //Sprint
        add(4164, 310120);         //Sprint
        add(4166, 310120);         //Sprint
        add(4168, 310120);         //Sprint
        add(4170, 310120);         //Sprint
        add(4171, 310120);         //Sprint
        add(4174, 310120);         //Sprint
        add(4180, 310120);         //Sprint
        add(4181, 310120);         //Sprint
        add(4182, 310010);         //Verizon
        add(4183, 310120);         //Sprint
        add(4186, 310120);         //Sprint
        add(4188, 310120);         //Sprint
        add(4190, 310120);         //Sprint
        add(4192, 310010);         //Verizon
        add(4195, 310120);         //Sprint
        add(4198, 310120);         //Sprint
        add(4199, 0);              // 3 Rivers Wireless
        add(4225, 310500);         //Alltel
        add(4274, 310120);         //Sprint
        add(4292, 310016);         //Cricket
        add(4325, 310016);         //Cricket
        add(4376, 310120);         //Sprint
        add(4381, 310016);         //Cricket
        add(4384, 310120);         //Sprint
        add(4390, 310120);         //Sprint
        add(4396, 310120);         //Sprint
        add(4413, 310016);         //Cricket
        add(4418, 310120);         //Sprint
        add(4509, 310016);         //Cricket
        add(4518, 310016);         //Cricket
        add(4535, 310016);         //Cricket
        add(4622, 310120);         //Sprint
        add(4647, 310016);         //Cricket
        add(4654, 310120);         //Sprint
        add(4667, 310016);         //Cricket
        add(4693, 310016);         //Cricket
        add(4694, 310120);         //Sprint
        add(4743, 310016);         //Cricket
        add(4771, 310016);         //Cricket
        add(4809, 310016);         //Cricket
        add(4812, 310120);         //Sprint
        add(4828, 0);              //Qwest
        add(4857, 310016);         //Cricket
        add(4923, 310016);         //Cricket
        add(4928, 0);              //Qwest
        add(4961, 310016);         //Cricket
        add(4973, 310016);         //Cricket
        add(4979, 0);              //??
        add(4982, 310120);         //Sprint
        add(5019, 310016);         //Cricket
        add(5027, 310016);         //Cricket
        add(5105, 310016);         //Cricket
        add(5116, 310120);         //Sprint
        add(5117, 310016);         //Cricket
        add(5142, 310120);         //Sprint
        add(5145, 310016);         //Cricket
        add(5173, 310016);         //Cricket
        add(5269, 310050);         //ACS Wireless
        add(5351, 0);              //Qwest
        add(5361, 310016);         //Cricket
        add(5386, 310016);         //Cricket
        add(5450, 310016);         //Cricket
        add(5458, 310016);         //Cricket
        add(5461, 0);              //Qwest
        add(5510, 310050);         //ACS Wireless
        add(5513, 310050);         //ACS Wireless
        add(5540, 310016);         //Cricket
        add(5586, 310016);         //Cricket
        add(5618, 310016);         //Cricket
        add(5660, 0);              //Cellular South
        add(5667, 310500);         //Alltel
        add(5682, 310050);         //ACS Wireless
        add(5685, 310050);         //ACS Wireless
        add(5756, 310016);         //Cricket
        add(5908, 310730);         //U.S.Cellular
        add(5911, 310730);         //U.S.Cellular
        add(5914, 310016);         //Cricket
        add(5945, 310016);         //Cricket
        add(6249, 310016);         //Cricket
        add(6323, 310016);         //Cricket
        add(6371, 310016);         //Cricket
        add(6415, 310016);         //Cricket
        add(6425, 310016);         //Cricket
        add(6439, 310016);         //Cricket
        add(6441, 310016);         //Cricket
        add(6488, 310730);         //U.S.Cellular
        add(6490, 311440);         //Bluegrass Cellular
        add(7316, 311040);         //Commnet
        add(8097, 0);              //Beuda Digita
        add(8176, 0);              //Oceanic Digital Jamaica
        add(8832, 0);              //Codetel Comunicaciones Moviles
        add(8861, 0);              //MoCelCo
        add(8863, 311040);         //Commnet
        add(8950, 311040);         //Commnet
        add(9246, 311040);         //Commnet
        add(9332, 311040);         //Commnet
        add(9562, 311040);         //Commnet
        add(16384, 30286);         //Telus
        add(16390, 302702);        //MT&T Mobility
        add(16408, 0);             //NBTel Mobility
        add(16410, 302654);         //SaskTel Mobility
        add(16412, 302654);         //SaskTel Mobility
        add(16414, 302703);         //NewTel Mobility
        add(16418, 0);             //ThunderBay Mobility
        add(16420, 302610);        //Bell Mobility
        add(16422, 30286);         //Telus
        add(16428, 302660);         //MTS
        add(16430, 0);             //IslandTel Mobility
        add(16462, 0);             //NorTel Mobility
        add(16472, 0);             //NMI Mobility
        add(25100, 0);             //Operadora Unefon (Mexico)
        add(30524, 310500);        //Alltel
        add(30635, 310500);        //Alltel
        add(31092, 311350);        //Sagebrush Cellular
//      Add(31092, 0);             //Triangle Communication Systems
    }

    private static void add(int mSid, int mMccMnc) {
        SidMccMnc mSidMccMnc = new SidMccMnc(mSid, mMccMnc);
        sSidMccMncList.add(mSidMccMnc);
    }

}
