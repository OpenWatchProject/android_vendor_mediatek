/*
 * Copyright (c) 2015-2018 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __TEEI_ASM_H__
#define __TEEI_ASM_H__

#ifndef ATF_MAJOR_VERSION
	error major;
#endif
#ifndef ATF_MINOR_VERSION
	error minor;
#endif

#define BYPASS(x) x
#if (ATF_MAJOR_VERSION == 1 && ATF_MINOR_VERSION >= 3) || (ATF_MAJOR_VERSION > 1)
	#define ENDFUNC(x)	BYPASS(endfunc x)
#elif (ATF_MAJOR_VERSION == 1 && ATF_MINOR_VERSION == 2)
	#define ENDFUNC(x)	BYPASS(endfunc x)
#else
	#define ENDFUNC(x)
#endif

#endif
