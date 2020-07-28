#define LOG_TAG "flash_tuning_custom_cct_sub.cpp"
#define MTK_LOG_ENABLE 1

#include <string.h>
#include "camera_custom_nvram.h"

#if FLASH_NVRAM
#else

/***********************************************************
 * Energy and FlashWB Gain
 **********************************************************/
// TODO: setup energy table
static short engTab[] = {
    -1, 271, 589, 888, 1183, 1447, 1728, 2006, 2319, 2808, 3396, 3850, 4280, 4698, 5196, 5570, 5934, 6374, 6711, 7023, 7276, 7654, 7952, 8189, 8455, 8737, 8944,
    218, 513, 817, 1120, 1406, 1678, 1965, 2239, 2550, 3039, 3626, 4078, 4507, 4923, 5419, 5798, 6157, 6598, 6928, 7251, 7504, 7868, 8152, 8403, 8659, 8944, -1,
    491, 767, 1080, 1376, 1672, 1947, 2228, 2502, 2795, 3290, 3878, 4328, 4752, 5167, 5659, 6039, 6400, 6841, 7166, 7478, 7729, 8127, 8386, 8634, 8883, 9178, -1,
    731, 1016, 1328, 1628, 1929, 2200, 2481, 2754, 3037, 3531, 4119, 4566, 4991, 5405, 5895, 6277, 6631, 7070, 7395, 7711, 7958, 8331, 8617, 8847, 9098, -1, -1,
    972, 1263, 1571, 1877, 2174, 2447, 2730, 3003, 3271, 3767, 4351, 4795, 5220, 5633, 6126, 6502, 6858, 7295, 7616, 7929, 8189, 8548, 8818, 9068, 9321, -1, -1,
    1197, 1481, 1794, 2099, 2397, 2672, 2955, 3231, 3501, 3994, 4577, 5021, 5443, 5854, 6352, 6724, 7083, 7513, 7829, 8147, 8397, 8765, 9031, 9278, -1, -1, -1,
    1425, 1708, 2026, 2332, 2634, 2907, 3190, 3466, 3722, 4214, 4794, 5236, 5657, 6072, 6564, 6939, 7291, 7722, 8046, 8348, 8595, 8969, 9233, 9476, -1, -1, -1,
    1640, 1932, 2250, 2560, 2859, 3133, 3416, 3693, 3939, 4427, 5004, 5448, 5870, 6285, 6771, 7142, 7495, 7924, 8247, 8549, 8800, 9172, 9431, -1, -1, -1, -1,
    1888, 2182, 2484, 2778, 3061, 3339, 3605, 3871, 4253, 4741, 5318, 5757, 6178, 6591, 7076, 7447, 7802, 8232, 8540, 8842, 9088, 9449, 9717, -1, -1, -1, -1,
    2283, 2578, 2879, 3173, 3459, 3736, 4005, 4269, 4648, 5131, 5708, 6148, 6570, 6979, 7462, 7829, 8176, 8599, 8916, 9221, 9459, 9814, -1, -1, -1, -1, -1,
    2752, 3048, 3354, 3648, 3933, 4210, 4477, 4741, 5119, 5600, 6174, 6611, 7031, 7438, 7913, 8280, 8625, 9045, 9352, 9649, 9885, -1, -1, -1, -1, -1, -1,
    3107, 3407, 3714, 4011, 4293, 4569, 4837, 5097, 5473, 5954, 6529, 6966, 7378, 7780, 8256, 8622, 8963, 9375, 9685, 9980, -1, -1, -1, -1, -1, -1, -1,
    3449, 3751, 4058, 4356, 4635, 4912, 5176, 5437, 5812, 6294, 6863, 7299, 7709, 8109, 8582, 8944, 9279, 9690, 9999, -1, -1, -1, -1, -1, -1, -1, -1,
    3787, 4087, 4392, 4684, 4967, 5239, 5505, 5764, 6138, 6615, 7184, 7618, 8026, 8422, 8891, 9252, 9582, 9992, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4181, 4484, 4782, 5075, 5352, 5625, 5887, 6147, 6523, 6997, 7560, 7989, 8395, 8788, 9252, 9608, 9936, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4481, 4777, 5080, 5375, 5649, 5920, 6183, 6440, 6811, 7284, 7844, 8270, 8675, 9070, 9527, 9874, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4762, 5058, 5362, 5651, 5928, 6195, 6458, 6717, 7084, 7558, 8115, 8534, 8936, 9326, 9784, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    5109, 5401, 5702, 5988, 6267, 6538, 6805, 7045, 7416, 7886, 8436, 8859, 9251, 9650, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    5366, 5659, 5960, 6245, 6521, 6783, 7045, 7297, 7659, 8148, 8678, 9092, 9487, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    5616, 5904, 6203, 6500, 6762, 7028, 7281, 7537, 7901, 8356, 8902, 9316, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    5817, 6106, 6403, 6691, 6968, 7221, 7480, 7725, 8126, 8550, 9095, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    6115, 6400, 6690, 6977, 7247, 7508, 7752, 8007, 8363, 8816, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    6324, 6618, 6905, 7196, 7455, 7726, 7959, 8208, 8580, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    6528, 6816, 7106, 7377, 7636, 7906, 8143, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    6725, 7011, 7303, 7562, 7828, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    6959, 7244, 7526, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    7137, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

// TODO: setup flash WB gain table
static AWB_GAIN_T flashWBGain[] = {
    
};

#endif

/***********************************************************
 * Public Method
 **********************************************************/
int cust_fillDefaultStrobeNVRam_sub(void* data)
{
    (void)data;
#if FLASH_NVRAM
#else
    NVRAM_CAMERA_STROBE_STRUCT* p = (NVRAM_CAMERA_STROBE_STRUCT*)data;
    memcpy(p->engTab.yTab, engTab, sizeof(engTab));

    // TODO: setup NVRAM strobe file version 
    p->u4Version = NVRAM_CAMERA_STROBE_FILE_VERSION;

    // TODO: setup tuning parameters
    for (int i = 0; i < 8; i++) {
        p->tuningPara[i].yTarget = 188;
        p->tuningPara[i].fgWIncreaseLevelbySize = 10;
        p->tuningPara[i].fgWIncreaseLevelbyRef = 5;
        p->tuningPara[i].ambientRefAccuracyRatio = 5;
        p->tuningPara[i].flashRefAccuracyRatio = 0;
        p->tuningPara[i].backlightAccuracyRatio = 18;
        p->tuningPara[i].backlightUnderY = 40;
        p->tuningPara[i].backlightWeakRefRatio = 32;
        p->tuningPara[i].safetyExp = 66644;
        p->tuningPara[i].maxUsableISO = 1200;
        p->tuningPara[i].yTargetWeight = 0;
        p->tuningPara[i].lowReflectanceThreshold = 13;
        p->tuningPara[i].flashReflectanceWeight = 0;
        p->tuningPara[i].bgSuppressMaxDecreaseEV = 20;
        p->tuningPara[i].bgSuppressMaxOverExpRatio = 6;
        p->tuningPara[i].fgEnhanceMaxIncreaseEV = 50;
        p->tuningPara[i].fgEnhanceMaxOverExpRatio = 2;
        p->tuningPara[i].isFollowCapPline = 1;
        p->tuningPara[i].histStretchMaxFgYTarget = 300;
        p->tuningPara[i].histStretchBrightestYTarget = 480;
        p->tuningPara[i].fgSizeShiftRatio = 0;
        p->tuningPara[i].backlitPreflashTriggerLV = 90;
        p->tuningPara[i].backlitMinYTarget = 90;
        p->tuningPara[i].minstameanpass = 80;
    }
    p->tuningPara[0].isFollowCapPline = 0;

    // TODO: setup tuning scene index for force on
    p->paraIdxForceOn[0]  = 1; //default
    p->paraIdxForceOn[1]  = 0; //LIB3A_AE_SCENE_OFF
    p->paraIdxForceOn[2]  = 0; //LIB3A_AE_SCENE_AUTO
    p->paraIdxForceOn[3]  = 1; //LIB3A_AE_SCENE_NIGHT
    p->paraIdxForceOn[4]  = 1; //LIB3A_AE_SCENE_ACTION
    p->paraIdxForceOn[5]  = 1; //LIB3A_AE_SCENE_BEACH
    p->paraIdxForceOn[6]  = 1; //LIB3A_AE_SCENE_CANDLELIGHT
    p->paraIdxForceOn[7]  = 1; //LIB3A_AE_SCENE_FIREWORKS
    p->paraIdxForceOn[8]  = 1; //LIB3A_AE_SCENE_LANDSCAPE
    p->paraIdxForceOn[9]  = 1; //LIB3A_AE_SCENE_PORTRAIT
    p->paraIdxForceOn[10] = 1; //LIB3A_AE_SCENE_NIGHT_PORTRAIT
    p->paraIdxForceOn[11] = 1; //LIB3A_AE_SCENE_PARTY
    p->paraIdxForceOn[12] = 1; //LIB3A_AE_SCENE_SNOW
    p->paraIdxForceOn[13] = 1; //LIB3A_AE_SCENE_SPORTS
    p->paraIdxForceOn[14] = 1; //LIB3A_AE_SCENE_STEADYPHOTO
    p->paraIdxForceOn[15] = 1; //LIB3A_AE_SCENE_SUNSET
    p->paraIdxForceOn[16] = 1; //LIB3A_AE_SCENE_THEATRE
    p->paraIdxForceOn[17] = 1; //LIB3A_AE_SCENE_ISO_ANTI_SHAKE
    p->paraIdxForceOn[18] = 1; //LIB3A_AE_SCENE_BACKLIGHT

    // TODO: setup tuning scene index for auto
    p->paraIdxAuto[0]  = 1; //default
    p->paraIdxAuto[1]  = 0; //LIB3A_AE_SCENE_OFF
    p->paraIdxAuto[2]  = 0; //LIB3A_AE_SCENE_AUTO
    p->paraIdxAuto[3]  = 1; //LIB3A_AE_SCENE_NIGHT
    p->paraIdxAuto[4]  = 1; //LIB3A_AE_SCENE_ACTION
    p->paraIdxAuto[5]  = 1; //LIB3A_AE_SCENE_BEACH
    p->paraIdxAuto[6]  = 1; //LIB3A_AE_SCENE_CANDLELIGHT
    p->paraIdxAuto[7]  = 1; //LIB3A_AE_SCENE_FIREWORKS
    p->paraIdxAuto[8]  = 1; //LIB3A_AE_SCENE_LANDSCAPE
    p->paraIdxAuto[9]  = 1; //LIB3A_AE_SCENE_PORTRAIT
    p->paraIdxAuto[10] = 1; //LIB3A_AE_SCENE_NIGHT_PORTRAIT
    p->paraIdxAuto[11] = 1; //LIB3A_AE_SCENE_PARTY
    p->paraIdxAuto[12] = 1; //LIB3A_AE_SCENE_SNOW
    p->paraIdxAuto[13] = 1; //LIB3A_AE_SCENE_SPORTS
    p->paraIdxAuto[14] = 1; //LIB3A_AE_SCENE_STEADYPHOTO
    p->paraIdxAuto[15] = 1; //LIB3A_AE_SCENE_SUNSET
    p->paraIdxAuto[16] = 1; //LIB3A_AE_SCENE_THEATRE
    p->paraIdxAuto[17] = 1; //LIB3A_AE_SCENE_ISO_ANTI_SHAKE
    p->paraIdxAuto[18] = 1; //LIB3A_AE_SCENE_BACKLIGHT

    // TODO: setup HT torch duty
    p->engLevel.torchDuty = 6;
    // TODO: setup HT AF lamp duty
    p->engLevel.afDuty = 6;
    // TODO: setup HT pre-flash duty
    p->engLevel.pfDuty = 6;
    // TODO: setup HT main flash duty range
    p->engLevel.mfDutyMax = 25;
    p->engLevel.mfDutyMin = -1;
    // TODO: setup HT low battery
    p->engLevel.IChangeByVBatEn = 0;
    p->engLevel.vBatL = 3600; //mv
    p->engLevel.pfDutyL = 6;
    p->engLevel.mfDutyMaxL = 6;
    p->engLevel.mfDutyMinL = -1;
    // TODO: setup HT burst shot
    p->engLevel.IChangeByBurstEn = 1;
    p->engLevel.pfDutyB = 6;
    p->engLevel.mfDutyMaxB = 6;
    p->engLevel.mfDutyMinB = -1;
    // TODO: setup HT high current
    p->engLevel.decSysIAtHighEn = 1;
    p->engLevel.dutyH = 20;

    // TODO: setup LT torch duty
    p->engLevelLT.torchDuty = 0;
    // TODO: setup LT AF lamp duty
    p->engLevelLT.afDuty = 0;
    // TODO: setup LT pre-flash duty
    p->engLevelLT.pfDuty = 0;
    // TODO: setup LT main flash duty range
    p->engLevelLT.mfDutyMax = 25;
    p->engLevelLT.mfDutyMin = -1;
    // TODO: setup HT low battery
    p->engLevelLT.pfDutyL = 0;
    p->engLevelLT.mfDutyMaxL = 6;
    p->engLevelLT.mfDutyMinL = -1;
    // TODO: setup HT burst shot
    p->engLevelLT.pfDutyB = 0;
    p->engLevelLT.mfDutyMaxB = 6;
    p->engLevelLT.mfDutyMinB = -1;

    // TODO: setup dual flash tuning parameters
    p->dualTuningPara.toleranceEV_pos = 30;
    p->dualTuningPara.toleranceEV_neg = 30;

    p->dualTuningPara.XYWeighting = 64;
    p->dualTuningPara.useAwbPreferenceGain = 1;
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
    p->dualTuningPara.ChooseColdOrWarm = FLASH_CHOOSE_COLD;
#endif
    return 0;
}

int cust_fillDefaultFlashCalibrationNVRam_sub(void* data)
{
    (void)data;
#if FLASH_NVRAM
#else
    NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT* p = (NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT*)data;
    memcpy(p->yTab, engTab, sizeof(engTab));
    memcpy(p->flashWBGain, flashWBGain, sizeof(flashWBGain));
#endif
    return 0;
}

