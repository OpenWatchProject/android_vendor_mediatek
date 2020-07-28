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

#include "audio_task_audplayback.h"
#include "audio_task_common.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <aurisys_scenario_dsp.h>
#endif

static struct audio_task_common *task_handle;

#define MAX_MSG_QUEUE_SIZE (24)
#define LOCAL_TASK_STACK_SIZE (2304)
#define LOCAL_TASK_NAME "aud_playback"
#define LOCAL_TASK_PRIORITY (3)
#define LOCAL_TASK_OUPUT_BUFFER_SIZE (4096)

AudioTask *task_audplayback_new(void)
{
    /* alloc object here */
    int ret = 0;
    AudioTask *task = NULL;
    ret = init_common_task(&task_handle);
    if (ret) {
        AUD_LOG_E("%s(), init_common_task fail!!\n", __func__);
        return NULL;
    }

    task = (AudioTask *)AUDIO_MALLOC(sizeof(AudioTask));
    if (task == NULL) {
        AUD_LOG_E("%s(), AUDIO_MALLOC fail!!\n", __func__);
        return NULL;
    }

    /* task attribute*/
    task_handle->max_queue_size = MAX_MSG_QUEUE_SIZE;
    task_handle->task_stack_size = LOCAL_TASK_STACK_SIZE;
    task_handle->task_priority = LOCAL_TASK_PRIORITY;
    task_handle->task_name = LOCAL_TASK_NAME;
    task_handle->task_scene = TASK_SCENE_AUDPLAYBACK;

    task_handle->playback_hwbuf_handle = -1;
    task_handle->capture_hwbuf_handle = -1;
    task_handle->ref_hwbuf_handle = -1;

    task_handle->reserved_mem_id = PLAYBACK_DMA_ID;

    task_handle->task_profile_id = TASK_AUDPLAYUBACK_TIME_ID1;
    task_handle->task_profile_interval = 60000000;

    task_handle->mtask_processing_buf_out_size = LOCAL_TASK_OUPUT_BUFFER_SIZE;
    task_handle->mtask_input_buf_size = LOCAL_TASK_OUPUT_BUFFER_SIZE;
    task_handle->mtask_default_buffer_size = LOCAL_TASK_OUPUT_BUFFER_SIZE;
    task_handle->mtask_iv_buf = AUDIO_MALLOC(LOCAL_TASK_OUPUT_BUFFER_SIZE);
    task_handle->mtask_iv_buf_size = LOCAL_TASK_OUPUT_BUFFER_SIZE;

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
    task_handle->mtask_output_buf_size = LOCAL_TASK_OUPUT_BUFFER_SIZE;
#else
    task_handle->AurisysScenario = AURISYS_SCENARIO_DSP_PLAYBACK;
#endif

#ifndef FORCE_ALL_TASK_DEBUG_LEVEL
    task_handle->debug_flag = TASK_DEBUG_DEBUG;
#else
    task_handle->debug_flag = FORCE_ALL_TASK_DEBUG_LEVEL;
#endif

    AUD_LOG_D("%s(), debug level = %d\n", __func__, task_handle->debug_flag);

    /* only assign methods, but not class members here */
    task->constructor       = task_common_constructor;
    task->destructor        = task_common_destructor;

    task->create_task_loop  = task_common_create_task_loop;

    task->recv_message      = task_common_recv_message;
    task->irq_hanlder       = task_common_irq_handler;

    task->hal_reboot_cbk    = task_common_hal_reboot_cbk;

    task->task_priv = task_handle;

    return task;
}

void task_audplayback_delete(AudioTask *task)
{
    AUD_LOG_V("%s(+), task_scene = %d\n", __func__, task->scene);

    int ret = 0;
    ret = deinit_common_task(&task_handle);
    if (ret) {
        AUD_LOG_E("%s(), deinit_common_task ret[%d]!!\n", __func__, ret);
    }

    if (task == NULL) {
        AUD_LOG_E("%s(), task is NULL!!\n", __func__);
        return;
    }
    AUDIO_FREE(task);

    if (task_handle->mtask_iv_buf != NULL) {
        AUDIO_FREE(task_handle->mtask_iv_buf);
        task_handle->mtask_iv_buf = NULL;
    }

    AUD_LOG_V("%s(-)\n", __func__);
}

