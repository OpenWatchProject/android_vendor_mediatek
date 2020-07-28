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
    {1226, 512, 840},  //duty=0, dutyLt=-1
    {1222, 512, 834},  //duty=1, dutyLt=-1
    {1222, 512, 830},  //duty=2, dutyLt=-1
    {1222, 512, 827},  //duty=3, dutyLt=-1
    {1222, 512, 823},  //duty=4, dutyLt=-1
    {1220, 512, 819},  //duty=5, dutyLt=-1
    {1220, 512, 809},  //duty=6, dutyLt=-1
    {1219, 512, 812},  //duty=7, dutyLt=-1
    {1220, 512, 810},  //duty=8, dutyLt=-1
    {1219, 512, 808},  //duty=9, dutyLt=-1
    {1219, 512, 805},  //duty=10, dutyLt=-1
    {1220, 512, 804},  //duty=11, dutyLt=-1
    {1219, 512, 801},  //duty=12, dutyLt=-1
    {1219, 512, 799},  //duty=13, dutyLt=-1
    {1219, 512, 796},  //duty=14, dutyLt=-1
    {1219, 512, 794},  //duty=15, dutyLt=-1
    {1219, 512, 790},  //duty=16, dutyLt=-1
    {1218, 512, 786},  //duty=17, dutyLt=-1
    {1218, 512, 782},  //duty=18, dutyLt=-1
    {1217, 512, 778},  //duty=19, dutyLt=-1
    {1217, 512, 775},  //duty=20, dutyLt=-1
    {1216, 512, 771},  //duty=21, dutyLt=-1
    {1216, 512, 767},  //duty=22, dutyLt=-1
    {1216, 512, 764},  //duty=23, dutyLt=-1
    {1216, 512, 760},  //duty=24, dutyLt=-1
    {1216, 512, 757},  //duty=25, dutyLt=-1
    {1216, 512, 754},  //duty=26, dutyLt=-1
    {1216, 512, 750},  //duty=27, dutyLt=-1
    {1217, 512, 747},  //duty=28, dutyLt=-1
    {1217, 512, 743},  //duty=29, dutyLt=-1
    { 573, 512,1805},  //duty=-1, dutyLt=0
    { 863, 512,1048},  //duty=0, dutyLt=0
    { 969, 512, 951},  //duty=1, dutyLt=0
    {1026, 512, 913},  //duty=2, dutyLt=0
    {1060, 512, 892},  //duty=3, dutyLt=0
    {1083, 512, 876},  //duty=4, dutyLt=0
    {1099, 512, 863},  //duty=5, dutyLt=0
    {1110, 512, 854},  //duty=6, dutyLt=0
    {1120, 512, 847},  //duty=7, dutyLt=0
    {1129, 512, 842},  //duty=8, dutyLt=0
    {1135, 512, 838},  //duty=9, dutyLt=0
    {1141, 512, 833},  //duty=10, dutyLt=0
    {1145, 512, 829},  //duty=11, dutyLt=0
    {1150, 512, 826},  //duty=12, dutyLt=0
    {1154, 512, 822},  //duty=13, dutyLt=0
    {1156, 512, 819},  //duty=14, dutyLt=0
    {1159, 512, 815},  //duty=15, dutyLt=0
    {1165, 512, 809},  //duty=16, dutyLt=0
    {1169, 512, 803},  //duty=17, dutyLt=0
    {1171, 512, 798},  //duty=18, dutyLt=0
    {1173, 512, 794},  //duty=19, dutyLt=0
    {1175, 512, 789},  //duty=20, dutyLt=0
    {1176, 512, 785},  //duty=21, dutyLt=0
    {1178, 512, 781},  //duty=22, dutyLt=0
    {1178, 512, 777},  //duty=23, dutyLt=0
    {1180, 512, 773},  //duty=24, dutyLt=0
    {1181, 512, 769},  //duty=25, dutyLt=0
    {1182, 512, 766},  //duty=26, dutyLt=0
    {1183, 512, 762},  //duty=27, dutyLt=0
    {1184, 512, 759},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 573, 512,1796},  //duty=-1, dutyLt=1
    { 757, 512,1175},  //duty=0, dutyLt=1
    { 859, 512,1042},  //duty=1, dutyLt=1
    { 921, 512, 982},  //duty=2, dutyLt=1
    { 963, 512, 947},  //duty=3, dutyLt=1
    { 994, 512, 921},  //duty=4, dutyLt=1
    {1016, 512, 903},  //duty=5, dutyLt=1
    {1034, 512, 891},  //duty=6, dutyLt=1
    {1049, 512, 881},  //duty=7, dutyLt=1
    {1062, 512, 872},  //duty=8, dutyLt=1
    {1072, 512, 866},  //duty=9, dutyLt=1
    {1081, 512, 859},  //duty=10, dutyLt=1
    {1089, 512, 853},  //duty=11, dutyLt=1
    {1095, 512, 848},  //duty=12, dutyLt=1
    {1101, 512, 843},  //duty=13, dutyLt=1
    {1106, 512, 839},  //duty=14, dutyLt=1
    {1111, 512, 834},  //duty=15, dutyLt=1
    {1119, 512, 827},  //duty=16, dutyLt=1
    {1125, 512, 820},  //duty=17, dutyLt=1
    {1130, 512, 814},  //duty=18, dutyLt=1
    {1134, 512, 808},  //duty=19, dutyLt=1
    {1138, 512, 803},  //duty=20, dutyLt=1
    {1141, 512, 798},  //duty=21, dutyLt=1
    {1143, 512, 793},  //duty=22, dutyLt=1
    {1145, 512, 789},  //duty=23, dutyLt=1
    {1148, 512, 785},  //duty=24, dutyLt=1
    {1150, 512, 781},  //duty=25, dutyLt=1
    {1152, 512, 777},  //duty=26, dutyLt=1
    {1153, 512, 773},  //duty=27, dutyLt=1
    {1155, 512, 769},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 572, 512,1788},  //duty=-1, dutyLt=2
    { 710, 512,1263},  //duty=0, dutyLt=2
    { 798, 512,1113},  //duty=1, dutyLt=2
    { 857, 512,1038},  //duty=2, dutyLt=2
    { 900, 512, 992},  //duty=3, dutyLt=2
    { 934, 512, 953},  //duty=4, dutyLt=2
    { 958, 512, 939},  //duty=5, dutyLt=2
    { 978, 512, 924},  //duty=6, dutyLt=2
    { 996, 512, 910},  //duty=7, dutyLt=2
    {1011, 512, 899},  //duty=8, dutyLt=2
    {1023, 512, 890},  //duty=9, dutyLt=2
    {1034, 512, 882},  //duty=10, dutyLt=2
    {1043, 512, 875},  //duty=11, dutyLt=2
    {1052, 512, 868},  //duty=12, dutyLt=2
    {1060, 512, 862},  //duty=13, dutyLt=2
    {1066, 512, 857},  //duty=14, dutyLt=2
    {1072, 512, 852},  //duty=15, dutyLt=2
    {1082, 512, 843},  //duty=16, dutyLt=2
    {1090, 512, 835},  //duty=17, dutyLt=2
    {1096, 512, 828},  //duty=18, dutyLt=2
    {1102, 512, 821},  //duty=19, dutyLt=2
    {1107, 512, 815},  //duty=20, dutyLt=2
    {1111, 512, 810},  //duty=21, dutyLt=2
    {1115, 512, 804},  //duty=22, dutyLt=2
    {1118, 512, 799},  //duty=23, dutyLt=2
    {1121, 512, 795},  //duty=24, dutyLt=2
    {1123, 512, 790},  //duty=25, dutyLt=2
    {1126, 512, 786},  //duty=26, dutyLt=2
    {1128, 512, 782},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 572, 512,1780},  //duty=-1, dutyLt=3
    { 682, 512,1327},  //duty=0, dutyLt=3
    { 758, 512,1168},  //duty=1, dutyLt=3
    { 814, 512,1085},  //duty=2, dutyLt=3
    { 856, 512,1030},  //duty=3, dutyLt=3
    { 888, 512, 996},  //duty=4, dutyLt=3
    { 913, 512, 972},  //duty=5, dutyLt=3
    { 935, 512, 952},  //duty=6, dutyLt=3
    { 955, 512, 938},  //duty=7, dutyLt=3
    { 971, 512, 925},  //duty=8, dutyLt=3
    { 985, 512, 913},  //duty=9, dutyLt=3
    { 996, 512, 904},  //duty=10, dutyLt=3
    {1006, 512, 895},  //duty=11, dutyLt=3
    {1016, 512, 888},  //duty=12, dutyLt=3
    {1024, 512, 880},  //duty=13, dutyLt=3
    {1031, 512, 874},  //duty=14, dutyLt=3
    {1038, 512, 868},  //duty=15, dutyLt=3
    {1050, 512, 858},  //duty=16, dutyLt=3
    {1060, 512, 849},  //duty=17, dutyLt=3
    {1068, 512, 841},  //duty=18, dutyLt=3
    {1074, 512, 833},  //duty=19, dutyLt=3
    {1080, 512, 827},  //duty=20, dutyLt=3
    {1085, 512, 821},  //duty=21, dutyLt=3
    {1089, 512, 815},  //duty=22, dutyLt=3
    {1093, 512, 809},  //duty=23, dutyLt=3
    {1097, 512, 804},  //duty=24, dutyLt=3
    {1101, 512, 800},  //duty=25, dutyLt=3
    {1103, 512, 795},  //duty=26, dutyLt=3
    {1106, 512, 791},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 571, 512,1777},  //duty=-1, dutyLt=4
    { 664, 512,1376},  //duty=0, dutyLt=4
    { 732, 512,1213},  //duty=1, dutyLt=4
    { 783, 512,1123},  //duty=2, dutyLt=4
    { 823, 512,1064},  //duty=3, dutyLt=4
    { 854, 512,1027},  //duty=4, dutyLt=4
    { 881, 512,1000},  //duty=5, dutyLt=4
    { 903, 512, 979},  //duty=6, dutyLt=4
    { 922, 512, 961},  //duty=7, dutyLt=4
    { 938, 512, 947},  //duty=8, dutyLt=4
    { 952, 512, 934},  //duty=9, dutyLt=4
    { 965, 512, 923},  //duty=10, dutyLt=4
    { 976, 512, 913},  //duty=11, dutyLt=4
    { 986, 512, 905},  //duty=12, dutyLt=4
    { 995, 512, 897},  //duty=13, dutyLt=4
    {1003, 512, 890},  //duty=14, dutyLt=4
    {1010, 512, 883},  //duty=15, dutyLt=4
    {1023, 512, 871},  //duty=16, dutyLt=4
    {1034, 512, 861},  //duty=17, dutyLt=4
    {1042, 512, 852},  //duty=18, dutyLt=4
    {1050, 512, 845},  //duty=19, dutyLt=4
    {1057, 512, 838},  //duty=20, dutyLt=4
    {1062, 512, 831},  //duty=21, dutyLt=4
    {1067, 512, 825},  //duty=22, dutyLt=4
    {1072, 512, 819},  //duty=23, dutyLt=4
    {1076, 512, 814},  //duty=24, dutyLt=4
    {1080, 512, 808},  //duty=25, dutyLt=4
    {1083, 512, 804},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 571, 512,1773},  //duty=-1, dutyLt=5
    { 651, 512,1411},  //duty=0, dutyLt=5
    { 712, 512,1250},  //duty=1, dutyLt=5
    { 760, 512,1153},  //duty=2, dutyLt=5
    { 796, 512,1096},  //duty=3, dutyLt=5
    { 827, 512,1055},  //duty=4, dutyLt=5
    { 853, 512,1026},  //duty=5, dutyLt=5
    { 875, 512,1003},  //duty=6, dutyLt=5
    { 894, 512, 983},  //duty=7, dutyLt=5
    { 911, 512, 967},  //duty=8, dutyLt=5
    { 925, 512, 953},  //duty=9, dutyLt=5
    { 938, 512, 941},  //duty=10, dutyLt=5
    { 950, 512, 931},  //duty=11, dutyLt=5
    { 960, 512, 921},  //duty=12, dutyLt=5
    { 969, 512, 912},  //duty=13, dutyLt=5
    { 977, 512, 904},  //duty=14, dutyLt=5
    { 985, 512, 897},  //duty=15, dutyLt=5
    {1000, 512, 884},  //duty=16, dutyLt=5
    {1011, 512, 873},  //duty=17, dutyLt=5
    {1020, 512, 864},  //duty=18, dutyLt=5
    {1029, 512, 855},  //duty=19, dutyLt=5
    {1036, 512, 848},  //duty=20, dutyLt=5
    {1042, 512, 840},  //duty=21, dutyLt=5
    {1048, 512, 834},  //duty=22, dutyLt=5
    {1052, 512, 828},  //duty=23, dutyLt=5
    {1057, 512, 822},  //duty=24, dutyLt=5
    {1061, 512, 817},  //duty=25, dutyLt=5
    {1065, 512, 812},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 571, 512,1771},  //duty=-1, dutyLt=6
    { 642, 512,1440},  //duty=0, dutyLt=6
    { 697, 512,1279},  //duty=1, dutyLt=6
    { 742, 512,1181},  //duty=2, dutyLt=6
    { 777, 512,1123},  //duty=3, dutyLt=6
    { 806, 512,1080},  //duty=4, dutyLt=6
    { 831, 512,1050},  //duty=5, dutyLt=6
    { 853, 512,1025},  //duty=6, dutyLt=6
    { 872, 512,1003},  //duty=7, dutyLt=6
    { 888, 512, 986},  //duty=8, dutyLt=6
    { 902, 512, 971},  //duty=9, dutyLt=6
    { 915, 512, 957},  //duty=10, dutyLt=6
    { 927, 512, 947},  //duty=11, dutyLt=6
    { 938, 512, 936},  //duty=12, dutyLt=6
    { 948, 512, 927},  //duty=13, dutyLt=6
    { 956, 512, 918},  //duty=14, dutyLt=6
    { 964, 512, 910},  //duty=15, dutyLt=6
    { 979, 512, 896},  //duty=16, dutyLt=6
    { 991, 512, 884},  //duty=17, dutyLt=6
    {1001, 512, 874},  //duty=18, dutyLt=6
    {1009, 512, 865},  //duty=19, dutyLt=6
    {1017, 512, 857},  //duty=20, dutyLt=6
    {1024, 512, 850},  //duty=21, dutyLt=6
    {1030, 512, 843},  //duty=22, dutyLt=6
    {1035, 512, 836},  //duty=23, dutyLt=6
    {1040, 512, 830},  //duty=24, dutyLt=6
    {1044, 512, 825},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 571, 512,1768},  //duty=-1, dutyLt=7
    { 634, 512,1462},  //duty=0, dutyLt=7
    { 687, 512,1298},  //duty=1, dutyLt=7
    { 726, 512,1207},  //duty=2, dutyLt=7
    { 760, 512,1147},  //duty=3, dutyLt=7
    { 788, 512,1103},  //duty=4, dutyLt=7
    { 813, 512,1071},  //duty=5, dutyLt=7
    { 833, 512,1044},  //duty=6, dutyLt=7
    { 853, 512,1021},  //duty=7, dutyLt=7
    { 868, 512,1003},  //duty=8, dutyLt=7
    { 883, 512, 987},  //duty=9, dutyLt=7
    { 896, 512, 973},  //duty=10, dutyLt=7
    { 908, 512, 961},  //duty=11, dutyLt=7
    { 919, 512, 950},  //duty=12, dutyLt=7
    { 928, 512, 940},  //duty=13, dutyLt=7
    { 937, 512, 931},  //duty=14, dutyLt=7
    { 945, 512, 923},  //duty=15, dutyLt=7
    { 960, 512, 907},  //duty=16, dutyLt=7
    { 972, 512, 895},  //duty=17, dutyLt=7
    { 983, 512, 884},  //duty=18, dutyLt=7
    { 992, 512, 875},  //duty=19, dutyLt=7
    {1000, 512, 866},  //duty=20, dutyLt=7
    {1007, 512, 859},  //duty=21, dutyLt=7
    {1013, 512, 851},  //duty=22, dutyLt=7
    {1019, 512, 845},  //duty=23, dutyLt=7
    {1024, 512, 838},  //duty=24, dutyLt=7
    {1029, 512, 832},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 571, 512,1764},  //duty=-1, dutyLt=8
    { 629, 512,1479},  //duty=0, dutyLt=8
    { 676, 512,1322},  //duty=1, dutyLt=8
    { 715, 512,1230},  //duty=2, dutyLt=8
    { 747, 512,1168},  //duty=3, dutyLt=8
    { 774, 512,1124},  //duty=4, dutyLt=8
    { 797, 512,1090},  //duty=5, dutyLt=8
    { 817, 512,1062},  //duty=6, dutyLt=8
    { 836, 512,1039},  //duty=7, dutyLt=8
    { 851, 512,1019},  //duty=8, dutyLt=8
    { 866, 512,1002},  //duty=9, dutyLt=8
    { 879, 512, 987},  //duty=10, dutyLt=8
    { 891, 512, 975},  //duty=11, dutyLt=8
    { 901, 512, 963},  //duty=12, dutyLt=8
    { 911, 512, 952},  //duty=13, dutyLt=8
    { 920, 512, 943},  //duty=14, dutyLt=8
    { 928, 512, 934},  //duty=15, dutyLt=8
    { 944, 512, 918},  //duty=16, dutyLt=8
    { 956, 512, 905},  //duty=17, dutyLt=8
    { 968, 512, 894},  //duty=18, dutyLt=8
    { 977, 512, 884},  //duty=19, dutyLt=8
    { 985, 512, 875},  //duty=20, dutyLt=8
    { 993, 512, 866},  //duty=21, dutyLt=8
    { 999, 512, 859},  //duty=22, dutyLt=8
    {1005, 512, 852},  //duty=23, dutyLt=8
    {1010, 512, 846},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 571, 512,1760},  //duty=-1, dutyLt=9
    { 626, 512,1488},  //duty=0, dutyLt=9
    { 669, 512,1342},  //duty=1, dutyLt=9
    { 705, 512,1249},  //duty=2, dutyLt=9
    { 735, 512,1188},  //duty=3, dutyLt=9
    { 762, 512,1143},  //duty=4, dutyLt=9
    { 784, 512,1107},  //duty=5, dutyLt=9
    { 804, 512,1079},  //duty=6, dutyLt=9
    { 821, 512,1055},  //duty=7, dutyLt=9
    { 837, 512,1034},  //duty=8, dutyLt=9
    { 851, 512,1017},  //duty=9, dutyLt=9
    { 864, 512,1001},  //duty=10, dutyLt=9
    { 876, 512, 988},  //duty=11, dutyLt=9
    { 887, 512, 976},  //duty=12, dutyLt=9
    { 896, 512, 964},  //duty=13, dutyLt=9
    { 906, 512, 955},  //duty=14, dutyLt=9
    { 913, 512, 945},  //duty=15, dutyLt=9
    { 930, 512, 928},  //duty=16, dutyLt=9
    { 942, 512, 914},  //duty=17, dutyLt=9
    { 953, 512, 903},  //duty=18, dutyLt=9
    { 963, 512, 892},  //duty=19, dutyLt=9
    { 971, 512, 883},  //duty=20, dutyLt=9
    { 979, 512, 874},  //duty=21, dutyLt=9
    { 986, 512, 866},  //duty=22, dutyLt=9
    { 992, 512, 859},  //duty=23, dutyLt=9
    { 997, 512, 853},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 572, 512,1754},  //duty=-1, dutyLt=10
    { 622, 512,1494},  //duty=0, dutyLt=10
    { 662, 512,1356},  //duty=1, dutyLt=10
    { 697, 512,1268},  //duty=2, dutyLt=10
    { 725, 512,1205},  //duty=3, dutyLt=10
    { 751, 512,1159},  //duty=4, dutyLt=10
    { 772, 512,1124},  //duty=5, dutyLt=10
    { 791, 512,1093},  //duty=6, dutyLt=10
    { 809, 512,1068},  //duty=7, dutyLt=10
    { 824, 512,1047},  //duty=8, dutyLt=10
    { 839, 512,1030},  //duty=9, dutyLt=10
    { 851, 512,1013},  //duty=10, dutyLt=10
    { 863, 512, 999},  //duty=11, dutyLt=10
    { 873, 512, 987},  //duty=12, dutyLt=10
    { 883, 512, 975},  //duty=13, dutyLt=10
    { 892, 512, 965},  //duty=14, dutyLt=10
    { 900, 512, 955},  //duty=15, dutyLt=10
    { 917, 512, 938},  //duty=16, dutyLt=10
    { 929, 512, 924},  //duty=17, dutyLt=10
    { 940, 512, 911},  //duty=18, dutyLt=10
    { 950, 512, 901},  //duty=19, dutyLt=10
    { 959, 512, 891},  //duty=20, dutyLt=10
    { 966, 512, 882},  //duty=21, dutyLt=10
    { 973, 512, 873},  //duty=22, dutyLt=10
    { 980, 512, 866},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 572, 512,1747},  //duty=-1, dutyLt=11
    { 618, 512,1506},  //duty=0, dutyLt=11
    { 657, 512,1373},  //duty=1, dutyLt=11
    { 689, 512,1283},  //duty=2, dutyLt=11
    { 717, 512,1222},  //duty=3, dutyLt=11
    { 741, 512,1174},  //duty=4, dutyLt=11
    { 762, 512,1138},  //duty=5, dutyLt=11
    { 781, 512,1108},  //duty=6, dutyLt=11
    { 798, 512,1081},  //duty=7, dutyLt=11
    { 813, 512,1061},  //duty=8, dutyLt=11
    { 827, 512,1041},  //duty=9, dutyLt=11
    { 839, 512,1025},  //duty=10, dutyLt=11
    { 851, 512,1010},  //duty=11, dutyLt=11
    { 861, 512, 997},  //duty=12, dutyLt=11
    { 871, 512, 985},  //duty=13, dutyLt=11
    { 880, 512, 974},  //duty=14, dutyLt=11
    { 888, 512, 965},  //duty=15, dutyLt=11
    { 904, 512, 946},  //duty=16, dutyLt=11
    { 917, 512, 932},  //duty=17, dutyLt=11
    { 929, 512, 919},  //duty=18, dutyLt=11
    { 938, 512, 908},  //duty=19, dutyLt=11
    { 947, 512, 898},  //duty=20, dutyLt=11
    { 955, 512, 889},  //duty=21, dutyLt=11
    { 962, 512, 881},  //duty=22, dutyLt=11
    { 968, 512, 873},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 573, 512,1730},  //duty=-1, dutyLt=12
    { 616, 512,1515},  //duty=0, dutyLt=12
    { 652, 512,1386},  //duty=1, dutyLt=12
    { 683, 512,1298},  //duty=2, dutyLt=12
    { 710, 512,1235},  //duty=3, dutyLt=12
    { 733, 512,1189},  //duty=4, dutyLt=12
    { 753, 512,1151},  //duty=5, dutyLt=12
    { 772, 512,1120},  //duty=6, dutyLt=12
    { 788, 512,1095},  //duty=7, dutyLt=12
    { 803, 512,1072},  //duty=8, dutyLt=12
    { 816, 512,1053},  //duty=9, dutyLt=12
    { 829, 512,1036},  //duty=10, dutyLt=12
    { 840, 512,1021},  //duty=11, dutyLt=12
    { 851, 512,1007},  //duty=12, dutyLt=12
    { 860, 512, 995},  //duty=13, dutyLt=12
    { 869, 512, 984},  //duty=14, dutyLt=12
    { 877, 512, 974},  //duty=15, dutyLt=12
    { 894, 512, 955},  //duty=16, dutyLt=12
    { 906, 512, 940},  //duty=17, dutyLt=12
    { 918, 512, 927},  //duty=18, dutyLt=12
    { 928, 512, 915},  //duty=19, dutyLt=12
    { 937, 512, 905},  //duty=20, dutyLt=12
    { 944, 512, 896},  //duty=21, dutyLt=12
    { 952, 512, 887},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 512, 512, 512},  //duty=28, dutyLt=12
    { 512, 512, 512},  //duty=29, dutyLt=12
    { 573, 512,1729},  //duty=-1, dutyLt=13
    { 613, 512,1524},  //duty=0, dutyLt=13
    { 648, 512,1398},  //duty=1, dutyLt=13
    { 678, 512,1311},  //duty=2, dutyLt=13
    { 703, 512,1249},  //duty=3, dutyLt=13
    { 726, 512,1201},  //duty=4, dutyLt=13
    { 745, 512,1164},  //duty=5, dutyLt=13
    { 763, 512,1132},  //duty=6, dutyLt=13
    { 779, 512,1105},  //duty=7, dutyLt=13
    { 794, 512,1083},  //duty=8, dutyLt=13
    { 807, 512,1063},  //duty=9, dutyLt=13
    { 819, 512,1046},  //duty=10, dutyLt=13
    { 831, 512,1031},  //duty=11, dutyLt=13
    { 841, 512,1017},  //duty=12, dutyLt=13
    { 850, 512,1004},  //duty=13, dutyLt=13
    { 859, 512, 993},  //duty=14, dutyLt=13
    { 867, 512, 983},  //duty=15, dutyLt=13
    { 884, 512, 964},  //duty=16, dutyLt=13
    { 897, 512, 948},  //duty=17, dutyLt=13
    { 908, 512, 934},  //duty=18, dutyLt=13
    { 918, 512, 923},  //duty=19, dutyLt=13
    { 927, 512, 912},  //duty=20, dutyLt=13
    { 935, 512, 902},  //duty=21, dutyLt=13
    { 942, 512, 893},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 573, 512,1726},  //duty=-1, dutyLt=14
    { 612, 512,1530},  //duty=0, dutyLt=14
    { 644, 512,1409},  //duty=1, dutyLt=14
    { 673, 512,1323},  //duty=2, dutyLt=14
    { 698, 512,1261},  //duty=3, dutyLt=14
    { 719, 512,1213},  //duty=4, dutyLt=14
    { 739, 512,1175},  //duty=5, dutyLt=14
    { 756, 512,1143},  //duty=6, dutyLt=14
    { 772, 512,1116},  //duty=7, dutyLt=14
    { 786, 512,1093},  //duty=8, dutyLt=14
    { 799, 512,1073},  //duty=9, dutyLt=14
    { 811, 512,1055},  //duty=10, dutyLt=14
    { 822, 512,1040},  //duty=11, dutyLt=14
    { 832, 512,1026},  //duty=12, dutyLt=14
    { 842, 512,1013},  //duty=13, dutyLt=14
    { 850, 512,1001},  //duty=14, dutyLt=14
    { 858, 512, 990},  //duty=15, dutyLt=14
    { 875, 512, 971},  //duty=16, dutyLt=14
    { 888, 512, 955},  //duty=17, dutyLt=14
    { 899, 512, 941},  //duty=18, dutyLt=14
    { 909, 512, 929},  //duty=19, dutyLt=14
    { 918, 512, 918},  //duty=20, dutyLt=14
    { 926, 512, 908},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 512, 512, 512},  //duty=28, dutyLt=14
    { 512, 512, 512},  //duty=29, dutyLt=14
    { 573, 512,1722},  //duty=-1, dutyLt=15
    { 610, 512,1537},  //duty=0, dutyLt=15
    { 642, 512,1417},  //duty=1, dutyLt=15
    { 669, 512,1333},  //duty=2, dutyLt=15
    { 692, 512,1271},  //duty=3, dutyLt=15
    { 714, 512,1224},  //duty=4, dutyLt=15
    { 732, 512,1185},  //duty=5, dutyLt=15
    { 749, 512,1153},  //duty=6, dutyLt=15
    { 764, 512,1126},  //duty=7, dutyLt=15
    { 779, 512,1103},  //duty=8, dutyLt=15
    { 792, 512,1083},  //duty=9, dutyLt=15
    { 803, 512,1064},  //duty=10, dutyLt=15
    { 814, 512,1048},  //duty=11, dutyLt=15
    { 824, 512,1034},  //duty=12, dutyLt=15
    { 833, 512,1021},  //duty=13, dutyLt=15
    { 842, 512,1009},  //duty=14, dutyLt=15
    { 850, 512, 998},  //duty=15, dutyLt=15
    { 866, 512, 978},  //duty=16, dutyLt=15
    { 879, 512, 962},  //duty=17, dutyLt=15
    { 890, 512, 948},  //duty=18, dutyLt=15
    { 900, 512, 935},  //duty=19, dutyLt=15
    { 909, 512, 924},  //duty=20, dutyLt=15
    { 917, 512, 914},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 574, 512,1718},  //duty=-1, dutyLt=16
    { 607, 512,1547},  //duty=0, dutyLt=16
    { 637, 512,1430},  //duty=1, dutyLt=16
    { 663, 512,1348},  //duty=2, dutyLt=16
    { 685, 512,1287},  //duty=3, dutyLt=16
    { 705, 512,1239},  //duty=4, dutyLt=16
    { 723, 512,1201},  //duty=5, dutyLt=16
    { 739, 512,1168},  //duty=6, dutyLt=16
    { 754, 512,1141},  //duty=7, dutyLt=16
    { 768, 512,1117},  //duty=8, dutyLt=16
    { 780, 512,1096},  //duty=9, dutyLt=16
    { 791, 512,1079},  //duty=10, dutyLt=16
    { 802, 512,1062},  //duty=11, dutyLt=16
    { 811, 512,1048},  //duty=12, dutyLt=16
    { 820, 512,1035},  //duty=13, dutyLt=16
    { 829, 512,1023},  //duty=14, dutyLt=16
    { 837, 512,1011},  //duty=15, dutyLt=16
    { 851, 512, 992},  //duty=16, dutyLt=16
    { 864, 512, 975},  //duty=17, dutyLt=16
    { 875, 512, 960},  //duty=18, dutyLt=16
    { 885, 512, 947},  //duty=19, dutyLt=16
    { 894, 512, 936},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 512, 512, 512},  //duty=28, dutyLt=16
    { 512, 512, 512},  //duty=29, dutyLt=16
    { 574, 512,1714},  //duty=-1, dutyLt=17
    { 605, 512,1556},  //duty=0, dutyLt=17
    { 633, 512,1442},  //duty=1, dutyLt=17
    { 657, 512,1362},  //duty=2, dutyLt=17
    { 678, 512,1304},  //duty=3, dutyLt=17
    { 697, 512,1255},  //duty=4, dutyLt=17
    { 715, 512,1216},  //duty=5, dutyLt=17
    { 730, 512,1184},  //duty=6, dutyLt=17
    { 744, 512,1156},  //duty=7, dutyLt=17
    { 757, 512,1132},  //duty=8, dutyLt=17
    { 769, 512,1111},  //duty=9, dutyLt=17
    { 780, 512,1093},  //duty=10, dutyLt=17
    { 790, 512,1076},  //duty=11, dutyLt=17
    { 799, 512,1062},  //duty=12, dutyLt=17
    { 808, 512,1048},  //duty=13, dutyLt=17
    { 816, 512,1035},  //duty=14, dutyLt=17
    { 825, 512,1024},  //duty=15, dutyLt=17
    { 839, 512,1004},  //duty=16, dutyLt=17
    { 851, 512, 987},  //duty=17, dutyLt=17
    { 862, 512, 971},  //duty=18, dutyLt=17
    { 873, 512, 958},  //duty=19, dutyLt=17
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
    { 574, 512,1712},  //duty=-1, dutyLt=18
    { 603, 512,1560},  //duty=0, dutyLt=18
    { 629, 512,1452},  //duty=1, dutyLt=18
    { 652, 512,1376},  //duty=2, dutyLt=18
    { 672, 512,1316},  //duty=3, dutyLt=18
    { 691, 512,1269},  //duty=4, dutyLt=18
    { 707, 512,1230},  //duty=5, dutyLt=18
    { 722, 512,1198},  //duty=6, dutyLt=18
    { 735, 512,1169},  //duty=7, dutyLt=18
    { 748, 512,1145},  //duty=8, dutyLt=18
    { 760, 512,1124},  //duty=9, dutyLt=18
    { 770, 512,1105},  //duty=10, dutyLt=18
    { 780, 512,1088},  //duty=11, dutyLt=18
    { 789, 512,1074},  //duty=12, dutyLt=18
    { 798, 512,1060},  //duty=13, dutyLt=18
    { 806, 512,1047},  //duty=14, dutyLt=18
    { 814, 512,1035},  //duty=15, dutyLt=18
    { 828, 512,1015},  //duty=16, dutyLt=18
    { 840, 512, 997},  //duty=17, dutyLt=18
    { 851, 512, 981},  //duty=18, dutyLt=18
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
    { 575, 512,1705},  //duty=-1, dutyLt=19
    { 602, 512,1564},  //duty=0, dutyLt=19
    { 627, 512,1462},  //duty=1, dutyLt=19
    { 648, 512,1386},  //duty=2, dutyLt=19
    { 668, 512,1329},  //duty=3, dutyLt=19
    { 685, 512,1280},  //duty=4, dutyLt=19
    { 701, 512,1243},  //duty=5, dutyLt=19
    { 715, 512,1209},  //duty=6, dutyLt=19
    { 728, 512,1181},  //duty=7, dutyLt=19
    { 740, 512,1157},  //duty=8, dutyLt=19
    { 751, 512,1136},  //duty=9, dutyLt=19
    { 762, 512,1116},  //duty=10, dutyLt=19
    { 772, 512,1100},  //duty=11, dutyLt=19
    { 781, 512,1085},  //duty=12, dutyLt=19
    { 789, 512,1070},  //duty=13, dutyLt=19
    { 797, 512,1057},  //duty=14, dutyLt=19
    { 805, 512,1045},  //duty=15, dutyLt=19
    { 818, 512,1024},  //duty=16, dutyLt=19
    { 831, 512,1006},  //duty=17, dutyLt=19
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
    { 575, 512,1701},  //duty=-1, dutyLt=20
    { 601, 512,1566},  //duty=0, dutyLt=20
    { 625, 512,1467},  //duty=1, dutyLt=20
    { 646, 512,1394},  //duty=2, dutyLt=20
    { 664, 512,1337},  //duty=3, dutyLt=20
    { 681, 512,1291},  //duty=4, dutyLt=20
    { 695, 512,1252},  //duty=5, dutyLt=20
    { 709, 512,1220},  //duty=6, dutyLt=20
    { 722, 512,1191},  //duty=7, dutyLt=20
    { 734, 512,1166},  //duty=8, dutyLt=20
    { 745, 512,1145},  //duty=9, dutyLt=20
    { 755, 512,1126},  //duty=10, dutyLt=20
    { 764, 512,1109},  //duty=11, dutyLt=20
    { 773, 512,1094},  //duty=12, dutyLt=20
    { 782, 512,1079},  //duty=13, dutyLt=20
    { 789, 512,1066},  //duty=14, dutyLt=20
    { 797, 512,1054},  //duty=15, dutyLt=20
    { 810, 512,1033},  //duty=16, dutyLt=20
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
    { 576, 512,1694},  //duty=-1, dutyLt=21
    { 600, 512,1568},  //duty=0, dutyLt=21
    { 623, 512,1472},  //duty=1, dutyLt=21
    { 643, 512,1401},  //duty=2, dutyLt=21
    { 661, 512,1345},  //duty=3, dutyLt=21
    { 677, 512,1299},  //duty=4, dutyLt=21
    { 691, 512,1261},  //duty=5, dutyLt=21
    { 705, 512,1228},  //duty=6, dutyLt=21
    { 717, 512,1201},  //duty=7, dutyLt=21
    { 729, 512,1175},  //duty=8, dutyLt=21
    { 739, 512,1154},  //duty=9, dutyLt=21
    { 749, 512,1135},  //duty=10, dutyLt=21
    { 758, 512,1118},  //duty=11, dutyLt=21
    { 767, 512,1102},  //duty=12, dutyLt=21
    { 775, 512,1087},  //duty=13, dutyLt=21
    { 783, 512,1074},  //duty=14, dutyLt=21
    { 790, 512,1062},  //duty=15, dutyLt=21
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
    { 577, 512,1690},  //duty=-1, dutyLt=22
    { 600, 512,1568},  //duty=0, dutyLt=22
    { 622, 512,1476},  //duty=1, dutyLt=22
    { 641, 512,1405},  //duty=2, dutyLt=22
    { 658, 512,1352},  //duty=3, dutyLt=22
    { 673, 512,1306},  //duty=4, dutyLt=22
    { 688, 512,1268},  //duty=5, dutyLt=22
    { 700, 512,1236},  //duty=6, dutyLt=22
    { 712, 512,1207},  //duty=7, dutyLt=22
    { 724, 512,1183},  //duty=8, dutyLt=22
    { 734, 512,1162},  //duty=9, dutyLt=22
    { 743, 512,1143},  //duty=10, dutyLt=22
    { 753, 512,1126},  //duty=11, dutyLt=22
    { 761, 512,1109},  //duty=12, dutyLt=22
    { 769, 512,1095},  //duty=13, dutyLt=22
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
    { 577, 512,1686},  //duty=-1, dutyLt=23
    { 600, 512,1568},  //duty=0, dutyLt=23
    { 621, 512,1479},  //duty=1, dutyLt=23
    { 639, 512,1410},  //duty=2, dutyLt=23
    { 656, 512,1356},  //duty=3, dutyLt=23
    { 671, 512,1312},  //duty=4, dutyLt=23
    { 685, 512,1275},  //duty=5, dutyLt=23
    { 697, 512,1243},  //duty=6, dutyLt=23
    { 709, 512,1214},  //duty=7, dutyLt=23
    { 720, 512,1189},  //duty=8, dutyLt=23
    { 730, 512,1168},  //duty=9, dutyLt=23
    { 739, 512,1149},  //duty=10, dutyLt=23
    { 748, 512,1132},  //duty=11, dutyLt=23
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
    { 578, 512,1681},  //duty=-1, dutyLt=24
    { 599, 512,1569},  //duty=0, dutyLt=24
    { 620, 512,1482},  //duty=1, dutyLt=24
    { 638, 512,1413},  //duty=2, dutyLt=24
    { 654, 512,1362},  //duty=3, dutyLt=24
    { 668, 512,1317},  //duty=4, dutyLt=24
    { 681, 512,1280},  //duty=5, dutyLt=24
    { 694, 512,1248},  //duty=6, dutyLt=24
    { 705, 512,1220},  //duty=7, dutyLt=24
    { 716, 512,1196},  //duty=8, dutyLt=24
    { 726, 512,1174},  //duty=9, dutyLt=24
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
    { 578, 512,1676},  //duty=-1, dutyLt=25
    { 600, 512,1569},  //duty=0, dutyLt=25
    { 619, 512,1482},  //duty=1, dutyLt=25
    { 637, 512,1416},  //duty=2, dutyLt=25
    { 652, 512,1365},  //duty=3, dutyLt=25
    { 667, 512,1321},  //duty=4, dutyLt=25
    { 680, 512,1284},  //duty=5, dutyLt=25
    { 691, 512,1252},  //duty=6, dutyLt=25
    { 703, 512,1224},  //duty=7, dutyLt=25
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
    { 579, 512,1671},  //duty=-1, dutyLt=26
    { 600, 512,1567},  //duty=0, dutyLt=26
    { 619, 512,1483},  //duty=1, dutyLt=26
    { 635, 512,1419},  //duty=2, dutyLt=26
    { 651, 512,1368},  //duty=3, dutyLt=26
    { 665, 512,1324},  //duty=4, dutyLt=26
    { 678, 512,1287},  //duty=5, dutyLt=26
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
    { 580, 512,1666},  //duty=-1, dutyLt=27
    { 600, 512,1564},  //duty=0, dutyLt=27
    { 619, 512,1484},  //duty=1, dutyLt=27
    { 635, 512,1419},  //duty=2, dutyLt=27
    { 650, 512,1370},  //duty=3, dutyLt=27
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
    { 581, 512,1662},  //duty=-1, dutyLt=28
    { 600, 512,1562},  //duty=0, dutyLt=28
    { 619, 512,1483},  //duty=1, dutyLt=28
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
    { 582, 512,1656},  //duty=-1, dutyLt=29
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
}}



