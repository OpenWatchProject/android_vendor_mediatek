/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

#include <stdio.h>
#include <mt_reg_base.h>
#include <platform_mtk.h>
#include <main.h>
#include "FreeRTOS.h" /*log */
#include "task.h"
#include <mpu_mtk.h>

/* example:
 * struct xthal_MPU_entry mpumap[] = {
 *   XTHAL_MPU_ENTRY ( 0x00000000, MPU_ENTRY_ENABLE, XTHAL_AR_RWrw, XTHAL_MEM_DEVICE),
 *   XTHAL_MPU_ENTRY ( 0x20000000, MPU_ENTRY_ENABLE, XTHAL_AR_RWrw,   XTHAL_MEM_NON_CACHEABLE),
 *   XTHAL_MPU_ENTRY ( 0x40000000, MPU_ENTRY_ENABLE, XTHAL_AR_RWXrwx,   XTHAL_MEM_NON_CACHEABLE),
 * };
 * xthal_write_map(mpumap, sizeof(mpumap) / sizeof(struct xthal_MPU_entry));
 *
 */
mpu_region_t adsp_mpu_region[] INTERNAL_INITIAL_DATA
        = {  /* size = 140B */
    {   /* MPU_ID_COMFREQ, with SYSREG (0x0000_0000-0x2FFF_FFFF) */
        .vaddr          = 0x10000000,
        .size           = 0x20006000,
        .access_right   = XTHAL_AR_RWrw,
        .memory_type    = XTHAL_MEM_DEVICE,
        .enable         = MPU_ENTRY_ENABLE,
    },
    {   /* MPU_ID_DTCM */
        .vaddr          = CFG_HIFI3_DRAM_ADDRESS,
        .size           = CFG_HIFI3_DRAM_SIZE,
        .access_right   = XTHAL_AR_RWrw,
        .memory_type    = XTHAL_MEM_NON_CACHEABLE,
        .enable         = MPU_ENTRY_ENABLE,
    },
    {   /* MPU_ID_ITCM */
        .vaddr          = CFG_HIFI3_IRAM_ADDRESS,
        .size           = CFG_HIFI3_IRAM_SIZE,
        .access_right   = XTHAL_AR_RXrx,
        .memory_type    = XTHAL_MEM_NON_CACHEABLE,
        .enable         = MPU_ENTRY_ENABLE,
    },
    {   /* MPU_ID_ADSP_PROGRAM_RO */
        .vaddr          = INVALID_BASE_ADDR,    /* runtime get from AP */
        .size           = 0x0,   /*TBD*/
        .access_right   = XTHAL_AR_RXrx,
        .memory_type    = XTHAL_MEM_WRITEBACK,
        .enable         = MPU_ENTRY_DISABLE,
    },

    {   /* MPU_ID_ADSP_SHARED_CACHEABLE */
        .vaddr          = INVALID_BASE_ADDR,    /* runtime get from AP */
        .size           = 0x0,   /*TBD*/
        .access_right   = XTHAL_AR_RWrw,
        .memory_type    = XTHAL_MEM_WRITEBACK,
        .enable         = MPU_ENTRY_DISABLE,
    },
    {   /* MPU_ID_ADSP_SHARED_NONCACHEABLE */
        .vaddr          = INVALID_BASE_ADDR,    /* runtime get from AP */
        .size           = 0x0,   /*TBD*/
        .access_right   = XTHAL_AR_RWrw,
        .memory_type    = XTHAL_MEM_NON_CACHEABLE,
        .enable         = MPU_ENTRY_DISABLE,
    },
    {   /* MPU_ID_MD1_CCB */
        .vaddr          = INVALID_BASE_ADDR,    /* updated by phone call */
        .size           = 0x0,    /*TBD*/
        .access_right   = XTHAL_AR_RWrw,
        .memory_type    = XTHAL_MEM_WRITEBACK,
        .enable         = MPU_ENTRY_DISABLE,
    },
    {   /* MPU_ID_AUDIOSYS */
        .vaddr          = CFG_AUDIO_BASE_ADDRESS,
        .size           = (CFG_AUDIO_CONFIG_SIZE + CFG_AUDIO_DATA_SIZE),
        .access_right   = XTHAL_AR_RWrw,
        .memory_type    = XTHAL_MEM_DEVICE,
        .enable         = MPU_ENTRY_ENABLE,
    },
};

