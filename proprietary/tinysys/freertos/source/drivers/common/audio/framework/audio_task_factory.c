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

#include "audio_task_factory.h"

#include <tinysys_config.h>

#include <audio_type.h>

#include <audio_task_utility.h>


#ifndef CFG_DO_ENABLED /* without DO */ /* TODO: check only once in one file */
#ifdef CFG_MTK_AURISYS_PHONE_CALL_SUPPORT
#include <audio_task_phone_call.h>
#endif
#ifdef CFG_MTK_AUDIO_TUNNELING_SUPPORT
#include <audio_task_offload_mp3.h>
#endif
#ifdef CFG_MTK_VOW_SUPPORT
#include <audio_task_vow.h>
#endif
#ifdef CFG_MTK_VOICE_ULTRASOUND_SUPPORT
#include <audio_task_ultrasound_proximity.h>
#endif
#endif /* end of !CFG_DO_ENABLED */

#ifdef CFG_MTK_SPEAKER_PROTECTION_SUPPORT
#include <audio_task_speaker_protection.h>
#endif

#ifdef CFG_MTK_AUDIO_FRAMEWORK_SUPPORT
#include <audio_task_primary.h>
#include <audio_task_audplayback.h>
#include <audio_task_deepbuffer.h>
#include <audio_task_voip.h>
#include <audio_task_daemon.h>
#include <audio_task_capture_ul1.h>
#include <audio_task_call_final.h>
#include <audio_task_ktv.h>
#ifdef CFG_A2DP_OFFLOAD_SUPPORT
#include <audio_task_a2dp.h>
#include <audio_task_dataprovider.h>
#endif
#endif

#if defined(CFG_SMARTPA_USE_CACHE) && defined(CFG_MTK_SPEAKER_PROTECTION_SUPPORT)
#include <cache_internal.h>
#endif

#if defined(CFG_MTK_AUDIODSP_SUPPORT) || defined(CFG_MTK_AUDIO_CM4_DMA_SUPPORT)
#include <audio_task_controller.h>
#endif

#if defined(CFG_SMARTPA_USE_CACHE) && defined(CFG_MTK_SPEAKER_PROTECTION_SUPPORT)
SRAM_REGION_VARIABLE
#endif
static AudioTask *g_task_array[TASK_SCENE_SIZE];

#ifdef CFG_DO_ENABLED /* with DO => need write back */
static data_buf_t g_task_write_back_data[TASK_SCENE_SIZE];
#endif

#ifndef CFG_DO_ENABLED /* without DO */
static void create_all_audio_task();
static void destroy_all_audio_task();
#endif


void audio_task_factory_init()
{
    int i = 0;
    AUD_LOG_D("%s\n", __func__);

    for (i = 0; i < TASK_SCENE_SIZE; i++) {
        g_task_array[i] = NULL;

#ifdef CFG_DO_ENABLED /* with DO => need write back */
        g_task_write_back_data[i].memory_size = 0;
        g_task_write_back_data[i].data_size = 0;
        g_task_write_back_data[i].p_buffer = NULL;
#endif
    }

#ifndef CFG_DO_ENABLED /* need to create all tasks in factory without DO */
    create_all_audio_task();
#endif
}


void audio_task_factory_deinit()
{
    int i = 0;

#ifndef CFG_DO_ENABLED /* need to destroy all tasks in factory without DO */
    destroy_all_audio_task();
#endif

    for (i = 0; i < TASK_SCENE_SIZE; i++) {
        AUD_ASSERT(g_task_array[i] == NULL);

#ifdef CFG_DO_ENABLED /* with DO => need write back */
        if (g_task_write_back_data[i].p_buffer != NULL) {
            AUD_ASSERT(g_task_write_back_data[i].memory_size != 0);
            kal_vPortFree(g_task_write_back_data[i].p_buffer);
            g_task_write_back_data[i].memory_size = 0;
            g_task_write_back_data[i].data_size = 0;
            g_task_write_back_data[i].p_buffer = NULL;
        }
#endif
    }
}


