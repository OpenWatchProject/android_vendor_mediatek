#define LOG_TAG "flash_custom.cpp"
#define MTK_LOG_ENABLE 1
#include "camera_custom_nvram.h"
#include "camera_custom_types.h"
#include <custom/aaa/AEPlinetable.h>
#include "camera_custom_nvram.h"
#include <log/log.h>
#include "flash_feature.h"
#include "flash_param.h"
#include "strobe_param.h"
#include "flash_tuning_custom.h"
#include <kd_camera_feature.h>


static int g_mainId=1;
static int g_subId=1;
static int g_main_secondId=1;
static int g_main_thirdId=1;
static int g_sub_secondId=1;
static int g_default=1;
static int g_cam_mode=0;


void cust_setFlashPartId_main(int id)
{
    g_mainId=id;
}
void cust_setFlashPartId_sub(int id)
{
    g_subId=id;
}

void cust_setFlashPartId(int sensorDev, int id)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
        g_mainId=id;
    else if(sensorDev==DUAL_CAMERA_SUB_SENSOR)
        g_subId=id;
    else if(sensorDev==DUAL_CAMERA_MAIN_2_SENSOR)
        g_main_secondId=id;
    else if(sensorDev==DUAL_CAMERA_MAIN_3_SENSOR)
        g_main_thirdId=id;
    else if(sensorDev==DUAL_CAMERA_SUB_2_SENSOR)
        g_sub_secondId=id;
    else
        g_default=id;
}
//FLASH_PROJECT_PARA& cust_getFlashProjectPara_mainV2(int AEMode, NVRAM_CAMERA_STROBE_STRUCT* nvrame)
FLASH_PROJECT_PARA& cust_getFlashProjectPara_V2(int sensorDev, int AEScene, NVRAM_CAMERA_STROBE_STRUCT* nvrame)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR || sensorDev == DUAL_CAMERA_MAIN_2_SENSOR || sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
    {
        if(g_mainId==1)
            return cust_getFlashProjectPara(AEScene, 0, nvrame);
        else //if(id==2)
            return cust_getFlashProjectPara_main2(AEScene, 0, nvrame);
    }
    else
    {
        if(g_subId==1)
            return cust_getFlashProjectPara_sub(AEScene, 0, nvrame);
        else //if(id==2)
            return cust_getFlashProjectPara_sub2(AEScene, 0, nvrame);
    }
}


FlashIMapFP cust_getFlashIMapFunc_main();
FlashIMapFP cust_getFlashIMapFunc_sub();
FlashIMapFP cust_getFlashIMapFunc_main2();
FlashIMapFP cust_getFlashIMapFunc_sub2();
FlashIMapFP cust_getFlashIMapFunc(int sensorDev)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR || sensorDev == DUAL_CAMERA_MAIN_2_SENSOR || sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
    {
        if(g_mainId==1)
            return cust_getFlashIMapFunc_main();
        else //if(id==2)
            return cust_getFlashIMapFunc_main2();
    }
    else
    {
        if(g_subId==1)
            return cust_getFlashIMapFunc_sub();
        else //if(id==2)
            return cust_getFlashIMapFunc_sub2();
    }
}


FLASH_PROJECT_PARA& cust_getFlashProjectPara_V3(int sensorDev, int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR || sensorDev == DUAL_CAMERA_MAIN_2_SENSOR || sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
    {
        if(g_mainId==1)
            return cust_getFlashProjectPara(AEScene, isForceFlash, nvrame);
        else //if(id==2)
            return cust_getFlashProjectPara_main2(AEScene, isForceFlash, nvrame);
    }
    else
    {
        if(g_subId==1)
            return cust_getFlashProjectPara_sub(AEScene, isForceFlash, nvrame);
        else //if(id==2)
            return cust_getFlashProjectPara_sub2(AEScene, isForceFlash, nvrame);
    }
}

