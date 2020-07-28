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

#include "main.h"
#include <string.h>
/*  Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <feature_manager.h>
#include "scp_ipi.h"
#include "dma.h"
#include "interrupt.h"
#include "scp_scpctl.h"
#include "dram_management_api.h"

#ifdef CFG_CACHE_SUPPORT
#include "cache_internal.h"
#endif

#ifdef CFG_AUDIO_SUPPORT
#include <audio.h>
#endif

#ifdef CFG_DO_ENABLED
#include "do_service.h"
#endif

#ifdef CFG_FEATURE01_SUPPORT
#include "feature01_sample.h"
#endif

#ifdef CFG_FEATURE03_SUPPORT
#include "feature03_os.h"
#endif

#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif

#ifdef CFG_SENSOR_GPIO_SWITCH_SUPPORT
#include "gpio_i2c_switch.h"
#endif

#ifdef CFG_SCP_STRESS_SUPPORT

#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif

unsigned int task_stress_flag = 0;
static void scp_ipi_stress(int id, void *data, unsigned int len)
{
    unsigned int buffer;
    buffer = *(unsigned int *)data;
    PRINTF_E("Get stress ipi buf=%u, id=%u\n", buffer, id);
    task_stress_flag = 1;
    PRINTF_E("Stress enabled\n");
}

static void vTaskSCPStress(void *pvParameters)
{
    portTickType xLastExecutionTime, xDelayTime;
    xLastExecutionTime = xTaskGetTickCount();
    unsigned int get_random_time;

    xDelayTime = (portTickType) 60000 / portTICK_RATE_MS ;
    /*init stress ipi handler*/
    scp_ipi_registration(IPI_TEST1, (ipi_handler_t)scp_ipi_stress, "IPIstress");
    do {
        PRINTF_E("Task Stress ");
        if (task_stress_flag == 0) {
            PRINTF_E("status=%u\n", task_stress_flag);
        } else {
            /* stress flag is enable*/
            PRINTF_E("status=%u, stress enable!\n", task_stress_flag);
            /* get 60 ~120 sec delay*/
#ifdef CFG_XGPT_SUPPORT
            get_random_time = (unsigned int)(get_boot_time_ns() % 60);
            get_random_time = (30 + get_random_time) * 1000;
            xDelayTime = (portTickType) get_random_time / portTICK_RATE_MS ;
#endif
            PRINTF_E("after:%ums, will abort...\n", xDelayTime);
            vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
            configASSERT(0);
        }
        xDelayTime = (portTickType) 60000 / portTICK_RATE_MS ;
        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
    } while (1);
}
#endif

#ifdef CFG_CACHE_SUPPORT
void l1c_remap(void)
{
    /* config l1-cache mapping, we set 4MB currently */
    struct scp_region_info_st* region_info_p = INFO_BASE_POINTER;
    uint32_t cache_dram_addres = (region_info_p->ap_cached_start >> L1C_EXT_ADDR_SHIFT);

    switch (L1C_DRAM_ADDR) {
    case 0x80000:
        /* Cache view starts from 0x80000, so the start address of DRAM should be set to
         * register, L1C_EXT_ADDR0, which is the least significant half of register,
         * L1C_REMAP_CFG0.
         */
        DRV_WriteReg32(L1C_REMAP_CFG0, ((cache_dram_addres + 1) << 16) | (cache_dram_addres + 0));
        DRV_WriteReg32(L1C_REMAP_CFG1, ((cache_dram_addres + 3) << 16) | (cache_dram_addres + 2));
        break;
    case 0x100000:
        /* Howerver, in this case, cache view starts from 0x100000 and the start address of
         * DRAM should be set to register, L1C_EXT_ADDR1, most significant half of reigster,
         * L1C_REMAP_CFG0.
         */
        DRV_WriteReg32(L1C_REMAP_CFG0, ((cache_dram_addres + 0) << 16) | (cache_dram_addres + 0));
        DRV_WriteReg32(L1C_REMAP_CFG1, ((cache_dram_addres + 2) << 16) | (cache_dram_addres + 1));
        break;
    default:
        configASSERT(0);
    }

    /* disable sleep protect */
    hal_cache_scp_sleep_protect_off();
}

