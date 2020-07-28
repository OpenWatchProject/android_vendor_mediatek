#define LOG_TAG "flash_tuning_custom_cct.cpp"
#include "string.h"
#include "camera_custom_nvram.h"
#include "camera_custom_types.h"
#include "camera_custom_AEPlinetable.h"
#include <cutils/log.h>
#include "flash_feature.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <kd_camera_feature.h>

//==============================================================================
//
//==============================================================================
int cust_fillDefaultStrobeNVRam_default2 (void* data)
{
    int i;
    NVRAM_CAMERA_STROBE_STRUCT* p;
    p = (NVRAM_CAMERA_STROBE_STRUCT*)data;

    static short engTab[]=
    {

        -1, 303, 654, 976,1284,1585,1874,2167,2445,2714,2976,3241,3478,3744,3986,4224,4454,4853,5271,5667,6045,6404,6739,7048,7345,7612,7869,8108,8310,
        -1, 594, 923,1245,1554,1853,2145,2426,2712,2982,3251,3489,3766,4014,4251,4496,4729,5127,5548,5951,6325,6688,7028,7340,7647,7915,8176,8415,8639,
        -1, 861,1194,1514,1820,2121,2415,2702,2971,3245,3518,3779,4031,4268,4523,4761,4986,5398,5819,6216,6597,6954,7289,7607,7911,8179,8435,8676,8890,
        -1,1118,1453,1772,2080,2381,2674,2953,3229,3509,3774,4035,4285,4500,4756,5011,5240,5643,6063,6462,6834,7192,7534,7837,8139,8407,8663,8912,9121,
        -1,1368,1700,2022,2326,2626,2917,3201,3481,3756,4022,4278,4534,4777,4980,5256,5484,5887,6301,6701,7072,7421,7754,8070,8363,8634,8884,9125,9333,
        -1,1601,1939,2260,2567,2863,3161,3438,3719,3992,4239,4492,4753,4979,5258,5488,5663,6118,6532,6925,7301,7651,7979,8288,8581,8848,9102,9337,  -1,
        -1,1836,2169,2490,2797,3107,3387,3686,3946,4226,4493,4749,5001,5247,5483,5715,5889,6344,6761,7149,7520,7864,8198,8503,8798,9064,9310,9545,  -1,
        -1,2057,2387,2712,3016,3330,3619,3896,4181,4446,4714,4968,5222,5462,5698,5931,6161,6553,6977,7361,7735,8077,8410,8723,8996,9274,9513,  -1,  -1,
        -1,2281,2617,2933,3252,3552,3843,4121,4405,4663,4931,5192,5440,5686,5888,6155,6370,6771,7182,7574,7934,8282,8608,8909,9207,9457,9708,  -1,  -1,
        -1,2480,2815,3154,3463,3758,4051,4337,4608,4882,5119,5395,5609,5868,6128,6347,6583,6969,7383,7766,8151,8484,8804,9116,9390,9655,  -1,  -1,  -1,
        -1,2693,3027,3363,3678,3977,4262,4535,4820,5091,5353,5603,5838,6085,6334,6554,6790,7175,7577,7960,8322,8667,8987,9286,9568,9829,  -1,  -1,  -1,
        -1,2884,3220,3544,3872,4172,4463,4723,5012,5289,5544,5789,6039,6240,6527,6756,6977,7355,7765,8145,8506,8842,9167,9466,9742,  -1,  -1,  -1,  -1,
        -1,3092,3433,3750,4048,4377,4642,4920,5188,5451,5738,5980,6196,6431,6658,6952,7171,7545,7949,8329,8690,9029,9344,9646,9919,  -1,  -1,  -1,  -1,
        -1,3282,3601,3953,4235,4533,4859,5102,5407,5664,5904,6189,6397,6668,6907,7058,7329,7719,8120,8498,8861,9195,9509,9830,  -1,  -1,  -1,  -1,  -1,
        -1,3456,3831,4152,4454,4728,5048,5328,5603,5859,6121,6358,6552,6833,7075,7311,7523,7885,8292,8666,9039,9363,9674,9968,  -1,  -1,  -1,  -1,  -1,
        -1,3665,3964,4297,4632,4938,5210,5459,5779,6035,6299,6550,6785,7021,7213,7468,7708,8054,8456,8830,9185,9516,9827,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,3827,4142,4512,4805,5096,5394,5629,5958,6213,6473,6709,6924,7182,7429,7651,7834,8219,8619,8987,9344,9681,9999,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,4140,4491,4814,5123,5425,5714,5991,6256,6519,6771,7016,7254,7483,7707,7922,8135,8543,8939,9306,9657,9984,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,4460,4809,5136,5441,5741,6027,6298,6567,6827,7079,7324,7557,7785,8007,8221,8429,8833,9237,9590,9934,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,4761,5109,5431,5737,6032,6320,6591,6853,7116,7362,7603,7836,8062,8284,8494,8702,9104,9485,9851,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,5046,5390,5711,6021,6313,6602,6869,7136,7385,7634,7873,8102,8324,8547,8756,8958,9363,9738,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,5315,5657,5977,6278,6574,6857,7127,7384,7635,7879,8115,8347,8569,8784,8987,9200,9586,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,5558,5900,6218,6518,6811,7098,7358,7620,7867,8111,8343,8577,8795,9005,9210,9408,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,5784,6131,6455,6745,7040,7314,7579,7834,8094,8323,8562,8815,9015,9215,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,6009,6351,6661,6962,7250,7522,7781,8039,8288,8523,8756,8971,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,6217,6552,6862,7155,7438,7717,7973,8232,8461,8700,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,6402,6751,7050,7349,7629,7895,8149,8401,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,6586,6925,7229,7512,7785,8048,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,6739,7085,7371,7662,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    };
    //version
    p->u4Version = NVRAM_CAMERA_STROBE_FILE_VERSION;
    //eng tab
    memcpy(p->engTab.yTab, engTab, sizeof(engTab));

    //tuningPara[8];
    for(i=0; i<8; i++)
    {
        p->tuningPara[i].yTarget = 188;
        p->tuningPara[i].fgWIncreaseLevelbySize = 10;
        p->tuningPara[i].fgWIncreaseLevelbyRef = 5;//0;
        p->tuningPara[i].ambientRefAccuracyRatio = 255;
        p->tuningPara[i].flashRefAccuracyRatio = 255;//1;
        p->tuningPara[i].backlightAccuracyRatio = 18;
        p->tuningPara[i].backlightUnderY = 40;
        p->tuningPara[i].backlightWeakRefRatio = 32;
        p->tuningPara[i].safetyExp =66644;//33322;
        p->tuningPara[i].maxUsableISO = 1200;//680;
        p->tuningPara[i].yTargetWeight = 0;
        p->tuningPara[i].lowReflectanceThreshold = 13;
        p->tuningPara[i].flashReflectanceWeight = 0;
        p->tuningPara[i].bgSuppressMaxDecreaseEV = 20;
        p->tuningPara[i].bgSuppressMaxOverExpRatio = 6;
        p->tuningPara[i].fgEnhanceMaxIncreaseEV = 50;
        p->tuningPara[i].fgEnhanceMaxOverExpRatio = 2;//6;
        p->tuningPara[i].isFollowCapPline = 1;
        p->tuningPara[i].histStretchMaxFgYTarget = 300;//285;//266;
        p->tuningPara[i].histStretchBrightestYTarget = 480;//404;//328;
        p->tuningPara[i].fgSizeShiftRatio = 0;
        p->tuningPara[i].backlitPreflashTriggerLV = 90;
        p->tuningPara[i].backlitMinYTarget = 90;//100;

    }

    p->tuningPara[0].isFollowCapPline = 0;

    p->paraIdxForceOn[0] =1;    //default
    p->paraIdxForceOn[1] =0;    //LIB3A_AE_SCENE_OFF
    p->paraIdxForceOn[2] =0;    //LIB3A_AE_SCENE_AUTO
    p->paraIdxForceOn[3] =1;    //LIB3A_AE_SCENE_NIGHT
    p->paraIdxForceOn[4] =1;    //LIB3A_AE_SCENE_ACTION
    p->paraIdxForceOn[5] =1;    //LIB3A_AE_SCENE_BEACH
    p->paraIdxForceOn[6] =1;    //LIB3A_AE_SCENE_CANDLELIGHT
    p->paraIdxForceOn[7] =1;    //LIB3A_AE_SCENE_FIREWORKS
    p->paraIdxForceOn[8] =1;    //LIB3A_AE_SCENE_LANDSCAPE
    p->paraIdxForceOn[9] =1;    //LIB3A_AE_SCENE_PORTRAIT
    p->paraIdxForceOn[10] =1;   //LIB3A_AE_SCENE_NIGHT_PORTRAIT
    p->paraIdxForceOn[11] =1;   //LIB3A_AE_SCENE_PARTY
    p->paraIdxForceOn[12] =1;   //LIB3A_AE_SCENE_SNOW
    p->paraIdxForceOn[13] =1;   //LIB3A_AE_SCENE_SPORTS
    p->paraIdxForceOn[14] =1;   //LIB3A_AE_SCENE_STEADYPHOTO
    p->paraIdxForceOn[15] =1;   //LIB3A_AE_SCENE_SUNSET
    p->paraIdxForceOn[16] =1;   //LIB3A_AE_SCENE_THEATRE
    p->paraIdxForceOn[17] =1;   //LIB3A_AE_SCENE_ISO_ANTI_SHAKE
    p->paraIdxForceOn[18] =1;   //LIB3A_AE_SCENE_BACKLIGHT

    p->paraIdxAuto[0] =1;  //default
    p->paraIdxAuto[1] =0;  //LIB3A_AE_SCENE_OFF
    p->paraIdxAuto[2] =0;  //LIB3A_AE_SCENE_AUTO
    p->paraIdxAuto[3] =1;  //LIB3A_AE_SCENE_NIGHT
    p->paraIdxAuto[4] =1;  //LIB3A_AE_SCENE_ACTION
    p->paraIdxAuto[5] =1;  //LIB3A_AE_SCENE_BEACH
    p->paraIdxAuto[6] =1;  //LIB3A_AE_SCENE_CANDLELIGHT
    p->paraIdxAuto[7] =1;  //LIB3A_AE_SCENE_FIREWORKS
    p->paraIdxAuto[8] =1;  //LIB3A_AE_SCENE_LANDSCAPE
    p->paraIdxAuto[9] =1;  //LIB3A_AE_SCENE_PORTRAIT
    p->paraIdxAuto[10] =1; //LIB3A_AE_SCENE_NIGHT_PORTRAIT
    p->paraIdxAuto[11] =1; //LIB3A_AE_SCENE_PARTY
    p->paraIdxAuto[12] =1; //LIB3A_AE_SCENE_SNOW
    p->paraIdxAuto[13] =1; //LIB3A_AE_SCENE_SPORTS
    p->paraIdxAuto[14] =1; //LIB3A_AE_SCENE_STEADYPHOTO
    p->paraIdxAuto[15] =1; //LIB3A_AE_SCENE_SUNSET
    p->paraIdxAuto[16] =1; //LIB3A_AE_SCENE_THEATRE
    p->paraIdxAuto[17] =1; //LIB3A_AE_SCENE_ISO_ANTI_SHAKE
    p->paraIdxAuto[18] =1; //LIB3A_AE_SCENE_BACKLIGHT



    //--------------------
    //eng level
    //index mode
    //torch
    p->engLevel.torchDuty = 6;
    //af
    p->engLevel.afDuty = 6;
    //pf, mf, normal
    p->engLevel.pfDuty = 6;
    p->engLevel.mfDutyMax = 27;
    p->engLevel.mfDutyMin = 0;
    //low bat
    p->engLevel.IChangeByVBatEn=0;
    p->engLevel.vBatL = 3600;//mv
    p->engLevel.pfDutyL = 6;
    p->engLevel.mfDutyMaxL = 6;
    p->engLevel.mfDutyMinL = 0;
    //burst setting
    p->engLevel.IChangeByBurstEn=1;
    p->engLevel.pfDutyB = 6;
    p->engLevel.mfDutyMaxB = 6;
    p->engLevel.mfDutyMinB = 0;
    //high current setting
    p->engLevel.decSysIAtHighEn = 1;
    p->engLevel.dutyH = 20;

    //LT
    p->engLevelLT.torchDuty = 0;
    //af
    p->engLevelLT.afDuty = 0;
    //pf, mf, normal
    p->engLevelLT.pfDuty = 0;
    p->engLevelLT.mfDutyMax = 27;
    p->engLevelLT.mfDutyMin = -1;
    //low bat
    p->engLevelLT.pfDutyL = 0;
    p->engLevelLT.mfDutyMaxL = 6;
    p->engLevelLT.mfDutyMinL = -1;
    //burst setting
    p->engLevelLT.pfDutyB = 0;
    p->engLevelLT.mfDutyMaxB = 6;
    p->engLevelLT.mfDutyMinB = -1;
    p->dualTuningPara.toleranceEV_pos = 10; //0.1 EV
    p->dualTuningPara.toleranceEV_neg = 10; //0.1 EV

    p->dualTuningPara.XYWeighting = 64;  //0.5  , 128 base
    p->dualTuningPara.useAwbPreferenceGain = 0; //the same with environment lighting condition
    p->dualTuningPara.envOffsetIndex[0] = -200;
    p->dualTuningPara.envOffsetIndex[1] = -100;
    p->dualTuningPara.envOffsetIndex[2] = 50;
    p->dualTuningPara.envOffsetIndex[3] = 150;

    p->dualTuningPara.envXrOffsetValue[0] = 0;
    p->dualTuningPara.envXrOffsetValue[1] = 0;
    p->dualTuningPara.envXrOffsetValue[2] = 0;
    p->dualTuningPara.envXrOffsetValue[3] = 0;

    p->dualTuningPara.envYrOffsetValue[0] = 0;
    p->dualTuningPara.envYrOffsetValue[0] = 0;
    p->dualTuningPara.envYrOffsetValue[0] = 0;
    p->dualTuningPara.envYrOffsetValue[0] = 0;

    p->dualTuningPara.VarianceTolerance = 23;
    p->dualTuningPara.ChooseColdOrWarm = FLASH_CHOOSE_COLD;


    return 0;
}

int cust_fillDefaultFlashCalibrationNVRam_default2 (void* data)
{

    NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT* d;
    d = (NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT*)data;

    static short engTab[]=
         {1012,1922,2745,3495,4873,6114,7242,8280,9202,9999};
    memcpy(d->yTab, engTab, sizeof(engTab));


    //d->flashWBGain

    return 0;
}


