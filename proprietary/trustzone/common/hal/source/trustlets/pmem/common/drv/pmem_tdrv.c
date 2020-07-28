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

#include <drv_defs.h>
#include <drv_error.h>
#include <drv_fwk.h>
#include <drv_fwk_macro.h>

#include <drpmem_api.h>

#define LOG_TAG "pmem_drv"
#include <pmem_cfg.h>
#include <pmem_sec_dbg.h>
#include <pmem_tdrv_ioctl.h>

#include "pmem_tdrv.h"

/* clang-format off */
enum PMEM_TDRV_ERROR_STATUS {
    PMEM_TDRV_OK = 0,
    PMEM_TDRV_ERROR = 1,
    PMEM_TDRV_MAP_MEM_FAIL = 2,
    PMEM_TDRV_UNMAP_MEM_FAIL = 3,
    PMEM_TDRV_PATTERN_CHECK_ZERO_FAIL = 4,
    PMEM_TDRV_PATTERN_WRITE_FAIL = 5,
    PMEM_TDRV_PATTERN_WRITE_CHECK_FAIL = 6,
    PMEM_TDRV_PATTERN_READ_CHECK_FAIL = 7,
    PMEM_TDRV_SIZE_IS_NOT_4BYTES_ALIGN = 8,
};
/* clang-format on */

#define PMEM_DRV_ID DRIVER_ID
#define DRVTAG LOG_TAG

#define SIZE_2M 0x00200000
#define SIZE_256M 0x10000000
#define MAX_MAP_CHUNK_ONE_TIME SIZE_2M
#define MIN(a, b) (a > b) ? b : a

static int pmem_pattern_check(int8_t* va, int size_bytes, uint32_t pattern)
{
    int size_word = size_bytes / 4;
    uint32_t* data = (uint32_t*)va;
    int i;

    for (i = 0; i < size_word; i++) {
        if (*(data + i) != pattern) {
            PMEM_TDRV_ERR("va[0x%x](%d): 0x%x != 0x%x\n", (va + i * 4), i,
                          *(data + i), pattern);
            return PMEM_TDRV_ERROR;
        }
    }

    return PMEM_TDRV_OK;
}

static int pmem_pattern_write(int8_t* va, int size_bytes, uint32_t pattern)
{
    int size_word = size_bytes / 4;
    uint32_t* data = (uint32_t*)va;
    int i;

    for (i = 0; i < size_word; i++) {
        *(data + i) = pattern;
    }

    return PMEM_TDRV_OK;
}

static int pmem_content_check(int8_t* va, int size_bytes, uint32_t pattern,
                              int check_zero, int flags, int is_write)
{
    int ret;

    if (check_zero) {
        /* invalid before read, but since it is just mapped, we don't need to do
         * this
         * extra operation */

        if (IS_GZ_MOCKED(flags)) {
            /* without real GZ to reset the contect, we return pass directly */
            ret = PMEM_TDRV_OK;
        } else {
            ret = pmem_pattern_check(va, size_bytes, 0x00000000);
            if (ret) {
                return PMEM_TDRV_PATTERN_CHECK_ZERO_FAIL;
            }
        }
    }

    if (is_write) {
        ret = pmem_pattern_write(va, size_bytes, pattern);
        if (ret) {
            return PMEM_TDRV_PATTERN_WRITE_FAIL;
        }

        ret = pmem_pattern_check(va, size_bytes, pattern);
        if (ret) {
            return PMEM_TDRV_PATTERN_WRITE_CHECK_FAIL;
        }

        /* flush after write if we are mapped with NS=0 */
        if (!IS_NON_SECURE_MAP(flags))
            msee_clean_invalidate_dcache_range((unsigned long)va, size_bytes);
    } else {
        ret = pmem_pattern_check(va, size_bytes, pattern);
        if (ret) {
            return PMEM_TDRV_PATTERN_READ_CHECK_FAIL;
        }
    }

    PMEM_TDRV_DBG("pmem_content_check: PASSED for %s (chk0:%d)!\n",
                  is_write ? "WRITE" : "READ", check_zero);
    return PMEM_TDRV_OK;
}

