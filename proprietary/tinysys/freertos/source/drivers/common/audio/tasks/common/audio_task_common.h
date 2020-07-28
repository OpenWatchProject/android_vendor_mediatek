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
#ifndef AUDIO_TASK_COMMON_H
#define AUDIO_TASK_COMMON_H

#include "audio_task_interface.h"
#include <stdarg.h>
#include <interrupt.h>
#include <dma.h>
#include <wrapped_errors.h>

#ifdef CFG_VCORE_DVFS_SUPPORT
#include <wakelock.h>
#include <feature_manager.h>
#include <dvfs.h>
#endif

#ifdef CFG_AUDIO_SUPPORT
#include <adsp_ipi.h>
#include <audio_ipi_dma.h>
#else
#include <scp_ipi.h>
#endif

#include "audio_messenger_ipi.h"
#include "audio_task_factory.h"
#include "arsi_api.h"
#include <audio_memory_control.h>

/* audio utility relate*/
#include <audio_ringbuf.h>
#include <audio_task_utility.h>
#include <audio_task_attr.h>
#include <audio_dsp_hw_hal.h>

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <wrapped_audio.h>
#include <audio_ringbuf_pure.h>
#include <audio_task_aurisys.h>
#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#include <aurisys_utility.h>
#include <aurisys_config.h>
#endif

#ifdef MCPS_PROF_SUPPORT
#include "mcps_prof.h"
#endif

#define task_default_buffer_size (4096)
#define LOOP_COUNT_MAX (20)

struct audio_task_common {
    /* task relate attribute */
    unsigned int max_queue_size;
    unsigned int task_stack_size;
    int task_priority;
    char *task_name;
    unsigned char task_scene;

    /* audio ul/dl buffer */
    struct audio_hw_buffer mtask_audio_dl_buf;
    struct audio_hw_buffer mtask_audio_ul_buf;
    struct audio_hw_buffer mtask_audio_buf_working;
    /* afe hardware buffer */
    struct audio_hw_buffer mtask_audio_afedl_buf;
    struct audio_hw_buffer mtask_audio_afeul_buf;
    struct audio_hw_buffer mtask_audio_ref_buf;

    /* ap dsp share memory */
    struct audio_dsp_dram mtask_dsp_atod_sharemm;
    struct audio_dsp_dram mtask_dsp_dtoa_sharemm;

    /* ap dsp share ring buffer */
    struct RingBuf mtask_audio_ring_buf;
    struct RingBuf mtask_audio_ring_buf_ref;

    /* buffer handle path related*/
    int playback_hwbuf_handle;
    int capture_hwbuf_handle;
    int ref_hwbuf_handle;

    /* time info*/
    struct task_time_info time_ul;
    struct task_time_info time_dl;
    struct task_time_info time_ref;

    int pcmdump_enable;

    int AurisysScenario;

    /* buufer move with dma */
    int dma_enable;
    int dummy_write;

    /* resource relate */
    int reserved_mem_id;

    /* profile_related */
    int task_profile_id;
    unsigned long long task_profile_interval;

    char *mtask_output_buf;
    int mtask_output_buf_size;

    char *mtask_input_buf;
    int mtask_input_buf_size;

    char *mtask_ref_buf;
    int mtask_ref_buf_size;

    char *mtask_iv_buf;
    int mtask_iv_buf_size;

    char *mtask_processing_buf;
    int mtask_processing_buf_out_size;
    int mtask_processing_buf_in_size;

    /* default task output size */
    unsigned int mtask_default_buffer_size;

    /* task aurisys data */
    struct audio_aurisys_handle_t *task_aurisys;

    /* read/write count loop */
    int loop_count;

    /* debug related */
    int debug_flag;

#ifdef MCPS_PROF_SUPPORT
    /* MCPS profiling */
    mcps_profile_t mcps_profile;
#endif

};

/* task related function*/
void task_common_task_loop(void *pvParameters);
void task_common_constructor(struct AudioTask *this);
void task_common_destructor(struct AudioTask *this);
void task_common_create_task_loop(struct AudioTask *this);
status_t task_common_recv_message(struct AudioTask *this,
                                  struct ipi_msg_t *ipi_msg);

void task_common_irq_handler(struct AudioTask *this,
                             uint32_t irq_type);
void task_common_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type);


int init_common_task(struct audio_task_common **task_common);
int deinit_common_task(struct audio_task_common **task_common);

/* when set start time, init task_time_info */
int set_task_time_start(int handle ,struct task_time_info *time_info);
int add_audio_time_stamp_frame(struct task_time_info *time_info, int frame);
int get_audio_time_stamp(int handle ,struct task_time_info *time_info);
void print_audio_time_stamp(int handle, struct task_time_info *time_info);
int task_require_resource(struct audio_task_common *task_commom);
int task_release_resource(struct audio_task_common *task_commom);
void task_print(struct audio_task_common *task_common, char *str);
void dump_task_hwbuf(struct audio_task_common *task_common, char *str);
#endif // end of AUDIO_TASK_COMMON_H

