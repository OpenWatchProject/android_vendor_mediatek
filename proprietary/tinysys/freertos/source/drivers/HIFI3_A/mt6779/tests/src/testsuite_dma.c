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
#include "FreeRTOS.h"
#include "task.h"
#include <adsp_ipi.h>
#include <driver_api.h>
#include "dma.h"
#include "hal_cache.h"
#include "mt_gpt.h"

typedef struct {
    uint32_t *src;
    uint32_t *dst;
    uint32_t from_dram;
    uint32_t to_dram;
} dma_dvt_entry_t;

enum dma_direction {
    DMA_TCM2EMI = 0,
    DMA_EMI2TCM = 1,
    DMA_AUD2EMI = 2,
    DMA_EMI2AUD = 3,
    DMA_TCM2AUD = 4,
    DMA_AUD2TCM = 5,
    DMA_EMI2EMI_0 = 6,
    DMA_EMI2EMI_1 = 7,
    DMA_EMI2EMI_2 = 8,
    DMA_EMI2EMI_3 = 9,
    DMA_EMI2EMI_4 = 10,
    DMA_EMI2EMI_5 = 11,
    DMA_NUM_DIRECTION,
};

const dma_dvt_entry_t dma_dvt_array[DMA_NUM_DIRECTION] = {
    {(unsigned int *)0x4ffb5700, (unsigned int *)0x562FC000, 0, 1},    // tcm      --> emi
    {(unsigned int *)0x562FC000, (unsigned int *)0x4ffb5700, 1, 0},    // emi      --> tcm
    {(unsigned int *)0xD1211000, (unsigned int *)0x562FC000, 0, 1},    // audiosys --> emi
    {(unsigned int *)0x562FC000, (unsigned int *)0xD1211000, 1, 0},    // emi      --> audiosys
    {(unsigned int *)0x4ffb5700, (unsigned int *)0xD1211000, 0, 0},    // tcm      --> audiosys
    {(unsigned int *)0xD1211000, (unsigned int *)0x4ffb5700, 0, 0},    // audiosys --> tcm
    {(unsigned int *)0X56290000, (unsigned int *)0X562C0000, 1, 1},    // emi      --> emi
    {(unsigned int *)0x56298000, (unsigned int *)0X562C8000, 1, 1},    // emi      --> emi
    {(unsigned int *)0X562A0000, (unsigned int *)0x562D0000, 1, 1},    // emi      --> emi
    {(unsigned int *)0X562A8000, (unsigned int *)0X562D8000, 1, 1},    // emi      --> emi
    {(unsigned int *)0x562B0000, (unsigned int *)0X562E0000, 1, 1},    // emi      --> emi
    {(unsigned int *)0X562B8000, (unsigned int *)0x562E8000, 1, 1},    // emi      --> emi
};

static int dma_memcmp(unsigned int *src, unsigned int *dst, int length, int opt)
{
    unsigned int i, a, b;
    int ret = 0;
    for (i = 0; i < (length / sizeof(int)) ; i++) {
        a = *(src + i);
        b = *(dst + i);
        if (a != b) {
            ret = DMA_RESULT_ERROR;

            if (opt & 0x1) {
                PRINTF_E("(%p)[%x] != (%p)[%x]\n", (src + i), a, (dst + i), b);
                continue;
            }
            break;
        }
    }

    return ret;
}

static int test_case_dma(int input, int len)
{
    int ret = DMA_RESULT_DONE;
    int i = 0;

    const dma_dvt_entry_t *p_entry = &dma_dvt_array[input];
    unsigned int *src = p_entry->src;
    unsigned int *dst = p_entry->dst;
    unsigned int from_dram = p_entry->from_dram;
    unsigned int to_dram = p_entry->to_dram;
    unsigned int dma_ch = 0;
    PRINTF_E("[DMA TEST]channel %d, src 0x%08X --> dst 0x%08X\n", dma_ch, (unsigned int)src, (unsigned int)dst);

    for (i = 0; i < (len / sizeof(int)); i++) {
        *(src + i) = (unsigned int)(src + i + input);
    }

    if (from_dram) {
        hal_cache_region_operation(HAL_CACHE_TYPE_DATA, HAL_CACHE_OPERATION_FLUSH, (void *)src, len);
    }

    memset(dst, 0, len);

    if (to_dram) {
        hal_cache_region_operation(HAL_CACHE_TYPE_DATA, HAL_CACHE_OPERATION_FLUSH, (void *)dst, len);
    }

    unsigned int start_time;
    unsigned int end_time;
    start_time = read_ccount();
    ret = scp_dma_transaction((unsigned int)dst, (unsigned int)src, len,
                              (MP3_DMA_ID + dma_ch), NO_RESERVED);

    end_time = read_ccount();
    PRINTF_D("[DMA]Profile:\t%d,\t%d,\t%d,\t%d\n", start_time, end_time,
             end_time - start_time, len);

    if (to_dram) {
        hal_cache_region_operation(HAL_CACHE_TYPE_DATA, HAL_CACHE_OPERATION_FLUSH_INVALIDATE, (void *)dst, len);
    }

    if (ret != DMA_RESULT_DONE) {
        PRINTF_E("DMA test fail %d. ret = %d\n", input, ret);
        return -1;
    }

    if (dma_memcmp(src, dst, len, 0x0)) {
        PRINTF_E("DMA compare fail %d. ret = %d\n", input, ret);
        return -1;
    }

    return 0;
}

