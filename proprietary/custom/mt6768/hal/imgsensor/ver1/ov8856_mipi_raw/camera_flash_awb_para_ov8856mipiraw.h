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
    { 502, 512, 522},  //duty=-1, dutyLt=-1
    { 997, 512, 781},  //duty=0, dutyLt=-1
    { 993, 512, 774},  //duty=1, dutyLt=-1
    { 991, 512, 771},  //duty=2, dutyLt=-1
    { 990, 512, 767},  //duty=3, dutyLt=-1
    { 990, 512, 764},  //duty=4, dutyLt=-1
    { 990, 512, 762},  //duty=5, dutyLt=-1
    { 990, 512, 757},  //duty=6, dutyLt=-1
    { 991, 512, 745},  //duty=7, dutyLt=-1
    { 992, 512, 744},  //duty=8, dutyLt=-1
    { 992, 512, 741},  //duty=9, dutyLt=-1
    { 991, 512, 739},  //duty=10, dutyLt=-1
    { 991, 512, 737},  //duty=11, dutyLt=-1
    { 991, 512, 735},  //duty=12, dutyLt=-1
    { 991, 512, 733},  //duty=13, dutyLt=-1
    { 990, 512, 731},  //duty=14, dutyLt=-1
    { 991, 512, 729},  //duty=15, dutyLt=-1
    { 990, 512, 726},  //duty=16, dutyLt=-1
    { 990, 512, 723},  //duty=17, dutyLt=-1
    { 990, 512, 720},  //duty=18, dutyLt=-1
    { 990, 512, 717},  //duty=19, dutyLt=-1
    { 990, 512, 714},  //duty=20, dutyLt=-1
    { 989, 512, 711},  //duty=21, dutyLt=-1
    { 990, 512, 708},  //duty=22, dutyLt=-1
    { 990, 512, 705},  //duty=23, dutyLt=-1
    { 989, 512, 703},  //duty=24, dutyLt=-1
    { 990, 512, 700},  //duty=25, dutyLt=-1
    { 991, 512, 698},  //duty=26, dutyLt=-1
    { 991, 512, 696},  //duty=27, dutyLt=-1
    { 991, 512, 693},  //duty=28, dutyLt=-1
    { 992, 512, 691},  //duty=29, dutyLt=-1
    { 543, 512,1369},  //duty=-1, dutyLt=0
    { 759, 512, 930},  //duty=0, dutyLt=0
    { 831, 512, 861},  //duty=1, dutyLt=0
    { 868, 512, 833},  //duty=2, dutyLt=0
    { 890, 512, 816},  //duty=3, dutyLt=0
    { 905, 512, 805},  //duty=4, dutyLt=0
    { 915, 512, 796},  //duty=5, dutyLt=0
    { 924, 512, 778},  //duty=6, dutyLt=0
    { 929, 512, 772},  //duty=7, dutyLt=0
    { 936, 512, 768},  //duty=8, dutyLt=0
    { 940, 512, 764},  //duty=9, dutyLt=0
    { 943, 512, 760},  //duty=10, dutyLt=0
    { 946, 512, 756},  //duty=11, dutyLt=0
    { 948, 512, 754},  //duty=12, dutyLt=0
    { 950, 512, 751},  //duty=13, dutyLt=0
    { 952, 512, 748},  //duty=14, dutyLt=0
    { 954, 512, 745},  //duty=15, dutyLt=0
    { 958, 512, 741},  //duty=16, dutyLt=0
    { 960, 512, 736},  //duty=17, dutyLt=0
    { 961, 512, 733},  //duty=18, dutyLt=0
    { 963, 512, 729},  //duty=19, dutyLt=0
    { 964, 512, 725},  //duty=20, dutyLt=0
    { 965, 512, 722},  //duty=21, dutyLt=0
    { 966, 512, 719},  //duty=22, dutyLt=0
    { 967, 512, 716},  //duty=23, dutyLt=0
    { 968, 512, 713},  //duty=24, dutyLt=0
    { 969, 512, 710},  //duty=25, dutyLt=0
    { 970, 512, 708},  //duty=26, dutyLt=0
    { 970, 512, 705},  //duty=27, dutyLt=0
    { 972, 512, 702},  //duty=28, dutyLt=0
    { 512, 512, 512},  //duty=29, dutyLt=0
    { 541, 512,1365},  //duty=-1, dutyLt=1
    { 683, 512,1016},  //duty=0, dutyLt=1
    { 754, 512, 925},  //duty=1, dutyLt=1
    { 798, 512, 883},  //duty=2, dutyLt=1
    { 826, 512, 857},  //duty=3, dutyLt=1
    { 847, 512, 839},  //duty=4, dutyLt=1
    { 863, 512, 823},  //duty=5, dutyLt=1
    { 875, 512, 805},  //duty=6, dutyLt=1
    { 884, 512, 797},  //duty=7, dutyLt=1
    { 893, 512, 791},  //duty=8, dutyLt=1
    { 899, 512, 785},  //duty=9, dutyLt=1
    { 905, 512, 780},  //duty=10, dutyLt=1
    { 910, 512, 776},  //duty=11, dutyLt=1
    { 914, 512, 771},  //duty=12, dutyLt=1
    { 918, 512, 767},  //duty=13, dutyLt=1
    { 921, 512, 764},  //duty=14, dutyLt=1
    { 924, 512, 761},  //duty=15, dutyLt=1
    { 929, 512, 755},  //duty=16, dutyLt=1
    { 933, 512, 749},  //duty=17, dutyLt=1
    { 936, 512, 745},  //duty=18, dutyLt=1
    { 939, 512, 740},  //duty=19, dutyLt=1
    { 942, 512, 736},  //duty=20, dutyLt=1
    { 943, 512, 732},  //duty=21, dutyLt=1
    { 945, 512, 729},  //duty=22, dutyLt=1
    { 947, 512, 726},  //duty=23, dutyLt=1
    { 948, 512, 722},  //duty=24, dutyLt=1
    { 950, 512, 719},  //duty=25, dutyLt=1
    { 951, 512, 716},  //duty=26, dutyLt=1
    { 952, 512, 713},  //duty=27, dutyLt=1
    { 954, 512, 711},  //duty=28, dutyLt=1
    { 512, 512, 512},  //duty=29, dutyLt=1
    { 539, 512,1361},  //duty=-1, dutyLt=2
    { 646, 512,1071},  //duty=0, dutyLt=2
    { 711, 512, 973},  //duty=1, dutyLt=2
    { 753, 512, 922},  //duty=2, dutyLt=2
    { 783, 512, 890},  //duty=3, dutyLt=2
    { 807, 512, 866},  //duty=4, dutyLt=2
    { 824, 512, 841},  //duty=5, dutyLt=2
    { 838, 512, 829},  //duty=6, dutyLt=2
    { 849, 512, 819},  //duty=7, dutyLt=2
    { 860, 512, 810},  //duty=8, dutyLt=2
    { 868, 512, 804},  //duty=9, dutyLt=2
    { 875, 512, 797},  //duty=10, dutyLt=2
    { 880, 512, 792},  //duty=11, dutyLt=2
    { 886, 512, 787},  //duty=12, dutyLt=2
    { 892, 512, 782},  //duty=13, dutyLt=2
    { 896, 512, 778},  //duty=14, dutyLt=2
    { 899, 512, 774},  //duty=15, dutyLt=2
    { 906, 512, 767},  //duty=16, dutyLt=2
    { 911, 512, 761},  //duty=17, dutyLt=2
    { 915, 512, 755},  //duty=18, dutyLt=2
    { 919, 512, 750},  //duty=19, dutyLt=2
    { 923, 512, 746},  //duty=20, dutyLt=2
    { 925, 512, 742},  //duty=21, dutyLt=2
    { 927, 512, 738},  //duty=22, dutyLt=2
    { 930, 512, 734},  //duty=23, dutyLt=2
    { 932, 512, 731},  //duty=24, dutyLt=2
    { 934, 512, 727},  //duty=25, dutyLt=2
    { 935, 512, 724},  //duty=26, dutyLt=2
    { 937, 512, 721},  //duty=27, dutyLt=2
    { 512, 512, 512},  //duty=28, dutyLt=2
    { 512, 512, 512},  //duty=29, dutyLt=2
    { 538, 512,1358},  //duty=-1, dutyLt=3
    { 625, 512,1111},  //duty=0, dutyLt=3
    { 682, 512,1010},  //duty=1, dutyLt=3
    { 722, 512, 955},  //duty=2, dutyLt=3
    { 752, 512, 918},  //duty=3, dutyLt=3
    { 775, 512, 881},  //duty=4, dutyLt=3
    { 794, 512, 864},  //duty=5, dutyLt=3
    { 809, 512, 851},  //duty=6, dutyLt=3
    { 821, 512, 839},  //duty=7, dutyLt=3
    { 832, 512, 829},  //duty=8, dutyLt=3
    { 842, 512, 820},  //duty=9, dutyLt=3
    { 849, 512, 813},  //duty=10, dutyLt=3
    { 857, 512, 807},  //duty=11, dutyLt=3
    { 862, 512, 801},  //duty=12, dutyLt=3
    { 868, 512, 796},  //duty=13, dutyLt=3
    { 873, 512, 791},  //duty=14, dutyLt=3
    { 877, 512, 786},  //duty=15, dutyLt=3
    { 885, 512, 778},  //duty=16, dutyLt=3
    { 892, 512, 772},  //duty=17, dutyLt=3
    { 897, 512, 765},  //duty=18, dutyLt=3
    { 901, 512, 760},  //duty=19, dutyLt=3
    { 905, 512, 755},  //duty=20, dutyLt=3
    { 909, 512, 750},  //duty=21, dutyLt=3
    { 912, 512, 746},  //duty=22, dutyLt=3
    { 914, 512, 742},  //duty=23, dutyLt=3
    { 917, 512, 738},  //duty=24, dutyLt=3
    { 919, 512, 735},  //duty=25, dutyLt=3
    { 921, 512, 731},  //duty=26, dutyLt=3
    { 923, 512, 728},  //duty=27, dutyLt=3
    { 512, 512, 512},  //duty=28, dutyLt=3
    { 512, 512, 512},  //duty=29, dutyLt=3
    { 538, 512,1356},  //duty=-1, dutyLt=4
    { 611, 512,1138},  //duty=0, dutyLt=4
    { 662, 512,1039},  //duty=1, dutyLt=4
    { 700, 512, 981},  //duty=2, dutyLt=4
    { 730, 512, 939},  //duty=3, dutyLt=4
    { 752, 512, 904},  //duty=4, dutyLt=4
    { 771, 512, 885},  //duty=5, dutyLt=4
    { 786, 512, 869},  //duty=6, dutyLt=4
    { 799, 512, 856},  //duty=7, dutyLt=4
    { 810, 512, 845},  //duty=8, dutyLt=4
    { 820, 512, 836},  //duty=9, dutyLt=4
    { 828, 512, 828},  //duty=10, dutyLt=4
    { 836, 512, 820},  //duty=11, dutyLt=4
    { 843, 512, 814},  //duty=12, dutyLt=4
    { 849, 512, 808},  //duty=13, dutyLt=4
    { 854, 512, 803},  //duty=14, dutyLt=4
    { 859, 512, 798},  //duty=15, dutyLt=4
    { 868, 512, 789},  //duty=16, dutyLt=4
    { 875, 512, 781},  //duty=17, dutyLt=4
    { 881, 512, 775},  //duty=18, dutyLt=4
    { 886, 512, 769},  //duty=19, dutyLt=4
    { 890, 512, 763},  //duty=20, dutyLt=4
    { 894, 512, 758},  //duty=21, dutyLt=4
    { 897, 512, 754},  //duty=22, dutyLt=4
    { 900, 512, 749},  //duty=23, dutyLt=4
    { 903, 512, 745},  //duty=24, dutyLt=4
    { 906, 512, 741},  //duty=25, dutyLt=4
    { 908, 512, 738},  //duty=26, dutyLt=4
    { 512, 512, 512},  //duty=27, dutyLt=4
    { 512, 512, 512},  //duty=28, dutyLt=4
    { 512, 512, 512},  //duty=29, dutyLt=4
    { 537, 512,1354},  //duty=-1, dutyLt=5
    { 601, 512,1159},  //duty=0, dutyLt=5
    { 647, 512,1062},  //duty=1, dutyLt=5
    { 683, 512,1002},  //duty=2, dutyLt=5
    { 711, 512, 951},  //duty=3, dutyLt=5
    { 733, 512, 923},  //duty=4, dutyLt=5
    { 751, 512, 903},  //duty=5, dutyLt=5
    { 766, 512, 885},  //duty=6, dutyLt=5
    { 780, 512, 872},  //duty=7, dutyLt=5
    { 791, 512, 860},  //duty=8, dutyLt=5
    { 802, 512, 850},  //duty=9, dutyLt=5
    { 811, 512, 841},  //duty=10, dutyLt=5
    { 818, 512, 833},  //duty=11, dutyLt=5
    { 825, 512, 826},  //duty=12, dutyLt=5
    { 832, 512, 820},  //duty=13, dutyLt=5
    { 837, 512, 813},  //duty=14, dutyLt=5
    { 842, 512, 808},  //duty=15, dutyLt=5
    { 852, 512, 799},  //duty=16, dutyLt=5
    { 860, 512, 790},  //duty=17, dutyLt=5
    { 866, 512, 783},  //duty=18, dutyLt=5
    { 872, 512, 777},  //duty=19, dutyLt=5
    { 876, 512, 771},  //duty=20, dutyLt=5
    { 881, 512, 766},  //duty=21, dutyLt=5
    { 885, 512, 761},  //duty=22, dutyLt=5
    { 888, 512, 756},  //duty=23, dutyLt=5
    { 891, 512, 752},  //duty=24, dutyLt=5
    { 894, 512, 748},  //duty=25, dutyLt=5
    { 897, 512, 745},  //duty=26, dutyLt=5
    { 512, 512, 512},  //duty=27, dutyLt=5
    { 512, 512, 512},  //duty=28, dutyLt=5
    { 512, 512, 512},  //duty=29, dutyLt=5
    { 537, 512,1351},  //duty=-1, dutyLt=6
    { 593, 512,1176},  //duty=0, dutyLt=6
    { 636, 512,1081},  //duty=1, dutyLt=6
    { 672, 512,1015},  //duty=2, dutyLt=6
    { 696, 512, 969},  //duty=3, dutyLt=6
    { 717, 512, 941},  //duty=4, dutyLt=6
    { 735, 512, 919},  //duty=5, dutyLt=6
    { 750, 512, 901},  //duty=6, dutyLt=6
    { 764, 512, 886},  //duty=7, dutyLt=6
    { 776, 512, 874},  //duty=8, dutyLt=6
    { 786, 512, 863},  //duty=9, dutyLt=6
    { 795, 512, 853},  //duty=10, dutyLt=6
    { 803, 512, 845},  //duty=11, dutyLt=6
    { 810, 512, 837},  //duty=12, dutyLt=6
    { 817, 512, 830},  //duty=13, dutyLt=6
    { 823, 512, 824},  //duty=14, dutyLt=6
    { 828, 512, 819},  //duty=15, dutyLt=6
    { 839, 512, 807},  //duty=16, dutyLt=6
    { 846, 512, 799},  //duty=17, dutyLt=6
    { 853, 512, 791},  //duty=18, dutyLt=6
    { 859, 512, 785},  //duty=19, dutyLt=6
    { 864, 512, 778},  //duty=20, dutyLt=6
    { 869, 512, 773},  //duty=21, dutyLt=6
    { 873, 512, 768},  //duty=22, dutyLt=6
    { 877, 512, 763},  //duty=23, dutyLt=6
    { 880, 512, 758},  //duty=24, dutyLt=6
    { 883, 512, 754},  //duty=25, dutyLt=6
    { 512, 512, 512},  //duty=26, dutyLt=6
    { 512, 512, 512},  //duty=27, dutyLt=6
    { 512, 512, 512},  //duty=28, dutyLt=6
    { 512, 512, 512},  //duty=29, dutyLt=6
    { 537, 512,1349},  //duty=-1, dutyLt=7
    { 588, 512,1188},  //duty=0, dutyLt=7
    { 627, 512,1096},  //duty=1, dutyLt=7
    { 658, 512,1024},  //duty=2, dutyLt=7
    { 684, 512, 985},  //duty=3, dutyLt=7
    { 705, 512, 957},  //duty=4, dutyLt=7
    { 722, 512, 933},  //duty=5, dutyLt=7
    { 737, 512, 914},  //duty=6, dutyLt=7
    { 750, 512, 899},  //duty=7, dutyLt=7
    { 762, 512, 886},  //duty=8, dutyLt=7
    { 772, 512, 874},  //duty=9, dutyLt=7
    { 781, 512, 864},  //duty=10, dutyLt=7
    { 789, 512, 855},  //duty=11, dutyLt=7
    { 797, 512, 847},  //duty=12, dutyLt=7
    { 804, 512, 840},  //duty=13, dutyLt=7
    { 810, 512, 834},  //duty=14, dutyLt=7
    { 815, 512, 827},  //duty=15, dutyLt=7
    { 826, 512, 816},  //duty=16, dutyLt=7
    { 835, 512, 807},  //duty=17, dutyLt=7
    { 842, 512, 799},  //duty=18, dutyLt=7
    { 848, 512, 792},  //duty=19, dutyLt=7
    { 853, 512, 785},  //duty=20, dutyLt=7
    { 858, 512, 779},  //duty=21, dutyLt=7
    { 862, 512, 774},  //duty=22, dutyLt=7
    { 866, 512, 769},  //duty=23, dutyLt=7
    { 870, 512, 764},  //duty=24, dutyLt=7
    { 873, 512, 760},  //duty=25, dutyLt=7
    { 512, 512, 512},  //duty=26, dutyLt=7
    { 512, 512, 512},  //duty=27, dutyLt=7
    { 512, 512, 512},  //duty=28, dutyLt=7
    { 512, 512, 512},  //duty=29, dutyLt=7
    { 537, 512,1347},  //duty=-1, dutyLt=8
    { 583, 512,1198},  //duty=0, dutyLt=8
    { 621, 512,1094},  //duty=1, dutyLt=8
    { 650, 512,1040},  //duty=2, dutyLt=8
    { 674, 512, 998},  //duty=3, dutyLt=8
    { 694, 512, 970},  //duty=4, dutyLt=8
    { 710, 512, 945},  //duty=5, dutyLt=8
    { 726, 512, 927},  //duty=6, dutyLt=8
    { 739, 512, 911},  //duty=7, dutyLt=8
    { 750, 512, 897},  //duty=8, dutyLt=8
    { 760, 512, 885},  //duty=9, dutyLt=8
    { 770, 512, 875},  //duty=10, dutyLt=8
    { 778, 512, 865},  //duty=11, dutyLt=8
    { 785, 512, 857},  //duty=12, dutyLt=8
    { 792, 512, 849},  //duty=13, dutyLt=8
    { 799, 512, 843},  //duty=14, dutyLt=8
    { 804, 512, 836},  //duty=15, dutyLt=8
    { 815, 512, 824},  //duty=16, dutyLt=8
    { 823, 512, 815},  //duty=17, dutyLt=8
    { 831, 512, 806},  //duty=18, dutyLt=8
    { 837, 512, 799},  //duty=19, dutyLt=8
    { 843, 512, 792},  //duty=20, dutyLt=8
    { 848, 512, 786},  //duty=21, dutyLt=8
    { 853, 512, 780},  //duty=22, dutyLt=8
    { 857, 512, 775},  //duty=23, dutyLt=8
    { 860, 512, 770},  //duty=24, dutyLt=8
    { 512, 512, 512},  //duty=25, dutyLt=8
    { 512, 512, 512},  //duty=26, dutyLt=8
    { 512, 512, 512},  //duty=27, dutyLt=8
    { 512, 512, 512},  //duty=28, dutyLt=8
    { 512, 512, 512},  //duty=29, dutyLt=8
    { 537, 512,1345},  //duty=-1, dutyLt=9
    { 580, 512,1205},  //duty=0, dutyLt=9
    { 614, 512,1106},  //duty=1, dutyLt=9
    { 642, 512,1051},  //duty=2, dutyLt=9
    { 665, 512,1011},  //duty=3, dutyLt=9
    { 684, 512, 981},  //duty=4, dutyLt=9
    { 701, 512, 958},  //duty=5, dutyLt=9
    { 716, 512, 939},  //duty=6, dutyLt=9
    { 728, 512, 922},  //duty=7, dutyLt=9
    { 740, 512, 908},  //duty=8, dutyLt=9
    { 750, 512, 895},  //duty=9, dutyLt=9
    { 759, 512, 884},  //duty=10, dutyLt=9
    { 767, 512, 874},  //duty=11, dutyLt=9
    { 775, 512, 866},  //duty=12, dutyLt=9
    { 782, 512, 858},  //duty=13, dutyLt=9
    { 788, 512, 850},  //duty=14, dutyLt=9
    { 794, 512, 844},  //duty=15, dutyLt=9
    { 805, 512, 831},  //duty=16, dutyLt=9
    { 814, 512, 821},  //duty=17, dutyLt=9
    { 821, 512, 813},  //duty=18, dutyLt=9
    { 828, 512, 805},  //duty=19, dutyLt=9
    { 834, 512, 798},  //duty=20, dutyLt=9
    { 839, 512, 791},  //duty=21, dutyLt=9
    { 844, 512, 785},  //duty=22, dutyLt=9
    { 848, 512, 780},  //duty=23, dutyLt=9
    { 852, 512, 775},  //duty=24, dutyLt=9
    { 512, 512, 512},  //duty=25, dutyLt=9
    { 512, 512, 512},  //duty=26, dutyLt=9
    { 512, 512, 512},  //duty=27, dutyLt=9
    { 512, 512, 512},  //duty=28, dutyLt=9
    { 512, 512, 512},  //duty=29, dutyLt=9
    { 537, 512,1343},  //duty=-1, dutyLt=10
    { 578, 512,1207},  //duty=0, dutyLt=10
    { 609, 512,1116},  //duty=1, dutyLt=10
    { 636, 512,1062},  //duty=2, dutyLt=10
    { 658, 512,1023},  //duty=3, dutyLt=10
    { 676, 512, 992},  //duty=4, dutyLt=10
    { 692, 512, 968},  //duty=5, dutyLt=10
    { 707, 512, 948},  //duty=6, dutyLt=10
    { 719, 512, 931},  //duty=7, dutyLt=10
    { 731, 512, 917},  //duty=8, dutyLt=10
    { 741, 512, 904},  //duty=9, dutyLt=10
    { 749, 512, 892},  //duty=10, dutyLt=10
    { 758, 512, 883},  //duty=11, dutyLt=10
    { 766, 512, 874},  //duty=12, dutyLt=10
    { 772, 512, 865},  //duty=13, dutyLt=10
    { 779, 512, 858},  //duty=14, dutyLt=10
    { 785, 512, 851},  //duty=15, dutyLt=10
    { 796, 512, 838},  //duty=16, dutyLt=10
    { 805, 512, 828},  //duty=17, dutyLt=10
    { 812, 512, 819},  //duty=18, dutyLt=10
    { 819, 512, 811},  //duty=19, dutyLt=10
    { 825, 512, 804},  //duty=20, dutyLt=10
    { 831, 512, 797},  //duty=21, dutyLt=10
    { 836, 512, 791},  //duty=22, dutyLt=10
    { 840, 512, 785},  //duty=23, dutyLt=10
    { 512, 512, 512},  //duty=24, dutyLt=10
    { 512, 512, 512},  //duty=25, dutyLt=10
    { 512, 512, 512},  //duty=26, dutyLt=10
    { 512, 512, 512},  //duty=27, dutyLt=10
    { 512, 512, 512},  //duty=28, dutyLt=10
    { 512, 512, 512},  //duty=29, dutyLt=10
    { 537, 512,1340},  //duty=-1, dutyLt=11
    { 575, 512,1200},  //duty=0, dutyLt=11
    { 605, 512,1124},  //duty=1, dutyLt=11
    { 630, 512,1072},  //duty=2, dutyLt=11
    { 651, 512,1032},  //duty=3, dutyLt=11
    { 669, 512,1002},  //duty=4, dutyLt=11
    { 685, 512, 977},  //duty=5, dutyLt=11
    { 699, 512, 958},  //duty=6, dutyLt=11
    { 711, 512, 940},  //duty=7, dutyLt=11
    { 722, 512, 926},  //duty=8, dutyLt=11
    { 733, 512, 912},  //duty=9, dutyLt=11
    { 741, 512, 901},  //duty=10, dutyLt=11
    { 750, 512, 891},  //duty=11, dutyLt=11
    { 757, 512, 881},  //duty=12, dutyLt=11
    { 764, 512, 873},  //duty=13, dutyLt=11
    { 770, 512, 865},  //duty=14, dutyLt=11
    { 776, 512, 859},  //duty=15, dutyLt=11
    { 787, 512, 845},  //duty=16, dutyLt=11
    { 797, 512, 834},  //duty=17, dutyLt=11
    { 804, 512, 825},  //duty=18, dutyLt=11
    { 811, 512, 816},  //duty=19, dutyLt=11
    { 817, 512, 809},  //duty=20, dutyLt=11
    { 823, 512, 802},  //duty=21, dutyLt=11
    { 828, 512, 796},  //duty=22, dutyLt=11
    { 832, 512, 791},  //duty=23, dutyLt=11
    { 512, 512, 512},  //duty=24, dutyLt=11
    { 512, 512, 512},  //duty=25, dutyLt=11
    { 512, 512, 512},  //duty=26, dutyLt=11
    { 512, 512, 512},  //duty=27, dutyLt=11
    { 512, 512, 512},  //duty=28, dutyLt=11
    { 512, 512, 512},  //duty=29, dutyLt=11
    { 538, 512,1336},  //duty=-1, dutyLt=12
    { 573, 512,1205},  //duty=0, dutyLt=12
    { 602, 512,1132},  //duty=1, dutyLt=12
    { 626, 512,1081},  //duty=2, dutyLt=12
    { 646, 512,1042},  //duty=3, dutyLt=12
    { 663, 512,1011},  //duty=4, dutyLt=12
    { 678, 512, 986},  //duty=5, dutyLt=12
    { 692, 512, 967},  //duty=6, dutyLt=12
    { 704, 512, 948},  //duty=7, dutyLt=12
    { 715, 512, 933},  //duty=8, dutyLt=12
    { 725, 512, 920},  //duty=9, dutyLt=12
    { 734, 512, 909},  //duty=10, dutyLt=12
    { 742, 512, 898},  //duty=11, dutyLt=12
    { 750, 512, 889},  //duty=12, dutyLt=12
    { 756, 512, 880},  //duty=13, dutyLt=12
    { 763, 512, 872},  //duty=14, dutyLt=12
    { 768, 512, 865},  //duty=15, dutyLt=12
    { 780, 512, 851},  //duty=16, dutyLt=12
    { 789, 512, 840},  //duty=17, dutyLt=12
    { 797, 512, 831},  //duty=18, dutyLt=12
    { 804, 512, 822},  //duty=19, dutyLt=12
    { 810, 512, 814},  //duty=20, dutyLt=12
    { 816, 512, 808},  //duty=21, dutyLt=12
    { 821, 512, 801},  //duty=22, dutyLt=12
    { 512, 512, 512},  //duty=23, dutyLt=12
    { 512, 512, 512},  //duty=24, dutyLt=12
    { 512, 512, 512},  //duty=25, dutyLt=12
    { 512, 512, 512},  //duty=26, dutyLt=12
    { 512, 512, 512},  //duty=27, dutyLt=12
    { 512, 512, 512},  //duty=28, dutyLt=12
    { 512, 512, 512},  //duty=29, dutyLt=12
    { 538, 512,1316},  //duty=-1, dutyLt=13
    { 571, 512,1210},  //duty=0, dutyLt=13
    { 598, 512,1140},  //duty=1, dutyLt=13
    { 621, 512,1088},  //duty=2, dutyLt=13
    { 641, 512,1050},  //duty=3, dutyLt=13
    { 658, 512,1020},  //duty=4, dutyLt=13
    { 672, 512, 995},  //duty=5, dutyLt=13
    { 686, 512, 974},  //duty=6, dutyLt=13
    { 698, 512, 957},  //duty=7, dutyLt=13
    { 708, 512, 941},  //duty=8, dutyLt=13
    { 718, 512, 928},  //duty=9, dutyLt=13
    { 727, 512, 916},  //duty=10, dutyLt=13
    { 735, 512, 905},  //duty=11, dutyLt=13
    { 743, 512, 895},  //duty=12, dutyLt=13
    { 750, 512, 886},  //duty=13, dutyLt=13
    { 756, 512, 878},  //duty=14, dutyLt=13
    { 762, 512, 871},  //duty=15, dutyLt=13
    { 773, 512, 857},  //duty=16, dutyLt=13
    { 782, 512, 846},  //duty=17, dutyLt=13
    { 790, 512, 836},  //duty=18, dutyLt=13
    { 797, 512, 827},  //duty=19, dutyLt=13
    { 803, 512, 820},  //duty=20, dutyLt=13
    { 809, 512, 812},  //duty=21, dutyLt=13
    { 814, 512, 806},  //duty=22, dutyLt=13
    { 512, 512, 512},  //duty=23, dutyLt=13
    { 512, 512, 512},  //duty=24, dutyLt=13
    { 512, 512, 512},  //duty=25, dutyLt=13
    { 512, 512, 512},  //duty=26, dutyLt=13
    { 512, 512, 512},  //duty=27, dutyLt=13
    { 512, 512, 512},  //duty=28, dutyLt=13
    { 512, 512, 512},  //duty=29, dutyLt=13
    { 538, 512,1314},  //duty=-1, dutyLt=14
    { 570, 512,1214},  //duty=0, dutyLt=14
    { 595, 512,1145},  //duty=1, dutyLt=14
    { 618, 512,1094},  //duty=2, dutyLt=14
    { 637, 512,1058},  //duty=3, dutyLt=14
    { 653, 512,1027},  //duty=4, dutyLt=14
    { 667, 512,1002},  //duty=5, dutyLt=14
    { 680, 512, 981},  //duty=6, dutyLt=14
    { 692, 512, 963},  //duty=7, dutyLt=14
    { 703, 512, 948},  //duty=8, dutyLt=14
    { 712, 512, 934},  //duty=9, dutyLt=14
    { 721, 512, 922},  //duty=10, dutyLt=14
    { 729, 512, 912},  //duty=11, dutyLt=14
    { 736, 512, 902},  //duty=12, dutyLt=14
    { 743, 512, 892},  //duty=13, dutyLt=14
    { 749, 512, 884},  //duty=14, dutyLt=14
    { 755, 512, 877},  //duty=15, dutyLt=14
    { 767, 512, 862},  //duty=16, dutyLt=14
    { 776, 512, 851},  //duty=17, dutyLt=14
    { 784, 512, 841},  //duty=18, dutyLt=14
    { 791, 512, 832},  //duty=19, dutyLt=14
    { 797, 512, 824},  //duty=20, dutyLt=14
    { 803, 512, 817},  //duty=21, dutyLt=14
    { 512, 512, 512},  //duty=22, dutyLt=14
    { 512, 512, 512},  //duty=23, dutyLt=14
    { 512, 512, 512},  //duty=24, dutyLt=14
    { 512, 512, 512},  //duty=25, dutyLt=14
    { 512, 512, 512},  //duty=26, dutyLt=14
    { 512, 512, 512},  //duty=27, dutyLt=14
    { 512, 512, 512},  //duty=28, dutyLt=14
    { 512, 512, 512},  //duty=29, dutyLt=14
    { 538, 512,1312},  //duty=-1, dutyLt=15
    { 568, 512,1217},  //duty=0, dutyLt=15
    { 593, 512,1150},  //duty=1, dutyLt=15
    { 614, 512,1101},  //duty=2, dutyLt=15
    { 633, 512,1063},  //duty=3, dutyLt=15
    { 649, 512,1033},  //duty=4, dutyLt=15
    { 662, 512,1009},  //duty=5, dutyLt=15
    { 675, 512, 988},  //duty=6, dutyLt=15
    { 687, 512, 970},  //duty=7, dutyLt=15
    { 697, 512, 954},  //duty=8, dutyLt=15
    { 707, 512, 940},  //duty=9, dutyLt=15
    { 715, 512, 928},  //duty=10, dutyLt=15
    { 723, 512, 917},  //duty=11, dutyLt=15
    { 730, 512, 907},  //duty=12, dutyLt=15
    { 737, 512, 898},  //duty=13, dutyLt=15
    { 744, 512, 890},  //duty=14, dutyLt=15
    { 749, 512, 882},  //duty=15, dutyLt=15
    { 761, 512, 868},  //duty=16, dutyLt=15
    { 770, 512, 856},  //duty=17, dutyLt=15
    { 778, 512, 846},  //duty=18, dutyLt=15
    { 785, 512, 837},  //duty=19, dutyLt=15
    { 791, 512, 828},  //duty=20, dutyLt=15
    { 797, 512, 821},  //duty=21, dutyLt=15
    { 512, 512, 512},  //duty=22, dutyLt=15
    { 512, 512, 512},  //duty=23, dutyLt=15
    { 512, 512, 512},  //duty=24, dutyLt=15
    { 512, 512, 512},  //duty=25, dutyLt=15
    { 512, 512, 512},  //duty=26, dutyLt=15
    { 512, 512, 512},  //duty=27, dutyLt=15
    { 512, 512, 512},  //duty=28, dutyLt=15
    { 512, 512, 512},  //duty=29, dutyLt=15
    { 538, 512,1311},  //duty=-1, dutyLt=16
    { 566, 512,1223},  //duty=0, dutyLt=16
    { 590, 512,1158},  //duty=1, dutyLt=16
    { 610, 512,1110},  //duty=2, dutyLt=16
    { 627, 512,1073},  //duty=3, dutyLt=16
    { 642, 512,1043},  //duty=4, dutyLt=16
    { 656, 512,1019},  //duty=5, dutyLt=16
    { 668, 512, 998},  //duty=6, dutyLt=16
    { 679, 512, 980},  //duty=7, dutyLt=16
    { 689, 512, 964},  //duty=8, dutyLt=16
    { 698, 512, 950},  //duty=9, dutyLt=16
    { 706, 512, 938},  //duty=10, dutyLt=16
    { 714, 512, 927},  //duty=11, dutyLt=16
    { 721, 512, 917},  //duty=12, dutyLt=16
    { 728, 512, 908},  //duty=13, dutyLt=16
    { 734, 512, 899},  //duty=14, dutyLt=16
    { 740, 512, 892},  //duty=15, dutyLt=16
    { 750, 512, 877},  //duty=16, dutyLt=16
    { 759, 512, 865},  //duty=17, dutyLt=16
    { 768, 512, 855},  //duty=18, dutyLt=16
    { 775, 512, 845},  //duty=19, dutyLt=16
    { 781, 512, 837},  //duty=20, dutyLt=16
    { 512, 512, 512},  //duty=21, dutyLt=16
    { 512, 512, 512},  //duty=22, dutyLt=16
    { 512, 512, 512},  //duty=23, dutyLt=16
    { 512, 512, 512},  //duty=24, dutyLt=16
    { 512, 512, 512},  //duty=25, dutyLt=16
    { 512, 512, 512},  //duty=26, dutyLt=16
    { 512, 512, 512},  //duty=27, dutyLt=16
    { 512, 512, 512},  //duty=28, dutyLt=16
    { 512, 512, 512},  //duty=29, dutyLt=16
    { 539, 512,1308},  //duty=-1, dutyLt=17
    { 564, 512,1226},  //duty=0, dutyLt=17
    { 586, 512,1165},  //duty=1, dutyLt=17
    { 605, 512,1118},  //duty=2, dutyLt=17
    { 622, 512,1083},  //duty=3, dutyLt=17
    { 636, 512,1053},  //duty=4, dutyLt=17
    { 649, 512,1028},  //duty=5, dutyLt=17
    { 661, 512,1008},  //duty=6, dutyLt=17
    { 672, 512, 990},  //duty=7, dutyLt=17
    { 681, 512, 974},  //duty=8, dutyLt=17
    { 690, 512, 960},  //duty=9, dutyLt=17
    { 698, 512, 948},  //duty=10, dutyLt=17
    { 706, 512, 936},  //duty=11, dutyLt=17
    { 713, 512, 926},  //duty=12, dutyLt=17
    { 719, 512, 917},  //duty=13, dutyLt=17
    { 725, 512, 908},  //duty=14, dutyLt=17
    { 731, 512, 900},  //duty=15, dutyLt=17
    { 741, 512, 886},  //duty=16, dutyLt=17
    { 750, 512, 874},  //duty=17, dutyLt=17
    { 758, 512, 863},  //duty=18, dutyLt=17
    { 765, 512, 853},  //duty=19, dutyLt=17
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
    { 539, 512,1307},  //duty=-1, dutyLt=18
    { 563, 512,1229},  //duty=0, dutyLt=18
    { 584, 512,1169},  //duty=1, dutyLt=18
    { 601, 512,1126},  //duty=2, dutyLt=18
    { 617, 512,1090},  //duty=3, dutyLt=18
    { 631, 512,1062},  //duty=4, dutyLt=18
    { 644, 512,1037},  //duty=5, dutyLt=18
    { 655, 512,1017},  //duty=6, dutyLt=18
    { 665, 512, 999},  //duty=7, dutyLt=18
    { 675, 512, 983},  //duty=8, dutyLt=18
    { 683, 512, 969},  //duty=9, dutyLt=18
    { 691, 512, 956},  //duty=10, dutyLt=18
    { 698, 512, 945},  //duty=11, dutyLt=18
    { 705, 512, 935},  //duty=12, dutyLt=18
    { 712, 512, 925},  //duty=13, dutyLt=18
    { 717, 512, 916},  //duty=14, dutyLt=18
    { 723, 512, 908},  //duty=15, dutyLt=18
    { 733, 512, 894},  //duty=16, dutyLt=18
    { 742, 512, 881},  //duty=17, dutyLt=18
    { 750, 512, 870},  //duty=18, dutyLt=18
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
    { 540, 512,1303},  //duty=-1, dutyLt=19
    { 562, 512,1231},  //duty=0, dutyLt=19
    { 582, 512,1174},  //duty=1, dutyLt=19
    { 599, 512,1131},  //duty=2, dutyLt=19
    { 613, 512,1097},  //duty=3, dutyLt=19
    { 627, 512,1069},  //duty=4, dutyLt=19
    { 639, 512,1045},  //duty=5, dutyLt=19
    { 650, 512,1024},  //duty=6, dutyLt=19
    { 660, 512,1006},  //duty=7, dutyLt=19
    { 669, 512, 990},  //duty=8, dutyLt=19
    { 677, 512, 976},  //duty=9, dutyLt=19
    { 685, 512, 964},  //duty=10, dutyLt=19
    { 692, 512, 952},  //duty=11, dutyLt=19
    { 699, 512, 942},  //duty=12, dutyLt=19
    { 705, 512, 932},  //duty=13, dutyLt=19
    { 711, 512, 923},  //duty=14, dutyLt=19
    { 716, 512, 915},  //duty=15, dutyLt=19
    { 727, 512, 901},  //duty=16, dutyLt=19
    { 736, 512, 887},  //duty=17, dutyLt=19
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
    { 540, 512,1301},  //duty=-1, dutyLt=20
    { 561, 512,1233},  //duty=0, dutyLt=20
    { 580, 512,1177},  //duty=1, dutyLt=20
    { 596, 512,1136},  //duty=2, dutyLt=20
    { 611, 512,1103},  //duty=3, dutyLt=20
    { 623, 512,1075},  //duty=4, dutyLt=20
    { 635, 512,1051},  //duty=5, dutyLt=20
    { 645, 512,1031},  //duty=6, dutyLt=20
    { 655, 512,1012},  //duty=7, dutyLt=20
    { 664, 512, 997},  //duty=8, dutyLt=20
    { 672, 512, 983},  //duty=9, dutyLt=20
    { 680, 512, 970},  //duty=10, dutyLt=20
    { 687, 512, 959},  //duty=11, dutyLt=20
    { 694, 512, 948},  //duty=12, dutyLt=20
    { 700, 512, 939},  //duty=13, dutyLt=20
    { 705, 512, 930},  //duty=14, dutyLt=20
    { 711, 512, 921},  //duty=15, dutyLt=20
    { 721, 512, 907},  //duty=16, dutyLt=20
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
    { 541, 512,1297},  //duty=-1, dutyLt=21
    { 560, 512,1233},  //duty=0, dutyLt=21
    { 579, 512,1180},  //duty=1, dutyLt=21
    { 594, 512,1140},  //duty=2, dutyLt=21
    { 608, 512,1107},  //duty=3, dutyLt=21
    { 620, 512,1080},  //duty=4, dutyLt=21
    { 631, 512,1056},  //duty=5, dutyLt=21
    { 642, 512,1036},  //duty=6, dutyLt=21
    { 651, 512,1018},  //duty=7, dutyLt=21
    { 660, 512,1003},  //duty=8, dutyLt=21
    { 668, 512, 989},  //duty=9, dutyLt=21
    { 675, 512, 976},  //duty=10, dutyLt=21
    { 682, 512, 964},  //duty=11, dutyLt=21
    { 689, 512, 954},  //duty=12, dutyLt=21
    { 695, 512, 944},  //duty=13, dutyLt=21
    { 700, 512, 936},  //duty=14, dutyLt=21
    { 706, 512, 927},  //duty=15, dutyLt=21
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
    { 541, 512,1295},  //duty=-1, dutyLt=22
    { 560, 512,1233},  //duty=0, dutyLt=22
    { 577, 512,1183},  //duty=1, dutyLt=22
    { 593, 512,1143},  //duty=2, dutyLt=22
    { 606, 512,1111},  //duty=3, dutyLt=22
    { 618, 512,1084},  //duty=4, dutyLt=22
    { 629, 512,1061},  //duty=5, dutyLt=22
    { 639, 512,1041},  //duty=6, dutyLt=22
    { 648, 512,1024},  //duty=7, dutyLt=22
    { 656, 512,1007},  //duty=8, dutyLt=22
    { 664, 512, 993},  //duty=9, dutyLt=22
    { 671, 512, 981},  //duty=10, dutyLt=22
    { 678, 512, 970},  //duty=11, dutyLt=22
    { 684, 512, 959},  //duty=12, dutyLt=22
    { 690, 512, 950},  //duty=13, dutyLt=22
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
    { 542, 512,1292},  //duty=-1, dutyLt=23
    { 560, 512,1233},  //duty=0, dutyLt=23
    { 577, 512,1184},  //duty=1, dutyLt=23
    { 591, 512,1146},  //duty=2, dutyLt=23
    { 604, 512,1115},  //duty=3, dutyLt=23
    { 616, 512,1088},  //duty=4, dutyLt=23
    { 626, 512,1065},  //duty=5, dutyLt=23
    { 636, 512,1046},  //duty=6, dutyLt=23
    { 645, 512,1027},  //duty=7, dutyLt=23
    { 654, 512,1012},  //duty=8, dutyLt=23
    { 661, 512, 998},  //duty=9, dutyLt=23
    { 668, 512, 985},  //duty=10, dutyLt=23
    { 675, 512, 974},  //duty=11, dutyLt=23
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
    { 542, 512,1290},  //duty=-1, dutyLt=24
    { 560, 512,1233},  //duty=0, dutyLt=24
    { 576, 512,1185},  //duty=1, dutyLt=24
    { 590, 512,1148},  //duty=2, dutyLt=24
    { 602, 512,1117},  //duty=3, dutyLt=24
    { 614, 512,1090},  //duty=4, dutyLt=24
    { 624, 512,1069},  //duty=5, dutyLt=24
    { 634, 512,1049},  //duty=6, dutyLt=24
    { 643, 512,1031},  //duty=7, dutyLt=24
    { 651, 512,1016},  //duty=8, dutyLt=24
    { 658, 512,1002},  //duty=9, dutyLt=24
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
    { 543, 512,1287},  //duty=-1, dutyLt=25
    { 560, 512,1232},  //duty=0, dutyLt=25
    { 575, 512,1186},  //duty=1, dutyLt=25
    { 589, 512,1149},  //duty=2, dutyLt=25
    { 601, 512,1120},  //duty=3, dutyLt=25
    { 613, 512,1093},  //duty=4, dutyLt=25
    { 623, 512,1071},  //duty=5, dutyLt=25
    { 632, 512,1052},  //duty=6, dutyLt=25
    { 641, 512,1035},  //duty=7, dutyLt=25
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
    { 543, 512,1285},  //duty=-1, dutyLt=26
    { 560, 512,1231},  //duty=0, dutyLt=26
    { 575, 512,1186},  //duty=1, dutyLt=26
    { 589, 512,1150},  //duty=2, dutyLt=26
    { 600, 512,1121},  //duty=3, dutyLt=26
    { 612, 512,1095},  //duty=4, dutyLt=26
    { 621, 512,1073},  //duty=5, dutyLt=26
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
    { 544, 512,1282},  //duty=-1, dutyLt=27
    { 560, 512,1230},  //duty=0, dutyLt=27
    { 575, 512,1186},  //duty=1, dutyLt=27
    { 588, 512,1151},  //duty=2, dutyLt=27
    { 600, 512,1122},  //duty=3, dutyLt=27
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
    { 545, 512,1279},  //duty=-1, dutyLt=28
    { 561, 512,1229},  //duty=0, dutyLt=28
    { 575, 512,1186},  //duty=1, dutyLt=28
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
    { 546, 512,1277},  //duty=-1, dutyLt=29
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