struct xthal_MPU_entry fg_entry[XCHAL_MPU_ENTRIES]
        INTERNAL_NON_INITIAL_DATA;   /*size = 128Byte*/
static mpu_info_t *adsp_mpu_obj;

/* declare to seperate RO and RW region */
static char mpu_ro_reserved_4k[4*1024] __attribute__ ((section(".mpu_ro_separator")));
static uint32_t mpu_get_prog_rodata_end_address(void);
static void mpu_create_map(xthal_MPU_entry *new_entry);
static int mpu_check_and_write_map(xthal_MPU_entry *entries, unsigned num_entries);

/* Depending on the MPU's state, this function requires from zero to three unused MPU entries.
 * @return  XTHAL_OUT_OF_ENTRIES, if free entries are less than 3.
 */
int32_t request_mpu_region(void)
{
    int32_t i = 0;

    xthal_read_map(fg_entry);
    for (i = 0; i < 3; i ++) {
        if (XTHAL_MPU_ENTRY_GET_VSTARTADDR(fg_entry[i]) != 0x0) {
            return XTHAL_OUT_OF_ENTRIES;
        }
    }
    return XTHAL_SUCCESS;
}

void dump_mpu_status(void)
{
#if MPU_DEBUG_SUPPORT
    int32_t i = 0;

    xthal_read_map(fg_entry);
    PRINTF_D("REGION\tVADDR\t\tACCESS RIGHT\tMEMORY TYPE\tVALID\n");
    PRINTF_D("============================================================\n");
    for (i = 0; i <= MAX_NUM_REGIONS - 1; i ++) {
        PRINTF_D("%8d\t0x%08x\t\t0x%0x\t0x%02x\t\t%d\n", i,
                 XTHAL_MPU_ENTRY_GET_VSTARTADDR(fg_entry[i]),
                 XTHAL_MPU_ENTRY_GET_ACCESS(fg_entry[i]),
                 XTHAL_MPU_ENTRY_GET_MEMORY_TYPE(fg_entry[i]),
                 XTHAL_MPU_ENTRY_GET_VALID(fg_entry[i]));
    }
#endif
}

/*for optimize: use in-line to write the enable for the MPU entries: wsr.mpuenb ... */
int32_t enable_mpu_region(uint32_t region_num)
{
    int32_t i;

    if (region_num < 0 || region_num >= MAX_NUM_REGIONS) {
        PRINTF_E("[mpu]region_num:%d is wrong\n", region_num);
        return XTHAL_OUT_OF_ENTRIES;
    }
    if (adsp_mpu_region[region_num].enable != MPU_ENTRY_DISABLE
        || (adsp_mpu_region[region_num].vaddr & (~0x1fUL)) == INVALID_BASE_ADDR)
        return XTHAL_INVALID_ADDRESS;

    xthal_read_map(fg_entry);
    for (i = MAX_NUM_REGIONS - 1; i >= 0; i--) {
        if (XTHAL_MPU_ENTRY_GET_VSTARTADDR(fg_entry[i]) ==
            adsp_mpu_region[region_num].vaddr) {
//            XTHAL_MPU_ENTRY_SET_VALID(fg_entry[i], 1);
            break;
        }
    }

    adsp_mpu_region[region_num].enable = MPU_ENTRY_ENABLE;
    if (i < 0)
        mpu_create_map(fg_entry);
    return mpu_check_and_write_map(fg_entry, MAX_NUM_REGIONS);

}

