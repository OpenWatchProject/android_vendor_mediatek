/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
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

#ifndef __MT_DRV_CALL_H__
#define __MT_DRV_CALL_H__

int mdrv_open(unsigned int driver_id, void *param);
int mdrv_close(int handle);
int mdrv_ioctl(int handle, unsigned int cmd_id, void *param);
void msee_ta_printf_va(const char *format, ...);
unsigned long long msee_ta_get_cntvct(void);
unsigned int msee_ta_get_cntfrq(void);
void msee_ta_trace_start(const char *name);
void msee_ta_trace_end(void);

#define msee_ta_printf(fmt, args...) do{}while(0)               /* empty implementation to force change API */

#if defined(DEBUG)
    #define msee_ta_logd(...)  msee_ta_printf_va(__VA_ARGS__)   /* debug message */
    #define msee_ta_loge(...)  msee_ta_printf_va(__VA_ARGS__)   /* error message */
#else
    #define msee_ta_logd(...)  do{}while(0)                     /* debug message */
    #define msee_ta_loge(...)  msee_ta_printf_va(__VA_ARGS__)   /* error message */
#endif

#define CONVERT_S_TO_US(count, freq) (count * 1000000 / freq)
#define CONVERT_US_TO_MS(us) (us / 1000)

#define ARM_GENERIC_TIMER_SUPPORT	(0)

#if ARM_GENERIC_TIMER_SUPPORT
    #define MSEE_TA_PROFILE_START \
        unsigned long long msee_time_1, msee_time_2, msee_time_diff; \
	unsigned int msee_time_freq = msee_ta_get_cntfrq(); \
        msee_time_1 = CONVERT_S_TO_US(msee_ta_get_cntvct(), msee_time_freq); \
        msee_ta_logd("[%s] profile start at (%u, %u) us\n", __FUNCTION__, (unsigned int)(msee_time_1>>32), (unsigned int)msee_time_1);

    #define MSEE_TA_PROFILE_END \
        msee_time_2 = CONVERT_S_TO_US(msee_ta_get_cntvct(), msee_time_freq); \
        msee_time_diff = msee_time_2 - msee_time_1; \
        msee_ta_logd("[%s] profile end at (%u, %u) us\n", __FUNCTION__, (unsigned int)(msee_time_2>>32), (unsigned int)msee_time_2); \
        msee_ta_loge("[%s] profile spend (%u, %u) us\n", __FUNCTION__, (unsigned int)(msee_time_diff>>32), (unsigned int)msee_time_diff);

    #define MSEE_TA_GET_TIME_DIFF (msee_time_diff)

#else

    #define MSEE_TA_PROFILE_START \
        unsigned long long msee_time_1, msee_time_2, msee_time_diff; \
        msee_time_1 = msee_ta_get_cntvct(); \
        msee_ta_logd("[%s] profile start at (%u, %u) ms\n", __FUNCTION__, (unsigned int)((msee_time_1>>32)&0xFFFFFFFF), (unsigned int)(msee_time_1&0xFFFFFFFF));

    #define MSEE_TA_PROFILE_END \
        msee_time_2 = msee_ta_get_cntvct(); \
        msee_time_diff = msee_time_2 - msee_time_1; \
        msee_ta_logd("[%s] profile end at (%u, %u) ms, spend (%u, %u) ms\n", __FUNCTION__, (unsigned int)((msee_time_2>>32)&0xFFFFFFFF), (unsigned int)(msee_time_2&0xFFFFFFFF), (unsigned int)((msee_time_diff>>32)&0xFFFFFFFF), (unsigned int)(msee_time_diff&0xFFFFFFFF));

    #define MSEE_TA_GET_TIME_DIFF (msee_time_diff)

#endif // ARM_GENERIC_TIMER_SUPPORT

#define MSEE_TA_GET_TIME_DIFF (msee_time_diff)

/*
 * TEE tracing log API
 */
#define MSEE_TA_TRACING_ENABLED	0
#define TEE_BEGIN_TRACE		"tee_trace_begin"
#define TEE_END_TRACE		"tee_trace_end"

#endif

