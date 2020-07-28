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

#include <drv_call.h>
#include <drv_defs.h>
#include <drv_error.h>

#define LOG_TAG "fod_drvapi"
//#include <pmem_sec_dbg.h>
#include <fod_tdrv_ioctl.h>

#include "drfod_api.h"

#define FOD_DRV_ID DRIVER_ID

static int fod_tdriver_ioctl(struct fod_tdrv_params* params)
{
    int drv_handle;
    int ret;

    msee_ta_logd("ioctl cmd: %d\n", params->cmd);

    do {
        drv_handle = mdrv_open(FOD_DRV_ID, NULL);
        ret = mdrv_ioctl(drv_handle, params->cmd, params);
        mdrv_close(drv_handle);
    } while (0);

    msee_ta_logd("ioctl cmd: %d, ret: %d\n", params->cmd, ret);
    return ret;
}

int fod_tdrv_exec1(uint32_t cmd, uintptr_t tapp_mem_va, uint32_t tapp_mem_size, uint32_t shm_offset, int* drv_ret)
{
    struct fod_tdrv_params params;
    int ret;

    params.cmd = cmd;
	params.mem_info.shm_pa = 0;
    params.mem_info.tapp_mem_va = tapp_mem_va;
    params.mem_info.tapp_mem_size = tapp_mem_size;
	params.mem_info.shm_offset = shm_offset;
	params.mem_info.buf_offset = 0;
	params.mem_info.buf_num = 0;

    ret = fod_tdriver_ioctl(&params);
    if (ret) {
        msee_ta_logd("fail to send cmd:%d to pmem tdrv:%d\n", cmd, ret);
        return ret;
    }

    *drv_ret = params.drv_ret;
    return 0;
}

int fod_tdrv_exec2(uint32_t cmd, uint32_t *buf_offset, uint32_t *buf_num, uint32_t *prop_offset, uint32_t *prop_size, uint32_t core, uint32_t prop_cmd)
{
    struct fod_tdrv_params params;
    int ret;

    params.cmd = cmd;
	params.mem_info.shm_pa = 0;
    params.mem_info.tapp_mem_va = 0;
    params.mem_info.tapp_mem_size = 0;
	params.mem_info.shm_offset = 0;
	params.mem_info.buf_offset = 0;
	params.mem_info.buf_num = 0;
	params.mem_info.prop_offset = 0;
	params.mem_info.prop_size = 0;
	params.mem_info.core = core;
	params.mem_info.prop_cmd = prop_cmd;

    ret = fod_tdriver_ioctl(&params);
    if (ret) {
        msee_ta_logd("fail to send cmd:%d to pmem tdrv:%d\n", cmd, ret);
        return ret;
    }

    *buf_offset = params.mem_info.buf_offset;
	*buf_num = params.mem_info.buf_num;
    *prop_offset = params.mem_info.prop_offset;
	*prop_size = params.mem_info.prop_size;
    return 0;
}

int fod_tdrv_exec0(uint32_t cmd, uint64_t shm_pa, uint32_t shm_size)
{
    struct fod_tdrv_params params;
    int ret;

    params.cmd = cmd;
	params.mem_info.shm_pa = shm_pa;
    params.mem_info.tapp_mem_va = 0;
    params.mem_info.tapp_mem_size = shm_size;
	params.mem_info.shm_offset = 0;
	params.mem_info.buf_offset = 0;
	params.mem_info.buf_num = 0;

    ret = fod_tdriver_ioctl(&params);
    if (ret) {
        msee_ta_logd("fail to send cmd:%d to pmem tdrv:%d\n", cmd, ret);
        return ret;
    }

    return 0;
}

int fod_tdrv_setup_shm_pa(uint64_t shm_pa, uint32_t shm_size)
{
    return fod_tdrv_exec0(FOD_TDRV_GET_SHM_PA, shm_pa, shm_size);
}

int fod_tdrv_mmap1_StaticGzMteeShm(uint64_t pa, uint32_t sz)
{
    return fod_tdrv_exec0(FOD_TDRV_MMAP1_STATIC_GZ_MTEE_SHM, pa, sz);
}

int fod_tdrv_mmap2_StaticGzMteeShm(uint64_t pa, uint32_t sz)
{
    return fod_tdrv_exec0(FOD_TDRV_MMAP2_STATIC_GZ_MTEE_SHM, pa, sz);
}

int fod_tdrv_test_StaticGzMteeShm(uint64_t pa, uint32_t sz)
{
    return fod_tdrv_exec0(FOD_TDRV_TEST_STATIC_GZ_MTEE_SHM, pa, sz);
}

int fod_tdrv_umap_StaticGzMteeShm(uint64_t pa, uint32_t sz)
{
    return fod_tdrv_exec0(FOD_TDRV_UMAP_STATIC_GZ_MTEE_SHM, pa, sz);
}

int fod_tdrv_mmap_shm(uintptr_t tapp_mem_va, uint32_t tapp_mem_size, uint32_t shm_offset, int* drv_ret)
{
    return fod_tdrv_exec1(FOD_TDRV_MMAP_SHM, tapp_mem_va, tapp_mem_size, shm_offset, drv_ret);
}

int fod_tdrv_unmap_shm(uintptr_t tapp_mem_va, uint32_t tapp_mem_size, uint32_t shm_offset, int* drv_ret)
{
    return fod_tdrv_exec1(FOD_TDRV_UNMAP_SHM, tapp_mem_va, tapp_mem_size, shm_offset, drv_ret);
}

int fod_tdrv_copy_2_shm(uintptr_t tapp_mem_va, uint32_t tapp_mem_size, uint32_t shm_offset, int* drv_ret)
{
    return fod_tdrv_exec1(FOD_TDRV_COPY_TO_SHM, tapp_mem_va, tapp_mem_size, shm_offset, drv_ret);
}

int fod_tdrv_copy_from_shm(uintptr_t tapp_mem_va, uint32_t tapp_mem_size, uint32_t shm_offset, int* drv_ret)
{
    return fod_tdrv_exec1(FOD_TDRV_COPY_FROM_SHM, tapp_mem_va, tapp_mem_size, shm_offset, drv_ret);
}

int fod_tdrv_setup_shm_cmd(uint32_t *buf_offset, uint32_t *buf_num, uint32_t *prop_offset, uint32_t *prop_size, uint32_t core, uint32_t prop_cmd)
{
    return fod_tdrv_exec2(FOD_TDRV_SETUP_SHM_CMD, buf_offset, buf_num, prop_offset, prop_size, core, prop_cmd);
}



