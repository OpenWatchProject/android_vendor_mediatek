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

#ifndef __DDP_PATH_H__
#define __DDP_PATH_H__

#include "ddp_info.h"
#include "cmdq_sec_record.h"

#define DDP_OVL_LAYER_MUN 4


enum DDP_MODE {
	DDP_VIDEO_MODE = 0,
	DDP_CMD_MODE,
};

enum CLK_MODE {
	LP_MODE = 0,
	HS_MODE,
};


enum DDP_SCENARIO_ENUM {
	DDP_SCENARIO_PRIMARY_DISP = 0, /* main path */
	DDP_SCENARIO_PRIMARY_BYPASS_PQ_DISP,	/* bypass pq module */
	DDP_SCENARIO_PRIMARY_RDMA0_COLOR0_DISP, /* by pass ovl */
	DDP_SCENARIO_PRIMARY_RDMA0_DISP, /* debug */
	DDP_SCENARIO_PRIMARY_OVL_MEMOUT, /* dc */
	DDP_SCENARIO_PRIMARY_ALL, /* main 1to2 */
	DDP_SCENARIO_SUB_DISP,
	DDP_SCENARIO_SUB_RDMA1_DISP,
	DDP_SCENARIO_SUB_OVL_MEMOUT,
	DDP_SCENARIO_SUB_ALL,
	DDP_SCENARIO_MAX
};

int ddp_get_module_num(enum DDP_SCENARIO_ENUM scenario);

void ddp_check_path(enum DDP_SCENARIO_ENUM scenario);
int ddp_check_path_strict(enum DDP_SCENARIO_ENUM scenario);

/*int ddp_mutex_set(int mutex_id, DDP_SCENARIO_ENUM scenario, DDP_MODE mode, void *handle);*/
int ddp_mutex_enable(int mutex_id, enum DDP_SCENARIO_ENUM scenario, void *handle);
int ddp_mutex_disable(int mutex_id, enum DDP_SCENARIO_ENUM scenario, void *handle);
void ddp_check_mutex(int mutex_id, enum DDP_SCENARIO_ENUM scenario, enum DDP_MODE mode);

int ddp_is_moudule_in_mutex(int mutex_id, enum DISP_MODULE_ENUM module);

enum DISP_MODULE_ENUM ddp_get_dst_module(enum DDP_SCENARIO_ENUM scenario);
int ddp_set_dst_module(enum DDP_SCENARIO_ENUM scenario, enum DISP_MODULE_ENUM dst_module);

int *ddp_get_scenario_list(enum DDP_SCENARIO_ENUM ddp_scenario);

char *ddp_get_scenario_name(enum DDP_SCENARIO_ENUM scenario);

int disp_get_dst_module(enum DDP_SCENARIO_ENUM scenario);
int ddp_is_module_in_scenario(enum DDP_SCENARIO_ENUM ddp_scenario, enum DISP_MODULE_ENUM module);
char *ddp_get_mutex_sof_name(unsigned int regval);
int ddp_path_init(void);

int ddp_mutex_enable_l(int mutex_idx, void *handle);
int ddp_mutex_reg_backup(int mutex_id);


#endif
