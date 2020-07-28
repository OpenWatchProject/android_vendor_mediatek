/*
 *  ion.c
 *
 * Memory Allocator functions for ion
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#define LOG_TAG "ion"

#include <cutils/log.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <ion/ion.h>
#include <linux/mtk_ion.h>
#include <ion.h>
#include <linux/ion_drv.h>

#include "ion_ulit/ion_ulit.h"

static int ion_set_client_name(int ion_fd, const char *name)
{
    ion_sys_data_t sys_data;

    sys_data.sys_cmd = ION_SYS_SET_CLIENT_NAME;

    strncpy(sys_data.client_name_param.name, name, sizeof(sys_data.client_name_param.name)-1);

    if(ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data))
    {
        //config error
        ALOGE("[ion_dbg] ion_set_client_name error\n");
        return -1;
    }

    return 0;
}

int mt_ion_open(const char *name)
{
    int fd;
    fd = ion_open();
    if(fd < 0)
    {
        ALOGE("ion_open failed!\n");
        return fd;
    }

    ion_set_client_name(fd, name);
    return fd;
}

int mt_ion_close(int fd)
{
	int ret = ion_close(fd);
	if (ret < 0)
		ALOGE("mt_ion_close failed! fd=%d\n", fd);

	return ret;
}

int ion_alloc_mm(int fd, size_t len, size_t align, unsigned int flags,
              ion_user_handle_t *handle)
{
    return ion_alloc(fd, len, align, ION_HEAP_MULTIMEDIA_MASK, flags, handle);
}

int ion_alloc_camera(int fd, size_t len, size_t align, unsigned int flags,
              ion_user_handle_t *handle)
{
        /*int ret;
        struct ion_allocation_data data = {
                .len = len,
                .align = align,
                .flags = flags,
                .heap_mask = ION_HEAP_MULTIMEDIA_MASK
        };

        ret = ion_ioctl(fd, ION_IOC_ALLOC, &data);
        if (ret < 0)
                return ret;
        *handle = data.handle;

        ion_alloc();

        return ret;*/

        return ion_alloc(fd, len, align, ION_HEAP_CAMERA_MASK, flags, handle);
}


int ion_alloc_syscontig(int fd, size_t len, size_t align, unsigned int flags, ion_user_handle_t *handle)
{
        /*int ret;
        struct ion_allocation_data data = {
                .len = len,
                .align = align,
                .flags = flags,
                .heap_mask = ION_HEAP_SYSTEM_CONTIG_MASK
        };

        ret = ion_ioctl(fd, ION_IOC_ALLOC, &data);
        if (ret < 0)
                return ret;
        *handle = data.handle;

        return ret;*/

        return ion_alloc(fd, len, align, ION_HEAP_SYSTEM_CONTIG_MASK, flags, handle);
}


int ion_alloc_camera_pool(int fd, size_t len, size_t align, unsigned int flags,
              unsigned int *ret, int cache_pool_cmd)
{
	struct ion_mm_data mm_data;
	mm_data.mm_cmd = cache_pool_cmd;
	mm_data.cache_pool_info_param.len = len;
	mm_data.cache_pool_info_param.align = align;
	mm_data.cache_pool_info_param.heap_id_mask = ION_HEAP_CAMERA_MASK;
	mm_data.cache_pool_info_param.flags = flags;
	if (ion_custom_ioctl(fd, ION_CMD_MULTIMEDIA, &mm_data))
	{
	    ALOGE("ion_alloc_camera_pool failed, fd %d!\n", fd);
	    return -errno;
	}

	if ((cache_pool_cmd == ION_MM_QRY_CACHE_POOL) && (ret != NULL))
	    *ret = mm_data.cache_pool_info_param.ret;
	return 0;
}

void* ion_mmap(int fd, void *addr, size_t length, int prot, int flags, int share_fd, off_t offset)
{
    void *mapping_address = NULL;

    mapping_address =  mmap(addr, length, prot, flags, share_fd, offset);

    if (mapping_address == MAP_FAILED) {
        ALOGE("ion_mmap failed fd = %d, addr = 0x%p, len = %zu, prot = %d, flags = %d, share_fd = %d, 0x%p: %s\n", fd, addr, length,
              prot, flags, share_fd, mapping_address, strerror(errno));
#ifdef _MTK_ION_DUMP_CS
        dumping_callstack(__func__, fd, share_fd);
#endif
    }

    return mapping_address;
}

int ion_munmap(int fd, void *addr, size_t length)
{
    int ret = munmap(addr, length);

    if (ret < 0) {
        ALOGE("ion_munmap failed fd = %d, addr = 0x%p, len = %zu, %d: %s\n", fd, addr, length,
              ret, strerror(errno));
    }
    return ret;
}

