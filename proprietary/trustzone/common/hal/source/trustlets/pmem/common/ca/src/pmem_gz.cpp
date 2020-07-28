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

#include <tz_cross/ta_test.h>
#include <uree/system.h>
#include <uree/mem.h>

#define LOG_TAG "PMEM_GZ"
/* LOG_TAG need to be defined before include log.h */
#include <android/log.h>

#include "pmem_dbg.h"
#include "pmem_share.h"

static const char echo_srv_name[] = "com.mediatek.geniezone.srv.echo";
static UREE_SESSION_HANDLE session = 0;
static int session_created = 0;

int gz_open()
{
    TZ_RESULT ret;

    if (session_created) {
        CONSOLE_INFO("[WARNING] session already created \n");
        return PMEM_SUCCESS;
    }

    ret = UREE_CreateSession(echo_srv_name, &session);
    if (ret != TZ_RESULT_SUCCESS) {
        CONSOLE_ERR("[ERROR] UREE_CreateSession fail \n");
        return PMEM_FAIL;
    }

    session_created = 1;
    return PMEM_SUCCESS;
}

int gz_close()
{
    TZ_RESULT ret;

    if (!session_created) {
        CONSOLE_INFO("[WARNING] session already closed \n");
        return PMEM_SUCCESS;
    }

    ret = UREE_CloseSession(session);
    if (ret != TZ_RESULT_SUCCESS) {
        CONSOLE_ERR("[ERROR] UREE_CloseSession fail \n");
        return PMEM_FAIL;
    }

    session_created = 0;
    return PMEM_SUCCESS;
}

int gz_write(int mem_handle, int mem_size, unsigned int pattern, bool check_zero)
{
    TZ_RESULT ret;
    UREE_SHAREDMEM_HANDLE shm_handle;
    MTEEC_PARAM param[4];
    uint32_t types;

    ret = UREE_ION_TO_SHM_HANDLE(mem_handle, &shm_handle);
    if (ret != TZ_RESULT_SUCCESS) {
        CONSOLE_ERR("[ERROR] UREE_ION_TO_SHM_HANDLE fail %d \n", ret);
        return PMEM_FAIL;
    }

    param[0].value.a = shm_handle;
    param[0].value.b = mem_size;
    param[1].value.a = pattern;
    param[1].value.b = check_zero;
    types =
        TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT);

    ret = UREE_TeeServiceCall(session, TZCMD_TEST_SCM_WRITE, types, param);
    if (ret != TZ_RESULT_SUCCESS) {
        CONSOLE_ERR("[ERROR] gz_write UREE_TeeServiceCall fail %d \n", ret);
        return PMEM_FAIL;
    }

    return param[2].value.a;
}

int gz_read(int mem_handle, int mem_size, unsigned int pattern)
{
    TZ_RESULT ret;
    UREE_SHAREDMEM_HANDLE shm_handle;
    MTEEC_PARAM param[4];
    uint32_t types;

    ret = UREE_ION_TO_SHM_HANDLE(mem_handle, &shm_handle);
    if (ret != TZ_RESULT_SUCCESS) {
        CONSOLE_ERR("[ERROR] UREE_ION_TO_SHM_HANDLE fail %d \n", ret);
        return PMEM_FAIL;
    }

    param[0].value.a = shm_handle;
    param[0].value.b = mem_size;
    param[1].value.a = pattern;
    types =
        TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT);

    ret = UREE_TeeServiceCall(session, TZCMD_TEST_SCM_READ, types, param);
    if (ret != TZ_RESULT_SUCCESS) {
        CONSOLE_ERR("[ERROR] gz_read UREE_TeeServiceCall fail %d \n", ret);
    }

    return param[2].value.a;
}
