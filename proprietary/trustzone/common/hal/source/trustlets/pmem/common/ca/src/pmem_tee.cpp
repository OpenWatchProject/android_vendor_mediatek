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

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LOG_TAG "PMEM_TEE"
/* LOG_TAG need to be defined before include log.h */
#include <android/log.h>

#ifdef TRUSTONIC_TEE_ENABLED
#include <MobiCoreDriverApi.h>
#endif
#ifdef GP_TEE_ENABLED
#include <tee_client_api.h>
#endif

#include <pmem_cfg.h>
#include <pmem_ta_cmd.h>

#include "pmem_dbg.h"
#include "pmem_lib.h"
#include "pmem_share.h"

#ifdef TRUSTONIC_TEE_ENABLED
// clang-format off
#define PMEM_DRIVER_UUID \
    {{ 0x08, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x19 }}
// clang-format on

static const uint32_t g_device_id = MC_DEVICE_ID_DEFAULT;
static const mcUuid_t g_drv_uuid = PMEM_DRIVER_UUID;

struct pmem_tci_params {
    int command;
    int ret;
    int params[8];
};

static mcSessionHandle_t pmem_drv_session;
static struct pmem_tci_params* pmem_drv_params = NULL;

static struct pmem_tci_params* TEE_Open(mcSessionHandle_t* pSessionHandle)
{
    struct pmem_tci_params* pTci = NULL;
    mcResult_t mcRet;

    do {
        if (pSessionHandle == NULL) {
            CONSOLE_ERR("TEE_Open(): Invalid session handle!");
            return NULL;
        }

        memset(pSessionHandle, 0, sizeof(mcSessionHandle_t));

        mcRet = mcOpenDevice(g_device_id);
        if ((MC_DRV_OK != mcRet) && (MC_DRV_ERR_DEVICE_ALREADY_OPEN != mcRet)) {
            CONSOLE_ERR("TEE_Open(): mcOpenDevice returned:", mcRet);
            return NULL;
        }

        mcRet = mcMallocWsm(g_device_id, 0, sizeof(struct pmem_tci_params),
                            (uint8_t**)&pTci, 0);
        if (MC_DRV_OK != mcRet) {
            mcCloseDevice(g_device_id);
            CONSOLE_ERR("TEE_Open(): mcMallocWsm returned:", mcRet);
            return NULL;
        }

        pSessionHandle->deviceId = g_device_id;
        mcRet = mcOpenSession(pSessionHandle, &g_drv_uuid, (uint8_t*)pTci,
                              (uint32_t)sizeof(struct pmem_tci_params));
        if (MC_DRV_OK != mcRet) {
            mcFreeWsm(g_device_id, (uint8_t*)pmem_drv_params);
            mcCloseDevice(g_device_id);
            CONSOLE_ERR("TEE_Open(): mcOpenSession returned:", mcRet);
            return NULL;
        }
    } while (false);

    return pTci;
}

static void TEE_Close(mcSessionHandle_t* pSessionHandle)
{
    mcResult_t mcRet;

    do {
        if (pSessionHandle == NULL) {
            CONSOLE_ERR("TEE_Close(): Invalid session handle!");
            break;
        }

        mcRet = mcCloseSession(pSessionHandle);
        if (MC_DRV_OK != mcRet) {
            CONSOLE_ERR("TEE_Close(): mcCloseSession returned:", mcRet);
            break;
        }

        mcRet = mcFreeWsm(g_device_id, (uint8_t*)pmem_drv_params);
        if (MC_DRV_OK != mcRet) {
            CONSOLE_ERR("TEE_Close(): mcFreeWsm returned:", mcRet);
            break;
        }

        mcRet = mcCloseDevice(g_device_id);
        if (MC_DRV_OK != mcRet) {
            CONSOLE_ERR("TEE_Close(): mcCloseDevice returned:", mcRet);
            break;
        }
    } while (false);
}

static int tee_open_pmem_tdriver()
{
    pmem_drv_params = TEE_Open(&pmem_drv_session);
    if (INVALID(pmem_drv_params)) {
        CONSOLE_ERR("open pmem t-driver failed!");
        return PMEM_FAIL;
    }

    return PMEM_SUCCESS;
}

