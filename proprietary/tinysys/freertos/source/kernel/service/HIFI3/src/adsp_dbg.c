/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver_api.h"
#include "mt_reg_base.h"
#include "hal_cache.h"
#include "adsp_ipi.h"
#include "adsp_dbg.h"
#if defined(CFG_UT_TESTSUITE_SUPPORT)
#include "test_main.h"
#endif


#define MINIMUM_TRACE_SIZE   (128 * 1024) // 128KB
#define MINIMUM_DEBUG_SIZE   (64 * 1024) + MINIMUM_TRACE_SIZE // unless 192KB

static char *pDebugBuf;
static unsigned long ulDebugBufSize;
static char *pTraceBuf;
static unsigned long ulTraceBufSize;
void set_debug_buffer(void* addr, unsigned long size)
{
    if (!addr || size < MINIMUM_DEBUG_SIZE) {
        PRINTF_E("%s(),fail NULL address or size < minimum debug size\n", __func__);
        return;
    }
    memset(addr, '\0', size);

    pTraceBuf = (void *)addr + size - MINIMUM_TRACE_SIZE;
    ulTraceBufSize = MINIMUM_TRACE_SIZE;
    pDebugBuf = addr;
    ulDebugBufSize = size - MINIMUM_TRACE_SIZE;

    printf("set Debug Dump: pDebugBuf %p, ulDebugBufSize %lu, pTraceBuf %p, ulTraceBufSize %lu\n",
           pDebugBuf, ulDebugBufSize, pTraceBuf, ulTraceBufSize);
}

int get_debug_buffer(char** addr, unsigned long *size)
{
    if (!pDebugBuf) {
        PRINTF_E("%s, ERROR NULL address\n", __func__);
        return -1;
    }
    *addr = pDebugBuf;
    *size = ulDebugBufSize;
    return 0;
}

#ifdef CFG_TASK_MONITOR
extern TimerHandle_t xTaskMonTimer;
void adsp_task_monitor_start(void *data)
{
    int value = *(int *)data;

    if (xTaskMonTimer == NULL) {
        PRINTF_W("%s(), xTaskMonTimer not Create\n", __func__);
        return;
    }

    if (value >= 50) //unless 50ms Period
        xTimerChangePeriod(xTaskMonTimer, value / portTICK_RATE_MS, 0);
    else
        xTimerStart(xTaskMonTimer, 0);
}

void adsp_task_monitor_stop(void *data)
{
    if (xTaskMonTimer == NULL) {
        PRINTF_W("%s(), xTaskMonTimer not Create\n", __func__);
        return;
    }

    if (xTimerIsTimerActive(xTaskMonTimer)) {
        xTimerStop(xTaskMonTimer, 0);
    }
}

void adsp_task_monitor_dump(void *data)
{
    char *buf;
    unsigned long size, n = 0;

    if (get_debug_buffer(&buf, &size) == 0) {
        memset(buf, '\0', size);
        n += snprintf(buf + n, size - n, "Heap:free/total size:%d/%d\n", xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);
        n += snprintf(buf + n, size - n, "Task Status:\n");
        vTaskList(buf + n);

        hal_cache_region_operation(HAL_CACHE_TYPE_DATA, HAL_CACHE_OPERATION_FLUSH,
                                   buf, size);
    }
}
#endif

typedef struct xTraceFrame
{
    unsigned long long time;
    unsigned int type;
    unsigned int id;
}TraceFrame;

static bool trace_enable;
static TraceFrame *pTf;
void adsp_trace_start(void *data)
{
    pTf = (TraceFrame *)pTraceBuf;
    memset(pTraceBuf, '\0', ulTraceBufSize);
    trace_enable = true;
}

void adsp_trace_stop(void *data)
{
    TraceFrame *ptmp;
    char *buf;
    unsigned long size, n = 0;

    trace_enable = false;

    if (get_debug_buffer(&buf, &size)) {
        PRINTF_E("%s(), get_debug_buffer FAIL! \n", __func__);
        return;
    }

    memset(buf, '\0', size);

    n += snprintf(buf + n, size - n, "Trace: time, type, id\n");
    for (ptmp = (TraceFrame *)pTraceBuf; ptmp < (TraceFrame *)(pTraceBuf + ulTraceBufSize); ptmp++) {
        if (ptmp->time == 0)
            continue;

        if (ptmp == pTf)
            n += snprintf(buf + n, size - n, "%llu, %u, %s <-- pTf here\n", ptmp->time, ptmp->type, (char *)ptmp->id);
        else
            n += snprintf(buf + n, size - n, "%llu, %u, %s\n", ptmp->time, ptmp->type, (char *)ptmp->id);
    }

    hal_cache_region_operation(HAL_CACHE_TYPE_DATA, HAL_CACHE_OPERATION_FLUSH,
                               buf, size);
}