#define IS_ALIGN_4B(sz) ((sz % 4) ? 0 : 1)
static int pmem_mem_check_and_update(struct pmem_info* mem_info, int is_write)
{
    int remain_sz = mem_info->size;
    uint64_t mem_start = mem_info->pa_start;
    uint64_t map_start;
    int rw_off, rw_sz;
    int8_t *vaddr, *rw_vaddr;
    int ret;
    unsigned int map_flags = MSEE_MAP_READABLE | MSEE_MAP_WRITABLE;

    if (!IS_ALIGN_4B(mem_info->size)) {
        PMEM_TDRV_ERR("size is not 4bytes aligned: 0x%x\n", mem_info->size);
        return PMEM_TDRV_SIZE_IS_NOT_4BYTES_ALIGN;
    }

    if (IS_NON_SECURE_MAP(mem_info->flags))
        map_flags |= MSEE_MAP_ALLOW_NONSECURE;

    /* align map pa start to map chunk size */
    rw_off = mem_start % MAX_MAP_CHUNK_ONE_TIME;
    map_start = mem_start - rw_off;

    PMEM_TDRV_DBG(
        "mem start:0x%08x%08x, map start: 0x%08x%08x, map flags: 0x%x\n",
        HIGH_DWORD(mem_start), LOW_DWORD(mem_start), HIGH_DWORD(map_start),
        LOW_DWORD(map_start), map_flags);

    rw_sz = MIN((MAX_MAP_CHUNK_ONE_TIME - rw_off), remain_sz);

    while (remain_sz > 0) {
        PMEM_TDRV_DBG(
            "map_start:0x%08x%08x, rw_off:0x%x, rw_size:0x%x, remain:0x%x\n",
            HIGH_DWORD(map_start), LOW_DWORD(map_start), rw_off, rw_sz,
            remain_sz);

        if ((ret = msee_mmap_region((uint64_t)map_start, (void**)&vaddr,
                                    MAX_MAP_CHUNK_ONE_TIME, map_flags)) !=
            DRV_FWK_API_OK) {
            PMEM_TDRV_ERR("msee_mmap_region failed %d\n", ret);
            return PMEM_TDRV_MAP_MEM_FAIL;
        }

        /* invalidate the mapped region before read */
        msee_clean_invalidate_dcache_range((unsigned long)vaddr, MAX_MAP_CHUNK_ONE_TIME);

        rw_vaddr = vaddr;
        rw_vaddr += rw_off;
        if ((ret = pmem_content_check(rw_vaddr, rw_sz, mem_info->pattern,
                                      mem_info->check_zero, mem_info->flags,
                                      is_write)) != PMEM_TDRV_OK) {
            PMEM_TDRV_ERR("pmem_mem_check failed %d (try unmap)\n", ret);
            msee_unmmap_region(vaddr, MAX_MAP_CHUNK_ONE_TIME);
            return ret;
        }

        if ((ret = msee_unmmap_region(vaddr, MAX_MAP_CHUNK_ONE_TIME)) !=
            DRV_FWK_API_OK) {
            PMEM_TDRV_ERR("msee_unmmap_region failed %d\n", ret);
            return PMEM_TDRV_UNMAP_MEM_FAIL;
        }

        remain_sz -= rw_sz;
        map_start += MAX_MAP_CHUNK_ONE_TIME;
        rw_off = 0;
        rw_sz = MIN((MAX_MAP_CHUNK_ONE_TIME - rw_off), remain_sz);
    }

    PMEM_TDRV_DBG("pmem_mem_check_and_update: PASSED!\n");
    return PMEM_TDRV_OK;
}