/* for optimize: use in-line to write the enable for the MPU entries: wsr.mpuenb ... */
int32_t disable_mpu_region(uint32_t region_num)
{
    int32_t i;

    if (region_num < 0 || region_num >= MAX_NUM_REGIONS) {
        PRINTF_E("[mpu]region_num:%d is wrong\n", region_num);
        return XTHAL_OUT_OF_ENTRIES;
    }
    if (adsp_mpu_region[region_num].enable != MPU_ENTRY_ENABLE)
        return XTHAL_INVALID_ADDRESS;

    xthal_read_map(fg_entry);
    for (i = MAX_NUM_REGIONS - 1; i >= 0; i--) {
        if (XTHAL_MPU_ENTRY_GET_VSTARTADDR(fg_entry[i]) ==
            adsp_mpu_region[region_num].vaddr) {
//            XTHAL_MPU_ENTRY_SET_VALID(fg_entry[i], 0);
            break;
        }
    }
    adsp_mpu_region[region_num].enable = MPU_ENTRY_DISABLE;
    return mpu_check_and_write_map(fg_entry, MAX_NUM_REGIONS);
}

/* disable region before calling this function.*/
void mpu_setup_region_address(uint32_t region_num, uint32_t vaddr,
                              uint32_t size)
{
    vaddr = ALIGN_MASK(vaddr, MPU_ALIGN_MASK);

    if (vaddr == NULL)
        return;

    if (adsp_mpu_region[region_num].enable != MPU_ENTRY_ENABLE) {
        adsp_mpu_region[region_num].vaddr = vaddr;
        adsp_mpu_region[region_num].size = ALIGN_MASK(size, MPU_ALIGN_MASK);
    }
}

static void mpu_create_map(xthal_MPU_entry *new_entry)
{

    int32_t idx = MAX_NUM_REGIONS - 1, id;
    uint32_t entry_end;
    uint32_t num_entry = (sizeof(adsp_mpu_region) / sizeof(mpu_region_t));

    for (id = num_entry - 1 ; id >= 0; id--) {
        if (adsp_mpu_region[id].enable == MPU_ENTRY_ENABLE) {
            entry_end = adsp_mpu_region[id].vaddr + adsp_mpu_region[id].size;
            if (id == num_entry - 1) {
                MPU_ENTRY_SET_AS(new_entry[idx], entry_end, MPU_ENTRY_ENABLE);
                MPU_ENTRY_SET_AT(new_entry[idx], XTHAL_AR_NONE, XTHAL_MEM_DEVICE);
                idx--;
            }
            else if (entry_end < adsp_mpu_region[id + 1].vaddr) {
                MPU_ENTRY_SET_AS(new_entry[idx], entry_end, MPU_ENTRY_ENABLE);
                MPU_ENTRY_SET_AT(new_entry[idx], XTHAL_AR_NONE, XTHAL_MEM_DEVICE);
                idx--;
            }
            MPU_ENTRY_SET_AS(new_entry[idx], adsp_mpu_region[id].vaddr,
                             adsp_mpu_region[id].enable);
            MPU_ENTRY_SET_AT(new_entry[idx], adsp_mpu_region[id].access_right,
                             adsp_mpu_region[id].memory_type);
            idx--;
        }
    }
    if (adsp_mpu_region[0].vaddr != 0x0) {
        MPU_ENTRY_SET_AS(new_entry[idx], 0x0, MPU_ENTRY_ENABLE);
        MPU_ENTRY_SET_AT(new_entry[idx], XTHAL_AR_NONE, XTHAL_MEM_DEVICE);
    }
}

/* Note: Before calling this funciton, to avoid asynchronous use of the MPU during the call,
 * 1. All interrupts must be disabled.
 * 2. The integrated DMA engine(iDMA), if configured, must be idle. //XCHAL_HAVE_IDMA is not configured at core-isa.h
 * 3. All previous block prefetch requests must be complete.
 */
static int mpu_check_and_write_map(xthal_MPU_entry *entries, unsigned num_entries)
{
    int32_t rv = XTHAL_SUCCESS;
    uint32_t mask;

    rv = xthal_check_map(entries, num_entries);
    if (rv != XTHAL_SUCCESS) {
        PRINTF_E("[mpu]%s is invalid (%d)\n", __FUNCTION__, rv);
        configASSERT(0);
        return rv;
    }

    mask = portSET_INTERRUPT_MASK_FROM_ISR();
    xthal_write_map(entries, num_entries);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(mask);
    dump_mpu_status();

    return XTHAL_SUCCESS;
}

