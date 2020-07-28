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
#if defined(CFG_MTK_AUDIODSP_SUPPORT)
#include <mt_gpt.h>
#endif
#include <audio_type.h>
#include <audio_ringbuf.h>
#include "audio_task_factory.h"
#include "audio_log.h"
#include "audio_hw.h"
#include "audio_task_utility.h"
#include "audio_dsp_hw_hal.h"
#include "audio_messenger_ipi.h"
#include "audio_task_attr.h"
#include <wrapped_audio.h>

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <aurisys_config.h>
#endif

#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif

#ifdef CFG_AUDIO_SUPPORT
#include <dma.h>
#endif

#ifdef CFG_AUDIO_SUPPORT
#if defined(CFG_MTK_AUDIODSP_SUPPORT)
#include <adsp_ipi.h>
#endif
#include <audio_ipi_dma.h>
#else
#include <scp_ipi.h>
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

struct aud_time_profile {
    unsigned long long time_interval_prev;
    unsigned long long time_interval_last;
    unsigned long long time_diff;
    unsigned long long threshold;
};

static struct aud_time_profile aud_time_profiler[NUM_OF_TASK_TIME_ID];

int aud_task_check_state(struct AudioTask *task, unsigned int state)
{
    if (task->state != state) {
        AUD_LOG_E("scene = %d Unexpected start with status=%lu\n", task->scene,
                  task->state);
        return -1;
    }
    return 0;
}

unsigned int frame_to_bytes(unsigned int framecount, unsigned int channels,
                            unsigned int format)
{
    framecount = framecount * channels;

    switch (format) {
        case SNDRV_PCM_FORMAT_S8:
        case SNDRV_PCM_FORMAT_U8:
            return framecount;
        case SNDRV_PCM_FORMAT_S16_LE:
        case SNDRV_PCM_FORMAT_S16_BE:
        case SNDRV_PCM_FORMAT_U16_LE:
        case SNDRV_PCM_FORMAT_U16_BE:
            return framecount * 2;
        case SNDRV_PCM_FORMAT_S24_LE:
        case SNDRV_PCM_FORMAT_S24_BE:
        case SNDRV_PCM_FORMAT_U24_LE:
        case SNDRV_PCM_FORMAT_U24_BE:
        case SNDRV_PCM_FORMAT_S32_LE:
        case SNDRV_PCM_FORMAT_S32_BE:
        case SNDRV_PCM_FORMAT_U32_LE:
        case SNDRV_PCM_FORMAT_U32_BE:
            return framecount * 4;
        default:
            return framecount;
    }
    return framecount;
}

void aud_task_delay(int delayms)
{
    vTaskDelay((portTickType) delayms / portTICK_RATE_MS);
}

unsigned int get_peroid_mstime(unsigned int rate, unsigned int period)
{
    unsigned int peroid_ms = 0;

    if (rate > 0)
       peroid_ms = ((period * 1000) / rate)+ 1;

    return peroid_ms;
}

unsigned int get_audio_irq_size(struct audio_hw_buffer *audio_hwbuf)
{
    int framesize = frame_to_bytes(
                        audio_hwbuf->aud_buffer.period_size,
                        audio_hwbuf->aud_buffer.buffer_attr.channel,
                        audio_hwbuf->aud_buffer.buffer_attr.format
                    );
    return framesize;
}

int get_audio_loop_count(struct audio_hw_buffer *audio_hwbuf,
                         int process_size,
                         int *count)
{
    unsigned int irq_size = get_audio_irq_size(audio_hwbuf);
    *count = (irq_size / process_size) + 1;
    return 0;
}


/* dma buffer is a pointer to audiohwbuffer*/
int get_audiobuf_from_msg(struct Audio_Task_Msg_t *AudioTaskMsg,
                          struct audio_hw_buffer *task_audio_buf)
{
    if (AudioTaskMsg == NULL) {
        return -1;
    }
    if (task_audio_buf == NULL) {
        return -1;
    }

    /* get buffer */
    memcpy(task_audio_buf, (char *)ap_to_scp((uint32_t)AudioTaskMsg->param2),
           sizeof(struct audio_hw_buffer));
    return 0;
}

/* dma buffer is a pointer to audiohwbuffer*/
int get_audioattr_from_msg(struct Audio_Task_Msg_t *AudioTaskMsg,
                           struct buf_attr *task_audio_attr)
{
    if (AudioTaskMsg == NULL) {
        return -1;
    }
    if (task_audio_attr == NULL) {
        return -1;
    }

    /* get buffer */
    AUD_LOG_D("AudioTaskMsg->dma_addr = 0x%x\n",
              (unsigned int)AudioTaskMsg->dma_addr);
    memcpy(task_audio_attr, (char *)ap_to_scp((uint32_t)AudioTaskMsg->dma_addr),
           sizeof(struct buf_attr));
    return 0;
}

