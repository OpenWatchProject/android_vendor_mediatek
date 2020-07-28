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

/** @addtogroup DRIVER_TUI
 * @{
 * @file   drUtils.h
 * @brief  Contains utility API declerations
 */

#ifndef __DRUTILS_H__
#define __DRUTILS_H__

#include "DrApi/DrApiThread.h"
#include "DrApi/DrApiMm.h"

#include "drTuiCommon.h"


/* Syscall definitions */
#define SYSCALL_THREAD_EX_REGS      12

/**
 * Restarts thread after setting sp (with clean stack) and ip accordingly
 *
 * @retval E_OK    no error.
 */
drApiResult_t drUtilsRestartThread(
    threadno_t threadno,
    addr_t ip,
    addr_t sp );


/**
 * Makes request to RTM to update notification thread
 *
 * @retval E_OK    no error.
 */
drApiResult_t drUtilsUpdateNotificationThread( threadno_t threadno );

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif // __DRUTILS_H__
