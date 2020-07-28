/*
 * Copyright (C) 2012-2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <trusty_std.h>

#include <tz_private/ta_sys_mem.h>
#include <tz_private/sys_mem.h>
#include <tz_private/log.h>
#include <tz_private/profile.h>
#include <unittest.h>

#include <tz_private/mtee_mmap_internal.h>
#include <unittest.h>
#include <lib/mtee/mtee_sys.h>
#include <trusty_syscalls.h>


#define LOG_TAG "MTEE RTC UT"

void test_rtc(MTEEC_PARAM *param)
{
	struct rtc_time tm;
	int i, count = 60, period = 1;
	struct rtc_handle rtc_fd;

	MTEE_rtc_open(&rtc_fd);

	DBG_LOG("param[0] a=%x , b=%x \n", param[0].value.a, param[0].value.b);
	DBG_LOG("param[1] a=%d , b=%x \n", param[1].value.a, param[1].value.b);

	if(param[0].value.b == 0x77){ //w
		tm.tm_year = param[1].value.a + 1900;
		tm.tm_min = ((param[1].value.b & (0xff << 0)) >> 0);
		tm.tm_hour = ((param[1].value.b & (0xff << 8)) >> 8);
		tm.tm_mday =((param[1].value.b & (0xff << 16)) >> 16);
		tm.tm_mon = ((param[1].value.b & (0xff << 24)) >> 24) + 1;

		DBG_LOG("set tc time = %04d-%02d-%02d %02d:%02d:%02d \n", tm.tm_year , tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		MTEE_rtc_write(&rtc_fd, &tm);
	}
	else if(param[0].value.b == 0x72){
		period = param[1].value.a;
		count = param[1].value.b;
	}

	for(i = 0; i < count; i++ ){

		//DBG_LOG("start %s %d-round(s) \n", __func__, i);

		memset(&tm, 0, sizeof(tm));

		//ioctl(SYS_MTEE_SYS_FD, MTEE_RTC_RD_TIME, &tm);
		MTEE_rtc_read(&rtc_fd, &tm);

		DBG_LOG("read tc time[%d], period=%d(s)= %04d/%02d/%02d %02d:%02d:%02d \n", i, period, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		nanosleep(0, 0, period * 1ULL * 1000 * 1000 * 1000);
	}

	MTEE_rtc_close(&rtc_fd);

}
