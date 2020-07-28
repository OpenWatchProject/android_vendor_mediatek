#define LOG_TAG "FlashNvram"

#include "flash_nvram.h"

/* aaa common headers */
#include "log_utils.h"
#include "property_utils.h"

/* aaa headers */
#include "nvbuf_util.h"

/* custom headers */
#include "camera_custom_nvram.h"

#include "tools/flash_cali_utils.h"

#define PROP_FLASH_GOLDEN_DATA_DUMP_EN "vendor.flash_golden_data_dump_en"


/***********************************************************
 * NVRAM_CAMERA_STROBE_STRUCT
 **********************************************************/
int FlashNvram::nvReadStrobeDefault(NVRAM_CAMERA_STROBE_STRUCT *&buf, int sensorDev)
{
    logI("nvReadStrobeDefault().");

    int err = NvBufUtil::getInstance().readDefault(CAMERA_NVRAM_DATA_STROBE, sensorDev);
    if (err)
        logE("nvReadStrobeDefault(): readDefault failed(%d).", err);
    err = NvBufUtil::getInstance().getBuf(CAMERA_NVRAM_DATA_STROBE, sensorDev, (void*&)buf);
    if (err)
        logE("nvReadStrobeDefault(): getBuf failed(%d).", err);
    return err;
}

int FlashNvram::nvReadStrobe(NVRAM_CAMERA_STROBE_STRUCT *&buf, int sensorDev, int isForceRead)
{
    logI("nvReadStrobe().");

    int err = NvBufUtil::getInstance().getBufAndRead(
            CAMERA_NVRAM_DATA_STROBE, sensorDev, (void *&)buf, isForceRead);
    if (err)
        logE("nvReadStrobe(): failed(%d).", err);
    return err;
}

int FlashNvram::nvWriteStrobe(int sensorDev)
{
    logI("nvWriteStrobe().");

    int err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_STROBE, sensorDev);
    if (err)
        logE("nvWriteStrobe(): failed(%d).", err);
    return err;
}

/***********************************************************
 * NVRAM_CAMERA_3A_STRUCT
 **********************************************************/
int FlashNvram::nvRead3ADefault(NVRAM_CAMERA_3A_STRUCT *&buf, int sensorDev)
{
    logI("nvRead3ADefault().");

    int err = NvBufUtil::getInstance().readDefault(CAMERA_NVRAM_DATA_3A, sensorDev);
    if (err)
        logE("nvRead3ADefault(): readDefault failed(%d).", err);
    err = NvBufUtil::getInstance().getBuf(CAMERA_NVRAM_DATA_3A, sensorDev, (void*&)buf);
    if (err)
        logE("nvRead3ADefault(): getBuf failed(%d).", err);
    return err;
}

int FlashNvram::nvRead3A(NVRAM_CAMERA_3A_STRUCT *&buf, int sensorDev, int isForceRead)
{
    logI("nvRead3A().");

    int err = NvBufUtil::getInstance().getBufAndRead(
            CAMERA_NVRAM_DATA_3A, sensorDev, (void *&)buf, isForceRead);
    if (err)
        logE("nvRead3A(): failed(%d).", err);
    return err;
}

int FlashNvram::nvWrite3A(int sensorDev)
{
    logI("nvWrite3A().");

    int err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_3A, sensorDev);
    if (err)
        logE("nvWrite3A(): failed(%d).", err);
    return err;
}

/***********************************************************
 * NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT
 **********************************************************/
int FlashNvram::nvReadFlashCaliDefault(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *&buf, int sensorDev)
{
    logI("nvReadFlashCaliDefault().");

    int err = NvBufUtil::getInstance().readDefault(CAMERA_NVRAM_DATA_FLASH_CALIBRATION, sensorDev);
    if (err)
        logE("nvReadFlashCaliDefault(): readDefault failed(%d).", err);
    err = NvBufUtil::getInstance().getBuf(CAMERA_NVRAM_DATA_FLASH_CALIBRATION, sensorDev, (void*&)buf);
    if (err)
        logE("nvReadFlashCaliDefault(): getBuf failed(%d).", err);

    int iGoldenDataDumpEn = 0;
    getPropInt(PROP_FLASH_GOLDEN_DATA_DUMP_EN, &iGoldenDataDumpEn, 0);
    if (iGoldenDataDumpEn)
        showFlashCali(buf);
    return err;
}

int FlashNvram::nvReadFlashCali(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *&buf, int sensorDev, int isForceRead)
{
    logI("nvReadFlashCali().");

    int err = NvBufUtil::getInstance().getBufAndRead(
            CAMERA_NVRAM_DATA_FLASH_CALIBRATION, sensorDev, (void *&)buf, isForceRead);
    if (err)
        logE("nvReadFlashCali(): failed(%d).", err);
    return err;
}

int FlashNvram::nvWriteFlashCali(int sensorDev)
{
    logI("nvWriteFlashCali().");

    int err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_FLASH_CALIBRATION, sensorDev);
    if (err)
        logE("nvWriteFlashCali(): failed(%d).", err);
    return err;
}


