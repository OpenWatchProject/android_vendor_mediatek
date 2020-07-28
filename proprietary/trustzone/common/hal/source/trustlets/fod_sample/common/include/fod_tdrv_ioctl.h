/*
 * Copyright (c) 2018 MediaTek Inc.
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

#ifndef FOD_TDRV_IOCTL_H
#define FOD_TDRV_IOCTL_H

enum FOD_TDRV_IOCTL_CMD {
	FOD_TDRV_GET_SHM_PA,
    FOD_TDRV_MMAP_SHM,
    FOD_TDRV_UNMAP_SHM,
	FOD_TDRV_COPY_TO_SHM,
	FOD_TDRV_COPY_FROM_SHM,
	FOD_TDRV_SETUP_SHM_CMD,
	FOD_TDRV_MMAP1_STATIC_GZ_MTEE_SHM = 0x10107,
	FOD_TDRV_MMAP2_STATIC_GZ_MTEE_SHM = 0x10108,
	FOD_TDRV_TEST_STATIC_GZ_MTEE_SHM = 0x10109,
	FOD_TDRV_UMAP_STATIC_GZ_MTEE_SHM = 0x1010a,
};

struct fod_mem_info {
	uint64_t shm_pa;
	uintptr_t tapp_mem_va;
	uint32_t tapp_mem_size;
	uint32_t shm_offset;
	uint32_t buf_offset;
	uint32_t buf_num;
	uint32_t prop_offset;
	uint32_t prop_size;
	uint32_t core;
	uint32_t prop_cmd;
};

struct fod_tdrv_params {
    uint32_t cmd;
    uint32_t drv_ret;
    struct fod_mem_info mem_info;
};

#endif /* end of FOD_TDRV_IOCTL_H */
