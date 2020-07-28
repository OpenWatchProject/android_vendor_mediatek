


#include "camera_custom_types.h"
#include "af_param.h"
#include "af_tuning_custom.h"

/*******************************************************************************
*
********************************************************************************/
AF_PARAM_T const&
getAFParam()
{
    static AF_PARAM_T g_AFparam =
    {
        2,   // i4AFS_MODE  0 : singleAF, 1:smoothAF      2:smoothAF + 3 points curve fitting.
        2,   // i4AFC_MODE  0 : singleAF, 1:smoothAF      2:smoothAF + 3 points curve fitting.
        2,   // i4VAFC_MODE  0 : singleAF, 1:smoothAF     2:smoothAF + 3 points curve fitting.
        1,   // i4ReadOTP 0 : disable, 1:enable
        5,   // i4FD_DETECT_CNT
        5,   // i4FD_NONE_CNT

        50,  // i4FV_SHOCK_THRES
        30000,  // i4FV_SHOCK_OFFSET
        5,   // i4FV_VALID_CNT 2
        20, // i4FV_SHOCK_FRM_CNT 100
        5,    // i4FV_SHOCK_CNT 2

        9, //i4TBLL
        {50, 100,150,200,400,800,1200,2000,3000,0,0,0,0,0,0}, //i4Dist
        {530,480,420,380,350,320, 300, 200, 195,0,0,0,0,0,0}, //i4Dacv
        { 10, 20, 40, 80,120,200, 250, 300, 350,0,0,0,0,0,0}, //i4Dacv
    };

    return g_AFparam;
}

AF_CONFIG_T const&
getAFConfig()
{
    static AF_CONFIG_T g_AFconfig;

    g_AFconfig.i4SGG_GAIN = 16;
    g_AFconfig.i4SGG_GMR1 = 31;
    g_AFconfig.i4SGG_GMR2 = 63;
    g_AFconfig.i4SGG_GMR3 = 127;
    g_AFconfig.AF_DECI_1  = 0;
    g_AFconfig.AF_ZIGZAG  = 0;
    g_AFconfig.AF_ODD     = 0;
    g_AFconfig.AF_FILT1[0] = 15;
    g_AFconfig.AF_FILT1[1] = 36;
    g_AFconfig.AF_FILT1[2] = 43;
    g_AFconfig.AF_FILT1[3] = 36;
    g_AFconfig.AF_FILT1[4] = 22;
    g_AFconfig.AF_FILT1[5] = 10;
    g_AFconfig.AF_FILT1[6] = 3;
    g_AFconfig.AF_FILT1[7] = 0;
    g_AFconfig.AF_FILT1[8] = 0;
    g_AFconfig.AF_FILT1[9] = 0;
    g_AFconfig.AF_FILT1[10] = 0;
    g_AFconfig.AF_FILT1[11] = 0;

    g_AFconfig.AF_FILT2[0] =0x100;
    g_AFconfig.AF_FILT2[1] =0x100;
    g_AFconfig.AF_FILT2[2] =0x700;
    g_AFconfig.AF_FILT2[3] =0x700;

    g_AFconfig.AF_TH[0] = 5;
    g_AFconfig.AF_TH[1] = 5;

    g_AFconfig.AF_THEX = 5;

    return g_AFconfig;
}

//////////////////////////////////////////////////
///
//   For multi sensor mode, extend lens_para_xxxx.cpp coef.
//   AF threshold setting for different sensor resolution
//
//   If current sensor output size match to one of this table,
//   AF will use the coef of the entry to do AF.
//   or, AF will use coef. of lens_para_xxxxx.cpp
//
//   input param. :
//     w: sensor mode output image width
//     h: sensor mode output image height
//     SensorID:  current sensor ID,
//
//     The mapping table for sensor ID can be found in kd_imgsensor.h (custom\common\kernel\imgsensor\inc)
//     for example,
//         #define OV8825_SENSOR_ID  0x8825
//         and the SensorID = 0x8825
//
/////////////////////////////////////////////////////////
#ifdef AFEXTENDCOEF
CustAF_THRES_T const& getAFExtendCoef(MINT32 w, MINT32 h, MINT32 SensorId)
{
    static AF_Extend_Coef_T AFTH_ExtCoef[]=
    {
        {//set1   //example
            0,0, 0, //320,240, 0x5648//resolution, SensorId
            {
                8, // i4ISONum;
                {100,150,200,300,400,600,800,1600}, // i4ISO[ISO_MAX_NUM];
                // SGG1~7
                {20, 19, 19, 18, 18, 17, 16, 15,
                 29, 29, 28, 28, 27, 26, 26, 24,
                 43, 42, 42, 41, 41, 40, 39, 37,
                 62, 61, 61, 60, 60, 59, 58, 56,
                 88, 88, 88, 87, 87, 86, 85, 84,
                 126, 126, 126, 125, 125, 125, 124, 123,
                 180, 180, 180, 179, 179, 179, 179, 178},
                {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
                {50000,50000,50000,50000,50000,50000,50000,50000}, // i4MIN_TH[ISO_MAX_NUM];
                {5,5,5,5,5,5,5,5}, // i4HW_TH[ISO_MAX_NUM];
                {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
                {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
                {5,5,5,5,5,5,5,5}  // i4HW_TH2[ISO_MAX_NUM];
            },
        },

        {//set2   //example
            0,0, 0,//800,600, 0x8825//resolution, SensorId
            {
                8, // i4ISONum;
                {101,151,201,300,400,600,800,1600}, // i4ISO[ISO_MAX_NUM];
                // SGG1~7
                {20, 19, 19, 18, 18, 17, 16, 15,
                 29, 29, 28, 28, 27, 26, 26, 24,
                 43, 42, 42, 41, 41, 40, 39, 37,
                 62, 61, 61, 60, 60, 59, 58, 56,
                 88, 88, 88, 87, 87, 86, 85, 84,
                 126, 126, 126, 125, 125, 125, 124, 123,
                 180, 180, 180, 179, 179, 179, 179, 178},
                {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
                {998888,998888,998888,998888,998888,998888,998888,998888}, // i4MIN_TH[ISO_MAX_NUM];
                {5,5,5,5,5,5,5,5}, // i4HW_TH[ISO_MAX_NUM];
                {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
                {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
                {5,5,5,5,5,5,5,5}  // i4HW_TH2[ISO_MAX_NUM];
            },
        },
        //..... increase set below if need.
    };

    MINT32 coefidx=0;
    for(coefidx=0;coefidx< sizeof(AFTH_ExtCoef)/sizeof(AF_Extend_Coef_T);coefidx++)
    {
        if((AFTH_ExtCoef[coefidx].i4ImageWidth==w)
         &&(AFTH_ExtCoef[coefidx].i4ImageHeight==h)
         &&(AFTH_ExtCoef[coefidx].i4SensorID==SensorId)
         && (w>0) && (h>0))
            return AFTH_ExtCoef[coefidx].Coef;
    }
    AFTH_ExtCoef[0].Coef.i4ISONum=0;
    return AFTH_ExtCoef[0].Coef;
}
#endif

