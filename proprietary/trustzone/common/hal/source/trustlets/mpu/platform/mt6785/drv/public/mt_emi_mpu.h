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

#ifndef __MT_EMI_MPU_H__
#define __MT_EMI_MPU_H__

#define EMI_MPU_BASE			(0x10226000)
/* EMI Memory Protect Unit */
#define EMI_MPU_CTRL			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x000))
#define EMI_MPU_DBG			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x004))
#define EMI_MPU_SA0			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x100))
#define EMI_MPU_EA0			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x200))
#define EMI_MPU_SA(region)		((volatile unsigned int)(EMI_MPU_SA0 + (region*4)))
#define EMI_MPU_EA(region)		((volatile unsigned int)(EMI_MPU_EA0 + (region*4)))
#define EMI_MPU_APC0			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x300))
#define EMI_MPU_APC(region,dgroup)	((volatile unsigned int)(EMI_MPU_APC0 + (region*4) +(dgroup*0x100)))
#define EMI_MPU_CTRL_D0			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x800))
#define EMI_MPU_CTRL_D(domain)		((volatile unsigned int)(EMI_MPU_CTRL_D0 + (domain*4)))
#define EMI_RG_MASK_D0			((volatile unsigned int)(EMI_MPU_VA_BUFFER + 0x900))
#define EMI_RG_MASK_D(domain)		((volatile unsigned int)(EMI_RG_MASK_D0 + (domain*4)))

#define EMI_MPU_DOMAIN_NUM	16
#define EMI_MPU_REGION_NUM	32
#define EMI_MPU_ALIGN_BITS	16
#define DRAM_OFFSET		(0x40000000 >> EMI_MPU_ALIGN_BITS)

#define MAX_CH	2
#define MAX_RK	2

#define SHARED_SECURE_MEMORY_MPU_REGION_ID	2
#define TUI_MEMORY_MPU_REGION_ID		7

#include "../../../../common/drv/public/emi_mpu_v1.h"

#endif // __MT_EMI_MPU_H__
