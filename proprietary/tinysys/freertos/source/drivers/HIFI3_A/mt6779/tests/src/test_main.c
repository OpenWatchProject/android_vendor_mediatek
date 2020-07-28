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


#include <string.h>
#include <stdio.h>
#include <interrupt.h>
#include "FreeRTOS.h"
#include "task.h"
#include <adsp_ipi.h>
#include "test_main.h"

enum testsuite_id {
    TEST_IPI = 0,
    TEST_DMA = 1,
    TEST_WDT = 2,
    TEST_XGPT = 3,
    TEST_CACHE = 4,
    TEST_DVFS = 5,
    NUMS_TESTSUITE,
};
#if TESTSUITE_DMA
extern void testsuite_dma_dispatcher(uint32_t id);
#endif
#if TESTSUITE_XGPT
extern void testsuite_xpgt_dispatcher(uint32_t id);
#endif
#if TESTSUITE_WDT
extern void testsuite_wdt_dispatcher(uint32_t id);
#endif
#if TESTSUITE_CACHE
extern void testsuite_cache_dispatcher(uint32_t id);
#endif
#if TESTSUITE_DVFS
extern void testsuite_dvfs_dispatcher(uint32_t id);
#endif

static void (*testsuite_dispatcher_func_array[NUMS_TESTSUITE])(uint32_t) = {
#if TESTSUITE_DMA
    [TEST_DMA] = testsuite_dma_dispatcher,
#endif
#if TESTSUITE_XGPT
    [TEST_XGPT] = testsuite_xpgt_dispatcher,
#endif
#if TESTSUITE_WDT
    [TEST_WDT] = testsuite_wdt_dispatcher,
#endif
#if TESTSUITE_CACHE
    [TEST_CACHE] = testsuite_cache_dispatcher,
#endif
#if TESTSUITE_DVFS
    [TEST_DVFS] = testsuite_dvfs_dispatcher,
#endif
};

static TaskHandle_t xtest_handle = NULL;

static uint32_t get_testsuite_id(uint32_t value)
{
    return (value >> 6) & 0xf; //[6:9] bits
}

static uint32_t get_testcase_id(uint32_t value)
{
    return value & 0x3f; //[0:5] bits
}

static void testsuite_processing_loop(void *pdata)
{
    uint32_t ulNotifiedValue;
    for(;;)
    {
        xTaskNotifyWait(0x00,      /* Don't clear any notification bits on entry. */
                        0xffffffff, /* Reset the notification value to 0 on exit. */
                        &ulNotifiedValue, /* Notified value pass out in ulNotifiedValue. */
                        portMAX_DELAY );  /* Block indefinitely. */
        uint32_t testsuite_id = get_testsuite_id(ulNotifiedValue);
        uint32_t testcase_id = get_testcase_id(ulNotifiedValue);

        if (testsuite_id < NUMS_TESTSUITE) {
            if (testsuite_dispatcher_func_array[testsuite_id] != NULL) {
                testsuite_dispatcher_func_array[testsuite_id](testcase_id);
            } else {
                PRINTF_D("testsuite id(%d) dispatcher not register\n", testsuite_id);
            }
        } else {
            PRINTF_D("don't have this testsuite %d\n", testsuite_id);
        }
    }
}

void testsuite_handler(void *data)
{
    unsigned int buffer = *(unsigned int *)data;

    if (!xtest_handle)
        return;

    if (is_in_isr()) { //if using adsp_ipi_send_ipc is in isr
        BaseType_t xHigherPriorityTaskWoken;
        xHigherPriorityTaskWoken = pdFALSE;
        if (xTaskNotifyFromISR(xtest_handle, buffer, eSetValueWithoutOverwrite,
                           &xHigherPriorityTaskWoken) != pdPASS) {
            PRINTF_D("[IN_ISR]The task's notification value was not updated\n");
        }
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    } else {
        if(xTaskNotify(xtest_handle, buffer, eSetValueWithoutOverwrite) != pdPASS) {
            PRINTF_D("The task's notification value was not updated\n");
        }
    }
};

void testsuite_init(void)
{
    // task create
    xTaskCreate(testsuite_processing_loop, "TestSuite", 1024, NULL, 2, &xtest_handle);
}

