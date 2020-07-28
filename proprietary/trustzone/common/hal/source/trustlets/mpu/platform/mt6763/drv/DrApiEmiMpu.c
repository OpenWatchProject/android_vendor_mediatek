/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "drv_mpu_api.h"
#include "tee_drStd.h"
#include <drv_fwk.h>

/* EMI memory protection align 64K */
#define EMI_MPU_ALIGNMENT   0x10000

#define IOMEM(reg) (reg)

unsigned char *emi_mpu_va_buffer;
#define EMI_MPU_VA_BUFFER       emi_mpu_va_buffer

#define IO_PHYS                 0x10000000
#define EMI_MPU_BASE            (IO_PHYS + 0x00226000)

/* EMI Memory Protect Unit */
#define EMI_MPU_CTRL                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x0))
#define EMI_MPU_DBG                 ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x4))
#define EMI_MPU_SA0                 ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x100))
#define EMI_MPU_EA0                 ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x200))
#define EMI_MPU_SA(region)          ((volatile unsigned int)(EMI_MPU_SA0 + (region*4)))
#define EMI_MPU_EA(region)          ((volatile unsigned int)(EMI_MPU_EA0 + (region*4)))
#define EMI_MPU_APC0                ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x300))
#define EMI_MPU_APC(region,domain)  ((volatile unsigned int)(EMI_MPU_APC0 + (region*4) +((domain/8)*0x100)))
#define EMI_MPU_CTRL_D0             ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x800))
#define EMI_MPU_CTRL_D(domain)      ((volatile unsigned int)(EMI_MPU_CTRL_D0 + (domain*4)))
#define EMI_RG_MASK_D0              ((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x900))
#define EMI_RG_MASK_D(domain)       ((volatile unsigned int)(EMI_RG_MASK_D0 + (domain*4)))

inline unsigned int readl(volatile unsigned int addr)
{
    return *( volatile unsigned int*)addr;
}

inline void writel(unsigned int val, volatile unsigned int addr)
{
    *(volatile unsigned int*)addr = val;
}

/*
 * emi_mpu_set_region_protection: protect a region.
 * @start: start address of the region
 * @end: end address of the region
 * @region: EMI MPU region id
 * @access_permission: EMI MPU access permission
 * Return 0 for success, otherwise negative status code.
 */
int emi_mpu_set_region_protection(
        unsigned long long start, unsigned long long end, int region, unsigned long long access_permission)
{
    int ret = 0;
    unsigned int apc8;
    unsigned int apc0;

    apc8 = (unsigned int)((access_permission >> 32) & 0xFFFFFFFF);
    apc0 = (unsigned int)(access_permission & 0xFFFFFFFF);

    if((end != 0) || (start !=0))
    {
        /*Address 64KB alignment*/
        start -= 0x40000000;
        end -= 0x40000000;
        start = (start >> 16) & 0x1FFFF;
        end = (end >> 16) & 0x1FFFF;

        if (end < start) {
            return -4;
        }
    }

    //map register set
    if ((ret = msee_mmap_region(EMI_MPU_BASE, (void **)&EMI_MPU_VA_BUFFER, SIZE_4KB, MSEE_MAP_HARDWARE)))
    {
        msee_loge("[emi_mpu_set_region_protection] map EMI_MPU_BASE failed! ret=%d\n", ret);
        return -3;
    }

    if ((region == MSEE_SHARED_SECURE_MEMORY_MPU_REGION_ID) || (region == MSEE_TUI_MEMORY_MPU_REGION_ID))
    {
        writel(start, EMI_MPU_SA(region));
        writel(end, EMI_MPU_EA(region));
        writel(apc8, EMI_MPU_APC(region, 8));
        writel(apc0, EMI_MPU_APC(region, 0));
    }
    else
    {
        msee_loge("[emi_mpu_set_region_protection] region %d is not supported\n", region);
        ret = -2;
    }

    //unmap register set
    if (msee_unmmap_region(EMI_MPU_VA_BUFFER, SIZE_4KB))
    {
        msee_loge("[emi_mpu_set_region_protection] unmap EMI_MPU_BASE failed!\n");
    }

    return ret;
}

