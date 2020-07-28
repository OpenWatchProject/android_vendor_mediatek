/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.wfo.impl;

public class WifiOffloadServiceResource  {
    public final static String [] wfcSupportedMccMncList = {
        //<string-array name="wfcSupportedMccMncList" translatable="false">
        //<!-- Greece -->
        "20201",
        "20202",
        //<!-- France -->
        "20801",
        //<!-- op03 EU -->
        //<!-- Poland -->
        "26003",
        "21403",
        "21409",
        "21411",
        //<!-- op05 mcc mnc list -->
        "23001",
        //<!-- A1/Austria OP188 -->
        "23201",
        //<!-- Connect Austria One/Austria OP11 -->
        "23205",
        //<!-- 3 AT/Austria OP11 -->
        "23210",
        //<!-- Poland -->
        "26002",
        "26003",
        "26034",
        //<!-- op06 EU mcc mnc list -->
        //<!-- Albanian -->
        "27602",
        //<!-- Cyprus -->
        "28001",
        //<!-- Czech -->
        "23003",
        "23099",
        //<!-- Faroe Islands -->
        "28802",
        //<!-- Germany -->
        "26202",
        "26203",
        "26204",
        "26209",
        "26277",
        //<!-- Greece -->
        "20205",
        //<!-- England -->
        "23403",
        "23415",
        "23591",
        //<!-- Magyar -->
        "21670",
        //<!-- IceLand -->
        "27402",
        "27403",
        //<!-- Ireland -->
        "27201",
        //<!-- Italy -->
        "22210",
        //<!-- Netherlands -->
        "20404",
        "20416",
        "20420",
        "20402",
        //<!-- Portugal -->
        "26801",
        "26803",
        "26899",
        //<!-- Romania -->
        "22601",
        "22605",
        "22610",
        //<!-- Spain -->
        "21401",
        "21406",
        //<!-- Turkey mcc mnc list -->
        //<!-- Turkcell -->
        "28601",
        //<!-- Vodafone -->
        "28602",
        //<!-- op07 mcc mnc list -->
        //<!-- US -->
        "310026",
        "310030",
        "310070",
        "310090",
        "310150",
        "310170",
        "310210",
        "310260",
        "310330",
        "310280",
        "310380",
        "310410",
        "310490",
        "310560",
        "310580",
        "310660",
        "310680",
        "310980",
        "310990",
        "311180",
        //<!-- op08 mcc mnc list -->
        //<!-- Germany -->
        "26201",
        "20206",
        //<!-- op11 mcc mnc list -->
        "24002",
        "23806",
        //<!-- op16 mcc mnc list -->
        "23430",
        "23431",
        "23432",
        "23501",
        "23502",
        //<!-- op18 mcc mnc list -->
        "40409",
        "40436",
        "40452",
        "40467",
        "40483",
        "40485",
        "40501",
        "40503",
        "40504",
        "40509",
        "40510",
        "40513",
        "405840",
        //<!-- op128 mcc mnc list -->
        "24403",
        "24412",
        "24409",
        "24499",
        //<!-- op131 mcc mnc list -->
        "52088",
        "52099",
        //<!-- op1001 mcc mnc list -->
        "262080",
        //<!-- op1003 mcc mnc list -->
        "24099",
        //<!-- O2 Germany mcc mnc list -->
        "26207",
        "26208",
        "26211",
        //<!-- RJIL mcc mnc list -->
        "40409",
        "40418",
        "40436",
        "40450",
        "40452",
        "40467",
        "40483",
        "40485",
        "40501",
        "40503",
        "40504",
        "40505",
        "40506",
        "40507",
        "40508",
        "40509",
        "40510",
        "40511",
        "40512",
        "40513",
        "40514",
        "40515",
        "40516",
        "40517",
        "40518",
        "40519",
        "40520",
        "40521",
        "40522",
        "40523",
        "405854",
        "405855",
        "405856",
        "405857",
        "405858",
        "405859",
        "405860",
        "405861",
        "405862",
        "405863",
        "405864",
        "405865",
        "405866",
        "405867",
        "405868",
        "405869",
        "405870",
        "405871",
        "405872",
        "405873",
        "405874",
        //<!-- Airtel mcc mnc list -->
        "40410",
        "40431",
        "40440",
        "40445",
        "40449",
        "40551",
        "40552",
        "40553",
        "40554",
        "40555",
        "40556",
        "40490",
        "40492",
        "40493",
        "40494",
        "40495",
        "40496",
        "40497",
        "40498",
        "40402",
        "40403",
        //<!-- Etisalat UAE -->
        "42402",
        //<!-- Vodacom South Africa -->
        "65501",
        //<!-- op19 mcc mnc list -->
        "50501",
        "50571",
        "50572",
        //<!-- op103 mcc mnc list -->
        "52501",
        //<!-- op127 mcc mnc list -->
        "25002",
        //<!-- op128 mcc mnc list -->
        "24403",
        "24412",
        //<!-- op131 mcc mnc list -->
        "52088",
        "52099",
        //<!-- op135 mcc mnc list -->
        "25001",
        //<!-- op139 mcc mnc list -->
        //<!-- France -->
        "20820",
        "20888",
        //<!-- op153 mcc mnc list -->
        "50503",
        "50506",
        //<!-- op154 mcc mnc list -->
        "24001",
        "24008",
        "24005",
        "24202",
        "24205",
        //<!-- Finland -->
        "24412",
        "24413",
        //<!-- Telia/Finland -->
        "24436",
        "24491",
        //<!-- op156 mcc mnc list -->
        //<!-- Denmark mcc mnc list -->
        "23802",
        "23877",
        //<!-- Norway mcc mnc list -->
        "24201",
        //<!-- op178 mcc mnc list -->
        //<!-- Cambodia mcc mnc list -->
        "45606",
        //<!-- op161 mcc mnc list -->
        //<!-- Poland mcc mnc list -->
        "26006",
        //<!-- op185 mcc mnc list -->
        //<!-- New Zealand mcc mnc list -->
        "53024",
        //<!-- op1001 mcc mnc list -->
        "262080",
        //<!-- op1003 mcc mnc list -->
        "24099",
        //<!-- O2 Germany mcc mnc list -->
        "26207",
        "26208",
        "26211",
        //<!-- APTG mcc mnc list -->
        "46605",
        //<!-- CHT mcc mnc list -->
        "46692",
        //<!-- Claro Brazil mcc mnc list -->
        "72405",
        "72412",
        "72438",
        //<!-- Brazil mcc mnc list -->
        "72401",
        "72419",
        "72411",
        "72410",
        "72406",
        "72423",
        //<!-- truemove mcc mnc list -->
        "52088",
        "52099",
        "52004",
        "52000",
        //<!-- Elisa FI mcc mnc list -->
        "24405",
        "24406",
        "24421",
        //<!-- APTG mcc mnc list -->
        "46605",
        //<!-- op107 SFR France mcc mnc list -->
        "20810",
        "20811",
        "20813",
        //<!-- Swisscom mcc mnc list -->
        "22801",
        "22802",
        "29501",
        //<!-- Vodafone turkey -->
        "28602",
        //<!-- Movistar Argentina -->
        "722070",
        //<!-- O2 Germany mcc mnc list -->
        "26207",
        "26208",
        "26211",
        //<!--Saudia Arabia - Zain-->
        "42004",
        //<!--Saudia Arabia - STC-->
        "42001",
        //<!-- Orange -->
        "60201",
        //<!-- Etisalat Egypt-->
        "60203",
        //<!-- Brazil mcc mnc list -->
        "72401",
        "72419",
        "72411",
        "72410",
        "72406",
        "72423",
        //<!--TIM Brazil mcc mnc list -->
        "72402",
        "72403",
        "72404",
        "72408",
        //<!-- GTE mcc mnc list -->
        "23453",
        "23454",
        //<!-- H3G UK -->
        "23420",
        "23430",
        //<!-- DTAG -->
        "26278",
        //<!-- DTAC -->
        "52005",
        "52018",
        //<!-- AIS -->
        "52001",
        "52003",
        //<!-- MTN South Africa-->
        "65510",
        //<!-- Movistar Peru-->
        "71606",
        //<!-- Claro Peru-->
        "71610",
        //<!-- Entel Peru-->
        "71617",
        //<!-- chile muvistar-->
        "73002",
        //<!-- Movistar Argentina -->
        "72207",
        "72234",
        //<!-- Movistar Colombia mcc mnc list -->
        "732123",
        //<!-- Claro Argentina -->
        "722310",
        "722320",
        "722330",
        "722070",
        //<!-- chile muvistar-->
        "73002",
        //<!-- chile WOM-->
        "73009",
        //<!-- chile Entel-->
        "73001",
        //<!-- chile Claro-->
        "73003",
        //<!-- Comcel -->
        "732101",
        //<!-- op149(cmhk) mcc mnc list -->
        "45412",
        //<!-- Denmark mcc mnc list -->
        "23802",
        "23877",
        //<!-- op162(Canada Freedom) mcc mnc list -->
        "302490",
        //<!-- FET mcc mnc list -->
        "46601",
        //<!-- Australia mcc mnc list -->
        "50502",
        //<!-- op177 Telecom Egypt mcc mnc list -->
        "60204",
        //<!-- Cell C-->
        "65507",
        //<!-- Srilanka- Dialog mcc mnc list -->
        "41302",
        //<!-- Telcel MX -->
        "334020",
        //<!-- Swaziland-->
        "65302",
        //<!-- Indonesia PT Smartfren Telecom -->
        "51009",
        "51028",
        //<!-- TelKom South Africa mcc mnc list -->
        "65502",
        //<!-- Orange Belgium -->
        "20610"
    };
    //</string-array>
    //<!-- WFC Supported ICC ID list -->
    //<string-array name="wfcSupportedIccIdList" translatable="false">
        //<!-- APTG Allow ICC ID list -->
    public final static String [] wfcSupportedIccIdList = {
        "8988605",
    };
    //</string-array>
};
