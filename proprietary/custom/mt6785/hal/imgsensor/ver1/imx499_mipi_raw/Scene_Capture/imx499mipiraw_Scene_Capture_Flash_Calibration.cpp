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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "camera_custom_nvram.h"
#include "imx499mipiraw_Scene_Capture.h"

const FLASH_CALIBRATION_NVRAM_T imx499mipiraw_Flash_Calibration_0000 = {
    .engTab = {
        .exp = 10000,
        .afe_gain = 1024,
        .isp_gain = 1024,
        .distance = 0,
        .yTab = {
           0, 260, 574, 886,1189,1486,1777,2060,2338,2608,2873,3131,3384,3632,3872,4107,4336,4799,5222,5625,6010,6371,6702,7025,7331,7597,7852,8086,8300,8491,8661,
         259, 548, 865,1175,1480,1776,2066,2349,2627,2896,3161,3419,3673,3919,4158,4393,4623,5079,5501,5906,6278,6637,6976,7285,7568,7860,8101,8327,8535,8724,  -1,
         554, 836,1150,1462,1763,2060,2348,2630,2908,3175,3440,3694,3948,4192,4432,4662,4888,5343,5761,6158,6529,6882,7220,7526,7820,8084,8352,8559,8759,8946,  -1,
         816,1105,1417,1728,2028,2324,2613,2892,3169,3435,3699,3953,4203,4447,4684,4914,5140,5591,6004,6400,6769,7120,7448,7761,8040,8328,8548,8774,8989,  -1,  -1,
        1076,1366,1680,1989,2290,2584,2871,3151,3426,3691,3953,4207,4457,4698,4933,5163,5387,5837,6249,6634,7006,7353,7678,7979,8264,8529,8769,9009,9196,  -1,  -1,
        1327,1614,1927,2235,2537,2831,3117,3396,3670,3935,4196,4447,4696,4938,5171,5399,5623,6071,6479,6866,7233,7577,7901,8202,8486,8749,8988,9197,  -1,  -1,  -1,
        1567,1855,2168,2477,2776,3069,3355,3634,3907,4170,4430,4680,4928,5170,5402,5628,5850,6295,6705,7088,7453,7800,8131,8413,8694,8954,9200,9406,  -1,  -1,  -1,
        1800,2087,2400,2708,3008,3301,3586,3863,4135,4399,4659,4908,5155,5395,5626,5852,6073,6519,6926,7310,7670,8012,8328,8626,8919,9163,9389,  -1,  -1,  -1,  -1,
        2028,2316,2630,2937,3236,3528,3812,4089,4361,4622,4880,5130,5374,5614,5842,6068,6288,6731,7134,7523,7881,8215,8533,8826,9104,9358,9592,  -1,  -1,  -1,  -1,
        2247,2535,2848,3155,3455,3746,4029,4306,4579,4838,5097,5343,5588,5826,6054,6279,6497,6938,7341,7720,8074,8412,8726,9015,9308,9557,  -1,  -1,  -1,  -1,  -1,
        2465,2753,3066,3374,3671,3963,4246,4522,4791,5052,5308,5557,5798,6034,6261,6487,6704,7144,7544,7927,8276,8611,8926,9212,9488,9733,  -1,  -1,  -1,  -1,  -1,
        2670,2958,3271,3578,3877,4168,4451,4725,4995,5256,5512,5759,6001,6237,6463,6685,6902,7340,7740,8119,8468,8803,9109,9397,9668,  -1,  -1,  -1,  -1,  -1,  -1,
        2875,3163,3475,3784,4081,4370,4653,4928,5196,5456,5710,5957,6197,6431,6660,6879,7096,7529,7930,8304,8656,8989,9294,9578,9853,  -1,  -1,  -1,  -1,  -1,  -1,
        3070,3358,3670,3977,4276,4564,4847,5119,5390,5647,5901,6149,6386,6622,6846,7071,7282,7722,8116,8486,8840,9166,9475,9758,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3261,3552,3864,4169,4466,4754,5037,5308,5577,5833,6091,6330,6572,6803,7031,7252,7464,7900,8297,8664,9018,9351,9657,9930,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3445,3736,4051,4355,4650,4939,5222,5492,5758,6015,6268,6511,6752,6982,7208,7426,7643,8077,8472,8841,9183,9512,9809,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3630,3919,4231,4535,4830,5121,5399,5670,5938,6192,6445,6689,6925,7158,7380,7597,7812,8255,8639,9006,9350,9670,9972,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3982,4274,4587,4892,5190,5477,5759,6028,6297,6552,6806,7050,7288,7518,7746,7958,8174,8590,8977,9336,9679,9999,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4320,4614,4924,5232,5527,5810,6092,6363,6625,6884,7132,7377,7612,7843,8064,8281,8488,8898,9291,9642,9987,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4636,4926,5240,5545,5843,6127,6405,6673,6940,7190,7439,7677,7917,8142,8362,8580,8787,9191,9570,9927,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4932,5220,5528,5833,6130,6417,6689,6958,7222,7472,7719,7956,8195,8417,8638,8853,9058,9467,9839,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5202,5493,5802,6103,6399,6682,6955,7223,7482,7737,7979,8222,8447,8671,8889,9102,9307,9710,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5461,5744,6053,6353,6642,6929,7211,7470,7727,7973,8221,8456,8693,8918,9125,9333,9534,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5693,5977,6283,6583,6873,7164,7446,7693,7946,8198,8438,8668,8895,9121,9346,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5909,6176,6487,6786,7068,7352,7626,7880,8136,8385,8629,8855,9092,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6104,6381,6682,6978,7264,7543,7808,8072,8326,8574,8805,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6296,6567,6874,7162,7441,7738,7987,8251,8493,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6456,6727,7023,7330,7601,7873,8129,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6624,6891,7173,7469,7761,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6759,7019,7303,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6882,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        },
    },
    .flashWBGain = {
    { 512, 512, 512},  //duty=-1, dutyLt=-1
    {1140, 512, 933},  //duty=0, dutyLt=-1
    {1144, 512, 950},  //duty=1, dutyLt=-1
    {1136, 512, 940},  //duty=2, dutyLt=-1
    {1133, 512, 933},  //duty=3, dutyLt=-1
    {1131, 512, 925},  //duty=4, dutyLt=-1
    {1129, 512, 917},  //duty=5, dutyLt=-1
    {1128, 512, 904},  //duty=6, dutyLt=-1
    {1139, 512, 913},  //duty=7, dutyLt=-1
    {1137, 512, 908},  //duty=8, dutyLt=-1
    {1135, 512, 903},  //duty=9, dutyLt=-1
    {1134, 512, 899},  //duty=10, dutyLt=-1
    {1134, 512, 893},  //duty=11, dutyLt=-1
    {1132, 512, 889},  //duty=12, dutyLt=-1
    {1131, 512, 885},  //duty=13, dutyLt=-1
    {1131, 512, 880},  //duty=14, dutyLt=-1
    {1130, 512, 875},  //duty=15, dutyLt=-1
    {1129, 512, 867},  //duty=16, dutyLt=-1
    {1128, 512, 858},  //duty=17, dutyLt=-1
    {1127, 512, 851},  //duty=18, dutyLt=-1
    {1126, 512, 844},  //duty=19, dutyLt=-1
    {1125, 512, 837},  //duty=20, dutyLt=-1
    {1125, 512, 831},  //duty=21, dutyLt=-1
    {1124, 512, 826},  //duty=22, dutyLt=-1
    {1123, 512, 821},  //duty=23, dutyLt=-1
    {1123, 512, 815},  //duty=24, dutyLt=-1
    {1123, 512, 811},  //duty=25, dutyLt=-1
    {1123, 512, 806},  //duty=26, dutyLt=-1
    {1123, 512, 802},  //duty=27, dutyLt=-1
    {1123, 512, 798},  //duty=28, dutyLt=-1
    {1123, 512, 794},  //duty=29, dutyLt=-1
    { 512, 512,2460},  //duty=-1, dutyLt=0
    { 721, 512,1256},  //duty=0, dutyLt=0
    { 833, 512,1166},  //duty=1, dutyLt=0
    { 894, 512,1090},  //duty=2, dutyLt=0
    { 933, 512,1048},  //duty=3, dutyLt=0
    { 959, 512,1018},  //duty=4, dutyLt=0
    { 978, 512, 992},  //duty=5, dutyLt=0
    {1002, 512, 989},  //duty=6, dutyLt=0
    {1013, 512, 976},  //duty=7, dutyLt=0
    {1021, 512, 966},  //duty=8, dutyLt=0
    {1029, 512, 957},  //duty=9, dutyLt=0
    {1035, 512, 948},  //duty=10, dutyLt=0
    {1040, 512, 942},  //duty=11, dutyLt=0
    {1044, 512, 935},  //duty=12, dutyLt=0
    {1049, 512, 929},  //duty=13, dutyLt=0
    {1052, 512, 923},  //duty=14, dutyLt=0
    {1055, 512, 918},  //duty=15, dutyLt=0
    {1060, 512, 907},  //duty=16, dutyLt=0
    {1064, 512, 898},  //duty=17, dutyLt=0
    {1067, 512, 890},  //duty=18, dutyLt=0
    {1069, 512, 881},  //duty=19, dutyLt=0
    {1072, 512, 874},  //duty=20, dutyLt=0
    {1073, 512, 867},  //duty=21, dutyLt=0
    {1075, 512, 860},  //duty=22, dutyLt=0
    {1077, 512, 853},  //duty=23, dutyLt=0
    {1079, 512, 847},  //duty=24, dutyLt=0
    {1080, 512, 841},  //duty=25, dutyLt=0
    {1081, 512, 835},  //duty=26, dutyLt=0
    {1082, 512, 830},  //duty=27, dutyLt=0
    {1083, 512, 825},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 512, 512,2219},  //duty=-1, dutyLt=1
    { 640, 512,1585},  //duty=0, dutyLt=1
    { 729, 512,1323},  //duty=1, dutyLt=1
    { 790, 512,1208},  //duty=2, dutyLt=1
    { 833, 512,1141},  //duty=3, dutyLt=1
    { 865, 512,1092},  //duty=4, dutyLt=1
    { 895, 512,1075},  //duty=5, dutyLt=1
    { 916, 512,1051},  //duty=6, dutyLt=1
    { 931, 512,1033},  //duty=7, dutyLt=1
    { 945, 512,1017},  //duty=8, dutyLt=1
    { 956, 512,1003},  //duty=9, dutyLt=1
    { 966, 512, 992},  //duty=10, dutyLt=1
    { 975, 512, 982},  //duty=11, dutyLt=1
    { 982, 512, 973},  //duty=12, dutyLt=1
    { 989, 512, 965},  //duty=13, dutyLt=1
    { 995, 512, 957},  //duty=14, dutyLt=1
    { 999, 512, 950},  //duty=15, dutyLt=1
    {1009, 512, 937},  //duty=16, dutyLt=1
    {1016, 512, 927},  //duty=17, dutyLt=1
    {1021, 512, 917},  //duty=18, dutyLt=1
    {1026, 512, 908},  //duty=19, dutyLt=1
    {1030, 512, 899},  //duty=20, dutyLt=1
    {1033, 512, 891},  //duty=21, dutyLt=1
    {1037, 512, 884},  //duty=22, dutyLt=1
    {1040, 512, 876},  //duty=23, dutyLt=1
    {1043, 512, 869},  //duty=24, dutyLt=1
    {1045, 512, 863},  //duty=25, dutyLt=1
    {1047, 512, 857},  //duty=26, dutyLt=1
    {1049, 512, 851},  //duty=27, dutyLt=1
    {1051, 512, 846},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 512, 512,2547},  //duty=-1, dutyLt=2
    { 605, 512,1724},  //duty=0, dutyLt=2
    { 679, 512,1437},  //duty=1, dutyLt=2
    { 733, 512,1300},  //duty=2, dutyLt=2
    { 774, 512,1215},  //duty=3, dutyLt=2
    { 806, 512,1145},  //duty=4, dutyLt=2
    { 836, 512,1136},  //duty=5, dutyLt=2
    { 858, 512,1105},  //duty=6, dutyLt=2
    { 877, 512,1081},  //duty=7, dutyLt=2
    { 891, 512,1062},  //duty=8, dutyLt=2
    { 905, 512,1045},  //duty=9, dutyLt=2
    { 916, 512,1031},  //duty=10, dutyLt=2
    { 927, 512,1018},  //duty=11, dutyLt=2
    { 935, 512,1007},  //duty=12, dutyLt=2
    { 943, 512, 997},  //duty=13, dutyLt=2
    { 950, 512, 988},  //duty=14, dutyLt=2
    { 956, 512, 980},  //duty=15, dutyLt=2
    { 968, 512, 964},  //duty=16, dutyLt=2
    { 977, 512, 951},  //duty=17, dutyLt=2
    { 984, 512, 941},  //duty=18, dutyLt=2
    { 991, 512, 930},  //duty=19, dutyLt=2
    { 996, 512, 921},  //duty=20, dutyLt=2
    {1000, 512, 912},  //duty=21, dutyLt=2
    {1005, 512, 904},  //duty=22, dutyLt=2
    {1009, 512, 896},  //duty=23, dutyLt=2
    {1013, 512, 889},  //duty=24, dutyLt=2
    {1015, 512, 881},  //duty=25, dutyLt=2
    {1018, 512, 875},  //duty=26, dutyLt=2
    {1021, 512, 869},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 511, 512,2501},  //duty=-1, dutyLt=3
    { 585, 512,1814},  //duty=0, dutyLt=3
    { 647, 512,1526},  //duty=1, dutyLt=3
    { 696, 512,1373},  //duty=2, dutyLt=3
    { 734, 512,1270},  //duty=3, dutyLt=3
    { 767, 512,1234},  //duty=4, dutyLt=3
    { 793, 512,1190},  //duty=5, dutyLt=3
    { 815, 512,1153},  //duty=6, dutyLt=3
    { 834, 512,1126},  //duty=7, dutyLt=3
    { 850, 512,1103},  //duty=8, dutyLt=3
    { 864, 512,1082},  //duty=9, dutyLt=3
    { 876, 512,1066},  //duty=10, dutyLt=3
    { 888, 512,1052},  //duty=11, dutyLt=3
    { 897, 512,1038},  //duty=12, dutyLt=3
    { 906, 512,1027},  //duty=13, dutyLt=3
    { 914, 512,1016},  //duty=14, dutyLt=3
    { 921, 512,1006},  //duty=15, dutyLt=3
    { 934, 512, 989},  //duty=16, dutyLt=3
    { 945, 512, 974},  //duty=17, dutyLt=3
    { 953, 512, 962},  //duty=18, dutyLt=3
    { 960, 512, 951},  //duty=19, dutyLt=3
    { 967, 512, 940},  //duty=20, dutyLt=3
    { 972, 512, 931},  //duty=21, dutyLt=3
    { 978, 512, 922},  //duty=22, dutyLt=3
    { 983, 512, 913},  //duty=23, dutyLt=3
    { 986, 512, 905},  //duty=24, dutyLt=3
    { 989, 512, 898},  //duty=25, dutyLt=3
    { 993, 512, 891},  //duty=26, dutyLt=3
    { 996, 512, 884},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 511, 512,2480},  //duty=-1, dutyLt=4
    { 573, 512,1881},  //duty=0, dutyLt=4
    { 627, 512,1595},  //duty=1, dutyLt=4
    { 670, 512,1430},  //duty=2, dutyLt=4
    { 707, 512,1358},  //duty=3, dutyLt=4
    { 737, 512,1287},  //duty=4, dutyLt=4
    { 762, 512,1236},  //duty=5, dutyLt=4
    { 784, 512,1197},  //duty=6, dutyLt=4
    { 802, 512,1165},  //duty=7, dutyLt=4
    { 818, 512,1139},  //duty=8, dutyLt=4
    { 833, 512,1117},  //duty=9, dutyLt=4
    { 845, 512,1098},  //duty=10, dutyLt=4
    { 857, 512,1082},  //duty=11, dutyLt=4
    { 867, 512,1067},  //duty=12, dutyLt=4
    { 876, 512,1054},  //duty=13, dutyLt=4
    { 884, 512,1042},  //duty=14, dutyLt=4
    { 892, 512,1031},  //duty=15, dutyLt=4
    { 906, 512,1012},  //duty=16, dutyLt=4
    { 917, 512, 996},  //duty=17, dutyLt=4
    { 926, 512, 982},  //duty=18, dutyLt=4
    { 935, 512, 969},  //duty=19, dutyLt=4
    { 942, 512, 958},  //duty=20, dutyLt=4
    { 948, 512, 948},  //duty=21, dutyLt=4
    { 953, 512, 938},  //duty=22, dutyLt=4
    { 959, 512, 929},  //duty=23, dutyLt=4
    { 963, 512, 921},  //duty=24, dutyLt=4
    { 967, 512, 913},  //duty=25, dutyLt=4
    { 970, 512, 905},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 511, 512,2468},  //duty=-1, dutyLt=5
    { 564, 512,1931},  //duty=0, dutyLt=5
    { 612, 512,1647},  //duty=1, dutyLt=5
    { 652, 512,1512},  //duty=2, dutyLt=5
    { 686, 512,1408},  //duty=3, dutyLt=5
    { 714, 512,1332},  //duty=4, dutyLt=5
    { 738, 512,1279},  //duty=5, dutyLt=5
    { 758, 512,1236},  //duty=6, dutyLt=5
    { 777, 512,1202},  //duty=7, dutyLt=5
    { 793, 512,1172},  //duty=8, dutyLt=5
    { 807, 512,1148},  //duty=9, dutyLt=5
    { 820, 512,1127},  //duty=10, dutyLt=5
    { 832, 512,1109},  //duty=11, dutyLt=5
    { 842, 512,1093},  //duty=12, dutyLt=5
    { 851, 512,1078},  //duty=13, dutyLt=5
    { 860, 512,1066},  //duty=14, dutyLt=5
    { 868, 512,1054},  //duty=15, dutyLt=5
    { 882, 512,1032},  //duty=16, dutyLt=5
    { 894, 512,1016},  //duty=17, dutyLt=5
    { 904, 512,1000},  //duty=18, dutyLt=5
    { 913, 512, 987},  //duty=19, dutyLt=5
    { 920, 512, 974},  //duty=20, dutyLt=5
    { 927, 512, 963},  //duty=21, dutyLt=5
    { 932, 512, 953},  //duty=22, dutyLt=5
    { 939, 512, 943},  //duty=23, dutyLt=5
    { 943, 512, 935},  //duty=24, dutyLt=5
    { 947, 512, 926},  //duty=25, dutyLt=5
    { 951, 512, 918},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 511, 512,2450},  //duty=-1, dutyLt=6
    { 558, 512,1965},  //duty=0, dutyLt=6
    { 602, 512,1679},  //duty=1, dutyLt=6
    { 638, 512,1558},  //duty=2, dutyLt=6
    { 668, 512,1450},  //duty=3, dutyLt=6
    { 695, 512,1373},  //duty=4, dutyLt=6
    { 718, 512,1316},  //duty=5, dutyLt=6
    { 738, 512,1271},  //duty=6, dutyLt=6
    { 756, 512,1233},  //duty=7, dutyLt=6
    { 772, 512,1203},  //duty=8, dutyLt=6
    { 786, 512,1177},  //duty=9, dutyLt=6
    { 798, 512,1154},  //duty=10, dutyLt=6
    { 810, 512,1134},  //duty=11, dutyLt=6
    { 820, 512,1117},  //duty=12, dutyLt=6
    { 830, 512,1102},  //duty=13, dutyLt=6
    { 839, 512,1088},  //duty=14, dutyLt=6
    { 847, 512,1075},  //duty=15, dutyLt=6
    { 862, 512,1053},  //duty=16, dutyLt=6
    { 874, 512,1034},  //duty=17, dutyLt=6
    { 884, 512,1017},  //duty=18, dutyLt=6
    { 893, 512,1003},  //duty=19, dutyLt=6
    { 901, 512, 990},  //duty=20, dutyLt=6
    { 908, 512, 979},  //duty=21, dutyLt=6
    { 914, 512, 968},  //duty=22, dutyLt=6
    { 920, 512, 958},  //duty=23, dutyLt=6
    { 925, 512, 948},  //duty=24, dutyLt=6
    { 929, 512, 939},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 511, 512,2434},  //duty=-1, dutyLt=7
    { 553, 512,1986},  //duty=0, dutyLt=7
    { 592, 512,1769},  //duty=1, dutyLt=7
    { 626, 512,1599},  //duty=2, dutyLt=7
    { 655, 512,1489},  //duty=3, dutyLt=7
    { 680, 512,1410},  //duty=4, dutyLt=7
    { 702, 512,1351},  //duty=5, dutyLt=7
    { 721, 512,1302},  //duty=6, dutyLt=7
    { 739, 512,1262},  //duty=7, dutyLt=7
    { 754, 512,1230},  //duty=8, dutyLt=7
    { 768, 512,1203},  //duty=9, dutyLt=7
    { 780, 512,1180},  //duty=10, dutyLt=7
    { 791, 512,1158},  //duty=11, dutyLt=7
    { 802, 512,1140},  //duty=12, dutyLt=7
    { 812, 512,1124},  //duty=13, dutyLt=7
    { 820, 512,1109},  //duty=14, dutyLt=7
    { 828, 512,1095},  //duty=15, dutyLt=7
    { 844, 512,1071},  //duty=16, dutyLt=7
    { 856, 512,1051},  //duty=17, dutyLt=7
    { 866, 512,1034},  //duty=18, dutyLt=7
    { 876, 512,1019},  //duty=19, dutyLt=7
    { 884, 512,1005},  //duty=20, dutyLt=7
    { 891, 512, 993},  //duty=21, dutyLt=7
    { 897, 512, 981},  //duty=22, dutyLt=7
    { 903, 512, 970},  //duty=23, dutyLt=7
    { 909, 512, 960},  //duty=24, dutyLt=7
    { 913, 512, 952},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 511, 512,2416},  //duty=-1, dutyLt=8
    { 549, 512,2077},  //duty=0, dutyLt=8
    { 585, 512,1804},  //duty=1, dutyLt=8
    { 617, 512,1636},  //duty=2, dutyLt=8
    { 644, 512,1526},  //duty=3, dutyLt=8
    { 668, 512,1444},  //duty=4, dutyLt=8
    { 688, 512,1381},  //duty=5, dutyLt=8
    { 707, 512,1331},  //duty=6, dutyLt=8
    { 724, 512,1290},  //duty=7, dutyLt=8
    { 739, 512,1256},  //duty=8, dutyLt=8
    { 752, 512,1227},  //duty=9, dutyLt=8
    { 765, 512,1203},  //duty=10, dutyLt=8
    { 776, 512,1181},  //duty=11, dutyLt=8
    { 786, 512,1161},  //duty=12, dutyLt=8
    { 796, 512,1144},  //duty=13, dutyLt=8
    { 804, 512,1128},  //duty=14, dutyLt=8
    { 812, 512,1114},  //duty=15, dutyLt=8
    { 828, 512,1088},  //duty=16, dutyLt=8
    { 840, 512,1068},  //duty=17, dutyLt=8
    { 851, 512,1049},  //duty=18, dutyLt=8
    { 860, 512,1033},  //duty=19, dutyLt=8
    { 868, 512,1019},  //duty=20, dutyLt=8
    { 876, 512,1006},  //duty=21, dutyLt=8
    { 882, 512, 994},  //duty=22, dutyLt=8
    { 889, 512, 983},  //duty=23, dutyLt=8
    { 894, 512, 973},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 512, 512,2392},  //duty=-1, dutyLt=9
    { 546, 512,2097},  //duty=0, dutyLt=9
    { 580, 512,1836},  //duty=1, dutyLt=9
    { 609, 512,1670},  //duty=2, dutyLt=9
    { 635, 512,1558},  //duty=3, dutyLt=9
    { 657, 512,1474},  //duty=4, dutyLt=9
    { 677, 512,1409},  //duty=5, dutyLt=9
    { 695, 512,1358},  //duty=6, dutyLt=9
    { 711, 512,1315},  //duty=7, dutyLt=9
    { 726, 512,1281},  //duty=8, dutyLt=9
    { 739, 512,1250},  //duty=9, dutyLt=9
    { 751, 512,1225},  //duty=10, dutyLt=9
    { 762, 512,1201},  //duty=11, dutyLt=9
    { 772, 512,1182},  //duty=12, dutyLt=9
    { 782, 512,1162},  //duty=13, dutyLt=9
    { 790, 512,1147},  //duty=14, dutyLt=9
    { 798, 512,1132},  //duty=15, dutyLt=9
    { 813, 512,1105},  //duty=16, dutyLt=9
    { 826, 512,1083},  //duty=17, dutyLt=9
    { 837, 512,1064},  //duty=18, dutyLt=9
    { 846, 512,1047},  //duty=19, dutyLt=9
    { 855, 512,1032},  //duty=20, dutyLt=9
    { 862, 512,1019},  //duty=21, dutyLt=9
    { 869, 512,1007},  //duty=22, dutyLt=9
    { 875, 512, 995},  //duty=23, dutyLt=9
    { 881, 512, 983},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 512, 512,2461},  //duty=-1, dutyLt=10
    { 544, 512,2113},  //duty=0, dutyLt=10
    { 575, 512,1861},  //duty=1, dutyLt=10
    { 603, 512,1700},  //duty=2, dutyLt=10
    { 627, 512,1585},  //duty=3, dutyLt=10
    { 648, 512,1500},  //duty=4, dutyLt=10
    { 667, 512,1433},  //duty=5, dutyLt=10
    { 685, 512,1382},  //duty=6, dutyLt=10
    { 700, 512,1339},  //duty=7, dutyLt=10
    { 714, 512,1302},  //duty=8, dutyLt=10
    { 727, 512,1271},  //duty=9, dutyLt=10
    { 739, 512,1245},  //duty=10, dutyLt=10
    { 750, 512,1220},  //duty=11, dutyLt=10
    { 760, 512,1200},  //duty=12, dutyLt=10
    { 769, 512,1181},  //duty=13, dutyLt=10
    { 778, 512,1163},  //duty=14, dutyLt=10
    { 786, 512,1148},  //duty=15, dutyLt=10
    { 801, 512,1120},  //duty=16, dutyLt=10
    { 813, 512,1097},  //duty=17, dutyLt=10
    { 824, 512,1078},  //duty=18, dutyLt=10
    { 834, 512,1060},  //duty=19, dutyLt=10
    { 842, 512,1044},  //duty=20, dutyLt=10
    { 850, 512,1031},  //duty=21, dutyLt=10
    { 857, 512,1018},  //duty=22, dutyLt=10
    { 863, 512,1005},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 512, 512,2453},  //duty=-1, dutyLt=11
    { 542, 512,2121},  //duty=0, dutyLt=11
    { 571, 512,1881},  //duty=1, dutyLt=11
    { 597, 512,1724},  //duty=2, dutyLt=11
    { 620, 512,1608},  //duty=3, dutyLt=11
    { 641, 512,1525},  //duty=4, dutyLt=11
    { 659, 512,1456},  //duty=5, dutyLt=11
    { 676, 512,1404},  //duty=6, dutyLt=11
    { 691, 512,1361},  //duty=7, dutyLt=11
    { 705, 512,1323},  //duty=8, dutyLt=11
    { 717, 512,1291},  //duty=9, dutyLt=11
    { 729, 512,1263},  //duty=10, dutyLt=11
    { 739, 512,1238},  //duty=11, dutyLt=11
    { 749, 512,1217},  //duty=12, dutyLt=11
    { 758, 512,1198},  //duty=13, dutyLt=11
    { 767, 512,1180},  //duty=14, dutyLt=11
    { 775, 512,1164},  //duty=15, dutyLt=11
    { 790, 512,1135},  //duty=16, dutyLt=11
    { 802, 512,1111},  //duty=17, dutyLt=11
    { 813, 512,1091},  //duty=18, dutyLt=11
    { 823, 512,1072},  //duty=19, dutyLt=11
    { 831, 512,1057},  //duty=20, dutyLt=11
    { 839, 512,1042},  //duty=21, dutyLt=11
    { 845, 512,1029},  //duty=22, dutyLt=11
    { 852, 512,1016},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 512, 512,2444},  //duty=-1, dutyLt=12
    { 540, 512,2125},  //duty=0, dutyLt=12
    { 568, 512,1896},  //duty=1, dutyLt=12
    { 592, 512,1741},  //duty=2, dutyLt=12
    { 614, 512,1629},  //duty=3, dutyLt=12
    { 634, 512,1545},  //duty=4, dutyLt=12
    { 652, 512,1478},  //duty=5, dutyLt=12
    { 668, 512,1426},  //duty=6, dutyLt=12
    { 682, 512,1379},  //duty=7, dutyLt=12
    { 696, 512,1342},  //duty=8, dutyLt=12
    { 708, 512,1309},  //duty=9, dutyLt=12
    { 719, 512,1281},  //duty=10, dutyLt=12
    { 730, 512,1256},  //duty=11, dutyLt=12
    { 740, 512,1233},  //duty=12, dutyLt=12
    { 748, 512,1213},  //duty=13, dutyLt=12
    { 757, 512,1195},  //duty=14, dutyLt=12
    { 765, 512,1179},  //duty=15, dutyLt=12
    { 780, 512,1149},  //duty=16, dutyLt=12
    { 792, 512,1124},  //duty=17, dutyLt=12
    { 803, 512,1103},  //duty=18, dutyLt=12
    { 812, 512,1085},  //duty=19, dutyLt=12
    { 821, 512,1068},  //duty=20, dutyLt=12
    { 829, 512,1053},  //duty=21, dutyLt=12
    { 835, 512,1039},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 512, 512, 512},  //duty=28, dutyLt=12
    { 512, 512, 512},  //duty=29, dutyLt=12
    { 512, 512,2436},  //duty=-1, dutyLt=13
    { 539, 512,2131},  //duty=0, dutyLt=13
    { 565, 512,1910},  //duty=1, dutyLt=13
    { 589, 512,1758},  //duty=2, dutyLt=13
    { 609, 512,1648},  //duty=3, dutyLt=13
    { 628, 512,1565},  //duty=4, dutyLt=13
    { 645, 512,1499},  //duty=5, dutyLt=13
    { 661, 512,1443},  //duty=6, dutyLt=13
    { 675, 512,1398},  //duty=7, dutyLt=13
    { 688, 512,1360},  //duty=8, dutyLt=13
    { 700, 512,1326},  //duty=9, dutyLt=13
    { 711, 512,1297},  //duty=10, dutyLt=13
    { 722, 512,1271},  //duty=11, dutyLt=13
    { 731, 512,1248},  //duty=12, dutyLt=13
    { 740, 512,1228},  //duty=13, dutyLt=13
    { 748, 512,1209},  //duty=14, dutyLt=13
    { 756, 512,1192},  //duty=15, dutyLt=13
    { 771, 512,1162},  //duty=16, dutyLt=13
    { 783, 512,1137},  //duty=17, dutyLt=13
    { 794, 512,1115},  //duty=18, dutyLt=13
    { 803, 512,1096},  //duty=19, dutyLt=13
    { 812, 512,1079},  //duty=20, dutyLt=13
    { 819, 512,1063},  //duty=21, dutyLt=13
    { 826, 512,1049},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 512, 512,2435},  //duty=-1, dutyLt=14
    { 537, 512,2134},  //duty=0, dutyLt=14
    { 562, 512,1923},  //duty=1, dutyLt=14
    { 585, 512,1775},  //duty=2, dutyLt=14
    { 605, 512,1668},  //duty=3, dutyLt=14
    { 623, 512,1583},  //duty=4, dutyLt=14
    { 640, 512,1517},  //duty=5, dutyLt=14
    { 655, 512,1460},  //duty=6, dutyLt=14
    { 669, 512,1415},  //duty=7, dutyLt=14
    { 681, 512,1376},  //duty=8, dutyLt=14
    { 693, 512,1342},  //duty=9, dutyLt=14
    { 704, 512,1313},  //duty=10, dutyLt=14
    { 714, 512,1286},  //duty=11, dutyLt=14
    { 723, 512,1262},  //duty=12, dutyLt=14
    { 732, 512,1242},  //duty=13, dutyLt=14
    { 740, 512,1223},  //duty=14, dutyLt=14
    { 748, 512,1205},  //duty=15, dutyLt=14
    { 762, 512,1174},  //duty=16, dutyLt=14
    { 774, 512,1148},  //duty=17, dutyLt=14
    { 785, 512,1126},  //duty=18, dutyLt=14
    { 795, 512,1106},  //duty=19, dutyLt=14
    { 803, 512,1089},  //duty=20, dutyLt=14
    { 811, 512,1072},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 512, 512, 512},  //duty=28, dutyLt=14
    { 512, 512, 512},  //duty=29, dutyLt=14
    { 513, 512,2424},  //duty=-1, dutyLt=15
    { 537, 512,2144},  //duty=0, dutyLt=15
    { 560, 512,1940},  //duty=1, dutyLt=15
    { 582, 512,1794},  //duty=2, dutyLt=15
    { 601, 512,1686},  //duty=3, dutyLt=15
    { 619, 512,1601},  //duty=4, dutyLt=15
    { 635, 512,1532},  //duty=5, dutyLt=15
    { 649, 512,1477},  //duty=6, dutyLt=15
    { 663, 512,1431},  //duty=7, dutyLt=15
    { 675, 512,1391},  //duty=8, dutyLt=15
    { 687, 512,1357},  //duty=9, dutyLt=15
    { 697, 512,1326},  //duty=10, dutyLt=15
    { 707, 512,1300},  //duty=11, dutyLt=15
    { 716, 512,1276},  //duty=12, dutyLt=15
    { 725, 512,1255},  //duty=13, dutyLt=15
    { 733, 512,1235},  //duty=14, dutyLt=15
    { 741, 512,1218},  //duty=15, dutyLt=15
    { 755, 512,1186},  //duty=16, dutyLt=15
    { 767, 512,1159},  //duty=17, dutyLt=15
    { 777, 512,1136},  //duty=18, dutyLt=15
    { 787, 512,1116},  //duty=19, dutyLt=15
    { 795, 512,1098},  //duty=20, dutyLt=15
    { 803, 512,1082},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 513, 512,2399},  //duty=-1, dutyLt=16
    { 535, 512,2155},  //duty=0, dutyLt=16
    { 557, 512,1958},  //duty=1, dutyLt=16
    { 577, 512,1818},  //duty=2, dutyLt=16
    { 595, 512,1709},  //duty=3, dutyLt=16
    { 611, 512,1627},  //duty=4, dutyLt=16
    { 626, 512,1560},  //duty=5, dutyLt=16
    { 640, 512,1505},  //duty=6, dutyLt=16
    { 653, 512,1457},  //duty=7, dutyLt=16
    { 665, 512,1417},  //duty=8, dutyLt=16
    { 676, 512,1383},  //duty=9, dutyLt=16
    { 686, 512,1352},  //duty=10, dutyLt=16
    { 696, 512,1324},  //duty=11, dutyLt=16
    { 705, 512,1299},  //duty=12, dutyLt=16
    { 713, 512,1277},  //duty=13, dutyLt=16
    { 721, 512,1258},  //duty=14, dutyLt=16
    { 728, 512,1239},  //duty=15, dutyLt=16
    { 741, 512,1207},  //duty=16, dutyLt=16
    { 753, 512,1180},  //duty=17, dutyLt=16
    { 764, 512,1156},  //duty=18, dutyLt=16
    { 773, 512,1135},  //duty=19, dutyLt=16
    { 781, 512,1116},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 512, 512, 512},  //duty=28, dutyLt=16
    { 512, 512, 512},  //duty=29, dutyLt=16
    { 513, 512,2394},  //duty=-1, dutyLt=17
    { 533, 512,2153},  //duty=0, dutyLt=17
    { 554, 512,1974},  //duty=1, dutyLt=17
    { 572, 512,1838},  //duty=2, dutyLt=17
    { 589, 512,1737},  //duty=3, dutyLt=17
    { 605, 512,1654},  //duty=4, dutyLt=17
    { 619, 512,1586},  //duty=5, dutyLt=17
    { 632, 512,1529},  //duty=6, dutyLt=17
    { 645, 512,1482},  //duty=7, dutyLt=17
    { 656, 512,1442},  //duty=8, dutyLt=17
    { 667, 512,1405},  //duty=9, dutyLt=17
    { 676, 512,1375},  //duty=10, dutyLt=17
    { 686, 512,1346},  //duty=11, dutyLt=17
    { 694, 512,1322},  //duty=12, dutyLt=17
    { 703, 512,1299},  //duty=13, dutyLt=17
    { 710, 512,1279},  //duty=14, dutyLt=17
    { 717, 512,1259},  //duty=15, dutyLt=17
    { 730, 512,1226},  //duty=16, dutyLt=17
    { 742, 512,1199},  //duty=17, dutyLt=17
    { 752, 512,1174},  //duty=18, dutyLt=17
    { 761, 512,1152},  //duty=19, dutyLt=17
    { 512, 512, 512},  //duty=20, dutyLt=17
    { 512, 512, 512},  //duty=21, dutyLt=17
    { 512, 512, 512},  //duty=22, dutyLt=17
    { 512, 512, 512},  //duty=23, dutyLt=17
    { 512, 512, 512},  //duty=24, dutyLt=17
    { 512, 512, 512},  //duty=25, dutyLt=17
    { 512, 512, 512},  //duty=26, dutyLt=17
    { 512, 512, 512},  //duty=27, dutyLt=17
    { 512, 512, 512},  //duty=28, dutyLt=17
    { 512, 512, 512},  //duty=29, dutyLt=17
    { 514, 512,2372},  //duty=-1, dutyLt=18
    { 533, 512,2161},  //duty=0, dutyLt=18
    { 552, 512,1986},  //duty=1, dutyLt=18
    { 569, 512,1856},  //duty=2, dutyLt=18
    { 585, 512,1755},  //duty=3, dutyLt=18
    { 600, 512,1674},  //duty=4, dutyLt=18
    { 614, 512,1607},  //duty=5, dutyLt=18
    { 626, 512,1552},  //duty=6, dutyLt=18
    { 638, 512,1504},  //duty=7, dutyLt=18
    { 649, 512,1463},  //duty=8, dutyLt=18
    { 659, 512,1427},  //duty=9, dutyLt=18
    { 669, 512,1396},  //duty=10, dutyLt=18
    { 677, 512,1366},  //duty=11, dutyLt=18
    { 686, 512,1341},  //duty=12, dutyLt=18
    { 694, 512,1318},  //duty=13, dutyLt=18
    { 701, 512,1297},  //duty=14, dutyLt=18
    { 708, 512,1277},  //duty=15, dutyLt=18
    { 721, 512,1243},  //duty=16, dutyLt=18
    { 732, 512,1215},  //duty=17, dutyLt=18
    { 742, 512,1189},  //duty=18, dutyLt=18
    { 512, 512, 512},  //duty=19, dutyLt=18
    { 512, 512, 512},  //duty=20, dutyLt=18
    { 512, 512, 512},  //duty=21, dutyLt=18
    { 512, 512, 512},  //duty=22, dutyLt=18
    { 512, 512, 512},  //duty=23, dutyLt=18
    { 512, 512, 512},  //duty=24, dutyLt=18
    { 512, 512, 512},  //duty=25, dutyLt=18
    { 512, 512, 512},  //duty=26, dutyLt=18
    { 512, 512, 512},  //duty=27, dutyLt=18
    { 512, 512, 512},  //duty=28, dutyLt=18
    { 512, 512, 512},  //duty=29, dutyLt=18
    { 514, 512,2368},  //duty=-1, dutyLt=19
    { 532, 512,2159},  //duty=0, dutyLt=19
    { 550, 512,1997},  //duty=1, dutyLt=19
    { 567, 512,1870},  //duty=2, dutyLt=19
    { 582, 512,1771},  //duty=3, dutyLt=19
    { 596, 512,1692},  //duty=4, dutyLt=19
    { 609, 512,1625},  //duty=5, dutyLt=19
    { 621, 512,1570},  //duty=6, dutyLt=19
    { 633, 512,1522},  //duty=7, dutyLt=19
    { 643, 512,1481},  //duty=8, dutyLt=19
    { 653, 512,1445},  //duty=9, dutyLt=19
    { 662, 512,1412},  //duty=10, dutyLt=19
    { 671, 512,1384},  //duty=11, dutyLt=19
    { 679, 512,1357},  //duty=12, dutyLt=19
    { 686, 512,1334},  //duty=13, dutyLt=19
    { 693, 512,1313},  //duty=14, dutyLt=19
    { 700, 512,1293},  //duty=15, dutyLt=19
    { 712, 512,1259},  //duty=16, dutyLt=19
    { 723, 512,1229},  //duty=17, dutyLt=19
    { 512, 512, 512},  //duty=18, dutyLt=19
    { 512, 512, 512},  //duty=19, dutyLt=19
    { 512, 512, 512},  //duty=20, dutyLt=19
    { 512, 512, 512},  //duty=21, dutyLt=19
    { 512, 512, 512},  //duty=22, dutyLt=19
    { 512, 512, 512},  //duty=23, dutyLt=19
    { 512, 512, 512},  //duty=24, dutyLt=19
    { 512, 512, 512},  //duty=25, dutyLt=19
    { 512, 512, 512},  //duty=26, dutyLt=19
    { 512, 512, 512},  //duty=27, dutyLt=19
    { 512, 512, 512},  //duty=28, dutyLt=19
    { 512, 512, 512},  //duty=29, dutyLt=19
    { 515, 512,2354},  //duty=-1, dutyLt=20
    { 532, 512,2159},  //duty=0, dutyLt=20
    { 549, 512,2003},  //duty=1, dutyLt=20
    { 565, 512,1883},  //duty=2, dutyLt=20
    { 579, 512,1786},  //duty=3, dutyLt=20
    { 593, 512,1706},  //duty=4, dutyLt=20
    { 605, 512,1641},  //duty=5, dutyLt=20
    { 617, 512,1586},  //duty=6, dutyLt=20
    { 628, 512,1537},  //duty=7, dutyLt=20
    { 638, 512,1497},  //duty=8, dutyLt=20
    { 647, 512,1460},  //duty=9, dutyLt=20
    { 656, 512,1428},  //duty=10, dutyLt=20
    { 665, 512,1399},  //duty=11, dutyLt=20
    { 673, 512,1373},  //duty=12, dutyLt=20
    { 680, 512,1348},  //duty=13, dutyLt=20
    { 687, 512,1328},  //duty=14, dutyLt=20
    { 694, 512,1307},  //duty=15, dutyLt=20
    { 706, 512,1272},  //duty=16, dutyLt=20
    { 512, 512, 512},  //duty=17, dutyLt=20
    { 512, 512, 512},  //duty=18, dutyLt=20
    { 512, 512, 512},  //duty=19, dutyLt=20
    { 512, 512, 512},  //duty=20, dutyLt=20
    { 512, 512, 512},  //duty=21, dutyLt=20
    { 512, 512, 512},  //duty=22, dutyLt=20
    { 512, 512, 512},  //duty=23, dutyLt=20
    { 512, 512, 512},  //duty=24, dutyLt=20
    { 512, 512, 512},  //duty=25, dutyLt=20
    { 512, 512, 512},  //duty=26, dutyLt=20
    { 512, 512, 512},  //duty=27, dutyLt=20
    { 512, 512, 512},  //duty=28, dutyLt=20
    { 512, 512, 512},  //duty=29, dutyLt=20
    { 515, 512,2339},  //duty=-1, dutyLt=21
    { 532, 512,2158},  //duty=0, dutyLt=21
    { 548, 512,2008},  //duty=1, dutyLt=21
    { 563, 512,1890},  //duty=2, dutyLt=21
    { 577, 512,1796},  //duty=3, dutyLt=21
    { 590, 512,1718},  //duty=4, dutyLt=21
    { 602, 512,1655},  //duty=5, dutyLt=21
    { 614, 512,1599},  //duty=6, dutyLt=21
    { 624, 512,1552},  //duty=7, dutyLt=21
    { 634, 512,1510},  //duty=8, dutyLt=21
    { 643, 512,1474},  //duty=9, dutyLt=21
    { 652, 512,1442},  //duty=10, dutyLt=21
    { 660, 512,1413},  //duty=11, dutyLt=21
    { 667, 512,1387},  //duty=12, dutyLt=21
    { 675, 512,1362},  //duty=13, dutyLt=21
    { 682, 512,1340},  //duty=14, dutyLt=21
    { 688, 512,1320},  //duty=15, dutyLt=21
    { 512, 512, 512},  //duty=16, dutyLt=21
    { 512, 512, 512},  //duty=17, dutyLt=21
    { 512, 512, 512},  //duty=18, dutyLt=21
    { 512, 512, 512},  //duty=19, dutyLt=21
    { 512, 512, 512},  //duty=20, dutyLt=21
    { 512, 512, 512},  //duty=21, dutyLt=21
    { 512, 512, 512},  //duty=22, dutyLt=21
    { 512, 512, 512},  //duty=23, dutyLt=21
    { 512, 512, 512},  //duty=24, dutyLt=21
    { 512, 512, 512},  //duty=25, dutyLt=21
    { 512, 512, 512},  //duty=26, dutyLt=21
    { 512, 512, 512},  //duty=27, dutyLt=21
    { 512, 512, 512},  //duty=28, dutyLt=21
    { 512, 512, 512},  //duty=29, dutyLt=21
    { 516, 512,2329},  //duty=-1, dutyLt=22
    { 532, 512,2158},  //duty=0, dutyLt=22
    { 547, 512,2012},  //duty=1, dutyLt=22
    { 562, 512,1897},  //duty=2, dutyLt=22
    { 575, 512,1804},  //duty=3, dutyLt=22
    { 588, 512,1730},  //duty=4, dutyLt=22
    { 599, 512,1667},  //duty=5, dutyLt=22
    { 611, 512,1610},  //duty=6, dutyLt=22
    { 621, 512,1564},  //duty=7, dutyLt=22
    { 630, 512,1522},  //duty=8, dutyLt=22
    { 639, 512,1486},  //duty=9, dutyLt=22
    { 648, 512,1453},  //duty=10, dutyLt=22
    { 656, 512,1423},  //duty=11, dutyLt=22
    { 663, 512,1398},  //duty=12, dutyLt=22
    { 670, 512,1374},  //duty=13, dutyLt=22
    { 512, 512, 512},  //duty=14, dutyLt=22
    { 512, 512, 512},  //duty=15, dutyLt=22
    { 512, 512, 512},  //duty=16, dutyLt=22
    { 512, 512, 512},  //duty=17, dutyLt=22
    { 512, 512, 512},  //duty=18, dutyLt=22
    { 512, 512, 512},  //duty=19, dutyLt=22
    { 512, 512, 512},  //duty=20, dutyLt=22
    { 512, 512, 512},  //duty=21, dutyLt=22
    { 512, 512, 512},  //duty=22, dutyLt=22
    { 512, 512, 512},  //duty=23, dutyLt=22
    { 512, 512, 512},  //duty=24, dutyLt=22
    { 512, 512, 512},  //duty=25, dutyLt=22
    { 512, 512, 512},  //duty=26, dutyLt=22
    { 512, 512, 512},  //duty=27, dutyLt=22
    { 512, 512, 512},  //duty=28, dutyLt=22
    { 512, 512, 512},  //duty=29, dutyLt=22
    { 516, 512,2322},  //duty=-1, dutyLt=23
    { 531, 512,2156},  //duty=0, dutyLt=23
    { 547, 512,2015},  //duty=1, dutyLt=23
    { 561, 512,1903},  //duty=2, dutyLt=23
    { 574, 512,1812},  //duty=3, dutyLt=23
    { 586, 512,1738},  //duty=4, dutyLt=23
    { 598, 512,1675},  //duty=5, dutyLt=23
    { 609, 512,1619},  //duty=6, dutyLt=23
    { 618, 512,1573},  //duty=7, dutyLt=23
    { 628, 512,1532},  //duty=8, dutyLt=23
    { 636, 512,1495},  //duty=9, dutyLt=23
    { 645, 512,1463},  //duty=10, dutyLt=23
    { 652, 512,1433},  //duty=11, dutyLt=23
    { 512, 512, 512},  //duty=12, dutyLt=23
    { 512, 512, 512},  //duty=13, dutyLt=23
    { 512, 512, 512},  //duty=14, dutyLt=23
    { 512, 512, 512},  //duty=15, dutyLt=23
    { 512, 512, 512},  //duty=16, dutyLt=23
    { 512, 512, 512},  //duty=17, dutyLt=23
    { 512, 512, 512},  //duty=18, dutyLt=23
    { 512, 512, 512},  //duty=19, dutyLt=23
    { 512, 512, 512},  //duty=20, dutyLt=23
    { 512, 512, 512},  //duty=21, dutyLt=23
    { 512, 512, 512},  //duty=22, dutyLt=23
    { 512, 512, 512},  //duty=23, dutyLt=23
    { 512, 512, 512},  //duty=24, dutyLt=23
    { 512, 512, 512},  //duty=25, dutyLt=23
    { 512, 512, 512},  //duty=26, dutyLt=23
    { 512, 512, 512},  //duty=27, dutyLt=23
    { 512, 512, 512},  //duty=28, dutyLt=23
    { 512, 512, 512},  //duty=29, dutyLt=23
    { 517, 512,2312},  //duty=-1, dutyLt=24
    { 532, 512,2155},  //duty=0, dutyLt=24
    { 547, 512,2017},  //duty=1, dutyLt=24
    { 560, 512,1906},  //duty=2, dutyLt=24
    { 573, 512,1819},  //duty=3, dutyLt=24
    { 585, 512,1744},  //duty=4, dutyLt=24
    { 596, 512,1682},  //duty=5, dutyLt=24
    { 606, 512,1628},  //duty=6, dutyLt=24
    { 616, 512,1581},  //duty=7, dutyLt=24
    { 625, 512,1540},  //duty=8, dutyLt=24
    { 634, 512,1503},  //duty=9, dutyLt=24
    { 512, 512, 512},  //duty=10, dutyLt=24
    { 512, 512, 512},  //duty=11, dutyLt=24
    { 512, 512, 512},  //duty=12, dutyLt=24
    { 512, 512, 512},  //duty=13, dutyLt=24
    { 512, 512, 512},  //duty=14, dutyLt=24
    { 512, 512, 512},  //duty=15, dutyLt=24
    { 512, 512, 512},  //duty=16, dutyLt=24
    { 512, 512, 512},  //duty=17, dutyLt=24
    { 512, 512, 512},  //duty=18, dutyLt=24
    { 512, 512, 512},  //duty=19, dutyLt=24
    { 512, 512, 512},  //duty=20, dutyLt=24
    { 512, 512, 512},  //duty=21, dutyLt=24
    { 512, 512, 512},  //duty=22, dutyLt=24
    { 512, 512, 512},  //duty=23, dutyLt=24
    { 512, 512, 512},  //duty=24, dutyLt=24
    { 512, 512, 512},  //duty=25, dutyLt=24
    { 512, 512, 512},  //duty=26, dutyLt=24
    { 512, 512, 512},  //duty=27, dutyLt=24
    { 512, 512, 512},  //duty=28, dutyLt=24
    { 512, 512, 512},  //duty=29, dutyLt=24
    { 518, 512,2305},  //duty=-1, dutyLt=25
    { 532, 512,2152},  //duty=0, dutyLt=25
    { 546, 512,2019},  //duty=1, dutyLt=25
    { 560, 512,1911},  //duty=2, dutyLt=25
    { 572, 512,1823},  //duty=3, dutyLt=25
    { 583, 512,1752},  //duty=4, dutyLt=25
    { 595, 512,1688},  //duty=5, dutyLt=25
    { 605, 512,1636},  //duty=6, dutyLt=25
    { 614, 512,1588},  //duty=7, dutyLt=25
    { 512, 512, 512},  //duty=8, dutyLt=25
    { 512, 512, 512},  //duty=9, dutyLt=25
    { 512, 512, 512},  //duty=10, dutyLt=25
    { 512, 512, 512},  //duty=11, dutyLt=25
    { 512, 512, 512},  //duty=12, dutyLt=25
    { 512, 512, 512},  //duty=13, dutyLt=25
    { 512, 512, 512},  //duty=14, dutyLt=25
    { 512, 512, 512},  //duty=15, dutyLt=25
    { 512, 512, 512},  //duty=16, dutyLt=25
    { 512, 512, 512},  //duty=17, dutyLt=25
    { 512, 512, 512},  //duty=18, dutyLt=25
    { 512, 512, 512},  //duty=19, dutyLt=25
    { 512, 512, 512},  //duty=20, dutyLt=25
    { 512, 512, 512},  //duty=21, dutyLt=25
    { 512, 512, 512},  //duty=22, dutyLt=25
    { 512, 512, 512},  //duty=23, dutyLt=25
    { 512, 512, 512},  //duty=24, dutyLt=25
    { 512, 512, 512},  //duty=25, dutyLt=25
    { 512, 512, 512},  //duty=26, dutyLt=25
    { 512, 512, 512},  //duty=27, dutyLt=25
    { 512, 512, 512},  //duty=28, dutyLt=25
    { 512, 512, 512},  //duty=29, dutyLt=25
    { 519, 512,2295},  //duty=-1, dutyLt=26
    { 532, 512,2148},  //duty=0, dutyLt=26
    { 546, 512,2017},  //duty=1, dutyLt=26
    { 559, 512,1915},  //duty=2, dutyLt=26
    { 572, 512,1827},  //duty=3, dutyLt=26
    { 583, 512,1754},  //duty=4, dutyLt=26
    { 594, 512,1693},  //duty=5, dutyLt=26
    { 512, 512, 512},  //duty=6, dutyLt=26
    { 512, 512, 512},  //duty=7, dutyLt=26
    { 512, 512, 512},  //duty=8, dutyLt=26
    { 512, 512, 512},  //duty=9, dutyLt=26
    { 512, 512, 512},  //duty=10, dutyLt=26
    { 512, 512, 512},  //duty=11, dutyLt=26
    { 512, 512, 512},  //duty=12, dutyLt=26
    { 512, 512, 512},  //duty=13, dutyLt=26
    { 512, 512, 512},  //duty=14, dutyLt=26
    { 512, 512, 512},  //duty=15, dutyLt=26
    { 512, 512, 512},  //duty=16, dutyLt=26
    { 512, 512, 512},  //duty=17, dutyLt=26
    { 512, 512, 512},  //duty=18, dutyLt=26
    { 512, 512, 512},  //duty=19, dutyLt=26
    { 512, 512, 512},  //duty=20, dutyLt=26
    { 512, 512, 512},  //duty=21, dutyLt=26
    { 512, 512, 512},  //duty=22, dutyLt=26
    { 512, 512, 512},  //duty=23, dutyLt=26
    { 512, 512, 512},  //duty=24, dutyLt=26
    { 512, 512, 512},  //duty=25, dutyLt=26
    { 512, 512, 512},  //duty=26, dutyLt=26
    { 512, 512, 512},  //duty=27, dutyLt=26
    { 512, 512, 512},  //duty=28, dutyLt=26
    { 512, 512, 512},  //duty=29, dutyLt=26
    { 519, 512,2289},  //duty=-1, dutyLt=27
    { 533, 512,2146},  //duty=0, dutyLt=27
    { 547, 512,2017},  //duty=1, dutyLt=27
    { 559, 512,1914},  //duty=2, dutyLt=27
    { 571, 512,1831},  //duty=3, dutyLt=27
    { 512, 512, 512},  //duty=4, dutyLt=27
    { 512, 512, 512},  //duty=5, dutyLt=27
    { 512, 512, 512},  //duty=6, dutyLt=27
    { 512, 512, 512},  //duty=7, dutyLt=27
    { 512, 512, 512},  //duty=8, dutyLt=27
    { 512, 512, 512},  //duty=9, dutyLt=27
    { 512, 512, 512},  //duty=10, dutyLt=27
    { 512, 512, 512},  //duty=11, dutyLt=27
    { 512, 512, 512},  //duty=12, dutyLt=27
    { 512, 512, 512},  //duty=13, dutyLt=27
    { 512, 512, 512},  //duty=14, dutyLt=27
    { 512, 512, 512},  //duty=15, dutyLt=27
    { 512, 512, 512},  //duty=16, dutyLt=27
    { 512, 512, 512},  //duty=17, dutyLt=27
    { 512, 512, 512},  //duty=18, dutyLt=27
    { 512, 512, 512},  //duty=19, dutyLt=27
    { 512, 512, 512},  //duty=20, dutyLt=27
    { 512, 512, 512},  //duty=21, dutyLt=27
    { 512, 512, 512},  //duty=22, dutyLt=27
    { 512, 512, 512},  //duty=23, dutyLt=27
    { 512, 512, 512},  //duty=24, dutyLt=27
    { 512, 512, 512},  //duty=25, dutyLt=27
    { 512, 512, 512},  //duty=26, dutyLt=27
    { 512, 512, 512},  //duty=27, dutyLt=27
    { 512, 512, 512},  //duty=28, dutyLt=27
    { 512, 512, 512},  //duty=29, dutyLt=27
    { 520, 512,2282},  //duty=-1, dutyLt=28
    { 533, 512,2140},  //duty=0, dutyLt=28
    { 547, 512,2015},  //duty=1, dutyLt=28
    { 512, 512, 512},  //duty=2, dutyLt=28
    { 512, 512, 512},  //duty=3, dutyLt=28
    { 512, 512, 512},  //duty=4, dutyLt=28
    { 512, 512, 512},  //duty=5, dutyLt=28
    { 512, 512, 512},  //duty=6, dutyLt=28
    { 512, 512, 512},  //duty=7, dutyLt=28
    { 512, 512, 512},  //duty=8, dutyLt=28
    { 512, 512, 512},  //duty=9, dutyLt=28
    { 512, 512, 512},  //duty=10, dutyLt=28
    { 512, 512, 512},  //duty=11, dutyLt=28
    { 512, 512, 512},  //duty=12, dutyLt=28
    { 512, 512, 512},  //duty=13, dutyLt=28
    { 512, 512, 512},  //duty=14, dutyLt=28
    { 512, 512, 512},  //duty=15, dutyLt=28
    { 512, 512, 512},  //duty=16, dutyLt=28
    { 512, 512, 512},  //duty=17, dutyLt=28
    { 512, 512, 512},  //duty=18, dutyLt=28
    { 512, 512, 512},  //duty=19, dutyLt=28
    { 512, 512, 512},  //duty=20, dutyLt=28
    { 512, 512, 512},  //duty=21, dutyLt=28
    { 512, 512, 512},  //duty=22, dutyLt=28
    { 512, 512, 512},  //duty=23, dutyLt=28
    { 512, 512, 512},  //duty=24, dutyLt=28
    { 512, 512, 512},  //duty=25, dutyLt=28
    { 512, 512, 512},  //duty=26, dutyLt=28
    { 512, 512, 512},  //duty=27, dutyLt=28
    { 512, 512, 512},  //duty=28, dutyLt=28
    { 512, 512, 512},  //duty=29, dutyLt=28
    { 521, 512,2273},  //duty=-1, dutyLt=29
    { 512, 512, 512},  //duty=0, dutyLt=29
    { 512, 512, 512},  //duty=1, dutyLt=29
    { 512, 512, 512},  //duty=2, dutyLt=29
    { 512, 512, 512},  //duty=3, dutyLt=29
    { 512, 512, 512},  //duty=4, dutyLt=29
    { 512, 512, 512},  //duty=5, dutyLt=29
    { 512, 512, 512},  //duty=6, dutyLt=29
    { 512, 512, 512},  //duty=7, dutyLt=29
    { 512, 512, 512},  //duty=8, dutyLt=29
    { 512, 512, 512},  //duty=9, dutyLt=29
    { 512, 512, 512},  //duty=10, dutyLt=29
    { 512, 512, 512},  //duty=11, dutyLt=29
    { 512, 512, 512},  //duty=12, dutyLt=29
    { 512, 512, 512},  //duty=13, dutyLt=29
    { 512, 512, 512},  //duty=14, dutyLt=29
    { 512, 512, 512},  //duty=15, dutyLt=29
    { 512, 512, 512},  //duty=16, dutyLt=29
    { 512, 512, 512},  //duty=17, dutyLt=29
    { 512, 512, 512},  //duty=18, dutyLt=29
    { 512, 512, 512},  //duty=19, dutyLt=29
    { 512, 512, 512},  //duty=20, dutyLt=29
    { 512, 512, 512},  //duty=21, dutyLt=29
    { 512, 512, 512},  //duty=22, dutyLt=29
    { 512, 512, 512},  //duty=23, dutyLt=29
    { 512, 512, 512},  //duty=24, dutyLt=29
    { 512, 512, 512},  //duty=25, dutyLt=29
    { 512, 512, 512},  //duty=26, dutyLt=29
    { 512, 512, 512},  //duty=27, dutyLt=29
    { 512, 512, 512},  //duty=28, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    { 512, 512, 512},  //duty=29, dutyLt=29
    },
};