static int tee_close_pmem_tdriver()
{
    if (VALID(pmem_drv_params)) {
        TEE_Close(&pmem_drv_session);
        pmem_drv_params = NULL;
    }
    return PMEM_SUCCESS;
}
#endif

#ifdef GP_TEE_ENABLED
// clang-format off
#define PMEM_TA_GP_UUID \
	{ 0x08040000, 0x0000, 0x0000, \
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x19 } }
// clang-format on

static TEEC_Context g_ta_context;
static TEEC_Session g_ta_session;
static TEEC_Operation g_ta_operation;
static int g_pmem_configs = 0;

static int tee_open_pmem_ta()
{
    TEEC_Result ret;
    TEEC_UUID destination = PMEM_TA_GP_UUID;

    memset(&g_ta_context, 0, sizeof(g_ta_context));
    ret = TEEC_InitializeContext(NULL, &g_ta_context);
    if (ret != TEEC_SUCCESS) {
        CONSOLE_ERR("Could not initialize context with the TEE!");
        return PMEM_FAIL;
    }

    memset(&g_ta_session, 0, sizeof(g_ta_session));
    ret = TEEC_OpenSession(&g_ta_context, &g_ta_session, &destination,
                           TEEC_LOGIN_PUBLIC, NULL, NULL, NULL);
    if (ret != TEEC_SUCCESS) {
        TEEC_FinalizeContext(&g_ta_context);
        CONSOLE_ERR("Could not open session with Trusted Application!");
        return PMEM_FAIL;
    }

    return PMEM_SUCCESS;
}

static int tee_close_pmem_ta()
{
    TEEC_CloseSession(&g_ta_session);
    TEEC_FinalizeContext(&g_ta_context);
    return PMEM_SUCCESS;
}

static int tee_open_session()
{
#ifdef TRUSTONIC_TEE_ENABLED
    if (tee_open_pmem_tdriver()) {
        return PMEM_OPEN_TDRIVER_FAIL;
    }
#endif

    if (tee_open_pmem_ta()) {
        return PMEM_OPEN_TA_FAIL;
    }

    return PMEM_SUCCESS;
}

static int tee_close_session()
{
    if (tee_close_pmem_ta()) {
        return PMEM_CLOSE_TA_FAIL;
    }

#ifdef TRUSTONIC_TEE_ENABLED
    if (tee_close_pmem_tdriver()) {
        return PMEM_CLOSE_TDRIVER_FAIL;
    }
#endif

    return PMEM_SUCCESS;
}

static int tee_invoke_cmd(int cmd, uint32_t handle, uint32_t size,
                          uint32_t pattern, uint32_t check_zero)
{
    TEEC_Result ret;
    int check_result;

    memset(&g_ta_operation, 0, sizeof(g_ta_operation));
    g_ta_operation.paramTypes = TEEC_PARAM_TYPES(
        TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_VALUE_OUTPUT);
    g_ta_operation.params[0].value.a = handle;
    g_ta_operation.params[0].value.b = size;
    g_ta_operation.params[1].value.a = pattern;
    g_ta_operation.params[1].value.b = check_zero;
    g_ta_operation.params[2].value.a = g_pmem_configs;

    ret = TEEC_InvokeCommand(&g_ta_session, cmd, &g_ta_operation, NULL);

    if (ret != TEEC_SUCCESS) {
        CONSOLE_ERR("Invoke command failed!");
        return PMEM_INVOKE_CMD_FAIL;
    }

    check_result = g_ta_operation.params[3].value.a;
    if (check_result != 0) {
        CONSOLE_DBG("Pattern check mismatched!");
        return check_result;
    }

    return PMEM_SUCCESS;
}
#endif

