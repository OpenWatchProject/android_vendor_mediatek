/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#include <adsp_ipi_queue.h>


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>


#include <tinysys_config.h>

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>

#include <interrupt.h>
#include <task.h>
#include <semphr.h>

#include <mt_gpt.h>


#ifdef CFG_MTK_AUDIODSP_SUPPORT
#include <adsp_ipi.h>
#else
#include <scp_ipi.h>
#endif

#include <audio_messenger_ipi.h>


#ifdef scp_debug
#undef scp_debug
#endif

/* debug only. might make performace degrade */
//#define ENABLE_SCP_DEBUG_LOG

#ifdef ENABLE_SCP_DEBUG_LOG
#define scp_debug(x...) PRINTF_W(x)
#else
#define scp_debug(x...)
#endif



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define MAX_SCP_MSG_NUM_IN_QUEUE (32)
#define SCP_MSG_BUFFER_SIZE ((SHARE_BUF_SIZE) - 16)



#define IPI_QUEUE_PUSH_SERVICE_TIMEOUT_NS (500000) /* 0.5 ms */


/*
 * =============================================================================
 *                     struct def
 * =============================================================================
 */

struct scp_msg_t {
    uint32_t ipi_id; /* enum ipi_id */
    uint8_t buf[SCP_MSG_BUFFER_SIZE]; /* TODO: use ring buf? */
    uint32_t len;

    uint64_t time_ns;

    /* recv msg callback */
    void (*ipi_handler)(int ipi_id, void *buf, unsigned int len);
};


enum { /* scp_path_t */
    SCP_PATH_A2S = 0, /* AP to SCP */
    SCP_PATH_S2A = 1, /* SCP to AP */
    SCP_NUM_PATH
};


struct scp_msg_queue_t {
    uint32_t scp_path;      /* enum scp_dir_t */

    bool init;

    uint8_t size;
    uint8_t idx_r;
    uint8_t idx_w;

    struct scp_msg_t msg_array[MAX_SCP_MSG_NUM_IN_QUEUE];

    TaskHandle_t task;
    xQueueHandle queue;
    SemaphoreHandle_t lock;

    /* scp_process_msg_from_ap() / scp_send_msg_to_ap() */
    int (*scp_process_msg_func)(struct scp_msg_t *p_scp_msg);
};

/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */

static struct scp_msg_queue_t *g_scp_msg_queue;


/*
 * =============================================================================
 *                     utilities
 * =============================================================================
 */

inline bool scp_check_idx_msg_valid(
    const struct scp_msg_queue_t *msg_queue,
    const uint8_t idx_msg);

inline bool scp_check_queue_empty(
    const struct scp_msg_queue_t *msg_queue);

inline bool scp_check_queue_to_be_full(
    const struct scp_msg_queue_t *msg_queue);

inline uint8_t scp_get_num_messages_in_queue(
    const struct scp_msg_queue_t *msg_queue);


