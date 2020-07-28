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

#include "audio_task_a2dp.h"
#include "audio_task_common.h"
#include <audio_ringbuf.h>
#include "audio_a2dp_msg_id.h"
#include "audio_a2dp_hw_reg.h"
#include <sbc_encoder.h>
#include <Shifter_exp.h>
#include <audio_fmt_conv.h>
#include <feature_manager.h>
#include <dvfs_config_parser.h>

static struct audio_task_common *task_handle;

#define MAX_MSG_QUEUE_SIZE (40)
#define LOCAL_TASK_STACK_SIZE (40 * 256)

#define LOCAL_TASK_NAME "aud_a2dp"
#define LOCAL_TASK_PRIORITY (2)
#define local_task_output_buffer_size (A2DP_EENCODED_DATA_SIZE)
#define local_task_input_buffer_size (8192)
#define local_task_bs_buffer_size (8192)

#define CODEC_INFO_SIZE (60)

// P80 TODO: Get btsram addr from Connsys Reg
#define A2DP_BTSRAM_ADDR (0x18092ac0)

#define A2DP_BTSRAM_SIZE (1024)
#define MAX_BT_QUEUE_SIZE (5)

// 40ms buffer in pcm_ringbuf
#define PCM_RINGBUF_SIZE (15360)

#define A2DP_TIMER_PERIOD (20)
#define TIMEOUT_CNT (9)
#define BT_WAKEUP_TIMEOUT_CNT (16)
#define TIMEOUT_CNT2 (2)

#define A2DPBUF_MAX_QUEUE_TIME (180) // ms
#define A2DPBUF_WAKEUP_THRESHOLD_TIME (60) // ms
#define A2DPBUF_MAX_WAKEUP (6)
#define A2DPBUF_MIN_WAKEUP (3)

#define BT_HEADER_SIZE (0)

//#define GET_CODECINFO_FROM_BT
//#define NO_ACCESS_BTSRAM
//#define A2DP_UT_WO_BT
#define NEW_CODEC_INFO
#define GET_SRAMINFO_FROM_REG

#ifdef NO_ACCESS_BTSRAM
static char btsram_dummy[A2DP_EENCODED_DATA_SIZE];
#endif

static struct audio_a2dp_struct *pAudio_a2dp_struct;

static void *pSBCEncoderBufAddr, *pRingbufAddr;
static void *sbcencode_handle;
static int a2dp_codec_type;
static FORMAT_INFO_T format_info;
static struct buf_attr source_attr;
static struct dvfs_swip_config_t dvfs_config;

static TimerHandle_t xA2DPTimer;
static int null_cnt, packet_cnt;
static bool bA2dpIsPaused;

static A2DPBUF_DATA_T *a2dpBuf[A2DPBUF_MAX_ELEMENT];
static A2DPBUF_INDEX_T a2dpBufIndex;
static ENCODE_INFO_T encode_info;
static void *pEncode_inputbuf;
static void *aud_fmt_conv_hdl;
static unsigned long long cur_time, bt_start_time, accu_time, accu_count;
#ifdef A2DP_HW_INTERRUPT_SUPPORT
static unsigned long long send_interrupt_time, rcv_interrupt_time, dif_interrupt_time;
#endif
static char *bt_sram_addr;
SemaphoreHandle_t xSemBTHandshake;
static int mHandshakeState;

enum {
    IDLE = 0,
    WAIT_BT_WAKEUP = 1,
    BT_WAKEUP_DONE = 2,
    WAIT_WRITE_BT_DONE = 3,
    WRITE_BT_DONE = 4,
};

// sbc parameters
static sbc_encoder_initial_parameter_t init_param;
static unsigned int bitpool_min, bitpool_max, cal_bitpool, bitpool, sbc_sample_rate, sbc_frame_pcm_bytes, sbc_frame_pcm_samples;
static unsigned int mtu_per_ms, a2dpbuf_max_queue, a2dpbuf_max_threshold, a2dpbuf_wakeup_bt_threshold;
static BS_BUFFER_T *BSbuf;
static unsigned int sbc_frm_length, sbc_frm_num;
static void task_a2dp_irq_handler(struct AudioTask *this, uint32_t irq_type);
static void task_a2dp_constructor(struct AudioTask *this);
static void task_a2dp_destructor(struct AudioTask *this);
static void clear_a2dp_interrupt(void);
static void task_a2dp_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type);

void task_a2dp_task_loop(void *pvParameters)
{
    AudioTask *this = (AudioTask *)pvParameters;
    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;
    uint8_t local_queue_idx = 0xFF;
    ipi_msg_t *ipi_msg;
    struct Audio_Task_Msg_t uAudioTaskMsg;
    unsigned int input_length, output_length;

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
                task_a2dp_timer_start();
                task_a2dp_encode_init();

                ipi_msg_t ipi_msg_to_task;
                packing_ipimsg(&ipi_msg_to_task, this->scene, AUDIO_IPI_LAYER_TO_DSP,
                               AUDIO_IPI_MSG_ONLY,
                               AUDIO_IPI_MSG_BYPASS_ACK,
                               AUDIO_DSP_TASK_A2DP_DATAREAD,
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

                task_a2dp_encode_deinit();
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

            case AUDIO_DSP_TASK_A2DP_DATAREAD: {
            if ((this->state == AUDIO_TASK_WORKING) && (bA2dpIsPaused == false)) {

            input_length = task_a2dp_get_encode_input_length();
            AUD_ASSERT(input_length > 0);

            task_copyfrom_pcmringbuf(input_length, task_common->mtask_input_buf);

#if 0
            if (task_common->pcmdump_enable == 1) {
                struct ipi_msg_t ipi_msg;
                char *data_buf = task_common->mtask_input_buf;
                uint32_t data_size = input_length;

                audio_send_ipi_msg(&ipi_msg, this->scene,
                                   AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                                   AUDIO_IPI_MSG_BYPASS_ACK,
                                   AUDIO_DSP_TASK_PCMDUMP_DATA,
                                   data_size,
                                   DEBUG_PCMDUMP_IN, //dump point serial number
                                   data_buf);

                AUD_LOG_D("AUDIO_DSP_TASK_A2DP_DATAREAD AUDIO_DSP_TASK_PCMDUMP_DATA in");
            }
#endif
            output_length = task_a2dp_encode_process(input_length); // Get 20ms data each time

            if (task_common->pcmdump_enable == 1) {
                struct ipi_msg_t ipi_msg;
                char *data_buf = task_common->mtask_output_buf;
                uint32_t data_size = output_length;

                audio_send_ipi_msg(&ipi_msg, this->scene,
                                   AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                                   AUDIO_IPI_MSG_BYPASS_ACK,
                                   AUDIO_DSP_TASK_PCMDUMP_DATA,
                                   data_size,
                                   DEBUG_PCMDUMP_RESERVED, //dump point serial number
                                   data_buf);

                AUD_LOG_D("AUDIO_DSP_TASK_A2DP_DATAREAD AUDIO_DSP_TASK_PCMDUMP_DATA out");
            }

            AUD_LOG_D("AUDIO_DSP_TASK_A2DP_DATAREAD ready to task_a2dp_write_process()");
            task_a2dp_write_process();

            ipi_msg_t ipi_msg_to_task;
            packing_ipimsg(&ipi_msg_to_task, this->scene, AUDIO_IPI_LAYER_TO_DSP,
                           AUDIO_IPI_MSG_ONLY,
                           AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_A2DP_DATAREAD,
                           0,
                           0,
                           NULL);

            send_task_message(this, &ipi_msg_to_task, task_common->max_queue_size);
            } else {
                AUD_LOG_W("%s() AUDIO_DSP_TASK_A2DP_DATAREAD this->state=%d, bA2dpIsPaused=%d ", __FUNCTION__,
                          this->state, bA2dpIsPaused);
            }

            break;
            }

            case AUDIO_DSP_TASK_PCMDUMP_ON:
                task_common->pcmdump_enable = uAudioTaskMsg.param1;
                break;

            case AUDIO_DSP_TASK_A2DP_CODECINFO: {
                // param1: codecinfo, param2: hal sample_rate
                AUD_LOG_D("AUDIO_DSP_TASK_A2DP_CODECINFO param1=%u, param2=%u", ipi_msg->param1, ipi_msg->param2);
                task_a2dp_set_codecinfo(ipi_msg->param1, ipi_msg->payload, ipi_msg->param2);
                break;
            }

            case AUDIO_DSP_TASK_A2DP_SUSPEND:
                 AUD_LOG_D("AUDIO_DSP_TASK_A2DP_SUSPEND bA2dpIsPaused=%d", bA2dpIsPaused);
                if (uAudioTaskMsg.param1 == 0) {
                    if (bA2dpIsPaused == 1) {
                        pAudio_a2dp_struct->bt_first_wakeup = true;

                        accu_count = 0;
                        bA2dpIsPaused = 0;

                        ipi_msg_t ipi_msg_to_task;
                        packing_ipimsg(&ipi_msg_to_task, this->scene, AUDIO_IPI_LAYER_TO_DSP,
                                   AUDIO_IPI_MSG_ONLY,
                                   AUDIO_IPI_MSG_BYPASS_ACK,
                                   AUDIO_DSP_TASK_A2DP_DATAREAD,
                                   0,
                                   0,
                                   NULL);
                        send_task_message(this, &ipi_msg_to_task, task_common->max_queue_size);
                    }
                 } else {
                    bA2dpIsPaused = 1;
                }
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

void task_a2dp_create_task_loop(struct AudioTask *this)
{
    /* Note: you can also bypass this function,
             and do kal_xTaskCreate until you really need it.
             Ex: create task after you do get the enable message. */

    BaseType_t xReturn = pdFAIL;

    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;

    xReturn = kal_xTaskCreate(
                  task_a2dp_task_loop,
                  task_common->task_name,
                  task_common->task_stack_size,
                  (void *)this,
                  task_common->task_priority,
                  &this->freertos_task);

    AUD_ASSERT(xReturn == pdPASS);
}

AudioTask *task_a2dp_new(void) {
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
    task_handle->task_scene = TASK_SCENE_A2DP;

    task_handle->playback_hwbuf_handle = -1;
    task_handle->capture_hwbuf_handle = -1;
    task_handle->AurisysScenario = AURISYS_SCENARIO_DSP_INVALID;

    // task_handle->reserved_mem_id = A2DP_DMA_ID;

    task_handle->task_profile_id = TASK_A2DP_TIME_ID1;
    task_handle->task_profile_interval = 60000000;

    // task_handle->mtask_processing_buf_size = local_task_output_buffer_size;
    task_handle->mtask_input_buf_size = local_task_input_buffer_size;
    task_handle->mtask_default_buffer_size = local_task_output_buffer_size;

    task_handle->mtask_output_buf_size = local_task_output_buffer_size;

#ifndef FORCE_ALL_TASK_DEBUG_LEVEL
    task_handle->debug_flag = TASK_DEBUG_DEBUG;
#else
    task_handle->debug_flag = FORCE_ALL_TASK_DEBUG_LEVEL;
#endif

    /* only assign methods, but not class members here */
    task->constructor       = task_a2dp_constructor;
    task->destructor        = task_a2dp_destructor;

    task->create_task_loop  = task_a2dp_create_task_loop;

    task->recv_message      = task_common_recv_message;
    task->irq_hanlder       = task_a2dp_irq_handler;
    task->hal_reboot_cbk    = task_a2dp_hal_reboot_cbk;

    task->task_priv = task_handle;
    AUD_LOG_D("-%s(), debug level = %d\n", __func__, task_handle->debug_flag);

    return task;
}

void task_a2dp_delete(AudioTask *task) {
    AUD_LOG_V("%s(+), task_scene = %d", __func__, task->scene);

    int ret = 0;
    ret = deinit_common_task(&task_handle);
    if (ret)
        AUD_LOG_E("%s(), deinit_common_task ret[%d]!!", __func__, ret);

    if (task == NULL) {
        AUD_LOG_E("%s(), task is NULL!!", __func__);
        return;
    }
    AUDIO_FREE(task);
    AUD_LOG_V("%s(-)", __func__);
}

#ifdef NEW_CODEC_INFO
unsigned int get_sbc_samplerate(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_CODEC_SAMP_FREQ_44:
        output = SBC_ENCODER_SAMPLING_RATE_44100HZ;
        break;
    case A2DP_CODEC_SAMP_FREQ_48:
        output = SBC_ENCODER_SAMPLING_RATE_48000HZ;
        break;
    default:
        AUD_LOG_W("%s() unsupported samplerate=%u ", __func__, input);
        AUD_ASSERT(0);
        break;
    }
    return output;
}

unsigned int get_sbc_ch_mode(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_CODEC_CH_MD_MONO:
        output = SBC_ENCODER_MONO;
        break;
    case A2DP_CODEC_CH_MD_STEREO:
        output = SBC_ENCODER_JOINT_STEREO;
        break;
    default:
        AUD_LOG_W("%s() unsupported ch_mode=%u ", __func__, input);
        AUD_ASSERT(0);
        break;
    }
    return output;
}