unsigned long long get_time_stamp()
{
#if defined(CFG_MTK_AUDIODSP_SUPPORT)
    return read_xgpt_stamp_ns();
    //return timer_get_global_timer_tick();
#endif
    return 0;
}

int init_time_interval(int id, unsigned long long thresholdns)
{
    if (id >= NUM_OF_TASK_TIME_ID) {
        AUD_LOG_E("%s(), id = %d \n", __func__, id);
        return -1;
    }
    memset((void *)&aud_time_profiler[id], 0 , sizeof(struct aud_time_profile));
    aud_time_profiler[id].threshold = thresholdns;
    return 0;
}

int record_time_interval(int id)
{
    int ret = 0;
    if (id >= NUM_OF_TASK_TIME_ID) {
        AUD_LOG_E("%s(), id = %d \n", __func__, id);
        return -1;
    }

    /* first time*/
    if (aud_time_profiler[id].time_interval_prev == 0) {
        aud_time_profiler[id].time_interval_prev = get_time_stamp();
        return 0;
    }
    else {
        aud_time_profiler[id].time_interval_last = get_time_stamp();
        aud_time_profiler[id].time_diff = aud_time_profiler[id].time_interval_last -
                                          aud_time_profiler[id].time_interval_prev ;
        if (aud_time_profiler[id].time_diff > aud_time_profiler[id].threshold) {
            AUD_LOG_D("%s id time %d > threshold %llu prev = %llu last =%llu diff = %llu ms = %llu\n",
                      __func__,
                      id,
                      aud_time_profiler[id].threshold,
                      aud_time_profiler[id].time_interval_prev,
                      aud_time_profiler[id].time_interval_last,
                      aud_time_profiler[id].time_diff,
                      aud_time_profiler[id].time_diff / 1000000
                     );
            ret = 1;
        }
        /* record for next */
        aud_time_profiler[id].time_interval_prev =
            aud_time_profiler[id].time_interval_last;
    }
    return ret;
}

int stop_time_interval(int id)
{
    if (id >= NUM_OF_TASK_TIME_ID) {
        AUD_LOG_E("%s(), id = %d \n", __func__, id);
        return -1;
    }
    aud_time_profiler[id].time_interval_prev = 0;
    aud_time_profiler[id].time_interval_last = 0;
    aud_time_profiler[id].time_diff = 0;
    return 0;
}

unsigned int getframesize(unsigned int channel, unsigned int format)
{
    int size = 0;
    switch (format) {
        case SNDRV_PCM_FORMAT_S8:
        case SNDRV_PCM_FORMAT_U8:
            size = 1;
            break;
        case SNDRV_PCM_FORMAT_S16_LE:
        case SNDRV_PCM_FORMAT_S16_BE:
        case SNDRV_PCM_FORMAT_U16_LE:
        case SNDRV_PCM_FORMAT_U16_BE:
            size = 2;
            break;
        case SNDRV_PCM_FORMAT_S24_LE:
        case SNDRV_PCM_FORMAT_S24_BE:
        case SNDRV_PCM_FORMAT_U24_LE:
        case SNDRV_PCM_FORMAT_U24_BE:
            size = 4;
            break;
        case SNDRV_PCM_FORMAT_S32_LE:
        case SNDRV_PCM_FORMAT_S32_BE:
        case SNDRV_PCM_FORMAT_U32_LE:
        case SNDRV_PCM_FORMAT_U32_BE:
            size = 4;
            break;
        default:
            AUD_LOG_W("%s unsupport format(%d)\n", __func__, format);
    }
    return channel * size;
}

void DumpTaskMsg(char *appendstring, struct Audio_Task_Msg_t *Msg)
{
    AUD_LOG_D("%s(), ts = %llu Msg =0x%x, 0x%lx, 0x%lx dma = 0x%x\n",
              appendstring, get_time_stamp(), Msg->msg_id, Msg->param1, Msg->param2,
              (unsigned int)Msg->dma_addr);
    return ;
}


