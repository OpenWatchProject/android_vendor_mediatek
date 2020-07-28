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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"


// device apc attribute
 typedef enum
 {
     E_L0 = 0,
     E_L1 = 1,
     E_L2 = 2,
     E_L3 = 3,
     E_MAX_APC_ATTR,
     E_APC_ATTR_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
 } APC_ATTR;

 // domain index
 typedef enum
 {
     E_DOMAIN_0 = 0,
     E_DOMAIN_1 = 1,
     E_DOMAIN_2 = 2,
     E_DOMAIN_3 = 3,
     E_DOMAIN_4 = 4,
     E_DOMAIN_5 = 5,
     E_DOMAIN_6 = 6,
     E_DOMAIN_7 = 7,
     E_MAX_MAS_DOM,
     E_MAS_DOM_RESERVRD = 0x7FFFFFFF  /* force enum to use 32 bits */
 } E_MAS_DOM;


#define MOD_NO_IN_1_DEVAPC      16
#define MASTER_MAX_INDEX        29
#define DEVAPC_DEVICE_NUMBER    221  //This number must be bigger than total slave


#define DEVAPC_AO_BASE         0x1000E000
#define DEVAPC_PD_BASE         0x10207000
#define INFRACFG_AO_base       0x10001000


#define DEVAPC_AO_VA_BUFFER (devapc_ao_vaddr)

#define DEVAPC_D0_APC_0            ((volatile unsigned int*)(DEVAPC_AO_VA_BUFFER+0x0000))
#define DEVAPC_D1_APC_0            ((volatile unsigned int*)(DEVAPC_AO_VA_BUFFER+0x0100))
#define DEVAPC_D2_APC_0            ((volatile unsigned int*)(DEVAPC_AO_VA_BUFFER+0x0200))
#define DEVAPC_D3_APC_0            ((volatile unsigned int*)(DEVAPC_AO_VA_BUFFER+0x0300))
#define DEVAPC_D4_APC_0            ((volatile unsigned int*)(DEVAPC_AO_VA_BUFFER+0x0400))
#define DEVAPC_D5_APC_0            ((volatile unsigned int*)(DEVAPC_AO_VA_BUFFER+0x0500))
#define DEVAPC_D6_APC_0            ((volatile unsigned int*)(DEVAPC_AO_VA_BUFFER+0x0600))
#define DEVAPC_D7_APC_0            ((volatile unsigned int*)(DEVAPC_AO_VA_BUFFER+0x0700))

#define DEVAPC_MAS_SEC             ((volatile unsigned int*)(DEVAPC_AO_VA_BUFFER+0x0B00))

#define DEVAPC_APC_CON             ((volatile unsigned int*)(DEVAPC_AO_VA_BUFFER+0x0F00))