#define IPI_ASSERT(exp) \
    do { \
        if (!(exp)) { \
            PRINTF_E("[AUDIO] ASSERT("#exp") fail!!\n"); \
            configASSERT(0); \
        } \
    } while(0)



/*
 * =============================================================================
 *                     private function declaration
 * =============================================================================
 */


static void scp_dump_msg_in_queue(struct scp_msg_queue_t *msg_queue);

static int scp_push_msg(
    struct scp_msg_queue_t *msg_queue,
    uint32_t ipi_id,
    void *buf,
    uint32_t len,
    void (*ipi_handler)(int ipi_id, void *buf, unsigned int len),
    uint8_t *p_idx_msg,
    uint64_t time_user_ns);

static int scp_pop_msg(struct scp_msg_queue_t *msg_queue);

static int scp_front_msg(
    struct scp_msg_queue_t *msg_queue,
    struct scp_msg_t **pp_scp_msg,
    uint8_t *p_idx_msg);


static void scp_process_msg_thread(void *data);

static int scp_process_msg_from_ap(struct scp_msg_t *p_scp_msg);

static int scp_send_msg_to_ap(struct scp_msg_t *p_scp_msg);



/*
 * =============================================================================
 *                     utilities
 * =============================================================================
 */

#define DUMP_IPC_MSG(description, p_scp_msg) \
    do { \
        struct ipi_msg_t *p_ipi_msg = NULL; \
        if ((void *)description == NULL ||(void *)(p_scp_msg) == NULL) \
            break; \
        if (p_scp_msg->len >= IPI_MSG_HEADER_SIZE) { \
            p_ipi_msg = (struct ipi_msg_t *)p_scp_msg->buf; \
            if (p_ipi_msg->magic == IPI_MSG_MAGIC_NUMBER) \
                DUMP_IPI_MSG(description, p_ipi_msg); \
        } else { \
            pr_info("%s, ipi_id: %u, buf %p, len: %u", \
                    description, \
                    p_scp_msg->ipi_id, \
                    p_scp_msg->buf, \
                    p_scp_msg->len); \
        } \
    } while (0)


inline bool scp_check_idx_msg_valid(
    const struct scp_msg_queue_t *msg_queue,
    const uint8_t idx_msg)
{
    if (msg_queue == NULL) {
        PRINTF_W("%s(), msg_queue == NULL!! return\n", __func__);
        return false;
    }

    return (idx_msg < msg_queue->size) ? true : false;
}


inline bool scp_check_queue_empty(const struct scp_msg_queue_t *msg_queue)
{
    if (msg_queue == NULL) {
        PRINTF_W("%s(), msg_queue == NULL!! return\n", __func__);
        return false;
    }

    return (msg_queue->idx_r == msg_queue->idx_w);
}


inline bool scp_check_queue_to_be_full(const struct scp_msg_queue_t *msg_queue)
{
    uint8_t idx_w_to_be = 0;

    if (msg_queue == NULL) {
        PRINTF_W("%s(), msg_queue == NULL!! return\n", __func__);
        return false;
    }

    idx_w_to_be = msg_queue->idx_w + 1;
    if (idx_w_to_be == msg_queue->size) {
        idx_w_to_be = 0;
    }

    return (idx_w_to_be == msg_queue->idx_r) ? true : false;
}


inline uint8_t scp_get_num_messages_in_queue(
    const struct scp_msg_queue_t *msg_queue)
{
    if (msg_queue == NULL) {
        PRINTF_W("%s(), msg_queue == NULL!! return\n", __func__);
        return 0;
    }

    return (msg_queue->idx_w >= msg_queue->idx_r) ?
           (msg_queue->idx_w - msg_queue->idx_r) :
           ((msg_queue->size - msg_queue->idx_r) + msg_queue->idx_w);
}


static void scp_dump_msg_in_queue(struct scp_msg_queue_t *msg_queue)
{
    struct scp_msg_t *p_scp_msg = NULL;
    uint8_t idx_dump = msg_queue->idx_r;
    char dump_str[64] = {0};

    PRINTF_I("%s(), idx_r: %d, idx_w: %d, queue(%d/%d)\n",
             __func__,
             msg_queue->idx_r,
             msg_queue->idx_w,
             scp_get_num_messages_in_queue(msg_queue),
             msg_queue->size);

    while (idx_dump != msg_queue->idx_w) {
        /* get head msg */
        p_scp_msg = &msg_queue->msg_array[idx_dump];

        snprintf(dump_str, sizeof(dump_str),
                 "#element[%u], user time [%lf]",
                 idx_dump, ((double)p_scp_msg->time_ns) / 1000000000);
        DUMP_IPC_MSG(dump_str, p_scp_msg);

        /* update dump index */
        idx_dump++;
        if (idx_dump == msg_queue->size) {
            idx_dump = 0;
        }
    }
}


static int scp_push_msg(
    struct scp_msg_queue_t *msg_queue,
    uint32_t ipi_id,
    void *buf,
    uint32_t len,
    void (*ipi_handler)(int ipi_id, void *buf, unsigned int len),
    uint8_t *p_idx_msg,
    uint64_t time_user_ns)
{
    struct scp_msg_t *p_scp_msg = NULL;
    uint64_t time_queue_ns = timer_get_ap_timestamp();

    if (time_queue_ns >= (time_user_ns + IPI_QUEUE_PUSH_SERVICE_TIMEOUT_NS)) {
        PRINTF_W("%s(), user [%lf], push queue [%lf] too long!!\n", __func__,
                 ((double)time_user_ns) / 1000000000,
                 ((double)time_queue_ns) / 1000000000);
    }


    if (msg_queue == NULL || buf == NULL || p_idx_msg == NULL) {
        PRINTF_W("%s(), NULL!! msg_queue: %p, buf: %p, p_idx_msg: %p\n",
                 __func__, msg_queue, buf, p_idx_msg);
        return -EFAULT;
    }

    /* check queue full */
    if (scp_check_queue_to_be_full(msg_queue) == true) {
        PRINTF_W("ipi_id: %u, queue overflow, idx_r: %d, idx_w: %d, drop it\n",
                 ipi_id, msg_queue->idx_r, msg_queue->idx_w);
        scp_dump_msg_in_queue(msg_queue);
        /* configASSERT(0); */
        return -EOVERFLOW;
    }

    if (scp_check_idx_msg_valid(msg_queue, msg_queue->idx_w) == false) {
        PRINTF_W("%s(), idx_w %u is invalid!! return\n",
                 __func__, msg_queue->idx_w);
        return -1;
    }


    /* push */
    *p_idx_msg = msg_queue->idx_w;
    msg_queue->idx_w++;
    if (msg_queue->idx_w == msg_queue->size) {
        msg_queue->idx_w = 0;
    }

    /* copy */
    p_scp_msg = &msg_queue->msg_array[*p_idx_msg];
    p_scp_msg->ipi_id = ipi_id;
    memcpy((void *)p_scp_msg->buf, buf, len);
    p_scp_msg->len = len;
    p_scp_msg->ipi_handler = ipi_handler;
    p_scp_msg->time_ns = time_user_ns;


    scp_debug("%s(), scp_path: %u, ipi_id: %u, idx_r: %d, idx_w: %d, queue(%d/%d), *p_idx_msg: %d\n",
              __func__,
              msg_queue->scp_path,
              p_scp_msg->ipi_id,
              msg_queue->idx_r,
              msg_queue->idx_w,
              scp_get_num_messages_in_queue(msg_queue),
              msg_queue->size,
              *p_idx_msg);

    return 0;
}


static int scp_pop_msg(struct scp_msg_queue_t *msg_queue)
{
#ifdef ENABLE_SCP_DEBUG_LOG /* for unused var build warning */
    struct scp_msg_t *p_scp_msg = NULL;
#endif

    if (msg_queue == NULL) {
        PRINTF_W("%s(), NULL!! msg_queue: %p\n", __func__, msg_queue);
        return -EFAULT;
    }

    /* check queue empty */
    if (scp_check_queue_empty(msg_queue) == true) {
        PRINTF_W("%s(), queue is empty, idx_r: %d, idx_w: %d\n",
                 __func__,
                 msg_queue->idx_r,
                 msg_queue->idx_w);
        return -1;
    }

    /* pop */
#ifdef ENABLE_SCP_DEBUG_LOG /* for unused var build warning */
    p_scp_msg = &msg_queue->msg_array[msg_queue->idx_r];
#endif
    msg_queue->idx_r++;
    if (msg_queue->idx_r == msg_queue->size) {
        msg_queue->idx_r = 0;
    }

    scp_debug("%s(), scp_path: %u, ipi_id: %u, idx_r: %d, idx_w: %d, queue(%d/%d)\n",
              __func__,
              msg_queue->scp_path,
              p_scp_msg->ipi_id,
              msg_queue->idx_r,
              msg_queue->idx_w,
              scp_get_num_messages_in_queue(msg_queue),
              msg_queue->size);

    return 0;
}


static int scp_front_msg(
    struct scp_msg_queue_t *msg_queue,
    struct scp_msg_t **pp_scp_msg,
    uint8_t *p_idx_msg)
{
    if (msg_queue == NULL || pp_scp_msg == NULL || p_idx_msg == NULL) {
        PRINTF_W("%s(), NULL!! msg_queue: %p, pp_scp_msg: %p, p_idx_msg: %p\n",
                 __func__, msg_queue, pp_scp_msg, p_idx_msg);
        return -EFAULT;
    }

    *pp_scp_msg = NULL;
    *p_idx_msg = 0xFF;

    /* check queue empty */
    if (scp_check_queue_empty(msg_queue) == true) {
        PRINTF_W("%s(), queue empty, idx_r: %d, idx_w: %d\n",
                 __func__, msg_queue->idx_r, msg_queue->idx_w);
        return -ENOMEM;
    }

    /* front */
    if (scp_check_idx_msg_valid(msg_queue, msg_queue->idx_r) == false) {
        PRINTF_W("%s(), idx_r %u is invalid!! return\n",
                 __func__, msg_queue->idx_r);
        return -1;
    }
    *p_idx_msg = msg_queue->idx_r;
    *pp_scp_msg = &msg_queue->msg_array[*p_idx_msg];

#if 0
    scp_debug("%s(), scp_path: %u, ipi_id: %u, idx_r: %d, idx_w: %d, queue(%d/%d), *p_idx_msg: %d\n",
              __func__,
              msg_queue->scp_path,
              msg_queue->msg_array[*p_idx_msg].ipi_id,
              msg_queue->idx_r,
              msg_queue->idx_w,
              scp_get_num_messages_in_queue(msg_queue),
              msg_queue->size,
              *p_idx_msg);
#endif

    return 0;
}


/*
 * =============================================================================
 *             create/destroy/init/deinit functions
 * =============================================================================
 */

int scp_ipi_queue_init(void)
{
    struct scp_msg_queue_t *msg_queue = NULL;
    struct scp_msg_t *p_scp_msg = NULL;

    uint32_t scp_path = 0;

    char task_name[SCP_NUM_PATH][8] = {"ipi_r", "ipi_s"};

    BaseType_t retval = pdFAIL;

    int i = 0;

    g_scp_msg_queue = (struct scp_msg_queue_t *)kal_pvPortMalloc(
                          sizeof(struct scp_msg_queue_t) * SCP_NUM_PATH);

    for (scp_path = 0; scp_path < SCP_NUM_PATH; scp_path++) {
        msg_queue = &g_scp_msg_queue[scp_path];

        msg_queue->scp_path = scp_path;

        /* check double init */
        if (msg_queue->init) {
            PRINTF_W("%s(), scp_path: %u already init!!\n", __func__, scp_path);
            continue;
        }
        msg_queue->init = true;

        msg_queue->size = MAX_SCP_MSG_NUM_IN_QUEUE;
        msg_queue->idx_r = 0;
        msg_queue->idx_w = 0;

        for (i = 0; i < msg_queue->size; i++) {
            p_scp_msg = &msg_queue->msg_array[i];

            p_scp_msg->ipi_id = 0;
            memset((void *)p_scp_msg->buf, 0, SCP_MSG_BUFFER_SIZE);
            p_scp_msg->len = 0;
            p_scp_msg->ipi_handler = NULL;
        }

        retval = kal_xTaskCreate(
                     scp_process_msg_thread,
                     task_name[scp_path],
                     2048,
                     (void *)msg_queue,
                     configMAX_PRIORITIES - 1,
                     &msg_queue->task);
        IPI_ASSERT(retval == pdPASS);
        IPI_ASSERT(msg_queue->task);

        msg_queue->queue = xQueueCreate(msg_queue->size, sizeof(uint8_t));
        IPI_ASSERT(msg_queue->queue != NULL);

        msg_queue->lock = xSemaphoreCreateMutex();
        IPI_ASSERT(msg_queue->lock != NULL);

        if (scp_path == SCP_PATH_A2S) {
            msg_queue->scp_process_msg_func = scp_process_msg_from_ap;
        } else if (scp_path == SCP_PATH_S2A) {
            msg_queue->scp_process_msg_func = scp_send_msg_to_ap;
        } else {
            configASSERT(0);
        }
    }

    return 0;
}


int scp_flush_msg_queue()
{
    return 0;
}



/*
 * =============================================================================
 *                     public functions
 * =============================================================================
 */

int scp_send_msg_to_queue(
    uint32_t ipi_id, /* enum ipi_id */
    void *buf,
    uint32_t len)
{
    struct scp_msg_queue_t *msg_queue = NULL;
    uint8_t idx_msg = 0;

    BaseType_t ret_send_queue = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    int retval = 0;
    uint64_t time_user_ns = timer_get_ap_timestamp();

    scp_debug("%s(+), ipi_id: %u, buf: %p, len: %u\n",
              __func__, ipi_id, buf, len);

    if (ipi_id >= NR_IPI) {
        PRINTF_W("%s(), ipi_id %u error!!\n", __func__, ipi_id);
        return -EOVERFLOW;
    }
    if (buf == NULL || len > SCP_MSG_BUFFER_SIZE) {
        PRINTF_W("%s(), buf: %p, len: %u!! return\n",
                 __func__, buf, len);
        return -EFAULT;
    }


    msg_queue = &g_scp_msg_queue[SCP_PATH_S2A];
    if (msg_queue->init == false) {
        PRINTF_W("%s(), not init!! return\n", __func__);
        return -1;
    }

    if (is_in_isr()) {
        if (xSemaphoreTakeFromISR(msg_queue->lock, NULL) != pdTRUE) {
            configASSERT(0);
            return -1;
        }
    } else {
        kal_taskENTER_CRITICAL();
    }

    retval = scp_push_msg(
                 msg_queue,
                 ipi_id,
                 buf,
                 len,
                 NULL, /* only for recv msg from scp */
                 &idx_msg,
                 time_user_ns);
    if (retval != 0) {
        PRINTF_W("%s(), push fail!!\n", __func__);
        if (is_in_isr()) {
            if (xSemaphoreGiveFromISR(msg_queue->lock, NULL) != pdTRUE) {
                configASSERT(0);
            }
        } else {
            kal_taskEXIT_CRITICAL();
        }
        return retval;
    }


    /* notify queue */
    if (is_in_isr()) {
        ret_send_queue = xQueueSendToBackFromISR(
                             msg_queue->queue,
                             &idx_msg,
                             &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        if (xSemaphoreGiveFromISR(msg_queue->lock, NULL) != pdTRUE) {
            configASSERT(0);
        }
    } else {
        ret_send_queue = xQueueSendToBack(
                             msg_queue->queue,
                             &idx_msg,
                             portMAX_DELAY);
        kal_taskEXIT_CRITICAL();
    }
    IPI_ASSERT(ret_send_queue == pdTRUE);


    scp_debug("%s(-), ipi_id: %u, buf: %p, len: %u\n",
              __func__, ipi_id, buf, len);

    return retval;
}


int scp_dispatch_ipi_hanlder_to_queue(
    uint32_t ipi_id, /* enum ipi_id */
    void *buf,
    uint32_t len,
    void (*ipi_handler)(int id, void *data, unsigned int len))
{
    struct scp_msg_queue_t *msg_queue = NULL;
    uint8_t idx_msg = 0;


    BaseType_t ret_send_queue = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    int retval = 0;
    uint64_t time_user_ns = timer_get_ap_timestamp();

    scp_debug("%s(+), ipi_id: %u, buf: %p, len: %u, ipi_handler: %p\n",
              __func__, ipi_id, buf, len, ipi_handler);

    if (ipi_id >= NR_IPI) {
        PRINTF_W("%s(), ipi_id %u error!!\n", __func__, ipi_id);
        return -EOVERFLOW;
    }
    if (buf == NULL || len > SCP_MSG_BUFFER_SIZE) {
        PRINTF_W("%s(), buf: %p, len: %u!! return\n",
                 __func__, buf, len);
        return -EFAULT;
    }
    if (ipi_handler == NULL) {
        PRINTF_W("%s(), NULL!! ipi_handler: %p\n",
                 __func__, ipi_handler);
        return -EFAULT;
    }


    msg_queue = &g_scp_msg_queue[SCP_PATH_A2S];
    if (msg_queue->init == false) {
        PRINTF_W("%s(), not init!! return\n", __func__);
        return -1;
    }

    retval = scp_push_msg(
                 msg_queue,
                 ipi_id,
                 buf,
                 len,
                 ipi_handler,
                 &idx_msg,
                 time_user_ns);
    if (retval != 0) {
        PRINTF_W("%s(), push fail!!\n", __func__);
        return retval;
    }


    /* notify queue */
    if (is_in_isr()) {
        ret_send_queue = xQueueSendToBackFromISR(
                             msg_queue->queue,
                             &idx_msg,
                             &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        ret_send_queue = xQueueSendToBack(
                             msg_queue->queue,
                             &idx_msg,
                             portMAX_DELAY);
    }
    IPI_ASSERT(ret_send_queue == pdTRUE);


    scp_debug("%s(-), ipi_id: %u, buf: %p, len: %u, ipi_handler: %p\n",
              __func__, ipi_id, buf, len, ipi_handler);

    return 0;
}



/*
 * =============================================================================
 *                     private implementation
 * =============================================================================
 */

static void scp_process_msg_thread(void *data)
{
    struct scp_msg_queue_t *msg_queue = (struct scp_msg_queue_t *)data;
    struct scp_msg_t *p_scp_msg = NULL;

    uint8_t idx_msg_from_queue = 0;
    uint8_t idx_msg = 0;

    int retval = 0;

    if (msg_queue == NULL) {
        PRINTF_W("%s(), msg_queue == NULL!! return\n", __func__);
        configASSERT(0);
        return;
    }

    if (msg_queue->queue == NULL) {
        PRINTF_W("%s(), queue == NULL!! return\n", __func__);
        configASSERT(0);
        return;
    }

    while (1) {
        xQueueReceive(msg_queue->queue, &idx_msg_from_queue, portMAX_DELAY);

        retval = scp_front_msg(msg_queue, &p_scp_msg, &idx_msg);
        if (retval != 0 || idx_msg != idx_msg_from_queue) {
            PRINTF_E("front fail!! idx_msg %d, idx_msg_from_queue %d, retval %d\n",
                     idx_msg, idx_msg_from_queue, retval);
            configASSERT(0);
            continue;
        }

        retval = msg_queue->scp_process_msg_func(p_scp_msg);
        if (retval != 0) {
            PRINTF_W("scp_process_msg_func fail!! retval %d\n",
                     retval);
        }

        scp_pop_msg(msg_queue);
    }
}


static int scp_process_msg_from_ap(struct scp_msg_t *p_scp_msg)
{
    if (p_scp_msg == NULL) {
        PRINTF_W("%s(), NULL!! p_scp_msg: %p\n",
                 __func__, p_scp_msg);
        return -EFAULT;
    }

    if (p_scp_msg->ipi_handler == NULL) {
        PRINTF_W("%s(), NULL!! p_scp_msg->ipi_handler: %p\n",
                 __func__, p_scp_msg->ipi_handler);
        return -EFAULT;
    }

    if (p_scp_msg->len == 0) {
        PRINTF_W("%s(), p_scp_msg->len: %u\n",
                 __func__, p_scp_msg->len);
        return -EFAULT;
    }

    /* TODO: add time info here */
    p_scp_msg->ipi_handler(
        p_scp_msg->ipi_id,
        p_scp_msg->buf,
        p_scp_msg->len);

    return 0;
}


static int scp_send_msg_to_ap(struct scp_msg_t *p_scp_msg)
{
    ipi_status send_status = ERROR;
    struct ipi_msg_t *p_ipi_msg = NULL;

    uint32_t try_cnt = 0;
    const uint32_t k_max_try_cnt = 2000; /* 2 sec for busy */
    const uint32_t k_restart_sleep_ms = 1;

    bool retry_flag = false;
    char dump_str[64] = {0};

    if (p_scp_msg == NULL) {
        PRINTF_W("%s(), NULL!! p_scp_msg: %p\n",
                 __func__, p_scp_msg);
        return -EFAULT;
    }

    for (try_cnt = 0; try_cnt < k_max_try_cnt; try_cnt++) {
#ifdef CFG_MTK_AUDIODSP_SUPPORT
        send_status = adsp_ipi_send_ipc(
                          p_scp_msg->ipi_id,
                          p_scp_msg->buf,
                          p_scp_msg->len,
                          1, /* wait until sent ot timeout */
                          IPI_ADSP2AP);
#else
        send_status = scp_ipi_send_ipc(
                          p_scp_msg->ipi_id,
                          p_scp_msg->buf,
                          p_scp_msg->len,
                          0, /* avoid busy waiting */
                          IPI_SCP2AP);
#endif
        if (send_status == DONE) {
            if (p_scp_msg->ipi_id == IPI_AUDIO) {
                p_ipi_msg = (struct ipi_msg_t *)p_scp_msg->buf;
                if (check_print_msg_info(p_ipi_msg) && !retry_flag) {
                    snprintf(dump_str, sizeof(dump_str),
                             "pass. user time [%lf]",
                             ((double)p_scp_msg->time_ns) / 1000000000);
                    DUMP_IPI_MSG(dump_str, p_ipi_msg);
                }
            }
            break;
        }
        if (send_status == ERROR) { /* fail */
            snprintf(dump_str, sizeof(dump_str),
                     "dsp error!! user time [%lf]",
                     ((double)p_scp_msg->time_ns) / 1000000000);
            DUMP_IPC_MSG(dump_str, p_scp_msg);
            break;
        }
        if (send_status == BUSY) { /* busy */
            if (try_cnt == 0) { /* only dump log once */
                snprintf(dump_str, sizeof(dump_str),
                         "IPC busy, retry!! user time [%lf]",
                         ((double)p_scp_msg->time_ns) / 1000000000);
                DUMP_IPC_MSG(dump_str, p_scp_msg);
            }
            retry_flag = true;
            vTaskDelay(pdMS_TO_TICKS(k_restart_sleep_ms));
            continue;
        }
    }

    if (retry_flag == true) {
        if (send_status == DONE) {
            snprintf(dump_str, sizeof(dump_str), "retry %u pass", try_cnt);
        } else {
            snprintf(dump_str, sizeof(dump_str),
                     "retry %u err, ret %d", try_cnt, send_status);
        }
        DUMP_IPC_MSG(dump_str, p_scp_msg);
    }

    return (send_status == DONE) ? 0 : -1;
}