int cust_getDefaultStrobeNVRam_V2(int sensorDev, void* data, int* ret_size)
{
#if 1
    (void)sensorDev;
    (void)data;
    (void)ret_size;
#else
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
    {
        if(g_mainId==1)
        {
            XLOGD("devid main id1");
            return getDefaultStrobeNVRam(DUAL_CAMERA_MAIN_SENSOR, data, ret_size);
        }
        else
        {
            XLOGD("devid main id2");
            return getDefaultStrobeNVRam_main2(data, ret_size);
        }
    }
    else
    {
        if(g_subId==1)
        {
            XLOGD("devid sub id1");
            return getDefaultStrobeNVRam(DUAL_CAMERA_SUB_SENSOR, data, ret_size);
        }
        else
        {
            XLOGD("devid sub id2");
            return getDefaultStrobeNVRam_sub2(data, ret_size);
        }
    }
#endif
    return 0;
}

int cust_fillDefaultStrobeNVRam(int sensorDev, void* data)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
    {
        if(g_mainId==1)
        {
            ALOGD("devid main id1");
            return cust_fillDefaultStrobeNVRam_main(data);
        }
        else
        {
            ALOGD("devid main id2");
            return cust_fillDefaultStrobeNVRam_main2(data);
        }
    }
    else if(sensorDev==DUAL_CAMERA_SUB_SENSOR)
    {
        if(g_subId==1)
        {
            ALOGD("devid sub id1");
            return cust_fillDefaultStrobeNVRam_sub(data);
        }
        else
        {
            ALOGD("devid sub id2");
            return cust_fillDefaultStrobeNVRam_sub2(data);
        }
    }
    else if(sensorDev==DUAL_CAMERA_MAIN_2_SENSOR)
    {
        if(g_main_secondId==1)
        {
            ALOGD("devid main_second id1");
            return cust_fillDefaultStrobeNVRam_main_second(data);
        }
        else
        {
            ALOGD("devid main_second id2");
            return cust_fillDefaultStrobeNVRam_main_second2(data);
        }
    }
    else if(sensorDev==DUAL_CAMERA_MAIN_3_SENSOR)
    {
        if(g_main_thirdId==1)
        {
            ALOGD("devid main_second id1");
            return cust_fillDefaultStrobeNVRam_main_third(data);
        }
        else
        {
            ALOGD("devid main_second id2");
            return cust_fillDefaultStrobeNVRam_main_third2(data);
        }
    }
    else if(sensorDev==DUAL_CAMERA_SUB_2_SENSOR)
    {
        if(g_sub_secondId==1)
        {
            ALOGD("devid sub_second id1");
            return cust_fillDefaultStrobeNVRam_sub_second(data);
        }
        else
        {
            ALOGD("devid sub_second id2");
            return cust_fillDefaultStrobeNVRam_sub_second2(data);
        }
    }
    else //default
    {
        if(g_default==1)
        {
            ALOGD("devid default id1");
            return cust_fillDefaultStrobeNVRam_default(data);
        }
        else
        {
            ALOGD("devid default id2");
            return cust_fillDefaultStrobeNVRam_default2(data);
        }
    }

}

int cust_fillDefaultFlashCalibrationNVRam(int sensorDev, void* data)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
    {
        if(g_mainId==1)
        {
            return cust_fillDefaultFlashCalibrationNVRam_main(data);
        }
        else
        {
            return cust_fillDefaultFlashCalibrationNVRam_main2(data);
        }
    }
    else if(sensorDev==DUAL_CAMERA_SUB_SENSOR)
    {
        if(g_subId==1)
        {
            return cust_fillDefaultFlashCalibrationNVRam_sub(data);
        }
        else
        {
            return cust_fillDefaultFlashCalibrationNVRam_sub2(data);
        }
    }
    else if(sensorDev==DUAL_CAMERA_MAIN_2_SENSOR)
    {
        if(g_main_secondId==1)
        {
            return cust_fillDefaultFlashCalibrationNVRam_main_second(data);
        }
        else
        {
            return cust_fillDefaultFlashCalibrationNVRam_main_second2(data);
        }
    }
    else if(sensorDev==DUAL_CAMERA_MAIN_3_SENSOR)
    {
        if(g_main_thirdId==1)
        {
            return cust_fillDefaultFlashCalibrationNVRam_main_third(data);
        }
        else
        {
            return cust_fillDefaultFlashCalibrationNVRam_main_third2(data);
        }
    }
    else if(sensorDev==DUAL_CAMERA_SUB_2_SENSOR)
    {
        if(g_sub_secondId==1)
        {
            return cust_fillDefaultFlashCalibrationNVRam_sub_second(data);
        }
        else
        {
            return cust_fillDefaultFlashCalibrationNVRam_sub_second2(data);
        }
    }
    else //default
    {
        if(g_default==1)
        {
            return cust_fillDefaultFlashCalibrationNVRam_default(data);
        }
        else
        {
            return cust_fillDefaultFlashCalibrationNVRam_default2(data);
        }
    }

}
int cust_isDualFlashSupport(int sensorDev)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR || sensorDev == DUAL_CAMERA_MAIN_2_SENSOR || sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
        return 1;
    else
        return 0;
}

