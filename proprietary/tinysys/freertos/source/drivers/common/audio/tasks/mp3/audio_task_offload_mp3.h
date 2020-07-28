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
#ifndef AUDIO_TASK_OFFLOADMP3_H
#define AUDIO_TASK_OFFLOADMP3_H

#include "audio_task_interface.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <wrapped_audio.h>
#include <audio_ringbuf_pure.h>
#include <audio_task_aurisys.h>
#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#include <aurisys_utility.h>
#include <aurisys_config.h>
#endif

AudioTask *task_offload_new();
void task_offload_delete(AudioTask *task);

enum {
    TASK_SCENE_OFFLOAD_MP3,
    TASK_SCENE_OFFLOAD_AAC
};

typedef uint8_t task_offload_scene_t;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
struct audio_task_offload_aurisys {
    /* task relate attribute */
    unsigned int max_queue_size;
    unsigned int task_stack_size;
    int task_priority;
    char *task_name;
    unsigned char task_scene;


    /* ap dsp share ring buffer */
    struct RingBuf mtask_audio_ring_buf;
    struct RingBuf mtask_audio_ring_buf_ref;

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

    char *mtask_processing_buf;
    int mtask_processing_buf_out_size;

    /* task aurisys data */
    struct audio_aurisys_handle_t *task_aurisys;

    /* debug related */
    int debug_flag;
    int loop_count;

#ifdef MCPS_PROF_SUPPORT
    /* MCPS profiling */
    mcps_profile_t mcps_profile;
#endif
};

#endif

#endif // end of AUDIO_TASK_OFFLOADMP3_H

