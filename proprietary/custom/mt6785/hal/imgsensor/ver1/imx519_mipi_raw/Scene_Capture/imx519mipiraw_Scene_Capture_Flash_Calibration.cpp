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
#include "imx519mipiraw_Scene_Capture.h"

const FLASH_CALIBRATION_NVRAM_T imx519mipiraw_Flash_Calibration_0000 = {
    .engTab = {
        .exp = 10000,
        .afe_gain = 1024,
        .isp_gain = 1024,
        .distance = 0,
        .yTab = {
           0, 301, 626, 946,1258,1558,1852,2136,2414,2682,2947,3204,3453,3698,3936,4168,4393,4843,5256,5649,6017,6371,6697,6989,7287,7555,7786,8003,8209,8385,8527,
         258, 580, 908,1229,1537,1836,2131,2414,2692,2960,3222,3481,3727,3971,4210,4439,4666,5115,5527,5920,6290,6631,6954,7253,7525,7784,8031,8283,8446,8615,  -1,
         531, 856,1188,1503,1815,2111,2405,2687,2964,3231,3495,3749,3997,4240,4476,4706,4931,5375,5785,6176,6534,6879,7196,7494,7770,8022,8261,8480,8665,8840,  -1,
         792,1119,1446,1765,2074,2372,2662,2944,3220,3485,3748,4003,4250,4490,4726,4954,5176,5622,6031,6418,6774,7115,7430,7725,7999,8254,8491,8701,8886,  -1,  -1,
        1049,1372,1701,2020,2328,2626,2916,3196,3471,3735,3996,4249,4496,4736,4970,5197,5420,5862,6267,6652,7010,7350,7658,7952,8224,8479,8709,8915,9106,  -1,  -1,
        1294,1616,1947,2263,2573,2867,3158,3438,3712,3975,4237,4490,4733,4972,5208,5437,5654,6094,6498,6880,7238,7572,7885,8177,8449,8692,8942,9136,  -1,  -1,  -1,
        1533,1858,2188,2505,2813,3108,3397,3676,3949,4213,4473,4725,4967,5206,5440,5667,5885,6324,6730,7105,7467,7794,8103,8394,8665,8921,9151,9360,  -1,  -1,  -1,
        1762,2089,2417,2735,3041,3339,3628,3907,4178,4442,4699,4951,5194,5433,5664,5890,6106,6545,6948,7325,7679,8012,8323,8611,8879,9122,9358,  -1,  -1,  -1,  -1,
        1985,2313,2642,2959,3266,3561,3849,4127,4400,4662,4920,5172,5412,5650,5882,6104,6327,6761,7163,7535,7890,8219,8527,8817,9081,9320,9552,  -1,  -1,  -1,  -1,
        2201,2528,2858,3174,3483,3778,4065,4343,4615,4875,5133,5384,5622,5862,6090,6313,6531,6968,7365,7740,8094,8420,8723,9008,9285,9517,  -1,  -1,  -1,  -1,  -1,
        2417,2742,3072,3387,3694,3988,4276,4553,4824,5085,5343,5593,5834,6068,6298,6520,6736,7170,7572,7939,8300,8621,8927,9208,9468,9713,  -1,  -1,  -1,  -1,  -1,
        2619,2947,3276,3592,3900,4192,4481,4758,5028,5289,5547,5793,6034,6269,6498,6719,6935,7371,7768,8140,8484,8821,9125,9395,9662,  -1,  -1,  -1,  -1,  -1,  -1,
        2822,3148,3478,3794,4101,4393,4682,4958,5226,5486,5742,5989,6230,6465,6691,6918,7128,7564,7956,8330,8675,8997,9302,9578,9852,  -1,  -1,  -1,  -1,  -1,  -1,
        3012,3341,3668,3986,4291,4583,4871,5147,5416,5679,5931,6178,6415,6648,6877,7096,7308,7748,8137,8508,8856,9179,9479,9761,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3202,3533,3861,4174,4480,4773,5062,5337,5604,5863,6116,6365,6600,6832,7061,7286,7494,7929,8323,8686,9032,9350,9643,9918,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3382,3711,4038,4356,4661,4954,5240,5515,5782,6041,6295,6541,6777,7008,7233,7452,7663,8097,8487,8851,9212,9521,9814,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3563,3891,4218,4532,4838,5132,5417,5690,5959,6214,6470,6714,6949,7181,7403,7621,7833,8270,8654,9025,9361,9683,9976,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3909,4242,4572,4888,5193,5489,5772,6047,6316,6576,6829,7070,7308,7546,7762,7981,8195,8601,8983,9348,9686,9999,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4236,4566,4898,5214,5517,5812,6092,6365,6634,6889,7147,7388,7621,7854,8077,8289,8504,8916,9308,9652,9991,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4546,4877,5205,5517,5821,6114,6398,6667,6931,7191,7440,7682,7915,8142,8370,8586,8788,9196,9573,9932,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4828,5161,5492,5804,6107,6402,6678,6950,7218,7468,7715,7955,8186,8419,8638,8856,9061,9456,9834,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5094,5426,5751,6072,6378,6657,6940,7207,7467,7720,7969,8207,8452,8670,8893,9099,9304,9704,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5350,5673,6000,6311,6611,6899,7176,7445,7707,7957,8200,8440,8666,8893,9108,9322,9520,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5566,5892,6239,6535,6833,7118,7400,7660,7921,8165,8413,8654,8878,9104,9317,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5768,6097,6424,6721,7022,7316,7583,7846,8098,8354,8594,8828,9050,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5963,6296,6622,6923,7212,7495,7766,8039,8302,8538,8777,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6154,6471,6794,7098,7396,7675,7934,8214,8466,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6319,6629,6949,7252,7541,7815,8093,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6458,6766,7099,7380,7679,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6586,6906,7211,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6708,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        },
    },
    .flashWBGain = {
    {   0, 512,   0},  //duty=-1, dutyLt=-1
    {1185, 512,1092},  //duty=0, dutyLt=-1
    {1189, 512,1096},  //duty=1, dutyLt=-1
    {1182, 512,1088},  //duty=2, dutyLt=-1
    {1179, 512,1084},  //duty=3, dutyLt=-1
    {1177, 512,1078},  //duty=4, dutyLt=-1
    {1175, 512,1073},  //duty=5, dutyLt=-1
    {1174, 512,1058},  //duty=6, dutyLt=-1
    {1181, 512,1080},  //duty=7, dutyLt=-1
    {1180, 512,1075},  //duty=8, dutyLt=-1
    {1177, 512,1071},  //duty=9, dutyLt=-1
    {1177, 512,1066},  //duty=10, dutyLt=-1
    {1176, 512,1063},  //duty=11, dutyLt=-1
    {1174, 512,1059},  //duty=12, dutyLt=-1
    {1174, 512,1055},  //duty=13, dutyLt=-1
    {1174, 512,1053},  //duty=14, dutyLt=-1
    {1172, 512,1050},  //duty=15, dutyLt=-1
    {1171, 512,1044},  //duty=16, dutyLt=-1
    {1171, 512,1039},  //duty=17, dutyLt=-1
    {1170, 512,1033},  //duty=18, dutyLt=-1
    {1170, 512,1028},  //duty=19, dutyLt=-1
    {1169, 512,1023},  //duty=20, dutyLt=-1
    {1169, 512,1018},  //duty=21, dutyLt=-1
    {1169, 512,1013},  //duty=22, dutyLt=-1
    {1168, 512,1009},  //duty=23, dutyLt=-1
    {1168, 512,1004},  //duty=24, dutyLt=-1
    {1169, 512, 998},  //duty=25, dutyLt=-1
    {1169, 512, 993},  //duty=26, dutyLt=-1
    {1169, 512, 988},  //duty=27, dutyLt=-1
    {1170, 512, 984},  //duty=28, dutyLt=-1
    {1170, 512, 978},  //duty=29, dutyLt=-1
    { 532, 512,3280},  //duty=-1, dutyLt=0
    { 777, 512,1558},  //duty=0, dutyLt=0
    { 886, 512,1341},  //duty=1, dutyLt=0
    { 946, 512,1258},  //duty=2, dutyLt=0
    { 983, 512,1216},  //duty=3, dutyLt=0
    {1009, 512,1186},  //duty=4, dutyLt=0
    {1027, 512,1156},  //duty=5, dutyLt=0
    {1048, 512,1161},  //duty=6, dutyLt=0
    {1058, 512,1149},  //duty=7, dutyLt=0
    {1066, 512,1138},  //duty=8, dutyLt=0
    {1074, 512,1129},  //duty=9, dutyLt=0
    {1080, 512,1121},  //duty=10, dutyLt=0
    {1085, 512,1113},  //duty=11, dutyLt=0
    {1089, 512,1107},  //duty=12, dutyLt=0
    {1093, 512,1100},  //duty=13, dutyLt=0
    {1096, 512,1095},  //duty=14, dutyLt=0
    {1099, 512,1091},  //duty=15, dutyLt=0
    {1104, 512,1081},  //duty=16, dutyLt=0
    {1109, 512,1073},  //duty=17, dutyLt=0
    {1111, 512,1065},  //duty=18, dutyLt=0
    {1115, 512,1059},  //duty=19, dutyLt=0
    {1117, 512,1052},  //duty=20, dutyLt=0
    {1118, 512,1045},  //duty=21, dutyLt=0
    {1120, 512,1040},  //duty=22, dutyLt=0
    {1123, 512,1033},  //duty=23, dutyLt=0
    {1125, 512,1027},  //duty=24, dutyLt=0
    {1126, 512,1021},  //duty=25, dutyLt=0
    {1128, 512,1017},  //duty=26, dutyLt=0
    {1129, 512,1010},  //duty=27, dutyLt=0
    {1130, 512,1004},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 532, 512,3260},  //duty=-1, dutyLt=1
    { 683, 512,1844},  //duty=0, dutyLt=1
    { 776, 512,1528},  //duty=1, dutyLt=1
    { 838, 512,1401},  //duty=2, dutyLt=1
    { 881, 512,1328},  //duty=3, dutyLt=1
    { 913, 512,1275},  //duty=4, dutyLt=1
    { 944, 512,1265},  //duty=5, dutyLt=1
    { 962, 512,1235},  //duty=6, dutyLt=1
    { 979, 512,1214},  //duty=7, dutyLt=1
    { 991, 512,1197},  //duty=8, dutyLt=1
    {1001, 512,1183},  //duty=9, dutyLt=1
    {1011, 512,1170},  //duty=10, dutyLt=1
    {1019, 512,1160},  //duty=11, dutyLt=1
    {1027, 512,1150},  //duty=12, dutyLt=1
    {1033, 512,1142},  //duty=13, dutyLt=1
    {1039, 512,1135},  //duty=14, dutyLt=1
    {1044, 512,1128},  //duty=15, dutyLt=1
    {1053, 512,1115},  //duty=16, dutyLt=1
    {1060, 512,1104},  //duty=17, dutyLt=1
    {1066, 512,1095},  //duty=18, dutyLt=1
    {1071, 512,1085},  //duty=19, dutyLt=1
    {1075, 512,1077},  //duty=20, dutyLt=1
    {1079, 512,1070},  //duty=21, dutyLt=1
    {1082, 512,1063},  //duty=22, dutyLt=1
    {1086, 512,1055},  //duty=23, dutyLt=1
    {1088, 512,1048},  //duty=24, dutyLt=1
    {1091, 512,1042},  //duty=25, dutyLt=1
    {1093, 512,1036},  //duty=26, dutyLt=1
    {1095, 512,1029},  //duty=27, dutyLt=1
    {1097, 512,1023},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 532, 512,3352},  //duty=-1, dutyLt=2
    { 642, 512,2028},  //duty=0, dutyLt=2
    { 721, 512,1674},  //duty=1, dutyLt=2
    { 777, 512,1516},  //duty=2, dutyLt=2
    { 819, 512,1419},  //duty=3, dutyLt=2
    { 855, 512,1379},  //duty=4, dutyLt=2
    { 881, 512,1333},  //duty=5, dutyLt=2
    { 903, 512,1300},  //duty=6, dutyLt=2
    { 920, 512,1272},  //duty=7, dutyLt=2
    { 935, 512,1251},  //duty=8, dutyLt=2
    { 949, 512,1232},  //duty=9, dutyLt=2
    { 960, 512,1216},  //duty=10, dutyLt=2
    { 970, 512,1202},  //duty=11, dutyLt=2
    { 979, 512,1190},  //duty=12, dutyLt=2
    { 987, 512,1180},  //duty=13, dutyLt=2
    { 994, 512,1171},  //duty=14, dutyLt=2
    {1000, 512,1162},  //duty=15, dutyLt=2
    {1012, 512,1146},  //duty=16, dutyLt=2
    {1021, 512,1133},  //duty=17, dutyLt=2
    {1029, 512,1122},  //duty=18, dutyLt=2
    {1035, 512,1110},  //duty=19, dutyLt=2
    {1040, 512,1101},  //duty=20, dutyLt=2
    {1045, 512,1092},  //duty=21, dutyLt=2
    {1050, 512,1085},  //duty=22, dutyLt=2
    {1054, 512,1076},  //duty=23, dutyLt=2
    {1058, 512,1069},  //duty=24, dutyLt=2
    {1062, 512,1062},  //duty=25, dutyLt=2
    {1064, 512,1054},  //duty=26, dutyLt=2
    {1066, 512,1047},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 532, 512,3306},  //duty=-1, dutyLt=3
    { 619, 512,2166},  //duty=0, dutyLt=3
    { 686, 512,1792},  //duty=1, dutyLt=3
    { 737, 512,1611},  //duty=2, dutyLt=3
    { 779, 512,1525},  //duty=3, dutyLt=3
    { 811, 512,1458},  //duty=4, dutyLt=3
    { 836, 512,1396},  //duty=5, dutyLt=3
    { 859, 512,1357},  //duty=6, dutyLt=3
    { 878, 512,1326},  //duty=7, dutyLt=3
    { 893, 512,1300},  //duty=8, dutyLt=3
    { 908, 512,1278},  //duty=9, dutyLt=3
    { 920, 512,1257},  //duty=10, dutyLt=3
    { 931, 512,1241},  //duty=11, dutyLt=3
    { 940, 512,1227},  //duty=12, dutyLt=3
    { 950, 512,1215},  //duty=13, dutyLt=3
    { 957, 512,1204},  //duty=14, dutyLt=3
    { 965, 512,1194},  //duty=15, dutyLt=3
    { 978, 512,1175},  //duty=16, dutyLt=3
    { 988, 512,1160},  //duty=17, dutyLt=3
    { 997, 512,1146},  //duty=18, dutyLt=3
    {1005, 512,1134},  //duty=19, dutyLt=3
    {1011, 512,1124},  //duty=20, dutyLt=3
    {1017, 512,1114},  //duty=21, dutyLt=3
    {1022, 512,1105},  //duty=22, dutyLt=3
    {1027, 512,1096},  //duty=23, dutyLt=3
    {1031, 512,1087},  //duty=24, dutyLt=3
    {1034, 512,1079},  //duty=25, dutyLt=3
    {1038, 512,1072},  //duty=26, dutyLt=3
    {1041, 512,1065},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 531, 512,3292},  //duty=-1, dutyLt=4
    { 605, 512,2270},  //duty=0, dutyLt=4
    { 663, 512,1888},  //duty=1, dutyLt=4
    { 710, 512,1686},  //duty=2, dutyLt=4
    { 747, 512,1593},  //duty=3, dutyLt=4
    { 778, 512,1519},  //duty=4, dutyLt=4
    { 803, 512,1458},  //duty=5, dutyLt=4
    { 825, 512,1411},  //duty=6, dutyLt=4
    { 844, 512,1373},  //duty=7, dutyLt=4
    { 860, 512,1343},  //duty=8, dutyLt=4
    { 874, 512,1317},  //duty=9, dutyLt=4
    { 887, 512,1296},  //duty=10, dutyLt=4
    { 899, 512,1279},  //duty=11, dutyLt=4
    { 909, 512,1262},  //duty=12, dutyLt=4
    { 918, 512,1248},  //duty=13, dutyLt=4
    { 927, 512,1236},  //duty=14, dutyLt=4
    { 935, 512,1224},  //duty=15, dutyLt=4
    { 949, 512,1203},  //duty=16, dutyLt=4
    { 960, 512,1186},  //duty=17, dutyLt=4
    { 970, 512,1170},  //duty=18, dutyLt=4
    { 978, 512,1157},  //duty=19, dutyLt=4
    { 986, 512,1145},  //duty=20, dutyLt=4
    { 992, 512,1134},  //duty=21, dutyLt=4
    { 997, 512,1125},  //duty=22, dutyLt=4
    {1003, 512,1114},  //duty=23, dutyLt=4
    {1007, 512,1106},  //duty=24, dutyLt=4
    {1012, 512,1098},  //duty=25, dutyLt=4
    {1015, 512,1089},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 531, 512,3272},  //duty=-1, dutyLt=5
    { 594, 512,2350},  //duty=0, dutyLt=5
    { 647, 512,1966},  //duty=1, dutyLt=5
    { 690, 512,1801},  //duty=2, dutyLt=5
    { 724, 512,1665},  //duty=3, dutyLt=5
    { 752, 512,1571},  //duty=4, dutyLt=5
    { 777, 512,1511},  //duty=5, dutyLt=5
    { 798, 512,1461},  //duty=6, dutyLt=5
    { 817, 512,1419},  //duty=7, dutyLt=5
    { 833, 512,1385},  //duty=8, dutyLt=5
    { 847, 512,1357},  //duty=9, dutyLt=5
    { 861, 512,1333},  //duty=10, dutyLt=5
    { 872, 512,1312},  //duty=11, dutyLt=5
    { 883, 512,1294},  //duty=12, dutyLt=5
    { 893, 512,1279},  //duty=13, dutyLt=5
    { 901, 512,1265},  //duty=14, dutyLt=5
    { 909, 512,1252},  //duty=15, dutyLt=5
    { 924, 512,1229},  //duty=16, dutyLt=5
    { 936, 512,1210},  //duty=17, dutyLt=5
    { 946, 512,1193},  //duty=18, dutyLt=5
    { 955, 512,1179},  //duty=19, dutyLt=5
    { 963, 512,1166},  //duty=20, dutyLt=5
    { 970, 512,1154},  //duty=21, dutyLt=5
    { 976, 512,1144},  //duty=22, dutyLt=5
    { 982, 512,1132},  //duty=23, dutyLt=5
    { 986, 512,1123},  //duty=24, dutyLt=5
    { 991, 512,1114},  //duty=25, dutyLt=5
    { 995, 512,1106},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 531, 512,3248},  //duty=-1, dutyLt=6
    { 587, 512,2414},  //duty=0, dutyLt=6
    { 636, 512,2016},  //duty=1, dutyLt=6
    { 673, 512,1854},  //duty=2, dutyLt=6
    { 705, 512,1732},  //duty=3, dutyLt=6
    { 732, 512,1627},  //duty=4, dutyLt=6
    { 756, 512,1553},  //duty=5, dutyLt=6
    { 777, 512,1501},  //duty=6, dutyLt=6
    { 795, 512,1458},  //duty=7, dutyLt=6
    { 810, 512,1423},  //duty=8, dutyLt=6
    { 825, 512,1393},  //duty=9, dutyLt=6
    { 838, 512,1367},  //duty=10, dutyLt=6
    { 850, 512,1343},  //duty=11, dutyLt=6
    { 861, 512,1325},  //duty=12, dutyLt=6
    { 870, 512,1309},  //duty=13, dutyLt=6
    { 880, 512,1293},  //duty=14, dutyLt=6
    { 888, 512,1278},  //duty=15, dutyLt=6
    { 903, 512,1253},  //duty=16, dutyLt=6
    { 915, 512,1233},  //duty=17, dutyLt=6
    { 926, 512,1214},  //duty=18, dutyLt=6
    { 935, 512,1199},  //duty=19, dutyLt=6
    { 943, 512,1185},  //duty=20, dutyLt=6
    { 950, 512,1172},  //duty=21, dutyLt=6
    { 956, 512,1161},  //duty=22, dutyLt=6
    { 963, 512,1150},  //duty=23, dutyLt=6
    { 968, 512,1139},  //duty=24, dutyLt=6
    { 972, 512,1130},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 531, 512,3231},  //duty=-1, dutyLt=7
    { 582, 512,2457},  //duty=0, dutyLt=7
    { 625, 512,2142},  //duty=1, dutyLt=7
    { 660, 512,1912},  //duty=2, dutyLt=7
    { 691, 512,1775},  //duty=3, dutyLt=7
    { 716, 512,1681},  //duty=4, dutyLt=7
    { 739, 512,1602},  //duty=5, dutyLt=7
    { 759, 512,1542},  //duty=6, dutyLt=7
    { 777, 512,1495},  //duty=7, dutyLt=7
    { 792, 512,1458},  //duty=8, dutyLt=7
    { 807, 512,1426},  //duty=9, dutyLt=7
    { 819, 512,1398},  //duty=10, dutyLt=7
    { 831, 512,1375},  //duty=11, dutyLt=7
    { 842, 512,1353},  //duty=12, dutyLt=7
    { 852, 512,1335},  //duty=13, dutyLt=7
    { 861, 512,1319},  //duty=14, dutyLt=7
    { 869, 512,1304},  //duty=15, dutyLt=7
    { 884, 512,1276},  //duty=16, dutyLt=7
    { 897, 512,1254},  //duty=17, dutyLt=7
    { 908, 512,1235},  //duty=18, dutyLt=7
    { 917, 512,1218},  //duty=19, dutyLt=7
    { 925, 512,1203},  //duty=20, dutyLt=7
    { 933, 512,1190},  //duty=21, dutyLt=7
    { 939, 512,1178},  //duty=22, dutyLt=7
    { 946, 512,1166},  //duty=23, dutyLt=7
    { 951, 512,1155},  //duty=24, dutyLt=7
    { 956, 512,1145},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 532, 512,3216},  //duty=-1, dutyLt=8
    { 578, 512,2565},  //duty=0, dutyLt=8
    { 617, 512,2174},  //duty=1, dutyLt=8
    { 650, 512,1976},  //duty=2, dutyLt=8
    { 679, 512,1814},  //duty=3, dutyLt=8
    { 703, 512,1717},  //duty=4, dutyLt=8
    { 725, 512,1644},  //duty=5, dutyLt=8
    { 744, 512,1583},  //duty=6, dutyLt=8
    { 761, 512,1533},  //duty=7, dutyLt=8
    { 777, 512,1492},  //duty=8, dutyLt=8
    { 791, 512,1458},  //duty=9, dutyLt=8
    { 803, 512,1428},  //duty=10, dutyLt=8
    { 815, 512,1403},  //duty=11, dutyLt=8
    { 825, 512,1380},  //duty=12, dutyLt=8
    { 835, 512,1361},  //duty=13, dutyLt=8
    { 844, 512,1343},  //duty=14, dutyLt=8
    { 852, 512,1327},  //duty=15, dutyLt=8
    { 868, 512,1298},  //duty=16, dutyLt=8
    { 881, 512,1275},  //duty=17, dutyLt=8
    { 892, 512,1254},  //duty=18, dutyLt=8
    { 901, 512,1236},  //duty=19, dutyLt=8
    { 910, 512,1221},  //duty=20, dutyLt=8
    { 917, 512,1207},  //duty=21, dutyLt=8
    { 924, 512,1194},  //duty=22, dutyLt=8
    { 931, 512,1181},  //duty=23, dutyLt=8
    { 936, 512,1170},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 533, 512,3196},  //duty=-1, dutyLt=9
    { 574, 512,2597},  //duty=0, dutyLt=9
    { 610, 512,2224},  //duty=1, dutyLt=9
    { 641, 512,2010},  //duty=2, dutyLt=9
    { 669, 512,1868},  //duty=3, dutyLt=9
    { 692, 512,1754},  //duty=4, dutyLt=9
    { 713, 512,1675},  //duty=5, dutyLt=9
    { 731, 512,1615},  //duty=6, dutyLt=9
    { 748, 512,1564},  //duty=7, dutyLt=9
    { 763, 512,1522},  //duty=8, dutyLt=9
    { 777, 512,1486},  //duty=9, dutyLt=9
    { 789, 512,1455},  //duty=10, dutyLt=9
    { 801, 512,1428},  //duty=11, dutyLt=9
    { 811, 512,1406},  //duty=12, dutyLt=9
    { 821, 512,1385},  //duty=13, dutyLt=9
    { 830, 512,1367},  //duty=14, dutyLt=9
    { 838, 512,1350},  //duty=15, dutyLt=9
    { 853, 512,1319},  //duty=16, dutyLt=9
    { 866, 512,1294},  //duty=17, dutyLt=9
    { 877, 512,1272},  //duty=18, dutyLt=9
    { 887, 512,1254},  //duty=19, dutyLt=9
    { 896, 512,1238},  //duty=20, dutyLt=9
    { 903, 512,1223},  //duty=21, dutyLt=9
    { 910, 512,1209},  //duty=22, dutyLt=9
    { 917, 512,1196},  //duty=23, dutyLt=9
    { 922, 512,1184},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 533, 512,3274},  //duty=-1, dutyLt=10
    { 571, 512,2615},  //duty=0, dutyLt=10
    { 605, 512,2267},  //duty=1, dutyLt=10
    { 634, 512,2049},  //duty=2, dutyLt=10
    { 660, 512,1904},  //duty=3, dutyLt=10
    { 682, 512,1797},  //duty=4, dutyLt=10
    { 702, 512,1712},  //duty=5, dutyLt=10
    { 720, 512,1645},  //duty=6, dutyLt=10
    { 736, 512,1593},  //duty=7, dutyLt=10
    { 751, 512,1550},  //duty=8, dutyLt=10
    { 764, 512,1513},  //duty=9, dutyLt=10
    { 777, 512,1481},  //duty=10, dutyLt=10
    { 788, 512,1454},  //duty=11, dutyLt=10
    { 799, 512,1430},  //duty=12, dutyLt=10
    { 808, 512,1408},  //duty=13, dutyLt=10
    { 817, 512,1389},  //duty=14, dutyLt=10
    { 825, 512,1371},  //duty=15, dutyLt=10
    { 840, 512,1339},  //duty=16, dutyLt=10
    { 853, 512,1313},  //duty=17, dutyLt=10
    { 864, 512,1290},  //duty=18, dutyLt=10
    { 874, 512,1270},  //duty=19, dutyLt=10
    { 883, 512,1254},  //duty=20, dutyLt=10
    { 891, 512,1239},  //duty=21, dutyLt=10
    { 898, 512,1224},  //duty=22, dutyLt=10
    { 904, 512,1210},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 533, 512,3268},  //duty=-1, dutyLt=11
    { 568, 512,2651},  //duty=0, dutyLt=11
    { 601, 512,2299},  //duty=1, dutyLt=11
    { 628, 512,2089},  //duty=2, dutyLt=11
    { 653, 512,1934},  //duty=3, dutyLt=11
    { 674, 512,1826},  //duty=4, dutyLt=11
    { 694, 512,1748},  //duty=5, dutyLt=11
    { 711, 512,1680},  //duty=6, dutyLt=11
    { 727, 512,1625},  //duty=7, dutyLt=11
    { 741, 512,1580},  //duty=8, dutyLt=11
    { 754, 512,1540},  //duty=9, dutyLt=11
    { 766, 512,1506},  //duty=10, dutyLt=11
    { 777, 512,1477},  //duty=11, dutyLt=11
    { 787, 512,1454},  //duty=12, dutyLt=11
    { 797, 512,1430},  //duty=13, dutyLt=11
    { 805, 512,1410},  //duty=14, dutyLt=11
    { 813, 512,1390},  //duty=15, dutyLt=11
    { 829, 512,1357},  //duty=16, dutyLt=11
    { 842, 512,1330},  //duty=17, dutyLt=11
    { 853, 512,1307},  //duty=18, dutyLt=11
    { 862, 512,1287},  //duty=19, dutyLt=11
    { 871, 512,1269},  //duty=20, dutyLt=11
    { 879, 512,1253},  //duty=21, dutyLt=11
    { 886, 512,1238},  //duty=22, dutyLt=11
    { 893, 512,1224},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 533, 512,3238},  //duty=-1, dutyLt=12
    { 567, 512,2670},  //duty=0, dutyLt=12
    { 597, 512,2333},  //duty=1, dutyLt=12
    { 623, 512,2116},  //duty=2, dutyLt=12
    { 647, 512,1970},  //duty=3, dutyLt=12
    { 667, 512,1856},  //duty=4, dutyLt=12
    { 686, 512,1770},  //duty=5, dutyLt=12
    { 703, 512,1703},  //duty=6, dutyLt=12
    { 718, 512,1649},  //duty=7, dutyLt=12
    { 732, 512,1602},  //duty=8, dutyLt=12
    { 745, 512,1562},  //duty=9, dutyLt=12
    { 756, 512,1528},  //duty=10, dutyLt=12
    { 767, 512,1500},  //duty=11, dutyLt=12
    { 777, 512,1474},  //duty=12, dutyLt=12
    { 786, 512,1449},  //duty=13, dutyLt=12
    { 795, 512,1429},  //duty=14, dutyLt=12
    { 803, 512,1409},  //duty=15, dutyLt=12
    { 818, 512,1376},  //duty=16, dutyLt=12
    { 831, 512,1347},  //duty=17, dutyLt=12
    { 842, 512,1323},  //duty=18, dutyLt=12
    { 852, 512,1301},  //duty=19, dutyLt=12
    { 861, 512,1283},  //duty=20, dutyLt=12
    { 869, 512,1267},  //duty=21, dutyLt=12
    { 876, 512,1251},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 512, 512, 512},  //duty=28, dutyLt=12
    { 512, 512, 512},  //duty=29, dutyLt=12
    { 533, 512,3241},  //duty=-1, dutyLt=13
    { 565, 512,2681},  //duty=0, dutyLt=13
    { 594, 512,2359},  //duty=1, dutyLt=13
    { 619, 512,2147},  //duty=2, dutyLt=13
    { 641, 512,1995},  //duty=3, dutyLt=13
    { 661, 512,1887},  //duty=4, dutyLt=13
    { 679, 512,1801},  //duty=5, dutyLt=13
    { 695, 512,1731},  //duty=6, dutyLt=13
    { 710, 512,1674},  //duty=7, dutyLt=13
    { 724, 512,1627},  //duty=8, dutyLt=13
    { 736, 512,1586},  //duty=9, dutyLt=13
    { 748, 512,1551},  //duty=10, dutyLt=13
    { 758, 512,1520},  //duty=11, dutyLt=13
    { 768, 512,1494},  //duty=12, dutyLt=13
    { 778, 512,1470},  //duty=13, dutyLt=13
    { 786, 512,1448},  //duty=14, dutyLt=13
    { 794, 512,1427},  //duty=15, dutyLt=13
    { 809, 512,1392},  //duty=16, dutyLt=13
    { 821, 512,1363},  //duty=17, dutyLt=13
    { 833, 512,1338},  //duty=18, dutyLt=13
    { 842, 512,1316},  //duty=19, dutyLt=13
    { 851, 512,1297},  //duty=20, dutyLt=13
    { 859, 512,1279},  //duty=21, dutyLt=13
    { 866, 512,1264},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 533, 512,3232},  //duty=-1, dutyLt=14
    { 563, 512,2697},  //duty=0, dutyLt=14
    { 591, 512,2383},  //duty=1, dutyLt=14
    { 615, 512,2173},  //duty=2, dutyLt=14
    { 636, 512,2022},  //duty=3, dutyLt=14
    { 656, 512,1909},  //duty=4, dutyLt=14
    { 673, 512,1824},  //duty=5, dutyLt=14
    { 689, 512,1755},  //duty=6, dutyLt=14
    { 704, 512,1698},  //duty=7, dutyLt=14
    { 717, 512,1648},  //duty=8, dutyLt=14
    { 729, 512,1606},  //duty=9, dutyLt=14
    { 740, 512,1570},  //duty=10, dutyLt=14
    { 750, 512,1540},  //duty=11, dutyLt=14
    { 760, 512,1513},  //duty=12, dutyLt=14
    { 769, 512,1487},  //duty=13, dutyLt=14
    { 777, 512,1465},  //duty=14, dutyLt=14
    { 786, 512,1444},  //duty=15, dutyLt=14
    { 800, 512,1408},  //duty=16, dutyLt=14
    { 813, 512,1378},  //duty=17, dutyLt=14
    { 824, 512,1352},  //duty=18, dutyLt=14
    { 834, 512,1330},  //duty=19, dutyLt=14
    { 842, 512,1310},  //duty=20, dutyLt=14
    { 850, 512,1292},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 512, 512, 512},  //duty=28, dutyLt=14
    { 512, 512, 512},  //duty=29, dutyLt=14
    { 533, 512,3217},  //duty=-1, dutyLt=15
    { 562, 512,2714},  //duty=0, dutyLt=15
    { 588, 512,2405},  //duty=1, dutyLt=15
    { 612, 512,2195},  //duty=2, dutyLt=15
    { 632, 512,2045},  //duty=3, dutyLt=15
    { 651, 512,1936},  //duty=4, dutyLt=15
    { 668, 512,1847},  //duty=5, dutyLt=15
    { 683, 512,1775},  //duty=6, dutyLt=15
    { 697, 512,1717},  //duty=7, dutyLt=15
    { 710, 512,1668},  //duty=8, dutyLt=15
    { 722, 512,1627},  //duty=9, dutyLt=15
    { 733, 512,1590},  //duty=10, dutyLt=15
    { 743, 512,1559},  //duty=11, dutyLt=15
    { 753, 512,1530},  //duty=12, dutyLt=15
    { 762, 512,1504},  //duty=13, dutyLt=15
    { 770, 512,1481},  //duty=14, dutyLt=15
    { 778, 512,1460},  //duty=15, dutyLt=15
    { 792, 512,1423},  //duty=16, dutyLt=15
    { 805, 512,1392},  //duty=17, dutyLt=15
    { 816, 512,1366},  //duty=18, dutyLt=15
    { 825, 512,1343},  //duty=19, dutyLt=15
    { 834, 512,1323},  //duty=20, dutyLt=15
    { 842, 512,1305},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 534, 512,3198},  //duty=-1, dutyLt=16
    { 560, 512,2733},  //duty=0, dutyLt=16
    { 585, 512,2442},  //duty=1, dutyLt=16
    { 606, 512,2234},  //duty=2, dutyLt=16
    { 625, 512,2088},  //duty=3, dutyLt=16
    { 643, 512,1978},  //duty=4, dutyLt=16
    { 659, 512,1888},  //duty=5, dutyLt=16
    { 673, 512,1816},  //duty=6, dutyLt=16
    { 687, 512,1756},  //duty=7, dutyLt=16
    { 699, 512,1706},  //duty=8, dutyLt=16
    { 710, 512,1662},  //duty=9, dutyLt=16
    { 721, 512,1625},  //duty=10, dutyLt=16
    { 731, 512,1592},  //duty=11, dutyLt=16
    { 740, 512,1563},  //duty=12, dutyLt=16
    { 749, 512,1535},  //duty=13, dutyLt=16
    { 757, 512,1511},  //duty=14, dutyLt=16
    { 765, 512,1489},  //duty=15, dutyLt=16
    { 779, 512,1451},  //duty=16, dutyLt=16
    { 791, 512,1419},  //duty=17, dutyLt=16
    { 801, 512,1391},  //duty=18, dutyLt=16
    { 811, 512,1367},  //duty=19, dutyLt=16
    { 820, 512,1346},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 512, 512, 512},  //duty=28, dutyLt=16
    { 512, 512, 512},  //duty=29, dutyLt=16
    { 534, 512,3183},  //duty=-1, dutyLt=17
    { 559, 512,2753},  //duty=0, dutyLt=17
    { 581, 512,2470},  //duty=1, dutyLt=17
    { 602, 512,2268},  //duty=2, dutyLt=17
    { 620, 512,2125},  //duty=3, dutyLt=17
    { 636, 512,2014},  //duty=4, dutyLt=17
    { 651, 512,1924},  //duty=5, dutyLt=17
    { 665, 512,1852},  //duty=6, dutyLt=17
    { 678, 512,1791},  //duty=7, dutyLt=17
    { 690, 512,1738},  //duty=8, dutyLt=17
    { 701, 512,1695},  //duty=9, dutyLt=17
    { 711, 512,1656},  //duty=10, dutyLt=17
    { 721, 512,1622},  //duty=11, dutyLt=17
    { 730, 512,1592},  //duty=12, dutyLt=17
    { 738, 512,1563},  //duty=13, dutyLt=17
    { 746, 512,1539},  //duty=14, dutyLt=17
    { 753, 512,1516},  //duty=15, dutyLt=17
    { 767, 512,1477},  //duty=16, dutyLt=17
    { 779, 512,1444},  //duty=17, dutyLt=17
    { 789, 512,1414},  //duty=18, dutyLt=17
    { 799, 512,1390},  //duty=19, dutyLt=17
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
    { 535, 512,3168},  //duty=-1, dutyLt=18
    { 558, 512,2766},  //duty=0, dutyLt=18
    { 579, 512,2492},  //duty=1, dutyLt=18
    { 598, 512,2298},  //duty=2, dutyLt=18
    { 615, 512,2158},  //duty=3, dutyLt=18
    { 631, 512,2046},  //duty=4, dutyLt=18
    { 645, 512,1955},  //duty=5, dutyLt=18
    { 658, 512,1882},  //duty=6, dutyLt=18
    { 671, 512,1820},  //duty=7, dutyLt=18
    { 682, 512,1768},  //duty=8, dutyLt=18
    { 693, 512,1723},  //duty=9, dutyLt=18
    { 703, 512,1684},  //duty=10, dutyLt=18
    { 712, 512,1650},  //duty=11, dutyLt=18
    { 721, 512,1617},  //duty=12, dutyLt=18
    { 729, 512,1589},  //duty=13, dutyLt=18
    { 736, 512,1564},  //duty=14, dutyLt=18
    { 744, 512,1540},  //duty=15, dutyLt=18
    { 757, 512,1499},  //duty=16, dutyLt=18
    { 769, 512,1465},  //duty=17, dutyLt=18
    { 779, 512,1435},  //duty=18, dutyLt=18
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
    { 535, 512,3154},  //duty=-1, dutyLt=19
    { 557, 512,2776},  //duty=0, dutyLt=19
    { 577, 512,2509},  //duty=1, dutyLt=19
    { 595, 512,2322},  //duty=2, dutyLt=19
    { 611, 512,2183},  //duty=3, dutyLt=19
    { 626, 512,2073},  //duty=4, dutyLt=19
    { 640, 512,1982},  //duty=5, dutyLt=19
    { 653, 512,1909},  //duty=6, dutyLt=19
    { 665, 512,1847},  //duty=7, dutyLt=19
    { 676, 512,1794},  //duty=8, dutyLt=19
    { 686, 512,1749},  //duty=9, dutyLt=19
    { 696, 512,1709},  //duty=10, dutyLt=19
    { 705, 512,1673},  //duty=11, dutyLt=19
    { 713, 512,1641},  //duty=12, dutyLt=19
    { 721, 512,1612},  //duty=13, dutyLt=19
    { 728, 512,1585},  //duty=14, dutyLt=19
    { 736, 512,1562},  //duty=15, dutyLt=19
    { 748, 512,1519},  //duty=16, dutyLt=19
    { 760, 512,1485},  //duty=17, dutyLt=19
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
    { 536, 512,3142},  //duty=-1, dutyLt=20
    { 556, 512,2780},  //duty=0, dutyLt=20
    { 576, 512,2523},  //duty=1, dutyLt=20
    { 593, 512,2344},  //duty=2, dutyLt=20
    { 608, 512,2205},  //duty=3, dutyLt=20
    { 623, 512,2095},  //duty=4, dutyLt=20
    { 636, 512,2005},  //duty=5, dutyLt=20
    { 648, 512,1931},  //duty=6, dutyLt=20
    { 660, 512,1869},  //duty=7, dutyLt=20
    { 670, 512,1817},  //duty=8, dutyLt=20
    { 681, 512,1771},  //duty=9, dutyLt=20
    { 690, 512,1730},  //duty=10, dutyLt=20
    { 698, 512,1696},  //duty=11, dutyLt=20
    { 707, 512,1662},  //duty=12, dutyLt=20
    { 714, 512,1633},  //duty=13, dutyLt=20
    { 722, 512,1605},  //duty=14, dutyLt=20
    { 729, 512,1581},  //duty=15, dutyLt=20
    { 741, 512,1539},  //duty=16, dutyLt=20
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
    { 536, 512,3135},  //duty=-1, dutyLt=21
    { 556, 512,2784},  //duty=0, dutyLt=21
    { 574, 512,2539},  //duty=1, dutyLt=21
    { 591, 512,2359},  //duty=2, dutyLt=21
    { 606, 512,2223},  //duty=3, dutyLt=21
    { 620, 512,2116},  //duty=4, dutyLt=21
    { 633, 512,2026},  //duty=5, dutyLt=21
    { 645, 512,1952},  //duty=6, dutyLt=21
    { 656, 512,1890},  //duty=7, dutyLt=21
    { 666, 512,1837},  //duty=8, dutyLt=21
    { 676, 512,1790},  //duty=9, dutyLt=21
    { 685, 512,1749},  //duty=10, dutyLt=21
    { 693, 512,1712},  //duty=11, dutyLt=21
    { 701, 512,1679},  //duty=12, dutyLt=21
    { 709, 512,1649},  //duty=13, dutyLt=21
    { 716, 512,1622},  //duty=14, dutyLt=21
    { 723, 512,1598},  //duty=15, dutyLt=21
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
    { 537, 512,3124},  //duty=-1, dutyLt=22
    { 556, 512,2782},  //duty=0, dutyLt=22
    { 573, 512,2552},  //duty=1, dutyLt=22
    { 590, 512,2372},  //duty=2, dutyLt=22
    { 604, 512,2238},  //duty=3, dutyLt=22
    { 617, 512,2131},  //duty=4, dutyLt=22
    { 630, 512,2043},  //duty=5, dutyLt=22
    { 642, 512,1969},  //duty=6, dutyLt=22
    { 652, 512,1906},  //duty=7, dutyLt=22
    { 662, 512,1854},  //duty=8, dutyLt=22
    { 672, 512,1807},  //duty=9, dutyLt=22
    { 681, 512,1766},  //duty=10, dutyLt=22
    { 689, 512,1729},  //duty=11, dutyLt=22
    { 697, 512,1696},  //duty=12, dutyLt=22
    { 704, 512,1665},  //duty=13, dutyLt=22
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
    { 538, 512,3107},  //duty=-1, dutyLt=23
    { 556, 512,2787},  //duty=0, dutyLt=23
    { 573, 512,2559},  //duty=1, dutyLt=23
    { 589, 512,2385},  //duty=2, dutyLt=23
    { 603, 512,2250},  //duty=3, dutyLt=23
    { 616, 512,2146},  //duty=4, dutyLt=23
    { 628, 512,2057},  //duty=5, dutyLt=23
    { 639, 512,1983},  //duty=6, dutyLt=23
    { 650, 512,1921},  //duty=7, dutyLt=23
    { 659, 512,1867},  //duty=8, dutyLt=23
    { 669, 512,1821},  //duty=9, dutyLt=23
    { 678, 512,1779},  //duty=10, dutyLt=23
    { 686, 512,1741},  //duty=11, dutyLt=23
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
    { 538, 512,3092},  //duty=-1, dutyLt=24
    { 556, 512,2790},  //duty=0, dutyLt=24
    { 572, 512,2562},  //duty=1, dutyLt=24
    { 588, 512,2397},  //duty=2, dutyLt=24
    { 601, 512,2263},  //duty=3, dutyLt=24
    { 614, 512,2157},  //duty=4, dutyLt=24
    { 626, 512,2069},  //duty=5, dutyLt=24
    { 637, 512,1997},  //duty=6, dutyLt=24
    { 647, 512,1935},  //duty=7, dutyLt=24
    { 657, 512,1881},  //duty=8, dutyLt=24
    { 666, 512,1835},  //duty=9, dutyLt=24
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
    { 539, 512,3084},  //duty=-1, dutyLt=25
    { 556, 512,2785},  //duty=0, dutyLt=25
    { 572, 512,2568},  //duty=1, dutyLt=25
    { 587, 512,2399},  //duty=2, dutyLt=25
    { 600, 512,2272},  //duty=3, dutyLt=25
    { 613, 512,2168},  //duty=4, dutyLt=25
    { 625, 512,2079},  //duty=5, dutyLt=25
    { 635, 512,2008},  //duty=6, dutyLt=25
    { 645, 512,1946},  //duty=7, dutyLt=25
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
    { 540, 512,3076},  //duty=-1, dutyLt=26
    { 557, 512,2780},  //duty=0, dutyLt=26
    { 572, 512,2570},  //duty=1, dutyLt=26
    { 587, 512,2406},  //duty=2, dutyLt=26
    { 600, 512,2278},  //duty=3, dutyLt=26
    { 612, 512,2173},  //duty=4, dutyLt=26
    { 623, 512,2089},  //duty=5, dutyLt=26
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
    { 541, 512,3058},  //duty=-1, dutyLt=27
    { 557, 512,2782},  //duty=0, dutyLt=27
    { 572, 512,2569},  //duty=1, dutyLt=27
    { 587, 512,2406},  //duty=2, dutyLt=27
    { 599, 512,2283},  //duty=3, dutyLt=27
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
    { 542, 512,3046},  //duty=-1, dutyLt=28
    { 558, 512,2771},  //duty=0, dutyLt=28
    { 573, 512,2569},  //duty=1, dutyLt=28
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
    { 543, 512,3033},  //duty=-1, dutyLt=29
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