#ifdef CFG_DO_ENABLED /* with DO => need write back */
static void write_back_data_to_task(AudioTask *task)
{
    data_buf_t *wb_buf = &g_task_write_back_data[task->scene];

    if (wb_buf->p_buffer != NULL) {
        AUD_ASSERT(wb_buf->memory_size != 0);

        if (task->do_write_back != NULL) {
            task->do_write_back(task, wb_buf);
        }

        kal_vPortFree(wb_buf->p_buffer);
        wb_buf->memory_size = 0;
        wb_buf->data_size = 0;
        wb_buf->p_buffer = NULL;
    }
}


static void copy_write_back_data_from_task(AudioTask *task)
{
    data_buf_t *wb_buf = &g_task_write_back_data[task->scene];

    if (task->get_write_back_size != NULL && task->copy_write_back_buffer != NULL) {
        AUD_ASSERT(wb_buf->memory_size == 0);
        AUD_ASSERT(wb_buf->data_size == 0);
        AUD_ASSERT(wb_buf->p_buffer == NULL);

        wb_buf->memory_size = task->get_write_back_size(task);
        if (wb_buf->memory_size != 0) {
            wb_buf->p_buffer = kal_pvPortMalloc(wb_buf->memory_size);
            if (wb_buf->p_buffer == NULL) {
                AUD_LOG_E("%s(), cannot alloc %u bytes write back data for task %d\n",
                          __func__, wb_buf->memory_size, task->scene);
                wb_buf->memory_size = 0;
            } else {
                memset(wb_buf->p_buffer, 0, wb_buf->memory_size);
                task->copy_write_back_buffer(task, wb_buf);
            }
        }
    }
}
#endif


#ifndef CFG_DO_ENABLED /* need to create/destroy all tasks in factory without DO */
static void create_all_audio_task()
{

    AUD_LOG_D("%s\n",  __func__);

#ifdef CFG_MTK_AURISYS_PHONE_CALL_SUPPORT
    create_audio_task(TASK_SCENE_PHONE_CALL, task_phone_call_new);
#endif


#ifdef CFG_MTK_AUDIO_TUNNELING_SUPPORT
    create_audio_task(TASK_SCENE_PLAYBACK_MP3, task_offload_new);
#endif


#ifdef CFG_MTK_VOW_SUPPORT
    create_audio_task(TASK_SCENE_VOW, task_vow_new);
#endif

#ifdef CFG_MTK_VOICE_ULTRASOUND_SUPPORT
    create_audio_task(TASK_SCENE_VOICE_ULTRASOUND, task_ultrasound_proximity_new);
#endif
#ifdef CFG_MTK_SPEAKER_PROTECTION_SUPPORT
    create_audio_task(TASK_SCENE_SPEAKER_PROTECTION, task_speaker_protection_new);
#endif

#ifdef CFG_MTK_AUDIO_FRAMEWORK_SUPPORT
    create_audio_task(TASK_SCENE_AUDPLAYBACK, task_audplayback_new);
    create_audio_task(TASK_SCENE_DEEPBUFFER, task_deepbuffer_new);
    create_audio_task(TASK_SCENE_AUD_DAEMON, task_daemon_new);
    create_audio_task(TASK_SCENE_PRIMARY, task_primary_new);
    create_audio_task(TASK_SCENE_VOIP, task_voip_new);
    create_audio_task(TASK_SCENE_CAPTURE_UL1, task_capture_ul1_new);
    create_audio_task(TASK_SCENE_CALL_FINAL, task_call_final_new);
    create_audio_task(TASK_SCENE_KTV, task_ktv_new);
#ifdef CFG_A2DP_OFFLOAD_SUPPORT
    create_audio_task(TASK_SCENE_A2DP, task_a2dp_new);
    create_audio_task(TASK_SCENE_DATAPROVIDER, task_dataprovider_new);
#endif
#endif

#if defined(CFG_MTK_AUDIODSP_SUPPORT)
    create_audio_task(TASK_SCENE_AUDIO_CONTROLLER_HIFI3_A, task_controller_new);
#elif defined(CFG_MTK_AUDIO_CM4_DMA_SUPPORT)
    create_audio_task(TASK_SCENE_AUDIO_CONTROLLER_CM4, task_controller_new);
#endif
}

