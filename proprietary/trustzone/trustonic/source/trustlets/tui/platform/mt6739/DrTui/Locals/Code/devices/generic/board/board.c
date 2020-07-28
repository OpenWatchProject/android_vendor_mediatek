/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
/**
 * Low level GENERIC platform
 */


#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "device.h"
#include "drTuiHal.h"

#define TAG "[BOARD] "

uint32_t tuiHalGetVersion(void)
{
    drApiLogPrintf("TUI HAL version %u.%u build %s, %s\n",
            TUI_HAL_VERSION_MAJOR, TUI_HAL_VERSION_MINOR, __DATE__,  __TIME__);

    return TUI_HAL_VERSION(TUI_HAL_VERSION_MAJOR, TUI_HAL_VERSION_MINOR);
}


int map_i2c_hw_register(void);

drTuiHalError_t tuiHalBoardInit(void) {
    drApiLogPrintf("==== tuiHalBoardInit()+ ====");
    map_i2c_hw_register();
    start_devapc();
    drApiLogPrintf("==== tuiHalBoardInit()- ====");
    return TUI_HAL_OK;
}

drTuiHalError_t tuiHalHandleNwdMessage(uint32_t value)
{
    return DRAPI_OK;
}