unsigned int get_sbc_blocks(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_BLOCKS_4:
        output = SBC_ENCODER_BLOCK_NUMBER_4;
        break;
    case A2DP_SBC_IE_BLOCKS_8:
        output = SBC_ENCODER_BLOCK_NUMBER_8;
        break;
    case A2DP_SBC_IE_BLOCKS_12:
        output = SBC_ENCODER_BLOCK_NUMBER_12;
        break;
    case A2DP_SBC_IE_BLOCKS_16:
        output = SBC_ENCODER_BLOCK_NUMBER_16;
        break;
    default:
        AUD_LOG_W("%s() unsupported blocks=%u ", __func__, input);
        AUD_ASSERT(0);
        break;
    }
    return output;
}

unsigned int get_sbc_subband(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_SUBBAND_4:
        output = SBC_ENCODER_SUBBAND_NUMBER_4;
        break;
    case A2DP_SBC_IE_SUBBAND_8:
        output = SBC_ENCODER_SUBBAND_NUMBER_8;
        break;
    default:
        AUD_LOG_W("%s() unsupported subband=%u ", __func__, input);
        AUD_ASSERT(0);
        break;
    }
    return output;
}

unsigned int get_sbc_alloc_md(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_ALLOC_MD_S:
        output = SBC_ENCODER_SNR;
        break;
    case A2DP_SBC_IE_ALLOC_MD_L:
        output = SBC_ENCODER_LOUDNESS;
        break;
    default:
        AUD_LOG_W("%s() unsupported allocation_md=%u ", __func__, input);
        AUD_ASSERT(0);
        break;
    }
    return output;
}

unsigned int get_sbc_bitpool(unsigned int sample_rate, unsigned int ch_mode, unsigned int encoded_bitrate) {
    unsigned int output;

    switch (sample_rate) {
    case A2DP_CODEC_SAMP_FREQ_44:
        if (ch_mode == A2DP_CODEC_CH_MD_MONO) {
            if (encoded_bitrate > 127000) {
                output = 31;
            } else {
                output = 19;
            }
        } else if (ch_mode == A2DP_CODEC_CH_MD_STEREO) {
            if (encoded_bitrate > 229000) {
                output = 53;
            } else {
                output = 35;
            }
        } else {
            AUD_LOG_W("%s() unsupported encoded_bitrate=%u!, set bitpool to default 53", __func__, encoded_bitrate);
            output = 53;
        }
        break;
    case A2DP_CODEC_SAMP_FREQ_48:
        if (ch_mode == A2DP_CODEC_CH_MD_MONO) {
            if (encoded_bitrate > 132000) {
                output = 29;
            } else {
                output = 18;
            }
        } else if (ch_mode == A2DP_CODEC_CH_MD_STEREO) {
            if (encoded_bitrate > 237000) {
                output = 51;
            } else {
                output = 33;
            }
        } else {
            AUD_LOG_W("%s() unsupported encoded_bitrate=%u!, set bitpool to default 51", __func__, encoded_bitrate);
            output = 51;
        }
        break;
    default:
        AUD_LOG_W("%s() unsupported sample_rate %u!", __func__, sample_rate);
        AUD_ASSERT(0);
        break;
    }
    AUD_LOG_D("%s() bitpool=%u", __func__, output);
    return output;
}

unsigned int sbc_samplerate_mapping(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_CODEC_SAMP_FREQ_44:
        output = 44100;
        break;
    case A2DP_CODEC_SAMP_FREQ_48:
        output = 48000;
        break;
    default:
        AUD_LOG_W("%s() unsupported samplerate=%u ", __func__, input);
        AUD_ASSERT(0);
        break;
    }
    return output;
}

unsigned int sbc_blocks_mapping(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_BLOCKS_4:
        output = 4;
        break;
    case A2DP_SBC_IE_BLOCKS_8:
        output = 8;
        break;
    case A2DP_SBC_IE_BLOCKS_12:
        output = 12;
        break;
    case A2DP_SBC_IE_BLOCKS_16:
        output = 16;
        break;
    default:
        AUD_LOG_W("%s() unsupported blocks=%u ", __func__, input);
        AUD_ASSERT(0);
        break;
    }
    return output;
}

unsigned int sbc_subband_mapping(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_SUBBAND_4:
        output = 4;
        break;
    case A2DP_SBC_IE_SUBBAND_8:
        output = 8;
        break;
    default:
        AUD_LOG_W("%s() unsupported subband=%u ", __func__, input);
        AUD_ASSERT(0);
        break;
    }
    return output;
}

#else
unsigned int get_sbc_samplerate(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_SAMP_FREQ_16:
        output = SBC_ENCODER_SAMPLING_RATE_16000HZ;
        break;
    case A2DP_SBC_IE_SAMP_FREQ_32:
        output = SBC_ENCODER_SAMPLING_RATE_32000HZ;
        break;
    case A2DP_SBC_IE_SAMP_FREQ_44:
        output = SBC_ENCODER_SAMPLING_RATE_44100HZ;
        break;
    case A2DP_SBC_IE_SAMP_FREQ_48:
        output = SBC_ENCODER_SAMPLING_RATE_48000HZ;
        break;
    default:
        output = SBC_ENCODER_SAMPLING_RATE_44100HZ;
        break;
    }
    return output;
}

unsigned int get_sbc_ch_mode(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_CH_MD_MONO:
        output = SBC_ENCODER_MONO;
        break;
    case A2DP_SBC_IE_CH_MD_DUAL:
        output = SBC_ENCODER_DUAL_CHANNEL;
        break;
    case A2DP_SBC_IE_CH_MD_STEREO:
        output = SBC_ENCODER_STEREO;
        break;
    case A2DP_SBC_IE_CH_MD_JOINT:
        output = SBC_ENCODER_JOINT_STEREO;
        break;
    default:
        output = SBC_ENCODER_JOINT_STEREO;
        break;
    }
    return output;
}