static void destroy_all_audio_task()
{

#ifdef CFG_MTK_AURISYS_PHONE_CALL_SUPPORT
    destroy_audio_task(TASK_SCENE_PHONE_CALL, task_phone_call_delete);
#endif


#ifdef CFG_MTK_AUDIO_TUNNELING_SUPPORT
    destroy_audio_task(TASK_SCENE_PLAYBACK_MP3, task_offload_delete);
#endif


#ifdef CFG_MTK_VOW_SUPPORT
    destroy_audio_task(TASK_SCENE_VOW, task_vow_delete);
#endif

#ifdef CFG_MTK_VOICE_ULTRASOUND_SUPPORT
    destroy_audio_task(TASK_SCENE_VOICE_ULTRASOUND, task_ultrasound_proximity_delete);
#endif
#ifdef CFG_MTK_SPEAKER_PROTECTION_SUPPORT
    destroy_audio_task(TASK_SCENE_SPEAKER_PROTECTION,
                       task_speaker_protection_delete);
#endif

#ifdef CFG_MTK_AUDIO_FRAMEWORK_SUPPORT
    destroy_audio_task(TASK_SCENE_PRIMARY, task_primary_delete);
    destroy_audio_task(TASK_SCENE_AUDPLAYBACK, task_audplayback_delete);
    destroy_audio_task(TASK_SCENE_DEEPBUFFER, task_deepbuffer_delete);
    destroy_audio_task(TASK_SCENE_VOIP, task_voip_delete);
    destroy_audio_task(TASK_SCENE_CAPTURE_UL1, task_capture_ul1_delete);
    destroy_audio_task(TASK_SCENE_CALL_FINAL, task_call_final_delete);
    destroy_audio_task(TASK_SCENE_KTV, task_ktv_delete);
#ifdef CFG_A2DP_OFFLOAD_SUPPORT
    destroy_audio_task(TASK_SCENE_A2DP, task_a2dp_delete);
    destroy_audio_task(TASK_SCENE_DATAPROVIDER, task_dataprovider_delete);
#endif
#endif

#if defined(CFG_MTK_AUDIODSP_SUPPORT)
    destroy_audio_task(TASK_SCENE_AUDIO_CONTROLLER_HIFI3_A, task_controller_delete);
#elif defined(CFG_MTK_AUDIO_CM4_DMA_SUPPORT)
    destroy_audio_task(TASK_SCENE_AUDIO_CONTROLLER_CM4, task_controller_delete);
#endif
}
#endif /* end of !CFG_DO_ENABLED */


int create_audio_task(const uint8_t task_scene, new_task_t new_task)
{
    AUD_LOG_D("%s(+), task_scene = %d, new_task = %p, Heap free/total = %d/%d\n\n",
              __func__, task_scene, new_task, xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);

    AudioTask *task = NULL;

    if (task_scene >= TASK_SCENE_SIZE) {
        AUD_LOG_W("%s(-), task_scene(%d) error\n", __func__, task_scene);
        return -1;
    }

    if (new_task == NULL) {
        AUD_LOG_E("%s(-), new_task == NULL!! task_scene = %d, return\n",
                  __func__, task_scene);
        return -1;
    }


    /* cannot create twice */
    AUD_ASSERT(g_task_array[task_scene] == NULL);

    /* new task */
    task = new_task();
    if (task == NULL) {
        AUD_LOG_E("%s(-), task == NULL!! task_scene = %d, return\n",
                  __func__, task_scene);
        return -1;
    }

    /* constructor */
    if (task->constructor != NULL) {
        task->constructor(task);
    }
    if (task->create_task_loop != NULL) {
        task->create_task_loop(task);
    }
    AUD_ASSERT(task->scene == task_scene);

#ifdef CFG_DO_ENABLED /* with DO => need write back */
    write_back_data_to_task(task);
#endif

    /* keep task address in factory */
    g_task_array[task_scene] = task;

    AUD_LOG_D("%s(-), task_scene = %d, Heap free/total = %d/%d\n\n", __func__,
              task_scene, xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);
    return 0;
}


