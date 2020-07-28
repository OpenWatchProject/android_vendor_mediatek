#define LOG_TAG "flash_custom_1.cpp"
#define MTK_LOG_ENABLE 1

#include <cutils/log.h>
#include "mtkcam/def/BuiltinTypes.h"
#include "kd_camera_feature.h"
#include "flash_feature.h"


int cust_getFlashModeStyle(int sensorDev, int flashMode)
{
    (void)sensorDev;

    /*
     * <kd_camera_feature.h>
     * typedef enum {
     *     DUAL_CAMERA_NONE_SENSOR        = 0,
     *     DUAL_CAMERA_MAIN_SENSOR        = 1,
     *     DUAL_CAMERA_SUB_SENSOR         = 2,
     *     DUAL_CAMERA_MAIN_2_SENSOR      = 4,
     *     DUAL_CAMERA_SENSOR_MAX,
     *     DUAL_CAMERA_MAIN_SECOND_SENSOR = 4,
     *     DUAL_CAMERA_SUB_2_SENSOR       = 8,
     * } CAMERA_DUAL_CAMERA_SENSOR_ENUM;
     */

    /*
     * <flash_feature.h>
     * typedef enum
     * {
     *     LIB3A_FLASH_MODE_UNSUPPORTED = -1,
     *     LIB3A_FLASH_MODE_AUTO        =  0,
     *     LIB3A_FLASH_MODE_SLOWSYNC    =  0,
     *     LIB3A_FLASH_MODE_FORCE_ON    =  1,
     *     LIB3A_FLASH_MODE_FORCE_OFF   =  2,
     *     LIB3A_FLASH_MODE_REDEYE      =  3,
     *     LIB3A_FLASH_MODE_FORCE_TORCH =  4,
     *     LIB3A_FLASH_MODE_TOTAL_NUM,
     *     LIB3A_FLASH_MODE_MIN = LIB3A_FLASH_MODE_AUTO,
     *     LIB3A_FLASH_MODE_MAX = LIB3A_FLASH_MODE_FORCE_TORCH,
     * } LIB3A_FLASH_MODE_T;
     *
     * enum {
     *     e_FLASH_STYLE_OFF_AUTO = 0,
     *     e_FLASH_STYLE_OFF_ON,
     *     e_FLASH_STYLE_OFF_OFF,
     *     e_FLASH_STYLE_ON_ON,
     *     e_FLASH_STYLE_ON_TORCH,
     * };
     */

    if (flashMode == LIB3A_FLASH_MODE_AUTO)
		return e_FLASH_STYLE_OFF_AUTO;
	else if (flashMode == LIB3A_FLASH_MODE_FORCE_ON)
		return e_FLASH_STYLE_OFF_ON;
		//return e_FLASH_STYLE_ON_ON;
	else if (flashMode == LIB3A_FLASH_MODE_FORCE_OFF)
		return e_FLASH_STYLE_OFF_OFF;
	else if (flashMode == LIB3A_FLASH_MODE_REDEYE)
		return e_FLASH_STYLE_OFF_AUTO;
	else if (flashMode == LIB3A_FLASH_MODE_FORCE_TORCH)
		return e_FLASH_STYLE_ON_ON;
	return e_FLASH_STYLE_OFF_AUTO;
}

int cust_getVideoFlashModeStyle(int sensorDev, int flashMode)
{
    (void)sensorDev;

    if (flashMode == LIB3A_FLASH_MODE_AUTO)
        return e_FLASH_STYLE_OFF_AUTO;
    else if (flashMode == LIB3A_FLASH_MODE_FORCE_ON)
        return e_FLASH_STYLE_ON_ON;
    else if (flashMode == LIB3A_FLASH_MODE_FORCE_OFF)
        return e_FLASH_STYLE_OFF_OFF;
    else if (flashMode == LIB3A_FLASH_MODE_REDEYE)
        return e_FLASH_STYLE_OFF_OFF;
    else if (flashMode == LIB3A_FLASH_MODE_FORCE_TORCH)
        return e_FLASH_STYLE_ON_ON;
    return e_FLASH_STYLE_OFF_OFF;
}

float evX[5] = {-3, -1.5, 0, 1.5, 3};
float evY[5] = {-2,   -1, 0,   1, 2};
float evL[5] = { 0,    0, 0,   3, 5};
void cust_getEvCompPara(int& maxEvTar10Bit, int& indNum, float*& evIndTab, float*& evTab, float*& evLevel)
{
    maxEvTar10Bit = 600;
    indNum = 5;
    evIndTab = evX;
    evTab = evY;
    evLevel = evL;
}

int cust_isNeedAFLamp(int flashMode, int afLampMode, int isBvHigherTriger)
{
    (void)afLampMode;

    /*
     * <kd_camera_feature_enum.h>
     * FID_TO_TYPE_ENUM(
     *     FID_AE_STROBE,
     *     FTYPE_ENUM(
     *         FLASHLIGHT_BEGIN = 0,
     *         FLASHLIGHT_AUTO = FLASHLIGHT_BEGIN,
     *         FLASHLIGHT_FORCE_ON,
     *         FLASHLIGHT_FORCE_OFF,
     *         FLASHLIGHT_REDEYE,
     *         FLASHLIGHT_TORCH,
     *         NUM_OF_FLASHLIGHT
     *     )
     * ) AE_STROBE_T;
     *
     * FID_TO_TYPE_ENUM(
     *     FID_AF_LAMP,
     *     FTYPE_ENUM(
     *         AF_LAMP_BEGIN = 0,
     *         AF_LAMP_OFF = AF_LAMP_BEGIN,
     *         AF_LAMP_ON,
     *         AF_LAMP_AUTO,
     *         AF_LAMP_FLASH,
     *         NUM_OF_AF_LAMP
     *     )
     * ) AF_LAMP_T;
     */

	if (flashMode == FLASHLIGHT_FORCE_OFF)
		return 0;
	if (flashMode == FLASHLIGHT_FORCE_ON)
		return 1;
	//if(afLampMode == AF_LAMP_OFF)
	//	return 0;
	//if(afLampMode == AF_LAMP_ON)
	//	return 1;
	if (isBvHigherTriger == 1)
		return 1;
	else
		return 0;
}

