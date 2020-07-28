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

#include "audio_task_dataprovider.h"
#include "audio_task_common.h"
#include "audio_task_a2dp.h"
#include "audio_a2dp_msg_id.h"

static struct audio_task_common *task_handle;
static bool bA2dpIsPaused;

#define MAX_MSG_QUEUE_SIZE (40)
#define LOCAL_TASK_STACK_SIZE (2048)
#define LOCAL_TASK_NAME "aud_dataprovider"
#define LOCAL_TASK_PRIORITY (2)
// must slign with UL memif interrupt rate
#define local_task_output_buffer_size (4096)

static void task_dataprovider_irq_handler(struct AudioTask *this, uint32_t irq_type);
static void task_dataprovider_constructor(struct AudioTask *this);
static void task_dataprovider_destructor(struct AudioTask *this);
static void task_dataprovider_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type);

static void task_copyto_pcmringbuf(unsigned int length, char *buf) {
    int ret = 0, wait_cnt = 0, threshold = 0;
    unsigned int spaces;

    struct audio_a2dp_struct *pAudio_a2dp_struct;
    pAudio_a2dp_struct = (struct audio_a2dp_struct *)get_a2dp_structure();

    threshold = 10;
    while (ret < length && wait_cnt < threshold) {
        wait_cnt++;
        spaces = RingBuf_getFreeSpace(pAudio_a2dp_struct->pcm_ring_buf);
        if (spaces >= length) {
            RingBuf_copyFromLinear(pAudio_a2dp_struct->pcm_ring_buf, buf, length);
            ret = length;
            AUD_LOG_D("%s() copy to pcm_ringbuf ret=%d, length=%d, spaces=%u", __func__, ret, length, spaces);
            AUD_LOG_D("%s() enter xSemaphoreGive xPCMRingBufSemphr2", __func__);
            xSemaphoreGive(pAudio_a2dp_struct->xPCMRingBufSemphr2);
        }
        else {
            AUD_LOG_D("%s() xPCMRingBufSemphr=%p", __func__, pAudio_a2dp_struct->xPCMRingBufSemphr);
            BaseType_t xResult;
            xResult = xSemaphoreTake(pAudio_a2dp_struct->xPCMRingBufSemphr, pdMS_TO_TICKS(20));
            if(xResult == pdTRUE) {
                AUD_LOG_D("%s() leave xPCMRingBufSemphr normally", __func__);
            }
            else {
                AUD_LOG_D("%s() leave xPCMRingBufSemphr TIMEOUT 20ms!", __func__);
            }
        }
    }

    if (wait_cnt >= threshold) {
        AUD_LOG_W("%s() wait_cnt > threshold(%d)!", __func__, threshold);
    }
}

