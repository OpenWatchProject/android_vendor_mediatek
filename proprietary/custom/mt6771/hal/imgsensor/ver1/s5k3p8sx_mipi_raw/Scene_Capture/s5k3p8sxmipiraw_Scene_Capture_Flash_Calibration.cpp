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
#include "s5k3p8sxmipiraw_Scene_Capture.h"

const FLASH_CALIBRATION_NVRAM_T s5k3p8sxmipiraw_Flash_Calibration_0000 = {
    .engTab = {
        .exp = 10000,
        .afe_gain = 1024,
        .isp_gain = 1024,
        .distance = 0,
        .yTab = {
            -1,370,766,1136,1490,1835,2170,2498,2813,3121,3419,3711,3996,4269,4543,4804,5066,5652,6128,6594,7024,7448,7843,8219,8565,8899,9242,
            282,676,1065,1438,1798,2144,2482,2809,3126,3434,3737,4032,4314,4591,4865,5128,5389,5972,6452,6909,7357,7761,8164,8541,8892,9229,-1, 
            588,987,1380,1753,2111,2460,2798,3125,3443,3751,4053,4349,4631,4908,5182,5444,5703,6299,6771,7232,7665,8072,8471,8851,9219,9569,-1, 
            877,1279,1676,2049,2410,2758,3097,3425,3743,4051,4353,4646,4932,5207,5480,5743,6011,6601,7073,7545,7967,8366,8779,9127,9493,-1,-1,  
            1158,1561,1957,2335,2695,3045,3382,3712,4027,4335,4636,4928,5213,5488,5758,6019,6281,6872,7348,7804,8230,8648,9040,9405,9748,-1,-1, 
            1423,1829,2227,2604,2966,3315,3653,3982,4298,4607,4908,5200,5481,5756,6027,6289,6543,7142,7621,8068,8495,8909,9284,9671,-1,-1,-1,   
            1689,2094,2493,2873,3234,3583,3921,4248,4565,4873,5171,5463,5744,6016,6287,6546,6798,7392,7878,8323,8751,9150,9536,9904,-1,-1,-1,   
            1938,2346,2747,3125,3486,3835,4173,4500,4817,5123,5422,5713,5992,6265,6533,6793,7046,7638,8115,8561,8990,9385,9787,-1,-1,-1,-1,     
            2186,2594,2995,3373,3734,4084,4420,4748,5061,5369,5668,5957,6238,6506,6775,7032,7280,7871,8351,8805,9209,9627,9999,-1,-1,-1,-1,     
            2421,2832,3232,3610,3972,4320,4657,4984,5300,5603,5901,6190,6466,6739,7005,7261,7508,8111,8570,9015,9447,9850,-1,-1,-1,-1,-1,       
            2660,3068,3470,3848,4209,4557,4893,5218,5531,5837,6135,6420,6697,6967,7230,7485,7735,8320,8804,9240,9656,-1,-1,-1,-1,-1,-1,         
            2891,3300,3701,4079,4440,4787,5121,5446,5760,6062,6359,6645,6920,7185,7449,7703,7953,8543,9007,9457,-1,-1,-1,-1,-1,-1,-1,           
            3114,3525,3923,4302,4659,5009,5342,5668,5978,6281,6577,6860,7133,7400,7661,7918,8162,8768,9221,-1,-1,-1,-1,-1,-1,-1,-1,             
            3332,3743,4142,4519,4879,5224,5558,5881,6192,6491,6787,7072,7344,7607,7869,8119,8369,8970,-1,-1,-1,-1,-1,-1,-1,-1,-1,               
            3547,3956,4357,4732,5088,5435,5767,6089,6400,6697,6989,7271,7542,7810,8069,8320,8566,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                 
            3753,4164,4561,4938,5293,5638,5968,6291,6597,6896,7188,7476,7739,8006,8265,8514,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                   
            3955,4365,4764,5136,5493,5848,6180,6497,6808,7107,7398,7675,7949,8211,8467,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                     
            4393,4805,5229,5606,5982,6321,6655,6973,7290,7588,7870,8156,8431,8690,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                   
            4770,5178,5596,5974,6342,6688,7011,7331,7651,7950,8220,8506,8800,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                         
            5125,5534,5944,6330,6691,7037,7359,7669,7976,8276,8551,8845,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                           
            5450,5858,6283,6651,7010,7350,7678,7994,8283,8599,8883,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                             
            5779,6178,6585,6958,7311,7654,7971,8298,8603,8887,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                               
            6072,6478,6886,7259,7605,7944,8262,8580,8870,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                                 
            6360,6765,7170,7541,7875,8210,8550,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                                     
            6663,7055,7450,7811,8168,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                                         
            6907,7310,7706,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,                                             
            7159,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1                                                 
        },
    },
    .flashWBGain = {
        {512,512,512 },  //duty=-1, dutyLt=-1
        {1306,512,846},  //duty=0, dutyLt=-1
        {1298,512,837},  //duty=1, dutyLt=-1
        {1299,512,835},  //duty=2, dutyLt=-1
        {1296,512,829},  //duty=3, dutyLt=-1
        {1295,512,823},  //duty=4, dutyLt=-1
        {1293,512,814},  //duty=5, dutyLt=-1
        {1309,512,816},  //duty=6, dutyLt=-1
        {1305,512,813},  //duty=7, dutyLt=-1
        {1304,512,809},  //duty=8, dutyLt=-1
        {1303,512,806},  //duty=9, dutyLt=-1
        {1301,512,802},  //duty=10, dutyLt=-1
        {1300,512,800},  //duty=11, dutyLt=-1
        {1299,512,797},  //duty=12, dutyLt=-1
        {1299,512,795},  //duty=13, dutyLt=-1
        {1297,512,792},  //duty=14, dutyLt=-1
        {1297,512,790},  //duty=15, dutyLt=-1
        {1295,512,785},  //duty=16, dutyLt=-1
        {1294,512,781},  //duty=17, dutyLt=-1
        {1292,512,776},  //duty=18, dutyLt=-1
        {1291,512,772},  //duty=19, dutyLt=-1
        {1290,512,769},  //duty=20, dutyLt=-1
        {1289,512,765},  //duty=21, dutyLt=-1
        {1287,512,762},  //duty=22, dutyLt=-1
        {1287,512,758},  //duty=23, dutyLt=-1
        {1286,512,755},  //duty=24, dutyLt=-1
        {1286,512,753},  //duty=25, dutyLt=-1
        {579,512,1929},  //duty=-1, dutyLt=0
        {862,512,1092},  //duty=0, dutyLt=0
        {980,512,975 },  //duty=1, dutyLt=0
        {1042,512,926},  //duty=2, dutyLt=0
        {1082,512,899},  //duty=3, dutyLt=0
        {1117,512,883},  //duty=4, dutyLt=0
        {1144,512,877},  //duty=5, dutyLt=0
        {1159,512,867},  //duty=6, dutyLt=0
        {1171,512,858},  //duty=7, dutyLt=0
        {1182,512,851},  //duty=8, dutyLt=0
        {1189,512,844},  //duty=9, dutyLt=0
        {1195,512,839},  //duty=10, dutyLt=0
        {1202,512,833},  //duty=11, dutyLt=0
        {1206,512,829},  //duty=12, dutyLt=0
        {1210,512,825},  //duty=13, dutyLt=0
        {1213,512,820},  //duty=14, dutyLt=0
        {1216,512,817},  //duty=15, dutyLt=0
        {1222,512,809},  //duty=16, dutyLt=0
        {1227,512,803},  //duty=17, dutyLt=0
        {1230,512,798},  //duty=18, dutyLt=0
        {1232,512,793},  //duty=19, dutyLt=0
        {1234,512,788},  //duty=20, dutyLt=0
        {1236,512,783},  //duty=21, dutyLt=0
        {1237,512,779},  //duty=22, dutyLt=0
        {1239,512,775},  //duty=23, dutyLt=0
        {1240,512,771},  //duty=24, dutyLt=0
        {512,512,512 },  //duty=25, dutyLt=0
        {578,512,1874},  //duty=-1, dutyLt=1
        {755,512,1234},  //duty=0, dutyLt=1
        {857,512,1075},  //duty=1, dutyLt=1
        {924,512,1003},  //duty=2, dutyLt=1
        {971,512,960 },  //duty=3, dutyLt=1
        {1011,512,942},  //duty=4, dutyLt=1
        {1044,512,929},  //duty=5, dutyLt=1
        {1066,512,913},  //duty=6, dutyLt=1
        {1082,512,899},  //duty=7, dutyLt=1
        {1096,512,889},  //duty=8, dutyLt=1
        {1109,512,879},  //duty=9, dutyLt=1
        {1119,512,871},  //duty=10, dutyLt=1
        {1128,512,864},  //duty=11, dutyLt=1
        {1136,512,858},  //duty=12, dutyLt=1
        {1143,512,852},  //duty=13, dutyLt=1
        {1149,512,846},  //duty=14, dutyLt=1
        {1155,512,842},  //duty=15, dutyLt=1
        {1164,512,832},  //duty=16, dutyLt=1
        {1171,512,824},  //duty=17, dutyLt=1
        {1178,512,817},  //duty=18, dutyLt=1
        {1183,512,811},  //duty=19, dutyLt=1
        {1186,512,805},  //duty=20, dutyLt=1
        {1190,512,799},  //duty=21, dutyLt=1
        {1194,512,795},  //duty=22, dutyLt=1
        {1196,512,790},  //duty=23, dutyLt=1
        {1200,512,787},  //duty=24, dutyLt=1
        {512,512,512 },  //duty=25, dutyLt=1
        {577,512,1886},  //duty=-1, dutyLt=2
        {708,512,1322},  //duty=0, dutyLt=2
        {795,512,1148},  //duty=1, dutyLt=2
        {856,512,1062},  //duty=2, dutyLt=2
        {912,512,1034},  //duty=3, dutyLt=2
        {947,512,998 },  //duty=4, dutyLt=2
        {977,512,972 },  //duty=5, dutyLt=2
        {1000,512,951},  //duty=6, dutyLt=2
        {1018,512,935},  //duty=7, dutyLt=2
        {1035,512,922},  //duty=8, dutyLt=2
        {1049,512,909},  //duty=9, dutyLt=2
        {1062,512,900},  //duty=10, dutyLt=2
        {1073,512,891},  //duty=11, dutyLt=2
        {1082,512,883},  //duty=12, dutyLt=2
        {1090,512,876},  //duty=13, dutyLt=2
        {1098,512,869},  //duty=14, dutyLt=2
        {1105,512,863},  //duty=15, dutyLt=2
        {1117,512,852},  //duty=16, dutyLt=2
        {1124,512,838},  //duty=17, dutyLt=2
        {1135,512,835},  //duty=18, dutyLt=2
        {1142,512,827},  //duty=19, dutyLt=2
        {1148,512,820},  //duty=20, dutyLt=2
        {1153,512,814},  //duty=21, dutyLt=2
        {1158,512,808},  //duty=22, dutyLt=2
        {1162,512,804},  //duty=23, dutyLt=2
        {512,512,512 },  //duty=24, dutyLt=2
        {512,512,512 },  //duty=25, dutyLt=2
        {576,512,1860},  //duty=-1, dutyLt=3
        {681,512,1380},  //duty=0, dutyLt=3
        {756,512,1202},  //duty=1, dutyLt=3
        {814,512,1104},  //duty=2, dutyLt=3
        {859,512,1073},  //duty=3, dutyLt=3
        {894,512,1032},  //duty=4, dutyLt=3
        {927,512,1009},  //duty=5, dutyLt=3
        {951,512,986 },  //duty=6, dutyLt=3
        {972,512,966 },  //duty=7, dutyLt=3
        {989,512,950 },  //duty=8, dutyLt=3
        {1004,512,936},  //duty=9, dutyLt=3
        {1018,512,924},  //duty=10, dutyLt=3
        {1029,512,914},  //duty=11, dutyLt=3
        {1040,512,905},  //duty=12, dutyLt=3
        {1050,512,897},  //duty=13, dutyLt=3
        {1058,512,889},  //duty=14, dutyLt=3
        {1066,512,883},  //duty=15, dutyLt=3
        {1080,512,870},  //duty=16, dutyLt=3
        {1091,512,859},  //duty=17, dutyLt=3
        {1100,512,849},  //duty=18, dutyLt=3
        {1108,512,841},  //duty=19, dutyLt=3
        {1115,512,834},  //duty=20, dutyLt=3
        {1121,512,827},  //duty=21, dutyLt=3
        {1127,512,821},  //duty=22, dutyLt=3
        {1131,512,816},  //duty=23, dutyLt=3
        {512,512,512 },  //duty=24, dutyLt=3
        {512,512,512 },  //duty=25, dutyLt=3
        {580,512,1855},  //duty=-1, dutyLt=4
        {664,512,1422},  //duty=0, dutyLt=4
        {731,512,1244},  //duty=1, dutyLt=4
        {788,512,1184},  //duty=2, dutyLt=4
        {829,512,1121},  //duty=3, dutyLt=4
        {862,512,1076},  //duty=4, dutyLt=4
        {890,512,1042},  //duty=5, dutyLt=4
        {914,512,1016},  //duty=6, dutyLt=4
        {934,512,994 },  //duty=7, dutyLt=4
        {952,512,976 },  //duty=8, dutyLt=4
        {968,512,961 },  //duty=9, dutyLt=4
        {982,512,948 },  //duty=10, dutyLt=4
        {994,512,936 },  //duty=11, dutyLt=4
        {1005,512,926},  //duty=12, dutyLt=4
        {1015,512,917},  //duty=13, dutyLt=4
        {1025,512,908},  //duty=14, dutyLt=4
        {1032,512,901},  //duty=15, dutyLt=4
        {1047,512,886},  //duty=16, dutyLt=4
        {1060,512,875},  //duty=17, dutyLt=4
        {1070,512,864},  //duty=18, dutyLt=4
        {1079,512,855},  //duty=19, dutyLt=4
        {1086,512,847},  //duty=20, dutyLt=4
        {1093,512,840},  //duty=21, dutyLt=4
        {1100,512,833},  //duty=22, dutyLt=4
        {512,512,512 },  //duty=23, dutyLt=4
        {512,512,512 },  //duty=24, dutyLt=4
        {512,512,512 },  //duty=25, dutyLt=4
        {579,512,1841},  //duty=-1, dutyLt=5
        {656,512,1464},  //duty=0, dutyLt=5
        {718,512,1282},  //duty=1, dutyLt=5
        {760,512,1215},  //duty=2, dutyLt=5
        {802,512,1156},  //duty=3, dutyLt=5
        {834,512,1108},  //duty=4, dutyLt=5
        {861,512,1072},  //duty=5, dutyLt=5
        {885,512,1044},  //duty=6, dutyLt=5
        {904,512,1019},  //duty=7, dutyLt=5
        {922,512,1001},  //duty=8, dutyLt=5
        {938,512,983 },  //duty=9, dutyLt=5
        {952,512,969 },  //duty=10, dutyLt=5
        {965,512,956 },  //duty=11, dutyLt=5
        {976,512,945 },  //duty=12, dutyLt=5
        {987,512,935 },  //duty=13, dutyLt=5
        {996,512,925 },  //duty=14, dutyLt=5
        {1004,512,917},  //duty=15, dutyLt=5
        {1020,512,901},  //duty=16, dutyLt=5
        {1033,512,889},  //duty=17, dutyLt=5
        {1044,512,877},  //duty=18, dutyLt=5
        {1053,512,867},  //duty=19, dutyLt=5
        {1062,512,859},  //duty=20, dutyLt=5
        {1070,512,851},  //duty=21, dutyLt=5
        {1076,512,844},  //duty=22, dutyLt=5
        {512,512,512 },  //duty=23, dutyLt=5
        {512,512,512 },  //duty=24, dutyLt=5
        {512,512,512 },  //duty=25, dutyLt=5
        {580,512,1824},  //duty=-1, dutyLt=6
        {647,512,1485},  //duty=0, dutyLt=6
        {703,512,1359},  //duty=1, dutyLt=6
        {747,512,1254},  //duty=2, dutyLt=6
        {782,512,1186},  //duty=3, dutyLt=6
        {812,512,1136},  //duty=4, dutyLt=6
        {838,512,1097},  //duty=5, dutyLt=6
        {861,512,1067},  //duty=6, dutyLt=6
        {880,512,1043},  //duty=7, dutyLt=6
        {898,512,1021},  //duty=8, dutyLt=6
        {913,512,1004},  //duty=9, dutyLt=6
        {927,512,988 },  //duty=10, dutyLt=6
        {940,512,974 },  //duty=11, dutyLt=6
        {952,512,963 },  //duty=12, dutyLt=6
        {962,512,951 },  //duty=13, dutyLt=6
        {972,512,941 },  //duty=14, dutyLt=6
        {981,512,932 },  //duty=15, dutyLt=6
        {997,512,915 },  //duty=16, dutyLt=6
        {1010,512,902},  //duty=17, dutyLt=6
        {1021,512,889},  //duty=18, dutyLt=6
        {1031,512,879},  //duty=19, dutyLt=6
        {1040,512,870},  //duty=20, dutyLt=6
        {1048,512,862},  //duty=21, dutyLt=6
        {512,512,512 },  //duty=22, dutyLt=6
        {512,512,512 },  //duty=23, dutyLt=6
        {512,512,512 },  //duty=24, dutyLt=6
        {512,512,512 },  //duty=25, dutyLt=6
        {580,512,1812},  //duty=-1, dutyLt=7
        {642,512,1563},  //duty=0, dutyLt=7
        {691,512,1387},  //duty=1, dutyLt=7
        {732,512,1285},  //duty=2, dutyLt=7
        {765,512,1214},  //duty=3, dutyLt=7
        {793,512,1162},  //duty=4, dutyLt=7
        {819,512,1122},  //duty=5, dutyLt=7
        {840,512,1090},  //duty=6, dutyLt=7
        {860,512,1063},  //duty=7, dutyLt=7
        {877,512,1041},  //duty=8, dutyLt=7
        {892,512,1022},  //duty=9, dutyLt=7
        {906,512,1006},  //duty=10, dutyLt=7
        {919,512,991 },  //duty=11, dutyLt=7
        {931,512,978 },  //duty=12, dutyLt=7
        {941,512,966 },  //duty=13, dutyLt=7
        {951,512,956 },  //duty=14, dutyLt=7
        {959,512,946 },  //duty=15, dutyLt=7
        {976,512,928 },  //duty=16, dutyLt=7
        {990,512,913 },  //duty=17, dutyLt=7
        {1002,512,901},  //duty=18, dutyLt=7
        {1012,512,890},  //duty=19, dutyLt=7
        {1021,512,880},  //duty=20, dutyLt=7
        {1029,512,871},  //duty=21, dutyLt=7
        {512,512,512 },  //duty=22, dutyLt=7
        {512,512,512 },  //duty=23, dutyLt=7
        {512,512,512 },  //duty=24, dutyLt=7
        {512,512,512 },  //duty=25, dutyLt=7
        {581,512,1798},  //duty=-1, dutyLt=8
        {637,512,1577},  //duty=0, dutyLt=8
        {682,512,1412},  //duty=1, dutyLt=8
        {720,512,1306},  //duty=2, dutyLt=8
        {751,512,1236},  //duty=3, dutyLt=8
        {778,512,1183},  //duty=4, dutyLt=8
        {802,512,1143},  //duty=5, dutyLt=8
        {824,512,1109},  //duty=6, dutyLt=8
        {843,512,1082},  //duty=7, dutyLt=8
        {860,512,1059},  //duty=8, dutyLt=8
        {874,512,1039},  //duty=9, dutyLt=8
        {888,512,1022},  //duty=10, dutyLt=8
        {901,512,1006},  //duty=11, dutyLt=8
        {912,512,993 },  //duty=12, dutyLt=8
        {923,512,981 },  //duty=13, dutyLt=8
        {932,512,969 },  //duty=14, dutyLt=8
        {941,512,959 },  //duty=15, dutyLt=8
        {958,512,940 },  //duty=16, dutyLt=8
        {972,512,925 },  //duty=17, dutyLt=8
        {984,512,911 },  //duty=18, dutyLt=8
        {994,512,900 },  //duty=19, dutyLt=8
        {1004,512,890},  //duty=20, dutyLt=8
        {512,512,512 },  //duty=21, dutyLt=8
        {512,512,512 },  //duty=22, dutyLt=8
        {512,512,512 },  //duty=23, dutyLt=8
        {512,512,512 },  //duty=24, dutyLt=8
        {512,512,512 },  //duty=25, dutyLt=8
        {581,512,1880},  //duty=-1, dutyLt=9
        {632,512,1593},  //duty=0, dutyLt=9
        {674,512,1431},  //duty=1, dutyLt=9
        {709,512,1329},  //duty=2, dutyLt=9
        {739,512,1258},  //duty=3, dutyLt=9
        {766,512,1204},  //duty=4, dutyLt=9
        {789,512,1162},  //duty=5, dutyLt=9
        {809,512,1127},  //duty=6, dutyLt=9
        {827,512,1100},  //duty=7, dutyLt=9
        {844,512,1076},  //duty=8, dutyLt=9
        {859,512,1055},  //duty=9, dutyLt=9
        {872,512,1037},  //duty=10, dutyLt=9
        {885,512,1021},  //duty=11, dutyLt=9
        {896,512,1007},  //duty=12, dutyLt=9
        {906,512,994 },  //duty=13, dutyLt=9
        {916,512,982 },  //duty=14, dutyLt=9
        {924,512,971 },  //duty=15, dutyLt=9
        {939,512,947 },  //duty=16, dutyLt=9
        {956,512,935 },  //duty=17, dutyLt=9
        {968,512,922 },  //duty=18, dutyLt=9
        {979,512,910 },  //duty=19, dutyLt=9
        {512,512,512 },  //duty=20, dutyLt=9
        {512,512,512 },  //duty=21, dutyLt=9
        {512,512,512 },  //duty=22, dutyLt=9
        {512,512,512 },  //duty=23, dutyLt=9
        {512,512,512 },  //duty=24, dutyLt=9
        {512,512,512 },  //duty=25, dutyLt=9
        {582,512,1869},  //duty=-1, dutyLt=10
        {628,512,1612},  //duty=0, dutyLt=10
        {668,512,1449},  //duty=1, dutyLt=10
        {701,512,1350},  //duty=2, dutyLt=10
        {729,512,1278},  //duty=3, dutyLt=10
        {755,512,1223},  //duty=4, dutyLt=10
        {777,512,1180},  //duty=5, dutyLt=10
        {797,512,1145},  //duty=6, dutyLt=10
        {814,512,1115},  //duty=7, dutyLt=10
        {830,512,1091},  //duty=8, dutyLt=10
        {845,512,1070},  //duty=9, dutyLt=10
        {859,512,1051},  //duty=10, dutyLt=10
        {870,512,1035},  //duty=11, dutyLt=10
        {881,512,1020},  //duty=12, dutyLt=10
        {892,512,1006},  //duty=13, dutyLt=10
        {902,512,994 },  //duty=14, dutyLt=10
        {910,512,983 },  //duty=15, dutyLt=10
        {927,512,962 },  //duty=16, dutyLt=10
        {941,512,946 },  //duty=17, dutyLt=10
        {953,512,931 },  //duty=18, dutyLt=10
        {512,512,512 },  //duty=19, dutyLt=10
        {512,512,512 },  //duty=20, dutyLt=10
        {512,512,512 },  //duty=21, dutyLt=10
        {512,512,512 },  //duty=22, dutyLt=10
        {512,512,512 },  //duty=23, dutyLt=10
        {512,512,512 },  //duty=24, dutyLt=10
        {512,512,512 },  //duty=25, dutyLt=10
        {581,512,1867},  //duty=-1, dutyLt=11
        {625,512,1617},  //duty=0, dutyLt=11
        {662,512,1467},  //duty=1, dutyLt=11
        {694,512,1366},  //duty=2, dutyLt=11
        {721,512,1295},  //duty=3, dutyLt=11
        {745,512,1239},  //duty=4, dutyLt=11
        {767,512,1196},  //duty=5, dutyLt=11
        {786,512,1161},  //duty=6, dutyLt=11
        {803,512,1130},  //duty=7, dutyLt=11
        {818,512,1106},  //duty=8, dutyLt=11
        {833,512,1083},  //duty=9, dutyLt=11
        {846,512,1064},  //duty=10, dutyLt=11
        {858,512,1047},  //duty=11, dutyLt=11
        {869,512,1032},  //duty=12, dutyLt=11
        {879,512,1018},  //duty=13, dutyLt=11
        {888,512,1006},  //duty=14, dutyLt=11
        {897,512,994 },  //duty=15, dutyLt=11
        {914,512,973 },  //duty=16, dutyLt=11
        {928,512,955 },  //duty=17, dutyLt=11
        {512,512,512 },  //duty=18, dutyLt=11
        {512,512,512 },  //duty=19, dutyLt=11
        {512,512,512 },  //duty=20, dutyLt=11
        {512,512,512 },  //duty=21, dutyLt=11
        {512,512,512 },  //duty=22, dutyLt=11
        {512,512,512 },  //duty=23, dutyLt=11
        {512,512,512 },  //duty=24, dutyLt=11
        {512,512,512 },  //duty=25, dutyLt=11
        {581,512,1867},  //duty=-1, dutyLt=12
        {622,512,1627},  //duty=0, dutyLt=12
        {658,512,1482},  //duty=1, dutyLt=12
        {688,512,1382},  //duty=2, dutyLt=12
        {714,512,1310},  //duty=3, dutyLt=12
        {737,512,1255},  //duty=4, dutyLt=12
        {757,512,1211},  //duty=5, dutyLt=12
        {776,512,1175},  //duty=6, dutyLt=12
        {793,512,1145},  //duty=7, dutyLt=12
        {808,512,1118},  //duty=8, dutyLt=12
        {822,512,1096},  //duty=9, dutyLt=12
        {835,512,1077},  //duty=10, dutyLt=12
        {846,512,1059},  //duty=11, dutyLt=12
        {857,512,1043},  //duty=12, dutyLt=12
        {867,512,1029},  //duty=13, dutyLt=12
        {877,512,1016},  //duty=14, dutyLt=12
        {885,512,1004},  //duty=15, dutyLt=12
        {902,512,982 },  //duty=16, dutyLt=12
        {512,512,512 },  //duty=17, dutyLt=12
        {512,512,512 },  //duty=18, dutyLt=12
        {512,512,512 },  //duty=19, dutyLt=12
        {512,512,512 },  //duty=20, dutyLt=12
        {512,512,512 },  //duty=21, dutyLt=12
        {512,512,512 },  //duty=22, dutyLt=12
        {512,512,512 },  //duty=23, dutyLt=12
        {512,512,512 },  //duty=24, dutyLt=12
        {512,512,512 },  //duty=25, dutyLt=12
        {582,512,1860},  //duty=-1, dutyLt=13
        {620,512,1636},  //duty=0, dutyLt=13
        {653,512,1492},  //duty=1, dutyLt=13
        {682,512,1396},  //duty=2, dutyLt=13
        {707,512,1324},  //duty=3, dutyLt=13
        {730,512,1269},  //duty=4, dutyLt=13
        {749,512,1224},  //duty=5, dutyLt=13
        {768,512,1188},  //duty=6, dutyLt=13
        {784,512,1157},  //duty=7, dutyLt=13
        {798,512,1131},  //duty=8, dutyLt=13
        {812,512,1108},  //duty=9, dutyLt=13
        {825,512,1087},  //duty=10, dutyLt=13
        {836,512,1070},  //duty=11, dutyLt=13
        {847,512,1054},  //duty=12, dutyLt=13
        {857,512,1039},  //duty=13, dutyLt=13
        {866,512,1026},  //duty=14, dutyLt=13
        {875,512,1014},  //duty=15, dutyLt=13
        {512,512,512 },  //duty=16, dutyLt=13
        {512,512,512 },  //duty=17, dutyLt=13
        {512,512,512 },  //duty=18, dutyLt=13
        {512,512,512 },  //duty=19, dutyLt=13
        {512,512,512 },  //duty=20, dutyLt=13
        {512,512,512 },  //duty=21, dutyLt=13
        {512,512,512 },  //duty=22, dutyLt=13
        {512,512,512 },  //duty=23, dutyLt=13
        {512,512,512 },  //duty=24, dutyLt=13
        {512,512,512 },  //duty=25, dutyLt=13
        {581,512,1852},  //duty=-1, dutyLt=14
        {618,512,1645},  //duty=0, dutyLt=14
        {650,512,1502},  //duty=1, dutyLt=14
        {677,512,1409},  //duty=2, dutyLt=14
        {702,512,1337},  //duty=3, dutyLt=14
        {723,512,1281},  //duty=4, dutyLt=14
        {742,512,1237},  //duty=5, dutyLt=14
        {760,512,1200},  //duty=6, dutyLt=14
        {776,512,1169},  //duty=7, dutyLt=14
        {790,512,1142},  //duty=8, dutyLt=14
        {804,512,1119},  //duty=9, dutyLt=14
        {816,512,1098},  //duty=10, dutyLt=14
        {827,512,1080},  //duty=11, dutyLt=14
        {837,512,1063},  //duty=12, dutyLt=14
        {847,512,1048},  //duty=13, dutyLt=14
        {856,512,1035},  //duty=14, dutyLt=14
        {512,512,512 },  //duty=15, dutyLt=14
        {512,512,512 },  //duty=16, dutyLt=14
        {512,512,512 },  //duty=17, dutyLt=14
        {512,512,512 },  //duty=18, dutyLt=14
        {512,512,512 },  //duty=19, dutyLt=14
        {512,512,512 },  //duty=20, dutyLt=14
        {512,512,512 },  //duty=21, dutyLt=14
        {512,512,512 },  //duty=22, dutyLt=14
        {512,512,512 },  //duty=23, dutyLt=14
        {512,512,512 },  //duty=24, dutyLt=14
        {512,512,512 },  //duty=25, dutyLt=14
        {581,512,1849},  //duty=-1, dutyLt=15
        {616,512,1649},  //duty=0, dutyLt=15
        {647,512,1513},  //duty=1, dutyLt=15
        {673,512,1419},  //duty=2, dutyLt=15
        {697,512,1349},  //duty=3, dutyLt=15
        {718,512,1293},  //duty=4, dutyLt=15
        {736,512,1249},  //duty=5, dutyLt=15
        {753,512,1211},  //duty=6, dutyLt=15
        {768,512,1179},  //duty=7, dutyLt=15
        {783,512,1153},  //duty=8, dutyLt=15
        {795,512,1129},  //duty=9, dutyLt=15
        {808,512,1108},  //duty=10, dutyLt=15
        {819,512,1090},  //duty=11, dutyLt=15
        {829,512,1073},  //duty=12, dutyLt=15
        {839,512,1058},  //duty=13, dutyLt=15
        {512,512,512 },  //duty=14, dutyLt=15
        {512,512,512 },  //duty=15, dutyLt=15
        {512,512,512 },  //duty=16, dutyLt=15
        {512,512,512 },  //duty=17, dutyLt=15
        {512,512,512 },  //duty=18, dutyLt=15
        {512,512,512 },  //duty=19, dutyLt=15
        {512,512,512 },  //duty=20, dutyLt=15
        {512,512,512 },  //duty=21, dutyLt=15
        {512,512,512 },  //duty=22, dutyLt=15
        {512,512,512 },  //duty=23, dutyLt=15
        {512,512,512 },  //duty=24, dutyLt=15
        {512,512,512 },  //duty=25, dutyLt=15
        {581,512,1847},  //duty=-1, dutyLt=16
        {613,512,1659},  //duty=0, dutyLt=16
        {643,512,1525},  //duty=1, dutyLt=16
        {669,512,1423},  //duty=2, dutyLt=16
        {690,512,1362},  //duty=3, dutyLt=16
        {710,512,1309},  //duty=4, dutyLt=16
        {727,512,1256},  //duty=5, dutyLt=16
        {744,512,1219},  //duty=6, dutyLt=16
        {757,512,1197},  //duty=7, dutyLt=16
        {771,512,1169},  //duty=8, dutyLt=16
        {783,512,1145},  //duty=9, dutyLt=16
        {795,512,1123},  //duty=10, dutyLt=16
        {806,512,1104},  //duty=11, dutyLt=16
        {815,512,1088},  //duty=12, dutyLt=16
        {512,512,512 },  //duty=13, dutyLt=16
        {512,512,512 },  //duty=14, dutyLt=16
        {512,512,512 },  //duty=15, dutyLt=16
        {512,512,512 },  //duty=16, dutyLt=16
        {512,512,512 },  //duty=17, dutyLt=16
        {512,512,512 },  //duty=18, dutyLt=16
        {512,512,512 },  //duty=19, dutyLt=16
        {512,512,512 },  //duty=20, dutyLt=16
        {512,512,512 },  //duty=21, dutyLt=16
        {512,512,512 },  //duty=22, dutyLt=16
        {512,512,512 },  //duty=23, dutyLt=16
        {512,512,512 },  //duty=24, dutyLt=16
        {512,512,512 },  //duty=25, dutyLt=16
        {582,512,1838},  //duty=-1, dutyLt=17
        {611,512,1666},  //duty=0, dutyLt=17
        {639,512,1539},  //duty=1, dutyLt=17
        {662,512,1449},  //duty=2, dutyLt=17
        {683,512,1381},  //duty=3, dutyLt=17
        {701,512,1326},  //duty=4, dutyLt=17
        {718,512,1283},  //duty=5, dutyLt=17
        {733,512,1244},  //duty=6, dutyLt=17
        {747,512,1213},  //duty=7, dutyLt=17
        {760,512,1186},  //duty=8, dutyLt=17
        {771,512,1161},  //duty=9, dutyLt=17
        {782,512,1139},  //duty=10, dutyLt=17
        {795,512,1110},  //duty=11, dutyLt=17
        {512,512,512 },  //duty=12, dutyLt=17
        {512,512,512 },  //duty=13, dutyLt=17
        {512,512,512 },  //duty=14, dutyLt=17
        {512,512,512 },  //duty=15, dutyLt=17
        {512,512,512 },  //duty=16, dutyLt=17
        {512,512,512 },  //duty=17, dutyLt=17
        {512,512,512 },  //duty=18, dutyLt=17
        {512,512,512 },  //duty=19, dutyLt=17
        {512,512,512 },  //duty=20, dutyLt=17
        {512,512,512 },  //duty=21, dutyLt=17
        {512,512,512 },  //duty=22, dutyLt=17
        {512,512,512 },  //duty=23, dutyLt=17
        {512,512,512 },  //duty=24, dutyLt=17
        {512,512,512 },  //duty=25, dutyLt=17
        {582,512,1831},  //duty=-1, dutyLt=18
        {610,512,1671},  //duty=0, dutyLt=18
        {635,512,1550},  //duty=1, dutyLt=18
        {657,512,1461},  //duty=2, dutyLt=18
        {677,512,1394},  //duty=3, dutyLt=18
        {694,512,1341},  //duty=4, dutyLt=18
        {710,512,1298},  //duty=5, dutyLt=18
        {724,512,1260},  //duty=6, dutyLt=18
        {738,512,1227},  //duty=7, dutyLt=18
        {750,512,1200},  //duty=8, dutyLt=18
        {761,512,1175},  //duty=9, dutyLt=18
        {772,512,1154},  //duty=10, dutyLt=18
        {512,512,512 },  //duty=11, dutyLt=18
        {512,512,512 },  //duty=12, dutyLt=18
        {512,512,512 },  //duty=13, dutyLt=18
        {512,512,512 },  //duty=14, dutyLt=18
        {512,512,512 },  //duty=15, dutyLt=18
        {512,512,512 },  //duty=16, dutyLt=18
        {512,512,512 },  //duty=17, dutyLt=18
        {512,512,512 },  //duty=18, dutyLt=18
        {512,512,512 },  //duty=19, dutyLt=18
        {512,512,512 },  //duty=20, dutyLt=18
        {512,512,512 },  //duty=21, dutyLt=18
        {512,512,512 },  //duty=22, dutyLt=18
        {512,512,512 },  //duty=23, dutyLt=18
        {512,512,512 },  //duty=24, dutyLt=18
        {512,512,512 },  //duty=25, dutyLt=18
        {583,512,1824},  //duty=-1, dutyLt=19
        {608,512,1672},  //duty=0, dutyLt=19
        {632,512,1557},  //duty=1, dutyLt=19
        {653,512,1472},  //duty=2, dutyLt=19
        {672,512,1406},  //duty=3, dutyLt=19
        {688,512,1354},  //duty=4, dutyLt=19
        {704,512,1310},  //duty=5, dutyLt=19
        {718,512,1272},  //duty=6, dutyLt=19
        {730,512,1241},  //duty=7, dutyLt=19
        {742,512,1213},  //duty=8, dutyLt=19
        {753,512,1188},  //duty=9, dutyLt=19
        {512,512,512 },  //duty=10, dutyLt=19
        {512,512,512 },  //duty=11, dutyLt=19
        {512,512,512 },  //duty=12, dutyLt=19
        {512,512,512 },  //duty=13, dutyLt=19
        {512,512,512 },  //duty=14, dutyLt=19
        {512,512,512 },  //duty=15, dutyLt=19
        {512,512,512 },  //duty=16, dutyLt=19
        {512,512,512 },  //duty=17, dutyLt=19
        {512,512,512 },  //duty=18, dutyLt=19
        {512,512,512 },  //duty=19, dutyLt=19
        {512,512,512 },  //duty=20, dutyLt=19
        {512,512,512 },  //duty=21, dutyLt=19
        {512,512,512 },  //duty=22, dutyLt=19
        {512,512,512 },  //duty=23, dutyLt=19
        {512,512,512 },  //duty=24, dutyLt=19
        {512,512,512 },  //duty=25, dutyLt=19
        {583,512,1820},  //duty=-1, dutyLt=20
        {607,512,1675},  //duty=0, dutyLt=20
        {630,512,1563},  //duty=1, dutyLt=20
        {650,512,1482},  //duty=2, dutyLt=20
        {668,512,1416},  //duty=3, dutyLt=20
        {684,512,1364},  //duty=4, dutyLt=20
        {699,512,1320},  //duty=5, dutyLt=20
        {712,512,1284},  //duty=6, dutyLt=20
        {723,512,1253},  //duty=7, dutyLt=20
        {736,512,1223},  //duty=8, dutyLt=20
        {512,512,512 },  //duty=9, dutyLt=20
        {512,512,512 },  //duty=10, dutyLt=20
        {512,512,512 },  //duty=11, dutyLt=20
        {512,512,512 },  //duty=12, dutyLt=20
        {512,512,512 },  //duty=13, dutyLt=20
        {512,512,512 },  //duty=14, dutyLt=20
        {512,512,512 },  //duty=15, dutyLt=20
        {512,512,512 },  //duty=16, dutyLt=20
        {512,512,512 },  //duty=17, dutyLt=20
        {512,512,512 },  //duty=18, dutyLt=20
        {512,512,512 },  //duty=19, dutyLt=20
        {512,512,512 },  //duty=20, dutyLt=20
        {512,512,512 },  //duty=21, dutyLt=20
        {512,512,512 },  //duty=22, dutyLt=20
        {512,512,512 },  //duty=23, dutyLt=20
        {512,512,512 },  //duty=24, dutyLt=20
        {512,512,512 },  //duty=25, dutyLt=20
        {583,512,1813},  //duty=-1, dutyLt=21
        {606,512,1676},  //duty=0, dutyLt=21
        {629,512,1567},  //duty=1, dutyLt=21
        {648,512,1488},  //duty=2, dutyLt=21
        {664,512,1424},  //duty=3, dutyLt=21
        {679,512,1374},  //duty=4, dutyLt=21
        {694,512,1330},  //duty=5, dutyLt=21
        {706,512,1293},  //duty=6, dutyLt=21
        {719,512,1261},  //duty=7, dutyLt=21
        {512,512,512 },  //duty=8, dutyLt=21
        {512,512,512 },  //duty=9, dutyLt=21
        {512,512,512 },  //duty=10, dutyLt=21
        {512,512,512 },  //duty=11, dutyLt=21
        {512,512,512 },  //duty=12, dutyLt=21
        {512,512,512 },  //duty=13, dutyLt=21
        {512,512,512 },  //duty=14, dutyLt=21
        {512,512,512 },  //duty=15, dutyLt=21
        {512,512,512 },  //duty=16, dutyLt=21
        {512,512,512 },  //duty=17, dutyLt=21
        {512,512,512 },  //duty=18, dutyLt=21
        {512,512,512 },  //duty=19, dutyLt=21
        {512,512,512 },  //duty=20, dutyLt=21
        {512,512,512 },  //duty=21, dutyLt=21
        {512,512,512 },  //duty=22, dutyLt=21
        {512,512,512 },  //duty=23, dutyLt=21
        {512,512,512 },  //duty=24, dutyLt=21
        {512,512,512 },  //duty=25, dutyLt=21
        {584,512,1807},  //duty=-1, dutyLt=22
        {606,512,1676},  //duty=0, dutyLt=22
        {627,512,1571},  //duty=1, dutyLt=22
        {645,512,1493},  //duty=2, dutyLt=22
        {661,512,1432},  //duty=3, dutyLt=22
        {676,512,1380},  //duty=4, dutyLt=22
        {694,512,1322},  //duty=5, dutyLt=22
        {512,512,512 },  //duty=6, dutyLt=22
        {512,512,512 },  //duty=7, dutyLt=22
        {512,512,512 },  //duty=8, dutyLt=22
        {512,512,512 },  //duty=9, dutyLt=22
        {512,512,512 },  //duty=10, dutyLt=22
        {512,512,512 },  //duty=11, dutyLt=22
        {512,512,512 },  //duty=12, dutyLt=22
        {512,512,512 },  //duty=13, dutyLt=22
        {512,512,512 },  //duty=14, dutyLt=22
        {512,512,512 },  //duty=15, dutyLt=22
        {512,512,512 },  //duty=16, dutyLt=22
        {512,512,512 },  //duty=17, dutyLt=22
        {512,512,512 },  //duty=18, dutyLt=22
        {512,512,512 },  //duty=19, dutyLt=22
        {512,512,512 },  //duty=20, dutyLt=22
        {512,512,512 },  //duty=21, dutyLt=22
        {512,512,512 },  //duty=22, dutyLt=22
        {512,512,512 },  //duty=23, dutyLt=22
        {512,512,512 },  //duty=24, dutyLt=22
        {512,512,512 },  //duty=25, dutyLt=22
        {584,512,1802},  //duty=-1, dutyLt=23
        {605,512,1678},  //duty=0, dutyLt=23
        {625,512,1576},  //duty=1, dutyLt=23
        {643,512,1498},  //duty=2, dutyLt=23
        {659,512,1438},  //duty=3, dutyLt=23
        {512,512,512 },  //duty=4, dutyLt=23
        {512,512,512 },  //duty=5, dutyLt=23
        {512,512,512 },  //duty=6, dutyLt=23
        {512,512,512 },  //duty=7, dutyLt=23
        {512,512,512 },  //duty=8, dutyLt=23
        {512,512,512 },  //duty=9, dutyLt=23
        {512,512,512 },  //duty=10, dutyLt=23
        {512,512,512 },  //duty=11, dutyLt=23
        {512,512,512 },  //duty=12, dutyLt=23
        {512,512,512 },  //duty=13, dutyLt=23
        {512,512,512 },  //duty=14, dutyLt=23
        {512,512,512 },  //duty=15, dutyLt=23
        {512,512,512 },  //duty=16, dutyLt=23
        {512,512,512 },  //duty=17, dutyLt=23
        {512,512,512 },  //duty=18, dutyLt=23
        {512,512,512 },  //duty=19, dutyLt=23
        {512,512,512 },  //duty=20, dutyLt=23
        {512,512,512 },  //duty=21, dutyLt=23
        {512,512,512 },  //duty=22, dutyLt=23
        {512,512,512 },  //duty=23, dutyLt=23
        {512,512,512 },  //duty=24, dutyLt=23
        {512,512,512 },  //duty=25, dutyLt=23
        {584,512,1797},  //duty=-1, dutyLt=24
        {605,512,1678},  //duty=0, dutyLt=24
        {624,512,1579},  //duty=1, dutyLt=24
        {512,512,512 },  //duty=2, dutyLt=24
        {512,512,512 },  //duty=3, dutyLt=24
        {512,512,512 },  //duty=4, dutyLt=24
        {512,512,512 },  //duty=5, dutyLt=24
        {512,512,512 },  //duty=6, dutyLt=24
        {512,512,512 },  //duty=7, dutyLt=24
        {512,512,512 },  //duty=8, dutyLt=24
        {512,512,512 },  //duty=9, dutyLt=24
        {512,512,512 },  //duty=10, dutyLt=24
        {512,512,512 },  //duty=11, dutyLt=24
        {512,512,512 },  //duty=12, dutyLt=24
        {512,512,512 },  //duty=13, dutyLt=24
        {512,512,512 },  //duty=14, dutyLt=24
        {512,512,512 },  //duty=15, dutyLt=24
        {512,512,512 },  //duty=16, dutyLt=24
        {512,512,512 },  //duty=17, dutyLt=24
        {512,512,512 },  //duty=18, dutyLt=24
        {512,512,512 },  //duty=19, dutyLt=24
        {512,512,512 },  //duty=20, dutyLt=24
        {512,512,512 },  //duty=21, dutyLt=24
        {512,512,512 },  //duty=22, dutyLt=24
        {512,512,512 },  //duty=23, dutyLt=24
        {512,512,512 },  //duty=24, dutyLt=24
        {512,512,512 },  //duty=25, dutyLt=24
        {585,512,1792},  //duty=-1, dutyLt=25
        {512,512,512 },  //duty=0, dutyLt=25
        {512,512,512 },  //duty=1, dutyLt=25
        {512,512,512 },  //duty=2, dutyLt=25
        {512,512,512 },  //duty=3, dutyLt=25
        {512,512,512 },  //duty=4, dutyLt=25
        {512,512,512 },  //duty=5, dutyLt=25
        {512,512,512 },  //duty=6, dutyLt=25
        {512,512,512 },  //duty=7, dutyLt=25
        {512,512,512 },  //duty=8, dutyLt=25
        {512,512,512 },  //duty=9, dutyLt=25
        {512,512,512 },  //duty=10, dutyLt=25
        {512,512,512 },  //duty=11, dutyLt=25
        {512,512,512 },  //duty=12, dutyLt=25
        {512,512,512 },  //duty=13, dutyLt=25
        {512,512,512 },  //duty=14, dutyLt=25
        {512,512,512 },  //duty=15, dutyLt=25
        {512,512,512 },  //duty=16, dutyLt=25
        {512,512,512 },  //duty=17, dutyLt=25
        {512,512,512 },  //duty=18, dutyLt=25
        {512,512,512 },  //duty=19, dutyLt=25
        {512,512,512 },  //duty=20, dutyLt=25
        {512,512,512 },  //duty=21, dutyLt=25
        {512,512,512 },  //duty=22, dutyLt=25
        {512,512,512 },  //duty=23, dutyLt=25
        {512,512,512 },  //duty=24, dutyLt=25
        {512,512,512 },  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
        { 512, 512, 512},  //duty=25, dutyLt=25
    },
};
