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

#define LOG_TAG "pmem_drvapi"
#include <pmem_sec_dbg.h>
#include <pmem_tdrv_ioctl.h>

#include "drpmem_api.h"

#define PMEM_DRV_ID DRIVER_ID

static int pmem_tdriver_ioctl(struct pmem_tdrv_params* params)
{
    int drv_handle;
    int ret;

    PMEM_TA_DBG("ioctl cmd: %d\n", params->cmd);

    do {
        drv_handle = mdrv_open(PMEM_DRV_ID, NULL);
        ret = mdrv_ioctl(drv_handle, params->cmd, params);
        mdrv_close(drv_handle);
    } while (0);

    PMEM_TA_DBG("ioctl cmd: %d, ret: %d\n", params->cmd, ret);
    return ret;
}

int pmem_tdrv_exec(uint32_t cmd, uint64_t pa, uint32_t size, uint32_t pattern,
                   int32_t check_zero, int flags, int* drv_ret)
{
    struct pmem_tdrv_params params;
    int ret;

    params.cmd = cmd;
    params.mem_info.pa_start = pa;
    params.mem_info.size = size;
    params.mem_info.pattern = pattern;
    params.mem_info.check_zero = check_zero;
    params.mem_info.flags = flags;

    ret = pmem_tdriver_ioctl(&params);
    if (ret) {
        PMEM_TA_ERR("fail to send cmd:%d to pmem tdrv:%d\n", cmd, ret);
        return ret;
    }

    *drv_ret = params.drv_ret;
    return 0;
}

int pmem_tdrv_write_pattern(uint64_t pa, uint32_t size, uint32_t pattern,
                            int32_t check_zero, int flags, int* drv_ret)
{
    return pmem_tdrv_exec(PMEM_TDRV_WRITE_PATTERN, pa, size, pattern,
                          check_zero, flags, drv_ret);
}

int pmem_tdrv_read_pattern(uint64_t pa, uint32_t size, uint32_t pattern,
                           int32_t check_zero, int flags, int* drv_ret)
{
    return pmem_tdrv_exec(PMEM_TDRV_READ_PATTERN, pa, size, pattern, check_zero,
                          flags, drv_ret);
}

int pmem_tdrv_map_test(uint64_t pa, uint32_t total_size, uint32_t map_chunk_size,
                       int32_t map_all, int flags, int* drv_ret)
{
    return pmem_tdrv_exec(PMEM_TDRV_MAP_TEST, pa, total_size, map_chunk_size,
                          map_all, flags, drv_ret);
}
