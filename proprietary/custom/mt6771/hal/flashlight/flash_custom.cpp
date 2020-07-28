#define LOG_TAG "flash_custom.cpp"
#define MTK_LOG_ENABLE 1

#include <cutils/log.h>
#include "mtkcam/def/BuiltinTypes.h"
#include "kd_camera_feature.h"
#include "camera_custom_nvram.h"
#include "flash_tuning_custom.h"
#include "strobe_param.h"


static int mainPartId = 1;
static int subPartId = 1;
void cust_setFlashPartId(int sensorDev, int id)
{
    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        mainPartId = id;
    else
        subPartId = id;
}

int cust_isDualFlashSupport(int sensorDev)
{
    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        return 1;
    else
        return 0;
}

STROBE_DEVICE_ENUM cust_getStrobeDevice(int sensorDev)
{
    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR)
        return STROBE_DEVICE_FLASHLIGHT;
    else if (sensorDev == DUAL_CAMERA_SUB_SENSOR)
        return STROBE_DEVICE_FLASHLIGHT;
    else if (sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
            sensorDev == DUAL_CAMERA_MAIN_SECOND_SENSOR)
        return STROBE_DEVICE_FLASHLIGHT;
    else if (sensorDev == DUAL_CAMERA_SUB_2_SENSOR)
        return STROBE_DEVICE_FLASHLIGHT;
    else if (sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        return STROBE_DEVICE_FLASHLIGHT;
    else
        return STROBE_DEVICE_FLASHLIGHT;
}

int cust_isIRProjectorSupport(int sensorDev __unused)
{
        return 0;
}

int cust_getHwTriggerSensor()
{
    return DUAL_CAMERA_SUB_2_SENSOR;
}

int cust_getFlashFrameRateDelay(int sensorDev)
{
    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR)
        return 1;
    else if (sensorDev == DUAL_CAMERA_SUB_SENSOR)
        return 1;
    else if (sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
            sensorDev == DUAL_CAMERA_MAIN_SECOND_SENSOR)
        return 1;
    else if (sensorDev == DUAL_CAMERA_SUB_2_SENSOR)
        return 1;
    else if (sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        return 1;
    else
        return 1;
}


/***********************************************************
 * flash_tuning_custom_cct_[main|main2|sub|sub2][_part2].cpp
 *
 * Consider variance
 * - LED driver IC
 * - LED component
 * - Image sensor
 * - ISP pass1 before AAO
 **********************************************************/
int cust_fillDefaultStrobeNVRam(int sensorDev, void* data)
{
    ALOGD("%s(): sensorDev(%d), mainPartId(%d), subPartId(%d).",
            __FUNCTION__, sensorDev, mainPartId, subPartId);

    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR)
        if (mainPartId == 1)
            return cust_fillDefaultStrobeNVRam_main(data);
        else
            return cust_fillDefaultStrobeNVRam_main_part2(data);
    else if (sensorDev==DUAL_CAMERA_SUB_SENSOR)
        if (subPartId == 1)
            return cust_fillDefaultStrobeNVRam_sub(data);
        else
            return cust_fillDefaultStrobeNVRam_sub_part2(data);
    else if (sensorDev == DUAL_CAMERA_MAIN_2_SENSOR)
        if (mainPartId == 1)
            return cust_fillDefaultStrobeNVRam_main2(data);
        else
            return cust_fillDefaultStrobeNVRam_main2_part2(data);
    else if (sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        if (mainPartId == 1)
            return cust_fillDefaultStrobeNVRam_main3(data);
        else
            return cust_fillDefaultStrobeNVRam_main3_part2(data);
    else //if (sensorDev == DUAL_CAMERA_SUB_2_SENSOR)
        if (subPartId == 1)
            return cust_fillDefaultStrobeNVRam_sub2(data);
        else
            return cust_fillDefaultStrobeNVRam_sub2_part2(data);
}

int cust_fillDefaultFlashCalibrationNVRam(int sensorDev, void* data)
{
    ALOGD("%s(): sensorDev(%d), mainPartId(%d), subPartId(%d).",
            __FUNCTION__, sensorDev, mainPartId, subPartId);

    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR)
        if (mainPartId == 1)
            return cust_fillDefaultFlashCalibrationNVRam_main(data);
        else
            return cust_fillDefaultFlashCalibrationNVRam_main_part2(data);
    else if (sensorDev==DUAL_CAMERA_SUB_SENSOR)
        if (subPartId == 1)
            return cust_fillDefaultFlashCalibrationNVRam_sub(data);
        else
            return cust_fillDefaultFlashCalibrationNVRam_sub_part2(data);
    else if (sensorDev == DUAL_CAMERA_MAIN_2_SENSOR)
        if (mainPartId == 1)
            return cust_fillDefaultFlashCalibrationNVRam_main2(data);
        else
            return cust_fillDefaultFlashCalibrationNVRam_main2_part2(data);
    else if (sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        if (mainPartId == 1)
            return cust_fillDefaultFlashCalibrationNVRam_main3(data);
        else
            return cust_fillDefaultFlashCalibrationNVRam_main3_part2(data);
    else //if (sensorDev == DUAL_CAMERA_SUB_2_SENSOR)
        if (subPartId == 1)
            return cust_fillDefaultFlashCalibrationNVRam_sub2(data);
        else
            return cust_fillDefaultFlashCalibrationNVRam_sub2_part2(data);
}


/***********************************************************
 * flash_tuning_custom_[main|main2|sub|sub2][_part2].cpp
 *
 * Consider variance
 * - LED driver IC
 * - LED component
 **********************************************************/