void trace_save(unsigned long long time, unsigned int type, unsigned int id)
{
    if (trace_enable) {
        pTf = pTf < (TraceFrame *)(pTraceBuf + ulTraceBufSize) ? pTf : (TraceFrame *)pTraceBuf;
        pTf->time = time;
        pTf->type = type;
        pTf->id = id;
        pTf++;
    }
}

#if defined(CFG_VCORE_DVFS_SUPPORT)
extern uint32_t dump_feature_status(char *buf, int size);
extern uint32_t dump_dvfs_status(char *buf, int size);
void adsp_dvfs_dump(void *data)
{
    char *buf;
    unsigned long size, n;

    if (get_debug_buffer(&buf, &size)) {
        PRINTF_E("%s, get_debug_buffer FAIL!\n", __func__);
        return;
    }

    memset(buf, '\0', size);
    n = 0;

    n += dump_feature_status(buf, size);
    n += dump_dvfs_status(buf + n, size - n);

    hal_cache_region_operation(HAL_CACHE_TYPE_DATA, HAL_CACHE_OPERATION_FLUSH,
                               buf, size);
}
#endif

void adsp_heap_dump(void *data)
{
    char *buf;
    unsigned long size, n;

    if (get_debug_buffer(&buf, &size)) {
        PRINTF_E("%s, get_debug_buffer FAIL!\n", __func__);
        return;
    }

    memset(buf, '\0', size);
    n = 0;

    n += vPortHeapDump(buf, size);

    hal_cache_region_operation(HAL_CACHE_TYPE_DATA, HAL_CACHE_OPERATION_FLUSH,
                               buf, size);
}

static const struct cmd_fn common_cmds[] = {
    CMDFN("trace_start", adsp_trace_start),
    CMDFN("trace_stop", adsp_trace_stop),
#if defined(CFG_TASK_MONITOR)
    CMDFN("task_monitor_start", adsp_task_monitor_start),
    CMDFN("task_monitor_stop", adsp_task_monitor_stop),
    CMDFN("task_monitor_dump", adsp_task_monitor_dump),
#endif
#if defined(CFG_VCORE_DVFS_SUPPORT)
    CMDFN("dvfs_dump", adsp_dvfs_dump),
#endif
    CMDFN("heap_dump", adsp_heap_dump),
#if defined(CFG_UT_TESTSUITE_SUPPORT)
    CMDFN("ut", testsuite_handler),
#endif
    CMDFN("cmds", adsp_dbg_cmds),
    {}
};

void adsp_dbg_cmds(void *data)
{
    const struct cmd_fn *cf;
    char *buf;
    unsigned long size, n;

    if (get_debug_buffer(&buf, &size)) {
        PRINTF_E("%s, get_debug_buffer FAIL!\n", __func__);
        return;
    }

    memset(buf, '\0', size);
    n = 0;

    for (cf = common_cmds; cf->cmd; cf++)
        n += snprintf(buf + n, size - n,  "%s\n", cf->cmd);

    hal_cache_region_operation(HAL_CACHE_TYPE_DATA, HAL_CACHE_OPERATION_FLUSH,
                               buf, size);
    return;
}

void dbg_cmds_handler(int id, void *data, unsigned int len)
{
    const struct cmd_fn *cf;
    char *temp = NULL, *token = NULL, *saveptr = NULL;
    char delim[] = " ,\t\n";
    int value = 0;

    temp = (char *)data;
    token = strtok_r(temp, delim, &saveptr);

    for (cf = common_cmds; cf->cmd; cf++) {
        if (strcmp(cf->cmd, token) == 0) {
            token = strtok_r(NULL, delim, &saveptr);
            value = (token == NULL) ? 0 : atol(token);
            return cf->fn((void *)&value);
        }
    }
}

void dbg_cmds_init(void)
{
    adsp_ipi_registration(IPI_ADSP_TIMER, dbg_cmds_handler, "DbgCmd");
}