void cust_getFlashQuick2CalibrationExp(int sensorDev, int* exp, int* afe, int* isp)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR || sensorDev == DUAL_CAMERA_MAIN_2_SENSOR || sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
    {
        if(g_mainId==1)
        {
            ALOGD("cust_getFlashQuick2CalibrationExp devid main id1");
            cust_getFlashQuick2CalibrationExp_main(exp, afe, isp);
        }
        else
        {
            ALOGD("cust_getFlashQuick2CalibrationExp devid main id2");
            cust_getFlashQuick2CalibrationExp_main2(exp, afe, isp);
        }
    }
    else
    {
        if(g_subId==1)
        {
            ALOGD("cust_getFlashQuick2CalibrationExp devid sub id1");
            cust_getFlashQuick2CalibrationExp_sub(exp, afe, isp);
        }
        else
        {
            ALOGD("cust_getFlashQuick2CalibrationExp devid sub id2");
            cust_getFlashQuick2CalibrationExp_sub2(exp, afe, isp);
        }
    }
}
void cust_getFlashITab2(int sensorDev, short* ITab2)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR || sensorDev == DUAL_CAMERA_MAIN_2_SENSOR || sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
    {
        if(g_mainId==1)
        {
            ALOGD("cust_getFlashITab2 devid main id1");
            cust_getFlashITab2_main(ITab2);
        }
        else
        {
            ALOGD("cust_getFlashITab2 devid main id2");
            cust_getFlashITab2_main2(ITab2);
        }
    }
    else
    {
        if(g_subId==1)
        {
            ALOGD("cust_getFlashITab2 devid sub id1");
            cust_getFlashITab2_sub(ITab2);
        }
        else
        {
            ALOGD("cust_getFlashITab2 devid sub id2");
            cust_getFlashITab2_sub2(ITab2);
        }
    }
}
void cust_getFlashITab1(int sensorDev, short* ITab1)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR || sensorDev == DUAL_CAMERA_MAIN_2_SENSOR || sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
    {
        if(g_mainId==1)
        {
            ALOGD("cust_getFlashITab1 devid main id1");
            cust_getFlashITab1_main(ITab1);
        }
        else
        {
            ALOGD("cust_getFlashITab1 devid main id2");
            cust_getFlashITab1_main2(ITab1);
        }
    }
    else
    {
        if(g_subId==1)
        {
            ALOGD("cust_getFlashITab1 devid sub id1");
            cust_getFlashITab1_sub(ITab1);
        }
        else
        {
            ALOGD("cust_getFlashITab1 devid sub id2");
            cust_getFlashITab1_sub2(ITab1);
        }
    }
}

void cust_getFlashMaxIDuty(int sensorDev, int dutyNum, int dutyNumLt, int* duty, int* dutyLt)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR || sensorDev == DUAL_CAMERA_MAIN_2_SENSOR || sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
    {
        if(g_mainId==1)
        {
            ALOGD("cust_getFlashMaxIDuty devid main id1");
            cust_getFlashMaxIDuty_main(dutyNum, dutyNumLt, duty, dutyLt);
        }
        else
        {
            ALOGD("cust_getFlashMaxIDuty devid main id2");
            cust_getFlashMaxIDuty_main2(dutyNum, dutyNumLt, duty, dutyLt);
        }
    }
    else
    {
        if(g_subId==1)
        {
            ALOGD("cust_getFlashMaxIDuty devid sub id1");
            cust_getFlashMaxIDuty_sub(dutyNum, dutyNumLt, duty, dutyLt);
        }
        else
        {
            ALOGD("cust_getFlashMaxIDuty devid sub id2");
            cust_getFlashMaxIDuty_sub2(dutyNum, dutyNumLt, duty, dutyLt);
        }
    }
}

