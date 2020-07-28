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

#include <lib/mtee/mtee_mmap.h>
#include <tz_private/log.h>

#include <mm.h>
#include <tz_private/mtee_mmap_internal.h>

#define LOG_TAG "mtee_mm"

extern uuid_t client_uuid;

TZ_RESULT MTEE_MmapRegion(unsigned long long pa, void **va, unsigned long size, unsigned int flags)
{
	long rc;
	user_mm_t mm_arg = {
		.pa = pa,
		.size = size,
		.flags = flags,
	};

	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_MMAP, &mm_arg);
	if (rc != NO_ERROR) {
		ERR_LOG("failed (%ld) to mmap region, pa=0x%llx size=0x%lx flags=0x%x\n",
				rc, pa, size, flags);
		return TZ_RESULT_ERROR_GENERIC;
	}

	*va = (void *)mm_arg.va;

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_UnmmapRegion(void *va, unsigned long size)
{
	long rc;
	user_mm_t mm_arg = {
		.va = (uint64_t)va,
		.size = size,
	};

	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_MUNMAP, &mm_arg);
	if (rc != NO_ERROR) {
		ERR_LOG("failed (%ld) to unmap region, va=0x%lx size=0x%x\n",
				rc, (unsigned long)va, size);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_MmapIORegion(unsigned long long pa, void **va, unsigned long size, unsigned int flags)
{
	long rc;
	user_mm_t mm_arg = {
		.pa = pa,
		.size = size,
		.flags = flags,
	};

	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_IOMMAP, &mm_arg);
	if (rc != NO_ERROR) {
		ERR_LOG("failed (%ld) to mmap region, pa=0x%llx size=0x%lx flags=0x%x",
				rc, pa, size, flags);
		return TZ_RESULT_ERROR_GENERIC;
	}

	*va = (void *)mm_arg.va;

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_UnmmapIORegion(void *va, unsigned long size)
{
	long rc;
	user_mm_t mm_arg = {
		.va = (uint32_t)va,
		.size = size,
	};

	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_IOMUNMAP, &mm_arg);
	if (rc != NO_ERROR) {
		ERR_LOG("failed (%ld) to unmap region, va=0x%lx size=0x%x\n",
				rc, (unsigned long)va, size);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}

uuid_t client_uuid = ZERO_UUID;
int MTEE_VAToPA(const void *va, uint64_t *pa)
{
	long rc;
	user_mm_t mm_arg = {
		.pa = 0,
		.va = (uint32_t)va,
		.uuid = client_uuid,
	};
	//printf ("====> MTEE_VAToPA\n");
	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_USER_VA_TO_PA, &mm_arg);
	if (rc != NO_ERROR)
		return rc;

	//printf ("====> PA = %llx\n", mm_arg.pa);

	*pa = mm_arg.pa;

	return NO_ERROR;
}

unsigned long MTEE_Physical(void* virt_addr)
{
	uint64_t pa;

	MTEE_VAToPA(virt_addr, &pa);
	return (unsigned long)pa;
}

void MTEE_CleanDcacheAll(void)
{
	long rc;
	user_mm_t mm_arg = {0};

	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_CLEAN_DCACHE_ALL, &mm_arg);
	if (rc != NO_ERROR)
		ERR_LOG("rc (%ld)",rc);
}

void MTEE_CleanInvalDcacheAll(void)
{
	long rc;
	user_mm_t mm_arg = {0};

	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_CLEAN_INVALIDATE_DCACHE_ALL, &mm_arg);
	if (rc != NO_ERROR)
		ERR_LOG("rc (%ld)",rc);
}

void MTEE_CleanDcacheRange(unsigned long addr, unsigned long size)
{
	long rc;
	user_mm_t mm_arg = {
		.va = addr,
		.size = size,
	};

	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_CLEAN_DCACHE_RANGE, &mm_arg);
	if (rc != NO_ERROR)
		ERR_LOG("rc (%ld)",rc);
}

