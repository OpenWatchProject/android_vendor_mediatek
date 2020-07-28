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

#ifndef __DRAPIPLAT_H__
#define __DRAPIPLAT_H__

unsigned int mt_get_chip_hw_code(void);
int emi_mpu_set_region_protection(unsigned long long start, unsigned long long end, int region, unsigned long long access_permission);

/* For EMI MPU */
#define SET_ACCESS_PERMISSON(lock, d15, d14, d13, d12, d11, d10, d9, d8, d7, d6, d5, d4, d3, d2, d1, d0) \
	((((unsigned long long) d15) << 53) | (((unsigned long long) d14) << 50) | (((unsigned long long) d13) << 47) | \
	 (((unsigned long long) d12) << 44) | (((unsigned long long) d11) << 41) | (((unsigned long long) d10) << 38) | \
	 (((unsigned long long) d9)  << 35) | (((unsigned long long) d8)  << 32) | (((unsigned long long) d7)  << 21) | \
	 (((unsigned long long) d6)  << 18) | (((unsigned long long) d5)  << 15) | (((unsigned long long) d4)  << 12) | \
	 (((unsigned long long) d3)  <<  9) | (((unsigned long long) d2)  <<  6) | (((unsigned long long) d1)  <<  3) | \
	  ((unsigned long long) d0) | ((unsigned long long) lock << 31))

#define SHARED_SECURE_MEMORY_MPU_REGION_ID  2
#define TUI_MEMORY_MPU_REGION_ID  5

#define EMI_MPU_LOCK                1
#define EMI_MPU_UNLOCK              0

#define EMI_MPU_NO_PROTECTION       0
#define EMI_MPU_SEC_RW              1
#define EMI_MPU_SEC_RW_NSEC_R       2
#define EMI_MPU_SEC_RW_NSEC_W       3
#define EMI_MPU_SEC_R_NSEC_R        4
#define EMI_MPU_FORBIDDEN           5
#define SEC_R_NSEC_RW               6

#define INTERAL_MAPPING_OFFSET (0x40000000)
#define INTERAL_MAPPING_LIMIT (INTERAL_MAPPING_OFFSET + 0x80000000)

#define MAPPING_DRAM_ACCESS_ADDR(phy_addr) (phy_addr)


#endif // __DRAPITIME_H__