unsigned int get_sbc_blocks(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_BLOCKS_4:
        output = SBC_ENCODER_BLOCK_NUMBER_4;
        break;
    case A2DP_SBC_IE_BLOCKS_8:
        output = SBC_ENCODER_BLOCK_NUMBER_8;
        break;
    case A2DP_SBC_IE_BLOCKS_12:
        output = SBC_ENCODER_BLOCK_NUMBER_12;
        break;
    case A2DP_SBC_IE_BLOCKS_16:
        output = SBC_ENCODER_BLOCK_NUMBER_16;
        break;
    default:
        output = SBC_ENCODER_BLOCK_NUMBER_16;
        break;
    }
    return output;
}

unsigned int get_sbc_subband(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_SUBBAND_4:
        output = SBC_ENCODER_SUBBAND_NUMBER_4;
        break;
    case A2DP_SBC_IE_SUBBAND_8:
        output = SBC_ENCODER_SUBBAND_NUMBER_8;
        break;
    default:
        output = SBC_ENCODER_SUBBAND_NUMBER_8;
        break;
    }
    return output;
}

unsigned int get_sbc_alloc_md(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_ALLOC_MD_S:
        output = SBC_ENCODER_SNR;
        break;
    case A2DP_SBC_IE_ALLOC_MD_L:
        output = SBC_ENCODER_LOUDNESS;
        break;
    default:
        output = SBC_ENCODER_LOUDNESS;
        break;
    }
    return output;
}

unsigned int get_sbc_bitpool(unsigned int sample_rate, unsigned int ch_mode) {
    unsigned int output;

    switch (sample_rate) {
    case A2DP_SBC_IE_SAMP_FREQ_44:
        if (ch_mode == A2DP_SBC_IE_CH_MD_MONO)
            output = 31;
        else if (ch_mode == A2DP_SBC_IE_CH_MD_JOINT)
            output = 53;
        break;
    case A2DP_SBC_IE_SAMP_FREQ_48:
        if (ch_mode == A2DP_SBC_IE_CH_MD_MONO)
            output = 29;
        else if (ch_mode == A2DP_SBC_IE_CH_MD_JOINT)
            output = 51;
        break;
    default:
        output = 0;
        break;
    }
    return output;
}

unsigned int sbc_samplerate_mapping(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_SAMP_FREQ_16:
        output = 16000;
        break;
    case A2DP_SBC_IE_SAMP_FREQ_32:
        output = 32000;
        break;
    case A2DP_SBC_IE_SAMP_FREQ_44:
        output = 44100;
        break;
    case A2DP_SBC_IE_SAMP_FREQ_48:
        output = 48000;
        break;
    default:
        output = 48000;
        break;
    }
    return output;
}

unsigned int sbc_blocks_mapping(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_BLOCKS_4:
        output = 4;
        break;
    case A2DP_SBC_IE_BLOCKS_8:
        output = 8;
        break;
    case A2DP_SBC_IE_BLOCKS_12:
        output = 12;
        break;
    case A2DP_SBC_IE_BLOCKS_16:
        output = 16;
        break;
    default:
        output = 16;
        break;
    }
    return output;
}

unsigned int sbc_subband_mapping(unsigned int input) {
    unsigned int output;

    switch (input) {
    case A2DP_SBC_IE_SUBBAND_4:
        output = 4;
        break;
    case A2DP_SBC_IE_SUBBAND_8:
        output = 8;
        break;
    default:
        output = 8;
        break;
    }
    return output;
}
#endif

void a2dp_structure_init(void) {
    int i;
    AUD_LOG_D("+%s()", __func__);
    pAudio_a2dp_struct = kal_pvPortMalloc(sizeof(struct audio_a2dp_struct));
    pAudio_a2dp_struct->pcm_ring_buf = kal_pvPortMalloc(sizeof(struct RingBuf));
    pRingbufAddr = kal_pvPortMalloc(PCM_RINGBUF_SIZE);
    memset(pRingbufAddr, 0, PCM_RINGBUF_SIZE);
    init_ring_buf(pAudio_a2dp_struct->pcm_ring_buf, pRingbufAddr, PCM_RINGBUF_SIZE);

    // Create Semaphore
    pAudio_a2dp_struct->xA2DPSemphrTX = NULL;
    pAudio_a2dp_struct->xA2DPSemphrTX = xSemaphoreCreateCounting(1, 0);
    if (pAudio_a2dp_struct->xA2DPSemphrTX == NULL) {
        AUD_LOG_W("+%s() xA2DPSemphrTX create fail!", __func__);
    }
    AUD_LOG_D("a2dp_structure_init() xA2DPSemphrTX=%p", pAudio_a2dp_struct->xA2DPSemphrTX);

    pAudio_a2dp_struct->xPCMRingBufSemphr = NULL;
    pAudio_a2dp_struct->xPCMRingBufSemphr = xSemaphoreCreateCounting(1, 0);
    if (pAudio_a2dp_struct->xPCMRingBufSemphr == NULL) {
        AUD_LOG_W("+%s() xPCMRingBufSemphr create fail!", __func__);
    }
    AUD_LOG_D("a2dp_structure_init() xPCMRingBufSemphr=%p", pAudio_a2dp_struct->xPCMRingBufSemphr);

    pAudio_a2dp_struct->xPCMRingBufSemphr2 = NULL;
    pAudio_a2dp_struct->xPCMRingBufSemphr2 = xSemaphoreCreateCounting(1, 0);
    if (pAudio_a2dp_struct->xPCMRingBufSemphr2 == NULL) {
        AUD_LOG_W("+%s() xPCMRingBufSemphr2 create fail!", __func__);
    }
    AUD_LOG_D("a2dp_structure_init() xPCMRingBufSemphr2=%p", pAudio_a2dp_struct->xPCMRingBufSemphr2);

    xSemBTHandshake = NULL;
    xSemBTHandshake = xSemaphoreCreateCounting(1, 0);
    if (xSemBTHandshake == NULL) {
        AUD_LOG_W("+%s() xSemBTHandshake create fail!", __func__);
    }
    AUD_LOG_D("a2dp_structure_init() xSemBTHandshake=%p", xSemBTHandshake);

    for (i = 0; i < A2DPBUF_MAX_ELEMENT; i++) {
       a2dpBuf[i] = (A2DPBUF_DATA_T *)kal_pvPortMalloc(sizeof(A2DPBUF_DATA_T));
       memset(a2dpBuf[i], 0, sizeof(A2DPBUF_DATA_T));
       AUD_LOG_D("a2dpBuf[%d] addr %p", i, a2dpBuf[i]);
    }
    xA2DPTimer = NULL;
    AUD_LOG_D("-%s()", __func__);
}

void a2dp_structure_deinit(void) {
    int i;
    AUD_LOG_D("+%s()", __func__);
    kal_vPortFree(pRingbufAddr);
    kal_vPortFree(pAudio_a2dp_struct->pcm_ring_buf);
    kal_vPortFree(pAudio_a2dp_struct->xPCMRingBufSemphr);
    kal_vPortFree(pAudio_a2dp_struct->xPCMRingBufSemphr2);
    kal_vPortFree(pAudio_a2dp_struct->xA2DPSemphrTX);
    kal_vPortFree(pAudio_a2dp_struct);
    kal_vPortFree(xSemBTHandshake);
    for (i = 0; i < A2DPBUF_MAX_ELEMENT; i++) {
        if (a2dpBuf[i] != NULL) {
            kal_vPortFree(a2dpBuf[i]);
        }
    }
    AUD_LOG_D("-%s()", __func__);
}

void a2dp_encoder_common_init(void) {
    int i;
    AUD_LOG_D("+%s()", __func__);
    BSbuf = kal_pvPortMalloc(sizeof(BS_BUFFER_T));
    BSbuf->pBufAddr = kal_pvPortMalloc(local_task_bs_buffer_size);
    BSbuf->u4BufferSize = local_task_bs_buffer_size;
    BSbuf->pWriteIdx = BSbuf->pBufAddr;
    BSbuf->pReadIdx = BSbuf->pBufAddr;
    BSbuf->u4DataRemained = 0;
    BSbuf->consumedBS= 0;
    memset(BSbuf->pBufAddr, 0, local_task_bs_buffer_size);
    pEncode_inputbuf = kal_pvPortMalloc(local_task_input_buffer_size >> 1);
    memset(pEncode_inputbuf, 0, local_task_input_buffer_size >> 1);
    memset(&a2dpBufIndex, 0, sizeof(a2dpBufIndex));
    for (i = 0; i < A2DPBUF_MAX_ELEMENT; i++) {
       memset(a2dpBuf[i], 0, sizeof(A2DPBUF_DATA_T));
    }
    AUD_LOG_D("-%s()", __func__);
}

void a2dp_encoder_common_deinit(void) {
    AUD_LOG_D("+%s()", __func__);
    kal_vPortFree(pEncode_inputbuf);
    kal_vPortFree(BSbuf->pBufAddr);
    kal_vPortFree(BSbuf);
    AUD_LOG_D("-%s()", __func__);
}