/*
 * interrupt must be disabled during this function.
 */
void mpu_init(void)
{
    int check_map = XTHAL_SUCCESS;
    int i = 0;
    uint32_t adsp_base_addr;

    adsp_mpu_obj = (mpu_info_t *)ADSP_MPU_INFO_BASE;
    adsp_base_addr = adsp_mpu_obj->prog_addr;

#if MPU_DEBUG_SUPPORT
    /* log out pre-load address */
    PRINTF_D("[mpu] adsp_mpu_obj = %x \n\r", ADSP_MPU_INFO_BASE);
#endif
    if (adsp_mpu_obj->prog_addr != CFG_HIFI3_SRAM_ADDRESS) {
        PRINTF_E("[MPU]Inconsistent adsp address with PREDEFINED_ADSP_VADDR.\n");
        configASSERT(0);
        return;
    } else {
        adsp_mpu_obj->prog_size = mpu_get_prog_rodata_end_address() - adsp_mpu_obj->prog_addr;
        adsp_mpu_obj->data_addr = adsp_mpu_obj->prog_addr +
                                 adsp_mpu_obj->prog_size;
        adsp_mpu_obj->data_size = adsp_mpu_obj->data_non_cache_addr -
                                 adsp_mpu_obj->data_addr;
#if MPU_DEBUG_SUPPORT
        PRINTF_D("[UPDATED]prog_addr=%x, prog_size=%x, ro_end_addr = %x\n", adsp_mpu_obj->prog_addr, adsp_mpu_obj->prog_size,  mpu_get_prog_rodata_end_address());
        PRINTF_D("[UPDATED]data_addr=%x, data_size=%x\n", adsp_mpu_obj->data_addr, adsp_mpu_obj->data_size);
        PRINTF_D("[UPDATED]nc_data_addr=%x, nc_data_size=%x\n", adsp_mpu_obj->data_non_cache_addr, adsp_mpu_obj->data_non_cache_size);
#endif
        mpu_setup_region_address(MPU_ID_ADSP_PROGRAM_RO,
                                 adsp_mpu_obj->prog_addr, adsp_mpu_obj->prog_size);
        mpu_setup_region_address(MPU_ID_ADSP_RW, adsp_mpu_obj->data_addr,
                                 adsp_mpu_obj->data_size);
        mpu_setup_region_address(MPU_ID_ADSP_RW_NC,
                                 adsp_mpu_obj->data_non_cache_addr, adsp_mpu_obj->data_non_cache_size);
    }

    for (i = 0; i < sizeof(adsp_mpu_region) / sizeof(mpu_region_t); i++) {
        adsp_mpu_region[i].enable = MPU_ENTRY_ENABLE;
        if (ALIGN_MASK(adsp_mpu_region[i].vaddr, MPU_ALIGN_MASK) == INVALID_BASE_ADDR) {
            PRINTF_W("[mpu]unable to get region address(%d:0x%x)\n", i, adsp_mpu_region[i].vaddr);
            adsp_mpu_region[i].enable = MPU_ENTRY_DISABLE;
        }
    }

    mpu_create_map(fg_entry);
    check_map = mpu_check_and_write_map(fg_entry,
                            sizeof(fg_entry) / sizeof(struct xthal_MPU_entry));
    if (check_map != XTHAL_SUCCESS) {
        PRINTF_E("[mpu] mpu init failed\n");
        configASSERT(0);
#if MPU_DEBUG_SUPPORT
    } else {
        PRINTF_D("[mpu] MPU setting... Done!\n");
#endif
    }
}

uint32_t mpu_get_adsp_sram_prot_size(void)
{
    return (adsp_mpu_obj->prog_size + adsp_mpu_obj->data_size
            + adsp_mpu_obj->data_non_cache_size);
}

static uint32_t mpu_get_prog_rodata_end_address(void)
{
    return ALIGN_MASK((uint32_t)&mpu_ro_reserved_4k, MPU_ALIGN_MASK);
}
