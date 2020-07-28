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
#include "imx350mipiraw_Scene_Capture.h"

const FLASH_CALIBRATION_NVRAM_T imx350mipiraw_Flash_Calibration_0000 = {
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
    { 510, 512, 512},  //duty=-1, dutyLt=-1
    {1016, 512, 892},  //duty=0, dutyLt=-1
    {1022, 512, 892},  //duty=1, dutyLt=-1
    {1018, 512, 887},  //duty=2, dutyLt=-1
    {1017, 512, 883},  //duty=3, dutyLt=-1
    {1016, 512, 878},  //duty=4, dutyLt=-1
    {1016, 512, 873},  //duty=5, dutyLt=-1
    {1015, 512, 859},  //duty=6, dutyLt=-1
    {1020, 512, 872},  //duty=7, dutyLt=-1
    {1019, 512, 868},  //duty=8, dutyLt=-1
    {1017, 512, 865},  //duty=9, dutyLt=-1
    {1017, 512, 862},  //duty=10, dutyLt=-1
    {1016, 512, 860},  //duty=11, dutyLt=-1
    {1016, 512, 857},  //duty=12, dutyLt=-1
    {1015, 512, 855},  //duty=13, dutyLt=-1
    {1015, 512, 853},  //duty=14, dutyLt=-1
    {1015, 512, 850},  //duty=15, dutyLt=-1
    {1015, 512, 846},  //duty=16, dutyLt=-1
    {1015, 512, 842},  //duty=17, dutyLt=-1
    {1014, 512, 838},  //duty=18, dutyLt=-1
    {1014, 512, 834},  //duty=19, dutyLt=-1
    {1014, 512, 831},  //duty=20, dutyLt=-1
    {1014, 512, 827},  //duty=21, dutyLt=-1
    {1014, 512, 823},  //duty=22, dutyLt=-1
    {1015, 512, 819},  //duty=23, dutyLt=-1
    {1015, 512, 816},  //duty=24, dutyLt=-1
    {1015, 512, 812},  //duty=25, dutyLt=-1
    {1016, 512, 807},  //duty=26, dutyLt=-1
    {1017, 512, 803},  //duty=27, dutyLt=-1
    {1017, 512, 799},  //duty=28, dutyLt=-1
    {1019, 512, 794},  //duty=29, dutyLt=-1
    { 464, 512,2342},  //duty=-1, dutyLt=0
    { 679, 512,1218},  //duty=0, dutyLt=0
    { 772, 512,1070},  //duty=1, dutyLt=0
    { 822, 512,1012},  //duty=2, dutyLt=0
    { 854, 512, 980},  //duty=3, dutyLt=0
    { 876, 512, 958},  //duty=4, dutyLt=0
    { 892, 512, 935},  //duty=5, dutyLt=0
    { 908, 512, 934},  //duty=6, dutyLt=0
    { 916, 512, 926},  //duty=7, dutyLt=0
    { 924, 512, 917},  //duty=8, dutyLt=0
    { 930, 512, 909},  //duty=9, dutyLt=0
    { 935, 512, 904},  //duty=10, dutyLt=0
    { 939, 512, 898},  //duty=11, dutyLt=0
    { 943, 512, 894},  //duty=12, dutyLt=0
    { 947, 512, 889},  //duty=13, dutyLt=0
    { 950, 512, 885},  //duty=14, dutyLt=0
    { 953, 512, 881},  //duty=15, dutyLt=0
    { 958, 512, 875},  //duty=16, dutyLt=0
    { 961, 512, 868},  //duty=17, dutyLt=0
    { 965, 512, 863},  //duty=18, dutyLt=0
    { 967, 512, 857},  //duty=19, dutyLt=0
    { 969, 512, 852},  //duty=20, dutyLt=0
    { 971, 512, 848},  //duty=21, dutyLt=0
    { 973, 512, 843},  //duty=22, dutyLt=0
    { 976, 512, 837},  //duty=23, dutyLt=0
    { 977, 512, 833},  //duty=24, dutyLt=0
    { 979, 512, 828},  //duty=25, dutyLt=0
    { 980, 512, 824},  //duty=26, dutyLt=0
    { 982, 512, 819},  //duty=27, dutyLt=0
    { 983, 512, 814},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 465, 512,2323},  //duty=-1, dutyLt=1
    { 598, 512,1414},  //duty=0, dutyLt=1
    { 679, 512,1203},  //duty=1, dutyLt=1
    { 731, 512,1114},  //duty=2, dutyLt=1
    { 768, 512,1061},  //duty=3, dutyLt=1
    { 795, 512,1022},  //duty=4, dutyLt=1
    { 818, 512,1007},  //duty=5, dutyLt=1
    { 834, 512, 988},  //duty=6, dutyLt=1
    { 848, 512, 972},  //duty=7, dutyLt=1
    { 859, 512, 960},  //duty=8, dutyLt=1
    { 869, 512, 949},  //duty=9, dutyLt=1
    { 878, 512, 941},  //duty=10, dutyLt=1
    { 885, 512, 933},  //duty=11, dutyLt=1
    { 891, 512, 926},  //duty=12, dutyLt=1
    { 897, 512, 920},  //duty=13, dutyLt=1
    { 902, 512, 914},  //duty=14, dutyLt=1
    { 906, 512, 909},  //duty=15, dutyLt=1
    { 914, 512, 899},  //duty=16, dutyLt=1
    { 921, 512, 892},  //duty=17, dutyLt=1
    { 926, 512, 884},  //duty=18, dutyLt=1
    { 931, 512, 878},  //duty=19, dutyLt=1
    { 934, 512, 871},  //duty=20, dutyLt=1
    { 938, 512, 866},  //duty=21, dutyLt=1
    { 940, 512, 860},  //duty=22, dutyLt=1
    { 944, 512, 854},  //duty=23, dutyLt=1
    { 947, 512, 849},  //duty=24, dutyLt=1
    { 949, 512, 843},  //duty=25, dutyLt=1
    { 951, 512, 838},  //duty=26, dutyLt=1
    { 953, 512, 834},  //duty=27, dutyLt=1
    { 954, 512, 829},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 464, 512,2368},  //duty=-1, dutyLt=2
    { 562, 512,1543},  //duty=0, dutyLt=2
    { 631, 512,1307},  //duty=1, dutyLt=2
    { 678, 512,1196},  //duty=2, dutyLt=2
    { 715, 512,1127},  //duty=3, dutyLt=2
    { 743, 512,1092},  //duty=4, dutyLt=2
    { 766, 512,1059},  //duty=5, dutyLt=2
    { 784, 512,1034},  //duty=6, dutyLt=2
    { 800, 512,1015},  //duty=7, dutyLt=2
    { 812, 512, 999},  //duty=8, dutyLt=2
    { 824, 512, 986},  //duty=9, dutyLt=2
    { 834, 512, 975},  //duty=10, dutyLt=2
    { 843, 512, 965},  //duty=11, dutyLt=2
    { 851, 512, 955},  //duty=12, dutyLt=2
    { 858, 512, 948},  //duty=13, dutyLt=2
    { 863, 512, 940},  //duty=14, dutyLt=2
    { 870, 512, 934},  //duty=15, dutyLt=2
    { 880, 512, 923},  //duty=16, dutyLt=2
    { 888, 512, 913},  //duty=17, dutyLt=2
    { 894, 512, 904},  //duty=18, dutyLt=2
    { 900, 512, 896},  //duty=19, dutyLt=2
    { 905, 512, 889},  //duty=20, dutyLt=2
    { 909, 512, 882},  //duty=21, dutyLt=2
    { 913, 512, 876},  //duty=22, dutyLt=2
    { 918, 512, 869},  //duty=23, dutyLt=2
    { 920, 512, 863},  //duty=24, dutyLt=2
    { 923, 512, 858},  //duty=25, dutyLt=2
    { 926, 512, 853},  //duty=26, dutyLt=2
    { 928, 512, 847},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 464, 512,2346},  //duty=-1, dutyLt=3
    { 542, 512,1637},  //duty=0, dutyLt=3
    { 601, 512,1388},  //duty=1, dutyLt=3
    { 644, 512,1263},  //duty=2, dutyLt=3
    { 679, 512,1200},  //duty=3, dutyLt=3
    { 705, 512,1144},  //duty=4, dutyLt=3
    { 728, 512,1106},  //duty=5, dutyLt=3
    { 747, 512,1078},  //duty=6, dutyLt=3
    { 762, 512,1054},  //duty=7, dutyLt=3
    { 777, 512,1035},  //duty=8, dutyLt=3
    { 789, 512,1019},  //duty=9, dutyLt=3
    { 800, 512,1006},  //duty=10, dutyLt=3
    { 809, 512, 994},  //duty=11, dutyLt=3
    { 818, 512, 983},  //duty=12, dutyLt=3
    { 826, 512, 974},  //duty=13, dutyLt=3
    { 833, 512, 966},  //duty=14, dutyLt=3
    { 839, 512, 958},  //duty=15, dutyLt=3
    { 850, 512, 944},  //duty=16, dutyLt=3
    { 860, 512, 933},  //duty=17, dutyLt=3
    { 868, 512, 923},  //duty=18, dutyLt=3
    { 874, 512, 914},  //duty=19, dutyLt=3
    { 880, 512, 906},  //duty=20, dutyLt=3
    { 885, 512, 898},  //duty=21, dutyLt=3
    { 889, 512, 891},  //duty=22, dutyLt=3
    { 894, 512, 884},  //duty=23, dutyLt=3
    { 897, 512, 877},  //duty=24, dutyLt=3
    { 901, 512, 871},  //duty=25, dutyLt=3
    { 904, 512, 866},  //duty=26, dutyLt=3
    { 906, 512, 860},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 464, 512,2332},  //duty=-1, dutyLt=4
    { 529, 512,1704},  //duty=0, dutyLt=4
    { 581, 512,1453},  //duty=1, dutyLt=4
    { 621, 512,1311},  //duty=2, dutyLt=4
    { 652, 512,1249},  //duty=3, dutyLt=4
    { 677, 512,1190},  //duty=4, dutyLt=4
    { 700, 512,1149},  //duty=5, dutyLt=4
    { 718, 512,1116},  //duty=6, dutyLt=4
    { 735, 512,1089},  //duty=7, dutyLt=4
    { 748, 512,1067},  //duty=8, dutyLt=4
    { 761, 512,1050},  //duty=9, dutyLt=4
    { 772, 512,1033},  //duty=10, dutyLt=4
    { 782, 512,1020},  //duty=11, dutyLt=4
    { 791, 512,1009},  //duty=12, dutyLt=4
    { 799, 512, 998},  //duty=13, dutyLt=4
    { 807, 512, 989},  //duty=14, dutyLt=4
    { 814, 512, 980},  //duty=15, dutyLt=4
    { 826, 512, 964},  //duty=16, dutyLt=4
    { 836, 512, 952},  //duty=17, dutyLt=4
    { 844, 512, 940},  //duty=18, dutyLt=4
    { 852, 512, 930},  //duty=19, dutyLt=4
    { 858, 512, 921},  //duty=20, dutyLt=4
    { 863, 512, 913},  //duty=21, dutyLt=4
    { 868, 512, 906},  //duty=22, dutyLt=4
    { 873, 512, 897},  //duty=23, dutyLt=4
    { 877, 512, 891},  //duty=24, dutyLt=4
    { 881, 512, 884},  //duty=25, dutyLt=4
    { 884, 512, 878},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 464, 512,2318},  //duty=-1, dutyLt=5
    { 520, 512,1757},  //duty=0, dutyLt=5
    { 567, 512,1504},  //duty=1, dutyLt=5
    { 601, 512,1386},  //duty=2, dutyLt=5
    { 631, 512,1294},  //duty=3, dutyLt=5
    { 656, 512,1233},  //duty=4, dutyLt=5
    { 677, 512,1187},  //duty=5, dutyLt=5
    { 695, 512,1150},  //duty=6, dutyLt=5
    { 712, 512,1122},  //duty=7, dutyLt=5
    { 726, 512,1098},  //duty=8, dutyLt=5
    { 738, 512,1078},  //duty=9, dutyLt=5
    { 749, 512,1060},  //duty=10, dutyLt=5
    { 760, 512,1045},  //duty=11, dutyLt=5
    { 769, 512,1033},  //duty=12, dutyLt=5
    { 777, 512,1021},  //duty=13, dutyLt=5
    { 785, 512,1010},  //duty=14, dutyLt=5
    { 792, 512,1001},  //duty=15, dutyLt=5
    { 805, 512, 983},  //duty=16, dutyLt=5
    { 815, 512, 969},  //duty=17, dutyLt=5
    { 824, 512, 957},  //duty=18, dutyLt=5
    { 832, 512, 946},  //duty=19, dutyLt=5
    { 839, 512, 936},  //duty=20, dutyLt=5
    { 844, 512, 927},  //duty=21, dutyLt=5
    { 850, 512, 919},  //duty=22, dutyLt=5
    { 855, 512, 911},  //duty=23, dutyLt=5
    { 859, 512, 904},  //duty=24, dutyLt=5
    { 863, 512, 897},  //duty=25, dutyLt=5
    { 867, 512, 890},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 464, 512,2310},  //duty=-1, dutyLt=6
    { 514, 512,1795},  //duty=0, dutyLt=6
    { 555, 512,1563},  //duty=1, dutyLt=6
    { 588, 512,1429},  //duty=2, dutyLt=6
    { 616, 512,1334},  //duty=3, dutyLt=6
    { 639, 512,1271},  //duty=4, dutyLt=6
    { 660, 512,1219},  //duty=5, dutyLt=6
    { 677, 512,1182},  //duty=6, dutyLt=6
    { 693, 512,1151},  //duty=7, dutyLt=6
    { 707, 512,1125},  //duty=8, dutyLt=6
    { 720, 512,1104},  //duty=9, dutyLt=6
    { 731, 512,1085},  //duty=10, dutyLt=6
    { 741, 512,1068},  //duty=11, dutyLt=6
    { 750, 512,1055},  //duty=12, dutyLt=6
    { 759, 512,1042},  //duty=13, dutyLt=6
    { 766, 512,1030},  //duty=14, dutyLt=6
    { 774, 512,1020},  //duty=15, dutyLt=6
    { 787, 512,1001},  //duty=16, dutyLt=6
    { 797, 512, 986},  //duty=17, dutyLt=6
    { 807, 512, 973},  //duty=18, dutyLt=6
    { 815, 512, 961},  //duty=19, dutyLt=6
    { 822, 512, 950},  //duty=20, dutyLt=6
    { 828, 512, 941},  //duty=21, dutyLt=6
    { 834, 512, 932},  //duty=22, dutyLt=6
    { 839, 512, 923},  //duty=23, dutyLt=6
    { 844, 512, 916},  //duty=24, dutyLt=6
    { 848, 512, 908},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 464, 512,2302},  //duty=-1, dutyLt=7
    { 510, 512,1821},  //duty=0, dutyLt=7
    { 547, 512,1604},  //duty=1, dutyLt=7
    { 577, 512,1467},  //duty=2, dutyLt=7
    { 603, 512,1370},  //duty=3, dutyLt=7
    { 626, 512,1305},  //duty=4, dutyLt=7
    { 645, 512,1251},  //duty=5, dutyLt=7
    { 662, 512,1210},  //duty=6, dutyLt=7
    { 677, 512,1178},  //duty=7, dutyLt=7
    { 691, 512,1151},  //duty=8, dutyLt=7
    { 703, 512,1128},  //duty=9, dutyLt=7
    { 714, 512,1108},  //duty=10, dutyLt=7
    { 724, 512,1090},  //duty=11, dutyLt=7
    { 734, 512,1075},  //duty=12, dutyLt=7
    { 742, 512,1061},  //duty=13, dutyLt=7
    { 750, 512,1049},  //duty=14, dutyLt=7
    { 757, 512,1038},  //duty=15, dutyLt=7
    { 771, 512,1018},  //duty=16, dutyLt=7
    { 782, 512,1002},  //duty=17, dutyLt=7
    { 791, 512, 988},  //duty=18, dutyLt=7
    { 800, 512, 975},  //duty=19, dutyLt=7
    { 807, 512, 963},  //duty=20, dutyLt=7
    { 813, 512, 954},  //duty=21, dutyLt=7
    { 819, 512, 944},  //duty=22, dutyLt=7
    { 825, 512, 935},  //duty=23, dutyLt=7
    { 829, 512, 927},  //duty=24, dutyLt=7
    { 833, 512, 920},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 465, 512,2290},  //duty=-1, dutyLt=8
    { 506, 512,1881},  //duty=0, dutyLt=8
    { 540, 512,1639},  //duty=1, dutyLt=8
    { 568, 512,1499},  //duty=2, dutyLt=8
    { 593, 512,1402},  //duty=3, dutyLt=8
    { 614, 512,1334},  //duty=4, dutyLt=8
    { 633, 512,1278},  //duty=5, dutyLt=8
    { 649, 512,1237},  //duty=6, dutyLt=8
    { 664, 512,1203},  //duty=7, dutyLt=8
    { 677, 512,1175},  //duty=8, dutyLt=8
    { 689, 512,1150},  //duty=9, dutyLt=8
    { 700, 512,1129},  //duty=10, dutyLt=8
    { 711, 512,1110},  //duty=11, dutyLt=8
    { 720, 512,1095},  //duty=12, dutyLt=8
    { 728, 512,1080},  //duty=13, dutyLt=8
    { 736, 512,1067},  //duty=14, dutyLt=8
    { 743, 512,1055},  //duty=15, dutyLt=8
    { 757, 512,1034},  //duty=16, dutyLt=8
    { 768, 512,1017},  //duty=17, dutyLt=8
    { 777, 512,1002},  //duty=18, dutyLt=8
    { 786, 512, 988},  //duty=19, dutyLt=8
    { 793, 512, 976},  //duty=20, dutyLt=8
    { 800, 512, 966},  //duty=21, dutyLt=8
    { 806, 512, 956},  //duty=22, dutyLt=8
    { 812, 512, 947},  //duty=23, dutyLt=8
    { 816, 512, 938},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 465, 512,2276},  //duty=-1, dutyLt=9
    { 502, 512,1905},  //duty=0, dutyLt=9
    { 534, 512,1670},  //duty=1, dutyLt=9
    { 561, 512,1526},  //duty=2, dutyLt=9
    { 584, 512,1432},  //duty=3, dutyLt=9
    { 604, 512,1360},  //duty=4, dutyLt=9
    { 622, 512,1305},  //duty=5, dutyLt=9
    { 638, 512,1262},  //duty=6, dutyLt=9
    { 653, 512,1226},  //duty=7, dutyLt=9
    { 666, 512,1197},  //duty=8, dutyLt=9
    { 678, 512,1171},  //duty=9, dutyLt=9
    { 688, 512,1148},  //duty=10, dutyLt=9
    { 698, 512,1129},  //duty=11, dutyLt=9
    { 707, 512,1113},  //duty=12, dutyLt=9
    { 716, 512,1098},  //duty=13, dutyLt=9
    { 724, 512,1083},  //duty=14, dutyLt=9
    { 731, 512,1071},  //duty=15, dutyLt=9
    { 744, 512,1049},  //duty=16, dutyLt=9
    { 755, 512,1031},  //duty=17, dutyLt=9
    { 765, 512,1015},  //duty=18, dutyLt=9
    { 774, 512,1001},  //duty=19, dutyLt=9
    { 781, 512, 988},  //duty=20, dutyLt=9
    { 788, 512, 977},  //duty=21, dutyLt=9
    { 794, 512, 967},  //duty=22, dutyLt=9
    { 800, 512, 957},  //duty=23, dutyLt=9
    { 805, 512, 948},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 465, 512,2321},  //duty=-1, dutyLt=10
    { 500, 512,1925},  //duty=0, dutyLt=10
    { 530, 512,1695},  //duty=1, dutyLt=10
    { 554, 512,1554},  //duty=2, dutyLt=10
    { 577, 512,1458},  //duty=3, dutyLt=10
    { 596, 512,1384},  //duty=4, dutyLt=10
    { 613, 512,1329},  //duty=5, dutyLt=10
    { 629, 512,1284},  //duty=6, dutyLt=10
    { 643, 512,1247},  //duty=7, dutyLt=10
    { 655, 512,1217},  //duty=8, dutyLt=10
    { 667, 512,1190},  //duty=9, dutyLt=10
    { 677, 512,1167},  //duty=10, dutyLt=10
    { 687, 512,1146},  //duty=11, dutyLt=10
    { 696, 512,1128},  //duty=12, dutyLt=10
    { 705, 512,1113},  //duty=13, dutyLt=10
    { 712, 512,1098},  //duty=14, dutyLt=10
    { 720, 512,1085},  //duty=15, dutyLt=10
    { 733, 512,1062},  //duty=16, dutyLt=10
    { 744, 512,1043},  //duty=17, dutyLt=10
    { 754, 512,1026},  //duty=18, dutyLt=10
    { 763, 512,1012},  //duty=19, dutyLt=10
    { 770, 512, 999},  //duty=20, dutyLt=10
    { 777, 512, 988},  //duty=21, dutyLt=10
    { 783, 512, 977},  //duty=22, dutyLt=10
    { 789, 512, 967},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 465, 512,2308},  //duty=-1, dutyLt=11
    { 497, 512,1942},  //duty=0, dutyLt=11
    { 525, 512,1717},  //duty=1, dutyLt=11
    { 549, 512,1577},  //duty=2, dutyLt=11
    { 570, 512,1481},  //duty=3, dutyLt=11
    { 589, 512,1406},  //duty=4, dutyLt=11
    { 605, 512,1350},  //duty=5, dutyLt=11
    { 620, 512,1304},  //duty=6, dutyLt=11
    { 634, 512,1267},  //duty=7, dutyLt=11
    { 646, 512,1235},  //duty=8, dutyLt=11
    { 658, 512,1208},  //duty=9, dutyLt=11
    { 668, 512,1184},  //duty=10, dutyLt=11
    { 678, 512,1163},  //duty=11, dutyLt=11
    { 687, 512,1144},  //duty=12, dutyLt=11
    { 695, 512,1129},  //duty=13, dutyLt=11
    { 703, 512,1113},  //duty=14, dutyLt=11
    { 710, 512,1100},  //duty=15, dutyLt=11
    { 723, 512,1075},  //duty=16, dutyLt=11
    { 734, 512,1056},  //duty=17, dutyLt=11
    { 744, 512,1039},  //duty=18, dutyLt=11
    { 753, 512,1024},  //duty=19, dutyLt=11
    { 761, 512,1010},  //duty=20, dutyLt=11
    { 767, 512, 998},  //duty=21, dutyLt=11
    { 774, 512, 987},  //duty=22, dutyLt=11
    { 780, 512, 976},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 465, 512,2310},  //duty=-1, dutyLt=12
    { 495, 512,1952},  //duty=0, dutyLt=12
    { 522, 512,1737},  //duty=1, dutyLt=12
    { 545, 512,1598},  //duty=2, dutyLt=12
    { 565, 512,1501},  //duty=3, dutyLt=12
    { 582, 512,1427},  //duty=4, dutyLt=12
    { 599, 512,1370},  //duty=5, dutyLt=12
    { 613, 512,1323},  //duty=6, dutyLt=12
    { 627, 512,1285},  //duty=7, dutyLt=12
    { 639, 512,1253},  //duty=8, dutyLt=12
    { 650, 512,1225},  //duty=9, dutyLt=12
    { 660, 512,1200},  //duty=10, dutyLt=12
    { 669, 512,1178},  //duty=11, dutyLt=12
    { 678, 512,1160},  //duty=12, dutyLt=12
    { 686, 512,1143},  //duty=13, dutyLt=12
    { 694, 512,1127},  //duty=14, dutyLt=12
    { 701, 512,1113},  //duty=15, dutyLt=12
    { 714, 512,1088},  //duty=16, dutyLt=12
    { 725, 512,1068},  //duty=17, dutyLt=12
    { 735, 512,1050},  //duty=18, dutyLt=12
    { 744, 512,1035},  //duty=19, dutyLt=12
    { 751, 512,1021},  //duty=20, dutyLt=12
    { 758, 512,1008},  //duty=21, dutyLt=12
    { 765, 512, 997},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 512, 512, 512},  //duty=28, dutyLt=12
    { 512, 512, 512},  //duty=29, dutyLt=12
    { 465, 512,2304},  //duty=-1, dutyLt=13
    { 493, 512,1962},  //duty=0, dutyLt=13
    { 519, 512,1754},  //duty=1, dutyLt=13
    { 541, 512,1617},  //duty=2, dutyLt=13
    { 560, 512,1522},  //duty=3, dutyLt=13
    { 577, 512,1447},  //duty=4, dutyLt=13
    { 593, 512,1388},  //duty=5, dutyLt=13
    { 607, 512,1341},  //duty=6, dutyLt=13
    { 620, 512,1302},  //duty=7, dutyLt=13
    { 632, 512,1269},  //duty=8, dutyLt=13
    { 643, 512,1240},  //duty=9, dutyLt=13
    { 652, 512,1216},  //duty=10, dutyLt=13
    { 662, 512,1193},  //duty=11, dutyLt=13
    { 670, 512,1174},  //duty=12, dutyLt=13
    { 678, 512,1157},  //duty=13, dutyLt=13
    { 686, 512,1141},  //duty=14, dutyLt=13
    { 693, 512,1126},  //duty=15, dutyLt=13
    { 706, 512,1100},  //duty=16, dutyLt=13
    { 717, 512,1079},  //duty=17, dutyLt=13
    { 727, 512,1061},  //duty=18, dutyLt=13
    { 735, 512,1045},  //duty=19, dutyLt=13
    { 743, 512,1031},  //duty=20, dutyLt=13
    { 750, 512,1018},  //duty=21, dutyLt=13
    { 756, 512,1006},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 465, 512,2293},  //duty=-1, dutyLt=14
    { 492, 512,1971},  //duty=0, dutyLt=14
    { 516, 512,1766},  //duty=1, dutyLt=14
    { 537, 512,1635},  //duty=2, dutyLt=14
    { 556, 512,1538},  //duty=3, dutyLt=14
    { 573, 512,1465},  //duty=4, dutyLt=14
    { 588, 512,1405},  //duty=5, dutyLt=14
    { 601, 512,1358},  //duty=6, dutyLt=14
    { 614, 512,1318},  //duty=7, dutyLt=14
    { 625, 512,1284},  //duty=8, dutyLt=14
    { 636, 512,1255},  //duty=9, dutyLt=14
    { 645, 512,1230},  //duty=10, dutyLt=14
    { 655, 512,1207},  //duty=11, dutyLt=14
    { 663, 512,1187},  //duty=12, dutyLt=14
    { 671, 512,1170},  //duty=13, dutyLt=14
    { 679, 512,1153},  //duty=14, dutyLt=14
    { 686, 512,1139},  //duty=15, dutyLt=14
    { 698, 512,1112},  //duty=16, dutyLt=14
    { 709, 512,1090},  //duty=17, dutyLt=14
    { 719, 512,1071},  //duty=18, dutyLt=14
    { 728, 512,1055},  //duty=19, dutyLt=14
    { 735, 512,1040},  //duty=20, dutyLt=14
    { 743, 512,1027},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 512, 512, 512},  //duty=28, dutyLt=14
    { 512, 512, 512},  //duty=29, dutyLt=14
    { 466, 512,2289},  //duty=-1, dutyLt=15
    { 491, 512,1978},  //duty=0, dutyLt=15
    { 514, 512,1782},  //duty=1, dutyLt=15
    { 534, 512,1650},  //duty=2, dutyLt=15
    { 552, 512,1554},  //duty=3, dutyLt=15
    { 568, 512,1481},  //duty=4, dutyLt=15
    { 583, 512,1421},  //duty=5, dutyLt=15
    { 596, 512,1373},  //duty=6, dutyLt=15
    { 608, 512,1333},  //duty=7, dutyLt=15
    { 619, 512,1299},  //duty=8, dutyLt=15
    { 630, 512,1268},  //duty=9, dutyLt=15
    { 639, 512,1242},  //duty=10, dutyLt=15
    { 649, 512,1220},  //duty=11, dutyLt=15
    { 657, 512,1200},  //duty=12, dutyLt=15
    { 665, 512,1181},  //duty=13, dutyLt=15
    { 672, 512,1165},  //duty=14, dutyLt=15
    { 679, 512,1150},  //duty=15, dutyLt=15
    { 692, 512,1122},  //duty=16, dutyLt=15
    { 702, 512,1100},  //duty=17, dutyLt=15
    { 712, 512,1081},  //duty=18, dutyLt=15
    { 721, 512,1064},  //duty=19, dutyLt=15
    { 728, 512,1049},  //duty=20, dutyLt=15
    { 736, 512,1036},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 466, 512,2271},  //duty=-1, dutyLt=16
    { 489, 512,1987},  //duty=0, dutyLt=16
    { 511, 512,1801},  //duty=1, dutyLt=16
    { 529, 512,1677},  //duty=2, dutyLt=16
    { 546, 512,1580},  //duty=3, dutyLt=16
    { 561, 512,1508},  //duty=4, dutyLt=16
    { 575, 512,1448},  //duty=5, dutyLt=16
    { 588, 512,1400},  //duty=6, dutyLt=16
    { 599, 512,1358},  //duty=7, dutyLt=16
    { 610, 512,1323},  //duty=8, dutyLt=16
    { 620, 512,1293},  //duty=9, dutyLt=16
    { 630, 512,1267},  //duty=10, dutyLt=16
    { 638, 512,1242},  //duty=11, dutyLt=16
    { 646, 512,1222},  //duty=12, dutyLt=16
    { 654, 512,1203},  //duty=13, dutyLt=16
    { 661, 512,1185},  //duty=14, dutyLt=16
    { 668, 512,1170},  //duty=15, dutyLt=16
    { 680, 512,1142},  //duty=16, dutyLt=16
    { 690, 512,1119},  //duty=17, dutyLt=16
    { 700, 512,1099},  //duty=18, dutyLt=16
    { 709, 512,1082},  //duty=19, dutyLt=16
    { 716, 512,1066},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 512, 512, 512},  //duty=28, dutyLt=16
    { 512, 512, 512},  //duty=29, dutyLt=16
    { 466, 512,2256},  //duty=-1, dutyLt=17
    { 488, 512,1996},  //duty=0, dutyLt=17
    { 508, 512,1821},  //duty=1, dutyLt=17
    { 526, 512,1699},  //duty=2, dutyLt=17
    { 541, 512,1606},  //duty=3, dutyLt=17
    { 555, 512,1532},  //duty=4, dutyLt=17
    { 569, 512,1472},  //duty=5, dutyLt=17
    { 581, 512,1423},  //duty=6, dutyLt=17
    { 592, 512,1381},  //duty=7, dutyLt=17
    { 602, 512,1346},  //duty=8, dutyLt=17
    { 612, 512,1316},  //duty=9, dutyLt=17
    { 621, 512,1288},  //duty=10, dutyLt=17
    { 629, 512,1264},  //duty=11, dutyLt=17
    { 637, 512,1242},  //duty=12, dutyLt=17
    { 644, 512,1222},  //duty=13, dutyLt=17
    { 651, 512,1205},  //duty=14, dutyLt=17
    { 658, 512,1188},  //duty=15, dutyLt=17
    { 670, 512,1160},  //duty=16, dutyLt=17
    { 680, 512,1136},  //duty=17, dutyLt=17
    { 690, 512,1115},  //duty=18, dutyLt=17
    { 698, 512,1097},  //duty=19, dutyLt=17
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
    { 466, 512,2257},  //duty=-1, dutyLt=18
    { 487, 512,2008},  //duty=0, dutyLt=18
    { 506, 512,1837},  //duty=1, dutyLt=18
    { 522, 512,1716},  //duty=2, dutyLt=18
    { 537, 512,1624},  //duty=3, dutyLt=18
    { 551, 512,1551},  //duty=4, dutyLt=18
    { 563, 512,1493},  //duty=5, dutyLt=18
    { 575, 512,1444},  //duty=6, dutyLt=18
    { 586, 512,1402},  //duty=7, dutyLt=18
    { 596, 512,1367},  //duty=8, dutyLt=18
    { 605, 512,1334},  //duty=9, dutyLt=18
    { 614, 512,1307},  //duty=10, dutyLt=18
    { 622, 512,1282},  //duty=11, dutyLt=18
    { 629, 512,1260},  //duty=12, dutyLt=18
    { 636, 512,1240},  //duty=13, dutyLt=18
    { 643, 512,1222},  //duty=14, dutyLt=18
    { 650, 512,1205},  //duty=15, dutyLt=18
    { 661, 512,1176},  //duty=16, dutyLt=18
    { 672, 512,1152},  //duty=17, dutyLt=18
    { 681, 512,1130},  //duty=18, dutyLt=18
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
    { 467, 512,2238},  //duty=-1, dutyLt=19
    { 486, 512,2007},  //duty=0, dutyLt=19
    { 504, 512,1847},  //duty=1, dutyLt=19
    { 520, 512,1732},  //duty=2, dutyLt=19
    { 534, 512,1641},  //duty=3, dutyLt=19
    { 547, 512,1570},  //duty=4, dutyLt=19
    { 559, 512,1511},  //duty=5, dutyLt=19
    { 570, 512,1461},  //duty=6, dutyLt=19
    { 580, 512,1419},  //duty=7, dutyLt=19
    { 590, 512,1384},  //duty=8, dutyLt=19
    { 599, 512,1351},  //duty=9, dutyLt=19
    { 607, 512,1324},  //duty=10, dutyLt=19
    { 615, 512,1298},  //duty=11, dutyLt=19
    { 623, 512,1276},  //duty=12, dutyLt=19
    { 630, 512,1255},  //duty=13, dutyLt=19
    { 636, 512,1236},  //duty=14, dutyLt=19
    { 642, 512,1220},  //duty=15, dutyLt=19
    { 654, 512,1190},  //duty=16, dutyLt=19
    { 664, 512,1165},  //duty=17, dutyLt=19
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
    { 467, 512,2238},  //duty=-1, dutyLt=20
    { 486, 512,2014},  //duty=0, dutyLt=20
    { 503, 512,1857},  //duty=1, dutyLt=20
    { 518, 512,1744},  //duty=2, dutyLt=20
    { 531, 512,1655},  //duty=3, dutyLt=20
    { 544, 512,1584},  //duty=4, dutyLt=20
    { 555, 512,1526},  //duty=5, dutyLt=20
    { 566, 512,1477},  //duty=6, dutyLt=20
    { 576, 512,1435},  //duty=7, dutyLt=20
    { 585, 512,1398},  //duty=8, dutyLt=20
    { 594, 512,1366},  //duty=9, dutyLt=20
    { 602, 512,1337},  //duty=10, dutyLt=20
    { 610, 512,1312},  //duty=11, dutyLt=20
    { 617, 512,1290},  //duty=12, dutyLt=20
    { 624, 512,1269},  //duty=13, dutyLt=20
    { 631, 512,1250},  //duty=14, dutyLt=20
    { 637, 512,1232},  //duty=15, dutyLt=20
    { 648, 512,1203},  //duty=16, dutyLt=20
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
    { 468, 512,2223},  //duty=-1, dutyLt=21
    { 486, 512,2014},  //duty=0, dutyLt=21
    { 502, 512,1866},  //duty=1, dutyLt=21
    { 516, 512,1753},  //duty=2, dutyLt=21
    { 529, 512,1666},  //duty=3, dutyLt=21
    { 541, 512,1597},  //duty=4, dutyLt=21
    { 552, 512,1539},  //duty=5, dutyLt=21
    { 563, 512,1490},  //duty=6, dutyLt=21
    { 572, 512,1448},  //duty=7, dutyLt=21
    { 581, 512,1414},  //duty=8, dutyLt=21
    { 590, 512,1380},  //duty=9, dutyLt=21
    { 598, 512,1351},  //duty=10, dutyLt=21
    { 606, 512,1325},  //duty=11, dutyLt=21
    { 613, 512,1302},  //duty=12, dutyLt=21
    { 619, 512,1281},  //duty=13, dutyLt=21
    { 625, 512,1262},  //duty=14, dutyLt=21
    { 632, 512,1244},  //duty=15, dutyLt=21
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
    { 468, 512,2214},  //duty=-1, dutyLt=22
    { 485, 512,2014},  //duty=0, dutyLt=22
    { 501, 512,1870},  //duty=1, dutyLt=22
    { 515, 512,1763},  //duty=2, dutyLt=22
    { 528, 512,1676},  //duty=3, dutyLt=22
    { 539, 512,1608},  //duty=4, dutyLt=22
    { 550, 512,1550},  //duty=5, dutyLt=22
    { 560, 512,1502},  //duty=6, dutyLt=22
    { 570, 512,1460},  //duty=7, dutyLt=22
    { 578, 512,1423},  //duty=8, dutyLt=22
    { 587, 512,1391},  //duty=9, dutyLt=22
    { 594, 512,1362},  //duty=10, dutyLt=22
    { 602, 512,1336},  //duty=11, dutyLt=22
    { 609, 512,1313},  //duty=12, dutyLt=22
    { 615, 512,1291},  //duty=13, dutyLt=22
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
    { 469, 512,2208},  //duty=-1, dutyLt=23
    { 485, 512,2016},  //duty=0, dutyLt=23
    { 500, 512,1874},  //duty=1, dutyLt=23
    { 514, 512,1770},  //duty=2, dutyLt=23
    { 526, 512,1683},  //duty=3, dutyLt=23
    { 538, 512,1616},  //duty=4, dutyLt=23
    { 548, 512,1559},  //duty=5, dutyLt=23
    { 558, 512,1511},  //duty=6, dutyLt=23
    { 567, 512,1469},  //duty=7, dutyLt=23
    { 576, 512,1433},  //duty=8, dutyLt=23
    { 584, 512,1400},  //duty=9, dutyLt=23
    { 591, 512,1371},  //duty=10, dutyLt=23
    { 599, 512,1345},  //duty=11, dutyLt=23
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
    { 469, 512,2203},  //duty=-1, dutyLt=24
    { 485, 512,2015},  //duty=0, dutyLt=24
    { 500, 512,1879},  //duty=1, dutyLt=24
    { 513, 512,1775},  //duty=2, dutyLt=24
    { 525, 512,1691},  //duty=3, dutyLt=24
    { 536, 512,1624},  //duty=4, dutyLt=24
    { 547, 512,1567},  //duty=5, dutyLt=24
    { 556, 512,1519},  //duty=6, dutyLt=24
    { 565, 512,1476},  //duty=7, dutyLt=24
    { 573, 512,1440},  //duty=8, dutyLt=24
    { 581, 512,1409},  //duty=9, dutyLt=24
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
    { 470, 512,2196},  //duty=-1, dutyLt=25
    { 485, 512,2015},  //duty=0, dutyLt=25
    { 500, 512,1880},  //duty=1, dutyLt=25
    { 513, 512,1780},  //duty=2, dutyLt=25
    { 524, 512,1697},  //duty=3, dutyLt=25
    { 535, 512,1633},  //duty=4, dutyLt=25
    { 545, 512,1574},  //duty=5, dutyLt=25
    { 555, 512,1526},  //duty=6, dutyLt=25
    { 563, 512,1484},  //duty=7, dutyLt=25
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
    { 471, 512,2187},  //duty=-1, dutyLt=26
    { 486, 512,2012},  //duty=0, dutyLt=26
    { 500, 512,1881},  //duty=1, dutyLt=26
    { 512, 512,1781},  //duty=2, dutyLt=26
    { 524, 512,1700},  //duty=3, dutyLt=26
    { 534, 512,1635},  //duty=4, dutyLt=26
    { 544, 512,1578},  //duty=5, dutyLt=26
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
    { 471, 512,2179},  //duty=-1, dutyLt=27
    { 486, 512,2010},  //duty=0, dutyLt=27
    { 500, 512,1882},  //duty=1, dutyLt=27
    { 512, 512,1783},  //duty=2, dutyLt=27
    { 523, 512,1706},  //duty=3, dutyLt=27
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
    { 472, 512,2172},  //duty=-1, dutyLt=28
    { 487, 512,2007},  //duty=0, dutyLt=28
    { 500, 512,1881},  //duty=1, dutyLt=28
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
    { 473, 512,2167},  //duty=-1, dutyLt=29
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