void sbc_encoder_param_init(void) {
    AUD_LOG_D("+%s()", __func__);

    sbcencode_handle = NULL;

    unsigned int internal_buffer_size = sbc_encoder_get_buffer_size();
    pSBCEncoderBufAddr = kal_pvPortMalloc(internal_buffer_size);
    memset(pSBCEncoderBufAddr, 0, internal_buffer_size);

#ifdef NEW_CODEC_INFO
    init_param.sampling_rate     = get_sbc_samplerate(format_info.sample_rate);
    init_param.channel_mode      = get_sbc_ch_mode(format_info.ch_mode);
    init_param.block_number 	 = get_sbc_blocks(format_info.codec_info[0] & A2DP_SBC_IE_BLOCKS_MSK);
    init_param.subband_number	 = get_sbc_subband(format_info.codec_info[0] & A2DP_SBC_IE_SUBBAND_MSK);
    init_param.allocation_method = get_sbc_alloc_md(format_info.codec_info[0] & A2DP_SBC_IE_ALLOC_MD_MSK);
    bitpool_min = format_info.codec_info[1];
    bitpool_max = format_info.codec_info[2];

    cal_bitpool = get_sbc_bitpool(format_info.sample_rate, format_info.ch_mode, format_info.encoded_audio_bitrate);
    bitpool = (cal_bitpool > bitpool_max) ? bitpool_max : cal_bitpool;
    bitpool = (cal_bitpool < bitpool_min) ? bitpool_min : cal_bitpool;

    sbc_sample_rate = sbc_samplerate_mapping(format_info.sample_rate);

    unsigned int nrof_subbands, nrof_blocks, nrof_channels, join, bitrate;

    // calculate sbc frame length
    nrof_subbands = sbc_subband_mapping(format_info.codec_info[0] & A2DP_SBC_IE_SUBBAND_MSK);
    nrof_blocks = sbc_blocks_mapping(format_info.codec_info[0] & A2DP_SBC_IE_BLOCKS_MSK);
#else
    init_param.sampling_rate	 = get_sbc_samplerate(format_info.codec_info[0] & A2DP_SBC_IE_SAMP_FREQ_MSK);
    init_param.channel_mode 	 = get_sbc_ch_mode(format_info.codec_info[0] & A2DP_SBC_IE_CH_MD_MSK);
    init_param.block_number 	 = get_sbc_blocks(format_info.codec_info[1] & A2DP_SBC_IE_BLOCKS_MSK);
    init_param.subband_number	 = get_sbc_subband(format_info.codec_info[1] & A2DP_SBC_IE_SUBBAND_MSK);
    init_param.allocation_method = get_sbc_alloc_md(format_info.codec_info[1] & A2DP_SBC_IE_ALLOC_MD_MSK);
    bitpool_min = format_info.codec_info[2];
    bitpool_max = format_info.codec_info[3];

    bitpool = get_sbc_bitpool(format_info.codec_info[0] & A2DP_SBC_IE_SAMP_FREQ_MSK,
                              format_info.codec_info[0] & A2DP_SBC_IE_CH_MD_MSK);

    int i;
    for(i = 0; i < 4; i++) {
        AUD_LOG_D("codec_info[%d]=0x%x", i, format_info.codec_info[i]);
    }
    AUD_ASSERT(bitpool <= bitpool_max);
    AUD_ASSERT(bitpool >= bitpool_min);

    sbc_sample_rate = sbc_samplerate_mapping(format_info.codec_info[0] & A2DP_SBC_IE_SAMP_FREQ_MSK);

    unsigned int nrof_subbands, nrof_blocks, nrof_channels, join, bitrate;

    // calculate sbc frame length
    nrof_subbands = sbc_subband_mapping(format_info.codec_info[1] & A2DP_SBC_IE_SUBBAND_MSK);
    nrof_blocks = sbc_blocks_mapping(format_info.codec_info[1] & A2DP_SBC_IE_BLOCKS_MSK);
#endif

    if (init_param.channel_mode == SBC_ENCODER_MONO)
        nrof_channels = 1;
    else
        nrof_channels = 2;

    sbc_frm_length = 4 + (nrof_subbands * nrof_channels >> 1);
        if (init_param.channel_mode == SBC_ENCODER_MONO || init_param.channel_mode == SBC_ENCODER_DUAL_CHANNEL) {
            sbc_frm_length += (nrof_blocks * nrof_channels * bitpool + 7) / 8;
    }
    else {
        join = (init_param.channel_mode == SBC_ENCODER_JOINT_STEREO) ? 1 : 0;
        sbc_frm_length += (join * nrof_subbands + nrof_blocks * bitpool + 7) / 8;
    }
    bitrate = 8 * sbc_frm_length * sbc_sample_rate / nrof_subbands / nrof_blocks;
    sbc_frame_pcm_bytes = nrof_subbands * nrof_blocks * nrof_channels * 2;
    sbc_frame_pcm_samples = nrof_subbands * nrof_blocks;

    AUD_LOG_D("bitpool_min=%u, bitpool_max=%u, cal_bitpool=%u, bitpool=%u", bitpool_min, bitpool_max, cal_bitpool, bitpool);
    AUD_LOG_D("nrof_subbands=%u, nrof_blocks=%u, nrof_channels=%u, alloc_md=%u", nrof_subbands, nrof_blocks, nrof_channels, init_param.allocation_method);
    AUD_LOG_D("sbc_sample_rate=%u, join=%u, sbc_frm_length=%u, bitrate=%u", sbc_sample_rate, join, sbc_frm_length, bitrate);
    AUD_LOG_D("sbc_frame_pcm_bytes=%u", sbc_frame_pcm_bytes);

    mtu_per_ms = sbc_sample_rate /sbc_frame_pcm_samples/((format_info.mtu - BT_HEADER_SIZE)/sbc_frm_length);
    a2dpbuf_max_queue = A2DPBUF_MAX_QUEUE_TIME * mtu_per_ms / 1000;
    if (a2dpbuf_max_queue > A2DPBUF_MAX_ELEMENT) {
        AUD_LOG_W("a2dpbuf_max_queue = %u too big, limit it to %u!", a2dpbuf_max_queue, A2DPBUF_MAX_ELEMENT);
        a2dpbuf_max_queue = A2DPBUF_MAX_ELEMENT;
    } else if (a2dpbuf_max_queue < A2DPBUF_MIN_ELEMENT) {
        AUD_LOG_W("a2dpbuf_max_queue = %u too small, limit it to %u!", a2dpbuf_max_queue, A2DPBUF_MIN_ELEMENT);
        a2dpbuf_max_queue = A2DPBUF_MIN_ELEMENT;
    }

    a2dpbuf_max_threshold = a2dpbuf_max_queue - 4;

    a2dpbuf_wakeup_bt_threshold = A2DPBUF_WAKEUP_THRESHOLD_TIME * mtu_per_ms / 1000;
    if (a2dpbuf_wakeup_bt_threshold > A2DPBUF_MAX_WAKEUP) {
        AUD_LOG_W("a2dpbuf_wakeup_bt_threshold = %u too big, limit it to %u!", a2dpbuf_wakeup_bt_threshold, A2DPBUF_MAX_WAKEUP);
        a2dpbuf_wakeup_bt_threshold = A2DPBUF_MAX_WAKEUP;
    } else if (a2dpbuf_wakeup_bt_threshold < A2DPBUF_MIN_WAKEUP) {
        AUD_LOG_W("a2dpbuf_wakeup_bt_threshold = %u too small, limit it to %u!", a2dpbuf_wakeup_bt_threshold, A2DPBUF_MIN_WAKEUP);
        a2dpbuf_wakeup_bt_threshold = A2DPBUF_MIN_WAKEUP;
    }

    AUD_LOG_D("mtu_per_ms=%u, a2dpbuf_max_queue=%u, a2dpbuf_max_threshold=%u, a2dpbuf_wakeup_bt_threshold=%u",
               mtu_per_ms, a2dpbuf_max_queue, a2dpbuf_max_threshold, a2dpbuf_wakeup_bt_threshold);

    // TODO: get format/codec info from audio hal
    struct aud_fmt_cfg_t source;
    struct aud_fmt_cfg_t target;

    source.audio_format = 4; // 4: AUDIO_FORMAT_PCM_8_24_BIT
    source.num_channels = 2;
    source.sample_rate  = source_attr.rate;

    target.audio_format = 1; // 1: AUDIO_FORMAT_PCM_16_BIT
    target.num_channels = 2;
    target.sample_rate  = sbc_sample_rate;

    AUD_LOG_D("source: format %u, ch %u, sample_rate %u", source.audio_format, source.num_channels ,source.sample_rate);
    AUD_LOG_D("target: format %u, ch %u, sample_rate %u", target.audio_format, target.num_channels ,target.sample_rate);

    aud_fmt_conv_create(&source, &target, &aud_fmt_conv_hdl);

    AUD_LOG_D("-%s()", __func__);
}

void sbc_encoder_param_deinit(void) {
    AUD_LOG_D("+%s()", __func__);
    kal_vPortFree(pSBCEncoderBufAddr);
    aud_fmt_conv_destroy(aud_fmt_conv_hdl);
    AUD_LOG_D("-%s()", __func__);
}