int destroy_audio_task(const uint8_t task_scene, delete_task_t delete_task)
{
    AUD_LOG_D("%s(+), task_scene = %d\n\n", __func__, task_scene);

    AudioTask *task = NULL;

    if (task_scene >= TASK_SCENE_SIZE) {
        AUD_LOG_W("%s(-), task_scene(%d) error\n", __func__, task_scene);
        return -1;
    }

    if (delete_task == NULL) {
        AUD_LOG_E("%s(-), delete_task == NULL!! task_scene = %d, return\n",
                  __func__, task_scene);
        return -1;
    }


    /* cannot destroy null */
    AUD_ASSERT(g_task_array[task_scene] != NULL);
    AUD_ASSERT(g_task_array[task_scene]->scene == task_scene);

    /* remove task address in factory */
    task = g_task_array[task_scene];
    g_task_array[task_scene] = NULL;

#ifdef CFG_DO_ENABLED /* with DO => need write back */
    copy_write_back_data_from_task(task);
#endif

    /* destructor */
    if (task->destroy_task_loop != NULL) {
        task->destroy_task_loop(task);
    }
    if (task->destructor != NULL) {
        task->destructor(task);
    }

    /* delete task */
    delete_task(task);

    AUD_LOG_D("%s(-), task_scene = %d\n\n", __func__, task_scene);
    return 0;
}

#if defined(CFG_SMARTPA_USE_CACHE) && defined(CFG_MTK_SPEAKER_PROTECTION_SUPPORT)
SRAM_REGION_FUNCTION
#endif
AudioTask *get_task_by_scene(const uint8_t task_scene)
{
    AudioTask *task = NULL;

    if (task_scene >= TASK_SCENE_SIZE) {
        AUD_LOG_W("%s(), task_scene(%d) error\n", __func__, task_scene);
        return NULL;
    }

    task = g_task_array[task_scene];
    if (task == NULL) {
        AUD_LOG_W("%s(), g_task_array[%d] == NULL\n", __func__, task_scene);
        return NULL;
    }
    AUD_ASSERT(task->scene == task_scene);

    //AUD_LOG_D("%s(), task_scene(%d), task = %p\n", __func__, task_scene, task);
    return task;
}


AudioTask *get_task_by_irq(unsigned int irq_type)
{
    AudioTask *task = NULL;
    int sceneid = 0;

    /* audio type */
    sceneid = adsp_audio_irq_entry();

    if (sceneid < 0) {
        AUD_LOG_W("%s(), sceneid[%d]\n", __func__, sceneid);
        return NULL;
    }

    task = g_task_array[sceneid];
    if (task == NULL) {
        AUD_LOG_W("%s(), g_audio_task_array[%d] == NULL\n", __func__, sceneid);
        return NULL;
    }
    if (task->scene != sceneid) {
        AUD_LOG_W("%s(), sceneid[%d] task->scene[%d]\n", __func__, sceneid, task->scene);
        AUD_ASSERT(task->scene == sceneid);
    }

    // AUD_LOG_D("%s(), task_scene(%d), task = %p\n", __func__, sceneid, task->scene);
    return task;
}


void notify_audio_task_hal_reboot(const uint8_t hal_type)
{
    AudioTask *task = NULL;
    uint8_t task_scene = 0;

    for (task_scene = 0; task_scene < TASK_SCENE_SIZE; task_scene++) {
        task = g_task_array[task_scene];
        if (task != NULL && task->hal_reboot_cbk != NULL) {
            task->hal_reboot_cbk(task, hal_type);
        }
    }
}



