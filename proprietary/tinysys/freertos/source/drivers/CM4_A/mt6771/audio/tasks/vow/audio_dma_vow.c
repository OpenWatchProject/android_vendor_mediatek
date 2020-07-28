
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
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Header Files
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#include <audio_type.h>
#include <dma.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#include <audio_task_vow_params.h>
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Static Variables
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/


/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        DMA WRAP API
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/


/**
* VOW DMA transfer
* @param destination address
* @param source address
* @param count number of bytes need to copy
*/

/*
typedef enum {
    DMA_RESULT_DONE = 0,
    DMA_RESULT_FULL,
    DMA_RESULT_RUNNING,
    DMA_RESULT_ERROR
} DMA_RESULT;
*/
uint32_t vow_dma_transaction(uint32_t dst_addr, uint32_t src_addr, uint32_t len)
{
    uint32_t ret;
#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_enable_DRAM_resource(VOW_MEM_ID);
#endif
    ret = (uint32_t)scp_dma_transaction(dst_addr, src_addr, len, VOW_DMA_ID, NO_RESERVED);
#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_disable_DRAM_resource(VOW_MEM_ID);
#endif
    return ret;
}
