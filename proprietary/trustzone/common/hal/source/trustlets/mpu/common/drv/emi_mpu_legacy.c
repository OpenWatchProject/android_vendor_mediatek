/*
 * Copyright (c) 2014 - 2018 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <mt_emi_mpu.h>
#include <emi_mpu_v1.h>
#include "tee_drStd.h"
#include <drv_fwk.h>

unsigned char *emi_mpu_va_buffer;
#define EMI_MPU_VA_BUFFER	emi_mpu_va_buffer

inline unsigned int readl(volatile unsigned int addr)
{
    return *((volatile unsigned int *)addr);
}

inline void writel(unsigned int val, volatile unsigned int addr)
{
    *((volatile unsigned int *)addr) = val;
}

int emi_mpu_set_protection(struct emi_region_info_t *region_info)
{
    int ret = 0;
    unsigned int start;
    unsigned int end;

    if (region_info->region >= EMI_MPU_REGION_NUM) {
        msee_loge("[emi_mpu_set_protection] region %d out of range\n",
            region_info->region);
        return -4;
    }

    start = (unsigned int)(region_info->start >> EMI_MPU_ALIGN_BITS);
    end = (unsigned int)(region_info->end >> EMI_MPU_ALIGN_BITS);

    if ((start >= DRAM_OFFSET) && (end >= start)) {
        start -= DRAM_OFFSET;
        end -= DRAM_OFFSET;
    } else
        return -3;

    //map register set
    if (msee_mmap_region(EMI_MPU_BASE, (void **)&EMI_MPU_VA_BUFFER, SIZE_4KB, MSEE_MAP_HARDWARE)) {
        msee_loge("[emi_mpu_set_protection] map EMI_MPU_BASE failed!\n");
        return -2;
    }

    switch (region_info->region) {
    case 0:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUI);
        writel((start << 16) | end, EMI_MPUA);
        writel(region_info->apc[0], EMI_MPUI);
        break;

    case 1:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUJ);
        writel((start << 16) | end, EMI_MPUB);
        writel(region_info->apc[0], EMI_MPUJ);
        break;

    case 2:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUK);
        writel((start << 16) | end, EMI_MPUC);
        writel(region_info->apc[0], EMI_MPUK);
        break;

    case 3:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUL);
        writel((start << 16) | end, EMI_MPUD);
        writel(region_info->apc[0], EMI_MPUL);
        break;

    case 4:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUI_2ND);
        writel((start << 16) | end, EMI_MPUE);
        writel(region_info->apc[0], EMI_MPUI_2ND);
        break;

    case 5:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUJ_2ND);
        writel((start << 16) | end, EMI_MPUF);
        writel(region_info->apc[0], EMI_MPUJ_2ND);
        break;

    case 6:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUK_2ND);
        writel((start << 16) | end, EMI_MPUG);
        writel(region_info->apc[0], EMI_MPUK_2ND);
        break;

    case 7:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUL_2ND);
        writel((start << 16) | end, EMI_MPUH);
        writel(region_info->apc[0], EMI_MPUL_2ND);
        break;

    case 8:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUI2);
        writel((start << 16) | end, EMI_MPUA2);
        writel(region_info->apc[0], EMI_MPUI2);
        break;

    case 9:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUJ2);
        writel((start << 16) | end, EMI_MPUB2);
        writel(region_info->apc[0], EMI_MPUJ2);
        break;

    case 10:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUK2);
        writel((start << 16) | end, EMI_MPUC2);
        writel(region_info->apc[0], EMI_MPUK2);
        break;

    case 11:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUL2);
        writel((start << 16) | end, EMI_MPUD2);
        writel(region_info->apc[0], EMI_MPUL2);
        break;

    case 12:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUI2_2ND);
        writel((start << 16) | end, EMI_MPUE2);
        writel(region_info->apc[0], EMI_MPUI2_2ND);
        break;

    case 13:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUJ2_2ND);
        writel((start << 16) | end, EMI_MPUF2);
        writel(region_info->apc[0], EMI_MPUJ2_2ND);
        break;

    case 14:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUK2_2ND);
        writel((start << 16) | end, EMI_MPUG2);
        writel(region_info->apc[0], EMI_MPUK2_2ND);
        break;

    case 15:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUL2_2ND);
        writel((start << 16) | end, EMI_MPUH2);
        writel(region_info->apc[0], EMI_MPUL2_2ND);
        break;

    case 16:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUI3);
        writel((start << 16) | end, EMI_MPUA3);
        writel(region_info->apc[0], EMI_MPUI3);
        break;

    case 17:
        /* Clear access right before setting MPU address */
        writel(0, EMI_MPUJ3);
        writel((start << 16) | end, EMI_MPUB3);
        writel(region_info->apc[0], EMI_MPUJ3);
        break;

    default:
        ret = -1;
        break;
    }

    //unmap register set
    if (msee_unmmap_region(EMI_MPU_VA_BUFFER, SIZE_4KB)) {
        msee_loge("[emi_mpu_set_protection] unmap EMI_MPU_BASE failed!\n");
        return -1;
    }

    return ret;
}

