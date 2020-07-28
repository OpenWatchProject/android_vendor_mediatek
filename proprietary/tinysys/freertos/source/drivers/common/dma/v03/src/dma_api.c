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
/*   Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <driver_api.h>
#include <interrupt.h>
#include <dma.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif

#define DMA_DEBUG   0
#define DMA_LARGE_DRAM   0

#if(DMA_DEBUG == 1)
#define dbgmsg PRINTF_D
#else
#define dbgmsg(...)
#endif
extern UBaseType_t uxCriticalNesting;
#if DMA_PROFILING
volatile unsigned int *ITM_CONTROL = (unsigned int *)0xE0000E80;
volatile unsigned int *DWT_CONTROL = (unsigned int *)0xE0001000;
volatile unsigned int *DWT_CYCCNT = (unsigned int *)0xE0001004;
volatile unsigned int *DEMCR = (unsigned int *)0xE000EDFC;

#define CPU_RESET_CYCLECOUNTER() \
    do { \
        *DEMCR = *DEMCR | 0x01000000; \
        *DWT_CYCCNT = 0; \
        *DWT_CONTROL = *DWT_CONTROL | 1 ; \
    } while(0)
unsigned int start_time;
unsigned int end_time;
#endif

extern struct dma_id_ctrl_t dma_id_ctrl[NUMBER_DMA_ID];

typedef struct {
    uint32_t ap_addr;
    uint32_t scp_addr;
} scp_addr_map_t;

static scp_addr_map_t scp_addr_map[] = {
    {.ap_addr    = 0x40000000, .scp_addr       = 0x20000000,},
    {.ap_addr    = 0x50000000, .scp_addr       = 0x30000000,},
    {.ap_addr    = 0x60000000, .scp_addr       = 0x60000000,},
    {.ap_addr    = 0x70000000, .scp_addr       = 0x70000000,},
    {.ap_addr    = 0x80000000, .scp_addr       = 0x80000000,},
    {.ap_addr    = 0x00000000, .scp_addr       = 0x90000000,},
    {.ap_addr    = 0x10000000, .scp_addr       = 0xA0000000,},
    {.ap_addr    = 0x20000000, .scp_addr       = 0xB0000000,},
    {.ap_addr    = 0x30000000, .scp_addr       = 0xC0000000,},
    {.ap_addr    = 0x90000000, .scp_addr       = 0xF0000000,},
};


uint32_t ap_to_scp(uint32_t ap_addr)
{
    uint32_t i;
    uint32_t num = sizeof(scp_addr_map) / sizeof(scp_addr_map_t);
    for (i = 0; i < num; i++) {
        if (scp_addr_map[i].ap_addr == (0xf0000000 & ap_addr)) {
            //PRINTF_D("ap:0x%x, scp:0x%x\n", ap_addr, scp_addr_map[i].scp_addr | (0x0fffffff & ap_addr));
            return (scp_addr_map[i].scp_addr | (0x0fffffff & ap_addr));
        }
    }
    PRINTF_E("err ap_addr:0x%lx\n", ap_addr);
    return 0;
}
uint32_t scp_to_ap(uint32_t scp_addr)
{
    uint32_t i;
    uint32_t num = sizeof(scp_addr_map) / sizeof(scp_addr_map_t);
    for (i = 0; i < num; i++) {
        if (scp_addr_map[i].scp_addr == (0xf0000000 & scp_addr)) {
            //PRINTF_D("ap:0x%x, scp:0x%x\n", (scp_addr_map[i].ap_addr | (0x0fffffff & scp_addr)), scp_addr);
            return (scp_addr_map[i].ap_addr | (0x0fffffff & scp_addr));
        }
    }
    PRINTF_E("err scp_addr:0x%lx\n", scp_addr);
    return 0;
}

void scp_remap_init(void)
{
    /* SCP addr  <->AP addr
     * 0x20000000<->0x40000000
     * 0x30000000<->0x50000000
     * 0x60000000<->0x60000000
     * 0x70000000<->0x70000000
     * 0x80000000<->0x80000000
     * 0x90000000<->0x00000000
     * 0xA0000000<->0x10000000
     * 0xB0000000<->0x20000000
     * 0xC0000000<->0x30000000
     * 0xF0000000<->0x90000000
     * */