int enter_write_cond(int handle,
                     struct AudioTask *task,
                     struct audio_hw_buffer *audio_hwbuf,
                     struct audio_hw_buffer *audio_afebuf,
                     struct RingBuf *pRingBuf,
                     unsigned int written_size)
{
    bool ret = true;

    if (handle < 0) {
        AUD_LOG_D("%s hw_buf_handle[%d]\n", __func__,
                  handle);
        return false;
    }
    /* no data to write */
    if (RingBuf_getDataCount(pRingBuf) < written_size) {
        AUD_LOG_V("%s written_size[%d] datacount[%d]\n", __func__,
                  written_size,
                  RingBuf_getDataCount(pRingBuf));
        return false;
    }

    /* when AP is starting but hardware is not starting, write until hw start.*/
    if (task->state == AUDIO_TASK_WORKING &&
        !(audio_dsp_hw_status(task, audio_afebuf) & STATE_RUN)) {
        AUD_LOG_V("%s handle[%d] task state[%d] audio_dsp_hw_status = %d\n",
                  __func__,
                  handle,
                  task->state,
                  audio_dsp_hw_status(task, audio_afebuf));
        return true;
    }

    /* state is not working*/
    if (task->state != AUDIO_TASK_WORKING) {
        AUD_LOG_V("%s handle[%d] task state[%d]\n", __func__, handle, task->state);
        return false;
    }

    if (audio_dsp_hw_getavail(handle) <= get_audio_irq_size(audio_afebuf)) {
        AUD_LOG_V("%s handle[%d] avail[%u] get_audio_irq_size[%u] task->state =%u audio_dsp_hw_status = %d\n",
                  __func__, handle,
                  audio_dsp_hw_getavail(handle),
                  get_audio_irq_size(audio_hwbuf),
                  task->state,
                  audio_dsp_hw_status(task, audio_afebuf));
        ret = false;
    }

    return ret;
}

int enter_read_cond(int handle,
                    struct AudioTask *task,
                    struct audio_hw_buffer *audio_hwbuf,
                    struct audio_hw_buffer *audio_afebuf,
                    struct RingBuf *pRingBuf,
                    unsigned int processing_in_size,
                    unsigned int processing_out_size)
{
    if (handle < 0) {
        AUD_LOG_D("%s hw_buf_handle[%d]\n", __func__,
                  handle);
        return false;
    }

    if (task->state != AUDIO_TASK_WORKING) {
        AUD_LOG_D("%s handle state[%lu]\n", __func__, task->state);
        return false;
    }

    /* check ring buffer space */
    if (RingBuf_getFreeSpace(pRingBuf) < processing_out_size) {
        AUD_LOG_D("%s processing_out_size[%d] datacount[%d]\n", __func__,
                  processing_out_size,
                  RingBuf_getDataCount(pRingBuf));
        return false;
    }

    /* check hardware buffer data count*/
    if (audio_dsp_hw_getavail(handle) < processing_in_size) {
        AUD_LOG_D("%s handle[%d] avail[%u] processing_in_size[%u] task->state =%lu\n",
                  __func__, handle,
                  audio_dsp_hw_getavail(handle),
                  processing_in_size,
                  task->state);
        return false;
    }

    return true;
}

int first_write_cond(int handle,
                     struct AudioTask *task,
                     struct audio_hw_buffer *audio_hwbuf,
                     struct audio_hw_buffer *audio_afebuf,
                     struct RingBuf *pRingBuf,
                     unsigned int written_size)
{
    bool ret = false;

    /* no data to write */
    if (RingBuf_getDataCount(pRingBuf) < written_size) {
        AUD_LOG_V("%s written_size[%d] datacount[%d]\n", __func__,
                  written_size,
                  RingBuf_getDataCount(pRingBuf));
        return false;
    }

    if (audio_dsp_hw_getavail(handle) <= get_audio_irq_size(audio_afebuf)) {
        AUD_LOG_V("%s avail[%u] get_audio_irq_size[%u]\n",
                  __func__,
                  audio_dsp_hw_getavail(handle),
                  get_audio_irq_size(audio_hwbuf));
        ret = false;
    }

    /* when start condition*/
    if (task->state == AUDIO_TASK_WORKING &&
        (audio_dsp_hw_status(task, audio_afebuf) & STATE_RUN)) {
        return true;
    }

    return ret;
}


/*
 * return scene number base on audio irq num
 */
int adsp_audio_irq_entry()
{
    int irqnum = adsp_audio_irq_handler();
    int sceneid = get_audio_scene_by_irqline(irqnum);
    return sceneid;
}

int send_task_message(struct AudioTask *this, struct ipi_msg_t *ipi_msg,
                      unsigned int max_queue_size)
{
    uint8_t queue_idx = 0, i = 0;
    taskENTER_CRITICAL();
    queue_idx = get_queue_idx(this, max_queue_size);
    this->num_queue_element++;

    if (this->msg_array[queue_idx].magic != 0) {
        AUD_LOG_E("%s queue_idx = %d\n", __func__, queue_idx);
        for (i = 0; i < max_queue_size; i++) {
            AUD_LOG_E("[%d] id = 0x%x\n", i, this->msg_array[i].msg_id);
        }
        AUD_ASSERT(this->msg_array[queue_idx].magic == 0); /* item is clean */
    }
    memcpy(&this->msg_array[queue_idx], ipi_msg, sizeof(ipi_msg_t));
    if (xQueueSendToBack(this->msg_idx_queue, &queue_idx,
                         (TickType_t) 20) != pdPASS) {
        taskEXIT_CRITICAL();
        AUD_LOG_E("xQueueSendToBack error msg_idx_queue[%p]\n", (this->msg_idx_queue));
        return UNKNOWN_ERROR;
    }
    taskEXIT_CRITICAL();
    return 0;
}

