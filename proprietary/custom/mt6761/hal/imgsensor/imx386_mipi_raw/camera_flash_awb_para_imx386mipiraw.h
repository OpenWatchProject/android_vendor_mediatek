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

// Flash AWB tuning parameter
{
    9,  //foreground percentage
    95, //background percentage
    2, //FgPercentage_Th1
    5, //FgPercentage_Th2
    10, //FgPercentage_Th3
    15, //FgPercentage_Th4
    250, //FgPercentage_Th1_Val
    250, //FgPercentage_Th2_Val
    250, //FgPercentage_Th3_Val
    250, //FgPercentage_Th4_Val
    10, //location_map_th1
    20, //location_map_th2
    40, //location_map_th3
    50, //location_map_th4
    100, //location_map_val1
    100, //location_map_val2
    100, //location_map_val3
    100, //location_map_val4
    0, //SelfTuningFbBgWeightTbl
    100, //FgBgTbl_Y0
    100, //FgBgTbl_Y1
    100, //FgBgTbl_Y2
    100, //FgBgTbl_Y3
    100, //FgBgTbl_Y4
    100, //FgBgTbl_Y5
    5, //YPrimeWeightTh[0]
    9, //YPrimeWeightTh[1]
    11, //YPrimeWeightTh[2]
    13, //YPrimeWeightTh[3]
    15, //YPrimeWeightTh[4]
    1, //YPrimeWeight[0]
    3, //YPrimeWeight[1]
    5, //YPrimeWeight[2]
    7, //YPrimeWeight[3]
    // FlashPreferenceGain
    {
        512,	// FlashPreferenceGain.i4R
        512,	// FlashPreferenceGain.i4G
        512	  // FlashPreferenceGain.i4B
    },
},