int ion_share_close(int fd, int share_fd)
{
    int ret = close(share_fd);
    if (ret < 0) {
        ALOGE("ion_share_close failed fd = %d, share_fd = %d, %d: %s\n", fd, share_fd,
              ret, strerror(errno));
    }
    return ret;
}


int ion_custom_ioctl(int fd, unsigned int cmd, void* arg)
{
    struct ion_custom_data custom_data;
    custom_data.cmd = cmd;
    custom_data.arg = (unsigned long) arg;

    int ret = ioctl(fd, ION_IOC_CUSTOM, &custom_data);
    if (ret < 0) {
        ALOGE("ion_custom_ioctl %x failed with code %d: %s\n", (unsigned int)ION_IOC_CUSTOM,
              ret, strerror(errno));
#ifdef _MTK_ION_DUMP_CS
        dumping_callstack(__func__, fd, 0);
#endif
        return -errno;
    }
    return ret;
}

int ion_cache_sync_flush_all(int fd) {
	int ret;
	struct ion_sys_data sys_data;
	sys_data.sys_cmd = ION_SYS_DMA_OP;

	sys_data.dma_param.dma_type = ION_DMA_CACHE_FLUSH_ALL;
	ret = ion_custom_ioctl(fd, ION_CMD_SYSTEM, &sys_data);
	if (ret)
		return -errno;
	return ret;
}

int ion_cache_sync_flush_range(int fd) {
	int ret;
	struct ion_sys_data sys_data;
	sys_data.sys_cmd = ION_SYS_DMA_OP;

	sys_data.dma_param.dma_type = ION_DMA_CACHE_FLUSH_ALL;
	ret = ion_custom_ioctl(fd, ION_CMD_SYSTEM, &sys_data);
	if (ret)
		return -errno;
	return ret;
}

int ion_cache_sync_flush_range_va(int fd, void *addr, size_t length) {
	int ret;
	struct ion_sys_data sys_data;
	sys_data.sys_cmd = ION_SYS_DMA_OP;
	sys_data.dma_param.va = addr;
	sys_data.dma_param.size = length;

	sys_data.dma_param.dma_type = ION_DMA_FLUSH_BY_RANGE_USE_VA;
	ret = ion_custom_ioctl(fd, ION_CMD_SYSTEM, &sys_data);
	if (ret)
		return -errno;
	return ret;
}

int ion_dma_map_area(int fd, ion_user_handle_t handle, int dir) {
	int ret;
	struct ion_sys_data sys_data;
	sys_data.sys_cmd = ION_SYS_DMA_OP;
	sys_data.dma_param.handle = handle;

	sys_data.dma_param.dma_type = ION_DMA_MAP_AREA;
	sys_data.dma_param.dma_dir = dir;
	ret = ion_custom_ioctl(fd, ION_CMD_SYSTEM, &sys_data);
	if (ret)
		return -errno;
	return ret;
}

int ion_dma_unmap_area(int fd, ion_user_handle_t handle, int dir) {
	int ret;
	struct ion_sys_data sys_data;
	sys_data.sys_cmd = ION_SYS_DMA_OP;
	sys_data.dma_param.handle = handle;

	sys_data.dma_param.dma_type = ION_DMA_UNMAP_AREA;
	sys_data.dma_param.dma_dir = dir;
	ret = ion_custom_ioctl(fd, ION_CMD_SYSTEM, &sys_data);
	if (ret)
		return -errno;
	return ret;
}

int ion_dma_map_area_va(int fd, void *addr, size_t length, int dir) {
	int ret;
	struct ion_sys_data sys_data;
	sys_data.sys_cmd = ION_SYS_DMA_OP;
	sys_data.dma_param.va = addr;
	sys_data.dma_param.size = length;

	sys_data.dma_param.dma_type = ION_DMA_MAP_AREA_VA;
	sys_data.dma_param.dma_dir = dir;
	ret = ion_custom_ioctl(fd, ION_CMD_SYSTEM, &sys_data);
	if (ret)
		return -errno;
	return ret;
}

int ion_dma_unmap_area_va(int fd, void * addr, size_t length, int dir) {
	int ret;
	struct ion_sys_data sys_data;
	sys_data.sys_cmd = ION_SYS_DMA_OP;
	sys_data.dma_param.va = addr;
	sys_data.dma_param.size = length;

	sys_data.dma_param.dma_type = ION_DMA_UNMAP_AREA_VA;
	sys_data.dma_param.dma_dir = dir;
	ret = ion_custom_ioctl(fd, ION_CMD_SYSTEM, &sys_data);
	if (ret)
		return -errno;
	return ret;
}
