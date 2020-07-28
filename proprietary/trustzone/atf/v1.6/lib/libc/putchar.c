/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <console.h>
#if defined(MTK_PLAT_PORTING_LAYER)
#include <log.h>	/* for log_write() */
#endif

int putchar(int c)
{
	int res;

#if defined(MTK_PLAT_PORTING_LAYER)
	if (log_write)
		(*log_write)((unsigned char)c);
#endif

	if (console_putc((unsigned char)c) >= 0)
		res = c;
	else
		res = EOF;

	return res;
}