void MTEE_CleanInvalDcacheRange(unsigned long addr, unsigned long size)
{
	long rc;
	user_mm_t mm_arg = {
		.va = addr,
		.size = size,
	};

	rc = ioctl(SYS_USER_MM_FD, MM_FUNC_CLEAN_INVALIDATE_DCACHE_RANGE, &mm_arg);
	if (rc != NO_ERROR)
		ERR_LOG("rc (%ld)",rc);
}

/**
 * Map a memory for device use.
 * This is for cache coherent operation for memory region used by both
 * CPU & HW device. Memory region is either used by CPU or HW device at
 * any given time. Before starting HW DMA, device driver must use MAP
 * function to mark the memory region as used by HW. After HW DMA is
 * completed and before CPU trying to access this memory, device driver
 * must call UNMAP function. These functions will do necessary cache
 * operation to ensure cache coherent between CPU & HW.
 *
 * NOTE:
 *   1. This function can only be used on secure DRAM and secure CM.
 *   2. The cpu_addr & size will be aligned to cache line.
 *
 * @param cpu_addr CPU address for the memory
 * @param size of the memory
 * @param direction The DMA direction.
 */
TZ_RESULT MTEE_DmaMapMemory(uint64_t *pa_list, void *cpu_addr, unsigned int size,
                       MTEE_DMA_DIRECTION direction,
					   MTEE_DMA_CACHEABILITY cacheable)
{
	int i, rc, ret = TZ_RESULT_SUCCESS;
	uint32_t flag, vaddr;
	struct dma_pmem *dma_arg;
	unsigned int pa_num = 0;

	if (!pa_list)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	DBG_LOG("dma map memory: cpu_addr = 0x%x, size = 0x%x\n", (unsigned int)cpu_addr, size);
	/* check page va alignment */
	vaddr = (uint32_t)cpu_addr;
	if (!cpu_addr || (vaddr % PAGE_SIZE))
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	/* check size alignment */
	if (!size || (size % PAGE_SIZE))
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	pa_num = size / PAGE_SIZE;

	dma_arg = (struct dma_pmem *)malloc(pa_num * sizeof(struct dma_pmem));
	if (!dma_arg)
		return TZ_RESULT_ERROR_OUT_OF_MEMORY;

	switch (direction) {
		case MTEE_DMA_TO_DEVICE:
			flag = DMA_FLAG_TO_DEVICE;
			break;
		case MTEE_DMA_FROM_DEVICE:
			flag = DMA_FLAG_FROM_DEVICE;
			break;
		case MTEE_DMA_BIDIRECTION:
			flag = DMA_FLAG_BIDIRECTION;
			break;
		default:
			flag = DMA_FLAG_BIDIRECTION;
	}

	if (cacheable == MTEE_DMA_NON_CACHEABLE) {
#ifndef __NEBULA_HEE__
		flag |= DMA_FLAG_NON_CACHEABLE;
#endif
	}

	flag |= DMA_FLAG_MULTI_PMEM;

	rc = prepare_dma(cpu_addr, size, flag, dma_arg);
	if (rc < 0) {
		ERR_LOG("prepare dma err: %d\n", ret);
		ret = TZ_RESULT_ERROR_GENERIC;
	} else {
		for (i = 0; i < pa_num; i++) {
			pa_list[i] = dma_arg[i].paddr;
		}
	}

	free(dma_arg);
	return ret;
}

TZ_RESULT MTEE_DmaUnmapMemory(uint64_t *pa_list, void *cpu_addr, unsigned int size,
                         MTEE_DMA_DIRECTION direction)
{
	int ret;
	uint32_t flag;

	if (!cpu_addr || !size || !pa_list)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	switch (direction) {
		case MTEE_DMA_TO_DEVICE:
			flag = DMA_FLAG_TO_DEVICE;
			break;
		case MTEE_DMA_FROM_DEVICE:
			flag = DMA_FLAG_FROM_DEVICE;
			break;
		case MTEE_DMA_BIDIRECTION:
			flag = DMA_FLAG_BIDIRECTION;
			break;
		default:
			flag = DMA_FLAG_BIDIRECTION;
	}

	ret = finish_dma(cpu_addr, size, flag);
	if (ret < 0) {
		DBG_LOG("finish dma err: %d\n", ret);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}
