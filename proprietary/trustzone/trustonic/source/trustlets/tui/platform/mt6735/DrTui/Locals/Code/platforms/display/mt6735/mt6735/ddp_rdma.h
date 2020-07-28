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

#ifndef _DDP_RDMA_API_H_
#define _DDP_RDMA_API_H_
#include "disp_session.h"
#include "DpDataType.h"
#include "ddp_hal.h"

#define RDMA_INSTANCES  3
#define RDMA_MAX_WIDTH  4095
#define RDMA_MAX_HEIGHT 4095

enum RDMA_OUTPUT_FORMAT {
    RDMA_OUTPUT_FORMAT_ARGB   = 0,
    RDMA_OUTPUT_FORMAT_YUV444 = 1,
};

enum RDMA_MODE {
    RDMA_MODE_DIRECT_LINK = 0,
    RDMA_MODE_MEMORY      = 1,
};

typedef struct _RDMA_BASIC_STRUCT
{
    unsigned long addr;
	unsigned int src_w;
    unsigned int src_h;
    unsigned int bpp;
}RDMA_BASIC_STRUCT;


void rdma_dump_reg(DISP_MODULE_ENUM module);
void rdma_dump_analysis(DISP_MODULE_ENUM module);

int rdma_reg_backup(DISP_MODULE_ENUM module);
int rdma_config(DISP_MODULE_ENUM module,
                    enum RDMA_MODE mode,
                    unsigned long address,
                    DpColorFormat inFormat,
                    unsigned pitch,
                    unsigned width,
                    unsigned height,
                    unsigned ufoe_enable,
		           	DISP_BUFFER_TYPE sec,
                    void * handle);
unsigned int rdma_enable_irq(DISP_MODULE_ENUM module, void *handle, DDP_IRQ_LEVLE irq_level);
unsigned int rdma_disable_irq_backup(DISP_MODULE_ENUM module, void *handle);
int rdma_irq_restore(DISP_MODULE_ENUM module, void *handle, unsigned int irq_val);

int rdma_config_input_mode(DISP_MODULE_ENUM module,
                    enum RDMA_MODE mode,
                    void * handle);

int rdma_config_mem(DISP_MODULE_ENUM module,
                    unsigned long address,
                    DpColorFormat inFormat,
                    unsigned pitch,
                    void * handle);
int rdma_irq_attach(DISP_MODULE_ENUM module);
int rdma_irq_dettach(DISP_MODULE_ENUM module);
int rdma_wait_frame_done_by_polling(DISP_MODULE_ENUM module);
int rdma_wait_frame_done_by_interrupt(DISP_MODULE_ENUM module);
int rdma_wait_frame_done_by_cmdq(DISP_MODULE_ENUM module, cmdqRecHandle cmdq_handle,
			int clear_before_wait, int clear_after_wait);
CMDQ_EVENT_ENUM rdma_get_EOF_cmdq_event(DISP_MODULE_ENUM module);
int rdma_is_mem_mode(DISP_MODULE_ENUM module);

#endif
