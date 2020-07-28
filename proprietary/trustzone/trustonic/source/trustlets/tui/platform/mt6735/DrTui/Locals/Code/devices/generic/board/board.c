/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
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
#include "log.h"

int map_i2c_hw_register(void);

drTuiHalError_t tuiHalBoardInit(void) {
    //return E_TUI_HAL_INTERNAL;
    printf("==== tuiHalBoardInit()+ ====");

	map_i2c_hw_register();
    start_devapc();

    printf("==== tuiHalBoardInit()- ====");
    return TUI_HAL_OK;
}
