/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 * Helper to implement TZ_GetErrorString in MTEE/KREE/UREE
 * Please don't include this.
 */

#ifndef __TZ_ERROR_STRINGS__
#define __TZ_ERROR_STRINGS__

static const char * const TZ_ErrorStrings[] = {
	"The operation was successful.",
	"Non-specific cause.",
	"Access privileges are not sufficient.",
	"The operation was cancelled.",
	"Concurrent accesses caused conflict.",
	"Too much data for the requested operation was passed.",
	"Input data was of invalid format.",
	"Input parameters were invalid.",
	"Operation is not valid in the current state.",
	"The requested data item is not found.",
	"The requested operation should exist but is not yet implemented.",
	"The requested operation is valid but is not supported in this Implementation.",
	"Expected data was missing.",
	"System ran out of resources.",
	"The system is busy working on something else.",
	"Communication with a remote party failed.",
	"A security fault was detected.",
	"The supplied buffer is too short for the generated output.",
	"The handle is invalid.",
	"Unknown error.",
};

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#define TZ_ErrorStrings_num \
		(ARRAY_SIZE(TZ_ErrorStrings))


static const char *_TZ_GetErrorString(TZ_RESULT res)
{
	unsigned int num;

	if (res == 0)
		return TZ_ErrorStrings[0];

	num = ((unsigned int)res & 0xffff) + 1;
	if (num > (TZ_ErrorStrings_num - 1))
		num = TZ_ErrorStrings_num - 1;
	return TZ_ErrorStrings[num];
}

#endif				/* __TZ_ERROR_STRINGS__ */
