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
    { 485, 512, 607},  //duty=-1, dutyLt=-1
    {1166, 512, 991},  //duty=0, dutyLt=-1
    {1165, 512, 986},  //duty=1, dutyLt=-1
    {1165, 512, 982},  //duty=2, dutyLt=-1
    {1165, 512, 978},  //duty=3, dutyLt=-1
    {1165, 512, 974},  //duty=4, dutyLt=-1
    {1164, 512, 969},  //duty=5, dutyLt=-1
    {1163, 512, 961},  //duty=6, dutyLt=-1
    {1161, 512, 951},  //duty=7, dutyLt=-1
    {1160, 512, 949},  //duty=8, dutyLt=-1
    {1162, 512, 947},  //duty=9, dutyLt=-1
    {1162, 512, 944},  //duty=10, dutyLt=-1
    {1162, 512, 942},  //duty=11, dutyLt=-1
    {1162, 512, 939},  //duty=12, dutyLt=-1
    {1163, 512, 936},  //duty=13, dutyLt=-1
    {1161, 512, 934},  //duty=14, dutyLt=-1
    {1161, 512, 932},  //duty=15, dutyLt=-1
    {1161, 512, 927},  //duty=16, dutyLt=-1
    {1161, 512, 922},  //duty=17, dutyLt=-1
    {1160, 512, 917},  //duty=18, dutyLt=-1
    {1160, 512, 913},  //duty=19, dutyLt=-1
    {1160, 512, 909},  //duty=20, dutyLt=-1
    {1159, 512, 904},  //duty=21, dutyLt=-1
    {1159, 512, 900},  //duty=22, dutyLt=-1
    {1158, 512, 896},  //duty=23, dutyLt=-1
    {1159, 512, 892},  //duty=24, dutyLt=-1
    {1158, 512, 888},  //duty=25, dutyLt=-1
    {1159, 512, 885},  //duty=26, dutyLt=-1
    {1159, 512, 881},  //duty=27, dutyLt=-1
    {1159, 512, 877},  //duty=28, dutyLt=-1
    {1160, 512, 873},  //duty=29, dutyLt=-1
    { 528, 512,1981},  //duty=-1, dutyLt=0
    { 778, 512,1251},  //duty=0, dutyLt=0
    { 884, 512,1137},  //duty=1, dutyLt=0
    { 944, 512,1090},  //duty=2, dutyLt=0
    { 981, 512,1063},  //duty=3, dutyLt=0
    {1007, 512,1043},  //duty=4, dutyLt=0
    {1025, 512,1026},  //duty=5, dutyLt=0
    {1037, 512,1010},  //duty=6, dutyLt=0
    {1049, 512,1003},  //duty=7, dutyLt=0
    {1059, 512, 994},  //duty=8, dutyLt=0
    {1065, 512, 989},  //duty=9, dutyLt=0
    {1072, 512, 983},  //duty=10, dutyLt=0
    {1077, 512, 978},  //duty=11, dutyLt=0
    {1082, 512, 973},  //duty=12, dutyLt=0
    {1086, 512, 969},  //duty=13, dutyLt=0
    {1089, 512, 965},  //duty=14, dutyLt=0
    {1092, 512, 960},  //duty=15, dutyLt=0
    {1100, 512, 952},  //duty=16, dutyLt=0
    {1104, 512, 947},  //duty=17, dutyLt=0
    {1106, 512, 940},  //duty=18, dutyLt=0
    {1109, 512, 934},  //duty=19, dutyLt=0
    {1111, 512, 929},  //duty=20, dutyLt=0
    {1113, 512, 924},  //duty=21, dutyLt=0
    {1115, 512, 919},  //duty=22, dutyLt=0
    {1116, 512, 914},  //duty=23, dutyLt=0
    {1117, 512, 910},  //duty=24, dutyLt=0
    {1119, 512, 906},  //duty=25, dutyLt=0
    {1119, 512, 901},  //duty=26, dutyLt=0
    {1120, 512, 898},  //duty=27, dutyLt=0
    {1122, 512, 894},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 527, 512,1975},  //duty=-1, dutyLt=1
    { 685, 512,1397},  //duty=0, dutyLt=1
    { 778, 512,1244},  //duty=1, dutyLt=1
    { 840, 512,1173},  //duty=2, dutyLt=1
    { 883, 512,1130},  //duty=3, dutyLt=1
    { 915, 512,1099},  //duty=4, dutyLt=1
    { 937, 512,1071},  //duty=5, dutyLt=1
    { 957, 512,1057},  //duty=6, dutyLt=1
    { 973, 512,1043},  //duty=7, dutyLt=1
    { 987, 512,1032},  //duty=8, dutyLt=1
    { 999, 512,1023},  //duty=9, dutyLt=1
    {1008, 512,1016},  //duty=10, dutyLt=1
    {1016, 512,1008},  //duty=11, dutyLt=1
    {1023, 512,1001},  //duty=12, dutyLt=1
    {1030, 512, 997},  //duty=13, dutyLt=1
    {1036, 512, 990},  //duty=14, dutyLt=1
    {1041, 512, 985},  //duty=15, dutyLt=1
    {1048, 512, 977},  //duty=16, dutyLt=1
    {1055, 512, 968},  //duty=17, dutyLt=1
    {1061, 512, 961},  //duty=18, dutyLt=1
    {1065, 512, 954},  //duty=19, dutyLt=1
    {1070, 512, 947},  //duty=20, dutyLt=1
    {1073, 512, 941},  //duty=21, dutyLt=1
    {1076, 512, 936},  //duty=22, dutyLt=1
    {1079, 512, 931},  //duty=23, dutyLt=1
    {1081, 512, 926},  //duty=24, dutyLt=1
    {1083, 512, 921},  //duty=25, dutyLt=1
    {1085, 512, 917},  //duty=26, dutyLt=1
    {1087, 512, 912},  //duty=27, dutyLt=1
    {1088, 512, 907},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 527, 512,1969},  //duty=-1, dutyLt=2
    { 643, 512,1486},  //duty=0, dutyLt=2
    { 722, 512,1323},  //duty=1, dutyLt=2
    { 779, 512,1238},  //duty=2, dutyLt=2
    { 822, 512,1184},  //duty=3, dutyLt=2
    { 854, 512,1140},  //duty=4, dutyLt=2
    { 878, 512,1114},  //duty=5, dutyLt=2
    { 901, 512,1096},  //duty=6, dutyLt=2
    { 919, 512,1079},  //duty=7, dutyLt=2
    { 934, 512,1066},  //duty=8, dutyLt=2
    { 947, 512,1054},  //duty=9, dutyLt=2
    { 959, 512,1044},  //duty=10, dutyLt=2
    { 969, 512,1036},  //duty=11, dutyLt=2
    { 977, 512,1027},  //duty=12, dutyLt=2
    { 986, 512,1021},  //duty=13, dutyLt=2
    { 993, 512,1014},  //duty=14, dutyLt=2
    { 999, 512,1007},  //duty=15, dutyLt=2
    {1009, 512, 997},  //duty=16, dutyLt=2
    {1018, 512, 987},  //duty=17, dutyLt=2
    {1025, 512, 979},  //duty=18, dutyLt=2
    {1031, 512, 970},  //duty=19, dutyLt=2
    {1036, 512, 963},  //duty=20, dutyLt=2
    {1041, 512, 957},  //duty=21, dutyLt=2
    {1045, 512, 951},  //duty=22, dutyLt=2
    {1049, 512, 945},  //duty=23, dutyLt=2
    {1052, 512, 939},  //duty=24, dutyLt=2
    {1055, 512, 934},  //duty=25, dutyLt=2
    {1058, 512, 930},  //duty=26, dutyLt=2
    {1060, 512, 924},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 526, 512,1966},  //duty=-1, dutyLt=3
    { 620, 512,1550},  //duty=0, dutyLt=3
    { 688, 512,1384},  //duty=1, dutyLt=3
    { 739, 512,1291},  //duty=2, dutyLt=3
    { 780, 512,1225},  //duty=3, dutyLt=3
    { 810, 512,1182},  //duty=4, dutyLt=3
    { 836, 512,1154},  //duty=5, dutyLt=3
    { 859, 512,1130},  //duty=6, dutyLt=3
    { 878, 512,1112},  //duty=7, dutyLt=3
    { 894, 512,1096},  //duty=8, dutyLt=3
    { 908, 512,1082},  //duty=9, dutyLt=3
    { 920, 512,1071},  //duty=10, dutyLt=3
    { 931, 512,1060},  //duty=11, dutyLt=3
    { 941, 512,1051},  //duty=12, dutyLt=3
    { 949, 512,1042},  //duty=13, dutyLt=3
    { 957, 512,1035},  //duty=14, dutyLt=3
    { 964, 512,1028},  //duty=15, dutyLt=3
    { 976, 512,1015},  //duty=16, dutyLt=3
    { 986, 512,1005},  //duty=17, dutyLt=3
    { 995, 512, 995},  //duty=18, dutyLt=3
    {1002, 512, 986},  //duty=19, dutyLt=3
    {1008, 512, 978},  //duty=20, dutyLt=3
    {1014, 512, 970},  //duty=21, dutyLt=3
    {1018, 512, 964},  //duty=22, dutyLt=3
    {1023, 512, 957},  //duty=23, dutyLt=3
    {1027, 512, 952},  //duty=24, dutyLt=3
    {1030, 512, 946},  //duty=25, dutyLt=3
    {1033, 512, 941},  //duty=26, dutyLt=3
    {1035, 512, 936},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 526, 512,1961},  //duty=-1, dutyLt=4
    { 604, 512,1598},  //duty=0, dutyLt=4
    { 664, 512,1432},  //duty=1, dutyLt=4
    { 711, 512,1333},  //duty=2, dutyLt=4
    { 747, 512,1262},  //duty=3, dutyLt=4
    { 778, 512,1218},  //duty=4, dutyLt=4
    { 804, 512,1188},  //duty=5, dutyLt=4
    { 826, 512,1161},  //duty=6, dutyLt=4
    { 845, 512,1140},  //duty=7, dutyLt=4
    { 861, 512,1123},  //duty=8, dutyLt=4
    { 876, 512,1108},  //duty=9, dutyLt=4
    { 888, 512,1094},  //duty=10, dutyLt=4
    { 900, 512,1083},  //duty=11, dutyLt=4
    { 910, 512,1072},  //duty=12, dutyLt=4
    { 919, 512,1063},  //duty=13, dutyLt=4
    { 928, 512,1054},  //duty=14, dutyLt=4
    { 935, 512,1046},  //duty=15, dutyLt=4
    { 948, 512,1033},  //duty=16, dutyLt=4
    { 959, 512,1020},  //duty=17, dutyLt=4
    { 969, 512,1010},  //duty=18, dutyLt=4
    { 977, 512,1000},  //duty=19, dutyLt=4
    { 983, 512, 992},  //duty=20, dutyLt=4
    { 990, 512, 984},  //duty=21, dutyLt=4
    { 995, 512, 977},  //duty=22, dutyLt=4
    {1000, 512, 970},  //duty=23, dutyLt=4
    {1004, 512, 964},  //duty=24, dutyLt=4
    {1008, 512, 958},  //duty=25, dutyLt=4
    {1011, 512, 952},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 526, 512,1957},  //duty=-1, dutyLt=5
    { 593, 512,1632},  //duty=0, dutyLt=5
    { 647, 512,1469},  //duty=1, dutyLt=5
    { 691, 512,1360},  //duty=2, dutyLt=5
    { 725, 512,1298},  //duty=3, dutyLt=5
    { 754, 512,1252},  //duty=4, dutyLt=5
    { 779, 512,1216},  //duty=5, dutyLt=5
    { 801, 512,1189},  //duty=6, dutyLt=5
    { 818, 512,1165},  //duty=7, dutyLt=5
    { 835, 512,1147},  //duty=8, dutyLt=5
    { 850, 512,1130},  //duty=9, dutyLt=5
    { 863, 512,1116},  //duty=10, dutyLt=5
    { 874, 512,1103},  //duty=11, dutyLt=5
    { 885, 512,1091},  //duty=12, dutyLt=5
    { 894, 512,1081},  //duty=13, dutyLt=5
    { 902, 512,1072},  //duty=14, dutyLt=5
    { 911, 512,1063},  //duty=15, dutyLt=5
    { 924, 512,1048},  //duty=16, dutyLt=5
    { 936, 512,1035},  //duty=17, dutyLt=5
    { 946, 512,1024},  //duty=18, dutyLt=5
    { 955, 512,1014},  //duty=19, dutyLt=5
    { 962, 512,1004},  //duty=20, dutyLt=5
    { 969, 512, 996},  //duty=21, dutyLt=5
    { 974, 512, 988},  //duty=22, dutyLt=5
    { 980, 512, 981},  //duty=23, dutyLt=5
    { 984, 512, 974},  //duty=24, dutyLt=5
    { 988, 512, 968},  //duty=25, dutyLt=5
    { 992, 512, 962},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 526, 512,1954},  //duty=-1, dutyLt=6
    { 585, 512,1659},  //duty=0, dutyLt=6
    { 634, 512,1498},  //duty=1, dutyLt=6
    { 674, 512,1392},  //duty=2, dutyLt=6
    { 706, 512,1325},  //duty=3, dutyLt=6
    { 734, 512,1280},  //duty=4, dutyLt=6
    { 758, 512,1243},  //duty=5, dutyLt=6
    { 779, 512,1213},  //duty=6, dutyLt=6
    { 797, 512,1189},  //duty=7, dutyLt=6
    { 813, 512,1169},  //duty=8, dutyLt=6
    { 827, 512,1150},  //duty=9, dutyLt=6
    { 840, 512,1135},  //duty=10, dutyLt=6
    { 852, 512,1122},  //duty=11, dutyLt=6
    { 863, 512,1109},  //duty=12, dutyLt=6
    { 872, 512,1099},  //duty=13, dutyLt=6
    { 881, 512,1088},  //duty=14, dutyLt=6
    { 890, 512,1080},  //duty=15, dutyLt=6
    { 903, 512,1063},  //duty=16, dutyLt=6
    { 916, 512,1049},  //duty=17, dutyLt=6
    { 926, 512,1037},  //duty=18, dutyLt=6
    { 935, 512,1026},  //duty=19, dutyLt=6
    { 943, 512,1016},  //duty=20, dutyLt=6
    { 950, 512,1007},  //duty=21, dutyLt=6
    { 956, 512, 999},  //duty=22, dutyLt=6
    { 962, 512, 992},  //duty=23, dutyLt=6
    { 966, 512, 985},  //duty=24, dutyLt=6
    { 971, 512, 978},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 526, 512,1951},  //duty=-1, dutyLt=7
    { 580, 512,1679},  //duty=0, dutyLt=7
    { 624, 512,1515},  //duty=1, dutyLt=7
    { 660, 512,1420},  //duty=2, dutyLt=7
    { 691, 512,1353},  //duty=3, dutyLt=7
    { 718, 512,1304},  //duty=4, dutyLt=7
    { 741, 512,1267},  //duty=5, dutyLt=7
    { 761, 512,1235},  //duty=6, dutyLt=7
    { 779, 512,1211},  //duty=7, dutyLt=7
    { 794, 512,1189},  //duty=8, dutyLt=7
    { 809, 512,1169},  //duty=9, dutyLt=7
    { 822, 512,1154},  //duty=10, dutyLt=7
    { 833, 512,1139},  //duty=11, dutyLt=7
    { 844, 512,1126},  //duty=12, dutyLt=7
    { 853, 512,1115},  //duty=13, dutyLt=7
    { 863, 512,1104},  //duty=14, dutyLt=7
    { 871, 512,1094},  //duty=15, dutyLt=7
    { 885, 512,1077},  //duty=16, dutyLt=7
    { 898, 512,1062},  //duty=17, dutyLt=7
    { 908, 512,1049},  //duty=18, dutyLt=7
    { 918, 512,1038},  //duty=19, dutyLt=7
    { 926, 512,1028},  //duty=20, dutyLt=7
    { 933, 512,1018},  //duty=21, dutyLt=7
    { 940, 512,1009},  //duty=22, dutyLt=7
    { 945, 512,1002},  //duty=23, dutyLt=7
    { 950, 512, 994},  //duty=24, dutyLt=7
    { 955, 512, 987},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 526, 512,1947},  //duty=-1, dutyLt=8
    { 576, 512,1692},  //duty=0, dutyLt=8
    { 616, 512,1538},  //duty=1, dutyLt=8
    { 650, 512,1444},  //duty=2, dutyLt=8
    { 679, 512,1378},  //duty=3, dutyLt=8
    { 704, 512,1326},  //duty=4, dutyLt=8
    { 726, 512,1288},  //duty=5, dutyLt=8
    { 746, 512,1255},  //duty=6, dutyLt=8
    { 763, 512,1229},  //duty=7, dutyLt=8
    { 779, 512,1206},  //duty=8, dutyLt=8
    { 793, 512,1188},  //duty=9, dutyLt=8
    { 805, 512,1171},  //duty=10, dutyLt=8
    { 817, 512,1155},  //duty=11, dutyLt=8
    { 828, 512,1142},  //duty=12, dutyLt=8
    { 837, 512,1130},  //duty=13, dutyLt=8
    { 846, 512,1118},  //duty=14, dutyLt=8
    { 855, 512,1108},  //duty=15, dutyLt=8
    { 869, 512,1090},  //duty=16, dutyLt=8
    { 882, 512,1074},  //duty=17, dutyLt=8
    { 893, 512,1061},  //duty=18, dutyLt=8
    { 902, 512,1049},  //duty=19, dutyLt=8
    { 911, 512,1038},  //duty=20, dutyLt=8
    { 918, 512,1028},  //duty=21, dutyLt=8
    { 924, 512,1019},  //duty=22, dutyLt=8
    { 930, 512,1011},  //duty=23, dutyLt=8
    { 935, 512,1003},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 527, 512,1942},  //duty=-1, dutyLt=9
    { 572, 512,1696},  //duty=0, dutyLt=9
    { 609, 512,1556},  //duty=1, dutyLt=9
    { 641, 512,1465},  //duty=2, dutyLt=9
    { 669, 512,1398},  //duty=3, dutyLt=9
    { 693, 512,1347},  //duty=4, dutyLt=9
    { 714, 512,1306},  //duty=5, dutyLt=9
    { 733, 512,1275},  //duty=6, dutyLt=9
    { 750, 512,1248},  //duty=7, dutyLt=9
    { 765, 512,1224},  //duty=8, dutyLt=9
    { 778, 512,1204},  //duty=9, dutyLt=9
    { 791, 512,1186},  //duty=10, dutyLt=9
    { 802, 512,1170},  //duty=11, dutyLt=9
    { 813, 512,1156},  //duty=12, dutyLt=9
    { 822, 512,1143},  //duty=13, dutyLt=9
    { 831, 512,1132},  //duty=14, dutyLt=9
    { 840, 512,1121},  //duty=15, dutyLt=9
    { 855, 512,1102},  //duty=16, dutyLt=9
    { 867, 512,1086},  //duty=17, dutyLt=9
    { 878, 512,1072},  //duty=18, dutyLt=9
    { 888, 512,1059},  //duty=19, dutyLt=9
    { 897, 512,1048},  //duty=20, dutyLt=9
    { 904, 512,1038},  //duty=21, dutyLt=9
    { 911, 512,1028},  //duty=22, dutyLt=9
    { 917, 512,1020},  //duty=23, dutyLt=9
    { 922, 512,1012},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 528, 512,1933},  //duty=-1, dutyLt=10
    { 569, 512,1708},  //duty=0, dutyLt=10
    { 603, 512,1572},  //duty=1, dutyLt=10
    { 634, 512,1481},  //duty=2, dutyLt=10
    { 660, 512,1416},  //duty=3, dutyLt=10
    { 683, 512,1365},  //duty=4, dutyLt=10
    { 704, 512,1323},  //duty=5, dutyLt=10
    { 721, 512,1292},  //duty=6, dutyLt=10
    { 738, 512,1264},  //duty=7, dutyLt=10
    { 753, 512,1238},  //duty=8, dutyLt=10
    { 766, 512,1219},  //duty=9, dutyLt=10
    { 779, 512,1200},  //duty=10, dutyLt=10
    { 790, 512,1183},  //duty=11, dutyLt=10
    { 800, 512,1169},  //duty=12, dutyLt=10
    { 810, 512,1156},  //duty=13, dutyLt=10
    { 818, 512,1144},  //duty=14, dutyLt=10
    { 827, 512,1133},  //duty=15, dutyLt=10
    { 842, 512,1113},  //duty=16, dutyLt=10
    { 854, 512,1097},  //duty=17, dutyLt=10
    { 866, 512,1082},  //duty=18, dutyLt=10
    { 875, 512,1069},  //duty=19, dutyLt=10
    { 884, 512,1057},  //duty=20, dutyLt=10
    { 892, 512,1047},  //duty=21, dutyLt=10
    { 898, 512,1037},  //duty=22, dutyLt=10
    { 905, 512,1029},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 528, 512,1919},  //duty=-1, dutyLt=11
    { 566, 512,1718},  //duty=0, dutyLt=11
    { 599, 512,1588},  //duty=1, dutyLt=11
    { 627, 512,1498},  //duty=2, dutyLt=11
    { 653, 512,1434},  //duty=3, dutyLt=11
    { 674, 512,1383},  //duty=4, dutyLt=11
    { 694, 512,1340},  //duty=5, dutyLt=11
    { 712, 512,1307},  //duty=6, dutyLt=11
    { 728, 512,1278},  //duty=7, dutyLt=11
    { 742, 512,1254},  //duty=8, dutyLt=11
    { 755, 512,1233},  //duty=9, dutyLt=11
    { 767, 512,1214},  //duty=10, dutyLt=11
    { 778, 512,1197},  //duty=11, dutyLt=11
    { 789, 512,1182},  //duty=12, dutyLt=11
    { 798, 512,1169},  //duty=13, dutyLt=11
    { 807, 512,1156},  //duty=14, dutyLt=11
    { 815, 512,1144},  //duty=15, dutyLt=11
    { 830, 512,1124},  //duty=16, dutyLt=11
    { 843, 512,1107},  //duty=17, dutyLt=11
    { 854, 512,1091},  //duty=18, dutyLt=11
    { 864, 512,1078},  //duty=19, dutyLt=11
    { 873, 512,1066},  //duty=20, dutyLt=11
    { 880, 512,1055},  //duty=21, dutyLt=11
    { 888, 512,1046},  //duty=22, dutyLt=11
    { 894, 512,1036},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 528, 512,1916},  //duty=-1, dutyLt=12
    { 563, 512,1726},  //duty=0, dutyLt=12
    { 595, 512,1602},  //duty=1, dutyLt=12
    { 622, 512,1512},  //duty=2, dutyLt=12
    { 646, 512,1449},  //duty=3, dutyLt=12
    { 667, 512,1397},  //duty=4, dutyLt=12
    { 686, 512,1354},  //duty=5, dutyLt=12
    { 703, 512,1322},  //duty=6, dutyLt=12
    { 719, 512,1292},  //duty=7, dutyLt=12
    { 733, 512,1267},  //duty=8, dutyLt=12
    { 746, 512,1245},  //duty=9, dutyLt=12
    { 757, 512,1226},  //duty=10, dutyLt=12
    { 768, 512,1209},  //duty=11, dutyLt=12
    { 778, 512,1194},  //duty=12, dutyLt=12
    { 788, 512,1179},  //duty=13, dutyLt=12
    { 796, 512,1167},  //duty=14, dutyLt=12
    { 804, 512,1155},  //duty=15, dutyLt=12
    { 820, 512,1133},  //duty=16, dutyLt=12
    { 833, 512,1116},  //duty=17, dutyLt=12
    { 844, 512,1100},  //duty=18, dutyLt=12
    { 854, 512,1087},  //duty=19, dutyLt=12
    { 862, 512,1074},  //duty=20, dutyLt=12
    { 870, 512,1063},  //duty=21, dutyLt=12
    { 877, 512,1053},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 512, 512, 512},  //duty=28, dutyLt=12
    { 512, 512, 512},  //duty=29, dutyLt=12
    { 528, 512,1914},  //duty=-1, dutyLt=13
    { 562, 512,1734},  //duty=0, dutyLt=13
    { 591, 512,1612},  //duty=1, dutyLt=13
    { 617, 512,1525},  //duty=2, dutyLt=13
    { 640, 512,1461},  //duty=3, dutyLt=13
    { 661, 512,1411},  //duty=4, dutyLt=13
    { 679, 512,1368},  //duty=5, dutyLt=13
    { 696, 512,1335},  //duty=6, dutyLt=13
    { 710, 512,1304},  //duty=7, dutyLt=13
    { 725, 512,1279},  //duty=8, dutyLt=13
    { 737, 512,1257},  //duty=9, dutyLt=13
    { 749, 512,1238},  //duty=10, dutyLt=13
    { 759, 512,1220},  //duty=11, dutyLt=13
    { 769, 512,1205},  //duty=12, dutyLt=13
    { 778, 512,1190},  //duty=13, dutyLt=13
    { 787, 512,1177},  //duty=14, dutyLt=13
    { 795, 512,1165},  //duty=15, dutyLt=13
    { 810, 512,1143},  //duty=16, dutyLt=13
    { 823, 512,1125},  //duty=17, dutyLt=13
    { 834, 512,1109},  //duty=18, dutyLt=13
    { 844, 512,1095},  //duty=19, dutyLt=13
    { 853, 512,1083},  //duty=20, dutyLt=13
    { 861, 512,1071},  //duty=21, dutyLt=13
    { 868, 512,1061},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 528, 512,1913},  //duty=-1, dutyLt=14
    { 560, 512,1740},  //duty=0, dutyLt=14
    { 588, 512,1622},  //duty=1, dutyLt=14
    { 613, 512,1536},  //duty=2, dutyLt=14
    { 635, 512,1474},  //duty=3, dutyLt=14
    { 655, 512,1424},  //duty=4, dutyLt=14
    { 673, 512,1380},  //duty=5, dutyLt=14
    { 689, 512,1347},  //duty=6, dutyLt=14
    { 703, 512,1317},  //duty=7, dutyLt=14
    { 717, 512,1291},  //duty=8, dutyLt=14
    { 729, 512,1268},  //duty=9, dutyLt=14
    { 740, 512,1248},  //duty=10, dutyLt=14
    { 751, 512,1231},  //duty=11, dutyLt=14
    { 761, 512,1215},  //duty=12, dutyLt=14
    { 770, 512,1200},  //duty=13, dutyLt=14
    { 778, 512,1187},  //duty=14, dutyLt=14
    { 786, 512,1175},  //duty=15, dutyLt=14
    { 802, 512,1152},  //duty=16, dutyLt=14
    { 814, 512,1133},  //duty=17, dutyLt=14
    { 825, 512,1117},  //duty=18, dutyLt=14
    { 835, 512,1103},  //duty=19, dutyLt=14
    { 844, 512,1090},  //duty=20, dutyLt=14
    { 852, 512,1078},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 512, 512, 512},  //duty=28, dutyLt=14
    { 512, 512, 512},  //duty=29, dutyLt=14
    { 528, 512,1908},  //duty=-1, dutyLt=15
    { 559, 512,1745},  //duty=0, dutyLt=15
    { 586, 512,1631},  //duty=1, dutyLt=15
    { 610, 512,1546},  //duty=2, dutyLt=15
    { 631, 512,1485},  //duty=3, dutyLt=15
    { 650, 512,1434},  //duty=4, dutyLt=15
    { 667, 512,1392},  //duty=5, dutyLt=15
    { 683, 512,1358},  //duty=6, dutyLt=15
    { 697, 512,1328},  //duty=7, dutyLt=15
    { 710, 512,1302},  //duty=8, dutyLt=15
    { 722, 512,1279},  //duty=9, dutyLt=15
    { 733, 512,1259},  //duty=10, dutyLt=15
    { 744, 512,1241},  //duty=11, dutyLt=15
    { 753, 512,1224},  //duty=12, dutyLt=15
    { 762, 512,1209},  //duty=13, dutyLt=15
    { 771, 512,1196},  //duty=14, dutyLt=15
    { 779, 512,1183},  //duty=15, dutyLt=15
    { 794, 512,1161},  //duty=16, dutyLt=15
    { 806, 512,1141},  //duty=17, dutyLt=15
    { 817, 512,1125},  //duty=18, dutyLt=15
    { 827, 512,1110},  //duty=19, dutyLt=15
    { 836, 512,1097},  //duty=20, dutyLt=15
    { 844, 512,1086},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 528, 512,1902},  //duty=-1, dutyLt=16
    { 556, 512,1752},  //duty=0, dutyLt=16
    { 582, 512,1643},  //duty=1, dutyLt=16
    { 605, 512,1562},  //duty=2, dutyLt=16
    { 624, 512,1499},  //duty=3, dutyLt=16
    { 642, 512,1451},  //duty=4, dutyLt=16
    { 659, 512,1409},  //duty=5, dutyLt=16
    { 674, 512,1376},  //duty=6, dutyLt=16
    { 687, 512,1345},  //duty=7, dutyLt=16
    { 700, 512,1319},  //duty=8, dutyLt=16
    { 711, 512,1295},  //duty=9, dutyLt=16
    { 722, 512,1275},  //duty=10, dutyLt=16
    { 732, 512,1256},  //duty=11, dutyLt=16
    { 741, 512,1240},  //duty=12, dutyLt=16
    { 750, 512,1225},  //duty=13, dutyLt=16
    { 758, 512,1212},  //duty=14, dutyLt=16
    { 766, 512,1199},  //duty=15, dutyLt=16
    { 779, 512,1176},  //duty=16, dutyLt=16
    { 792, 512,1157},  //duty=17, dutyLt=16
    { 803, 512,1139},  //duty=18, dutyLt=16
    { 812, 512,1125},  //duty=19, dutyLt=16
    { 821, 512,1111},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 512, 512, 512},  //duty=28, dutyLt=16
    { 512, 512, 512},  //duty=29, dutyLt=16
    { 529, 512,1895},  //duty=-1, dutyLt=17
    { 554, 512,1758},  //duty=0, dutyLt=17
    { 579, 512,1653},  //duty=1, dutyLt=17
    { 600, 512,1576},  //duty=2, dutyLt=17
    { 618, 512,1517},  //duty=3, dutyLt=17
    { 635, 512,1468},  //duty=4, dutyLt=17
    { 651, 512,1426},  //duty=5, dutyLt=17
    { 665, 512,1392},  //duty=6, dutyLt=17
    { 678, 512,1361},  //duty=7, dutyLt=17
    { 690, 512,1335},  //duty=8, dutyLt=17
    { 701, 512,1311},  //duty=9, dutyLt=17
    { 712, 512,1292},  //duty=10, dutyLt=17
    { 721, 512,1272},  //duty=11, dutyLt=17
    { 730, 512,1255},  //duty=12, dutyLt=17
    { 739, 512,1240},  //duty=13, dutyLt=17
    { 746, 512,1226},  //duty=14, dutyLt=17
    { 754, 512,1213},  //duty=15, dutyLt=17
    { 767, 512,1190},  //duty=16, dutyLt=17
    { 780, 512,1170},  //duty=17, dutyLt=17
    { 790, 512,1153},  //duty=18, dutyLt=17
    { 800, 512,1137},  //duty=19, dutyLt=17
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
    { 529, 512,1895},  //duty=-1, dutyLt=18
    { 553, 512,1764},  //duty=0, dutyLt=18
    { 576, 512,1663},  //duty=1, dutyLt=18
    { 596, 512,1589},  //duty=2, dutyLt=18
    { 613, 512,1531},  //duty=3, dutyLt=18
    { 629, 512,1481},  //duty=4, dutyLt=18
    { 644, 512,1441},  //duty=5, dutyLt=18
    { 658, 512,1406},  //duty=6, dutyLt=18
    { 670, 512,1376},  //duty=7, dutyLt=18
    { 682, 512,1349},  //duty=8, dutyLt=18
    { 693, 512,1326},  //duty=9, dutyLt=18
    { 703, 512,1305},  //duty=10, dutyLt=18
    { 712, 512,1286},  //duty=11, dutyLt=18
    { 721, 512,1269},  //duty=12, dutyLt=18
    { 729, 512,1253},  //duty=13, dutyLt=18
    { 737, 512,1239},  //duty=14, dutyLt=18
    { 744, 512,1226},  //duty=15, dutyLt=18
    { 757, 512,1203},  //duty=16, dutyLt=18
    { 769, 512,1182},  //duty=17, dutyLt=18
    { 780, 512,1164},  //duty=18, dutyLt=18
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
    { 530, 512,1890},  //duty=-1, dutyLt=19
    { 552, 512,1767},  //duty=0, dutyLt=19
    { 574, 512,1671},  //duty=1, dutyLt=19
    { 592, 512,1599},  //duty=2, dutyLt=19
    { 609, 512,1541},  //duty=3, dutyLt=19
    { 625, 512,1493},  //duty=4, dutyLt=19
    { 639, 512,1453},  //duty=5, dutyLt=19
    { 652, 512,1418},  //duty=6, dutyLt=19
    { 664, 512,1388},  //duty=7, dutyLt=19
    { 675, 512,1362},  //duty=8, dutyLt=19
    { 685, 512,1339},  //duty=9, dutyLt=19
    { 695, 512,1317},  //duty=10, dutyLt=19
    { 704, 512,1298},  //duty=11, dutyLt=19
    { 713, 512,1281},  //duty=12, dutyLt=19
    { 721, 512,1265},  //duty=13, dutyLt=19
    { 728, 512,1251},  //duty=14, dutyLt=19
    { 735, 512,1237},  //duty=15, dutyLt=19
    { 748, 512,1214},  //duty=16, dutyLt=19
    { 760, 512,1193},  //duty=17, dutyLt=19
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
    { 530, 512,1882},  //duty=-1, dutyLt=20
    { 552, 512,1768},  //duty=0, dutyLt=20
    { 572, 512,1675},  //duty=1, dutyLt=20
    { 590, 512,1606},  //duty=2, dutyLt=20
    { 606, 512,1551},  //duty=3, dutyLt=20
    { 621, 512,1502},  //duty=4, dutyLt=20
    { 634, 512,1464},  //duty=5, dutyLt=20
    { 647, 512,1430},  //duty=6, dutyLt=20
    { 659, 512,1399},  //duty=7, dutyLt=20
    { 669, 512,1372},  //duty=8, dutyLt=20
    { 679, 512,1349},  //duty=9, dutyLt=20
    { 689, 512,1328},  //duty=10, dutyLt=20
    { 698, 512,1309},  //duty=11, dutyLt=20
    { 706, 512,1291},  //duty=12, dutyLt=20
    { 714, 512,1276},  //duty=13, dutyLt=20
    { 721, 512,1261},  //duty=14, dutyLt=20
    { 728, 512,1248},  //duty=15, dutyLt=20
    { 741, 512,1224},  //duty=16, dutyLt=20
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
    { 531, 512,1879},  //duty=-1, dutyLt=21
    { 551, 512,1771},  //duty=0, dutyLt=21
    { 571, 512,1680},  //duty=1, dutyLt=21
    { 588, 512,1613},  //duty=2, dutyLt=21
    { 603, 512,1559},  //duty=3, dutyLt=21
    { 618, 512,1512},  //duty=4, dutyLt=21
    { 631, 512,1472},  //duty=5, dutyLt=21
    { 643, 512,1438},  //duty=6, dutyLt=21
    { 654, 512,1409},  //duty=7, dutyLt=21
    { 664, 512,1382},  //duty=8, dutyLt=21
    { 674, 512,1359},  //duty=9, dutyLt=21
    { 683, 512,1338},  //duty=10, dutyLt=21
    { 692, 512,1318},  //duty=11, dutyLt=21
    { 700, 512,1301},  //duty=12, dutyLt=21
    { 708, 512,1285},  //duty=13, dutyLt=21
    { 715, 512,1270},  //duty=14, dutyLt=21
    { 721, 512,1257},  //duty=15, dutyLt=21
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
    { 531, 512,1873},  //duty=-1, dutyLt=22
    { 551, 512,1770},  //duty=0, dutyLt=22
    { 570, 512,1684},  //duty=1, dutyLt=22
    { 586, 512,1619},  //duty=2, dutyLt=22
    { 601, 512,1565},  //duty=3, dutyLt=22
    { 615, 512,1520},  //duty=4, dutyLt=22
    { 628, 512,1480},  //duty=5, dutyLt=22
    { 639, 512,1447},  //duty=6, dutyLt=22
    { 650, 512,1417},  //duty=7, dutyLt=22
    { 660, 512,1390},  //duty=8, dutyLt=22
    { 670, 512,1367},  //duty=9, dutyLt=22
    { 679, 512,1346},  //duty=10, dutyLt=22
    { 687, 512,1328},  //duty=11, dutyLt=22
    { 695, 512,1310},  //duty=12, dutyLt=22
    { 703, 512,1293},  //duty=13, dutyLt=22
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
    { 532, 512,1870},  //duty=-1, dutyLt=23
    { 551, 512,1771},  //duty=0, dutyLt=23
    { 569, 512,1687},  //duty=1, dutyLt=23
    { 585, 512,1623},  //duty=2, dutyLt=23
    { 599, 512,1570},  //duty=3, dutyLt=23
    { 613, 512,1525},  //duty=4, dutyLt=23
    { 625, 512,1487},  //duty=5, dutyLt=23
    { 636, 512,1454},  //duty=6, dutyLt=23
    { 647, 512,1424},  //duty=7, dutyLt=23
    { 657, 512,1398},  //duty=8, dutyLt=23
    { 666, 512,1375},  //duty=9, dutyLt=23
    { 675, 512,1353},  //duty=10, dutyLt=23
    { 683, 512,1334},  //duty=11, dutyLt=23
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
    { 533, 512,1866},  //duty=-1, dutyLt=24
    { 551, 512,1769},  //duty=0, dutyLt=24
    { 568, 512,1689},  //duty=1, dutyLt=24
    { 584, 512,1626},  //duty=2, dutyLt=24
    { 597, 512,1575},  //duty=3, dutyLt=24
    { 611, 512,1531},  //duty=4, dutyLt=24
    { 623, 512,1492},  //duty=5, dutyLt=24
    { 634, 512,1459},  //duty=6, dutyLt=24
    { 644, 512,1430},  //duty=7, dutyLt=24
    { 654, 512,1404},  //duty=8, dutyLt=24
    { 663, 512,1381},  //duty=9, dutyLt=24
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
    { 534, 512,1862},  //duty=-1, dutyLt=25
    { 551, 512,1769},  //duty=0, dutyLt=25
    { 568, 512,1690},  //duty=1, dutyLt=25
    { 583, 512,1629},  //duty=2, dutyLt=25
    { 597, 512,1578},  //duty=3, dutyLt=25
    { 609, 512,1534},  //duty=4, dutyLt=25
    { 621, 512,1497},  //duty=5, dutyLt=25
    { 632, 512,1464},  //duty=6, dutyLt=25
    { 642, 512,1435},  //duty=7, dutyLt=25
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
    { 535, 512,1857},  //duty=-1, dutyLt=26
    { 551, 512,1767},  //duty=0, dutyLt=26
    { 568, 512,1690},  //duty=1, dutyLt=26
    { 582, 512,1630},  //duty=2, dutyLt=26
    { 595, 512,1581},  //duty=3, dutyLt=26
    { 608, 512,1537},  //duty=4, dutyLt=26
    { 619, 512,1500},  //duty=5, dutyLt=26
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
    { 535, 512,1853},  //duty=-1, dutyLt=27
    { 552, 512,1765},  //duty=0, dutyLt=27
    { 568, 512,1690},  //duty=1, dutyLt=27
    { 582, 512,1631},  //duty=2, dutyLt=27
    { 595, 512,1583},  //duty=3, dutyLt=27
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
    { 536, 512,1847},  //duty=-1, dutyLt=28
    { 552, 512,1763},  //duty=0, dutyLt=28
    { 568, 512,1690},  //duty=1, dutyLt=28
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
    { 537, 512,1843},  //duty=-1, dutyLt=29
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