void task_a2dp_encode_init(void) {
    AUD_LOG_D("+%s()", __func__);

    int i;
#ifdef NO_ACCESS_BTSRAM
    bt_sram_addr = btsram_dummy;
#else
#if defined(GET_CODECINFO_FROM_BT) && !defined(GET_SRAMINFO_FROM_REG)
    bt_sram_addr = (char *)A2DP_BTSRAM_ADDR;
#endif
#endif
#ifdef GET_CODECINFO_FROM_BT
    int cnt = 0;
    // waiting until BT side fill codec info
    do {
        aud_task_delay(1);	// wait 1 ms
        cnt++;
    } while((*bt_sram_addr != 0xFF) && (cnt < 30));

    /* put hifi3 in memw */
    __asm(
    "memw;\n"
    );

    AUD_LOG_D("cnt = %d", cnt);
    for(i = 0; i < 100; i++) {
        AUD_LOG_D("%d: %x", i, *(bt_sram_addr + i));
    }

    memcpy(&format_info, (void *)(bt_sram_addr + 12), sizeof(FORMAT_INFO_T));

    /* put hifi3 in memw */
    __asm(
    "memw;\n"
    );

    *(bt_sram_addr + 1) = 1; // set DSP done = 1
#endif

#ifndef NEW_CODEC_INFO
    unsigned char tmp[] = {0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                           0x1, 0x00, 0x0, 0x0,  // codec type
                           0x1, 0x0, 0x0, 0x0,   //sample rate
                           0x40, 0x01,0x05,0x0, //bitrate 0x50140=328k
                           0x0, 0x0,
                           0x0, 0x0,
                           0x33, 0x0, //acl
                           0x42, 0x0, //l2c_rcid
                           0x94, 0x2, //mtu
                           0x1, //bits per sample
                           0x2, // ch mode
                           0x15,0x02,0x35,0x0 // codec indo 0 ~ 3
                           0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                           0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
    };
    memcpy(&format_info, (void *)(tmp + 12), sizeof(FORMAT_INFO_T));
#endif
    char *ptr = (char *)(&format_info);
    for(i = 0; i < sizeof(FORMAT_INFO_T); i++) {
        AUD_LOG_D("%d: %x", i, *(ptr + i));
    }

    a2dp_codec_type = format_info.codec_type; //1: SBC, 2: AAC

    // codec init
    a2dp_encoder_common_init();

    if(a2dp_codec_type == 1) { // SBC codec
        sbc_encoder_param_init();
        sbc_encoder_init(&sbcencode_handle, pSBCEncoderBufAddr, &init_param);

        memset(&dvfs_config, 0, sizeof(struct dvfs_swip_config_t));
        dvfs_config.p_config = (void *)(&init_param);
        dvfs_config.dl_out.sample_rate = sbc_sample_rate;
        dvfs_add_swip(A2DP_PLAYBACK_FEATURE_ID, SWIP_ID_CODEC_SBC_ENCODER, &dvfs_config);
    } else if(a2dp_codec_type == 2) { // TODO:AAC codec
        // aac_encoder_param_init(void);
    } else {
        AUD_LOG_E("%s() unsupported a2dp_codec_type=%d !", __func__, a2dp_codec_type);
        AUD_ASSERT(0);
    }
    mHandshakeState = IDLE;

    AUD_LOG_D("-%s()", __func__);
}

void task_a2dp_encode_deinit(void) {
    AUD_LOG_D("+%s()", __func__);

    a2dp_encoder_common_deinit();

    // codec deinit
    if(a2dp_codec_type == 1) { // SBC codec
        sbc_encoder_param_deinit();
        dvfs_delete_swip(A2DP_PLAYBACK_FEATURE_ID, SWIP_ID_CODEC_SBC_ENCODER, &dvfs_config);
    } else if(a2dp_codec_type == 2) { // AAC codec

    } else {
        AUD_LOG_E("%s() unsupported a2dp_codec_type=%d !", __func__, a2dp_codec_type);
        AUD_ASSERT(0);
    }

    task_a2dp_timer_stop();
    clear_a2dp_interrupt();
    AUD_LOG_D("-%s()", __func__);
}

unsigned int task_a2dp_encode_process(unsigned int input_length) {
    AUD_LOG_D("+%s()", __func__);
    int result;
    sbc_encoder_runtime_parameter_t runtime_parameter;
    char *bs_ptr, *ptr;
    void *p_in_buf_16, *p_in_buf_24;
    uint32_t bs_size, input_size, total_output_size;
    uint32_t output_buf_size = task_handle->mtask_output_buf_size;
    uint32_t raw_size, bcv_size;

    encode_info.input_length = input_length;
    runtime_parameter.bitpool_value = bitpool;
    AUD_LOG_D("runtime_parameter.bitpool_value=%u", runtime_parameter.bitpool_value);

    p_in_buf_24 = (void *)(task_handle->mtask_input_buf);
    p_in_buf_16 = pEncode_inputbuf;
    memset(p_in_buf_16, 0, (local_task_input_buffer_size >> 1));

    AUD_LOG_V("p_in_buf_24=%p, p_in_buf_16=%p", p_in_buf_24, p_in_buf_16);

    bs_ptr = (char *)(BSbuf->pBufAddr);
    raw_size = encode_info.input_length;
    bcv_size = local_task_input_buffer_size >> 1;

    // use formart converter according to src/target format info from hal
    void *bcv_buf;
    aud_fmt_conv_process(p_in_buf_24, raw_size, &bcv_buf, &bcv_size, aud_fmt_conv_hdl);
    memcpy(p_in_buf_16, bcv_buf, bcv_size);

    if (task_handle->pcmdump_enable == 1) {
        struct ipi_msg_t ipi_msg;
        char *data_buf = (char *)p_in_buf_16;
        uint32_t data_size = bcv_size;

        audio_send_ipi_msg(&ipi_msg, TASK_SCENE_A2DP,
                           AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                           AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_PCMDUMP_DATA,
                           data_size,
                           DEBUG_PCMDUMP_IN, //dump point serial number
                           data_buf);

        AUD_LOG_D("AUDIO_DSP_TASK_A2DP_DATAREAD AUDIO_DSP_TASK_PCMDUMP_DATA reserved");
    }

    AUD_LOG_D("%s() bcv_size=%u, BSbuf->u4DataRemained=%d", __func__, bcv_size, BSbuf->u4DataRemained);
    AUD_ASSERT((bcv_size + BSbuf->u4DataRemained) <= BSbuf->u4BufferSize);

    memcpy(&bs_ptr[BSbuf->u4DataRemained], p_in_buf_16, bcv_size);
    input_size = BSbuf->u4DataRemained + bcv_size;
    bs_size = input_size;

    output_buf_size = format_info.mtu - BT_HEADER_SIZE; // subtract AVDTP/MediaPacket/L2CAP header size;
    AUD_LOG_D("set output_buf_size to mtu size %u", output_buf_size);
    ptr = task_handle->mtask_output_buf;
    total_output_size = 0;

    if(a2dp_codec_type == 1) { // SBC codec
        do {
            result = sbc_encoder_process(sbcencode_handle, (int16_t *)bs_ptr, &bs_size, (uint8_t *)ptr, &output_buf_size, &runtime_parameter);
            AUD_LOG_D("sbc consumed bs_size=%u, output_buf_size=%u", bs_size, output_buf_size);
            int residual = output_buf_size % sbc_frm_length;
            AUD_ASSERT(residual == 0);
            sbc_frm_num = output_buf_size/sbc_frm_length;
            encode_info.frame_length = sbc_frm_length;
            encode_info.frame_num = sbc_frm_num;

            ptr += output_buf_size;
            total_output_size += output_buf_size;

            BSbuf->u4DataRemained = input_size - bs_size; // remained size = total input size - consumed size
            memmove(bs_ptr, &bs_ptr[bs_size], BSbuf->u4DataRemained);
            bs_size = BSbuf->u4DataRemained;
            input_size = BSbuf->u4DataRemained;
        } while (BSbuf->u4DataRemained >= sbc_frame_pcm_bytes * ((format_info.mtu - BT_HEADER_SIZE)/sbc_frm_length));
        AUD_LOG_D("sbc sbc_frm_length=%u, sbc_frm_num=%u, total_output_size=%u", sbc_frm_length, sbc_frm_num, total_output_size);
    }
    else if(a2dp_codec_type == 2) { // TODO: AAC codec
        // aac_encoder_process(aacencode_handle, (int16_t *)bs_ptr, &bs_size, (uint8_t *)task_handle->mtask_output_buf, &output_buf_size, &runtime_parameter);
        // BSbuf->u4DataRemained = input_size - bs_size; // remained size = total input size - consumed size
        // memmove(bs_ptr, &bs_ptr[bs_size], BSbuf->u4DataRemained);
    }
    else {
        AUD_LOG_E("%s() unsupported a2dp_codec_type=%d !", __func__, a2dp_codec_type);
        AUD_ASSERT(0);
    }
    encode_info.output_length = total_output_size;

    AUD_ASSERT(encode_info.output_length <= task_handle->mtask_output_buf_size);
    AUD_LOG_D("-%s() BSbuf->u4DataRemained=%d", __func__, BSbuf->u4DataRemained);
    return encode_info.output_length;
}

int task_a2dp_get_encode_input_length(void) {
    // return size of 8_24 data format
    if(a2dp_codec_type == 1) { // SBC codec
        #define SBC_ENCODER_INTERVAL_MS (20)
        return (source_attr.rate * 8 * SBC_ENCODER_INTERVAL_MS / 1000); // Get 20ms data for each sbc input frame
    }
    else if(a2dp_codec_type == 2) { // AAC codec
        #define AAC_ENCODER_INPUT_SAMPLES (1024)
        return (AAC_ENCODER_INPUT_SAMPLES * 8);
    }
    else {
        AUD_LOG_E("%s() unsupported a2dp_codec_type=%d !", __func__, a2dp_codec_type);
        AUD_ASSERT(0);
        return 0;
    }
}

