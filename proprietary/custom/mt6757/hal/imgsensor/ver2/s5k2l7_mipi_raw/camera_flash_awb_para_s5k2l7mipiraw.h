/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
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
   {1022, 512, 939},  //duty=0, dutyLt=-1
   {1024, 512, 937},  //duty=1, dutyLt=-1
   {1022, 512, 928},  //duty=2, dutyLt=-1
   {1019, 512, 922},  //duty=3, dutyLt=-1
   {1017, 512, 915},  //duty=4, dutyLt=-1
   {1015, 512, 907},  //duty=5, dutyLt=-1
   {1014, 512, 890},  //duty=6, dutyLt=-1
   {1020, 512, 908},  //duty=7, dutyLt=-1
   {1020, 512, 902},  //duty=8, dutyLt=-1
   {1019, 512, 895},  //duty=9, dutyLt=-1
   {1017, 512, 890},  //duty=10, dutyLt=-1
   {1017, 512, 884},  //duty=11, dutyLt=-1
   {1016, 512, 880},  //duty=12, dutyLt=-1
   {1015, 512, 875},  //duty=13, dutyLt=-1
   {1015, 512, 870},  //duty=14, dutyLt=-1
   {1014, 512, 866},  //duty=15, dutyLt=-1
   {1014, 512, 862},  //duty=16, dutyLt=-1
   {1013, 512, 859},  //duty=17, dutyLt=-1
   {1013, 512, 855},  //duty=18, dutyLt=-1
   {1013, 512, 852},  //duty=19, dutyLt=-1
   {1012, 512, 849},  //duty=20, dutyLt=-1
   {1012, 512, 845},  //duty=21, dutyLt=-1
   {1012, 512, 843},  //duty=22, dutyLt=-1
   {1012, 512, 840},  //duty=23, dutyLt=-1
   {1011, 512, 837},  //duty=24, dutyLt=-1
   {1011, 512, 834},  //duty=25, dutyLt=-1
   { 522, 512,1993},  //duty=-1, dutyLt=0
   { 730, 512,1215},  //duty=0, dutyLt=0
   { 818, 512,1086},  //duty=1, dutyLt=0
   { 862, 512,1033},  //duty=2, dutyLt=0
   { 889, 512,1002},  //duty=3, dutyLt=0
   { 905, 512, 981},  //duty=4, dutyLt=0
   { 917, 512, 958},  //duty=5, dutyLt=0
   { 933, 512, 962},  //duty=6, dutyLt=0
   { 942, 512, 952},  //duty=7, dutyLt=0
   { 952, 512, 939},  //duty=8, dutyLt=0
   { 960, 512, 926},  //duty=9, dutyLt=0
   { 966, 512, 918},  //duty=10, dutyLt=0
   { 969, 512, 909},  //duty=11, dutyLt=0
   { 972, 512, 900},  //duty=12, dutyLt=0
   { 976, 512, 895},  //duty=13, dutyLt=0
   { 977, 512, 890},  //duty=14, dutyLt=0
   { 979, 512, 886},  //duty=15, dutyLt=0
   { 981, 512, 879},  //duty=16, dutyLt=0
   { 982, 512, 875},  //duty=17, dutyLt=0
   { 983, 512, 871},  //duty=18, dutyLt=0
   { 983, 512, 867},  //duty=19, dutyLt=0
   { 985, 512, 863},  //duty=20, dutyLt=0
   { 985, 512, 860},  //duty=21, dutyLt=0
   { 986, 512, 856},  //duty=22, dutyLt=0
   { 987, 512, 853},  //duty=23, dutyLt=0
   { 987, 512, 850},  //duty=24, dutyLt=0
   { 512, 512, 512},  //duty=25, dutyLt=0
   { 521, 512,1978},  //duty=-1, dutyLt=1
   { 649, 512,1379},  //duty=0, dutyLt=1
   { 727, 512,1203},  //duty=1, dutyLt=1
   { 775, 512,1122},  //duty=2, dutyLt=1
   { 807, 512,1073},  //duty=3, dutyLt=1
   { 830, 512,1037},  //duty=4, dutyLt=1
   { 854, 512,1026},  //duty=5, dutyLt=1
   { 868, 512,1011},  //duty=6, dutyLt=1
   { 884, 512, 996},  //duty=7, dutyLt=1
   { 900, 512, 975},  //duty=8, dutyLt=1
   { 915, 512, 956},  //duty=9, dutyLt=1
   { 923, 512, 944},  //duty=10, dutyLt=1
   { 930, 512, 934},  //duty=11, dutyLt=1
   { 936, 512, 925},  //duty=12, dutyLt=1
   { 941, 512, 916},  //duty=13, dutyLt=1
   { 945, 512, 910},  //duty=14, dutyLt=1
   { 948, 512, 903},  //duty=15, dutyLt=1
   { 951, 512, 896},  //duty=16, dutyLt=1
   { 954, 512, 891},  //duty=17, dutyLt=1
   { 956, 512, 886},  //duty=18, dutyLt=1
   { 958, 512, 882},  //duty=19, dutyLt=1
   { 960, 512, 877},  //duty=20, dutyLt=1
   { 961, 512, 873},  //duty=21, dutyLt=1
   { 962, 512, 870},  //duty=22, dutyLt=1
   { 964, 512, 865},  //duty=23, dutyLt=1
   { 965, 512, 861},  //duty=24, dutyLt=1
   { 512, 512, 512},  //duty=25, dutyLt=1
   { 521, 512,1997},  //duty=-1, dutyLt=2
   { 614, 512,1478},  //duty=0, dutyLt=2
   { 679, 512,1287},  //duty=1, dutyLt=2
   { 724, 512,1191},  //duty=2, dutyLt=2
   { 757, 512,1129},  //duty=3, dutyLt=2
   { 786, 512,1103},  //duty=4, dutyLt=2
   { 805, 512,1076},  //duty=5, dutyLt=2
   { 822, 512,1053},  //duty=6, dutyLt=2
   { 840, 512,1031},  //duty=7, dutyLt=2
   { 860, 512,1006},  //duty=8, dutyLt=2
   { 878, 512, 984},  //duty=9, dutyLt=2
   { 889, 512, 969},  //duty=10, dutyLt=2
   { 898, 512, 956},  //duty=11, dutyLt=2
   { 905, 512, 946},  //duty=12, dutyLt=2
   { 913, 512, 935},  //duty=13, dutyLt=2
   { 918, 512, 927},  //duty=14, dutyLt=2
   { 922, 512, 920},  //duty=15, dutyLt=2
   { 926, 512, 912},  //duty=16, dutyLt=2
   { 930, 512, 906},  //duty=17, dutyLt=2
   { 933, 512, 901},  //duty=18, dutyLt=2
   { 935, 512, 896},  //duty=19, dutyLt=2
   { 938, 512, 890},  //duty=20, dutyLt=2
   { 940, 512, 886},  //duty=21, dutyLt=2
   { 942, 512, 881},  //duty=22, dutyLt=2
   { 944, 512, 877},  //duty=23, dutyLt=2
   { 512, 512, 512},  //duty=24, dutyLt=2
   { 512, 512, 512},  //duty=25, dutyLt=2
   { 521, 512,1982},  //duty=-1, dutyLt=3
   { 594, 512,1545},  //duty=0, dutyLt=3
   { 651, 512,1350},  //duty=1, dutyLt=3
   { 692, 512,1243},  //duty=2, dutyLt=3
   { 727, 512,1190},  //duty=3, dutyLt=3
   { 750, 512,1149},  //duty=4, dutyLt=3
   { 770, 512,1117},  //duty=5, dutyLt=3
   { 788, 512,1092},  //duty=6, dutyLt=3
   { 808, 512,1065},  //duty=7, dutyLt=3
   { 829, 512,1035},  //duty=8, dutyLt=3
   { 849, 512,1008},  //duty=9, dutyLt=3
   { 861, 512, 991},  //duty=10, dutyLt=3
   { 871, 512, 976},  //duty=11, dutyLt=3
   { 880, 512, 965},  //duty=12, dutyLt=3
   { 889, 512, 952},  //duty=13, dutyLt=3
   { 895, 512, 944},  //duty=14, dutyLt=3
   { 899, 512, 935},  //duty=15, dutyLt=3
   { 905, 512, 926},  //duty=16, dutyLt=3
   { 909, 512, 919},  //duty=17, dutyLt=3
   { 913, 512, 914},  //duty=18, dutyLt=3
   { 916, 512, 909},  //duty=19, dutyLt=3
   { 919, 512, 902},  //duty=20, dutyLt=3
   { 922, 512, 897},  //duty=21, dutyLt=3
   { 924, 512, 893},  //duty=22, dutyLt=3
   { 926, 512, 888},  //duty=23, dutyLt=3
   { 512, 512, 512},  //duty=24, dutyLt=3
   { 512, 512, 512},  //duty=25, dutyLt=3
   { 521, 512,1974},  //duty=-1, dutyLt=4
   { 583, 512,1589},  //duty=0, dutyLt=4
   { 632, 512,1396},  //duty=1, dutyLt=4
   { 672, 512,1276},  //duty=2, dutyLt=4
   { 702, 512,1231},  //duty=3, dutyLt=4
   { 725, 512,1187},  //duty=4, dutyLt=4
   { 745, 512,1151},  //duty=5, dutyLt=4
   { 762, 512,1122},  //duty=6, dutyLt=4
   { 782, 512,1093},  //duty=7, dutyLt=4
   { 804, 512,1061},  //duty=8, dutyLt=4
   { 824, 512,1031},  //duty=9, dutyLt=4
   { 838, 512,1012},  //duty=10, dutyLt=4
   { 849, 512, 996},  //duty=11, dutyLt=4
   { 858, 512, 982},  //duty=12, dutyLt=4
   { 868, 512, 969},  //duty=13, dutyLt=4
   { 874, 512, 959},  //duty=14, dutyLt=4
   { 880, 512, 951},  //duty=15, dutyLt=4
   { 886, 512, 940},  //duty=16, dutyLt=4
   { 891, 512, 933},  //duty=17, dutyLt=4
   { 894, 512, 926},  //duty=18, dutyLt=4
   { 898, 512, 921},  //duty=19, dutyLt=4
   { 902, 512, 914},  //duty=20, dutyLt=4
   { 905, 512, 908},  //duty=21, dutyLt=4
   { 908, 512, 904},  //duty=22, dutyLt=4
   { 512, 512, 512},  //duty=23, dutyLt=4
   { 512, 512, 512},  //duty=24, dutyLt=4
   { 512, 512, 512},  //duty=25, dutyLt=4
   { 521, 512,1966},  //duty=-1, dutyLt=5
   { 574, 512,1623},  //duty=0, dutyLt=5
   { 619, 512,1430},  //duty=1, dutyLt=5
   { 655, 512,1341},  //duty=2, dutyLt=5
   { 683, 512,1270},  //duty=3, dutyLt=5
   { 705, 512,1222},  //duty=4, dutyLt=5
   { 724, 512,1184},  //duty=5, dutyLt=5
   { 741, 512,1153},  //duty=6, dutyLt=5
   { 760, 512,1120},  //duty=7, dutyLt=5
   { 782, 512,1084},  //duty=8, dutyLt=5
   { 804, 512,1051},  //duty=9, dutyLt=5
   { 818, 512,1031},  //duty=10, dutyLt=5
   { 830, 512,1014},  //duty=11, dutyLt=5
   { 840, 512, 999},  //duty=12, dutyLt=5
   { 850, 512, 984},  //duty=13, dutyLt=5
   { 857, 512, 973},  //duty=14, dutyLt=5
   { 863, 512, 963},  //duty=15, dutyLt=5
   { 870, 512, 953},  //duty=16, dutyLt=5
   { 874, 512, 945},  //duty=17, dutyLt=5
   { 879, 512, 938},  //duty=18, dutyLt=5
   { 882, 512, 932},  //duty=19, dutyLt=5
   { 887, 512, 924},  //duty=20, dutyLt=5
   { 890, 512, 918},  //duty=21, dutyLt=5
   { 893, 512, 913},  //duty=22, dutyLt=5
   { 512, 512, 512},  //duty=23, dutyLt=5
   { 512, 512, 512},  //duty=24, dutyLt=5
   { 512, 512, 512},  //duty=25, dutyLt=5
   { 521, 512,1959},  //duty=-1, dutyLt=6
   { 568, 512,1645},  //duty=0, dutyLt=6
   { 609, 512,1482},  //duty=1, dutyLt=6
   { 641, 512,1374},  //duty=2, dutyLt=6
   { 667, 512,1302},  //duty=3, dutyLt=6
   { 688, 512,1253},  //duty=4, dutyLt=6
   { 707, 512,1213},  //duty=5, dutyLt=6
   { 723, 512,1180},  //duty=6, dutyLt=6
   { 743, 512,1145},  //duty=7, dutyLt=6
   { 765, 512,1106},  //duty=8, dutyLt=6
   { 786, 512,1071},  //duty=9, dutyLt=6
   { 801, 512,1049},  //duty=10, dutyLt=6
   { 813, 512,1030},  //duty=11, dutyLt=6
   { 823, 512,1015},  //duty=12, dutyLt=6
   { 834, 512, 998},  //duty=13, dutyLt=6
   { 841, 512, 987},  //duty=14, dutyLt=6
   { 848, 512, 976},  //duty=15, dutyLt=6
   { 853, 512, 962},  //duty=16, dutyLt=6
   { 860, 512, 957},  //duty=17, dutyLt=6
   { 865, 512, 949},  //duty=18, dutyLt=6
   { 869, 512, 943},  //duty=19, dutyLt=6
   { 874, 512, 935},  //duty=20, dutyLt=6
   { 877, 512, 928},  //duty=21, dutyLt=6
   { 512, 512, 512},  //duty=22, dutyLt=6
   { 512, 512, 512},  //duty=23, dutyLt=6
   { 512, 512, 512},  //duty=24, dutyLt=6
   { 512, 512, 512},  //duty=25, dutyLt=6
   { 521, 512,1949},  //duty=-1, dutyLt=7
   { 563, 512,1660},  //duty=0, dutyLt=7
   { 597, 512,1526},  //duty=1, dutyLt=7
   { 625, 512,1421},  //duty=2, dutyLt=7
   { 649, 512,1349},  //duty=3, dutyLt=7
   { 668, 512,1294},  //duty=4, dutyLt=7
   { 686, 512,1252},  //duty=5, dutyLt=7
   { 701, 512,1218},  //duty=6, dutyLt=7
   { 721, 512,1178},  //duty=7, dutyLt=7
   { 743, 512,1136},  //duty=8, dutyLt=7
   { 765, 512,1098},  //duty=9, dutyLt=7
   { 779, 512,1074},  //duty=10, dutyLt=7
   { 791, 512,1053},  //duty=11, dutyLt=7
   { 802, 512,1036},  //duty=12, dutyLt=7
   { 813, 512,1018},  //duty=13, dutyLt=7
   { 821, 512,1005},  //duty=14, dutyLt=7
   { 828, 512, 994},  //duty=15, dutyLt=7
   { 835, 512, 982},  //duty=16, dutyLt=7
   { 841, 512, 973},  //duty=17, dutyLt=7
   { 846, 512, 965},  //duty=18, dutyLt=7
   { 850, 512, 958},  //duty=19, dutyLt=7
   { 855, 512, 949},  //duty=20, dutyLt=7
   { 859, 512, 942},  //duty=21, dutyLt=7
   { 512, 512, 512},  //duty=22, dutyLt=7
   { 512, 512, 512},  //duty=23, dutyLt=7
   { 512, 512, 512},  //duty=24, dutyLt=7
   { 512, 512, 512},  //duty=25, dutyLt=7
   { 524, 512,1921},  //duty=-1, dutyLt=8
   { 556, 512,1718},  //duty=0, dutyLt=8
   { 586, 512,1567},  //duty=1, dutyLt=8
   { 611, 512,1466},  //duty=2, dutyLt=8
   { 632, 512,1394},  //duty=3, dutyLt=8
   { 650, 512,1338},  //duty=4, dutyLt=8
   { 666, 512,1294},  //duty=5, dutyLt=8
   { 680, 512,1259},  //duty=6, dutyLt=8
   { 699, 512,1215},  //duty=7, dutyLt=8
   { 720, 512,1170},  //duty=8, dutyLt=8
   { 741, 512,1129},  //duty=9, dutyLt=8
   { 755, 512,1102},  //duty=10, dutyLt=8
   { 768, 512,1080},  //duty=11, dutyLt=8
   { 779, 512,1061},  //duty=12, dutyLt=8
   { 790, 512,1042},  //duty=13, dutyLt=8
   { 798, 512,1028},  //duty=14, dutyLt=8
   { 806, 512,1016},  //duty=15, dutyLt=8
   { 813, 512,1002},  //duty=16, dutyLt=8
   { 819, 512, 993},  //duty=17, dutyLt=8
   { 824, 512, 984},  //duty=18, dutyLt=8
   { 829, 512, 976},  //duty=19, dutyLt=8
   { 835, 512, 967},  //duty=20, dutyLt=8
   { 512, 512, 512},  //duty=21, dutyLt=8
   { 512, 512, 512},  //duty=22, dutyLt=8
   { 512, 512, 512},  //duty=23, dutyLt=8
   { 512, 512, 512},  //duty=24, dutyLt=8
   { 512, 512, 512},  //duty=25, dutyLt=8
   { 522, 512,1951},  //duty=-1, dutyLt=9
   { 551, 512,1743},  //duty=0, dutyLt=9
   { 576, 512,1604},  //duty=1, dutyLt=9
   { 598, 512,1509},  //duty=2, dutyLt=9
   { 616, 512,1437},  //duty=3, dutyLt=9
   { 633, 512,1381},  //duty=4, dutyLt=9
   { 648, 512,1338},  //duty=5, dutyLt=9
   { 661, 512,1299},  //duty=6, dutyLt=9
   { 678, 512,1253},  //duty=7, dutyLt=9
   { 698, 512,1207},  //duty=8, dutyLt=9
   { 719, 512,1162},  //duty=9, dutyLt=9
   { 733, 512,1134},  //duty=10, dutyLt=9
   { 745, 512,1110},  //duty=11, dutyLt=9
   { 756, 512,1089},  //duty=12, dutyLt=9
   { 767, 512,1069},  //duty=13, dutyLt=9
   { 775, 512,1053},  //duty=14, dutyLt=9
   { 783, 512,1041},  //duty=15, dutyLt=9
   { 791, 512,1025},  //duty=16, dutyLt=9
   { 797, 512,1015},  //duty=17, dutyLt=9
   { 803, 512,1005},  //duty=18, dutyLt=9
   { 807, 512, 997},  //duty=19, dutyLt=9
   { 512, 512, 512},  //duty=20, dutyLt=9
   { 512, 512, 512},  //duty=21, dutyLt=9
   { 512, 512, 512},  //duty=22, dutyLt=9
   { 512, 512, 512},  //duty=23, dutyLt=9
   { 512, 512, 512},  //duty=24, dutyLt=9
   { 512, 512, 512},  //duty=25, dutyLt=9
   { 522, 512,1943},  //duty=-1, dutyLt=10
   { 548, 512,1757},  //duty=0, dutyLt=10
   { 570, 512,1628},  //duty=1, dutyLt=10
   { 590, 512,1536},  //duty=2, dutyLt=10
   { 607, 512,1465},  //duty=3, dutyLt=10
   { 623, 512,1409},  //duty=4, dutyLt=10
   { 636, 512,1364},  //duty=5, dutyLt=10
   { 649, 512,1327},  //duty=6, dutyLt=10
   { 666, 512,1281},  //duty=7, dutyLt=10
   { 685, 512,1231},  //duty=8, dutyLt=10
   { 704, 512,1186},  //duty=9, dutyLt=10
   { 718, 512,1156},  //duty=10, dutyLt=10
   { 730, 512,1130},  //duty=11, dutyLt=10
   { 741, 512,1110},  //duty=12, dutyLt=10
   { 752, 512,1088},  //duty=13, dutyLt=10
   { 760, 512,1072},  //duty=14, dutyLt=10
   { 768, 512,1058},  //duty=15, dutyLt=10
   { 776, 512,1042},  //duty=16, dutyLt=10
   { 782, 512,1031},  //duty=17, dutyLt=10
   { 788, 512,1020},  //duty=18, dutyLt=10
   { 512, 512, 512},  //duty=19, dutyLt=10
   { 512, 512, 512},  //duty=20, dutyLt=10
   { 512, 512, 512},  //duty=21, dutyLt=10
   { 512, 512, 512},  //duty=22, dutyLt=10
   { 512, 512, 512},  //duty=23, dutyLt=10
   { 512, 512, 512},  //duty=24, dutyLt=10
   { 512, 512, 512},  //duty=25, dutyLt=10
   { 522, 512,1937},  //duty=-1, dutyLt=11
   { 545, 512,1768},  //duty=0, dutyLt=11
   { 566, 512,1647},  //duty=1, dutyLt=11
   { 584, 512,1557},  //duty=2, dutyLt=11
   { 600, 512,1487},  //duty=3, dutyLt=11
   { 615, 512,1432},  //duty=4, dutyLt=11
   { 627, 512,1389},  //duty=5, dutyLt=11
   { 639, 512,1351},  //duty=6, dutyLt=11
   { 655, 512,1303},  //duty=7, dutyLt=11
   { 673, 512,1254},  //duty=8, dutyLt=11
   { 693, 512,1206},  //duty=9, dutyLt=11
   { 706, 512,1175},  //duty=10, dutyLt=11
   { 718, 512,1150},  //duty=11, dutyLt=11
   { 728, 512,1127},  //duty=12, dutyLt=11
   { 739, 512,1104},  //duty=13, dutyLt=11
   { 748, 512,1088},  //duty=14, dutyLt=11
   { 755, 512,1073},  //duty=15, dutyLt=11
   { 763, 512,1057},  //duty=16, dutyLt=11
   { 769, 512,1045},  //duty=17, dutyLt=11
   { 512, 512, 512},  //duty=18, dutyLt=11
   { 512, 512, 512},  //duty=19, dutyLt=11
   { 512, 512, 512},  //duty=20, dutyLt=11
   { 512, 512, 512},  //duty=21, dutyLt=11
   { 512, 512, 512},  //duty=22, dutyLt=11
   { 512, 512, 512},  //duty=23, dutyLt=11
   { 512, 512, 512},  //duty=24, dutyLt=11
   { 512, 512, 512},  //duty=25, dutyLt=11
   { 522, 512,1933},  //duty=-1, dutyLt=12
   { 543, 512,1776},  //duty=0, dutyLt=12
   { 563, 512,1662},  //duty=1, dutyLt=12
   { 579, 512,1574},  //duty=2, dutyLt=12
   { 594, 512,1508},  //duty=3, dutyLt=12
   { 608, 512,1453},  //duty=4, dutyLt=12
   { 620, 512,1409},  //duty=5, dutyLt=12
   { 631, 512,1372},  //duty=6, dutyLt=12
   { 646, 512,1324},  //duty=7, dutyLt=12
   { 664, 512,1274},  //duty=8, dutyLt=12
   { 683, 512,1225},  //duty=9, dutyLt=12
   { 696, 512,1193},  //duty=10, dutyLt=12
   { 707, 512,1166},  //duty=11, dutyLt=12
   { 717, 512,1144},  //duty=12, dutyLt=12
   { 728, 512,1120},  //duty=13, dutyLt=12
   { 737, 512,1103},  //duty=14, dutyLt=12
   { 744, 512,1088},  //duty=15, dutyLt=12
   { 752, 512,1071},  //duty=16, dutyLt=12
   { 512, 512, 512},  //duty=17, dutyLt=12
   { 512, 512, 512},  //duty=18, dutyLt=12
   { 512, 512, 512},  //duty=19, dutyLt=12
   { 512, 512, 512},  //duty=20, dutyLt=12
   { 512, 512, 512},  //duty=21, dutyLt=12
   { 512, 512, 512},  //duty=22, dutyLt=12
   { 512, 512, 512},  //duty=23, dutyLt=12
   { 512, 512, 512},  //duty=24, dutyLt=12
   { 512, 512, 512},  //duty=25, dutyLt=12
   { 522, 512,1926},  //duty=-1, dutyLt=13
   { 542, 512,1783},  //duty=0, dutyLt=13
   { 559, 512,1676},  //duty=1, dutyLt=13
   { 575, 512,1593},  //duty=2, dutyLt=13
   { 588, 512,1529},  //duty=3, dutyLt=13
   { 601, 512,1475},  //duty=4, dutyLt=13
   { 612, 512,1431},  //duty=5, dutyLt=13
   { 623, 512,1392},  //duty=6, dutyLt=13
   { 637, 512,1346},  //duty=7, dutyLt=13
   { 654, 512,1295},  //duty=8, dutyLt=13
   { 672, 512,1245},  //duty=9, dutyLt=13
   { 685, 512,1213},  //duty=10, dutyLt=13
   { 695, 512,1186},  //duty=11, dutyLt=13
   { 706, 512,1162},  //duty=12, dutyLt=13
   { 717, 512,1137},  //duty=13, dutyLt=13
   { 725, 512,1119},  //duty=14, dutyLt=13
   { 732, 512,1104},  //duty=15, dutyLt=13
   { 512, 512, 512},  //duty=16, dutyLt=13
   { 512, 512, 512},  //duty=17, dutyLt=13
   { 512, 512, 512},  //duty=18, dutyLt=13
   { 512, 512, 512},  //duty=19, dutyLt=13
   { 512, 512, 512},  //duty=20, dutyLt=13
   { 512, 512, 512},  //duty=21, dutyLt=13
   { 512, 512, 512},  //duty=22, dutyLt=13
   { 512, 512, 512},  //duty=23, dutyLt=13
   { 512, 512, 512},  //duty=24, dutyLt=13
   { 512, 512, 512},  //duty=25, dutyLt=13
   { 523, 512,1921},  //duty=-1, dutyLt=14
   { 540, 512,1786},  //duty=0, dutyLt=14
   { 557, 512,1684},  //duty=1, dutyLt=14
   { 572, 512,1605},  //duty=2, dutyLt=14
   { 585, 512,1541},  //duty=3, dutyLt=14
   { 597, 512,1488},  //duty=4, dutyLt=14
   { 607, 512,1445},  //duty=5, dutyLt=14
   { 618, 512,1408},  //duty=6, dutyLt=14
   { 631, 512,1361},  //duty=7, dutyLt=14
   { 647, 512,1310},  //duty=8, dutyLt=14
   { 665, 512,1260},  //duty=9, dutyLt=14
   { 676, 512,1229},  //duty=10, dutyLt=14
   { 688, 512,1200},  //duty=11, dutyLt=14
   { 698, 512,1176},  //duty=12, dutyLt=14
   { 709, 512,1150},  //duty=13, dutyLt=14
   { 716, 512,1132},  //duty=14, dutyLt=14
   { 512, 512, 512},  //duty=15, dutyLt=14
   { 512, 512, 512},  //duty=16, dutyLt=14
   { 512, 512, 512},  //duty=17, dutyLt=14
   { 512, 512, 512},  //duty=18, dutyLt=14
   { 512, 512, 512},  //duty=19, dutyLt=14
   { 512, 512, 512},  //duty=20, dutyLt=14
   { 512, 512, 512},  //duty=21, dutyLt=14
   { 512, 512, 512},  //duty=22, dutyLt=14
   { 512, 512, 512},  //duty=23, dutyLt=14
   { 512, 512, 512},  //duty=24, dutyLt=14
   { 512, 512, 512},  //duty=25, dutyLt=14
   { 523, 512,1916},  //duty=-1, dutyLt=15
   { 540, 512,1791},  //duty=0, dutyLt=15
   { 555, 512,1693},  //duty=1, dutyLt=15
   { 569, 512,1616},  //duty=2, dutyLt=15
   { 582, 512,1552},  //duty=3, dutyLt=15
   { 593, 512,1502},  //duty=4, dutyLt=15
   { 604, 512,1458},  //duty=5, dutyLt=15
   { 613, 512,1421},  //duty=6, dutyLt=15
   { 626, 512,1374},  //duty=7, dutyLt=15
   { 642, 512,1323},  //duty=8, dutyLt=15
   { 659, 512,1272},  //duty=9, dutyLt=15
   { 670, 512,1240},  //duty=10, dutyLt=15
   { 681, 512,1211},  //duty=11, dutyLt=15
   { 691, 512,1187},  //duty=12, dutyLt=15
   { 702, 512,1161},  //duty=13, dutyLt=15
   { 512, 512, 512},  //duty=14, dutyLt=15
   { 512, 512, 512},  //duty=15, dutyLt=15
   { 512, 512, 512},  //duty=16, dutyLt=15
   { 512, 512, 512},  //duty=17, dutyLt=15
   { 512, 512, 512},  //duty=18, dutyLt=15
   { 512, 512, 512},  //duty=19, dutyLt=15
   { 512, 512, 512},  //duty=20, dutyLt=15
   { 512, 512, 512},  //duty=21, dutyLt=15
   { 512, 512, 512},  //duty=22, dutyLt=15
   { 512, 512, 512},  //duty=23, dutyLt=15
   { 512, 512, 512},  //duty=24, dutyLt=15
   { 512, 512, 512},  //duty=25, dutyLt=15
   { 523, 512,1912},  //duty=-1, dutyLt=16
   { 538, 512,1794},  //duty=0, dutyLt=16
   { 553, 512,1701},  //duty=1, dutyLt=16
   { 566, 512,1625},  //duty=2, dutyLt=16
   { 578, 512,1564},  //duty=3, dutyLt=16
   { 589, 512,1514},  //duty=4, dutyLt=16
   { 599, 512,1472},  //duty=5, dutyLt=16
   { 608, 512,1435},  //duty=6, dutyLt=16
   { 621, 512,1389},  //duty=7, dutyLt=16
   { 636, 512,1337},  //duty=8, dutyLt=16
   { 652, 512,1289},  //duty=9, dutyLt=16
   { 663, 512,1255},  //duty=10, dutyLt=16
   { 674, 512,1225},  //duty=11, dutyLt=16
   { 683, 512,1201},  //duty=12, dutyLt=16
   { 512, 512, 512},  //duty=13, dutyLt=16
   { 512, 512, 512},  //duty=14, dutyLt=16
   { 512, 512, 512},  //duty=15, dutyLt=16
   { 512, 512, 512},  //duty=16, dutyLt=16
   { 512, 512, 512},  //duty=17, dutyLt=16
   { 512, 512, 512},  //duty=18, dutyLt=16
   { 512, 512, 512},  //duty=19, dutyLt=16
   { 512, 512, 512},  //duty=20, dutyLt=16
   { 512, 512, 512},  //duty=21, dutyLt=16
   { 512, 512, 512},  //duty=22, dutyLt=16
   { 512, 512, 512},  //duty=23, dutyLt=16
   { 512, 512, 512},  //duty=24, dutyLt=16
   { 512, 512, 512},  //duty=25, dutyLt=16
   { 523, 512,1906},  //duty=-1, dutyLt=17
   { 538, 512,1795},  //duty=0, dutyLt=17
   { 552, 512,1705},  //duty=1, dutyLt=17
   { 564, 512,1632},  //duty=2, dutyLt=17
   { 576, 512,1573},  //duty=3, dutyLt=17
   { 586, 512,1524},  //duty=4, dutyLt=17
   { 596, 512,1481},  //duty=5, dutyLt=17
   { 605, 512,1446},  //duty=6, dutyLt=17
   { 617, 512,1398},  //duty=7, dutyLt=17
   { 632, 512,1348},  //duty=8, dutyLt=17
   { 647, 512,1297},  //duty=9, dutyLt=17
   { 659, 512,1264},  //duty=10, dutyLt=17
   { 669, 512,1234},  //duty=11, dutyLt=17
   { 512, 512, 512},  //duty=12, dutyLt=17
   { 512, 512, 512},  //duty=13, dutyLt=17
   { 512, 512, 512},  //duty=14, dutyLt=17
   { 512, 512, 512},  //duty=15, dutyLt=17
   { 512, 512, 512},  //duty=16, dutyLt=17
   { 512, 512, 512},  //duty=17, dutyLt=17
   { 512, 512, 512},  //duty=18, dutyLt=17
   { 512, 512, 512},  //duty=19, dutyLt=17
   { 512, 512, 512},  //duty=20, dutyLt=17
   { 512, 512, 512},  //duty=21, dutyLt=17
   { 512, 512, 512},  //duty=22, dutyLt=17
   { 512, 512, 512},  //duty=23, dutyLt=17
   { 512, 512, 512},  //duty=24, dutyLt=17
   { 512, 512, 512},  //duty=25, dutyLt=17
   { 523, 512,1903},  //duty=-1, dutyLt=18
   { 537, 512,1797},  //duty=0, dutyLt=18
   { 550, 512,1710},  //duty=1, dutyLt=18
   { 563, 512,1639},  //duty=2, dutyLt=18
   { 574, 512,1581},  //duty=3, dutyLt=18
   { 584, 512,1532},  //duty=4, dutyLt=18
   { 593, 512,1491},  //duty=5, dutyLt=18
   { 602, 512,1455},  //duty=6, dutyLt=18
   { 614, 512,1407},  //duty=7, dutyLt=18
   { 628, 512,1358},  //duty=8, dutyLt=18
   { 644, 512,1307},  //duty=9, dutyLt=18
   { 654, 512,1273},  //duty=10, dutyLt=18
   { 512, 512, 512},  //duty=11, dutyLt=18
   { 512, 512, 512},  //duty=12, dutyLt=18
   { 512, 512, 512},  //duty=13, dutyLt=18
   { 512, 512, 512},  //duty=14, dutyLt=18
   { 512, 512, 512},  //duty=15, dutyLt=18
   { 512, 512, 512},  //duty=16, dutyLt=18
   { 512, 512, 512},  //duty=17, dutyLt=18
   { 512, 512, 512},  //duty=18, dutyLt=18
   { 512, 512, 512},  //duty=19, dutyLt=18
   { 512, 512, 512},  //duty=20, dutyLt=18
   { 512, 512, 512},  //duty=21, dutyLt=18
   { 512, 512, 512},  //duty=22, dutyLt=18
   { 512, 512, 512},  //duty=23, dutyLt=18
   { 512, 512, 512},  //duty=24, dutyLt=18
   { 512, 512, 512},  //duty=25, dutyLt=18
   { 523, 512,1899},  //duty=-1, dutyLt=19
   { 537, 512,1796},  //duty=0, dutyLt=19
   { 550, 512,1711},  //duty=1, dutyLt=19
   { 561, 512,1643},  //duty=2, dutyLt=19
   { 572, 512,1587},  //duty=3, dutyLt=19
   { 582, 512,1538},  //duty=4, dutyLt=19
   { 591, 512,1497},  //duty=5, dutyLt=19
   { 600, 512,1462},  //duty=6, dutyLt=19
   { 611, 512,1415},  //duty=7, dutyLt=19
   { 625, 512,1365},  //duty=8, dutyLt=19
   { 640, 512,1315},  //duty=9, dutyLt=19
   { 512, 512, 512},  //duty=10, dutyLt=19
   { 512, 512, 512},  //duty=11, dutyLt=19
   { 512, 512, 512},  //duty=12, dutyLt=19
   { 512, 512, 512},  //duty=13, dutyLt=19
   { 512, 512, 512},  //duty=14, dutyLt=19
   { 512, 512, 512},  //duty=15, dutyLt=19
   { 512, 512, 512},  //duty=16, dutyLt=19
   { 512, 512, 512},  //duty=17, dutyLt=19
   { 512, 512, 512},  //duty=18, dutyLt=19
   { 512, 512, 512},  //duty=19, dutyLt=19
   { 512, 512, 512},  //duty=20, dutyLt=19
   { 512, 512, 512},  //duty=21, dutyLt=19
   { 512, 512, 512},  //duty=22, dutyLt=19
   { 512, 512, 512},  //duty=23, dutyLt=19
   { 512, 512, 512},  //duty=24, dutyLt=19
   { 512, 512, 512},  //duty=25, dutyLt=19
   { 524, 512,1894},  //duty=-1, dutyLt=20
   { 537, 512,1796},  //duty=0, dutyLt=20
   { 549, 512,1714},  //duty=1, dutyLt=20
   { 560, 512,1649},  //duty=2, dutyLt=20
   { 570, 512,1595},  //duty=3, dutyLt=20
   { 580, 512,1546},  //duty=4, dutyLt=20
   { 588, 512,1507},  //duty=5, dutyLt=20
   { 597, 512,1470},  //duty=6, dutyLt=20
   { 608, 512,1425},  //duty=7, dutyLt=20
   { 622, 512,1375},  //duty=8, dutyLt=20
   { 512, 512, 512},  //duty=9, dutyLt=20
   { 512, 512, 512},  //duty=10, dutyLt=20
   { 512, 512, 512},  //duty=11, dutyLt=20
   { 512, 512, 512},  //duty=12, dutyLt=20
   { 512, 512, 512},  //duty=13, dutyLt=20
   { 512, 512, 512},  //duty=14, dutyLt=20
   { 512, 512, 512},  //duty=15, dutyLt=20
   { 512, 512, 512},  //duty=16, dutyLt=20
   { 512, 512, 512},  //duty=17, dutyLt=20
   { 512, 512, 512},  //duty=18, dutyLt=20
   { 512, 512, 512},  //duty=19, dutyLt=20
   { 512, 512, 512},  //duty=20, dutyLt=20
   { 512, 512, 512},  //duty=21, dutyLt=20
   { 512, 512, 512},  //duty=22, dutyLt=20
   { 512, 512, 512},  //duty=23, dutyLt=20
   { 512, 512, 512},  //duty=24, dutyLt=20
   { 512, 512, 512},  //duty=25, dutyLt=20
   { 524, 512,1890},  //duty=-1, dutyLt=21
   { 537, 512,1795},  //duty=0, dutyLt=21
   { 549, 512,1717},  //duty=1, dutyLt=21
   { 560, 512,1651},  //duty=2, dutyLt=21
   { 569, 512,1599},  //duty=3, dutyLt=21
   { 578, 512,1552},  //duty=4, dutyLt=21
   { 586, 512,1514},  //duty=5, dutyLt=21
   { 595, 512,1478},  //duty=6, dutyLt=21
   { 606, 512,1433},  //duty=7, dutyLt=21
   { 512, 512, 512},  //duty=8, dutyLt=21
   { 512, 512, 512},  //duty=9, dutyLt=21
   { 512, 512, 512},  //duty=10, dutyLt=21
   { 512, 512, 512},  //duty=11, dutyLt=21
   { 512, 512, 512},  //duty=12, dutyLt=21
   { 512, 512, 512},  //duty=13, dutyLt=21
   { 512, 512, 512},  //duty=14, dutyLt=21
   { 512, 512, 512},  //duty=15, dutyLt=21
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
   { 525, 512,1885},  //duty=-1, dutyLt=22
   { 537, 512,1793},  //duty=0, dutyLt=22
   { 548, 512,1717},  //duty=1, dutyLt=22
   { 559, 512,1654},  //duty=2, dutyLt=22
   { 569, 512,1601},  //duty=3, dutyLt=22
   { 577, 512,1555},  //duty=4, dutyLt=22
   { 585, 512,1518},  //duty=5, dutyLt=22
   { 512, 512, 512},  //duty=6, dutyLt=22
   { 512, 512, 512},  //duty=7, dutyLt=22
   { 512, 512, 512},  //duty=8, dutyLt=22
   { 512, 512, 512},  //duty=9, dutyLt=22
   { 512, 512, 512},  //duty=10, dutyLt=22
   { 512, 512, 512},  //duty=11, dutyLt=22
   { 512, 512, 512},  //duty=12, dutyLt=22
   { 512, 512, 512},  //duty=13, dutyLt=22
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
   { 525, 512,1883},  //duty=-1, dutyLt=23
   { 537, 512,1794},  //duty=0, dutyLt=23
   { 548, 512,1718},  //duty=1, dutyLt=23
   { 558, 512,1658},  //duty=2, dutyLt=23
   { 567, 512,1607},  //duty=3, dutyLt=23
   { 512, 512, 512},  //duty=4, dutyLt=23
   { 512, 512, 512},  //duty=5, dutyLt=23
   { 512, 512, 512},  //duty=6, dutyLt=23
   { 512, 512, 512},  //duty=7, dutyLt=23
   { 512, 512, 512},  //duty=8, dutyLt=23
   { 512, 512, 512},  //duty=9, dutyLt=23
   { 512, 512, 512},  //duty=10, dutyLt=23
   { 512, 512, 512},  //duty=11, dutyLt=23
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
   { 525, 512,1878},  //duty=-1, dutyLt=24
   { 536, 512,1793},  //duty=0, dutyLt=24
   { 547, 512,1721},  //duty=1, dutyLt=24
   { 512, 512, 512},  //duty=2, dutyLt=24
   { 512, 512, 512},  //duty=3, dutyLt=24
   { 512, 512, 512},  //duty=4, dutyLt=24
   { 512, 512, 512},  //duty=5, dutyLt=24
   { 512, 512, 512},  //duty=6, dutyLt=24
   { 512, 512, 512},  //duty=7, dutyLt=24
   { 512, 512, 512},  //duty=8, dutyLt=24
   { 512, 512, 512},  //duty=9, dutyLt=24
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
   { 525, 512,1873},  //duty=-1, dutyLt=25
   { 512, 512, 512},  //duty=0, dutyLt=25
   { 512, 512, 512},  //duty=1, dutyLt=25
   { 512, 512, 512},  //duty=2, dutyLt=25
   { 512, 512, 512},  //duty=3, dutyLt=25
   { 512, 512, 512},  //duty=4, dutyLt=25
   { 512, 512, 512},  //duty=5, dutyLt=25
   { 512, 512, 512},  //duty=6, dutyLt=25
   { 512, 512, 512},  //duty=7, dutyLt=25
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
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512},  //duty=25, dutyLt=25
   { 512, 512, 512}
}}