void l1c_recovery(void)
{
    struct scp_region_info_st* region_info_p = INFO_BASE_POINTER;
    int cached_size = (int) region_info_p->ap_cached_size;

    if (cached_size <= 0)
        return;

    PRINTF_E("recover dram region, size = %d\n", cached_size);

    /* trun on dram */
    dvfs_enable_DRAM_resource(DRAM_MANAGER_MEM_ID);

    /* invlaidate all cache entries */
    hal_cache_invalidate_all_cache_lines(CACHE_DCACHE);
    hal_cache_invalidate_all_cache_lines(CACHE_ICACHE);

    memcpy((void *)L1C_DRAM_ADDR,
           (void *)(L1C_DRAM_ADDR + L1C_DRAM_SIZE), /* l1c's backup address */
           cached_size);

    /* flush all cached data to memory */
    hal_cache_flush_all_cache_lines(CACHE_ICACHE);
    hal_cache_flush_all_cache_lines(CACHE_DCACHE);

    /* trun off dram */
    dvfs_disable_DRAM_resource(DRAM_MANAGER_MEM_ID);
}

#ifdef CFG_CACHE_MON_DRAM_LATENCY
#define NS_PER_CYCLE    4 /* freq:250Mhz -> 4ns per cycle  */
unsigned int latency_avg[4] = {0};

static void __cache_miss_latency_test(void)
{
    uint32_t cycles; /* number of cycles */
    static int offset = 0;
    unsigned int rvalue;
    uint64_t  cache_unhit_cs_ori, cache_unhit_cs_now;

    if (latency_avg[0] > 256)
        return;

    if (latency_avg[3] == 0)
        latency_avg[3] = 0x7fff;

#define CPU_RESET_CYCLECOUNTER() do { \
                    *DEMCR = *DEMCR | 0x01000000; \
                    *DWT_CYCCNT = 0; \
                    *DWT_CONTROL = *DWT_CONTROL | 1 ; \
                } while(0)

    //volatile unsigned int *ITM_CONTROL = (unsigned int *)0xE0000E80;
    volatile unsigned int *DWT_CONTROL = (unsigned int *)0xE0001000;
    volatile unsigned int *DWT_CYCCNT = (unsigned int *)0xE0001004;
    volatile unsigned int *DEMCR = (unsigned int *)0xE000EDFC;

    while (1) {
        cache_unhit_cs_ori = cache_access_count(CACHE_DCACHE) - cache_hit_count(CACHE_DCACHE);
        CPU_RESET_CYCLECOUNTER();

        /* access the range of dram, 0x80010 ~ 0x100000 */
        rvalue = *(volatile unsigned int *)(0x80010 + offset); /* do cache miss */

        /* get cycle counter */
        cycles = DRV_Reg32(DWT_CYCCNT);

        cache_unhit_cs_now = cache_access_count(CACHE_DCACHE) - cache_hit_count(CACHE_DCACHE);
        if (cache_unhit_cs_now - cache_unhit_cs_ori == 1) {
            latency_avg[0]++;               /* total times of dram access */
            latency_avg[1] += cycles;       /* total cycles of dram access */
            if (cycles > latency_avg[2])    /* maximal cycles of single dram access */
                latency_avg[2] = cycles;

            if (cycles < latency_avg[3])
                latency_avg[3] = cycles;    /* minimal cycles of single dram access */
            PRINTF_E("CLatency [%d]=%08x cyc=%d(%d) avg=%d.%d min:%d max:%d\n",
                     latency_avg[0], rvalue, cycles * NS_PER_CYCLE, cycles,
                     NS_PER_CYCLE * latency_avg[1] / latency_avg[0],
                     NS_PER_CYCLE * ((latency_avg[1] % latency_avg[0]) * 100) / latency_avg[0],
                     NS_PER_CYCLE * latency_avg[3],
                     NS_PER_CYCLE * latency_avg[2]);
            break;
        }

        offset += 32;
        if (offset >= 0x0100000)
            offset = 0;
    }
}
#endif

#ifdef CFG_CACHE_MON_HIT_RATIO
static void __cache_hit_ratio(void)
{
    uint64_t  cache_access_cs, cache_unhit_cs;
    double tmp;
    cache_access_cs = ((uint64_t)hal_cache_access_countu(CACHE_ICACHE) << 32) +
                      (uint64_t)hal_cache_access_countl(CACHE_ICACHE);
    cache_unhit_cs = ((uint64_t)hal_cache_hit_countu(CACHE_ICACHE) << 32) +
                     (uint64_t)hal_cache_hit_countl(CACHE_ICACHE);
    cache_unhit_cs = cache_access_cs - cache_unhit_cs;
    tmp = (double)(cache_unhit_cs * 100) / (double)cache_access_cs;
    PRINTF_D("I unhit %llu/%llu(%f)\n", cache_unhit_cs, cache_access_cs, tmp);
    cache_access_cs = ((uint64_t)hal_cache_access_countu(CACHE_DCACHE) << 32) +
                      (uint64_t)hal_cache_access_countl(CACHE_DCACHE);
    cache_unhit_cs = ((uint64_t)hal_cache_hit_countu(CACHE_DCACHE) << 32) +
                     (uint64_t)hal_cache_hit_countl(CACHE_DCACHE);
    cache_unhit_cs = cache_access_cs - cache_unhit_cs;
    tmp = (double)(cache_unhit_cs * 100) / (double)cache_access_cs;
    PRINTF_D("D unhit %llu/%llu(%f)\n", cache_unhit_cs, cache_access_cs, tmp);
}
#endif
#endif /* CFG_CACHE_SUPPORT */

