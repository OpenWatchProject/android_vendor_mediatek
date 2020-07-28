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

#include <main.h>
#include <FreeRTOS.h>
#include <task.h>
#include <driver_api.h>
#include <platform_mtk.h>
#include <cache_internal.h>
#include <mt_reg_base.h>
#include <driver_api.h>

#define UNUSED(x)       (void)(x)
#define SCP_SLP_PROT_EN (P_CACHE_SLP_PROT_EN | D_CACHE_SLP_PROT_EN)

static void __hal_cache_invalidate_one_cache_line(uint32_t cache_type, uint32_t address)
{
    /* Invalidate CACHE line by address */
    CACHE_SEL(cache_type)->CACHE_OP = (address & CACHE_OP_TADDR_MASK);
    CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_INVALIDATE_ONE_LINE_BY_ADDRESS << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);
}

static void __hal_dcache_drain(void)
{
    volatile int drain_read;
    /* Invalidate CACHE line by address */
    __hal_cache_invalidate_one_cache_line(CACHE_DCACHE, L1C_DRAM_ADDR);
    drain_read = *((int *)L1C_DRAM_ADDR);
    UNUSED(drain_read);
    __DSB();
}

/*TODO porting code, it should be removed*/
uint32_t save_and_set_interrupt_mask()
{
    taskENTER_CRITICAL();
    return 0;
}
void restore_interrupt_mask(uint32_t irq_flag)
{
    taskEXIT_CRITICAL();
}
/*end*/