void cust_getFlashHalTorchDuty(int sensorDev, int* duty, int* dutyLt)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR || sensorDev == DUAL_CAMERA_MAIN_2_SENSOR || sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
    {
        if(g_mainId==1)
        {
            ALOGD("cust_getFlashHalTorchDuty devid main id1");
            cust_getFlashHalTorchDuty_main(duty, dutyLt);
        }
        else
        {
            ALOGD("cust_getFlashHalTorchDuty devid main id2");
            cust_getFlashHalTorchDuty_main2(duty, dutyLt);
        }
    }
    else
    {
        if(g_subId==1)
        {
            ALOGD("cust_getFlashHalTorchDuty devid sub id1");
            cust_getFlashHalTorchDuty_sub(duty, dutyLt);
        }
        else
        {
            ALOGD("cust_getFlashHalTorchDuty devid sub id2");
            cust_getFlashHalTorchDuty_sub2(duty, dutyLt);
        }
    }
}

int cust_getFlashFrameRateDelay_main(){return 1;}
int cust_getFlashFrameRateDelay_sub(){return 0;}
int cust_getFlashFrameRateDelay_main2(){return 0;}
int cust_getFlashFrameRateDelay_sub2(){return 0;}
int cust_getFlashFrameRateDelay(int sensorDev)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
    {
        if(g_mainId==1)
        {
            ALOGD("cust_getFlashFrameRateDelay devid main id1");
            return cust_getFlashFrameRateDelay_main();
        }
        else
        {
            ALOGD("cust_getFlashFrameRateDelay devid main id2");
            return cust_getFlashFrameRateDelay_main2();
        }
    }
    else
    {
        if(g_subId==1)
        {
            ALOGD("cust_getFlashFrameRateDelay devid sub id1");
            return cust_getFlashFrameRateDelay_sub();
        }
        else
        {
            ALOGD("cust_getFlashFrameRateDelay devid sub id2");
            return cust_getFlashFrameRateDelay_sub2();
        }
    }
}

int cust_getFlashSttStableFrame_main(){return 0;}
int cust_getFlashSttStableFrame_sub(){return 0;}
int cust_getFlashSttStableFrame_main2(){return 0;}
int cust_getFlashSttStableFrame_sub2(){return 1;}
int cust_getFlashSttStableFrame(int sensorDev)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
    {
        if(g_mainId==1)
        {
            ALOGD("cust_getFlashSttStableFrame devid main id1");
            return cust_getFlashSttStableFrame_main();
        }
        else
        {
            ALOGD("cust_getFlashSttStableFrame devid main id2");
            return cust_getFlashSttStableFrame_main2();
        }
    }
    else
    {
        if(g_subId==1)
        {
            ALOGD("cust_getFlashSttStableFrame devid sub id1");
            return cust_getFlashSttStableFrame_sub();
        }
        else
        {
            ALOGD("cust_getFlashSttStableFrame devid sub id2");
            return cust_getFlashSttStableFrame_sub2();
        }
    }
}

bool cust_isDoPreCapInFlashAF(int sensorDev)
{
    if(g_cam_mode == 3) // byPass EM mode
    {
        return false;
    }

    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
    {
        return false;
    }
    else
    {
        return false;
    }
}

// cam mode enum in <isp_tuning.h>
// typedef enum
// {
//     EOperMode_Normal    = 0,
//     EOperMode_PureRaw,
//     EOperMode_Meta,
//     EOperMode_EM
// } EOperMode_T;
void cust_setFlashPartCamMode(int camMode)
{
    ALOGD("cust_setFlashPartCamMode prev_mode(%d), new_mode(%d)", g_cam_mode, camMode);
    g_cam_mode = camMode;
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

int cust_getTorchDuty(int sensorDev, int level, int* duty, int* dutyLt)
{
    (void)sensorDev;
    if (level == 0) {
        *duty = 0;
        *dutyLt = 0;
    }
    else if (level == 1) {
        *duty = 6;
        *dutyLt = 0;
    }
    else if (level == 2) {
        *duty = 12;
        *dutyLt = 0;
    }
    else{
        *duty = 6;
        *dutyLt = 0;
    }
    return 0;
}