#if DMA_LARGE_DRAM
    if ((drv_reg32(0xA0001F00) & 0x2000)) {
        /* large DRAM mode enable is in preloader:
         * The SCP view of DRAM map is different in large DRAM mode and  non-large DRAM mode.
         * */
        drv_write_reg32(REMAP_CFG1, 0x0A0B0908);
        drv_write_reg32(REMAP_CFG2, 0x0201000C);
        drv_write_reg32(REMAP_CFG3, 0x10000A03);
        PRINTF_E("Support Large DRAM\n");
    } else {
#endif

        drv_write_reg32(REMAP_CFG1, 0x07060504);
        drv_write_reg32(REMAP_CFG2, 0x02010008);
        drv_write_reg32(REMAP_CFG3, 0x10090A03);
#if DMA_LARGE_DRAM
        PRINTF_E("Not support Large DRAM\n");
    }
#endif
}

DMA_RESULT scp_dma_transaction(uint32_t dst_addr, uint32_t src_addr, uint32_t len, DMA_ID scp_dma_id, DMA_CHAN ch)
{
    struct mt_gdma_conf conf;
    DMA_RESULT ret;
    int32_t config_return;
    int32_t size_per_count;
    int32_t dma_ch;
#if DMA_INTERRUPT_MODE
    UBaseType_t current_task_priority;
#endif
    //dbgmsg("[DMA]id:%d\n", scp_dma_id);
    if (len == 0) {
        PRINTF_E("DMA len=0\n");
        ret = DMA_RESULT_ERROR;
        goto _exit;
    }

    if (len > MAX_DMA_TRAN_SIZE) {
        PRINTF_E("DMA len>MAX\n");
        ret = DMA_RESULT_ERROR;
        goto _exit;
    }

    if ((scp_dma_id < I2C0_DMA_ID) || (scp_dma_id >= NUMS_DMA_ID)) {
        PRINTF_E("DMA ID err\n");
        ret = DMA_RESULT_ERROR;
        goto _exit;
    }

    if (is_in_isr()) {
        PRINTF_E("DMA id:%d can't in isr\n", scp_dma_id);
        configASSERT(0);
    }

    memset(&conf, 0, sizeof(conf));
    /*setting dma id ctrl*/
    config_return = mt_set_dma_ctrl(scp_dma_id, DMA_ID_RUNNING);
    if (config_return == DMA_RESULT_RUNNING) {
        PRINTF_E("DMA id:%d runing\n", scp_dma_id);
        ret = DMA_RESULT_RUNNING;
        goto _exit;
    }

    /*get free dma channel*/
    if (ch != NO_RESERVED) {
        /*specific ch*/
        dma_ch = mt_req_gdma(ch);
    } else {
        dma_ch = mt_request_dma_channel(scp_dma_id);
        while (dma_ch < 0) {
            dma_ch = mt_request_dma_channel(scp_dma_id);
            /*in isr or critical context*/
            if (is_in_isr() || uxCriticalNesting > 0) {
                ret = DMA_RESULT_NO_FREE_CH;
                goto _exit;
            }
        }
    }

    if (dma_ch < 0) {
        PRINTF_E("DMA req ch:%ld fail\n", dma_ch);
        ret = DMA_RESULT_INVALID_CH;
        goto _exit;
    }

#if DMA_INTERRUPT_MODE
    current_task_priority = uxTaskPriorityGet(NULL);
    /*in isr or critical context*/
    if (is_in_isr() || uxCriticalNesting > 0) {
        /*do not change priority in critical or isr context*/
    } else {
        /* rise task priority and suspend to wait DMA interrupt response here*/
        vTaskPrioritySet(NULL, PRI_DMA);
    }
#endif


    /* check count value*/
    if (dst_addr & 0x1 || src_addr & 0x1 || len & 0x1) {
        conf.size_per_count = DMA_CON_SIZE_1BYTE;
        size_per_count = 1;
        conf.burst = DMA_CON_BURST_16BEAT;
    } else if (dst_addr & 0x2 || src_addr & 0x2 || len & 0x2) {
        conf.size_per_count = DMA_CON_SIZE_2BYTE;
        size_per_count = 2;
        conf.burst = DMA_CON_BURST_16BEAT;
    } else {
        conf.size_per_count = DMA_CON_SIZE_4BYTE;
        size_per_count = 4;
        conf.burst = DMA_CON_BURST_16BEAT;
    }
    conf.count = len / size_per_count;
    conf.src = src_addr;
    conf.dst = dst_addr;
#if DMA_INTERRUPT_MODE
    /*in isr or critical context*/
    if (is_in_isr() || uxCriticalNesting > 0) {
        conf.iten = DMA_FALSE;
    } else {
        conf.iten = DMA_TRUE;
    }
#else
    conf.iten = DMA_FALSE;
#endif
    conf.isr_cb = NULL;
    conf.data = (void *)dma_ch;
    conf.sinc = DMA_TRUE;
    conf.dinc = DMA_TRUE;
    conf.limiter = 0;
    conf.wpen = 0;
    conf.wpsd = 0;

    if (mt_config_gdma(dma_ch, &conf, ALL) != 0) {
        PRINTF_E("DMA cfg err\n");
        ret = DMA_RESULT_ERROR;
        mt_free_gdma(dma_ch);
        mt_free_dma_ctrl(scp_dma_id);
        goto _exit;
    }
#if DMA_PROFILING
    CPU_RESET_CYCLECOUNTER();
    start_time = *DWT_CYCCNT;
#endif
    taskENTER_CRITICAL();
    mt_start_gdma(dma_ch);
    taskEXIT_CRITICAL();
#if DMA_INTERRUPT_MODE
    /*DMA interrupt mode*/
    if (is_in_isr() || uxCriticalNesting > 0) {
        /*in isr or critical context no need get sema.*/
        /*use polling*/
        while (mt_polling_gdma(dma_ch, 0x10000)) {
            PRINTF_E("DMA ch:%ld poll timeout\n", dma_ch);
        }
        mt_stop_gdma(dma_ch);
    } else {
        if (xSemaphoreTake(dma_id_ctrl[scp_dma_id].xDMASemaphore, (TickType_t)10000) != pdTRUE) {
            PRINTF_E("DMA id:%d err\n", scp_dma_id);
            configASSERT(0);
        }
    }
#else
    /*DMA polling mode*/
    while (mt_polling_gdma(dma_ch, 0x10000)) {
        PRINTF_E("DMA ch:%ld poll timeout\n", dma_ch);
    }
    mt_stop_gdma(dma_ch);
#endif
    ret = DMA_RESULT_DONE;


#if DMA_PROFILING
    end_time = *DWT_CYCCNT;
    PRINTF_D("[DMA] %d, %d, %d, %d\n", start_time, end_time, end_time - start_time, len);
#endif

#if DMA_INTERRUPT_MODE
    /*in isr or critical context need to free ch*/
    if (is_in_isr() || uxCriticalNesting > 0) {
        mt_free_gdma(dma_ch);
    }
#else
    mt_free_gdma(dma_ch);
#endif
    mt_free_dma_ctrl(scp_dma_id);
#if DMA_INTERRUPT_MODE
    /* resume task priority here*/
    /*in isr or critical context need to free ch*/
    if (is_in_isr() || uxCriticalNesting > 0) {
    } else {
        vTaskPrioritySet(NULL, current_task_priority);
    }

#endif
    if (ret != DMA_RESULT_DONE)
        PRINTF_E("DMA fail,ret=%d\n", ret);

_exit:
    return ret;
}