void task_dataprovider_task_loop(void *pvParameters)
{
    AudioTask *this = (AudioTask *)pvParameters;
    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;
    uint8_t local_queue_idx = 0xFF;
    ipi_msg_t *ipi_msg;
    struct Audio_Task_Msg_t uAudioTaskMsg;
    int ret = 0;

    if (task_common->debug_flag >= TASK_DEBUG_VERBOSE) {
        AUD_LOG_V("%s() start \n", __func__);
    }

    while (1) {
        xQueueReceive(this->msg_idx_queue, &local_queue_idx, portMAX_DELAY);
        ipi_msg = &this->msg_array[local_queue_idx];

        uAudioTaskMsg.msg_id = ipi_msg->msg_id;
        uAudioTaskMsg.param1 = ipi_msg->param1;
        uAudioTaskMsg.param2 = ipi_msg->param2;

        if (task_common->debug_flag >= TASK_DEBUG_VERBOSE)
            AUD_LOG_V("%s() loop start msg_id = %d param1 = %d param2 = %d local_queue_idx[%d]",
                      __func__, uAudioTaskMsg.msg_id, uAudioTaskMsg.param1, uAudioTaskMsg.param2,
                      local_queue_idx);

        switch (uAudioTaskMsg.msg_id) {
            case AUDIO_DSP_TASK_OPEN: {
                AUD_LOG_D("%s() AUDIO_DSP_TASK_OPEN", task_common->task_name);
                task_require_resource(task_common);
                break;
            }
            case AUDIO_DSP_TASK_CLOSE: {
                AUD_LOG_D("%s() AUDIO_DSP_TASK_CLOSE", task_common->task_name);
                task_common->pcmdump_enable = 0;
                this->state = AUDIO_TASK_DEINIT;
                task_release_resource(task_common);
                break;
            }
            case AUDIO_DSP_TASK_HWPARAM: {
                if (task_common->debug_flag >= TASK_DEBUG_INFO) {
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_HWPARAM", task_common->task_name);
                }
                unsigned int addr = (unsigned int)task_common->mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)addr,
                                           task_common->mtask_dsp_atod_sharemm.size
                                          );
                memset((void *)&task_common->mtask_audio_dl_buf, 0,
                       sizeof(struct audio_hw_buffer));
                get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_dl_buf);
                RingBuf_Map_RingBuf_bridge(
                    &task_common->mtask_audio_dl_buf.aud_buffer.buf_bridge,
                    &task_common->mtask_audio_ring_buf);

                dump_task_hwbuf(task_common, "AUDIO_DSP_TASK_HWPARAM");
                break;
            }
            case AUDIO_DSP_TASK_PREPARE: {
                task_print(task_common, "AUDIO_DSP_TASK_PREPARE");
                memset(task_common->mtask_output_buf, 0, task_common->mtask_output_buf_size);
                memset(task_common->mtask_input_buf, 0, task_common->mtask_input_buf_size);

                struct audio_a2dp_struct *pAudio_a2dp_struct;
                pAudio_a2dp_struct = (struct audio_a2dp_struct *)get_a2dp_structure();
                RingBuf_Reset(pAudio_a2dp_struct->pcm_ring_buf);
                this->state = AUDIO_TASK_INIT;
                break;
            }
            case AUDIO_DSP_TASK_MSGA2DSHAREMEM: {
                DumpTaskMsg("AUDIO_DSP_TASK_MSGA2DSHAREMEM", &uAudioTaskMsg);
                memcpy((void *)&task_common->mtask_dsp_atod_sharemm, ipi_msg->payload,
                       ipi_msg->param1);
                dump_audio_dsp_dram(&task_common->mtask_dsp_atod_sharemm);
                break;
            }
            case AUDIO_DSP_TASK_MSGD2ASHAREMEM: {
                DumpTaskMsg("AUDIO_DSP_TASK_MSGA2DSHAREMEM", &uAudioTaskMsg);
                memcpy((void *)&task_common->mtask_dsp_dtoa_sharemm, ipi_msg->payload,
                       ipi_msg->param1);
                dump_audio_dsp_dram(&task_common->mtask_dsp_dtoa_sharemm);
                break;
            }
            case AUDIO_DSP_TASK_START: {
                this->state = AUDIO_TASK_WORKING;
                bA2dpIsPaused = false;

                ipi_msg_t ipi_msg_to_task;
                packing_ipimsg(&ipi_msg_to_task, this->scene, AUDIO_IPI_LAYER_TO_DSP,
                               AUDIO_IPI_MSG_ONLY,
                               AUDIO_IPI_MSG_BYPASS_ACK,
                               AUDIO_DSP_TASK_DATAPROVIDER_READ,
                               0,
                               0,
                               NULL);
                send_task_message(this, &ipi_msg_to_task, task_common->max_queue_size);
                task_print(task_common, "AUDIO_DSP_TASK_START");
                break;
            }

            case AUDIO_DSP_TASK_STOP: {
                task_print(task_common, "AUDIO_DSP_TASK_STOP");
                this->state = AUDIO_TASK_IDLE;
                audio_dsp_hw_stop(task_common->playback_hwbuf_handle);
                audio_dsp_hw_stop(task_common->capture_hwbuf_handle);
                break;
            }
            case AUDIO_DSP_TASK_PCM_HWPARAM: {
                unsigned int addr = (unsigned int)task_common->mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)addr,
                                           task_common->mtask_dsp_atod_sharemm.size
                                          );
                get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_buf_working);
                if (task_common->mtask_audio_buf_working.aud_buffer.buffer_attr.direction  ==
                    AUDIO_DSP_TASK_PCM_HWPARAM_DL) {
                    AUD_LOG_D("AUDIO_DSP_TASK_PCM_HWPARAM_DL task_name %s",
                              task_common->task_name);
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_afedl_buf);
                    dump_audio_hwbuffer(&task_common->mtask_audio_afedl_buf);
                }
                else if (task_common->mtask_audio_buf_working.aud_buffer.buffer_attr.direction
                         ==
                         AUDIO_DSP_TASK_PCM_HWPARAM_UL) {
                    AUD_LOG_D("AUDIO_DSP_TASK_PCM_HWPARAM_UL task_name %s",
                              task_common->task_name);
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_afeul_buf);
                    dump_audio_hwbuffer(&task_common->mtask_audio_afeul_buf);
                }
                else if (task_common->mtask_audio_buf_working.aud_buffer.buffer_attr.direction
                         ==
                         AUDIO_DSP_TASK_PCM_HWPARAM_REF) {
                    AUD_LOG_D("AUDIO_DSP_TASK_PCM_HWPARAM_REF task_name %s",
                              task_common->task_name);
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_ref_buf);
                    dump_audio_hwbuffer(&task_common->mtask_audio_ref_buf);
                }
                else {
                    AUD_LOG_W("AUDIO_DSP_TASK_PCM_HWPARAM err  task_name %s",
                              task_common->task_name);
                }
                break;
            }

            case AUDIO_DSP_TASK_PCM_HWFREE: {
                if (task_common->debug_flag >= TASK_DEBUG_INFO) {
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_HWFREE", __FUNCTION__);
                }

                if (uAudioTaskMsg.param1 == AUDIO_DSP_TASK_PCM_HWPARAM_UL) {
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_HWFREE_UL", __FUNCTION__);
                    audio_dsp_hw_close(task_common->capture_hwbuf_handle);
                    task_common->capture_hwbuf_handle = -1;
                }
                break;
            }
            case AUDIO_DSP_TASK_PCM_PREPARE: {
                if (task_common->debug_flag >= TASK_DEBUG_INFO) {
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_PREPARE", __FUNCTION__);
                }

                unsigned int addr = (unsigned int)task_common->mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)addr,
                                           task_common->mtask_dsp_atod_sharemm.size
                                          );
                get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_buf_working);
                dump_audio_hwbuffer(&task_common->mtask_audio_buf_working);

                if (task_common->mtask_audio_buf_working.aud_buffer.buffer_attr.direction  ==
                    AUDIO_DSP_TASK_PCM_HWPARAM_DL) {
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_PREPARE DL", __FUNCTION__);
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_afedl_buf);
                    dump_audio_hwbuffer(&task_common->mtask_audio_afedl_buf);
                    if (task_is_dl(task_common->task_scene) == false) {
                        task_common->mtask_audio_afedl_buf.ignore_irq = true;
                    }
                    if (task_common->dummy_write)
                        task_common->playback_hwbuf_handle =
                            audio_dsp_dummy_open(this, &task_common->mtask_audio_afedl_buf);
                    else {
                        task_common->playback_hwbuf_handle =
                            audio_dsp_hw_open(this, &task_common->mtask_audio_afedl_buf);
                    }
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_PREPARE_DL playback_hwbuf_handle[%d]",
                              __FUNCTION__, task_common->playback_hwbuf_handle);
                }
                else if (task_common->mtask_audio_buf_working.aud_buffer.buffer_attr.direction
                         == AUDIO_DSP_TASK_PCM_HWPARAM_UL) {
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_afeul_buf);
                    dump_audio_hwbuffer(&task_common->mtask_audio_afeul_buf);
                    if (task_common->dummy_write) {
                        task_common->capture_hwbuf_handle =
                            audio_dsp_dummy_open(this, &task_common->mtask_audio_afeul_buf);
                    }
                    else {
                        task_common->capture_hwbuf_handle =
                            audio_dsp_hw_open(this, &task_common->mtask_audio_afeul_buf);
                    }
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_PREPARE_UL capture_hwbuf_handle[%d]",
                              __FUNCTION__, task_common->capture_hwbuf_handle);
                }
                else if (task_common->mtask_audio_buf_working.aud_buffer.buffer_attr.direction
                         == AUDIO_DSP_TASK_PCM_HWPARAM_REF) {
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_ref_buf);
                    dump_audio_hwbuffer(&task_common->mtask_audio_ref_buf);
                    task_common->mtask_audio_ref_buf.ignore_irq = true;
                    if (task_common->dummy_write) {
                        task_common->ref_hwbuf_handle =
                            audio_dsp_dummy_open(this, &task_common->mtask_audio_ref_buf);
                    }
                    else {
                        task_common->ref_hwbuf_handle =
                            audio_dsp_hw_open(this, &task_common->mtask_audio_ref_buf);
                    }
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_PREPARE_REF ref_hwbuf_handle[%d]",
                              __FUNCTION__, task_common->ref_hwbuf_handle);
                }
                else {
                    AUD_LOG_W("%s() direction = %u",
                              __FUNCTION__,
                              task_common->mtask_audio_buf_working.aud_buffer.buffer_attr.direction
                             );
                }
                break;
            }

            case AUDIO_DSP_TASK_DATAPROVIDER_READ: {
                if (this->state == AUDIO_TASK_WORKING) {
                    if (bA2dpIsPaused == true) {
                        aud_task_delay(10);  // must align local_task_output_buffer_size
                        AUD_LOG_D("%s() bA2dpIsPaused == true, write mute data!", __FUNCTION__);
                        memset(task_common->mtask_input_buf, 0, task_common->mtask_input_buf_size);
                    } else {
                        ret = audio_dsp_hw_read(task_common->capture_hwbuf_handle,
                                                task_common->mtask_input_buf,
                                                task_common->mtask_input_buf_size);
                        if ((ret != task_common->mtask_input_buf_size) && (ret != -1)) { // read data not enough & not overflow
                            AUD_LOG_E("%s() read fail, ret=%d, write mute data!", __FUNCTION__, ret);
                            memset(task_common->mtask_input_buf, 0, task_common->mtask_input_buf_size);
                        }
                    }
#if 0
                    if (task_common->pcmdump_enable == 1) {
                        struct ipi_msg_t ipi_msg;
                        char *data_buf = task_common->mtask_input_buf;
                        uint32_t data_size = task_common->mtask_input_buf_size;

                        audio_send_ipi_msg(&ipi_msg, this->scene,
                                           AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                                           AUDIO_IPI_MSG_BYPASS_ACK,
                                           AUDIO_DSP_TASK_PCMDUMP_DATA,
                                           data_size,
                                           DEBUG_PCMDUMP_IN, //dump point serial number
                                           data_buf);

                        AUD_LOG_D("AUDIO_DSP_TASK_DATAPROVIDER_READ AUDIO_DSP_TASK_PCMDUMP_DATA in\n");
                    }

                    task_do_processing(task_common->mtask_input_buf,
                                       task_common->mtask_input_buf_size,
                                       task_common->mtask_output_buf,
                                       &task_common->mtask_output_buf_size, NULL, 0);


                    if (task_common->pcmdump_enable == 1) {
                        struct ipi_msg_t ipi_msg;
                        char *data_buf = task_common->mtask_output_buf;
                        uint32_t data_size = task_common->mtask_output_buf_size;

                        audio_send_ipi_msg(&ipi_msg, this->scene,
                                           AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                                           AUDIO_IPI_MSG_BYPASS_ACK,
                                           AUDIO_DSP_TASK_PCMDUMP_DATA,
                                           data_size,
                                           DEBUG_PCMDUMP_OUT, //dump point serial number
                                           data_buf);

                        AUD_LOG_D("AUDIO_DSP_TASK_DATAPROVIDER_READ AUDIO_DSP_TASK_PCMDUMP_DATA out\n");
                    }
#endif
                    task_copyto_pcmringbuf(task_common->mtask_input_buf_size, task_common->mtask_input_buf);

                    ipi_msg_t ipi_msg_to_task;
                    packing_ipimsg(&ipi_msg_to_task, this->scene, AUDIO_IPI_LAYER_TO_DSP,
                                   AUDIO_IPI_MSG_ONLY,
                                   AUDIO_IPI_MSG_BYPASS_ACK,
                                   AUDIO_DSP_TASK_DATAPROVIDER_READ,
                                   0,
                                   0,
                                   NULL);

                    send_task_message(this, &ipi_msg_to_task, task_common->max_queue_size);
                } else {
                    AUD_LOG_W("%s() AUDIO_DSP_TASK_DATAPROVIDER_READ this->state=%d, bA2dpIsPaused=%d ", __FUNCTION__,
                              this->state, bA2dpIsPaused);
                }
                break;
            }

            case AUDIO_DSP_TASK_PCMDUMP_ON:
                task_common->pcmdump_enable = uAudioTaskMsg.param1;
                break;

            case AUDIO_DSP_TASK_DATAPROVIDER_SUSPEND:
                if (uAudioTaskMsg.param1 == 0) {
                    bA2dpIsPaused = 0;
                } else {
                    audio_dsp_hw_stop(task_common->capture_hwbuf_handle);

                    struct audio_a2dp_struct *pAudio_a2dp_struct;
                    pAudio_a2dp_struct = (struct audio_a2dp_struct *)get_a2dp_structure();
                    memset(pAudio_a2dp_struct->pcm_ring_buf->pBufBase, 0, pAudio_a2dp_struct->pcm_ring_buf->bufLen);

                    pAudio_a2dp_struct->pcm_ring_buf->pRead = pAudio_a2dp_struct->pcm_ring_buf->pBufBase;
                    pAudio_a2dp_struct->pcm_ring_buf->pWrite = pAudio_a2dp_struct->pcm_ring_buf->pBufBase;
                    pAudio_a2dp_struct->pcm_ring_buf->datacount = 0;

                    pAudio_a2dp_struct->bt_first_wakeup = true;
                    bA2dpIsPaused = 1;
                }
                AUD_LOG_D("AUDIO_DSP_TASK_DATAPROVIDER_SUSPEND bA2dpIsPaused=%d", bA2dpIsPaused);
                break;

            default:
                AUD_LOG_W("%s() id = %d task[%s]", __FUNCTION__,
                          uAudioTaskMsg.msg_id, task_common->task_name);
                break;
        }

        /* send ack back if need */
        audio_send_ipi_msg_ack_back(ipi_msg);

        /* clean msg */
        memset(&this->msg_array[local_queue_idx], 0, sizeof(ipi_msg_t));
        this->num_queue_element--;
    }
}

