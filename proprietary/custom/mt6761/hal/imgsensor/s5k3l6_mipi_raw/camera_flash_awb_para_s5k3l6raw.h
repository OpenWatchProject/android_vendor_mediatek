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
{{
    { 512, 512, 512},  //duty=-1, dutyLt=-1
    {1153, 512, 928},  //duty=0, dutyLt=-1
    {1145, 512, 917},  //duty=1, dutyLt=-1
    {1142, 512, 912},  //duty=2, dutyLt=-1
    {1146, 512, 912},  //duty=3, dutyLt=-1
    {1149, 512, 911},  //duty=4, dutyLt=-1
    {1150, 512, 906},  //duty=5, dutyLt=-1
    {1150, 512, 899},  //duty=6, dutyLt=-1
    {1148, 512, 898},  //duty=7, dutyLt=-1
    {1146, 512, 894},  //duty=8, dutyLt=-1
    {1144, 512, 891},  //duty=9, dutyLt=-1
    {1143, 512, 889},  //duty=10, dutyLt=-1
    {1144, 512, 886},  //duty=11, dutyLt=-1
    {1143, 512, 884},  //duty=12, dutyLt=-1
    {1143, 512, 881},  //duty=13, dutyLt=-1
    {1142, 512, 878},  //duty=14, dutyLt=-1
    {1142, 512, 877},  //duty=15, dutyLt=-1
    {1142, 512, 873},  //duty=16, dutyLt=-1
    {1140, 512, 869},  //duty=17, dutyLt=-1
    {1140, 512, 865},  //duty=18, dutyLt=-1
    {1139, 512, 861},  //duty=19, dutyLt=-1
    {1138, 512, 858},  //duty=20, dutyLt=-1
    {1138, 512, 854},  //duty=21, dutyLt=-1
    {1138, 512, 851},  //duty=22, dutyLt=-1
    {1138, 512, 848},  //duty=23, dutyLt=-1
    {1138, 512, 845},  //duty=24, dutyLt=-1
    {1138, 512, 842},  //duty=25, dutyLt=-1
    {1138, 512, 839},  //duty=26, dutyLt=-1
    {1138, 512, 837},  //duty=27, dutyLt=-1
    { 584, 512,1948},  //duty=-1, dutyLt=0
    { 816, 512,1183},  //duty=0, dutyLt=0
    { 906, 512,1069},  //duty=1, dutyLt=0
    { 959, 512,1026},  //duty=2, dutyLt=0
    { 995, 512,1001},  //duty=3, dutyLt=0
    {1017, 512, 983},  //duty=4, dutyLt=0
    {1034, 512, 964},  //duty=5, dutyLt=0
    {1043, 512, 950},  //duty=6, dutyLt=0
    {1052, 512, 942},  //duty=7, dutyLt=0
    {1058, 512, 936},  //duty=8, dutyLt=0
    {1063, 512, 929},  //duty=9, dutyLt=0
    {1068, 512, 924},  //duty=10, dutyLt=0
    {1073, 512, 919},  //duty=11, dutyLt=0
    {1076, 512, 914},  //duty=12, dutyLt=0
    {1079, 512, 910},  //duty=13, dutyLt=0
    {1081, 512, 907},  //duty=14, dutyLt=0
    {1084, 512, 904},  //duty=15, dutyLt=0
    {1089, 512, 898},  //duty=16, dutyLt=0
    {1092, 512, 892},  //duty=17, dutyLt=0
    {1095, 512, 887},  //duty=18, dutyLt=0
    {1097, 512, 883},  //duty=19, dutyLt=0
    {1099, 512, 878},  //duty=20, dutyLt=0
    {1100, 512, 874},  //duty=21, dutyLt=0
    {1102, 512, 870},  //duty=22, dutyLt=0
    {1103, 512, 867},  //duty=23, dutyLt=0
    {1104, 512, 862},  //duty=24, dutyLt=0
    {1105, 512, 859},  //duty=25, dutyLt=0
    {1107, 512, 856},  //duty=26, dutyLt=0
    {1108, 512, 852},  //duty=27, dutyLt=0
    { 583, 512,1949},  //duty=-1, dutyLt=1
    { 727, 512,1321},  //duty=0, dutyLt=1
    { 811, 512,1170},  //duty=1, dutyLt=1
    { 871, 512,1115},  //duty=2, dutyLt=1
    { 910, 512,1074},  //duty=3, dutyLt=1
    { 938, 512,1043},  //duty=4, dutyLt=1
    { 950, 512,1001},  //duty=5, dutyLt=1
    { 966, 512, 986},  //duty=6, dutyLt=1
    { 979, 512, 974},  //duty=7, dutyLt=1
    { 991, 512, 963},  //duty=8, dutyLt=1
    {1005, 512, 961},  //duty=9, dutyLt=1
    {1014, 512, 955},  //duty=10, dutyLt=1
    {1021, 512, 949},  //duty=11, dutyLt=1
    {1027, 512, 943},  //duty=12, dutyLt=1
    {1031, 512, 938},  //duty=13, dutyLt=1
    {1036, 512, 933},  //duty=14, dutyLt=1
    {1040, 512, 928},  //duty=15, dutyLt=1
    {1048, 512, 921},  //duty=16, dutyLt=1
    {1053, 512, 914},  //duty=17, dutyLt=1
    {1058, 512, 908},  //duty=18, dutyLt=1
    {1062, 512, 901},  //duty=19, dutyLt=1
    {1065, 512, 896},  //duty=20, dutyLt=1
    {1068, 512, 891},  //duty=21, dutyLt=1
    {1071, 512, 886},  //duty=22, dutyLt=1
    {1073, 512, 882},  //duty=23, dutyLt=1
    {1075, 512, 877},  //duty=24, dutyLt=1
    {1078, 512, 874},  //duty=25, dutyLt=1
    {1080, 512, 869},  //duty=26, dutyLt=1
    {1082, 512, 866},  //duty=27, dutyLt=1
    { 581, 512,1931},  //duty=-1, dutyLt=2
    { 693, 512,1429},  //duty=0, dutyLt=2
    { 763, 512,1254},  //duty=1, dutyLt=2
    { 818, 512,1184},  //duty=2, dutyLt=2
    { 855, 512,1131},  //duty=3, dutyLt=2
    { 878, 512,1070},  //duty=4, dutyLt=2
    { 899, 512,1045},  //duty=5, dutyLt=2
    { 917, 512,1025},  //duty=6, dutyLt=2
    { 933, 512,1011},  //duty=7, dutyLt=2
    { 946, 512, 997},  //duty=8, dutyLt=2
    { 957, 512, 986},  //duty=9, dutyLt=2
    { 966, 512, 977},  //duty=10, dutyLt=2
    { 974, 512, 968},  //duty=11, dutyLt=2
    { 981, 512, 961},  //duty=12, dutyLt=2
    { 988, 512, 954},  //duty=13, dutyLt=2
    { 999, 512, 954},  //duty=14, dutyLt=2
    {1005, 512, 951},  //duty=15, dutyLt=2
    {1015, 512, 941},  //duty=16, dutyLt=2
    {1022, 512, 933},  //duty=17, dutyLt=2
    {1028, 512, 925},  //duty=18, dutyLt=2
    {1034, 512, 918},  //duty=19, dutyLt=2
    {1039, 512, 911},  //duty=20, dutyLt=2
    {1042, 512, 905},  //duty=21, dutyLt=2
    {1046, 512, 900},  //duty=22, dutyLt=2
    {1049, 512, 895},  //duty=23, dutyLt=2
    {1052, 512, 891},  //duty=24, dutyLt=2
    {1056, 512, 886},  //duty=25, dutyLt=2
    {1058, 512, 882},  //duty=26, dutyLt=2
    {1060, 512, 877},  //duty=27, dutyLt=2
    { 582, 512,1925},  //duty=-1, dutyLt=3
    { 673, 512,1493},  //duty=0, dutyLt=3
    { 736, 512,1328},  //duty=1, dutyLt=3
    { 778, 512,1225},  //duty=2, dutyLt=3
    { 821, 512,1165},  //duty=3, dutyLt=3
    { 840, 512,1115},  //duty=4, dutyLt=3
    { 862, 512,1085},  //duty=5, dutyLt=3
    { 881, 512,1062},  //duty=6, dutyLt=3
    { 896, 512,1043},  //duty=7, dutyLt=3
    { 910, 512,1028},  //duty=8, dutyLt=3
    { 922, 512,1015},  //duty=9, dutyLt=3
    { 933, 512,1003},  //duty=10, dutyLt=3
    { 942, 512, 993},  //duty=11, dutyLt=3
    { 950, 512, 985},  //duty=12, dutyLt=3
    { 957, 512, 977},  //duty=13, dutyLt=3
    { 964, 512, 970},  //duty=14, dutyLt=3
    { 970, 512, 963},  //duty=15, dutyLt=3
    { 981, 512, 952},  //duty=16, dutyLt=3
    { 989, 512, 942},  //duty=17, dutyLt=3
    {1003, 512, 941},  //duty=18, dutyLt=3
    {1009, 512, 933},  //duty=19, dutyLt=3
    {1014, 512, 926},  //duty=20, dutyLt=3
    {1020, 512, 919},  //duty=21, dutyLt=3
    {1024, 512, 913},  //duty=22, dutyLt=3
    {1028, 512, 909},  //duty=23, dutyLt=3
    {1032, 512, 902},  //duty=24, dutyLt=3
    {1035, 512, 897},  //duty=25, dutyLt=3
    {1037, 512, 889},  //duty=26, dutyLt=3
    {1041, 512, 888},  //duty=27, dutyLt=3
    { 583, 512,1917},  //duty=-1, dutyLt=4
    { 658, 512,1544},  //duty=0, dutyLt=4
    { 714, 512,1377},  //duty=1, dutyLt=4
    { 758, 512,1276},  //duty=2, dutyLt=4
    { 788, 512,1208},  //duty=3, dutyLt=4
    { 811, 512,1152},  //duty=4, dutyLt=4
    { 833, 512,1120},  //duty=5, dutyLt=4
    { 852, 512,1094},  //duty=6, dutyLt=4
    { 868, 512,1072},  //duty=7, dutyLt=4
    { 882, 512,1056},  //duty=8, dutyLt=4
    { 894, 512,1040},  //duty=9, dutyLt=4
    { 905, 512,1027},  //duty=10, dutyLt=4
    { 916, 512,1016},  //duty=11, dutyLt=4
    { 924, 512,1006},  //duty=12, dutyLt=4
    { 932, 512, 998},  //duty=13, dutyLt=4
    { 939, 512, 989},  //duty=14, dutyLt=4
    { 945, 512, 981},  //duty=15, dutyLt=4
    { 958, 512, 969},  //duty=16, dutyLt=4
    { 967, 512, 957},  //duty=17, dutyLt=4
    { 975, 512, 947},  //duty=18, dutyLt=4
    { 982, 512, 938},  //duty=19, dutyLt=4
    { 994, 512, 940},  //duty=20, dutyLt=4
    {1000, 512, 932},  //duty=21, dutyLt=4
    {1005, 512, 925},  //duty=22, dutyLt=4
    {1009, 512, 919},  //duty=23, dutyLt=4
    {1014, 512, 913},  //duty=24, dutyLt=4
    {1018, 512, 908},  //duty=25, dutyLt=4
    {1021, 512, 903},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 583, 512,1913},  //duty=-1, dutyLt=5
    { 648, 512,1577},  //duty=0, dutyLt=5
    { 699, 512,1414},  //duty=1, dutyLt=5
    { 735, 512,1310},  //duty=2, dutyLt=5
    { 767, 512,1246},  //duty=3, dutyLt=5
    { 792, 512,1199},  //duty=4, dutyLt=5
    { 811, 512,1152},  //duty=5, dutyLt=5
    { 829, 512,1123},  //duty=6, dutyLt=5
    { 845, 512,1100},  //duty=7, dutyLt=5
    { 859, 512,1080},  //duty=8, dutyLt=5
    { 872, 512,1064},  //duty=9, dutyLt=5
    { 883, 512,1050},  //duty=10, dutyLt=5
    { 893, 512,1037},  //duty=11, dutyLt=5
    { 902, 512,1026},  //duty=12, dutyLt=5
    { 910, 512,1016},  //duty=13, dutyLt=5
    { 917, 512,1007},  //duty=14, dutyLt=5
    { 924, 512, 999},  //duty=15, dutyLt=5
    { 937, 512, 985},  //duty=16, dutyLt=5
    { 947, 512, 972},  //duty=17, dutyLt=5
    { 955, 512, 961},  //duty=18, dutyLt=5
    { 964, 512, 951},  //duty=19, dutyLt=5
    { 971, 512, 944},  //duty=20, dutyLt=5
    { 977, 512, 936},  //duty=21, dutyLt=5
    { 987, 512, 933},  //duty=22, dutyLt=5
    { 992, 512, 930},  //duty=23, dutyLt=5
    { 997, 512, 924},  //duty=24, dutyLt=5
    {1001, 512, 918},  //duty=25, dutyLt=5
    {1005, 512, 913},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 583, 512,1914},  //duty=-1, dutyLt=6
    { 641, 512,1608},  //duty=0, dutyLt=6
    { 690, 512,1434},  //duty=1, dutyLt=6
    { 721, 512,1342},  //duty=2, dutyLt=6
    { 750, 512,1278},  //duty=3, dutyLt=6
    { 774, 512,1230},  //duty=4, dutyLt=6
    { 792, 512,1179},  //duty=5, dutyLt=6
    { 810, 512,1149},  //duty=6, dutyLt=6
    { 826, 512,1124},  //duty=7, dutyLt=6
    { 840, 512,1104},  //duty=8, dutyLt=6
    { 853, 512,1086},  //duty=9, dutyLt=6
    { 864, 512,1070},  //duty=10, dutyLt=6
    { 874, 512,1057},  //duty=11, dutyLt=6
    { 883, 512,1045},  //duty=12, dutyLt=6
    { 892, 512,1035},  //duty=13, dutyLt=6
    { 899, 512,1024},  //duty=14, dutyLt=6
    { 905, 512,1015},  //duty=15, dutyLt=6
    { 919, 512, 999},  //duty=16, dutyLt=6
    { 930, 512, 986},  //duty=17, dutyLt=6
    { 939, 512, 975},  //duty=18, dutyLt=6
    { 947, 512, 965},  //duty=19, dutyLt=6
    { 960, 512, 963},  //duty=20, dutyLt=6
    { 966, 512, 955},  //duty=21, dutyLt=6
    { 972, 512, 948},  //duty=22, dutyLt=6
    { 978, 512, 941},  //duty=23, dutyLt=6
    { 982, 512, 934},  //duty=24, dutyLt=6
    { 987, 512, 928},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 583, 512,1906},  //duty=-1, dutyLt=7
    { 636, 512,1626},  //duty=0, dutyLt=7
    { 676, 512,1467},  //duty=1, dutyLt=7
    { 708, 512,1374},  //duty=2, dutyLt=7
    { 736, 512,1307},  //duty=3, dutyLt=7
    { 760, 512,1256},  //duty=4, dutyLt=7
    { 779, 512,1217},  //duty=5, dutyLt=7
    { 794, 512,1172},  //duty=6, dutyLt=7
    { 810, 512,1146},  //duty=7, dutyLt=7
    { 823, 512,1124},  //duty=8, dutyLt=7
    { 836, 512,1106},  //duty=9, dutyLt=7
    { 848, 512,1090},  //duty=10, dutyLt=7
    { 857, 512,1075},  //duty=11, dutyLt=7
    { 867, 512,1063},  //duty=12, dutyLt=7
    { 875, 512,1051},  //duty=13, dutyLt=7
    { 883, 512,1041},  //duty=14, dutyLt=7
    { 889, 512,1031},  //duty=15, dutyLt=7
    { 903, 512,1013},  //duty=16, dutyLt=7
    { 914, 512, 999},  //duty=17, dutyLt=7
    { 924, 512, 987},  //duty=18, dutyLt=7
    { 932, 512, 976},  //duty=19, dutyLt=7
    { 940, 512, 967},  //duty=20, dutyLt=7
    { 946, 512, 955},  //duty=21, dutyLt=7
    { 958, 512, 957},  //duty=22, dutyLt=7
    { 964, 512, 951},  //duty=23, dutyLt=7
    { 969, 512, 944},  //duty=24, dutyLt=7
    { 974, 512, 938},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 581, 512,1887},  //duty=-1, dutyLt=8
    { 631, 512,1636},  //duty=0, dutyLt=8
    { 668, 512,1491},  //duty=1, dutyLt=8
    { 699, 512,1400},  //duty=2, dutyLt=8
    { 725, 512,1330},  //duty=3, dutyLt=8
    { 747, 512,1279},  //duty=4, dutyLt=8
    { 767, 512,1238},  //duty=5, dutyLt=8
    { 784, 512,1206},  //duty=6, dutyLt=8
    { 796, 512,1167},  //duty=7, dutyLt=8
    { 810, 512,1144},  //duty=8, dutyLt=8
    { 822, 512,1124},  //duty=9, dutyLt=8
    { 833, 512,1106},  //duty=10, dutyLt=8
    { 843, 512,1092},  //duty=11, dutyLt=8
    { 852, 512,1078},  //duty=12, dutyLt=8
    { 861, 512,1067},  //duty=13, dutyLt=8
    { 868, 512,1055},  //duty=14, dutyLt=8
    { 875, 512,1045},  //duty=15, dutyLt=8
    { 889, 512,1027},  //duty=16, dutyLt=8
    { 900, 512,1012},  //duty=17, dutyLt=8
    { 910, 512, 999},  //duty=18, dutyLt=8
    { 919, 512, 987},  //duty=19, dutyLt=8
    { 927, 512, 977},  //duty=20, dutyLt=8
    { 934, 512, 969},  //duty=21, dutyLt=8
    { 940, 512, 959},  //duty=22, dutyLt=8
    { 946, 512, 952},  //duty=23, dutyLt=8
    { 951, 512, 944},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 584, 512,1899},  //duty=-1, dutyLt=9
    { 627, 512,1648},  //duty=0, dutyLt=9
    { 661, 512,1514},  //duty=1, dutyLt=9
    { 691, 512,1421},  //duty=2, dutyLt=9
    { 715, 512,1350},  //duty=3, dutyLt=9
    { 737, 512,1299},  //duty=4, dutyLt=9
    { 756, 512,1259},  //duty=5, dutyLt=9
    { 772, 512,1224},  //duty=6, dutyLt=9
    { 784, 512,1185},  //duty=7, dutyLt=9
    { 798, 512,1161},  //duty=8, dutyLt=9
    { 810, 512,1140},  //duty=9, dutyLt=9
    { 821, 512,1123},  //duty=10, dutyLt=9
    { 831, 512,1108},  //duty=11, dutyLt=9
    { 839, 512,1093},  //duty=12, dutyLt=9
    { 848, 512,1080},  //duty=13, dutyLt=9
    { 856, 512,1069},  //duty=14, dutyLt=9
    { 863, 512,1058},  //duty=15, dutyLt=9
    { 877, 512,1039},  //duty=16, dutyLt=9
    { 888, 512,1024},  //duty=17, dutyLt=9
    { 898, 512,1010},  //duty=18, dutyLt=9
    { 907, 512, 998},  //duty=19, dutyLt=9
    { 915, 512, 985},  //duty=20, dutyLt=9
    { 922, 512, 978},  //duty=21, dutyLt=9
    { 929, 512, 969},  //duty=22, dutyLt=9
    { 935, 512, 961},  //duty=23, dutyLt=9
    { 940, 512, 953},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 583, 512,1870},  //duty=-1, dutyLt=10
    { 624, 512,1662},  //duty=0, dutyLt=10
    { 657, 512,1532},  //duty=1, dutyLt=10
    { 684, 512,1439},  //duty=2, dutyLt=10
    { 708, 512,1368},  //duty=3, dutyLt=10
    { 728, 512,1317},  //duty=4, dutyLt=10
    { 747, 512,1276},  //duty=5, dutyLt=10
    { 762, 512,1242},  //duty=6, dutyLt=10
    { 777, 512,1213},  //duty=7, dutyLt=10
    { 787, 512,1176},  //duty=8, dutyLt=10
    { 799, 512,1156},  //duty=9, dutyLt=10
    { 810, 512,1137},  //duty=10, dutyLt=10
    { 819, 512,1121},  //duty=11, dutyLt=10
    { 829, 512,1106},  //duty=12, dutyLt=10
    { 836, 512,1094},  //duty=13, dutyLt=10
    { 844, 512,1081},  //duty=14, dutyLt=10
    { 852, 512,1071},  //duty=15, dutyLt=10
    { 866, 512,1051},  //duty=16, dutyLt=10
    { 877, 512,1032},  //duty=17, dutyLt=10
    { 887, 512,1020},  //duty=18, dutyLt=10
    { 896, 512,1005},  //duty=19, dutyLt=10
    { 904, 512, 997},  //duty=20, dutyLt=10
    { 912, 512, 987},  //duty=21, dutyLt=10
    { 918, 512, 977},  //duty=22, dutyLt=10
    { 925, 512, 969},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 582, 512,1867},  //duty=-1, dutyLt=11
    { 622, 512,1675},  //duty=0, dutyLt=11
    { 652, 512,1547},  //duty=1, dutyLt=11
    { 678, 512,1454},  //duty=2, dutyLt=11
    { 701, 512,1386},  //duty=3, dutyLt=11
    { 720, 512,1333},  //duty=4, dutyLt=11
    { 738, 512,1292},  //duty=5, dutyLt=11
    { 753, 512,1259},  //duty=6, dutyLt=11
    { 764, 512,1217},  //duty=7, dutyLt=11
    { 778, 512,1192},  //duty=8, dutyLt=11
    { 789, 512,1169},  //duty=9, dutyLt=11
    { 800, 512,1151},  //duty=10, dutyLt=11
    { 810, 512,1134},  //duty=11, dutyLt=11
    { 818, 512,1119},  //duty=12, dutyLt=11
    { 826, 512,1106},  //duty=13, dutyLt=11
    { 834, 512,1093},  //duty=14, dutyLt=11
    { 841, 512,1082},  //duty=15, dutyLt=11
    { 856, 512,1061},  //duty=16, dutyLt=11
    { 867, 512,1045},  //duty=17, dutyLt=11
    { 877, 512,1031},  //duty=18, dutyLt=11
    { 887, 512,1017},  //duty=19, dutyLt=11
    { 894, 512,1003},  //duty=20, dutyLt=11
    { 902, 512, 995},  //duty=21, dutyLt=11
    { 909, 512, 986},  //duty=22, dutyLt=11
    { 915, 512, 977},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 583, 512,1862},  //duty=-1, dutyLt=12
    { 619, 512,1684},  //duty=0, dutyLt=12
    { 648, 512,1560},  //duty=1, dutyLt=12
    { 673, 512,1468},  //duty=2, dutyLt=12
    { 695, 512,1400},  //duty=3, dutyLt=12
    { 714, 512,1348},  //duty=4, dutyLt=12
    { 731, 512,1306},  //duty=5, dutyLt=12
    { 746, 512,1272},  //duty=6, dutyLt=12
    { 760, 512,1242},  //duty=7, dutyLt=12
    { 772, 512,1216},  //duty=8, dutyLt=12
    { 780, 512,1182},  //duty=9, dutyLt=12
    { 791, 512,1164},  //duty=10, dutyLt=12
    { 800, 512,1145},  //duty=11, dutyLt=12
    { 810, 512,1131},  //duty=12, dutyLt=12
    { 818, 512,1117},  //duty=13, dutyLt=12
    { 825, 512,1104},  //duty=14, dutyLt=12
    { 832, 512,1092},  //duty=15, dutyLt=12
    { 846, 512,1072},  //duty=16, dutyLt=12
    { 858, 512,1054},  //duty=17, dutyLt=12
    { 868, 512,1040},  //duty=18, dutyLt=12
    { 878, 512,1026},  //duty=19, dutyLt=12
    { 886, 512,1014},  //duty=20, dutyLt=12
    { 893, 512,1004},  //duty=21, dutyLt=12
    { 900, 512, 994},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 582, 512,1865},  //duty=-1, dutyLt=13
    { 618, 512,1693},  //duty=0, dutyLt=13
    { 645, 512,1571},  //duty=1, dutyLt=13
    { 669, 512,1482},  //duty=2, dutyLt=13
    { 690, 512,1414},  //duty=3, dutyLt=13
    { 708, 512,1362},  //duty=4, dutyLt=13
    { 724, 512,1320},  //duty=5, dutyLt=13
    { 739, 512,1284},  //duty=6, dutyLt=13
    { 752, 512,1255},  //duty=7, dutyLt=13
    { 762, 512,1216},  //duty=8, dutyLt=13
    { 773, 512,1194},  //duty=9, dutyLt=13
    { 783, 512,1174},  //duty=10, dutyLt=13
    { 793, 512,1157},  //duty=11, dutyLt=13
    { 801, 512,1142},  //duty=12, dutyLt=13
    { 809, 512,1127},  //duty=13, dutyLt=13
    { 817, 512,1115},  //duty=14, dutyLt=13
    { 824, 512,1103},  //duty=15, dutyLt=13
    { 838, 512,1081},  //duty=16, dutyLt=13
    { 850, 512,1064},  //duty=17, dutyLt=13
    { 860, 512,1048},  //duty=18, dutyLt=13
    { 869, 512,1035},  //duty=19, dutyLt=13
    { 877, 512,1022},  //duty=20, dutyLt=13
    { 885, 512,1012},  //duty=21, dutyLt=13
    { 892, 512,1001},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 583, 512,1861},  //duty=-1, dutyLt=14
    { 616, 512,1699},  //duty=0, dutyLt=14
    { 642, 512,1579},  //duty=1, dutyLt=14
    { 665, 512,1492},  //duty=2, dutyLt=14
    { 685, 512,1426},  //duty=3, dutyLt=14
    { 703, 512,1373},  //duty=4, dutyLt=14
    { 718, 512,1331},  //duty=5, dutyLt=14
    { 733, 512,1297},  //duty=6, dutyLt=14
    { 746, 512,1266},  //duty=7, dutyLt=14
    { 755, 512,1228},  //duty=8, dutyLt=14
    { 766, 512,1205},  //duty=9, dutyLt=14
    { 776, 512,1185},  //duty=10, dutyLt=14
    { 785, 512,1168},  //duty=11, dutyLt=14
    { 794, 512,1151},  //duty=12, dutyLt=14
    { 802, 512,1137},  //duty=13, dutyLt=14
    { 809, 512,1125},  //duty=14, dutyLt=14
    { 817, 512,1112},  //duty=15, dutyLt=14
    { 830, 512,1090},  //duty=16, dutyLt=14
    { 842, 512,1072},  //duty=17, dutyLt=14
    { 852, 512,1056},  //duty=18, dutyLt=14
    { 861, 512,1043},  //duty=19, dutyLt=14
    { 870, 512,1030},  //duty=20, dutyLt=14
    { 877, 512,1019},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 583, 512,1859},  //duty=-1, dutyLt=15
    { 615, 512,1706},  //duty=0, dutyLt=15
    { 640, 512,1589},  //duty=1, dutyLt=15
    { 662, 512,1502},  //duty=2, dutyLt=15
    { 681, 512,1436},  //duty=3, dutyLt=15
    { 698, 512,1385},  //duty=4, dutyLt=15
    { 714, 512,1343},  //duty=5, dutyLt=15
    { 728, 512,1308},  //duty=6, dutyLt=15
    { 740, 512,1277},  //duty=7, dutyLt=15
    { 752, 512,1251},  //duty=8, dutyLt=15
    { 763, 512,1228},  //duty=9, dutyLt=15
    { 773, 512,1207},  //duty=10, dutyLt=15
    { 779, 512,1177},  //duty=11, dutyLt=15
    { 787, 512,1161},  //duty=12, dutyLt=15
    { 795, 512,1146},  //duty=13, dutyLt=15
    { 803, 512,1133},  //duty=14, dutyLt=15
    { 810, 512,1121},  //duty=15, dutyLt=15
    { 823, 512,1099},  //duty=16, dutyLt=15
    { 835, 512,1080},  //duty=17, dutyLt=15
    { 845, 512,1065},  //duty=18, dutyLt=15
    { 854, 512,1050},  //duty=19, dutyLt=15
    { 863, 512,1035},  //duty=20, dutyLt=15
    { 870, 512,1026},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 583, 512,1856},  //duty=-1, dutyLt=16
    { 612, 512,1714},  //duty=0, dutyLt=16
    { 636, 512,1600},  //duty=1, dutyLt=16
    { 656, 512,1518},  //duty=2, dutyLt=16
    { 674, 512,1455},  //duty=3, dutyLt=16
    { 692, 512,1397},  //duty=4, dutyLt=16
    { 705, 512,1361},  //duty=5, dutyLt=16
    { 718, 512,1326},  //duty=6, dutyLt=16
    { 731, 512,1295},  //duty=7, dutyLt=16
    { 742, 512,1269},  //duty=8, dutyLt=16
    { 752, 512,1246},  //duty=9, dutyLt=16
    { 760, 512,1209},  //duty=10, dutyLt=16
    { 768, 512,1195},  //duty=11, dutyLt=16
    { 776, 512,1178},  //duty=12, dutyLt=16
    { 783, 512,1163},  //duty=13, dutyLt=16
    { 791, 512,1149},  //duty=14, dutyLt=16
    { 798, 512,1137},  //duty=15, dutyLt=16
    { 811, 512,1115},  //duty=16, dutyLt=16
    { 822, 512,1096},  //duty=17, dutyLt=16
    { 833, 512,1075},  //duty=18, dutyLt=16
    { 842, 512,1064},  //duty=19, dutyLt=16
    { 850, 512,1051},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 583, 512,1850},  //duty=-1, dutyLt=17
    { 611, 512,1719},  //duty=0, dutyLt=17
    { 632, 512,1613},  //duty=1, dutyLt=17
    { 651, 512,1532},  //duty=2, dutyLt=17
    { 670, 512,1466},  //duty=3, dutyLt=17
    { 686, 512,1413},  //duty=4, dutyLt=17
    { 698, 512,1378},  //duty=5, dutyLt=17
    { 711, 512,1342},  //duty=6, dutyLt=17
    { 722, 512,1312},  //duty=7, dutyLt=17
    { 733, 512,1285},  //duty=8, dutyLt=17
    { 743, 512,1263},  //duty=9, dutyLt=17
    { 749, 512,1229},  //duty=10, dutyLt=17
    { 758, 512,1210},  //duty=11, dutyLt=17
    { 766, 512,1194},  //duty=12, dutyLt=17
    { 774, 512,1178},  //duty=13, dutyLt=17
    { 781, 512,1164},  //duty=14, dutyLt=17
    { 788, 512,1151},  //duty=15, dutyLt=17
    { 800, 512,1129},  //duty=16, dutyLt=17
    { 811, 512,1110},  //duty=17, dutyLt=17
    { 822, 512,1093},  //duty=18, dutyLt=17
    { 831, 512,1076},  //duty=19, dutyLt=17
    { 512, 512, 512},  //duty=20, dutyLt=17
    { 512, 512, 512},  //duty=21, dutyLt=17
    { 512, 512, 512},  //duty=22, dutyLt=17
    { 512, 512, 512},  //duty=23, dutyLt=17
    { 512, 512, 512},  //duty=24, dutyLt=17
    { 512, 512, 512},  //duty=25, dutyLt=17
    { 512, 512, 512},  //duty=26, dutyLt=17
    { 512, 512, 512},  //duty=27, dutyLt=17
    { 584, 512,1845},  //duty=-1, dutyLt=18
    { 609, 512,1723},  //duty=0, dutyLt=18
    { 630, 512,1621},  //duty=1, dutyLt=18
    { 648, 512,1545},  //duty=2, dutyLt=18
    { 664, 512,1484},  //duty=3, dutyLt=18
    { 681, 512,1428},  //duty=4, dutyLt=18
    { 692, 512,1392},  //duty=5, dutyLt=18
    { 706, 512,1351},  //duty=6, dutyLt=18
    { 715, 512,1326},  //duty=7, dutyLt=18
    { 726, 512,1300},  //duty=8, dutyLt=18
    { 735, 512,1276},  //duty=9, dutyLt=18
    { 747, 512,1250},  //duty=10, dutyLt=18
    { 749, 512,1225},  //duty=11, dutyLt=18
    { 757, 512,1208},  //duty=12, dutyLt=18
    { 765, 512,1191},  //duty=13, dutyLt=18
    { 772, 512,1178},  //duty=14, dutyLt=18
    { 781, 512,1159},  //duty=15, dutyLt=18
    { 791, 512,1141},  //duty=16, dutyLt=18
    { 802, 512,1122},  //duty=17, dutyLt=18
    { 812, 512,1105},  //duty=18, dutyLt=18
    { 512, 512, 512},  //duty=19, dutyLt=18
    { 512, 512, 512},  //duty=20, dutyLt=18
    { 512, 512, 512},  //duty=21, dutyLt=18
    { 512, 512, 512},  //duty=22, dutyLt=18
    { 512, 512, 512},  //duty=23, dutyLt=18
    { 512, 512, 512},  //duty=24, dutyLt=18
    { 512, 512, 512},  //duty=25, dutyLt=18
    { 512, 512, 512},  //duty=26, dutyLt=18
    { 512, 512, 512},  //duty=27, dutyLt=18
    { 584, 512,1838},  //duty=-1, dutyLt=19
    { 608, 512,1727},  //duty=0, dutyLt=19
    { 628, 512,1627},  //duty=1, dutyLt=19
    { 645, 512,1555},  //duty=2, dutyLt=19
    { 663, 512,1488},  //duty=3, dutyLt=19
    { 674, 512,1447},  //duty=4, dutyLt=19
    { 686, 512,1406},  //duty=5, dutyLt=19
    { 698, 512,1370},  //duty=6, dutyLt=19
    { 709, 512,1341},  //duty=7, dutyLt=19
    { 719, 512,1314},  //duty=8, dutyLt=19
    { 728, 512,1290},  //duty=9, dutyLt=19
    { 734, 512,1255},  //duty=10, dutyLt=19
    { 742, 512,1237},  //duty=11, dutyLt=19
    { 750, 512,1220},  //duty=12, dutyLt=19
    { 761, 512,1198},  //duty=13, dutyLt=19
    { 764, 512,1190},  //duty=14, dutyLt=19
    { 774, 512,1170},  //duty=15, dutyLt=19
    { 783, 512,1154},  //duty=16, dutyLt=19
    { 794, 512,1133},  //duty=17, dutyLt=19
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
    { 585, 512,1833},  //duty=-1, dutyLt=20
    { 607, 512,1732},  //duty=0, dutyLt=20
    { 626, 512,1634},  //duty=1, dutyLt=20
    { 645, 512,1554},  //duty=2, dutyLt=20
    { 657, 512,1504},  //duty=3, dutyLt=20
    { 670, 512,1456},  //duty=4, dutyLt=20
    { 682, 512,1416},  //duty=5, dutyLt=20
    { 693, 512,1381},  //duty=6, dutyLt=20
    { 704, 512,1349},  //duty=7, dutyLt=20
    { 714, 512,1324},  //duty=8, dutyLt=20
    { 723, 512,1300},  //duty=9, dutyLt=20
    { 731, 512,1279},  //duty=10, dutyLt=20
    { 737, 512,1247},  //duty=11, dutyLt=20
    { 744, 512,1231},  //duty=12, dutyLt=20
    { 751, 512,1214},  //duty=13, dutyLt=20
    { 758, 512,1199},  //duty=14, dutyLt=20
    { 765, 512,1186},  //duty=15, dutyLt=20
    { 776, 512,1163},  //duty=16, dutyLt=20
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
    { 585, 512,1829},  //duty=-1, dutyLt=21
    { 607, 512,1729},  //duty=0, dutyLt=21
    { 625, 512,1639},  //duty=1, dutyLt=21
    { 640, 512,1569},  //duty=2, dutyLt=21
    { 654, 512,1511},  //duty=3, dutyLt=21
    { 667, 512,1466},  //duty=4, dutyLt=21
    { 679, 512,1426},  //duty=5, dutyLt=21
    { 693, 512,1382},  //duty=6, dutyLt=21
    { 700, 512,1360},  //duty=7, dutyLt=21
    { 709, 512,1334},  //duty=8, dutyLt=21
    { 718, 512,1310},  //duty=9, dutyLt=21
    { 726, 512,1290},  //duty=10, dutyLt=21
    { 732, 512,1257},  //duty=11, dutyLt=21
    { 739, 512,1240},  //duty=12, dutyLt=21
    { 746, 512,1223},  //duty=13, dutyLt=21
    { 752, 512,1211},  //duty=14, dutyLt=21
    { 759, 512,1195},  //duty=15, dutyLt=21
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
    { 586, 512,1824},  //duty=-1, dutyLt=22
    { 607, 512,1730},  //duty=0, dutyLt=22
    { 623, 512,1645},  //duty=1, dutyLt=22
    { 639, 512,1574},  //duty=2, dutyLt=22
    { 652, 512,1520},  //duty=3, dutyLt=22
    { 664, 512,1473},  //duty=4, dutyLt=22
    { 676, 512,1433},  //duty=5, dutyLt=22
    { 686, 512,1399},  //duty=6, dutyLt=22
    { 700, 512,1359},  //duty=7, dutyLt=22
    { 705, 512,1343},  //duty=8, dutyLt=22
    { 714, 512,1320},  //duty=9, dutyLt=22
    { 722, 512,1299},  //duty=10, dutyLt=22
    { 727, 512,1265},  //duty=11, dutyLt=22
    { 734, 512,1249},  //duty=12, dutyLt=22
    { 741, 512,1232},  //duty=13, dutyLt=22
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
    { 587, 512,1818},  //duty=-1, dutyLt=23
    { 606, 512,1731},  //duty=0, dutyLt=23
    { 622, 512,1647},  //duty=1, dutyLt=23
    { 637, 512,1580},  //duty=2, dutyLt=23
    { 650, 512,1524},  //duty=3, dutyLt=23
    { 662, 512,1479},  //duty=4, dutyLt=23
    { 673, 512,1441},  //duty=5, dutyLt=23
    { 682, 512,1408},  //duty=6, dutyLt=23
    { 693, 512,1376},  //duty=7, dutyLt=23
    { 702, 512,1350},  //duty=8, dutyLt=23
    { 711, 512,1326},  //duty=9, dutyLt=23
    { 718, 512,1306},  //duty=10, dutyLt=23
    { 725, 512,1288},  //duty=11, dutyLt=23
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
    { 587, 512,1813},  //duty=-1, dutyLt=24
    { 606, 512,1730},  //duty=0, dutyLt=24
    { 622, 512,1646},  //duty=1, dutyLt=24
    { 635, 512,1585},  //duty=2, dutyLt=24
    { 654, 512,1516},  //duty=3, dutyLt=24
    { 665, 512,1472},  //duty=4, dutyLt=24
    { 670, 512,1448},  //duty=5, dutyLt=24
    { 681, 512,1413},  //duty=6, dutyLt=24
    { 691, 512,1383},  //duty=7, dutyLt=24
    { 699, 512,1358},  //duty=8, dutyLt=24
    { 707, 512,1335},  //duty=9, dutyLt=24
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
    { 590, 512,1827},  //duty=-1, dutyLt=25
    { 606, 512,1729},  //duty=0, dutyLt=25
    { 625, 512,1638},  //duty=1, dutyLt=25
    { 634, 512,1588},  //duty=2, dutyLt=25
    { 646, 512,1536},  //duty=3, dutyLt=25
    { 659, 512,1489},  //duty=4, dutyLt=25
    { 669, 512,1451},  //duty=5, dutyLt=25
    { 679, 512,1418},  //duty=6, dutyLt=25
    { 689, 512,1388},  //duty=7, dutyLt=25
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
    { 590, 512,1825},  //duty=-1, dutyLt=26
    { 605, 512,1730},  //duty=0, dutyLt=26
    { 621, 512,1649},  //duty=1, dutyLt=26
    { 633, 512,1591},  //duty=2, dutyLt=26
    { 646, 512,1538},  //duty=3, dutyLt=26
    { 657, 512,1494},  //duty=4, dutyLt=26
    { 666, 512,1458},  //duty=5, dutyLt=26
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
    { 591, 512,1822},  //duty=-1, dutyLt=27
    { 605, 512,1731},  //duty=0, dutyLt=27
    { 621, 512,1651},  //duty=1, dutyLt=27
    { 633, 512,1590},  //duty=2, dutyLt=27
    { 645, 512,1541},  //duty=3, dutyLt=27
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
    { 512, 512, 512},  //duty=27, dutyLt=27
}}




