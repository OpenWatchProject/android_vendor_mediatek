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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
/*
 * File name:  RpIrMccConvertMappingTables.cpp
 * Author: Gang Xu (MTK80181)
 * Description:
 * RIL proxy IR MCC convert mapping tables class
 */

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RpIrMccConvertMappingTables.h"

const RpIrMccIddNddSid RpIrMccConvertMappingTables::MCC_IDD_NDD_SID_MAP[] = {
    RpIrMccIddNddSid(302, "1"  , 16384, 18431, "011"  , "1"),  // canada
    RpIrMccIddNddSid(310, "1"  , 1    , 2175 , "011"  , "1"),  // usa
    RpIrMccIddNddSid(311, "1"  , 2304 , 7679 , "011"  , "1"),  // usa
    RpIrMccIddNddSid(312, "1"  , 0    , 0    , "011"  , "1"),  // usa
    RpIrMccIddNddSid(313, "1"  , 0    , 0    , "011"  , "1"),  // usa
    RpIrMccIddNddSid(314, "1"  , 0    , 0    , "011"  , "1"),  // usa
    RpIrMccIddNddSid(315, "1"  , 0    , 0    , "011"  , "1"),  // usa
    RpIrMccIddNddSid(316, "1"  , 0    , 0    , "011"  , "1"),  // usa
    RpIrMccIddNddSid(334, "52" , 24576, 25075, "00"   , "01"),  // mexico
    RpIrMccIddNddSid(334, "52" , 25100, 25124, "00"   , "01"),  // mexico
    RpIrMccIddNddSid(404, "91" , 14464, 14847, "00"   , "0"),  // India
    RpIrMccIddNddSid(425, "972", 8448 , 8479 , "00"   , "0"),  // Israel
    RpIrMccIddNddSid(428, "976", 15520, 15551, "002"  , "0"),  // Mongolia
    RpIrMccIddNddSid(440, "81" , 12288, 13311, "010"  , "0"),  // Japan
    RpIrMccIddNddSid(450, "82" , 2176 , 2303 , "00700", "0"),  // Korea(South)
    RpIrMccIddNddSid(452, "84" , 13312, 13439, "00"   , "0"),  // Vietnam
    RpIrMccIddNddSid(454, "852", 10640, 10655, "001"  , ""),  // Hong Kong
    RpIrMccIddNddSid(455, "853", 11296, 11311, "00"   , "0"),  // Macao/Macau
    RpIrMccIddNddSid(460, "86" , 13568, 14335, "00"   , "0"),  // china
    RpIrMccIddNddSid(460, "86" , 25600, 26111, "00"   , "0"),  // china
    RpIrMccIddNddSid(466, "886", 13504, 13535, "005"  , "0"),  // Taiwan
    RpIrMccIddNddSid(470, "880", 13472, 13503, "00"   , "0"),  // Bangladesh
    RpIrMccIddNddSid(510, "62" , 10496, 10623, "001"  , "0")  // Indonesia
};

