/*
* Copyright (c) 2015 MediaTek Inc.
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

#ifndef __MTEE_PROFILE_H__
#define __MTEE_PROFILE_H__

#include <tz_private/log.h>

/* global variables
*/
static uint64_t _gz_timeCountStart, _gz_timeCountEnd;
static uint32_t _gz_timeCountFreq;

/* profiling log define
*/
#define MTEE_PROFILE_LOG(fmt, ...) \
	MTEE_LOG(MTEE_LOG_LVL_PRINTF, fmt,  ## __VA_ARGS__)

//MTEE_LOG(MTEE_LOG_LVL_DEBUG, fmt,  ## __VA_ARGS__)

/* profiling functions
*/
#define MTEE_PROFILE_TIME_START() \
	_gz_timeCountFreq = MTEE_GetSystemCntFrq(); \
	_gz_timeCountStart = MTEE_GetSystemCnt();

#define MTEE_PROFILE_TIME_END(fmt, ...) _gz_timeCountEnd = MTEE_GetSystemCnt(); \	
	MTEE_PROFILE_LOG ("====> [profile time] time diff = %f us (freq = %f MHz, count = %lld)\n", \
	(1000000.0 *(float)(_gz_timeCountEnd-_gz_timeCountStart))/(float)_gz_timeCountFreq,	\			
				(float) _gz_timeCountFreq / 1000000.0, _gz_timeCountEnd-_gz_timeCountStart);

/* profiling start/end
*/
//#define MTEE_PROFILE 1
#define MTEE_PROFILE 0

#define MTEE_PROFILE_START(fmt, ...) do {if (MTEE_PROFILE) { \
	MTEE_PROFILE_LOG ("====> [profile start] " fmt,  ## __VA_ARGS__); \
	MTEE_PROFILE_TIME_START() \
	}} while (0)
	
#define MTEE_PROFILE_END(fmt, ...) do {if (MTEE_PROFILE) { \
	MTEE_PROFILE_TIME_END() \
	MTEE_PROFILE_LOG ("====> [profile end] " fmt,  ## __VA_ARGS__); \
	}} while (0)



#define TEST_ITEM(item_name, result, func, p0, ...) \
	DBG_LOG ("test %s\n", item_name); \
	MTEE_PROFILE_START ("%s\n", item_name); \
	*result = func (p0, ## __VA_ARGS__); \
	MTEE_PROFILE_END ("%s\n", item_name);

#define TEST_ITEM1(item_name, result, func, p0, ...) \
	DBG_LOG ("test %s\n", item_name); \
	MTEE_PROFILE_START ("%s\n", item_name); \
	func (p0, ## __VA_ARGS__); \
	MTEE_PROFILE_END ("%s\n", item_name);

/*#define TEST_ITEM2(item_name, result, func, p0, ...) \
	DBG_LOG ("test %s\n", item_name); \
	MTEE_PROFILE_START ("%s\n", item_name); \
	result = func (p0, ## __VA_ARGS__); \
	MTEE_PROFILE_END ("%s\n", item_name);
*/

#endif /* __MTEE_PROFILE_H__ */

