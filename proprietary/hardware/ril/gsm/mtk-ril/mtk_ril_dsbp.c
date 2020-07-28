/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include <stdbool.h>
#include <cutils/properties.h>
#include <libmtkrilutils.h>
#include <telephony/mtk_ril.h>
#include <atchannels.h>
#include <at_tok.h>
#include <ril_callbacks.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RIL-DSBP"


#ifdef MTK_RIL
static const RIL_SOCKET_ID s_pollSimId = RIL_SOCKET_1;
static const RIL_SOCKET_ID s_pollSimId2 = RIL_SOCKET_2;
static const RIL_SOCKET_ID s_pollSimId3 = RIL_SOCKET_3;
static const RIL_SOCKET_ID s_pollSimId4 = RIL_SOCKET_4;
#endif /* MTK_RIL */

extern SIM_Status getSIMStatus(RIL_SOCKET_ID rid);

/**
 * ALPS02301695. Dynamic-SBP: Run-time configure SBP-ID according to SIM MCC/MNC.
 */

/*  {mccMnc-start, mccMnc-end, SBP-ID}  */

typedef struct SBPTABLE {
    int mccMnc_range_start;
    int mccMnc_range_end;
    int sbpId;
} SbpTable;

int multi_ims = 0;
int multi_ps = 0;

