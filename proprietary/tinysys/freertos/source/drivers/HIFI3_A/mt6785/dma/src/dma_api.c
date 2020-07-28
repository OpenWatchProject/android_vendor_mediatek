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

#if DMA_PROFILING
unsigned int start_time;
unsigned int end_time;
#endif

extern struct dma_id_ctrl_t dma_id_ctrl[NUMBER_DMA_ID];

typedef struct {
    uint32_t ap_addr;
    uint32_t scp_addr;
    uint32_t size;
} scp_addr_map_t;

static scp_addr_map_t scp_addr_map[] = {
    {.ap_addr    = 0x10610000, .scp_addr       = CFG_HIFI3_DRAM_ADDRESS, .size   = CFG_HIFI3_DRAM_SIZE}, //Hifi3 DTCM
    {.ap_addr    = 0x10630000, .scp_addr       = CFG_HIFI3_IRAM_ADDRESS, .size   = CFG_HIFI3_IRAM_SIZE}, //Hifi3 ITCM
    {.ap_addr    = 0x11220000, .scp_addr       = CFG_AUDIO_BASE_ADDRESS, .size   = (CFG_AUDIO_CONFIG_SIZE+CFG_AUDIO_DATA_SIZE)}, //audiosys
    {.ap_addr = CFG_HIFI3_SRAM_ADDRESS, .scp_addr = CFG_HIFI3_SRAM_ADDRESS, .size   = CFG_HIFI3_SRAM_SIZE}, //dram
};


uint32_t ap_to_scp(uint32_t ap_addr)
{
    uint32_t i;
    uint32_t num = sizeof(scp_addr_map) / sizeof(scp_addr_map_t);
    for (i = 0; i < num; i++) {
        if (ap_addr >= scp_addr_map[i].ap_addr &&
            ap_addr < scp_addr_map[i].ap_addr + scp_addr_map[i].size) {
            //PRINTF_D("ap:0x%x, scp:0x%x\n", ap_addr, ap_addr - scp_addr_map[i].ap_addr + scp_addr_map[i].scp_addr);
            return ap_addr - scp_addr_map[i].ap_addr + scp_addr_map[i].scp_addr;
        }
    }

    PRINTF_E("err ap_addr:0x%x\n", ap_addr);
    return 0;
}
uint32_t scp_to_ap(uint32_t scp_addr)
{
    uint32_t i;
    uint32_t num = sizeof(scp_addr_map) / sizeof(scp_addr_map_t);
    for (i = 0; i < num; i++) {
        if (scp_addr >= scp_addr_map[i].scp_addr &&
            scp_addr < scp_addr_map[i].scp_addr + scp_addr_map[i].size) {
            //PRINTF_D("scp:0x%x, ap:0x%x\n", scp_addr, scp_addr - scp_addr_map[i].scp_addr + scp_addr_map[i].ap_addr);
            return scp_addr - scp_addr_map[i].scp_addr + scp_addr_map[i].ap_addr;
        }
    }
    PRINTF_E("err scp_addr:0x%x\n", scp_addr);
    return 0;
}

int32_t set_scp_addr_map(uint32_t id, uint32_t ap_addr,
                         uint32_t scp_addr, uint32_t size)
{
    if (id >= NUMS_MEM)
        return -1;

    scp_addr_map[id].ap_addr = ap_addr;
    scp_addr_map[id].scp_addr = scp_addr;
    scp_addr_map[id].size = size;
    return 0;
}

#if 0
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
    }
    else {
#endif

        drv_write_reg32(REMAP_CFG1, 0x07060504);
        drv_write_reg32(REMAP_CFG2, 0x02010008);
        drv_write_reg32(REMAP_CFG3, 0x10090A03);
#if DMA_LARGE_DRAM
        PRINTF_E("Not support Large DRAM\n");
    }
#endif
}
#endif

