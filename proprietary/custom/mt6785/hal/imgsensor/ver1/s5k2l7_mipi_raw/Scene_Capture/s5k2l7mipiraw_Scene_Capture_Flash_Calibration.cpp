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
#include "s5k2l7mipiraw_Scene_Capture.h"

const FLASH_CALIBRATION_NVRAM_T s5k2l7mipiraw_Flash_Calibration_0000 = {
    .engTab = {
        .exp = 10000,
        .afe_gain = 1024,
        .isp_gain = 1024,
        .distance = 0,
        .yTab = {
           0, 291, 605, 909,1204,1488,1770,2041,2309,2569,2825,3075,3318,3556,3788,4013,4237,4671,5076,5459,5818,6157,6474,6777,7052,7307,7548,7758,7941,8087,8248,
         238, 562, 875,1179,1475,1761,2044,2317,2586,2850,3105,3356,3601,3840,4072,4300,4518,4958,5363,5747,6106,6448,6756,7051,7329,7583,7815,8018,8206,8374,  -1,
         510, 828,1146,1451,1750,2036,2319,2593,2864,3126,3383,3634,3879,4117,4351,4576,4798,5234,5644,6023,6379,6717,7035,7325,7601,7860,8087,8286,8457,8639,  -1,
         767,1090,1408,1715,2014,2302,2586,2862,3132,3395,3652,3902,4147,4387,4618,4846,5062,5502,5904,6282,6640,6977,7291,7582,7854,8100,8320,8528,8708,  -1,  -1,
        1024,1347,1668,1974,2275,2563,2849,3122,3391,3656,3912,4162,4407,4646,4877,5102,5319,5756,6158,6534,6893,7224,7537,7818,8097,8342,8561,8758,8933,  -1,  -1,
        1271,1594,1915,2224,2525,2813,3098,3373,3642,3906,4162,4413,4658,4894,5126,5352,5569,6014,6406,6784,7138,7468,7774,8063,8338,8581,8803,8989,  -1,  -1,  -1,
        1511,1841,2162,2472,2772,3062,3347,3621,3891,4153,4409,4662,4902,5141,5371,5594,5812,6248,6646,7023,7375,7703,8008,8292,8568,8808,9029,9220,  -1,  -1,  -1,
        1743,2073,2395,2706,3007,3296,3582,3857,4126,4389,4644,4893,5135,5375,5602,5827,6042,6475,6875,7248,7603,7929,8229,8514,8780,9024,9240,  -1,  -1,  -1,  -1,
        1973,2303,2627,2938,3239,3530,3815,4088,4359,4621,4876,5124,5366,5601,5831,6052,6269,6700,7098,7470,7820,8150,8450,8730,9001,9240,9455,  -1,  -1,  -1,  -1,
        2192,2524,2849,3160,3462,3751,4037,4311,4580,4843,5098,5343,5586,5822,6049,6270,6487,6917,7314,7686,8031,8357,8661,8940,9203,9443,  -1,  -1,  -1,  -1,  -1,
        2410,2742,3066,3379,3680,3969,4255,4528,4797,5057,5312,5560,5798,6036,6262,6485,6700,7132,7523,7892,8240,8567,8863,9138,9408,9642,  -1,  -1,  -1,  -1,  -1,
        2619,2953,3278,3590,3892,4182,4466,4739,5009,5269,5523,5769,6009,6243,6470,6691,6904,7339,7729,8097,8441,8770,9065,9342,9620,  -1,  -1,  -1,  -1,  -1,  -1,
        2829,3163,3490,3800,4103,4391,4676,4948,5216,5475,5730,5977,6219,6451,6677,6898,7108,7545,7933,8307,8648,8962,9258,9549,9834,  -1,  -1,  -1,  -1,  -1,  -1,
        3028,3361,3690,3999,4302,4591,4874,5146,5413,5673,5926,6171,6414,6643,6869,7088,7298,7732,8118,8485,8831,9148,9448,9718,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3224,3557,3886,4196,4498,4786,5072,5341,5609,5867,6118,6362,6599,6834,7057,7275,7486,7918,8307,8673,9017,9328,9619,9891,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3407,3747,4069,4381,4684,4972,5254,5526,5792,6050,6305,6545,6785,7013,7240,7458,7667,8096,8485,8844,9196,9501,9809,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3599,3932,4259,4568,4869,5157,5439,5709,5978,6233,6484,6727,6962,7191,7414,7630,7839,8273,8656,9015,9352,9664,9953,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3952,4290,4616,4929,5234,5520,5801,6074,6339,6591,6845,7088,7326,7554,7781,7995,8201,8610,8990,9352,9704,9999,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4294,4627,4953,5263,5562,5848,6135,6402,6663,6921,7168,7417,7645,7878,8101,8316,8520,8919,9299,9653,9983,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4600,4938,5261,5573,5873,6159,6440,6707,6967,7220,7468,7708,7947,8173,8400,8608,8812,9211,9580,9937,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4885,5229,5567,5861,6162,6440,6728,7000,7253,7509,7754,7991,8219,8442,8661,8886,9082,9499,9849,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5174,5503,5820,6129,6431,6709,6991,7253,7509,7760,8009,8245,8486,8703,8930,9129,9322,9716,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5416,5749,6071,6377,6680,6947,7223,7482,7749,7998,8237,8472,8699,8919,9135,9349,9552,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5638,5965,6289,6594,6887,7170,7448,7734,7976,8210,8464,8688,8916,9134,9347,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5835,6155,6480,6800,7073,7362,7628,7883,8167,8403,8636,8865,9095,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6037,6360,6687,6992,7307,7579,7817,8074,8339,8569,8810,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6207,6523,6852,7149,7437,7704,7987,8250,8476,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6365,6677,6991,7291,7565,7841,8119,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6491,6808,7124,7428,7795,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6649,6966,7270,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6731,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        },
    },
    .flashWBGain = {
    { 498, 512, 572},  //duty=-1, dutyLt=-1
    {1158, 512, 871},  //duty=0, dutyLt=-1
    {1168, 512, 871},  //duty=1, dutyLt=-1
    {1170, 512, 866},  //duty=2, dutyLt=-1
    {1169, 512, 862},  //duty=3, dutyLt=-1
    {1168, 512, 857},  //duty=4, dutyLt=-1
    {1166, 512, 851},  //duty=5, dutyLt=-1
    {1165, 512, 842},  //duty=6, dutyLt=-1
    {1163, 512, 820},  //duty=7, dutyLt=-1
    {1165, 512, 817},  //duty=8, dutyLt=-1
    {1165, 512, 815},  //duty=9, dutyLt=-1
    {1162, 512, 811},  //duty=10, dutyLt=-1
    {1163, 512, 809},  //duty=11, dutyLt=-1
    {1162, 512, 806},  //duty=12, dutyLt=-1
    {1163, 512, 804},  //duty=13, dutyLt=-1
    {1161, 512, 801},  //duty=14, dutyLt=-1
    {1161, 512, 799},  //duty=15, dutyLt=-1
    {1160, 512, 794},  //duty=16, dutyLt=-1
    {1159, 512, 789},  //duty=17, dutyLt=-1
    {1158, 512, 785},  //duty=18, dutyLt=-1
    {1158, 512, 781},  //duty=19, dutyLt=-1
    {1158, 512, 777},  //duty=20, dutyLt=-1
    {1158, 512, 773},  //duty=21, dutyLt=-1
    {1157, 512, 770},  //duty=22, dutyLt=-1
    {1157, 512, 766},  //duty=23, dutyLt=-1
    {1157, 512, 763},  //duty=24, dutyLt=-1
    {1157, 512, 759},  //duty=25, dutyLt=-1
    {1157, 512, 756},  //duty=26, dutyLt=-1
    {1158, 512, 753},  //duty=27, dutyLt=-1
    {1158, 512, 749},  //duty=28, dutyLt=-1
    {1160, 512, 746},  //duty=29, dutyLt=-1
    { 575, 512,1974},  //duty=-1, dutyLt=0
    { 811, 512,1157},  //duty=0, dutyLt=0
    { 911, 512,1035},  //duty=1, dutyLt=0
    { 965, 512, 983},  //duty=2, dutyLt=0
    { 999, 512, 952},  //duty=3, dutyLt=0
    {1021, 512, 930},  //duty=4, dutyLt=0
    {1038, 512, 910},  //duty=5, dutyLt=0
    {1053, 512, 883},  //duty=6, dutyLt=0
    {1062, 512, 874},  //duty=7, dutyLt=0
    {1069, 512, 866},  //duty=8, dutyLt=0
    {1076, 512, 860},  //duty=9, dutyLt=0
    {1083, 512, 854},  //duty=10, dutyLt=0
    {1087, 512, 849},  //duty=11, dutyLt=0
    {1091, 512, 844},  //duty=12, dutyLt=0
    {1094, 512, 839},  //duty=13, dutyLt=0
    {1097, 512, 835},  //duty=14, dutyLt=0
    {1099, 512, 832},  //duty=15, dutyLt=0
    {1104, 512, 824},  //duty=16, dutyLt=0
    {1107, 512, 817},  //duty=17, dutyLt=0
    {1110, 512, 812},  //duty=18, dutyLt=0
    {1112, 512, 806},  //duty=19, dutyLt=0
    {1114, 512, 801},  //duty=20, dutyLt=0
    {1115, 512, 797},  //duty=21, dutyLt=0
    {1116, 512, 792},  //duty=22, dutyLt=0
    {1119, 512, 787},  //duty=23, dutyLt=0
    {1120, 512, 783},  //duty=24, dutyLt=0
    {1122, 512, 779},  //duty=25, dutyLt=0
    {1123, 512, 775},  //duty=26, dutyLt=0
    {1124, 512, 772},  //duty=27, dutyLt=0
    {1125, 512, 768},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 575, 512,1980},  //duty=-1, dutyLt=1
    { 723, 512,1324},  //duty=0, dutyLt=1
    { 811, 512,1153},  //duty=1, dutyLt=1
    { 866, 512,1074},  //duty=2, dutyLt=1
    { 906, 512,1025},  //duty=3, dutyLt=1
    { 935, 512, 990},  //duty=4, dutyLt=1
    { 959, 512, 951},  //duty=5, dutyLt=1
    { 977, 512, 934},  //duty=6, dutyLt=1
    { 991, 512, 921},  //duty=7, dutyLt=1
    {1004, 512, 910},  //duty=8, dutyLt=1
    {1014, 512, 900},  //duty=9, dutyLt=1
    {1023, 512, 892},  //duty=10, dutyLt=1
    {1030, 512, 884},  //duty=11, dutyLt=1
    {1036, 512, 878},  //duty=12, dutyLt=1
    {1042, 512, 872},  //duty=13, dutyLt=1
    {1047, 512, 866},  //duty=14, dutyLt=1
    {1052, 512, 860},  //duty=15, dutyLt=1
    {1060, 512, 851},  //duty=16, dutyLt=1
    {1065, 512, 843},  //duty=17, dutyLt=1
    {1070, 512, 835},  //duty=18, dutyLt=1
    {1074, 512, 828},  //duty=19, dutyLt=1
    {1078, 512, 823},  //duty=20, dutyLt=1
    {1081, 512, 817},  //duty=21, dutyLt=1
    {1084, 512, 812},  //duty=22, dutyLt=1
    {1088, 512, 806},  //duty=23, dutyLt=1
    {1089, 512, 801},  //duty=24, dutyLt=1
    {1092, 512, 797},  //duty=25, dutyLt=1
    {1094, 512, 793},  //duty=26, dutyLt=1
    {1095, 512, 788},  //duty=27, dutyLt=1
    {1098, 512, 785},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 576, 512,2013},  //duty=-1, dutyLt=2
    { 684, 512,1433},  //duty=0, dutyLt=2
    { 757, 512,1243},  //duty=1, dutyLt=2
    { 809, 512,1147},  //duty=2, dutyLt=2
    { 848, 512,1084},  //duty=3, dutyLt=2
    { 879, 512,1031},  //duty=4, dutyLt=2
    { 903, 512,1002},  //duty=5, dutyLt=2
    { 923, 512, 982},  //duty=6, dutyLt=2
    { 940, 512, 964},  //duty=7, dutyLt=2
    { 954, 512, 950},  //duty=8, dutyLt=2
    { 965, 512, 937},  //duty=9, dutyLt=2
    { 977, 512, 926},  //duty=10, dutyLt=2
    { 985, 512, 917},  //duty=11, dutyLt=2
    { 993, 512, 908},  //duty=12, dutyLt=2
    {1000, 512, 900},  //duty=13, dutyLt=2
    {1007, 512, 894},  //duty=14, dutyLt=2
    {1013, 512, 887},  //duty=15, dutyLt=2
    {1022, 512, 875},  //duty=16, dutyLt=2
    {1030, 512, 865},  //duty=17, dutyLt=2
    {1037, 512, 857},  //duty=18, dutyLt=2
    {1043, 512, 849},  //duty=19, dutyLt=2
    {1048, 512, 842},  //duty=20, dutyLt=2
    {1052, 512, 836},  //duty=21, dutyLt=2
    {1056, 512, 830},  //duty=22, dutyLt=2
    {1060, 512, 823},  //duty=23, dutyLt=2
    {1063, 512, 818},  //duty=24, dutyLt=2
    {1065, 512, 813},  //duty=25, dutyLt=2
    {1068, 512, 809},  //duty=26, dutyLt=2
    {1070, 512, 804},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 575, 512,2001},  //duty=-1, dutyLt=3
    { 661, 512,1509},  //duty=0, dutyLt=3
    { 724, 512,1312},  //duty=1, dutyLt=3
    { 771, 512,1205},  //duty=2, dutyLt=3
    { 809, 512,1126},  //duty=3, dutyLt=3
    { 838, 512,1082},  //duty=4, dutyLt=3
    { 863, 512,1048},  //duty=5, dutyLt=3
    { 883, 512,1022},  //duty=6, dutyLt=3
    { 900, 512,1002},  //duty=7, dutyLt=3
    { 915, 512, 985},  //duty=8, dutyLt=3
    { 929, 512, 970},  //duty=9, dutyLt=3
    { 940, 512, 957},  //duty=10, dutyLt=3
    { 949, 512, 945},  //duty=11, dutyLt=3
    { 958, 512, 936},  //duty=12, dutyLt=3
    { 966, 512, 926},  //duty=13, dutyLt=3
    { 973, 512, 919},  //duty=14, dutyLt=3
    { 980, 512, 911},  //duty=15, dutyLt=3
    { 991, 512, 897},  //duty=16, dutyLt=3
    {1001, 512, 886},  //duty=17, dutyLt=3
    {1008, 512, 876},  //duty=18, dutyLt=3
    {1015, 512, 867},  //duty=19, dutyLt=3
    {1021, 512, 860},  //duty=20, dutyLt=3
    {1025, 512, 853},  //duty=21, dutyLt=3
    {1030, 512, 846},  //duty=22, dutyLt=3
    {1035, 512, 839},  //duty=23, dutyLt=3
    {1039, 512, 833},  //duty=24, dutyLt=3
    {1042, 512, 828},  //duty=25, dutyLt=3
    {1045, 512, 823},  //duty=26, dutyLt=3
    {1048, 512, 818},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 575, 512,1996},  //duty=-1, dutyLt=4
    { 647, 512,1562},  //duty=0, dutyLt=4
    { 702, 512,1365},  //duty=1, dutyLt=4
    { 745, 512,1246},  //duty=2, dutyLt=4
    { 780, 512,1173},  //duty=3, dutyLt=4
    { 808, 512,1123},  //duty=4, dutyLt=4
    { 832, 512,1088},  //duty=5, dutyLt=4
    { 852, 512,1059},  //duty=6, dutyLt=4
    { 869, 512,1036},  //duty=7, dutyLt=4
    { 885, 512,1016},  //duty=8, dutyLt=4
    { 898, 512, 999},  //duty=9, dutyLt=4
    { 909, 512, 985},  //duty=10, dutyLt=4
    { 920, 512, 972},  //duty=11, dutyLt=4
    { 930, 512, 961},  //duty=12, dutyLt=4
    { 938, 512, 951},  //duty=13, dutyLt=4
    { 945, 512, 942},  //duty=14, dutyLt=4
    { 953, 512, 933},  //duty=15, dutyLt=4
    { 965, 512, 918},  //duty=16, dutyLt=4
    { 975, 512, 906},  //duty=17, dutyLt=4
    { 984, 512, 895},  //duty=18, dutyLt=4
    { 991, 512, 885},  //duty=19, dutyLt=4
    { 997, 512, 876},  //duty=20, dutyLt=4
    {1003, 512, 868},  //duty=21, dutyLt=4
    {1008, 512, 861},  //duty=22, dutyLt=4
    {1013, 512, 854},  //duty=23, dutyLt=4
    {1017, 512, 847},  //duty=24, dutyLt=4
    {1021, 512, 842},  //duty=25, dutyLt=4
    {1024, 512, 836},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 575, 512,1992},  //duty=-1, dutyLt=5
    { 636, 512,1602},  //duty=0, dutyLt=5
    { 686, 512,1404},  //duty=1, dutyLt=5
    { 726, 512,1287},  //duty=2, dutyLt=5
    { 758, 512,1214},  //duty=3, dutyLt=5
    { 784, 512,1164},  //duty=4, dutyLt=5
    { 807, 512,1125},  //duty=5, dutyLt=5
    { 827, 512,1093},  //duty=6, dutyLt=5
    { 844, 512,1067},  //duty=7, dutyLt=5
    { 859, 512,1045},  //duty=8, dutyLt=5
    { 873, 512,1027},  //duty=9, dutyLt=5
    { 884, 512,1010},  //duty=10, dutyLt=5
    { 895, 512, 997},  //duty=11, dutyLt=5
    { 905, 512, 984},  //duty=12, dutyLt=5
    { 913, 512, 973},  //duty=13, dutyLt=5
    { 922, 512, 963},  //duty=14, dutyLt=5
    { 929, 512, 954},  //duty=15, dutyLt=5
    { 942, 512, 937},  //duty=16, dutyLt=5
    { 952, 512, 923},  //duty=17, dutyLt=5
    { 962, 512, 912},  //duty=18, dutyLt=5
    { 970, 512, 901},  //duty=19, dutyLt=5
    { 977, 512, 891},  //duty=20, dutyLt=5
    { 983, 512, 883},  //duty=21, dutyLt=5
    { 988, 512, 875},  //duty=22, dutyLt=5
    { 994, 512, 868},  //duty=23, dutyLt=5
    { 998, 512, 861},  //duty=24, dutyLt=5
    {1002, 512, 855},  //duty=25, dutyLt=5
    {1005, 512, 849},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 575, 512,1987},  //duty=-1, dutyLt=6
    { 630, 512,1630},  //duty=0, dutyLt=6
    { 674, 512,1438},  //duty=1, dutyLt=6
    { 710, 512,1324},  //duty=2, dutyLt=6
    { 740, 512,1252},  //duty=3, dutyLt=6
    { 766, 512,1198},  //duty=4, dutyLt=6
    { 788, 512,1155},  //duty=5, dutyLt=6
    { 807, 512,1122},  //duty=6, dutyLt=6
    { 824, 512,1094},  //duty=7, dutyLt=6
    { 839, 512,1071},  //duty=8, dutyLt=6
    { 852, 512,1051},  //duty=9, dutyLt=6
    { 864, 512,1034},  //duty=10, dutyLt=6
    { 874, 512,1019},  //duty=11, dutyLt=6
    { 885, 512,1006},  //duty=12, dutyLt=6
    { 893, 512, 993},  //duty=13, dutyLt=6
    { 901, 512, 983},  //duty=14, dutyLt=6
    { 909, 512, 973},  //duty=15, dutyLt=6
    { 922, 512, 955},  //duty=16, dutyLt=6
    { 933, 512, 940},  //duty=17, dutyLt=6
    { 943, 512, 927},  //duty=18, dutyLt=6
    { 951, 512, 916},  //duty=19, dutyLt=6
    { 958, 512, 906},  //duty=20, dutyLt=6
    { 965, 512, 897},  //duty=21, dutyLt=6
    { 970, 512, 889},  //duty=22, dutyLt=6
    { 976, 512, 880},  //duty=23, dutyLt=6
    { 981, 512, 873},  //duty=24, dutyLt=6
    { 985, 512, 867},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 575, 512,1980},  //duty=-1, dutyLt=7
    { 626, 512,1641},  //duty=0, dutyLt=7
    { 665, 512,1469},  //duty=1, dutyLt=7
    { 698, 512,1361},  //duty=2, dutyLt=7
    { 726, 512,1283},  //duty=3, dutyLt=7
    { 750, 512,1228},  //duty=4, dutyLt=7
    { 772, 512,1183},  //duty=5, dutyLt=7
    { 790, 512,1149},  //duty=6, dutyLt=7
    { 807, 512,1120},  //duty=7, dutyLt=7
    { 821, 512,1095},  //duty=8, dutyLt=7
    { 834, 512,1074},  //duty=9, dutyLt=7
    { 846, 512,1056},  //duty=10, dutyLt=7
    { 857, 512,1040},  //duty=11, dutyLt=7
    { 866, 512,1025},  //duty=12, dutyLt=7
    { 875, 512,1013},  //duty=13, dutyLt=7
    { 884, 512,1001},  //duty=14, dutyLt=7
    { 891, 512, 991},  //duty=15, dutyLt=7
    { 905, 512, 971},  //duty=16, dutyLt=7
    { 916, 512, 955},  //duty=17, dutyLt=7
    { 926, 512, 942},  //duty=18, dutyLt=7
    { 935, 512, 930},  //duty=19, dutyLt=7
    { 942, 512, 919},  //duty=20, dutyLt=7
    { 949, 512, 910},  //duty=21, dutyLt=7
    { 955, 512, 901},  //duty=22, dutyLt=7
    { 961, 512, 893},  //duty=23, dutyLt=7
    { 966, 512, 885},  //duty=24, dutyLt=7
    { 970, 512, 878},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 576, 512,1972},  //duty=-1, dutyLt=8
    { 620, 512,1672},  //duty=0, dutyLt=8
    { 658, 512,1495},  //duty=1, dutyLt=8
    { 688, 512,1391},  //duty=2, dutyLt=8
    { 715, 512,1310},  //duty=3, dutyLt=8
    { 738, 512,1255},  //duty=4, dutyLt=8
    { 758, 512,1209},  //duty=5, dutyLt=8
    { 776, 512,1173},  //duty=6, dutyLt=8
    { 792, 512,1142},  //duty=7, dutyLt=8
    { 806, 512,1117},  //duty=8, dutyLt=8
    { 819, 512,1095},  //duty=9, dutyLt=8
    { 830, 512,1075},  //duty=10, dutyLt=8
    { 841, 512,1059},  //duty=11, dutyLt=8
    { 851, 512,1044},  //duty=12, dutyLt=8
    { 860, 512,1030},  //duty=13, dutyLt=8
    { 868, 512,1018},  //duty=14, dutyLt=8
    { 875, 512,1007},  //duty=15, dutyLt=8
    { 889, 512, 987},  //duty=16, dutyLt=8
    { 901, 512, 970},  //duty=17, dutyLt=8
    { 911, 512, 955},  //duty=18, dutyLt=8
    { 920, 512, 943},  //duty=19, dutyLt=8
    { 928, 512, 932},  //duty=20, dutyLt=8
    { 935, 512, 922},  //duty=21, dutyLt=8
    { 941, 512, 913},  //duty=22, dutyLt=8
    { 947, 512, 904},  //duty=23, dutyLt=8
    { 951, 512, 896},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 576, 512,1965},  //duty=-1, dutyLt=9
    { 617, 512,1692},  //duty=0, dutyLt=9
    { 651, 512,1523},  //duty=1, dutyLt=9
    { 680, 512,1415},  //duty=2, dutyLt=9
    { 706, 512,1337},  //duty=3, dutyLt=9
    { 727, 512,1278},  //duty=4, dutyLt=9
    { 747, 512,1233},  //duty=5, dutyLt=9
    { 764, 512,1196},  //duty=6, dutyLt=9
    { 779, 512,1164},  //duty=7, dutyLt=9
    { 793, 512,1137},  //duty=8, dutyLt=9
    { 806, 512,1114},  //duty=9, dutyLt=9
    { 817, 512,1094},  //duty=10, dutyLt=9
    { 828, 512,1077},  //duty=11, dutyLt=9
    { 837, 512,1060},  //duty=12, dutyLt=9
    { 846, 512,1047},  //duty=13, dutyLt=9
    { 855, 512,1034},  //duty=14, dutyLt=9
    { 862, 512,1022},  //duty=15, dutyLt=9
    { 876, 512,1001},  //duty=16, dutyLt=9
    { 888, 512, 984},  //duty=17, dutyLt=9
    { 898, 512, 969},  //duty=18, dutyLt=9
    { 907, 512, 956},  //duty=19, dutyLt=9
    { 915, 512, 944},  //duty=20, dutyLt=9
    { 922, 512, 933},  //duty=21, dutyLt=9
    { 928, 512, 924},  //duty=22, dutyLt=9
    { 934, 512, 914},  //duty=23, dutyLt=9
    { 939, 512, 906},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 576, 512,1965},  //duty=-1, dutyLt=10
    { 614, 512,1704},  //duty=0, dutyLt=10
    { 646, 512,1544},  //duty=1, dutyLt=10
    { 673, 512,1436},  //duty=2, dutyLt=10
    { 697, 512,1360},  //duty=3, dutyLt=10
    { 718, 512,1301},  //duty=4, dutyLt=10
    { 737, 512,1254},  //duty=5, dutyLt=10
    { 753, 512,1215},  //duty=6, dutyLt=10
    { 768, 512,1183},  //duty=7, dutyLt=10
    { 782, 512,1156},  //duty=8, dutyLt=10
    { 795, 512,1132},  //duty=9, dutyLt=10
    { 806, 512,1111},  //duty=10, dutyLt=10
    { 816, 512,1093},  //duty=11, dutyLt=10
    { 825, 512,1076},  //duty=12, dutyLt=10
    { 834, 512,1062},  //duty=13, dutyLt=10
    { 842, 512,1048},  //duty=14, dutyLt=10
    { 850, 512,1037},  //duty=15, dutyLt=10
    { 864, 512,1015},  //duty=16, dutyLt=10
    { 875, 512, 997},  //duty=17, dutyLt=10
    { 886, 512, 981},  //duty=18, dutyLt=10
    { 895, 512, 967},  //duty=19, dutyLt=10
    { 903, 512, 955},  //duty=20, dutyLt=10
    { 910, 512, 944},  //duty=21, dutyLt=10
    { 916, 512, 934},  //duty=22, dutyLt=10
    { 922, 512, 925},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 576, 512,1964},  //duty=-1, dutyLt=11
    { 611, 512,1715},  //duty=0, dutyLt=11
    { 642, 512,1564},  //duty=1, dutyLt=11
    { 667, 512,1456},  //duty=2, dutyLt=11
    { 690, 512,1381},  //duty=3, dutyLt=11
    { 710, 512,1320},  //duty=4, dutyLt=11
    { 728, 512,1272},  //duty=5, dutyLt=11
    { 744, 512,1234},  //duty=6, dutyLt=11
    { 759, 512,1201},  //duty=7, dutyLt=11
    { 772, 512,1173},  //duty=8, dutyLt=11
    { 784, 512,1149},  //duty=9, dutyLt=11
    { 795, 512,1127},  //duty=10, dutyLt=11
    { 805, 512,1108},  //duty=11, dutyLt=11
    { 815, 512,1091},  //duty=12, dutyLt=11
    { 824, 512,1076},  //duty=13, dutyLt=11
    { 832, 512,1062},  //duty=14, dutyLt=11
    { 839, 512,1050},  //duty=15, dutyLt=11
    { 853, 512,1027},  //duty=16, dutyLt=11
    { 865, 512,1009},  //duty=17, dutyLt=11
    { 875, 512, 993},  //duty=18, dutyLt=11
    { 884, 512, 978},  //duty=19, dutyLt=11
    { 892, 512, 966},  //duty=20, dutyLt=11
    { 899, 512, 954},  //duty=21, dutyLt=11
    { 905, 512, 945},  //duty=22, dutyLt=11
    { 912, 512, 935},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 576, 512,1964},  //duty=-1, dutyLt=12
    { 609, 512,1726},  //duty=0, dutyLt=12
    { 638, 512,1578},  //duty=1, dutyLt=12
    { 663, 512,1475},  //duty=2, dutyLt=12
    { 684, 512,1398},  //duty=3, dutyLt=12
    { 703, 512,1338},  //duty=4, dutyLt=12
    { 721, 512,1290},  //duty=5, dutyLt=12
    { 736, 512,1251},  //duty=6, dutyLt=12
    { 751, 512,1218},  //duty=7, dutyLt=12
    { 764, 512,1189},  //duty=8, dutyLt=12
    { 775, 512,1164},  //duty=9, dutyLt=12
    { 787, 512,1142},  //duty=10, dutyLt=12
    { 796, 512,1123},  //duty=11, dutyLt=12
    { 805, 512,1105},  //duty=12, dutyLt=12
    { 814, 512,1090},  //duty=13, dutyLt=12
    { 822, 512,1075},  //duty=14, dutyLt=12
    { 829, 512,1063},  //duty=15, dutyLt=12
    { 843, 512,1039},  //duty=16, dutyLt=12
    { 855, 512,1020},  //duty=17, dutyLt=12
    { 865, 512,1003},  //duty=18, dutyLt=12
    { 874, 512, 988},  //duty=19, dutyLt=12
    { 882, 512, 976},  //duty=20, dutyLt=12
    { 889, 512, 964},  //duty=21, dutyLt=12
    { 896, 512, 954},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 512, 512, 512},  //duty=28, dutyLt=12
    { 512, 512, 512},  //duty=29, dutyLt=12
    { 577, 512,1962},  //duty=-1, dutyLt=13
    { 607, 512,1736},  //duty=0, dutyLt=13
    { 635, 512,1591},  //duty=1, dutyLt=13
    { 658, 512,1490},  //duty=2, dutyLt=13
    { 679, 512,1414},  //duty=3, dutyLt=13
    { 698, 512,1355},  //duty=4, dutyLt=13
    { 714, 512,1306},  //duty=5, dutyLt=13
    { 730, 512,1267},  //duty=6, dutyLt=13
    { 743, 512,1232},  //duty=7, dutyLt=13
    { 756, 512,1203},  //duty=8, dutyLt=13
    { 767, 512,1178},  //duty=9, dutyLt=13
    { 778, 512,1155},  //duty=10, dutyLt=13
    { 788, 512,1136},  //duty=11, dutyLt=13
    { 797, 512,1118},  //duty=12, dutyLt=13
    { 805, 512,1102},  //duty=13, dutyLt=13
    { 813, 512,1088},  //duty=14, dutyLt=13
    { 820, 512,1074},  //duty=15, dutyLt=13
    { 834, 512,1050},  //duty=16, dutyLt=13
    { 846, 512,1030},  //duty=17, dutyLt=13
    { 856, 512,1013},  //duty=18, dutyLt=13
    { 865, 512, 998},  //duty=19, dutyLt=13
    { 873, 512, 985},  //duty=20, dutyLt=13
    { 880, 512, 973},  //duty=21, dutyLt=13
    { 887, 512, 962},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 577, 512,1956},  //duty=-1, dutyLt=14
    { 606, 512,1746},  //duty=0, dutyLt=14
    { 632, 512,1602},  //duty=1, dutyLt=14
    { 654, 512,1505},  //duty=2, dutyLt=14
    { 675, 512,1428},  //duty=3, dutyLt=14
    { 692, 512,1369},  //duty=4, dutyLt=14
    { 708, 512,1320},  //duty=5, dutyLt=14
    { 723, 512,1280},  //duty=6, dutyLt=14
    { 736, 512,1246},  //duty=7, dutyLt=14
    { 749, 512,1217},  //duty=8, dutyLt=14
    { 760, 512,1191},  //duty=9, dutyLt=14
    { 771, 512,1168},  //duty=10, dutyLt=14
    { 780, 512,1148},  //duty=11, dutyLt=14
    { 789, 512,1130},  //duty=12, dutyLt=14
    { 797, 512,1114},  //duty=13, dutyLt=14
    { 805, 512,1099},  //duty=14, dutyLt=14
    { 813, 512,1085},  //duty=15, dutyLt=14
    { 826, 512,1060},  //duty=16, dutyLt=14
    { 838, 512,1040},  //duty=17, dutyLt=14
    { 848, 512,1023},  //duty=18, dutyLt=14
    { 857, 512,1008},  //duty=19, dutyLt=14
    { 865, 512, 994},  //duty=20, dutyLt=14
    { 872, 512, 982},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 512, 512, 512},  //duty=28, dutyLt=14
    { 512, 512, 512},  //duty=29, dutyLt=14
    { 577, 512,1952},  //duty=-1, dutyLt=15
    { 605, 512,1754},  //duty=0, dutyLt=15
    { 629, 512,1614},  //duty=1, dutyLt=15
    { 651, 512,1518},  //duty=2, dutyLt=15
    { 671, 512,1442},  //duty=3, dutyLt=15
    { 688, 512,1383},  //duty=4, dutyLt=15
    { 703, 512,1334},  //duty=5, dutyLt=15
    { 717, 512,1294},  //duty=6, dutyLt=15
    { 731, 512,1259},  //duty=7, dutyLt=15
    { 743, 512,1229},  //duty=8, dutyLt=15
    { 754, 512,1202},  //duty=9, dutyLt=15
    { 764, 512,1180},  //duty=10, dutyLt=15
    { 773, 512,1159},  //duty=11, dutyLt=15
    { 782, 512,1141},  //duty=12, dutyLt=15
    { 791, 512,1124},  //duty=13, dutyLt=15
    { 798, 512,1109},  //duty=14, dutyLt=15
    { 805, 512,1096},  //duty=15, dutyLt=15
    { 819, 512,1071},  //duty=16, dutyLt=15
    { 830, 512,1050},  //duty=17, dutyLt=15
    { 840, 512,1032},  //duty=18, dutyLt=15
    { 849, 512,1016},  //duty=19, dutyLt=15
    { 857, 512,1002},  //duty=20, dutyLt=15
    { 864, 512, 990},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 577, 512,1943},  //duty=-1, dutyLt=16
    { 603, 512,1762},  //duty=0, dutyLt=16
    { 626, 512,1631},  //duty=1, dutyLt=16
    { 646, 512,1536},  //duty=2, dutyLt=16
    { 664, 512,1463},  //duty=3, dutyLt=16
    { 680, 512,1404},  //duty=4, dutyLt=16
    { 695, 512,1356},  //duty=5, dutyLt=16
    { 708, 512,1315},  //duty=6, dutyLt=16
    { 721, 512,1280},  //duty=7, dutyLt=16
    { 732, 512,1250},  //duty=8, dutyLt=16
    { 743, 512,1224},  //duty=9, dutyLt=16
    { 753, 512,1200},  //duty=10, dutyLt=16
    { 762, 512,1179},  //duty=11, dutyLt=16
    { 770, 512,1160},  //duty=12, dutyLt=16
    { 778, 512,1143},  //duty=13, dutyLt=16
    { 786, 512,1127},  //duty=14, dutyLt=16
    { 793, 512,1113},  //duty=15, dutyLt=16
    { 806, 512,1088},  //duty=16, dutyLt=16
    { 817, 512,1067},  //duty=17, dutyLt=16
    { 827, 512,1049},  //duty=18, dutyLt=16
    { 835, 512,1033},  //duty=19, dutyLt=16
    { 843, 512,1018},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 512, 512, 512},  //duty=28, dutyLt=16
    { 512, 512, 512},  //duty=29, dutyLt=16
    { 577, 512,1938},  //duty=-1, dutyLt=17
    { 601, 512,1768},  //duty=0, dutyLt=17
    { 623, 512,1645},  //duty=1, dutyLt=17
    { 641, 512,1554},  //duty=2, dutyLt=17
    { 658, 512,1481},  //duty=3, dutyLt=17
    { 674, 512,1423},  //duty=4, dutyLt=17
    { 688, 512,1376},  //duty=5, dutyLt=17
    { 701, 512,1335},  //duty=6, dutyLt=17
    { 713, 512,1300},  //duty=7, dutyLt=17
    { 723, 512,1269},  //duty=8, dutyLt=17
    { 734, 512,1242},  //duty=9, dutyLt=17
    { 743, 512,1219},  //duty=10, dutyLt=17
    { 752, 512,1197},  //duty=11, dutyLt=17
    { 760, 512,1178},  //duty=12, dutyLt=17
    { 768, 512,1161},  //duty=13, dutyLt=17
    { 775, 512,1144},  //duty=14, dutyLt=17
    { 782, 512,1130},  //duty=15, dutyLt=17
    { 795, 512,1104},  //duty=16, dutyLt=17
    { 806, 512,1082},  //duty=17, dutyLt=17
    { 815, 512,1063},  //duty=18, dutyLt=17
    { 824, 512,1046},  //duty=19, dutyLt=17
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
    { 578, 512,1936},  //duty=-1, dutyLt=18
    { 600, 512,1773},  //duty=0, dutyLt=18
    { 620, 512,1655},  //duty=1, dutyLt=18
    { 638, 512,1567},  //duty=2, dutyLt=18
    { 654, 512,1497},  //duty=3, dutyLt=18
    { 668, 512,1440},  //duty=4, dutyLt=18
    { 682, 512,1391},  //duty=5, dutyLt=18
    { 694, 512,1352},  //duty=6, dutyLt=18
    { 706, 512,1317},  //duty=7, dutyLt=18
    { 716, 512,1285},  //duty=8, dutyLt=18
    { 726, 512,1257},  //duty=9, dutyLt=18
    { 735, 512,1234},  //duty=10, dutyLt=18
    { 744, 512,1213},  //duty=11, dutyLt=18
    { 752, 512,1193},  //duty=12, dutyLt=18
    { 759, 512,1176},  //duty=13, dutyLt=18
    { 766, 512,1159},  //duty=14, dutyLt=18
    { 773, 512,1144},  //duty=15, dutyLt=18
    { 785, 512,1118},  //duty=16, dutyLt=18
    { 796, 512,1096},  //duty=17, dutyLt=18
    { 806, 512,1076},  //duty=18, dutyLt=18
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
    { 578, 512,1929},  //duty=-1, dutyLt=19
    { 599, 512,1776},  //duty=0, dutyLt=19
    { 618, 512,1665},  //duty=1, dutyLt=19
    { 635, 512,1578},  //duty=2, dutyLt=19
    { 650, 512,1509},  //duty=3, dutyLt=19
    { 664, 512,1454},  //duty=4, dutyLt=19
    { 677, 512,1406},  //duty=5, dutyLt=19
    { 689, 512,1366},  //duty=6, dutyLt=19
    { 700, 512,1330},  //duty=7, dutyLt=19
    { 710, 512,1300},  //duty=8, dutyLt=19
    { 720, 512,1273},  //duty=9, dutyLt=19
    { 729, 512,1248},  //duty=10, dutyLt=19
    { 737, 512,1226},  //duty=11, dutyLt=19
    { 745, 512,1206},  //duty=12, dutyLt=19
    { 752, 512,1188},  //duty=13, dutyLt=19
    { 759, 512,1172},  //duty=14, dutyLt=19
    { 766, 512,1157},  //duty=15, dutyLt=19
    { 777, 512,1131},  //duty=16, dutyLt=19
    { 788, 512,1108},  //duty=17, dutyLt=19
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
    { 579, 512,1921},  //duty=-1, dutyLt=20
    { 599, 512,1779},  //duty=0, dutyLt=20
    { 617, 512,1670},  //duty=1, dutyLt=20
    { 633, 512,1587},  //duty=2, dutyLt=20
    { 647, 512,1520},  //duty=3, dutyLt=20
    { 661, 512,1465},  //duty=4, dutyLt=20
    { 673, 512,1418},  //duty=5, dutyLt=20
    { 685, 512,1378},  //duty=6, dutyLt=20
    { 695, 512,1343},  //duty=7, dutyLt=20
    { 705, 512,1312},  //duty=8, dutyLt=20
    { 714, 512,1285},  //duty=9, dutyLt=20
    { 723, 512,1260},  //duty=10, dutyLt=20
    { 731, 512,1238},  //duty=11, dutyLt=20
    { 739, 512,1218},  //duty=12, dutyLt=20
    { 746, 512,1201},  //duty=13, dutyLt=20
    { 753, 512,1184},  //duty=14, dutyLt=20
    { 759, 512,1168},  //duty=15, dutyLt=20
    { 771, 512,1142},  //duty=16, dutyLt=20
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
    { 579, 512,1919},  //duty=-1, dutyLt=21
    { 599, 512,1781},  //duty=0, dutyLt=21
    { 616, 512,1675},  //duty=1, dutyLt=21
    { 631, 512,1596},  //duty=2, dutyLt=21
    { 645, 512,1529},  //duty=3, dutyLt=21
    { 658, 512,1474},  //duty=4, dutyLt=21
    { 670, 512,1427},  //duty=5, dutyLt=21
    { 681, 512,1387},  //duty=6, dutyLt=21
    { 691, 512,1353},  //duty=7, dutyLt=21
    { 701, 512,1322},  //duty=8, dutyLt=21
    { 710, 512,1295},  //duty=9, dutyLt=21
    { 719, 512,1270},  //duty=10, dutyLt=21
    { 726, 512,1249},  //duty=11, dutyLt=21
    { 734, 512,1228},  //duty=12, dutyLt=21
    { 741, 512,1210},  //duty=13, dutyLt=21
    { 747, 512,1194},  //duty=14, dutyLt=21
    { 753, 512,1179},  //duty=15, dutyLt=21
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
    { 580, 512,1913},  //duty=-1, dutyLt=22
    { 598, 512,1779},  //duty=0, dutyLt=22
    { 615, 512,1679},  //duty=1, dutyLt=22
    { 630, 512,1600},  //duty=2, dutyLt=22
    { 644, 512,1535},  //duty=3, dutyLt=22
    { 656, 512,1482},  //duty=4, dutyLt=22
    { 667, 512,1436},  //duty=5, dutyLt=22
    { 677, 512,1398},  //duty=6, dutyLt=22
    { 688, 512,1363},  //duty=7, dutyLt=22
    { 698, 512,1331},  //duty=8, dutyLt=22
    { 706, 512,1304},  //duty=9, dutyLt=22
    { 715, 512,1280},  //duty=10, dutyLt=22
    { 722, 512,1258},  //duty=11, dutyLt=22
    { 729, 512,1237},  //duty=12, dutyLt=22
    { 736, 512,1219},  //duty=13, dutyLt=22
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
    { 580, 512,1908},  //duty=-1, dutyLt=23
    { 598, 512,1780},  //duty=0, dutyLt=23
    { 614, 512,1682},  //duty=1, dutyLt=23
    { 628, 512,1605},  //duty=2, dutyLt=23
    { 642, 512,1540},  //duty=3, dutyLt=23
    { 654, 512,1488},  //duty=4, dutyLt=23
    { 666, 512,1443},  //duty=5, dutyLt=23
    { 676, 512,1403},  //duty=6, dutyLt=23
    { 685, 512,1371},  //duty=7, dutyLt=23
    { 695, 512,1339},  //duty=8, dutyLt=23
    { 703, 512,1311},  //duty=9, dutyLt=23
    { 712, 512,1286},  //duty=10, dutyLt=23
    { 719, 512,1265},  //duty=11, dutyLt=23
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
    { 581, 512,1904},  //duty=-1, dutyLt=24
    { 598, 512,1779},  //duty=0, dutyLt=24
    { 614, 512,1685},  //duty=1, dutyLt=24
    { 628, 512,1609},  //duty=2, dutyLt=24
    { 640, 512,1548},  //duty=3, dutyLt=24
    { 652, 512,1495},  //duty=4, dutyLt=24
    { 664, 512,1448},  //duty=5, dutyLt=24
    { 674, 512,1409},  //duty=6, dutyLt=24
    { 683, 512,1376},  //duty=7, dutyLt=24
    { 692, 512,1345},  //duty=8, dutyLt=24
    { 701, 512,1318},  //duty=9, dutyLt=24
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
    { 582, 512,1898},  //duty=-1, dutyLt=25
    { 599, 512,1778},  //duty=0, dutyLt=25
    { 614, 512,1685},  //duty=1, dutyLt=25
    { 627, 512,1611},  //duty=2, dutyLt=25
    { 640, 512,1549},  //duty=3, dutyLt=25
    { 651, 512,1497},  //duty=4, dutyLt=25
    { 662, 512,1453},  //duty=5, dutyLt=25
    { 672, 512,1415},  //duty=6, dutyLt=25
    { 682, 512,1380},  //duty=7, dutyLt=25
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
    { 583, 512,1892},  //duty=-1, dutyLt=26
    { 599, 512,1776},  //duty=0, dutyLt=26
    { 614, 512,1685},  //duty=1, dutyLt=26
    { 627, 512,1612},  //duty=2, dutyLt=26
    { 640, 512,1550},  //duty=3, dutyLt=26
    { 651, 512,1500},  //duty=4, dutyLt=26
    { 661, 512,1456},  //duty=5, dutyLt=26
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
    { 583, 512,1886},  //duty=-1, dutyLt=27
    { 600, 512,1774},  //duty=0, dutyLt=27
    { 614, 512,1684},  //duty=1, dutyLt=27
    { 627, 512,1614},  //duty=2, dutyLt=27
    { 637, 512,1558},  //duty=3, dutyLt=27
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
    { 584, 512,1882},  //duty=-1, dutyLt=28
    { 599, 512,1773},  //duty=0, dutyLt=28
    { 614, 512,1686},  //duty=1, dutyLt=28
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
    { 585, 512,1876},  //duty=-1, dutyLt=29
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
