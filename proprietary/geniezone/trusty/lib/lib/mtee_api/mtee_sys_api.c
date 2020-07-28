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
#include <tz_private/system.h>
#include <tz_private/log.h>
#include <lib/mtee/mtee_sys.h>
#include <tz_private/mtee_mmap_internal.h>

#define LOG_TAG "MTEE SYS API"


TZ_RESULT ioctl_func(const char *tag, enum MTEE_SYS_CMD cmd, void *ptr)
{
	int rc;

	rc = ioctl(SYS_MTEE_SYS_FD, cmd, ptr);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl %s failed. rc = %d\n", tag, rc);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_GetVersion(mtee_info_t *info)
{
	TZ_RESULT ret;

	ret = ioctl_func("get version", MTEE_SYS_GET_VERSION, info->version);

	return ret;
}

TZ_RESULT MTEE_GetUUID(uint8_t *hwuid)
{
    TZ_RESULT ret;

    ret = ioctl_func("get hwuid", MTEE_SYS_GET_HWUID, hwuid);

    return ret;
}


TZ_RESULT MTEE_SetUart(int control)
{
	TZ_RESULT ret;

	ret = ioctl_func("set uart", MTEE_UART_SET, &control);

	return ret;
}



TZ_RESULT MTEE_rtc_open(struct rtc_handle *h)
{
	h->fd = SYS_MTEE_SYS_FD;

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_rtc_close(struct rtc_handle *handle)
{
	// not support close

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_rtc_write(struct rtc_handle *handle, struct rtc_time *tv)
{
	TZ_RESULT ret;

	ret = ioctl_func("write rtc", MTEE_RTC_SET_TIME, tv);

	return ret;

}

TZ_RESULT MTEE_rtc_read(struct rtc_handle *handle, struct rtc_time *tv)
{
	TZ_RESULT ret;

	ret = ioctl_func("read rtc", MTEE_RTC_RD_TIME, tv);

	return ret;
}

TZ_RESULT MTEE_GetCellInfoBuf(cellinfo_buffer_t *buf_handle)
{
	TZ_RESULT ret;

	ret = ioctl_func("get cellinfo buffer", MTEE_CELLINFO_GET_BUF, buf_handle);

	return ret;

}

TZ_RESULT MTEE_DumpAppMMU(void)
{
	TZ_RESULT ret;

	ret = ioctl_func("dump app mmu", MTEE_DUMP_APP_MMU, NULL);

	return ret;

}

TZ_RESULT MTEE_DumpKernelHeap(void)
{
	TZ_RESULT ret;

	ret = ioctl_func("dump kernel heap", MTEE_DUMP_KERNEL_HEAP, NULL);

	return ret;
}

TZ_RESULT MTEE_DumpRamUsage(void)
{
	TZ_RESULT ret;

	ret = ioctl_func("dump RAM usage", MTEE_DUMP_RAM_USAGE, NULL);

	return ret;
}

void MTEE_DumpAppHeap(void)
{
#ifndef __NEBULA_HEE__
	malloc_stats();
#endif
}

int __MTEE_OperationMteeTeeStaticShm(void **va, u32 *size, const char *name, enum MTEE_SYS_CMD cmdid)
{
	long rc;
	user_mm_t mm_arg = {
		.va = *(uintptr_t *)va,
		.size = *size,
	};
	strlcpy((void *)&mm_arg.uuid, name, sizeof(mm_arg.uuid));

	rc = ioctl(SYS_MTEE_SYS_FD, cmdid, &mm_arg);
	if (rc != NO_ERROR) {
		ERR_LOG("rc=%ld\n", rc);
		return -1;
	} else {
		*(uintptr_t *)va = mm_arg.va;
		*size = mm_arg.size;
	}
	return rc;
}

int MTEE_AcquireMteeTeeStaticShm(void **va, u32 *size, const char *name)
{
#if 1
	return __MTEE_OperationMteeTeeStaticShm(va, size, name, MTEE_ACQUIRE_MTEE_TEE_STATIC_SHM);
#else
	long rc = 0;
	user_mm_t mm_arg;
	strlcpy((void *)&mm_arg.uuid, name, strlen(name)+1);

	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_ACQUIRE_MTEE_TEE_STATIC_SHM, &mm_arg);
	if (rc != NO_ERROR) {
		ERR_LOG("rc=%ld\n", rc);
	} else {
		*(uintptr_t *)va = mm_arg.va;
		*size = mm_arg.size;
	}
	return rc;
#endif
}

int MTEE_ReleaseMteeTeeStaticShm(void **va, u32 *size, const char *name)
{
#if 1
	return __MTEE_OperationMteeTeeStaticShm(va, size, name, MTEE_RELEASE_MTEE_TEE_STATIC_SHM);
#else
	long rc;
	user_mm_t mm_arg = {
		.va = *(uintptr_t *)va,
		.size = *size,
	};
	strlcpy((void *)&mm_arg.uuid, name, sizeof(mm_arg.uuid));

	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_RELEASE_MTEE_TEE_STATIC_SHM, &mm_arg);
	if (rc != NO_ERROR) {
		ERR_LOG("rc=%ld\n", rc);
	} else {
		*(uintptr_t *)va = mm_arg.va;
		*size = mm_arg.size;
	}
	return rc;
#endif
}

int MTEE_DumpMteeTeeStaticShm(void)
{
	long rc;
	user_mm_t mm_arg;
	strlcpy((void *)&mm_arg.uuid, "pAsSw0rd", sizeof("pAsSw0rd"));
	rc = ioctl(SYS_USER_MM_FD, MTEE_DUMP_MTEE_TEE_STATIC_SHM, &mm_arg);
	return rc;
}

