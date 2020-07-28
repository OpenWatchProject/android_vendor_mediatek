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
    {1165, 512, 999},  //duty=0, dutyLt=-1
    {1157, 512, 992},  //duty=1, dutyLt=-1
    {1163, 512, 992},  //duty=2, dutyLt=-1
    {1164, 512, 983},  //duty=3, dutyLt=-1
    {1162, 512, 976},  //duty=4, dutyLt=-1
    {1158, 512, 970},  //duty=5, dutyLt=-1
    {1155, 512, 956},  //duty=6, dutyLt=-1
    {1158, 512, 952},  //duty=7, dutyLt=-1
    {1162, 512, 948},  //duty=8, dutyLt=-1
    {1164, 512, 945},  //duty=9, dutyLt=-1
    {1163, 512, 940},  //duty=10, dutyLt=-1
    {1164, 512, 933},  //duty=11, dutyLt=-1
    {1164, 512, 927},  //duty=12, dutyLt=-1
    {1164, 512, 923},  //duty=13, dutyLt=-1
    {1163, 512, 917},  //duty=14, dutyLt=-1
    {1162, 512, 914},  //duty=15, dutyLt=-1
    {1161, 512, 910},  //duty=16, dutyLt=-1
    {1160, 512, 907},  //duty=17, dutyLt=-1
    {1159, 512, 904},  //duty=18, dutyLt=-1
    {1158, 512, 901},  //duty=19, dutyLt=-1
    {1158, 512, 898},  //duty=20, dutyLt=-1
    {1158, 512, 895},  //duty=21, dutyLt=-1
    {1158, 512, 892},  //duty=22, dutyLt=-1
    {1157, 512, 889},  //duty=23, dutyLt=-1
    {1158, 512, 888},  //duty=24, dutyLt=-1
    {1159, 512, 884},  //duty=25, dutyLt=-1
    { 537, 512,2217},  //duty=-1, dutyLt=0
    { 772, 512,1294},  //duty=0, dutyLt=0
    { 889, 512,1172},  //duty=1, dutyLt=0
    { 946, 512,1115},  //duty=2, dutyLt=0
    { 981, 512,1078},  //duty=3, dutyLt=0
    {1004, 512,1053},  //duty=4, dutyLt=0
    {1022, 512,1031},  //duty=5, dutyLt=0
    {1035, 512,1018},  //duty=6, dutyLt=0
    {1047, 512,1008},  //duty=7, dutyLt=0
    {1067, 512, 998},  //duty=8, dutyLt=0
    {1080, 512, 987},  //duty=9, dutyLt=0
    {1089, 512, 977},  //duty=10, dutyLt=0
    {1095, 512, 967},  //duty=11, dutyLt=0
    {1101, 512, 958},  //duty=12, dutyLt=0
    {1107, 512, 952},  //duty=13, dutyLt=0
    {1109, 512, 944},  //duty=14, dutyLt=0
    {1110, 512, 938},  //duty=15, dutyLt=0
    {1111, 512, 934},  //duty=16, dutyLt=0
    {1114, 512, 930},  //duty=17, dutyLt=0
    {1116, 512, 926},  //duty=18, dutyLt=0
    {1116, 512, 921},  //duty=19, dutyLt=0
    {1117, 512, 918},  //duty=20, dutyLt=0
    {1119, 512, 915},  //duty=21, dutyLt=0
    {1120, 512, 910},  //duty=22, dutyLt=0
    {1121, 512, 907},  //duty=23, dutyLt=0
    {1122, 512, 904},  //duty=24, dutyLt=0
    { 512, 512, 512},  //duty=25, dutyLt=0
    { 536, 512,2216},  //duty=-1, dutyLt=1
    { 686, 512,1483},  //duty=0, dutyLt=1
    { 776, 512,1293},  //duty=1, dutyLt=1
    { 839, 512,1213},  //duty=2, dutyLt=1
    { 880, 512,1158},  //duty=3, dutyLt=1
    { 912, 512,1116},  //duty=4, dutyLt=1
    { 934, 512,1087},  //duty=5, dutyLt=1
    { 955, 512,1072},  //duty=6, dutyLt=1
    { 972, 512,1059},  //duty=7, dutyLt=1
    { 999, 512,1040},  //duty=8, dutyLt=1
    {1018, 512,1024},  //duty=9, dutyLt=1
    {1032, 512,1009},  //duty=10, dutyLt=1
    {1043, 512, 997},  //duty=11, dutyLt=1
    {1051, 512, 985},  //duty=12, dutyLt=1
    {1058, 512, 976},  //duty=13, dutyLt=1
    {1065, 512, 970},  //duty=14, dutyLt=1
    {1067, 512, 961},  //duty=15, dutyLt=1
    {1071, 512, 955},  //duty=16, dutyLt=1
    {1074, 512, 949},  //duty=17, dutyLt=1
    {1077, 512, 944},  //duty=18, dutyLt=1
    {1080, 512, 939},  //duty=19, dutyLt=1
    {1084, 512, 936},  //duty=20, dutyLt=1
    {1086, 512, 931},  //duty=21, dutyLt=1
    {1088, 512, 927},  //duty=22, dutyLt=1
    {1090, 512, 923},  //duty=23, dutyLt=1
    {1092, 512, 919},  //duty=24, dutyLt=1
    { 512, 512, 512},  //duty=25, dutyLt=1
    { 536, 512,2191},  //duty=-1, dutyLt=2
    { 644, 512,1599},  //duty=0, dutyLt=2
    { 720, 512,1390},  //duty=1, dutyLt=2
    { 774, 512,1282},  //duty=2, dutyLt=2
    { 820, 512,1222},  //duty=3, dutyLt=2
    { 849, 512,1168},  //duty=4, dutyLt=2
    { 877, 512,1141},  //duty=5, dutyLt=2
    { 900, 512,1121},  //duty=6, dutyLt=2
    { 920, 512,1104},  //duty=7, dutyLt=2
    { 949, 512,1078},  //duty=8, dutyLt=2
    { 971, 512,1057},  //duty=9, dutyLt=2
    { 987, 512,1038},  //duty=10, dutyLt=2
    {1000, 512,1023},  //duty=11, dutyLt=2
    {1010, 512,1010},  //duty=12, dutyLt=2
    {1020, 512, 999},  //duty=13, dutyLt=2
    {1026, 512, 989},  //duty=14, dutyLt=2
    {1032, 512, 981},  //duty=15, dutyLt=2
    {1038, 512, 974},  //duty=16, dutyLt=2
    {1042, 512, 968},  //duty=17, dutyLt=2
    {1046, 512, 962},  //duty=18, dutyLt=2
    {1050, 512, 957},  //duty=19, dutyLt=2
    {1054, 512, 951},  //duty=20, dutyLt=2
    {1059, 512, 948},  //duty=21, dutyLt=2
    {1061, 512, 942},  //duty=22, dutyLt=2
    {1063, 512, 938},  //duty=23, dutyLt=2
    { 512, 512, 512},  //duty=24, dutyLt=2
    { 512, 512, 512},  //duty=25, dutyLt=2
    { 535, 512,2200},  //duty=-1, dutyLt=3
    { 621, 512,1676},  //duty=0, dutyLt=3
    { 686, 512,1462},  //duty=1, dutyLt=3
    { 736, 512,1343},  //duty=2, dutyLt=3
    { 776, 512,1256},  //duty=3, dutyLt=3
    { 804, 512,1215},  //duty=4, dutyLt=3
    { 836, 512,1190},  //duty=5, dutyLt=3
    { 860, 512,1165},  //duty=6, dutyLt=3
    { 880, 512,1144},  //duty=7, dutyLt=3
    { 909, 512,1112},  //duty=8, dutyLt=3
    { 933, 512,1086},  //duty=9, dutyLt=3
    { 951, 512,1064},  //duty=10, dutyLt=3
    { 965, 512,1047},  //duty=11, dutyLt=3
    { 976, 512,1032},  //duty=12, dutyLt=3
    { 986, 512,1019},  //duty=13, dutyLt=3
    { 994, 512,1009},  //duty=14, dutyLt=3
    {1001, 512, 998},  //duty=15, dutyLt=3
    {1009, 512, 991},  //duty=16, dutyLt=3
    {1014, 512, 984},  //duty=17, dutyLt=3
    {1020, 512, 977},  //duty=18, dutyLt=3
    {1024, 512, 971},  //duty=19, dutyLt=3
    {1029, 512, 966},  //duty=20, dutyLt=3
    {1032, 512, 960},  //duty=21, dutyLt=3
    {1036, 512, 955},  //duty=22, dutyLt=3
    {1041, 512, 952},  //duty=23, dutyLt=3
    { 512, 512, 512},  //duty=24, dutyLt=3
    { 512, 512, 512},  //duty=25, dutyLt=3
    { 535, 512,2195},  //duty=-1, dutyLt=4
    { 607, 512,1732},  //duty=0, dutyLt=4
    { 664, 512,1519},  //duty=1, dutyLt=4
    { 710, 512,1388},  //duty=2, dutyLt=4
    { 743, 512,1306},  //duty=3, dutyLt=4
    { 774, 512,1259},  //duty=4, dutyLt=4
    { 801, 512,1224},  //duty=5, dutyLt=4
    { 828, 512,1203},  //duty=6, dutyLt=4
    { 848, 512,1179},  //duty=7, dutyLt=4
    { 878, 512,1141},  //duty=8, dutyLt=4
    { 900, 512,1112},  //duty=9, dutyLt=4
    { 920, 512,1089},  //duty=10, dutyLt=4
    { 935, 512,1069},  //duty=11, dutyLt=4
    { 947, 512,1052},  //duty=12, dutyLt=4
    { 958, 512,1038},  //duty=13, dutyLt=4
    { 968, 512,1026},  //duty=14, dutyLt=4
    { 976, 512,1016},  //duty=15, dutyLt=4
    { 983, 512,1007},  //duty=16, dutyLt=4
    { 989, 512, 999},  //duty=17, dutyLt=4
    { 996, 512, 993},  //duty=18, dutyLt=4
    {1001, 512, 985},  //duty=19, dutyLt=4
    {1006, 512, 979},  //duty=20, dutyLt=4
    {1011, 512, 974},  //duty=21, dutyLt=4
    {1016, 512, 969},  //duty=22, dutyLt=4
    { 512, 512, 512},  //duty=23, dutyLt=4
    { 512, 512, 512},  //duty=24, dutyLt=4
    { 512, 512, 512},  //duty=25, dutyLt=4
    { 535, 512,2187},  //duty=-1, dutyLt=5
    { 597, 512,1775},  //duty=0, dutyLt=5
    { 648, 512,1560},  //duty=1, dutyLt=5
    { 687, 512,1422},  //duty=2, dutyLt=5
    { 722, 512,1351},  //duty=3, dutyLt=5
    { 753, 512,1299},  //duty=4, dutyLt=5
    { 777, 512,1261},  //duty=5, dutyLt=5
    { 799, 512,1229},  //duty=6, dutyLt=5
    { 822, 512,1211},  //duty=7, dutyLt=5
    { 851, 512,1168},  //duty=8, dutyLt=5
    { 874, 512,1137},  //duty=9, dutyLt=5
    { 894, 512,1111},  //duty=10, dutyLt=5
    { 909, 512,1089},  //duty=11, dutyLt=5
    { 923, 512,1071},  //duty=12, dutyLt=5
    { 934, 512,1056},  //duty=13, dutyLt=5
    { 945, 512,1042},  //duty=14, dutyLt=5
    { 954, 512,1032},  //duty=15, dutyLt=5
    { 962, 512,1022},  //duty=16, dutyLt=5
    { 969, 512,1014},  //duty=17, dutyLt=5
    { 975, 512,1006},  //duty=18, dutyLt=5
    { 981, 512, 998},  //duty=19, dutyLt=5
    { 987, 512, 992},  //duty=20, dutyLt=5
    { 991, 512, 986},  //duty=21, dutyLt=5
    { 996, 512, 980},  //duty=22, dutyLt=5
    { 512, 512, 512},  //duty=23, dutyLt=5
    { 512, 512, 512},  //duty=24, dutyLt=5
    { 512, 512, 512},  //duty=25, dutyLt=5
    { 538, 512,2201},  //duty=-1, dutyLt=6
    { 590, 512,1805},  //duty=0, dutyLt=6
    { 638, 512,1584},  //duty=1, dutyLt=6
    { 673, 512,1461},  //duty=2, dutyLt=6
    { 706, 512,1389},  //duty=3, dutyLt=6
    { 735, 512,1334},  //duty=4, dutyLt=6
    { 758, 512,1292},  //duty=5, dutyLt=6
    { 778, 512,1257},  //duty=6, dutyLt=6
    { 799, 512,1238},  //duty=7, dutyLt=6
    { 828, 512,1194},  //duty=8, dutyLt=6
    { 852, 512,1159},  //duty=9, dutyLt=6
    { 871, 512,1131},  //duty=10, dutyLt=6
    { 888, 512,1108},  //duty=11, dutyLt=6
    { 902, 512,1088},  //duty=12, dutyLt=6
    { 914, 512,1072},  //duty=13, dutyLt=6
    { 925, 512,1058},  //duty=14, dutyLt=6
    { 934, 512,1046},  //duty=15, dutyLt=6
    { 943, 512,1036},  //duty=16, dutyLt=6
    { 950, 512,1027},  //duty=17, dutyLt=6
    { 958, 512,1018},  //duty=18, dutyLt=6
    { 963, 512,1011},  //duty=19, dutyLt=6
    { 969, 512,1004},  //duty=20, dutyLt=6
    { 974, 512, 997},  //duty=21, dutyLt=6
    { 512, 512, 512},  //duty=22, dutyLt=6
    { 512, 512, 512},  //duty=23, dutyLt=6
    { 512, 512, 512},  //duty=24, dutyLt=6
    { 512, 512, 512},  //duty=25, dutyLt=6
    { 535, 512,2173},  //duty=-1, dutyLt=7
    { 585, 512,1824},  //duty=0, dutyLt=7
    { 626, 512,1604},  //duty=1, dutyLt=7
    { 662, 512,1496},  //duty=2, dutyLt=7
    { 692, 512,1422},  //duty=3, dutyLt=7
    { 720, 512,1364},  //duty=4, dutyLt=7
    { 745, 512,1333},  //duty=5, dutyLt=7
    { 761, 512,1283},  //duty=6, dutyLt=7
    { 778, 512,1253},  //duty=7, dutyLt=7
    { 809, 512,1216},  //duty=8, dutyLt=7
    { 833, 512,1179},  //duty=9, dutyLt=7
    { 853, 512,1150},  //duty=10, dutyLt=7
    { 869, 512,1126},  //duty=11, dutyLt=7
    { 883, 512,1105},  //duty=12, dutyLt=7
    { 896, 512,1087},  //duty=13, dutyLt=7
    { 907, 512,1073},  //duty=14, dutyLt=7
    { 917, 512,1060},  //duty=15, dutyLt=7
    { 926, 512,1049},  //duty=16, dutyLt=7
    { 934, 512,1039},  //duty=17, dutyLt=7
    { 941, 512,1031},  //duty=18, dutyLt=7
    { 948, 512,1023},  //duty=19, dutyLt=7
    { 954, 512,1015},  //duty=20, dutyLt=7
    { 959, 512,1008},  //duty=21, dutyLt=7
    { 512, 512, 512},  //duty=22, dutyLt=7
    { 512, 512, 512},  //duty=23, dutyLt=7
    { 512, 512, 512},  //duty=24, dutyLt=7
    { 512, 512, 512},  //duty=25, dutyLt=7
    { 538, 512,2175},  //duty=-1, dutyLt=8
    { 579, 512,1840},  //duty=0, dutyLt=8
    { 616, 512,1677},  //duty=1, dutyLt=8
    { 647, 512,1571},  //duty=2, dutyLt=8
    { 672, 512,1476},  //duty=3, dutyLt=8
    { 696, 512,1415},  //duty=4, dutyLt=8
    { 715, 512,1367},  //duty=5, dutyLt=8
    { 735, 512,1343},  //duty=6, dutyLt=8
    { 751, 512,1308},  //duty=7, dutyLt=8
    { 778, 512,1257},  //duty=8, dutyLt=8
    { 802, 512,1217},  //duty=9, dutyLt=8
    { 822, 512,1183},  //duty=10, dutyLt=8
    { 838, 512,1157},  //duty=11, dutyLt=8
    { 853, 512,1134},  //duty=12, dutyLt=8
    { 866, 512,1115},  //duty=13, dutyLt=8
    { 878, 512,1099},  //duty=14, dutyLt=8
    { 888, 512,1085},  //duty=15, dutyLt=8
    { 897, 512,1073},  //duty=16, dutyLt=8
    { 906, 512,1063},  //duty=17, dutyLt=8
    { 912, 512,1052},  //duty=18, dutyLt=8
    { 919, 512,1044},  //duty=19, dutyLt=8
    { 926, 512,1036},  //duty=20, dutyLt=8
    { 512, 512, 512},  //duty=21, dutyLt=8
    { 512, 512, 512},  //duty=22, dutyLt=8
    { 512, 512, 512},  //duty=23, dutyLt=8
    { 512, 512, 512},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 539, 512,2104},  //duty=-1, dutyLt=9
    { 575, 512,1875},  //duty=0, dutyLt=9
    { 607, 512,1722},  //duty=1, dutyLt=9
    { 634, 512,1615},  //duty=2, dutyLt=9
    { 658, 512,1535},  //duty=3, dutyLt=9
    { 679, 512,1469},  //duty=4, dutyLt=9
    { 697, 512,1419},  //duty=5, dutyLt=9
    { 714, 512,1378},  //duty=6, dutyLt=9
    { 729, 512,1344},  //duty=7, dutyLt=9
    { 755, 512,1290},  //duty=8, dutyLt=9
    { 778, 512,1248},  //duty=9, dutyLt=9
    { 798, 512,1211},  //duty=10, dutyLt=9
    { 815, 512,1183},  //duty=11, dutyLt=9
    { 829, 512,1160},  //duty=12, dutyLt=9
    { 842, 512,1139},  //duty=13, dutyLt=9
    { 854, 512,1121},  //duty=14, dutyLt=9
    { 864, 512,1106},  //duty=15, dutyLt=9
    { 874, 512,1094},  //duty=16, dutyLt=9
    { 882, 512,1082},  //duty=17, dutyLt=9
    { 889, 512,1072},  //duty=18, dutyLt=9
    { 897, 512,1063},  //duty=19, dutyLt=9
    { 512, 512, 512},  //duty=20, dutyLt=9
    { 512, 512, 512},  //duty=21, dutyLt=9
    { 512, 512, 512},  //duty=22, dutyLt=9
    { 512, 512, 512},  //duty=23, dutyLt=9
    { 512, 512, 512},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 540, 512,2111},  //duty=-1, dutyLt=10
    { 571, 512,1904},  //duty=0, dutyLt=10
    { 600, 512,1752},  //duty=1, dutyLt=10
    { 624, 512,1649},  //duty=2, dutyLt=10
    { 643, 512,1577},  //duty=3, dutyLt=10
    { 665, 512,1503},  //duty=4, dutyLt=10
    { 682, 512,1452},  //duty=5, dutyLt=10
    { 698, 512,1411},  //duty=6, dutyLt=10
    { 712, 512,1377},  //duty=7, dutyLt=10
    { 737, 512,1320},  //duty=8, dutyLt=10
    { 759, 512,1274},  //duty=9, dutyLt=10
    { 779, 512,1237},  //duty=10, dutyLt=10
    { 795, 512,1207},  //duty=11, dutyLt=10
    { 810, 512,1182},  //duty=12, dutyLt=10
    { 823, 512,1161},  //duty=13, dutyLt=10
    { 834, 512,1143},  //duty=14, dutyLt=10
    { 844, 512,1127},  //duty=15, dutyLt=10
    { 853, 512,1113},  //duty=16, dutyLt=10
    { 863, 512,1106},  //duty=17, dutyLt=10
    { 871, 512,1092},  //duty=18, dutyLt=10
    { 512, 512, 512},  //duty=19, dutyLt=10
    { 512, 512, 512},  //duty=20, dutyLt=10
    { 512, 512, 512},  //duty=21, dutyLt=10
    { 512, 512, 512},  //duty=22, dutyLt=10
    { 512, 512, 512},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 540, 512,2113},  //duty=-1, dutyLt=11
    { 568, 512,1917},  //duty=0, dutyLt=11
    { 593, 512,1779},  //duty=1, dutyLt=11
    { 616, 512,1675},  //duty=2, dutyLt=11
    { 636, 512,1595},  //duty=3, dutyLt=11
    { 654, 512,1532},  //duty=4, dutyLt=11
    { 670, 512,1480},  //duty=5, dutyLt=11
    { 685, 512,1440},  //duty=6, dutyLt=11
    { 699, 512,1404},  //duty=7, dutyLt=11
    { 724, 512,1345},  //duty=8, dutyLt=11
    { 746, 512,1294},  //duty=9, dutyLt=11
    { 763, 512,1259},  //duty=10, dutyLt=11
    { 779, 512,1229},  //duty=11, dutyLt=11
    { 793, 512,1203},  //duty=12, dutyLt=11
    { 806, 512,1181},  //duty=13, dutyLt=11
    { 818, 512,1161},  //duty=14, dutyLt=11
    { 828, 512,1145},  //duty=15, dutyLt=11
    { 837, 512,1131},  //duty=16, dutyLt=11
    { 846, 512,1118},  //duty=17, dutyLt=11
    { 512, 512, 512},  //duty=18, dutyLt=11
    { 512, 512, 512},  //duty=19, dutyLt=11
    { 512, 512, 512},  //duty=20, dutyLt=11
    { 512, 512, 512},  //duty=21, dutyLt=11
    { 512, 512, 512},  //duty=22, dutyLt=11
    { 512, 512, 512},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 541, 512,2106},  //duty=-1, dutyLt=12
    { 566, 512,1930},  //duty=0, dutyLt=12
    { 589, 512,1796},  //duty=1, dutyLt=12
    { 610, 512,1696},  //duty=2, dutyLt=12
    { 628, 512,1620},  //duty=3, dutyLt=12
    { 646, 512,1557},  //duty=4, dutyLt=12
    { 661, 512,1506},  //duty=5, dutyLt=12
    { 676, 512,1464},  //duty=6, dutyLt=12
    { 689, 512,1426},  //duty=7, dutyLt=12
    { 712, 512,1366},  //duty=8, dutyLt=12
    { 732, 512,1318},  //duty=9, dutyLt=12
    { 750, 512,1280},  //duty=10, dutyLt=12
    { 766, 512,1248},  //duty=11, dutyLt=12
    { 780, 512,1220},  //duty=12, dutyLt=12
    { 792, 512,1198},  //duty=13, dutyLt=12
    { 803, 512,1178},  //duty=14, dutyLt=12
    { 814, 512,1161},  //duty=15, dutyLt=12
    { 823, 512,1146},  //duty=16, dutyLt=12
    { 512, 512, 512},  //duty=17, dutyLt=12
    { 512, 512, 512},  //duty=18, dutyLt=12
    { 512, 512, 512},  //duty=19, dutyLt=12
    { 512, 512, 512},  //duty=20, dutyLt=12
    { 512, 512, 512},  //duty=21, dutyLt=12
    { 512, 512, 512},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 541, 512,2109},  //duty=-1, dutyLt=13
    { 564, 512,1941},  //duty=0, dutyLt=13
    { 585, 512,1813},  //duty=1, dutyLt=13
    { 604, 512,1715},  //duty=2, dutyLt=13
    { 622, 512,1639},  //duty=3, dutyLt=13
    { 639, 512,1577},  //duty=4, dutyLt=13
    { 654, 512,1524},  //duty=5, dutyLt=13
    { 667, 512,1484},  //duty=6, dutyLt=13
    { 680, 512,1447},  //duty=7, dutyLt=13
    { 701, 512,1388},  //duty=8, dutyLt=13
    { 722, 512,1338},  //duty=9, dutyLt=13
    { 739, 512,1298},  //duty=10, dutyLt=13
    { 755, 512,1265},  //duty=11, dutyLt=13
    { 766, 512,1244},  //duty=12, dutyLt=13
    { 780, 512,1213},  //duty=13, dutyLt=13
    { 792, 512,1193},  //duty=14, dutyLt=13
    { 802, 512,1178},  //duty=15, dutyLt=13
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
    { 541, 512,2099},  //duty=-1, dutyLt=14
    { 562, 512,1947},  //duty=0, dutyLt=14
    { 583, 512,1823},  //duty=1, dutyLt=14
    { 601, 512,1728},  //duty=2, dutyLt=14
    { 617, 512,1657},  //duty=3, dutyLt=14
    { 634, 512,1595},  //duty=4, dutyLt=14
    { 648, 512,1544},  //duty=5, dutyLt=14
    { 659, 512,1505},  //duty=6, dutyLt=14
    { 672, 512,1466},  //duty=7, dutyLt=14
    { 694, 512,1403},  //duty=8, dutyLt=14
    { 713, 512,1353},  //duty=9, dutyLt=14
    { 730, 512,1312},  //duty=10, dutyLt=14
    { 745, 512,1281},  //duty=11, dutyLt=14
    { 757, 512,1254},  //duty=12, dutyLt=14
    { 770, 512,1228},  //duty=13, dutyLt=14
    { 781, 512,1208},  //duty=14, dutyLt=14
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
    { 541, 512,2100},  //duty=-1, dutyLt=15
    { 561, 512,1948},  //duty=0, dutyLt=15
    { 581, 512,1832},  //duty=1, dutyLt=15
    { 598, 512,1741},  //duty=2, dutyLt=15
    { 615, 512,1667},  //duty=3, dutyLt=15
    { 630, 512,1607},  //duty=4, dutyLt=15
    { 643, 512,1560},  //duty=5, dutyLt=15
    { 654, 512,1519},  //duty=6, dutyLt=15
    { 667, 512,1479},  //duty=7, dutyLt=15
    { 688, 512,1417},  //duty=8, dutyLt=15
    { 705, 512,1369},  //duty=9, dutyLt=15
    { 722, 512,1326},  //duty=10, dutyLt=15
    { 736, 512,1294},  //duty=11, dutyLt=15
    { 750, 512,1265},  //duty=12, dutyLt=15
    { 762, 512,1241},  //duty=13, dutyLt=15
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
    { 542, 512,2092},  //duty=-1, dutyLt=16
    { 561, 512,1950},  //duty=0, dutyLt=16
    { 579, 512,1838},  //duty=1, dutyLt=16
    { 596, 512,1751},  //duty=2, dutyLt=16
    { 612, 512,1678},  //duty=3, dutyLt=16
    { 626, 512,1620},  //duty=4, dutyLt=16
    { 639, 512,1570},  //duty=5, dutyLt=16
    { 651, 512,1530},  //duty=6, dutyLt=16
    { 661, 512,1493},  //duty=7, dutyLt=16
    { 681, 512,1432},  //duty=8, dutyLt=16
    { 700, 512,1379},  //duty=9, dutyLt=16
    { 715, 512,1339},  //duty=10, dutyLt=16
    { 727, 512,1309},  //duty=11, dutyLt=16
    { 742, 512,1277},  //duty=12, dutyLt=16
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
    { 542, 512,2087},  //duty=-1, dutyLt=17
    { 560, 512,1954},  //duty=0, dutyLt=17
    { 577, 512,1847},  //duty=1, dutyLt=17
    { 594, 512,1757},  //duty=2, dutyLt=17
    { 608, 512,1692},  //duty=3, dutyLt=17
    { 622, 512,1635},  //duty=4, dutyLt=17
    { 635, 512,1583},  //duty=5, dutyLt=17
    { 646, 512,1543},  //duty=6, dutyLt=17
    { 657, 512,1503},  //duty=7, dutyLt=17
    { 677, 512,1440},  //duty=8, dutyLt=17
    { 695, 512,1391},  //duty=9, dutyLt=17
    { 710, 512,1349},  //duty=10, dutyLt=17
    { 723, 512,1315},  //duty=11, dutyLt=17
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
    { 543, 512,2079},  //duty=-1, dutyLt=18
    { 560, 512,1948},  //duty=0, dutyLt=18
    { 572, 512,1871},  //duty=1, dutyLt=18
    { 593, 512,1764},  //duty=2, dutyLt=18
    { 607, 512,1698},  //duty=3, dutyLt=18
    { 621, 512,1639},  //duty=4, dutyLt=18
    { 631, 512,1594},  //duty=5, dutyLt=18
    { 643, 512,1551},  //duty=6, dutyLt=18
    { 653, 512,1514},  //duty=7, dutyLt=18
    { 672, 512,1452},  //duty=8, dutyLt=18
    { 690, 512,1399},  //duty=9, dutyLt=18
    { 704, 512,1360},  //duty=10, dutyLt=18
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
    { 543, 512,2073},  //duty=-1, dutyLt=19
    { 559, 512,1956},  //duty=0, dutyLt=19
    { 576, 512,1853},  //duty=1, dutyLt=19
    { 592, 512,1770},  //duty=2, dutyLt=19
    { 605, 512,1703},  //duty=3, dutyLt=19
    { 618, 512,1648},  //duty=4, dutyLt=19
    { 630, 512,1597},  //duty=5, dutyLt=19
    { 641, 512,1556},  //duty=6, dutyLt=19
    { 649, 512,1525},  //duty=7, dutyLt=19
    { 669, 512,1460},  //duty=8, dutyLt=19
    { 685, 512,1411},  //duty=9, dutyLt=19
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
    { 542, 512,2083},  //duty=-1, dutyLt=20
    { 561, 512,1949},  //duty=0, dutyLt=20
    { 577, 512,1850},  //duty=1, dutyLt=20
    { 590, 512,1776},  //duty=2, dutyLt=20
    { 604, 512,1709},  //duty=3, dutyLt=20
    { 616, 512,1654},  //duty=4, dutyLt=20
    { 627, 512,1608},  //duty=5, dutyLt=20
    { 637, 512,1568},  //duty=6, dutyLt=20
    { 649, 512,1526},  //duty=7, dutyLt=20
    { 666, 512,1467},  //duty=8, dutyLt=20
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
    { 544, 512,2071},  //duty=-1, dutyLt=21
    { 561, 512,1948},  //duty=0, dutyLt=21
    { 575, 512,1857},  //duty=1, dutyLt=21
    { 588, 512,1785},  //duty=2, dutyLt=21
    { 602, 512,1714},  //duty=3, dutyLt=21
    { 614, 512,1662},  //duty=4, dutyLt=21
    { 626, 512,1610},  //duty=5, dutyLt=21
    { 636, 512,1571},  //duty=6, dutyLt=21
    { 646, 512,1534},  //duty=7, dutyLt=21
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
    { 546, 512,2053},  //duty=-1, dutyLt=22
    { 561, 512,1944},  //duty=0, dutyLt=22
    { 577, 512,1849},  //duty=1, dutyLt=22
    { 589, 512,1778},  //duty=2, dutyLt=22
    { 602, 512,1716},  //duty=3, dutyLt=22
    { 613, 512,1666},  //duty=4, dutyLt=22
    { 626, 512,1613},  //duty=5, dutyLt=22
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
    { 547, 512,2044},  //duty=-1, dutyLt=23
    { 561, 512,1942},  //duty=0, dutyLt=23
    { 577, 512,1848},  //duty=1, dutyLt=23
    { 588, 512,1783},  //duty=2, dutyLt=23
    { 601, 512,1719},  //duty=3, dutyLt=23
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
    { 545, 512,2055},  //duty=-1, dutyLt=24
    { 561, 512,1940},  //duty=0, dutyLt=24
    { 575, 512,1856},  //duty=1, dutyLt=24
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
    { 541, 512,2074},  //duty=-1, dutyLt=25
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

