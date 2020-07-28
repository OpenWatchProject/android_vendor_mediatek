// Flash AWB tuning parameter
{
    9, //foreground percentage
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
   {1160, 512, 932},  //duty=0, dutyLt=-1
   {1165, 512, 929},  //duty=1, dutyLt=-1
   {1164, 512, 917},  //duty=2, dutyLt=-1
   {1160, 512, 906},  //duty=3, dutyLt=-1
   {1154, 512, 899},  //duty=4, dutyLt=-1
   {1150, 512, 896},  //duty=5, dutyLt=-1
   {1153, 512, 887},  //duty=6, dutyLt=-1
   {1150, 512, 878},  //duty=7, dutyLt=-1
   {1151, 512, 870},  //duty=8, dutyLt=-1
   {1154, 512, 861},  //duty=9, dutyLt=-1
   {1151, 512, 856},  //duty=10, dutyLt=-1
   {1148, 512, 850},  //duty=11, dutyLt=-1
   {1146, 512, 846},  //duty=12, dutyLt=-1
   {1144, 512, 839},  //duty=13, dutyLt=-1
   {1142, 512, 834},  //duty=14, dutyLt=-1
   {1141, 512, 830},  //duty=15, dutyLt=-1
   {1142, 512, 826},  //duty=16, dutyLt=-1
   {1141, 512, 823},  //duty=17, dutyLt=-1
   {1140, 512, 819},  //duty=18, dutyLt=-1
   {1140, 512, 817},  //duty=19, dutyLt=-1
   {1140, 512, 813},  //duty=20, dutyLt=-1
   {1139, 512, 811},  //duty=21, dutyLt=-1
   {1139, 512, 808},  //duty=22, dutyLt=-1
   {1139, 512, 805},  //duty=23, dutyLt=-1
   {1139, 512, 802},  //duty=24, dutyLt=-1
   {1140, 512, 800},  //duty=25, dutyLt=-1
   { 551, 512,2359},  //duty=-1, dutyLt=0
   { 788, 512,1248},  //duty=0, dutyLt=0
   { 902, 512,1097},  //duty=1, dutyLt=0
   { 955, 512,1032},  //duty=2, dutyLt=0
   { 989, 512, 995},  //duty=3, dutyLt=0
   {1012, 512, 973},  //duty=4, dutyLt=0
   {1030, 512, 957},  //duty=5, dutyLt=0
   {1045, 512, 937},  //duty=6, dutyLt=0
   {1053, 512, 926},  //duty=7, dutyLt=0
   {1071, 512, 910},  //duty=8, dutyLt=0
   {1081, 512, 895},  //duty=9, dutyLt=0
   {1085, 512, 885},  //duty=10, dutyLt=0
   {1089, 512, 877},  //duty=11, dutyLt=0
   {1092, 512, 869},  //duty=12, dutyLt=0
   {1095, 512, 861},  //duty=13, dutyLt=0
   {1098, 512, 855},  //duty=14, dutyLt=0
   {1099, 512, 850},  //duty=15, dutyLt=0
   {1102, 512, 844},  //duty=16, dutyLt=0
   {1103, 512, 841},  //duty=17, dutyLt=0
   {1104, 512, 837},  //duty=18, dutyLt=0
   {1106, 512, 834},  //duty=19, dutyLt=0
   {1107, 512, 829},  //duty=20, dutyLt=0
   {1108, 512, 826},  //duty=21, dutyLt=0
   {1108, 512, 822},  //duty=22, dutyLt=0
   {1109, 512, 820},  //duty=23, dutyLt=0
   {1111, 512, 816},  //duty=24, dutyLt=0
   { 512, 512, 512},  //duty=25, dutyLt=0
   { 548, 512,2246},  //duty=-1, dutyLt=1
   { 696, 512,1443},  //duty=0, dutyLt=1
   { 786, 512,1232},  //duty=1, dutyLt=1
   { 846, 512,1134},  //duty=2, dutyLt=1
   { 892, 512,1075},  //duty=3, dutyLt=1
   { 919, 512,1044},  //duty=4, dutyLt=1
   { 942, 512,1012},  //duty=5, dutyLt=1
   { 962, 512, 989},  //duty=6, dutyLt=1
   { 985, 512, 972},  //duty=7, dutyLt=1
   {1006, 512, 948},  //duty=8, dutyLt=1
   {1023, 512, 927},  //duty=9, dutyLt=1
   {1033, 512, 914},  //duty=10, dutyLt=1
   {1041, 512, 903},  //duty=11, dutyLt=1
   {1046, 512, 893},  //duty=12, dutyLt=1
   {1054, 512, 883},  //duty=13, dutyLt=1
   {1059, 512, 876},  //duty=14, dutyLt=1
   {1063, 512, 870},  //duty=15, dutyLt=1
   {1067, 512, 863},  //duty=16, dutyLt=1
   {1070, 512, 858},  //duty=17, dutyLt=1
   {1072, 512, 853},  //duty=18, dutyLt=1
   {1074, 512, 849},  //duty=19, dutyLt=1
   {1077, 512, 844},  //duty=20, dutyLt=1
   {1079, 512, 840},  //duty=21, dutyLt=1
   {1080, 512, 837},  //duty=22, dutyLt=1
   {1082, 512, 833},  //duty=23, dutyLt=1
   {1084, 512, 829},  //duty=24, dutyLt=1
   { 512, 512, 512},  //duty=25, dutyLt=1
   { 550, 512,2230},  //duty=-1, dutyLt=2
   { 652, 512,1567},  //duty=0, dutyLt=2
   { 730, 512,1331},  //duty=1, dutyLt=2
   { 785, 512,1212},  //duty=2, dutyLt=2
   { 827, 512,1143},  //duty=3, dutyLt=2
   { 857, 512,1100},  //duty=4, dutyLt=2
   { 886, 512,1060},  //duty=5, dutyLt=2
   { 906, 512,1037},  //duty=6, dutyLt=2
   { 931, 512,1012},  //duty=7, dutyLt=2
   { 954, 512, 982},  //duty=8, dutyLt=2
   { 978, 512, 957},  //duty=9, dutyLt=2
   { 990, 512, 940},  //duty=10, dutyLt=2
   {1001, 512, 927},  //duty=11, dutyLt=2
   {1010, 512, 916},  //duty=12, dutyLt=2
   {1019, 512, 904},  //duty=13, dutyLt=2
   {1026, 512, 895},  //duty=14, dutyLt=2
   {1031, 512, 888},  //duty=15, dutyLt=2
   {1037, 512, 879},  //duty=16, dutyLt=2
   {1041, 512, 874},  //duty=17, dutyLt=2
   {1044, 512, 868},  //duty=18, dutyLt=2
   {1047, 512, 864},  //duty=19, dutyLt=2
   {1050, 512, 858},  //duty=20, dutyLt=2
   {1053, 512, 854},  //duty=21, dutyLt=2
   {1056, 512, 850},  //duty=22, dutyLt=2
   {1058, 512, 846},  //duty=23, dutyLt=2
   { 512, 512, 512},  //duty=24, dutyLt=2
   { 512, 512, 512},  //duty=25, dutyLt=2
   { 548, 512,2198},  //duty=-1, dutyLt=3
   { 630, 512,1652},  //duty=0, dutyLt=3
   { 696, 512,1407},  //duty=1, dutyLt=3
   { 747, 512,1275},  //duty=2, dutyLt=3
   { 785, 512,1198},  //duty=3, dutyLt=3
   { 817, 512,1141},  //duty=4, dutyLt=3
   { 842, 512,1108},  //duty=5, dutyLt=3
   { 862, 512,1080},  //duty=6, dutyLt=3
   { 890, 512,1048},  //duty=7, dutyLt=3
   { 917, 512,1014},  //duty=8, dutyLt=3
   { 941, 512, 984},  //duty=9, dutyLt=3
   { 956, 512, 964},  //duty=10, dutyLt=3
   { 968, 512, 949},  //duty=11, dutyLt=3
   { 979, 512, 936},  //duty=12, dutyLt=3
   { 990, 512, 922},  //duty=13, dutyLt=3
   { 997, 512, 913},  //duty=14, dutyLt=3
   {1004, 512, 905},  //duty=15, dutyLt=3
   {1011, 512, 895},  //duty=16, dutyLt=3
   {1015, 512, 889},  //duty=17, dutyLt=3
   {1019, 512, 882},  //duty=18, dutyLt=3
   {1023, 512, 878},  //duty=19, dutyLt=3
   {1028, 512, 871},  //duty=20, dutyLt=3
   {1031, 512, 866},  //duty=21, dutyLt=3
   {1034, 512, 862},  //duty=22, dutyLt=3
   {1037, 512, 858},  //duty=23, dutyLt=3
   { 512, 512, 512},  //duty=24, dutyLt=3
   { 512, 512, 512},  //duty=25, dutyLt=3
   { 548, 512,2185},  //duty=-1, dutyLt=4
   { 617, 512,1706},  //duty=0, dutyLt=4
   { 675, 512,1457},  //duty=1, dutyLt=4
   { 720, 512,1321},  //duty=2, dutyLt=4
   { 759, 512,1237},  //duty=3, dutyLt=4
   { 787, 512,1184},  //duty=4, dutyLt=4
   { 810, 512,1146},  //duty=5, dutyLt=4
   { 831, 512,1117},  //duty=6, dutyLt=4
   { 857, 512,1081},  //duty=7, dutyLt=4
   { 886, 512,1042},  //duty=8, dutyLt=4
   { 911, 512,1008},  //duty=9, dutyLt=4
   { 928, 512, 986},  //duty=10, dutyLt=4
   { 941, 512, 969},  //duty=11, dutyLt=4
   { 953, 512, 955},  //duty=12, dutyLt=4
   { 965, 512, 940},  //duty=13, dutyLt=4
   { 973, 512, 929},  //duty=14, dutyLt=4
   { 980, 512, 920},  //duty=15, dutyLt=4
   { 988, 512, 910},  //duty=16, dutyLt=4
   { 993, 512, 903},  //duty=17, dutyLt=4
   { 998, 512, 896},  //duty=18, dutyLt=4
   {1002, 512, 890},  //duty=19, dutyLt=4
   {1007, 512, 884},  //duty=20, dutyLt=4
   {1011, 512, 878},  //duty=21, dutyLt=4
   {1014, 512, 874},  //duty=22, dutyLt=4
   { 512, 512, 512},  //duty=23, dutyLt=4
   { 512, 512, 512},  //duty=24, dutyLt=4
   { 512, 512, 512},  //duty=25, dutyLt=4
   { 549, 512,2179},  //duty=-1, dutyLt=5
   { 607, 512,1747},  //duty=0, dutyLt=5
   { 658, 512,1504},  //duty=1, dutyLt=5
   { 700, 512,1368},  //duty=2, dutyLt=5
   { 735, 512,1279},  //duty=3, dutyLt=5
   { 761, 512,1224},  //duty=4, dutyLt=5
   { 784, 512,1184},  //duty=5, dutyLt=5
   { 806, 512,1150},  //duty=6, dutyLt=5
   { 832, 512,1109},  //duty=7, dutyLt=5
   { 860, 512,1068},  //duty=8, dutyLt=5
   { 887, 512,1030},  //duty=9, dutyLt=5
   { 904, 512,1007},  //duty=10, dutyLt=5
   { 918, 512, 988},  //duty=11, dutyLt=5
   { 930, 512, 972},  //duty=12, dutyLt=5
   { 943, 512, 956},  //duty=13, dutyLt=5
   { 952, 512, 944},  //duty=14, dutyLt=5
   { 959, 512, 934},  //duty=15, dutyLt=5
   { 967, 512, 923},  //duty=16, dutyLt=5
   { 974, 512, 916},  //duty=17, dutyLt=5
   { 979, 512, 908},  //duty=18, dutyLt=5
   { 983, 512, 903},  //duty=19, dutyLt=5
   { 989, 512, 895},  //duty=20, dutyLt=5
   { 994, 512, 890},  //duty=21, dutyLt=5
   { 997, 512, 885},  //duty=22, dutyLt=5
   { 512, 512, 512},  //duty=23, dutyLt=5
   { 512, 512, 512},  //duty=24, dutyLt=5
   { 512, 512, 512},  //duty=25, dutyLt=5
   { 549, 512,2169},  //duty=-1, dutyLt=6
   { 599, 512,1773},  //duty=0, dutyLt=6
   { 645, 512,1539},  //duty=1, dutyLt=6
   { 685, 512,1408},  //duty=2, dutyLt=6
   { 716, 512,1319},  //duty=3, dutyLt=6
   { 741, 512,1260},  //duty=4, dutyLt=6
   { 765, 512,1217},  //duty=5, dutyLt=6
   { 785, 512,1181},  //duty=6, dutyLt=6
   { 811, 512,1137},  //duty=7, dutyLt=6
   { 838, 512,1091},  //duty=8, dutyLt=6
   { 865, 512,1051},  //duty=9, dutyLt=6
   { 882, 512,1026},  //duty=10, dutyLt=6
   { 897, 512,1006},  //duty=11, dutyLt=6
   { 910, 512, 989},  //duty=12, dutyLt=6
   { 923, 512, 971},  //duty=13, dutyLt=6
   { 933, 512, 959},  //duty=14, dutyLt=6
   { 941, 512, 948},  //duty=15, dutyLt=6
   { 949, 512, 936},  //duty=16, dutyLt=6
   { 956, 512, 928},  //duty=17, dutyLt=6
   { 962, 512, 920},  //duty=18, dutyLt=6
   { 967, 512, 915},  //duty=19, dutyLt=6
   { 973, 512, 906},  //duty=20, dutyLt=6
   { 978, 512, 900},  //duty=21, dutyLt=6
   { 512, 512, 512},  //duty=22, dutyLt=6
   { 512, 512, 512},  //duty=23, dutyLt=6
   { 512, 512, 512},  //duty=24, dutyLt=6
   { 512, 512, 512},  //duty=25, dutyLt=6
   { 548, 512,2147},  //duty=-1, dutyLt=7
   { 593, 512,1794},  //duty=0, dutyLt=7
   { 635, 512,1577},  //duty=1, dutyLt=7
   { 668, 512,1456},  //duty=2, dutyLt=7
   { 695, 512,1365},  //duty=3, dutyLt=7
   { 720, 512,1306},  //duty=4, dutyLt=7
   { 741, 512,1261},  //duty=5, dutyLt=7
   { 760, 512,1223},  //duty=6, dutyLt=7
   { 785, 512,1173},  //duty=7, dutyLt=7
   { 812, 512,1125},  //duty=8, dutyLt=7
   { 838, 512,1079},  //duty=9, dutyLt=7
   { 856, 512,1052},  //duty=10, dutyLt=7
   { 871, 512,1031},  //duty=11, dutyLt=7
   { 885, 512,1012},  //duty=12, dutyLt=7
   { 898, 512, 992},  //duty=13, dutyLt=7
   { 908, 512, 979},  //duty=14, dutyLt=7
   { 917, 512, 968},  //duty=15, dutyLt=7
   { 926, 512, 954},  //duty=16, dutyLt=7
   { 933, 512, 945},  //duty=17, dutyLt=7
   { 939, 512, 937},  //duty=18, dutyLt=7
   { 944, 512, 931},  //duty=19, dutyLt=7
   { 951, 512, 922},  //duty=20, dutyLt=7
   { 956, 512, 915},  //duty=21, dutyLt=7
   { 512, 512, 512},  //duty=22, dutyLt=7
   { 512, 512, 512},  //duty=23, dutyLt=7
   { 512, 512, 512},  //duty=24, dutyLt=7
   { 512, 512, 512},  //duty=25, dutyLt=7
   { 549, 512,2130},  //duty=-1, dutyLt=8
   { 590, 512,1808},  //duty=0, dutyLt=8
   { 622, 512,1634},  //duty=1, dutyLt=8
   { 650, 512,1503},  //duty=2, dutyLt=8
   { 675, 512,1420},  //duty=3, dutyLt=8
   { 698, 512,1359},  //duty=4, dutyLt=8
   { 718, 512,1310},  //duty=5, dutyLt=8
   { 736, 512,1267},  //duty=6, dutyLt=8
   { 759, 512,1213},  //duty=7, dutyLt=8
   { 784, 512,1161},  //duty=8, dutyLt=8
   { 810, 512,1113},  //duty=9, dutyLt=8
   { 828, 512,1085},  //duty=10, dutyLt=8
   { 843, 512,1060},  //duty=11, dutyLt=8
   { 856, 512,1040},  //duty=12, dutyLt=8
   { 871, 512,1018},  //duty=13, dutyLt=8
   { 881, 512,1003},  //duty=14, dutyLt=8
   { 890, 512, 990},  //duty=15, dutyLt=8
   { 900, 512, 976},  //duty=16, dutyLt=8
   { 907, 512, 966},  //duty=17, dutyLt=8
   { 914, 512, 957},  //duty=18, dutyLt=8
   { 919, 512, 950},  //duty=19, dutyLt=8
   { 926, 512, 941},  //duty=20, dutyLt=8
   { 512, 512, 512},  //duty=21, dutyLt=8
   { 512, 512, 512},  //duty=22, dutyLt=8
   { 512, 512, 512},  //duty=23, dutyLt=8
   { 512, 512, 512},  //duty=24, dutyLt=8
   { 512, 512, 512},  //duty=25, dutyLt=8
   { 551, 512,2097},  //duty=-1, dutyLt=9
   { 582, 512,1853},  //duty=0, dutyLt=9
   { 610, 512,1673},  //duty=1, dutyLt=9
   { 637, 512,1556},  //duty=2, dutyLt=9
   { 659, 512,1475},  //duty=3, dutyLt=9
   { 680, 512,1413},  //duty=4, dutyLt=9
   { 697, 512,1358},  //duty=5, dutyLt=9
   { 713, 512,1312},  //duty=6, dutyLt=9
   { 734, 512,1256},  //duty=7, dutyLt=9
   { 758, 512,1201},  //duty=8, dutyLt=9
   { 783, 512,1150},  //duty=9, dutyLt=9
   { 801, 512,1119},  //duty=10, dutyLt=9
   { 816, 512,1093},  //duty=11, dutyLt=9
   { 829, 512,1070},  //duty=12, dutyLt=9
   { 844, 512,1046},  //duty=13, dutyLt=9
   { 854, 512,1030},  //duty=14, dutyLt=9
   { 863, 512,1016},  //duty=15, dutyLt=9
   { 873, 512,1001},  //duty=16, dutyLt=9
   { 881, 512, 990},  //duty=17, dutyLt=9
   { 888, 512, 980},  //duty=18, dutyLt=9
   { 894, 512, 973},  //duty=19, dutyLt=9
   { 512, 512, 512},  //duty=20, dutyLt=9
   { 512, 512, 512},  //duty=21, dutyLt=9
   { 512, 512, 512},  //duty=22, dutyLt=9
   { 512, 512, 512},  //duty=23, dutyLt=9
   { 512, 512, 512},  //duty=24, dutyLt=9
   { 512, 512, 512},  //duty=25, dutyLt=9
   { 550, 512,2094},  //duty=-1, dutyLt=10
   { 579, 512,1860},  //duty=0, dutyLt=10
   { 605, 512,1699},  //duty=1, dutyLt=10
   { 629, 512,1591},  //duty=2, dutyLt=10
   { 650, 512,1511},  //duty=3, dutyLt=10
   { 668, 512,1444},  //duty=4, dutyLt=10
   { 684, 512,1388},  //duty=5, dutyLt=10
   { 699, 512,1340},  //duty=6, dutyLt=10
   { 719, 512,1286},  //duty=7, dutyLt=10
   { 742, 512,1228},  //duty=8, dutyLt=10
   { 766, 512,1176},  //duty=9, dutyLt=10
   { 783, 512,1143},  //duty=10, dutyLt=10
   { 798, 512,1115},  //duty=11, dutyLt=10
   { 811, 512,1092},  //duty=12, dutyLt=10
   { 826, 512,1067},  //duty=13, dutyLt=10
   { 836, 512,1050},  //duty=14, dutyLt=10
   { 845, 512,1035},  //duty=15, dutyLt=10
   { 856, 512,1019},  //duty=16, dutyLt=10
   { 864, 512,1008},  //duty=17, dutyLt=10
   { 871, 512, 997},  //duty=18, dutyLt=10
   { 512, 512, 512},  //duty=19, dutyLt=10
   { 512, 512, 512},  //duty=20, dutyLt=10
   { 512, 512, 512},  //duty=21, dutyLt=10
   { 512, 512, 512},  //duty=22, dutyLt=10
   { 512, 512, 512},  //duty=23, dutyLt=10
   { 512, 512, 512},  //duty=24, dutyLt=10
   { 512, 512, 512},  //duty=25, dutyLt=10
   { 551, 512,2082},  //duty=-1, dutyLt=11
   { 577, 512,1868},  //duty=0, dutyLt=11
   { 601, 512,1721},  //duty=1, dutyLt=11
   { 622, 512,1618},  //duty=2, dutyLt=11
   { 641, 512,1539},  //duty=3, dutyLt=11
   { 658, 512,1469},  //duty=4, dutyLt=11
   { 674, 512,1414},  //duty=5, dutyLt=11
   { 688, 512,1366},  //duty=6, dutyLt=11
   { 706, 512,1311},  //duty=7, dutyLt=11
   { 729, 512,1252},  //duty=8, dutyLt=11
   { 753, 512,1199},  //duty=9, dutyLt=11
   { 769, 512,1165},  //duty=10, dutyLt=11
   { 784, 512,1136},  //duty=11, dutyLt=11
   { 797, 512,1111},  //duty=12, dutyLt=11
   { 811, 512,1085},  //duty=13, dutyLt=11
   { 821, 512,1067},  //duty=14, dutyLt=11
   { 831, 512,1052},  //duty=15, dutyLt=11
   { 841, 512,1035},  //duty=16, dutyLt=11
   { 849, 512,1024},  //duty=17, dutyLt=11
   { 512, 512, 512},  //duty=18, dutyLt=11
   { 512, 512, 512},  //duty=19, dutyLt=11
   { 512, 512, 512},  //duty=20, dutyLt=11
   { 512, 512, 512},  //duty=21, dutyLt=11
   { 512, 512, 512},  //duty=22, dutyLt=11
   { 512, 512, 512},  //duty=23, dutyLt=11
   { 512, 512, 512},  //duty=24, dutyLt=11
   { 512, 512, 512},  //duty=25, dutyLt=11
   { 552, 512,2067},  //duty=-1, dutyLt=12
   { 575, 512,1877},  //duty=0, dutyLt=12
   { 597, 512,1740},  //duty=1, dutyLt=12
   { 617, 512,1639},  //duty=2, dutyLt=12
   { 635, 512,1560},  //duty=3, dutyLt=12
   { 650, 512,1491},  //duty=4, dutyLt=12
   { 665, 512,1435},  //duty=5, dutyLt=12
   { 678, 512,1389},  //duty=6, dutyLt=12
   { 696, 512,1334},  //duty=7, dutyLt=12
   { 718, 512,1275},  //duty=8, dutyLt=12
   { 741, 512,1221},  //duty=9, dutyLt=12
   { 757, 512,1185},  //duty=10, dutyLt=12
   { 771, 512,1155},  //duty=11, dutyLt=12
   { 784, 512,1129},  //duty=12, dutyLt=12
   { 798, 512,1102},  //duty=13, dutyLt=12
   { 808, 512,1084},  //duty=14, dutyLt=12
   { 817, 512,1068},  //duty=15, dutyLt=12
   { 828, 512,1051},  //duty=16, dutyLt=12
   { 512, 512, 512},  //duty=17, dutyLt=12
   { 512, 512, 512},  //duty=18, dutyLt=12
   { 512, 512, 512},  //duty=19, dutyLt=12
   { 512, 512, 512},  //duty=20, dutyLt=12
   { 512, 512, 512},  //duty=21, dutyLt=12
   { 512, 512, 512},  //duty=22, dutyLt=12
   { 512, 512, 512},  //duty=23, dutyLt=12
   { 512, 512, 512},  //duty=24, dutyLt=12
   { 512, 512, 512},  //duty=25, dutyLt=12
   { 553, 512,2054},  //duty=-1, dutyLt=13
   { 574, 512,1885},  //duty=0, dutyLt=13
   { 594, 512,1758},  //duty=1, dutyLt=13
   { 612, 512,1662},  //duty=2, dutyLt=13
   { 627, 512,1580},  //duty=3, dutyLt=13
   { 643, 512,1513},  //duty=4, dutyLt=13
   { 656, 512,1459},  //duty=5, dutyLt=13
   { 669, 512,1413},  //duty=6, dutyLt=13
   { 686, 512,1358},  //duty=7, dutyLt=13
   { 707, 512,1298},  //duty=8, dutyLt=13
   { 729, 512,1243},  //duty=9, dutyLt=13
   { 745, 512,1206},  //duty=10, dutyLt=13
   { 758, 512,1175},  //duty=11, dutyLt=13
   { 771, 512,1149},  //duty=12, dutyLt=13
   { 784, 512,1121},  //duty=13, dutyLt=13
   { 794, 512,1103},  //duty=14, dutyLt=13
   { 804, 512,1087},  //duty=15, dutyLt=13
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
   { 553, 512,2049},  //duty=-1, dutyLt=14
   { 573, 512,1890},  //duty=0, dutyLt=14
   { 591, 512,1772},  //duty=1, dutyLt=14
   { 608, 512,1676},  //duty=2, dutyLt=14
   { 623, 512,1595},  //duty=3, dutyLt=14
   { 637, 512,1528},  //duty=4, dutyLt=14
   { 650, 512,1475},  //duty=5, dutyLt=14
   { 663, 512,1431},  //duty=6, dutyLt=14
   { 679, 512,1374},  //duty=7, dutyLt=14
   { 700, 512,1316},  //duty=8, dutyLt=14
   { 721, 512,1259},  //duty=9, dutyLt=14
   { 736, 512,1222},  //duty=10, dutyLt=14
   { 749, 512,1191},  //duty=11, dutyLt=14
   { 761, 512,1163},  //duty=12, dutyLt=14
   { 775, 512,1136},  //duty=13, dutyLt=14
   { 785, 512,1116},  //duty=14, dutyLt=14
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
   { 553, 512,2040},  //duty=-1, dutyLt=15
   { 572, 512,1895},  //duty=0, dutyLt=15
   { 589, 512,1780},  //duty=1, dutyLt=15
   { 605, 512,1683},  //duty=2, dutyLt=15
   { 619, 512,1605},  //duty=3, dutyLt=15
   { 634, 512,1541},  //duty=4, dutyLt=15
   { 646, 512,1490},  //duty=5, dutyLt=15
   { 658, 512,1447},  //duty=6, dutyLt=15
   { 674, 512,1390},  //duty=7, dutyLt=15
   { 693, 512,1331},  //duty=8, dutyLt=15
   { 714, 512,1273},  //duty=9, dutyLt=15
   { 729, 512,1235},  //duty=10, dutyLt=15
   { 742, 512,1204},  //duty=11, dutyLt=15
   { 754, 512,1177},  //duty=12, dutyLt=15
   { 767, 512,1148},  //duty=13, dutyLt=15
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
   { 554, 512,2032},  //duty=-1, dutyLt=16
   { 571, 512,1899},  //duty=0, dutyLt=16
   { 587, 512,1787},  //duty=1, dutyLt=16
   { 602, 512,1694},  //duty=2, dutyLt=16
   { 616, 512,1618},  //duty=3, dutyLt=16
   { 629, 512,1557},  //duty=4, dutyLt=16
   { 641, 512,1508},  //duty=5, dutyLt=16
   { 653, 512,1463},  //duty=6, dutyLt=16
   { 668, 512,1406},  //duty=7, dutyLt=16
   { 686, 512,1348},  //duty=8, dutyLt=16
   { 706, 512,1288},  //duty=9, dutyLt=16
   { 720, 512,1252},  //duty=10, dutyLt=16
   { 734, 512,1219},  //duty=11, dutyLt=16
   { 745, 512,1192},  //duty=12, dutyLt=16
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
   { 554, 512,2027},  //duty=-1, dutyLt=17
   { 570, 512,1903},  //duty=0, dutyLt=17
   { 586, 512,1793},  //duty=1, dutyLt=17
   { 601, 512,1700},  //duty=2, dutyLt=17
   { 614, 512,1628},  //duty=3, dutyLt=17
   { 627, 512,1567},  //duty=4, dutyLt=17
   { 638, 512,1518},  //duty=5, dutyLt=17
   { 649, 512,1473},  //duty=6, dutyLt=17
   { 664, 512,1418},  //duty=7, dutyLt=17
   { 681, 512,1361},  //duty=8, dutyLt=17
   { 701, 512,1301},  //duty=9, dutyLt=17
   { 715, 512,1263},  //duty=10, dutyLt=17
   { 728, 512,1231},  //duty=11, dutyLt=17
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
   { 554, 512,2023},  //duty=-1, dutyLt=18
   { 570, 512,1901},  //duty=0, dutyLt=18
   { 585, 512,1794},  //duty=1, dutyLt=18
   { 599, 512,1707},  //duty=2, dutyLt=18
   { 612, 512,1636},  //duty=3, dutyLt=18
   { 624, 512,1577},  //duty=4, dutyLt=18
   { 636, 512,1527},  //duty=5, dutyLt=18
   { 646, 512,1483},  //duty=6, dutyLt=18
   { 660, 512,1429},  //duty=7, dutyLt=18
   { 678, 512,1371},  //duty=8, dutyLt=18
   { 697, 512,1311},  //duty=9, dutyLt=18
   { 710, 512,1273},  //duty=10, dutyLt=18
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
   { 555, 512,2019},  //duty=-1, dutyLt=19
   { 570, 512,1900},  //duty=0, dutyLt=19
   { 585, 512,1794},  //duty=1, dutyLt=19
   { 598, 512,1709},  //duty=2, dutyLt=19
   { 611, 512,1643},  //duty=3, dutyLt=19
   { 623, 512,1584},  //duty=4, dutyLt=19
   { 634, 512,1534},  //duty=5, dutyLt=19
   { 644, 512,1491},  //duty=6, dutyLt=19
   { 656, 512,1439},  //duty=7, dutyLt=19
   { 675, 512,1378},  //duty=8, dutyLt=19
   { 693, 512,1320},  //duty=9, dutyLt=19
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
   { 555, 512,2013},  //duty=-1, dutyLt=20
   { 570, 512,1900},  //duty=0, dutyLt=20
   { 584, 512,1797},  //duty=1, dutyLt=20
   { 597, 512,1715},  //duty=2, dutyLt=20
   { 609, 512,1650},  //duty=3, dutyLt=20
   { 621, 512,1593},  //duty=4, dutyLt=20
   { 631, 512,1543},  //duty=5, dutyLt=20
   { 641, 512,1501},  //duty=6, dutyLt=20
   { 655, 512,1448},  //duty=7, dutyLt=20
   { 671, 512,1390},  //duty=8, dutyLt=20
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
   { 556, 512,2007},  //duty=-1, dutyLt=21
   { 570, 512,1897},  //duty=0, dutyLt=21
   { 584, 512,1796},  //duty=1, dutyLt=21
   { 596, 512,1719},  //duty=2, dutyLt=21
   { 608, 512,1655},  //duty=3, dutyLt=21
   { 619, 512,1600},  //duty=4, dutyLt=21
   { 630, 512,1551},  //duty=5, dutyLt=21
   { 639, 512,1509},  //duty=6, dutyLt=21
   { 652, 512,1457},  //duty=7, dutyLt=21
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
   { 557, 512,2003},  //duty=-1, dutyLt=22
   { 570, 512,1894},  //duty=0, dutyLt=22
   { 583, 512,1798},  //duty=1, dutyLt=22
   { 596, 512,1721},  //duty=2, dutyLt=22
   { 608, 512,1658},  //duty=3, dutyLt=22
   { 618, 512,1603},  //duty=4, dutyLt=22
   { 628, 512,1556},  //duty=5, dutyLt=22
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
   { 557, 512,1999},  //duty=-1, dutyLt=23
   { 571, 512,1891},  //duty=0, dutyLt=23
   { 583, 512,1799},  //duty=1, dutyLt=23
   { 596, 512,1725},  //duty=2, dutyLt=23
   { 607, 512,1662},  //duty=3, dutyLt=23
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
   { 558, 512,1990},  //duty=-1, dutyLt=24
   { 571, 512,1887},  //duty=0, dutyLt=24
   { 584, 512,1798},  //duty=1, dutyLt=24
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
   { 559, 512,1982},  //duty=-1, dutyLt=25
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
