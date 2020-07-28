/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __IMG_MMU_H__
#define __IMG_MMU_H__

#ifdef MTK_M4U_SUPPORT
#include "m4u_lib.h"
#else
typedef int M4U_MODULE_ID_ENUM;
#endif

#include "../inc/img_common_def.h"

#ifndef JUINT32
  typedef unsigned int JUINT32;
#endif

#ifndef JINT32
  typedef int JINT32;
#endif

enum {
     SYNC_HW_READ = 0
    ,SYNC_HW_WRITE = 1
};

#define IMGMMU_PORT_JPEG_READ  (0x01)
#define IMGMMU_PORT_JPEG_WRITE (0x02)

#define ION_M4U_USE_MAP_API

#ifdef MTK_M4U_SUPPORT
bool imgMmu_create(MTKM4UDrv **ppM4uDrv, M4U_MODULE_ID_ENUM mID);
bool imgMmu_cfg_port(MTKM4UDrv *pM4uDrv, M4U_MODULE_ID_ENUM mID, M4U_PORT_ID_ENUM pID);
bool imgMmu_cfg_port_mci(MTKM4UDrv *pM4uDrv, M4U_MODULE_ID_ENUM mID, M4U_PORT_ID_ENUM pID);
bool imgMmu_pa_map_tlb(MTKM4UDrv **ppM4uDrv, M4U_MODULE_ID_ENUM mID, JUINT32 size, JUINT32 *pa);
void imgMmu_pa_unmap_tlb(MTKM4UDrv *pM4uDrv, M4U_MODULE_ID_ENUM mID, JUINT32 size, JUINT32 *pa);
bool imgMmu_reset(MTKM4UDrv **ppM4uDrv, M4U_MODULE_ID_ENUM mID);
bool imgMmu_alloc_pa_mci(MTKM4UDrv **ppM4uDrv, M4U_MODULE_ID_ENUM mID, void* va, JUINT32 size, JUINT32 *pa);
bool imgMmu_alloc_pa(MTKM4UDrv **ppM4uDrv, M4U_MODULE_ID_ENUM mID, void* va, JUINT32 size, JUINT32 *pa);
bool imgMmu_sync(MTKM4UDrv *pM4uDrv, M4U_MODULE_ID_ENUM mID, void* va, JUINT32 mva, JUINT32 size, JUINT32 isWrite );
void imgMmu_dealloc_pa(MTKM4UDrv *pM4uDrv, M4U_MODULE_ID_ENUM mID, void* va, JUINT32 size, JUINT32 *pa);
#endif

bool imgIon_open(JINT32 *ionFD );
bool imgIon_freeVA(void **va, JUINT32 size);
bool imgIon_close(JINT32 ionFD);
bool imgIon_getPA(JINT32 ionFD, JINT32 bufFD, M4U_MODULE_ID_ENUM mID, void *va, JUINT32 size, JINT32 *bufHandle, JUINT32 *pa);
bool imgIon_getVA(JINT32 bufFD, JUINT32 size, void **va);
bool imgIon_sync(JINT32 ionFD, JINT32 bufHandle, JUINT32 isWrite = 0);
#ifdef ION_M4U_USE_MAP_API
bool imgIon_syncDone(JINT32 ionFD, JINT32 bufHandle, JUINT32 isWrite = 0);
#endif

#endif