static SbpTable s_mtk_sbp_table[] = {
    // OP01
    {46000, 46000, 1},    // CMCC - China
    {46002, 46002, 1},    // China
    {46004, 46004, 1},    // China
    {46007, 46007, 1},    // China
    {46008, 46008, 1},    // China
    {46070, 46070, 1},    // China
    {46096, 46096, 1},    // China Test SIM

    // OP02
    {46001, 46001, 2},    // CU - China
    {46006, 46006, 2},    // China

    // OP03
    {20610, 20610, 3},    // Orange - Belgium
    {20801, 20802, 3},    // Orange - France
    {20829, 20829, 3},    // France
    {21403, 21403, 3},    // Spain
    {21409, 21409, 3},    // Spain
    {21411, 21411, 3},    // Spain
    {22610, 22610, 3},    // Romania
    {26003, 26003, 3},    // Poland
    {23101, 23101, 3},    // Slovakia
    {34001, 34001, 3},    // French Guiana
    {60201, 60201, 3},    // Egypt

    // OP05
    {20201, 20202, 5},    // DTAG - Greece
    {20416, 20416, 5},    // Netherlands
    {20420, 20420, 5},    // Netherlands
    {21630, 21630, 5},    // TMOEU - Hungary
    {21901, 21901, 5},    // Telekom - Croatia
    {23001, 23001, 5},    // Czech
    {23102, 23102, 5},    // Telekom - Slovakia
    {23104, 23104, 5},    // Telekom - Slovakia
    {23203, 23203, 5},    // TMOEU
    {23207, 23207, 5},    // TMOEU
    {26002, 26002, 5},    // Poland
    {26034, 26034, 5},    // Poland
    {26201, 26201, 5},    // Germany
    {26206, 26206, 5},    // Germany

    // OP06
    {20205, 20205, 6},    // Vodafone - Greece
    {20404, 20404, 6},    // NL
    {21401, 21401, 6},    // Spain
    {21406, 21406, 6},    // Spain
    {22206, 22206, 6},    // Italy
    {22210, 22210, 6},    // Italy
    {23003, 23003, 6},    // CZ
    {23415, 23415, 6},    // United Kingdom
    {23427, 23427, 6},    // United Kingdom
    {23491, 23491, 6},    // United Kingdom
    {26202, 26202, 6},    // Germany
    {26204, 26204, 6},    // Germany
    {26209, 26209, 6},    // Germany
    {26801, 26801, 6},    // Portugal
    {27201, 27201, 6},    // Vodafone Ireland
    {28602, 28602, 6},    // Turkey
    {53001, 53001, 6},    // NewZealand
    {530001, 530001, 6},    // NewZealand
    {60202, 60202, 6},    // Egypt
    {64505, 64505, 6},    // Zambia
    {65501, 65501, 6},    // ZA

    // OP07
    {310030, 310030, 7},    // AT&T - United States
    {310070, 310070, 7},    // United States
    {310090, 310090, 7},    // United States
    {310680, 310680, 7},    // United States
    {310170, 310170, 7},    // United States
    {310280, 310280, 7},    // United States
    {310380, 310380, 7},    // United States
    {310410, 310410, 7},    // United States
    {310560, 310560, 7},    // United States
    {310950, 310950, 7},    // United States
    {311180, 311180, 7},    // United States
    {312670, 312670, 7},    // United States
    {313100, 313100, 7},    // United States
    {334050, 334050, 7},    // Maxico
    {334090, 334090, 7},    // Maxico

    // OP08
    {31031, 31032, 8},      // United States
    {310160, 310160, 8},    // United States
    {310200, 310200, 8},    // United States
    {310210, 310210, 8},    // United States
    {310220, 310220, 8},    // United States
    {310230, 310230, 8},    // United States
    {310240, 310240, 8},    // United States
    {310250, 310250, 8},    // United States
    {310260, 310260, 8},    // United States
    {310270, 310270, 8},    // United States
    {310300, 310300, 8},    // United States
    {310310, 310310, 8},    // United States
    {310330, 310330, 8},    // United States
    {310490, 310490, 8},    // United States
    {310580, 310580, 8},    // United States
    {310660, 310660, 8},    // United States
    {310800, 310800, 8},    // United States

    // OP09
    {45502, 45502, 9},    // Macao China
    {45507, 45507, 9},    // Macao China
    {46003, 46003, 9},    // China
    {46005, 46005, 9},    // China
    {46011, 46011, 9},    // China

    // OP10

    // OP11
    {23205, 23205, 11},    // 3 Austria
    {23210, 23210, 11},    // 3 Austria
    {23420, 23420, 11},    // United Kingdom
    {23594, 23594, 11},    // United Kingdom
    {24002, 24002, 11},    // 3 mobile - Sweden
    {23806, 23806, 11},    // H3G - Denmark

    // OP12
    {31004, 31004, 12},    //United States
    {31010, 31010, 12},    //United States
    {31012, 31013, 12},    //United States
    {310590, 310590, 12},    //United States
    {310890, 310890, 12},    //United States
    {310910, 310910, 12},    //United States

    {311110, 311110, 12},    //United States
    {311270, 311289, 12},    //United States
    {311390, 311390, 12},    //United States
    {311480, 311489, 12},    //United States

    // OP15
    {23208, 23208, 15},    //Austria
    {23410, 23410, 15},    //United Kingdom
    {26203, 26203, 15},    //Germany
    {26207, 26207, 15},    //DE
    {26208, 26208, 15},    //Telefonica
    {26277, 26277, 15},    //Germany

    // OP16
    {23430, 23434, 16},    // United Kingdom
    {23502, 23502, 16},    // United Kingdom

    // OP17
    {310370, 310370, 17},  // Guam
    {310470, 310470, 17},  // Guam
    {44010, 44010, 17},    // Japan

    // OP18
    {40587, 40587, 18},    //India
    {405840, 405840, 18},   //India
    {405854, 405874, 18},   //India

    // OP19
    {50501, 50501, 19},    //Australia
    {50511, 50511, 19},    //Australia
    {50571, 50572, 19},    //Australia

    {53004, 53004, 19},    //New Zealand

    // OP20
    {310120, 310120, 20},    //United States
    {311490, 311490, 20},    //United States

    // OP50
    {44000, 44000, 50},    //Japan
    {44020, 44020, 50},    //Japan

    // OP100
    {45400, 45400, 100},   //Hongkong China
    {45402, 45402, 100},   //Hongkong China
    {45410, 45410, 100},   //Hongkong China
    {45418, 45418, 100},   //Hongkong China

    // OP101
    {45416, 45416, 101},   //Hongkong China
    {45419, 45420, 101},   //Hongkong China
    {45429, 45429, 101},   //Hongkong China

    // OP102
    {45406, 45406, 102},   //Hongkong China

    // OP103
    {52501, 52502, 103},   //Singapore
    {52507, 52507, 103},   //Singapore

    // OP104
    {52505, 52506, 104},   //Singapore

    // OP105
    {74402, 74402, 105},   //Paraguay

    // OP106
    {45403, 45405, 106},   //Hongkong China
    {45414, 45414, 106},   //Hongkong China

    // OP107
    {20809, 20811, 107},   //France
    {64710, 64710, 107},   //Reunion

    // OP108
    {46697, 46697, 108},   //Taiwan
    {46699, 46699, 108},   //Taiwan

    // OP109
    {46692, 46692, 109},   //Taiwan

    // OP110
    {46601, 46603, 110},   //Taiwan
    {46606, 46607, 110},   //Taiwan
    {46688, 46688, 110},   //Taiwan

    // OP111
    {40401, 40401, 111},   // Vodafone India
    {40405, 40405, 111},   // India
    {40411, 40411, 111},   // India
    {40413, 40413, 111},   // India
    {40415, 40415, 111},   // India
    {40420, 40420, 111},   // India
    {40427, 40427, 111},   // India
    {40430, 40430, 111},   // India
    {40443, 40443, 111},   // India
    {40446, 40446, 111},   // India
    {40460, 40460, 111},   // India
    {40484, 40484, 111},   // India
    {40486, 40486, 111},   // India
    {40488, 40488, 111},   // India
    {40566, 40567, 111},   // India
    {405750, 405756, 111}, // India

    // OP112
    {33402, 33402, 112},   // TelCel Maxico
    {334020, 334020, 112}, // TelCel Maxico
    {34003, 34003, 112},   // French Antilles
    {36251, 36251, 112},   // Former Netherlands Antilles
    {73404, 73404, 112},   // Venezuela

    // OP113
    {25099, 25099, 113},   //Venezuela

    // OP114
    {45002, 45002, 114},   //South Korea
    {45004, 45004, 114},   //South Korea

    // OP115
    {45005, 45005, 115},   //South Korea

    // OP116
    {45006, 45006, 116},   //South Korea

    // OP117
    {51009, 51009, 117},   //Indonesia
    {51028, 51028, 117},   //Indonesia

    // OP118
    {502152, 502152, 118},   //Malaysia

    // OP119
    {37203, 37203, 119},   //Haiti
    {62140, 62140, 119},   //Natcom - Nigeria

    // OP120
    {71610, 71610, 120},    // Claro - Peru
    {722310, 722310, 120},  // Argentina
    {722320, 722320, 120},  // Argentina
    {722330, 722330, 120},  // Argentina
    {72405, 72405, 120},    // Brazil
    {73003, 73003, 120},    // Chile (TS.25)
    {730003, 730003, 120},  // Chile (operator confirmed)
    {732101, 732101, 120},  // Colombia

    // OP121
    {302610, 302610, 121},   //Canada
    {302640, 302640, 121},   //Canada
    {302690, 302690, 121},   //Canada

    // OP122
    {52001, 52001, 122},   //Thailand
    {52003, 52003, 122},   //Thailand
    {52023, 52023, 122},   //Thailand

    // OP124
    {46605, 46605, 124},   //Taiwan

    // OP125
    {52005, 52005, 125},   //Thailand

    // OP126
    {28603, 28603, 126},   //Turkey

    // OP127
    {25002, 25002, 127},   // Megafone - Russian
    {25011, 25011, 127},   // Russian
    {25030, 25030, 127},   // Russian

    // OP128
    {24412, 24412, 128},   //Finland
    {24413, 24413, 128},   //Finland

    // OP129
    {44007, 44008, 129},   // Japan
    {44050, 44056, 129},   // Japan
    {44070, 44079, 129},   // Japan
    {44088, 44089, 129},   // Japan
    {44170, 44170, 129},   // Japan

    // OP130
    {22201, 22201, 130},   // Italy
    {72402, 72404, 130},   // Tim - Brazil

    // OP131
    {52000, 52000, 131},   // TH
    {52004, 52004, 131},   // TH

    // OP132
    {33403, 33403, 132},   // Movistar - Maxico
    {334030, 334030, 132}, // Maxico
    {71606, 71606, 132},   // Peru
    {72207, 72207, 132},   // Argentina
    {72406, 72406, 132},   // Vivo-Brazil
    {72410, 72411, 132},   // Vivo-Brazil
    {72423, 72423, 132},   // Vivo-Brazil
    {73002, 73002, 132},   // Chile
    {732123, 732123, 132}, // Columbia
    {74000, 74000, 132},   // Ecuador
    {74807, 74807, 132},   // Uruguay

    // OP133
    {42403, 42403, 133},   // DU

    // OP134
    {24421, 24421, 134},   // Elisa-Finland
    {24405, 24405, 134},   // Elisa-Finland

    // OP135
    {25001, 25001, 135},   // MTS

    // OP136
    {71617, 71617, 136},   // Entel - Peru
    {73001, 73001, 136},   // Entel - Chile

    // OP137
    {20402, 20402, 137},   // Tele2 - Netherlands
    {24007, 24007, 137},   // Sweden
    {25020, 25020, 137},   // Russian
    {28808, 28808, 137},   // Tele2-Sweden

    // OP139
    {20820, 20820, 139},   // Bouygues - France
    {20888, 20888, 139},   // France

    // OP140
    {65510, 65510, 140},   // MTN South Africa

    // OP141
    {65507, 65507, 141},   // Cell C South Africa

    // OP143
    {28601, 28601, 143},   // Turkcell

    // OP144
    {62127, 62127, 144},   // Smile - Nigeria
    {64011, 64011, 144},   // Tanzania
    {64133, 64133, 144},   // Uganda

    // OP145
    {310150, 310150, 145},   // Cricket-USA-HTC/TCL

    // OP146
    {42402, 42402, 146},   // Etisalat - UAE
    {60203, 60203, 146},   // Egypt

    // OP147
    {40402, 40403, 147},   // Airtel - India
    {40410, 40410, 147},   // India
    {40416, 40416, 147},   // India
    {40431, 40431, 147},   // India
    {40440, 40440, 147},   // India
    {40445, 40445, 147},   // India
    {40449, 40449, 147},   // India
    {40470, 40470, 147},   // India
    {40490, 40490, 147},   // India
    {40492, 40498, 147},   // India
    {40551, 40556, 147},   // India

    // OP148
    {45611, 45611, 148},   // Seatel - Cambodia

    // OP149
    {45412, 45412, 149},   // CMHK - Sony

    // OP150
    {22801, 22801, 150},   // Swisscom Schweiz AG
    {29501, 29501, 150},   // Swisscom Schweiz AG

    // OP151
    {52503, 52503, 151},   // M1 Singapore

    // OP152
    {50502, 50502, 152},   // Optus Australia

    // OP153
    {50503, 50503, 153},   // VHA Australia
    {50506, 50506, 153},   // VHA Australia

    // OP154
    {24001, 24001, 154},   // Telia Sweden
    {24005, 24005, 154},   // Telia Sweden
    {24202, 24202, 154},   // Telia norway
    {24205, 24205, 154},   // Telia norway
    {24491, 24491, 154},   // Telia Finland
    {24601, 24601, 154},   // Lithuania/Omnitel(Telia)
    {24801, 24801, 154},   // Estonia/Telia(EMT)

    // OP155
    {21603, 21603, 155},   // Digi-Hungry-Romania
    {22605, 22605, 155},   // Digi-Hungry-Romania
    {50216, 50216, 155},   // Digi-Malaysia

    // OP156
    {21601, 21601, 156},   // Telenor - Hungary
    {23802, 23802, 156},   // Denmark
    {23877, 23877, 156},   // Denmark
    {24008, 24008, 156},   // Norway
    {24201, 24201, 156},   // Norway
    {28405, 28405, 156},   // Bulgaria
    {41006, 41006, 156},   // Telenor - Pakistan
    {41406, 41406, 156},   // Telenor - Myanmar

    // OP157
    {302220, 302221, 157},   // Telus - Canada

    // OP158
    {42004, 42004, 158},   // Zain - Saudi Arabia

    // OP159
    {42001, 42001, 159},   // STC - Saudi Arabia

    // OP160
    {502153, 502153, 160},   // Web - Malaysia

    // OP161
    {26006, 26006, 161},   // Poland - Sony
    {26098, 26098, 161},   // Poland - Sony

    // OP162
    {302490, 302490, 162},   // Freedom - Canada

    // OP163
    {41302, 41302, 163},   // Dialog - Sri-Lanka

    // OP165
    {22802, 22802, 165},   // Sunrise - Switzerland

    // OP171
    {73009, 73009, 171},   // WOM - Chile

    // OP175
    {23801, 23801, 175},   // Denmark TDC - Sony
    {23810, 23810, 175},   // Denmark TDC - Sony

    // OP176
    {46689, 46689, 176},   // TStar - Taiwan

    // OP177
    {60204, 60204, 177},   // Telecom - Egypt

    // OP178
    {45606, 45606, 178},   // Smart - Cambodia
    {51503, 51503, 178},   // Philippines

    // OP179
    {41866, 41866, 179},   // Allai Newroz Telecom - Iraq

    // OP180
    {65302, 65302, 180},   // Swazi Mobile - Swaziland

    // OP181
    {65502, 65502, 181},   // TelKom - South Africa

    // OP182
    {20601, 20601, 182},   // Belgium/Proximus

    // OP183
    {72234, 72234, 183},   // Personal - Argentina

    // OP184
    {45204, 45204, 184},   // Viettel - Vietnum

    // OP185
    {53024, 53024, 185},   // 2degrees - New Zealand

    // OP186
    {40404, 40404, 186},   // Idea - India
    {40407, 40407, 186},   // Idea - India
    {40412, 40412, 186},   // Idea - India
    {40414, 40414, 186},   // Idea - India
    {40419, 40419, 186},   // Idea - India
    {40422, 40422, 186},   // Idea - India
    {40424, 40424, 186},   // Idea - India
    {40444, 40444, 186},   // Idea - India
    {40456, 40456, 186},   // Idea - India
    {40478, 40478, 186},   // Idea - India
    {40482, 40482, 186},   // Idea - India
    {40487, 40487, 186},   // Idea - India
    {40489, 40489, 186},   // Idea - India
    {40570, 40570, 186},   // Idea - India
    {405799, 405799, 186},   // Idea - India
    {405845, 405846, 186},   // Idea - India
    {405848, 405850, 186},   // Idea - India
    {405852, 405853, 186},   // Idea - India

    // OP187
    {63902, 63902, 187},   // Safaricom - Kenya

    // OP188
    {23201, 23201, 188},   // A1 - Austria

    // OP189
    {50218, 50218, 189},   // U Mobile - Malaysia

    // OP190
    {63907, 63907, 190},   // Telkom - Kenya

    // OP195
    {23002, 23002, 195},   // O2 - Czech

    // OP199
    {28403, 28403, 199},   // Vivacom - Bulgaria

      // OP200
    {63910, 63910, 200},   // JTL - kenya

    // OP201
    {20469, 20469, 201},  // KPN Netherlands
    {20408, 20408, 201},  // KPN Netherlands

    // OP203
    {26803, 26803, 203},   // NOS - Portugal
    {26899, 26899, 203},   // NOS - Portugal

    // OP204
    {302370, 302370, 204},  // Rogers Canada
    {302720, 302720, 204},  // Rogers Canada

    // OP205
    {334140, 334140, 205},  // ALTAN Mexico

    // OP207
    {45501, 45501, 207},  // CTM
    {45504, 45504, 207},  // CTM

    // OP208
    {45503, 45503, 208},  // Macau
    {45505, 45505, 208},  // Macau

    // OP209
    {732130, 732130, 209},  // Avantel Colombia

    // OP211
    {71401, 71401, 211},  // C&W Panama
    {714010, 714010, 211},  // C&W Panama

    // OP1001
    {262080, 262080, 1001},   //Ericsson IODT

    // OP1002
    {23112, 23112, 1002},   // Nokia IODT

    // OP1003
    {24099, 24099, 1003},    //MTK_HQLAB-Ericsson

    // OP1004
    {46020, 46020, 1004},    //Huawei IODT
};