unsigned int get_queue_idx(struct AudioTask *this,  unsigned int max_queue_size)
{
    unsigned int queue_idx = this->queue_idx;

    this->queue_idx++;
    if (this->queue_idx == max_queue_size) {
        this->queue_idx = 0;
    }

    return queue_idx;
}

int send_task_message_fromisr(struct AudioTask *this, struct ipi_msg_t *ipi_msg,
                              unsigned int max_queue_size)
{
    unsigned char queue_idx = 0;
    unsigned int i = 0;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    queue_idx = get_queue_idx(this, max_queue_size);
    this->num_queue_element++;
    if (this->msg_array[queue_idx].magic != 0) {
        AUD_LOG_E("%s queue_idx = %d\n", __func__, queue_idx);
        for (i = 0; i < max_queue_size; i++) {
            AUD_LOG_E("[%d] id = 0x%x scene = %d\n", i, this->msg_array[i].msg_id, this->scene);
        }
        AUD_ASSERT(this->msg_array[queue_idx].magic == 0); /* item is clean */
    }
    memcpy(&this->msg_array[queue_idx], ipi_msg, sizeof(ipi_msg_t));
    if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
                                &xHigherPriorityTaskWoken) != pdPASS) {
        AUD_LOG_E("xQueueSendToBackFromISR error msg_idx_queue[%p]\n",
                  (this->msg_idx_queue));
        return UNKNOWN_ERROR;
    }
    return 0;
}

/* example only: place porcessing */
int task_do_processing(char *inputbuffer, int inputbuffersize,
                       char *outputbuffer, int *outputbuffersize,
                       char *referencebuffer, int referencebuffersize)
{
    *outputbuffersize =  inputbuffersize;
    memcpy(outputbuffer, inputbuffer, inputbuffersize);
    ALOGD("%s referencebuffer = %p referencebuffersize = %d\n", __func__,
        referencebuffer, referencebuffersize);
    return 0;
}

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
void cal_output_buffer_size(struct stream_attribute_dsp *attr_in,
                            struct stream_attribute_dsp *attr_out,
                            uint32_t size_in,
                            uint32_t *size_out)
{
    double times = 1.0;

    if (attr_in == NULL || attr_out == NULL || size_out == NULL || size_in == 0) {
        return;
    }

    uint32_t size_per_frame_target = attr_out->num_channels *
                                     AUDIO_BYTES_PER_SAMPLE(attr_out->audio_format);
    uint32_t unit_bytes_source = attr_in->num_channels * AUDIO_BYTES_PER_SAMPLE(
                                     attr_in->audio_format) * attr_in->sample_rate;
    uint32_t unit_bytes_target = size_per_frame_target * attr_out->sample_rate;

    if (unit_bytes_source == 0 || unit_bytes_target == 0) {
        return;
    }

    times = (double)unit_bytes_target / (double)unit_bytes_source;
    *size_out = (uint32_t)(times * size_in);

    /* align */
    if (*size_out % size_per_frame_target != 0) {
        *size_out = ((*size_out / size_per_frame_target) + 1) * size_per_frame_target;
    }
}

void cal_buffer_size(struct stream_attribute_dsp *attr, int *size) {
    if (attr == NULL || size == NULL) {
        AUD_LOG_E("%s(), attr or size should not be NULL", __func__);
        return;
    }

    uint32_t size_per_frame = attr->num_channels * AUDIO_BYTES_PER_SAMPLE(attr->audio_format);
    *size = (size_per_frame * attr->sample_rate * attr->latency_ms) / 1000;

    if (size_per_frame == 0) {
        AUD_LOG_E("%s(), size_per_frame == 0", __func__);
        return;
    }

    /* align */
    if (*size % size_per_frame != 0) {
        *size = ((*size / size_per_frame) + 1) * size_per_frame;
    }
}
#endif

bool task_is_dl(int task_scene)
{
    if (task_scene == TASK_SCENE_PRIMARY ||
        task_scene == TASK_SCENE_DEEPBUFFER ||
        task_scene == TASK_SCENE_PLAYBACK_MP3 ||
        task_scene == TASK_SCENE_VOIP) {
        return true;
    }
    return false;
}

/* todo : add with ul path*/
bool task_is_ul(int task_scene)
{
    if (task_scene == TASK_SCENE_CAPTURE_UL1) {
        return true;
    }

    return false;
}