// Flash AWB calibration
{
    {
        { 512, 512, 512},  //duty=-1, dutyLt=-1
        {1244, 512,1028},  //duty=0, dutyLt=-1
        {1172, 512, 980},  //duty=1, dutyLt=-1
        {1163, 512, 968},  //duty=2, dutyLt=-1
        {1157, 512, 960},  //duty=3, dutyLt=-1
        {1153, 512, 953},  //duty=4, dutyLt=-1
        {1151, 512, 945},  //duty=5, dutyLt=-1
        {1151, 512, 921},  //duty=6, dutyLt=-1
        {1151, 512, 940},  //duty=7, dutyLt=-1
        {1148, 512, 937},  //duty=8, dutyLt=-1
        {1148, 512, 933},  //duty=9, dutyLt=-1
        {1148, 512, 930},  //duty=10, dutyLt=-1
        {1147, 512, 927},  //duty=11, dutyLt=-1
        {1146, 512, 923},  //duty=12, dutyLt=-1
        {1146, 512, 920},  //duty=13, dutyLt=-1
        {1145, 512, 917},  //duty=14, dutyLt=-1
        {1145, 512, 914},  //duty=15, dutyLt=-1
        {1144, 512, 909},  //duty=16, dutyLt=-1
        {1144, 512, 904},  //duty=17, dutyLt=-1
        {1144, 512, 899},  //duty=18, dutyLt=-1
        {1143, 512, 893},  //duty=19, dutyLt=-1
        {1143, 512, 889},  //duty=20, dutyLt=-1
        {1142, 512, 884},  //duty=21, dutyLt=-1
        {1142, 512, 879},  //duty=22, dutyLt=-1
        {1142, 512, 875},  //duty=23, dutyLt=-1
        {1142, 512, 871},  //duty=24, dutyLt=-1
        {1142, 512, 867},  //duty=25, dutyLt=-1
        {1142, 512, 862},  //duty=26, dutyLt=-1
        {1143, 512, 859},  //duty=27, dutyLt=-1
        { 514, 512,3255},  //duty=-1, dutyLt=0
        { 769, 512,1314},  //duty=0, dutyLt=0
        { 873, 512,1161},  //duty=1, dutyLt=0
        { 925, 512,1102},  //duty=2, dutyLt=0
        { 960, 512,1066},  //duty=3, dutyLt=0
        { 984, 512,1040},  //duty=4, dutyLt=0
        {1002, 512,1014},  //duty=5, dutyLt=0
        {1025, 512,1007},  //duty=6, dutyLt=0
        {1035, 512, 997},  //duty=7, dutyLt=0
        {1043, 512, 991},  //duty=8, dutyLt=0
        {1052, 512, 983},  //duty=9, dutyLt=0
        {1057, 512, 976},  //duty=10, dutyLt=0
        {1061, 512, 970},  //duty=11, dutyLt=0
        {1067, 512, 964},  //duty=12, dutyLt=0
        {1070, 512, 959},  //duty=13, dutyLt=0
        {1074, 512, 954},  //duty=14, dutyLt=0
        {1073, 512, 945},  //duty=15, dutyLt=0
        {1080, 512, 937},  //duty=16, dutyLt=0
        {1084, 512, 929},  //duty=17, dutyLt=0
        {1088, 512, 923},  //duty=18, dutyLt=0
        {1091, 512, 916},  //duty=19, dutyLt=0
        {1093, 512, 910},  //duty=20, dutyLt=0
        {1095, 512, 905},  //duty=21, dutyLt=0
        {1096, 512, 899},  //duty=22, dutyLt=0
        {1098, 512, 894},  //duty=23, dutyLt=0
        {1100, 512, 889},  //duty=24, dutyLt=0
        {1102, 512, 884},  //duty=25, dutyLt=0
        {1103, 512, 880},  //duty=26, dutyLt=0
        {1104, 512, 875},  //duty=27, dutyLt=0
        { 512, 512,2799},  //duty=-1, dutyLt=1
        { 667, 512,1518},  //duty=0, dutyLt=1
        { 764, 512,1301},  //duty=1, dutyLt=1
        { 824, 512,1203},  //duty=2, dutyLt=1
        { 859, 512,1148},  //duty=3, dutyLt=1
        { 890, 512,1105},  //duty=4, dutyLt=1
        { 920, 512,1082},  //duty=5, dutyLt=1
        { 940, 512,1062},  //duty=6, dutyLt=1
        { 956, 512,1047},  //duty=7, dutyLt=1
        { 969, 512,1035},  //duty=8, dutyLt=1
        { 980, 512,1024},  //duty=9, dutyLt=1
        { 991, 512,1014},  //duty=10, dutyLt=1
        { 998, 512,1006},  //duty=11, dutyLt=1
        {1006, 512, 998},  //duty=12, dutyLt=1
        {1011, 512, 990},  //duty=13, dutyLt=1
        {1018, 512, 984},  //duty=14, dutyLt=1
        {1022, 512, 978},  //duty=15, dutyLt=1
        {1031, 512, 967},  //duty=16, dutyLt=1
        {1038, 512, 958},  //duty=17, dutyLt=1
        {1045, 512, 950},  //duty=18, dutyLt=1
        {1050, 512, 942},  //duty=19, dutyLt=1
        {1054, 512, 935},  //duty=20, dutyLt=1
        {1054, 512, 924},  //duty=21, dutyLt=1
        {1058, 512, 917},  //duty=22, dutyLt=1
        {1061, 512, 912},  //duty=23, dutyLt=1
        {1063, 512, 906},  //duty=24, dutyLt=1
        {1065, 512, 901},  //duty=25, dutyLt=1
        {1068, 512, 897},  //duty=26, dutyLt=1
        {1070, 512, 893},  //duty=27, dutyLt=1
        { 508, 512,2498},  //duty=-1, dutyLt=2
        { 624, 512,1646},  //duty=0, dutyLt=2
        { 700, 512,1405},  //duty=1, dutyLt=2
        { 762, 512,1287},  //duty=2, dutyLt=2
        { 796, 512,1214},  //duty=3, dutyLt=2
        { 834, 512,1167},  //duty=4, dutyLt=2
        { 860, 512,1137},  //duty=5, dutyLt=2
        { 881, 512,1110},  //duty=6, dutyLt=2
        { 899, 512,1092},  //duty=7, dutyLt=2
        { 915, 512,1074},  //duty=8, dutyLt=2
        { 927, 512,1061},  //duty=9, dutyLt=2
        { 939, 512,1049},  //duty=10, dutyLt=2
        { 949, 512,1038},  //duty=11, dutyLt=2
        { 958, 512,1028},  //duty=12, dutyLt=2
        { 965, 512,1019},  //duty=13, dutyLt=2
        { 973, 512,1012},  //duty=14, dutyLt=2
        { 979, 512,1004},  //duty=15, dutyLt=2
        { 991, 512, 991},  //duty=16, dutyLt=2
        {1001, 512, 980},  //duty=17, dutyLt=2
        {1008, 512, 969},  //duty=18, dutyLt=2
        {1015, 512, 961},  //duty=19, dutyLt=2
        {1020, 512, 952},  //duty=20, dutyLt=2
        {1025, 512, 945},  //duty=21, dutyLt=2
        {1030, 512, 938},  //duty=22, dutyLt=2
        {1034, 512, 931},  //duty=23, dutyLt=2
        {1037, 512, 924},  //duty=24, dutyLt=2
        {1037, 512, 915},  //duty=25, dutyLt=2
        {1040, 512, 909},  //duty=26, dutyLt=2
        {1043, 512, 904},  //duty=27, dutyLt=2
        { 511, 512,2473},  //duty=-1, dutyLt=3
        { 602, 512,1728},  //duty=0, dutyLt=3
        { 668, 512,1479},  //duty=1, dutyLt=3
        { 719, 512,1348},  //duty=2, dutyLt=3
        { 756, 512,1259},  //duty=3, dutyLt=3
        { 791, 512,1218},  //duty=4, dutyLt=3
        { 817, 512,1183},  //duty=5, dutyLt=3
        { 839, 512,1153},  //duty=6, dutyLt=3
        { 857, 512,1131},  //duty=7, dutyLt=3
        { 873, 512,1110},  //duty=8, dutyLt=3
        { 887, 512,1094},  //duty=9, dutyLt=3
        { 899, 512,1080},  //duty=10, dutyLt=3
        { 910, 512,1067},  //duty=11, dutyLt=3
        { 920, 512,1056},  //duty=12, dutyLt=3
        { 928, 512,1045},  //duty=13, dutyLt=3
        { 936, 512,1036},  //duty=14, dutyLt=3
        { 944, 512,1028},  //duty=15, dutyLt=3
        { 957, 512,1013},  //duty=16, dutyLt=3
        { 967, 512,1000},  //duty=17, dutyLt=3
        { 977, 512, 988},  //duty=18, dutyLt=3
        { 984, 512, 978},  //duty=19, dutyLt=3
        { 991, 512, 969},  //duty=20, dutyLt=3
        { 997, 512, 961},  //duty=21, dutyLt=3
        {1002, 512, 953},  //duty=22, dutyLt=3
        {1007, 512, 945},  //duty=23, dutyLt=3
        {1011, 512, 938},  //duty=24, dutyLt=3
        {1015, 512, 932},  //duty=25, dutyLt=3
        {1018, 512, 926},  //duty=26, dutyLt=3
        {1022, 512, 920},  //duty=27, dutyLt=3
        { 511, 512,2447},  //duty=-1, dutyLt=4
        { 589, 512,1803},  //duty=0, dutyLt=4
        { 642, 512,1551},  //duty=1, dutyLt=4
        { 694, 512,1403},  //duty=2, dutyLt=4
        { 723, 512,1322},  //duty=3, dutyLt=4
        { 753, 512,1267},  //duty=4, dutyLt=4
        { 785, 512,1224},  //duty=5, dutyLt=4
        { 805, 512,1190},  //duty=6, dutyLt=4
        { 824, 512,1165},  //duty=7, dutyLt=4
        { 840, 512,1143},  //duty=8, dutyLt=4
        { 854, 512,1124},  //duty=9, dutyLt=4
        { 867, 512,1108},  //duty=10, dutyLt=4
        { 878, 512,1094},  //duty=11, dutyLt=4
        { 888, 512,1081},  //duty=12, dutyLt=4
        { 898, 512,1070},  //duty=13, dutyLt=4
        { 906, 512,1059},  //duty=14, dutyLt=4
        { 914, 512,1050},  //duty=15, dutyLt=4
        { 928, 512,1033},  //duty=16, dutyLt=4
        { 939, 512,1019},  //duty=17, dutyLt=4
        { 949, 512,1006},  //duty=18, dutyLt=4
        { 958, 512, 995},  //duty=19, dutyLt=4
        { 966, 512, 985},  //duty=20, dutyLt=4
        { 972, 512, 976},  //duty=21, dutyLt=4
        { 978, 512, 967},  //duty=22, dutyLt=4
        { 984, 512, 959},  //duty=23, dutyLt=4
        { 989, 512, 953},  //duty=24, dutyLt=4
        { 992, 512, 945},  //duty=25, dutyLt=4
        { 997, 512, 938},  //duty=26, dutyLt=4
        { 512, 512, 512},  //duty=27, dutyLt=4
        { 509, 512,2420},  //duty=-1, dutyLt=5
        { 578, 512,1851},  //duty=0, dutyLt=5
        { 630, 512,1599},  //duty=1, dutyLt=5
        { 667, 512,1458},  //duty=2, dutyLt=5
        { 700, 512,1368},  //duty=3, dutyLt=5
        { 728, 512,1308},  //duty=4, dutyLt=5
        { 752, 512,1263},  //duty=5, dutyLt=5
        { 780, 512,1226},  //duty=6, dutyLt=5
        { 797, 512,1198},  //duty=7, dutyLt=5
        { 814, 512,1173},  //duty=8, dutyLt=5
        { 828, 512,1153},  //duty=9, dutyLt=5
        { 840, 512,1134},  //duty=10, dutyLt=5
        { 852, 512,1119},  //duty=11, dutyLt=5
        { 863, 512,1105},  //duty=12, dutyLt=5
        { 872, 512,1093},  //duty=13, dutyLt=5
        { 881, 512,1081},  //duty=14, dutyLt=5
        { 889, 512,1070},  //duty=15, dutyLt=5
        { 904, 512,1052},  //duty=16, dutyLt=5
        { 917, 512,1037},  //duty=17, dutyLt=5
        { 926, 512,1023},  //duty=18, dutyLt=5
        { 936, 512,1011},  //duty=19, dutyLt=5
        { 944, 512,1000},  //duty=20, dutyLt=5
        { 951, 512, 990},  //duty=21, dutyLt=5
        { 957, 512, 981},  //duty=22, dutyLt=5
        { 962, 512, 972},  //duty=23, dutyLt=5
        { 968, 512, 964},  //duty=24, dutyLt=5
        { 972, 512, 956},  //duty=25, dutyLt=5
        { 977, 512, 950},  //duty=26, dutyLt=5
        { 512, 512, 512},  //duty=27, dutyLt=5
        { 510, 512,2417},  //duty=-1, dutyLt=6
        { 570, 512,1889},  //duty=0, dutyLt=6
        { 619, 512,1628},  //duty=1, dutyLt=6
        { 651, 512,1499},  //duty=2, dutyLt=6
        { 682, 512,1406},  //duty=3, dutyLt=6
        { 709, 512,1344},  //duty=4, dutyLt=6
        { 732, 512,1297},  //duty=5, dutyLt=6
        { 752, 512,1259},  //duty=6, dutyLt=6
        { 776, 512,1226},  //duty=7, dutyLt=6
        { 791, 512,1200},  //duty=8, dutyLt=6
        { 806, 512,1178},  //duty=9, dutyLt=6
        { 819, 512,1159},  //duty=10, dutyLt=6
        { 830, 512,1142},  //duty=11, dutyLt=6
        { 841, 512,1126},  //duty=12, dutyLt=6
        { 851, 512,1113},  //duty=13, dutyLt=6
        { 859, 512,1101},  //duty=14, dutyLt=6
        { 867, 512,1089},  //duty=15, dutyLt=6
        { 883, 512,1070},  //duty=16, dutyLt=6
        { 895, 512,1053},  //duty=17, dutyLt=6
        { 906, 512,1039},  //duty=18, dutyLt=6
        { 916, 512,1025},  //duty=19, dutyLt=6
        { 924, 512,1013},  //duty=20, dutyLt=6
        { 932, 512,1003},  //duty=21, dutyLt=6
        { 938, 512, 993},  //duty=22, dutyLt=6
        { 945, 512, 985},  //duty=23, dutyLt=6
        { 950, 512, 977},  //duty=24, dutyLt=6
        { 955, 512, 968},  //duty=25, dutyLt=6
        { 512, 512, 512},  //duty=26, dutyLt=6
        { 512, 512, 512},  //duty=27, dutyLt=6
        { 510, 512,2405},  //duty=-1, dutyLt=7
        { 564, 512,1913},  //duty=0, dutyLt=7
        { 603, 512,1677},  //duty=1, dutyLt=7
        { 638, 512,1535},  //duty=2, dutyLt=7
        { 668, 512,1443},  //duty=3, dutyLt=7
        { 692, 512,1378},  //duty=4, dutyLt=7
        { 715, 512,1328},  //duty=5, dutyLt=7
        { 734, 512,1287},  //duty=6, dutyLt=7
        { 751, 512,1255},  //duty=7, dutyLt=7
        { 773, 512,1225},  //duty=8, dutyLt=7
        { 787, 512,1202},  //duty=9, dutyLt=7
        { 800, 512,1182},  //duty=10, dutyLt=7
        { 811, 512,1163},  //duty=11, dutyLt=7
        { 822, 512,1147},  //duty=12, dutyLt=7
        { 831, 512,1132},  //duty=13, dutyLt=7
        { 840, 512,1119},  //duty=14, dutyLt=7
        { 849, 512,1108},  //duty=15, dutyLt=7
        { 864, 512,1086},  //duty=16, dutyLt=7
        { 877, 512,1068},  //duty=17, dutyLt=7
        { 888, 512,1053},  //duty=18, dutyLt=7
        { 898, 512,1039},  //duty=19, dutyLt=7
        { 907, 512,1027},  //duty=20, dutyLt=7
        { 915, 512,1015},  //duty=21, dutyLt=7
        { 922, 512,1005},  //duty=22, dutyLt=7
        { 928, 512, 996},  //duty=23, dutyLt=7
        { 933, 512, 987},  //duty=24, dutyLt=7
        { 939, 512, 979},  //duty=25, dutyLt=7
        { 512, 512, 512},  //duty=26, dutyLt=7
        { 512, 512, 512},  //duty=27, dutyLt=7
        { 506, 512,2395},  //duty=-1, dutyLt=8
        { 558, 512,1913},  //duty=0, dutyLt=8
        { 595, 512,1707},  //duty=1, dutyLt=8
        { 628, 512,1567},  //duty=2, dutyLt=8
        { 656, 512,1474},  //duty=3, dutyLt=8
        { 679, 512,1407},  //duty=4, dutyLt=8
        { 700, 512,1355},  //duty=5, dutyLt=8
        { 719, 512,1313},  //duty=6, dutyLt=8
        { 736, 512,1279},  //duty=7, dutyLt=8
        { 757, 512,1249},  //duty=8, dutyLt=8
        { 771, 512,1224},  //duty=9, dutyLt=8
        { 783, 512,1202},  //duty=10, dutyLt=8
        { 795, 512,1183},  //duty=11, dutyLt=8
        { 806, 512,1166},  //duty=12, dutyLt=8
        { 815, 512,1151},  //duty=13, dutyLt=8
        { 824, 512,1137},  //duty=14, dutyLt=8
        { 832, 512,1124},  //duty=15, dutyLt=8
        { 848, 512,1102},  //duty=16, dutyLt=8
        { 861, 512,1083},  //duty=17, dutyLt=8
        { 872, 512,1067},  //duty=18, dutyLt=8
        { 883, 512,1052},  //duty=19, dutyLt=8
        { 891, 512,1039},  //duty=20, dutyLt=8
        { 899, 512,1027},  //duty=21, dutyLt=8
        { 906, 512,1016},  //duty=22, dutyLt=8
        { 913, 512,1007},  //duty=23, dutyLt=8
        { 919, 512, 998},  //duty=24, dutyLt=8
        { 512, 512, 512},  //duty=25, dutyLt=8
        { 512, 512, 512},  //duty=26, dutyLt=8
        { 512, 512, 512},  //duty=27, dutyLt=8
        { 511, 512,2373},  //duty=-1, dutyLt=9
        { 557, 512,1951},  //duty=0, dutyLt=9
        { 588, 512,1733},  //duty=1, dutyLt=9
        { 619, 512,1596},  //duty=2, dutyLt=9
        { 645, 512,1502},  //duty=3, dutyLt=9
        { 668, 512,1434},  //duty=4, dutyLt=9
        { 689, 512,1380},  //duty=5, dutyLt=9
        { 706, 512,1337},  //duty=6, dutyLt=9
        { 723, 512,1302},  //duty=7, dutyLt=9
        { 738, 512,1272},  //duty=8, dutyLt=9
        { 757, 512,1244},  //duty=9, dutyLt=9
        { 769, 512,1222},  //duty=10, dutyLt=9
        { 780, 512,1201},  //duty=11, dutyLt=9
        { 791, 512,1184},  //duty=12, dutyLt=9
        { 801, 512,1168},  //duty=13, dutyLt=9
        { 809, 512,1154},  //duty=14, dutyLt=9
        { 818, 512,1141},  //duty=15, dutyLt=9
        { 834, 512,1117},  //duty=16, dutyLt=9
        { 847, 512,1097},  //duty=17, dutyLt=9
        { 858, 512,1080},  //duty=18, dutyLt=9
        { 868, 512,1065},  //duty=19, dutyLt=9
        { 877, 512,1051},  //duty=20, dutyLt=9
        { 885, 512,1038},  //duty=21, dutyLt=9
        { 893, 512,1028},  //duty=22, dutyLt=9
        { 899, 512,1017},  //duty=23, dutyLt=9
        { 905, 512,1007},  //duty=24, dutyLt=9
        { 512, 512, 512},  //duty=25, dutyLt=9
        { 512, 512, 512},  //duty=26, dutyLt=9
        { 512, 512, 512},  //duty=27, dutyLt=9
        { 512, 512,2349},  //duty=-1, dutyLt=10
        { 553, 512,1965},  //duty=0, dutyLt=10
        { 583, 512,1754},  //duty=1, dutyLt=10
        { 612, 512,1619},  //duty=2, dutyLt=10
        { 637, 512,1527},  //duty=3, dutyLt=10
        { 659, 512,1458},  //duty=4, dutyLt=10
        { 678, 512,1403},  //duty=5, dutyLt=10
        { 696, 512,1359},  //duty=6, dutyLt=10
        { 712, 512,1322},  //duty=7, dutyLt=10
        { 725, 512,1291},  //duty=8, dutyLt=10
        { 739, 512,1264},  //duty=9, dutyLt=10
        { 757, 512,1240},  //duty=10, dutyLt=10
        { 768, 512,1219},  //duty=11, dutyLt=10
        { 779, 512,1200},  //duty=12, dutyLt=10
        { 788, 512,1184},  //duty=13, dutyLt=10
        { 797, 512,1169},  //duty=14, dutyLt=10
        { 805, 512,1155},  //duty=15, dutyLt=10
        { 821, 512,1131},  //duty=16, dutyLt=10
        { 834, 512,1110},  //duty=17, dutyLt=10
        { 845, 512,1092},  //duty=18, dutyLt=10
        { 856, 512,1076},  //duty=19, dutyLt=10
        { 865, 512,1062},  //duty=20, dutyLt=10
        { 873, 512,1049},  //duty=21, dutyLt=10
        { 880, 512,1037},  //duty=22, dutyLt=10
        { 887, 512,1027},  //duty=23, dutyLt=10
        { 512, 512, 512},  //duty=24, dutyLt=10
        { 512, 512, 512},  //duty=25, dutyLt=10
        { 512, 512, 512},  //duty=26, dutyLt=10
        { 512, 512, 512},  //duty=27, dutyLt=10
        { 512, 512,2342},  //duty=-1, dutyLt=11
        { 551, 512,1980},  //duty=0, dutyLt=11
        { 578, 512,1777},  //duty=1, dutyLt=11
        { 605, 512,1644},  //duty=2, dutyLt=11
        { 629, 512,1551},  //duty=3, dutyLt=11
        { 651, 512,1481},  //duty=4, dutyLt=11
        { 669, 512,1425},  //duty=5, dutyLt=11
        { 686, 512,1379},  //duty=6, dutyLt=11
        { 702, 512,1341},  //duty=7, dutyLt=11
        { 715, 512,1310},  //duty=8, dutyLt=11
        { 728, 512,1282},  //duty=9, dutyLt=11
        { 740, 512,1259},  //duty=10, dutyLt=11
        { 757, 512,1235},  //duty=11, dutyLt=11
        { 767, 512,1216},  //duty=12, dutyLt=11
        { 777, 512,1198},  //duty=13, dutyLt=11
        { 785, 512,1183},  //duty=14, dutyLt=11
        { 793, 512,1169},  //duty=15, dutyLt=11
        { 809, 512,1143},  //duty=16, dutyLt=11
        { 822, 512,1122},  //duty=17, dutyLt=11
        { 834, 512,1104},  //duty=18, dutyLt=11
        { 844, 512,1087},  //duty=19, dutyLt=11
        { 853, 512,1073},  //duty=20, dutyLt=11
        { 862, 512,1060},  //duty=21, dutyLt=11
        { 869, 512,1048},  //duty=22, dutyLt=11
        { 876, 512,1036},  //duty=23, dutyLt=11
        { 512, 512, 512},  //duty=24, dutyLt=11
        { 512, 512, 512},  //duty=25, dutyLt=11
        { 512, 512, 512},  //duty=26, dutyLt=11
        { 512, 512, 512},  //duty=27, dutyLt=11
        { 512, 512,2345},  //duty=-1, dutyLt=12
        { 548, 512,1991},  //duty=0, dutyLt=12
        { 574, 512,1796},  //duty=1, dutyLt=12
        { 600, 512,1665},  //duty=2, dutyLt=12
        { 623, 512,1572},  //duty=3, dutyLt=12
        { 643, 512,1500},  //duty=4, dutyLt=12
        { 661, 512,1443},  //duty=5, dutyLt=12
        { 678, 512,1398},  //duty=6, dutyLt=12
        { 693, 512,1359},  //duty=7, dutyLt=12
        { 706, 512,1326},  //duty=8, dutyLt=12
        { 719, 512,1299},  //duty=9, dutyLt=12
        { 730, 512,1274},  //duty=10, dutyLt=12
        { 741, 512,1252},  //duty=11, dutyLt=12
        { 757, 512,1230},  //duty=12, dutyLt=12
        { 766, 512,1213},  //duty=13, dutyLt=12
        { 775, 512,1196},  //duty=14, dutyLt=12
        { 783, 512,1182},  //duty=15, dutyLt=12
        { 798, 512,1155},  //duty=16, dutyLt=12
        { 812, 512,1133},  //duty=17, dutyLt=12
        { 823, 512,1115},  //duty=18, dutyLt=12
        { 834, 512,1098},  //duty=19, dutyLt=12
        { 843, 512,1083},  //duty=20, dutyLt=12
        { 851, 512,1069},  //duty=21, dutyLt=12
        { 859, 512,1057},  //duty=22, dutyLt=12
        { 512, 512, 512},  //duty=23, dutyLt=12
        { 512, 512, 512},  //duty=24, dutyLt=12
        { 512, 512, 512},  //duty=25, dutyLt=12
        { 512, 512, 512},  //duty=26, dutyLt=12
        { 512, 512, 512},  //duty=27, dutyLt=12
        { 512, 512,2344},  //duty=-1, dutyLt=13
        { 546, 512,2006},  //duty=0, dutyLt=13
        { 570, 512,1815},  //duty=1, dutyLt=13
        { 595, 512,1685},  //duty=2, dutyLt=13
        { 617, 512,1591},  //duty=3, dutyLt=13
        { 637, 512,1519},  //duty=4, dutyLt=13
        { 654, 512,1461},  //duty=5, dutyLt=13
        { 670, 512,1414},  //duty=6, dutyLt=13
        { 685, 512,1376},  //duty=7, dutyLt=13
        { 698, 512,1343},  //duty=8, dutyLt=13
        { 710, 512,1314},  //duty=9, dutyLt=13
        { 722, 512,1288},  //duty=10, dutyLt=13
        { 732, 512,1266},  //duty=11, dutyLt=13
        { 748, 512,1244},  //duty=12, dutyLt=13
        { 757, 512,1226},  //duty=13, dutyLt=13
        { 765, 512,1209},  //duty=14, dutyLt=13
        { 773, 512,1195},  //duty=15, dutyLt=13
        { 789, 512,1167},  //duty=16, dutyLt=13
        { 802, 512,1145},  //duty=17, dutyLt=13
        { 814, 512,1125},  //duty=18, dutyLt=13
        { 824, 512,1108},  //duty=19, dutyLt=13
        { 833, 512,1093},  //duty=20, dutyLt=13
        { 841, 512,1079},  //duty=21, dutyLt=13
        { 849, 512,1066},  //duty=22, dutyLt=13
        { 512, 512, 512},  //duty=23, dutyLt=13
        { 512, 512, 512},  //duty=24, dutyLt=13
        { 512, 512, 512},  //duty=25, dutyLt=13
        { 512, 512, 512},  //duty=26, dutyLt=13
        { 512, 512, 512},  //duty=27, dutyLt=13
        { 512, 512,2327},  //duty=-1, dutyLt=14
        { 544, 512,2018},  //duty=0, dutyLt=14
        { 567, 512,1830},  //duty=1, dutyLt=14
        { 591, 512,1700},  //duty=2, dutyLt=14
        { 612, 512,1607},  //duty=3, dutyLt=14
        { 631, 512,1536},  //duty=4, dutyLt=14
        { 648, 512,1479},  //duty=5, dutyLt=14
        { 664, 512,1431},  //duty=6, dutyLt=14
        { 678, 512,1391},  //duty=7, dutyLt=14
        { 691, 512,1358},  //duty=8, dutyLt=14
        { 703, 512,1328},  //duty=9, dutyLt=14
        { 714, 512,1302},  //duty=10, dutyLt=14
        { 724, 512,1279},  //duty=11, dutyLt=14
        { 734, 512,1259},  //duty=12, dutyLt=14
        { 749, 512,1239},  //duty=13, dutyLt=14
        { 757, 512,1222},  //duty=14, dutyLt=14
        { 765, 512,1206},  //duty=15, dutyLt=14
        { 780, 512,1178},  //duty=16, dutyLt=14
        { 793, 512,1155},  //duty=17, dutyLt=14
        { 805, 512,1135},  //duty=18, dutyLt=14
        { 815, 512,1117},  //duty=19, dutyLt=14
        { 824, 512,1101},  //duty=20, dutyLt=14
        { 833, 512,1087},  //duty=21, dutyLt=14
        { 512, 512, 512},  //duty=22, dutyLt=14
        { 512, 512, 512},  //duty=23, dutyLt=14
        { 512, 512, 512},  //duty=24, dutyLt=14
        { 512, 512, 512},  //duty=25, dutyLt=14
        { 512, 512, 512},  //duty=26, dutyLt=14
        { 512, 512, 512},  //duty=27, dutyLt=14
        { 512, 512,2316},  //duty=-1, dutyLt=15
        { 543, 512,2024},  //duty=0, dutyLt=15
        { 564, 512,1842},  //duty=1, dutyLt=15
        { 588, 512,1714},  //duty=2, dutyLt=15
        { 608, 512,1622},  //duty=3, dutyLt=15
        { 626, 512,1552},  //duty=4, dutyLt=15
        { 643, 512,1494},  //duty=5, dutyLt=15
        { 658, 512,1446},  //duty=6, dutyLt=15
        { 672, 512,1406},  //duty=7, dutyLt=15
        { 684, 512,1371},  //duty=8, dutyLt=15
        { 696, 512,1341},  //duty=9, dutyLt=15
        { 707, 512,1315},  //duty=10, dutyLt=15
        { 717, 512,1291},  //duty=11, dutyLt=15
        { 726, 512,1270},  //duty=12, dutyLt=15
        { 735, 512,1251},  //duty=13, dutyLt=15
        { 750, 512,1233},  //duty=14, dutyLt=15
        { 758, 512,1217},  //duty=15, dutyLt=15
        { 772, 512,1189},  //duty=16, dutyLt=15
        { 785, 512,1165},  //duty=17, dutyLt=15
        { 797, 512,1144},  //duty=18, dutyLt=15
        { 807, 512,1126},  //duty=19, dutyLt=15
        { 816, 512,1110},  //duty=20, dutyLt=15
        { 824, 512,1095},  //duty=21, dutyLt=15
        { 512, 512, 512},  //duty=22, dutyLt=15
        { 512, 512, 512},  //duty=23, dutyLt=15
        { 512, 512, 512},  //duty=24, dutyLt=15
        { 512, 512, 512},  //duty=25, dutyLt=15
        { 512, 512, 512},  //duty=26, dutyLt=15
        { 512, 512, 512},  //duty=27, dutyLt=15
        { 512, 512,2311},  //duty=-1, dutyLt=16
        { 540, 512,2033},  //duty=0, dutyLt=16
        { 561, 512,1856},  //duty=1, dutyLt=16
        { 582, 512,1736},  //duty=2, dutyLt=16
        { 601, 512,1646},  //duty=3, dutyLt=16
        { 619, 512,1574},  //duty=4, dutyLt=16
        { 634, 512,1517},  //duty=5, dutyLt=16
        { 648, 512,1469},  //duty=6, dutyLt=16
        { 662, 512,1429},  //duty=7, dutyLt=16
        { 674, 512,1394},  //duty=8, dutyLt=16
        { 685, 512,1365},  //duty=9, dutyLt=16
        { 695, 512,1337},  //duty=10, dutyLt=16
        { 705, 512,1313},  //duty=11, dutyLt=16
        { 714, 512,1292},  //duty=12, dutyLt=16
        { 723, 512,1271},  //duty=13, dutyLt=16
        { 731, 512,1255},  //duty=14, dutyLt=16
        { 745, 512,1236},  //duty=15, dutyLt=16
        { 758, 512,1208},  //duty=16, dutyLt=16
        { 771, 512,1183},  //duty=17, dutyLt=16
        { 782, 512,1162},  //duty=18, dutyLt=16
        { 792, 512,1144},  //duty=19, dutyLt=16
        { 802, 512,1127},  //duty=20, dutyLt=16
        { 512, 512, 512},  //duty=21, dutyLt=16
        { 512, 512, 512},  //duty=22, dutyLt=16
        { 512, 512, 512},  //duty=23, dutyLt=16
        { 512, 512, 512},  //duty=24, dutyLt=16
        { 512, 512, 512},  //duty=25, dutyLt=16
        { 512, 512, 512},  //duty=26, dutyLt=16
        { 512, 512, 512},  //duty=27, dutyLt=16
        { 512, 512,2297},  //duty=-1, dutyLt=17
        { 539, 512,2046},  //duty=0, dutyLt=17
        { 557, 512,1877},  //duty=1, dutyLt=17
        { 577, 512,1760},  //duty=2, dutyLt=17
        { 595, 512,1669},  //duty=3, dutyLt=17
        { 611, 512,1599},  //duty=4, dutyLt=17
        { 626, 512,1540},  //duty=5, dutyLt=17
        { 640, 512,1492},  //duty=6, dutyLt=17
        { 652, 512,1452},  //duty=7, dutyLt=17
        { 664, 512,1416},  //duty=8, dutyLt=17
        { 674, 512,1386},  //duty=9, dutyLt=17
        { 685, 512,1358},  //duty=10, dutyLt=17
        { 695, 512,1333},  //duty=11, dutyLt=17
        { 703, 512,1312},  //duty=12, dutyLt=17
        { 712, 512,1292},  //duty=13, dutyLt=17
        { 719, 512,1273},  //duty=14, dutyLt=17
        { 727, 512,1256},  //duty=15, dutyLt=17
        { 746, 512,1226},  //duty=16, dutyLt=17
        { 759, 512,1201},  //duty=17, dutyLt=17
        { 769, 512,1178},  //duty=18, dutyLt=17
        { 780, 512,1159},  //duty=19, dutyLt=17
        { 512, 512, 512},  //duty=20, dutyLt=17
        { 512, 512, 512},  //duty=21, dutyLt=17
        { 512, 512, 512},  //duty=22, dutyLt=17
        { 512, 512, 512},  //duty=23, dutyLt=17
        { 512, 512, 512},  //duty=24, dutyLt=17
        { 512, 512, 512},  //duty=25, dutyLt=17
        { 512, 512, 512},  //duty=26, dutyLt=17
        { 512, 512, 512},  //duty=27, dutyLt=17
        { 513, 512,2293},  //duty=-1, dutyLt=18
        { 537, 512,2050},  //duty=0, dutyLt=18
        { 555, 512,1889},  //duty=1, dutyLt=18
        { 573, 512,1776},  //duty=2, dutyLt=18
        { 590, 512,1690},  //duty=3, dutyLt=18
        { 606, 512,1618},  //duty=4, dutyLt=18
        { 620, 512,1561},  //duty=5, dutyLt=18
        { 633, 512,1513},  //duty=6, dutyLt=18
        { 645, 512,1472},  //duty=7, dutyLt=18
        { 656, 512,1435},  //duty=8, dutyLt=18
        { 667, 512,1404},  //duty=9, dutyLt=18
        { 676, 512,1376},  //duty=10, dutyLt=18
        { 686, 512,1351},  //duty=11, dutyLt=18
        { 694, 512,1329},  //duty=12, dutyLt=18
        { 703, 512,1308},  //duty=13, dutyLt=18
        { 710, 512,1289},  //duty=14, dutyLt=18
        { 717, 512,1272},  //duty=15, dutyLt=18
        { 730, 512,1243},  //duty=16, dutyLt=18
        { 748, 512,1215},  //duty=17, dutyLt=18
        { 759, 512,1192},  //duty=18, dutyLt=18
        { 512, 512, 512},  //duty=19, dutyLt=18
        { 512, 512, 512},  //duty=20, dutyLt=18
        { 512, 512, 512},  //duty=21, dutyLt=18
        { 512, 512, 512},  //duty=22, dutyLt=18
        { 512, 512, 512},  //duty=23, dutyLt=18
        { 512, 512, 512},  //duty=24, dutyLt=18
        { 512, 512, 512},  //duty=25, dutyLt=18
        { 512, 512, 512},  //duty=26, dutyLt=18
        { 512, 512, 512},  //duty=27, dutyLt=18
        { 513, 512,2278},  //duty=-1, dutyLt=19
        { 536, 512,2056},  //duty=0, dutyLt=19
        { 552, 512,1901},  //duty=1, dutyLt=19
        { 570, 512,1791},  //duty=2, dutyLt=19
        { 586, 512,1704},  //duty=3, dutyLt=19
        { 601, 512,1635},  //duty=4, dutyLt=19
        { 615, 512,1577},  //duty=5, dutyLt=19
        { 627, 512,1528},  //duty=6, dutyLt=19
        { 639, 512,1488},  //duty=7, dutyLt=19
        { 649, 512,1452},  //duty=8, dutyLt=19
        { 660, 512,1420},  //duty=9, dutyLt=19
        { 669, 512,1392},  //duty=10, dutyLt=19
        { 678, 512,1367},  //duty=11, dutyLt=19
        { 686, 512,1345},  //duty=12, dutyLt=19
        { 694, 512,1323},  //duty=13, dutyLt=19
        { 701, 512,1305},  //duty=14, dutyLt=19
        { 709, 512,1287},  //duty=15, dutyLt=19
        { 721, 512,1257},  //duty=16, dutyLt=19
        { 733, 512,1230},  //duty=17, dutyLt=19
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
        { 513, 512,2270},  //duty=-1, dutyLt=20
        { 535, 512,2064},  //duty=0, dutyLt=20
        { 550, 512,1912},  //duty=1, dutyLt=20
        { 567, 512,1804},  //duty=2, dutyLt=20
        { 583, 512,1718},  //duty=3, dutyLt=20
        { 597, 512,1649},  //duty=4, dutyLt=20
        { 610, 512,1593},  //duty=5, dutyLt=20
        { 622, 512,1545},  //duty=6, dutyLt=20
        { 633, 512,1502},  //duty=7, dutyLt=20
        { 644, 512,1466},  //duty=8, dutyLt=20
        { 653, 512,1437},  //duty=9, dutyLt=20
        { 662, 512,1408},  //duty=10, dutyLt=20
        { 671, 512,1380},  //duty=11, dutyLt=20
        { 679, 512,1360},  //duty=12, dutyLt=20
        { 687, 512,1338},  //duty=13, dutyLt=20
        { 694, 512,1318},  //duty=14, dutyLt=20
        { 701, 512,1300},  //duty=15, dutyLt=20
        { 714, 512,1270},  //duty=16, dutyLt=20
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
        { 509, 512,2265},  //duty=-1, dutyLt=21
        { 530, 512,2066},  //duty=0, dutyLt=21
        { 549, 512,1920},  //duty=1, dutyLt=21
        { 565, 512,1814},  //duty=2, dutyLt=21
        { 580, 512,1732},  //duty=3, dutyLt=21
        { 594, 512,1661},  //duty=4, dutyLt=21
        { 606, 512,1605},  //duty=5, dutyLt=21
        { 618, 512,1558},  //duty=6, dutyLt=21
        { 628, 512,1516},  //duty=7, dutyLt=21
        { 639, 512,1480},  //duty=8, dutyLt=21
        { 648, 512,1447},  //duty=9, dutyLt=21
        { 657, 512,1419},  //duty=10, dutyLt=21
        { 665, 512,1394},  //duty=11, dutyLt=21
        { 673, 512,1370},  //duty=12, dutyLt=21
        { 681, 512,1348},  //duty=13, dutyLt=21
        { 688, 512,1329},  //duty=14, dutyLt=21
        { 695, 512,1311},  //duty=15, dutyLt=21
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
        { 510, 512,2259},  //duty=-1, dutyLt=22
        { 529, 512,2067},  //duty=0, dutyLt=22
        { 548, 512,1924},  //duty=1, dutyLt=22
        { 563, 512,1822},  //duty=2, dutyLt=22
        { 578, 512,1740},  //duty=3, dutyLt=22
        { 591, 512,1672},  //duty=4, dutyLt=22
        { 603, 512,1616},  //duty=5, dutyLt=22
        { 614, 512,1567},  //duty=6, dutyLt=22
        { 625, 512,1528},  //duty=7, dutyLt=22
        { 635, 512,1491},  //duty=8, dutyLt=22
        { 643, 512,1460},  //duty=9, dutyLt=22
        { 652, 512,1431},  //duty=10, dutyLt=22
        { 661, 512,1404},  //duty=11, dutyLt=22
        { 668, 512,1381},  //duty=12, dutyLt=22
        { 676, 512,1360},  //duty=13, dutyLt=22
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
        { 510, 512,2252},  //duty=-1, dutyLt=23
        { 529, 512,2068},  //duty=0, dutyLt=23
        { 546, 512,1929},  //duty=1, dutyLt=23
        { 562, 512,1828},  //duty=2, dutyLt=23
        { 576, 512,1747},  //duty=3, dutyLt=23
        { 589, 512,1680},  //duty=4, dutyLt=23
        { 600, 512,1626},  //duty=5, dutyLt=23
        { 611, 512,1578},  //duty=6, dutyLt=23
        { 622, 512,1536},  //duty=7, dutyLt=23
        { 631, 512,1501},  //duty=8, dutyLt=23
        { 640, 512,1468},  //duty=9, dutyLt=23
        { 648, 512,1441},  //duty=10, dutyLt=23
        { 657, 512,1414},  //duty=11, dutyLt=23
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
        { 510, 512,2248},  //duty=-1, dutyLt=24
        { 529, 512,2066},  //duty=0, dutyLt=24
        { 546, 512,1932},  //duty=1, dutyLt=24
        { 560, 512,1834},  //duty=2, dutyLt=24
        { 574, 512,1755},  //duty=3, dutyLt=24
        { 587, 512,1687},  //duty=4, dutyLt=24
        { 598, 512,1636},  //duty=5, dutyLt=24
        { 609, 512,1587},  //duty=6, dutyLt=24
        { 619, 512,1546},  //duty=7, dutyLt=24
        { 628, 512,1510},  //duty=8, dutyLt=24
        { 637, 512,1477},  //duty=9, dutyLt=24
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
        { 511, 512,2238},  //duty=-1, dutyLt=25
        { 529, 512,2065},  //duty=0, dutyLt=25
        { 545, 512,1936},  //duty=1, dutyLt=25
        { 559, 512,1839},  //duty=2, dutyLt=25
        { 572, 512,1762},  //duty=3, dutyLt=25
        { 585, 512,1695},  //duty=4, dutyLt=25
        { 596, 512,1640},  //duty=5, dutyLt=25
        { 607, 512,1593},  //duty=6, dutyLt=25
        { 616, 512,1553},  //duty=7, dutyLt=25
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
        { 512, 512,2231},  //duty=-1, dutyLt=26
        { 529, 512,2066},  //duty=0, dutyLt=26
        { 545, 512,1936},  //duty=1, dutyLt=26
        { 559, 512,1841},  //duty=2, dutyLt=26
        { 571, 512,1765},  //duty=3, dutyLt=26
        { 583, 512,1703},  //duty=4, dutyLt=26
        { 594, 512,1648},  //duty=5, dutyLt=26
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
        { 512, 512,2223},  //duty=-1, dutyLt=27
        { 529, 512,2064},  //duty=0, dutyLt=27
        { 545, 512,1938},  //duty=1, dutyLt=27
        { 558, 512,1844},  //duty=2, dutyLt=27
        { 570, 512,1769},  //duty=3, dutyLt=27
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
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512},  //duty=27, dutyLt=27
        { 512, 512, 512}
    }
}

