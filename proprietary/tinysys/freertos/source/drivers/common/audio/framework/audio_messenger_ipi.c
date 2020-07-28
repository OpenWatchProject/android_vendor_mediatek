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

#include "audio_messenger_ipi.h"

#include <audio_task_interface.h>
#include <audio_task_factory.h>

#ifdef CFG_MTK_AUDIODSP_SUPPORT
#include <adsp_ipi.h>
#else
#include <scp_ipi.h>
#endif

#if defined(CFG_MTK_AUDIODSP_SUPPORT) || defined(CFG_MTK_AUDIO_CM4_DMA_SUPPORT)
#include <audio_ipi_dma.h>
#endif

#include <audio_log_hal.h>
#include <audio_memory_control.h>

#ifdef CFG_MTK_AUDIODSP_SUPPORT
#include <audio_lock.h>
#endif


#ifdef CFG_MTK_AUDIODSP_SUPPORT
#include <audio_task_attr.h>
#endif



/*==============================================================================
 *                     LOG
 *============================================================================*/

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "[IPI]"


/*==============================================================================
 *                     private global members
 *============================================================================*/

#ifdef CFG_MTK_AUDIODSP_SUPPORT
static struct alock_t *g_task_dma_lock[TASK_SCENE_SIZE];
#endif


/*==============================================================================
 *                     private functions - declaration
 *============================================================================*/

/* msg related */
inline uint16_t get_message_buf_size(const ipi_msg_t *p_ipi_msg);

static int check_msg_format(const ipi_msg_t *p_ipi_msg, unsigned int len);

static void audio_ipi_msg_dispatcher(int id, void *data, unsigned int len);


/*==============================================================================
 *                     private functions - implementation
 *============================================================================*/

uint16_t get_message_buf_size(const ipi_msg_t *p_ipi_msg)
{
    if (p_ipi_msg->data_type == AUDIO_IPI_MSG_ONLY) {
        return IPI_MSG_HEADER_SIZE;
    } else if (p_ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
        return (IPI_MSG_HEADER_SIZE + p_ipi_msg->payload_size);
    } else if (p_ipi_msg->data_type == AUDIO_IPI_DMA) {
        return (IPI_MSG_HEADER_SIZE + IPI_MSG_DMA_INFO_SIZE);
    } else {
        return 0;
    }
}


static int check_msg_format(const ipi_msg_t *p_ipi_msg, unsigned int len)
{
    if (p_ipi_msg->magic != IPI_MSG_MAGIC_NUMBER) {
        AUD_LOG_W("%s(), magic 0x%x error!!",
                  __func__, p_ipi_msg->magic);
        return -1;
    }

    if (p_ipi_msg->task_scene >= TASK_SCENE_SIZE) {
        AUD_LOG_W("%s(), task_scene %d error!!",
                  __func__, p_ipi_msg->task_scene);
        return -1;
    }

    if (p_ipi_msg->source_layer >= AUDIO_IPI_LAYER_FROM_SIZE) {
        AUD_LOG_W("%s(), source_layer %d error!!",
                  __func__, p_ipi_msg->source_layer);
        return -1;
    }

    if (p_ipi_msg->target_layer >= AUDIO_IPI_LAYER_TO_SIZE) {
        AUD_LOG_W("%s(), target_layer %d error!!",
                  __func__, p_ipi_msg->target_layer);
        return -1;
    }

    if (p_ipi_msg->data_type >= AUDIO_IPI_TYPE_SIZE) {
        AUD_LOG_W("%s(), data_type %d error!!",
                  __func__, p_ipi_msg->data_type);
        return -1;
    }

    if (p_ipi_msg->ack_type > AUDIO_IPI_MSG_DIRECT_SEND &&
        p_ipi_msg->ack_type != AUDIO_IPI_MSG_CANCELED) {
        AUD_LOG_W("%s(), ack_type %d error!!",
                  __func__, p_ipi_msg->ack_type);
        return -1;
    }

    if (get_message_buf_size(p_ipi_msg) != len) {
        AUD_LOG_W("%s(), len 0x%x error!!", __func__, len);
        return -1;
    }

    if (p_ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
        if (p_ipi_msg->payload_size == 0 ||
            p_ipi_msg->payload_size > MAX_IPI_MSG_PAYLOAD_SIZE) {
            AUD_LOG_W("%s(), payload_size %u error!!",
                      __func__, (unsigned int) p_ipi_msg->payload_size);
            return -1;
        }
    }

    if (p_ipi_msg->data_type == AUDIO_IPI_DMA) {
        if (p_ipi_msg->dma_info.data_size == 0) {
            AUD_LOG_W("%s(), dma data_size %u error!!",
                      __func__, (unsigned int) p_ipi_msg->dma_info.data_size);
            return -1;
        }
    }

    return 0;
}