void task_dataprovider_create_task_loop(struct AudioTask *this)
{
    /* Note: you can also bypass this function,
             and do kal_xTaskCreate until you really need it.
             Ex: create task after you do get the enable message. */

    BaseType_t xReturn = pdFAIL;
    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;

    xReturn = kal_xTaskCreate(
                  task_dataprovider_task_loop,
                  task_common->task_name,
                  task_common->task_stack_size,
                  (void *)this,
                  task_common->task_priority,
                  &this->freertos_task);

    AUD_ASSERT(xReturn == pdPASS);
}

AudioTask *task_dataprovider_new(void) {
    /* alloc object here */
    int ret = 0;
    AudioTask *task = NULL;
    ret = init_common_task(&task_handle);
    if (ret) {
        AUD_LOG_E("%s(), init_common_task fail!!", __func__);
        return NULL;
    }

    task = (AudioTask *)AUDIO_MALLOC(sizeof(AudioTask));
    if (task == NULL) {
        AUD_LOG_E("%s(), AUDIO_MALLOC fail!!", __func__);
        return NULL;
    }

    /* task attribute*/
    task_handle->max_queue_size = MAX_MSG_QUEUE_SIZE;
    task_handle->task_stack_size = LOCAL_TASK_STACK_SIZE;
    task_handle->task_priority = LOCAL_TASK_PRIORITY;
    task_handle->task_name = LOCAL_TASK_NAME;
    task_handle->task_scene = TASK_SCENE_DATAPROVIDER;

    task_handle->playback_hwbuf_handle = -1;
    task_handle->capture_hwbuf_handle = -1;
    task_handle->AurisysScenario = AURISYS_SCENARIO_DSP_INVALID;

    // task_handle->reserved_mem_id = DATAPROVIDER_DMA_ID;

    task_handle->task_profile_id = TASK_DATAPROVIDER_TIME_ID1;
    task_handle->task_profile_interval = 60000000;

    // task_handle->mtask_processing_buf_size = local_task_output_buffer_size;
    task_handle->mtask_input_buf_size = local_task_output_buffer_size;
    task_handle->mtask_default_buffer_size = local_task_output_buffer_size;

    task_handle->mtask_output_buf_size = local_task_output_buffer_size;

#ifndef FORCE_ALL_TASK_DEBUG_LEVEL
    task_handle->debug_flag = TASK_DEBUG_DEBUG;
#else
    task_handle->debug_flag = FORCE_ALL_TASK_DEBUG_LEVEL;
#endif

    AUD_LOG_D("%s(), debug level = %d\n", __func__, task_handle->debug_flag);

    /* only assign methods, but not class members here */
    task->constructor       = task_dataprovider_constructor;
    task->destructor        = task_dataprovider_destructor;

    task->create_task_loop  = task_dataprovider_create_task_loop;

    task->recv_message      = task_common_recv_message;
    task->irq_hanlder       = task_dataprovider_irq_handler;
    task->hal_reboot_cbk    = task_dataprovider_hal_reboot_cbk;

    task->task_priv = task_handle;

    return task;
}

