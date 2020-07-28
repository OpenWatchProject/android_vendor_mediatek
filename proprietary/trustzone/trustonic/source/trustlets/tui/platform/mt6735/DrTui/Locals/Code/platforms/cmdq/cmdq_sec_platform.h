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

#ifndef __CMDQ_SEC_PLATFORM_H__
#define __CMDQ_SEC_PLATFORM_H__

/* platform dependent utilities, format: cmdq_{util_type}_{name} */

#include "cmdq_sec_def.h"
#include "cmdq_sec_core.h"

#define CMDQ_SPECIAL_SUBSYS_ADDR 99

void cmdq_tz_poke_notify_loop(void);

/* IRQ handler */
void cmdq_tz_irq_handler(void);

const bool cmdq_tz_is_a_secure_thread(const int32_t thread);

/*
 * GCE capability
 */

/* get LSB for subsys encoding in argA (range: 0 - 31) */
const uint32_t cmdq_tz_get_subsys_LSB_in_argA(void);
/* get subsys from physical address */
int32_t cmdq_tz_subsys_from_phys_addr(uint32_t physAddr);

/* scenario */
bool cmdq_tz_is_disp_scenario(const CMDQ_SCENARIO_ENUM scenario);
CMDQ_HW_THREAD_PRIORITY_ENUM cmdq_tz_priority_from_scenario(CMDQ_SCENARIO_ENUM scenario);
int cmdq_tz_thread_index_from_scenario(CMDQ_SCENARIO_ENUM scenario);

/**
 * Record usage
 *
 */
uint64_t cmdq_tz_rec_flag_from_scenario(CMDQ_SCENARIO_ENUM scn);
bool cmdq_tz_should_enable_prefetch(CMDQ_SCENARIO_ENUM scenario);

/**
 * Debug
 *
 */
void cmdq_tz_dump_mmsys_config(void);

/**
 * Security
 */
int32_t cmdq_tz_get_DAPC_security_reg_and_mask(
			const CMDQ_SCENARIO_ENUM scenario,
			uint64_t engineFlag,
			uint32_t *pInstrA, uint32_t *pInstrB, uint32_t *pValueEnabled, uint32_t *pValueDisabled);
int32_t cmdq_tz_get_port_security_reg_and_mask(
			uint64_t engineFlag, uint32_t *pInstrA, uint32_t *pInstrB, uint32_t *pValueEnabled, uint32_t *pValueDisabled);

#endif				/* __CMDQ_SEC_PLATFORM_H__ */
