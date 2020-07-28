/*
* Copyright (c) 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <trusty_std.h>
#include <errno.h>
#include <err.h>
#include <tz_private/system.h>

extern uuid_t client_uuid;

TZ_RESULT MTEE_GetSystemTime(MTEE_Time *time)
{
	uint64_t time_ns, time_ms;
	long rc;

	rc = gettime(0, 0, (int64_t *)&time_ns);

	if (rc != NO_ERROR)
		return TZ_RESULT_ERROR_GENERIC;

	time_ms = time_ns / 1000 / 1000;

	time->seconds = time_ms / 1000;
	time->millis = time_ms % 1000;

	return TZ_RESULT_SUCCESS;
}


int gettimeofday(struct timeval *__tv, struct timezone *__tz)
{
	uint64_t time_ns, time_us;
	long rc;

	if (__tz != NULL) return EINVAL;
	if (__tv == NULL) return EFAULT;

	rc = gettime(0, 0, (int64_t *)&time_ns);

	if (rc != NO_ERROR)
		return EINVAL;

	time_us = time_ns / 1000;

	__tv->tv_sec = (long int)time_us / 1000000;
	__tv->tv_usec = (long int)time_us % 1000000;

	return 0;
}