DMA_RESULT scp_dma_transaction(uint32_t dst_addr, uint32_t src_addr,
                               uint32_t len, DMA_ID scp_dma_id, DMA_CHAN ch)
{
    uint32_t remain_len = 0;
    struct mt_gdma_conf conf;
    DMA_RESULT ret;
    int32_t config_return;
    int32_t size_per_count;
    int32_t dma_ch;
    UBaseType_t current_task_priority;
    //dbgmsg("[DMA]id:%d\n", scp_dma_id);
    if (len == 0) {
        PRINTF_E("DMA len=0\n");
        ret = DMA_RESULT_ERROR;
        goto _exit;
    }

    if ((scp_dma_id < 0) || (scp_dma_id >= NUMS_DMA_ID)) {
        PRINTF_E("DMA ID err\n");
        ret = DMA_RESULT_ERROR;
        goto _exit;
    }

    if (is_in_isr()) {
        PRINTF_E("DMA id:%d can't in isr\n", scp_dma_id);
        configASSERT(0);
    }

    memset(&conf, 0, sizeof(struct mt_gdma_conf));
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
    }
    else {
        dma_ch = mt_request_dma_channel(scp_dma_id);
        while (dma_ch < 0) {
            dma_ch = mt_request_dma_channel(scp_dma_id);
            /*in isr or critical context*/
            if (is_in_isr()) {
                ret = DMA_RESULT_NO_FREE_CH;
                goto _exit;
            }
        }
    }

    if (dma_ch < 0) {
        PRINTF_E("DMA req ch:%d fail\n", dma_ch);
        ret = DMA_RESULT_INVALID_CH;
        goto _exit;
    }

    current_task_priority = uxTaskPriorityGet(NULL);
    /*in isr or critical context*/
    if (is_in_isr()) {
        /*do not change priority in critical or isr context*/
    }
    else {
        /* rise task priority and suspend to wait DMA interrupt response here*/
        vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);
    }
    /*support data size more than DMA max length */
    remain_len = len;
    while (remain_len > 0) {
        if (remain_len > MAX_DMA_TRAN_SIZE) {
            len = MAX_DMA_TRAN_SIZE;
        }
        else {
            remain_len = 0;
        }

        /* check count value*/
        if (dst_addr & 0x1 || src_addr & 0x1 || len & 0x1) {
            conf.size_per_count = DMA_CON_SIZE_1BYTE;
            size_per_count = 1;
            conf.burst = DMA_CON_BURST_MAXBEAT;
        }
        else if (dst_addr & 0x2 || src_addr & 0x2 || len & 0x2) {
            conf.size_per_count = DMA_CON_SIZE_2BYTE;
            size_per_count = 2;
            conf.burst = DMA_CON_BURST_MAXBEAT;
        }
        else {
            conf.size_per_count = DMA_CON_SIZE_4BYTE;
            size_per_count = 4;
            conf.burst = DMA_CON_BURST_MAXBEAT;
        }
        conf.count = len / size_per_count;
        conf.src = src_addr;
        conf.dst = dst_addr;
        /*in isr or critical context*/
        if (is_in_isr()) {
            conf.iten = DMA_FALSE;
        }
        else {
            conf.iten = DMA_TRUE;
        }
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
        start_time = read_ccount();
#endif
        taskENTER_CRITICAL();
        mt_start_gdma(dma_ch);
        taskEXIT_CRITICAL();
        /*DMA interrupt mode*/
        if (is_in_isr()) {
            /*in isr or critical context no need get sema.*/
            /*use polling*/
            while (mt_polling_gdma(dma_ch, 0x10000)) {
                PRINTF_E("DMA ch:%d poll timeout\n", dma_ch);
                configASSERT(0);
            }
            mt_stop_gdma(dma_ch);
        }
        else {
            if (xSemaphoreTake(dma_id_ctrl[scp_dma_id].xDMASemaphore,
                               (TickType_t) 10000) != pdTRUE) {
                PRINTF_E("DMA id:%d err\n", scp_dma_id);
                configASSERT(0);
            }
        }
        ret = DMA_RESULT_DONE;
        if ((remain_len > 0) && (ret == DMA_RESULT_DONE)) {
            dst_addr += MAX_DMA_TRAN_SIZE;
            src_addr += MAX_DMA_TRAN_SIZE;
            remain_len -= MAX_DMA_TRAN_SIZE;
            len = remain_len;
        }
    }/*support data size more than max length */
#if DMA_PROFILING
    end_time = read_ccount();
    PRINTF_D("[DMA] %d, %d, %d, %d\n", start_time, end_time, end_time - start_time,
             len);
#endif

    /*in isr or critical context need to free ch*/
    if (is_in_isr()) {
        mt_free_gdma(dma_ch);
    }
    mt_free_dma_ctrl(scp_dma_id);
    /* resume task priority here*/
    /*in isr or critical context need to free ch*/
    if (is_in_isr()) {
    }
    else {
        vTaskPrioritySet(NULL, current_task_priority);
    }

_exit:
    if (ret != DMA_RESULT_DONE) {
        PRINTF_E("DMA fail,ret=%d\n", ret);
    }
    return ret;
}