extern int readMccMnc(RIL_SOCKET_ID rid, char *mccmnc);

static const struct timeval TIMEVAL_MccMnc = { 0, 100000 };

typedef enum {
    DSBP_PHASE_DONE = 0,    // Default Value
    DSBP_PHASE_DETECT_SBP_ID = 1,
    DSBP_PHASE_IMS_DEREG = 2,
    DSBP_PHASE_SETUP_SBP = 3,
} DSBP_EXECUTION_PHASE;

static int E_RETRY = -1;
static int E_TERMINATE = -2;
static int S_OK = 0;
static int S_STOP = 1;

typedef struct {
    DSBP_EXECUTION_PHASE executionPhase;
    char sbpId[10];
} ExecutionContext;
static ExecutionContext sExecutionContext[SIM_COUNT];

static void initialDynamicSBPExecutionContext(RIL_SOCKET_ID rid)
{
    static int sIsInitialized[SIM_COUNT] = {0};

    if (!sIsInitialized[rid]) {
        RLOGD("initial Dynamic-SBP ExecutionContext-%d", rid);
        memset(&sExecutionContext[rid], 0, sizeof(ExecutionContext));
        sIsInitialized[rid] = 1;
    }
}

static int getSBPIdFromMccMnc(int mccMnc) {
    int i;
    int len = sizeof(s_mtk_sbp_table) / sizeof(s_mtk_sbp_table[0]);
    for(i = 0; i < len; i++) {
        if(s_mtk_sbp_table[i].mccMnc_range_start <= mccMnc
                && mccMnc <= s_mtk_sbp_table[i].mccMnc_range_end) {
            return s_mtk_sbp_table[i].sbpId;
        }
    }

    return 0;
}

