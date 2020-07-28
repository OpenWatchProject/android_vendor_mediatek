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
        { 512, 512, 512},	//duty=-1, dutyLt=-1
        {1176, 512, 986},	//duty=0, dutyLt=-1
        {1164, 512, 973},	//duty=1, dutyLt=-1
        {1158, 512, 964},	//duty=2, dutyLt=-1
        {1156, 512, 958},	//duty=3, dutyLt=-1
        {1154, 512, 952},	//duty=4, dutyLt=-1
        {1153, 512, 944},	//duty=5, dutyLt=-1
        {1152, 512, 927},	//duty=6, dutyLt=-1
        {1156, 512, 946},	//duty=7, dutyLt=-1
        {1154, 512, 941},	//duty=8, dutyLt=-1
        {1154, 512, 938},	//duty=9, dutyLt=-1
        {1153, 512, 934},	//duty=10, dutyLt=-1
        {1152, 512, 931},	//duty=11, dutyLt=-1
        {1151, 512, 928},	//duty=12, dutyLt=-1
        {1151, 512, 924},	//duty=13, dutyLt=-1
        {1150, 512, 921},	//duty=14, dutyLt=-1
        {1151, 512, 918},	//duty=15, dutyLt=-1
        {1149, 512, 907},	//duty=16, dutyLt=-1
        {1147, 512, 901},	//duty=17, dutyLt=-1
        {1147, 512, 896},	//duty=18, dutyLt=-1
        {1147, 512, 891},	//duty=19, dutyLt=-1
        {1147, 512, 886},	//duty=20, dutyLt=-1
        {1146, 512, 881},	//duty=21, dutyLt=-1
        {1146, 512, 877},	//duty=22, dutyLt=-1
        {1146, 512, 872},	//duty=23, dutyLt=-1
        {1146, 512, 867},	//duty=24, dutyLt=-1
        {1146, 512, 863},	//duty=25, dutyLt=-1
        {1146, 512, 859},	//duty=26, dutyLt=-1
        {1146, 512, 854},	//duty=27, dutyLt=-1
        { 511, 512,2599},	//duty=-1, dutyLt=0
        { 767, 512,1298},	//duty=0, dutyLt=0
        { 872, 512,1149},	//duty=1, dutyLt=0
        { 930, 512,1090},	//duty=2, dutyLt=0
        { 966, 512,1056},	//duty=3, dutyLt=0
        { 991, 512,1031},	//duty=4, dutyLt=0
        {1010, 512,1006},	//duty=5, dutyLt=0
        {1027, 512,1005},	//duty=6, dutyLt=0
        {1037, 512, 996},	//duty=7, dutyLt=0
        {1046, 512, 988},	//duty=8, dutyLt=0
        {1053, 512, 981},	//duty=9, dutyLt=0
        {1058, 512, 971},	//duty=10, dutyLt=0
        {1064, 512, 968},	//duty=11, dutyLt=0
        {1068, 512, 963},	//duty=12, dutyLt=0
        {1073, 512, 957},	//duty=13, dutyLt=0
        {1075, 512, 952},	//duty=14, dutyLt=0
        {1078, 512, 948},	//duty=15, dutyLt=0
        {1084, 512, 935},	//duty=16, dutyLt=0
        {1088, 512, 927},	//duty=17, dutyLt=0
        {1091, 512, 921},	//duty=18, dutyLt=0
        {1095, 512, 914},	//duty=19, dutyLt=0
        {1097, 512, 908},	//duty=20, dutyLt=0
        {1099, 512, 902},	//duty=21, dutyLt=0
        {1100, 512, 896},	//duty=22, dutyLt=0
        {1102, 512, 891},	//duty=23, dutyLt=0
        {1103, 512, 886},	//duty=24, dutyLt=0
        {1105, 512, 881},	//duty=25, dutyLt=0
        {1106, 512, 876},	//duty=26, dutyLt=0
        {1107, 512, 871},	//duty=27, dutyLt=0
        { 512, 512,2461},	//duty=-1, dutyLt=1
        { 669, 512,1507},	//duty=0, dutyLt=1
        { 763, 512,1282},	//duty=1, dutyLt=1
        { 823, 512,1190},	//duty=2, dutyLt=1
        { 865, 512,1136},	//duty=3, dutyLt=1
        { 897, 512,1095},	//duty=4, dutyLt=1
        { 923, 512,1079},	//duty=5, dutyLt=1
        { 942, 512,1059},	//duty=6, dutyLt=1
        { 957, 512,1042},	//duty=7, dutyLt=1
        { 972, 512,1031},	//duty=8, dutyLt=1
        { 982, 512,1021},	//duty=9, dutyLt=1
        { 991, 512,1011},	//duty=10, dutyLt=1
        {1001, 512,1003},	//duty=11, dutyLt=1
        {1009, 512, 995},	//duty=12, dutyLt=1
        {1014, 512, 987},	//duty=13, dutyLt=1
        {1020, 512, 981},	//duty=14, dutyLt=1
        {1025, 512, 975},	//duty=15, dutyLt=1
        {1032, 512, 961},	//duty=16, dutyLt=1
        {1039, 512, 951},	//duty=17, dutyLt=1
        {1044, 512, 942},	//duty=18, dutyLt=1
        {1050, 512, 935},	//duty=19, dutyLt=1
        {1054, 512, 928},	//duty=20, dutyLt=1
        {1058, 512, 921},	//duty=21, dutyLt=1
        {1061, 512, 914},	//duty=22, dutyLt=1
        {1064, 512, 908},	//duty=23, dutyLt=1
        {1067, 512, 902},	//duty=24, dutyLt=1
        {1069, 512, 897},	//duty=25, dutyLt=1
        {1071, 512, 892},	//duty=26, dutyLt=1
        {1073, 512, 886},	//duty=27, dutyLt=1
        { 509, 512,2459},	//duty=-1, dutyLt=2
        { 626, 512,1636},	//duty=0, dutyLt=2
        { 703, 512,1403},	//duty=1, dutyLt=2
        { 761, 512,1273},	//duty=2, dutyLt=2
        { 803, 512,1202},	//duty=3, dutyLt=2
        { 836, 512,1164},	//duty=4, dutyLt=2
        { 862, 512,1131},	//duty=5, dutyLt=2
        { 883, 512,1105},	//duty=6, dutyLt=2
        { 901, 512,1084},	//duty=7, dutyLt=2
        { 916, 512,1071},	//duty=8, dutyLt=2
        { 930, 512,1057},	//duty=9, dutyLt=2
        { 941, 512,1044},	//duty=10, dutyLt=2
        { 952, 512,1034},	//duty=11, dutyLt=2
        { 958, 512,1021},	//duty=12, dutyLt=2
        { 968, 512,1015},	//duty=13, dutyLt=2
        { 976, 512,1008},	//duty=14, dutyLt=2
        { 982, 512,1000},	//duty=15, dutyLt=2
        { 991, 512, 984},	//duty=16, dutyLt=2
        {1000, 512, 973},	//duty=17, dutyLt=2
        {1008, 512, 962},	//duty=18, dutyLt=2
        {1014, 512, 953},	//duty=19, dutyLt=2
        {1020, 512, 945},	//duty=20, dutyLt=2
        {1026, 512, 937},	//duty=21, dutyLt=2
        {1030, 512, 930},	//duty=22, dutyLt=2
        {1034, 512, 923},	//duty=23, dutyLt=2
        {1037, 512, 917},	//duty=24, dutyLt=2
        {1040, 512, 911},	//duty=25, dutyLt=2
        {1044, 512, 905},	//duty=26, dutyLt=2
        {1046, 512, 899},	//duty=27, dutyLt=2
        { 509, 512,2426},	//duty=-1, dutyLt=3
        { 602, 512,1725},	//duty=0, dutyLt=3
        { 669, 512,1483},	//duty=1, dutyLt=3
        { 721, 512,1339},	//duty=2, dutyLt=3
        { 763, 512,1237},	//duty=3, dutyLt=3
        { 792, 512,1215},	//duty=4, dutyLt=3
        { 818, 512,1174},	//duty=5, dutyLt=3
        { 840, 512,1146},	//duty=6, dutyLt=3
        { 859, 512,1124},	//duty=7, dutyLt=3
        { 875, 512,1106},	//duty=8, dutyLt=3
        { 888, 512,1091},	//duty=9, dutyLt=3
        { 901, 512,1075},	//duty=10, dutyLt=3
        { 912, 512,1063},	//duty=11, dutyLt=3
        { 922, 512,1051},	//duty=12, dutyLt=3
        { 929, 512,1038},	//duty=13, dutyLt=3
        { 939, 512,1032},	//duty=14, dutyLt=3
        { 946, 512,1024},	//duty=15, dutyLt=3
        { 957, 512,1005},	//duty=16, dutyLt=3
        { 968, 512, 992},	//duty=17, dutyLt=3
        { 977, 512, 981},	//duty=18, dutyLt=3
        { 984, 512, 970},	//duty=19, dutyLt=3
        { 991, 512, 961},	//duty=20, dutyLt=3
        { 997, 512, 953},	//duty=21, dutyLt=3
        {1002, 512, 945},	//duty=22, dutyLt=3
        {1007, 512, 937},	//duty=23, dutyLt=3
        {1011, 512, 931},	//duty=24, dutyLt=3
        {1015, 512, 924},	//duty=25, dutyLt=3
        {1018, 512, 918},	//duty=26, dutyLt=3
        {1021, 512, 912},	//duty=27, dutyLt=3
        { 509, 512,2415},	//duty=-1, dutyLt=4
        { 588, 512,1769},	//duty=0, dutyLt=4
        { 647, 512,1528},	//duty=1, dutyLt=4
        { 694, 512,1387},	//duty=2, dutyLt=4
        { 730, 512,1319},	//duty=3, dutyLt=4
        { 759, 512,1258},	//duty=4, dutyLt=4
        { 785, 512,1217},	//duty=5, dutyLt=4
        { 807, 512,1183},	//duty=6, dutyLt=4
        { 826, 512,1159},	//duty=7, dutyLt=4
        { 842, 512,1138},	//duty=8, dutyLt=4
        { 855, 512,1116},	//duty=9, dutyLt=4
        { 868, 512,1100},	//duty=10, dutyLt=4
        { 882, 512,1088},	//duty=11, dutyLt=4
        { 889, 512,1073},	//duty=12, dutyLt=4
        { 899, 512,1065},	//duty=13, dutyLt=4
        { 909, 512,1055},	//duty=14, dutyLt=4
        { 914, 512,1042},	//duty=15, dutyLt=4
        { 928, 512,1025},	//duty=16, dutyLt=4
        { 940, 512,1011},	//duty=17, dutyLt=4
        { 950, 512, 998},	//duty=18, dutyLt=4
        { 959, 512, 987},	//duty=19, dutyLt=4
        { 966, 512, 976},	//duty=20, dutyLt=4
        { 973, 512, 967},	//duty=21, dutyLt=4
        { 978, 512, 959},	//duty=22, dutyLt=4
        { 983, 512, 951},	//duty=23, dutyLt=4
        { 988, 512, 943},	//duty=24, dutyLt=4
        { 992, 512, 937},	//duty=25, dutyLt=4
        { 996, 512, 930},	//duty=26, dutyLt=4
        { 512, 512, 512},	//duty=27, dutyLt=4
        { 510, 512,2370},	//duty=-1, dutyLt=5
        { 578, 512,1817},	//duty=0, dutyLt=5
        { 630, 512,1578},	//duty=1, dutyLt=5
        { 673, 512,1445},	//duty=2, dutyLt=5
        { 706, 512,1360},	//duty=3, dutyLt=5
        { 734, 512,1301},	//duty=4, dutyLt=5
        { 759, 512,1253},	//duty=5, dutyLt=5
        { 780, 512,1220},	//duty=6, dutyLt=5
        { 798, 512,1189},	//duty=7, dutyLt=5
        { 814, 512,1167},	//duty=8, dutyLt=5
        { 829, 512,1147},	//duty=9, dutyLt=5
        { 842, 512,1129},	//duty=10, dutyLt=5
        { 854, 512,1114},	//duty=11, dutyLt=5
        { 864, 512,1100},	//duty=12, dutyLt=5
        { 874, 512,1087},	//duty=13, dutyLt=5
        { 883, 512,1075},	//duty=14, dutyLt=5
        { 889, 512,1062},	//duty=15, dutyLt=5
        { 904, 512,1043},	//duty=16, dutyLt=5
        { 916, 512,1028},	//duty=17, dutyLt=5
        { 927, 512,1015},	//duty=18, dutyLt=5
        { 935, 512,1002},	//duty=19, dutyLt=5
        { 944, 512, 991},	//duty=20, dutyLt=5
        { 951, 512, 981},	//duty=21, dutyLt=5
        { 957, 512, 972},	//duty=22, dutyLt=5
        { 963, 512, 963},	//duty=23, dutyLt=5
        { 967, 512, 956},	//duty=24, dutyLt=5
        { 972, 512, 948},	//duty=25, dutyLt=5
        { 976, 512, 941},	//duty=26, dutyLt=5
        { 512, 512, 512},	//duty=27, dutyLt=5
        { 510, 512,2360},	//duty=-1, dutyLt=6
        { 570, 512,1857},	//duty=0, dutyLt=6
        { 619, 512,1608},	//duty=1, dutyLt=6
        { 656, 512,1487},	//duty=2, dutyLt=6
        { 688, 512,1398},	//duty=3, dutyLt=6
        { 714, 512,1336},	//duty=4, dutyLt=6
        { 738, 512,1287},	//duty=5, dutyLt=6
        { 759, 512,1249},	//duty=6, dutyLt=6
        { 776, 512,1220},	//duty=7, dutyLt=6
        { 792, 512,1193},	//duty=8, dutyLt=6
        { 806, 512,1172},	//duty=9, dutyLt=6
        { 820, 512,1153},	//duty=10, dutyLt=6
        { 831, 512,1136},	//duty=11, dutyLt=6
        { 842, 512,1121},	//duty=12, dutyLt=6
        { 852, 512,1106},	//duty=13, dutyLt=6
        { 861, 512,1095},	//duty=14, dutyLt=6
        { 869, 512,1084},	//duty=15, dutyLt=6
        { 883, 512,1061},	//duty=16, dutyLt=6
        { 896, 512,1044},	//duty=17, dutyLt=6
        { 906, 512,1030},	//duty=18, dutyLt=6
        { 916, 512,1017},	//duty=19, dutyLt=6
        { 924, 512,1005},	//duty=20, dutyLt=6
        { 932, 512, 994},	//duty=21, dutyLt=6
        { 938, 512, 985},	//duty=22, dutyLt=6
        { 944, 512, 976},	//duty=23, dutyLt=6
        { 950, 512, 968},	//duty=24, dutyLt=6
        { 954, 512, 960},	//duty=25, dutyLt=6
        { 512, 512, 512},	//duty=26, dutyLt=6
        { 512, 512, 512},	//duty=27, dutyLt=6
        { 510, 512,2347},	//duty=-1, dutyLt=7
        { 564, 512,1882},	//duty=0, dutyLt=7
        { 607, 512,1665},	//duty=1, dutyLt=7
        { 643, 512,1526},	//duty=2, dutyLt=7
        { 672, 512,1437},	//duty=3, dutyLt=7
        { 698, 512,1370},	//duty=4, dutyLt=7
        { 720, 512,1319},	//duty=5, dutyLt=7
        { 740, 512,1279},	//duty=6, dutyLt=7
        { 757, 512,1247},	//duty=7, dutyLt=7
        { 774, 512,1218},	//duty=8, dutyLt=7
        { 787, 512,1196},	//duty=9, dutyLt=7
        { 800, 512,1175},	//duty=10, dutyLt=7
        { 812, 512,1158},	//duty=11, dutyLt=7
        { 823, 512,1142},	//duty=12, dutyLt=7
        { 834, 512,1125},	//duty=13, dutyLt=7
        { 842, 512,1114},	//duty=14, dutyLt=7
        { 851, 512,1102},	//duty=15, dutyLt=7
        { 864, 512,1077},	//duty=16, dutyLt=7
        { 877, 512,1060},	//duty=17, dutyLt=7
        { 889, 512,1044},	//duty=18, dutyLt=7
        { 898, 512,1030},	//duty=19, dutyLt=7
        { 907, 512,1018},	//duty=20, dutyLt=7
        { 915, 512,1007},	//duty=21, dutyLt=7
        { 921, 512, 996},	//duty=22, dutyLt=7
        { 928, 512, 987},	//duty=23, dutyLt=7
        { 933, 512, 978},	//duty=24, dutyLt=7
        { 939, 512, 970},	//duty=25, dutyLt=7
        { 512, 512, 512},	//duty=26, dutyLt=7
        { 512, 512, 512},	//duty=27, dutyLt=7
        { 510, 512,2336},	//duty=-1, dutyLt=8
        { 561, 512,1892},	//duty=0, dutyLt=8
        { 599, 512,1689},	//duty=1, dutyLt=8
        { 631, 512,1560},	//duty=2, dutyLt=8
        { 660, 512,1467},	//duty=3, dutyLt=8
        { 685, 512,1398},	//duty=4, dutyLt=8
        { 706, 512,1345},	//duty=5, dutyLt=8
        { 725, 512,1304},	//duty=6, dutyLt=8
        { 742, 512,1270},	//duty=7, dutyLt=8
        { 757, 512,1242},	//duty=8, dutyLt=8
        { 773, 512,1216},	//duty=9, dutyLt=8
        { 784, 512,1196},	//duty=10, dutyLt=8
        { 795, 512,1172},	//duty=11, dutyLt=8
        { 807, 512,1160},	//duty=12, dutyLt=8
        { 816, 512,1145},	//duty=13, dutyLt=8
        { 826, 512,1131},	//duty=14, dutyLt=8
        { 834, 512,1119},	//duty=15, dutyLt=8
        { 849, 512,1092},	//duty=16, dutyLt=8
        { 862, 512,1074},	//duty=17, dutyLt=8
        { 873, 512,1057},	//duty=18, dutyLt=8
        { 883, 512,1044},	//duty=19, dutyLt=8
        { 892, 512,1030},	//duty=20, dutyLt=8
        { 899, 512,1018},	//duty=21, dutyLt=8
        { 907, 512,1008},	//duty=22, dutyLt=8
        { 913, 512, 998},	//duty=23, dutyLt=8
        { 919, 512, 989},	//duty=24, dutyLt=8
        { 512, 512, 512},	//duty=25, dutyLt=8
        { 512, 512, 512},	//duty=26, dutyLt=8
        { 512, 512, 512},	//duty=27, dutyLt=8
        { 510, 512,2323},	//duty=-1, dutyLt=9
        { 555, 512,1941},	//duty=0, dutyLt=9
        { 592, 512,1722},	//duty=1, dutyLt=9
        { 623, 512,1590},	//duty=2, dutyLt=9
        { 649, 512,1496},	//duty=3, dutyLt=9
        { 673, 512,1426},	//duty=4, dutyLt=9
        { 693, 512,1371},	//duty=5, dutyLt=9
        { 713, 512,1327},	//duty=6, dutyLt=9
        { 728, 512,1293},	//duty=7, dutyLt=9
        { 743, 512,1265},	//duty=8, dutyLt=9
        { 757, 512,1239},	//duty=9, dutyLt=9
        { 770, 512,1215},	//duty=10, dutyLt=9
        { 782, 512,1194},	//duty=11, dutyLt=9
        { 792, 512,1178},	//duty=12, dutyLt=9
        { 801, 512,1163},	//duty=13, dutyLt=9
        { 811, 512,1147},	//duty=14, dutyLt=9
        { 819, 512,1135},	//duty=15, dutyLt=9
        { 834, 512,1107},	//duty=16, dutyLt=9
        { 847, 512,1088},	//duty=17, dutyLt=9
        { 858, 512,1070},	//duty=18, dutyLt=9
        { 868, 512,1055},	//duty=19, dutyLt=9
        { 878, 512,1042},	//duty=20, dutyLt=9
        { 886, 512,1030},	//duty=21, dutyLt=9
        { 893, 512,1018},	//duty=22, dutyLt=9
        { 899, 512,1008},	//duty=23, dutyLt=9
        { 905, 512, 999},	//duty=24, dutyLt=9
        { 512, 512, 512},	//duty=25, dutyLt=9
        { 512, 512, 512},	//duty=26, dutyLt=9
        { 512, 512, 512},	//duty=27, dutyLt=9
        { 512, 512,2293},	//duty=-1, dutyLt=10
        { 553, 512,1955},	//duty=0, dutyLt=10
        { 587, 512,1741},	//duty=1, dutyLt=10
        { 617, 512,1609},	//duty=2, dutyLt=10
        { 641, 512,1520},	//duty=3, dutyLt=10
        { 663, 512,1450},	//duty=4, dutyLt=10
        { 683, 512,1394},	//duty=5, dutyLt=10
        { 702, 512,1346},	//duty=6, dutyLt=10
        { 717, 512,1314},	//duty=7, dutyLt=10
        { 731, 512,1283},	//duty=8, dutyLt=10
        { 745, 512,1257},	//duty=9, dutyLt=10
        { 757, 512,1232},	//duty=10, dutyLt=10
        { 769, 512,1211},	//duty=11, dutyLt=10
        { 779, 512,1190},	//duty=12, dutyLt=10
        { 788, 512,1178},	//duty=13, dutyLt=10
        { 797, 512,1163},	//duty=14, dutyLt=10
        { 806, 512,1149},	//duty=15, dutyLt=10
        { 821, 512,1120},	//duty=16, dutyLt=10
        { 834, 512,1100},	//duty=17, dutyLt=10
        { 846, 512,1082},	//duty=18, dutyLt=10
        { 856, 512,1066},	//duty=19, dutyLt=10
        { 865, 512,1053},	//duty=20, dutyLt=10
        { 873, 512,1040},	//duty=21, dutyLt=10
        { 880, 512,1028},	//duty=22, dutyLt=10
        { 887, 512,1018},	//duty=23, dutyLt=10
        { 512, 512, 512},	//duty=24, dutyLt=10
        { 512, 512, 512},	//duty=25, dutyLt=10
        { 512, 512, 512},	//duty=26, dutyLt=10
        { 512, 512, 512},	//duty=27, dutyLt=10
        { 511, 512,2337},	//duty=-1, dutyLt=11
        { 549, 512,1973},	//duty=0, dutyLt=11
        { 581, 512,1770},	//duty=1, dutyLt=11
        { 609, 512,1635},	//duty=2, dutyLt=11
        { 634, 512,1537},	//duty=3, dutyLt=11
        { 654, 512,1473},	//duty=4, dutyLt=11
        { 674, 512,1414},	//duty=5, dutyLt=11
        { 692, 512,1366},	//duty=6, dutyLt=11
        { 707, 512,1328},	//duty=7, dutyLt=11
        { 721, 512,1300},	//duty=8, dutyLt=11
        { 734, 512,1274},	//duty=9, dutyLt=11
        { 746, 512,1249},	//duty=10, dutyLt=11
        { 757, 512,1224},	//duty=11, dutyLt=11
        { 767, 512,1205},	//duty=12, dutyLt=11
        { 777, 512,1188},	//duty=13, dutyLt=11
        { 785, 512,1178},	//duty=14, dutyLt=11
        { 794, 512,1164},	//duty=15, dutyLt=11
        { 809, 512,1134},	//duty=16, dutyLt=11
        { 822, 512,1112},	//duty=17, dutyLt=11
        { 834, 512,1094},	//duty=18, dutyLt=11
        { 844, 512,1078},	//duty=19, dutyLt=11
        { 853, 512,1063},	//duty=20, dutyLt=11
        { 862, 512,1050},	//duty=21, dutyLt=11
        { 869, 512,1038},	//duty=22, dutyLt=11
        { 876, 512,1027},	//duty=23, dutyLt=11
        { 512, 512, 512},	//duty=24, dutyLt=11
        { 512, 512, 512},	//duty=25, dutyLt=11
        { 512, 512, 512},	//duty=26, dutyLt=11
        { 512, 512, 512},	//duty=27, dutyLt=11
        { 511, 512,2325},	//duty=-1, dutyLt=12
        { 546, 512,1985},	//duty=0, dutyLt=12
        { 578, 512,1781},	//duty=1, dutyLt=12
        { 603, 512,1655},	//duty=2, dutyLt=12
        { 627, 512,1559},	//duty=3, dutyLt=12
        { 648, 512,1485},	//duty=4, dutyLt=12
        { 665, 512,1436},	//duty=5, dutyLt=12
        { 683, 512,1384},	//duty=6, dutyLt=12
        { 697, 512,1351},	//duty=7, dutyLt=12
        { 711, 512,1319},	//duty=8, dutyLt=12
        { 724, 512,1289},	//duty=9, dutyLt=12
        { 735, 512,1266},	//duty=10, dutyLt=12
        { 748, 512,1241},	//duty=11, dutyLt=12
        { 757, 512,1225},	//duty=12, dutyLt=12
        { 766, 512,1207},	//duty=13, dutyLt=12
        { 777, 512,1189},	//duty=14, dutyLt=12
        { 784, 512,1176},	//duty=15, dutyLt=12
        { 799, 512,1146},	//duty=16, dutyLt=12
        { 812, 512,1124},	//duty=17, dutyLt=12
        { 823, 512,1104},	//duty=18, dutyLt=12
        { 834, 512,1088},	//duty=19, dutyLt=12
        { 843, 512,1073},	//duty=20, dutyLt=12
        { 851, 512,1059},	//duty=21, dutyLt=12
        { 859, 512,1048},	//duty=22, dutyLt=12
        { 512, 512, 512},	//duty=23, dutyLt=12
        { 512, 512, 512},	//duty=24, dutyLt=12
        { 512, 512, 512},	//duty=25, dutyLt=12
        { 512, 512, 512},	//duty=26, dutyLt=12
        { 512, 512, 512},	//duty=27, dutyLt=12
        { 510, 512,2321},	//duty=-1, dutyLt=13
        { 546, 512,1988},	//duty=0, dutyLt=13
        { 573, 512,1808},	//duty=1, dutyLt=13
        { 598, 512,1676},	//duty=2, dutyLt=13
        { 620, 512,1581},	//duty=3, dutyLt=13
        { 641, 512,1508},	//duty=4, dutyLt=13
        { 658, 512,1454},	//duty=5, dutyLt=13
        { 674, 512,1407},	//duty=6, dutyLt=13
        { 689, 512,1369},	//duty=7, dutyLt=13
        { 702, 512,1336},	//duty=8, dutyLt=13
        { 715, 512,1306},	//duty=9, dutyLt=13
        { 727, 512,1280},	//duty=10, dutyLt=13
        { 738, 512,1252},	//duty=11, dutyLt=13
        { 748, 512,1238},	//duty=12, dutyLt=13
        { 757, 512,1220},	//duty=13, dutyLt=13
        { 765, 512,1204},	//duty=14, dutyLt=13
        { 774, 512,1188},	//duty=15, dutyLt=13
        { 790, 512,1156},	//duty=16, dutyLt=13
        { 802, 512,1134},	//duty=17, dutyLt=13
        { 814, 512,1115},	//duty=18, dutyLt=13
        { 824, 512,1098},	//duty=19, dutyLt=13
        { 833, 512,1082},	//duty=20, dutyLt=13
        { 842, 512,1069},	//duty=21, dutyLt=13
        { 849, 512,1056},	//duty=22, dutyLt=13
        { 512, 512, 512},	//duty=23, dutyLt=13
        { 512, 512, 512},	//duty=24, dutyLt=13
        { 512, 512, 512},	//duty=25, dutyLt=13
        { 512, 512, 512},	//duty=26, dutyLt=13
        { 512, 512, 512},	//duty=27, dutyLt=13
        { 510, 512,2316},	//duty=-1, dutyLt=14
        { 542, 512,2007},	//duty=0, dutyLt=14
        { 570, 512,1819},	//duty=1, dutyLt=14
        { 595, 512,1688},	//duty=2, dutyLt=14
        { 615, 512,1599},	//duty=3, dutyLt=14
        { 634, 512,1528},	//duty=4, dutyLt=14
        { 652, 512,1470},	//duty=5, dutyLt=14
        { 669, 512,1415},	//duty=6, dutyLt=14
        { 681, 512,1384},	//duty=7, dutyLt=14
        { 695, 512,1351},	//duty=8, dutyLt=14
        { 707, 512,1321},	//duty=9, dutyLt=14
        { 718, 512,1295},	//duty=10, dutyLt=14
        { 729, 512,1272},	//duty=11, dutyLt=14
        { 739, 512,1252},	//duty=12, dutyLt=14
        { 750, 512,1230},	//duty=13, dutyLt=14
        { 757, 512,1216},	//duty=14, dutyLt=14
        { 765, 512,1201},	//duty=15, dutyLt=14
        { 781, 512,1168},	//duty=16, dutyLt=14
        { 793, 512,1144},	//duty=17, dutyLt=14
        { 805, 512,1125},	//duty=18, dutyLt=14
        { 815, 512,1107},	//duty=19, dutyLt=14
        { 825, 512,1091},	//duty=20, dutyLt=14
        { 833, 512,1077},	//duty=21, dutyLt=14
        { 512, 512, 512},	//duty=22, dutyLt=14
        { 512, 512, 512},	//duty=23, dutyLt=14
        { 512, 512, 512},	//duty=24, dutyLt=14
        { 512, 512, 512},	//duty=25, dutyLt=14
        { 512, 512, 512},	//duty=26, dutyLt=14
        { 512, 512, 512},	//duty=27, dutyLt=14
        { 510, 512,2312},	//duty=-1, dutyLt=15
        { 541, 512,2017},	//duty=0, dutyLt=15
        { 567, 512,1832},	//duty=1, dutyLt=15
        { 590, 512,1709},	//duty=2, dutyLt=15
        { 611, 512,1614},	//duty=3, dutyLt=15
        { 629, 512,1543},	//duty=4, dutyLt=15
        { 646, 512,1485},	//duty=5, dutyLt=15
        { 663, 512,1430},	//duty=6, dutyLt=15
        { 675, 512,1399},	//duty=7, dutyLt=15
        { 688, 512,1364},	//duty=8, dutyLt=15
        { 700, 512,1335},	//duty=9, dutyLt=15
        { 711, 512,1307},	//duty=10, dutyLt=15
        { 723, 512,1282},	//duty=11, dutyLt=15
        { 732, 512,1263},	//duty=12, dutyLt=15
        { 740, 512,1245},	//duty=13, dutyLt=15
        { 749, 512,1228},	//duty=14, dutyLt=15
        { 758, 512,1211},	//duty=15, dutyLt=15
        { 773, 512,1178},	//duty=16, dutyLt=15
        { 786, 512,1154},	//duty=17, dutyLt=15
        { 797, 512,1134},	//duty=18, dutyLt=15
        { 807, 512,1116},	//duty=19, dutyLt=15
        { 816, 512,1100},	//duty=20, dutyLt=15
        { 825, 512,1086},	//duty=21, dutyLt=15
        { 512, 512, 512},	//duty=22, dutyLt=15
        { 512, 512, 512},	//duty=23, dutyLt=15
        { 512, 512, 512},	//duty=24, dutyLt=15
        { 512, 512, 512},	//duty=25, dutyLt=15
        { 512, 512, 512},	//duty=26, dutyLt=15
        { 512, 512, 512},	//duty=27, dutyLt=15
        { 512, 512,2284},	//duty=-1, dutyLt=16
        { 540, 512,2017},	//duty=0, dutyLt=16
        { 565, 512,1837},	//duty=1, dutyLt=16
        { 587, 512,1718},	//duty=2, dutyLt=16
        { 606, 512,1627},	//duty=3, dutyLt=16
        { 624, 512,1557},	//duty=4, dutyLt=16
        { 639, 512,1499},	//duty=5, dutyLt=16
        { 654, 512,1453},	//duty=6, dutyLt=16
        { 667, 512,1413},	//duty=7, dutyLt=16
        { 679, 512,1379},	//duty=8, dutyLt=16
        { 691, 512,1349},	//duty=9, dutyLt=16
        { 701, 512,1323},	//duty=10, dutyLt=16
        { 711, 512,1299},	//duty=11, dutyLt=16
        { 720, 512,1278},	//duty=12, dutyLt=16
        { 729, 512,1258},	//duty=13, dutyLt=16
        { 737, 512,1240},	//duty=14, dutyLt=16
        { 745, 512,1225},	//duty=15, dutyLt=16
        { 759, 512,1196},	//duty=16, dutyLt=16
        { 771, 512,1173},	//duty=17, dutyLt=16
        { 782, 512,1151},	//duty=18, dutyLt=16
        { 792, 512,1132},	//duty=19, dutyLt=16
        { 802, 512,1116},	//duty=20, dutyLt=16
        { 512, 512, 512},	//duty=21, dutyLt=16
        { 512, 512, 512},	//duty=22, dutyLt=16
        { 512, 512, 512},	//duty=23, dutyLt=16
        { 512, 512, 512},	//duty=24, dutyLt=16
        { 512, 512, 512},	//duty=25, dutyLt=16
        { 512, 512, 512},	//duty=26, dutyLt=16
        { 512, 512, 512},	//duty=27, dutyLt=16
        { 512, 512,2271},	//duty=-1, dutyLt=17
        { 538, 512,2024},	//duty=0, dutyLt=17
        { 562, 512,1854},	//duty=1, dutyLt=17
        { 582, 512,1737},	//duty=2, dutyLt=17
        { 600, 512,1649},	//duty=3, dutyLt=17
        { 617, 512,1579},	//duty=4, dutyLt=17
        { 632, 512,1522},	//duty=5, dutyLt=17
        { 645, 512,1475},	//duty=6, dutyLt=17
        { 658, 512,1435},	//duty=7, dutyLt=17
        { 670, 512,1400},	//duty=8, dutyLt=17
        { 680, 512,1370},	//duty=9, dutyLt=17
        { 691, 512,1343},	//duty=10, dutyLt=17
        { 700, 512,1319},	//duty=11, dutyLt=17
        { 709, 512,1297},	//duty=12, dutyLt=17
        { 718, 512,1277},	//duty=13, dutyLt=17
        { 726, 512,1259},	//duty=14, dutyLt=17
        { 733, 512,1242},	//duty=15, dutyLt=17
        { 747, 512,1214},	//duty=16, dutyLt=17
        { 759, 512,1190},	//duty=17, dutyLt=17
        { 770, 512,1167},	//duty=18, dutyLt=17
        { 780, 512,1147},	//duty=19, dutyLt=17
        { 512, 512, 512},	//duty=20, dutyLt=17
        { 512, 512, 512},	//duty=21, dutyLt=17
        { 512, 512, 512},	//duty=22, dutyLt=17
        { 512, 512, 512},	//duty=23, dutyLt=17
        { 512, 512, 512},	//duty=24, dutyLt=17
        { 512, 512, 512},	//duty=25, dutyLt=17
        { 512, 512, 512},	//duty=26, dutyLt=17
        { 512, 512, 512},	//duty=27, dutyLt=17
        { 512, 512,2263},	//duty=-1, dutyLt=18
        { 537, 512,2028},	//duty=0, dutyLt=18
        { 559, 512,1868},	//duty=1, dutyLt=18
        { 578, 512,1754},	//duty=2, dutyLt=18
        { 595, 512,1668},	//duty=3, dutyLt=18
        { 611, 512,1598},	//duty=4, dutyLt=18
        { 625, 512,1540},	//duty=5, dutyLt=18
        { 638, 512,1495},	//duty=6, dutyLt=18
        { 651, 512,1453},	//duty=7, dutyLt=18
        { 662, 512,1419},	//duty=8, dutyLt=18
        { 672, 512,1387},	//duty=9, dutyLt=18
        { 682, 512,1360},	//duty=10, dutyLt=18
        { 691, 512,1335},	//duty=11, dutyLt=18
        { 700, 512,1314},	//duty=12, dutyLt=18
        { 708, 512,1293},	//duty=13, dutyLt=18
        { 716, 512,1275},	//duty=14, dutyLt=18
        { 723, 512,1258},	//duty=15, dutyLt=18
        { 736, 512,1229},	//duty=16, dutyLt=18
        { 749, 512,1203},	//duty=17, dutyLt=18
        { 760, 512,1181},	//duty=18, dutyLt=18
        { 512, 512, 512},	//duty=19, dutyLt=18
        { 512, 512, 512},	//duty=20, dutyLt=18
        { 512, 512, 512},	//duty=21, dutyLt=18
        { 512, 512, 512},	//duty=22, dutyLt=18
        { 512, 512, 512},	//duty=23, dutyLt=18
        { 512, 512, 512},	//duty=24, dutyLt=18
        { 512, 512, 512},	//duty=25, dutyLt=18
        { 512, 512, 512},	//duty=26, dutyLt=18
        { 512, 512, 512},	//duty=27, dutyLt=18
        { 513, 512,2252},	//duty=-1, dutyLt=19
        { 535, 512,2034},	//duty=0, dutyLt=19
        { 557, 512,1876},	//duty=1, dutyLt=19
        { 575, 512,1767},	//duty=2, dutyLt=19
        { 590, 512,1683},	//duty=3, dutyLt=19
        { 606, 512,1615},	//duty=4, dutyLt=19
        { 619, 512,1560},	//duty=5, dutyLt=19
        { 632, 512,1511},	//duty=6, dutyLt=19
        { 644, 512,1471},	//duty=7, dutyLt=19
        { 655, 512,1434},	//duty=8, dutyLt=19
        { 665, 512,1404},	//duty=9, dutyLt=19
        { 675, 512,1376},	//duty=10, dutyLt=19
        { 684, 512,1351},	//duty=11, dutyLt=19
        { 692, 512,1328},	//duty=12, dutyLt=19
        { 700, 512,1307},	//duty=13, dutyLt=19
        { 708, 512,1290},	//duty=14, dutyLt=19
        { 715, 512,1272},	//duty=15, dutyLt=19
        { 728, 512,1243},	//duty=16, dutyLt=19
        { 740, 512,1216},	//duty=17, dutyLt=19
        { 512, 512, 512},	//duty=18, dutyLt=19
        { 512, 512, 512},	//duty=19, dutyLt=19
        { 512, 512, 512},	//duty=20, dutyLt=19
        { 512, 512, 512},	//duty=21, dutyLt=19
        { 512, 512, 512},	//duty=22, dutyLt=19
        { 512, 512, 512},	//duty=23, dutyLt=19
        { 512, 512, 512},	//duty=24, dutyLt=19
        { 512, 512, 512},	//duty=25, dutyLt=19
        { 512, 512, 512},	//duty=26, dutyLt=19
        { 512, 512, 512},	//duty=27, dutyLt=19
        { 513, 512,2241},	//duty=-1, dutyLt=20
        { 534, 512,2037},	//duty=0, dutyLt=20
        { 554, 512,1885},	//duty=1, dutyLt=20
        { 572, 512,1780},	//duty=2, dutyLt=20
        { 588, 512,1696},	//duty=3, dutyLt=20
        { 602, 512,1627},	//duty=4, dutyLt=20
        { 615, 512,1573},	//duty=5, dutyLt=20
        { 627, 512,1526},	//duty=6, dutyLt=20
        { 639, 512,1484},	//duty=7, dutyLt=20
        { 649, 512,1449},	//duty=8, dutyLt=20
        { 659, 512,1418},	//duty=9, dutyLt=20
        { 668, 512,1389},	//duty=10, dutyLt=20
        { 677, 512,1365},	//duty=11, dutyLt=20
        { 685, 512,1342},	//duty=12, dutyLt=20
        { 693, 512,1321},	//duty=13, dutyLt=20
        { 701, 512,1302},	//duty=14, dutyLt=20
        { 707, 512,1285},	//duty=15, dutyLt=20
        { 720, 512,1254},	//duty=16, dutyLt=20
        { 512, 512, 512},	//duty=17, dutyLt=20
        { 512, 512, 512},	//duty=18, dutyLt=20
        { 512, 512, 512},	//duty=19, dutyLt=20
        { 512, 512, 512},	//duty=20, dutyLt=20
        { 512, 512, 512},	//duty=21, dutyLt=20
        { 512, 512, 512},	//duty=22, dutyLt=20
        { 512, 512, 512},	//duty=23, dutyLt=20
        { 512, 512, 512},	//duty=24, dutyLt=20
        { 512, 512, 512},	//duty=25, dutyLt=20
        { 512, 512, 512},	//duty=26, dutyLt=20
        { 512, 512, 512},	//duty=27, dutyLt=20
        { 513, 512,2234},	//duty=-1, dutyLt=21
        { 534, 512,2036},	//duty=0, dutyLt=21
        { 553, 512,1892},	//duty=1, dutyLt=21
        { 570, 512,1788},	//duty=2, dutyLt=21
        { 585, 512,1707},	//duty=3, dutyLt=21
        { 599, 512,1639},	//duty=4, dutyLt=21
        { 611, 512,1584},	//duty=5, dutyLt=21
        { 623, 512,1538},	//duty=6, dutyLt=21
        { 634, 512,1497},	//duty=7, dutyLt=21
        { 645, 512,1461},	//duty=8, dutyLt=21
        { 654, 512,1431},	//duty=9, dutyLt=21
        { 663, 512,1401},	//duty=10, dutyLt=21
        { 671, 512,1377},	//duty=11, dutyLt=21
        { 679, 512,1354},	//duty=12, dutyLt=21
        { 687, 512,1333},	//duty=13, dutyLt=21
        { 694, 512,1314},	//duty=14, dutyLt=21
        { 701, 512,1296},	//duty=15, dutyLt=21
        { 512, 512, 512},	//duty=16, dutyLt=21
        { 512, 512, 512},	//duty=17, dutyLt=21
        { 512, 512, 512},	//duty=18, dutyLt=21
        { 512, 512, 512},	//duty=19, dutyLt=21
        { 512, 512, 512},	//duty=20, dutyLt=21
        { 512, 512, 512},	//duty=21, dutyLt=21
        { 512, 512, 512},	//duty=22, dutyLt=21
        { 512, 512, 512},	//duty=23, dutyLt=21
        { 512, 512, 512},	//duty=24, dutyLt=21
        { 512, 512, 512},	//duty=25, dutyLt=21
        { 512, 512, 512},	//duty=26, dutyLt=21
        { 512, 512, 512},	//duty=27, dutyLt=21
        { 514, 512,2224},	//duty=-1, dutyLt=22
        { 533, 512,2035},	//duty=0, dutyLt=22
        { 552, 512,1897},	//duty=1, dutyLt=22
        { 568, 512,1796},	//duty=2, dutyLt=22
        { 583, 512,1717},	//duty=3, dutyLt=22
        { 596, 512,1650},	//duty=4, dutyLt=22
        { 608, 512,1594},	//duty=5, dutyLt=22
        { 620, 512,1548},	//duty=6, dutyLt=22
        { 630, 512,1507},	//duty=7, dutyLt=22
        { 640, 512,1474},	//duty=8, dutyLt=22
        { 650, 512,1441},	//duty=9, dutyLt=22
        { 658, 512,1413},	//duty=10, dutyLt=22
        { 666, 512,1389},	//duty=11, dutyLt=22
        { 674, 512,1366},	//duty=12, dutyLt=22
        { 682, 512,1343},	//duty=13, dutyLt=22
        { 512, 512, 512},	//duty=14, dutyLt=22
        { 512, 512, 512},	//duty=15, dutyLt=22
        { 512, 512, 512},	//duty=16, dutyLt=22
        { 512, 512, 512},	//duty=17, dutyLt=22
        { 512, 512, 512},	//duty=18, dutyLt=22
        { 512, 512, 512},	//duty=19, dutyLt=22
        { 512, 512, 512},	//duty=20, dutyLt=22
        { 512, 512, 512},	//duty=21, dutyLt=22
        { 512, 512, 512},	//duty=22, dutyLt=22
        { 512, 512, 512},	//duty=23, dutyLt=22
        { 512, 512, 512},	//duty=24, dutyLt=22
        { 512, 512, 512},	//duty=25, dutyLt=22
        { 512, 512, 512},	//duty=26, dutyLt=22
        { 512, 512, 512},	//duty=27, dutyLt=22
        { 514, 512,2216},	//duty=-1, dutyLt=23
        { 533, 512,2037},	//duty=0, dutyLt=23
        { 551, 512,1901},	//duty=1, dutyLt=23
        { 566, 512,1801},	//duty=2, dutyLt=23
        { 581, 512,1725},	//duty=3, dutyLt=23
        { 593, 512,1659},	//duty=4, dutyLt=23
        { 606, 512,1603},	//duty=5, dutyLt=23
        { 617, 512,1557},	//duty=6, dutyLt=23
        { 627, 512,1517},	//duty=7, dutyLt=23
        { 636, 512,1483},	//duty=8, dutyLt=23
        { 646, 512,1450},	//duty=9, dutyLt=23
        { 654, 512,1422},	//duty=10, dutyLt=23
        { 663, 512,1396},	//duty=11, dutyLt=23
        { 512, 512, 512},	//duty=12, dutyLt=23
        { 512, 512, 512},	//duty=13, dutyLt=23
        { 512, 512, 512},	//duty=14, dutyLt=23
        { 512, 512, 512},	//duty=15, dutyLt=23
        { 512, 512, 512},	//duty=16, dutyLt=23
        { 512, 512, 512},	//duty=17, dutyLt=23
        { 512, 512, 512},	//duty=18, dutyLt=23
        { 512, 512, 512},	//duty=19, dutyLt=23
        { 512, 512, 512},	//duty=20, dutyLt=23
        { 512, 512, 512},	//duty=21, dutyLt=23
        { 512, 512, 512},	//duty=22, dutyLt=23
        { 512, 512, 512},	//duty=23, dutyLt=23
        { 512, 512, 512},	//duty=24, dutyLt=23
        { 512, 512, 512},	//duty=25, dutyLt=23
        { 512, 512, 512},	//duty=26, dutyLt=23
        { 512, 512, 512},	//duty=27, dutyLt=23
        { 515, 512,2207},	//duty=-1, dutyLt=24
        { 533, 512,2037},	//duty=0, dutyLt=24
        { 550, 512,1905},	//duty=1, dutyLt=24
        { 565, 512,1807},	//duty=2, dutyLt=24
        { 579, 512,1731},	//duty=3, dutyLt=24
        { 592, 512,1666},	//duty=4, dutyLt=24
        { 603, 512,1612},	//duty=5, dutyLt=24
        { 614, 512,1566},	//duty=6, dutyLt=24
        { 624, 512,1525},	//duty=7, dutyLt=24
        { 634, 512,1490},	//duty=8, dutyLt=24
        { 643, 512,1459},	//duty=9, dutyLt=24
        { 512, 512, 512},	//duty=10, dutyLt=24
        { 512, 512, 512},	//duty=11, dutyLt=24
        { 512, 512, 512},	//duty=12, dutyLt=24
        { 512, 512, 512},	//duty=13, dutyLt=24
        { 512, 512, 512},	//duty=14, dutyLt=24
        { 512, 512, 512},	//duty=15, dutyLt=24
        { 512, 512, 512},	//duty=16, dutyLt=24
        { 512, 512, 512},	//duty=17, dutyLt=24
        { 512, 512, 512},	//duty=18, dutyLt=24
        { 512, 512, 512},	//duty=19, dutyLt=24
        { 512, 512, 512},	//duty=20, dutyLt=24
        { 512, 512, 512},	//duty=21, dutyLt=24
        { 512, 512, 512},	//duty=22, dutyLt=24
        { 512, 512, 512},	//duty=23, dutyLt=24
        { 512, 512, 512},	//duty=24, dutyLt=24
        { 512, 512, 512},	//duty=25, dutyLt=24
        { 512, 512, 512},	//duty=26, dutyLt=24
        { 512, 512, 512},	//duty=27, dutyLt=24
        { 515, 512,2201},	//duty=-1, dutyLt=25
        { 533, 512,2035},	//duty=0, dutyLt=25
        { 550, 512,1907},	//duty=1, dutyLt=25
        { 564, 512,1811},	//duty=2, dutyLt=25
        { 577, 512,1737},	//duty=3, dutyLt=25
        { 590, 512,1674},	//duty=4, dutyLt=25
        { 601, 512,1618},	//duty=5, dutyLt=25
        { 612, 512,1573},	//duty=6, dutyLt=25
        { 622, 512,1532},	//duty=7, dutyLt=25
        { 512, 512, 512},	//duty=8, dutyLt=25
        { 512, 512, 512},	//duty=9, dutyLt=25
        { 512, 512, 512},	//duty=10, dutyLt=25
        { 512, 512, 512},	//duty=11, dutyLt=25
        { 512, 512, 512},	//duty=12, dutyLt=25
        { 512, 512, 512},	//duty=13, dutyLt=25
        { 512, 512, 512},	//duty=14, dutyLt=25
        { 512, 512, 512},	//duty=15, dutyLt=25
        { 512, 512, 512},	//duty=16, dutyLt=25
        { 512, 512, 512},	//duty=17, dutyLt=25
        { 512, 512, 512},	//duty=18, dutyLt=25
        { 512, 512, 512},	//duty=19, dutyLt=25
        { 512, 512, 512},	//duty=20, dutyLt=25
        { 512, 512, 512},	//duty=21, dutyLt=25
        { 512, 512, 512},	//duty=22, dutyLt=25
        { 512, 512, 512},	//duty=23, dutyLt=25
        { 512, 512, 512},	//duty=24, dutyLt=25
        { 512, 512, 512},	//duty=25, dutyLt=25
        { 512, 512, 512},	//duty=26, dutyLt=25
        { 512, 512, 512},	//duty=27, dutyLt=25
        { 515, 512,2197},	//duty=-1, dutyLt=26
        { 533, 512,2034},	//duty=0, dutyLt=26
        { 549, 512,1909},	//duty=1, dutyLt=26
        { 563, 512,1816},	//duty=2, dutyLt=26
        { 576, 512,1741},	//duty=3, dutyLt=26
        { 589, 512,1676},	//duty=4, dutyLt=26
        { 600, 512,1623},	//duty=5, dutyLt=26
        { 512, 512, 512},	//duty=6, dutyLt=26
        { 512, 512, 512},	//duty=7, dutyLt=26
        { 512, 512, 512},	//duty=8, dutyLt=26
        { 512, 512, 512},	//duty=9, dutyLt=26
        { 512, 512, 512},	//duty=10, dutyLt=26
        { 512, 512, 512},	//duty=11, dutyLt=26
        { 512, 512, 512},	//duty=12, dutyLt=26
        { 512, 512, 512},	//duty=13, dutyLt=26
        { 512, 512, 512},	//duty=14, dutyLt=26
        { 512, 512, 512},	//duty=15, dutyLt=26
        { 512, 512, 512},	//duty=16, dutyLt=26
        { 512, 512, 512},	//duty=17, dutyLt=26
        { 512, 512, 512},	//duty=18, dutyLt=26
        { 512, 512, 512},	//duty=19, dutyLt=26
        { 512, 512, 512},	//duty=20, dutyLt=26
        { 512, 512, 512},	//duty=21, dutyLt=26
        { 512, 512, 512},	//duty=22, dutyLt=26
        { 512, 512, 512},	//duty=23, dutyLt=26
        { 512, 512, 512},	//duty=24, dutyLt=26
        { 512, 512, 512},	//duty=25, dutyLt=26
        { 512, 512, 512},	//duty=26, dutyLt=26
        { 512, 512, 512},	//duty=27, dutyLt=26
        { 516, 512,2187},	//duty=-1, dutyLt=27
        { 533, 512,2031},	//duty=0, dutyLt=27
        { 549, 512,1909},	//duty=1, dutyLt=27
        { 563, 512,1817},	//duty=2, dutyLt=27
        { 576, 512,1744},	//duty=3, dutyLt=27
        { 512, 512, 512},	//duty=4, dutyLt=27
        { 512, 512, 512},	//duty=5, dutyLt=27
        { 512, 512, 512},	//duty=6, dutyLt=27
        { 512, 512, 512},	//duty=7, dutyLt=27
        { 512, 512, 512},	//duty=8, dutyLt=27
        { 512, 512, 512},	//duty=9, dutyLt=27
        { 512, 512, 512},	//duty=10, dutyLt=27
        { 512, 512, 512},	//duty=11, dutyLt=27
        { 512, 512, 512},	//duty=12, dutyLt=27
        { 512, 512, 512},	//duty=13, dutyLt=27
        { 512, 512, 512},	//duty=14, dutyLt=27
        { 512, 512, 512},	//duty=15, dutyLt=27
        { 512, 512, 512},	//duty=16, dutyLt=27
        { 512, 512, 512},	//duty=17, dutyLt=27
        { 512, 512, 512},	//duty=18, dutyLt=27
        { 512, 512, 512},	//duty=19, dutyLt=27
        { 512, 512, 512},	//duty=20, dutyLt=27
        { 512, 512, 512},	//duty=21, dutyLt=27
        { 512, 512, 512},	//duty=22, dutyLt=27
        { 512, 512, 512},	//duty=23, dutyLt=27
        { 512, 512, 512},	//duty=24, dutyLt=27
        { 512, 512, 512},	//duty=25, dutyLt=27
        { 512, 512, 512},	//duty=26, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512},	//duty=27, dutyLt=27
        { 512, 512, 512}
    }
}