#define MAX_READ_LEN (32)
static int pmem_read_cfg()
{
    int pmem_fd;
    char core_ops[MAX_READ_LEN + 1] = {0};
    int ret;

    if ((pmem_fd = pmem_open()) < 0) {
        CONSOLE_DBG("open pmem proc entry fail");
        return PMEM_PROC_ENTRY_DISABLE;
    }

    ret = pmem_read(pmem_fd, core_ops, MAX_READ_LEN);
    pmem_close(pmem_fd);
    if (ret != PMEM_SUCCESS) {
        CONSOLE_DBG("open read proc entry fail");
        return PMEM_PROC_READ_FAIL;
    }

    CONSOLE_DBG("core ops is:", core_ops);

    if (!strncmp(core_ops, "PMEM_MOCK_ALL", strlen("PMEM_MOCK_ALL")) ||
        !strncmp(core_ops, "PMEM_MOCK_MTEE", strlen("PMEM_MOCK_MTEE"))) {
        return PMEM_PROC_MOCK_GZ;
    }

    return PMEM_SUCCESS;
}

static int tee_write_cfg(int flags)
{
    UNUSED(flags);

    return PMEM_FAIL;
}

int tee_open()
{
#ifdef GP_TEE_ENABLED
    if (pmem_read_cfg() == PMEM_PROC_MOCK_GZ) {
        g_pmem_configs |= PMEM_CONFIG_MOCK_GZ;
    } else {
        g_pmem_configs |= PMEM_CONFIG_MOCK_NONE;
    }

#ifdef TEE_MEMORY_MAP_NS_FLAGS
    g_pmem_configs |= PMEM_CONFIG_TEE_MAP_NONE_SECURE;
#endif

    return tee_open_session();
#else
    return PMEM_NOT_IMPLEMENTED;
#endif
}

int tee_close()
{
#ifdef GP_TEE_ENABLED
    return tee_close_session();
#else
    return PMEM_NOT_IMPLEMENTED;
#endif
}

int tee_write(int mem_handle, int mem_size, unsigned int pattern,
              bool check_zero)
{
#ifdef GP_TEE_ENABLED
    return tee_invoke_cmd(PMEM_CMD_WRITE_PATTERN, mem_handle, mem_size, pattern,
                          check_zero);
#else
    UNUSED(mem_handle);
    UNUSED(mem_size);
    UNUSED(pattern);
    UNUSED(check_zero);
    return PMEM_NOT_IMPLEMENTED;
#endif
}

int tee_read(int mem_handle, int mem_size, unsigned int pattern)
{
#ifdef GP_TEE_ENABLED
    return tee_invoke_cmd(PMEM_CMD_READ_PATTERN, mem_handle, mem_size, pattern,
                          0);
#else
    UNUSED(mem_handle);
    UNUSED(mem_size);
    UNUSED(pattern);
    return PMEM_NOT_IMPLEMENTED;
#endif
}

#ifdef PMEM_MOCK_GZ
int tee_mock_write(int mem_handle, int mem_size, unsigned int pattern,
                   bool check_zero)
{
#ifdef GP_TEE_ENABLED
    return tee_invoke_cmd(PMEM_CMD_MOCK_WRITE_PATTERN, mem_handle, mem_size,
                          pattern, check_zero);
#else
    UNUSED(mem_handle);
    UNUSED(mem_size);
    UNUSED(pattern);
    UNUSED(check_zero);
    return PMEM_NOT_IMPLEMENTED;
#endif
}

int tee_mock_read(int mem_handle, int mem_size, unsigned int pattern)
{
#ifdef GP_TEE_ENABLED
    return tee_invoke_cmd(PMEM_CMD_MOCK_READ_PATTERN, mem_handle, mem_size,
                          pattern, 0);
#else
    UNUSED(mem_handle);
    UNUSED(mem_size);
    UNUSED(pattern);
    return PMEM_NOT_IMPLEMENTED;
#endif
}
#endif

#ifdef TEE_MEMORY_MAP_TEST_ENABLE
int tee_mem_map(int mem_handle, int total_size, int map_chunk_size,
                int try_map_total)
{
#ifdef GP_TEE_ENABLED
    return tee_invoke_cmd(PMEM_CMD_TEE_MAP_TEST, mem_handle, total_size,
                          map_chunk_size, try_map_total);
#else
    UNUSED(mem_handle);
    UNUSED(total_size);
    UNUSED(map_chunk_size);
    UNUSED(try_map_total);
    return PMEM_NOT_IMPLEMENTED;
#endif
}
#endif