static unsigned int ix = 0;

#ifdef CFG_CACHE_MON_DRAM_LATENCY
#define mainCHECK_DELAY                                         ((portTickType) 50 / portTICK_RATE_MS)
#else
#define mainCHECK_DELAY                                         ((portTickType) 60000 / portTICK_RATE_MS)
#endif

#ifdef CFG_DEBUG_TASK_STATUS
static char list_buffer[512];

static void __task_status(void)
{
    vTaskList(list_buffer);
    PRINTF_E("[%d]Heap:free/total:%d/%d\n", ix, xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);
    PRINTF_E("Task Status:\n\r%s", list_buffer);
    PRINTF_E("max dur.:%llu,limit:%llu\n", get_max_cs_duration_time(), get_max_cs_limit());
}
#endif

void vTaskMonitor(void *pvParameters)
{
    portTickType xLastExecutionTime, xDelayTime;
    xDelayTime = mainCHECK_DELAY;

    do {
        xLastExecutionTime = xTaskGetTickCount();
#ifdef CFG_CACHE_SUPPORT
#ifdef CFG_CACHE_MON_DRAM_LATENCY
        __cache_miss_latency_test();
#endif
#ifdef CFG_CACHE_MON_HIT_RATIO
        __cache_hit_ratio();
#endif
#endif  /* CFG_CACHE_SUPPORT */
#ifdef CFG_DEBUG_TASK_STATUS
        __task_status();
#endif
#ifdef CFG_IPI_STAMP_SUPPORT
        ipi_status_dump();
#endif
        ix ++;
        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
#ifdef PBFR_SUPPORT_PERF_BUDGET
        pbfr_report_loadinfo(PERIOD_MOD);
#endif
    } while (1);
}


void ap_wdt_irq_init(void)
{
    NVIC_SetPriority(AP_WDT_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY - 1);
    NVIC_EnableIRQ(AP_WDT_IRQn);
}

/***************************************************************************
 * Get parameters from AP sensor drivers
 * @Return: base address of parameter buffer specific to devleoper
 ***************************************************************************
 * Example:
 *  int *params = (int *)get_ap_params();
 *      PRINTF_E("@@ Example @@: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
 *       params, params[0], params[1], params[2], params[3]);
 */
unsigned int get_ap_params(void)
{
    struct scp_region_info_st* region_info_p = INFO_BASE_POINTER;
    return ap_to_scp(region_info_p->ap_params_start);
}

void platform_init(void)
{
    PRINTF_D("Init platform\n");

#ifdef CFG_FREERTOS_TRACE_SUPPORT
    extern int ondiemet_main(void);
    ondiemet_main();
#endif

#ifdef PBFR_SUPPORT_PERF_BUDGET
    pbfr_init();
    pbfr_start_loadinfo(PERIOD_MOD);
#endif /* PBFR_SUPPORT_PERF_BUDGET */
    scpctl_init();
#ifdef CFG_SCP_STRESS_SUPPORT
    xTaskCreate(vTaskSCPStress, "Stres", 384, (void*)4, PRI_STRESS, NULL);
#endif  /* CFG_SCP_STRESS_SUPPORT */

#ifdef CFG_AUDIO_SUPPORT
    audio_init();
#endif  /* CFG_AUDIO_SUPPORT */
#ifdef CFG_CHRE_SUPPORT
    extern void CHRE_init(void);
    CHRE_init();
#endif  /* CFG_CHRE_SUPPORT */

#ifdef CFG_DO_ENABLED
    do_service_init();
#else
#ifdef CFG_FEATURE03_SUPPORT
    feat3_init();
#endif
#endif  /*CFG_DO_ENABLED*/

#ifdef CFG_FEATURE01_SUPPORT
    fb_sample_task_init();
#endif

#ifdef CFG_SENSOR_GPIO_SWITCH_SUPPORT
    gpio_i2c_switch();
#endif
    ipi_status_dump();
    irq_status_dump();
    feature_manager_init();

    return;
}

void platform_post_init()
{
#ifdef CFG_CACHE_SUPPORT
    /*
     * Assure the function, xDramManagerInit, locates behind any function
     * instances in DRAM to avoid MemMgr exception fault happening before
     * vTaskStartScheduler().
     * Auto-dram on/off depends on task context in current design.
     */
    xDramManagerInit();
#endif
}

