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

#ifndef _DVFS_COMMON_
#define _DVFS_COMMON_

#include <dma.h>
#include <sleep.h>

/***********************************************************************************
** Definitions
************************************************************************************/

/* DVFS Status */
#define DVFS_STATUS_OK                           0
#define DVFS_STATUS_BUSY                         -1
#define DVFS_REQUEST_SAME_CLOCK                  -2
#define DVFS_STATUS_ERR                          -3
#define DVFS_STATUS_TIMEOUT                      -4
#define DVFS_CLK_ERROR                           -5
#define DVFS_STATUS_CMD_FIX                      -6
#define DVFS_STATUS_CMD_LIMITED                  -7
#define DVFS_STATUS_CMD_DISABLE                  -8

#define SCP_IDLE_MODE                             0
#define SCP_SLEEP_MODE                            1

#define SCP_ENABLE                                1
#define SCP_DISABLE                               0

#define NEED_WAIT                                 1
#define NO_WAIT                                   0
 /*****************************************
 * ADB CMD Control APIs
 ****************************************/
extern void dvfs_debug_set(int id, void* data, unsigned int len);

/*****************************************
 * AP/INFRA Resource APIs
 ****************************************/
extern void enable_AP_resource(scp_reserve_mem_id_t dma_id);
extern void disable_AP_resource(scp_reserve_mem_id_t dma_id);
extern void enable_infra(scp_reserve_mem_id_t dma_id, uint32_t wait_ack);
extern void disable_infra(scp_reserve_mem_id_t dma_id);

#endif /* _DVFS_COMMON_ */
