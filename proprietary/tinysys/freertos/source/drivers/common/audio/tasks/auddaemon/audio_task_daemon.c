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

#include "audio_task_daemon.h"

#include <stdarg.h>
#include <interrupt.h>
#include <wrapped_errors.h>
#include <dma.h>

#ifdef CFG_AUDIO_SUPPORT
#include <adsp_ipi.h>
#include <audio_ipi_dma.h>
#else
#include <scp_ipi.h>
#endif

#ifdef CFG_VCORE_DVFS_SUPPORT
#include <feature_manager.h>
#include <dvfs.h>
#include <wakelock.h>
#endif

#include "audio_messenger_ipi.h"
#include "audio_task_factory.h"
#include "arsi_api.h"

/* audio utility relate*/
#include <audio_ringbuf.h>
#include <audio_task_utility.h>
#include <audio_task_attr.h>
#include <audio_dsp_hw_hal.h>

#define MAX_MSG_QUEUE_SIZE (10)

#define LOCAL_TASK_STACK_SIZE (256)
#define LOCAL_TASK_NAME "aud_daemon"
#define LOCAL_TASK_PRIORITY (2)

/*
 *  static variables.
 */

/*
 *  The task like HISR.
 *  Please do data moving inside the task.
 */

static void task_auddaemon_task_loop(void *pvParameters)
{
    AudioTask *this = (AudioTask *)pvParameters;
    uint8_t local_queue_idx = 0xFF;
    ipi_msg_t *ipi_msg;
    struct Audio_Task_Msg_t uAudioTaskMsg;
    AUD_LOG_D("%s()  start \n", __func__);

    while (1) {
        xQueueReceive(this->msg_idx_queue, &local_queue_idx, portMAX_DELAY);
        ipi_msg = &this->msg_array[local_queue_idx];

        switch (uAudioTaskMsg.msg_id) {
            default:
                break;
        }

        /* clean msg */
        memset(&this->msg_array[local_queue_idx], 0, sizeof(ipi_msg_t));
        this->num_queue_element--;
    }
}


// ============================================================================

static void task_daemon_constructor(struct AudioTask *this)
{
    AUD_ASSERT(this != NULL);

    /* assign initial value for class members & alloc private memory here */
    this->scene = TASK_SCENE_AUD_DAEMON;
    this->state = AUDIO_TASK_IDLE;

    /* queue */
    this->queue_idx = 0;
    this->num_queue_element = 0;

    this->msg_array = (ipi_msg_t *)kal_pvPortMalloc(
                          MAX_MSG_QUEUE_SIZE * sizeof(ipi_msg_t));

    AUD_ASSERT(this->msg_array != NULL);
    memset(this->msg_array, 0, MAX_MSG_QUEUE_SIZE * sizeof(ipi_msg_t));

    this->msg_idx_queue = xQueueCreate(MAX_MSG_QUEUE_SIZE,
                                       sizeof(uint8_t));
    AUD_ASSERT(this->msg_idx_queue != NULL);

}

static void task_daemon_destructor(struct AudioTask *this)
{
    AUD_LOG_D("%s(), task_scene = %d", __func__, this->scene);

    AUD_ASSERT(this != NULL);
    if (this->msg_array != NULL) {
        kal_vPortFree(this->msg_array);
    }

}

static void task_daemon_create_task_loop(struct AudioTask *this)
{
    /* Note: you can also bypass this function,
             and do kal_xTaskCreate until you really need it.
             Ex: create task after you do get the enable message. */

    BaseType_t xReturn = pdFAIL;

    xReturn = kal_xTaskCreate(
                  task_auddaemon_task_loop,
                  LOCAL_TASK_NAME,
                  LOCAL_TASK_STACK_SIZE,
                  (void *)this,
                  LOCAL_TASK_PRIORITY,
                  &this->freertos_task);

    AUD_ASSERT(xReturn == pdPASS);
}

static status_t task_daemon_recv_message(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg)
{

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uint8_t queue_idx = 0, i = 0;
    this->num_queue_element++;
    queue_idx = get_queue_idx(this, MAX_MSG_QUEUE_SIZE);

    /* check message */
    if (this->msg_array[queue_idx].magic != 0) {
        AUD_LOG_E("queue_idx = %d\n", queue_idx);
        for (i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
            AUD_LOG_E("[%d] id = 0x%x\n", i, this->msg_array[i].msg_id);
        }
        AUD_ASSERT(this->msg_array[queue_idx].magic == 0); /* item is clean */
    }

    /* copy to array */
    memcpy(&this->msg_array[queue_idx], ipi_msg, sizeof(ipi_msg_t));

    if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
                                &xHigherPriorityTaskWoken) != pdTRUE) {
        return UNKNOWN_ERROR;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return NO_ERROR;
}

/* global for debug*/
uint32_t IRQSource;
static void task_daemon_irq_hanlder(
    struct AudioTask *this,
    uint32_t irq_type)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (irq_type == AUDIO_IRQn) {
        AUD_LOG_D("%s()", __func__);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


static void task_daemon_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type) {
    AUD_LOG_D("%s(), scene %d\n", __func__, this->scene);
}


AudioTask *task_daemon_new(void)
{
    /* alloc object here */
    AudioTask *task = (AudioTask *)kal_pvPortMalloc(sizeof(AudioTask));
    if (task == NULL) {
        AUD_LOG_E("%s(), kal_pvPortMalloc fail!!\n", __func__);
        return NULL;
    }
    AUD_LOG_D("%s()", __func__);

    /* only assign methods, but not class members here */
    task->constructor       = task_daemon_constructor;
    task->destructor        = task_daemon_destructor;

    task->create_task_loop  = task_daemon_create_task_loop;

    task->recv_message      = task_daemon_recv_message;

    task->irq_hanlder       = task_daemon_irq_hanlder;

    task->hal_reboot_cbk    = task_daemon_hal_reboot_cbk;

    return task;
}

void task_daemon_delete(AudioTask *task)
{
    if (task == NULL) {
        AUD_LOG_E("%s(), task is NULL!!\n", __func__);
        return;
    }

    vPortFree(task);

    AUD_LOG_V("%s(-)\n", __func__);
}


