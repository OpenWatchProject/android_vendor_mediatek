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
#include "FreeRTOS.h"
#include "driver_api.h"
#include "mtk_overlay_init.h"
#ifdef CFG_DMA_SUPPORT
#include "dma.h"
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
#include "dvfs.h"
#endif

extern struct overlay_init_s __overlay_struct_start[];
extern struct overlay_init_s __overlay_struct_end[];
extern char __overlay_ws_01_start;  /* overlay section 01 start */
extern char __overlay_ws_01_end;    /* overlay section 02 start */
extern char __overlay_ws_02_end;    /* overlay section 03 start */
extern char __overlay_ws_03_end;    /* overlay section 04 start */
extern char __overlay_ws_04_end;    /* overlay section 05 start */
extern char __overlay_ws_05_end;    /* overlay section 06 start */
extern char __overlay_ws_06_end;    /* overlay section 07 start */
extern char __overlay_ws_07_end;    /* overlay section 08 start */
extern char __overlay_ws_08_end;    /* overlay section 09 start */
extern char __overlay_ws_09_end;    /* overlay section 10 start */
/*overlay variable*/
volatile unsigned int ovl_image_start_addr;
volatile unsigned int ovl_image_size_addr;

/*overlay struct*/
struct overlay_record_s {
    char* name;
    char* overlay_load_start;
    char* overlay_load_end;
    void* data;
} scp_overlay_record[NUM_OVERLAY_WORK];


#ifdef CFG_OVERLAY_DEBUG_SUPPORT
/******************************************************************************
* _ovly_table[] and _novlys are used to make overlay remapping automatic in gdb.
******************************************************************************/
struct std_overlay_table {
    unsigned long vma;     /* The overlay's mapped address. */
    unsigned long size;    /* The size of the overlay, in bytes. */
    unsigned long lma;     /* The overlay's load address. */
    unsigned long mapped;  /* Non-zero if the overlay is currently mapped; zero otherwise. */
} _ovly_table[NUM_OVERLAY_WORK];

/* 4 bytes integer holding the total number of elements in _ovly_table. */
int _novlys = NUM_OVERLAY_WORK;
#endif  // CFG_OVERLAY_DEBUG_SUPPORT


/******************************************************************************
******************************************************************************/
static void init_std_overlay_data(void)
{
#ifdef CFG_OVERLAY_DEBUG_SUPPORT
    memset(_ovly_table, 0, sizeof(_ovly_table));
    _novlys = NUM_OVERLAY_WORK;
#endif  // CFG_OVERLAY_DEBUG_SUPPORT
}


/******************************************************************************
******************************************************************************/
static void set_std_overlay_table(const struct overlay_init_s *mod)
{
#ifdef CFG_OVERLAY_DEBUG_SUPPORT
    int index = mod->overlay_section;
    unsigned long vma = 0;
    int size = (int)(mod->overlay_load_end - mod->overlay_load_start);

    switch (mod->overlay_section) {
        case OVERLAY_WORK_00:
            vma = (unsigned long)&__overlay_ws_01_start;
            break;
        case OVERLAY_WORK_01:
            vma = (unsigned long)&__overlay_ws_01_end;
            break;
        case OVERLAY_WORK_02:
            vma = (unsigned long)&__overlay_ws_02_end;
            break;
        case OVERLAY_WORK_03:
            vma = (unsigned long)&__overlay_ws_03_end;
            break;
        case OVERLAY_WORK_04:
            vma = (unsigned long)&__overlay_ws_04_end;
            break;
        case OVERLAY_WORK_05:
            vma = (unsigned long)&__overlay_ws_05_end;
            break;
        case OVERLAY_WORK_06:
            vma = (unsigned long)&__overlay_ws_06_end;
            break;
        case OVERLAY_WORK_07:
            vma = (unsigned long)&__overlay_ws_07_end;
            break;
        case OVERLAY_WORK_08:
            vma = (unsigned long)&__overlay_ws_08_end;
            break;
        case OVERLAY_WORK_09:
            vma = (unsigned long)&__overlay_ws_09_end;
            break;
        default:
            PRINTF_E("Err: ovly index\n");
            break;
    }

    _ovly_table[index].vma = vma;
    _ovly_table[index].size = size;
    _ovly_table[index].lma = (unsigned long)mod->overlay_load_start;
    _ovly_table[index].mapped = (size != 0) ? 1 : 0;

    PRINTF_E("_ovly_table[%d].vma = %x\n", index, (unsigned int)_ovly_table[index].vma);
    PRINTF_E("_ovly_table[%d].size = %x\n", index, (unsigned int)_ovly_table[index].size);
    PRINTF_E("_ovly_table[%d].lma = %x\n", index, (unsigned int)_ovly_table[index].lma);
    PRINTF_E("_ovly_table[%d].mapped = %d\n", index, (unsigned int)_ovly_table[index].mapped);
#endif  // CFG_OVERLAY_DEBUG_SUPPORT
}


