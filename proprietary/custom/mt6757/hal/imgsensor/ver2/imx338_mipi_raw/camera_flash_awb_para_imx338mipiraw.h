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
   {1068, 512,1103},  //duty=0, dutyLt=-1
   {1072, 512,1098},  //duty=1, dutyLt=-1
   {1068, 512,1089},  //duty=2, dutyLt=-1
   {1066, 512,1080},  //duty=3, dutyLt=-1
   {1065, 512,1072},  //duty=4, dutyLt=-1
   {1064, 512,1061},  //duty=5, dutyLt=-1
   {1068, 512,1067},  //duty=6, dutyLt=-1
   {1068, 512,1063},  //duty=7, dutyLt=-1
   {1067, 512,1052},  //duty=8, dutyLt=-1
   {1064, 512,1041},  //duty=9, dutyLt=-1
   {1064, 512,1034},  //duty=10, dutyLt=-1
   {1063, 512,1026},  //duty=11, dutyLt=-1
   {1061, 512,1020},  //duty=12, dutyLt=-1
   {1061, 512,1013},  //duty=13, dutyLt=-1
   {1060, 512,1006},  //duty=14, dutyLt=-1
   {1059, 512,1001},  //duty=15, dutyLt=-1
   {1058, 512, 994},  //duty=16, dutyLt=-1
   {1058, 512, 989},  //duty=17, dutyLt=-1
   {1058, 512, 984},  //duty=18, dutyLt=-1
   {1058, 512, 980},  //duty=19, dutyLt=-1
   {1057, 512, 976},  //duty=20, dutyLt=-1
   {1057, 512, 971},  //duty=21, dutyLt=-1
   {1057, 512, 968},  //duty=22, dutyLt=-1
   {1058, 512, 964},  //duty=23, dutyLt=-1
   {1057, 512, 959},  //duty=24, dutyLt=-1
   {1057, 512, 956},  //duty=25, dutyLt=-1
   { 523, 512,2358},  //duty=-1, dutyLt=0
   { 746, 512,1428},  //duty=0, dutyLt=0
   { 843, 512,1269},  //duty=1, dutyLt=0
   { 894, 512,1208},  //duty=2, dutyLt=0
   { 924, 512,1172},  //duty=3, dutyLt=0
   { 943, 512,1146},  //duty=4, dutyLt=0
   { 958, 512,1109},  //duty=5, dutyLt=0
   { 972, 512,1124},  //duty=6, dutyLt=0
   { 981, 512,1111},  //duty=7, dutyLt=0
   { 992, 512,1093},  //duty=8, dutyLt=0
   {1002, 512,1076},  //duty=9, dutyLt=0
   {1008, 512,1065},  //duty=10, dutyLt=0
   {1011, 512,1054},  //duty=11, dutyLt=0
   {1015, 512,1046},  //duty=12, dutyLt=0
   {1019, 512,1036},  //duty=13, dutyLt=0
   {1020, 512,1028},  //duty=14, dutyLt=0
   {1022, 512,1021},  //duty=15, dutyLt=0
   {1024, 512,1013},  //duty=16, dutyLt=0
   {1025, 512,1008},  //duty=17, dutyLt=0
   {1027, 512,1003},  //duty=18, dutyLt=0
   {1028, 512, 999},  //duty=19, dutyLt=0
   {1029, 512, 992},  //duty=20, dutyLt=0
   {1030, 512, 987},  //duty=21, dutyLt=0
   {1031, 512, 983},  //duty=22, dutyLt=0
   {1031, 512, 979},  //duty=23, dutyLt=0
   {1032, 512, 973},  //duty=24, dutyLt=0
   { 512, 512, 512},  //duty=25, dutyLt=0
   { 522, 512,2330},  //duty=-1, dutyLt=1
   { 658, 512,1619},  //duty=0, dutyLt=1
   { 743, 512,1409},  //duty=1, dutyLt=1
   { 796, 512,1314},  //duty=2, dutyLt=1
   { 834, 512,1256},  //duty=3, dutyLt=1
   { 860, 512,1207},  //duty=4, dutyLt=1
   { 881, 512,1199},  //duty=5, dutyLt=1
   { 898, 512,1181},  //duty=6, dutyLt=1
   { 918, 512,1157},  //duty=7, dutyLt=1
   { 935, 512,1134},  //duty=8, dutyLt=1
   { 952, 512,1111},  //duty=9, dutyLt=1
   { 961, 512,1096},  //duty=10, dutyLt=1
   { 969, 512,1083},  //duty=11, dutyLt=1
   { 975, 512,1071},  //duty=12, dutyLt=1
   { 981, 512,1059},  //duty=13, dutyLt=1
   { 985, 512,1050},  //duty=14, dutyLt=1
   { 989, 512,1042},  //duty=15, dutyLt=1
   { 992, 512,1033},  //duty=16, dutyLt=1
   { 995, 512,1026},  //duty=17, dutyLt=1
   { 998, 512,1020},  //duty=18, dutyLt=1
   {1000, 512,1015},  //duty=19, dutyLt=1
   {1002, 512,1008},  //duty=20, dutyLt=1
   {1004, 512,1002},  //duty=21, dutyLt=1
   {1006, 512, 998},  //duty=22, dutyLt=1
   {1007, 512, 993},  //duty=23, dutyLt=1
   {1009, 512, 988},  //duty=24, dutyLt=1
   { 512, 512, 512},  //duty=25, dutyLt=1
   { 522, 512,2375},  //duty=-1, dutyLt=2
   { 620, 512,1742},  //duty=0, dutyLt=2
   { 691, 512,1513},  //duty=1, dutyLt=2
   { 741, 512,1396},  //duty=2, dutyLt=2
   { 779, 512,1318},  //duty=3, dutyLt=2
   { 807, 512,1292},  //duty=4, dutyLt=2
   { 828, 512,1258},  //duty=5, dutyLt=2
   { 847, 512,1230},  //duty=6, dutyLt=2
   { 869, 512,1200},  //duty=7, dutyLt=2
   { 892, 512,1171},  //duty=8, dutyLt=2
   { 912, 512,1142},  //duty=9, dutyLt=2
   { 924, 512,1124},  //duty=10, dutyLt=2
   { 934, 512,1108},  //duty=11, dutyLt=2
   { 942, 512,1095},  //duty=12, dutyLt=2
   { 950, 512,1080},  //duty=13, dutyLt=2
   { 956, 512,1070},  //duty=14, dutyLt=2
   { 960, 512,1061},  //duty=15, dutyLt=2
   { 966, 512,1052},  //duty=16, dutyLt=2
   { 969, 512,1043},  //duty=17, dutyLt=2
   { 973, 512,1037},  //duty=18, dutyLt=2
   { 976, 512,1031},  //duty=19, dutyLt=2
   { 979, 512,1023},  //duty=20, dutyLt=2
   { 982, 512,1018},  //duty=21, dutyLt=2
   { 984, 512,1012},  //duty=22, dutyLt=2
   { 986, 512,1007},  //duty=23, dutyLt=2
   { 512, 512, 512},  //duty=24, dutyLt=2
   { 512, 512, 512},  //duty=25, dutyLt=2
   { 521, 512,2331},  //duty=-1, dutyLt=3
   { 599, 512,1820},  //duty=0, dutyLt=3
   { 660, 512,1588},  //duty=1, dutyLt=3
   { 706, 512,1456},  //duty=2, dutyLt=3
   { 740, 512,1400},  //duty=3, dutyLt=3
   { 766, 512,1342},  //duty=4, dutyLt=3
   { 789, 512,1307},  //duty=5, dutyLt=3
   { 809, 512,1274},  //duty=6, dutyLt=3
   { 834, 512,1240},  //duty=7, dutyLt=3
   { 857, 512,1203},  //duty=8, dutyLt=3
   { 879, 512,1170},  //duty=9, dutyLt=3
   { 893, 512,1149},  //duty=10, dutyLt=3
   { 905, 512,1132},  //duty=11, dutyLt=3
   { 914, 512,1117},  //duty=12, dutyLt=3
   { 924, 512,1101},  //duty=13, dutyLt=3
   { 930, 512,1089},  //duty=14, dutyLt=3
   { 936, 512,1079},  //duty=15, dutyLt=3
   { 943, 512,1068},  //duty=16, dutyLt=3
   { 947, 512,1060},  //duty=17, dutyLt=3
   { 951, 512,1052},  //duty=18, dutyLt=3
   { 955, 512,1046},  //duty=19, dutyLt=3
   { 958, 512,1037},  //duty=20, dutyLt=3
   { 961, 512,1031},  //duty=21, dutyLt=3
   { 964, 512,1025},  //duty=22, dutyLt=3
   { 967, 512,1019},  //duty=23, dutyLt=3
   { 512, 512, 512},  //duty=24, dutyLt=3
   { 512, 512, 512},  //duty=25, dutyLt=3
   { 521, 512,2332},  //duty=-1, dutyLt=4
   { 586, 512,1875},  //duty=0, dutyLt=4
   { 640, 512,1642},  //duty=1, dutyLt=4
   { 682, 512,1518},  //duty=2, dutyLt=4
   { 714, 512,1446},  //duty=3, dutyLt=4
   { 740, 512,1389},  //duty=4, dutyLt=4
   { 762, 512,1341},  //duty=5, dutyLt=4
   { 781, 512,1314},  //duty=6, dutyLt=4
   { 805, 512,1275},  //duty=7, dutyLt=4
   { 829, 512,1233},  //duty=8, dutyLt=4
   { 852, 512,1197},  //duty=9, dutyLt=4
   { 867, 512,1173},  //duty=10, dutyLt=4
   { 880, 512,1154},  //duty=11, dutyLt=4
   { 890, 512,1137},  //duty=12, dutyLt=4
   { 901, 512,1120},  //duty=13, dutyLt=4
   { 908, 512,1107},  //duty=14, dutyLt=4
   { 915, 512,1096},  //duty=15, dutyLt=4
   { 922, 512,1084},  //duty=16, dutyLt=4
   { 927, 512,1075},  //duty=17, dutyLt=4
   { 931, 512,1067},  //duty=18, dutyLt=4
   { 935, 512,1060},  //duty=19, dutyLt=4
   { 940, 512,1051},  //duty=20, dutyLt=4
   { 943, 512,1044},  //duty=21, dutyLt=4
   { 946, 512,1038},  //duty=22, dutyLt=4
   { 512, 512, 512},  //duty=23, dutyLt=4
   { 512, 512, 512},  //duty=24, dutyLt=4
   { 512, 512, 512},  //duty=25, dutyLt=4
   { 521, 512,2322},  //duty=-1, dutyLt=5
   { 577, 512,1915},  //duty=0, dutyLt=5
   { 626, 512,1678},  //duty=1, dutyLt=5
   { 662, 512,1569},  //duty=2, dutyLt=5
   { 693, 512,1487},  //duty=3, dutyLt=5
   { 717, 512,1432},  //duty=4, dutyLt=5
   { 739, 512,1382},  //duty=5, dutyLt=5
   { 757, 512,1349},  //duty=6, dutyLt=5
   { 781, 512,1306},  //duty=7, dutyLt=5
   { 806, 512,1261},  //duty=8, dutyLt=5
   { 830, 512,1220},  //duty=9, dutyLt=5
   { 846, 512,1195},  //duty=10, dutyLt=5
   { 858, 512,1174},  //duty=11, dutyLt=5
   { 870, 512,1156},  //duty=12, dutyLt=5
   { 881, 512,1137},  //duty=13, dutyLt=5
   { 889, 512,1123},  //duty=14, dutyLt=5
   { 896, 512,1112},  //duty=15, dutyLt=5
   { 904, 512,1099},  //duty=16, dutyLt=5
   { 910, 512,1090},  //duty=17, dutyLt=5
   { 915, 512,1081},  //duty=18, dutyLt=5
   { 919, 512,1074},  //duty=19, dutyLt=5
   { 924, 512,1064},  //duty=20, dutyLt=5
   { 927, 512,1056},  //duty=21, dutyLt=5
   { 931, 512,1050},  //duty=22, dutyLt=5
   { 512, 512, 512},  //duty=23, dutyLt=5
   { 512, 512, 512},  //duty=24, dutyLt=5
   { 512, 512, 512},  //duty=25, dutyLt=5
   { 521, 512,2311},  //duty=-1, dutyLt=6
   { 570, 512,1940},  //duty=0, dutyLt=6
   { 613, 512,1743},  //duty=1, dutyLt=6
   { 648, 512,1613},  //duty=2, dutyLt=6
   { 676, 512,1523},  //duty=3, dutyLt=6
   { 699, 512,1470},  //duty=4, dutyLt=6
   { 720, 512,1417},  //duty=5, dutyLt=6
   { 738, 512,1378},  //duty=6, dutyLt=6
   { 762, 512,1334},  //duty=7, dutyLt=6
   { 787, 512,1287},  //duty=8, dutyLt=6
   { 811, 512,1243},  //duty=9, dutyLt=6
   { 827, 512,1217},  //duty=10, dutyLt=6
   { 840, 512,1194},  //duty=11, dutyLt=6
   { 851, 512,1174},  //duty=12, dutyLt=6
   { 863, 512,1154},  //duty=13, dutyLt=6
   { 872, 512,1139},  //duty=14, dutyLt=6
   { 879, 512,1127},  //duty=15, dutyLt=6
   { 888, 512,1113},  //duty=16, dutyLt=6
   { 894, 512,1103},  //duty=17, dutyLt=6
   { 899, 512,1094},  //duty=18, dutyLt=6
   { 903, 512,1086},  //duty=19, dutyLt=6
   { 909, 512,1076},  //duty=20, dutyLt=6
   { 913, 512,1068},  //duty=21, dutyLt=6
   { 512, 512, 512},  //duty=22, dutyLt=6
   { 512, 512, 512},  //duty=23, dutyLt=6
   { 512, 512, 512},  //duty=24, dutyLt=6
   { 512, 512, 512},  //duty=25, dutyLt=6
   { 521, 512,2301},  //duty=-1, dutyLt=7
   { 567, 512,1945},  //duty=0, dutyLt=7
   { 601, 512,1794},  //duty=1, dutyLt=7
   { 632, 512,1663},  //duty=2, dutyLt=7
   { 657, 512,1576},  //duty=3, dutyLt=7
   { 680, 512,1515},  //duty=4, dutyLt=7
   { 699, 512,1460},  //duty=5, dutyLt=7
   { 716, 512,1422},  //duty=6, dutyLt=7
   { 738, 512,1372},  //duty=7, dutyLt=7
   { 762, 512,1322},  //duty=8, dutyLt=7
   { 786, 512,1275},  //duty=9, dutyLt=7
   { 802, 512,1245},  //duty=10, dutyLt=7
   { 816, 512,1221},  //duty=11, dutyLt=7
   { 828, 512,1199},  //duty=12, dutyLt=7
   { 841, 512,1177},  //duty=13, dutyLt=7
   { 850, 512,1162},  //duty=14, dutyLt=7
   { 858, 512,1149},  //duty=15, dutyLt=7
   { 866, 512,1133},  //duty=16, dutyLt=7
   { 873, 512,1123},  //duty=17, dutyLt=7
   { 878, 512,1112},  //duty=18, dutyLt=7
   { 883, 512,1103},  //duty=19, dutyLt=7
   { 889, 512,1093},  //duty=20, dutyLt=7
   { 893, 512,1085},  //duty=21, dutyLt=7
   { 512, 512, 512},  //duty=22, dutyLt=7
   { 512, 512, 512},  //duty=23, dutyLt=7
   { 512, 512, 512},  //duty=24, dutyLt=7
   { 512, 512, 512},  //duty=25, dutyLt=7
   { 523, 512,2296},  //duty=-1, dutyLt=8
   { 558, 512,2021},  //duty=0, dutyLt=8
   { 589, 512,1847},  //duty=1, dutyLt=8
   { 616, 512,1715},  //duty=2, dutyLt=8
   { 639, 512,1627},  //duty=3, dutyLt=8
   { 659, 512,1566},  //duty=4, dutyLt=8
   { 677, 512,1509},  //duty=5, dutyLt=8
   { 693, 512,1469},  //duty=6, dutyLt=8
   { 713, 512,1416},  //duty=7, dutyLt=8
   { 737, 512,1361},  //duty=8, dutyLt=8
   { 761, 512,1312},  //duty=9, dutyLt=8
   { 777, 512,1279},  //duty=10, dutyLt=8
   { 790, 512,1252},  //duty=11, dutyLt=8
   { 803, 512,1229},  //duty=12, dutyLt=8
   { 816, 512,1206},  //duty=13, dutyLt=8
   { 825, 512,1189},  //duty=14, dutyLt=8
   { 833, 512,1174},  //duty=15, dutyLt=8
   { 842, 512,1157},  //duty=16, dutyLt=8
   { 849, 512,1146},  //duty=17, dutyLt=8
   { 855, 512,1135},  //duty=18, dutyLt=8
   { 859, 512,1125},  //duty=19, dutyLt=8
   { 866, 512,1113},  //duty=20, dutyLt=8
   { 512, 512, 512},  //duty=21, dutyLt=8
   { 512, 512, 512},  //duty=22, dutyLt=8
   { 512, 512, 512},  //duty=23, dutyLt=8
   { 512, 512, 512},  //duty=24, dutyLt=8
   { 512, 512, 512},  //duty=25, dutyLt=8
   { 522, 512,2302},  //duty=-1, dutyLt=9
   { 552, 512,2047},  //duty=0, dutyLt=9
   { 579, 512,1876},  //duty=1, dutyLt=9
   { 602, 512,1764},  //duty=2, dutyLt=9
   { 623, 512,1685},  //duty=3, dutyLt=9
   { 640, 512,1613},  //duty=4, dutyLt=9
   { 657, 512,1565},  //duty=5, dutyLt=9
   { 672, 512,1516},  //duty=6, dutyLt=9
   { 691, 512,1461},  //duty=7, dutyLt=9
   { 713, 512,1405},  //duty=8, dutyLt=9
   { 736, 512,1351},  //duty=9, dutyLt=9
   { 752, 512,1316},  //duty=10, dutyLt=9
   { 765, 512,1287},  //duty=11, dutyLt=9
   { 777, 512,1262},  //duty=12, dutyLt=9
   { 790, 512,1237},  //duty=13, dutyLt=9
   { 800, 512,1219},  //duty=14, dutyLt=9
   { 808, 512,1202},  //duty=15, dutyLt=9
   { 818, 512,1184},  //duty=16, dutyLt=9
   { 824, 512,1171},  //duty=17, dutyLt=9
   { 830, 512,1159},  //duty=18, dutyLt=9
   { 836, 512,1149},  //duty=19, dutyLt=9
   { 512, 512, 512},  //duty=20, dutyLt=9
   { 512, 512, 512},  //duty=21, dutyLt=9
   { 512, 512, 512},  //duty=22, dutyLt=9
   { 512, 512, 512},  //duty=23, dutyLt=9
   { 512, 512, 512},  //duty=24, dutyLt=9
   { 512, 512, 512},  //duty=25, dutyLt=9
   { 522, 512,2287},  //duty=-1, dutyLt=10
   { 549, 512,2060},  //duty=0, dutyLt=10
   { 574, 512,1894},  //duty=1, dutyLt=10
   { 594, 512,1796},  //duty=2, dutyLt=10
   { 613, 512,1716},  //duty=3, dutyLt=10
   { 629, 512,1647},  //duty=4, dutyLt=10
   { 645, 512,1597},  //duty=5, dutyLt=10
   { 658, 512,1549},  //duty=6, dutyLt=10
   { 677, 512,1492},  //duty=7, dutyLt=10
   { 698, 512,1434},  //duty=8, dutyLt=10
   { 720, 512,1378},  //duty=9, dutyLt=10
   { 735, 512,1342},  //duty=10, dutyLt=10
   { 749, 512,1311},  //duty=11, dutyLt=10
   { 761, 512,1287},  //duty=12, dutyLt=10
   { 774, 512,1260},  //duty=13, dutyLt=10
   { 784, 512,1240},  //duty=14, dutyLt=10
   { 792, 512,1223},  //duty=15, dutyLt=10
   { 801, 512,1204},  //duty=16, dutyLt=10
   { 808, 512,1190},  //duty=17, dutyLt=10
   { 814, 512,1177},  //duty=18, dutyLt=10
   { 512, 512, 512},  //duty=19, dutyLt=10
   { 512, 512, 512},  //duty=20, dutyLt=10
   { 512, 512, 512},  //duty=21, dutyLt=10
   { 512, 512, 512},  //duty=22, dutyLt=10
   { 512, 512, 512},  //duty=23, dutyLt=10
   { 512, 512, 512},  //duty=24, dutyLt=10
   { 512, 512, 512},  //duty=25, dutyLt=10
   { 522, 512,2274},  //duty=-1, dutyLt=11
   { 546, 512,2069},  //duty=0, dutyLt=11
   { 568, 512,1927},  //duty=1, dutyLt=11
   { 588, 512,1825},  //duty=2, dutyLt=11
   { 605, 512,1743},  //duty=3, dutyLt=11
   { 621, 512,1675},  //duty=4, dutyLt=11
   { 635, 512,1623},  //duty=5, dutyLt=11
   { 648, 512,1578},  //duty=6, dutyLt=11
   { 665, 512,1520},  //duty=7, dutyLt=11
   { 686, 512,1460},  //duty=8, dutyLt=11
   { 707, 512,1403},  //duty=9, dutyLt=11
   { 722, 512,1366},  //duty=10, dutyLt=11
   { 735, 512,1335},  //duty=11, dutyLt=11
   { 747, 512,1308},  //duty=12, dutyLt=11
   { 760, 512,1279},  //duty=13, dutyLt=11
   { 769, 512,1260},  //duty=14, dutyLt=11
   { 778, 512,1242},  //duty=15, dutyLt=11
   { 787, 512,1222},  //duty=16, dutyLt=11
   { 794, 512,1207},  //duty=17, dutyLt=11
   { 512, 512, 512},  //duty=18, dutyLt=11
   { 512, 512, 512},  //duty=19, dutyLt=11
   { 512, 512, 512},  //duty=20, dutyLt=11
   { 512, 512, 512},  //duty=21, dutyLt=11
   { 512, 512, 512},  //duty=22, dutyLt=11
   { 512, 512, 512},  //duty=23, dutyLt=11
   { 512, 512, 512},  //duty=24, dutyLt=11
   { 512, 512, 512},  //duty=25, dutyLt=11
   { 522, 512,2263},  //duty=-1, dutyLt=12
   { 544, 512,2077},  //duty=0, dutyLt=12
   { 564, 512,1946},  //duty=1, dutyLt=12
   { 582, 512,1849},  //duty=2, dutyLt=12
   { 599, 512,1766},  //duty=3, dutyLt=12
   { 613, 512,1701},  //duty=4, dutyLt=12
   { 627, 512,1647},  //duty=5, dutyLt=12
   { 639, 512,1603},  //duty=6, dutyLt=12
   { 656, 512,1545},  //duty=7, dutyLt=12
   { 675, 512,1483},  //duty=8, dutyLt=12
   { 696, 512,1425},  //duty=9, dutyLt=12
   { 710, 512,1388},  //duty=10, dutyLt=12
   { 723, 512,1355},  //duty=11, dutyLt=12
   { 735, 512,1328},  //duty=12, dutyLt=12
   { 748, 512,1299},  //duty=13, dutyLt=12
   { 757, 512,1277},  //duty=14, dutyLt=12
   { 765, 512,1259},  //duty=15, dutyLt=12
   { 775, 512,1238},  //duty=16, dutyLt=12
   { 512, 512, 512},  //duty=17, dutyLt=12
   { 512, 512, 512},  //duty=18, dutyLt=12
   { 512, 512, 512},  //duty=19, dutyLt=12
   { 512, 512, 512},  //duty=20, dutyLt=12
   { 512, 512, 512},  //duty=21, dutyLt=12
   { 512, 512, 512},  //duty=22, dutyLt=12
   { 512, 512, 512},  //duty=23, dutyLt=12
   { 512, 512, 512},  //duty=24, dutyLt=12
   { 512, 512, 512},  //duty=25, dutyLt=12
   { 522, 512,2259},  //duty=-1, dutyLt=13
   { 542, 512,2089},  //duty=0, dutyLt=13
   { 560, 512,1962},  //duty=1, dutyLt=13
   { 577, 512,1868},  //duty=2, dutyLt=13
   { 592, 512,1790},  //duty=3, dutyLt=13
   { 606, 512,1725},  //duty=4, dutyLt=13
   { 618, 512,1674},  //duty=5, dutyLt=13
   { 630, 512,1629},  //duty=6, dutyLt=13
   { 646, 512,1570},  //duty=7, dutyLt=13
   { 664, 512,1508},  //duty=8, dutyLt=13
   { 684, 512,1449},  //duty=9, dutyLt=13
   { 698, 512,1411},  //duty=10, dutyLt=13
   { 711, 512,1378},  //duty=11, dutyLt=13
   { 723, 512,1349},  //duty=12, dutyLt=13
   { 735, 512,1319},  //duty=13, dutyLt=13
   { 744, 512,1297},  //duty=14, dutyLt=13
   { 752, 512,1278},  //duty=15, dutyLt=13
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
   { 522, 512,2246},  //duty=-1, dutyLt=14
   { 541, 512,2094},  //duty=0, dutyLt=14
   { 558, 512,1978},  //duty=1, dutyLt=14
   { 574, 512,1883},  //duty=2, dutyLt=14
   { 588, 512,1803},  //duty=3, dutyLt=14
   { 601, 512,1745},  //duty=4, dutyLt=14
   { 613, 512,1689},  //duty=5, dutyLt=14
   { 624, 512,1646},  //duty=6, dutyLt=14
   { 640, 512,1588},  //duty=7, dutyLt=14
   { 657, 512,1526},  //duty=8, dutyLt=14
   { 677, 512,1467},  //duty=9, dutyLt=14
   { 693, 512,1417},  //duty=10, dutyLt=14
   { 703, 512,1395},  //duty=11, dutyLt=14
   { 714, 512,1365},  //duty=12, dutyLt=14
   { 726, 512,1334},  //duty=13, dutyLt=14
   { 735, 512,1312},  //duty=14, dutyLt=14
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
   { 522, 512,2250},  //duty=-1, dutyLt=15
   { 540, 512,2105},  //duty=0, dutyLt=15
   { 556, 512,1985},  //duty=1, dutyLt=15
   { 571, 512,1891},  //duty=2, dutyLt=15
   { 585, 512,1821},  //duty=3, dutyLt=15
   { 597, 512,1759},  //duty=4, dutyLt=15
   { 608, 512,1708},  //duty=5, dutyLt=15
   { 619, 512,1660},  //duty=6, dutyLt=15
   { 634, 512,1603},  //duty=7, dutyLt=15
   { 651, 512,1542},  //duty=8, dutyLt=15
   { 670, 512,1483},  //duty=9, dutyLt=15
   { 683, 512,1443},  //duty=10, dutyLt=15
   { 695, 512,1409},  //duty=11, dutyLt=15
   { 706, 512,1379},  //duty=12, dutyLt=15
   { 718, 512,1348},  //duty=13, dutyLt=15
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
   { 522, 512,2242},  //duty=-1, dutyLt=16
   { 539, 512,2106},  //duty=0, dutyLt=16
   { 557, 512,1971},  //duty=1, dutyLt=16
   { 568, 512,1904},  //duty=2, dutyLt=16
   { 581, 512,1835},  //duty=3, dutyLt=16
   { 593, 512,1773},  //duty=4, dutyLt=16
   { 603, 512,1724},  //duty=5, dutyLt=16
   { 614, 512,1677},  //duty=6, dutyLt=16
   { 628, 512,1620},  //duty=7, dutyLt=16
   { 645, 512,1560},  //duty=8, dutyLt=16
   { 663, 512,1501},  //duty=9, dutyLt=16
   { 675, 512,1460},  //duty=10, dutyLt=16
   { 687, 512,1426},  //duty=11, dutyLt=16
   { 698, 512,1396},  //duty=12, dutyLt=16
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
   { 522, 512,2234},  //duty=-1, dutyLt=17
   { 538, 512,2101},  //duty=0, dutyLt=17
   { 553, 512,1997},  //duty=1, dutyLt=17
   { 570, 512,1888},  //duty=2, dutyLt=17
   { 578, 512,1843},  //duty=3, dutyLt=17
   { 590, 512,1785},  //duty=4, dutyLt=17
   { 600, 512,1733},  //duty=5, dutyLt=17
   { 610, 512,1690},  //duty=6, dutyLt=17
   { 624, 512,1634},  //duty=7, dutyLt=17
   { 640, 512,1575},  //duty=8, dutyLt=17
   { 657, 512,1516},  //duty=9, dutyLt=17
   { 670, 512,1472},  //duty=10, dutyLt=17
   { 682, 512,1437},  //duty=11, dutyLt=17
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
   { 523, 512,2230},  //duty=-1, dutyLt=18
   { 537, 512,2109},  //duty=0, dutyLt=18
   { 552, 512,2003},  //duty=1, dutyLt=18
   { 564, 512,1919},  //duty=2, dutyLt=18
   { 576, 512,1853},  //duty=3, dutyLt=18
   { 587, 512,1792},  //duty=4, dutyLt=18
   { 597, 512,1744},  //duty=5, dutyLt=18
   { 607, 512,1700},  //duty=6, dutyLt=18
   { 620, 512,1645},  //duty=7, dutyLt=18
   { 635, 512,1586},  //duty=8, dutyLt=18
   { 653, 512,1524},  //duty=9, dutyLt=18
   { 665, 512,1483},  //duty=10, dutyLt=18
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
   { 523, 512,2222},  //duty=-1, dutyLt=19
   { 537, 512,2104},  //duty=0, dutyLt=19
   { 551, 512,2008},  //duty=1, dutyLt=19
   { 563, 512,1924},  //duty=2, dutyLt=19
   { 575, 512,1857},  //duty=3, dutyLt=19
   { 585, 512,1800},  //duty=4, dutyLt=19
   { 595, 512,1751},  //duty=5, dutyLt=19
   { 604, 512,1709},  //duty=6, dutyLt=19
   { 618, 512,1654},  //duty=7, dutyLt=19
   { 633, 512,1594},  //duty=8, dutyLt=19
   { 649, 512,1534},  //duty=9, dutyLt=19
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
   { 523, 512,2217},  //duty=-1, dutyLt=20
   { 537, 512,2106},  //duty=0, dutyLt=20
   { 550, 512,2011},  //duty=1, dutyLt=20
   { 562, 512,1929},  //duty=2, dutyLt=20
   { 573, 512,1866},  //duty=3, dutyLt=20
   { 583, 512,1808},  //duty=4, dutyLt=20
   { 592, 512,1764},  //duty=5, dutyLt=20
   { 602, 512,1719},  //duty=6, dutyLt=20
   { 614, 512,1667},  //duty=7, dutyLt=20
   { 629, 512,1605},  //duty=8, dutyLt=20
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
   { 523, 512,2211},  //duty=-1, dutyLt=21
   { 537, 512,2100},  //duty=0, dutyLt=21
   { 549, 512,2011},  //duty=1, dutyLt=21
   { 561, 512,1934},  //duty=2, dutyLt=21
   { 571, 512,1871},  //duty=3, dutyLt=21
   { 582, 512,1816},  //duty=4, dutyLt=21
   { 591, 512,1769},  //duty=5, dutyLt=21
   { 599, 512,1728},  //duty=6, dutyLt=21
   { 611, 512,1673},  //duty=7, dutyLt=21
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
   { 524, 512,2209},  //duty=-1, dutyLt=22
   { 536, 512,2100},  //duty=0, dutyLt=22
   { 549, 512,2010},  //duty=1, dutyLt=22
   { 560, 512,1938},  //duty=2, dutyLt=22
   { 570, 512,1879},  //duty=3, dutyLt=22
   { 580, 512,1822},  //duty=4, dutyLt=22
   { 589, 512,1774},  //duty=5, dutyLt=22
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
   { 524, 512,2200},  //duty=-1, dutyLt=23
   { 536, 512,2101},  //duty=0, dutyLt=23
   { 548, 512,2011},  //duty=1, dutyLt=23
   { 559, 512,1940},  //duty=2, dutyLt=23
   { 569, 512,1880},  //duty=3, dutyLt=23
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
   { 524, 512,2194},  //duty=-1, dutyLt=24
   { 536, 512,2095},  //duty=0, dutyLt=24
   { 548, 512,2012},  //duty=1, dutyLt=24
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
   { 524, 512,2190},  //duty=-1, dutyLt=25
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
