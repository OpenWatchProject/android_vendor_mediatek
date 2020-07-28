#pragma once

#ifdef LINUX
#include "flash_cali_tools.h"
#else
#include "camera_custom_nvram.h"
#endif

int writeFlashCali(const char *fname, PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash);
int readFlashCali(const char *fname, PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash);
int showFlashCali(PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash);
int showFlashCali(PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash, int dutyNum, int dutyNumLt);