/*
* scp_copy_overlay : copy overlay section from scp dram image to sram
* return  0: sucess
*        -1: fail
*/
int scp_copy_overlay(const struct overlay_init_s * mod)
{
    /*get scp image dram address*/
    volatile unsigned int image_start_addr;
    volatile unsigned int ap_image_start_addr;
    volatile unsigned int scp_copy_start_addr;
    int length = (int)(mod->overlay_load_end - mod->overlay_load_start);
    char *dest_addr = 0;
    const struct scp_region_info_st *scp_region_info;
    int ret = 0;

    scp_region_info = (struct scp_region_info_st *)SCP_IMAGE_INFO_ADDR;
    image_start_addr = scp_region_info->ap_firmware_start;

    /*record for debug*/
    ovl_image_start_addr = scp_region_info->ap_firmware_start;
    ovl_image_size_addr = scp_region_info->ap_firmware_size;
    ap_image_start_addr = ap_to_scp(image_start_addr);

    /*count sram offset*/
    scp_copy_start_addr =(unsigned int)mod->overlay_load_start + ap_image_start_addr - SCP_IMAGE_FW_OFFSET;

#ifdef CFG_OVERLAY_DEBUG_SUPPORT
    PRINTF_E("scp_region_info:\n");
    PRINTF_E("ap_loader_start=%x\n", scp_region_info->ap_loader_start);
    PRINTF_E("ap_loader_size=%x\n", scp_region_info->ap_loader_size);
    PRINTF_E("ap_firmware_start=%x\n", scp_region_info->ap_firmware_start);
    PRINTF_E("ap_firmware_size=%x\n", scp_region_info->ap_firmware_size);
    PRINTF_E("load from dram addr=%x\n", scp_copy_start_addr);
#endif

#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_enable_DRAM_resource(DMA_MEM_ID);
#endif

    switch (mod->overlay_section) {
            case OVERLAY_WORK_00:
                dest_addr = &__overlay_ws_01_start;
                break;
            case OVERLAY_WORK_01:
                dest_addr = &__overlay_ws_01_end;
                break;
            case OVERLAY_WORK_02:
                dest_addr = &__overlay_ws_02_end;
                break;
            case OVERLAY_WORK_03:
                dest_addr = &__overlay_ws_03_end;
                break;
            case OVERLAY_WORK_04:
                dest_addr = &__overlay_ws_04_end;
                break;
            case OVERLAY_WORK_05:
                dest_addr = &__overlay_ws_05_end;
                break;
            case OVERLAY_WORK_06:
                dest_addr = &__overlay_ws_06_end;
                break;
            case OVERLAY_WORK_07:
                dest_addr = &__overlay_ws_07_end;
                break;
            case OVERLAY_WORK_08:
                dest_addr = &__overlay_ws_08_end;
                break;
            case OVERLAY_WORK_09:
                dest_addr = &__overlay_ws_09_end;
                break;
            default:
                PRINTF_E("ovl cp ERR\n");
                ret = -1;
                break;
    }
    if (ret == 0) {
#ifdef CFG_OVERLAY_DEBUG_SUPPORT
        int dest_section = mod->overlay_section + 1;
        PRINTF_E("load to sec%02d addr=%p\n", dest_section, dest_addr);
#endif
        memcpy(dest_addr, (char *)scp_copy_start_addr, length);
        //ret = scp_dma_transaction_dram((unsigned int)dest_addr, scp_copy_start_addr, length, LOGGER_DMA_ID, NO_RESERVED);
        //    if (ret != DMA_RESULT_DONE) {
        //        PRINTF_E("overlay dma trans fail%u\n", ret);
        //    }
    }

#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_disable_DRAM_resource(DMA_MEM_ID);
#endif

    PRINTF_E("ovl select done\n");
    return ret;
}


/*
* scp_overlay_select : select and move overlay section to scp sram
* return : data pointer
*/
void * tinysys_overlay_select(char* name)
{
    const struct overlay_init_s *mod;
    int found_overlay_section = 0;
    int ret = 0;

    for (mod = __overlay_struct_start; mod < __overlay_struct_end; mod++) {
#ifdef CFG_OVERLAY_DEBUG_SUPPORT
            PRINTF_E("mod=%p\n",mod);
            PRINTF_E("mod->name=%s\n",mod->name);
#endif
        if (!strcmp(mod->name,name)) {
            /*copy overlay code here*/
            PRINTF_E("ovl select\n");
#ifdef CFG_OVERLAY_DEBUG_SUPPORT
            PRINTF_E("&__overlay_struct_start=%p\n",&__overlay_struct_start);
            PRINTF_E("&__overlay_struct_end=%p\n",&__overlay_struct_end);
            PRINTF_E("name =%s\n",mod->name);
            PRINTF_E("start=%p\n",mod->overlay_load_start);
            PRINTF_E("stop =%p\n",mod->overlay_load_end);
            PRINTF_E("size =%u\n",(mod->overlay_load_end - mod->overlay_load_start));
            PRINTF_E("section=%u\n", (mod->overlay_section));
            PRINTF_E("data=%p\n", (mod->data));
#endif
            ret = scp_copy_overlay(mod);
            if (ret == 0) {
                found_overlay_section = 1;
                /*record overlay section*/
                scp_overlay_record[mod->overlay_section].name = mod->name;
                scp_overlay_record[mod->overlay_section].overlay_load_start = mod->overlay_load_start;
                scp_overlay_record[mod->overlay_section].overlay_load_end = mod->overlay_load_end;
                scp_overlay_record[mod->overlay_section].data = mod->data;
                set_std_overlay_table(mod);
            }

            return mod->data;
        }
    }
    if (found_overlay_section == 0){
        PRINTF_E("ERR No ovl found\n");
    }
    /*return pointer*/
    return 0;
}

/*
* tinysys_overlay_init : init overlay struct
*/
void tinysys_overlay_init()
{
    struct overlay_init_s *mod;

    for (mod = __overlay_struct_start; mod < __overlay_struct_end; mod++) {
        /*section and data can not reset because of declare has been setting*/
        mod->overlay_load_start = NULL;
        mod->overlay_load_end = NULL;
        mod->name = "NULL";
    }
    init_std_overlay_data();
}