void task_a2dp_write_process(void)
{
    int i, mtu_num;
    AUD_LOG_D("+%s()", __func__);

    int residual = encode_info.output_length % (encode_info.frame_length * encode_info.frame_num);
    AUD_ASSERT(residual == 0);

    mtu_num = encode_info.output_length / (encode_info.frame_length * encode_info.frame_num);
    AUD_LOG_D("%s() mtu_num = %u", __func__, mtu_num);

    char *ptr = task_handle->mtask_output_buf;

    for (i = 0; i < mtu_num; i++) {
    // write to a2dp bitstream buffer and update r/w index, count
    taskENTER_CRITICAL();
    unsigned int index = a2dpBufIndex.w_index;

    a2dpBuf[index]->frame_num = encode_info.frame_num;
    a2dpBuf[index]->frame_length = encode_info.frame_length;
    a2dpBuf[index]->encoded_data_length = encode_info.frame_length * encode_info.frame_num;
    a2dpBuf[index]->mtu_num = 1;

    memcpy(a2dpBuf[index]->encoded_data, ptr, a2dpBuf[index]->encoded_data_length);

    hal_cache_region_operation (HAL_CACHE_TYPE_DATA , HAL_CACHE_OPERATION_FLUSH, (void *)(a2dpBuf[index]->encoded_data), A2DP_EENCODED_DATA_SIZE);

    ptr += a2dpBuf[index]->encoded_data_length;
    a2dpBufIndex.count++;
    a2dpBufIndex.w_index++;

    if (a2dpBufIndex.w_index == a2dpbuf_max_queue)
        a2dpBufIndex.w_index -= a2dpbuf_max_queue;

    if (a2dpBufIndex.count == a2dpbuf_max_queue) { // when a2dp buffer full
        AUD_LOG_W("a2dpBuf full, overwrite oldest data!");
        // buffer full, overwrite the oldest data.
        a2dpBufIndex.r_index++;
        if (a2dpBufIndex.r_index == a2dpbuf_max_queue) {
            a2dpBufIndex.r_index -= a2dpbuf_max_queue;
        }

        a2dpBufIndex.count--;
        AUD_LOG_W("a2dpBufIndex.count=%d, w_index=%d, r_index=%d", a2dpBufIndex.count, a2dpBufIndex.w_index, a2dpBufIndex.r_index);
    }
    taskEXIT_CRITICAL();
    }

    AUD_LOG_D("%s() a2dpBufIndex.count=%d, w_index=%d, r_index=%d", __func__, a2dpBufIndex.count, a2dpBufIndex.w_index, a2dpBufIndex.r_index);

    if (a2dpBufIndex.count >= a2dpbuf_max_threshold) {
        BaseType_t xResult;
        xResult = xSemaphoreTake(pAudio_a2dp_struct->xA2DPSemphrTX, pdMS_TO_TICKS(30)); // semaphone give by vA2DPTimerCallback
        if(xResult == pdTRUE) {
            AUD_LOG_D("-AUDIO_DSP_TASK_A2DP_DATAREAD leave xA2DPSemphrTX normally");
        }
        else {
            AUD_LOG_D("-AUDIO_DSP_TASK_A2DP_DATAREAD leave xA2DPSemphrTX TIMEOUT 30ms!");
        }
    }

    AUD_LOG_D("-%s()", __func__);
}

void *get_a2dp_structure(void) {
    AUD_LOG_V("+%s() pAudio_a2dp_struct=0x%p", __func__, pAudio_a2dp_struct);
    return pAudio_a2dp_struct;
}

void task_copyfrom_pcmringbuf(unsigned int length, char *buf) {
    int ret = 0, wait_cnt = 0, threshold = 0;
    unsigned int spaces;

    struct audio_a2dp_struct *pAudio_a2dp_struct;
    pAudio_a2dp_struct = (struct audio_a2dp_struct *)get_a2dp_structure();
    AUD_LOG_D("%s() pcm_ring_buf=%p, length=%u", __func__, pAudio_a2dp_struct->pcm_ring_buf, length);

    threshold = 10;
    while (ret < length && wait_cnt < threshold) {
        wait_cnt++;
        spaces = RingBuf_getDataCount(pAudio_a2dp_struct->pcm_ring_buf);
        if (spaces >= length) {
            RingBuf_copyToLinear(buf, pAudio_a2dp_struct->pcm_ring_buf, length);
            ret = length;
            AUD_LOG_D("%s() copy from pcm_ringbuf ret=%d, length=%d, data=%u", __func__, ret, length, spaces);
            AUD_LOG_D("%s() enter xPCMRingBufSemphr=%p", __func__, pAudio_a2dp_struct->xPCMRingBufSemphr);
            xSemaphoreGive(pAudio_a2dp_struct->xPCMRingBufSemphr);
        }
        else {
            BaseType_t xResult;
            xResult = xSemaphoreTake(pAudio_a2dp_struct->xPCMRingBufSemphr2, pdMS_TO_TICKS(20));
            if(xResult == pdTRUE) {
                AUD_LOG_D("%s() leave xPCMRingBufSemphr2 normally", __func__);
            }
            else {
                AUD_LOG_D("%s() leave xPCMRingBufSemphr2 TIMEOUT 20ms!", __func__);
            }
        }
    }

    if (wait_cnt >= threshold) {
        AUD_LOG_W("%s() wait_cnt > threshold(%d)!", __func__, threshold);
    }
}

void task_a2dp_constructor(struct AudioTask *this) {
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
    a2dp_structure_init();
    AUD_ASSERT(this->msg_idx_queue != NULL);
}

void task_a2dp_destructor(struct AudioTask *this) {
    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;

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
    a2dp_structure_deinit();
    AUD_LOG_D("-%s()", __func__);
}

void task_a2dp_set_codecinfo(uint32_t size_info, void *info, uint32_t sample_rate) {

    AUD_ASSERT(size_info == sizeof(FORMAT_INFO_T));
    memcpy(&format_info, info, size_info);
    source_attr.rate = sample_rate;
}


void send_a2dp_interrupt(void) {
#ifdef A2DP_HW_INTERRUPT_SUPPORT
    send_interrupt_time = get_time_stamp();

    ADSP_GENERAL_IRQ_SET = A2DP_IRQ_TO_CONNSYS_BIT;
    AUD_LOG_D("%s() ADSP_GENERAL_IRQ_SET=%x", __func__, ADSP_GENERAL_IRQ_SET);
#endif
}

void clear_a2dp_interrupt(void) {
#ifdef A2DP_HW_INTERRUPT_SUPPORT
    ADSP_GENERAL_IRQ_CLR = A2DP_IRQ_TO_CONNSYS_BIT;
    AUD_LOG_D("%s() ADSP_GENERAL_IRQ_SET=%x", __func__, ADSP_GENERAL_IRQ_SET);
#endif
}

void clear_connsys_a2dp_interrupt(void) {
#ifdef A2DP_HW_INTERRUPT_SUPPORT
    rcv_interrupt_time = get_time_stamp();
    dif_interrupt_time = rcv_interrupt_time - send_interrupt_time;

    A2DP_IRQ_CONNSYS_REG |= A2DP_IRQ_CONNSYS_BITMASK ;
    AUD_LOG_D("%s() dif_int_time=%lld, ADSP_GENERAL_IRQ_SET=%x", __func__, dif_interrupt_time, ADSP_GENERAL_IRQ_SET);
#endif
}

