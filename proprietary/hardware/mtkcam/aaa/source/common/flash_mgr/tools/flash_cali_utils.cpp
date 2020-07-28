#define LOG_TAG "FlashCaliUtils"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "flash_custom_utils.h"
#include "flash_custom_adapter.h"

#ifdef LINUX
#include "flash_cali_tools.h"
#else
#include "camera_custom_nvram.h"
#include "flash_utils.h"
#endif

int writeFlashCali(const char *fname, PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash)
{
    /* verify arguments */
    if (!fname) {
        logE("writeFlashCali(): invalid arguments.");
        return -1;
    }

    logI("writeFlashCali().");

    /* set creation mask */
    int mask = umask(0);

    /* write data */
    FILE *fp = fopen(fname, "wb");
    if (!fp) {
        logE("writeFlashCali(): failed to open.");
        umask(mask);
        return -1;
    }
    fwrite(nvFlash, 1, sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT), fp);
    fclose(fp);

    /* restore mask */
    umask(mask);

    return 0;
}

int readFlashCali(const char *fname, PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash)
{
    /* verify arguments */
    if (!fname) {
        logE("readFlashCali(): invalid arguments.");
        return -1;
    }

    logI("readFlashCali().");

    /* set creation mask */
    int mask = umask(0);

    /* read data */
    FILE *fp = fopen(fname, "rb");
    if (!fp) {
        logE("readFlashCali(): failed to open.");
        umask(mask);
        return -1;
    }
    int ret = fread(nvFlash, 1, sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT), fp);
    if (ret != (int)sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT)) {
        logE("readFlashCali(): failed to read.");
        fclose(fp);
        umask(mask);
        return -1;
    }
    fclose(fp);

    /* restore mask */
    umask(mask);

    return 0;
}

int showFlashCali(PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash)
{
    /* verify arguments */
    if (!nvFlash) {
        logE("showFlashCali(): invalid arguments.");
        return -1;
    }

    logI("showFlashCali().");
#if (!CAM3_3A_ISP_30_EN && !CAM3_3A_ISP_40_EN)
    dumpYTab(nvFlash->Flash_Calibration[0].engTab.yTab,
            FLASH_CUSTOM_MAX_DUTY_NUM_HT,
            FLASH_CUSTOM_MAX_DUTY_NUM_LT);
    logI("showFlashCali(): flashWbGain:");
    dumpFwbGain(nvFlash->Flash_Calibration[0].flashWBGain,
            FLASH_CUSTOM_MAX_DUTY_NUM_HT,
            FLASH_CUSTOM_MAX_DUTY_NUM_LT);
#else
    dumpYTab(nvFlash->yTab,
            FLASH_CUSTOM_MAX_DUTY_NUM_HT,
            FLASH_CUSTOM_MAX_DUTY_NUM_LT);
    logI("showFlashCali(): flashWbGain:");
    dumpFwbGain(nvFlash->flashWBGain,
            FLASH_CUSTOM_MAX_DUTY_NUM_HT,
            FLASH_CUSTOM_MAX_DUTY_NUM_LT);
#endif
    return 0;
}

int showFlashCali(PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash, int dutyNum, int dutyNumLt)
{
    /* verify arguments */
    if (!nvFlash) {
        logE("showFlashCali(): invalid arguments.");
        return -1;
    }

    /* duty index include off status if dual */
    int dutyIndex = dutyNum;
    int dutyLtIndex = dutyNumLt;
    if (dutyNumLt) {
        dutyIndex++;
        dutyLtIndex++;
    }

#if (!CAM3_3A_ISP_30_EN && !CAM3_3A_ISP_40_EN)
    logI("showFlashCali().");
    dumpYTab(nvFlash->Flash_Calibration[0].engTab.yTab, dutyIndex, dutyLtIndex);

    logI("showFlashCali(): flashWbGain:");
    dumpFwbGain(nvFlash->Flash_Calibration[0].flashWBGain, dutyIndex, dutyLtIndex);
#else
    logI("showFlashCali().");
    dumpYTab(nvFlash->yTab, dutyIndex, dutyLtIndex);

    logI("showFlashCali(): flashWbGain:");
    dumpFwbGain(nvFlash->flashWBGain, dutyIndex, dutyLtIndex);
#endif
    return 0;
}
