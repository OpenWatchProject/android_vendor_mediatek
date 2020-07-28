const ISP_NVRAM_LCE_TUNING_PARAM_T imx519mipiraw_LCE_%04d = {
    .rAutoLCEParam = {
            .rLCEBasic = {

                {  30,  50,  80, 110 },  // base strength LV (base10)
                { 150, 200, 250, 250 },  // dark strength base ratio
                { 300, 400, 400, 420 },  // bright strength base ratio

               //  LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9   LV10   LV11   LV12   LV13   LV14   LV15   LV16   LV17   LV18
                { 1350,  1350,  1500,  1500,  1600,  1650,  1700,  1750,  1800,  1800,  1800,  1800,  1800,  1800,  1800,  1800,  1800,  1800,  1800 }, // LVTarget
                {  128,   180,   200,   250,   315,   315,   315,   394,   394,   493,   493,   493,   512,   512,   512,   512,   512,   512,   512 }, // MaxLceGain
                { 1600,  1800,  1800,  1800,  1800,  1950,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000,  2000 }  // MaxFinalTarget
        },

        .rLCEStats = {
            20,     //HistCCLB
            1600,   //HistCCUB
            500,    //GCEratio
            1000,   //i4TCPLB
            20000,  //I4TCPUB
            0,      //Reserve
            20,     //i4DetailRangeRatio --> base 1000
            64,     //i4CenSlopeMin

            600,    //DR_Low_Y_Ratio
            800,    //DR_High_Y_Ratio
            1,      //DR_CompEnable
            800,    //DR_Comp_Y_High_Ratio
            2000,   //DR_Diff_Maximum
            1000,   //DR_Diff_Minimum
            500,    //DR_Flat_Ratio_High_Bound

            1,      //multi-slope enable
            0,      //SlopePoint0
            1600,   //SlopePoint1
            2100,   //SlopePoint2
            2800,   //SlopePoint3
            3450,   //SlopePoint4
            51,     //SlopeLow --> 0.2*256

            435,  //SlopeHigh0 --> 1.7*256
            435,  //SlopeHigh1
            435,  //SlopeHigh2
            435,  //SlopeHigh3
            435,  //SlopeHigh4

            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve

        },

        .rLCELUTs = {//i4LCETbl

            {//  /*                                                Dark    Strength                                               */
             //   LV0   LV1   LV2   LV3   LV4   LV5   LV6   LV7   LV8   LV9   LV10  LV11  LV12  LV13  LV14  LV15  LV16  LV17  LV18
                { 128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128,  128 },   //  0 DR index
                { 256,  256,  256,  256,  256,  256,  256,  256,  256,  256,  256,  256,  256,  256,  256,  256,  256,  256,  256 },   //  1
                { 384,  384,  384,  384,  384,  384,  384,  384,  384,  384,  384,  384,  384,  384,  384,  384,  384,  384,  384 },   //  2
                { 512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512 },   //  3
                { 512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512 },   //  4
                { 512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512 },   //  5
                { 512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512 },   //  6
                { 512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512 },   //  7
                { 512,  512,  512,  512,  512,  512,  512,  576,  576,  576,  576,  576,  640,  640,  640,  640,  640,  640,  640 },   //  8
                { 512,  512,  512,  512,  512,  512,  512,  576,  576,  576,  576,  576,  640,  768,  768,  768,  768,  768,  768 },   //  9
                { 512,  512,  512,  512,  512,  512,  512,  576,  576,  576,  576,  576,  640,  768,  768,  768,  768,  768,  768 }    // 10
            },

            {//    /*   Bright    Strength   */
             //    LV0   LV3   LV6   LV9   LV12
                {   0,    0,    0,    0,    0 },   //  0 DR index
                {  64,   64,   64,   64,   64 },   //  1
                { 128,  128,  128,  128,  128 },   //  2
                { 256,  256,  256,  256,  256 },   //  3
                { 384,  384,  384,  384,  384 },   //  4
                { 512,  512,  512,  512,  512 },   //  5
                { 512,  512,  512,  512,  512 },   //  6
                { 512,  512,  512,  512,  512 },   //  7
                { 512,  512,  512,  512,  512 },   //  8
                { 512,  512,  512,  512,  512 },   //  9
                { 512,  512,  512,  512,  512 }    // 10
            },

            { //i4CenSlopeEnhance
              //  0   1   2   3   4   5   6   7   8   9   10   11
                  0,  3,  5, 10, 12, 16, 12, 10,  5,  3,   0,   0
            }

        },

        .rLCEPara = {
            5,    // FlatAreaPercentage
            400,  // FlatRatioTH
            0,    // dark sky protection enable
            300,  // dark sky percentage
            1700, // dark y low bound
            2000, // dark y high bound
            30,   // dark sky lv low bound
            60,   // dark sky lv high bound
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve

            //{//  LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9   LV10   LV11   LV12   LV13   LV14   LV15   LV16   LV17   LV18
                {   50,    50,    60,    60,    60,    60,    80,   100,   120,   120,   120,   120,   120,   120,   120,   120,   120,   120,   120 }, // 0 LVBriRatio
                {  276,   346,   346,   387,   433,   433,   485,   542,   542,   679,   679,   679,   679,   679,   679,   679,   679,   679,   679 }, // 1 LVBriLimit
                {  141,   141,   141,   141,   141,   141,   141,   141,   141,   141,   141,   141,   141,   141,   141,   141,   141,   141,   141 }, // 2 FlatLumaLoBound
                {  679,   679,   679,   679,   679,   679,   679,   679,   679,   679,   679,   679,   679,   679,   679,   679,   679,   679,   679 }, // 3 FlatLumaHiBound
                {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 }, // 4 i4LCEParaTbl0
                {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 }, // 5 i4LCEParaTbl1
                {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 }, // 6 i4LCEParaTbl2
                {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 } // 7 i4LCEParaTbl3
            //}
        },

        .rFaceLCE = {    //i4FaceLCEPara
            1,   //LCE_FD_Enable
            128, //LoBoundGainRatio
            128, //HiBoundGainRatio
            105, //BrightTH
            1600,//DarkFaceLimit
            1600,//FrontLightTH_L
            2400,//FrontLightTH_H
            0,   //dark curve linear blending ratio
            1,   //face multi-slope enable
            435, //SlopeHigh0 -> 1.7*256
            435, //SlopeHigh1
            435, //SlopeHigh2
            435, //SlopeHigh3
            435, //SlopeHigh4
            179, //SlopeLow -> 0.7*256
            900, //Dark face range
            71,  //face para9
            70,  //face para10
            0,   //reserve0
            0,   //reserve1
            0,   //reserve2
            0,   //reserve3
            0,   //reserve4
            0,   //reserve5
            0,   //reserve6
            0,   //reserve7
            0,   //reserve8
            0,   //reserve9

                 // Front lignt gain ratio
                 //LV0  LV2   LV4   LV6   LV8   LV10  LV12  LV14  LV16  rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv
            {     128,  128,  128,  128,  192,  192,  192,  192,  192,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },   //  i4LCEfaceTbl0
            {       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },   //  i4LCEfaceTbl1


            {    //i4CenSlopeEnhance
                 //  0   1   2   3   4   5   6   7   8   9   10   11
                     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,   0
            },

            MTKLCE_FACE_PROTECT_KEEP_LIGHT_MODE, //eFaceLCEProtectMode
            1,   //bFaceLCESmoothLinkEnable
            0,   //u4RobustFaceSlowSpeedCnt
            20,  //i4RobustFaceSmoothSpeed
            10,  //i4RobustFaceSmoothSlowSpeed
            1,   //bNonRobustFaceLockLCE
            3,   //u4FaceLCEStableMaxCnt
            0,   //i4FaceLCEInStableThd
            0    //i4FaceLCEOutStableThd

        },

        .rLCESmooth = {  // rLCESmooth
            1,        // bSmoothEnable
            70,       // i4LCEPosSpeed
            3,        // i4LCENegSpeed
            3,        // i4LCECrosSpeed
            3,        // i4LCESpeed4AEStable
            0,        //reserve
            0,        //reserve
            0,        //reserve
            0,        //reserve
        },

        .rLCECustom = {  // rLCECustom
            0,   // calculate lce current gain enable (for 3rd party HDR)
            128,   // max_lce_gain_ratio
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve
            0,   //reserve

         // rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv   rsv
            { 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },   //  i4ReserveTbl0
            { 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },   //  i4ReserveTbl1
            { 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },   //  i4ReserveTbl2
            { 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },   //  i4ReserveTbl3
        },
    },
};