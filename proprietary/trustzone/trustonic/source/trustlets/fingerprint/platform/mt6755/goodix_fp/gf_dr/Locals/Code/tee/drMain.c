/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include "drStd.h"
#include "DrApi/DrApi.h"

DECLARE_DRIVER_MAIN_STACK(2048);

/* Driver version (used by the IPC handler) */
#define DRIVER_VERSION       1

extern void drIpchInit(void);
extern _NORETURN void drExchLoop(void);

/**
 * Initialization code
 */
void InitDriver(void) {
    // Nothing to do here now
}

/**
 * Main routine for the example driver.
 * Initializes the Api data structures and starts the required threads.
 */
_DRAPI_ENTRY void drMain(const addr_t dciBuffer, const uint32_t dciBufferLen) {
    drApiLogPrintf("[<t Driver DrGoodixUtils] %d.%d, Build " __DATE__ ", " __TIME__ EOL,
            DRIVER_VERSION_MAJOR, DRIVER_VERSION_MINOR);
    /* Start IPC handler */
    InitDriver();

    /* Start IPC handler */
    drIpchInit();

    /* Start exception handler */
    drExchLoop();
}