void task_loop_of_dma(void *pvParameters)
{
    int i;
    int ret = 0;
    int id = (int)pvParameters;
    const dma_dvt_entry_t *p_entry = &dma_dvt_array[DMA_EMI2EMI_0 + id];
    unsigned int *src = p_entry->src;
    unsigned int *dst = p_entry->dst;
    int len = 0x3000;
    int pattern = 0xAAAAAAAA;

    for(i = 0; i < 5; i++)
    {
        pattern = (i % 2) ? 0xAAAAAAAA : 0x55555555;
        memset((void*)src, pattern, len);
        hal_cache_region_operation(HAL_CACHE_TYPE_DATA, HAL_CACHE_OPERATION_FLUSH, (void *)src, len);

        ret = scp_dma_transaction((unsigned int)dst, (unsigned int)src, len,
                                  id, NO_RESERVED);

        hal_cache_region_operation(HAL_CACHE_TYPE_DATA, HAL_CACHE_OPERATION_INVALIDATE, (void *)dst, len);

        if (ret != DMA_RESULT_DONE) {
            PRINTF_D("[DMA_TEST]thread(%d) loop(%d) transfer error %d\n", id, i, ret);
            continue;
        }

        if (dma_memcmp(src, dst, len, 0x0)) {
            PRINTF_D("[DMA_TEST]thread(%d) loop(%d) compare error\n", id, i);
            continue;
        }

        PRINTF_D("[DMA_TEST]thread(%d) loop(%d) done(%d)\n", id, i, ret);
    }

    vTaskDelete(NULL);
}

int test_case_multi_dma(void)
{
    xTaskCreate(task_loop_of_dma, "TEST_DMA_0", 256, (void *)0, 1, NULL);
    xTaskCreate(task_loop_of_dma, "TEST_DMA_1", 256, (void *)1, 1, NULL);
    xTaskCreate(task_loop_of_dma, "TEST_DMA_2", 256, (void *)2, 1, NULL);
    xTaskCreate(task_loop_of_dma, "TEST_DMA_3", 256, (void *)3, 1, NULL);
    xTaskCreate(task_loop_of_dma, "TEST_DMA_4", 256, (void *)4, 1, NULL);
    xTaskCreate(task_loop_of_dma, "TEST_DMA_5", 256, (void *)5, 1, NULL);

    return 0;
}

void testsuite_dma_dispatcher(uint32_t id)
{
    PRINTF_D("testsuite_dma_dispatcher id(%d)\n", id);
    int ret = 0;
    if (id == 2 || id == 3 || id == 4 || id == 5)
        PRINTF_D("this case(%d) need audiosys clk on to access audio sram and audio_intbus_clk with 137MHz\n", id);

    switch (id) { // start from 64 + id
        case 0:
            ret = test_case_dma(DMA_TCM2EMI, 0x2000);// tcm      --> emi
            break;
        case 1:
            ret = test_case_dma(DMA_EMI2TCM, 0x2000);// emi      --> tcm
            break;
        case 2:
            ret = test_case_dma(DMA_AUD2EMI, 0x2000);// audiosys --> emi
            break;
        case 3:
            ret = test_case_dma(DMA_EMI2AUD, 0x2000);// emi      --> audiosys
            break;
        case 4:
            ret = test_case_dma(DMA_TCM2AUD, 0x2000);// tcm      --> audiosys
            break;
        case 5:
            ret = test_case_dma(DMA_AUD2TCM, 0x2000);// audiosys --> tcm
            break;
        case 6:
            ret = test_case_dma(DMA_EMI2EMI_0, 0x2000);// emi      --> emi
            break;
        case 7:
            ret = test_case_dma(DMA_EMI2EMI_0, 0x1fff);// 1-byte alignment
            break;
        case 8:
            ret = test_case_multi_dma();// multi thread - 6 (emi -> emi)
            break;
        default:
            PRINTF_D("testsuite_dma_dispatcher no this testcase id(%d)\n", id);
    }
}

