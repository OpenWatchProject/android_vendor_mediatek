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

#ifndef _MPU_MTK_H_
#define _MPU_MTK_H_

#include <stdint.h>
#include <xtensa/config/core.h>
#include "utils.h"

#define MPU_DEBUG_SUPPORT       (0)

/* MPU definition */
#define MAX_NUM_REGIONS         (XCHAL_MPU_ENTRIES)
#define MPU_ALIGN_MASK          (0x0FFF)
#define INVALID_BASE_ADDR       ALIGN_MASK(0xFFFFFFFF, MPU_ALIGN_MASK)

#define MPU_REGION_GET_VADDR(x)     (adsp_mpu_region[(x)].vaddr)
#define MPU_REGION_GET_SIZE(x)      (adsp_mpu_region[(x)].size)
#define MPU_REGION_GET_ACCESS(x)    (adsp_mpu_region[(x)].access_right)
#define MPU_REGION_GET_MEMORY_TYPE(x)   (adsp_mpu_region[(x)].memory_type)
#define MPU_REGION_GET_VALID(x)     (adsp_mpu_region[(x)].enable)

#define MPU_ENTRY_SET_AS(x, vaddr, valid)   (x).as = \
    (((vaddr) & 0xffffffe0) | ((valid & 0x1)))
#define MPU_ENTRY_SET_AT(x, access, memtype)    (x).at = \
    (((XTHAL_ENCODE_MEMORY_TYPE(memtype)) << 12) | (((access) & 0xf) << 8))

/* ID */
enum {
    MPU_ID_COMFREQ      = 0UL,
    MPU_ID_DTCM,
    MPU_ID_ITCM,
    MPU_ID_ADSP_PROGRAM_RO,
    MPU_ID_ADSP_RW,
    MPU_ID_ADSP_RW_NC,
    MPU_ID_MD_SHARED_RW,
    MPU_ID_AUDIOSYS,
};

/* ENABLE */
enum {
    MPU_ENTRY_DISABLE   = 0UL,
    MPU_ENTRY_ENABLE    = 1UL,
};

typedef struct {
    uint32_t    vaddr;
    uint32_t    size;
    uint32_t    access_right;
    uint32_t    memory_type;
    uint32_t    enable;
} mpu_region_t;

/* size=16bytes */
typedef struct {
    uint32_t    prog_addr;
    uint32_t    prog_size;
    uint32_t    data_addr;
    uint32_t    data_size;
    uint32_t    data_non_cache_addr;
    uint32_t    data_non_cache_size;
} mpu_info_t;


void dump_mpu_status(void);
int32_t enable_mpu_region(uint32_t region_num);
int32_t disable_mpu_region(uint32_t region_num);
void mpu_init(void);
int32_t request_mpu_region(void);
void mpu_setup_region_address(uint32_t region_num, uint32_t vaddr, uint32_t size);
uint32_t mpu_get_adsp_sram_prot_size(void);

#endif /*_MPU_MTK_H_*/