void cust_getFlashQuick2CalibrationExp(int sensorDev, int* exp, int* afe, int* isp)
{
    ALOGD("%s(): sensorDev(%d), mainPartId(%d), subPartId(%d).",
            __FUNCTION__, sensorDev, mainPartId, subPartId);

    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        if (mainPartId == 1)
            cust_getFlashQuick2CalibrationExp_main(exp, afe, isp);
        else
            cust_getFlashQuick2CalibrationExp_main_part2(exp, afe, isp);
    else
        if (subPartId == 1)
            cust_getFlashQuick2CalibrationExp_sub(exp, afe, isp);
        else
            cust_getFlashQuick2CalibrationExp_sub_part2(exp, afe, isp);
}

void cust_getFlashHalTorchDuty(int sensorDev, int* duty, int* dutyLt)
{
    ALOGD("%s(): sensorDev(%d), mainPartId(%d), subPartId(%d).",
            __FUNCTION__, sensorDev, mainPartId, subPartId);

    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        if (mainPartId == 1)
            cust_getFlashHalTorchDuty_main(duty, dutyLt);
        else
            cust_getFlashHalTorchDuty_main_part2(duty, dutyLt);
    else
        if (subPartId == 1)
            cust_getFlashHalTorchDuty_sub(duty, dutyLt);
        else
            cust_getFlashHalTorchDuty_sub_part2(duty, dutyLt);
}

FlashIMapFP cust_getFlashIMapFunc(int sensorDev)
{
    ALOGD("%s(): sensorDev(%d), mainPartId(%d), subPartId(%d).",
            __FUNCTION__, sensorDev, mainPartId, subPartId);

    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        if (mainPartId == 1)
            return cust_getFlashIMapFunc_main();
        else
            return cust_getFlashIMapFunc_main_part2();
    else
        if (subPartId == 1)
            return cust_getFlashIMapFunc_sub();
        else
            return cust_getFlashIMapFunc_sub_part2();
}

void cust_getFlashITab1(int sensorDev, short* ITab1)
{
    ALOGD("%s(): sensorDev(%d), mainPartId(%d), subPartId(%d).",
            __FUNCTION__, sensorDev, mainPartId, subPartId);

    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        if (mainPartId == 1)
            cust_getFlashITab1_main(ITab1);
        else
            cust_getFlashITab1_main_part2(ITab1);
    else
        if (subPartId == 1)
            cust_getFlashITab1_sub(ITab1);
        else
            cust_getFlashITab1_sub_part2(ITab1);
}

void cust_getFlashITab2(int sensorDev, short* ITab2)
{
    ALOGD("%s(): sensorDev(%d), mainPartId(%d), subPartId(%d).",
            __FUNCTION__, sensorDev, mainPartId, subPartId);

    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        if (mainPartId == 1)
            cust_getFlashITab2_main(ITab2);
        else
            cust_getFlashITab2_main_part2(ITab2);
    else
        if (subPartId == 1)
            cust_getFlashITab2_sub(ITab2);
        else
            cust_getFlashITab2_sub_part2(ITab2);
}

void cust_getFlashMaxIDuty(int sensorDev, int dutyNum, int dutyNumLt, int* duty, int* dutyLt)
{
    ALOGD("%s(): sensorDev(%d), mainPartId(%d), subPartId(%d).",
            __FUNCTION__, sensorDev, mainPartId, subPartId);

    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        if (mainPartId == 1)
            cust_getFlashMaxIDuty_main(dutyNum, dutyNumLt, duty, dutyLt);
        else
            cust_getFlashMaxIDuty_main_part2(dutyNum, dutyNumLt, duty, dutyLt);
    else
        if (subPartId == 1)
            cust_getFlashMaxIDuty_sub(dutyNum, dutyNumLt, duty, dutyLt);
        else
            cust_getFlashMaxIDuty_sub_part2(dutyNum, dutyNumLt, duty, dutyLt);
}

FLASH_PROJECT_PARA& cust_getFlashProjectPara_V2(int sensorDev, int AEScene, NVRAM_CAMERA_STROBE_STRUCT* nvrame)
{
    ALOGD("%s(): sensorDev(%d), mainPartId(%d), subPartId(%d).",
            __FUNCTION__, sensorDev, mainPartId, subPartId);

    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        if (mainPartId == 1)
            return cust_getFlashProjectPara_main(AEScene, 0, nvrame);
        else
            return cust_getFlashProjectPara_main_part2(AEScene, 0, nvrame);
    else
        if (subPartId == 1)
            return cust_getFlashProjectPara_sub(AEScene, 0, nvrame);
        else
            return cust_getFlashProjectPara_sub_part2(AEScene, 0, nvrame);
}

FLASH_PROJECT_PARA& cust_getFlashProjectPara_V3(int sensorDev, int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame)
{
    ALOGD("%s(): sensorDev(%d), mainPartId(%d), subPartId(%d).",
            __FUNCTION__, sensorDev, mainPartId, subPartId);

    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
        sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        if (mainPartId == 1)
            return cust_getFlashProjectPara_main(AEScene, isForceFlash, nvrame);
        else
            return cust_getFlashProjectPara_main_part2(AEScene, isForceFlash, nvrame);
    else
        if (subPartId == 1)
            return cust_getFlashProjectPara_sub(AEScene, isForceFlash, nvrame);
        else
            return cust_getFlashProjectPara_sub_part2(AEScene, isForceFlash, nvrame);
}

