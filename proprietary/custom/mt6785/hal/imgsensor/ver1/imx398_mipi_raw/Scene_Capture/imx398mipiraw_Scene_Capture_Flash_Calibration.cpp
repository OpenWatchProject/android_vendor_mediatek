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
#include "imx398mipiraw_Scene_Capture.h"

const FLASH_CALIBRATION_NVRAM_T imx398mipiraw_Flash_Calibration_0000 = {
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
    { 535, 512, 697},  //duty=-1, dutyLt=-1
    {1194, 512, 953},  //duty=0, dutyLt=-1
    {1206, 512, 958},  //duty=1, dutyLt=-1
    {1201, 512, 952},  //duty=2, dutyLt=-1
    {1199, 512, 947},  //duty=3, dutyLt=-1
    {1198, 512, 944},  //duty=4, dutyLt=-1
    {1198, 512, 939},  //duty=5, dutyLt=-1
    {1198, 512, 923},  //duty=6, dutyLt=-1
    {1206, 512, 938},  //duty=7, dutyLt=-1
    {1206, 512, 934},  //duty=8, dutyLt=-1
    {1204, 512, 932},  //duty=9, dutyLt=-1
    {1204, 512, 928},  //duty=10, dutyLt=-1
    {1203, 512, 926},  //duty=11, dutyLt=-1
    {1201, 512, 923},  //duty=12, dutyLt=-1
    {1201, 512, 920},  //duty=13, dutyLt=-1
    {1199, 512, 918},  //duty=14, dutyLt=-1
    {1201, 512, 916},  //duty=15, dutyLt=-1
    {1199, 512, 910},  //duty=16, dutyLt=-1
    {1199, 512, 906},  //duty=17, dutyLt=-1
    {1198, 512, 902},  //duty=18, dutyLt=-1
    {1198, 512, 897},  //duty=19, dutyLt=-1
    {1198, 512, 892},  //duty=20, dutyLt=-1
    {1198, 512, 888},  //duty=21, dutyLt=-1
    {1198, 512, 884},  //duty=22, dutyLt=-1
    {1198, 512, 879},  //duty=23, dutyLt=-1
    {1198, 512, 874},  //duty=24, dutyLt=-1
    {1198, 512, 870},  //duty=25, dutyLt=-1
    {1198, 512, 865},  //duty=26, dutyLt=-1
    {1199, 512, 861},  //duty=27, dutyLt=-1
    {1200, 512, 856},  //duty=28, dutyLt=-1
    {1202, 512, 851},  //duty=29, dutyLt=-1
    { 529, 512,2791},  //duty=-1, dutyLt=0
    { 783, 512,1345},  //duty=0, dutyLt=0
    { 895, 512,1165},  //duty=1, dutyLt=0
    { 956, 512,1098},  //duty=2, dutyLt=0
    { 996, 512,1062},  //duty=3, dutyLt=0
    {1023, 512,1036},  //duty=4, dutyLt=0
    {1043, 512,1010},  //duty=5, dutyLt=0
    {1064, 512,1011},  //duty=6, dutyLt=0
    {1076, 512,1000},  //duty=7, dutyLt=0
    {1085, 512, 990},  //duty=8, dutyLt=0
    {1094, 512, 983},  //duty=9, dutyLt=0
    {1099, 512, 975},  //duty=10, dutyLt=0
    {1106, 512, 971},  //duty=11, dutyLt=0
    {1109, 512, 965},  //duty=12, dutyLt=0
    {1115, 512, 960},  //duty=13, dutyLt=0
    {1118, 512, 955},  //duty=14, dutyLt=0
    {1121, 512, 951},  //duty=15, dutyLt=0
    {1126, 512, 942},  //duty=16, dutyLt=0
    {1131, 512, 936},  //duty=17, dutyLt=0
    {1135, 512, 929},  //duty=18, dutyLt=0
    {1138, 512, 923},  //duty=19, dutyLt=0
    {1141, 512, 917},  //duty=20, dutyLt=0
    {1143, 512, 912},  //duty=21, dutyLt=0
    {1145, 512, 907},  //duty=22, dutyLt=0
    {1148, 512, 900},  //duty=23, dutyLt=0
    {1150, 512, 895},  //duty=24, dutyLt=0
    {1152, 512, 890},  //duty=25, dutyLt=0
    {1153, 512, 885},  //duty=26, dutyLt=0
    {1155, 512, 880},  //duty=27, dutyLt=0
    {1156, 512, 874},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 529, 512,2780},  //duty=-1, dutyLt=1
    { 685, 512,1584},  //duty=0, dutyLt=1
    { 782, 512,1326},  //duty=1, dutyLt=1
    { 845, 512,1220},  //duty=2, dutyLt=1
    { 890, 512,1159},  //duty=3, dutyLt=1
    { 924, 512,1112},  //duty=4, dutyLt=1
    { 953, 512,1096},  //duty=5, dutyLt=1
    { 974, 512,1074},  //duty=6, dutyLt=1
    { 992, 512,1056},  //duty=7, dutyLt=1
    {1004, 512,1041},  //duty=8, dutyLt=1
    {1016, 512,1031},  //duty=9, dutyLt=1
    {1028, 512,1019},  //duty=10, dutyLt=1
    {1035, 512,1012},  //duty=11, dutyLt=1
    {1044, 512,1003},  //duty=12, dutyLt=1
    {1050, 512, 995},  //duty=13, dutyLt=1
    {1057, 512, 989},  //duty=14, dutyLt=1
    {1062, 512, 983},  //duty=15, dutyLt=1
    {1072, 512, 972},  //duty=16, dutyLt=1
    {1080, 512, 962},  //duty=17, dutyLt=1
    {1086, 512, 955},  //duty=18, dutyLt=1
    {1092, 512, 947},  //duty=19, dutyLt=1
    {1096, 512, 939},  //duty=20, dutyLt=1
    {1100, 512, 933},  //duty=21, dutyLt=1
    {1104, 512, 927},  //duty=22, dutyLt=1
    {1108, 512, 919},  //duty=23, dutyLt=1
    {1111, 512, 913},  //duty=24, dutyLt=1
    {1114, 512, 908},  //duty=25, dutyLt=1
    {1116, 512, 902},  //duty=26, dutyLt=1
    {1118, 512, 896},  //duty=27, dutyLt=1
    {1121, 512, 890},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 529, 512,2887},  //duty=-1, dutyLt=2
    { 644, 512,1749},  //duty=0, dutyLt=2
    { 724, 512,1455},  //duty=1, dutyLt=2
    { 782, 512,1320},  //duty=2, dutyLt=2
    { 826, 512,1237},  //duty=3, dutyLt=2
    { 861, 512,1196},  //duty=4, dutyLt=2
    { 888, 512,1160},  //duty=5, dutyLt=2
    { 912, 512,1130},  //duty=6, dutyLt=2
    { 930, 512,1107},  //duty=7, dutyLt=2
    { 947, 512,1087},  //duty=8, dutyLt=2
    { 960, 512,1073},  //duty=9, dutyLt=2
    { 973, 512,1059},  //duty=10, dutyLt=2
    { 984, 512,1048},  //duty=11, dutyLt=2
    { 993, 512,1038},  //duty=12, dutyLt=2
    {1001, 512,1028},  //duty=13, dutyLt=2
    {1009, 512,1020},  //duty=14, dutyLt=2
    {1015, 512,1013},  //duty=15, dutyLt=2
    {1028, 512, 999},  //duty=16, dutyLt=2
    {1038, 512, 988},  //duty=17, dutyLt=2
    {1046, 512, 978},  //duty=18, dutyLt=2
    {1053, 512, 969},  //duty=19, dutyLt=2
    {1059, 512, 960},  //duty=20, dutyLt=2
    {1064, 512, 953},  //duty=21, dutyLt=2
    {1069, 512, 945},  //duty=22, dutyLt=2
    {1074, 512, 937},  //duty=23, dutyLt=2
    {1078, 512, 931},  //duty=24, dutyLt=2
    {1081, 512, 924},  //duty=25, dutyLt=2
    {1084, 512, 918},  //duty=26, dutyLt=2
    {1087, 512, 912},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 528, 512,2861},  //duty=-1, dutyLt=3
    { 619, 512,1870},  //duty=0, dutyLt=3
    { 689, 512,1558},  //duty=1, dutyLt=3
    { 741, 512,1403},  //duty=2, dutyLt=3
    { 782, 512,1324},  //duty=3, dutyLt=3
    { 815, 512,1260},  //duty=4, dutyLt=3
    { 842, 512,1217},  //duty=5, dutyLt=3
    { 865, 512,1182},  //duty=6, dutyLt=3
    { 885, 512,1153},  //duty=7, dutyLt=3
    { 902, 512,1129},  //duty=8, dutyLt=3
    { 917, 512,1112},  //duty=9, dutyLt=3
    { 930, 512,1096},  //duty=10, dutyLt=3
    { 942, 512,1082},  //duty=11, dutyLt=3
    { 952, 512,1070},  //duty=12, dutyLt=3
    { 962, 512,1059},  //duty=13, dutyLt=3
    { 970, 512,1050},  //duty=14, dutyLt=3
    { 978, 512,1041},  //duty=15, dutyLt=3
    { 992, 512,1024},  //duty=16, dutyLt=3
    {1003, 512,1011},  //duty=17, dutyLt=3
    {1012, 512, 999},  //duty=18, dutyLt=3
    {1021, 512, 989},  //duty=19, dutyLt=3
    {1027, 512, 979},  //duty=20, dutyLt=3
    {1034, 512, 971},  //duty=21, dutyLt=3
    {1039, 512, 963},  //duty=22, dutyLt=3
    {1044, 512, 954},  //duty=23, dutyLt=3
    {1049, 512, 947},  //duty=24, dutyLt=3
    {1053, 512, 940},  //duty=25, dutyLt=3
    {1057, 512, 933},  //duty=26, dutyLt=3
    {1060, 512, 926},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 528, 512,2839},  //duty=-1, dutyLt=4
    { 605, 512,1965},  //duty=0, dutyLt=4
    { 665, 512,1641},  //duty=1, dutyLt=4
    { 713, 512,1460},  //duty=2, dutyLt=4
    { 750, 512,1386},  //duty=3, dutyLt=4
    { 781, 512,1316},  //duty=4, dutyLt=4
    { 807, 512,1266},  //duty=5, dutyLt=4
    { 830, 512,1228},  //duty=6, dutyLt=4
    { 850, 512,1196},  //duty=7, dutyLt=4
    { 868, 512,1169},  //duty=8, dutyLt=4
    { 883, 512,1149},  //duty=9, dutyLt=4
    { 896, 512,1130},  //duty=10, dutyLt=4
    { 908, 512,1114},  //duty=11, dutyLt=4
    { 919, 512,1100},  //duty=12, dutyLt=4
    { 929, 512,1088},  //duty=13, dutyLt=4
    { 938, 512,1077},  //duty=14, dutyLt=4
    { 946, 512,1067},  //duty=15, dutyLt=4
    { 962, 512,1048},  //duty=16, dutyLt=4
    { 973, 512,1033},  //duty=17, dutyLt=4
    { 983, 512,1020},  //duty=18, dutyLt=4
    { 993, 512,1008},  //duty=19, dutyLt=4
    {1000, 512, 998},  //duty=20, dutyLt=4
    {1007, 512, 988},  //duty=21, dutyLt=4
    {1013, 512, 979},  //duty=22, dutyLt=4
    {1019, 512, 970},  //duty=23, dutyLt=4
    {1024, 512, 963},  //duty=24, dutyLt=4
    {1028, 512, 955},  //duty=25, dutyLt=4
    {1032, 512, 948},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 528, 512,2816},  //duty=-1, dutyLt=5
    { 594, 512,2036},  //duty=0, dutyLt=5
    { 648, 512,1704},  //duty=1, dutyLt=5
    { 691, 512,1559},  //duty=2, dutyLt=5
    { 726, 512,1443},  //duty=3, dutyLt=5
    { 755, 512,1367},  //duty=4, dutyLt=5
    { 781, 512,1313},  //duty=5, dutyLt=5
    { 803, 512,1270},  //duty=6, dutyLt=5
    { 823, 512,1235},  //duty=7, dutyLt=5
    { 839, 512,1206},  //duty=8, dutyLt=5
    { 854, 512,1183},  //duty=9, dutyLt=5
    { 868, 512,1162},  //duty=10, dutyLt=5
    { 881, 512,1144},  //duty=11, dutyLt=5
    { 892, 512,1129},  //duty=12, dutyLt=5
    { 902, 512,1115},  //duty=13, dutyLt=5
    { 911, 512,1102},  //duty=14, dutyLt=5
    { 920, 512,1091},  //duty=15, dutyLt=5
    { 935, 512,1071},  //duty=16, dutyLt=5
    { 948, 512,1054},  //duty=17, dutyLt=5
    { 959, 512,1040},  //duty=18, dutyLt=5
    { 968, 512,1027},  //duty=19, dutyLt=5
    { 977, 512,1015},  //duty=20, dutyLt=5
    { 984, 512,1005},  //duty=21, dutyLt=5
    { 990, 512, 995},  //duty=22, dutyLt=5
    { 997, 512, 986},  //duty=23, dutyLt=5
    {1002, 512, 978},  //duty=24, dutyLt=5
    {1006, 512, 969},  //duty=25, dutyLt=5
    {1010, 512, 962},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 528, 512,2816},  //duty=-1, dutyLt=6
    { 587, 512,2089},  //duty=0, dutyLt=6
    { 635, 512,1798},  //duty=1, dutyLt=6
    { 675, 512,1615},  //duty=2, dutyLt=6
    { 708, 512,1496},  //duty=3, dutyLt=6
    { 735, 512,1415},  //duty=4, dutyLt=6
    { 760, 512,1355},  //duty=5, dutyLt=6
    { 781, 512,1308},  //duty=6, dutyLt=6
    { 800, 512,1269},  //duty=7, dutyLt=6
    { 816, 512,1240},  //duty=8, dutyLt=6
    { 831, 512,1213},  //duty=9, dutyLt=6
    { 845, 512,1192},  //duty=10, dutyLt=6
    { 857, 512,1172},  //duty=11, dutyLt=6
    { 869, 512,1156},  //duty=12, dutyLt=6
    { 879, 512,1140},  //duty=13, dutyLt=6
    { 888, 512,1126},  //duty=14, dutyLt=6
    { 897, 512,1114},  //duty=15, dutyLt=6
    { 913, 512,1092},  //duty=16, dutyLt=6
    { 926, 512,1074},  //duty=17, dutyLt=6
    { 937, 512,1058},  //duty=18, dutyLt=6
    { 947, 512,1044},  //duty=19, dutyLt=6
    { 956, 512,1032},  //duty=20, dutyLt=6
    { 963, 512,1021},  //duty=21, dutyLt=6
    { 970, 512,1011},  //duty=22, dutyLt=6
    { 976, 512,1000},  //duty=23, dutyLt=6
    { 982, 512, 991},  //duty=24, dutyLt=6
    { 987, 512, 983},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 528, 512,2805},  //duty=-1, dutyLt=7
    { 582, 512,2124},  //duty=0, dutyLt=7
    { 626, 512,1849},  //duty=1, dutyLt=7
    { 661, 512,1662},  //duty=2, dutyLt=7
    { 692, 512,1542},  //duty=3, dutyLt=7
    { 719, 512,1455},  //duty=4, dutyLt=7
    { 741, 512,1392},  //duty=5, dutyLt=7
    { 762, 512,1344},  //duty=6, dutyLt=7
    { 781, 512,1302},  //duty=7, dutyLt=7
    { 797, 512,1272},  //duty=8, dutyLt=7
    { 812, 512,1242},  //duty=9, dutyLt=7
    { 825, 512,1220},  //duty=10, dutyLt=7
    { 837, 512,1198},  //duty=11, dutyLt=7
    { 849, 512,1180},  //duty=12, dutyLt=7
    { 859, 512,1164},  //duty=13, dutyLt=7
    { 868, 512,1149},  //duty=14, dutyLt=7
    { 877, 512,1136},  //duty=15, dutyLt=7
    { 893, 512,1112},  //duty=16, dutyLt=7
    { 907, 512,1093},  //duty=17, dutyLt=7
    { 918, 512,1076},  //duty=18, dutyLt=7
    { 928, 512,1061},  //duty=19, dutyLt=7
    { 937, 512,1048},  //duty=20, dutyLt=7
    { 945, 512,1036},  //duty=21, dutyLt=7
    { 952, 512,1026},  //duty=22, dutyLt=7
    { 959, 512,1015},  //duty=23, dutyLt=7
    { 964, 512,1005},  //duty=24, dutyLt=7
    { 969, 512, 996},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 529, 512,2789},  //duty=-1, dutyLt=8
    { 577, 512,2212},  //duty=0, dutyLt=8
    { 617, 512,1884},  //duty=1, dutyLt=8
    { 651, 512,1705},  //duty=2, dutyLt=8
    { 680, 512,1581},  //duty=3, dutyLt=8
    { 705, 512,1494},  //duty=4, dutyLt=8
    { 727, 512,1427},  //duty=5, dutyLt=8
    { 747, 512,1375},  //duty=6, dutyLt=8
    { 765, 512,1334},  //duty=7, dutyLt=8
    { 780, 512,1300},  //duty=8, dutyLt=8
    { 795, 512,1269},  //duty=9, dutyLt=8
    { 808, 512,1245},  //duty=10, dutyLt=8
    { 820, 512,1222},  //duty=11, dutyLt=8
    { 832, 512,1202},  //duty=12, dutyLt=8
    { 842, 512,1186},  //duty=13, dutyLt=8
    { 851, 512,1170},  //duty=14, dutyLt=8
    { 860, 512,1156},  //duty=15, dutyLt=8
    { 876, 512,1130},  //duty=16, dutyLt=8
    { 889, 512,1110},  //duty=17, dutyLt=8
    { 901, 512,1092},  //duty=18, dutyLt=8
    { 912, 512,1077},  //duty=19, dutyLt=8
    { 921, 512,1063},  //duty=20, dutyLt=8
    { 928, 512,1050},  //duty=21, dutyLt=8
    { 936, 512,1039},  //duty=22, dutyLt=8
    { 942, 512,1027},  //duty=23, dutyLt=8
    { 948, 512,1018},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 530, 512,2770},  //duty=-1, dutyLt=9
    { 573, 512,2249},  //duty=0, dutyLt=9
    { 611, 512,1925},  //duty=1, dutyLt=9
    { 642, 512,1741},  //duty=2, dutyLt=9
    { 670, 512,1618},  //duty=3, dutyLt=9
    { 693, 512,1529},  //duty=4, dutyLt=9
    { 715, 512,1460},  //duty=5, dutyLt=9
    { 734, 512,1405},  //duty=6, dutyLt=9
    { 751, 512,1363},  //duty=7, dutyLt=9
    { 766, 512,1327},  //duty=8, dutyLt=9
    { 781, 512,1295},  //duty=9, dutyLt=9
    { 793, 512,1269},  //duty=10, dutyLt=9
    { 805, 512,1246},  //duty=11, dutyLt=9
    { 816, 512,1225},  //duty=12, dutyLt=9
    { 827, 512,1206},  //duty=13, dutyLt=9
    { 836, 512,1190},  //duty=14, dutyLt=9
    { 844, 512,1176},  //duty=15, dutyLt=9
    { 861, 512,1148},  //duty=16, dutyLt=9
    { 874, 512,1127},  //duty=17, dutyLt=9
    { 886, 512,1108},  //duty=18, dutyLt=9
    { 897, 512,1092},  //duty=19, dutyLt=9
    { 906, 512,1077},  //duty=20, dutyLt=9
    { 914, 512,1064},  //duty=21, dutyLt=9
    { 921, 512,1052},  //duty=22, dutyLt=9
    { 928, 512,1040},  //duty=23, dutyLt=9
    { 934, 512,1030},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 529, 512,2830},  //duty=-1, dutyLt=10
    { 570, 512,2277},  //duty=0, dutyLt=10
    { 605, 512,1961},  //duty=1, dutyLt=10
    { 635, 512,1775},  //duty=2, dutyLt=10
    { 661, 512,1651},  //duty=3, dutyLt=10
    { 683, 512,1558},  //duty=4, dutyLt=10
    { 704, 512,1489},  //duty=5, dutyLt=10
    { 723, 512,1433},  //duty=6, dutyLt=10
    { 739, 512,1389},  //duty=7, dutyLt=10
    { 754, 512,1350},  //duty=8, dutyLt=10
    { 768, 512,1319},  //duty=9, dutyLt=10
    { 781, 512,1290},  //duty=10, dutyLt=10
    { 792, 512,1267},  //duty=11, dutyLt=10
    { 803, 512,1246},  //duty=12, dutyLt=10
    { 814, 512,1225},  //duty=13, dutyLt=10
    { 823, 512,1208},  //duty=14, dutyLt=10
    { 831, 512,1193},  //duty=15, dutyLt=10
    { 847, 512,1165},  //duty=16, dutyLt=10
    { 861, 512,1142},  //duty=17, dutyLt=10
    { 873, 512,1123},  //duty=18, dutyLt=10
    { 883, 512,1106},  //duty=19, dutyLt=10
    { 892, 512,1090},  //duty=20, dutyLt=10
    { 901, 512,1076},  //duty=21, dutyLt=10
    { 908, 512,1064},  //duty=22, dutyLt=10
    { 915, 512,1052},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 529, 512,2822},  //duty=-1, dutyLt=11
    { 567, 512,2293},  //duty=0, dutyLt=11
    { 600, 512,1992},  //duty=1, dutyLt=11
    { 628, 512,1806},  //duty=2, dutyLt=11
    { 653, 512,1679},  //duty=3, dutyLt=11
    { 675, 512,1587},  //duty=4, dutyLt=11
    { 695, 512,1516},  //duty=5, dutyLt=11
    { 713, 512,1460},  //duty=6, dutyLt=11
    { 729, 512,1413},  //duty=7, dutyLt=11
    { 744, 512,1374},  //duty=8, dutyLt=11
    { 757, 512,1341},  //duty=9, dutyLt=11
    { 769, 512,1312},  //duty=10, dutyLt=11
    { 781, 512,1286},  //duty=11, dutyLt=11
    { 792, 512,1264},  //duty=12, dutyLt=11
    { 801, 512,1244},  //duty=13, dutyLt=11
    { 811, 512,1226},  //duty=14, dutyLt=11
    { 819, 512,1210},  //duty=15, dutyLt=11
    { 835, 512,1181},  //duty=16, dutyLt=11
    { 848, 512,1157},  //duty=17, dutyLt=11
    { 860, 512,1137},  //duty=18, dutyLt=11
    { 871, 512,1119},  //duty=19, dutyLt=11
    { 880, 512,1103},  //duty=20, dutyLt=11
    { 888, 512,1089},  //duty=21, dutyLt=11
    { 896, 512,1076},  //duty=22, dutyLt=11
    { 903, 512,1064},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 530, 512,2832},  //duty=-1, dutyLt=12
    { 565, 512,2300},  //duty=0, dutyLt=12
    { 596, 512,2017},  //duty=1, dutyLt=12
    { 623, 512,1835},  //duty=2, dutyLt=12
    { 647, 512,1706},  //duty=3, dutyLt=12
    { 668, 512,1614},  //duty=4, dutyLt=12
    { 687, 512,1541},  //duty=5, dutyLt=12
    { 704, 512,1483},  //duty=6, dutyLt=12
    { 720, 512,1434},  //duty=7, dutyLt=12
    { 734, 512,1396},  //duty=8, dutyLt=12
    { 747, 512,1361},  //duty=9, dutyLt=12
    { 759, 512,1332},  //duty=10, dutyLt=12
    { 771, 512,1305},  //duty=11, dutyLt=12
    { 781, 512,1282},  //duty=12, dutyLt=12
    { 791, 512,1262},  //duty=13, dutyLt=12
    { 800, 512,1243},  //duty=14, dutyLt=12
    { 809, 512,1226},  //duty=15, dutyLt=12
    { 824, 512,1196},  //duty=16, dutyLt=12
    { 838, 512,1171},  //duty=17, dutyLt=12
    { 849, 512,1150},  //duty=18, dutyLt=12
    { 860, 512,1132},  //duty=19, dutyLt=12
    { 869, 512,1115},  //duty=20, dutyLt=12
    { 877, 512,1101},  //duty=21, dutyLt=12
    { 885, 512,1087},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 512, 512, 512},  //duty=28, dutyLt=12
    { 512, 512, 512},  //duty=29, dutyLt=12
    { 530, 512,2823},  //duty=-1, dutyLt=13
    { 563, 512,2320},  //duty=0, dutyLt=13
    { 593, 512,2038},  //duty=1, dutyLt=13
    { 618, 512,1860},  //duty=2, dutyLt=13
    { 641, 512,1733},  //duty=3, dutyLt=13
    { 661, 512,1639},  //duty=4, dutyLt=13
    { 679, 512,1566},  //duty=5, dutyLt=13
    { 696, 512,1505},  //duty=6, dutyLt=13
    { 712, 512,1457},  //duty=7, dutyLt=13
    { 726, 512,1417},  //duty=8, dutyLt=13
    { 739, 512,1381},  //duty=9, dutyLt=13
    { 750, 512,1350},  //duty=10, dutyLt=13
    { 762, 512,1323},  //duty=11, dutyLt=13
    { 772, 512,1299},  //duty=12, dutyLt=13
    { 782, 512,1278},  //duty=13, dutyLt=13
    { 790, 512,1259},  //duty=14, dutyLt=13
    { 799, 512,1242},  //duty=15, dutyLt=13
    { 814, 512,1210},  //duty=16, dutyLt=13
    { 828, 512,1185},  //duty=17, dutyLt=13
    { 839, 512,1163},  //duty=18, dutyLt=13
    { 850, 512,1144},  //duty=19, dutyLt=13
    { 859, 512,1127},  //duty=20, dutyLt=13
    { 867, 512,1111},  //duty=21, dutyLt=13
    { 874, 512,1098},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 530, 512,2805},  //duty=-1, dutyLt=14
    { 561, 512,2341},  //duty=0, dutyLt=14
    { 590, 512,2057},  //duty=1, dutyLt=14
    { 614, 512,1882},  //duty=2, dutyLt=14
    { 636, 512,1756},  //duty=3, dutyLt=14
    { 656, 512,1661},  //duty=4, dutyLt=14
    { 673, 512,1585},  //duty=5, dutyLt=14
    { 690, 512,1526},  //duty=6, dutyLt=14
    { 704, 512,1478},  //duty=7, dutyLt=14
    { 718, 512,1434},  //duty=8, dutyLt=14
    { 731, 512,1399},  //duty=9, dutyLt=14
    { 742, 512,1367},  //duty=10, dutyLt=14
    { 754, 512,1340},  //duty=11, dutyLt=14
    { 763, 512,1316},  //duty=12, dutyLt=14
    { 773, 512,1294},  //duty=13, dutyLt=14
    { 782, 512,1274},  //duty=14, dutyLt=14
    { 790, 512,1256},  //duty=15, dutyLt=14
    { 805, 512,1224},  //duty=16, dutyLt=14
    { 818, 512,1198},  //duty=17, dutyLt=14
    { 830, 512,1176},  //duty=18, dutyLt=14
    { 840, 512,1155},  //duty=19, dutyLt=14
    { 850, 512,1138},  //duty=20, dutyLt=14
    { 858, 512,1123},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 512, 512, 512},  //duty=28, dutyLt=14
    { 512, 512, 512},  //duty=29, dutyLt=14
    { 530, 512,2785},  //duty=-1, dutyLt=15
    { 560, 512,2353},  //duty=0, dutyLt=15
    { 587, 512,2078},  //duty=1, dutyLt=15
    { 611, 512,1902},  //duty=2, dutyLt=15
    { 632, 512,1776},  //duty=3, dutyLt=15
    { 651, 512,1680},  //duty=4, dutyLt=15
    { 668, 512,1606},  //duty=5, dutyLt=15
    { 684, 512,1546},  //duty=6, dutyLt=15
    { 698, 512,1494},  //duty=7, dutyLt=15
    { 711, 512,1453},  //duty=8, dutyLt=15
    { 724, 512,1416},  //duty=9, dutyLt=15
    { 735, 512,1384},  //duty=10, dutyLt=15
    { 746, 512,1355},  //duty=11, dutyLt=15
    { 756, 512,1331},  //duty=12, dutyLt=15
    { 765, 512,1309},  //duty=13, dutyLt=15
    { 774, 512,1288},  //duty=14, dutyLt=15
    { 782, 512,1270},  //duty=15, dutyLt=15
    { 797, 512,1237},  //duty=16, dutyLt=15
    { 810, 512,1210},  //duty=17, dutyLt=15
    { 821, 512,1187},  //duty=18, dutyLt=15
    { 832, 512,1167},  //duty=19, dutyLt=15
    { 841, 512,1149},  //duty=20, dutyLt=15
    { 849, 512,1132},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 530, 512,2790},  //duty=-1, dutyLt=16
    { 558, 512,2366},  //duty=0, dutyLt=16
    { 583, 512,2107},  //duty=1, dutyLt=16
    { 605, 512,1938},  //duty=2, dutyLt=16
    { 625, 512,1812},  //duty=3, dutyLt=16
    { 642, 512,1718},  //duty=4, dutyLt=16
    { 658, 512,1642},  //duty=5, dutyLt=16
    { 673, 512,1578},  //duty=6, dutyLt=16
    { 688, 512,1527},  //duty=7, dutyLt=16
    { 700, 512,1484},  //duty=8, dutyLt=16
    { 712, 512,1446},  //duty=9, dutyLt=16
    { 723, 512,1413},  //duty=10, dutyLt=16
    { 733, 512,1384},  //duty=11, dutyLt=16
    { 743, 512,1357},  //duty=12, dutyLt=16
    { 752, 512,1334},  //duty=13, dutyLt=16
    { 760, 512,1313},  //duty=14, dutyLt=16
    { 768, 512,1294},  //duty=15, dutyLt=16
    { 783, 512,1261},  //duty=16, dutyLt=16
    { 795, 512,1233},  //duty=17, dutyLt=16
    { 807, 512,1208},  //duty=18, dutyLt=16
    { 817, 512,1187},  //duty=19, dutyLt=16
    { 826, 512,1168},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 512, 512, 512},  //duty=28, dutyLt=16
    { 512, 512, 512},  //duty=29, dutyLt=16
    { 531, 512,2766},  //duty=-1, dutyLt=17
    { 556, 512,2379},  //duty=0, dutyLt=17
    { 580, 512,2133},  //duty=1, dutyLt=17
    { 600, 512,1964},  //duty=2, dutyLt=17
    { 619, 512,1843},  //duty=3, dutyLt=17
    { 636, 512,1746},  //duty=4, dutyLt=17
    { 651, 512,1671},  //duty=5, dutyLt=17
    { 665, 512,1610},  //duty=6, dutyLt=17
    { 678, 512,1556},  //duty=7, dutyLt=17
    { 691, 512,1512},  //duty=8, dutyLt=17
    { 702, 512,1473},  //duty=9, dutyLt=17
    { 713, 512,1439},  //duty=10, dutyLt=17
    { 723, 512,1408},  //duty=11, dutyLt=17
    { 732, 512,1382},  //duty=12, dutyLt=17
    { 741, 512,1358},  //duty=13, dutyLt=17
    { 749, 512,1337},  //duty=14, dutyLt=17
    { 757, 512,1317},  //duty=15, dutyLt=17
    { 771, 512,1282},  //duty=16, dutyLt=17
    { 783, 512,1253},  //duty=17, dutyLt=17
    { 794, 512,1228},  //duty=18, dutyLt=17
    { 804, 512,1206},  //duty=19, dutyLt=17
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
    { 531, 512,2745},  //duty=-1, dutyLt=18
    { 555, 512,2391},  //duty=0, dutyLt=18
    { 577, 512,2153},  //duty=1, dutyLt=18
    { 596, 512,1991},  //duty=2, dutyLt=18
    { 614, 512,1870},  //duty=3, dutyLt=18
    { 630, 512,1773},  //duty=4, dutyLt=18
    { 645, 512,1696},  //duty=5, dutyLt=18
    { 658, 512,1636},  //duty=6, dutyLt=18
    { 671, 512,1582},  //duty=7, dutyLt=18
    { 683, 512,1537},  //duty=8, dutyLt=18
    { 694, 512,1497},  //duty=9, dutyLt=18
    { 704, 512,1462},  //duty=10, dutyLt=18
    { 714, 512,1430},  //duty=11, dutyLt=18
    { 723, 512,1404},  //duty=12, dutyLt=18
    { 731, 512,1379},  //duty=13, dutyLt=18
    { 739, 512,1357},  //duty=14, dutyLt=18
    { 746, 512,1338},  //duty=15, dutyLt=18
    { 760, 512,1301},  //duty=16, dutyLt=18
    { 772, 512,1272},  //duty=17, dutyLt=18
    { 783, 512,1246},  //duty=18, dutyLt=18
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
    { 531, 512,2736},  //duty=-1, dutyLt=19
    { 554, 512,2398},  //duty=0, dutyLt=19
    { 575, 512,2170},  //duty=1, dutyLt=19
    { 593, 512,2012},  //duty=2, dutyLt=19
    { 610, 512,1893},  //duty=3, dutyLt=19
    { 626, 512,1799},  //duty=4, dutyLt=19
    { 640, 512,1720},  //duty=5, dutyLt=19
    { 653, 512,1658},  //duty=6, dutyLt=19
    { 665, 512,1605},  //duty=7, dutyLt=19
    { 676, 512,1558},  //duty=8, dutyLt=19
    { 687, 512,1519},  //duty=9, dutyLt=19
    { 697, 512,1483},  //duty=10, dutyLt=19
    { 706, 512,1451},  //duty=11, dutyLt=19
    { 715, 512,1423},  //duty=12, dutyLt=19
    { 723, 512,1398},  //duty=13, dutyLt=19
    { 731, 512,1375},  //duty=14, dutyLt=19
    { 738, 512,1355},  //duty=15, dutyLt=19
    { 751, 512,1318},  //duty=16, dutyLt=19
    { 764, 512,1287},  //duty=17, dutyLt=19
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
    { 532, 512,2724},  //duty=-1, dutyLt=20
    { 554, 512,2400},  //duty=0, dutyLt=20
    { 574, 512,2183},  //duty=1, dutyLt=20
    { 591, 512,2029},  //duty=2, dutyLt=20
    { 607, 512,1912},  //duty=3, dutyLt=20
    { 622, 512,1817},  //duty=4, dutyLt=20
    { 635, 512,1740},  //duty=5, dutyLt=20
    { 648, 512,1677},  //duty=6, dutyLt=20
    { 660, 512,1624},  //duty=7, dutyLt=20
    { 671, 512,1577},  //duty=8, dutyLt=20
    { 681, 512,1537},  //duty=9, dutyLt=20
    { 691, 512,1501},  //duty=10, dutyLt=20
    { 700, 512,1469},  //duty=11, dutyLt=20
    { 708, 512,1441},  //duty=12, dutyLt=20
    { 716, 512,1417},  //duty=13, dutyLt=20
    { 724, 512,1392},  //duty=14, dutyLt=20
    { 731, 512,1371},  //duty=15, dutyLt=20
    { 744, 512,1333},  //duty=16, dutyLt=20
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
    { 533, 512,2715},  //duty=-1, dutyLt=21
    { 554, 512,2405},  //duty=0, dutyLt=21
    { 572, 512,2193},  //duty=1, dutyLt=21
    { 589, 512,2043},  //duty=2, dutyLt=21
    { 605, 512,1926},  //duty=3, dutyLt=21
    { 619, 512,1834},  //duty=4, dutyLt=21
    { 632, 512,1758},  //duty=5, dutyLt=21
    { 644, 512,1694},  //duty=6, dutyLt=21
    { 655, 512,1641},  //duty=7, dutyLt=21
    { 666, 512,1594},  //duty=8, dutyLt=21
    { 676, 512,1553},  //duty=9, dutyLt=21
    { 685, 512,1518},  //duty=10, dutyLt=21
    { 694, 512,1484},  //duty=11, dutyLt=21
    { 703, 512,1456},  //duty=12, dutyLt=21
    { 710, 512,1430},  //duty=13, dutyLt=21
    { 718, 512,1407},  //duty=14, dutyLt=21
    { 725, 512,1384},  //duty=15, dutyLt=21
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
    { 533, 512,2697},  //duty=-1, dutyLt=22
    { 553, 512,2407},  //duty=0, dutyLt=22
    { 572, 512,2203},  //duty=1, dutyLt=22
    { 588, 512,2055},  //duty=2, dutyLt=22
    { 603, 512,1939},  //duty=3, dutyLt=22
    { 616, 512,1849},  //duty=4, dutyLt=22
    { 629, 512,1771},  //duty=5, dutyLt=22
    { 641, 512,1709},  //duty=6, dutyLt=22
    { 652, 512,1655},  //duty=7, dutyLt=22
    { 662, 512,1609},  //duty=8, dutyLt=22
    { 672, 512,1567},  //duty=9, dutyLt=22
    { 681, 512,1531},  //duty=10, dutyLt=22
    { 690, 512,1498},  //duty=11, dutyLt=22
    { 698, 512,1470},  //duty=12, dutyLt=22
    { 706, 512,1443},  //duty=13, dutyLt=22
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
    { 534, 512,2687},  //duty=-1, dutyLt=23
    { 553, 512,2409},  //duty=0, dutyLt=23
    { 571, 512,2210},  //duty=1, dutyLt=23
    { 587, 512,2064},  //duty=2, dutyLt=23
    { 601, 512,1950},  //duty=3, dutyLt=23
    { 615, 512,1859},  //duty=4, dutyLt=23
    { 627, 512,1783},  //duty=5, dutyLt=23
    { 638, 512,1721},  //duty=6, dutyLt=23
    { 649, 512,1667},  //duty=7, dutyLt=23
    { 659, 512,1619},  //duty=8, dutyLt=23
    { 669, 512,1578},  //duty=9, dutyLt=23
    { 678, 512,1543},  //duty=10, dutyLt=23
    { 686, 512,1509},  //duty=11, dutyLt=23
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
    { 535, 512,2678},  //duty=-1, dutyLt=24
    { 553, 512,2405},  //duty=0, dutyLt=24
    { 570, 512,2214},  //duty=1, dutyLt=24
    { 585, 512,2074},  //duty=2, dutyLt=24
    { 600, 512,1958},  //duty=3, dutyLt=24
    { 613, 512,1867},  //duty=4, dutyLt=24
    { 625, 512,1793},  //duty=5, dutyLt=24
    { 636, 512,1731},  //duty=6, dutyLt=24
    { 647, 512,1677},  //duty=7, dutyLt=24
    { 656, 512,1630},  //duty=8, dutyLt=24
    { 666, 512,1588},  //duty=9, dutyLt=24
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
    { 535, 512,2668},  //duty=-1, dutyLt=25
    { 554, 512,2405},  //duty=0, dutyLt=25
    { 570, 512,2216},  //duty=1, dutyLt=25
    { 585, 512,2075},  //duty=2, dutyLt=25
    { 599, 512,1964},  //duty=3, dutyLt=25
    { 612, 512,1875},  //duty=4, dutyLt=25
    { 623, 512,1802},  //duty=5, dutyLt=25
    { 634, 512,1740},  //duty=6, dutyLt=25
    { 645, 512,1685},  //duty=7, dutyLt=25
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
    { 536, 512,2657},  //duty=-1, dutyLt=26
    { 554, 512,2404},  //duty=0, dutyLt=26
    { 570, 512,2217},  //duty=1, dutyLt=26
    { 585, 512,2079},  //duty=2, dutyLt=26
    { 598, 512,1970},  //duty=3, dutyLt=26
    { 611, 512,1883},  //duty=4, dutyLt=26
    { 622, 512,1809},  //duty=5, dutyLt=26
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
    { 537, 512,2645},  //duty=-1, dutyLt=27
    { 554, 512,2400},  //duty=0, dutyLt=27
    { 570, 512,2219},  //duty=1, dutyLt=27
    { 584, 512,2083},  //duty=2, dutyLt=27
    { 598, 512,1976},  //duty=3, dutyLt=27
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
    { 538, 512,2639},  //duty=-1, dutyLt=28
    { 555, 512,2394},  //duty=0, dutyLt=28
    { 570, 512,2215},  //duty=1, dutyLt=28
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
    { 539, 512,2625},  //duty=-1, dutyLt=29
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