#define MAX_MAP_CNT (SIZE_256M / SIZE_2M)
static int pmem_mem_map_test(struct pmem_info* mem_info)
{
    int ret = PMEM_TDRV_OK;
    uint64_t pa_start = mem_info->pa_start;
    uint32_t total_sz = mem_info->size;
    uint64_t map_pa;
    uint32_t map_chunk_sz = mem_info->pattern;
    uint32_t tay_map_total_sz = mem_info->check_zero;
    uint32_t total_mapped_sz = 0;
    int32_t map_cnt = tay_map_total_sz / map_chunk_sz;
    int8_t* vaddr[MAX_MAP_CNT];
    int32_t idx;

    PMEM_TDRV_DBG("[map] pa_start:0x%08x%08x, total_sz:0x%x\n",
                  HIGH_DWORD(pa_start), LOW_DWORD(pa_start), total_sz);
    PMEM_TDRV_DBG("[map] map_sz:0x%x, try map total:0x%x, map count:%d\n",
                  map_chunk_sz, tay_map_total_sz, map_cnt);

    for (idx = 0; idx < MAX_MAP_CNT; idx++) {
        vaddr[idx] = NULL;
    }

    for (idx = 0; idx < map_cnt; idx++) {
        map_pa = pa_start + ((uint64_t)map_chunk_sz * idx);
        if ((ret = msee_mmap_region(
                 (uint64_t)map_pa, (void**)(&vaddr[idx]), map_chunk_sz,
                 MSEE_MAP_READABLE | MSEE_MAP_WRITABLE)) != DRV_FWK_API_OK) {
            PMEM_TDRV_ERR(
                "[map] msee_mmap_region failed %d, idx:%d, map_pa:0x%08x%08x\n",
                ret, idx, HIGH_DWORD(map_pa), LOW_DWORD(map_pa));
            ret = PMEM_TDRV_MAP_MEM_FAIL;
            goto map_region_fail;
        }

        PMEM_TDRV_DBG(
            "[map] msee_mmap_region pass for idx:%d, map_pa:0x%08x%08x\n", idx,
            HIGH_DWORD(map_pa), LOW_DWORD(map_pa));
        total_mapped_sz += map_chunk_sz;

        if (total_mapped_sz >= total_sz)
            break;
    }

map_region_fail:
    for (idx = 0; idx < map_cnt; idx++) {
        if (!VALID(vaddr[idx]))
            continue;
        if ((ret = msee_unmmap_region(vaddr[idx], map_chunk_sz)) !=
            DRV_FWK_API_OK) {
            PMEM_TDRV_ERR("[map] msee_unmmap_region failed %d, idx:%d\n", ret,
                          idx);
            ret = PMEM_TDRV_UNMAP_MEM_FAIL;
            goto unmap_region_fail;
        }
        PMEM_TDRV_DBG("[map] msee_unmmap_region pass for idx:%d\n", idx);
    }
unmap_region_fail:
    PMEM_TDRV_DBG("[map] total mapped size: 0x%x, should try: 0x%x (ret:%d)\n",
                  total_mapped_sz, tay_map_total_sz, ret);

    return ret;
}

static int pmem_invoke_command(unsigned int sid, unsigned int cmd,
                               unsigned long args)
{
    int fwk_ret = DRV_FWK_API_OK;
    int drv_ret = DRV_FWK_API_OK;
    struct pmem_tdrv_params* ta_params = NULL;
    UNUSED(sid);
    PMEM_TDRV_TRACE_FN();

    PMEM_TDRV_DBG("sid:%d, cmd:%d, args:0x%x\n", sid, cmd, (unsigned int)args);

    fwk_ret =
        msee_map_user((void*)&ta_params, (void*)args,
                      sizeof(struct pmem_tdrv_params), MSEE_MAP_USER_DEFAULT);
    if (fwk_ret != DRV_FWK_API_OK) {
        PMEM_TDRV_ERR("map_user failed %d\n", fwk_ret);
        return fwk_ret;
    }

    switch (cmd) {
    case PMEM_TDRV_WRITE_PATTERN:
        PMEM_TDRV_DBG("PMEM_TDRV_WRITE_PATTERN\n");
        drv_ret = pmem_mem_check_and_update(&ta_params->mem_info, 1);
        break;
    case PMEM_TDRV_READ_PATTERN:
        PMEM_TDRV_DBG("PMEM_TDRV_READ_PATTERN\n");
        drv_ret = pmem_mem_check_and_update(&ta_params->mem_info, 0);
        break;
    case PMEM_TDRV_MAP_TEST:
        PMEM_TDRV_DBG("PMEM_TDRV_MAP_TEST\n");
        drv_ret = pmem_mem_map_test(&ta_params->mem_info);
        break;
    default:
        PMEM_TDRV_DBG("command is not supported %d", cmd);
        drv_ret = DRV_FWK_API_INVALIDATE_PARAMETERS;
        break;
    }

    ta_params->drv_ret = drv_ret;

    fwk_ret = msee_unmap_user(ta_params);
    if (fwk_ret != DRV_FWK_API_OK) {
        PMEM_TDRV_ERR("unmap_user failed %d\n", fwk_ret);
        return fwk_ret;
    }

    return DRV_FWK_API_OK;
}

static int pmem_open_session(unsigned int sid, unsigned long args)
{
    UNUSED(sid);
    UNUSED(args);
    PMEM_TDRV_TRACE_FN();
    return 0;
}

static int pmem_close_session(unsigned int sid)
{
    UNUSED(sid);
    PMEM_TDRV_TRACE_FN();
    return 0;
}

static int pmem_init(const struct msee_drv_device* dev)
{
    UNUSED(dev);
    PMEM_TDRV_TRACE_FN();

    return 0;
}

DECLARE_DRIVER_MODULE(DRVTAG, PMEM_DRV_ID, pmem_init, pmem_invoke_command, NULL,
                      pmem_open_session, pmem_close_session);