void task_a2dp_irq_handler(struct AudioTask *this, uint32_t irq_type) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    clear_connsys_a2dp_interrupt();

    if ((this->state != AUDIO_TASK_IDLE) && (mHandshakeState == WAIT_BT_WAKEUP || mHandshakeState == WAIT_WRITE_BT_DONE)) {
        AUD_LOG_D("%s() mHandshakeState=%u ", __func__, mHandshakeState);
        xSemaphoreGiveFromISR(xSemBTHandshake, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void vA2DPTimerCallback(TimerHandle_t pxTimer) {
    AUD_LOG_D("+%s()", __func__);
    int i, j, bt_queue, dsp_send_times, wakeup_threshold;

#ifdef NO_ACCESS_BTSRAM
    bt_sram_addr = btsram_dummy;
#else
#ifndef GET_SRAMINFO_FROM_REG
    bt_sram_addr = (char *)A2DP_BTSRAM_ADDR;
#endif
#endif
    if (xA2DPTimer == NULL) {
        null_cnt++;
    }
    if (null_cnt == 10) {
        AUD_ASSERT(0);
    }

    taskENTER_CRITICAL();
    if ((a2dpBufIndex.count == 0) || (bA2dpIsPaused == true)) {
        taskEXIT_CRITICAL();
        AUD_LOG_W("a2dpBufIndex.count = %d, bA2dpIsPaused = %d return!", a2dpBufIndex.count, bA2dpIsPaused);
        return;
    }
    taskEXIT_CRITICAL();

    if (pAudio_a2dp_struct->bt_first_wakeup == true) {
        wakeup_threshold = a2dpbuf_wakeup_bt_threshold;
    } else {
        wakeup_threshold = 1;
    }
    if (a2dpBufIndex.count >= wakeup_threshold) {
        AUD_LOG_D("Try to wakeup BT");
#ifndef A2DP_HW_INTERRUPT_SUPPORT
        // DSP set byte0 to 1(DSP trigger BT)
        *(bt_sram_addr) = 1;

        /* put hifi3 in memw before check polling byte0 */
        __asm(
        "memw;\n"
        );

        // Wait until BT wakeup and fill queue info to byte2 and set byte0 to 2(BT wakeup)
        // If BT queue = 0, means BT is full and DSP can not write date to BT in this timercallback.
        int cnt = 0;
        do {
            aud_task_delay(1);	// wait 1 ms
            cnt++;
        } while((*(bt_sram_addr) != 0x2) && (cnt < TIMEOUT_CNT));

        if (cnt == TIMEOUT_CNT) {
            AUD_LOG_W("Wait BT wakeup timeout! return");
            return;
        }
        AUD_LOG_D("wait BT wakeup OK, cnt=%d", cnt);
#else
        AUD_LOG_D("mHandshakeState=%d", mHandshakeState);
        AUD_ASSERT(mHandshakeState == IDLE || mHandshakeState == WRITE_BT_DONE);
        mHandshakeState = WAIT_BT_WAKEUP;
        send_a2dp_interrupt();
        AUD_LOG_D("xSemaphoreTake(1)+");
        BaseType_t xResult = xSemaphoreTake(xSemBTHandshake, pdMS_TO_TICKS(BT_WAKEUP_TIMEOUT_CNT));
        AUD_LOG_D("xSemaphoreTake(1)-");

        if(xResult == pdFALSE) {
            mHandshakeState = IDLE;
            clear_a2dp_interrupt();
            // Error handling of semaphone when bt interrupt comes right at timeout threshold immediately
            if (uxSemaphoreGetCount(xSemBTHandshake) != 0) {
                AUD_LOG_D("uxSemaphoreGetCount()=%d", uxSemaphoreGetCount(xSemBTHandshake));
                xSemaphoreTake(xSemBTHandshake, 0);
            }
            AUD_LOG_D("Wait BT wakeup timeout a2dpBufIndex.count=%d, r_index=%d, w_index=%d", a2dpBufIndex.count,
                       a2dpBufIndex.r_index, a2dpBufIndex.w_index);

            if (a2dpBufIndex.count >= (a2dpbuf_max_queue - 1)) { // when queue is near full, drop data to avoid VUL4 HW overflow
                taskENTER_CRITICAL();
                a2dpBufIndex.r_index++;
                if (a2dpBufIndex.r_index == a2dpbuf_max_queue) {
                    a2dpBufIndex.r_index -= a2dpbuf_max_queue;
                }
                a2dpBufIndex.count--;
                taskEXIT_CRITICAL();
                xSemaphoreGive(pAudio_a2dp_struct->xA2DPSemphrTX);
            }

            AUD_LOG_W("Wait BT wakeup timeout! return");
            return;
        }
        mHandshakeState = BT_WAKEUP_DONE;
        uint32_t connsys_irq_rg = A2DP_IRQ_CONNSYS_REG;
        uint32_t connsys_reserved_rg = A2DP_RESERVED_CONNSYS_REG;
        AUD_LOG_D("BT_WAKEUP_DONE! A2DP_IRQ_CONNSYS_REG=%x, A2DP_RESERVED_CONNSYS_REG=%x", connsys_irq_rg, connsys_reserved_rg);
#endif
        if (pAudio_a2dp_struct->bt_first_wakeup) {
            bt_start_time = get_time_stamp();
            pAudio_a2dp_struct->bt_first_wakeup = false;

            // limite the queue size to a2dpbuf_max_threshold when start,
            // and reserve queue size 3 (60ms) buffer space to avoid overflow due to bt+wifi max transmission waiting time.
            if (a2dpBufIndex.count > a2dpbuf_max_threshold) {
                AUD_LOG_D("a2dpBufIndex.count=%d, limite the a2dpbuf count to %d when start", a2dpBufIndex.count, a2dpbuf_max_threshold);
                // drop oldest data
                int drop_count = a2dpBufIndex.count - a2dpbuf_max_threshold;
                taskENTER_CRITICAL();
                for (i = 0; i < drop_count; i++) {
                    a2dpBufIndex.r_index++;
                    if (a2dpBufIndex.r_index == a2dpbuf_max_queue) {
                        a2dpBufIndex.r_index -= a2dpbuf_max_queue;
                    }
                    a2dpBufIndex.count--;
                }

                taskEXIT_CRITICAL();
                AUD_ASSERT(a2dpBufIndex.count <= a2dpbuf_max_threshold);
            }
            AUD_LOG_D("bt_first_wakeup a2dpBufIndex.count=%d, w_index=%d, r_index=%d", a2dpBufIndex.count, a2dpBufIndex.w_index, a2dpBufIndex.r_index);

#ifdef GET_SRAMINFO_FROM_REG
            uint32_t btsram_datasize, btsram_readstart, btsram_writestart;

            btsram_datasize = BTSRAM_DATASIZE_CONNSYS_REG;
            btsram_readstart = BTSRAM_READSTART_CONNSYS_REG;
            btsram_writestart = BTSRAM_WRITESTART_CONNSYS_REG;
            bt_sram_addr = (char *)btsram_writestart - BTSRAM_REGION_1 + BTSRAM_INFRA_MAPPING;
            AUD_LOG_D("btsram_datasize=0x%x, btsram_readstart=0x%x, btsram_writestart=0x%x, bt_sram_addr=%p",
                      btsram_datasize, btsram_readstart, btsram_writestart, bt_sram_addr);
            AUD_ASSERT(btsram_datasize == A2DP_BTSRAM_SIZE);
            AUD_ASSERT(btsram_readstart == btsram_writestart);
            AUD_ASSERT(((btsram_writestart & 0xFFFF0000) == BTSRAM_REGION_1) || ((btsram_writestart & 0xFFFF0000) == BTSRAM_REGION_2));
#endif
        }
        cur_time = get_time_stamp();
        accu_time = cur_time - bt_start_time + (A2DP_TIMER_PERIOD * 1000000);
        AUD_LOG_D("cur_time=%llu, accu_time=%llu", cur_time, accu_time);

        /* put hifi3 in memw before check get byte2 */
        __asm(
        "memw;\n"
        );

        bt_queue = *(bt_sram_addr + 2);
        AUD_LOG_D("a2dpBufIndex.r_index=%d, bt_queue=%d, a2dpBufIndex.count=%d", a2dpBufIndex.r_index, bt_queue, a2dpBufIndex.count);
        AUD_LOG_D("frame_num=%d, frame_length=%d, encoded_data_length=%d", a2dpBuf[a2dpBufIndex.r_index]->frame_num,
                  a2dpBuf[a2dpBufIndex.r_index]->frame_length, a2dpBuf[a2dpBufIndex.r_index]->encoded_data_length);
#ifdef A2DP_UT_WO_BT
        bt_queue = 1;
#endif
        AUD_ASSERT(bt_queue <= MAX_BT_QUEUE_SIZE);
    }
    else {
        AUD_LOG_D("a2dpBufIndex.count %d < wakeup_threshold %d",a2dpBufIndex.count, wakeup_threshold);
        return;
    }

    dsp_send_times = (bt_queue < a2dpBufIndex.count) ? bt_queue : a2dpBufIndex.count ;
    AUD_LOG_D("[DSP_SEND_COUNT] dsp_send_times=%d, a2dpBufIndex.count=%d, accu_count=%llu", dsp_send_times, a2dpBufIndex.count, accu_count);
    /* put hifi3 in memw before write data to BT */
    __asm(
    "memw;\n"
    );

    AUD_LOG_D("start write encoded data to BT sram");
    j = dsp_send_times;
    do {
        AUD_ASSERT(a2dpBufIndex.count <= a2dpbuf_max_queue);
        AUD_ASSERT(a2dpBuf[a2dpBufIndex.r_index]->frame_length <= SBC_MAX_FRAME_LENGTH);
        unsigned int mtu_num = a2dpBuf[a2dpBufIndex.r_index]->mtu_num;
        AUD_LOG_D("j=%d, frame_num=%d, frame_length=%d, encoded_data_length=%d, mtu_num=%u",
                  j, a2dpBuf[a2dpBufIndex.r_index]->frame_num, a2dpBuf[a2dpBufIndex.r_index]->frame_length,
                  a2dpBuf[a2dpBufIndex.r_index]->encoded_data_length, mtu_num);
        if ((j < mtu_num) || (j == 0)) {
            AUD_LOG_D("j %d < mtu_num %u", j, mtu_num);
            packet_cnt++;
            taskENTER_CRITICAL();
            *(bt_sram_addr) = (packet_cnt & 0xff);
            *(bt_sram_addr + 1) = (packet_cnt & 0xff00) >> 8;
            *(bt_sram_addr + 2) = 0; // tell BT how many times DSP will write
            taskEXIT_CRITICAL();
            AUD_LOG_D("packet_cnt=0x%x%x", *(bt_sram_addr + 1), *(bt_sram_addr));

            /* put hifi3 in memw before set byte0 for BT polling */
            __asm(
            "memw;\n"
            );
#ifndef A2DP_HW_INTERRUPT_SUPPORT
            *(bt_sram_addr) = 3;

            /* put hifi3 in memw before polling BT done */
            __asm(
            "memw;\n"
            );

            // wait BT done
            int cnt = 0;
            do {
                aud_task_delay(1); // wait 1 ms
                cnt++;
            } while((*(bt_sram_addr) != 0x4) && (cnt < TIMEOUT_CNT));

            if (cnt == TIMEOUT_CNT) {
                AUD_LOG_E("j < mtu_num, Wait BT done timeout!, return");
                return;
            }
#else
            AUD_ASSERT(mHandshakeState == BT_WAKEUP_DONE || mHandshakeState == WRITE_BT_DONE);
            mHandshakeState = WAIT_WRITE_BT_DONE;
            send_a2dp_interrupt();
            AUD_LOG_D("xSemaphoreTake(2)+");
            BaseType_t xResult = xSemaphoreTake(xSemBTHandshake, pdMS_TO_TICKS(TIMEOUT_CNT2));
            AUD_LOG_D("xSemaphoreTake(2)-");

            if(xResult == pdFALSE) {
                mHandshakeState = IDLE;
                clear_a2dp_interrupt();
                if (uxSemaphoreGetCount(xSemBTHandshake) != 0) {
                    AUD_LOG_D("uxSemaphoreGetCount()=%d", uxSemaphoreGetCount(xSemBTHandshake));
                    xSemaphoreTake(xSemBTHandshake, 0);
                }
                AUD_LOG_E("j < mtu_num, Wait BT done timeout, return!");
                return;
            }
            mHandshakeState = WRITE_BT_DONE;
            uint32_t connsys_irq_rg = A2DP_IRQ_CONNSYS_REG;
            uint32_t connsys_reserved_rg = A2DP_RESERVED_CONNSYS_REG;
            AUD_LOG_D("(1)WRITE_BT_DONE! A2DP_IRQ_CONNSYS_REG=%x, A2DP_RESERVED_CONNSYS_REG=%x", connsys_irq_rg, connsys_reserved_rg);
#endif
            AUD_LOG_D("j < mtu_num, wait BT done OK, return!");
            return;
        }

        taskENTER_CRITICAL();
        unsigned char *ptr = a2dpBuf[a2dpBufIndex.r_index]->encoded_data;
        unsigned int length = a2dpBuf[a2dpBufIndex.r_index]->frame_num * a2dpBuf[a2dpBufIndex.r_index]->frame_length;
        unsigned int total_len = 0;
        unsigned int encoded_data_length = a2dpBuf[a2dpBufIndex.r_index]->encoded_data_length;
        taskEXIT_CRITICAL();

        // DSP write data according to BT queue info.
        // byte0: 3(DSP write data), byte2: times that DSP will write data, max: a2dpbuf_max_queue
        // byte3: frame_num, byte4~11: frame_length, byte12 ~ 1023: encoded data
        do {
            packet_cnt++;
            taskENTER_CRITICAL();
            *(bt_sram_addr) = (packet_cnt & 0xff);
            *(bt_sram_addr + 1) = (packet_cnt & 0xff00) >> 8;
            *(bt_sram_addr + 2) = j; // tell BT how many times DSP will write
            *(bt_sram_addr + 3) = a2dpBuf[a2dpBufIndex.r_index]->frame_num;
            *(bt_sram_addr + 4) = a2dpBuf[a2dpBufIndex.r_index]->frame_length;
            *(bt_sram_addr + 6) = (sbc_frame_pcm_samples & 0xff);
            *(bt_sram_addr + 7) = (sbc_frame_pcm_samples & 0xff00) >> 8;
            memcpy(bt_sram_addr + 12, ptr, length);
            taskEXIT_CRITICAL();

            AUD_LOG_D("sram: times=%d, fram_num=%d, frame_length1=%d, frame_length2=%d",
                      *(bt_sram_addr + 2), *(bt_sram_addr + 3), *(bt_sram_addr + 4), *(bt_sram_addr + 5));
            AUD_LOG_D("sram: sync=%x, packet_cnt=0x%x%x", *(bt_sram_addr + 12), *(bt_sram_addr + 1), *(bt_sram_addr));

            /* put hifi3 in memw before set byte0 for BT polling */
            __asm(
            "memw;\n"
            );
#ifndef A2DP_HW_INTERRUPT_SUPPORT
            *(bt_sram_addr) = 3;

            /* put hifi3 in memw before polling BT done */
            __asm(
            "memw;\n"
            );

            // wait BT done
            int cnt = 0;
            do {
                aud_task_delay(1); // wait 1 ms
                cnt++;
            } while((*(bt_sram_addr) != 0x4) && (cnt < TIMEOUT_CNT));

            /* put hifi3 in memw before next write */
            __asm(
            "memw;\n"
            );

            if (cnt == TIMEOUT_CNT) {
                AUD_LOG_E("Wait BT done timeout!!!");
                return;
            }
            AUD_LOG_D("wait BT done OK, cnt=%d", cnt);
#else
            AUD_ASSERT(mHandshakeState == BT_WAKEUP_DONE || mHandshakeState == WRITE_BT_DONE);
            mHandshakeState = WAIT_WRITE_BT_DONE;
            send_a2dp_interrupt();
            AUD_LOG_D("xSemaphoreTake(3)+");
            BaseType_t xResult = xSemaphoreTake(xSemBTHandshake, pdMS_TO_TICKS(TIMEOUT_CNT2));
            AUD_LOG_D("xSemaphoreTake(3)-");

            if(xResult == pdFALSE) {
                mHandshakeState = IDLE;
                clear_a2dp_interrupt();
                if (uxSemaphoreGetCount(xSemBTHandshake) != 0) {
                    AUD_LOG_D("uxSemaphoreGetCount()=%d", uxSemaphoreGetCount(xSemBTHandshake));
                    xSemaphoreTake(xSemBTHandshake, 0);
                }
                AUD_LOG_E("Wait BT done timeout, return!");
                return;
            }
            uint32_t connsys_irq_rg = A2DP_IRQ_CONNSYS_REG;
            uint32_t connsys_reserved_rg = A2DP_RESERVED_CONNSYS_REG;
            AUD_LOG_D("(2)WRITE_BT_DONE! A2DP_IRQ_CONNSYS_REG=%x, A2DP_RESERVED_CONNSYS_REG=%x", connsys_irq_rg, connsys_reserved_rg);
            mHandshakeState = WRITE_BT_DONE;
#endif
            ptr += length;
            total_len += length;
            if (mtu_num != 0) {
                mtu_num--;
            }
            j--;
        } while (mtu_num > 0);

        AUD_ASSERT(total_len == encoded_data_length);

        if (task_handle->pcmdump_enable == 1) {
            struct ipi_msg_t ipi_msg;
            unsigned char *data_buf = a2dpBuf[a2dpBufIndex.r_index]->encoded_data;
            uint32_t data_size = a2dpBuf[a2dpBufIndex.r_index]->encoded_data_length;

            audio_send_ipi_msg(&ipi_msg, TASK_SCENE_A2DP,
                               AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                               AUDIO_IPI_MSG_BYPASS_ACK,
                               AUDIO_DSP_TASK_PCMDUMP_DATA,
                               data_size,
                               DEBUG_PCMDUMP_OUT, //dump point serial number
                               data_buf);

            AUD_LOG_D("%s() AUDIO_DSP_TASK_PCMDUMP_DATA out", __FUNCTION__);
        }
        taskENTER_CRITICAL();
        a2dpBufIndex.r_index++;
        if (a2dpBufIndex.r_index == a2dpbuf_max_queue) {
            a2dpBufIndex.r_index -= a2dpbuf_max_queue;
        }
        if (a2dpBufIndex.count != 0) {
            a2dpBufIndex.count--;
        }
        taskEXIT_CRITICAL();

        accu_count++;
        AUD_LOG_D("a2dpBufIndex.count=%u", a2dpBufIndex.count);
    } while(j > 0);

    AUD_LOG_D("%s() a2dpBufIndex.count=%u, ready to xSemaphoreGive(xA2DPSemphrTX)", __func__, a2dpBufIndex.count);
    xSemaphoreGive(pAudio_a2dp_struct->xA2DPSemphrTX);
    AUD_LOG_D("-%s()", __func__);
}

void task_a2dp_timer_start(void) {
    AUD_LOG_D("%s() start a2dp timer", __func__);
    // timer init
    AUD_ASSERT(xA2DPTimer == NULL);

    xA2DPTimer = xTimerCreate(
        "A2DPTimer",
        (A2DP_TIMER_PERIOD / portTICK_PERIOD_MS),
        pdTRUE,
        0,
        vA2DPTimerCallback);

    if(xA2DPTimer == NULL)
    {
        /* The timer was not created. */
        AUD_LOG_W("%s() Timer create fail!!!", __func__);
    }
    else
    {
        if (xTimerStart(xA2DPTimer, 0) != pdPASS)
        {
            /* The timer could not be set into the Active state. */
            AUD_LOG_W("%s() Timer start fail!!!", __func__);
        }
    }
    null_cnt = 0;
    pAudio_a2dp_struct->bt_first_wakeup = true;
    accu_count = 0;
    packet_cnt = 0;
}

void task_a2dp_timer_stop(void) {
    AUD_LOG_D("%s() stop a2dp timer", __func__);
    // timer deinit
    if (xA2DPTimer != NULL) {
        if (xTimerStop(xA2DPTimer, 0) != pdPASS) {
        /* The timer could not be stop. */
        AUD_LOG_W("%s() Timer stop fail!!!", __func__);
        }
        if (xTimerDelete(xA2DPTimer, 0) != pdPASS) {
        /* The timer could not be delete. */
        AUD_LOG_W("%s() Timer delete fail!!!", __func__);
        }
        xA2DPTimer = NULL;
	}
}

void task_a2dp_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type) {
    if (this == NULL || hal_type != 0) {
        AUD_LOG_W("%s(), this %p type %d fail!!\n", __func__, this, hal_type);
        return;
    }
    AUD_LOG_D("%s(), scene %d", __func__, this->scene);
}

