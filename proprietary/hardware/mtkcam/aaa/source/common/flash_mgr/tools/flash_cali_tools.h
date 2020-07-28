#pragma once


/***********************************************************
 * Redefinition for Non-Android
 **********************************************************/

/* custom/[PLATFORM]/hal/inc/MediaTypes.h */
#define INT32               signed int
typedef unsigned char       UINT8;

/* custom/[PLATFORM]/hal/inc/[VERSION]/CFG_Camera_File_Max_Size.h */
#define MAXIMUM_NVRAM_CAMERA_FLASH_CALIBRATION_FILE_SIZE   (12800)

/* custom/[PLATFORM]/hal/inc/[VERSION]/camera_custom_nvram.h */
#define FLASH_CUSTOM_MAX_DUTY_NUM_HT (40) // Note, related to NVRAM spec
#define FLASH_CUSTOM_MAX_DUTY_NUM_LT (40) // Note, related to NVRAM spec
#define FLASH_CUSTOM_MAX_DUTY_NUM (FLASH_CUSTOM_MAX_DUTY_NUM_HT * FLASH_CUSTOM_MAX_DUTY_NUM_LT)

typedef struct {
	INT32 i4R; // R gain
	INT32 i4G; // G gain
	INT32 i4B; // B gain
} AWB_GAIN_T;

typedef struct {
    int exp;
    int afe_gain;
    int isp_gain;
    int distance;
    short yTab[FLASH_CUSTOM_MAX_DUTY_NUM];
} NVRAM_FLASH_CCT_ENG_TABLE;

typedef union {
	struct {
		short yTab[FLASH_CUSTOM_MAX_DUTY_NUM];
		AWB_GAIN_T flashWBGain[FLASH_CUSTOM_MAX_DUTY_NUM];
	};
	UINT8 temp[MAXIMUM_NVRAM_CAMERA_FLASH_CALIBRATION_FILE_SIZE];
} NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT, *PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT;


/* device/mediatek/common/kernel-headers/kd_camera_feature.h */
typedef enum {
	DUAL_CAMERA_NONE_SENSOR        = 0,
	DUAL_CAMERA_MAIN_SENSOR        = 1,
	DUAL_CAMERA_SUB_SENSOR         = 2,
	DUAL_CAMERA_MAIN_2_SENSOR      = 4,
	DUAL_CAMERA_SENSOR_MAX,
	/* for backward compatible */
	DUAL_CAMERA_MAIN_SECOND_SENSOR = 4,
	DUAL_CAMERA_SUB_2_SENSOR       = 8,
} CAMERA_DUAL_CAMERA_SENSOR_ENUM;

/* mtkcam/aaa/source/[PLATFORM]/flash_mgr/flash_cct.h */
#include <deque>
#define CCT_FLASH_CALCULATE_FRAME 2
class CaliData
{
    public:
        int duty;
        int dutyLT;
        float ref;
        int exp;
        int afe;
        int isp;
        float y[CCT_FLASH_CALCULATE_FRAME];
        float r[CCT_FLASH_CALCULATE_FRAME];
        float g[CCT_FLASH_CALCULATE_FRAME];
        float b[CCT_FLASH_CALCULATE_FRAME];
        int t[CCT_FLASH_CALCULATE_FRAME];
};
typedef std::deque<CaliData> dqCaliData;


/***********************************************************
 * Tools
 **********************************************************/
#define logD(fmt, arg...) do {printf(fmt"\n", ##arg);} while (0)
#define logI(fmt, arg...) do {printf(fmt"\n", ##arg);} while (0)
#define logE(fmt, arg...) do {printf(fmt"\n", ##arg);} while (0)