static void audio_ipi_msg_dispatcher(int id, void *data, unsigned int len)
{
    ipi_msg_t *p_ipi_msg = NULL;
    AudioTask *task    = NULL;

    AUD_LOG_V("%s(), data = %p, len = %u", __func__, data, len);

    if (data == NULL) {
        AUD_LOG_W("%s(), drop msg due to data = NULL", __func__);
        return;
    }
    if (len < IPI_MSG_HEADER_SIZE || len > MAX_IPI_MSG_BUF_SIZE) {
        AUD_LOG_W("%s(), drop msg due to len(%u) error!!", __func__, len);
        return;
    }

    p_ipi_msg = (ipi_msg_t *)data;
    if (check_msg_format(p_ipi_msg, len) != 0) {
        AUD_LOG_W("%s(), drop msg due to ipi fmt err", __func__);
        return;
    }

    //DUMP_IPI_MSG("get msg", p_ipi_msg);

    task = get_task_by_scene(p_ipi_msg->task_scene);
    if (task == NULL) {
        AUD_LOG_W("%s(), task = NULL, drop msg. task_scene = %d. msg_id = 0x%x",
                  __func__, p_ipi_msg->task_scene, p_ipi_msg->msg_id);
        audio_send_ipi_msg_ack_back(p_ipi_msg);
        return;
    }

    task->recv_message(task, p_ipi_msg);
}


/*==============================================================================
 *                     public functions - implementation
 *============================================================================*/

void audio_messenger_ipi_init(void)
{
    ipi_status retval = ERROR;

#ifdef CFG_MTK_AUDIODSP_SUPPORT
    uint8_t scene = 0;

    for (scene = 0; scene < TASK_SCENE_SIZE; scene++) {
        NEW_ALOCK(g_task_dma_lock[scene]);
    }
    AUD_LOG_D("+%s() adsp, Heap free = %d", __func__, xPortGetFreeHeapSize());
    retval = adsp_ipi_registration(IPI_AUDIO, audio_ipi_msg_dispatcher, "audio");
    adsp_ipi_wakeup_ap_registration(IPI_AUDIO);
#else
    AUD_LOG_D("+%s() scp, Heap free = %d", __func__, xPortGetFreeHeapSize());
    retval = scp_ipi_registration(IPI_AUDIO, audio_ipi_msg_dispatcher, "audio");
    scp_ipi_wakeup_ap_registration(IPI_AUDIO);
#endif
    if (retval != DONE) {
        AUD_LOG_E("%s(), scp_ipi_registration fail!!", __func__);
    }
    AUD_LOG_D("-%s(), Heap free = %d", __func__, xPortGetFreeHeapSize());
}