const RpIrMccSidLtmOff RpIrMccConvertMappingTables::MCC_SID_LTM_OFF_MAP[] = {
    RpIrMccSidLtmOff(310, 1    , -20, -10), RpIrMccSidLtmOff(404, 1    , 11, 11),
    RpIrMccSidLtmOff(310, 7    , -20, -10), RpIrMccSidLtmOff(404, 7    , 11, 11),

    RpIrMccSidLtmOff(310, 13   , -20, -10), RpIrMccSidLtmOff(454, 13   , 16, 16),

    RpIrMccSidLtmOff(310, 1111 , -20, -10), RpIrMccSidLtmOff(450, 1111 , 18, 18),
    RpIrMccSidLtmOff(310, 1112 , -20, -10), RpIrMccSidLtmOff(450, 1112 , 18, 18),
    RpIrMccSidLtmOff(310, 1113 , -20, -10), RpIrMccSidLtmOff(450, 1113 , 18, 18),
    RpIrMccSidLtmOff(310, 1700 , -20, -10), RpIrMccSidLtmOff(450, 1700 , 18, 18),
    RpIrMccSidLtmOff(310, 2177 , -20, -10), RpIrMccSidLtmOff(450, 2177 , 18, 18),
    RpIrMccSidLtmOff(310, 2179 , -20, -10), RpIrMccSidLtmOff(450, 2179 , 18, 18),
    RpIrMccSidLtmOff(310, 2181 , -20, -10), RpIrMccSidLtmOff(450, 2181 , 18, 18),
    RpIrMccSidLtmOff(310, 2183 , -20, -10), RpIrMccSidLtmOff(450, 2183 , 18, 18),
    RpIrMccSidLtmOff(310, 2185 , -20, -10), RpIrMccSidLtmOff(450, 2185 , 18, 18),
    RpIrMccSidLtmOff(310, 2187 , -20, -10), RpIrMccSidLtmOff(450, 2187 , 18, 18),
    RpIrMccSidLtmOff(310, 2189 , -20, -10), RpIrMccSidLtmOff(450, 2189 , 18, 18),
    RpIrMccSidLtmOff(310, 2191 , -20, -10), RpIrMccSidLtmOff(450, 2191 , 18, 18),
    RpIrMccSidLtmOff(310, 2193 , -20, -10), RpIrMccSidLtmOff(450, 2193 , 18, 18),
    RpIrMccSidLtmOff(310, 2195 , -20, -10), RpIrMccSidLtmOff(450, 2195 , 18, 18),
    RpIrMccSidLtmOff(310, 2197 , -20, -10), RpIrMccSidLtmOff(450, 2197 , 18, 18),
    RpIrMccSidLtmOff(310, 2199 , -20, -10), RpIrMccSidLtmOff(450, 2199 , 18, 18),

    RpIrMccSidLtmOff(310, 2201 , -20, -10), RpIrMccSidLtmOff(450, 2201 , 18, 18),
    RpIrMccSidLtmOff(310, 2203 , -20, -10), RpIrMccSidLtmOff(450, 2203 , 18, 18),
    RpIrMccSidLtmOff(310, 2205 , -20, -10), RpIrMccSidLtmOff(450, 2205 , 18, 18),
    RpIrMccSidLtmOff(310, 2207 , -20, -10), RpIrMccSidLtmOff(450, 2207 , 18, 18),
    RpIrMccSidLtmOff(310, 2209 , -20, -10), RpIrMccSidLtmOff(450, 2209 , 18, 18),
    RpIrMccSidLtmOff(310, 2211 , -20, -10), RpIrMccSidLtmOff(450, 2211 , 18, 18),
    RpIrMccSidLtmOff(310, 2213 , -20, -10), RpIrMccSidLtmOff(450, 2213 , 18, 18),
    RpIrMccSidLtmOff(310, 2215 , -20, -10), RpIrMccSidLtmOff(450, 2215 , 18, 18),
    RpIrMccSidLtmOff(310, 2217 , -20, -10), RpIrMccSidLtmOff(450, 2217 , 18, 18),
    RpIrMccSidLtmOff(310, 2219 , -20, -10), RpIrMccSidLtmOff(450, 2219 , 18, 18),
    RpIrMccSidLtmOff(310, 2221 , -20, -10), RpIrMccSidLtmOff(450, 2221 , 18, 18),
    RpIrMccSidLtmOff(310, 2223 , -20, -10), RpIrMccSidLtmOff(450, 2223 , 18, 18),
    RpIrMccSidLtmOff(310, 2225 , -20, -10), RpIrMccSidLtmOff(450, 2225 , 18, 18),
    RpIrMccSidLtmOff(310, 2227 , -20, -10), RpIrMccSidLtmOff(450, 2227 , 18, 18),
    RpIrMccSidLtmOff(310, 2229 , -20, -10), RpIrMccSidLtmOff(450, 2229 , 18, 18),
    RpIrMccSidLtmOff(310, 2231 , -20, -10), RpIrMccSidLtmOff(450, 2231 , 18, 18),
    RpIrMccSidLtmOff(310, 2233 , -20, -10), RpIrMccSidLtmOff(450, 2233 , 18, 18),
    RpIrMccSidLtmOff(310, 2235 , -20, -10), RpIrMccSidLtmOff(450, 2235 , 18, 18),
    RpIrMccSidLtmOff(310, 2237 , -20, -10), RpIrMccSidLtmOff(450, 2237 , 18, 18),
    RpIrMccSidLtmOff(310, 2239 , -20, -10), RpIrMccSidLtmOff(450, 2239 , 18, 18),
    RpIrMccSidLtmOff(310, 2241 , -20, -10), RpIrMccSidLtmOff(450, 2241 , 18, 18),
    RpIrMccSidLtmOff(310, 2243 , -20, -10), RpIrMccSidLtmOff(450, 2243 , 18, 18),
    RpIrMccSidLtmOff(310, 2301 , -20, -10), RpIrMccSidLtmOff(450, 2301 , 18, 18),
    RpIrMccSidLtmOff(310, 2303 , -20, -10), RpIrMccSidLtmOff(450, 2303 , 18, 18),
    RpIrMccSidLtmOff(310, 2369 , -20, -10), RpIrMccSidLtmOff(450, 2369 , 18, 18),
    RpIrMccSidLtmOff(310, 2370 , -20, -10), RpIrMccSidLtmOff(450, 2370 , 18, 18),
    RpIrMccSidLtmOff(310, 2371 , -20, -10), RpIrMccSidLtmOff(450, 2371 , 18, 18),

    RpIrMccSidLtmOff(450, 2222 , 18 ,  18), RpIrMccSidLtmOff(404, 2222 , 11, 11),

    RpIrMccSidLtmOff(440, 12461, 18 ,  18), RpIrMccSidLtmOff(470, 12461, 12, 12),
    RpIrMccSidLtmOff(440, 12463, 18 ,  18), RpIrMccSidLtmOff(470, 12463, 12, 12),
    RpIrMccSidLtmOff(440, 12464, 18 ,  18), RpIrMccSidLtmOff(470, 12464, 12, 12)
};