static bool isValidRid(RIL_SOCKET_ID rid) {
    bool valid = true;

    if (rid >= SIM_COUNT) {
        RLOGD("executeDynamicSBP-invalid rid: %d", rid);
        valid = false;
    }

    /*
     * Multi IMS support for gemini
     */
    if (multi_ps == 1) {
        return valid;
    }

    /*
     * Check the main protocol rid before execution becuase
     * it may changed anytime!
     */
    if(getMainProtocolRid() != rid) {
        RLOGD("executeDynamicSBP-%d: main protocol rid %d changed, stop execution...",
                rid, getMainProtocolRid());
        valid = false;
    }
    return valid;
}

/**
 * Here we check for secondary proptocol when MPS is enabled without M-IMS,
 * In this case, we only set system property, while not sending any AT cmd to modem
 */
static bool isSecondaryProtocolWithMps(RIL_SOCKET_ID rid) {
    bool ret = false;
    // directly by pass to set property, as per discussion we have decided
    // to set property but not send ESBP cmd for non main protocol when only
    // MPS is enabled.
    if ((multi_ps == 1 && multi_ims <= 1) &&
            rid != getMainProtocolRid()) {
        RLOGD("DynamicSBP-%d: isSecondaryProtocolWithMps: true", rid);
        ret = true;
    }
    return ret;
}
static void setRadioSbpProperty(RIL_SOCKET_ID rid) {
    if (rid >= SIM_COUNT) {
        RLOGW("setRadioSbpProperty-invalid rid: %d", rid);
        return;
    }
    if (multi_ps == 1) {
        if (RIL_SOCKET_1 == rid) {
            property_set("persist.vendor.radio.sim.opid", sExecutionContext[rid].sbpId);
        } else if (RIL_SOCKET_2 == rid) {
            property_set("persist.vendor.radio.sim.opid_1", sExecutionContext[rid].sbpId);
        }
    } else {
        property_set("persist.vendor.radio.sim.opid", sExecutionContext[rid].sbpId);
    }
}