bool check_print_msg_info(const struct ipi_msg_t *p_ipi_msg)
{
    if (p_ipi_msg == NULL) {
        return false;
    }

#ifdef CFG_MTK_AUDIODSP_SUPPORT
    if (p_ipi_msg->task_scene == TASK_SCENE_PRIMARY &&
        p_ipi_msg->msg_id == AUDIO_DSP_TASK_DLCOPY) {
        return false;
    }

    if (p_ipi_msg->task_scene == TASK_SCENE_DEEPBUFFER &&
        p_ipi_msg->msg_id == AUDIO_DSP_TASK_DLCOPY) {
        return false;
    }

    if (p_ipi_msg->task_scene == TASK_SCENE_VOIP &&
        p_ipi_msg->msg_id == AUDIO_DSP_TASK_DLCOPY) {
        return false;
    }

    if (p_ipi_msg->task_scene == TASK_SCENE_CAPTURE_UL1 &&
        p_ipi_msg->msg_id == AUDIO_DSP_TASK_ULCOPY) {
        return false;
    }
#endif

    if (p_ipi_msg->ack_type == AUDIO_IPI_MSG_NEED_ACK ||
        p_ipi_msg->ack_type == AUDIO_IPI_MSG_ACK_BACK) {
        return true;
    }
    return false;
}


int audio_send_ipi_msg(
    struct ipi_msg_t *p_ipi_msg,
    uint8_t task_scene, /* task_scene_t */
    uint8_t target_layer, /* audio_ipi_msg_target_layer_t */
    uint8_t data_type, /* audio_ipi_msg_data_t */
    uint8_t ack_type, /* audio_ipi_msg_ack_t */
    uint16_t msg_id,
    uint32_t param1, /* data_size for payload & dma */
    uint32_t param2,
    void    *data_buffer) /* buffer for payload & dma */
{
#if defined(CFG_MTK_AUDIODSP_SUPPORT) || defined(CFG_MTK_AUDIO_CM4_DMA_SUPPORT)
    struct ipi_msg_dma_info_t *dma_info = NULL;
#endif

    uint32_t ipi_msg_len = 0;
    int ret = 0;

    if (p_ipi_msg == NULL) {
        AUD_LOG_E("%s(), p_ipi_msg = NULL, return", __func__);
        return -1;
    }

    memset(p_ipi_msg, 0, MAX_IPI_MSG_BUF_SIZE);

    p_ipi_msg->magic      = IPI_MSG_MAGIC_NUMBER;
    p_ipi_msg->task_scene = task_scene;
    p_ipi_msg->source_layer = AUDIO_IPI_LAYER_FROM_DSP;
    p_ipi_msg->target_layer = target_layer;
    p_ipi_msg->data_type  = data_type;
    p_ipi_msg->ack_type   = ack_type;
    p_ipi_msg->msg_id     = msg_id;
    p_ipi_msg->param1     = param1;
    p_ipi_msg->param2     = param2;

    if (p_ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
        if (data_buffer == NULL) {
            AUD_LOG_W("payload data_buffer NULL, return");
            return -1;
        }
        if (p_ipi_msg->payload_size > MAX_IPI_MSG_PAYLOAD_SIZE) {
            AUD_LOG_W("payload_size %u error!!",
                      (unsigned int) p_ipi_msg->payload_size);
            return -1;
        }
        memcpy(p_ipi_msg->payload, data_buffer, p_ipi_msg->payload_size);
    } else if (p_ipi_msg->data_type == AUDIO_IPI_DMA) {
        if (data_buffer == NULL) {
            AUD_LOG_W("dma data_buffer NULL, return");
            return -1;
        }

#ifdef CFG_MTK_AUDIODSP_SUPPORT
        LOCK_ALOCK(g_task_dma_lock[task_scene]);
#endif
        p_ipi_msg->dma_addr = (char *)data_buffer;
#if defined(CFG_MTK_AUDIODSP_SUPPORT) || defined(CFG_MTK_AUDIO_CM4_DMA_SUPPORT)
        if (param1 > 1) {
            dma_info = &p_ipi_msg->dma_info;
            dma_info->data_size = param1;

            ret = audio_ipi_dma_write_region(
                      p_ipi_msg->task_scene,
                      data_buffer,
                      dma_info->data_size,
                      &dma_info->rw_idx);

            //DUMP_IPI_MSG("dma dsp -> kernel", p_ipi_msg);
        }
#endif
    }

    if (ret != 0) {
        DUMP_IPI_MSG("dma fail!!", p_ipi_msg);
        goto send_ipi_done;
    }

    ipi_msg_len = get_message_buf_size(p_ipi_msg);
    if (check_msg_format(p_ipi_msg, ipi_msg_len) != 0) {
        AUD_LOG_W("%s(), drop msg due to ipi fmt err", __func__);
        ret = -1;
        goto send_ipi_done;
    }

    ret = audio_send_ipi_msg_to_kernel(p_ipi_msg);

send_ipi_done:
#ifdef CFG_MTK_AUDIODSP_SUPPORT
    if (p_ipi_msg->data_type == AUDIO_IPI_DMA) {
        UNLOCK_ALOCK(g_task_dma_lock[task_scene]);
    }
#endif

    return ret;
}

