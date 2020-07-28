/*
 * Copyright (c) 2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef __TBASE_H__
#define __TBASE_H__

#include <bl_common.h>
#include <tbase_platform.h>

// No flags defined so far
#define TBASE_MONITOR_FLAGS_DEFAULT (0)

// Minimum registers available in SWd entry
#define TBASE_MONITOR_CALL_REGS_MIN 4
#define TBASE_NWD_REGISTERS_MIN 8

#define TBASE_MONITOR_INTERFACE_VERSION 1


#ifndef __ASSEMBLY__

typedef struct {
	uint64_t x1;
	uint64_t x2;
	uint64_t x3;
	uint64_t x4;
} fc_response_t;


// Forward fastcall to tbase
// Returns 0 in case of successfull call, and non-zero in case of error.
// If resp is non-NULL, return values are put there.
// If previous context was NON_SECURE, caller must save it before call.
// Function changes to SECURE context.
uint64_t tbase_monitor_fastcall(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  fc_response_t *resp );


#endif

#endif /* __TBASE_H__ */