DMA_RESULT scp_dma_transaction_dram(uint32_t dst_addr, uint32_t src_addr, uint32_t len, DMA_ID scp_dma_id, DMA_CHAN ch)
{
    DMA_RESULT ret;

#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_enable_DRAM_resource(DMA_MEM_ID);
#endif
    ret = scp_dma_transaction(dst_addr, src_addr, len, scp_dma_id, ch); //busy wait, use general channel
#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_disable_DRAM_resource(DMA_MEM_ID);
#endif
    return ret;
}

/*
 * NOTICE:
 * This 16 bits DMA API is only for access infra usage,
 * before useing DRAM need to enable DRAM resource
 */
DMA_RESULT scp_dma_transaction_16bit(uint32_t dst_addr, uint32_t src_addr, uint32_t len, DMA_ID scp_dma_id, DMA_CHAN ch)
{
    struct mt_gdma_conf conf;
    DMA_RESULT ret;
    int32_t config_return;
    int32_t size_per_count;
    int32_t dma_ch;

    //dbgmsg("DMA16 id:%d\n", scp_dma_id);
    if (len == 0) {
        PRINTF_E("DMA16 len=0\n");
        ret = DMA_RESULT_ERROR;
        goto _16bitexit;
    }

    if (len > MAX_DMA_TRAN_SIZE) {
        PRINTF_E("DMA16 len>MAX\n");
        ret = DMA_RESULT_ERROR;
        goto _16bitexit;
    }

    if ((scp_dma_id < I2C0_DMA_ID) || (scp_dma_id >= NUMS_DMA_ID)) {
        PRINTF_E("DMA16 ID err\n");
        ret = DMA_RESULT_ERROR;
        goto _16bitexit;
    }

    if (len % 2 || dst_addr % 2 || src_addr % 2) {
        PRINTF_E("DMA16 dst:0x%lx src:0x%lx len:%lu not 2byte align\n", dst_addr, src_addr, len);
        ret = DMA_RESULT_ERROR;
        goto _16bitexit;
    }
    /*get free dma channel*/
    if (ch != NO_RESERVED) {
        /*specific ch*/
        dma_ch = mt_req_gdma(ch);
    } else {
        dma_ch = mt_request_dma_channel(scp_dma_id);
        while (dma_ch < 0) {
            dma_ch = mt_request_dma_channel(scp_dma_id);
        }
    }

    if (dma_ch < 0) {
        //PRINTF_E("DMA req ch:%ld fail\n",dma_ch);
        ret = DMA_RESULT_INVALID_CH;
        goto _16bitexit;
    }
    memset(&conf, 0, sizeof(conf));
    /*setting dma id ctrl*/
    config_return = mt_set_dma_ctrl(scp_dma_id, DMA_ID_RUNNING);
    if (config_return == DMA_RESULT_RUNNING) {
        //PRINTF_E("DMA16 id:%d runing\n", scp_dma_id);
        ret = DMA_RESULT_RUNNING;
        goto _16bitexit;
    }

    /*get free dma channel*/
    dma_ch = mt_request_dma_channel(scp_dma_id);
    if (dma_ch < 0) {
        //PRINTF_E("DMA16 no free\n");
#if DMA_DUMP_INFO
        mt_dump_dma_struct();
#endif
        mt_free_dma_ctrl(scp_dma_id);
        ret = DMA_RESULT_NO_FREE_CH;
        goto _16bitexit;
    }

    /*16 bit trasaction setup*/
    conf.size_per_count = DMA_CON_SIZE_2BYTE;
    size_per_count = 2;
    conf.burst = DMA_CON_BURST_16BEAT;

    conf.count = len / size_per_count;
    conf.src = src_addr;
    conf.dst = dst_addr;
    conf.iten = DMA_FALSE;
    //conf.iten = DMA_TRUE;
    conf.isr_cb = NULL;
    conf.data = (void *)dma_ch;
    conf.sinc = DMA_TRUE;
    conf.dinc = DMA_TRUE;
    conf.limiter = 0;
    conf.wpen = 0;
    conf.wpsd = 0;

    if (mt_config_gdma(dma_ch, &conf, ALL) != 0) {
        PRINTF_E("DMA16 cfg err\n");
        ret = DMA_RESULT_ERROR;
        mt_free_gdma(dma_ch);
        mt_free_dma_ctrl(scp_dma_id);
        goto _16bitexit;
    }
#if DMA_PROFILING
    CPU_RESET_CYCLECOUNTER();
    start_time = *DWT_CYCCNT;
#endif
    taskENTER_CRITICAL();
    mt_start_gdma(dma_ch);

    /*DMA polling mode*/
    while (mt_polling_gdma(dma_ch, 0x10000)) {
        PRINTF_E("DMA16 ch:%ld polling timeout\n", dma_ch);
    }
    mt_stop_gdma(dma_ch);
    ret = DMA_RESULT_DONE;
    taskEXIT_CRITICAL();
#if DMA_PROFILING
    end_time = *DWT_CYCCNT;
    PRINTF_D("[dma] %d, %d, %d, %d\n", start_time, end_time, end_time - start_time, len);
#endif


    mt_free_gdma(dma_ch);
    mt_free_dma_ctrl(scp_dma_id);
    if (ret != DMA_RESULT_DONE)
        PRINTF_E("DMA16 fail,ret=%d\n", ret);

_16bitexit:
    return ret;
}