static ipi_msg_t last_ipi_msg;
int audio_send_ipi_msg_to_kernel(const ipi_msg_t *p_ipi_msg)
{
    ipi_status send_status = ERROR;
    ipi_msg_t *p_last_ipi_msg = NULL;

    const int k_max_try_count = 10000;
    int try_count = 0;

    /* error handling */
    if (p_ipi_msg == NULL) {
        AUD_LOG_E("%s(), p_ipi_msg = NULL, return", __func__);
        return -1;
    }

    for (try_count = 0; try_count < k_max_try_count; try_count++) {
#ifdef CFG_MTK_AUDIODSP_SUPPORT
        //DUMP_IPI_MSG("adsp", p_ipi_msg);
        send_status = adsp_ipi_send(
                          IPI_AUDIO,
                          (void *)p_ipi_msg,
                          get_message_buf_size(p_ipi_msg),
                          0, /* default don't wait */
                          IPI_ADSP2AP);
#else
        //DUMP_IPI_MSG("cm4", p_ipi_msg);
        send_status = scp_ipi_send(
                          IPI_AUDIO,
                          (void *)p_ipi_msg,
                          get_message_buf_size(p_ipi_msg),
                          0, /* default don't wait */
                          IPI_SCP2AP);
#endif
        if (send_status == DONE) {
            break;
        } else {
            AUD_LOG_V("%s(), #%d scp_ipi_send error %d",
                      __func__, try_count, send_status);
        }
    }

    if (send_status != DONE) {
        AUD_LOG_E("%s(), scp_ipi_send error %d", __func__, send_status);
        DUMP_IPI_MSG("fail", p_ipi_msg);
        p_last_ipi_msg = &last_ipi_msg;
        DUMP_IPI_MSG("fail", p_last_ipi_msg);
    } else {
        //DUMP_IPI_MSG("pass", p_ipi_msg);
        //save for last msg
        memcpy((void *)&last_ipi_msg, (void *)p_ipi_msg, sizeof(ipi_msg_t));
    }

    return (send_status == DONE) ? 0 : -1;
}


int audio_send_ipi_msg_ack_back(ipi_msg_t *p_ipi_msg)
{
    if (p_ipi_msg->ack_type == AUDIO_IPI_MSG_NEED_ACK) {

        int ret = 0;
        AUD_LOG_V("%s(),task: %d, id: 0x%x, ack: %d, p1: 0x%x, p2: 0x%x ",
                  __func__,
                  p_ipi_msg->task_scene,
                  p_ipi_msg->msg_id,
                  p_ipi_msg->ack_type,
                  p_ipi_msg->param1,
                  p_ipi_msg->param2);
        if (p_ipi_msg->data_type == AUDIO_IPI_DMA) {
            AUD_LOG_V("%s(),data_type: %d, wb_dram(data_size: %p, memory_size: %u, data_size: %u)",
                      __func__,
                      p_ipi_msg->data_type,
                      p_ipi_msg->dma_info.wb_dram.addr,
                      p_ipi_msg->dma_info.wb_dram.memory_size,
                      p_ipi_msg->dma_info.wb_dram.data_size);
        }
        p_ipi_msg->ack_type = AUDIO_IPI_MSG_ACK_BACK;
        ret = audio_send_ipi_msg_to_kernel(p_ipi_msg);
        // send fail , need to retry
        if (ret != 0) {
            p_ipi_msg->ack_type = AUDIO_IPI_MSG_NEED_ACK;
        }
        return ret;
    } else {
        return 0;
    }
}

