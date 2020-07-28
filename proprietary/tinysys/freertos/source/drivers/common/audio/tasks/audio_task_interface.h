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

/* must implemented in each audio_task_xxx.c */

#ifndef AUDIO_TASK_INTERFACE_H
#define AUDIO_TASK_INTERFACE_H

#include <wrapped_errors.h>

#include <audio_type.h>
#include <arsi_type.h>
#include <audio_task.h>


struct ipi_msg_t;

typedef enum {
    AUDIO_TASK_IDLE,
    AUDIO_TASK_OPEN,
    AUDIO_TASK_INIT,
    AUDIO_TASK_PREPARED,
    AUDIO_TASK_WORKING,
    AUDIO_TASK_DEINIT
} task_state_t;


/* the definition for the AudioTask class */
typedef struct AudioTask {
    /* attributes */
    uint8_t  scene;
    uint32_t state;

    TaskHandle_t freertos_task;

    uint8_t queue_idx;
    struct ipi_msg_t *msg_array;
    uint8_t num_queue_element;
    xQueueHandle msg_idx_queue;

    /* constructor/destructor */
    void (*constructor)(struct AudioTask *this);
    void (*destructor)(struct AudioTask *this);

    /* create RTOS task */
    void (*create_task_loop)(struct AudioTask *this);
    void (*destroy_task_loop)(struct AudioTask *this);

    /* receive ipi message */
    status_t (*recv_message)(
        struct AudioTask *this,
        struct ipi_msg_t *ipi_msg);

    /* irq handler */
    void (*irq_hanlder)(struct AudioTask *this, uint32_t irq_type);

    /* write back */
    uint32_t (*get_write_back_size)(struct AudioTask *this);
    void (*copy_write_back_buffer)(struct AudioTask *this, data_buf_t *buf);
    void (*do_write_back)(struct AudioTask *this, data_buf_t *buf);

    /* recovery */
    void (*hal_reboot_cbk)(struct AudioTask *this, const uint8_t hal_type);

    void* task_priv;

} AudioTask;

#endif // end of AUDIO_TASK_INTERFACE_H