void task_dataprovider_delete(AudioTask *task) {
    AUD_LOG_D("%s(+), task_scene = %d", __func__, task->scene);

    int ret = 0;
    ret = deinit_common_task(&task_handle);
    if (ret)
        AUD_LOG_E("%s(), deinit_common_task ret[%d]!!", __func__, ret);

    if (task == NULL) {
        AUD_LOG_E("%s(), task is NULL!!", __func__);
        return;
    }

    AUDIO_FREE(task);
    AUD_LOG_D("%s(-)", __func__);
}

void task_dataprovider_irq_handler(struct AudioTask *this, uint32_t irq_type) {
    struct audio_task_common *task_common = this->task_priv;

    if (task_common == NULL || (task_common->playback_hwbuf_handle < 0 &&
                                task_common->capture_hwbuf_handle < 0)) {
        AUD_LOG_E("%s task_common = %p\n", __func__, task_common);
        if (task_common != NULL)
            AUD_LOG_E("playback_hwbuf_handle = %d capture_hwbuf_handle = %d\n",
                      task_common->playback_hwbuf_handle,
                      task_common->capture_hwbuf_handle);
        return;
    }
    if (this->state == AUDIO_TASK_IDLE) {
        AUD_LOG_E("%s this->state = %d\n", __func__, this->state);
        return;
    }

    audio_dsp_hw_irq(this, task_common->capture_hwbuf_handle);
}