hal_cache_status_t hal_cache_init(uint32_t cache_type)
{
    hal_cache_region_t region;

    /* If cache is enable , flush and invalidate all cache lines */
    if (CACHE_SEL(cache_type)->CACHE_CON & CACHE_CON_MCEN_MASK) {
        CACHE_SEL(cache_type)->CACHE_OP &= ~CACHE_OP_OP_MASK;
        CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_FLUSH_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);
        CACHE_SEL(cache_type)->CACHE_OP &= ~CACHE_OP_OP_MASK;
        CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_INVALIDATE_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);
    }

    /* Set CACHE related registers to default value*/
    CACHE_SEL(cache_type)->CACHE_CON = 0;
    CACHE_SEL(cache_type)->CACHE_REGION_EN = 0;

    /* Set CACHE region registers to default value and update the global variable */
    for (region = HAL_CACHE_REGION_0; region < HAL_CACHE_REGION_MAX; region ++) {
        /* Set CACHE related registers to default value */
        CACHE_SEL(cache_type)->CACHE_ENTRY_N[region] = 0;
        CACHE_SEL(cache_type)->CACHE_END_ENTRY_N[region] = 0;
    }
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_deinit(uint32_t cache_type)
{
    hal_cache_region_t region;
    /* flush and invalidate all cache lines */
    CACHE_SEL(cache_type)->CACHE_OP &= ~CACHE_OP_OP_MASK;
    CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_FLUSH_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);
    CACHE_SEL(cache_type)->CACHE_OP &= ~CACHE_OP_OP_MASK;
    CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_INVALIDATE_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);
    /* Set CACHE related registers to default value */
    CACHE_SEL(cache_type)->CACHE_CON = 0;
    CACHE_SEL(cache_type)->CACHE_REGION_EN = 0;
    /* Update the global variable*/
    g_cache_con.w = 0;
    g_cache_region_en = 0;
    /* Set CACHE region registers to default value and update the global variable */
    for (region = HAL_CACHE_REGION_0; region < HAL_CACHE_REGION_MAX; region ++) {
        /* Set CACHE related registers to default value */
        CACHE_SEL(cache_type)->CACHE_ENTRY_N[region] = 0;
        CACHE_SEL(cache_type)->CACHE_END_ENTRY_N[region] = 0;
        /* Update the global variable */
        g_cache_entry[region].cache_entry_n.w = 0;
        g_cache_entry[region].cache_end_entry_n.w = 0;
    }

    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_enable(uint32_t cache_type)
{
    /* Enable CACHE */
    CACHE_SEL(cache_type)->CACHE_CON |= CACHE_CON_MCEN_MASK | CACHE_CON_CNTEN0_MASK | CACHE_CON_CNTEN1_MASK;

    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_disable(uint32_t cache_type)
{
    /* If CACHE is enabled, flush and invalidate all cache lines */
    if (CACHE_SEL(cache_type)->CACHE_CON & CACHE_CON_MCEN_MASK) {
        CACHE_SEL(cache_type)->CACHE_OP &= ~CACHE_OP_OP_MASK;
        CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_FLUSH_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);
        CACHE_SEL(cache_type)->CACHE_OP &= ~CACHE_OP_OP_MASK;
        CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_INVALIDATE_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);
    }
    /* Disable CACHE */
    CACHE_SEL(cache_type)->CACHE_CON &= ~CACHE_CON_MCEN_MASK;
    /* Update the global variable */
    g_cache_con.w = CACHE_SEL(cache_type)->CACHE_CON;
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_region_enable(uint32_t cache_type, hal_cache_region_t region)
{
    /* Region is invalid */
    if (region >= HAL_CACHE_REGION_MAX) {
        return HAL_CACHE_STATUS_ERROR_REGION;
    }

    /* The region should be configured before region is enabled */
    if (CACHE_SEL(cache_type)->CACHE_ENTRY_N[region] & CACHE_ENTRY_N_C_MASK) {
        CACHE_SEL(cache_type)->CACHE_REGION_EN |= (1 << region);
    } else {
        return HAL_CACHE_STATUS_ERROR;
    }

    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_region_disable(uint32_t cache_type, hal_cache_region_t region)
{
    /* Region is invalid */
    if (region >= HAL_CACHE_REGION_MAX) {
        return HAL_CACHE_STATUS_ERROR_REGION;
    }
    /* Disable the corresponding region */
    CACHE_SEL(cache_type)->CACHE_REGION_EN &= ~(1 << region);
    /* Update the global variable */
    g_cache_region_en = CACHE_SEL(cache_type)->CACHE_REGION_EN;
    /* The region setting information is cleard */
    g_cache_entry[region].cache_entry_n.w = 0;
    g_cache_entry[region].cache_end_entry_n.w = 0;
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_set_size(uint32_t cache_type, hal_cache_size_t cache_size)
{
    /* CACHE size is invalid */
    if (cache_size >= HAL_CACHE_SIZE_MAX) {
        return HAL_CACHE_STATUS_ERROR_CACHE_SIZE;
    }
    /* Set CACHE size */
    CACHE_SEL(cache_type)->CACHE_CON &= ~CACHE_CON_CACHESIZE_MASK;
    CACHE_SEL(cache_type)->CACHE_CON |= (cache_size << CACHE_CON_CACHESIZE_OFFSET);

    /* When CACHE size is 0KB, make sure the CACHE is disabled */
    if (cache_size == HAL_CACHE_SIZE_0KB) {
        CACHE_SEL(cache_type)->CACHE_CON = 0;
    }

    PRINTF_E("CACHE_CON, addr:0x%x, %d\n", &CACHE_SEL(cache_type)->CACHE_CON, __LINE__);
    PRINTF_E("CACHE_CON, value:0x%x, %d\n", CACHE_SEL(cache_type)->CACHE_CON, __LINE__);

    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_region_config(uint32_t cache_type, hal_cache_region_t region,
        const hal_cache_region_config_t *region_config)
{
    /* Region is invalid */
    if (region >= HAL_CACHE_REGION_MAX) {
        return HAL_CACHE_STATUS_ERROR_REGION;
    }

    /* Parameter check */
    if (region_config == NULL) {
        return HAL_CACHE_STATUS_INVALID_PARAMETER;
    }

    /* The region address must be 4KB aligned */
    if (region_config->cache_region_address & (MTK_CACHE_REGION_SIZE_UNIT - 1)) {
        //assert(0);
        return HAL_CACHE_STATUS_ERROR_REGION_ADDRESS;
    }

    /* The region size must be 4KB aligned */
    if (region_config->cache_region_size & (MTK_CACHE_REGION_SIZE_UNIT - 1)) {
        //assert(0);
        return HAL_CACHE_STATUS_ERROR_REGION_SIZE;
    }

    /* Write the region setting to corresponding register */
    CACHE_SEL(cache_type)->CACHE_ENTRY_N[region] = region_config->cache_region_address;
    CACHE_SEL(cache_type)->CACHE_END_ENTRY_N[region] = region_config->cache_region_address +
            region_config->cache_region_size;

    /* Set this bit when region is configured, and this bit will be double checked in hal_cache_region_enable() function */
    CACHE_SEL(cache_type)->CACHE_ENTRY_N[region] |= CACHE_ENTRY_N_C_MASK;
    /* Update the global variable */

    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_invalidate_one_cache_line(uint32_t cache_type, uint32_t address)
{
    uint32_t irq_flag;

    if (address < (uint32_t)CFG_L1C_DRAM_ADDR)
        return HAL_CACHE_STATUS_OK;

    /* Make sure address is cache line size aligned */
    if (address & (MTK_CACHE_LINE_SIZE - 1)) {
        //assert(0);
        return HAL_CACHE_STATUS_INVALID_PARAMETER;
    }
    /* In order to prevent race condition, interrupt should be disabled when query and update global variable which indicates the module status */
    irq_flag = save_and_set_interrupt_mask();
    __hal_cache_invalidate_one_cache_line(cache_type, address);
    /* Restore the previous status of interrupt */
    restore_interrupt_mask(irq_flag);
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_invalidate_multiple_cache_lines(uint32_t cache_type, uint32_t address, uint32_t length)
{
    uint32_t irq_flag;
    uint32_t end_address = address + length;

    if (address < (uint32_t)CFG_L1C_DRAM_ADDR)
        return HAL_CACHE_STATUS_OK;

    /* Make sure address and length are both cache line size aligned */
    if ((address & (MTK_CACHE_LINE_SIZE - 1)) || (length & (MTK_CACHE_LINE_SIZE - 1))) {
        //assert(0);
        return HAL_CACHE_STATUS_INVALID_PARAMETER;
    }
    /* In order to prevent race condition, interrupt should be disabled when query and update global variable which indicates the module status */
    irq_flag = save_and_set_interrupt_mask();
    /* Invalidate CACHE lines by address and length */
    while (address < end_address) {
        hal_cache_invalidate_one_cache_line(cache_type, address);
        address += MTK_CACHE_LINE_SIZE;
    }
    /* Restore the previous status of interrupt */
    restore_interrupt_mask(irq_flag);
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_invalidate_all_cache_lines(uint32_t cache_type)
{
    uint32_t irq_flag;
    /* In order to prevent race condition, interrupt should be disabled when query and update global variable which indicates the module status */
    irq_flag = save_and_set_interrupt_mask();
    /* Invalidate all CACHE lines */
    CACHE_SEL(cache_type)->CACHE_OP &= ~CACHE_OP_OP_MASK;
    CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_INVALIDATE_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);
    /* Restore the previous status of interrupt */
    restore_interrupt_mask(irq_flag);
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_invalidate_all_cache_lines_from_isr(uint32_t cache_type)
{
    /* Invalidate all CACHE lines */
    CACHE_SEL(cache_type)->CACHE_OP &= ~CACHE_OP_OP_MASK;
    CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_INVALIDATE_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);
    /* Restore the previous status of interrupt */
    return HAL_CACHE_STATUS_OK;
}


hal_cache_status_t hal_cache_flush_one_cache_line(uint32_t cache_type, uint32_t address)
{
    uint32_t irq_flag;

    if (address < (uint32_t)CFG_L1C_DRAM_ADDR)
        return HAL_CACHE_STATUS_OK;

    /* Make sure address is cache line size aligned */
    if (address & (MTK_CACHE_LINE_SIZE - 1)) {
        //assert(0);
        return HAL_CACHE_STATUS_INVALID_PARAMETER;
    }
    /* Interrupt is masked to make sure flush or invalidate operation can not be interrupted */
    irq_flag = save_and_set_interrupt_mask();
    /* Flush CACHE line by address */
    CACHE_SEL(cache_type)->CACHE_OP = (address & CACHE_OP_TADDR_MASK);
    CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_FLUSH_ONE_LINE_BY_ADDRESS << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);

    /* drain write buffer to dram */
    if(cache_type == CACHE_DCACHE)
        __hal_dcache_drain();

    /* Restore the previous status of interrupt */
    restore_interrupt_mask(irq_flag);
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_flush_multiple_cache_lines(uint32_t cache_type, uint32_t address, uint32_t length)
{
    uint32_t irq_flag;
    uint32_t end_address = address + length;

    if (address < (uint32_t)CFG_L1C_DRAM_ADDR)
        return HAL_CACHE_STATUS_OK;

    /* Make sure address and length are both cache line size aligned */
    if ((address & (MTK_CACHE_LINE_SIZE - 1)) || (length & (MTK_CACHE_LINE_SIZE - 1))) {
        //assert(0);
        return HAL_CACHE_STATUS_INVALID_PARAMETER;
    }
    /* Interrupt is masked to make sure flush or invalidate operation can not be interrupted */
    irq_flag = save_and_set_interrupt_mask();
    /* Flush CACHE lines by address and length */
    while (address < end_address) {
        hal_cache_flush_one_cache_line(cache_type, address);
        address += MTK_CACHE_LINE_SIZE;
    }

    /* drain write buffer to dram */
    if(cache_type == CACHE_DCACHE)
        __hal_dcache_drain();

    /* Restore the previous status of interrupt */
    restore_interrupt_mask(irq_flag);
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_flush_all_cache_lines(uint32_t cache_type)
{
    uint32_t irq_flag;
    /* Interrupt is masked to make sure flush or invalidate operation can not be interrupted */
    irq_flag = save_and_set_interrupt_mask();
    /* Flush all CACHE lines */
    CACHE_SEL(cache_type)->CACHE_OP &= ~CACHE_OP_OP_MASK;
    CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_FLUSH_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);

    /* drain write buffer to dram */
    if(cache_type == CACHE_DCACHE)
        __hal_dcache_drain();

    /* Restore the previous status of interrupt */
    restore_interrupt_mask(irq_flag);
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_flush_all_cache_lines_from_isr(uint32_t cache_type)
{
    /* Interrupt is masked to make sure flush or invalidate operation can not be interrupted */
    /* Flush all CACHE lines */
    CACHE_SEL(cache_type)->CACHE_OP &= ~CACHE_OP_OP_MASK;
    CACHE_SEL(cache_type)->CACHE_OP |= ((CACHE_FLUSH_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK);

    /* drain write buffer to dram */
    if(cache_type == CACHE_DCACHE)
        __hal_dcache_drain();

    return HAL_CACHE_STATUS_OK;
}

void hal_cache_scp_sleep_protect_on()
{
    DRV_WriteReg32(SCP_SLP_PROTECT_CFG,
                DRV_Reg32(SCP_SLP_PROTECT_CFG) | SCP_SLP_PROT_EN);
}

void hal_cache_scp_sleep_protect_off()
{
    DRV_WriteReg32(SCP_SLP_PROTECT_CFG,
                DRV_Reg32(SCP_SLP_PROTECT_CFG) & ~SCP_SLP_PROT_EN);
}

uint32_t hal_cache_access_countl(uint32_t cache_type)
{
    return  CACHE_SEL(cache_type)->CACHE_CCNT0L;
}
uint32_t hal_cache_hit_countl(uint32_t cache_type)
{
    return  CACHE_SEL(cache_type)->CACHE_HCNT0L;
}
uint32_t hal_cache_access_countu(uint32_t cache_type)
{
    return  CACHE_SEL(cache_type)->CACHE_CCNT0U;
}
uint32_t hal_cache_hit_countu(uint32_t cache_type)
{
    return  CACHE_SEL(cache_type)->CACHE_HCNT0U;
}

uint64_t cache_hit_count(uint32_t cache_type)
{
    return (((uint64_t)hal_cache_hit_countu(cache_type) << 32) + (uint64_t)hal_cache_hit_countl(cache_type));
}

uint64_t cache_access_count(uint32_t cache_type)
{
    return (((uint64_t)hal_cache_access_countu(cache_type) << 32) + (uint64_t)hal_cache_access_countl(cache_type));
}

void bus_qos(uint32_t set)
{
    if(set)
        DRV_WriteReg32(BUS_QOS, DRV_Reg32(BUS_QOS) | ((2 << AXI_ARULTRA) | (2 << AXI_AWULTRA)));
    else
        DRV_WriteReg32(BUS_QOS, DRV_Reg32(BUS_QOS) & ~((2 << AXI_ARULTRA) | (2 << AXI_AWULTRA)));
}