const RpIrSidMccMnc RpIrMccConvertMappingTables::SID_MCC_MNC_LIST[] = {
    RpIrSidMccMnc(2, 310010),      // Verizon
    RpIrSidMccMnc(3, 310010),      // Verizon
    RpIrSidMccMnc(4, 310010),      // Verizon
    RpIrSidMccMnc(5, 310730),      // U.S.Cellular
    RpIrSidMccMnc(6, 310010),      // Verizon
    RpIrSidMccMnc(8, 310010),      // Verizon
    RpIrSidMccMnc(10, 310070),     // Cingular
    RpIrSidMccMnc(12, 310010),     // Verizon
    RpIrSidMccMnc(15, 310010),     // Verizon
    RpIrSidMccMnc(17, 310010),     // Verizon
    RpIrSidMccMnc(18, 310010),     // Verizon
    RpIrSidMccMnc(20, 310010),     // Verizon
    RpIrSidMccMnc(21, 310010),     // Verizon
    RpIrSidMccMnc(22, 310010),     // Verizon
    RpIrSidMccMnc(26, 310010),     // Verizon
    RpIrSidMccMnc(28, 310010),     // Verizon
    RpIrSidMccMnc(30, 310010),     // Verizon
    RpIrSidMccMnc(32, 310010),     // Verizon
    RpIrSidMccMnc(40, 310010),     // Verizon
    RpIrSidMccMnc(41, 310010),     // Verizon
    RpIrSidMccMnc(42, 310500),     // Alltel
    RpIrSidMccMnc(44, 310070),     // Cingular
    RpIrSidMccMnc(45, 310500),     // Alltel
    RpIrSidMccMnc(46, 310070),     // Cingular
    RpIrSidMccMnc(48, 310010),     // Verizon
    RpIrSidMccMnc(51, 310010),     // Verizon
    RpIrSidMccMnc(53, 310500),     // Alltel
    RpIrSidMccMnc(54, 310500),     // Alltel
    RpIrSidMccMnc(56, 310010),     // Verizon
    RpIrSidMccMnc(57, 310500),     // Alltel
    RpIrSidMccMnc(58, 310010),     // Verizon
    RpIrSidMccMnc(59, 310010),     // Verizon
    RpIrSidMccMnc(60, 310010),     // Verizon
    RpIrSidMccMnc(64, 310010),     // Verizon
    RpIrSidMccMnc(65, 310010),     // Verizon
    RpIrSidMccMnc(69, 310010),     // Verizon
    RpIrSidMccMnc(73, 310010),     // Verizon
    RpIrSidMccMnc(74, 310500),     // Alltel
    RpIrSidMccMnc(78, 310010),     // Verizon
    RpIrSidMccMnc(79, 310500),     // Alltel
    RpIrSidMccMnc(80, 310010),     // Verizon
    RpIrSidMccMnc(81, 310070),     // Cingular
    RpIrSidMccMnc(83, 310500),     // Alltel
    RpIrSidMccMnc(84, 310500),     // Alltel
    RpIrSidMccMnc(85, 310500),     // Alltel
    RpIrSidMccMnc(81, 310070),     // Cingular
    RpIrSidMccMnc(83, 310500),     // Alltel
    RpIrSidMccMnc(84, 310500),     // Alltel
    RpIrSidMccMnc(85, 310500),     // Alltel
    RpIrSidMccMnc(86, 310010),     // Verizon
    RpIrSidMccMnc(92, 310010),     // Verizon
    RpIrSidMccMnc(93, 310010),     // Verizon
    RpIrSidMccMnc(94, 310010),     // Verizon
    RpIrSidMccMnc(95, 310010),     // Verizon
    RpIrSidMccMnc(96, 310010),     // Verizon
    RpIrSidMccMnc(97, 310500),     // Alltel
    RpIrSidMccMnc(100, 310500),    // Alltel
    RpIrSidMccMnc(106, 310070),    // Cingular
    RpIrSidMccMnc(110, 310010),    // Verizon
    RpIrSidMccMnc(112, 310010),    // Verizon
    RpIrSidMccMnc(113, 310010),    // Verizon
    RpIrSidMccMnc(114, 310500),    // Alltel
    RpIrSidMccMnc(116, 310500),    // Alltel
    RpIrSidMccMnc(119, 310010),    // Verizon
    RpIrSidMccMnc(120, 310500),    // Alltel
    RpIrSidMccMnc(126, 310500),    // Alltel
    RpIrSidMccMnc(127, 310010),    // Verizon
    RpIrSidMccMnc(130, 310500),    // Alltel
    RpIrSidMccMnc(133, 310010),    // Verizon
    RpIrSidMccMnc(137, 310010),    // Verizon
    RpIrSidMccMnc(138, 310070),    // Cingular
    RpIrSidMccMnc(139, 310010),    // Verizon
    RpIrSidMccMnc(140, 310010),    // Verizon
    RpIrSidMccMnc(142, 310500),    // Alltel
    RpIrSidMccMnc(143, 310010),    // Verizon
    RpIrSidMccMnc(144, 310500),    // Alltel
    RpIrSidMccMnc(150, 310010),    // Verizon
    RpIrSidMccMnc(152, 310500),    // Alltel
    RpIrSidMccMnc(154, 310010),    // Verizon
    RpIrSidMccMnc(156, 310500),    // Alltel
    RpIrSidMccMnc(162, 310010),    // Verizon
    RpIrSidMccMnc(163, 310010),    // Verizon
    RpIrSidMccMnc(165, 310010),    // Verizon
    RpIrSidMccMnc(166, 310730),    // U.S.Cellular
    RpIrSidMccMnc(169, 310500),    // Alltel
    RpIrSidMccMnc(179, 310010),    // Verizon
    RpIrSidMccMnc(180, 310010),    // Verizon
    RpIrSidMccMnc(181, 310010),    // Verizon
    RpIrSidMccMnc(182, 310500),    // Alltel
    RpIrSidMccMnc(186, 310010),    // Verizon
    RpIrSidMccMnc(188, 310500),    // Alltel
    RpIrSidMccMnc(189, 310010),    // Verizon
    RpIrSidMccMnc(190, 310010),    // Verizon
    RpIrSidMccMnc(204, 310500),    // Alltel
    RpIrSidMccMnc(205, 310500),    // Alltel
    RpIrSidMccMnc(208, 310500),    // Alltel
    RpIrSidMccMnc(212, 310500),    // Alltel
    RpIrSidMccMnc(214, 310010),    // Verizon
    RpIrSidMccMnc(215, 310070),    // Cingular
    RpIrSidMccMnc(216, 310500),    // Alltel
    RpIrSidMccMnc(220, 310500),    // Alltel
    RpIrSidMccMnc(222, 310010),    // Verizon
    RpIrSidMccMnc(224, 310010),    // Verizon
    RpIrSidMccMnc(226, 310010),    // Verizon
    RpIrSidMccMnc(228, 310010),    // Verizon
    RpIrSidMccMnc(229, 310070),    // Cingular
    RpIrSidMccMnc(234, 310050),    // ACS Wireless
    RpIrSidMccMnc(240, 310500),    // Alltel
    RpIrSidMccMnc(241, 310010),    // Verizon
    RpIrSidMccMnc(244, 310500),    // Alltel
    RpIrSidMccMnc(249, 311130),    // Amarillo Cellular
    RpIrSidMccMnc(250, 310010),    // Verizon
    RpIrSidMccMnc(256, 310500),    // Alltel
    RpIrSidMccMnc(260, 310500),    // Alltel
    RpIrSidMccMnc(262, 310010),    // Verizon
    RpIrSidMccMnc(264, 0),         // Cellular South
    RpIrSidMccMnc(266, 310010),    // Verizon
    RpIrSidMccMnc(272, 310010),    // Verizon
    RpIrSidMccMnc(276, 310010),    // Verizon
    RpIrSidMccMnc(277, 310030),    // CENT USA
    RpIrSidMccMnc(281, 310500),    // Alltel
    RpIrSidMccMnc(284, 310010),    // Verizon
    RpIrSidMccMnc(285, 310500),    // Alltel
    RpIrSidMccMnc(286, 310010),    // Verizon
    RpIrSidMccMnc(294, 310010),    // Verizon
    RpIrSidMccMnc(298, 310730),    // U.S.Cellular
    RpIrSidMccMnc(299, 310010),    // Verizon
    RpIrSidMccMnc(300, 310010),    // Verizon
    RpIrSidMccMnc(302, 310500),    // Alltel
    RpIrSidMccMnc(312, 310500),    // Alltel
    RpIrSidMccMnc(316, 310010),    // Verizon
    RpIrSidMccMnc(318, 310500),    // Alltel
    RpIrSidMccMnc(319, 0),         // Midwest Wireless
    RpIrSidMccMnc(323, 310010),    // Verizon
    RpIrSidMccMnc(324, 0),         // Pioneer/Enid Cellular
    RpIrSidMccMnc(328, 310010),    // Verizon
    RpIrSidMccMnc(329, 310010),    // Verizon
    RpIrSidMccMnc(330, 310010),    // Verizon
    RpIrSidMccMnc(340, 310500),    // Alltel
    RpIrSidMccMnc(341, 0),         // Dobson Cellular Systems
    RpIrSidMccMnc(342, 310500),    // Alltel
    RpIrSidMccMnc(348, 310500),    // Alltel
    RpIrSidMccMnc(349, 310010),    // Verizon
    RpIrSidMccMnc(350, 310500),    // Alltel
    RpIrSidMccMnc(359, 310070),    // Cingular
    RpIrSidMccMnc(361, 310070),    // Cingular Wireless
    RpIrSidMccMnc(362, 42502),     // Cellcom Israel
    RpIrSidMccMnc(364, 310730),    // U.S.Cellular
    RpIrSidMccMnc(368, 310500),    // Alltel
    RpIrSidMccMnc(370, 310500),    // Alltel
    RpIrSidMccMnc(371, 310500),    // Alltel
    RpIrSidMccMnc(374, 310500),    // Alltel
    RpIrSidMccMnc(376, 310500),    // Alltel
    RpIrSidMccMnc(377, 310010),    // Verizon
    RpIrSidMccMnc(384, 310730),    // U.S.Cellular
    RpIrSidMccMnc(386, 310500),    // Alltel
    RpIrSidMccMnc(392, 310500),    // Alltel
    RpIrSidMccMnc(396, 310500),    // Alltel
    RpIrSidMccMnc(400, 310070),    // Cingular
    RpIrSidMccMnc(403, 310030),    // CENT USA
    RpIrSidMccMnc(404, 310010),    // Verizon
    RpIrSidMccMnc(414, 310070),    // Cingular
    RpIrSidMccMnc(416, 310500),    // Alltel
    RpIrSidMccMnc(418, 310500),    // Alltel
    RpIrSidMccMnc(424, 310500),    // Alltel
    RpIrSidMccMnc(426, 310070),    // Cingular
    RpIrSidMccMnc(428, 310010),    // Verizon
    RpIrSidMccMnc(436, 310730),    // U.S.Cellular
    RpIrSidMccMnc(440, 310500),    // Alltel
    RpIrSidMccMnc(443, 310010),    // Verizon
    RpIrSidMccMnc(444, 310500),    // Alltel
    RpIrSidMccMnc(447, 310010),    // Verizon
    RpIrSidMccMnc(448, 310500),    // Alltel
    RpIrSidMccMnc(451, 310500),    // Alltel
    RpIrSidMccMnc(463, 310070),    // Cingular Wireless
    RpIrSidMccMnc(478, 310500),    // Alltel
    RpIrSidMccMnc(486, 310010),    // Verizon
    RpIrSidMccMnc(487, 310500),    // Alltel
    RpIrSidMccMnc(498, 310010),    // Verizon
    RpIrSidMccMnc(502, 310010),    // Verizon
    RpIrSidMccMnc(506, 310010),    // Verizon
    RpIrSidMccMnc(510, 310180),    // West Central Wireless
    RpIrSidMccMnc(511, 310500),    // Alltel
    RpIrSidMccMnc(520, 310500),    // Alltel
    RpIrSidMccMnc(528, 310010),    // Verizon
    RpIrSidMccMnc(529, 310500),    // Alltel
    RpIrSidMccMnc(530, 310010),    // Verizon
    RpIrSidMccMnc(532, 310010),    // Verizon
    RpIrSidMccMnc(539, 310010),    // Verizon
    RpIrSidMccMnc(544, 310500),    // Alltel
    RpIrSidMccMnc(546, 310500),    // Alltel
    RpIrSidMccMnc(550, 310500),    // Alltel
    RpIrSidMccMnc(555, 310500),    // Alltel
    RpIrSidMccMnc(574, 310730),    // U.S.Cellular
    RpIrSidMccMnc(578, 310500),    // Alltel
    RpIrSidMccMnc(579, 310070),    // Cingular Wireless
    RpIrSidMccMnc(580, 310730),    // U.S.Cellular
    RpIrSidMccMnc(587, 310070),    // Cingular Wireless
    RpIrSidMccMnc(607, 310070),    // Cingular
    RpIrSidMccMnc(1015, 310010),   // Verizon
    RpIrSidMccMnc(1018, 310050),   // ACS Wireless
    RpIrSidMccMnc(1022, 310050),   // ACS Wireless
    RpIrSidMccMnc(1024, 310350),   // Mohave Cellular
    RpIrSidMccMnc(1026, 310010),   // Verizon
    RpIrSidMccMnc(1027, 310320),   // Smith Bagley
    RpIrSidMccMnc(1028, 310010),   // Verizon
    RpIrSidMccMnc(1029, 310500),   // Alltel
    RpIrSidMccMnc(1030, 310010),   // Verizon
    RpIrSidMccMnc(1034, 310010),   // Verizon
    RpIrSidMccMnc(1038, 310500),   // Alltel
    RpIrSidMccMnc(1055, 310070),   // Cingular
    RpIrSidMccMnc(1058, 310500),   // Alltel
    RpIrSidMccMnc(1060, 310010),   // Verizon
    RpIrSidMccMnc(1064, 311590),   // Golden State Cellular
    RpIrSidMccMnc(1069, 310500),   // Alltel
    RpIrSidMccMnc(1083, 310010),   // Verizon
    RpIrSidMccMnc(1086, 310010),   // Verizon
    RpIrSidMccMnc(1088, 310010),   // Verizon
    RpIrSidMccMnc(1093, 310500),   // Alltel
    RpIrSidMccMnc(1101, 310500),   // Alltel
    RpIrSidMccMnc(1124, 310500),   // Alltel
    RpIrSidMccMnc(1129, 310010),   // Verizon
    RpIrSidMccMnc(1139, 310010),   // Verizon
    RpIrSidMccMnc(1148, 310500),   // Alltel
    RpIrSidMccMnc(1151, 310010),   // Verizon
    RpIrSidMccMnc(1153, 310010),   // Verizon
    RpIrSidMccMnc(1155, 310070),   // Cingular Wireless
    RpIrSidMccMnc(1164, 310010),   // Verizon
    RpIrSidMccMnc(1165, 310500),   // Alltel
    RpIrSidMccMnc(1173, 310500),   // Alltel
    RpIrSidMccMnc(1174, 310010),   // Verizon
    RpIrSidMccMnc(1180, 310010),   // Verizon
    RpIrSidMccMnc(1189, 310010),   // Verizon
    RpIrSidMccMnc(1192, 310500),   // Alltel
    RpIrSidMccMnc(1200, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1211, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1212, 311430),   // Cellular 29 Plus
    RpIrSidMccMnc(1213, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1215, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1216, 0),        // Midwest Wireless
    RpIrSidMccMnc(1220, 310010),   // Verizon
    RpIrSidMccMnc(1232, 0),        // Midwest Wireless
    RpIrSidMccMnc(1234, 0),        // Midwest Wireless
    RpIrSidMccMnc(1236, 0),        // Midwest Wireless
    RpIrSidMccMnc(1255, 310890),   // Rural Cellular
    RpIrSidMccMnc(1258, 310500),   // Alltel
    RpIrSidMccMnc(1267, 310890),   // Rural Cellular
    RpIrSidMccMnc(1271, 310500),   // Alltel
    RpIrSidMccMnc(1272, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1280, 311440),   // Bluegrass Cellular
    RpIrSidMccMnc(1290, 0),        // Appalachian Wireless
    RpIrSidMccMnc(1317, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1320, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1332, 310500),   // Alltel
    RpIrSidMccMnc(1333, 0),        // Dobson Cellular Systems
    RpIrSidMccMnc(1335, 0),        // Dobson Cellular Systems
    RpIrSidMccMnc(1336, 310500),   // Alltel
    RpIrSidMccMnc(1337, 0),        // Dobson Cellular Systems
    RpIrSidMccMnc(1338, 310500),   // Alltel
    RpIrSidMccMnc(1341, 310030),   // CENT USA
    RpIrSidMccMnc(1345, 310030),   // CENT USA
    RpIrSidMccMnc(1350, 311050),   // Thumb Cellular
    RpIrSidMccMnc(1367, 310500),   // Alltel
    RpIrSidMccMnc(1369, 310500),   // Alltel
    RpIrSidMccMnc(1370, 0),        // ??
    RpIrSidMccMnc(1372, 0),        // Midwest Wireless
    RpIrSidMccMnc(1375, 310500),   // Alltel
    RpIrSidMccMnc(1382, 0),        // Cellular South
    RpIrSidMccMnc(1383, 310500),   // Alltel
    RpIrSidMccMnc(1385, 310500),   // Alltel
    RpIrSidMccMnc(1393, 310500),   // Alltel
    RpIrSidMccMnc(1394, 0),        // Cellular South
    RpIrSidMccMnc(1396, 311420),   // Northwest Missouri Cellular
    RpIrSidMccMnc(1399, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1400, 310500),   // Alltel
    RpIrSidMccMnc(1403, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1406, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1408, 310010),   // Verizon
    RpIrSidMccMnc(1411, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1419, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1423, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1425, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1434, 310010),   // Verizon
    RpIrSidMccMnc(1441, 310500),   // Alltel
    RpIrSidMccMnc(1453, 311030),   // Indigo Wireless
    RpIrSidMccMnc(1465, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1466, 310500),   // Alltel
    RpIrSidMccMnc(1473, 310500),   // Alltel
    RpIrSidMccMnc(1484, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1493, 310500),   // Alltel
    RpIrSidMccMnc(1496, 310100),   // Plateau Wireless
    RpIrSidMccMnc(1499, 310500),   // Alltel
    RpIrSidMccMnc(1500, 310100),   // Plateau Wireless
    RpIrSidMccMnc(1504, 310100),   // Plateau Wireless
    RpIrSidMccMnc(1506, 310010),   // Verizon
    RpIrSidMccMnc(1508, 310010),   // Verizon
    RpIrSidMccMnc(1516, 310010),   // Verizon
    RpIrSidMccMnc(1522, 310130),   // Carolina West Wireless
    RpIrSidMccMnc(1528, 310500),   // Alltel
    RpIrSidMccMnc(1530, 310500),   // Alltel
    RpIrSidMccMnc(1532, 310500),   // Alltel
    RpIrSidMccMnc(1534, 310500),   // Alltel
    RpIrSidMccMnc(1536, 310500),   // Alltel
    RpIrSidMccMnc(1538, 310500),   // Alltel
    RpIrSidMccMnc(1540, 310500),   // Alltel
    RpIrSidMccMnc(1541, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1542, 310500),   // Alltel
    RpIrSidMccMnc(1544, 310500),   // Alltel
    RpIrSidMccMnc(1546, 310500),   // Alltel
    RpIrSidMccMnc(1548, 310010),   // Verizon
    RpIrSidMccMnc(1559, 310010),   // Verizon
    RpIrSidMccMnc(1567, 310010),   // Verizon
    RpIrSidMccMnc(1574, 310500),   // Alltel
    RpIrSidMccMnc(1590, 0),        // Dobson Cellular Systems
    RpIrSidMccMnc(1595, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1598, 311080),   // Pine Cellular
    RpIrSidMccMnc(1607, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1608, 0),        // Ramcell
    RpIrSidMccMnc(1609, 310890),   // Rural Cellular
    RpIrSidMccMnc(1610, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1640, 310500),   // Alltel
    RpIrSidMccMnc(1643, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1645, 0),        // Triton
    RpIrSidMccMnc(1650, 310500),   // Alltel
    RpIrSidMccMnc(1652, 310500),   // Alltel
    RpIrSidMccMnc(1661, 310500),   // Alltel
    RpIrSidMccMnc(1692, 310950),   // XIT
    RpIrSidMccMnc(1696, 310100),   // Plateau Wireless
    RpIrSidMccMnc(1703, 310500),   // Alltel
    RpIrSidMccMnc(1739, 310500),   // Alltel
    RpIrSidMccMnc(1740, 310010),   // Verizon
    RpIrSidMccMnc(1741, 310500),   // Alltel
    RpIrSidMccMnc(1742, 310020),   // UnionTel
    RpIrSidMccMnc(1748, 310010),   // Verizon
    RpIrSidMccMnc(1749, 310010),   // Verizon
    RpIrSidMccMnc(1759, 310500),   // Alltel
    RpIrSidMccMnc(1776, 310010),   // Verizon
    RpIrSidMccMnc(1779, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1784, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1794, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1802, 310730),   // U.S.Cellular
    RpIrSidMccMnc(1812, 0),        // Midwest Wireless
    RpIrSidMccMnc(1818, 310500),   // Alltel
    RpIrSidMccMnc(1823, 310500),   // Alltel
    RpIrSidMccMnc(1825, 310500),   // Alltel
    RpIrSidMccMnc(1826, 310010),   // Verizon
    RpIrSidMccMnc(1827, 310010),   // Verizon
    RpIrSidMccMnc(1828, 310020),   // UnionTel
    RpIrSidMccMnc(1830, 310010),   // Verizon
    RpIrSidMccMnc(1868, 310980),   // AT&amp,T
    RpIrSidMccMnc(1892, 310860),   // Five Star Wireless
    RpIrSidMccMnc(1902, 310500),   // Alltel
    RpIrSidMccMnc(1912, 310010),   // Verizon
    RpIrSidMccMnc(1922, 311150),   // Wilkes Cellular
    RpIrSidMccMnc(1932, 311000),   // Mid-Tex Cellular
    RpIrSidMccMnc(1949, 311040),   // Commnet
    RpIrSidMccMnc(1970, 310540),   // Oklahoma Western Telephone
    RpIrSidMccMnc(1976, 0),        // Brazos Celllular
    RpIrSidMccMnc(1989, 310500),   // Alltel
    RpIrSidMccMnc(1996, 0),        // Cellular South
    RpIrSidMccMnc(2038, 310500),   // Alltel
    RpIrSidMccMnc(2058, 310010),   // Verizon
    RpIrSidMccMnc(2115, 310010),   // Verizon
    RpIrSidMccMnc(2119, 310010),   // Verizon
    RpIrSidMccMnc(2129, 310890),   // Rural Cellular
    RpIrSidMccMnc(2141, 310730),   // U.S.Cellular
    RpIrSidMccMnc(3000, 311040),   // Commnet
    RpIrSidMccMnc(3034, 311040),   // Commnet
    RpIrSidMccMnc(3066, 310010),   // Verizon
    RpIrSidMccMnc(3076, 310020),   // UnionTel
    RpIrSidMccMnc(3226, 310500),   // Alltel
    RpIrSidMccMnc(3462, 0),        // Custer Telephone Cooperative
    RpIrSidMccMnc(4103, 310120),   // Sprint
    RpIrSidMccMnc(4106, 310120),   // Sprint
    RpIrSidMccMnc(4107, 310120),   // Sprint
    RpIrSidMccMnc(4110, 310500),   // Alltel
    RpIrSidMccMnc(4119, 310010),   // Verizon
    RpIrSidMccMnc(4120, 310120),   // Sprint
    RpIrSidMccMnc(4121, 310120),   // Sprint
    RpIrSidMccMnc(4124, 310120),   // Sprint
    RpIrSidMccMnc(4126, 310120),   // Sprint
    RpIrSidMccMnc(4132, 310120),   // Sprint
    RpIrSidMccMnc(4135, 310120),   // Sprint
    RpIrSidMccMnc(4138, 310010),   // Verizon
    RpIrSidMccMnc(4139, 310120),   // Sprint
    RpIrSidMccMnc(4144, 310120),   // Sprint
    RpIrSidMccMnc(4145, 310120),   // Sprint
    RpIrSidMccMnc(4148, 310120),   // Sprint
    RpIrSidMccMnc(4151, 310120),   // Sprint
    RpIrSidMccMnc(4152, 310010),   // Verizon
    RpIrSidMccMnc(4153, 310120),   // Sprint
    RpIrSidMccMnc(4154, 310010),   // Verizon
    RpIrSidMccMnc(4155, 310120),   // Sprint
    RpIrSidMccMnc(4157, 310120),   // Sprint
    RpIrSidMccMnc(4159, 310120),   // Sprint
    RpIrSidMccMnc(4160, 310010),   // Verizon
    RpIrSidMccMnc(4162, 310120),   // Sprint
    RpIrSidMccMnc(4164, 310120),   // Sprint
    RpIrSidMccMnc(4166, 310120),   // Sprint
    RpIrSidMccMnc(4168, 310120),   // Sprint
    RpIrSidMccMnc(4170, 310120),   // Sprint
    RpIrSidMccMnc(4171, 310120),   // Sprint
    RpIrSidMccMnc(4174, 310120),   // Sprint
    RpIrSidMccMnc(4180, 310120),   // Sprint
    RpIrSidMccMnc(4181, 310120),   // Sprint
    RpIrSidMccMnc(4182, 310010),   // Verizon
    RpIrSidMccMnc(4183, 310120),   // Sprint
    RpIrSidMccMnc(4186, 310120),   // Sprint
    RpIrSidMccMnc(4188, 310120),   // Sprint
    RpIrSidMccMnc(4190, 310120),   // Sprint
    RpIrSidMccMnc(4192, 310010),   // Verizon
    RpIrSidMccMnc(4195, 310120),   // Sprint
    RpIrSidMccMnc(4198, 310120),   // Sprint
    RpIrSidMccMnc(4199, 0),        // 3 Rivers Wireless
    RpIrSidMccMnc(4225, 310500),   // Alltel
    RpIrSidMccMnc(4274, 310120),   // Sprint
    RpIrSidMccMnc(4292, 310016),   // Cricket
    RpIrSidMccMnc(4325, 310016),   // Cricket
    RpIrSidMccMnc(4376, 310120),   // Sprint
    RpIrSidMccMnc(4381, 310016),   // Cricket
    RpIrSidMccMnc(4384, 310120),   // Sprint
    RpIrSidMccMnc(4390, 310120),   // Sprint
    RpIrSidMccMnc(4396, 310120),   // Sprint
    RpIrSidMccMnc(4413, 310016),   // Cricket
    RpIrSidMccMnc(4418, 310120),   // Sprint
    RpIrSidMccMnc(4509, 310016),   // Cricket
    RpIrSidMccMnc(4518, 310016),   // Cricket
    RpIrSidMccMnc(4535, 310016),   // Cricket
    RpIrSidMccMnc(4622, 310120),   // Sprint
    RpIrSidMccMnc(4647, 310016),   // Cricket
    RpIrSidMccMnc(4654, 310120),   // Sprint
    RpIrSidMccMnc(4667, 310016),   // Cricket
    RpIrSidMccMnc(4693, 310016),   // Cricket
    RpIrSidMccMnc(4694, 310120),   // Sprint
    RpIrSidMccMnc(4743, 310016),   // Cricket
    RpIrSidMccMnc(4771, 310016),   // Cricket
    RpIrSidMccMnc(4809, 310016),   // Cricket
    RpIrSidMccMnc(4812, 310120),   // Sprint
    RpIrSidMccMnc(4828, 0),        // Qwest
    RpIrSidMccMnc(4857, 310016),   // Cricket
    RpIrSidMccMnc(4923, 310016),   // Cricket
    RpIrSidMccMnc(4928, 0),        // Qwest
    RpIrSidMccMnc(4961, 310016),   // Cricket
    RpIrSidMccMnc(4973, 310016),   // Cricket
    RpIrSidMccMnc(4979, 0),        // ??
    RpIrSidMccMnc(4982, 310120),   // Sprint
    RpIrSidMccMnc(5019, 310016),   // Cricket
    RpIrSidMccMnc(5027, 310016),   // Cricket
    RpIrSidMccMnc(5105, 310016),   // Cricket
    RpIrSidMccMnc(5116, 310120),   // Sprint
    RpIrSidMccMnc(5117, 310016),   // Cricket
    RpIrSidMccMnc(5142, 310120),   // Sprint
    RpIrSidMccMnc(5145, 310016),   // Cricket
    RpIrSidMccMnc(5173, 310016),   // Cricket
    RpIrSidMccMnc(5269, 310050),   // ACS Wireless
    RpIrSidMccMnc(5351, 0),        // Qwest
    RpIrSidMccMnc(5361, 310016),   // Cricket
    RpIrSidMccMnc(5386, 310016),   // Cricket
    RpIrSidMccMnc(5450, 310016),   // Cricket
    RpIrSidMccMnc(5458, 310016),   // Cricket
    RpIrSidMccMnc(5461, 0),        // Qwest
    RpIrSidMccMnc(5510, 310050),   // ACS Wireless
    RpIrSidMccMnc(5513, 310050),   // ACS Wireless
    RpIrSidMccMnc(5540, 310016),   // Cricket
    RpIrSidMccMnc(5586, 310016),   // Cricket
    RpIrSidMccMnc(5618, 310016),   // Cricket
    RpIrSidMccMnc(5660, 0),        // Cellular South
    RpIrSidMccMnc(5667, 310500),   // Alltel
    RpIrSidMccMnc(5682, 310050),   // ACS Wireless
    RpIrSidMccMnc(5685, 310050),   // ACS Wireless
    RpIrSidMccMnc(5756, 310016),   // Cricket
    RpIrSidMccMnc(5908, 310730),   // U.S.Cellular
    RpIrSidMccMnc(5911, 310730),   // U.S.Cellular
    RpIrSidMccMnc(5914, 310016),   // Cricket
    RpIrSidMccMnc(5945, 310016),   // Cricket
    RpIrSidMccMnc(6249, 310016),   // Cricket
    RpIrSidMccMnc(6323, 310016),   // Cricket
    RpIrSidMccMnc(6371, 310016),   // Cricket
    RpIrSidMccMnc(6415, 310016),   // Cricket
    RpIrSidMccMnc(6425, 310016),   // Cricket
    RpIrSidMccMnc(6439, 310016),   // Cricket
    RpIrSidMccMnc(6441, 310016),   // Cricket
    RpIrSidMccMnc(6488, 310730),   // U.S.Cellular
    RpIrSidMccMnc(6490, 311440),   // Bluegrass Cellular
    RpIrSidMccMnc(7316, 311040),   // Commnet
    RpIrSidMccMnc(8097, 0),        // Beuda Digita
    RpIrSidMccMnc(8176, 0),        // Oceanic Digital Jamaica
    RpIrSidMccMnc(8832, 0),        // Codetel Comunicaciones Moviles
    RpIrSidMccMnc(8861, 0),        // MoCelCo
    RpIrSidMccMnc(8863, 311040),   // Commnet
    RpIrSidMccMnc(8950, 311040),   // Commnet
    RpIrSidMccMnc(9246, 311040),   // Commnet
    RpIrSidMccMnc(9332, 311040),   // Commnet
    RpIrSidMccMnc(9562, 311040),   // Commnet
    RpIrSidMccMnc(16384, 30286),   // Telus
    RpIrSidMccMnc(16390, 302702),  // MT&T Mobility
    RpIrSidMccMnc(16408, 0),       // NBTel Mobility
    RpIrSidMccMnc(16410, 302654),  // SaskTel Mobility
    RpIrSidMccMnc(16412, 302654),  // SaskTel Mobility
    RpIrSidMccMnc(16414, 302703),  // NewTel Mobility
    RpIrSidMccMnc(16418, 0),       // ThunderBay Mobility
    RpIrSidMccMnc(16420, 302610),  // Bell Mobility
    RpIrSidMccMnc(16422, 30286),   // Telus
    RpIrSidMccMnc(16428, 302660),  // MTS
    RpIrSidMccMnc(16430, 0),       // IslandTel Mobility
    RpIrSidMccMnc(16462, 0),       // NorTel Mobility
    RpIrSidMccMnc(16472, 0),       // NMI Mobility
    RpIrSidMccMnc(25100, 0),       // Operadora Unefon (Mexico)
    RpIrSidMccMnc(30524, 310500),  // Alltel
    RpIrSidMccMnc(30635, 310500),  // Alltel
    RpIrSidMccMnc(31092, 311350)   // Sagebrush Cellular
//    RpIrSidMccMnc(31092, 0),       // Triangle Communication Systems
};

const int RpIrMccConvertMappingTables::MCC_IDD_NDD_SID_MAP_SIZE = sizeof(RpIrMccConvertMappingTables::MCC_IDD_NDD_SID_MAP)/sizeof(RpIrMccIddNddSid);
const int RpIrMccConvertMappingTables::MCC_SID_LTM_OFF_MAP_SIZE = sizeof(RpIrMccConvertMappingTables::MCC_SID_LTM_OFF_MAP)/sizeof(RpIrMccSidLtmOff);
const int RpIrMccConvertMappingTables::SID_MCC_MNC_LIST_SIZE = sizeof(RpIrMccConvertMappingTables::SID_MCC_MNC_LIST)/sizeof(RpIrSidMccMnc);

