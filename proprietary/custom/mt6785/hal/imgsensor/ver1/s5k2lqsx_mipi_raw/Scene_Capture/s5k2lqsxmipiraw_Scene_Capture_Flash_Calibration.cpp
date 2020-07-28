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
#include "s5k2lqsxmipiraw_Scene_Capture.h"

const FLASH_CALIBRATION_NVRAM_T s5k2lqsxmipiraw_Flash_Calibration_0000 = {
    .engTab = {
        .exp = 10000,
        .afe_gain = 1024,
        .isp_gain = 1024,
        .distance = 0,
        .yTab = {
           0, 317, 653, 973,1283,1587,1882,2172,2453,2728,2999,3260,3520,3769,4014,4251,4482,4954,5383,5794,6181,6542,6890,7203,7501,7781,8040,8278,8490,8685,8863,
         233, 573, 906,1226,1536,1839,2136,2424,2707,2980,3252,3516,3774,4024,4269,4503,4735,5205,5636,6043,6434,6795,7136,7459,7742,8030,8280,8512,8731,8923,  -1,
         478, 822,1156,1477,1788,2091,2387,2674,2956,3229,3501,3762,4020,4270,4514,4748,4978,5449,5876,6282,6669,7035,7372,7694,7978,8254,8510,8740,8952,9141,  -1,
         721,1062,1396,1717,2029,2332,2627,2914,3195,3467,3739,3999,4256,4504,4747,4983,5211,5682,6107,6512,6903,7259,7597,7911,8196,8475,8736,8967,9177,  -1,  -1,
         953,1297,1632,1954,2264,2567,2861,3148,3428,3701,3972,4230,4488,4736,4977,5214,5441,5909,6335,6738,7125,7482,7816,8132,8416,8692,8943,9182,9388,  -1,  -1,
        1176,1521,1855,2179,2488,2791,3085,3373,3653,3924,4194,4454,4709,4956,5198,5433,5661,6131,6553,6960,7341,7698,8039,8348,8643,8922,9164,9384,  -1,  -1,  -1,
        1394,1740,2075,2398,2708,3011,3305,3592,3871,4142,4410,4670,4925,5172,5412,5645,5873,6340,6774,7163,7546,7903,8239,8552,8834,9101,9350,9573,  -1,  -1,  -1,
        1604,1951,2289,2608,2919,3224,3517,3804,4083,4354,4622,4880,5135,5381,5621,5854,6080,6550,6972,7373,7751,8105,8440,8750,9033,9307,9555,  -1,  -1,  -1,  -1,
        1812,2159,2495,2818,3127,3430,3722,4009,4289,4557,4827,5082,5338,5583,5823,6055,6281,6750,7172,7575,7951,8308,8644,8954,9255,9517,9751,  -1,  -1,  -1,  -1,
        2011,2359,2696,3018,3328,3630,3923,4210,4487,4756,5024,5282,5534,5780,6019,6251,6477,6945,7365,7763,8141,8489,8822,9130,9413,9685,  -1,  -1,  -1,  -1,  -1,
        2208,2557,2893,3215,3524,3826,4118,4407,4681,4951,5219,5474,5728,5972,6211,6443,6667,7135,7551,7950,8328,8677,9008,9315,9591,9867,  -1,  -1,  -1,  -1,  -1,
        2394,2743,3079,3402,3711,4014,4306,4590,4867,5136,5403,5659,5911,6156,6392,6622,6849,7316,7735,8133,8508,8853,9183,9487,9774,  -1,  -1,  -1,  -1,  -1,  -1,
        2581,2932,3269,3590,3898,4199,4491,4776,5052,5321,5587,5843,6094,6337,6573,6805,7025,7492,7909,8314,8683,9030,9358,9665,9943,  -1,  -1,  -1,  -1,  -1,  -1,
        2759,3109,3446,3766,4076,4377,4667,4952,5228,5496,5762,6018,6270,6511,6746,6977,7200,7667,8083,8482,8854,9193,9523,9821,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        2937,3287,3623,3944,4254,4554,4844,5129,5403,5670,5936,6191,6441,6683,6918,7146,7368,7838,8248,8644,9014,9362,9687,9985,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3108,3457,3793,4113,4423,4721,5012,5296,5570,5836,6100,6353,6604,6847,7084,7309,7533,8003,8411,8804,9176,9520,9840,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3275,3626,3960,4280,4590,4889,5179,5460,5733,5999,6264,6516,6768,7007,7242,7467,7688,8159,8575,8969,9330,9681,9999,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3604,3957,4293,4617,4924,5228,5517,5801,6074,6341,6609,6864,7112,7357,7588,7821,8039,8480,8885,9271,9641,9976,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3911,4264,4598,4921,5228,5528,5816,6100,6373,6636,6904,7154,7404,7642,7875,8104,8325,8754,9175,9554,9915,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4198,4552,4883,5211,5513,5812,6099,6382,6652,6914,7182,7433,7676,7915,8153,8380,8605,9026,9429,9812,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4462,4816,5148,5475,5783,6077,6363,6642,6911,7173,7440,7687,7933,8170,8403,8629,8842,9270,9670,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4715,5067,5397,5722,6023,6320,6607,6883,7156,7417,7679,7927,8172,8409,8634,8871,9079,9505,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4948,5298,5632,5949,6247,6550,6833,7114,7380,7641,7902,8148,8394,8626,8854,9070,9291,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5166,5520,5845,6161,6463,6758,7051,7316,7585,7841,8098,8347,8589,8820,9044,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5351,5697,6046,6346,6644,6937,7218,7490,7764,8017,8275,8516,8765,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5553,5891,6216,6530,6828,7126,7400,7673,7940,8196,8447,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5710,6055,6384,6691,6992,7281,7560,7834,8096,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5879,6220,6543,6846,7136,7428,7708,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6017,6361,6674,6998,7285,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6151,6475,6802,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6293,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        },
    },
    .flashWBGain = {
    { 506, 512, 512},  //duty=-1, dutyLt=-1
    {1010, 512, 851},  //duty=0, dutyLt=-1
    {1012, 512, 848},  //duty=1, dutyLt=-1
    {1009, 512, 842},  //duty=2, dutyLt=-1
    {1007, 512, 836},  //duty=3, dutyLt=-1
    {1005, 512, 832},  //duty=4, dutyLt=-1
    {1004, 512, 828},  //duty=5, dutyLt=-1
    {1003, 512, 821},  //duty=6, dutyLt=-1
    {1010, 512, 825},  //duty=7, dutyLt=-1
    {1009, 512, 822},  //duty=8, dutyLt=-1
    {1007, 512, 818},  //duty=9, dutyLt=-1
    {1006, 512, 815},  //duty=10, dutyLt=-1
    {1005, 512, 812},  //duty=11, dutyLt=-1
    {1004, 512, 809},  //duty=12, dutyLt=-1
    {1003, 512, 807},  //duty=13, dutyLt=-1
    {1003, 512, 804},  //duty=14, dutyLt=-1
    {1002, 512, 801},  //duty=15, dutyLt=-1
    {1001, 512, 797},  //duty=16, dutyLt=-1
    {1000, 512, 793},  //duty=17, dutyLt=-1
    { 999, 512, 789},  //duty=18, dutyLt=-1
    { 999, 512, 785},  //duty=19, dutyLt=-1
    { 998, 512, 781},  //duty=20, dutyLt=-1
    { 998, 512, 778},  //duty=21, dutyLt=-1
    { 998, 512, 775},  //duty=22, dutyLt=-1
    { 998, 512, 772},  //duty=23, dutyLt=-1
    { 998, 512, 769},  //duty=24, dutyLt=-1
    { 999, 512, 766},  //duty=25, dutyLt=-1
    { 999, 512, 763},  //duty=26, dutyLt=-1
    {1000, 512, 760},  //duty=27, dutyLt=-1
    {1001, 512, 758},  //duty=28, dutyLt=-1
    {1002, 512, 755},  //duty=29, dutyLt=-1
    { 504, 512,1871},  //duty=-1, dutyLt=0
    { 722, 512,1082},  //duty=0, dutyLt=0
    { 808, 512, 980},  //duty=1, dutyLt=0
    { 850, 512, 936},  //duty=2, dutyLt=0
    { 876, 512, 910},  //duty=3, dutyLt=0
    { 894, 512, 893},  //duty=4, dutyLt=0
    { 907, 512, 878},  //duty=5, dutyLt=0
    { 923, 512, 874},  //duty=6, dutyLt=0
    { 930, 512, 865},  //duty=7, dutyLt=0
    { 935, 512, 857},  //duty=8, dutyLt=0
    { 940, 512, 852},  //duty=9, dutyLt=0
    { 943, 512, 847},  //duty=10, dutyLt=0
    { 946, 512, 841},  //duty=11, dutyLt=0
    { 949, 512, 837},  //duty=12, dutyLt=0
    { 951, 512, 832},  //duty=13, dutyLt=0
    { 953, 512, 828},  //duty=14, dutyLt=0
    { 954, 512, 825},  //duty=15, dutyLt=0
    { 957, 512, 818},  //duty=16, dutyLt=0
    { 960, 512, 812},  //duty=17, dutyLt=0
    { 962, 512, 807},  //duty=18, dutyLt=0
    { 963, 512, 802},  //duty=19, dutyLt=0
    { 965, 512, 798},  //duty=20, dutyLt=0
    { 966, 512, 793},  //duty=21, dutyLt=0
    { 967, 512, 790},  //duty=22, dutyLt=0
    { 969, 512, 786},  //duty=23, dutyLt=0
    { 971, 512, 782},  //duty=24, dutyLt=0
    { 972, 512, 779},  //duty=25, dutyLt=0
    { 973, 512, 776},  //duty=26, dutyLt=0
    { 975, 512, 773},  //duty=27, dutyLt=0
    { 976, 512, 770},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 503, 512,1842},  //duty=-1, dutyLt=1
    { 645, 512,1230},  //duty=0, dutyLt=1
    { 721, 512,1079},  //duty=1, dutyLt=1
    { 768, 512,1011},  //duty=2, dutyLt=1
    { 801, 512, 971},  //duty=3, dutyLt=1
    { 825, 512, 943},  //duty=4, dutyLt=1
    { 848, 512, 930},  //duty=5, dutyLt=1
    { 862, 512, 913},  //duty=6, dutyLt=1
    { 873, 512, 901},  //duty=7, dutyLt=1
    { 883, 512, 891},  //duty=8, dutyLt=1
    { 890, 512, 882},  //duty=9, dutyLt=1
    { 896, 512, 874},  //duty=10, dutyLt=1
    { 902, 512, 867},  //duty=11, dutyLt=1
    { 907, 512, 861},  //duty=12, dutyLt=1
    { 911, 512, 855},  //duty=13, dutyLt=1
    { 914, 512, 850},  //duty=14, dutyLt=1
    { 917, 512, 845},  //duty=15, dutyLt=1
    { 923, 512, 837},  //duty=16, dutyLt=1
    { 928, 512, 829},  //duty=17, dutyLt=1
    { 931, 512, 823},  //duty=18, dutyLt=1
    { 935, 512, 817},  //duty=19, dutyLt=1
    { 937, 512, 812},  //duty=20, dutyLt=1
    { 940, 512, 807},  //duty=21, dutyLt=1
    { 942, 512, 803},  //duty=22, dutyLt=1
    { 945, 512, 798},  //duty=23, dutyLt=1
    { 947, 512, 794},  //duty=24, dutyLt=1
    { 949, 512, 790},  //duty=25, dutyLt=1
    { 951, 512, 787},  //duty=26, dutyLt=1
    { 953, 512, 783},  //duty=27, dutyLt=1
    { 954, 512, 780},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 503, 512,1867},  //duty=-1, dutyLt=2
    { 608, 512,1320},  //duty=0, dutyLt=2
    { 674, 512,1153},  //duty=1, dutyLt=2
    { 720, 512,1071},  //duty=2, dutyLt=2
    { 753, 512,1022},  //duty=3, dutyLt=2
    { 779, 512, 983},  //duty=4, dutyLt=2
    { 802, 512, 968},  //duty=5, dutyLt=2
    { 818, 512, 949},  //duty=6, dutyLt=2
    { 831, 512, 933},  //duty=7, dutyLt=2
    { 842, 512, 919},  //duty=8, dutyLt=2
    { 852, 512, 909},  //duty=9, dutyLt=2
    { 860, 512, 899},  //duty=10, dutyLt=2
    { 867, 512, 891},  //duty=11, dutyLt=2
    { 872, 512, 883},  //duty=12, dutyLt=2
    { 878, 512, 876},  //duty=13, dutyLt=2
    { 882, 512, 870},  //duty=14, dutyLt=2
    { 887, 512, 864},  //duty=15, dutyLt=2
    { 895, 512, 854},  //duty=16, dutyLt=2
    { 901, 512, 845},  //duty=17, dutyLt=2
    { 906, 512, 838},  //duty=18, dutyLt=2
    { 910, 512, 831},  //duty=19, dutyLt=2
    { 914, 512, 825},  //duty=20, dutyLt=2
    { 918, 512, 820},  //duty=21, dutyLt=2
    { 921, 512, 815},  //duty=22, dutyLt=2
    { 924, 512, 810},  //duty=23, dutyLt=2
    { 927, 512, 806},  //duty=24, dutyLt=2
    { 929, 512, 802},  //duty=25, dutyLt=2
    { 932, 512, 798},  //duty=26, dutyLt=2
    { 934, 512, 794},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 502, 512,1848},  //duty=-1, dutyLt=3
    { 587, 512,1384},  //duty=0, dutyLt=3
    { 645, 512,1212},  //duty=1, dutyLt=3
    { 687, 512,1122},  //duty=2, dutyLt=3
    { 720, 512,1062},  //duty=3, dutyLt=3
    { 748, 512,1033},  //duty=4, dutyLt=3
    { 767, 512,1004},  //duty=5, dutyLt=3
    { 785, 512, 980},  //duty=6, dutyLt=3
    { 798, 512, 962},  //duty=7, dutyLt=3
    { 810, 512, 947},  //duty=8, dutyLt=3
    { 821, 512, 934},  //duty=9, dutyLt=3
    { 829, 512, 922},  //duty=10, dutyLt=3
    { 838, 512, 913},  //duty=11, dutyLt=3
    { 844, 512, 904},  //duty=12, dutyLt=3
    { 851, 512, 895},  //duty=13, dutyLt=3
    { 856, 512, 889},  //duty=14, dutyLt=3
    { 861, 512, 882},  //duty=15, dutyLt=3
    { 870, 512, 870},  //duty=16, dutyLt=3
    { 877, 512, 861},  //duty=17, dutyLt=3
    { 883, 512, 852},  //duty=18, dutyLt=3
    { 889, 512, 845},  //duty=19, dutyLt=3
    { 893, 512, 838},  //duty=20, dutyLt=3
    { 898, 512, 832},  //duty=21, dutyLt=3
    { 901, 512, 827},  //duty=22, dutyLt=3
    { 905, 512, 821},  //duty=23, dutyLt=3
    { 908, 512, 816},  //duty=24, dutyLt=3
    { 911, 512, 812},  //duty=25, dutyLt=3
    { 914, 512, 808},  //duty=26, dutyLt=3
    { 916, 512, 804},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 502, 512,1840},  //duty=-1, dutyLt=4
    { 573, 512,1430},  //duty=0, dutyLt=4
    { 625, 512,1258},  //duty=1, dutyLt=4
    { 664, 512,1162},  //duty=2, dutyLt=4
    { 696, 512,1109},  //duty=3, dutyLt=4
    { 721, 512,1066},  //duty=4, dutyLt=4
    { 741, 512,1034},  //duty=5, dutyLt=4
    { 758, 512,1008},  //duty=6, dutyLt=4
    { 772, 512, 988},  //duty=7, dutyLt=4
    { 785, 512, 971},  //duty=8, dutyLt=4
    { 796, 512, 956},  //duty=9, dutyLt=4
    { 805, 512, 943},  //duty=10, dutyLt=4
    { 813, 512, 932},  //duty=11, dutyLt=4
    { 821, 512, 922},  //duty=12, dutyLt=4
    { 828, 512, 914},  //duty=13, dutyLt=4
    { 834, 512, 905},  //duty=14, dutyLt=4
    { 839, 512, 898},  //duty=15, dutyLt=4
    { 849, 512, 885},  //duty=16, dutyLt=4
    { 857, 512, 875},  //duty=17, dutyLt=4
    { 864, 512, 865},  //duty=18, dutyLt=4
    { 871, 512, 857},  //duty=19, dutyLt=4
    { 876, 512, 850},  //duty=20, dutyLt=4
    { 881, 512, 844},  //duty=21, dutyLt=4
    { 885, 512, 838},  //duty=22, dutyLt=4
    { 889, 512, 832},  //duty=23, dutyLt=4
    { 892, 512, 827},  //duty=24, dutyLt=4
    { 896, 512, 822},  //duty=25, dutyLt=4
    { 898, 512, 817},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 502, 512,1831},  //duty=-1, dutyLt=5
    { 564, 512,1465},  //duty=0, dutyLt=5
    { 610, 512,1295},  //duty=1, dutyLt=5
    { 647, 512,1193},  //duty=2, dutyLt=5
    { 677, 512,1142},  //duty=3, dutyLt=5
    { 700, 512,1097},  //duty=4, dutyLt=5
    { 720, 512,1062},  //duty=5, dutyLt=5
    { 737, 512,1034},  //duty=6, dutyLt=5
    { 751, 512,1011},  //duty=7, dutyLt=5
    { 764, 512, 993},  //duty=8, dutyLt=5
    { 775, 512, 976},  //duty=9, dutyLt=5
    { 785, 512, 963},  //duty=10, dutyLt=5
    { 793, 512, 950},  //duty=11, dutyLt=5
    { 801, 512, 939},  //duty=12, dutyLt=5
    { 808, 512, 931},  //duty=13, dutyLt=5
    { 815, 512, 922},  //duty=14, dutyLt=5
    { 820, 512, 914},  //duty=15, dutyLt=5
    { 831, 512, 899},  //duty=16, dutyLt=5
    { 840, 512, 888},  //duty=17, dutyLt=5
    { 848, 512, 877},  //duty=18, dutyLt=5
    { 855, 512, 869},  //duty=19, dutyLt=5
    { 860, 512, 861},  //duty=20, dutyLt=5
    { 865, 512, 854},  //duty=21, dutyLt=5
    { 870, 512, 848},  //duty=22, dutyLt=5
    { 874, 512, 842},  //duty=23, dutyLt=5
    { 878, 512, 836},  //duty=24, dutyLt=5
    { 881, 512, 831},  //duty=25, dutyLt=5
    { 885, 512, 826},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 502, 512,1823},  //duty=-1, dutyLt=6
    { 556, 512,1491},  //duty=0, dutyLt=6
    { 599, 512,1325},  //duty=1, dutyLt=6
    { 634, 512,1237},  //duty=2, dutyLt=6
    { 661, 512,1172},  //duty=3, dutyLt=6
    { 684, 512,1124},  //duty=4, dutyLt=6
    { 703, 512,1086},  //duty=5, dutyLt=6
    { 719, 512,1057},  //duty=6, dutyLt=6
    { 734, 512,1033},  //duty=7, dutyLt=6
    { 746, 512,1013},  //duty=8, dutyLt=6
    { 757, 512, 996},  //duty=9, dutyLt=6
    { 767, 512, 981},  //duty=10, dutyLt=6
    { 776, 512, 968},  //duty=11, dutyLt=6
    { 785, 512, 956},  //duty=12, dutyLt=6
    { 792, 512, 945},  //duty=13, dutyLt=6
    { 798, 512, 936},  //duty=14, dutyLt=6
    { 804, 512, 927},  //duty=15, dutyLt=6
    { 816, 512, 913},  //duty=16, dutyLt=6
    { 825, 512, 900},  //duty=17, dutyLt=6
    { 833, 512, 889},  //duty=18, dutyLt=6
    { 840, 512, 880},  //duty=19, dutyLt=6
    { 846, 512, 871},  //duty=20, dutyLt=6
    { 852, 512, 864},  //duty=21, dutyLt=6
    { 856, 512, 858},  //duty=22, dutyLt=6
    { 861, 512, 851},  //duty=23, dutyLt=6
    { 865, 512, 845},  //duty=24, dutyLt=6
    { 868, 512, 840},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 502, 512,1819},  //duty=-1, dutyLt=7
    { 551, 512,1514},  //duty=0, dutyLt=7
    { 592, 512,1343},  //duty=1, dutyLt=7
    { 623, 512,1265},  //duty=2, dutyLt=7
    { 648, 512,1196},  //duty=3, dutyLt=7
    { 670, 512,1148},  //duty=4, dutyLt=7
    { 689, 512,1109},  //duty=5, dutyLt=7
    { 705, 512,1077},  //duty=6, dutyLt=7
    { 719, 512,1053},  //duty=7, dutyLt=7
    { 731, 512,1032},  //duty=8, dutyLt=7
    { 742, 512,1012},  //duty=9, dutyLt=7
    { 752, 512, 997},  //duty=10, dutyLt=7
    { 761, 512, 983},  //duty=11, dutyLt=7
    { 769, 512, 971},  //duty=12, dutyLt=7
    { 777, 512, 960},  //duty=13, dutyLt=7
    { 784, 512, 950},  //duty=14, dutyLt=7
    { 790, 512, 941},  //duty=15, dutyLt=7
    { 802, 512, 925},  //duty=16, dutyLt=7
    { 811, 512, 912},  //duty=17, dutyLt=7
    { 820, 512, 900},  //duty=18, dutyLt=7
    { 827, 512, 890},  //duty=19, dutyLt=7
    { 833, 512, 882},  //duty=20, dutyLt=7
    { 839, 512, 874},  //duty=21, dutyLt=7
    { 844, 512, 867},  //duty=22, dutyLt=7
    { 849, 512, 860},  //duty=23, dutyLt=7
    { 853, 512, 854},  //duty=24, dutyLt=7
    { 856, 512, 848},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 502, 512,1813},  //duty=-1, dutyLt=8
    { 547, 512,1529},  //duty=0, dutyLt=8
    { 584, 512,1388},  //duty=1, dutyLt=8
    { 613, 512,1287},  //duty=2, dutyLt=8
    { 637, 512,1219},  //duty=3, dutyLt=8
    { 658, 512,1168},  //duty=4, dutyLt=8
    { 676, 512,1128},  //duty=5, dutyLt=8
    { 692, 512,1097},  //duty=6, dutyLt=8
    { 706, 512,1070},  //duty=7, dutyLt=8
    { 718, 512,1049},  //duty=8, dutyLt=8
    { 729, 512,1029},  //duty=9, dutyLt=8
    { 739, 512,1012},  //duty=10, dutyLt=8
    { 748, 512, 998},  //duty=11, dutyLt=8
    { 756, 512, 985},  //duty=12, dutyLt=8
    { 764, 512, 973},  //duty=13, dutyLt=8
    { 771, 512, 963},  //duty=14, dutyLt=8
    { 777, 512, 954},  //duty=15, dutyLt=8
    { 789, 512, 936},  //duty=16, dutyLt=8
    { 799, 512, 923},  //duty=17, dutyLt=8
    { 808, 512, 911},  //duty=18, dutyLt=8
    { 815, 512, 900},  //duty=19, dutyLt=8
    { 822, 512, 891},  //duty=20, dutyLt=8
    { 828, 512, 883},  //duty=21, dutyLt=8
    { 833, 512, 875},  //duty=22, dutyLt=8
    { 838, 512, 868},  //duty=23, dutyLt=8
    { 842, 512, 862},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 503, 512,1810},  //duty=-1, dutyLt=9
    { 546, 512,1532},  //duty=0, dutyLt=9
    { 578, 512,1407},  //duty=1, dutyLt=9
    { 605, 512,1309},  //duty=2, dutyLt=9
    { 629, 512,1240},  //duty=3, dutyLt=9
    { 649, 512,1188},  //duty=4, dutyLt=9
    { 666, 512,1147},  //duty=5, dutyLt=9
    { 681, 512,1115},  //duty=6, dutyLt=9
    { 695, 512,1087},  //duty=7, dutyLt=9
    { 707, 512,1064},  //duty=8, dutyLt=9
    { 718, 512,1043},  //duty=9, dutyLt=9
    { 728, 512,1026},  //duty=10, dutyLt=9
    { 737, 512,1011},  //duty=11, dutyLt=9
    { 745, 512, 998},  //duty=12, dutyLt=9
    { 753, 512, 986},  //duty=13, dutyLt=9
    { 760, 512, 975},  //duty=14, dutyLt=9
    { 766, 512, 966},  //duty=15, dutyLt=9
    { 778, 512, 948},  //duty=16, dutyLt=9
    { 788, 512, 933},  //duty=17, dutyLt=9
    { 797, 512, 920},  //duty=18, dutyLt=9
    { 804, 512, 910},  //duty=19, dutyLt=9
    { 811, 512, 900},  //duty=20, dutyLt=9
    { 817, 512, 891},  //duty=21, dutyLt=9
    { 822, 512, 883},  //duty=22, dutyLt=9
    { 827, 512, 876},  //duty=23, dutyLt=9
    { 832, 512, 870},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 503, 512,1803},  //duty=-1, dutyLt=10
    { 542, 512,1572},  //duty=0, dutyLt=10
    { 573, 512,1422},  //duty=1, dutyLt=10
    { 599, 512,1327},  //duty=2, dutyLt=10
    { 621, 512,1257},  //duty=3, dutyLt=10
    { 640, 512,1205},  //duty=4, dutyLt=10
    { 657, 512,1164},  //duty=5, dutyLt=10
    { 672, 512,1129},  //duty=6, dutyLt=10
    { 685, 512,1101},  //duty=7, dutyLt=10
    { 697, 512,1078},  //duty=8, dutyLt=10
    { 708, 512,1057},  //duty=9, dutyLt=10
    { 718, 512,1040},  //duty=10, dutyLt=10
    { 727, 512,1024},  //duty=11, dutyLt=10
    { 735, 512,1010},  //duty=12, dutyLt=10
    { 743, 512, 997},  //duty=13, dutyLt=10
    { 750, 512, 986},  //duty=14, dutyLt=10
    { 756, 512, 976},  //duty=15, dutyLt=10
    { 768, 512, 958},  //duty=16, dutyLt=10
    { 778, 512, 943},  //duty=17, dutyLt=10
    { 787, 512, 930},  //duty=18, dutyLt=10
    { 795, 512, 918},  //duty=19, dutyLt=10
    { 802, 512, 908},  //duty=20, dutyLt=10
    { 808, 512, 899},  //duty=21, dutyLt=10
    { 813, 512, 891},  //duty=22, dutyLt=10
    { 818, 512, 884},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 504, 512,1791},  //duty=-1, dutyLt=11
    { 539, 512,1582},  //duty=0, dutyLt=11
    { 568, 512,1439},  //duty=1, dutyLt=11
    { 593, 512,1343},  //duty=2, dutyLt=11
    { 614, 512,1274},  //duty=3, dutyLt=11
    { 633, 512,1221},  //duty=4, dutyLt=11
    { 649, 512,1179},  //duty=5, dutyLt=11
    { 664, 512,1144},  //duty=6, dutyLt=11
    { 677, 512,1116},  //duty=7, dutyLt=11
    { 688, 512,1091},  //duty=8, dutyLt=11
    { 699, 512,1071},  //duty=9, dutyLt=11
    { 709, 512,1052},  //duty=10, dutyLt=11
    { 718, 512,1036},  //duty=11, dutyLt=11
    { 726, 512,1022},  //duty=12, dutyLt=11
    { 734, 512,1009},  //duty=13, dutyLt=11
    { 740, 512, 997},  //duty=14, dutyLt=11
    { 747, 512, 987},  //duty=15, dutyLt=11
    { 759, 512, 968},  //duty=16, dutyLt=11
    { 769, 512, 952},  //duty=17, dutyLt=11
    { 778, 512, 939},  //duty=18, dutyLt=11
    { 786, 512, 927},  //duty=19, dutyLt=11
    { 793, 512, 917},  //duty=20, dutyLt=11
    { 799, 512, 908},  //duty=21, dutyLt=11
    { 805, 512, 899},  //duty=22, dutyLt=11
    { 810, 512, 891},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 504, 512,1820},  //duty=-1, dutyLt=12
    { 537, 512,1592},  //duty=0, dutyLt=12
    { 565, 512,1452},  //duty=1, dutyLt=12
    { 588, 512,1356},  //duty=2, dutyLt=12
    { 609, 512,1289},  //duty=3, dutyLt=12
    { 627, 512,1235},  //duty=4, dutyLt=12
    { 642, 512,1192},  //duty=5, dutyLt=12
    { 656, 512,1158},  //duty=6, dutyLt=12
    { 669, 512,1129},  //duty=7, dutyLt=12
    { 680, 512,1103},  //duty=8, dutyLt=12
    { 691, 512,1082},  //duty=9, dutyLt=12
    { 701, 512,1064},  //duty=10, dutyLt=12
    { 710, 512,1047},  //duty=11, dutyLt=12
    { 718, 512,1032},  //duty=12, dutyLt=12
    { 725, 512,1019},  //duty=13, dutyLt=12
    { 732, 512,1008},  //duty=14, dutyLt=12
    { 739, 512, 997},  //duty=15, dutyLt=12
    { 751, 512, 977},  //duty=16, dutyLt=12
    { 761, 512, 961},  //duty=17, dutyLt=12
    { 770, 512, 947},  //duty=18, dutyLt=12
    { 778, 512, 935},  //duty=19, dutyLt=12
    { 785, 512, 924},  //duty=20, dutyLt=12
    { 791, 512, 915},  //duty=21, dutyLt=12
    { 797, 512, 906},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 512, 512, 512},  //duty=28, dutyLt=12
    { 512, 512, 512},  //duty=29, dutyLt=12
    { 504, 512,1813},  //duty=-1, dutyLt=13
    { 535, 512,1598},  //duty=0, dutyLt=13
    { 561, 512,1462},  //duty=1, dutyLt=13
    { 584, 512,1370},  //duty=2, dutyLt=13
    { 604, 512,1302},  //duty=3, dutyLt=13
    { 621, 512,1248},  //duty=4, dutyLt=13
    { 636, 512,1206},  //duty=5, dutyLt=13
    { 650, 512,1171},  //duty=6, dutyLt=13
    { 662, 512,1140},  //duty=7, dutyLt=13
    { 674, 512,1116},  //duty=8, dutyLt=13
    { 684, 512,1094},  //duty=9, dutyLt=13
    { 694, 512,1075},  //duty=10, dutyLt=13
    { 702, 512,1058},  //duty=11, dutyLt=13
    { 710, 512,1042},  //duty=12, dutyLt=13
    { 718, 512,1029},  //duty=13, dutyLt=13
    { 725, 512,1017},  //duty=14, dutyLt=13
    { 731, 512,1006},  //duty=15, dutyLt=13
    { 743, 512, 985},  //duty=16, dutyLt=13
    { 753, 512, 969},  //duty=17, dutyLt=13
    { 763, 512, 955},  //duty=18, dutyLt=13
    { 770, 512, 942},  //duty=19, dutyLt=13
    { 778, 512, 932},  //duty=20, dutyLt=13
    { 784, 512, 922},  //duty=21, dutyLt=13
    { 789, 512, 913},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 504, 512,1809},  //duty=-1, dutyLt=14
    { 534, 512,1605},  //duty=0, dutyLt=14
    { 559, 512,1472},  //duty=1, dutyLt=14
    { 581, 512,1382},  //duty=2, dutyLt=14
    { 599, 512,1314},  //duty=3, dutyLt=14
    { 616, 512,1260},  //duty=4, dutyLt=14
    { 631, 512,1217},  //duty=5, dutyLt=14
    { 644, 512,1182},  //duty=6, dutyLt=14
    { 656, 512,1152},  //duty=7, dutyLt=14
    { 667, 512,1126},  //duty=8, dutyLt=14
    { 678, 512,1104},  //duty=9, dutyLt=14
    { 687, 512,1084},  //duty=10, dutyLt=14
    { 696, 512,1068},  //duty=11, dutyLt=14
    { 704, 512,1052},  //duty=12, dutyLt=14
    { 711, 512,1039},  //duty=13, dutyLt=14
    { 718, 512,1026},  //duty=14, dutyLt=14
    { 724, 512,1014},  //duty=15, dutyLt=14
    { 737, 512, 994},  //duty=16, dutyLt=14
    { 747, 512, 977},  //duty=17, dutyLt=14
    { 755, 512, 962},  //duty=18, dutyLt=14
    { 763, 512, 950},  //duty=19, dutyLt=14
    { 771, 512, 938},  //duty=20, dutyLt=14
    { 777, 512, 928},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 512, 512, 512},  //duty=28, dutyLt=14
    { 512, 512, 512},  //duty=29, dutyLt=14
    { 504, 512,1805},  //duty=-1, dutyLt=15
    { 532, 512,1609},  //duty=0, dutyLt=15
    { 556, 512,1481},  //duty=1, dutyLt=15
    { 577, 512,1392},  //duty=2, dutyLt=15
    { 595, 512,1325},  //duty=3, dutyLt=15
    { 612, 512,1271},  //duty=4, dutyLt=15
    { 626, 512,1228},  //duty=5, dutyLt=15
    { 639, 512,1192},  //duty=6, dutyLt=15
    { 651, 512,1162},  //duty=7, dutyLt=15
    { 662, 512,1137},  //duty=8, dutyLt=15
    { 672, 512,1114},  //duty=9, dutyLt=15
    { 681, 512,1094},  //duty=10, dutyLt=15
    { 690, 512,1077},  //duty=11, dutyLt=15
    { 698, 512,1062},  //duty=12, dutyLt=15
    { 705, 512,1047},  //duty=13, dutyLt=15
    { 712, 512,1034},  //duty=14, dutyLt=15
    { 718, 512,1023},  //duty=15, dutyLt=15
    { 730, 512,1002},  //duty=16, dutyLt=15
    { 740, 512, 985},  //duty=17, dutyLt=15
    { 749, 512, 970},  //duty=18, dutyLt=15
    { 757, 512, 957},  //duty=19, dutyLt=15
    { 764, 512, 945},  //duty=20, dutyLt=15
    { 771, 512, 935},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 505, 512,1797},  //duty=-1, dutyLt=16
    { 530, 512,1616},  //duty=0, dutyLt=16
    { 553, 512,1497},  //duty=1, dutyLt=16
    { 572, 512,1408},  //duty=2, dutyLt=16
    { 589, 512,1342},  //duty=3, dutyLt=16
    { 604, 512,1289},  //duty=4, dutyLt=16
    { 618, 512,1247},  //duty=5, dutyLt=16
    { 631, 512,1212},  //duty=6, dutyLt=16
    { 642, 512,1180},  //duty=7, dutyLt=16
    { 652, 512,1154},  //duty=8, dutyLt=16
    { 662, 512,1132},  //duty=9, dutyLt=16
    { 671, 512,1112},  //duty=10, dutyLt=16
    { 680, 512,1093},  //duty=11, dutyLt=16
    { 687, 512,1077},  //duty=12, dutyLt=16
    { 695, 512,1063},  //duty=13, dutyLt=16
    { 701, 512,1050},  //duty=14, dutyLt=16
    { 708, 512,1038},  //duty=15, dutyLt=16
    { 719, 512,1017},  //duty=16, dutyLt=16
    { 729, 512, 999},  //duty=17, dutyLt=16
    { 738, 512, 983},  //duty=18, dutyLt=16
    { 746, 512, 970},  //duty=19, dutyLt=16
    { 753, 512, 958},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 512, 512, 512},  //duty=28, dutyLt=16
    { 512, 512, 512},  //duty=29, dutyLt=16
    { 504, 512,1789},  //duty=-1, dutyLt=17
    { 529, 512,1622},  //duty=0, dutyLt=17
    { 550, 512,1508},  //duty=1, dutyLt=17
    { 568, 512,1422},  //duty=2, dutyLt=17
    { 584, 512,1358},  //duty=3, dutyLt=17
    { 598, 512,1305},  //duty=4, dutyLt=17
    { 611, 512,1263},  //duty=5, dutyLt=17
    { 623, 512,1226},  //duty=6, dutyLt=17
    { 634, 512,1196},  //duty=7, dutyLt=17
    { 645, 512,1170},  //duty=8, dutyLt=17
    { 654, 512,1147},  //duty=9, dutyLt=17
    { 663, 512,1127},  //duty=10, dutyLt=17
    { 671, 512,1108},  //duty=11, dutyLt=17
    { 678, 512,1092},  //duty=12, dutyLt=17
    { 686, 512,1077},  //duty=13, dutyLt=17
    { 692, 512,1064},  //duty=14, dutyLt=17
    { 698, 512,1051},  //duty=15, dutyLt=17
    { 710, 512,1029},  //duty=16, dutyLt=17
    { 719, 512,1012},  //duty=17, dutyLt=17
    { 728, 512, 995},  //duty=18, dutyLt=17
    { 736, 512, 981},  //duty=19, dutyLt=17
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
    { 505, 512,1783},  //duty=-1, dutyLt=18
    { 527, 512,1627},  //duty=0, dutyLt=18
    { 547, 512,1516},  //duty=1, dutyLt=18
    { 564, 512,1436},  //duty=2, dutyLt=18
    { 579, 512,1371},  //duty=3, dutyLt=18
    { 593, 512,1320},  //duty=4, dutyLt=18
    { 606, 512,1278},  //duty=5, dutyLt=18
    { 617, 512,1241},  //duty=6, dutyLt=18
    { 628, 512,1210},  //duty=7, dutyLt=18
    { 638, 512,1184},  //duty=8, dutyLt=18
    { 647, 512,1160},  //duty=9, dutyLt=18
    { 655, 512,1140},  //duty=10, dutyLt=18
    { 664, 512,1122},  //duty=11, dutyLt=18
    { 671, 512,1105},  //duty=12, dutyLt=18
    { 678, 512,1090},  //duty=13, dutyLt=18
    { 684, 512,1076},  //duty=14, dutyLt=18
    { 690, 512,1064},  //duty=15, dutyLt=18
    { 702, 512,1041},  //duty=16, dutyLt=18
    { 711, 512,1023},  //duty=17, dutyLt=18
    { 720, 512,1006},  //duty=18, dutyLt=18
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
    { 505, 512,1776},  //duty=-1, dutyLt=19
    { 527, 512,1629},  //duty=0, dutyLt=19
    { 545, 512,1524},  //duty=1, dutyLt=19
    { 562, 512,1445},  //duty=2, dutyLt=19
    { 576, 512,1383},  //duty=3, dutyLt=19
    { 589, 512,1332},  //duty=4, dutyLt=19
    { 601, 512,1289},  //duty=5, dutyLt=19
    { 612, 512,1254},  //duty=6, dutyLt=19
    { 623, 512,1223},  //duty=7, dutyLt=19
    { 632, 512,1196},  //duty=8, dutyLt=19
    { 641, 512,1174},  //duty=9, dutyLt=19
    { 650, 512,1152},  //duty=10, dutyLt=19
    { 657, 512,1133},  //duty=11, dutyLt=19
    { 665, 512,1116},  //duty=12, dutyLt=19
    { 671, 512,1101},  //duty=13, dutyLt=19
    { 678, 512,1087},  //duty=14, dutyLt=19
    { 684, 512,1074},  //duty=15, dutyLt=19
    { 694, 512,1052},  //duty=16, dutyLt=19
    { 704, 512,1033},  //duty=17, dutyLt=19
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
    { 506, 512,1771},  //duty=-1, dutyLt=20
    { 526, 512,1630},  //duty=0, dutyLt=20
    { 544, 512,1529},  //duty=1, dutyLt=20
    { 559, 512,1453},  //duty=2, dutyLt=20
    { 573, 512,1391},  //duty=3, dutyLt=20
    { 586, 512,1342},  //duty=4, dutyLt=20
    { 598, 512,1300},  //duty=5, dutyLt=20
    { 609, 512,1264},  //duty=6, dutyLt=20
    { 618, 512,1234},  //duty=7, dutyLt=20
    { 628, 512,1207},  //duty=8, dutyLt=20
    { 636, 512,1183},  //duty=9, dutyLt=20
    { 645, 512,1163},  //duty=10, dutyLt=20
    { 652, 512,1143},  //duty=11, dutyLt=20
    { 659, 512,1126},  //duty=12, dutyLt=20
    { 666, 512,1111},  //duty=13, dutyLt=20
    { 672, 512,1097},  //duty=14, dutyLt=20
    { 678, 512,1084},  //duty=15, dutyLt=20
    { 688, 512,1062},  //duty=16, dutyLt=20
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
    { 506, 512,1767},  //duty=-1, dutyLt=21
    { 525, 512,1631},  //duty=0, dutyLt=21
    { 543, 512,1534},  //duty=1, dutyLt=21
    { 557, 512,1459},  //duty=2, dutyLt=21
    { 571, 512,1399},  //duty=3, dutyLt=21
    { 583, 512,1350},  //duty=4, dutyLt=21
    { 595, 512,1309},  //duty=5, dutyLt=21
    { 605, 512,1274},  //duty=6, dutyLt=21
    { 615, 512,1243},  //duty=7, dutyLt=21
    { 624, 512,1217},  //duty=8, dutyLt=21
    { 632, 512,1193},  //duty=9, dutyLt=21
    { 640, 512,1171},  //duty=10, dutyLt=21
    { 647, 512,1153},  //duty=11, dutyLt=21
    { 654, 512,1135},  //duty=12, dutyLt=21
    { 661, 512,1120},  //duty=13, dutyLt=21
    { 667, 512,1105},  //duty=14, dutyLt=21
    { 673, 512,1093},  //duty=15, dutyLt=21
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
    { 507, 512,1760},  //duty=-1, dutyLt=22
    { 525, 512,1632},  //duty=0, dutyLt=22
    { 542, 512,1537},  //duty=1, dutyLt=22
    { 556, 512,1464},  //duty=2, dutyLt=22
    { 569, 512,1406},  //duty=3, dutyLt=22
    { 581, 512,1357},  //duty=4, dutyLt=22
    { 592, 512,1318},  //duty=5, dutyLt=22
    { 602, 512,1281},  //duty=6, dutyLt=22
    { 612, 512,1251},  //duty=7, dutyLt=22
    { 620, 512,1225},  //duty=8, dutyLt=22
    { 629, 512,1201},  //duty=9, dutyLt=22
    { 637, 512,1180},  //duty=10, dutyLt=22
    { 644, 512,1160},  //duty=11, dutyLt=22
    { 651, 512,1143},  //duty=12, dutyLt=22
    { 657, 512,1127},  //duty=13, dutyLt=22
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
    { 507, 512,1755},  //duty=-1, dutyLt=23
    { 525, 512,1633},  //duty=0, dutyLt=23
    { 541, 512,1541},  //duty=1, dutyLt=23
    { 555, 512,1469},  //duty=2, dutyLt=23
    { 568, 512,1410},  //duty=3, dutyLt=23
    { 579, 512,1363},  //duty=4, dutyLt=23
    { 590, 512,1323},  //duty=5, dutyLt=23
    { 600, 512,1288},  //duty=6, dutyLt=23
    { 609, 512,1258},  //duty=7, dutyLt=23
    { 618, 512,1231},  //duty=8, dutyLt=23
    { 626, 512,1207},  //duty=9, dutyLt=23
    { 634, 512,1186},  //duty=10, dutyLt=23
    { 641, 512,1167},  //duty=11, dutyLt=23
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
    { 508, 512,1752},  //duty=-1, dutyLt=24
    { 525, 512,1631},  //duty=0, dutyLt=24
    { 540, 512,1542},  //duty=1, dutyLt=24
    { 554, 512,1472},  //duty=2, dutyLt=24
    { 566, 512,1415},  //duty=3, dutyLt=24
    { 578, 512,1369},  //duty=4, dutyLt=24
    { 588, 512,1328},  //duty=5, dutyLt=24
    { 598, 512,1293},  //duty=6, dutyLt=24
    { 607, 512,1264},  //duty=7, dutyLt=24
    { 616, 512,1238},  //duty=8, dutyLt=24
    { 624, 512,1213},  //duty=9, dutyLt=24
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
    { 509, 512,1746},  //duty=-1, dutyLt=25
    { 525, 512,1631},  //duty=0, dutyLt=25
    { 540, 512,1544},  //duty=1, dutyLt=25
    { 554, 512,1476},  //duty=2, dutyLt=25
    { 566, 512,1420},  //duty=3, dutyLt=25
    { 577, 512,1372},  //duty=4, dutyLt=25
    { 587, 512,1333},  //duty=5, dutyLt=25
    { 596, 512,1299},  //duty=6, dutyLt=25
    { 606, 512,1269},  //duty=7, dutyLt=25
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
    { 509, 512,1741},  //duty=-1, dutyLt=26
    { 526, 512,1630},  //duty=0, dutyLt=26
    { 540, 512,1546},  //duty=1, dutyLt=26
    { 553, 512,1477},  //duty=2, dutyLt=26
    { 565, 512,1423},  //duty=3, dutyLt=26
    { 576, 512,1376},  //duty=4, dutyLt=26
    { 586, 512,1337},  //duty=5, dutyLt=26
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
    { 510, 512,1736},  //duty=-1, dutyLt=27
    { 526, 512,1628},  //duty=0, dutyLt=27
    { 540, 512,1545},  //duty=1, dutyLt=27
    { 553, 512,1479},  //duty=2, dutyLt=27
    { 564, 512,1425},  //duty=3, dutyLt=27
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
    { 511, 512,1732},  //duty=-1, dutyLt=28
    { 527, 512,1626},  //duty=0, dutyLt=28
    { 540, 512,1544},  //duty=1, dutyLt=28
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
    { 511, 512,1729},  //duty=-1, dutyLt=29
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
