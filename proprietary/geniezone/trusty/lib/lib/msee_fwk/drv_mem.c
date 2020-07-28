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

#include <msee_fwk/drv_fwk.h>
#include <msee_fwk/drv_defs.h>
#include <msee_fwk/drv_error.h>
#include <tz_private/system.h>
#include <tz_private/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "MSEE_DRV_MEM"

void msee_clean_dcache_all(void)
{
	MTEE_CleanDcacheAll();
}

void msee_clean_invalidate_dcache_all(void)
{
	MTEE_CleanInvalDcacheAll();
}

void msee_clean_dcache_range(unsigned long addr, unsigned long size)
{
	MTEE_CleanDcacheRange(addr, size);
}

void msee_clean_invalidate_dcache_range(unsigned long addr, unsigned long size)
{
	MTEE_CleanInvalDcacheRange(addr, size);
}

MSEE_Result msee_map_user(void **to, const void *from, unsigned long size, unsigned int flags)
{
	// fix me: todo implement
	ERR_LOG("[ERROR] msee_map_user not yet implemented \n");
	return DRV_FWK_API_OK;
}
MSEE_Result msee_unmap_user(void *to)
{
	// fix me: todo implement
	ERR_LOG("[ERROR] msee_unmap_user not yet implemented \n");
	return DRV_FWK_API_OK;
}

MSEE_Result msee_mmap_region(unsigned long long pa, void **va, unsigned long size, unsigned int flags)
{
	TZ_RESULT ret;

	ret = MTEE_MmapIORegion(pa, va, size, flags ? flags : MTEE_MAP_HARDWARE);
	if(ret != TZ_RESULT_SUCCESS)
	{
		return DRV_FWK_API_MAP_HARDWARE_FAILED;
	}

	return DRV_FWK_API_OK;
}

MSEE_Result msee_unmmap_region(void *va, unsigned long size)
{
	TZ_RESULT ret;

	ret = MTEE_UnmmapIORegion(va, size);
	if(ret != TZ_RESULT_SUCCESS)
	{
		return DRV_FWK_API_MAP_HARDWARE_FAILED;
	}

	return DRV_FWK_API_OK;
}

void* msee_malloc(unsigned long size)
{
	// fix me: todo implement
	ERR_LOG("[ERROR] msee_malloc not yet implemented \n");
	return NULL;
}

void* msee_realloc(void *buf, unsigned long size)
{
	// fix me: todo implement
	ERR_LOG("[ERROR] msee_realloc not yet implemented \n");
	return NULL;
}

void msee_free(void *buf)
{
	// fix me: todo implement
	ERR_LOG("[ERROR] msee_free not yet implemented \n");
	return 0;
}

void* msee_memcpy(void *dest, const void *src, unsigned long n)
{
	return memcpy(dest, src, n);
}

void* msee_memset(void *s, int c, unsigned long n)
{
	return memset(s, c, n);
}

char* msee_strncpy(char *dest, const char *src, unsigned long n)
{
	return strncpy(dest, src, n);
}

unsigned long msee_strlen(const char *str)
{
	return strlen(str);
}