void task_dataprovider_constructor(struct AudioTask *this) {
    AUD_ASSERT(this != NULL);

    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;

    /* assign initial value for class members & alloc private memory here */
    this->scene = task_common->task_scene;
    this->state = AUDIO_TASK_IDLE;

    /* queue */
    this->queue_idx = 0;
    this->num_queue_element = 0;

    this->msg_array = (ipi_msg_t *)AUDIO_MALLOC(
                          task_common->max_queue_size * sizeof(ipi_msg_t));

    AUD_ASSERT(this->msg_array != NULL);
    memset(this->msg_array, 0, task_common->max_queue_size * sizeof(ipi_msg_t));

    this->msg_idx_queue = xQueueCreate(task_common->max_queue_size,
                                       sizeof(uint8_t));
    AUD_ASSERT(this->msg_idx_queue != NULL);

    task_common->mtask_output_buf = (char *)AUDIO_MALLOC(
                                         task_common->mtask_output_buf_size);
    task_common->mtask_input_buf = (char *)AUDIO_MALLOC(
                                         task_common->mtask_input_buf_size);
    AUD_ASSERT(this->msg_idx_queue != NULL);
}

void task_dataprovider_destructor(struct AudioTask *this) {
    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;

    AUD_LOG_D("+%s()", __func__);

    if (task_common->mtask_output_buf != NULL) {
        AUDIO_FREE(task_common->mtask_output_buf);
    }
    if (task_common->mtask_input_buf != NULL) {
        AUDIO_FREE(task_common->mtask_input_buf);
    }

    AUD_ASSERT(this != NULL);
    if (this->msg_array != NULL) {
        AUDIO_FREE(this->msg_array);
    }
}

void task_dataprovider_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type) {
    if (this == NULL || hal_type != 0) {
        AUD_LOG_W("%s(), this %p type %d fail!!\n", __func__, this, hal_type);
        return;
    }
    AUD_LOG_D("%s(), scene %d", __func__, this->scene);
}

