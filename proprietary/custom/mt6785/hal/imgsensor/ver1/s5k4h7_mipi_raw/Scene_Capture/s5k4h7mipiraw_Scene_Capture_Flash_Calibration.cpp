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
#include "s5k4h7mipiraw_Scene_Capture.h"

const FLASH_CALIBRATION_NVRAM_T s5k4h7mipiraw_Flash_Calibration_0000 = {
    .engTab = {
        .exp = 10000,
        .afe_gain = 1024,
        .isp_gain = 1024,
        .distance = 0,
        .yTab = {
           0, 319, 658, 981,1294,1602,1900,2191,2475,2752,3029,3293,3555,3808,4055,4296,4528,5006,5440,5859,6258,6621,6968,7302,7593,7879,8143,8392,8606,8821,8996,
         234, 577, 912,1235,1546,1853,2151,2444,2728,3003,3279,3544,3805,4059,4304,4544,4776,5254,5688,6101,6503,6862,7210,7535,7820,8110,8373,8608,8854,9032,  -1,
         481, 827,1163,1485,1798,2104,2401,2692,2975,3252,3526,3790,4050,4301,4547,4785,5017,5493,5927,6338,6731,7095,7433,7763,8046,8339,8584,8835,9046,9248,  -1,
         724,1067,1403,1727,2038,2343,2641,2931,3214,3488,3761,4026,4284,4535,4780,5016,5247,5721,6152,6567,6953,7318,7659,7979,8270,8544,8800,9037,9261,  -1,  -1,
         956,1302,1638,1961,2273,2579,2875,3164,3447,3721,3993,4255,4512,4762,5006,5241,5473,5946,6375,6781,7170,7531,7873,8191,8479,8756,9007,9239,9470,  -1,  -1,
        1177,1525,1861,2185,2496,2802,3097,3387,3669,3941,4212,4475,4732,4980,5223,5457,5688,6160,6587,6992,7385,7745,8082,8402,8680,8960,9216,9445,  -1,  -1,  -1,
        1397,1745,2081,2405,2716,3021,3316,3604,3885,4156,4428,4688,4943,5192,5434,5668,5900,6370,6797,7201,7587,7943,8279,8597,8880,9158,9409,9649,  -1,  -1,  -1,
        1608,1955,2291,2616,2927,3231,3526,3814,4095,4366,4636,4896,5151,5399,5640,5874,6101,6573,6997,7403,7783,8139,8476,8792,9083,9347,9599,  -1,  -1,  -1,  -1,
        1814,2163,2500,2823,3133,3439,3732,4019,4299,4569,4841,5098,5352,5598,5841,6072,6299,6771,7193,7595,7987,8341,8668,8980,9268,9543,9794,  -1,  -1,  -1,  -1,
        2013,2361,2698,3021,3332,3636,3931,4217,4496,4766,5033,5292,5545,5792,6032,6264,6492,6962,7383,7784,8163,8519,8848,9165,9441,9708,  -1,  -1,  -1,  -1,  -1,
        2208,2558,2895,3217,3528,3831,4124,4410,4689,4959,5226,5484,5736,5983,6221,6451,6678,7149,7570,7976,8348,8699,9043,9340,9621,9897,  -1,  -1,  -1,  -1,  -1,
        2394,2744,3083,3404,3714,4018,4311,4595,4872,5141,5409,5666,5918,6161,6402,6635,6856,7326,7751,8146,8523,8877,9208,9511,9792,  -1,  -1,  -1,  -1,  -1,  -1,
        2582,2932,3268,3590,3901,4201,4493,4780,5056,5323,5591,5847,6098,6340,6581,6808,7031,7501,7920,8318,8694,9042,9369,9677,9957,  -1,  -1,  -1,  -1,  -1,  -1,
        2759,3109,3446,3767,4076,4379,4670,4955,5231,5498,5766,6021,6272,6513,6750,6981,7202,7680,8091,8486,8858,9214,9533,9841,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        2936,3286,3622,3943,4252,4553,4844,5127,5403,5668,5937,6190,6439,6684,6920,7148,7370,7840,8254,8654,9023,9367,9692,9996,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3103,3454,3790,4111,4420,4720,5011,5293,5568,5835,6102,6356,6603,6843,7080,7307,7531,8005,8416,8807,9180,9530,9851,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3273,3621,3958,4277,4585,4885,5177,5458,5731,6000,6260,6514,6765,7003,7239,7467,7692,8164,8575,8962,9335,9674,9999,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3598,3952,4290,4612,4919,5221,5511,5794,6069,6333,6600,6855,7109,7347,7581,7809,8034,8471,8875,9260,9637,9974,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3905,4256,4593,4917,5221,5522,5811,6093,6367,6631,6893,7147,7393,7634,7867,8097,8316,8748,9154,9540,9904,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4188,4541,4873,5195,5500,5797,6097,6369,6641,6905,7173,7422,7669,7907,8139,8367,8582,9008,9410,9790,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4457,4807,5140,5459,5763,6060,6348,6626,6899,7161,7422,7673,7918,8158,8386,8613,8829,9265,9659,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4710,5056,5388,5710,6011,6307,6594,6874,7139,7402,7664,7910,8153,8399,8630,8845,9067,9485,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4939,5287,5619,5935,6240,6531,6824,7095,7361,7623,7876,8133,8375,8605,8834,9057,9267,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5153,5498,5831,6144,6444,6739,7024,7308,7567,7837,8083,8337,8589,8815,9044,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5347,5686,6027,6333,6636,6936,7204,7480,7756,8001,8254,8502,8750,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5532,5871,6198,6516,6809,7100,7378,7659,7924,8175,8429,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5706,6041,6383,6688,6985,7265,7548,7818,8083,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5875,6205,6544,6844,7131,7424,7697,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6021,6344,6670,6975,7264,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6129,6459,6788,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6257,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        },
    },
    .flashWBGain = {
    { 503, 512, 512},  //duty=-1, dutyLt=-1
    {1009, 512, 852},  //duty=0, dutyLt=-1
    {1012, 512, 848},  //duty=1, dutyLt=-1
    {1008, 512, 842},  //duty=2, dutyLt=-1
    {1006, 512, 837},  //duty=3, dutyLt=-1
    {1005, 512, 833},  //duty=4, dutyLt=-1
    {1004, 512, 828},  //duty=5, dutyLt=-1
    {1003, 512, 822},  //duty=6, dutyLt=-1
    {1010, 512, 824},  //duty=7, dutyLt=-1
    {1009, 512, 821},  //duty=8, dutyLt=-1
    {1007, 512, 817},  //duty=9, dutyLt=-1
    {1006, 512, 814},  //duty=10, dutyLt=-1
    {1005, 512, 811},  //duty=11, dutyLt=-1
    {1004, 512, 809},  //duty=12, dutyLt=-1
    {1003, 512, 805},  //duty=13, dutyLt=-1
    {1002, 512, 803},  //duty=14, dutyLt=-1
    {1002, 512, 801},  //duty=15, dutyLt=-1
    {1000, 512, 796},  //duty=16, dutyLt=-1
    { 999, 512, 791},  //duty=17, dutyLt=-1
    { 999, 512, 788},  //duty=18, dutyLt=-1
    { 998, 512, 783},  //duty=19, dutyLt=-1
    { 998, 512, 780},  //duty=20, dutyLt=-1
    { 997, 512, 777},  //duty=21, dutyLt=-1
    { 997, 512, 774},  //duty=22, dutyLt=-1
    { 998, 512, 770},  //duty=23, dutyLt=-1
    { 998, 512, 768},  //duty=24, dutyLt=-1
    { 998, 512, 765},  //duty=25, dutyLt=-1
    { 998, 512, 762},  //duty=26, dutyLt=-1
    { 999, 512, 759},  //duty=27, dutyLt=-1
    {1000, 512, 757},  //duty=28, dutyLt=-1
    {1001, 512, 754},  //duty=29, dutyLt=-1
    { 503, 512,1868},  //duty=-1, dutyLt=0
    { 722, 512,1082},  //duty=0, dutyLt=0
    { 807, 512, 982},  //duty=1, dutyLt=0
    { 850, 512, 936},  //duty=2, dutyLt=0
    { 876, 512, 911},  //duty=3, dutyLt=0
    { 894, 512, 893},  //duty=4, dutyLt=0
    { 907, 512, 878},  //duty=5, dutyLt=0
    { 922, 512, 872},  //duty=6, dutyLt=0
    { 930, 512, 864},  //duty=7, dutyLt=0
    { 935, 512, 857},  //duty=8, dutyLt=0
    { 940, 512, 851},  //duty=9, dutyLt=0
    { 944, 512, 845},  //duty=10, dutyLt=0
    { 946, 512, 840},  //duty=11, dutyLt=0
    { 949, 512, 836},  //duty=12, dutyLt=0
    { 950, 512, 831},  //duty=13, dutyLt=0
    { 953, 512, 827},  //duty=14, dutyLt=0
    { 955, 512, 824},  //duty=15, dutyLt=0
    { 957, 512, 817},  //duty=16, dutyLt=0
    { 959, 512, 811},  //duty=17, dutyLt=0
    { 961, 512, 806},  //duty=18, dutyLt=0
    { 963, 512, 801},  //duty=19, dutyLt=0
    { 965, 512, 796},  //duty=20, dutyLt=0
    { 966, 512, 792},  //duty=21, dutyLt=0
    { 967, 512, 789},  //duty=22, dutyLt=0
    { 969, 512, 785},  //duty=23, dutyLt=0
    { 970, 512, 781},  //duty=24, dutyLt=0
    { 971, 512, 778},  //duty=25, dutyLt=0
    { 973, 512, 775},  //duty=26, dutyLt=0
    { 974, 512, 772},  //duty=27, dutyLt=0
    { 975, 512, 769},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 503, 512,1844},  //duty=-1, dutyLt=1
    { 645, 512,1229},  //duty=0, dutyLt=1
    { 721, 512,1079},  //duty=1, dutyLt=1
    { 768, 512,1012},  //duty=2, dutyLt=1
    { 801, 512, 972},  //duty=3, dutyLt=1
    { 825, 512, 944},  //duty=4, dutyLt=1
    { 849, 512, 929},  //duty=5, dutyLt=1
    { 862, 512, 912},  //duty=6, dutyLt=1
    { 873, 512, 900},  //duty=7, dutyLt=1
    { 881, 512, 890},  //duty=8, dutyLt=1
    { 890, 512, 881},  //duty=9, dutyLt=1
    { 896, 512, 873},  //duty=10, dutyLt=1
    { 901, 512, 866},  //duty=11, dutyLt=1
    { 906, 512, 860},  //duty=12, dutyLt=1
    { 910, 512, 854},  //duty=13, dutyLt=1
    { 914, 512, 849},  //duty=14, dutyLt=1
    { 917, 512, 845},  //duty=15, dutyLt=1
    { 923, 512, 836},  //duty=16, dutyLt=1
    { 927, 512, 828},  //duty=17, dutyLt=1
    { 931, 512, 822},  //duty=18, dutyLt=1
    { 934, 512, 816},  //duty=19, dutyLt=1
    { 937, 512, 811},  //duty=20, dutyLt=1
    { 940, 512, 806},  //duty=21, dutyLt=1
    { 942, 512, 802},  //duty=22, dutyLt=1
    { 945, 512, 797},  //duty=23, dutyLt=1
    { 947, 512, 794},  //duty=24, dutyLt=1
    { 948, 512, 790},  //duty=25, dutyLt=1
    { 950, 512, 786},  //duty=26, dutyLt=1
    { 952, 512, 783},  //duty=27, dutyLt=1
    { 954, 512, 780},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 503, 512,1867},  //duty=-1, dutyLt=2
    { 608, 512,1320},  //duty=0, dutyLt=2
    { 674, 512,1154},  //duty=1, dutyLt=2
    { 720, 512,1073},  //duty=2, dutyLt=2
    { 753, 512,1022},  //duty=3, dutyLt=2
    { 779, 512, 984},  //duty=4, dutyLt=2
    { 802, 512, 968},  //duty=5, dutyLt=2
    { 818, 512, 948},  //duty=6, dutyLt=2
    { 831, 512, 933},  //duty=7, dutyLt=2
    { 842, 512, 919},  //duty=8, dutyLt=2
    { 852, 512, 908},  //duty=9, dutyLt=2
    { 859, 512, 898},  //duty=10, dutyLt=2
    { 866, 512, 890},  //duty=11, dutyLt=2
    { 872, 512, 882},  //duty=12, dutyLt=2
    { 877, 512, 875},  //duty=13, dutyLt=2
    { 882, 512, 869},  //duty=14, dutyLt=2
    { 887, 512, 864},  //duty=15, dutyLt=2
    { 894, 512, 853},  //duty=16, dutyLt=2
    { 900, 512, 844},  //duty=17, dutyLt=2
    { 906, 512, 837},  //duty=18, dutyLt=2
    { 910, 512, 831},  //duty=19, dutyLt=2
    { 914, 512, 825},  //duty=20, dutyLt=2
    { 917, 512, 819},  //duty=21, dutyLt=2
    { 920, 512, 815},  //duty=22, dutyLt=2
    { 924, 512, 809},  //duty=23, dutyLt=2
    { 926, 512, 805},  //duty=24, dutyLt=2
    { 929, 512, 801},  //duty=25, dutyLt=2
    { 931, 512, 797},  //duty=26, dutyLt=2
    { 933, 512, 793},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 502, 512,1850},  //duty=-1, dutyLt=3
    { 587, 512,1383},  //duty=0, dutyLt=3
    { 644, 512,1212},  //duty=1, dutyLt=3
    { 687, 512,1122},  //duty=2, dutyLt=3
    { 719, 512,1063},  //duty=3, dutyLt=3
    { 747, 512,1032},  //duty=4, dutyLt=3
    { 767, 512,1003},  //duty=5, dutyLt=3
    { 784, 512, 980},  //duty=6, dutyLt=3
    { 798, 512, 961},  //duty=7, dutyLt=3
    { 810, 512, 946},  //duty=8, dutyLt=3
    { 821, 512, 933},  //duty=9, dutyLt=3
    { 829, 512, 922},  //duty=10, dutyLt=3
    { 837, 512, 912},  //duty=11, dutyLt=3
    { 844, 512, 903},  //duty=12, dutyLt=3
    { 850, 512, 895},  //duty=13, dutyLt=3
    { 856, 512, 888},  //duty=14, dutyLt=3
    { 861, 512, 882},  //duty=15, dutyLt=3
    { 870, 512, 870},  //duty=16, dutyLt=3
    { 877, 512, 860},  //duty=17, dutyLt=3
    { 884, 512, 852},  //duty=18, dutyLt=3
    { 889, 512, 844},  //duty=19, dutyLt=3
    { 893, 512, 838},  //duty=20, dutyLt=3
    { 897, 512, 831},  //duty=21, dutyLt=3
    { 901, 512, 826},  //duty=22, dutyLt=3
    { 905, 512, 821},  //duty=23, dutyLt=3
    { 908, 512, 816},  //duty=24, dutyLt=3
    { 911, 512, 811},  //duty=25, dutyLt=3
    { 913, 512, 807},  //duty=26, dutyLt=3
    { 916, 512, 803},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 502, 512,1839},  //duty=-1, dutyLt=4
    { 573, 512,1429},  //duty=0, dutyLt=4
    { 624, 512,1258},  //duty=1, dutyLt=4
    { 663, 512,1163},  //duty=2, dutyLt=4
    { 696, 512,1109},  //duty=3, dutyLt=4
    { 721, 512,1066},  //duty=4, dutyLt=4
    { 741, 512,1033},  //duty=5, dutyLt=4
    { 758, 512,1009},  //duty=6, dutyLt=4
    { 773, 512, 988},  //duty=7, dutyLt=4
    { 785, 512, 971},  //duty=8, dutyLt=4
    { 796, 512, 956},  //duty=9, dutyLt=4
    { 805, 512, 943},  //duty=10, dutyLt=4
    { 814, 512, 932},  //duty=11, dutyLt=4
    { 821, 512, 922},  //duty=12, dutyLt=4
    { 828, 512, 913},  //duty=13, dutyLt=4
    { 834, 512, 905},  //duty=14, dutyLt=4
    { 839, 512, 898},  //duty=15, dutyLt=4
    { 849, 512, 885},  //duty=16, dutyLt=4
    { 857, 512, 874},  //duty=17, dutyLt=4
    { 864, 512, 864},  //duty=18, dutyLt=4
    { 870, 512, 857},  //duty=19, dutyLt=4
    { 876, 512, 849},  //duty=20, dutyLt=4
    { 880, 512, 843},  //duty=21, dutyLt=4
    { 884, 512, 837},  //duty=22, dutyLt=4
    { 888, 512, 831},  //duty=23, dutyLt=4
    { 892, 512, 826},  //duty=24, dutyLt=4
    { 895, 512, 821},  //duty=25, dutyLt=4
    { 898, 512, 817},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 502, 512,1832},  //duty=-1, dutyLt=5
    { 563, 512,1464},  //duty=0, dutyLt=5
    { 610, 512,1295},  //duty=1, dutyLt=5
    { 647, 512,1194},  //duty=2, dutyLt=5
    { 676, 512,1142},  //duty=3, dutyLt=5
    { 701, 512,1097},  //duty=4, dutyLt=5
    { 720, 512,1061},  //duty=5, dutyLt=5
    { 737, 512,1033},  //duty=6, dutyLt=5
    { 751, 512,1011},  //duty=7, dutyLt=5
    { 764, 512, 992},  //duty=8, dutyLt=5
    { 775, 512, 976},  //duty=9, dutyLt=5
    { 785, 512, 962},  //duty=10, dutyLt=5
    { 794, 512, 950},  //duty=11, dutyLt=5
    { 801, 512, 939},  //duty=12, dutyLt=5
    { 808, 512, 930},  //duty=13, dutyLt=5
    { 815, 512, 921},  //duty=14, dutyLt=5
    { 820, 512, 913},  //duty=15, dutyLt=5
    { 831, 512, 899},  //duty=16, dutyLt=5
    { 840, 512, 887},  //duty=17, dutyLt=5
    { 848, 512, 877},  //duty=18, dutyLt=5
    { 854, 512, 868},  //duty=19, dutyLt=5
    { 860, 512, 861},  //duty=20, dutyLt=5
    { 865, 512, 854},  //duty=21, dutyLt=5
    { 869, 512, 847},  //duty=22, dutyLt=5
    { 874, 512, 841},  //duty=23, dutyLt=5
    { 878, 512, 836},  //duty=24, dutyLt=5
    { 881, 512, 830},  //duty=25, dutyLt=5
    { 884, 512, 826},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 502, 512,1824},  //duty=-1, dutyLt=6
    { 556, 512,1492},  //duty=0, dutyLt=6
    { 599, 512,1326},  //duty=1, dutyLt=6
    { 633, 512,1237},  //duty=2, dutyLt=6
    { 661, 512,1172},  //duty=3, dutyLt=6
    { 683, 512,1122},  //duty=4, dutyLt=6
    { 703, 512,1086},  //duty=5, dutyLt=6
    { 719, 512,1056},  //duty=6, dutyLt=6
    { 734, 512,1033},  //duty=7, dutyLt=6
    { 746, 512,1012},  //duty=8, dutyLt=6
    { 757, 512, 996},  //duty=9, dutyLt=6
    { 768, 512, 980},  //duty=10, dutyLt=6
    { 776, 512, 967},  //duty=11, dutyLt=6
    { 784, 512, 955},  //duty=12, dutyLt=6
    { 791, 512, 945},  //duty=13, dutyLt=6
    { 798, 512, 936},  //duty=14, dutyLt=6
    { 804, 512, 927},  //duty=15, dutyLt=6
    { 815, 512, 912},  //duty=16, dutyLt=6
    { 825, 512, 900},  //duty=17, dutyLt=6
    { 833, 512, 889},  //duty=18, dutyLt=6
    { 840, 512, 879},  //duty=19, dutyLt=6
    { 846, 512, 871},  //duty=20, dutyLt=6
    { 851, 512, 864},  //duty=21, dutyLt=6
    { 856, 512, 857},  //duty=22, dutyLt=6
    { 860, 512, 850},  //duty=23, dutyLt=6
    { 864, 512, 845},  //duty=24, dutyLt=6
    { 868, 512, 839},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 502, 512,1820},  //duty=-1, dutyLt=7
    { 551, 512,1513},  //duty=0, dutyLt=7
    { 591, 512,1344},  //duty=1, dutyLt=7
    { 622, 512,1263},  //duty=2, dutyLt=7
    { 648, 512,1196},  //duty=3, dutyLt=7
    { 670, 512,1146},  //duty=4, dutyLt=7
    { 688, 512,1108},  //duty=5, dutyLt=7
    { 705, 512,1078},  //duty=6, dutyLt=7
    { 718, 512,1052},  //duty=7, dutyLt=7
    { 731, 512,1031},  //duty=8, dutyLt=7
    { 742, 512,1013},  //duty=9, dutyLt=7
    { 752, 512, 997},  //duty=10, dutyLt=7
    { 761, 512, 983},  //duty=11, dutyLt=7
    { 769, 512, 971},  //duty=12, dutyLt=7
    { 777, 512, 959},  //duty=13, dutyLt=7
    { 783, 512, 950},  //duty=14, dutyLt=7
    { 790, 512, 940},  //duty=15, dutyLt=7
    { 802, 512, 924},  //duty=16, dutyLt=7
    { 811, 512, 912},  //duty=17, dutyLt=7
    { 819, 512, 900},  //duty=18, dutyLt=7
    { 827, 512, 890},  //duty=19, dutyLt=7
    { 833, 512, 881},  //duty=20, dutyLt=7
    { 839, 512, 873},  //duty=21, dutyLt=7
    { 843, 512, 866},  //duty=22, dutyLt=7
    { 849, 512, 860},  //duty=23, dutyLt=7
    { 853, 512, 854},  //duty=24, dutyLt=7
    { 856, 512, 848},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 502, 512,1813},  //duty=-1, dutyLt=8
    { 547, 512,1529},  //duty=0, dutyLt=8
    { 584, 512,1385},  //duty=1, dutyLt=8
    { 613, 512,1287},  //duty=2, dutyLt=8
    { 637, 512,1220},  //duty=3, dutyLt=8
    { 658, 512,1168},  //duty=4, dutyLt=8
    { 676, 512,1129},  //duty=5, dutyLt=8
    { 692, 512,1097},  //duty=6, dutyLt=8
    { 706, 512,1070},  //duty=7, dutyLt=8
    { 718, 512,1048},  //duty=8, dutyLt=8
    { 729, 512,1028},  //duty=9, dutyLt=8
    { 739, 512,1012},  //duty=10, dutyLt=8
    { 748, 512, 997},  //duty=11, dutyLt=8
    { 756, 512, 984},  //duty=12, dutyLt=8
    { 764, 512, 973},  //duty=13, dutyLt=8
    { 771, 512, 963},  //duty=14, dutyLt=8
    { 778, 512, 953},  //duty=15, dutyLt=8
    { 789, 512, 936},  //duty=16, dutyLt=8
    { 799, 512, 923},  //duty=17, dutyLt=8
    { 807, 512, 910},  //duty=18, dutyLt=8
    { 815, 512, 900},  //duty=19, dutyLt=8
    { 822, 512, 891},  //duty=20, dutyLt=8
    { 827, 512, 882},  //duty=21, dutyLt=8
    { 833, 512, 875},  //duty=22, dutyLt=8
    { 837, 512, 868},  //duty=23, dutyLt=8
    { 842, 512, 862},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 502, 512,1807},  //duty=-1, dutyLt=9
    { 545, 512,1532},  //duty=0, dutyLt=9
    { 578, 512,1407},  //duty=1, dutyLt=9
    { 605, 512,1309},  //duty=2, dutyLt=9
    { 628, 512,1240},  //duty=3, dutyLt=9
    { 648, 512,1187},  //duty=4, dutyLt=9
    { 666, 512,1146},  //duty=5, dutyLt=9
    { 681, 512,1114},  //duty=6, dutyLt=9
    { 695, 512,1087},  //duty=7, dutyLt=9
    { 707, 512,1063},  //duty=8, dutyLt=9
    { 718, 512,1043},  //duty=9, dutyLt=9
    { 728, 512,1026},  //duty=10, dutyLt=9
    { 737, 512,1011},  //duty=11, dutyLt=9
    { 745, 512, 998},  //duty=12, dutyLt=9
    { 752, 512, 985},  //duty=13, dutyLt=9
    { 759, 512, 975},  //duty=14, dutyLt=9
    { 766, 512, 965},  //duty=15, dutyLt=9
    { 778, 512, 947},  //duty=16, dutyLt=9
    { 788, 512, 933},  //duty=17, dutyLt=9
    { 797, 512, 921},  //duty=18, dutyLt=9
    { 804, 512, 910},  //duty=19, dutyLt=9
    { 811, 512, 900},  //duty=20, dutyLt=9
    { 817, 512, 892},  //duty=21, dutyLt=9
    { 822, 512, 883},  //duty=22, dutyLt=9
    { 827, 512, 876},  //duty=23, dutyLt=9
    { 832, 512, 870},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 503, 512,1802},  //duty=-1, dutyLt=10
    { 541, 512,1571},  //duty=0, dutyLt=10
    { 573, 512,1423},  //duty=1, dutyLt=10
    { 599, 512,1327},  //duty=2, dutyLt=10
    { 621, 512,1257},  //duty=3, dutyLt=10
    { 640, 512,1205},  //duty=4, dutyLt=10
    { 657, 512,1164},  //duty=5, dutyLt=10
    { 672, 512,1129},  //duty=6, dutyLt=10
    { 685, 512,1102},  //duty=7, dutyLt=10
    { 697, 512,1077},  //duty=8, dutyLt=10
    { 707, 512,1057},  //duty=9, dutyLt=10
    { 718, 512,1039},  //duty=10, dutyLt=10
    { 727, 512,1024},  //duty=11, dutyLt=10
    { 735, 512,1010},  //duty=12, dutyLt=10
    { 743, 512, 998},  //duty=13, dutyLt=10
    { 749, 512, 987},  //duty=14, dutyLt=10
    { 756, 512, 976},  //duty=15, dutyLt=10
    { 768, 512, 958},  //duty=16, dutyLt=10
    { 778, 512, 943},  //duty=17, dutyLt=10
    { 787, 512, 930},  //duty=18, dutyLt=10
    { 795, 512, 918},  //duty=19, dutyLt=10
    { 801, 512, 908},  //duty=20, dutyLt=10
    { 807, 512, 899},  //duty=21, dutyLt=10
    { 813, 512, 891},  //duty=22, dutyLt=10
    { 818, 512, 884},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 504, 512,1792},  //duty=-1, dutyLt=11
    { 539, 512,1580},  //duty=0, dutyLt=11
    { 568, 512,1439},  //duty=1, dutyLt=11
    { 593, 512,1344},  //duty=2, dutyLt=11
    { 614, 512,1274},  //duty=3, dutyLt=11
    { 633, 512,1220},  //duty=4, dutyLt=11
    { 649, 512,1179},  //duty=5, dutyLt=11
    { 664, 512,1145},  //duty=6, dutyLt=11
    { 676, 512,1116},  //duty=7, dutyLt=11
    { 688, 512,1092},  //duty=8, dutyLt=11
    { 699, 512,1070},  //duty=9, dutyLt=11
    { 709, 512,1052},  //duty=10, dutyLt=11
    { 718, 512,1036},  //duty=11, dutyLt=11
    { 726, 512,1022},  //duty=12, dutyLt=11
    { 733, 512,1009},  //duty=13, dutyLt=11
    { 740, 512, 998},  //duty=14, dutyLt=11
    { 747, 512, 987},  //duty=15, dutyLt=11
    { 759, 512, 968},  //duty=16, dutyLt=11
    { 769, 512, 952},  //duty=17, dutyLt=11
    { 778, 512, 939},  //duty=18, dutyLt=11
    { 786, 512, 927},  //duty=19, dutyLt=11
    { 793, 512, 917},  //duty=20, dutyLt=11
    { 799, 512, 907},  //duty=21, dutyLt=11
    { 804, 512, 899},  //duty=22, dutyLt=11
    { 809, 512, 891},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 504, 512,1819},  //duty=-1, dutyLt=12
    { 536, 512,1589},  //duty=0, dutyLt=12
    { 565, 512,1452},  //duty=1, dutyLt=12
    { 588, 512,1358},  //duty=2, dutyLt=12
    { 608, 512,1287},  //duty=3, dutyLt=12
    { 626, 512,1235},  //duty=4, dutyLt=12
    { 642, 512,1192},  //duty=5, dutyLt=12
    { 656, 512,1158},  //duty=6, dutyLt=12
    { 669, 512,1129},  //duty=7, dutyLt=12
    { 680, 512,1104},  //duty=8, dutyLt=12
    { 691, 512,1082},  //duty=9, dutyLt=12
    { 700, 512,1064},  //duty=10, dutyLt=12
    { 709, 512,1048},  //duty=11, dutyLt=12
    { 718, 512,1033},  //duty=12, dutyLt=12
    { 725, 512,1019},  //duty=13, dutyLt=12
    { 732, 512,1008},  //duty=14, dutyLt=12
    { 738, 512, 997},  //duty=15, dutyLt=12
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
    { 504, 512,1816},  //duty=-1, dutyLt=13
    { 535, 512,1599},  //duty=0, dutyLt=13
    { 561, 512,1463},  //duty=1, dutyLt=13
    { 584, 512,1370},  //duty=2, dutyLt=13
    { 604, 512,1302},  //duty=3, dutyLt=13
    { 621, 512,1248},  //duty=4, dutyLt=13
    { 636, 512,1206},  //duty=5, dutyLt=13
    { 650, 512,1170},  //duty=6, dutyLt=13
    { 662, 512,1141},  //duty=7, dutyLt=13
    { 673, 512,1116},  //duty=8, dutyLt=13
    { 684, 512,1094},  //duty=9, dutyLt=13
    { 693, 512,1075},  //duty=10, dutyLt=13
    { 702, 512,1058},  //duty=11, dutyLt=13
    { 710, 512,1043},  //duty=12, dutyLt=13
    { 718, 512,1029},  //duty=13, dutyLt=13
    { 725, 512,1017},  //duty=14, dutyLt=13
    { 731, 512,1006},  //duty=15, dutyLt=13
    { 743, 512, 986},  //duty=16, dutyLt=13
    { 753, 512, 969},  //duty=17, dutyLt=13
    { 762, 512, 956},  //duty=18, dutyLt=13
    { 770, 512, 943},  //duty=19, dutyLt=13
    { 777, 512, 932},  //duty=20, dutyLt=13
    { 784, 512, 922},  //duty=21, dutyLt=13
    { 789, 512, 913},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 504, 512,1808},  //duty=-1, dutyLt=14
    { 534, 512,1603},  //duty=0, dutyLt=14
    { 559, 512,1472},  //duty=1, dutyLt=14
    { 580, 512,1381},  //duty=2, dutyLt=14
    { 599, 512,1313},  //duty=3, dutyLt=14
    { 616, 512,1260},  //duty=4, dutyLt=14
    { 631, 512,1217},  //duty=5, dutyLt=14
    { 644, 512,1181},  //duty=6, dutyLt=14
    { 656, 512,1152},  //duty=7, dutyLt=14
    { 667, 512,1127},  //duty=8, dutyLt=14
    { 678, 512,1105},  //duty=9, dutyLt=14
    { 687, 512,1085},  //duty=10, dutyLt=14
    { 696, 512,1067},  //duty=11, dutyLt=14
    { 704, 512,1052},  //duty=12, dutyLt=14
    { 711, 512,1038},  //duty=13, dutyLt=14
    { 718, 512,1026},  //duty=14, dutyLt=14
    { 724, 512,1015},  //duty=15, dutyLt=14
    { 736, 512, 994},  //duty=16, dutyLt=14
    { 746, 512, 978},  //duty=17, dutyLt=14
    { 755, 512, 962},  //duty=18, dutyLt=14
    { 763, 512, 950},  //duty=19, dutyLt=14
    { 770, 512, 939},  //duty=20, dutyLt=14
    { 777, 512, 929},  //duty=21, dutyLt=14
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
    { 556, 512,1480},  //duty=1, dutyLt=15
    { 577, 512,1393},  //duty=2, dutyLt=15
    { 595, 512,1324},  //duty=3, dutyLt=15
    { 612, 512,1272},  //duty=4, dutyLt=15
    { 626, 512,1227},  //duty=5, dutyLt=15
    { 639, 512,1193},  //duty=6, dutyLt=15
    { 651, 512,1162},  //duty=7, dutyLt=15
    { 662, 512,1137},  //duty=8, dutyLt=15
    { 672, 512,1114},  //duty=9, dutyLt=15
    { 681, 512,1094},  //duty=10, dutyLt=15
    { 690, 512,1077},  //duty=11, dutyLt=15
    { 698, 512,1061},  //duty=12, dutyLt=15
    { 705, 512,1047},  //duty=13, dutyLt=15
    { 712, 512,1035},  //duty=14, dutyLt=15
    { 718, 512,1023},  //duty=15, dutyLt=15
    { 730, 512,1002},  //duty=16, dutyLt=15
    { 740, 512, 985},  //duty=17, dutyLt=15
    { 749, 512, 970},  //duty=18, dutyLt=15
    { 757, 512, 957},  //duty=19, dutyLt=15
    { 764, 512, 945},  //duty=20, dutyLt=15
    { 770, 512, 935},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 504, 512,1796},  //duty=-1, dutyLt=16
    { 530, 512,1616},  //duty=0, dutyLt=16
    { 552, 512,1495},  //duty=1, dutyLt=16
    { 572, 512,1409},  //duty=2, dutyLt=16
    { 588, 512,1343},  //duty=3, dutyLt=16
    { 604, 512,1290},  //duty=4, dutyLt=16
    { 618, 512,1247},  //duty=5, dutyLt=16
    { 630, 512,1212},  //duty=6, dutyLt=16
    { 642, 512,1180},  //duty=7, dutyLt=16
    { 652, 512,1154},  //duty=8, dutyLt=16
    { 662, 512,1131},  //duty=9, dutyLt=16
    { 671, 512,1112},  //duty=10, dutyLt=16
    { 679, 512,1094},  //duty=11, dutyLt=16
    { 687, 512,1077},  //duty=12, dutyLt=16
    { 694, 512,1063},  //duty=13, dutyLt=16
    { 701, 512,1050},  //duty=14, dutyLt=16
    { 707, 512,1038},  //duty=15, dutyLt=16
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
    { 504, 512,1790},  //duty=-1, dutyLt=17
    { 528, 512,1621},  //duty=0, dutyLt=17
    { 549, 512,1507},  //duty=1, dutyLt=17
    { 567, 512,1423},  //duty=2, dutyLt=17
    { 584, 512,1358},  //duty=3, dutyLt=17
    { 598, 512,1306},  //duty=4, dutyLt=17
    { 611, 512,1263},  //duty=5, dutyLt=17
    { 623, 512,1227},  //duty=6, dutyLt=17
    { 634, 512,1197},  //duty=7, dutyLt=17
    { 644, 512,1170},  //duty=8, dutyLt=17
    { 654, 512,1147},  //duty=9, dutyLt=17
    { 662, 512,1127},  //duty=10, dutyLt=17
    { 671, 512,1108},  //duty=11, dutyLt=17
    { 678, 512,1092},  //duty=12, dutyLt=17
    { 685, 512,1077},  //duty=13, dutyLt=17
    { 692, 512,1064},  //duty=14, dutyLt=17
    { 698, 512,1052},  //duty=15, dutyLt=17
    { 709, 512,1030},  //duty=16, dutyLt=17
    { 719, 512,1011},  //duty=17, dutyLt=17
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
    { 564, 512,1434},  //duty=2, dutyLt=18
    { 579, 512,1371},  //duty=3, dutyLt=18
    { 593, 512,1319},  //duty=4, dutyLt=18
    { 605, 512,1278},  //duty=5, dutyLt=18
    { 617, 512,1242},  //duty=6, dutyLt=18
    { 628, 512,1211},  //duty=7, dutyLt=18
    { 637, 512,1184},  //duty=8, dutyLt=18
    { 647, 512,1161},  //duty=9, dutyLt=18
    { 655, 512,1140},  //duty=10, dutyLt=18
    { 663, 512,1121},  //duty=11, dutyLt=18
    { 671, 512,1105},  //duty=12, dutyLt=18
    { 678, 512,1090},  //duty=13, dutyLt=18
    { 684, 512,1076},  //duty=14, dutyLt=18
    { 690, 512,1064},  //duty=15, dutyLt=18
    { 701, 512,1041},  //duty=16, dutyLt=18
    { 711, 512,1022},  //duty=17, dutyLt=18
    { 719, 512,1006},  //duty=18, dutyLt=18
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
    { 526, 512,1628},  //duty=0, dutyLt=19
    { 545, 512,1523},  //duty=1, dutyLt=19
    { 561, 512,1444},  //duty=2, dutyLt=19
    { 576, 512,1382},  //duty=3, dutyLt=19
    { 589, 512,1331},  //duty=4, dutyLt=19
    { 601, 512,1289},  //duty=5, dutyLt=19
    { 612, 512,1254},  //duty=6, dutyLt=19
    { 623, 512,1223},  //duty=7, dutyLt=19
    { 632, 512,1196},  //duty=8, dutyLt=19
    { 641, 512,1173},  //duty=9, dutyLt=19
    { 649, 512,1152},  //duty=10, dutyLt=19
    { 657, 512,1133},  //duty=11, dutyLt=19
    { 664, 512,1117},  //duty=12, dutyLt=19
    { 671, 512,1101},  //duty=13, dutyLt=19
    { 677, 512,1087},  //duty=14, dutyLt=19
    { 683, 512,1074},  //duty=15, dutyLt=19
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
    { 526, 512,1631},  //duty=0, dutyLt=20
    { 543, 512,1530},  //duty=1, dutyLt=20
    { 559, 512,1453},  //duty=2, dutyLt=20
    { 573, 512,1392},  //duty=3, dutyLt=20
    { 586, 512,1341},  //duty=4, dutyLt=20
    { 598, 512,1300},  //duty=5, dutyLt=20
    { 608, 512,1265},  //duty=6, dutyLt=20
    { 618, 512,1234},  //duty=7, dutyLt=20
    { 627, 512,1207},  //duty=8, dutyLt=20
    { 636, 512,1184},  //duty=9, dutyLt=20
    { 644, 512,1162},  //duty=10, dutyLt=20
    { 652, 512,1143},  //duty=11, dutyLt=20
    { 659, 512,1127},  //duty=12, dutyLt=20
    { 665, 512,1112},  //duty=13, dutyLt=20
    { 671, 512,1097},  //duty=14, dutyLt=20
    { 677, 512,1085},  //duty=15, dutyLt=20
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
    { 506, 512,1764},  //duty=-1, dutyLt=21
    { 525, 512,1631},  //duty=0, dutyLt=21
    { 542, 512,1534},  //duty=1, dutyLt=21
    { 557, 512,1459},  //duty=2, dutyLt=21
    { 571, 512,1400},  //duty=3, dutyLt=21
    { 583, 512,1350},  //duty=4, dutyLt=21
    { 594, 512,1310},  //duty=5, dutyLt=21
    { 605, 512,1274},  //duty=6, dutyLt=21
    { 614, 512,1243},  //duty=7, dutyLt=21
    { 623, 512,1217},  //duty=8, dutyLt=21
    { 632, 512,1193},  //duty=9, dutyLt=21
    { 639, 512,1172},  //duty=10, dutyLt=21
    { 647, 512,1153},  //duty=11, dutyLt=21
    { 654, 512,1136},  //duty=12, dutyLt=21
    { 661, 512,1121},  //duty=13, dutyLt=21
    { 667, 512,1106},  //duty=14, dutyLt=21
    { 672, 512,1093},  //duty=15, dutyLt=21
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
    { 506, 512,1760},  //duty=-1, dutyLt=22
    { 525, 512,1631},  //duty=0, dutyLt=22
    { 541, 512,1538},  //duty=1, dutyLt=22
    { 556, 512,1464},  //duty=2, dutyLt=22
    { 569, 512,1406},  //duty=3, dutyLt=22
    { 581, 512,1357},  //duty=4, dutyLt=22
    { 592, 512,1317},  //duty=5, dutyLt=22
    { 602, 512,1282},  //duty=6, dutyLt=22
    { 611, 512,1252},  //duty=7, dutyLt=22
    { 620, 512,1226},  //duty=8, dutyLt=22
    { 628, 512,1201},  //duty=9, dutyLt=22
    { 636, 512,1181},  //duty=10, dutyLt=22
    { 643, 512,1162},  //duty=11, dutyLt=22
    { 650, 512,1144},  //duty=12, dutyLt=22
    { 656, 512,1129},  //duty=13, dutyLt=22
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
    { 540, 512,1541},  //duty=1, dutyLt=23
    { 555, 512,1470},  //duty=2, dutyLt=23
    { 567, 512,1412},  //duty=3, dutyLt=23
    { 579, 512,1364},  //duty=4, dutyLt=23
    { 590, 512,1324},  //duty=5, dutyLt=23
    { 600, 512,1289},  //duty=6, dutyLt=23
    { 609, 512,1259},  //duty=7, dutyLt=23
    { 618, 512,1232},  //duty=8, dutyLt=23
    { 626, 512,1208},  //duty=9, dutyLt=23
    { 633, 512,1187},  //duty=10, dutyLt=23
    { 640, 512,1168},  //duty=11, dutyLt=23
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
    { 508, 512,1751},  //duty=-1, dutyLt=24
    { 525, 512,1632},  //duty=0, dutyLt=24
    { 540, 512,1542},  //duty=1, dutyLt=24
    { 554, 512,1472},  //duty=2, dutyLt=24
    { 566, 512,1415},  //duty=3, dutyLt=24
    { 578, 512,1368},  //duty=4, dutyLt=24
    { 588, 512,1328},  //duty=5, dutyLt=24
    { 598, 512,1294},  //duty=6, dutyLt=24
    { 607, 512,1264},  //duty=7, dutyLt=24
    { 615, 512,1238},  //duty=8, dutyLt=24
    { 623, 512,1214},  //duty=9, dutyLt=24
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
    { 508, 512,1746},  //duty=-1, dutyLt=25
    { 525, 512,1631},  //duty=0, dutyLt=25
    { 540, 512,1545},  //duty=1, dutyLt=25
    { 553, 512,1475},  //duty=2, dutyLt=25
    { 565, 512,1420},  //duty=3, dutyLt=25
    { 576, 512,1373},  //duty=4, dutyLt=25
    { 586, 512,1334},  //duty=5, dutyLt=25
    { 596, 512,1299},  //duty=6, dutyLt=25
    { 605, 512,1270},  //duty=7, dutyLt=25
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
    { 525, 512,1629},  //duty=0, dutyLt=26
    { 539, 512,1546},  //duty=1, dutyLt=26
    { 553, 512,1478},  //duty=2, dutyLt=26
    { 565, 512,1423},  //duty=3, dutyLt=26
    { 575, 512,1377},  //duty=4, dutyLt=26
    { 586, 512,1338},  //duty=5, dutyLt=26
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
    { 526, 512,1627},  //duty=0, dutyLt=27
    { 540, 512,1546},  //duty=1, dutyLt=27
    { 552, 512,1479},  //duty=2, dutyLt=27
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
    { 526, 512,1626},  //duty=0, dutyLt=28
    { 540, 512,1545},  //duty=1, dutyLt=28
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
    { 511, 512,1728},  //duty=-1, dutyLt=29
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