/* New logic getSBPIdFromIccid() is implemented as per requirement from APTG operator.
 * This is required as -> if it is APTG SIM, always consider it as OP124(APTG) even
 * the MCCMNC might change to Starhub (OP104). CR ID: ALPS03154177, ALPS03139882
 */
static int getSBPIdFromIccid(RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    int err, i = 0;
    char *line;
    char *iccId;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RILChannelCtx *p_channel = getChannelCtxbyProxy();
    char tmp[PROPERTY_VALUE_MAX] = {0};

    RLOGD("DynamicSBP-getSBPIdFromIccid-%d:", rid);

    err = at_send_command_singleline("AT+ICCID?", "+ICCID:", &p_response, p_channel);

    if (err < 0 || NULL == p_response) {
        RLOGD("DynamicSBP-getSBPIdFromIccid-%d: Error or no SIM inserted!", rid);
        at_response_free(p_response);
        p_response = NULL;
        return -1;
    }

    if (p_response->success == 0) {
        RLOGD("DynamicSBP-getSBPIdFromIccid-%d: Error or no SIM inserted!", rid);
        at_response_free(p_response);
        p_response = NULL;
        return -1;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start (&line);
    if (err < 0) {
        RLOGD("DynamicSBP-getSBPIdFromIccid-%d: Error: %d", rid, err);
        at_response_free(p_response);
        p_response = NULL;
        return -1;
    }

    err = at_tok_nextstr(&line, &iccId);
    if (err < 0) {
        RLOGD("DynamicSBP-getSBPIdFromIccid-%d: Error: %d", rid, err);
        at_response_free(p_response);
        p_response = NULL;
        return -1;
    }
    if (iccId != NULL && strlen(iccId) > 0) {
        // Get SBP ID 124 for operator APTG
        char *p = strstr(iccId, "8988605");
        // check if string found in beginning
        if (p!= NULL && p == iccId) {
            RLOGD("DynamicSBP-getSBPIdFromIccid-%d: APTG matched", rid);
            at_response_free(p_response);
            p_response = NULL;
            return 124;
        }
    }
    RLOGD("DynamicSBP-getSBPIdFromIccid-%d: APTG match failed", rid);
    at_response_free(p_response);
    p_response = NULL;
    return -1;
}


void removeChar(char *str, char garbage) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

static int detectSBPId(RIL_SOCKET_ID rid) {
    RLOGD("DynamicSBP-detectSBPId-%d: start!!", rid);

    SIM_Status simState = getSIMStatus(rid);
    if (simState == SIM_READY || simState == USIM_READY) {
        RLOGD("DynamicSBP-detectSBPId-%d: SIM is ready, start to query IMSI", rid);

        char mccMnc[10];
        char errorMsg[20];
        memset(&mccMnc, 0, sizeof(mccMnc));
        memset(&errorMsg, 0, sizeof(errorMsg));

        /*
         * 1 Get SBP ID
         */
        // We first get mcc mnc status if it is ready then only move to read iccid
        // because it tries unnecessary AT cmds requests for iccid.
        int retStatus = readMccMnc(rid, mccMnc);
        if (retStatus == READ_MCC_MNC_ERR_NOT_READY) {
            RLOGD("DynamicSBP-detectSBPId-%d: rerty to query MCC/MNC", rid);
            return E_RETRY;
        }
        int sbpCode = getSBPIdFromIccid(rid);
        if (sbpCode >= 0) {
            // 1.1 Get SBP ID from iccid.
            snprintf(sExecutionContext[rid].sbpId, 10, "%d", sbpCode);
        } else if (retStatus == READ_MCC_MNC_ERR_SUCCESS){
            // remove "."
            removeChar(&mccMnc[0], '.');
            // 1.2 Get SBP ID from MCC-MNC.
            snprintf(sExecutionContext[rid].sbpId, 10, "%d", getSBPIdFromMccMnc(atoi(mccMnc)));
        } else {
            RLOGD("DynamicSBP-detectSBPId-%d: failed to query MCC/MNC", rid);
            return E_TERMINATE;
        }

        RLOGD("DynamicSBP-detectSBPId-%d: MCC/MNC=%s, mapping to SBP-ID=%s",
                rid, mccMnc, sExecutionContext[rid].sbpId);

        if (isSecondaryProtocolWithMps(rid) == true) {
            setRadioSbpProperty(rid);
            RLOGD("DynamicSBP-detectSBPId-%d: only system property updated, finished...", rid);
            return S_STOP;
        } else {
            /*
             * 2 Check if need to change SBP ID.
             */
            ATResponse *p_response = NULL;
            RILChannelCtx *p_channel = getChannelCtxbyProxy();
            int err = 0;
            /*
             * AT+ESBP=? to query current SPB ID in MD.
             * +ESBP: <dsbp_support>,<sbp_id>,<ims_op_code>
             */
            char *line, *token, *imsSbpId = NULL;
            while (true) {
                int ret = 0;
                err = at_send_command_singleline("AT+ESBP=?", "+ESBP:", &p_response, p_channel);
                if (err < 0 || p_response->success == 0) {
                    RLOGD("DynamicSBP-detectSBPId-%d: AT+ESBP=? failed, continue...", rid);
                    break;
                }
                if (p_response->p_intermediates == NULL) {
                    RLOGD("DynamicSBP-detectSBPId-%d: NULL response, continue...", rid);
                    break;
                }
                line = p_response->p_intermediates->line;
                ret = at_tok_start(&line);
                if (ret < 0) {
                    RLOGD("DynamicSBP-detectSBPId-%d: error token start, continue...", rid);
                    break;
                }
                // <dsbp_support>
                ret = at_tok_nextstr(&line, &token);
                if (ret < 0) {
                    RLOGD("DynamicSBP-detectSBPId-%d: error token <dsbp_support>, continue...", rid);
                    break;
                }
                // <sbp_id>
                ret = at_tok_nextstr(&line, &token);
                if (ret < 0) {
                    RLOGD("DynamicSBP-detectSBPId-%d: error token <sbp_id>, continue...", rid);
                    break;
                }
                // <ims_op_code>
                if (at_tok_hasmore(&line)) {
                    ret = at_tok_nextstr(&line, &imsSbpId);
                    if (ret < 0) {
                        RLOGD("DynamicSBP-detectSBPId-%d: error token <ims_op_code>, continue...", rid);
                        break;
                    }
                }
                // For modem not support <ims_op_code>, will set direcly.
                if (imsSbpId != NULL) {
                    if (!strcmp(sExecutionContext[rid].sbpId, imsSbpId)) {
                        RLOGD("DynamicSBP-detectSBPId-%d: SBP-ID is the same, finished...", rid);
                        // Force update here to support OM_OP load, becasue modem already be
                        // right SBP ID.
                        setRadioSbpProperty(rid);
                        return S_STOP;
                    } else {
                        RLOGD("DynamicSBP-detectSBPId-%d: SBP-ID is different, old:%s, new:%s",
                                rid, imsSbpId, sExecutionContext[rid].sbpId);
                        break;
                    }
                } else {
                    RLOGD("DynamicSBP-detectSBPId-%d: error get SBP-ID, continue...", rid);
                    break;
                }
            }
            AT_RSP_FREE(p_response);
        }
    } else if (simState == SIM_BUSY) {
        RLOGD("DynamicSBP-detectSBPId-%d: SIM is busy, re-try later", rid);
        return E_RETRY;
    } else {
        RLOGD("DynamicSBP-detectSBPId-%d: SIM isn't ready: %d, stop procedure...", rid, simState);
        return S_STOP;
    }

    RLOGD("DynamicSBP-detectSBPId-%d: Finished!!", rid);
    return S_OK;
}

static int imsDeRegistration(RIL_SOCKET_ID rid) {
    RLOGD("DynamicSBP-imsDeReg-%d: start!!", rid);
    ATResponse *p_response = NULL;

    RILChannelCtx *p_channel = getChannelCtxbyProxy();

    char* cmd = NULL;
    int err = 0;

    // AT+EIMSDEREG=<cause>
    err = at_send_command("AT+EIMSDEREG=3", &p_response, p_channel);
    if (err < 0 || p_response->success == 0) {
        RLOGD("DynamicSBP-imsDeReg-%d: AT+EIMSDEREG=3 failed", rid);
    } else {
        RLOGD("DynamicSBP-imsDeReg-%d: AT+EIMSDEREG=3 success~", rid);
    }
    at_response_free(p_response);
    p_response = NULL;

    RLOGD("DynamicSBP-imsDeReg-%d: Finished!!", rid);
    return S_OK;
}

static int setupSBP(RIL_SOCKET_ID rid) {
    if (sExecutionContext[rid].sbpId[0] == 0) {
        RLOGD("DynamicSBP-setupSBP-%d: unexpected SBP ID", rid);
        return E_TERMINATE;
    }

    RLOGD("DynamicSBP-setupSBP-%d: start!!", rid);

    char prevSbp[PROP_VALUE_MAX] = {0};
    char prevSbp_1[PROP_VALUE_MAX] = {0};
    ATResponse *p_response = NULL;

    RILChannelCtx *p_channel = getChannelCtxbyProxy();

    char* cmd = NULL;
    int err = 0;

    /*
     * 1 AT+EBOOT=1 to ignore SIM PIN lock. for ALPS02403830.
     */
    err = at_send_command("AT+EBOOT=1", &p_response, p_channel);
    if (err < 0 || p_response->success == 0) {
        RLOGD("DynamicSBP-setupSBP-%d: AT+EBOOT=1 failed", rid);
    } else {
        RLOGD("DynamicSBP-turnOffRadio-%d: AT+EBOOT=1 success~", rid);
    }
    at_response_free(p_response);
    p_response = NULL;

    /*
     * 2 AT+ESBP=id to configure SBP ID.
     */
    // Store property (temporary)for case AT+DSBP cmd failed
    if (multi_ps == 1 && RIL_SOCKET_2 == rid) {
        property_get("persist.vendor.radio.sim.opid_1", prevSbp_1, "");
    } else {
        property_get("persist.vendor.radio.sim.opid", prevSbp, "");
    }
    // Set property
    setRadioSbpProperty(rid);
    asprintf(&cmd, "AT+ESBP=0,%s", sExecutionContext[rid].sbpId);
    if (cmd == NULL) {
        RLOGD("DynamicSBP-setupSBP-%d: asprintf memory allocate failed", rid);
        return E_TERMINATE;
    }
    err = at_send_command(cmd, &p_response, p_channel);

    if (err < 0 || p_response->success == 0) {
        RLOGD("DynamicSBP-setupSBP-%d: %s failed", rid, cmd);
        // restore property as AT+DSBP cmd is failed
        if (multi_ps == 1 && RIL_SOCKET_2 == rid) {
            property_set("persist.vendor.radio.sim.opid_1", prevSbp_1);
        } else {
            property_set("persist.vendor.radio.sim.opid", prevSbp);
        }
        free(cmd);
        at_response_free(p_response);
        return E_RETRY;
    } else {
        RLOGD("DynamicSBP-setupSBP-%d: %s success~", rid, cmd);
        //property is set before sending AT+ESBP, due to timing issue: ALPS02963193
    }

    free(cmd);
    at_response_free(p_response);
    p_response = NULL;

    RLOGD("DynamicSBP-setupSBP-%d: Finished!!", rid);
    return S_OK;
}

/**
 * Must run on Proxy Thread.
 */
static void executeDynamicSBP(void *param) {
    if (param == NULL) {
        RLOGD("executeDynamicSBP: no socket information");
        return;
    }

    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *) param);
    RIL_SOCKET_ID mainRid = getMainProtocolRid();
    if (multi_ps != 1 && rid != mainRid) {
        const RIL_SOCKET_ID *p_rilId = &s_pollSimId;

        if (RIL_SOCKET_2 == mainRid) {
           p_rilId = &s_pollSimId2;
        } else if (RIL_SOCKET_3 == mainRid) {/* Gemini plus 3 SIM*/
           p_rilId = &s_pollSimId3;
        } else if (RIL_SOCKET_4 == mainRid) {/* Gemini plus 4 SIM*/
           p_rilId = &s_pollSimId4;
        }

        RIL_requestProxyTimedCallback(
                executeDynamicSBP, (void *)p_rilId, &TIMEVAL_MccMnc,
                getRILChannelId(RIL_SIM, mainRid), "executeDynamicSBP");
        RLOGD("executeDynamicSBP-%d: send to main protocol(%d) to execute",
                rid, mainRid);
        return;
    }

    while(1) {
        RLOGD("executeDynamicSBP-%d: executionPhase=%d",
                rid, sExecutionContext[rid].executionPhase);

        int ret = S_OK;
        DSBP_EXECUTION_PHASE nextPhase;

        if (!isValidRid(rid)) {
            sExecutionContext[rid].executionPhase = DSBP_PHASE_DONE;
        }

        switch (sExecutionContext[rid].executionPhase) {
            case DSBP_PHASE_DETECT_SBP_ID:
                ret = detectSBPId(rid);
                nextPhase = DSBP_PHASE_IMS_DEREG;
                break;
            case DSBP_PHASE_IMS_DEREG:
                ret = imsDeRegistration(rid);
                nextPhase = DSBP_PHASE_SETUP_SBP;
                break;
            case DSBP_PHASE_SETUP_SBP:
                ret = setupSBP(rid);
                nextPhase = DSBP_PHASE_DONE;
                break;
            case DSBP_PHASE_DONE:
                RLOGD("executeDynamicSBP-%d: Finish all procedures!", rid);
            default:
                return;
        }

        if (ret == S_OK) {
            sExecutionContext[rid].executionPhase = nextPhase;
        } else {
            if (ret == E_RETRY) {
                RIL_requestProxyTimedCallback(
                        executeDynamicSBP, param, &TIMEVAL_MccMnc,
                        getRILChannelId(RIL_SIM, rid), "executeDynamicSBP");
                RLOGD("executeDynamicSBP-%d: Retry it later...", rid);
            } else {
                // ret == E_TERMINATE || ret == S_STOP
                sExecutionContext[rid].executionPhase = DSBP_PHASE_DONE;
                RLOGD("executeDynamicSBP-%d: Stop all procedures...", rid);
            }
            return;
        }
    }
}

