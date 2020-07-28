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

#ifdef DMA_PROFILING
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
    dbgmsg("scp_addr_map num:%d\n", num);
    for (i = 0; i < num; i++) {
        if (scp_addr_map[i].ap_addr == (0xf0000000 & ap_addr)) {
            dbgmsg("ap:0x%x, scp:0x%x\n", ap_addr, scp_addr_map[i].scp_addr | (0x0fffffff & ap_addr));
            return (scp_addr_map[i].scp_addr | (0x0fffffff & ap_addr));
        }
    }
    PRINTF_E("no mapping for ap_addr:0x%x\n", ap_addr);
    return 0;
}
uint32_t scp_to_ap(uint32_t scp_addr)
{
    uint32_t i;
    uint32_t num = sizeof(scp_addr_map) / sizeof(scp_addr_map_t);
    dbgmsg("scp_addr_map num:%d\n", num);
    for (i = 0; i < num; i++) {
        if (scp_addr_map[i].scp_addr == (0xf0000000 & scp_addr)) {
            dbgmsg("ap:0x%x, scp:0x%x\n", (scp_addr_map[i].ap_addr | (0x0fffffff & scp_addr)), scp_addr);
            return (scp_addr_map[i].ap_addr | (0x0fffffff & scp_addr));
        }
    }
    PRINTF_E("no mapping for scp_addr:0x%x\n", scp_addr);
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

        if (SCP_CORE == SCP_B_ID){
            drv_write_reg32(REMAP_CFG1_DUAL, 0x0A0B0908);
            drv_write_reg32(REMAP_CFG2_DUAL, 0x0201000C);
            drv_write_reg32(REMAP_CFG3_DUAL, 0x10000A03);
        } else {
            drv_write_reg32(REMAP_CFG1, 0x0A0B0908);
            drv_write_reg32(REMAP_CFG2, 0x0201000C);
            drv_write_reg32(REMAP_CFG3, 0x10000A03);
        }
        PRINTF_E("Support Large DRAM, remap reg init\n");
    } else {
#endif
            if (SCP_CORE == SCP_B_ID){
            drv_write_reg32(REMAP_CFG1_DUAL, 0x07060504);
            drv_write_reg32(REMAP_CFG2_DUAL, 0x02010008);
            drv_write_reg32(REMAP_CFG3_DUAL, 0x10090A03);
        } else {
            drv_write_reg32(REMAP_CFG1, 0x07060504);
            drv_write_reg32(REMAP_CFG2, 0x02010008);
            drv_write_reg32(REMAP_CFG3, 0x10090A03);
        }
#if DMA_LARGE_DRAM
        PRINTF_E("Not support Large DRAM\n");
    }
#endif
}

DMA_RESULT dma_transaction_manual(uint32_t dst_addr, uint32_t src_addr, uint32_t len, void (*isr_cb)(void *),
                                  uint32_t *ch)
{
    int32_t channel = 0;
    uint32_t remain_len = 0;
    DMA_RESULT ret;
    struct mt_gdma_conf conf;
    int32_t size_per_count;

    dbgmsg("DMA ch:%d\n", *ch);
    if (len == 0) {
        PRINTF_E("DMA len=0\n");
        ret = DMA_RESULT_ERROR;
        goto _exit;
    }
    if (isr_cb != NULL) {
        PRINTF_E("DMA dis interrupt mode\n");
        ret = DMA_RESULT_ERROR;
        goto _exit;
    }
    if (ch == NULL || *ch > GENERAL_DMA_CH) {
        PRINTF_E("DMA ch error\n");
        ret = DMA_RESULT_ERROR;
        goto _exit;
    }

    memset(&conf, 0, sizeof(struct mt_gdma_conf));
    /* append scp_dual address*/
    dst_addr = scp_dual_address_append(dst_addr);
    src_addr = scp_dual_address_append(src_addr);

    /*support data size more than DMA max length */
    remain_len = len;
    while (remain_len > 0) {
        if (remain_len > MAX_DMA_TRAN_SIZE) {
                len = MAX_DMA_TRAN_SIZE;
        } else {
                remain_len = 0;
        }

        if ((channel = mt_req_gdma(*ch)) < 0) {
            PRINTF_E("DMA ch:%d is busy\n", *ch);
            ret = DMA_RESULT_FULL;
            goto _exit;
        }

        /* check count value*/
        if ( dst_addr & 0x1 || src_addr & 0x1 || len & 0x1) {
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
            conf.burst= DMA_CON_BURST_16BEAT;
        }
        conf.count = len / size_per_count;
        conf.src = src_addr;
        conf.dst = dst_addr;
        conf.iten = (isr_cb) ? DMA_TRUE : DMA_FALSE;
        conf.isr_cb = isr_cb;
        conf.data = (void *)channel;
        conf.sinc = DMA_TRUE;
        conf.dinc = DMA_TRUE;
        conf.limiter = 0;
        conf.wpen = 0;
        conf.wpsd = 0;

        if (mt_config_gdma(channel, &conf, ALL) != 0) {
            PRINTF_E("Err DMA config\n");
            ret = DMA_RESULT_ERROR;
            mt_free_gdma(channel);
            goto _exit;
        }
        /*critical section is used to ensure there is no interrupt between mt_polling_gdma */
        taskENTER_CRITICAL();
#ifdef DMA_PROFILING
        CPU_RESET_CYCLECOUNTER();
        start_time = *DWT_CYCCNT;
#endif

        mt_start_gdma(channel);
        if (!conf.iten) {
            while (mt_polling_gdma(channel, 0x10000)) {
                PRINTF_E("GDMA_%ld polling timeout\n", channel);
            }
            mt_free_gdma(channel);
            ret = DMA_RESULT_DONE;
        } else {
            *ch = channel;
            ret = DMA_RESULT_RUNNING;
        }

#ifdef DMA_PROFILING
        end_time = *DWT_CYCCNT;
        PRINTF_D("[dma] %d, %d, %d, %d\n", start_time, end_time, end_time - start_time, len);
#endif
        if ((remain_len > 0) && (ret == DMA_RESULT_DONE)){
            dst_addr += MAX_DMA_TRAN_SIZE;
            src_addr += MAX_DMA_TRAN_SIZE;
            remain_len -= MAX_DMA_TRAN_SIZE;
            len = remain_len;
        }
        if (ret != DMA_RESULT_DONE) {
            PRINTF_E("DMA fail,ret=%u,remain len=%u\n", ret, remain_len);
            remain_len = 0;
        }
        taskEXIT_CRITICAL();

    }/*support data size more than max length */

_exit:

    return ret;
}

