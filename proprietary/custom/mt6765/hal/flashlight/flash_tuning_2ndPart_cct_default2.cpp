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
int cust_fillDefaultStrobeNVRam_default2 (void* data)
{
    int i;
    NVRAM_CAMERA_STROBE_STRUCT* p;
    p = (NVRAM_CAMERA_STROBE_STRUCT*)data;

    static short engTab[]=
    {
          -1, 422, 822,1204,1570,1928,2272,2609,2937,3251,3560,3860,4154,4438,4717,4989,5252,5799,6313,6768,7220,7643,8041,8415,8802,9106,9421,
         320, 734,1128,1505,1871,2227,2572,2907,3235,3550,3857,4159,4456,4736,5013,5287,5549,6098,6593,7060,7510,7937,8343,8726,9093,9429,  -1,
         612,1024,1421,1799,2163,2521,2864,3200,3526,3841,4149,4448,4743,5025,5302,5572,5835,6383,6882,7356,7804,8220,8628,9026,9372,9717,  -1,
         887,1302,1698,2076,2442,2798,3141,3475,3801,4115,4422,4722,5013,5295,5571,5840,6101,6648,7152,7612,8062,8476,8888,9247,9614,  -1,  -1,
        1152,1568,1965,2344,2708,3065,3408,3742,4067,4380,4687,4985,5277,5554,5834,6101,6364,6362,7404,7868,8331,8734,9126,9500,9841,  -1,  -1,
        1403,1818,2216,2594,2958,3314,3654,3989,4314,4627,4930,5228,5518,5801,6072,6337,6602,7152,7636,8105,8537,8966,9359,9725,  -1,  -1,  -1,
        1642,2060,2457,2837,3200,3557,3900,4232,4555,4866,5171,5468,5758,6038,6310,6576,6839,7396,7877,8339,8789,9197,9592,9947,  -1,  -1,  -1,
        1874,2293,2690,3068,3432,3789,4130,4463,4787,5096,5400,5696,5988,6266,6537,6800,7061,7618,8096,8554,8996,9417,9792,  -1,  -1,  -1,  -1,
        2105,2523,2922,3299,3663,4019,4360,4692,5014,5326,5627,5926,6212,6488,6762,7024,7282,7838,8318,8783,9211,9623,9999,  -1,  -1,  -1,  -1,
        2324,2743,3141,3520,3884,4239,4579,4910,5234,5542,5845,6139,6427,6700,6973,7237,7496,8040,8526,8991,9419,9836,  -1,  -1,  -1,  -1,  -1,
        2540,2961,3359,3737,4100,4456,4795,5124,5446,5753,6055,6349,6635,6911,7177,7443,7694,8247,8727,9186,9619,  -1,  -1,  -1,  -1,  -1,  -1,
        2755,3174,3572,3953,4312,4668,5007,5338,5658,5963,6263,6562,6843,7118,7384,7647,7899,8448,8919,9367,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        2961,3381,3777,4154,4519,4871,5211,5539,5859,6162,6460,6755,7039,7309,7579,7833,8091,8626,9105,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3161,3580,3977,4352,4714,5067,5405,5733,6053,6357,6656,6943,7228,7501,7764,8028,8279,8818,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3357,3774,4171,4547,4909,5261,5596,5925,6241,6544,6840,7128,7413,7684,7951,8207,8455,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3548,3966,4365,4736,5097,5450,5781,6110,6424,6727,7023,7314,7592,7863,8126,8381,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        3729,4151,4545,4918,5277,5626,5961,6287,6599,6902,7200,7487,7769,8035,8293,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4090,4512,4920,5292,5657,6003,6341,6665,6973,7281,7584,7859,8149,8418,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4436,4848,5254,5627,5995,6334,6670,6989,7299,7602,7908,8184,8459,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        4754,5174,5575,5949,6310,6649,6984,7303,7607,7920,8206,8500,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5056,5477,5874,6245,6603,6950,7276,7590,7895,8202,8491,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5337,5765,6160,6532,6885,7224,7548,7870,8168,8476,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5614,6031,6488,6803,7156,7492,7808,8130,8438,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5866,6297,6694,7047,7412,7751,8068,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6130,6544,6962,7303,7652,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6364,6780,7172,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        6605,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
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
    p->engLevel.mfDutyMax = 25;
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
    p->engLevelLT.mfDutyMax = 25;
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

