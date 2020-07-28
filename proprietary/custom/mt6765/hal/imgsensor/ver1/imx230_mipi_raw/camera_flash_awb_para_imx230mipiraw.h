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
    {
         5, //YPrimeWeightTh[0]
         9, //YPrimeWeightTh[1]
         11, //YPrimeWeightTh[2]
         13, //YPrimeWeightTh[3]
         15 //YPrimeWeightTh[4]
    },
    {
         1, //YPrimeWeight[0]
         3, //YPrimeWeight[1]
         5, //YPrimeWeight[2]
         7 //YPrimeWeight[3]
    },
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
   {1042, 512, 866},  //duty=0, dutyLt=-1
   {1040, 512, 862},  //duty=1, dutyLt=-1
   {1040, 512, 859},  //duty=2, dutyLt=-1
   {1041, 512, 856},  //duty=3, dutyLt=-1
   {1041, 512, 852},  //duty=4, dutyLt=-1
   {1043, 512, 844},  //duty=5, dutyLt=-1
   {1039, 512, 848},  //duty=6, dutyLt=-1
   {1039, 512, 845},  //duty=7, dutyLt=-1
   {1039, 512, 843},  //duty=8, dutyLt=-1
   {1039, 512, 841},  //duty=9, dutyLt=-1
   {1039, 512, 838},  //duty=10, dutyLt=-1
   {1039, 512, 836},  //duty=11, dutyLt=-1
   {1038, 512, 834},  //duty=12, dutyLt=-1
   {1038, 512, 831},  //duty=13, dutyLt=-1
   {1038, 512, 830},  //duty=14, dutyLt=-1
   {1038, 512, 828},  //duty=15, dutyLt=-1
   {1038, 512, 824},  //duty=16, dutyLt=-1
   {1037, 512, 821},  //duty=17, dutyLt=-1
   {1037, 512, 816},  //duty=18, dutyLt=-1
   {1037, 512, 813},  //duty=19, dutyLt=-1
   {1037, 512, 810},  //duty=20, dutyLt=-1
   {1037, 512, 806},  //duty=21, dutyLt=-1
   {1037, 512, 803},  //duty=22, dutyLt=-1
   {1037, 512, 801},  //duty=23, dutyLt=-1
   {1037, 512, 797},  //duty=24, dutyLt=-1
   {1037, 512, 794},  //duty=25, dutyLt=-1
   { 526, 512,1802},  //duty=-1, dutyLt=0
   { 749, 512,1090},  //duty=0, dutyLt=0
   { 834, 512, 994},  //duty=1, dutyLt=0
   { 879, 512, 953},  //duty=2, dutyLt=0
   { 908, 512, 930},  //duty=3, dutyLt=0
   { 928, 512, 912},  //duty=4, dutyLt=0
   { 939, 512, 901},  //duty=5, dutyLt=0
   { 949, 512, 892},  //duty=6, dutyLt=0
   { 958, 512, 885},  //duty=7, dutyLt=0
   { 964, 512, 879},  //duty=8, dutyLt=0
   { 969, 512, 874},  //duty=9, dutyLt=0
   { 974, 512, 869},  //duty=10, dutyLt=0
   { 977, 512, 865},  //duty=11, dutyLt=0
   { 981, 512, 861},  //duty=12, dutyLt=0
   { 983, 512, 858},  //duty=13, dutyLt=0
   { 986, 512, 854},  //duty=14, dutyLt=0
   { 988, 512, 851},  //duty=15, dutyLt=0
   { 993, 512, 845},  //duty=16, dutyLt=0
   { 996, 512, 840},  //duty=17, dutyLt=0
   { 998, 512, 835},  //duty=18, dutyLt=0
   {1001, 512, 830},  //duty=19, dutyLt=0
   {1002, 512, 826},  //duty=20, dutyLt=0
   {1003, 512, 822},  //duty=21, dutyLt=0
   {1005, 512, 818},  //duty=22, dutyLt=0
   {1006, 512, 815},  //duty=23, dutyLt=0
   {1008, 512, 811},  //duty=24, dutyLt=0
   { 512, 512, 512},  //duty=25, dutyLt=0
   { 525, 512,1802},  //duty=-1, dutyLt=1
   { 671, 512,1221},  //duty=0, dutyLt=1
   { 750, 512,1087},  //duty=1, dutyLt=1
   { 798, 512,1026},  //duty=2, dutyLt=1
   { 833, 512, 987},  //duty=3, dutyLt=1
   { 856, 512, 962},  //duty=4, dutyLt=1
   { 875, 512, 944},  //duty=5, dutyLt=1
   { 889, 512, 931},  //duty=6, dutyLt=1
   { 902, 512, 921},  //duty=7, dutyLt=1
   { 911, 512, 912},  //duty=8, dutyLt=1
   { 919, 512, 904},  //duty=9, dutyLt=1
   { 927, 512, 897},  //duty=10, dutyLt=1
   { 933, 512, 891},  //duty=11, dutyLt=1
   { 938, 512, 885},  //duty=12, dutyLt=1
   { 943, 512, 880},  //duty=13, dutyLt=1
   { 947, 512, 876},  //duty=14, dutyLt=1
   { 950, 512, 871},  //duty=15, dutyLt=1
   { 956, 512, 864},  //duty=16, dutyLt=1
   { 961, 512, 857},  //duty=17, dutyLt=1
   { 966, 512, 851},  //duty=18, dutyLt=1
   { 970, 512, 846},  //duty=19, dutyLt=1
   { 973, 512, 841},  //duty=20, dutyLt=1
   { 976, 512, 836},  //duty=21, dutyLt=1
   { 978, 512, 832},  //duty=22, dutyLt=1
   { 980, 512, 828},  //duty=23, dutyLt=1
   { 983, 512, 824},  //duty=24, dutyLt=1
   { 512, 512, 512},  //duty=25, dutyLt=1
   { 524, 512,1790},  //duty=-1, dutyLt=2
   { 634, 512,1306},  //duty=0, dutyLt=2
   { 703, 512,1158},  //duty=1, dutyLt=2
   { 750, 512,1082},  //duty=2, dutyLt=2
   { 786, 512,1031},  //duty=3, dutyLt=2
   { 809, 512,1004},  //duty=4, dutyLt=2
   { 829, 512, 983},  //duty=5, dutyLt=2
   { 846, 512, 965},  //duty=6, dutyLt=2
   { 860, 512, 951},  //duty=7, dutyLt=2
   { 871, 512, 940},  //duty=8, dutyLt=2
   { 881, 512, 930},  //duty=9, dutyLt=2
   { 890, 512, 922},  //duty=10, dutyLt=2
   { 897, 512, 914},  //duty=11, dutyLt=2
   { 904, 512, 907},  //duty=12, dutyLt=2
   { 910, 512, 901},  //duty=13, dutyLt=2
   { 915, 512, 895},  //duty=14, dutyLt=2
   { 920, 512, 890},  //duty=15, dutyLt=2
   { 928, 512, 881},  //duty=16, dutyLt=2
   { 935, 512, 873},  //duty=17, dutyLt=2
   { 940, 512, 866},  //duty=18, dutyLt=2
   { 945, 512, 860},  //duty=19, dutyLt=2
   { 949, 512, 854},  //duty=20, dutyLt=2
   { 953, 512, 849},  //duty=21, dutyLt=2
   { 956, 512, 843},  //duty=22, dutyLt=2
   { 959, 512, 839},  //duty=23, dutyLt=2
   { 512, 512, 512},  //duty=24, dutyLt=2
   { 512, 512, 512},  //duty=25, dutyLt=2
   { 524, 512,1788},  //duty=-1, dutyLt=3
   { 613, 512,1366},  //duty=0, dutyLt=3
   { 673, 512,1213},  //duty=1, dutyLt=3
   { 717, 512,1127},  //duty=2, dutyLt=3
   { 749, 512,1076},  //duty=3, dutyLt=3
   { 775, 512,1041},  //duty=4, dutyLt=3
   { 796, 512,1014},  //duty=5, dutyLt=3
   { 813, 512, 996},  //duty=6, dutyLt=3
   { 828, 512, 979},  //duty=7, dutyLt=3
   { 840, 512, 966},  //duty=8, dutyLt=3
   { 850, 512, 954},  //duty=9, dutyLt=3
   { 860, 512, 944},  //duty=10, dutyLt=3
   { 869, 512, 934},  //duty=11, dutyLt=3
   { 876, 512, 927},  //duty=12, dutyLt=3
   { 883, 512, 919},  //duty=13, dutyLt=3
   { 888, 512, 913},  //duty=14, dutyLt=3
   { 894, 512, 907},  //duty=15, dutyLt=3
   { 894, 512, 907},  //duty=16, dutyLt=3
   { 911, 512, 887},  //duty=17, dutyLt=3
   { 918, 512, 879},  //duty=18, dutyLt=3
   { 923, 512, 873},  //duty=19, dutyLt=3
   { 928, 512, 866},  //duty=20, dutyLt=3
   { 933, 512, 860},  //duty=21, dutyLt=3
   { 937, 512, 854},  //duty=22, dutyLt=3
   { 940, 512, 849},  //duty=23, dutyLt=3
   { 512, 512, 512},  //duty=24, dutyLt=3
   { 512, 512, 512},  //duty=25, dutyLt=3
   { 523, 512,1784},  //duty=-1, dutyLt=4
   { 599, 512,1410},  //duty=0, dutyLt=4
   { 653, 512,1253},  //duty=1, dutyLt=4
   { 696, 512,1157},  //duty=2, dutyLt=4
   { 724, 512,1111},  //duty=3, dutyLt=4
   { 749, 512,1071},  //duty=4, dutyLt=4
   { 770, 512,1044},  //duty=5, dutyLt=4
   { 787, 512,1021},  //duty=6, dutyLt=4
   { 803, 512,1002},  //duty=7, dutyLt=4
   { 816, 512, 987},  //duty=8, dutyLt=4
   { 827, 512, 974},  //duty=9, dutyLt=4
   { 836, 512, 963},  //duty=10, dutyLt=4
   { 845, 512, 953},  //duty=11, dutyLt=4
   { 853, 512, 944},  //duty=12, dutyLt=4
   { 860, 512, 936},  //duty=13, dutyLt=4
   { 866, 512, 929},  //duty=14, dutyLt=4
   { 872, 512, 922},  //duty=15, dutyLt=4
   { 883, 512, 910},  //duty=16, dutyLt=4
   { 891, 512, 901},  //duty=17, dutyLt=4
   { 899, 512, 892},  //duty=18, dutyLt=4
   { 905, 512, 883},  //duty=19, dutyLt=4
   { 911, 512, 877},  //duty=20, dutyLt=4
   { 916, 512, 870},  //duty=21, dutyLt=4
   { 920, 512, 864},  //duty=22, dutyLt=4
   { 512, 512, 512},  //duty=23, dutyLt=4
   { 512, 512, 512},  //duty=24, dutyLt=4
   { 512, 512, 512},  //duty=25, dutyLt=4
   { 523, 512,1782},  //duty=-1, dutyLt=5
   { 589, 512,1443},  //duty=0, dutyLt=5
   { 639, 512,1285},  //duty=1, dutyLt=5
   { 675, 512,1196},  //duty=2, dutyLt=5
   { 704, 512,1139},  //duty=3, dutyLt=5
   { 730, 512,1099},  //duty=4, dutyLt=5
   { 750, 512,1070},  //duty=5, dutyLt=5
   { 767, 512,1044},  //duty=6, dutyLt=5
   { 782, 512,1024},  //duty=7, dutyLt=5
   { 794, 512,1007},  //duty=8, dutyLt=5
   { 806, 512, 993},  //duty=9, dutyLt=5
   { 817, 512, 981},  //duty=10, dutyLt=5
   { 825, 512, 970},  //duty=11, dutyLt=5
   { 834, 512, 960},  //duty=12, dutyLt=5
   { 841, 512, 952},  //duty=13, dutyLt=5
   { 847, 512, 943},  //duty=14, dutyLt=5
   { 854, 512, 936},  //duty=15, dutyLt=5
   { 864, 512, 924},  //duty=16, dutyLt=5
   { 874, 512, 913},  //duty=17, dutyLt=5
   { 882, 512, 903},  //duty=18, dutyLt=5
   { 888, 512, 895},  //duty=19, dutyLt=5
   { 895, 512, 887},  //duty=20, dutyLt=5
   { 900, 512, 880},  //duty=21, dutyLt=5
   { 905, 512, 873},  //duty=22, dutyLt=5
   { 512, 512, 512},  //duty=23, dutyLt=5
   { 512, 512, 512},  //duty=24, dutyLt=5
   { 512, 512, 512},  //duty=25, dutyLt=5
   { 523, 512,1779},  //duty=-1, dutyLt=6
   { 582, 512,1468},  //duty=0, dutyLt=6
   { 627, 512,1312},  //duty=1, dutyLt=6
   { 661, 512,1224},  //duty=2, dutyLt=6
   { 689, 512,1166},  //duty=3, dutyLt=6
   { 713, 512,1124},  //duty=4, dutyLt=6
   { 733, 512,1091},  //duty=5, dutyLt=6
   { 749, 512,1065},  //duty=6, dutyLt=6
   { 765, 512,1044},  //duty=7, dutyLt=6
   { 778, 512,1026},  //duty=8, dutyLt=6
   { 789, 512,1011},  //duty=9, dutyLt=6
   { 799, 512, 997},  //duty=10, dutyLt=6
   { 809, 512, 986},  //duty=11, dutyLt=6
   { 817, 512, 975},  //duty=12, dutyLt=6
   { 824, 512, 966},  //duty=13, dutyLt=6
   { 831, 512, 957},  //duty=14, dutyLt=6
   { 838, 512, 949},  //duty=15, dutyLt=6
   { 849, 512, 936},  //duty=16, dutyLt=6
   { 858, 512, 924},  //duty=17, dutyLt=6
   { 867, 512, 914},  //duty=18, dutyLt=6
   { 874, 512, 905},  //duty=19, dutyLt=6
   { 880, 512, 897},  //duty=20, dutyLt=6
   { 886, 512, 889},  //duty=21, dutyLt=6
   { 512, 512, 512},  //duty=22, dutyLt=6
   { 512, 512, 512},  //duty=23, dutyLt=6
   { 512, 512, 512},  //duty=24, dutyLt=6
   { 512, 512, 512},  //duty=25, dutyLt=6
   { 523, 512,1776},  //duty=-1, dutyLt=7
   { 577, 512,1484},  //duty=0, dutyLt=7
   { 618, 512,1335},  //duty=1, dutyLt=7
   { 650, 512,1250},  //duty=2, dutyLt=7
   { 676, 512,1190},  //duty=3, dutyLt=7
   { 699, 512,1145},  //duty=4, dutyLt=7
   { 718, 512,1111},  //duty=5, dutyLt=7
   { 735, 512,1084},  //duty=6, dutyLt=7
   { 750, 512,1062},  //duty=7, dutyLt=7
   { 763, 512,1043},  //duty=8, dutyLt=7
   { 774, 512,1027},  //duty=9, dutyLt=7
   { 784, 512,1013},  //duty=10, dutyLt=7
   { 794, 512,1000},  //duty=11, dutyLt=7
   { 802, 512, 989},  //duty=12, dutyLt=7
   { 810, 512, 979},  //duty=13, dutyLt=7
   { 817, 512, 970},  //duty=14, dutyLt=7
   { 823, 512, 961},  //duty=15, dutyLt=7
   { 835, 512, 947},  //duty=16, dutyLt=7
   { 845, 512, 935},  //duty=17, dutyLt=7
   { 853, 512, 924},  //duty=18, dutyLt=7
   { 861, 512, 914},  //duty=19, dutyLt=7
   { 868, 512, 906},  //duty=20, dutyLt=7
   { 873, 512, 897},  //duty=21, dutyLt=7
   { 512, 512, 512},  //duty=22, dutyLt=7
   { 512, 512, 512},  //duty=23, dutyLt=7
   { 512, 512, 512},  //duty=24, dutyLt=7
   { 512, 512, 512},  //duty=25, dutyLt=7
   { 523, 512,1773},  //duty=-1, dutyLt=8
   { 572, 512,1499},  //duty=0, dutyLt=8
   { 609, 512,1356},  //duty=1, dutyLt=8
   { 640, 512,1271},  //duty=2, dutyLt=8
   { 666, 512,1210},  //duty=3, dutyLt=8
   { 688, 512,1165},  //duty=4, dutyLt=8
   { 706, 512,1130},  //duty=5, dutyLt=8
   { 722, 512,1101},  //duty=6, dutyLt=8
   { 737, 512,1078},  //duty=7, dutyLt=8
   { 749, 512,1058},  //duty=8, dutyLt=8
   { 761, 512,1041},  //duty=9, dutyLt=8
   { 771, 512,1026},  //duty=10, dutyLt=8
   { 781, 512,1014},  //duty=11, dutyLt=8
   { 789, 512,1002},  //duty=12, dutyLt=8
   { 797, 512, 991},  //duty=13, dutyLt=8
   { 804, 512, 982},  //duty=14, dutyLt=8
   { 810, 512, 973},  //duty=15, dutyLt=8
   { 822, 512, 957},  //duty=16, dutyLt=8
   { 833, 512, 945},  //duty=17, dutyLt=8
   { 841, 512, 933},  //duty=18, dutyLt=8
   { 849, 512, 923},  //duty=19, dutyLt=8
   { 856, 512, 914},  //duty=20, dutyLt=8
   { 512, 512, 512},  //duty=21, dutyLt=8
   { 512, 512, 512},  //duty=22, dutyLt=8
   { 512, 512, 512},  //duty=23, dutyLt=8
   { 512, 512, 512},  //duty=24, dutyLt=8
   { 512, 512, 512},  //duty=25, dutyLt=8
   { 523, 512,1766},  //duty=-1, dutyLt=9
   { 568, 512,1515},  //duty=0, dutyLt=9
   { 603, 512,1378},  //duty=1, dutyLt=9
   { 632, 512,1290},  //duty=2, dutyLt=9
   { 657, 512,1229},  //duty=3, dutyLt=9
   { 678, 512,1183},  //duty=4, dutyLt=9
   { 696, 512,1147},  //duty=5, dutyLt=9
   { 712, 512,1117},  //duty=6, dutyLt=9
   { 726, 512,1093},  //duty=7, dutyLt=9
   { 738, 512,1073},  //duty=8, dutyLt=9
   { 749, 512,1055},  //duty=9, dutyLt=9
   { 759, 512,1039},  //duty=10, dutyLt=9
   { 769, 512,1026},  //duty=11, dutyLt=9
   { 777, 512,1014},  //duty=12, dutyLt=9
   { 785, 512,1003},  //duty=13, dutyLt=9
   { 792, 512, 993},  //duty=14, dutyLt=9
   { 799, 512, 984},  //duty=15, dutyLt=9
   { 811, 512, 967},  //duty=16, dutyLt=9
   { 821, 512, 954},  //duty=17, dutyLt=9
   { 831, 512, 942},  //duty=18, dutyLt=9
   { 839, 512, 932},  //duty=19, dutyLt=9
   { 512, 512, 512},  //duty=20, dutyLt=9
   { 512, 512, 512},  //duty=21, dutyLt=9
   { 512, 512, 512},  //duty=22, dutyLt=9
   { 512, 512, 512},  //duty=23, dutyLt=9
   { 512, 512, 512},  //duty=24, dutyLt=9
   { 512, 512, 512},  //duty=25, dutyLt=9
   { 525, 512,1757},  //duty=-1, dutyLt=10
   { 565, 512,1521},  //duty=0, dutyLt=10
   { 598, 512,1394},  //duty=1, dutyLt=10
   { 625, 512,1309},  //duty=2, dutyLt=10
   { 649, 512,1246},  //duty=3, dutyLt=10
   { 669, 512,1200},  //duty=4, dutyLt=10
   { 686, 512,1162},  //duty=5, dutyLt=10
   { 702, 512,1132},  //duty=6, dutyLt=10
   { 716, 512,1108},  //duty=7, dutyLt=10
   { 728, 512,1086},  //duty=8, dutyLt=10
   { 739, 512,1068},  //duty=9, dutyLt=10
   { 749, 512,1052},  //duty=10, dutyLt=10
   { 759, 512,1038},  //duty=11, dutyLt=10
   { 767, 512,1025},  //duty=12, dutyLt=10
   { 775, 512,1013},  //duty=13, dutyLt=10
   { 782, 512,1004},  //duty=14, dutyLt=10
   { 789, 512, 994},  //duty=15, dutyLt=10
   { 802, 512, 977},  //duty=16, dutyLt=10
   { 812, 512, 962},  //duty=17, dutyLt=10
   { 821, 512, 950},  //duty=18, dutyLt=10
   { 512, 512, 512},  //duty=19, dutyLt=10
   { 512, 512, 512},  //duty=20, dutyLt=10
   { 512, 512, 512},  //duty=21, dutyLt=10
   { 512, 512, 512},  //duty=22, dutyLt=10
   { 512, 512, 512},  //duty=23, dutyLt=10
   { 512, 512, 512},  //duty=24, dutyLt=10
   { 512, 512, 512},  //duty=25, dutyLt=10
   { 524, 512,1744},  //duty=-1, dutyLt=11
   { 562, 512,1531},  //duty=0, dutyLt=11
   { 594, 512,1406},  //duty=1, dutyLt=11
   { 619, 512,1323},  //duty=2, dutyLt=11
   { 642, 512,1261},  //duty=3, dutyLt=11
   { 661, 512,1214},  //duty=4, dutyLt=11
   { 678, 512,1177},  //duty=5, dutyLt=11
   { 694, 512,1146},  //duty=6, dutyLt=11
   { 707, 512,1121},  //duty=7, dutyLt=11
   { 719, 512,1098},  //duty=8, dutyLt=11
   { 731, 512,1080},  //duty=9, dutyLt=11
   { 740, 512,1063},  //duty=10, dutyLt=11
   { 750, 512,1049},  //duty=11, dutyLt=11
   { 758, 512,1036},  //duty=12, dutyLt=11
   { 766, 512,1024},  //duty=13, dutyLt=11
   { 773, 512,1013},  //duty=14, dutyLt=11
   { 779, 512,1003},  //duty=15, dutyLt=11
   { 792, 512, 985},  //duty=16, dutyLt=11
   { 803, 512, 971},  //duty=17, dutyLt=11
   { 512, 512, 512},  //duty=18, dutyLt=11
   { 512, 512, 512},  //duty=19, dutyLt=11
   { 512, 512, 512},  //duty=20, dutyLt=11
   { 512, 512, 512},  //duty=21, dutyLt=11
   { 512, 512, 512},  //duty=22, dutyLt=11
   { 512, 512, 512},  //duty=23, dutyLt=11
   { 512, 512, 512},  //duty=24, dutyLt=11
   { 512, 512, 512},  //duty=25, dutyLt=11
   { 524, 512,1743},  //duty=-1, dutyLt=12
   { 560, 512,1542},  //duty=0, dutyLt=12
   { 590, 512,1417},  //duty=1, dutyLt=12
   { 615, 512,1336},  //duty=2, dutyLt=12
   { 636, 512,1275},  //duty=3, dutyLt=12
   { 655, 512,1227},  //duty=4, dutyLt=12
   { 671, 512,1189},  //duty=5, dutyLt=12
   { 686, 512,1158},  //duty=6, dutyLt=12
   { 699, 512,1132},  //duty=7, dutyLt=12
   { 711, 512,1110},  //duty=8, dutyLt=12
   { 722, 512,1091},  //duty=9, dutyLt=12
   { 732, 512,1074},  //duty=10, dutyLt=12
   { 741, 512,1059},  //duty=11, dutyLt=12
   { 749, 512,1046},  //duty=12, dutyLt=12
   { 757, 512,1033},  //duty=13, dutyLt=12
   { 764, 512,1022},  //duty=14, dutyLt=12
   { 771, 512,1012},  //duty=15, dutyLt=12
   { 784, 512, 994},  //duty=16, dutyLt=12
   { 512, 512, 512},  //duty=17, dutyLt=12
   { 512, 512, 512},  //duty=18, dutyLt=12
   { 512, 512, 512},  //duty=19, dutyLt=12
   { 512, 512, 512},  //duty=20, dutyLt=12
   { 512, 512, 512},  //duty=21, dutyLt=12
   { 512, 512, 512},  //duty=22, dutyLt=12
   { 512, 512, 512},  //duty=23, dutyLt=12
   { 512, 512, 512},  //duty=24, dutyLt=12
   { 512, 512, 512},  //duty=25, dutyLt=12
   { 524, 512,1747},  //duty=-1, dutyLt=13
   { 558, 512,1552},  //duty=0, dutyLt=13
   { 586, 512,1429},  //duty=1, dutyLt=13
   { 610, 512,1348},  //duty=2, dutyLt=13
   { 631, 512,1287},  //duty=3, dutyLt=13
   { 649, 512,1239},  //duty=4, dutyLt=13
   { 665, 512,1201},  //duty=5, dutyLt=13
   { 679, 512,1169},  //duty=6, dutyLt=13
   { 693, 512,1142},  //duty=7, dutyLt=13
   { 704, 512,1121},  //duty=8, dutyLt=13
   { 715, 512,1101},  //duty=9, dutyLt=13
   { 725, 512,1084},  //duty=10, dutyLt=13
   { 734, 512,1068},  //duty=11, dutyLt=13
   { 742, 512,1055},  //duty=12, dutyLt=13
   { 750, 512,1042},  //duty=13, dutyLt=13
   { 757, 512,1031},  //duty=14, dutyLt=13
   { 763, 512,1021},  //duty=15, dutyLt=13
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
   { 523, 512,1746},  //duty=-1, dutyLt=14
   { 556, 512,1561},  //duty=0, dutyLt=14
   { 583, 512,1440},  //duty=1, dutyLt=14
   { 606, 512,1358},  //duty=2, dutyLt=14
   { 626, 512,1298},  //duty=3, dutyLt=14
   { 644, 512,1250},  //duty=4, dutyLt=14
   { 659, 512,1211},  //duty=5, dutyLt=14
   { 674, 512,1180},  //duty=6, dutyLt=14
   { 687, 512,1153},  //duty=7, dutyLt=14
   { 698, 512,1130},  //duty=8, dutyLt=14
   { 708, 512,1110},  //duty=9, dutyLt=14
   { 718, 512,1093},  //duty=10, dutyLt=14
   { 727, 512,1076},  //duty=11, dutyLt=14
   { 735, 512,1063},  //duty=12, dutyLt=14
   { 743, 512,1050},  //duty=13, dutyLt=14
   { 750, 512,1039},  //duty=14, dutyLt=14
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
   { 524, 512,1745},  //duty=-1, dutyLt=15
   { 555, 512,1566},  //duty=0, dutyLt=15
   { 581, 512,1449},  //duty=1, dutyLt=15
   { 603, 512,1369},  //duty=2, dutyLt=15
   { 622, 512,1308},  //duty=3, dutyLt=15
   { 639, 512,1260},  //duty=4, dutyLt=15
   { 654, 512,1222},  //duty=5, dutyLt=15
   { 668, 512,1190},  //duty=6, dutyLt=15
   { 681, 512,1162},  //duty=7, dutyLt=15
   { 692, 512,1139},  //duty=8, dutyLt=15
   { 702, 512,1119},  //duty=9, dutyLt=15
   { 712, 512,1102},  //duty=10, dutyLt=15
   { 721, 512,1085},  //duty=11, dutyLt=15
   { 729, 512,1071},  //duty=12, dutyLt=15
   { 737, 512,1058},  //duty=13, dutyLt=15
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
   { 524, 512,1740},  //duty=-1, dutyLt=16
   { 552, 512,1575},  //duty=0, dutyLt=16
   { 577, 512,1462},  //duty=1, dutyLt=16
   { 597, 512,1383},  //duty=2, dutyLt=16
   { 616, 512,1323},  //duty=3, dutyLt=16
   { 632, 512,1276},  //duty=4, dutyLt=16
   { 646, 512,1238},  //duty=5, dutyLt=16
   { 659, 512,1206},  //duty=6, dutyLt=16
   { 671, 512,1179},  //duty=7, dutyLt=16
   { 683, 512,1155},  //duty=8, dutyLt=16
   { 692, 512,1134},  //duty=9, dutyLt=16
   { 702, 512,1116},  //duty=10, dutyLt=16
   { 710, 512,1100},  //duty=11, dutyLt=16
   { 718, 512,1085},  //duty=12, dutyLt=16
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
   { 523, 512,1737},  //duty=-1, dutyLt=17
   { 550, 512,1581},  //duty=0, dutyLt=17
   { 574, 512,1473},  //duty=1, dutyLt=17
   { 593, 512,1396},  //duty=2, dutyLt=17
   { 610, 512,1338},  //duty=3, dutyLt=17
   { 626, 512,1291},  //duty=4, dutyLt=17
   { 639, 512,1253},  //duty=5, dutyLt=17
   { 652, 512,1220},  //duty=6, dutyLt=17
   { 664, 512,1193},  //duty=7, dutyLt=17
   { 674, 512,1169},  //duty=8, dutyLt=17
   { 684, 512,1149},  //duty=9, dutyLt=17
   { 693, 512,1130},  //duty=10, dutyLt=17
   { 701, 512,1113},  //duty=11, dutyLt=17
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
   { 524, 512,1731},  //duty=-1, dutyLt=18
   { 549, 512,1587},  //duty=0, dutyLt=18
   { 570, 512,1482},  //duty=1, dutyLt=18
   { 589, 512,1408},  //duty=2, dutyLt=18
   { 606, 512,1350},  //duty=3, dutyLt=18
   { 620, 512,1304},  //duty=4, dutyLt=18
   { 633, 512,1267},  //duty=5, dutyLt=18
   { 646, 512,1234},  //duty=6, dutyLt=18
   { 657, 512,1206},  //duty=7, dutyLt=18
   { 667, 512,1183},  //duty=8, dutyLt=18
   { 677, 512,1161},  //duty=9, dutyLt=18
   { 685, 512,1143},  //duty=10, dutyLt=18
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
   { 524, 512,1726},  //duty=-1, dutyLt=19
   { 547, 512,1590},  //duty=0, dutyLt=19
   { 568, 512,1488},  //duty=1, dutyLt=19
   { 586, 512,1417},  //duty=2, dutyLt=19
   { 602, 512,1361},  //duty=3, dutyLt=19
   { 616, 512,1316},  //duty=4, dutyLt=19
   { 628, 512,1278},  //duty=5, dutyLt=19
   { 640, 512,1245},  //duty=6, dutyLt=19
   { 651, 512,1218},  //duty=7, dutyLt=19
   { 661, 512,1194},  //duty=8, dutyLt=19
   { 670, 512,1173},  //duty=9, dutyLt=19
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
   { 525, 512,1720},  //duty=-1, dutyLt=20
   { 547, 512,1591},  //duty=0, dutyLt=20
   { 566, 512,1495},  //duty=1, dutyLt=20
   { 583, 512,1427},  //duty=2, dutyLt=20
   { 599, 512,1370},  //duty=3, dutyLt=20
   { 612, 512,1325},  //duty=4, dutyLt=20
   { 624, 512,1287},  //duty=5, dutyLt=20
   { 635, 512,1256},  //duty=6, dutyLt=20
   { 646, 512,1228},  //duty=7, dutyLt=20
   { 656, 512,1204},  //duty=8, dutyLt=20
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
   { 525, 512,1718},  //duty=-1, dutyLt=21
   { 546, 512,1593},  //duty=0, dutyLt=21
   { 564, 512,1504},  //duty=1, dutyLt=21
   { 581, 512,1433},  //duty=2, dutyLt=21
   { 596, 512,1378},  //duty=3, dutyLt=21
   { 609, 512,1334},  //duty=4, dutyLt=21
   { 621, 512,1296},  //duty=5, dutyLt=21
   { 631, 512,1265},  //duty=6, dutyLt=21
   { 642, 512,1237},  //duty=7, dutyLt=21
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
   { 526, 512,1712},  //duty=-1, dutyLt=22
   { 546, 512,1595},  //duty=0, dutyLt=22
   { 564, 512,1505},  //duty=1, dutyLt=22
   { 580, 512,1438},  //duty=2, dutyLt=22
   { 593, 512,1385},  //duty=3, dutyLt=22
   { 606, 512,1341},  //duty=4, dutyLt=22
   { 618, 512,1304},  //duty=5, dutyLt=22
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
   { 526, 512,1707},  //duty=-1, dutyLt=23
   { 545, 512,1595},  //duty=0, dutyLt=23
   { 562, 512,1509},  //duty=1, dutyLt=23
   { 578, 512,1444},  //duty=2, dutyLt=23
   { 592, 512,1391},  //duty=3, dutyLt=23
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
   { 527, 512,1705},  //duty=-1, dutyLt=24
   { 545, 512,1597},  //duty=0, dutyLt=24
   { 562, 512,1511},  //duty=1, dutyLt=24
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
   { 527, 512,1701},  //duty=-1, dutyLt=25
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