/**
 * Must run on Proxy Thread.
 */
static void startDynamicSBP(void *param) {

    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *) param);
    if (rid >= SIM_COUNT) {
        RLOGD("startDynamicSBP: invalid rid: %d", rid);
        return;
    }

    initialDynamicSBPExecutionContext(rid);

    RLOGD("startDynamicSBP-%d: start!!", rid);

    if (sExecutionContext[rid].executionPhase == DSBP_PHASE_DONE) {
        sExecutionContext[rid].executionPhase = DSBP_PHASE_DETECT_SBP_ID;
        executeDynamicSBP(param);
    } else {
        RLOGD("startDynamicSBP-%d: there are tasks under going, queue this new tasks first", rid);
        RIL_requestProxyTimedCallback(
                startDynamicSBP, param, &TIMEVAL_MccMnc,
                getRILChannelId(RIL_SIM, rid), "startDynamicSBP");
    }
    // ToDo: what if interrupt during procedures?
}


void setupDynamicSBP(RIL_SOCKET_ID rid) {
    RLOGD("setupDynamicSBP-%d: start!!", rid);

    char dsbp_prop[PROPERTY_VALUE_MAX] = {0};
    char multi_ps_prop[PROPERTY_VALUE_MAX] = {0};
    char multi_ims_prop[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.radio.mtk_dsbp_support", dsbp_prop, "");
    // ePDG/RDS need M-DSBP feature even if M-IMS feature is disable i.e.
    // They need this feature when M-PS is enabled CR ID: ALPS02998407
    property_get("ro.vendor.mtk_data_config", multi_ps_prop, "0");
    multi_ps = atoi(multi_ps_prop);

    property_get("persist.vendor.mims_support", multi_ims_prop, "0");
    multi_ims = atoi(multi_ims_prop);

    if (strcmp(dsbp_prop, "1") != 0) {
        RLOGD("setupDynamicSBP-%d: mtk_dynamic_sbp_support=%s, finish directly", rid, dsbp_prop);
        return;
    }

    /*
     * Currently SBP only work for MD1, even setupDynamicSBPImp can support MD1/MD2 both.
     * So now check and return directly for MD2 only when multi ps is disabled
     */
    if (multi_ims > 1) {
        RLOGD("setupDynamicSBP-%d: mtk_dynamic_sbp_support=%s, multi_ims enabled=%d",
                rid, dsbp_prop, multi_ims);
    } else if(multi_ps == 1) {
        RLOGD("setupDynamicSBP-%d: mtk_dynamic_sbp_support=%s, multi_ps enabled=%d",
                rid, dsbp_prop, multi_ps);
    } else if(getMainProtocolRid() != rid) {
        RLOGD("setupDynamicSBP-%d: SBP is only needed in SIM%d/MD1, stop procedure...",
                rid, getMainProtocolRid());
        return;
    }

    const RIL_SOCKET_ID *p_rilId = &s_pollSimId;

    if (RIL_SOCKET_2 == rid) {
       p_rilId = &s_pollSimId2;
    } else if (RIL_SOCKET_3 == rid) {/* Gemini plus 3 SIM*/
       p_rilId = &s_pollSimId3;
    } else if (RIL_SOCKET_4 == rid) {/* Gemini plus 4 SIM*/
       p_rilId = &s_pollSimId4;
    }

    RIL_requestProxyTimedCallback(
            startDynamicSBP, (void*) p_rilId, &TIMEVAL_MccMnc,
            getRILChannelId(RIL_SIM, rid), "startDynamicSBP");
}
