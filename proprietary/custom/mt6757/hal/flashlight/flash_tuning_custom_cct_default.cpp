#define LOG_TAG "flash_tuning_custom_cct.cpp"
#define MTK_LOG_ENABLE 1
#include "string.h"
#include "camera_custom_nvram.h"
#include "camera_custom_types.h"
#include <custom/aaa/AEPlinetable.h>
#include <log/log.h>
#include "flash_feature.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <kd_camera_feature.h>

//==============================================================================
//
//==============================================================================
int cust_fillDefaultStrobeNVRam_default (void* data)
{
    int i;
    NVRAM_CAMERA_STROBE_STRUCT* p;
    p = (NVRAM_CAMERA_STROBE_STRUCT*)data;

    static short engTab[]=
    {
           0, 338, 671, 994,1307,1611,1909,2196,2476,2751,3019,3280,3539,3789,4034,4270,4505,4985,5417,5827,6222,6589,6933,7258,7564,7846,8109,8348,8614,8774,8944,
         267, 594, 926,1247,1563,1868,2164,2452,2733,3007,3276,3540,3797,4047,4289,4530,4762,5241,5680,6085,6481,6849,7199,7535,7836,8119,8389,8625,8853,9059,  -1,
         511, 843,1178,1500,1813,2117,2414,2701,2982,3255,3523,3784,4041,4291,4534,4771,5003,5497,5928,6343,6734,7101,7446,7771,8079,8360,8630,8867,9085,9286,  -1,
         750,1083,1415,1735,2051,2355,2651,2938,3219,3492,3759,4020,4280,4525,4767,5002,5234,5728,6160,6567,6955,7322,7667,7993,8295,8603,8852,9134,9302,  -1,  -1,
         978,1313,1646,1969,2283,2587,2883,3168,3449,3720,3988,4247,4505,4751,4992,5227,5457,5951,6380,6787,7175,7540,7881,8210,8511,8798,9050,9283,9531,  -1,  -1,
        1198,1538,1868,2191,2505,2807,3104,3388,3669,3941,4207,4465,4722,4968,5208,5442,5681,6167,6596,6998,7387,7755,8090,8411,8715,9023,9258,9482,  -1,  -1,  -1,
        1412,1752,2082,2405,2720,3022,3316,3602,3881,4152,4418,4677,4930,5178,5419,5652,5877,6374,6810,7203,7590,7949,8289,8604,8906,9183,9439,9679,  -1,  -1,  -1,
        1618,1958,2289,2610,2925,3227,3519,3805,4082,4352,4618,4875,5129,5377,5612,5847,6079,6569,6993,7394,7777,8147,8476,8795,9092,9364,9616,  -1,  -1,  -1,  -1,
        1821,2159,2491,2813,3124,3428,3721,4005,4284,4553,4816,5077,5327,5573,5811,6040,6268,6764,7189,7587,7973,8328,8661,8978,9316,9560,9812,  -1,  -1,  -1,  -1,
        2017,2355,2687,3009,3321,3622,3917,4200,4478,4747,5011,5265,5520,5761,5999,6231,6460,6951,7374,7773,8153,8512,8846,9164,9452,9731,  -1,  -1,  -1,  -1,  -1,
        2211,2549,2880,3202,3515,3816,4108,4392,4669,4937,5199,5456,5707,5950,6187,6419,6644,7137,7560,7957,8337,8701,9059,9347,9638,9904,  -1,  -1,  -1,  -1,  -1,
        2393,2733,3064,3385,3697,3999,4292,4575,4850,5119,5382,5636,5887,6130,6366,6597,6820,7315,7736,8132,8508,8863,9194,9507,9795,  -1,  -1,  -1,  -1,  -1,  -1,
        2579,2917,3249,3571,3884,4182,4474,4757,5031,5301,5562,5820,6066,6311,6545,6775,6997,7487,7904,8301,8680,9030,9366,9670,9955,  -1,  -1,  -1,  -1,  -1,  -1,
        2754,3095,3424,3746,4061,4361,4651,4935,5211,5475,5735,5988,6238,6480,6714,6943,7167,7655,8073,8465,8843,9193,9527,9830,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        2928,3268,3599,3923,4231,4531,4824,5104,5380,5646,5906,6159,6410,6651,6894,7114,7337,7827,8239,8631,9021,9389,9708,9999,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3096,3436,3768,4088,4399,4697,4989,5271,5542,5808,6071,6322,6571,6811,7044,7270,7494,7983,8395,8790,9159,9509,9835,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3261,3601,3931,4251,4562,4861,5151,5438,5705,5970,6230,6480,6729,6969,7200,7428,7649,8135,8549,8935,9309,9661,9985,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3581,3932,4278,4600,4913,5218,5510,5793,6073,6344,6605,6855,7105,7342,7576,7803,8027,8455,8866,9250,9621,9970,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3888,4237,4588,4908,5219,5522,5819,6098,6380,6645,6903,7154,7403,7638,7873,8097,8318,8744,9154,9537,9906,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4172,4522,4871,5196,5502,5806,6095,6379,6658,6922,7180,7431,7679,7913,8146,8371,8595,9021,9427,9805,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4445,4806,5146,5466,5775,6077,6366,6649,6924,7192,7446,7716,7952,8181,8413,8639,8853,9277,9679,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4706,5053,5400,5720,6031,6332,6625,6895,7172,7438,7694,7945,8191,8422,8650,8869,9092,9542,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4941,5304,5647,5962,6270,6572,6862,7138,7411,7672,7926,8177,8445,8655,8882,9098,9316,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5184,5529,5867,6193,6499,6788,7078,7369,7627,7897,8143,8393,8632,8894,9097,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5395,5757,6087,6415,6732,7005,7293,7568,7838,8096,8347,8588,8828,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5573,5936,6275,6595,6892,7224,7478,7749,8028,8286,8536,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5757,6119,6461,6777,7080,7408,7656,7929,8192,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5929,6283,6624,6972,7239,7546,7820,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6088,6446,6807,7100,7416,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6228,6586,6930,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6378,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    };
    //version
    p->u4Version = NVRAM_CAMERA_STROBE_FILE_VERSION;
    //eng tab
    memcpy(p->engTab.yTab, engTab, sizeof(engTab));

    //tuningPara[8];
    for(i=0;i<8;i++)
    {
        p->tuningPara[i].yTarget = 188;
        p->tuningPara[i].fgWIncreaseLevelbySize = 10;
        p->tuningPara[i].fgWIncreaseLevelbyRef = 5;//0;
        p->tuningPara[i].ambientRefAccuracyRatio = 5;
        p->tuningPara[i].flashRefAccuracyRatio = 0;//1;
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
        p->tuningPara[i].minstameanpass = 80;
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
    p->engLevel.mfDutyMax = 29;
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
    p->engLevelLT.mfDutyMax = 29;
    p->engLevelLT.mfDutyMin = -1;
    //low bat
    p->engLevelLT.pfDutyL = 0;
    p->engLevelLT.mfDutyMaxL = 6;
    p->engLevelLT.mfDutyMinL = -1;
    //burst setting
    p->engLevelLT.pfDutyB = 0;
    p->engLevelLT.mfDutyMaxB = 6;
    p->engLevelLT.mfDutyMinB = -1;

    p->dualTuningPara.toleranceEV_pos = 30; //0.1 EV
    p->dualTuningPara.toleranceEV_neg = 30; //0.1 EV

    p->dualTuningPara.XYWeighting = 64;  //0.5  , 128 base
    p->dualTuningPara.useAwbPreferenceGain = 1; //the same with environment lighting condition
    p->dualTuningPara.envOffsetIndex[0] = -200;
    p->dualTuningPara.envOffsetIndex[1] = -100;
    p->dualTuningPara.envOffsetIndex[2] = 50;
    p->dualTuningPara.envOffsetIndex[3] = 150;

    p->dualTuningPara.envXrOffsetValue[0] = 0;
    p->dualTuningPara.envXrOffsetValue[1] = 0;
    p->dualTuningPara.envXrOffsetValue[2] = 0;
    p->dualTuningPara.envXrOffsetValue[3] = 0;

    p->dualTuningPara.envYrOffsetValue[0] = 0;
    p->dualTuningPara.envYrOffsetValue[1] = 0;
    p->dualTuningPara.envYrOffsetValue[2] = 0;
    p->dualTuningPara.envYrOffsetValue[3] = 0;

    p->dualTuningPara.VarianceTolerance = 1;
    p->dualTuningPara.ChooseColdOrWarm = NVRAM_FLASH_CHOOSE_COLD;
    return 0;
}

int cust_fillDefaultFlashCalibrationNVRam_default (void* data)
{

    NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT* d;
    d = (NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT*)data;

    static short engTab[]=
         {1012,1922,2745,3495,4873,6114,7242,8280,9202,9999};
    memcpy(d->yTab, engTab, sizeof(engTab));


    //d->flashWBGain

    return 0;
}