int packing_ipimsg(
    ipi_msg_t *p_ipi_msg,
    uint8_t task_scene, /* task_scene_t */
    uint8_t target_layer, /* audio_ipi_msg_target_layer_t */
    uint8_t data_type, /* audio_ipi_msg_data_t */
    uint8_t ack_type, /* audio_ipi_msg_ack_t */
    uint16_t msg_id,
    uint32_t param1,
    uint32_t param2,
    void    *data_buffer)
{

    memset(p_ipi_msg, 0, MAX_IPI_MSG_BUF_SIZE);

    p_ipi_msg->magic        = IPI_MSG_MAGIC_NUMBER;
    p_ipi_msg->task_scene   = task_scene;
    p_ipi_msg->target_layer = target_layer;
    p_ipi_msg->data_type    = data_type;
    p_ipi_msg->ack_type     = ack_type;
    p_ipi_msg->msg_id       = msg_id;
    p_ipi_msg->param1       = param1;
    p_ipi_msg->param2       = param2;

    if (p_ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
        if (data_buffer == NULL) {
            AUD_LOG_W("payload data_buffer NULL, return");
            return -1;
        }
        if (p_ipi_msg->payload_size > MAX_IPI_MSG_PAYLOAD_SIZE) {
            AUD_LOG_W("payload_size %u error!!",
                      (unsigned int) p_ipi_msg->payload_size);
            return -1;
        }
        memcpy(p_ipi_msg->payload, data_buffer, p_ipi_msg->param1);
    } else if (p_ipi_msg->data_type == AUDIO_IPI_DMA) {
        if (data_buffer == NULL) {
            AUD_LOG_W("dma data_buffer NULL, return");
            return -1;
        }
        p_ipi_msg->dma_addr = data_buffer;
    }
    return 0;
}

int audio_get_dma_from_msg(
    const ipi_msg_t *ipi_msg,
    void **data_buf,
    uint32_t *data_size)
{
#if defined(CFG_MTK_AUDIODSP_SUPPORT) || defined(CFG_MTK_AUDIO_CM4_DMA_SUPPORT)
#if defined(CFG_MTK_AUDIODSP_SUPPORT)
    void *buf = NULL;
#endif
    uint32_t size = 0;

    int ret = 0;

    if (!ipi_msg || !data_buf || !data_size) {
        AUD_LOG_W("msg: %p, buf: %p, sz: %p NULL!!",
                  ipi_msg, data_buf, data_size);
        return -EFAULT;
    }

    if (ipi_msg->data_type != AUDIO_IPI_DMA) {
        DUMP_IPI_MSG("not dma!!", ipi_msg);
        return -EFAULT;
    }

    size = ipi_msg->dma_info.data_size;
#if defined(CFG_MTK_AUDIODSP_SUPPORT)
    AUDIO_ALLOC_BUFFER(buf, size);
    AUD_LOG_V("+msg: %p, buf: %p, size: %u",
              ipi_msg, buf, size);
    ret = audio_ipi_dma_read_region(
              ipi_msg->task_scene,
              buf,
              size,
              ipi_msg->dma_info.rw_idx);
    if (ret != 0) {
        AUDIO_FREE_POINTER(buf);
    } else {
        *data_buf = buf;
        *data_size = size;
    }
#endif
#if defined(CFG_MTK_AUDIO_CM4_DMA_SUPPORT)
    ret = audio_ipi_dma_read_region(
              ipi_msg->task_scene,
              *data_buf,
              size,
              ipi_msg->dma_info.rw_idx);
    if (ret == 0) {
        *data_size = size;
    }
#endif
    AUD_LOG_V("-msg: %p, data_buf: %p, data_size: %u",
              ipi_msg, *data_buf, *data_size);

    return ret;
#else
    (void)ipi_msg;
    (void)data_buf;
    (void)data_size;

    return -EFAULT;
#endif
}