DMA_RESULT dma_transaction(uint32_t dst_addr, uint32_t src_addr, uint32_t len)
{
    DMA_RESULT ret;
    uint32_t ch;

    ch = GENERAL_DMA_CH;
#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_enable_DRAM_resource(RTOS_MEM_ID);
#endif
    ret = dma_transaction_manual(dst_addr, src_addr, len, NULL, &ch); //busy wait, use general channel
#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_disable_DRAM_resource(RTOS_MEM_ID);
#endif
    return ret;
}

/*
 * NOTICE:
 * This 16 bits DMA API is only for access infra usage,
 * before useing DRAM need to enable DRAM resource
 */
DMA_RESULT dma_transaction_16bit(uint32_t dst_addr, uint32_t src_addr, uint32_t len)
{
    int32_t channel;
    uint32_t remain_len = 0;
    DMA_RESULT ret;
    struct mt_gdma_conf conf;
    int32_t size_per_count;
    uint32_t ch;
    ch = GENERAL_DMA_CH;
    dbgmsg("DMA ch:%d\n", ch);
    if (len == 0) {
        PRINTF_E("DMA len=0\n");
        ret = DMA_RESULT_ERROR;
        goto _16bitexit;
    }
    /* append scp_dual address*/
    dst_addr = scp_dual_address_append(dst_addr);
    src_addr = scp_dual_address_append(src_addr);

    if (len % 2 || dst_addr % 2 || src_addr % 2) {
        PRINTF_E("DMA dst_addr:0x%x or src_addr:0x%x or length:%d not 2 byte align\n", dst_addr, src_addr, len);
        ret = DMA_RESULT_ERROR;
        goto _16bitexit;
    }

    memset(&conf, 0, sizeof(struct mt_gdma_conf));
    /*support data size more than DMA max length */
    remain_len = len;
    while (remain_len > 0) {
        if (remain_len > MAX_DMA_TRAN_SIZE) {
                len = MAX_DMA_TRAN_SIZE;
        } else {
                remain_len = 0;
        }

        if ((channel = mt_req_gdma(ch)) < 0) {
            PRINTF_E("DMA ch:%d busy\n", ch);
            ret = DMA_RESULT_FULL;
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
        conf.isr_cb = NULL;
        conf.data = (void *)channel;
        conf.sinc = DMA_TRUE;
        conf.dinc = DMA_TRUE;
        conf.limiter = 0;
        conf.wpen = 0;
        conf.wpsd = 0;

        if (mt_config_gdma(channel, &conf, ALL) != 0) {
            PRINTF_E("DMA config err\n");
            ret = DMA_RESULT_ERROR;
            mt_free_gdma(channel);
            goto _16bitexit;
        }
#ifdef CFG_VCORE_DVFS_SUPPORT
    /* dvfs_enable_DRAM_resource(RTOS_MEM_ID);*/
#endif
        /*critical section is used to ensure there is no interrupt between mt_polling_gdma */
        taskENTER_CRITICAL();
#ifdef DMA_PROFILING
        CPU_RESET_CYCLECOUNTER();
        start_time = *DWT_CYCCNT;
#endif
        mt_start_gdma(channel);

        if (!conf.iten) {
            while (mt_polling_gdma(channel, 0x10000)) {
                PRINTF_E("DMA %ld polling timeout\n", channel);
            }
            mt_free_gdma(channel);
            ret = DMA_RESULT_DONE;
        } else {
            ret = DMA_RESULT_RUNNING;
        }
#ifdef DMA_PROFILING
        end_time = *DWT_CYCCNT;
        PRINTF_D("[DMA] %d, %d, %d, %d\n", start_time, end_time, end_time - start_time, len);
#endif
        if ((remain_len > 0) && (ret == DMA_RESULT_DONE)){
            dst_addr += MAX_DMA_TRAN_SIZE;
            src_addr += MAX_DMA_TRAN_SIZE;
            remain_len -= MAX_DMA_TRAN_SIZE;
            len = remain_len;
        }
        if (ret != DMA_RESULT_DONE) {
            PRINTF_E("DMA fail,ret=%u,remain len=%u\n", ret, remain_len);
            remain_len = 0;
        }
        taskEXIT_CRITICAL();
    }/*support data size more than DMA max length */
#ifdef CFG_VCORE_DVFS_SUPPORT
    /*dvfs_disable_DRAM_resource(RTOS_MEM_ID);*/
#endif
_16bitexit:
    return ret;
}


/*
 *  dst_str: dest start
 *  dst_addr: dest addr
 *  dst_sz : dest size (byte)
 *  src_addr: source address
 *  len: data size (byte)
 */
DMA_RESULT dma_transaction_wrap(uint32_t dst_str, uint32_t dst_addr, uint32_t dst_sz, uint32_t src_addr, uint32_t len, uint32_t *ch)
{
    int32_t channel = 0;
    DMA_RESULT ret;
    struct mt_gdma_conf conf;
    int32_t size_per_count;

    dbgmsg("DMA ch:%d\n", *ch);
    /*
    if (isr_cb != NULL) {
        PRINTF_E("Disable interrupt mode\n");
        ret = DMA_RESULT_ERROR;
        goto _exit;
    }
    */
    if (dst_addr < dst_str) {
        ret = DMA_RESULT_ERROR;
        goto _exit;
    }
    if (ch == NULL) {
        ret = DMA_RESULT_ERROR;
        goto _exit;
    } else if (*ch <= GENERAL_DMA_CH) {
        if ((channel = mt_req_gdma(*ch)) < 0) {
            dbgmsg("DMA ch:%d is busy\n", *ch);
            ret = DMA_RESULT_FULL;
            goto _exit;
        }
    }
    memset(&conf, 0, sizeof(struct mt_gdma_conf));
    /* append scp_dual address*/
    dst_addr = scp_dual_address_append(dst_addr);
    src_addr = scp_dual_address_append(src_addr);
    //PRINTF_D("[DMA]dst_addr = %x\n",dst_addr);
    //PRINTF_D("[DMA]src_addr = %x\n",src_addr);
    /* check count value*/
    if ( dst_addr & 0x1 || src_addr & 0x1 || len & 0x1) {
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
        conf.burst= DMA_CON_BURST_16BEAT;
    }
    conf.count = len / size_per_count;
    conf.src = src_addr;
    conf.dst = dst_addr;
    conf.iten = DMA_FALSE;
    conf.isr_cb = NULL;
    conf.data = (void *)channel;
    conf.sinc = DMA_TRUE;
    conf.dinc = DMA_TRUE;
    conf.limiter = 0;
    conf.wpen = 1;
    conf.wpsd = 1;
    conf.wplen = (dst_sz/size_per_count) - ((dst_addr - dst_str)/size_per_count);
    //PRINTF_D("size_per_count = %d\n",size_per_count);
    //PRINTF_D("conf.wplen = %x\n",conf.wplen);
    //PRINTF_D("(dst_addr - dst_str) = %x\n",(dst_addr - dst_str));
    conf.wpto = dst_str;
    //PRINTF_D("conf.wpto = %x\n",conf.wpto);
    if (mt_config_gdma(channel, &conf, ALL) != 0) {
        dbgmsg("Err DMA config\n");
        ret = DMA_RESULT_ERROR;
        mt_free_gdma(channel);
        goto _exit;
    }
#ifdef CFG_VCORE_DVFS_SUPPORT
    /*This function must enable infra to access*/
    /*dvfs_enable_DRAM_resource(RTOS_MEM_ID);*/
#endif
    /*critical section is used to ensure there is no interrupt between semaphore
     * take and release*/
    taskENTER_CRITICAL();

#ifdef DMA_PROFILING
    CPU_RESET_CYCLECOUNTER();
    start_time = *DWT_CYCCNT;
#endif
    mt_start_gdma(channel);
    if (!conf.iten) {
        while (mt_polling_gdma(channel, 0x10000)) {
            dbgmsg("GDMA_%ld polling timeout\n", channel);
        }
        mt_free_gdma(channel);
        ret = DMA_RESULT_DONE;
    } else {
        *ch = channel;
        ret = DMA_RESULT_RUNNING;
    }
#ifdef DMA_PROFILING
    end_time = *DWT_CYCCNT;
    PRINTF_D("[dma] %d, %d, %d, %d\n", start_time, end_time, end_time - start_time, len);
#endif
    taskEXIT_CRITICAL();
#ifdef CFG_VCORE_DVFS_SUPPORT
    /*This function must enable infra to access*/
    /*dvfs_disable_DRAM_resource(RTOS_MEM_ID);*/
#endif
_exit:
    return ret;
}

