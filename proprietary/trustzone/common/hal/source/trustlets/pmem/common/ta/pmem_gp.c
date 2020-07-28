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
#include <tee_taStd.h>

#include <drSecMemApi.h>
#include <drpmem_api.h>
#include <tee_internal_api.h>

#define LOG_TAG "pmem_ta"
#include <pmem_cfg.h>
#include <pmem_sec_dbg.h>
#include <pmem_ta_cmd.h>

/* clang-format off */
#define ASSERT_PARAM_TYPE(pt) \
    do { \
        if ((pt) != nParamTypes) \
        return TEE_ERROR_BAD_PARAMETERS; \
    } while (0)
/* clang-format on */

DECLARE_TRUSTED_APPLICATION_MAIN_STACK(2048);

TEE_Result TA_EXPORT TA_CreateEntryPoint(void)
{
    PMEM_TA_TRACE_FN();
    return TEE_SUCCESS;
}

void TA_EXPORT TA_DestroyEntryPoint(void)
{
    PMEM_TA_TRACE_FN();
}

TEE_Result TA_EXPORT TA_OpenSessionEntryPoint(uint32_t nParamTypes,
                                              TEE_Param pParams[4],
                                              void** ppSessionContext)
{
    UNUSED(nParamTypes);
    UNUSED(pParams);
    UNUSED(ppSessionContext);
    PMEM_TA_TRACE_FN();

    return TEE_SUCCESS;
}

void TA_EXPORT TA_CloseSessionEntryPoint(void* pSessionContext)
{
    UNUSED(pSessionContext);
    PMEM_TA_TRACE_FN();
}

#define PMEM_64BIT_PHYS_SHIFT (10)
#define HANDLE_TO_PA(handle)                       \
    (((uint64_t)handle << PMEM_64BIT_PHYS_SHIFT) & \
     ~((1 << PMEM_64BIT_PHYS_SHIFT) - 1))

static int pmem_process_cmd(int cmd, unsigned int handle, int size,
                            unsigned int pattern, int check_zero, int flags,
                            int* drv_ret)
{
    uint64_t query_handle, query_pa;
    int ret = TEE_SUCCESS;

    PMEM_TA_DBG(
        "cmd: %d, handle: 0x%x\n, size: 0x%x, pattern: 0x%x, flags: 0x%x, "
        "check_zero: %d\n",
        cmd, handle, size, pattern, flags, check_zero);

    query_handle = (uint64_t)handle;

    PMEM_TA_DBG("send handle:0x%08x%08x\n", HIGH_DWORD(query_handle),
                LOW_DWORD(query_handle));

    if (IS_GZ_MOCKED(flags)) {
        query_pa = HANDLE_TO_PA(query_handle);
    } else if (IS_NON_SECURE_MAP(flags)) {
        ret = drProtMemRemappedPAQuery(query_handle, &query_pa);
        if (ret) {
            PMEM_TA_ERR("fail to query remapped pa by handle!\n");
            return TEE_ERROR_ITEM_NOT_FOUND;
        }
    } else {
        ret = drMemPAQueryByType(query_handle, &query_pa, MEM_PROT);
        if (ret) {
            PMEM_TA_ERR("fail to query pa by handle!\n");
            return TEE_ERROR_ITEM_NOT_FOUND;
        }
    }

    PMEM_TA_DBG("got pa:0x%08x%08x from handle:0x%08x%08x\n",
                HIGH_DWORD(query_pa), LOW_DWORD(query_pa),
                HIGH_DWORD(query_handle), LOW_DWORD(query_handle));

    if ((cmd == PMEM_CMD_WRITE_PATTERN) ||
        (cmd == PMEM_CMD_MOCK_WRITE_PATTERN)) {
        ret = pmem_tdrv_write_pattern(query_pa, size, pattern, check_zero,
                                      flags, drv_ret);
    } else if ((cmd == PMEM_CMD_READ_PATTERN) ||
               (cmd == PMEM_CMD_MOCK_READ_PATTERN)) {
        ret = pmem_tdrv_read_pattern(query_pa, size, pattern, check_zero, flags,
                                     drv_ret);
    } else if (cmd == PMEM_CMD_TEE_MAP_TEST) {
        ret = pmem_tdrv_map_test(query_pa, size, pattern, check_zero, flags,
                                 drv_ret);
    } else {
        ret = TEE_ERROR_NOT_SUPPORTED;
    }

    return ret;
}

TEE_Result TA_EXPORT TA_InvokeCommandEntryPoint(void* pSessionContext,
                                                uint32_t nCommandID,
                                                uint32_t nParamTypes,
                                                TEE_Param pParams[4])
{
    int drv_ret;
    int ret = TEE_SUCCESS;
    UNUSED(pSessionContext);

    ASSERT_PARAM_TYPE(TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_VALUE_INPUT,
        TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_VALUE_OUTPUT));

    switch (nCommandID) {
    case PMEM_CMD_WRITE_PATTERN:
    case PMEM_CMD_READ_PATTERN:
    case PMEM_CMD_MOCK_WRITE_PATTERN:
    case PMEM_CMD_MOCK_READ_PATTERN:
    case PMEM_CMD_TEE_MAP_TEST:
        PMEM_TA_DBG("PMEM_CMD: %d\n", nCommandID);
        ret = pmem_process_cmd(nCommandID, pParams[0].value.a,
                               pParams[0].value.b, pParams[1].value.a,
                               pParams[1].value.b, pParams[2].value.a, &drv_ret);
        break;
    default:
        PMEM_TA_DBG("command is not supported %d", nCommandID);
        drv_ret = TEE_ERROR_NOT_SUPPORTED;
        break;
    }

    if (ret != 0) {
        return TEE_ERROR_GENERIC;
    }

    pParams[3].value.a = drv_ret;
    return TEE_SUCCESS;
}
