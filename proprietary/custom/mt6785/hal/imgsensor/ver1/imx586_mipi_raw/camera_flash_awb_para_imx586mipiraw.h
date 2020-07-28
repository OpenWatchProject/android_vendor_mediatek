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
			{1245, 512,1037},  //duty=0, dutyLt=-1	
			{1248, 512,1030},  //duty=1, dutyLt=-1	
			{1244, 512,1021},  //duty=2, dutyLt=-1	
			{1238, 512,1011},  //duty=3, dutyLt=-1	
			{1232, 512,1004},  //duty=4, dutyLt=-1	
			{1228, 512, 997},  //duty=5, dutyLt=-1	
			{1226, 512, 992},  //duty=6, dutyLt=-1	
			{1222, 512, 980},  //duty=7, dutyLt=-1	
			{1225, 512, 971},  //duty=8, dutyLt=-1	
			{1224, 512, 967},  //duty=9, dutyLt=-1	
			{1224, 512, 962},  //duty=10, dutyLt=-1  
			{1222, 512, 957},  //duty=11, dutyLt=-1  
			{1223, 512, 953},  //duty=12, dutyLt=-1  
			{1222, 512, 948},  //duty=13, dutyLt=-1  
			{1222, 512, 944},  //duty=14, dutyLt=-1  
			{1221, 512, 940},  //duty=15, dutyLt=-1  
			{1221, 512, 935},  //duty=16, dutyLt=-1  
			{1220, 512, 927},  //duty=17, dutyLt=-1  
			{1220, 512, 923},  //duty=18, dutyLt=-1  
			{1220, 512, 926},  //duty=19, dutyLt=-1  
			{1220, 512, 915},  //duty=20, dutyLt=-1  
			{1219, 512, 918},  //duty=21, dutyLt=-1  
			{1220, 512, 907},  //duty=22, dutyLt=-1  
			{1219, 512, 912},  //duty=23, dutyLt=-1  
			{1219, 512, 907},  //duty=24, dutyLt=-1  
			{1222, 512, 893},  //duty=25, dutyLt=-1  
			{ 560, 512,2552},  //duty=-1, dutyLt=0	
			{ 765, 512,1475},  //duty=0, dutyLt=0  
			{ 885, 512,1279},  //duty=1, dutyLt=0  
			{ 950, 512,1195},  //duty=2, dutyLt=0  
			{ 993, 512,1144},  //duty=3, dutyLt=0  
			{1021, 512,1114},  //duty=4, dutyLt=0  
			{1043, 512,1090},  //duty=5, dutyLt=0  
			{1061, 512,1068},  //duty=6, dutyLt=0  
			{1077, 512,1053},  //duty=7, dutyLt=0  
			{1098, 512,1035},  //duty=8, dutyLt=0  
			{1116, 512,1019},  //duty=9, dutyLt=0  
			{1127, 512,1008},  //duty=10, dutyLt=0	
			{1136, 512, 999},  //duty=11, dutyLt=0	
			{1143, 512, 992},  //duty=12, dutyLt=0	
			{1149, 512, 983},  //duty=13, dutyLt=0	
			{1152, 512, 975},  //duty=14, dutyLt=0	
			{1157, 512, 971},  //duty=15, dutyLt=0	
			{1161, 512, 964},  //duty=16, dutyLt=0	
			{1163, 512, 956},  //duty=17, dutyLt=0	
			{1165, 512, 954},  //duty=18, dutyLt=0	
			{1167, 512, 950},  //duty=19, dutyLt=0	
			{1170, 512, 945},  //duty=20, dutyLt=0	
			{1171, 512, 941},  //duty=21, dutyLt=0	
			{1172, 512, 937},  //duty=22, dutyLt=0	
			{1174, 512, 931},  //duty=23, dutyLt=0	
			{1176, 512, 928},  //duty=24, dutyLt=0	
			{ 512, 512, 512},  //duty=25, dutyLt=0	
			{ 561, 512,2584},  //duty=-1, dutyLt=1	
			{ 680, 512,1729},  //duty=0, dutyLt=1  
			{ 768, 512,1459},  //duty=1, dutyLt=1  
			{ 829, 512,1330},  //duty=2, dutyLt=1  
			{ 872, 512,1258},  //duty=3, dutyLt=1  
			{ 907, 512,1213},  //duty=4, dutyLt=1  
			{ 935, 512,1170},  //duty=5, dutyLt=1  
			{ 958, 512,1146},  //duty=6, dutyLt=1  
			{ 985, 512,1119},  //duty=7, dutyLt=1  
			{1013, 512,1091},  //duty=8, dutyLt=1  
			{1040, 512,1066},  //duty=9, dutyLt=1  
			{1055, 512,1051},  //duty=10, dutyLt=1	
			{1067, 512,1036},  //duty=11, dutyLt=1	
			{1079, 512,1027},  //duty=12, dutyLt=1	
			{1090, 512,1015},  //duty=13, dutyLt=1	
			{1096, 512,1004},  //duty=14, dutyLt=1	
			{1104, 512, 999},  //duty=15, dutyLt=1	
			{1110, 512, 990},  //duty=16, dutyLt=1	
			{1115, 512, 984},  //duty=17, dutyLt=1	
			{1119, 512, 978},  //duty=18, dutyLt=1	
			{1122, 512, 974},  //duty=19, dutyLt=1	
			{1125, 512, 963},  //duty=20, dutyLt=1	
			{1130, 512, 962},  //duty=21, dutyLt=1	
			{1132, 512, 957},  //duty=22, dutyLt=1	
			{1135, 512, 951},  //duty=23, dutyLt=1	
			{1137, 512, 946},  //duty=24, dutyLt=1	
			{ 512, 512, 512},  //duty=25, dutyLt=1	
			{ 562, 512,2512},  //duty=-1, dutyLt=2	
			{ 644, 512,1858},  //duty=0, dutyLt=2  
			{ 714, 512,1583},  //duty=1, dutyLt=2  
			{ 767, 512,1435},  //duty=2, dutyLt=2  
			{ 809, 512,1345},  //duty=3, dutyLt=2  
			{ 840, 512,1285},  //duty=4, dutyLt=2  
			{ 868, 512,1245},  //duty=5, dutyLt=2  
			{ 892, 512,1211},  //duty=6, dutyLt=2  
			{ 922, 512,1178},  //duty=7, dutyLt=2  
			{ 953, 512,1142},  //duty=8, dutyLt=2  
			{ 983, 512,1109},  //duty=9, dutyLt=2  
			{1001, 512,1090},  //duty=10, dutyLt=2	
			{1016, 512,1074},  //duty=11, dutyLt=2	
			{1029, 512,1059},  //duty=12, dutyLt=2	
			{1042, 512,1042},  //duty=13, dutyLt=2	
			{1052, 512,1034},  //duty=14, dutyLt=2	
			{1060, 512,1025},  //duty=15, dutyLt=2	
			{1069, 512,1015},  //duty=16, dutyLt=2	
			{1075, 512,1007},  //duty=17, dutyLt=2	
			{1080, 512,1000},  //duty=18, dutyLt=2	
			{1084, 512, 995},  //duty=19, dutyLt=2	
			{1090, 512, 987},  //duty=20, dutyLt=2	
			{1094, 512, 981},  //duty=21, dutyLt=2	
			{1095, 512, 968},  //duty=22, dutyLt=2	
			{1099, 512, 962},  //duty=23, dutyLt=2	
			{ 512, 512, 512},  //duty=24, dutyLt=2	
			{ 512, 512, 512},  //duty=25, dutyLt=2	
			{ 561, 512,2473},  //duty=-1, dutyLt=3	
			{ 626, 512,1941},  //duty=0, dutyLt=3  
			{ 683, 512,1671},  //duty=1, dutyLt=3  
			{ 729, 512,1517},  //duty=2, dutyLt=3  
			{ 767, 512,1414},  //duty=3, dutyLt=3  
			{ 797, 512,1350},  //duty=4, dutyLt=3  
			{ 824, 512,1302},  //duty=5, dutyLt=3  
			{ 846, 512,1272},  //duty=6, dutyLt=3  
			{ 876, 512,1225},  //duty=7, dutyLt=3  
			{ 908, 512,1186},  //duty=8, dutyLt=3  
			{ 938, 512,1146},  //duty=9, dutyLt=3  
			{ 958, 512,1123},  //duty=10, dutyLt=3	
			{ 974, 512,1103},  //duty=11, dutyLt=3	
			{ 990, 512,1088},  //duty=12, dutyLt=3	
			{1005, 512,1071},  //duty=13, dutyLt=3	
			{1016, 512,1059},  //duty=14, dutyLt=3	
			{1024, 512,1049},  //duty=15, dutyLt=3	
			{1035, 512,1037},  //duty=16, dutyLt=3	
			{1042, 512,1028},  //duty=17, dutyLt=3	
			{1048, 512,1020},  //duty=18, dutyLt=3	
			{1053, 512,1015},  //duty=19, dutyLt=3	
			{1057, 512,1002},  //duty=20, dutyLt=3	
			{1064, 512, 998},  //duty=21, dutyLt=3	
			{1068, 512, 993},  //duty=22, dutyLt=3	
			{1073, 512, 988},  //duty=23, dutyLt=3	
			{ 512, 512, 512},  //duty=24, dutyLt=3	
			{ 512, 512, 512},  //duty=25, dutyLt=3	
			{ 561, 512,2449},  //duty=-1, dutyLt=4	
			{ 615, 512,1998},  //duty=0, dutyLt=4  
			{ 665, 512,1733},  //duty=1, dutyLt=4  
			{ 706, 512,1574},  //duty=2, dutyLt=4  
			{ 740, 512,1470},  //duty=3, dutyLt=4  
			{ 768, 512,1406},  //duty=4, dutyLt=4  
			{ 794, 512,1355},  //duty=5, dutyLt=4  
			{ 816, 512,1313},  //duty=6, dutyLt=4  
			{ 842, 512,1272},  //duty=7, dutyLt=4  
			{ 873, 512,1226},  //duty=8, dutyLt=4  
			{ 905, 512,1184},  //duty=9, dutyLt=4  
			{ 925, 512,1155},  //duty=10, dutyLt=4	
			{ 942, 512,1135},  //duty=11, dutyLt=4	
			{ 957, 512,1116},  //duty=12, dutyLt=4	
			{ 973, 512,1097},  //duty=13, dutyLt=4	
			{ 985, 512,1084},  //duty=14, dutyLt=4	
			{ 994, 512,1071},  //duty=15, dutyLt=4	
			{1002, 512,1054},  //duty=16, dutyLt=4	
			{1013, 512,1048},  //duty=17, dutyLt=4	
			{1020, 512,1040},  //duty=18, dutyLt=4	
			{1024, 512,1033},  //duty=19, dutyLt=4	
			{1032, 512,1023},  //duty=20, dutyLt=4	
			{1038, 512,1016},  //duty=21, dutyLt=4	
			{1042, 512,1010},  //duty=22, dutyLt=4	
			{ 512, 512, 512},  //duty=23, dutyLt=4	
			{ 512, 512, 512},  //duty=24, dutyLt=4	
			{ 512, 512, 512},  //duty=25, dutyLt=4	
			{ 561, 512,2430},  //duty=-1, dutyLt=5	
			{ 607, 512,2038},  //duty=0, dutyLt=5  
			{ 651, 512,1775},  //duty=1, dutyLt=5  
			{ 688, 512,1620},  //duty=2, dutyLt=5  
			{ 719, 512,1529},  //duty=3, dutyLt=5  
			{ 745, 512,1457},  //duty=4, dutyLt=5  
			{ 767, 512,1408},  //duty=5, dutyLt=5  
			{ 788, 512,1362},  //duty=6, dutyLt=5  
			{ 815, 512,1314},  //duty=7, dutyLt=5  
			{ 845, 512,1263},  //duty=8, dutyLt=5  
			{ 876, 512,1215},  //duty=9, dutyLt=5  
			{ 897, 512,1185},  //duty=10, dutyLt=5	
			{ 914, 512,1161},  //duty=11, dutyLt=5	
			{ 930, 512,1141},  //duty=12, dutyLt=5	
			{ 947, 512,1121},  //duty=13, dutyLt=5	
			{ 959, 512,1105},  //duty=14, dutyLt=5	
			{ 966, 512,1089},  //duty=15, dutyLt=5	
			{ 980, 512,1077},  //duty=16, dutyLt=5	
			{ 986, 512,1065},  //duty=17, dutyLt=5	
			{ 992, 512,1053},  //duty=18, dutyLt=5	
			{1001, 512,1050},  //duty=19, dutyLt=5	
			{1009, 512,1040},  //duty=20, dutyLt=5	
			{1011, 512,1026},  //duty=21, dutyLt=5	
			{1020, 512,1025},  //duty=22, dutyLt=5	
			{ 512, 512, 512},  //duty=23, dutyLt=5	
			{ 512, 512, 512},  //duty=24, dutyLt=5	
			{ 512, 512, 512},  //duty=25, dutyLt=5	
			{ 562, 512,2422},  //duty=-1, dutyLt=6	
			{ 600, 512,2073},  //duty=0, dutyLt=6  
			{ 641, 512,1813},  //duty=1, dutyLt=6  
			{ 672, 512,1679},  //duty=2, dutyLt=6  
			{ 701, 512,1579},  //duty=3, dutyLt=6  
			{ 725, 512,1508},  //duty=4, dutyLt=6  
			{ 749, 512,1444},  //duty=5, dutyLt=6  
			{ 767, 512,1403},  //duty=6, dutyLt=6  
			{ 793, 512,1350},  //duty=7, dutyLt=6  
			{ 823, 512,1293},  //duty=8, dutyLt=6  
			{ 853, 512,1245},  //duty=9, dutyLt=6  
			{ 874, 512,1214},  //duty=10, dutyLt=6	
			{ 890, 512,1185},  //duty=11, dutyLt=6	
			{ 907, 512,1165},  //duty=12, dutyLt=6	
			{ 923, 512,1142},  //duty=13, dutyLt=6	
			{ 935, 512,1126},  //duty=14, dutyLt=6	
			{ 946, 512,1112},  //duty=15, dutyLt=6	
			{ 958, 512,1096},  //duty=16, dutyLt=6	
			{ 964, 512,1081},  //duty=17, dutyLt=6	
			{ 971, 512,1070},  //duty=18, dutyLt=6	
			{ 976, 512,1062},  //duty=19, dutyLt=6	
			{ 988, 512,1055},  //duty=20, dutyLt=6	
			{ 994, 512,1046},  //duty=21, dutyLt=6	
			{ 512, 512, 512},  //duty=22, dutyLt=6	
			{ 512, 512, 512},  //duty=23, dutyLt=6	
			{ 512, 512, 512},  //duty=24, dutyLt=6	
			{ 512, 512, 512},  //duty=25, dutyLt=6	
			{ 561, 512,2415},  //duty=-1, dutyLt=7	
			{ 597, 512,2078},  //duty=0, dutyLt=7  
			{ 629, 512,1872},  //duty=1, dutyLt=7  
			{ 659, 512,1730},  //duty=2, dutyLt=7  
			{ 684, 512,1631},  //duty=3, dutyLt=7  
			{ 706, 512,1556},  //duty=4, dutyLt=7  
			{ 724, 512,1505},  //duty=5, dutyLt=7  
			{ 743, 512,1457},  //duty=6, dutyLt=7  
			{ 767, 512,1399},  //duty=7, dutyLt=7  
			{ 795, 512,1340},  //duty=8, dutyLt=7  
			{ 825, 512,1285},  //duty=9, dutyLt=7  
			{ 845, 512,1250},  //duty=10, dutyLt=7	
			{ 862, 512,1222},  //duty=11, dutyLt=7	
			{ 877, 512,1197},  //duty=12, dutyLt=7	
			{ 895, 512,1172},  //duty=13, dutyLt=7	
			{ 907, 512,1155},  //duty=14, dutyLt=7	
			{ 918, 512,1139},  //duty=15, dutyLt=7	
			{ 929, 512,1122},  //duty=16, dutyLt=7	
			{ 936, 512,1106},  //duty=17, dutyLt=7	
			{ 945, 512,1096},  //duty=18, dutyLt=7	
			{ 952, 512,1089},  //duty=19, dutyLt=7	
			{ 957, 512,1072},  //duty=20, dutyLt=7	
			{ 967, 512,1067},  //duty=21, dutyLt=7	
			{ 512, 512, 512},  //duty=22, dutyLt=7	
			{ 512, 512, 512},  //duty=23, dutyLt=7	
			{ 512, 512, 512},  //duty=24, dutyLt=7	
			{ 512, 512, 512},  //duty=25, dutyLt=7	
			{ 563, 512,2354},  //duty=-1, dutyLt=8	
			{ 591, 512,2100},  //duty=0, dutyLt=8  
			{ 618, 512,1932},  //duty=1, dutyLt=8  
			{ 643, 512,1796},  //duty=2, dutyLt=8  
			{ 664, 512,1700},  //duty=3, dutyLt=8  
			{ 684, 512,1628},  //duty=4, dutyLt=8  
			{ 702, 512,1567},  //duty=5, dutyLt=8  
			{ 719, 512,1516},  //duty=6, dutyLt=8  
			{ 742, 512,1454},  //duty=7, dutyLt=8  
			{ 768, 512,1391},  //duty=8, dutyLt=8  
			{ 796, 512,1330},  //duty=9, dutyLt=8  
			{ 814, 512,1294},  //duty=10, dutyLt=8	
			{ 831, 512,1263},  //duty=11, dutyLt=8	
			{ 847, 512,1236},  //duty=12, dutyLt=8	
			{ 864, 512,1208},  //duty=13, dutyLt=8	
			{ 874, 512,1185},  //duty=14, dutyLt=8	
			{ 886, 512,1172},  //duty=15, dutyLt=8	
			{ 899, 512,1152},  //duty=16, dutyLt=8	
			{ 905, 512,1135},  //duty=17, dutyLt=8	
			{ 913, 512,1123},  //duty=18, dutyLt=8	
			{ 922, 512,1118},  //duty=19, dutyLt=8	
			{ 931, 512,1104},  //duty=20, dutyLt=8	
			{ 512, 512, 512},  //duty=21, dutyLt=8	
			{ 512, 512, 512},  //duty=22, dutyLt=8	
			{ 512, 512, 512},  //duty=23, dutyLt=8	
			{ 512, 512, 512},  //duty=24, dutyLt=8	
			{ 512, 512, 512},  //duty=25, dutyLt=8	
			{ 563, 512,2348},  //duty=-1, dutyLt=9	
			{ 587, 512,2131},  //duty=0, dutyLt=9  
			{ 613, 512,1956},  //duty=1, dutyLt=9  
			{ 630, 512,1854},  //duty=2, dutyLt=9  
			{ 649, 512,1761},  //duty=3, dutyLt=9  
			{ 670, 512,1674},  //duty=4, dutyLt=9  
			{ 683, 512,1627},  //duty=5, dutyLt=9  
			{ 698, 512,1576},  //duty=6, dutyLt=9  
			{ 718, 512,1512},  //duty=7, dutyLt=9  
			{ 742, 512,1445},  //duty=8, dutyLt=9  
			{ 768, 512,1383},  //duty=9, dutyLt=9  
			{ 786, 512,1342},  //duty=10, dutyLt=9	
			{ 802, 512,1308},  //duty=11, dutyLt=9	
			{ 817, 512,1279},  //duty=12, dutyLt=9	
			{ 834, 512,1248},  //duty=13, dutyLt=9	
			{ 846, 512,1227},  //duty=14, dutyLt=9	
			{ 856, 512,1208},  //duty=15, dutyLt=9	
			{ 868, 512,1187},  //duty=16, dutyLt=9	
			{ 877, 512,1173},  //duty=17, dutyLt=9	
			{ 885, 512,1159},  //duty=18, dutyLt=9	
			{ 892, 512,1149},  //duty=19, dutyLt=9	
			{ 512, 512, 512},  //duty=20, dutyLt=9	
			{ 512, 512, 512},  //duty=21, dutyLt=9	
			{ 512, 512, 512},  //duty=22, dutyLt=9	
			{ 512, 512, 512},  //duty=23, dutyLt=9	
			{ 512, 512, 512},  //duty=24, dutyLt=9	
			{ 512, 512, 512},  //duty=25, dutyLt=9	
			{ 563, 512,2335},  //duty=-1, dutyLt=10  
			{ 584, 512,2151},  //duty=0, dutyLt=10	
			{ 605, 512,2002},  //duty=1, dutyLt=10	
			{ 623, 512,1889},  //duty=2, dutyLt=10	
			{ 641, 512,1800},  //duty=3, dutyLt=10	
			{ 657, 512,1727},  //duty=4, dutyLt=10	
			{ 671, 512,1668},  //duty=5, dutyLt=10	
			{ 685, 512,1615},  //duty=6, dutyLt=10	
			{ 704, 512,1550},  //duty=7, dutyLt=10	
			{ 727, 512,1483},  //duty=8, dutyLt=10	
			{ 751, 512,1417},  //duty=9, dutyLt=10	
			{ 769, 512,1374},  //duty=10, dutyLt=10  
			{ 784, 512,1339},  //duty=11, dutyLt=10  
			{ 798, 512,1308},  //duty=12, dutyLt=10  
			{ 814, 512,1277},  //duty=13, dutyLt=10  
			{ 826, 512,1254},  //duty=14, dutyLt=10  
			{ 836, 512,1235},  //duty=15, dutyLt=10  
			{ 849, 512,1213},  //duty=16, dutyLt=10  
			{ 857, 512,1196},  //duty=17, dutyLt=10  
			{ 865, 512,1182},  //duty=18, dutyLt=10  
			{ 512, 512, 512},  //duty=19, dutyLt=10  
			{ 512, 512, 512},  //duty=20, dutyLt=10  
			{ 512, 512, 512},  //duty=21, dutyLt=10  
			{ 512, 512, 512},  //duty=22, dutyLt=10  
			{ 512, 512, 512},  //duty=23, dutyLt=10  
			{ 512, 512, 512},  //duty=24, dutyLt=10  
			{ 512, 512, 512},  //duty=25, dutyLt=10  
			{ 564, 512,2328},  //duty=-1, dutyLt=11  
			{ 583, 512,2166},  //duty=0, dutyLt=11	
			{ 601, 512,2024},  //duty=1, dutyLt=11	
			{ 618, 512,1917},  //duty=2, dutyLt=11	
			{ 634, 512,1831},  //duty=3, dutyLt=11	
			{ 648, 512,1761},  //duty=4, dutyLt=11	
			{ 662, 512,1700},  //duty=5, dutyLt=11	
			{ 675, 512,1647},  //duty=6, dutyLt=11	
			{ 693, 512,1583},  //duty=7, dutyLt=11	
			{ 714, 512,1514},  //duty=8, dutyLt=11	
			{ 739, 512,1441},  //duty=9, dutyLt=11	
			{ 754, 512,1403},  //duty=10, dutyLt=11  
			{ 769, 512,1368},  //duty=11, dutyLt=11  
			{ 782, 512,1336},  //duty=12, dutyLt=11  
			{ 798, 512,1302},  //duty=13, dutyLt=11  
			{ 810, 512,1273},  //duty=14, dutyLt=11  
			{ 820, 512,1253},  //duty=15, dutyLt=11  
			{ 831, 512,1235},  //duty=16, dutyLt=11  
			{ 840, 512,1220},  //duty=17, dutyLt=11  
			{ 512, 512, 512},  //duty=18, dutyLt=11  
			{ 512, 512, 512},  //duty=19, dutyLt=11  
			{ 512, 512, 512},  //duty=20, dutyLt=11  
			{ 512, 512, 512},  //duty=21, dutyLt=11  
			{ 512, 512, 512},  //duty=22, dutyLt=11  
			{ 512, 512, 512},  //duty=23, dutyLt=11  
			{ 512, 512, 512},  //duty=24, dutyLt=11  
			{ 512, 512, 512},  //duty=25, dutyLt=11  
			{ 564, 512,2325},  //duty=-1, dutyLt=12  
			{ 580, 512,2174},  //duty=0, dutyLt=12	
			{ 598, 512,2042},  //duty=1, dutyLt=12	
			{ 613, 512,1941},  //duty=2, dutyLt=12	
			{ 628, 512,1857},  //duty=3, dutyLt=12	
			{ 642, 512,1787},  //duty=4, dutyLt=12	
			{ 654, 512,1729},  //duty=5, dutyLt=12	
			{ 666, 512,1678},  //duty=6, dutyLt=12	
			{ 683, 512,1613},  //duty=7, dutyLt=12	
			{ 703, 512,1543},  //duty=8, dutyLt=12	
			{ 725, 512,1475},  //duty=9, dutyLt=12	
			{ 741, 512,1431},  //duty=10, dutyLt=12  
			{ 756, 512,1393},  //duty=11, dutyLt=12  
			{ 769, 512,1361},  //duty=12, dutyLt=12  
			{ 784, 512,1326},  //duty=13, dutyLt=12  
			{ 795, 512,1301},  //duty=14, dutyLt=12  
			{ 805, 512,1281},  //duty=15, dutyLt=12  
			{ 817, 512,1251},  //duty=16, dutyLt=12  
			{ 512, 512, 512},  //duty=17, dutyLt=12  
			{ 512, 512, 512},  //duty=18, dutyLt=12  
			{ 512, 512, 512},  //duty=19, dutyLt=12  
			{ 512, 512, 512},  //duty=20, dutyLt=12  
			{ 512, 512, 512},  //duty=21, dutyLt=12  
			{ 512, 512, 512},  //duty=22, dutyLt=12  
			{ 512, 512, 512},  //duty=23, dutyLt=12  
			{ 512, 512, 512},  //duty=24, dutyLt=12  
			{ 512, 512, 512},  //duty=25, dutyLt=12  
			{ 564, 512,2316},  //duty=-1, dutyLt=13  
			{ 579, 512,2178},  //duty=0, dutyLt=13	
			{ 595, 512,2061},  //duty=1, dutyLt=13	
			{ 609, 512,1963},  //duty=2, dutyLt=13	
			{ 623, 512,1883},  //duty=3, dutyLt=13	
			{ 635, 512,1815},  //duty=4, dutyLt=13	
			{ 647, 512,1758},  //duty=5, dutyLt=13	
			{ 658, 512,1707},  //duty=6, dutyLt=13	
			{ 674, 512,1643},  //duty=7, dutyLt=13	
			{ 693, 512,1573},  //duty=8, dutyLt=13	
			{ 714, 512,1505},  //duty=9, dutyLt=13	
			{ 729, 512,1459},  //duty=10, dutyLt=13  
			{ 742, 512,1423},  //duty=11, dutyLt=13  
			{ 755, 512,1388},  //duty=12, dutyLt=13  
			{ 770, 512,1352},  //duty=13, dutyLt=13  
			{ 782, 512,1319},  //duty=14, dutyLt=13  
			{ 790, 512,1305},  //duty=15, dutyLt=13  
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
			{ 564, 512,2314},  //duty=-1, dutyLt=14  
			{ 578, 512,2185},  //duty=0, dutyLt=14	
			{ 593, 512,2070},  //duty=1, dutyLt=14	
			{ 606, 512,1977},  //duty=2, dutyLt=14	
			{ 619, 512,1900},  //duty=3, dutyLt=14	
			{ 631, 512,1835},  //duty=4, dutyLt=14	
			{ 642, 512,1779},  //duty=5, dutyLt=14	
			{ 653, 512,1729},  //duty=6, dutyLt=14	
			{ 667, 512,1665},  //duty=7, dutyLt=14	
			{ 686, 512,1595},  //duty=8, dutyLt=14	
			{ 706, 512,1526},  //duty=9, dutyLt=14	
			{ 720, 512,1481},  //duty=10, dutyLt=14  
			{ 734, 512,1442},  //duty=11, dutyLt=14  
			{ 746, 512,1408},  //duty=12, dutyLt=14  
			{ 760, 512,1371},  //duty=13, dutyLt=14  
			{ 771, 512,1345},  //duty=14, dutyLt=14  
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
			{ 564, 512,2310},  //duty=-1, dutyLt=15  
			{ 578, 512,2186},  //duty=0, dutyLt=15	
			{ 591, 512,2079},  //duty=1, dutyLt=15	
			{ 604, 512,1992},  //duty=2, dutyLt=15	
			{ 616, 512,1915},  //duty=3, dutyLt=15	
			{ 627, 512,1852},  //duty=4, dutyLt=15	
			{ 641, 512,1779},  //duty=5, dutyLt=15	
			{ 648, 512,1747},  //duty=6, dutyLt=15	
			{ 663, 512,1683},  //duty=7, dutyLt=15	
			{ 679, 512,1617},  //duty=8, dutyLt=15	
			{ 699, 512,1547},  //duty=9, dutyLt=15	
			{ 713, 512,1500},  //duty=10, dutyLt=15  
			{ 726, 512,1459},  //duty=11, dutyLt=15  
			{ 738, 512,1424},  //duty=12, dutyLt=15  
			{ 752, 512,1388},  //duty=13, dutyLt=15  
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
			{ 564, 512,2306},  //duty=-1, dutyLt=16  
			{ 577, 512,2189},  //duty=0, dutyLt=16	
			{ 589, 512,2093},  //duty=1, dutyLt=16	
			{ 602, 512,2004},  //duty=2, dutyLt=16	
			{ 617, 512,1909},  //duty=3, dutyLt=16	
			{ 623, 512,1871},  //duty=4, dutyLt=16	
			{ 634, 512,1815},  //duty=5, dutyLt=16	
			{ 643, 512,1768},  //duty=6, dutyLt=16	
			{ 656, 512,1707},  //duty=7, dutyLt=16	
			{ 673, 512,1636},  //duty=8, dutyLt=16	
			{ 692, 512,1568},  //duty=9, dutyLt=16	
			{ 705, 512,1522},  //duty=10, dutyLt=16  
			{ 717, 512,1483},  //duty=11, dutyLt=16  
			{ 728, 512,1448},  //duty=12, dutyLt=16  
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
			{ 565, 512,2300},  //duty=-1, dutyLt=17  
			{ 577, 512,2189},  //duty=0, dutyLt=17	
			{ 589, 512,2092},  //duty=1, dutyLt=17	
			{ 600, 512,2012},  //duty=2, dutyLt=17	
			{ 610, 512,1946},  //duty=3, dutyLt=17	
			{ 621, 512,1882},  //duty=4, dutyLt=17	
			{ 630, 512,1831},  //duty=5, dutyLt=17	
			{ 642, 512,1772},  //duty=6, dutyLt=17	
			{ 657, 512,1700},  //duty=7, dutyLt=17	
			{ 673, 512,1634},  //duty=8, dutyLt=17	
			{ 686, 512,1582},  //duty=9, dutyLt=17	
			{ 700, 512,1537},  //duty=10, dutyLt=17  
			{ 716, 512,1481},  //duty=11, dutyLt=17  
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
			{ 565, 512,2295},  //duty=-1, dutyLt=18  
			{ 578, 512,2181},  //duty=0, dutyLt=18	
			{ 588, 512,2098},  //duty=1, dutyLt=18	
			{ 599, 512,2020},  //duty=2, dutyLt=18	
			{ 609, 512,1952},  //duty=3, dutyLt=18	
			{ 619, 512,1892},  //duty=4, dutyLt=18	
			{ 628, 512,1840},  //duty=5, dutyLt=18	
			{ 637, 512,1794},  //duty=6, dutyLt=18	
			{ 649, 512,1734},  //duty=7, dutyLt=18	
			{ 665, 512,1666},  //duty=8, dutyLt=18	
			{ 682, 512,1598},  //duty=9, dutyLt=18	
			{ 694, 512,1551},  //duty=10, dutyLt=18  
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
			{ 569, 512,2260},  //duty=-1, dutyLt=19  
			{ 576, 512,2194},  //duty=0, dutyLt=19	
			{ 587, 512,2104},  //duty=1, dutyLt=19	
			{ 598, 512,2027},  //duty=2, dutyLt=19	
			{ 608, 512,1960},  //duty=3, dutyLt=19	
			{ 618, 512,1900},  //duty=4, dutyLt=19	
			{ 626, 512,1850},  //duty=5, dutyLt=19	
			{ 634, 512,1807},  //duty=6, dutyLt=19	
			{ 647, 512,1742},  //duty=7, dutyLt=19	
			{ 662, 512,1676},  //duty=8, dutyLt=19	
			{ 678, 512,1608},  //duty=9, dutyLt=19	
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
			{ 566, 512,2287},  //duty=-1, dutyLt=20  
			{ 576, 512,2192},  //duty=0, dutyLt=20	
			{ 589, 512,2089},  //duty=1, dutyLt=20	
			{ 597, 512,2033},  //duty=2, dutyLt=20	
			{ 606, 512,1969},  //duty=3, dutyLt=20	
			{ 615, 512,1912},  //duty=4, dutyLt=20	
			{ 623, 512,1865},  //duty=5, dutyLt=20	
			{ 632, 512,1818},  //duty=6, dutyLt=20	
			{ 649, 512,1731},  //duty=7, dutyLt=20	
			{ 658, 512,1691},  //duty=8, dutyLt=20	
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
			{ 565, 512,2288},  //duty=-1, dutyLt=21  
			{ 581, 512,2152},  //duty=0, dutyLt=21	
			{ 586, 512,2113},  //duty=1, dutyLt=21	
			{ 596, 512,2038},  //duty=2, dutyLt=21	
			{ 605, 512,1976},  //duty=3, dutyLt=21	
			{ 614, 512,1918},  //duty=4, dutyLt=21	
			{ 622, 512,1871},  //duty=5, dutyLt=21	
			{ 630, 512,1825},  //duty=6, dutyLt=21	
			{ 641, 512,1768},  //duty=7, dutyLt=21	
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
			{ 571, 512,2237},  //duty=-1, dutyLt=22  
			{ 576, 512,2192},  //duty=0, dutyLt=22	
			{ 586, 512,2109},  //duty=1, dutyLt=22	
			{ 596, 512,2039},  //duty=2, dutyLt=22	
			{ 603, 512,1983},  //duty=3, dutyLt=22	
			{ 613, 512,1924},  //duty=4, dutyLt=22	
			{ 620, 512,1879},  //duty=5, dutyLt=22	
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
			{ 566, 512,2281},  //duty=-1, dutyLt=23  
			{ 577, 512,2188},  //duty=0, dutyLt=23	
			{ 586, 512,2110},  //duty=1, dutyLt=23	
			{ 595, 512,2043},  //duty=2, dutyLt=23	
			{ 604, 512,1980},  //duty=3, dutyLt=23	
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
			{ 567, 512,2265},  //duty=-1, dutyLt=24  
			{ 583, 512,2139},  //duty=0, dutyLt=24	
			{ 593, 512,2062},  //duty=1, dutyLt=24	
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
			{ 567, 512,2267},  //duty=-1, dutyLt=25  
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

