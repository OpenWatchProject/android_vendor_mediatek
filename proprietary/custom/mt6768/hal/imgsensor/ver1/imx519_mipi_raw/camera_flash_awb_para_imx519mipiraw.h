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
    {   0, 512,   0},  //duty=-1, dutyLt=-1
    {1151, 512,1082},  //duty=0, dutyLt=-1
    {1161, 512,1087},  //duty=1, dutyLt=-1
    {1158, 512,1081},  //duty=2, dutyLt=-1
    {1156, 512,1076},  //duty=3, dutyLt=-1
    {1155, 512,1072},  //duty=4, dutyLt=-1
    {1154, 512,1067},  //duty=5, dutyLt=-1
    {1153, 512,1059},  //duty=6, dutyLt=-1
    {1152, 512,1056},  //duty=7, dutyLt=-1
    {1153, 512,1054},  //duty=8, dutyLt=-1
    {1151, 512,1050},  //duty=9, dutyLt=-1
    {1152, 512,1048},  //duty=10, dutyLt=-1
    {1151, 512,1044},  //duty=11, dutyLt=-1
    {1151, 512,1042},  //duty=12, dutyLt=-1
    {1150, 512,1039},  //duty=13, dutyLt=-1
    {1151, 512,1036},  //duty=14, dutyLt=-1
    {1150, 512,1033},  //duty=15, dutyLt=-1
    {1150, 512,1028},  //duty=16, dutyLt=-1
    {1151, 512,1023},  //duty=17, dutyLt=-1
    {1150, 512,1018},  //duty=18, dutyLt=-1
    {1150, 512,1014},  //duty=19, dutyLt=-1
    {1150, 512,1009},  //duty=20, dutyLt=-1
    {1149, 512,1004},  //duty=21, dutyLt=-1
    {1150, 512,1000},  //duty=22, dutyLt=-1
    {1150, 512, 996},  //duty=23, dutyLt=-1
    {1150, 512, 991},  //duty=24, dutyLt=-1
    {1149, 512, 987},  //duty=25, dutyLt=-1
    {1150, 512, 983},  //duty=26, dutyLt=-1
    {1150, 512, 981},  //duty=27, dutyLt=-1
    {1150, 512, 975},  //duty=28, dutyLt=-1
    {1151, 512, 971},  //duty=29, dutyLt=-1
    { 539, 512,2621},  //duty=-1, dutyLt=0
    { 792, 512,1419},  //duty=0, dutyLt=0
    { 899, 512,1278},  //duty=1, dutyLt=0
    { 954, 512,1216},  //duty=2, dutyLt=0
    { 989, 512,1181},  //duty=3, dutyLt=0
    {1012, 512,1158},  //duty=4, dutyLt=0
    {1029, 512,1138},  //duty=5, dutyLt=0
    {1040, 512,1123},  //duty=6, dutyLt=0
    {1050, 512,1114},  //duty=7, dutyLt=0
    {1058, 512,1105},  //duty=8, dutyLt=0
    {1065, 512,1098},  //duty=9, dutyLt=0
    {1071, 512,1093},  //duty=10, dutyLt=0
    {1075, 512,1087},  //duty=11, dutyLt=0
    {1080, 512,1081},  //duty=12, dutyLt=0
    {1083, 512,1075},  //duty=13, dutyLt=0
    {1086, 512,1071},  //duty=14, dutyLt=0
    {1089, 512,1067},  //duty=15, dutyLt=0
    {1096, 512,1059},  //duty=16, dutyLt=0
    {1099, 512,1052},  //duty=17, dutyLt=0
    {1102, 512,1045},  //duty=18, dutyLt=0
    {1105, 512,1039},  //duty=19, dutyLt=0
    {1107, 512,1033},  //duty=20, dutyLt=0
    {1109, 512,1027},  //duty=21, dutyLt=0
    {1110, 512,1022},  //duty=22, dutyLt=0
    {1112, 512,1017},  //duty=23, dutyLt=0
    {1113, 512,1012},  //duty=24, dutyLt=0
    {1115, 512,1007},  //duty=25, dutyLt=0
    {1115, 512,1003},  //duty=26, dutyLt=0
    {1117, 512, 998},  //duty=27, dutyLt=0
    {1118, 512, 994},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 538, 512,2642},  //duty=-1, dutyLt=1
    { 702, 512,1637},  //duty=0, dutyLt=1
    { 795, 512,1423},  //duty=1, dutyLt=1
    { 855, 512,1325},  //duty=2, dutyLt=1
    { 895, 512,1269},  //duty=3, dutyLt=1
    { 926, 512,1230},  //duty=4, dutyLt=1
    { 948, 512,1199},  //duty=5, dutyLt=1
    { 966, 512,1180},  //duty=6, dutyLt=1
    { 980, 512,1165},  //duty=7, dutyLt=1
    { 992, 512,1152},  //duty=8, dutyLt=1
    {1003, 512,1141},  //duty=9, dutyLt=1
    {1012, 512,1132},  //duty=10, dutyLt=1
    {1020, 512,1123},  //duty=11, dutyLt=1
    {1026, 512,1115},  //duty=12, dutyLt=1
    {1032, 512,1108},  //duty=13, dutyLt=1
    {1037, 512,1103},  //duty=14, dutyLt=1
    {1042, 512,1096},  //duty=15, dutyLt=1
    {1050, 512,1087},  //duty=16, dutyLt=1
    {1056, 512,1078},  //duty=17, dutyLt=1
    {1061, 512,1070},  //duty=18, dutyLt=1
    {1066, 512,1062},  //duty=19, dutyLt=1
    {1070, 512,1055},  //duty=20, dutyLt=1
    {1073, 512,1048},  //duty=21, dutyLt=1
    {1076, 512,1042},  //duty=22, dutyLt=1
    {1079, 512,1036},  //duty=23, dutyLt=1
    {1081, 512,1030},  //duty=24, dutyLt=1
    {1083, 512,1025},  //duty=25, dutyLt=1
    {1085, 512,1020},  //duty=26, dutyLt=1
    {1087, 512,1015},  //duty=27, dutyLt=1
    {1089, 512,1010},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 538, 512,2679},  //duty=-1, dutyLt=2
    { 659, 512,1792},  //duty=0, dutyLt=2
    { 739, 512,1535},  //duty=1, dutyLt=2
    { 795, 512,1414},  //duty=2, dutyLt=2
    { 837, 512,1341},  //duty=3, dutyLt=2
    { 869, 512,1282},  //duty=4, dutyLt=2
    { 892, 512,1257},  //duty=5, dutyLt=2
    { 912, 512,1231},  //duty=6, dutyLt=2
    { 929, 512,1211},  //duty=7, dutyLt=2
    { 942, 512,1194},  //duty=8, dutyLt=2
    { 956, 512,1180},  //duty=9, dutyLt=2
    { 966, 512,1168},  //duty=10, dutyLt=2
    { 976, 512,1156},  //duty=11, dutyLt=2
    { 984, 512,1147},  //duty=12, dutyLt=2
    { 991, 512,1139},  //duty=13, dutyLt=2
    { 998, 512,1131},  //duty=14, dutyLt=2
    {1004, 512,1123},  //duty=15, dutyLt=2
    {1013, 512,1111},  //duty=16, dutyLt=2
    {1022, 512,1101},  //duty=17, dutyLt=2
    {1029, 512,1091},  //duty=18, dutyLt=2
    {1035, 512,1082},  //duty=19, dutyLt=2
    {1040, 512,1074},  //duty=20, dutyLt=2
    {1045, 512,1066},  //duty=21, dutyLt=2
    {1049, 512,1059},  //duty=22, dutyLt=2
    {1052, 512,1053},  //duty=23, dutyLt=2
    {1055, 512,1047},  //duty=24, dutyLt=2
    {1058, 512,1041},  //duty=25, dutyLt=2
    {1061, 512,1036},  //duty=26, dutyLt=2
    {1063, 512,1030},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 537, 512,2668},  //duty=-1, dutyLt=3
    { 635, 512,1891},  //duty=0, dutyLt=3
    { 704, 512,1626},  //duty=1, dutyLt=3
    { 755, 512,1489},  //duty=2, dutyLt=3
    { 796, 512,1402},  //duty=3, dutyLt=3
    { 824, 512,1344},  //duty=4, dutyLt=3
    { 851, 512,1305},  //duty=5, dutyLt=3
    { 872, 512,1275},  //duty=6, dutyLt=3
    { 889, 512,1252},  //duty=7, dutyLt=3
    { 905, 512,1233},  //duty=8, dutyLt=3
    { 918, 512,1215},  //duty=9, dutyLt=3
    { 930, 512,1201},  //duty=10, dutyLt=3
    { 941, 512,1188},  //duty=11, dutyLt=3
    { 950, 512,1177},  //duty=12, dutyLt=3
    { 957, 512,1166},  //duty=13, dutyLt=3
    { 965, 512,1157},  //duty=14, dutyLt=3
    { 972, 512,1149},  //duty=15, dutyLt=3
    { 983, 512,1134},  //duty=16, dutyLt=3
    { 993, 512,1122},  //duty=17, dutyLt=3
    {1001, 512,1111},  //duty=18, dutyLt=3
    {1008, 512,1101},  //duty=19, dutyLt=3
    {1014, 512,1092},  //duty=20, dutyLt=3
    {1020, 512,1083},  //duty=21, dutyLt=3
    {1024, 512,1076},  //duty=22, dutyLt=3
    {1029, 512,1069},  //duty=23, dutyLt=3
    {1032, 512,1062},  //duty=24, dutyLt=3
    {1036, 512,1055},  //duty=25, dutyLt=3
    {1038, 512,1049},  //duty=26, dutyLt=3
    {1041, 512,1044},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 537, 512,2668},  //duty=-1, dutyLt=4
    { 619, 512,1973},  //duty=0, dutyLt=4
    { 680, 512,1699},  //duty=1, dutyLt=4
    { 727, 512,1552},  //duty=2, dutyLt=4
    { 763, 512,1453},  //duty=3, dutyLt=4
    { 793, 512,1396},  //duty=4, dutyLt=4
    { 818, 512,1349},  //duty=5, dutyLt=4
    { 840, 512,1316},  //duty=6, dutyLt=4
    { 858, 512,1289},  //duty=7, dutyLt=4
    { 874, 512,1267},  //duty=8, dutyLt=4
    { 888, 512,1248},  //duty=9, dutyLt=4
    { 900, 512,1231},  //duty=10, dutyLt=4
    { 911, 512,1217},  //duty=11, dutyLt=4
    { 921, 512,1203},  //duty=12, dutyLt=4
    { 930, 512,1192},  //duty=13, dutyLt=4
    { 938, 512,1182},  //duty=14, dutyLt=4
    { 945, 512,1173},  //duty=15, dutyLt=4
    { 958, 512,1156},  //duty=16, dutyLt=4
    { 968, 512,1142},  //duty=17, dutyLt=4
    { 977, 512,1129},  //duty=18, dutyLt=4
    { 985, 512,1119},  //duty=19, dutyLt=4
    { 992, 512,1109},  //duty=20, dutyLt=4
    { 998, 512,1100},  //duty=21, dutyLt=4
    {1003, 512,1091},  //duty=22, dutyLt=4
    {1008, 512,1084},  //duty=23, dutyLt=4
    {1012, 512,1077},  //duty=24, dutyLt=4
    {1015, 512,1069},  //duty=25, dutyLt=4
    {1019, 512,1063},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 537, 512,2667},  //duty=-1, dutyLt=5
    { 608, 512,2031},  //duty=0, dutyLt=5
    { 663, 512,1759},  //duty=1, dutyLt=5
    { 707, 512,1601},  //duty=2, dutyLt=5
    { 740, 512,1499},  //duty=3, dutyLt=5
    { 769, 512,1439},  //duty=4, dutyLt=5
    { 793, 512,1392},  //duty=5, dutyLt=5
    { 815, 512,1354},  //duty=6, dutyLt=5
    { 833, 512,1323},  //duty=7, dutyLt=5
    { 849, 512,1299},  //duty=8, dutyLt=5
    { 863, 512,1278},  //duty=9, dutyLt=5
    { 876, 512,1259},  //duty=10, dutyLt=5
    { 887, 512,1243},  //duty=11, dutyLt=5
    { 897, 512,1228},  //duty=12, dutyLt=5
    { 906, 512,1216},  //duty=13, dutyLt=5
    { 915, 512,1205},  //duty=14, dutyLt=5
    { 922, 512,1194},  //duty=15, dutyLt=5
    { 935, 512,1176},  //duty=16, dutyLt=5
    { 947, 512,1161},  //duty=17, dutyLt=5
    { 956, 512,1147},  //duty=18, dutyLt=5
    { 965, 512,1135},  //duty=19, dutyLt=5
    { 972, 512,1124},  //duty=20, dutyLt=5
    { 978, 512,1115},  //duty=21, dutyLt=5
    { 984, 512,1105},  //duty=22, dutyLt=5
    { 989, 512,1097},  //duty=23, dutyLt=5
    { 994, 512,1089},  //duty=24, dutyLt=5
    { 998, 512,1082},  //duty=25, dutyLt=5
    {1001, 512,1075},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 537, 512,2658},  //duty=-1, dutyLt=6
    { 600, 512,2077},  //duty=0, dutyLt=6
    { 650, 512,1807},  //duty=1, dutyLt=6
    { 688, 512,1655},  //duty=2, dutyLt=6
    { 722, 512,1545},  //duty=3, dutyLt=6
    { 750, 512,1476},  //duty=4, dutyLt=6
    { 774, 512,1428},  //duty=5, dutyLt=6
    { 794, 512,1389},  //duty=6, dutyLt=6
    { 812, 512,1355},  //duty=7, dutyLt=6
    { 828, 512,1328},  //duty=8, dutyLt=6
    { 842, 512,1305},  //duty=9, dutyLt=6
    { 855, 512,1285},  //duty=10, dutyLt=6
    { 866, 512,1267},  //duty=11, dutyLt=6
    { 876, 512,1253},  //duty=12, dutyLt=6
    { 886, 512,1238},  //duty=13, dutyLt=6
    { 894, 512,1226},  //duty=14, dutyLt=6
    { 902, 512,1215},  //duty=15, dutyLt=6
    { 916, 512,1195},  //duty=16, dutyLt=6
    { 928, 512,1178},  //duty=17, dutyLt=6
    { 937, 512,1163},  //duty=18, dutyLt=6
    { 946, 512,1151},  //duty=19, dutyLt=6
    { 954, 512,1139},  //duty=20, dutyLt=6
    { 961, 512,1129},  //duty=21, dutyLt=6
    { 967, 512,1119},  //duty=22, dutyLt=6
    { 973, 512,1110},  //duty=23, dutyLt=6
    { 977, 512,1102},  //duty=24, dutyLt=6
    { 982, 512,1094},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 537, 512,2655},  //duty=-1, dutyLt=7
    { 593, 512,2117},  //duty=0, dutyLt=7
    { 640, 512,1843},  //duty=1, dutyLt=7
    { 676, 512,1685},  //duty=2, dutyLt=7
    { 707, 512,1592},  //duty=3, dutyLt=7
    { 734, 512,1515},  //duty=4, dutyLt=7
    { 756, 512,1460},  //duty=5, dutyLt=7
    { 776, 512,1418},  //duty=6, dutyLt=7
    { 794, 512,1384},  //duty=7, dutyLt=7
    { 809, 512,1355},  //duty=8, dutyLt=7
    { 824, 512,1331},  //duty=9, dutyLt=7
    { 836, 512,1309},  //duty=10, dutyLt=7
    { 848, 512,1290},  //duty=11, dutyLt=7
    { 858, 512,1274},  //duty=12, dutyLt=7
    { 868, 512,1259},  //duty=13, dutyLt=7
    { 876, 512,1246},  //duty=14, dutyLt=7
    { 884, 512,1234},  //duty=15, dutyLt=7
    { 898, 512,1213},  //duty=16, dutyLt=7
    { 911, 512,1195},  //duty=17, dutyLt=7
    { 921, 512,1180},  //duty=18, dutyLt=7
    { 930, 512,1166},  //duty=19, dutyLt=7
    { 939, 512,1154},  //duty=20, dutyLt=7
    { 945, 512,1142},  //duty=21, dutyLt=7
    { 952, 512,1132},  //duty=22, dutyLt=7
    { 958, 512,1124},  //duty=23, dutyLt=7
    { 962, 512,1114},  //duty=24, dutyLt=7
    { 967, 512,1107},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 537, 512,2647},  //duty=-1, dutyLt=8
    { 589, 512,2145},  //duty=0, dutyLt=8
    { 630, 512,1870},  //duty=1, dutyLt=8
    { 665, 512,1715},  //duty=2, dutyLt=8
    { 695, 512,1621},  //duty=3, dutyLt=8
    { 720, 512,1551},  //duty=4, dutyLt=8
    { 742, 512,1494},  //duty=5, dutyLt=8
    { 762, 512,1447},  //duty=6, dutyLt=8
    { 779, 512,1411},  //duty=7, dutyLt=8
    { 794, 512,1380},  //duty=8, dutyLt=8
    { 808, 512,1355},  //duty=9, dutyLt=8
    { 820, 512,1331},  //duty=10, dutyLt=8
    { 832, 512,1312},  //duty=11, dutyLt=8
    { 842, 512,1294},  //duty=12, dutyLt=8
    { 852, 512,1279},  //duty=13, dutyLt=8
    { 861, 512,1265},  //duty=14, dutyLt=8
    { 869, 512,1253},  //duty=15, dutyLt=8
    { 883, 512,1230},  //duty=16, dutyLt=8
    { 896, 512,1211},  //duty=17, dutyLt=8
    { 906, 512,1195},  //duty=18, dutyLt=8
    { 916, 512,1180},  //duty=19, dutyLt=8
    { 924, 512,1167},  //duty=20, dutyLt=8
    { 931, 512,1156},  //duty=21, dutyLt=8
    { 938, 512,1145},  //duty=22, dutyLt=8
    { 944, 512,1135},  //duty=23, dutyLt=8
    { 949, 512,1126},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 537, 512,2643},  //duty=-1, dutyLt=9
    { 586, 512,2160},  //duty=0, dutyLt=9
    { 623, 512,1911},  //duty=1, dutyLt=9
    { 656, 512,1756},  //duty=2, dutyLt=9
    { 684, 512,1648},  //duty=3, dutyLt=9
    { 709, 512,1576},  //duty=4, dutyLt=9
    { 730, 512,1520},  //duty=5, dutyLt=9
    { 749, 512,1475},  //duty=6, dutyLt=9
    { 766, 512,1437},  //duty=7, dutyLt=9
    { 781, 512,1404},  //duty=8, dutyLt=9
    { 794, 512,1377},  //duty=9, dutyLt=9
    { 807, 512,1354},  //duty=10, dutyLt=9
    { 818, 512,1333},  //duty=11, dutyLt=9
    { 828, 512,1314},  //duty=12, dutyLt=9
    { 838, 512,1298},  //duty=13, dutyLt=9
    { 847, 512,1283},  //duty=14, dutyLt=9
    { 855, 512,1269},  //duty=15, dutyLt=9
    { 870, 512,1246},  //duty=16, dutyLt=9
    { 882, 512,1226},  //duty=17, dutyLt=9
    { 893, 512,1209},  //duty=18, dutyLt=9
    { 903, 512,1194},  //duty=19, dutyLt=9
    { 911, 512,1180},  //duty=20, dutyLt=9
    { 919, 512,1169},  //duty=21, dutyLt=9
    { 925, 512,1157},  //duty=22, dutyLt=9
    { 931, 512,1147},  //duty=23, dutyLt=9
    { 936, 512,1137},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 538, 512,2634},  //duty=-1, dutyLt=10
    { 581, 512,2155},  //duty=0, dutyLt=10
    { 618, 512,1934},  //duty=1, dutyLt=10
    { 649, 512,1791},  //duty=2, dutyLt=10
    { 676, 512,1682},  //duty=3, dutyLt=10
    { 699, 512,1602},  //duty=4, dutyLt=10
    { 720, 512,1543},  //duty=5, dutyLt=10
    { 738, 512,1497},  //duty=6, dutyLt=10
    { 754, 512,1458},  //duty=7, dutyLt=10
    { 769, 512,1425},  //duty=8, dutyLt=10
    { 782, 512,1397},  //duty=9, dutyLt=10
    { 795, 512,1373},  //duty=10, dutyLt=10
    { 806, 512,1352},  //duty=11, dutyLt=10
    { 816, 512,1332},  //duty=12, dutyLt=10
    { 826, 512,1316},  //duty=13, dutyLt=10
    { 834, 512,1300},  //duty=14, dutyLt=10
    { 842, 512,1286},  //duty=15, dutyLt=10
    { 857, 512,1261},  //duty=16, dutyLt=10
    { 870, 512,1240},  //duty=17, dutyLt=10
    { 881, 512,1222},  //duty=18, dutyLt=10
    { 891, 512,1206},  //duty=19, dutyLt=10
    { 899, 512,1192},  //duty=20, dutyLt=10
    { 907, 512,1180},  //duty=21, dutyLt=10
    { 914, 512,1168},  //duty=22, dutyLt=10
    { 920, 512,1157},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 539, 512,2616},  //duty=-1, dutyLt=11
    { 579, 512,2183},  //duty=0, dutyLt=11
    { 612, 512,1955},  //duty=1, dutyLt=11
    { 642, 512,1812},  //duty=2, dutyLt=11
    { 668, 512,1713},  //duty=3, dutyLt=11
    { 690, 512,1635},  //duty=4, dutyLt=11
    { 710, 512,1571},  //duty=5, dutyLt=11
    { 728, 512,1522},  //duty=6, dutyLt=11
    { 744, 512,1482},  //duty=7, dutyLt=11
    { 758, 512,1447},  //duty=8, dutyLt=11
    { 772, 512,1418},  //duty=9, dutyLt=11
    { 784, 512,1393},  //duty=10, dutyLt=11
    { 795, 512,1370},  //duty=11, dutyLt=11
    { 805, 512,1350},  //duty=12, dutyLt=11
    { 814, 512,1332},  //duty=13, dutyLt=11
    { 823, 512,1316},  //duty=14, dutyLt=11
    { 831, 512,1302},  //duty=15, dutyLt=11
    { 846, 512,1275},  //duty=16, dutyLt=11
    { 859, 512,1253},  //duty=17, dutyLt=11
    { 870, 512,1235},  //duty=18, dutyLt=11
    { 880, 512,1219},  //duty=19, dutyLt=11
    { 889, 512,1204},  //duty=20, dutyLt=11
    { 896, 512,1191},  //duty=21, dutyLt=11
    { 903, 512,1179},  //duty=22, dutyLt=11
    { 909, 512,1168},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 539, 512,2564},  //duty=-1, dutyLt=12
    { 576, 512,2210},  //duty=0, dutyLt=12
    { 608, 512,1983},  //duty=1, dutyLt=12
    { 637, 512,1834},  //duty=2, dutyLt=12
    { 661, 512,1730},  //duty=3, dutyLt=12
    { 683, 512,1653},  //duty=4, dutyLt=12
    { 702, 512,1593},  //duty=5, dutyLt=12
    { 720, 512,1544},  //duty=6, dutyLt=12
    { 735, 512,1501},  //duty=7, dutyLt=12
    { 749, 512,1466},  //duty=8, dutyLt=12
    { 763, 512,1435},  //duty=9, dutyLt=12
    { 774, 512,1410},  //duty=10, dutyLt=12
    { 785, 512,1386},  //duty=11, dutyLt=12
    { 795, 512,1366},  //duty=12, dutyLt=12
    { 805, 512,1347},  //duty=13, dutyLt=12
    { 813, 512,1332},  //duty=14, dutyLt=12
    { 821, 512,1316},  //duty=15, dutyLt=12
    { 837, 512,1289},  //duty=16, dutyLt=12
    { 849, 512,1266},  //duty=17, dutyLt=12
    { 860, 512,1248},  //duty=18, dutyLt=12
    { 870, 512,1230},  //duty=19, dutyLt=12
    { 879, 512,1215},  //duty=20, dutyLt=12
    { 886, 512,1201},  //duty=21, dutyLt=12
    { 893, 512,1189},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 512, 512, 512},  //duty=28, dutyLt=12
    { 512, 512, 512},  //duty=29, dutyLt=12
    { 539, 512,2566},  //duty=-1, dutyLt=13
    { 574, 512,2213},  //duty=0, dutyLt=13
    { 605, 512,2009},  //duty=1, dutyLt=13
    { 632, 512,1863},  //duty=2, dutyLt=13
    { 656, 512,1753},  //duty=3, dutyLt=13
    { 676, 512,1673},  //duty=4, dutyLt=13
    { 695, 512,1610},  //duty=5, dutyLt=13
    { 712, 512,1560},  //duty=6, dutyLt=13
    { 727, 512,1518},  //duty=7, dutyLt=13
    { 741, 512,1484},  //duty=8, dutyLt=13
    { 754, 512,1454},  //duty=9, dutyLt=13
    { 765, 512,1427},  //duty=10, dutyLt=13
    { 776, 512,1403},  //duty=11, dutyLt=13
    { 786, 512,1382},  //duty=12, dutyLt=13
    { 795, 512,1363},  //duty=13, dutyLt=13
    { 804, 512,1346},  //duty=14, dutyLt=13
    { 812, 512,1331},  //duty=15, dutyLt=13
    { 827, 512,1302},  //duty=16, dutyLt=13
    { 839, 512,1279},  //duty=17, dutyLt=13
    { 851, 512,1259},  //duty=18, dutyLt=13
    { 861, 512,1242},  //duty=19, dutyLt=13
    { 870, 512,1227},  //duty=20, dutyLt=13
    { 877, 512,1212},  //duty=21, dutyLt=13
    { 884, 512,1199},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 539, 512,2579},  //duty=-1, dutyLt=14
    { 573, 512,2228},  //duty=0, dutyLt=14
    { 602, 512,2020},  //duty=1, dutyLt=14
    { 628, 512,1881},  //duty=2, dutyLt=14
    { 651, 512,1778},  //duty=3, dutyLt=14
    { 671, 512,1697},  //duty=4, dutyLt=14
    { 689, 512,1633},  //duty=5, dutyLt=14
    { 705, 512,1583},  //duty=6, dutyLt=14
    { 720, 512,1539},  //duty=7, dutyLt=14
    { 734, 512,1501},  //duty=8, dutyLt=14
    { 746, 512,1470},  //duty=9, dutyLt=14
    { 758, 512,1442},  //duty=10, dutyLt=14
    { 768, 512,1417},  //duty=11, dutyLt=14
    { 778, 512,1397},  //duty=12, dutyLt=14
    { 787, 512,1377},  //duty=13, dutyLt=14
    { 795, 512,1359},  //duty=14, dutyLt=14
    { 803, 512,1343},  //duty=15, dutyLt=14
    { 819, 512,1314},  //duty=16, dutyLt=14
    { 831, 512,1290},  //duty=17, dutyLt=14
    { 842, 512,1270},  //duty=18, dutyLt=14
    { 852, 512,1252},  //duty=19, dutyLt=14
    { 861, 512,1236},  //duty=20, dutyLt=14
    { 869, 512,1221},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 512, 512, 512},  //duty=28, dutyLt=14
    { 512, 512, 512},  //duty=29, dutyLt=14
    { 539, 512,2559},  //duty=-1, dutyLt=15
    { 571, 512,2250},  //duty=0, dutyLt=15
    { 599, 512,2039},  //duty=1, dutyLt=15
    { 624, 512,1892},  //duty=2, dutyLt=15
    { 646, 512,1791},  //duty=3, dutyLt=15
    { 666, 512,1711},  //duty=4, dutyLt=15
    { 683, 512,1648},  //duty=5, dutyLt=15
    { 699, 512,1598},  //duty=6, dutyLt=15
    { 714, 512,1553},  //duty=7, dutyLt=15
    { 727, 512,1516},  //duty=8, dutyLt=15
    { 739, 512,1485},  //duty=9, dutyLt=15
    { 751, 512,1458},  //duty=10, dutyLt=15
    { 761, 512,1432},  //duty=11, dutyLt=15
    { 771, 512,1410},  //duty=12, dutyLt=15
    { 780, 512,1390},  //duty=13, dutyLt=15
    { 788, 512,1372},  //duty=14, dutyLt=15
    { 796, 512,1356},  //duty=15, dutyLt=15
    { 811, 512,1326},  //duty=16, dutyLt=15
    { 823, 512,1301},  //duty=17, dutyLt=15
    { 835, 512,1280},  //duty=18, dutyLt=15
    { 844, 512,1262},  //duty=19, dutyLt=15
    { 853, 512,1246},  //duty=20, dutyLt=15
    { 861, 512,1231},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 539, 512,2565},  //duty=-1, dutyLt=16
    { 569, 512,2263},  //duty=0, dutyLt=16
    { 596, 512,2059},  //duty=1, dutyLt=16
    { 619, 512,1925},  //duty=2, dutyLt=16
    { 639, 512,1822},  //duty=3, dutyLt=16
    { 658, 512,1739},  //duty=4, dutyLt=16
    { 675, 512,1676},  //duty=5, dutyLt=16
    { 690, 512,1625},  //duty=6, dutyLt=16
    { 704, 512,1580},  //duty=7, dutyLt=16
    { 717, 512,1543},  //duty=8, dutyLt=16
    { 728, 512,1511},  //duty=9, dutyLt=16
    { 739, 512,1481},  //duty=10, dutyLt=16
    { 750, 512,1455},  //duty=11, dutyLt=16
    { 759, 512,1433},  //duty=12, dutyLt=16
    { 767, 512,1413},  //duty=13, dutyLt=16
    { 776, 512,1394},  //duty=14, dutyLt=16
    { 783, 512,1378},  //duty=15, dutyLt=16
    { 797, 512,1348},  //duty=16, dutyLt=16
    { 810, 512,1323},  //duty=17, dutyLt=16
    { 821, 512,1301},  //duty=18, dutyLt=16
    { 831, 512,1281},  //duty=19, dutyLt=16
    { 839, 512,1264},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 512, 512, 512},  //duty=28, dutyLt=16
    { 512, 512, 512},  //duty=29, dutyLt=16
    { 540, 512,2554},  //duty=-1, dutyLt=17
    { 567, 512,2277},  //duty=0, dutyLt=17
    { 592, 512,2084},  //duty=1, dutyLt=17
    { 614, 512,1948},  //duty=2, dutyLt=17
    { 633, 512,1846},  //duty=3, dutyLt=17
    { 651, 512,1767},  //duty=4, dutyLt=17
    { 667, 512,1704},  //duty=5, dutyLt=17
    { 681, 512,1651},  //duty=6, dutyLt=17
    { 695, 512,1606},  //duty=7, dutyLt=17
    { 707, 512,1567},  //duty=8, dutyLt=17
    { 718, 512,1535},  //duty=9, dutyLt=17
    { 729, 512,1506},  //duty=10, dutyLt=17
    { 739, 512,1479},  //duty=11, dutyLt=17
    { 748, 512,1456},  //duty=12, dutyLt=17
    { 756, 512,1435},  //duty=13, dutyLt=17
    { 764, 512,1416},  //duty=14, dutyLt=17
    { 772, 512,1398},  //duty=15, dutyLt=17
    { 786, 512,1368},  //duty=16, dutyLt=17
    { 798, 512,1342},  //duty=17, dutyLt=17
    { 808, 512,1319},  //duty=18, dutyLt=17
    { 818, 512,1299},  //duty=19, dutyLt=17
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
    { 540, 512,2549},  //duty=-1, dutyLt=18
    { 565, 512,2290},  //duty=0, dutyLt=18
    { 589, 512,2100},  //duty=1, dutyLt=18
    { 609, 512,1968},  //duty=2, dutyLt=18
    { 628, 512,1871},  //duty=3, dutyLt=18
    { 645, 512,1792},  //duty=4, dutyLt=18
    { 660, 512,1729},  //duty=5, dutyLt=18
    { 674, 512,1675},  //duty=6, dutyLt=18
    { 687, 512,1629},  //duty=7, dutyLt=18
    { 699, 512,1589},  //duty=8, dutyLt=18
    { 710, 512,1556},  //duty=9, dutyLt=18
    { 720, 512,1527},  //duty=10, dutyLt=18
    { 730, 512,1500},  //duty=11, dutyLt=18
    { 739, 512,1476},  //duty=12, dutyLt=18
    { 747, 512,1455},  //duty=13, dutyLt=18
    { 755, 512,1435},  //duty=14, dutyLt=18
    { 762, 512,1418},  //duty=15, dutyLt=18
    { 775, 512,1386},  //duty=16, dutyLt=18
    { 787, 512,1359},  //duty=17, dutyLt=18
    { 798, 512,1336},  //duty=18, dutyLt=18
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
    { 541, 512,2541},  //duty=-1, dutyLt=19
    { 564, 512,2302},  //duty=0, dutyLt=19
    { 587, 512,2114},  //duty=1, dutyLt=19
    { 606, 512,1988},  //duty=2, dutyLt=19
    { 624, 512,1892},  //duty=3, dutyLt=19
    { 640, 512,1815},  //duty=4, dutyLt=19
    { 655, 512,1750},  //duty=5, dutyLt=19
    { 668, 512,1696},  //duty=6, dutyLt=19
    { 681, 512,1649},  //duty=7, dutyLt=19
    { 692, 512,1610},  //duty=8, dutyLt=19
    { 703, 512,1576},  //duty=9, dutyLt=19
    { 712, 512,1548},  //duty=10, dutyLt=19
    { 722, 512,1520},  //duty=11, dutyLt=19
    { 731, 512,1495},  //duty=12, dutyLt=19
    { 739, 512,1473},  //duty=13, dutyLt=19
    { 746, 512,1454},  //duty=14, dutyLt=19
    { 754, 512,1435},  //duty=15, dutyLt=19
    { 767, 512,1403},  //duty=16, dutyLt=19
    { 779, 512,1375},  //duty=17, dutyLt=19
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
    { 541, 512,2539},  //duty=-1, dutyLt=20
    { 564, 512,2304},  //duty=0, dutyLt=20
    { 585, 512,2126},  //duty=1, dutyLt=20
    { 604, 512,2007},  //duty=2, dutyLt=20
    { 620, 512,1911},  //duty=3, dutyLt=20
    { 636, 512,1832},  //duty=4, dutyLt=20
    { 650, 512,1769},  //duty=5, dutyLt=20
    { 663, 512,1715},  //duty=6, dutyLt=20
    { 675, 512,1668},  //duty=7, dutyLt=20
    { 686, 512,1628},  //duty=8, dutyLt=20
    { 696, 512,1593},  //duty=9, dutyLt=20
    { 706, 512,1563},  //duty=10, dutyLt=20
    { 715, 512,1536},  //duty=11, dutyLt=20
    { 724, 512,1511},  //duty=12, dutyLt=20
    { 732, 512,1490},  //duty=13, dutyLt=20
    { 739, 512,1469},  //duty=14, dutyLt=20
    { 746, 512,1450},  //duty=15, dutyLt=20
    { 759, 512,1419},  //duty=16, dutyLt=20
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
    { 542, 512,2532},  //duty=-1, dutyLt=21
    { 563, 512,2313},  //duty=0, dutyLt=21
    { 584, 512,2137},  //duty=1, dutyLt=21
    { 601, 512,2018},  //duty=2, dutyLt=21
    { 618, 512,1925},  //duty=3, dutyLt=21
    { 633, 512,1847},  //duty=4, dutyLt=21
    { 646, 512,1784},  //duty=5, dutyLt=21
    { 659, 512,1730},  //duty=6, dutyLt=21
    { 670, 512,1685},  //duty=7, dutyLt=21
    { 681, 512,1643},  //duty=8, dutyLt=21
    { 692, 512,1609},  //duty=9, dutyLt=21
    { 701, 512,1579},  //duty=10, dutyLt=21
    { 709, 512,1553},  //duty=11, dutyLt=21
    { 718, 512,1526},  //duty=12, dutyLt=21
    { 726, 512,1504},  //duty=13, dutyLt=21
    { 733, 512,1483},  //duty=14, dutyLt=21
    { 740, 512,1464},  //duty=15, dutyLt=21
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
    { 542, 512,2528},  //duty=-1, dutyLt=22
    { 563, 512,2313},  //duty=0, dutyLt=22
    { 583, 512,2145},  //duty=1, dutyLt=22
    { 600, 512,2032},  //duty=2, dutyLt=22
    { 615, 512,1938},  //duty=3, dutyLt=22
    { 630, 512,1861},  //duty=4, dutyLt=22
    { 643, 512,1798},  //duty=5, dutyLt=22
    { 655, 512,1746},  //duty=6, dutyLt=22
    { 667, 512,1698},  //duty=7, dutyLt=22
    { 677, 512,1658},  //duty=8, dutyLt=22
    { 687, 512,1623},  //duty=9, dutyLt=22
    { 696, 512,1593},  //duty=10, dutyLt=22
    { 705, 512,1564},  //duty=11, dutyLt=22
    { 712, 512,1542},  //duty=12, dutyLt=22
    { 720, 512,1517},  //duty=13, dutyLt=22
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
    { 543, 512,2519},  //duty=-1, dutyLt=23
    { 562, 512,2317},  //duty=0, dutyLt=23
    { 582, 512,2155},  //duty=1, dutyLt=23
    { 598, 512,2041},  //duty=2, dutyLt=23
    { 613, 512,1951},  //duty=3, dutyLt=23
    { 627, 512,1873},  //duty=4, dutyLt=23
    { 640, 512,1810},  //duty=5, dutyLt=23
    { 652, 512,1757},  //duty=6, dutyLt=23
    { 663, 512,1711},  //duty=7, dutyLt=23
    { 674, 512,1670},  //duty=8, dutyLt=23
    { 683, 512,1635},  //duty=9, dutyLt=23
    { 692, 512,1604},  //duty=10, dutyLt=23
    { 701, 512,1576},  //duty=11, dutyLt=23
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
    { 544, 512,2509},  //duty=-1, dutyLt=24
    { 563, 512,2317},  //duty=0, dutyLt=24
    { 581, 512,2159},  //duty=1, dutyLt=24
    { 598, 512,2046},  //duty=2, dutyLt=24
    { 612, 512,1955},  //duty=3, dutyLt=24
    { 625, 512,1885},  //duty=4, dutyLt=24
    { 638, 512,1820},  //duty=5, dutyLt=24
    { 650, 512,1766},  //duty=6, dutyLt=24
    { 660, 512,1721},  //duty=7, dutyLt=24
    { 670, 512,1682},  //duty=8, dutyLt=24
    { 680, 512,1647},  //duty=9, dutyLt=24
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
    { 545, 512,2502},  //duty=-1, dutyLt=25
    { 563, 512,2316},  //duty=0, dutyLt=25
    { 581, 512,2164},  //duty=1, dutyLt=25
    { 596, 512,2054},  //duty=2, dutyLt=25
    { 611, 512,1965},  //duty=3, dutyLt=25
    { 623, 512,1892},  //duty=4, dutyLt=25
    { 636, 512,1827},  //duty=5, dutyLt=25
    { 647, 512,1775},  //duty=6, dutyLt=25
    { 658, 512,1730},  //duty=7, dutyLt=25
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
    { 545, 512,2496},  //duty=-1, dutyLt=26
    { 563, 512,2314},  //duty=0, dutyLt=26
    { 581, 512,2165},  //duty=1, dutyLt=26
    { 595, 512,2060},  //duty=2, dutyLt=26
    { 610, 512,1970},  //duty=3, dutyLt=26
    { 623, 512,1898},  //duty=4, dutyLt=26
    { 635, 512,1837},  //duty=5, dutyLt=26
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
    { 546, 512,2491},  //duty=-1, dutyLt=27
    { 564, 512,2312},  //duty=0, dutyLt=27
    { 580, 512,2171},  //duty=1, dutyLt=27
    { 596, 512,2061},  //duty=2, dutyLt=27
    { 609, 512,1978},  //duty=3, dutyLt=27
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
    { 547, 512,2483},  //duty=-1, dutyLt=28
    { 564, 512,2310},  //duty=0, dutyLt=28
    { 581, 512,2168},  //duty=1, dutyLt=28
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
    { 548, 512,2477},  //duty=-1, dutyLt=29
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

