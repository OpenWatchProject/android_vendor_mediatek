#define LOG_TAG "flash_tuning_custom_cct_main.cpp"
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
    -1, 218, 473, 728, 972, 1196, 1427, 1659, 1914, 2321, 2826, 3205, 3568, 3928, 4355, 4691, 5002, 5397, 5547, 5816, 6202, 6326, 6825, 6750, 7323, 7571, 7322,
    263, 498, 756, 1006, 1255, 1481, 1715, 1944, 2199, 2618, 3108, 3489, 3853, 4211, 4638, 4920, 5283, 5674, 5884, 6247, 6464, 6840, 7091, 7337, 7520, 7857, -1,
    563, 803, 1063, 1318, 1556, 1782, 2020, 2248, 2496, 2906, 3395, 3774, 4097, 4493, 4921, 5161, 5557, 5955, 6249, 6530, 6743, 6962, 7379, 7593, 7809, 8120, -1,
    849, 1089, 1353, 1608, 1857, 2072, 2311, 2540, 2772, 3182, 3672, 4049, 4413, 4764, 5117, 5520, 5833, 6222, 6514, 6800, 7019, 7362, 7623, 7522, 7713, -1, -1,
    1131, 1371, 1635, 1891, 2141, 2366, 2599, 2825, 3028, 3450, 3914, 4281, 4634, 5034, 5458, 5787, 6095, 6485, 6768, 7059, 7284, 7473, 7878, 8111, 8386, -1, -1,
    1381, 1623, 1891, 2148, 2400, 2627, 2857, 3085, 3304, 3713, 4205, 4541, 4940, 5299, 5724, 6050, 6356, 6601, 7030, 7317, 7515, 7873, 8135, 8377, -1, -1, -1,
    1644, 1888, 2157, 2416, 2668, 2891, 3124, 3352, 3558, 3969, 4457, 4832, 5165, 5547, 5973, 6300, 6486, 6985, 7181, 7356, 7763, 8116, 8072, 8605, -1, -1, -1,
    1894, 2143, 2415, 2672, 2923, 3152, 3388, 3612, 3806, 4192, 4705, 5082, 5386, 5794, 6219, 6543, 6847, 7236, 7347, 7587, 7764, 8347, 8594, -1, -1, -1, -1,
    2190, 2442, 2700, 2944, 3179, 3409, 3637, 3855, 4172, 4581, 5070, 5443, 5805, 6150, 6577, 6905, 7208, 7585, 7685, 8033, 8340, 8418, 8935, -1, -1, -1, -1,
    2642, 2901, 3163, 3412, 3649, 3884, 4104, 4322, 4638, 5046, 5507, 5903, 6263, 6613, 7034, 7240, 7661, 8028, 8131, 8364, 8796, 9142, -1, -1, -1, -1, -1,
    3191, 3452, 3686, 3965, 4204, 4415, 4659, 4882, 5191, 5602, 6088, 6462, 6810, 7171, 7587, 7905, 8197, 8585, 8862, 9122, 9351, -1, -1, -1, -1, -1, -1,
    3617, 3885, 4147, 4398, 4633, 4866, 5091, 5304, 5619, 6028, 6504, 6872, 7232, 7576, 8011, 8330, 8620, 8998, 9261, 9523, -1, -1, -1, -1, -1, -1, -1,
    4024, 4297, 4561, 4809, 5052, 5285, 5496, 5713, 6031, 6441, 6835, 7287, 7648, 7996, 8408, 8572, 8840, 9383, 9678, -1, -1, -1, -1, -1, -1, -1, -1,
    4428, 4702, 4961, 5216, 5452, 5680, 5903, 6124, 6434, 6830, 7315, 7688, 8033, 8393, 8800, 9112, 9412, 9554, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    4912, 5175, 5439, 5693, 5922, 6152, 6371, 6590, 6903, 7297, 7789, 8145, 8530, 8860, 9270, 9369, 9858, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    5290, 5555, 5811, 6062, 6288, 6529, 6739, 6957, 7273, 7674, 8145, 8520, 8867, 9217, 9621, 9921, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    5642, 5899, 6160, 6421, 6642, 6881, 6962, 7303, 7602, 8029, 8507, 8860, 9192, 9533, 9940, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    6084, 6337, 6621, 6841, 6912, 7315, 7520, 7730, 8060, 8434, 8917, 9295, 9645, 9999, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    6417, 6664, 6903, 7169, 7429, 7629, 7867, 7968, 8177, 8569, 9237, 9605, 9694, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    6748, 6913, 7251, 7505, 7733, 7954, 8163, 8377, 8699, 9080, 9556, 9912, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    6802, 7292, 7538, 7780, 8003, 8213, 8435, 8676, 8930, 9329, 9803, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    7444, 7683, 7783, 8185, 8399, 8618, 8842, 9032, 9026, 9711, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    7814, 7671, 8279, 8484, 8712, 8900, 9132, 9311, 9624, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    7687, 8285, 8511, 8730, 9018, 9170, 9410, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    8425, 8561, 8798, 9038, 9207, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    8696, 8454, 8655, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    9032, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

// TODO: setup flash WB gain table
static AWB_GAIN_T flashWBGain[] = {
    
};

#endif

/***********************************************************
 * Public Method
 **********************************************************/
int cust_fillDefaultStrobeNVRam_main(void* data)
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

int cust_fillDefaultFlashCalibrationNVRam_main(void* data)
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

