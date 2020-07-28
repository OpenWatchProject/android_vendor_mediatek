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

#include <mt_emi_mpu.h>
#include <emi_mpu_v1.h>
#include "tee_drStd.h"
#include <drv_fwk.h>

unsigned char *emi_mpu_va_buffer;
#define EMI_MPU_VA_BUFFER	emi_mpu_va_buffer

inline unsigned int readl(volatile unsigned int addr)
{
    return *((volatile unsigned int *)((uintptr_t)addr));
}

inline void writel(unsigned int val, volatile unsigned int addr)
{
    *((volatile unsigned int *)((uintptr_t)addr)) = val;
}

int emi_mpu_set_protection(struct emi_region_info_t *region_info)
{
    int i;
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

#ifdef MPU_LAGACY
    writel((start << 16) | end, EMI_MPU_A(region_info->region));
#else
    writel(start, EMI_MPU_SA(region_info->region));
    writel(end, EMI_MPU_EA(region_info->region));
#endif
    for (i = EMI_MPU_DGROUP_NUM - 1; i >= 0; i--)
        writel(region_info->apc[i], EMI_MPU_APC(region_info->region, i));

    //unmap register set
    if (msee_unmmap_region(EMI_MPU_VA_BUFFER, SIZE_4KB)) {
        msee_loge("[emi_mpu_set_protection] unmap EMI_MPU_BASE failed!\n");
        return -1;
    }

    return 0;
}

