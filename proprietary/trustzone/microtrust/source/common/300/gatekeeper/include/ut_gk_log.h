/*
 * Copyright (C) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __UT_GK_LOG_H__
#define __UT_GK_LOG_H__

#include <imsg_log.h>

#define GK_LOG_INF_F(fmt, ...) imsg_print_wrapper(INFO, fmt, ##__VA_ARGS__)

#endif
