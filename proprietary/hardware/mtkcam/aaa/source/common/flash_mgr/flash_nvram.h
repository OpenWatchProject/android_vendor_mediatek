#pragma once

/* custom headers */
#include "camera_custom_nvram.h"

class FlashNvram
{
    public:
		static int nvReadStrobeDefault(NVRAM_CAMERA_STROBE_STRUCT *&buf, int sensorDev);
        static int nvReadStrobe(NVRAM_CAMERA_STROBE_STRUCT *&buf, int sensorDev, int isForceRead = 0);
        static int nvWriteStrobe(int sensorDev);
        static int nvRead3ADefault(NVRAM_CAMERA_3A_STRUCT *&buf, int sensorDev);
        static int nvRead3A(NVRAM_CAMERA_3A_STRUCT *&buf, int sensorDev, int isForceRead = 0);
        static int nvWrite3A(int sensorDev);
        static int nvReadFlashCaliDefault(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *&buf, int sensorDev);
        static int nvReadFlashCali(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *&buf, int sensorDev, int isForceRead = 0);
        static int nvWriteFlashCali(int sensorDev);
};

