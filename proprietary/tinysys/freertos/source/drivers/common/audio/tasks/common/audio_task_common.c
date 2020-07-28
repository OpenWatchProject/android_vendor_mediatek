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

#include "audio_task_common.h"

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <audio_task_aurisys.h>
#include <aurisys_config.h>
#endif

void dump_task_hwbuf(struct audio_task_common *task_common,
                            char *str)
{
    if (task_common->debug_flag >= TASK_DEBUG_VERBOSE) {
        AUD_LOG_D("%s ", str);
        dump_audio_hwbuffer(&task_common->mtask_audio_dl_buf);
        dump_audio_hwbuffer(&task_common->mtask_audio_ul_buf);
    }
}

static void dump_task_buf(struct audio_task_common *task_common,
                          char *str)
{

    if (task_common->debug_flag > TASK_DEBUG_VERBOSE) {
        dump_rbuf_bridge_s(str,
                           &task_common->mtask_audio_dl_buf.aud_buffer.buf_bridge);
        dump_rbuf_s(str, &task_common->mtask_audio_ring_buf);
    }
}

void task_print(struct audio_task_common *task_common,
                       char *str)
{
    if (task_common->debug_flag >= TASK_DEBUG_INFO) {
        AUD_LOG_D("%s %s", str, task_common->task_name);
    }
}

static void dump_task_check_buf(struct audio_task_common *task_common)
{
    Ringbuf_Check(&task_common->mtask_audio_ring_buf);
    Ringbuf_Bridge_Check(&task_common->mtask_audio_dl_buf.aud_buffer.buf_bridge);
}

int task_release_resource(struct audio_task_common *task_common)
{
#ifdef CFG_VCORE_DVFS_SUPPORT
    uint32_t feature_id = get_feature_id(task_common->task_scene);

    dvfs_deregister_feature(feature_id);
    scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, false);
    dvfs_disable_DRAM_resource(task_common->reserved_mem_id);
    disable_clk_bus(task_common->reserved_mem_id);
#endif

#ifdef MCPS_PROF_SUPPORT
    mcps_prof_show(&task_common->mcps_profile);
#endif

    stop_time_interval(task_common->task_profile_id);
    if (task_common->debug_flag >= TASK_DEBUG_VERBOSE) {
        AUD_LOG_D("%s id[%d]\n", __func__, task_common->reserved_mem_id);
    }
    return 0;
}

int task_require_resource(struct audio_task_common *task_common)
{
#ifdef CFG_VCORE_DVFS_SUPPORT
    uint32_t feature_id = get_feature_id(task_common->task_scene);

    dvfs_register_feature(feature_id);
    scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, true);
    enable_clk_bus(task_common->reserved_mem_id);
    dvfs_enable_DRAM_resource(task_common->reserved_mem_id);
#endif
    if (task_common->debug_flag >= TASK_DEBUG_VERBOSE) {
        AUD_LOG_D("%s id[%d]\n", __func__, task_common->reserved_mem_id);
    }

#ifdef MCPS_PROF_SUPPORT
    mcps_prof_init(&task_common->mcps_profile);
#endif

    /* set task profile flag */
    init_time_interval(task_common->task_profile_id,
                       task_common->task_profile_interval);

    return 0;
}

static bool ref_enable(AudioTask *this)
{
    struct audio_task_common *task_common = this->task_priv;
    if (task_common->ref_hwbuf_handle > 0) {
        return true;
    }
    else {
        return false;
    };
}

/* when set start time, init task_time_info */
int set_task_time_start(int handle , struct task_time_info *time_info)
{
    if (handle < 0) {
        AUD_LOG_W("%s() handle[%d] \n", __func__, handle);
        return -1;
    }

    time_info->start_time = get_time_stamp();
    time_info->current_time = time_info->start_time;
    time_info->frames = 0;
    time_info->remaind_frames_hw = audio_dsp_hw_getavail(handle);
    return 0;
}

int add_audio_time_stamp_frame(struct task_time_info *time_info, int frame)
{
    if (frame < 0) {
        AUD_LOG_W("%s() frame[%d] \n", __func__, frame);
        return -1;
    }
    time_info->frames += frame;
    return 0;
}

int get_audio_time_stamp(int handle , struct task_time_info *time_info)
{
    if (handle < 0) {
        AUD_LOG_W("%s() handle[%d]\n", __func__, handle);
        return -1;
    }

    time_info->current_time = get_time_stamp();
    audio_dsp_hw_update_ptr(handle);
    time_info->remaind_frames_hw = audio_dsp_hw_getavail(handle);
    return 0;
}

void print_audio_time_stamp(int handle, struct task_time_info *time_info)
{
    if (time_info == NULL || handle < 0) {
        AUD_LOG_W("%s() time_info == NULL handle[%d]\n", __func__, handle);
        return;
    }
    get_audio_time_stamp(handle, time_info);
    AUD_LOG_D("%s() start_time[%llu] current_time[%llu] frames[%llu] remaind_frames_hw[%llu]\n",
              __func__,
              time_info->start_time, time_info->current_time,
              time_info->frames, time_info->remaind_frames_hw);
}

void task_common_task_loop(void *pvParameters)
{
    AudioTask *this = (AudioTask *)pvParameters;
    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;
    uint8_t local_queue_idx = 0xFF;
    ipi_msg_t *ipi_msg;
    struct Audio_Task_Msg_t uAudioTaskMsg;
    int ret = 0;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    void *data_buf = NULL;
    uint32_t data_size = 0;
    struct data_buf_t param_list;

    uint32_t expect_out_size = 0;
#endif

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
            AUD_LOG_V("%s() loop start msg_id = %d param1 = %d param2 = %d local_queue_idx[%d]\n",
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
                if (task_is_dl(task_common->task_scene)) {
                    task_print(task_common, "task_is_dl AUDIO_DSP_TASK_PREPARE");
                    unsigned int addr = (unsigned int)task_common->mtask_dsp_atod_sharemm.phy_addr;
                    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                               HAL_CACHE_OPERATION_INVALIDATE,
                                               (void *)addr,
                                               task_common->mtask_dsp_atod_sharemm.size
                                              );
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_dl_buf);
                    RingBuf_Map_RingBuf_bridge(
                        &task_common->mtask_audio_dl_buf.aud_buffer.buf_bridge,
                        &task_common->mtask_audio_ring_buf);
                    RingBuf_Reset(&task_common->mtask_audio_ring_buf);
                    dump_task_hwbuf(task_common, "AUDIO_DSP_TASK_PREPARE");
                }
                else if (task_is_ul(task_common->task_scene)) {
                    unsigned int addr = (unsigned int)task_common->mtask_dsp_atod_sharemm.phy_addr;
                    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                               HAL_CACHE_OPERATION_INVALIDATE,
                                               (void *)addr,
                                               task_common->mtask_dsp_atod_sharemm.size
                                              );
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_ul_buf);
                    RingBuf_Map_RingBuf_bridge(
                        &task_common->mtask_audio_ul_buf.aud_buffer.buf_bridge,
                        &task_common->mtask_audio_ring_buf);
                    dump_rbuf_bridge_s("AUDIO_DSP_TASK_PREPARE",
                                       &task_common->mtask_audio_ul_buf.aud_buffer.buf_bridge);
                    task_print(task_common, "task_is_ul AUDIO_DSP_TASK_PREPARE");
                }
                else {
#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
                    memset(task_common->mtask_output_buf, 0, task_common->mtask_output_buf_size);
#endif
                    memset(task_common->mtask_input_buf, 0, task_common->mtask_input_buf_size);
                }
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
            case AUDIO_DSP_TASK_DLCOPY: {
                int written = 0;

                if (((this->state == AUDIO_TASK_INIT) || (this->state == AUDIO_TASK_WORKING)) == false) {
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_DLCOPY this->state[%d]\n", __FUNCTION__, this->state);
                    break;
                }

                record_time_interval(task_common->task_profile_id);

                dump_task_buf(task_common, "+DLCOPY");
                dump_task_check_buf(task_common);

                /* first wrtie one time when starting*/
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)task_common->mtask_audio_ring_buf.pBufBase,
                                           task_common->mtask_audio_ring_buf.bufLen
                                          );
                if (first_write_cond(task_common->playback_hwbuf_handle,
                                     this,
                                     &task_common->mtask_audio_dl_buf,
                                     &task_common->mtask_audio_afedl_buf,
                                     &task_common->mtask_audio_ring_buf,
                                     task_common->mtask_processing_buf_out_size)) {
                    /* copy buffer to linear buffer */
                    if (task_common->dma_enable)
                        RingBuf_copyToLinear_dma(task_common->mtask_processing_buf,
                                                 &task_common->mtask_audio_ring_buf,
                                                 task_common->mtask_processing_buf_out_size,
                                                 true,
                                                 task_common->reserved_mem_id);
                    else
                        RingBuf_copyToLinear(task_common->mtask_processing_buf,
                                             &task_common->mtask_audio_ring_buf,
                                             task_common->mtask_processing_buf_out_size);

                    dump_task_check_buf(task_common);

                    if (task_common->pcmdump_enable == 1) {
                        struct ipi_msg_t ipi_msg;
                        audio_send_ipi_msg(&ipi_msg, this->scene,
                                           AUDIO_IPI_LAYER_TO_HAL,
                                           AUDIO_IPI_DMA,
                                           AUDIO_IPI_MSG_BYPASS_ACK,
                                           AUDIO_DSP_TASK_PCMDUMP_DATA,
                                           task_common->mtask_processing_buf_out_size,
                                           DEBUG_PCMDUMP_IN,
                                           task_common->mtask_processing_buf);
                    }

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
                    task_do_processing(task_common->mtask_processing_buf,
                                       task_common->mtask_processing_buf_out_size,
                                       task_common->mtask_output_buf,
                                       &task_common->mtask_output_buf_size,
                                       NULL, 0);
#else
                    if (!get_aurisys_on()) {
                        task_common->mtask_output_buf = task_common->mtask_processing_buf;
                        task_common->mtask_output_buf_size = task_common->mtask_processing_buf_out_size;
                    }
                    else {
                        AUDIO_CHECK(task_common->mtask_processing_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                        AUDIO_CHECK(task_common->task_aurisys->mLinearOut->p_buffer);
                        AUDIO_CHECK(task_common->task_aurisys->mAudioPoolBufDlOut->ringbuf.base);

                        cal_output_buffer_size(&task_common->task_aurisys->dsp_config->attribute[DATA_BUF_DOWNLINK_IN],
                                               &task_common->task_aurisys->dsp_config->attribute[DATA_BUF_DOWNLINK_OUT],
                                               task_common->mtask_processing_buf_out_size, &expect_out_size);
                        AUD_ASSERT(expect_out_size != 0);


                        audio_pool_buf_copy_from_linear(
                            task_common->task_aurisys->mAudioPoolBufDlIn,
                            task_common->mtask_processing_buf,
                            task_common->mtask_processing_buf_out_size);

#ifdef MCPS_PROF_SUPPORT
                        mcps_prof_start(&task_common->mcps_profile);
#endif

                        // post processing + SRC + Bit conversion
                        aurisys_process_dl_only(task_common->task_aurisys->manager,
                                                task_common->task_aurisys->mAudioPoolBufDlIn,
                                                task_common->task_aurisys->mAudioPoolBufDlOut);

#ifdef MCPS_PROF_SUPPORT
                        mcps_prof_stop(&task_common->mcps_profile, SCENE_DL);
#endif

                        // data alignment
                        uint32_t data_size_align =
                            audio_ringbuf_count(&task_common->task_aurisys->mAudioPoolBufDlOut->ringbuf);
                        if (data_size_align != expect_out_size) {
                            AUD_LOG_W("data_size_align = %d, expect_out_size = %d", data_size_align,
                                      expect_out_size);
                            if (data_size_align > expect_out_size) {
                                data_size_align = expect_out_size;
                            }
                        }

                        data_size_align &= (~0x7F);

                        audio_pool_buf_copy_to_linear(
                            &task_common->task_aurisys->mLinearOut->p_buffer,
                            &task_common->task_aurisys->mLinearOut->memory_size,
                            task_common->task_aurisys->mAudioPoolBufDlOut,
                            data_size_align);
                        task_common->mtask_output_buf = task_common->task_aurisys->mLinearOut->p_buffer;
                        task_common->mtask_output_buf_size = data_size_align;

                        AUDIO_CHECK(task_common->mtask_processing_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                        AUDIO_CHECK(task_common->task_aurisys->mLinearOut->p_buffer);
                        AUDIO_CHECK(task_common->task_aurisys->mAudioPoolBufDlOut->ringbuf.base);
                    }
#endif
                    written = audio_dsp_hw_write(task_common->playback_hwbuf_handle,
                                                 task_common->mtask_output_buf,
                                                 task_common->mtask_output_buf_size);
                    if (written < 0) {
                        AUD_LOG_W("DLCOPY fail audio_dsp_hw_write written[%d]", written);
                        break;
                    }
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
                    if (get_aurisys_on()) {
                        AUDIO_CHECK(task_common->mtask_processing_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                    }
#endif
                    if (task_common->pcmdump_enable == 1) {
                        struct ipi_msg_t ipi_msg;
                        audio_send_ipi_msg(&ipi_msg, this->scene,
                                           AUDIO_IPI_LAYER_TO_HAL,
                                           AUDIO_IPI_DMA,
                                           AUDIO_IPI_MSG_BYPASS_ACK,
                                           AUDIO_DSP_TASK_PCMDUMP_DATA,
                                           task_common->mtask_output_buf_size,
                                           DEBUG_PCMDUMP_OUT,
                                           task_common->mtask_output_buf);
                    }
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
                    if (get_aurisys_on()) {
                        AUDIO_CHECK(task_common->mtask_processing_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                    }
#endif
                }
                unsigned int addr = (unsigned int)task_common->mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)addr,
                                           task_common->mtask_dsp_atod_sharemm.size
                                          );

                /* get message status and sync write point */
                if (ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_buf_working);
                    dump_task_buf(task_common, "COPY_STEP1");
                    task_common->mtask_audio_dl_buf.aud_buffer.buf_bridge.pWrite =
                        task_common->mtask_audio_buf_working.aud_buffer.buf_bridge.pWrite;
                    sync_ringbuf_writeidx(&task_common->mtask_audio_ring_buf,
                                          &task_common->mtask_audio_dl_buf.aud_buffer.buf_bridge);
                }

                dump_task_buf(task_common, "COPY_STEP2");
                dump_task_check_buf(task_common);

                /* send ack back if need ,here need to send back
                 * to tell kernel side copy is complete.
                 */
                audio_send_ipi_msg_ack_back(ipi_msg);

                task_common->loop_count = 0;
                while (enter_write_cond(task_common->playback_hwbuf_handle,
                                        this,
                                        &task_common->mtask_audio_dl_buf,
                                        &task_common->mtask_audio_afedl_buf,
                                        &task_common->mtask_audio_ring_buf,
                                        task_common->mtask_processing_buf_out_size) &&
                                        (task_common->loop_count++ < LOOP_COUNT_MAX)) {
                    /* copy buffer to linear buffer */
                    if (task_common->dma_enable)
                        RingBuf_copyToLinear_dma(task_common->mtask_processing_buf,
                                                 &task_common->mtask_audio_ring_buf,
                                                 task_common->mtask_processing_buf_out_size,
                                                 true,
                                                 task_common->reserved_mem_id);
                    else
                        RingBuf_copyToLinear(task_common->mtask_processing_buf,
                                             &task_common->mtask_audio_ring_buf,
                                             task_common->mtask_processing_buf_out_size);

                    dump_task_check_buf(task_common);

                    if (task_common->pcmdump_enable == 1) {
                    struct ipi_msg_t ipi_msg;
                    audio_send_ipi_msg(&ipi_msg, this->scene,
                                       AUDIO_IPI_LAYER_TO_HAL,
                                       AUDIO_IPI_DMA,
                                       AUDIO_IPI_MSG_BYPASS_ACK,
                                       AUDIO_DSP_TASK_PCMDUMP_DATA,
                                       task_common->mtask_processing_buf_out_size,
                                       DEBUG_PCMDUMP_IN,
                                       task_common->mtask_processing_buf);
                    }

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
                    task_do_processing(task_common->mtask_processing_buf,
                                       task_common->mtask_processing_buf_out_size,
                                       task_common->mtask_output_buf,
                                       &task_common->mtask_output_buf_size);
                                       NULL, 0);
#else
                    if (!get_aurisys_on()) {
                        task_common->mtask_output_buf = task_common->mtask_processing_buf;
                        task_common->mtask_output_buf_size = task_common->mtask_processing_buf_out_size;
                    }
                    else {
                        AUDIO_CHECK(task_common->mtask_processing_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                        AUDIO_CHECK(task_common->task_aurisys->mLinearOut->p_buffer);
                        AUDIO_CHECK(task_common->task_aurisys->mAudioPoolBufDlOut->ringbuf.base);

                        cal_output_buffer_size(&task_common->task_aurisys->dsp_config->attribute[DATA_BUF_DOWNLINK_IN],
                                               &task_common->task_aurisys->dsp_config->attribute[DATA_BUF_DOWNLINK_OUT],
                                               task_common->mtask_processing_buf_out_size, &expect_out_size);
                        AUD_ASSERT(expect_out_size != 0);


                        audio_pool_buf_copy_from_linear(
                            task_common->task_aurisys->mAudioPoolBufDlIn,
                            task_common->mtask_processing_buf,
                            task_common->mtask_processing_buf_out_size);

#ifdef MCPS_PROF_SUPPORT
                        mcps_prof_start(&task_common->mcps_profile);
#endif

                        // post processing + SRC + Bit conversion
                        aurisys_process_dl_only(task_common->task_aurisys->manager,
                                                task_common->task_aurisys->mAudioPoolBufDlIn,
                                                task_common->task_aurisys->mAudioPoolBufDlOut);

#ifdef MCPS_PROF_SUPPORT
                        mcps_prof_stop(&task_common->mcps_profile, SCENE_DL);
#endif

                        // data alignment
                        uint32_t data_size_align =
                            audio_ringbuf_count(&task_common->task_aurisys->mAudioPoolBufDlOut->ringbuf);
                        if (data_size_align != expect_out_size) {
                            AUD_LOG_W("data_size_align = %d, expect_out_size = %d", data_size_align,
                                      expect_out_size);
                            if (data_size_align > expect_out_size) {
                                data_size_align = expect_out_size;
                            }
                        }

                        data_size_align &= (~0x7F);

                        audio_pool_buf_copy_to_linear(
                            &task_common->task_aurisys->mLinearOut->p_buffer,
                            &task_common->task_aurisys->mLinearOut->memory_size,
                            task_common->task_aurisys->mAudioPoolBufDlOut,
                            data_size_align);
                        task_common->mtask_output_buf = task_common->task_aurisys->mLinearOut->p_buffer;
                        task_common->mtask_output_buf_size = data_size_align;

                        AUDIO_CHECK(task_common->mtask_processing_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                        AUDIO_CHECK(task_common->task_aurisys->mLinearOut->p_buffer);
                        AUDIO_CHECK(task_common->task_aurisys->mAudioPoolBufDlOut->ringbuf.base);
                    }
#endif

                    if (task_common->dummy_write)
                        written = audio_dsp_dummy_write(task_common->playback_hwbuf_handle,
                                                        task_common->mtask_output_buf,
                                                        task_common->mtask_output_buf_size);
                    else
                        written = audio_dsp_hw_write(task_common->playback_hwbuf_handle,
                                                     task_common->mtask_output_buf,
                                                     task_common->mtask_output_buf_size);
                    if (written < 0) {
                        AUD_LOG_W("DLCOPY fail audio_dsp_hw_write written[%d]", written);
                        break;
                    }

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
                    if (get_aurisys_on()) {
                        AUDIO_CHECK(task_common->mtask_processing_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                    }
#endif
                    if (task_common->pcmdump_enable == 1) {
                        struct ipi_msg_t ipi_msg;
                        audio_send_ipi_msg(&ipi_msg, this->scene,
                                           AUDIO_IPI_LAYER_TO_HAL,
                                           AUDIO_IPI_DMA,
                                           AUDIO_IPI_MSG_BYPASS_ACK,
                                           AUDIO_DSP_TASK_PCMDUMP_DATA,
                                           task_common->mtask_output_buf_size,
                                           DEBUG_PCMDUMP_OUT, //dump point serial number
                                           task_common->mtask_output_buf);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
                        if (get_aurisys_on()) {
                            AUDIO_CHECK(task_common->mtask_processing_buf);
                            AUDIO_CHECK(task_common->mtask_output_buf);
                        }
#endif
                    }

                    if (written != task_common->mtask_output_buf_size) {
                        AUD_LOG_D("%s AUDIO_DSP_TASK_DLCOPY written = %d\n", __func__, written);
                    }

                    if (task_common->debug_flag >= TASK_DEBUG_VERBOSE)
                        AUD_LOG_D("%s -audio_dsp_write = %d ts = %llu written = %d\n",
                                  __func__,
                                  written,
                                  get_time_stamp(),
                                  written);
                    dump_task_buf(task_common, "COPY_STEP3");
                }
                unsigned int addr_pa = (unsigned int)
                                       task_common->mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)addr_pa,
                                           task_common->mtask_dsp_dtoa_sharemm.size
                                          );

                if (record_time_interval(task_common->task_profile_id)) {
                    dump_task_check_buf(task_common);
                    dump_task_hwbuf(task_common, "COPY_TIMEOUT");
                }

                if(task_common->loop_count >= LOOP_COUNT_MAX)
                    AUD_LOG_W("DL_COPY %s count = %d\n", __func__, task_common->loop_count);

                stop_time_interval(task_common->task_profile_id);
                break;
            }
            case AUDIO_DSP_TASK_ULCOPY: {
                if (((this->state == AUDIO_TASK_INIT) || (this->state == AUDIO_TASK_WORKING)) == false) {
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_ULCOPY this->state[%d]\n", __FUNCTION__, this->state);
                    break;
                }

                /* read data from ul memory interface*/
                if (task_common->debug_flag >= TASK_DEBUG_INFO) {
                    DumpTaskMsg("AUDIO_DSP_TASK_ULCOPY", &uAudioTaskMsg);
                }

                unsigned int addr = (unsigned int)task_common->mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)addr,
                                           task_common->mtask_dsp_atod_sharemm.size
                                          );

                /* get message status and sync write point */
                if (ipi_msg->data_type == AUDIO_IPI_PAYLOAD) {
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_buf_working);
                }

                task_common->mtask_audio_ul_buf.aud_buffer.buf_bridge.pRead =
                    task_common->mtask_audio_buf_working.aud_buffer.buf_bridge.pRead;

                sync_ringbuf_readidx(&task_common->mtask_audio_ring_buf,
                                     &task_common->mtask_audio_ul_buf.aud_buffer.buf_bridge);

                if (task_common->debug_flag >= TASK_DEBUG_DEBUG) {
                    dump_rbuf_s("AUDIO_DSP_TASK_ULCOPY", &task_common->mtask_audio_ring_buf);
                    dump_rbuf_bridge_s("AUDIO_DSP_TASK_ULCOPY",
                                       &task_common->mtask_audio_ul_buf.aud_buffer.buf_bridge);

                    print_audio_time_stamp(task_common->capture_hwbuf_handle,&task_common->time_ul);
                    print_audio_time_stamp(task_common->ref_hwbuf_handle,&task_common->time_ref);
                }
                break;
            }
            case AUDIO_DSP_TASK_IRQDL: {
                if (this->state != AUDIO_TASK_WORKING) {
                    AUD_LOG_D("AUDIO_DSP_TASK_IRQDL but state = %d\n", this->state);
                    break;
                }
                ipi_msg_t ipi_msg_irqdl;
                audio_send_ipi_msg(&ipi_msg_irqdl,
                                   this->scene,
                                   AUDIO_IPI_LAYER_TO_KERNEL,
                                   AUDIO_IPI_MSG_ONLY,
                                   AUDIO_IPI_MSG_BYPASS_ACK,
                                   AUDIO_DSP_TASK_IRQDL,
                                   0,
                                   sizeof(struct audio_hw_buffer),
                                   0);
                break;
            }
            case AUDIO_DSP_TASK_IRQUL: {
                /* data can do processing*/
                int readdata = 0;
                if (this->state != AUDIO_TASK_WORKING) {
                    AUD_LOG_D("AUDIO_DSP_TASK_IRQUL but state = %d\n", this->state);
                    break;
                }
                /* can enter read condition*/
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
                cal_buffer_size(&task_common->task_aurisys->dsp_config->attribute[DATA_BUF_UPLINK_IN],
                                &task_common->mtask_processing_buf_in_size);
                cal_buffer_size(&task_common->task_aurisys->dsp_config->attribute[DATA_BUF_UPLINK_OUT],
                                &task_common->mtask_processing_buf_out_size);

                AUD_ASSERT((task_common->mtask_processing_buf_in_size != 0) &&
                           (task_common->mtask_processing_buf_out_size != 0));

                if (ref_enable(this) == true) {
                    cal_buffer_size(&task_common->task_aurisys->dsp_config->attribute[DATA_BUF_ECHO_REF],
                                    &task_common->mtask_ref_buf_size);
                    AUD_ASSERT(task_common->mtask_ref_buf_size != 0);
                }
#endif

                get_audio_loop_count(&task_common->mtask_audio_afeul_buf,
                                     task_common->mtask_processing_buf_in_size,
                                     &task_common->loop_count);

                while (enter_read_cond(task_common->capture_hwbuf_handle,
                                       this,
                                       &task_common->mtask_audio_ul_buf,
                                       &task_common->mtask_audio_afeul_buf,
                                       &task_common->mtask_audio_ring_buf,
                                       task_common->mtask_processing_buf_in_size,
                                       task_common->mtask_processing_buf_out_size)&&
                                       (task_common->loop_count-- > 0) ) {
                    /* copy buffer */
                    readdata = audio_dsp_hw_read(task_common->capture_hwbuf_handle,
                                                 task_common->mtask_processing_buf,
                                                 task_common->mtask_processing_buf_in_size);
                    add_audio_time_stamp_frame(&task_common->time_ul, readdata);

                    if (readdata != task_common->mtask_processing_buf_in_size) {
                        AUD_LOG_W("hw_read readdata[%d] mtask_processing_buf_in_size[%d]",
                                  readdata, task_common->mtask_processing_buf_in_size);
                        break;
                    }

                    /* if reference data support ,also read reference data*/
                    if (ref_enable(this) == true) {
                        /* copy buffer */
                        readdata = audio_dsp_hw_read(task_common->ref_hwbuf_handle,
                                                     task_common->mtask_ref_buf,
                                                     task_common->mtask_ref_buf_size);
                        add_audio_time_stamp_frame(&task_common->time_ref, readdata);
                        if (readdata != task_common->mtask_ref_buf_size) {
                            AUD_LOG_W("hw_read readdata[%d] mtask_ref_buf_size[%d]",
                                      readdata, task_common->mtask_ref_buf_size);
                            break;
                        }
                    }

                    if (task_common->pcmdump_enable == 1) {
                        struct ipi_msg_t ipi_msg;
                        audio_send_ipi_msg(&ipi_msg, this->scene,
                                           AUDIO_IPI_LAYER_TO_HAL,
                                           AUDIO_IPI_DMA,
                                           AUDIO_IPI_MSG_BYPASS_ACK,
                                           AUDIO_DSP_TASK_PCMDUMP_DATA,
                                           task_common->mtask_processing_buf_in_size,
                                           DEBUG_PCMDUMP_IN,
                                           task_common->mtask_processing_buf);
                        if (ref_enable(this) == true) {
                            audio_send_ipi_msg(&ipi_msg, this->scene,
                                               AUDIO_IPI_LAYER_TO_HAL,
                                               AUDIO_IPI_DMA,
                                               AUDIO_IPI_MSG_BYPASS_ACK,
                                               AUDIO_DSP_TASK_PCMDUMP_DATA,
                                               task_common->mtask_ref_buf_size,
                                               DEBUG_PCMDUMP_RESERVED,
                                               task_common->mtask_ref_buf);
                        }
                    }

                    /* do pre-processing*/
#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
                    if (ref_enable(this) == false) {
                        task_do_processing(task_common->mtask_processing_buf,
                                           task_common->mtask_processing_buf_in_size,
                                           task_common->mtask_input_buf,
                                           &task_common->mtask_input_buf_size,
                                           NULL, 0);
                    }
                    else {
                        task_do_processing(task_common->mtask_processing_buf,
                                           task_common->mtask_processing_buf_in_size,
                                           task_common->mtask_input_buf,
                                           &task_common->mtask_input_buf_size,
                                           task_common->mtask_ref_buf,
                                           task_common->mtask_ref_buf_size
                                          );
                    }
#else
                    if (!get_aurisys_on()) {
                        task_common->mtask_input_buf = task_common->mtask_processing_buf;
                        task_common->mtask_input_buf_size = task_common->mtask_processing_buf_in_size;
                    } else {
                        AUDIO_CHECK(task_common->mtask_processing_buf);
                        AUDIO_CHECK(task_common->mtask_input_buf);
                        AUDIO_CHECK(task_common->mtask_ref_buf);
                        AUDIO_CHECK(task_common->task_aurisys->mLinearOut->p_buffer);
                        AUDIO_CHECK(task_common->task_aurisys->mAudioPoolBufUlOut->ringbuf.base);

                        expect_out_size = task_common->mtask_processing_buf_out_size;

                        audio_pool_buf_copy_from_linear(
                            task_common->task_aurisys->mAudioPoolBufUlIn,
                            task_common->mtask_processing_buf,
                            task_common->mtask_processing_buf_in_size);

                        if ((ref_enable(this)) && (task_common->task_aurisys->mAudioPoolBufUlAec != NULL)) {
                            audio_pool_buf_copy_from_linear(
                                task_common->task_aurisys->mAudioPoolBufUlAec,
                                task_common->mtask_ref_buf,
                                task_common->mtask_ref_buf_size);
                        }

#ifdef MCPS_PROF_SUPPORT
                        mcps_prof_start(&task_common->mcps_profile);
#endif

                        // post processing + SRC + Bit conversion
                        aurisys_process_ul_only(task_common->task_aurisys->manager,
                                                task_common->task_aurisys->mAudioPoolBufUlIn,
                                                task_common->task_aurisys->mAudioPoolBufUlOut,
                                                task_common->task_aurisys->mAudioPoolBufUlAec);

#ifdef MCPS_PROF_SUPPORT
                        mcps_prof_stop(&task_common->mcps_profile, SCENE_UL);
#endif

                        // data alignment
                        uint32_t data_size_align =
                            audio_ringbuf_count(&task_common->task_aurisys->mAudioPoolBufUlOut->ringbuf);
                        if (data_size_align != expect_out_size) {
                            AUD_LOG_W("data_size_align = %d, expect_out_size = %d", data_size_align,
                                      expect_out_size);
                            if (data_size_align > expect_out_size) {
                                data_size_align = expect_out_size;
                            }
                        }

                        data_size_align &= (~0x7F);

                        audio_pool_buf_copy_to_linear(
                            &task_common->task_aurisys->mLinearOut->p_buffer,
                            &task_common->task_aurisys->mLinearOut->memory_size,
                            task_common->task_aurisys->mAudioPoolBufUlOut,
                            data_size_align);
                        task_common->mtask_input_buf = task_common->task_aurisys->mLinearOut->p_buffer;
                        task_common->mtask_input_buf_size = data_size_align;

                        AUDIO_CHECK(task_common->mtask_processing_buf);
                        AUDIO_CHECK(task_common->mtask_input_buf);
                        AUDIO_CHECK(task_common->task_aurisys->mLinearOut->p_buffer);
                        AUDIO_CHECK(task_common->task_aurisys->mAudioPoolBufUlOut->ringbuf.base);
                    }
#endif
                    if (task_common->pcmdump_enable == 1) {
                        struct ipi_msg_t ipi_msg;
                        audio_send_ipi_msg(&ipi_msg, this->scene,
                                           AUDIO_IPI_LAYER_TO_HAL,
                                           AUDIO_IPI_DMA,
                                           AUDIO_IPI_MSG_BYPASS_ACK,
                                           AUDIO_DSP_TASK_PCMDUMP_DATA,
                                           task_common->mtask_input_buf_size,
                                           DEBUG_PCMDUMP_OUT,
                                           task_common->mtask_input_buf);
                    }

                    /* copy to ring buffer */
                    if (task_common->dma_enable)
                        RingBuf_copyFromLinear_dma(&task_common->mtask_audio_ring_buf,
                                                   task_common->mtask_input_buf,
                                                   task_common->mtask_input_buf_size,
                                                   true,
                                                   task_common->reserved_mem_id);
                    else
                        RingBuf_cache_copyFromLinear(&task_common->mtask_audio_ring_buf,
                                                     task_common->mtask_input_buf,
                                                     task_common->mtask_input_buf_size);

                    /* sync buffer bridge*/
                    sync_bridge_ringbuf_writeidx(
                        &task_common->mtask_audio_ul_buf.aud_buffer.buf_bridge,
                        &task_common->mtask_audio_ring_buf);

                    if (task_common->debug_flag >= TASK_DEBUG_DEBUG) {
                        dump_task_check_buf(task_common);
                    }
                }

                if (task_common->debug_flag >= TASK_DEBUG_DEBUG) {
                    print_audio_time_stamp(task_common->capture_hwbuf_handle,&task_common->time_ul);
                    print_audio_time_stamp(task_common->ref_hwbuf_handle,&task_common->time_ref);

                    dump_rbuf_s("AUDIO_DSP_TASK_IRQUL", &task_common->mtask_audio_ring_buf);
                    dump_rbuf_bridge_s("AUDIO_DSP_TASK_IRQUL",
                                       &task_common->mtask_audio_ul_buf.aud_buffer.buf_bridge);
                }
                struct audio_hw_buffer *phw_buffer = &task_common->mtask_audio_ul_buf;
                unsigned int share_mem_addr = (unsigned int)
                                              task_common->mtask_dsp_dtoa_sharemm.phy_addr;
                memcpy((void *)share_mem_addr, (void *)phw_buffer,
                       sizeof(struct audio_hw_buffer));
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_FLUSH,
                                           (void *)share_mem_addr,
                                           task_common->mtask_dsp_dtoa_sharemm.size
                                          );

                ipi_msg_t ipi_msg_irqul;
                audio_send_ipi_msg(&ipi_msg_irqul,
                                   this->scene,
                                   AUDIO_IPI_LAYER_TO_KERNEL,
                                   AUDIO_IPI_MSG_ONLY,
                                   AUDIO_IPI_MSG_BYPASS_ACK,
                                   AUDIO_DSP_TASK_IRQUL,
                                   0,
                                   sizeof(struct audio_hw_buffer),
                                   0);
                break;
            }
            case AUDIO_DSP_TASK_START: {
                this->state = AUDIO_TASK_WORKING;
                if (task_is_dl(task_common->task_scene)) {
                    ipi_msg_t ipi_msg_to_task;
                    packing_ipimsg(&ipi_msg_to_task,
                                   this->scene,
                                   AUDIO_IPI_LAYER_TO_DSP,
                                   AUDIO_IPI_MSG_ONLY,
                                   AUDIO_IPI_MSG_BYPASS_ACK,
                                   AUDIO_DSP_TASK_DLCOPY,
                                   0,
                                   0,
                                   0);
                    send_task_message(this, &ipi_msg_to_task, task_common->max_queue_size);
                    AUD_LOG_D("AUDIO_DSP_TASK_START with AUDIO_DSP_TASK_DLCOPY\n");
                }
                else if (task_is_ul(task_common->task_scene)) {
                    taskENTER_CRITICAL();
                    if (ref_enable(this) == true) {
                        audio_dsp_hw_trigger(task_common->ref_hwbuf_handle, TRIGGER_START);
                        set_task_time_start(task_common->ref_hwbuf_handle, &task_common->time_ref);
                    }
                    audio_dsp_hw_trigger(task_common->capture_hwbuf_handle, TRIGGER_START);
                    set_task_time_start(task_common->capture_hwbuf_handle, &task_common->time_ul);
                    taskEXIT_CRITICAL();
                    AUD_LOG_D("AUDIO_DSP_TASK_START with AUDIO_DSP_TASK_UL\n");
                }
                else {
                    ipi_msg_t ipi_msg_to_task;
                    packing_ipimsg(&ipi_msg_to_task, this->scene, AUDIO_IPI_LAYER_TO_DSP,
                                   AUDIO_IPI_MSG_ONLY,
                                   AUDIO_IPI_MSG_BYPASS_ACK,
                                   AUDIO_DSP_TASK_DATAREAD,
                                   0,
                                   0,
                                   NULL);
                    send_task_message(this, &ipi_msg_to_task, task_common->max_queue_size);
                    taskENTER_CRITICAL();
                    if (ref_enable(this) == true) {
                        audio_dsp_hw_trigger(task_common->ref_hwbuf_handle, TRIGGER_START);
                    }
                    if(task_common->capture_hwbuf_handle >=0) {
                        audio_dsp_hw_trigger(task_common->capture_hwbuf_handle, TRIGGER_START);
                    }
                    taskEXIT_CRITICAL();
                }
                task_print(task_common, "AUDIO_DSP_TASK_START");
                break;
            }
            case AUDIO_DSP_TASK_DATAREAD: {
                bool overflow = false;
                if (this->state == AUDIO_TASK_WORKING) {
                    if (task_common->dummy_write) {
                        ret = audio_dsp_dummy_read(task_common->capture_hwbuf_handle,
                                                   task_common->mtask_input_buf,
                                                   task_common->mtask_input_buf_size);
                        if (ret != task_common->mtask_input_buf_size) {
                            AUD_LOG_E("%s() read fail\n", __FUNCTION__);
                        }
                        memcpy(task_common->mtask_output_buf, task_common->mtask_input_buf,
                               task_common->mtask_output_buf_size);
                        ret = audio_dsp_dummy_write(task_common->playback_hwbuf_handle,
                                                    task_common->mtask_output_buf,
                                                    task_common->mtask_output_buf_size);
                        AUD_LOG_D("%s() audio_dsp_dummy_write DONE \n", __FUNCTION__);
                    }
                    else {
                        ret = audio_dsp_hw_read(task_common->capture_hwbuf_handle,
                                                task_common->mtask_input_buf,
                                                task_common->mtask_input_buf_size);
                        if (ret != task_common->mtask_input_buf_size) {
                            AUD_LOG_E("%s() audio_dsp_hw_read fail ret [%d]\n", __FUNCTION__, ret);
                            overflow = true;
                        }

                        if (task_common->task_aurisys->dsp_config->iv_on
                            && (true == ref_enable(this))
                            && (NULL != task_common->mtask_iv_buf)) {
                            ret = audio_dsp_hw_read(task_common->ref_hwbuf_handle,
                                                    task_common->mtask_iv_buf,
                                                    task_common->mtask_iv_buf_size);
                        }
                        if (ret != task_common->mtask_iv_buf_size) {
                            AUD_LOG_E("%s() audio_dsp_hw_read iv fail ret [%d]\n", __FUNCTION__, ret);
                            overflow = true;
                        }
                    }

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
                        //IV data Dump
                        if ((task_common->task_aurisys->dsp_config->iv_on)
                            && (true == ref_enable(this))
                            && (NULL != task_common->mtask_iv_buf)) {
                                char *data_buf = task_common->mtask_iv_buf;
                                uint32_t data_size = task_common->mtask_iv_buf_size;
                                audio_send_ipi_msg(&ipi_msg, this->scene,
                                                   AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                                                   AUDIO_IPI_MSG_BYPASS_ACK,
                                                   AUDIO_DSP_TASK_PCMDUMP_DATA,
                                                   data_size,
                                                   DEBUG_PCMDUMP_IV, //dump point serial number
                                                   data_buf);
                        }
                    }

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
                    task_do_processing(task_common->mtask_input_buf,
                                       task_common->mtask_input_buf_size,
                                       task_common->mtask_output_buf,
                                       &task_common->mtask_output_buf_size,
                                       NULL, 0);
#else
                    if (!get_aurisys_on()) {
                        task_common->mtask_output_buf = task_common->mtask_input_buf;
                        task_common->mtask_output_buf_size = task_common->mtask_input_buf_size;
                    }
                    else {
                        AUDIO_CHECK(task_common->mtask_input_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                        AUDIO_CHECK(task_common->task_aurisys->mLinearOut->p_buffer);
                        AUDIO_CHECK(task_common->task_aurisys->mAudioPoolBufDlOut->ringbuf.base);

                        /* TODO: calculate by in/out attribute */
                        expect_out_size = task_common->mtask_input_buf_size;

                        audio_pool_buf_copy_from_linear(
                            task_common->task_aurisys->mAudioPoolBufDlIn,
                            task_common->mtask_input_buf,
                            task_common->mtask_input_buf_size);
                        if ((task_common->task_aurisys->dsp_config->iv_on) &&
                            (NULL != task_common->task_aurisys->mAudioPoolBufDlIV)) {
                            audio_pool_buf_copy_from_linear(
                                task_common->task_aurisys->mAudioPoolBufDlIV,
                                task_common->mtask_iv_buf,
                                task_common->mtask_iv_buf_size);
                        }

#ifdef MCPS_PROF_SUPPORT
                        mcps_prof_start(&task_common->mcps_profile);
#endif

                        // post processing + SRC + Bit conversion
                        aurisys_process_dl_only(task_common->task_aurisys->manager,
                                                task_common->task_aurisys->mAudioPoolBufDlIn,
                                                task_common->task_aurisys->mAudioPoolBufDlOut);

#ifdef MCPS_PROF_SUPPORT
                        mcps_prof_stop(&task_common->mcps_profile, SCENE_DL);
#endif

                        // data alignment
                        uint32_t data_size_align = audio_ringbuf_count(
                                                       &task_common->task_aurisys->mAudioPoolBufDlOut->ringbuf);
                        if (data_size_align > expect_out_size) {
                            data_size_align = expect_out_size;
                        }
                        data_size_align &= (~0x7F);

                        audio_pool_buf_copy_to_linear(
                            &task_common->task_aurisys->mLinearOut->p_buffer,
                            &task_common->task_aurisys->mLinearOut->memory_size,
                            task_common->task_aurisys->mAudioPoolBufDlOut,
                            data_size_align);
                        task_common->mtask_output_buf = task_common->task_aurisys->mLinearOut->p_buffer;
                        task_common->mtask_output_buf_size = data_size_align;

                        AUDIO_CHECK(task_common->mtask_input_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                        AUDIO_CHECK(task_common->task_aurisys->mLinearOut->p_buffer);
                        AUDIO_CHECK(task_common->task_aurisys->mAudioPoolBufDlOut->ringbuf.base);
                    }
#endif
                    // pcm dump for debug
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
                    }

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
                    if (get_aurisys_on()) {
                        AUDIO_CHECK(task_common->mtask_input_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                    }
#endif
                    ret = audio_dsp_hw_write(task_common->playback_hwbuf_handle,
                                             task_common->mtask_output_buf,
                                             task_common->mtask_output_buf_size);
                    if (ret != task_common->mtask_output_buf_size) {
                        AUD_LOG_E("%s() audio_dsp_hw_write fail ret[%d]\n", __FUNCTION__, ret);
                        overflow = true;
                    }

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
                    if (get_aurisys_on()) {
                        AUDIO_CHECK(task_common->mtask_input_buf);
                        AUDIO_CHECK(task_common->mtask_output_buf);
                    }
#endif
                    ipi_msg_t ipi_msg_to_task;
                    packing_ipimsg(&ipi_msg_to_task, this->scene, AUDIO_IPI_LAYER_TO_DSP,
                                   AUDIO_IPI_MSG_ONLY,
                                   AUDIO_IPI_MSG_BYPASS_ACK,
                                   AUDIO_DSP_TASK_DATAREAD,
                                   0,
                                   0,
                                   NULL);

                    send_task_message(this, &ipi_msg_to_task, task_common->max_queue_size);
                }
                else {
                    AUD_LOG_W("%s() AUDIO_DSP_TASK_DATAREAD err state = %d \n", __FUNCTION__,
                              this->state);
                }
                if (overflow == true) {
                    AUD_LOG_W("%s() AUDIO_DSP_TASK_DATAREAD overflow !!\n", __FUNCTION__);
                    if (task_common->playback_hwbuf_handle)
                        audio_dsp_hw_stop(task_common->playback_hwbuf_handle);
                    if (task_common->capture_hwbuf_handle)
                        audio_dsp_hw_stop(task_common->capture_hwbuf_handle);
                    if (task_common->ref_hwbuf_handle)
                        audio_dsp_hw_stop(task_common->ref_hwbuf_handle);
                    taskENTER_CRITICAL();
                    if (ref_enable(this) == true) {
                        audio_dsp_hw_trigger(task_common->ref_hwbuf_handle, TRIGGER_START);
                    }
                    if(task_common->capture_hwbuf_handle >= 0) {
                        audio_dsp_hw_trigger(task_common->capture_hwbuf_handle, TRIGGER_START);
                    }
                    taskEXIT_CRITICAL();
                    overflow = false;
                }
                break;
            }
            case AUDIO_DSP_TASK_STOP: {
                task_print(task_common, "AUDIO_DSP_TASK_STOP");
                this->state = AUDIO_TASK_IDLE;
                if (task_common->dummy_write) {
                    if (task_is_dl(task_common->task_scene)) {
                        audio_dsp_dummy_stop(task_common->playback_hwbuf_handle);
                    }
                    else if (task_is_ul(task_common->task_scene)) {
                        audio_dsp_dummy_stop(task_common->ref_hwbuf_handle);
                    }
                    else {
                        audio_dsp_dummy_stop(task_common->playback_hwbuf_handle);
                        audio_dsp_dummy_stop(task_common->capture_hwbuf_handle);
                        if (task_common->ref_hwbuf_handle > 0) {
                            audio_dsp_dummy_stop(task_common->ref_hwbuf_handle);
                        }
                    }
                }
                else {
                    if (task_is_dl(task_common->task_scene)) {
                        audio_dsp_hw_stop(task_common->playback_hwbuf_handle);
                    }
                    else if (task_is_ul(task_common->task_scene)) {
                        audio_dsp_hw_stop(task_common->capture_hwbuf_handle);
                        if (task_common->ref_hwbuf_handle > 0) {
                            audio_dsp_hw_stop(task_common->ref_hwbuf_handle);
                        }
                    }
                    else {
                        audio_dsp_hw_stop(task_common->playback_hwbuf_handle);
                        audio_dsp_hw_stop(task_common->capture_hwbuf_handle);
                        if (task_common->ref_hwbuf_handle > 0) {
                            audio_dsp_hw_stop(task_common->ref_hwbuf_handle);
                        }
                    }
                }
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
                    AUD_LOG_D("AUDIO_DSP_TASK_PCM_HWPARAM_DL task_name %s\n",
                              task_common->task_name);
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_afedl_buf);
                    dump_audio_hwbuffer(&task_common->mtask_audio_afedl_buf);
                }
                else if (task_common->mtask_audio_buf_working.aud_buffer.buffer_attr.direction
                         ==
                         AUDIO_DSP_TASK_PCM_HWPARAM_UL) {
                    AUD_LOG_D("AUDIO_DSP_TASK_PCM_HWPARAM_UL task_name %s\n",
                              task_common->task_name);
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_afeul_buf);
                    dump_audio_hwbuffer(&task_common->mtask_audio_afeul_buf);
                }
                else if (task_common->mtask_audio_buf_working.aud_buffer.buffer_attr.direction
                         ==
                         AUDIO_DSP_TASK_PCM_HWPARAM_REF) {
                    AUD_LOG_D("AUDIO_DSP_TASK_PCM_HWPARAM_REF task_name %s\n",
                              task_common->task_name);
                    get_audiobuf_from_msg(&uAudioTaskMsg, &task_common->mtask_audio_ref_buf);
                    dump_audio_hwbuffer(&task_common->mtask_audio_ref_buf);
                }
                else {
                    AUD_LOG_W("AUDIO_DSP_TASK_PCM_HWPARAM err  task_name %s\n",
                              task_common->task_name);
                }

                break;
            }
            case AUDIO_DSP_TASK_PCM_HWFREE: {
                if (task_common->debug_flag >= TASK_DEBUG_INFO) {
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_HWFREE\n", __FUNCTION__);
                }

                if (task_is_dl(task_common->task_scene)) {
                    if (task_common->dummy_write) {
                        audio_dsp_dummy_close(task_common->playback_hwbuf_handle);
                    }

                    else {
                        audio_dsp_hw_close(task_common->playback_hwbuf_handle);
                    }
                    task_common->playback_hwbuf_handle = -1;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
                    task_common->mtask_output_buf = NULL;
#endif
                }
                else if (task_is_ul(task_common->task_scene)) {
                    if (task_common->dummy_write) {
                        audio_dsp_dummy_close(task_common->capture_hwbuf_handle);
                    }
                    else {
                        audio_dsp_hw_close(task_common->capture_hwbuf_handle);
                        if (ref_enable(this) == true) {
                            audio_dsp_hw_close(task_common->ref_hwbuf_handle);
                            task_common->ref_hwbuf_handle = -1;
                        }
                    }
                    task_common->capture_hwbuf_handle = -1;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
                    task_common->mtask_input_buf = NULL;
#endif
                }
                else {
                    if (uAudioTaskMsg.param1 == AUDIO_DSP_TASK_PCM_HWPARAM_DL) {
                        AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_HWFREE_DL \n", __FUNCTION__);
                        if (task_common->dummy_write) {
                            audio_dsp_dummy_close(task_common->playback_hwbuf_handle);
                        }
                        else {
                            audio_dsp_hw_close(task_common->playback_hwbuf_handle);
                        }
                        task_common->playback_hwbuf_handle = -1;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
                        task_common->mtask_output_buf = NULL;
#endif
                    }
                    else if (uAudioTaskMsg.param1 == AUDIO_DSP_TASK_PCM_HWPARAM_REF) {
                        AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_HWFREE_REF \n", __FUNCTION__);
                        if (task_common->dummy_write) {
                            audio_dsp_dummy_close(task_common->ref_hwbuf_handle);
                        }
                        else {
                            audio_dsp_hw_close(task_common->ref_hwbuf_handle);
                        }
                        task_common->ref_hwbuf_handle = -1;
                    }
                    else {
                        AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_HWFREE_UL \n", __FUNCTION__);
                        if (task_common->dummy_write) {
                            audio_dsp_dummy_close(task_common->capture_hwbuf_handle);
                        }
                        else {
                            audio_dsp_hw_close(task_common->capture_hwbuf_handle);
                        }
                        task_common->capture_hwbuf_handle = -1;
                    }
                }
                break;
            }
            case AUDIO_DSP_TASK_PCM_PREPARE: {
                if (task_common->debug_flag >= TASK_DEBUG_INFO) {
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_PREPARE\n", __FUNCTION__);
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
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_PREPARE DL\n", __FUNCTION__);
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
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_PREPARE_DL playback_hwbuf_handle[%d]\n",
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
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_PREPARE_UL capture_hwbuf_handle[%d]\n",
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
                    AUD_LOG_D("%s() AUDIO_DSP_TASK_PCM_PREPARE_REF ref_hwbuf_handle[%d]\n",
                              __FUNCTION__, task_common->ref_hwbuf_handle);
                }
                else {
                    AUD_LOG_W("%s() direction = %u\n",
                              __FUNCTION__,
                              task_common->mtask_audio_buf_working.aud_buffer.buffer_attr.direction
                             );
                }
                break;
            }
            case AUDIO_DSP_TASK_PCMDUMP_ON:
                task_common->pcmdump_enable = uAudioTaskMsg.param1;
                break;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
            case AUDIO_DSP_TASK_AURISYS_INIT:
                audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);
                if (data_size < sizeof(struct aurisys_dsp_config_t)) {
                    AUD_WARNING("dsp config fail");
                    AUDIO_FREE_POINTER(data_buf);
                    break;
                }
                if (data_size == sizeof(struct aurisys_dsp_config_t)) {
                    AUD_WARNING("param fail");
                    AUDIO_FREE_POINTER(data_buf);
                    break;
                }

                AUDIO_ALLOC_STRUCT(struct aurisys_dsp_config_t,
                                   task_common->task_aurisys->dsp_config);
                memcpy(task_common->task_aurisys->dsp_config, data_buf,
                       sizeof(struct aurisys_dsp_config_t));

                param_list.data_size = data_size - sizeof(struct aurisys_dsp_config_t);
                param_list.memory_size = param_list.data_size;
                param_list.p_buffer = (uint8_t *)data_buf + sizeof(struct aurisys_dsp_config_t);

                CreateAurisysLibManager(task_common->task_aurisys, &param_list, task_common->task_scene);

                AUDIO_FREE_POINTER(data_buf);
                break;
            case AUDIO_DSP_TASK_AURISYS_DEINIT:
                DestroyAurisysLibManager(task_common->task_aurisys, task_common->task_scene);
                AUDIO_FREE_POINTER(task_common->task_aurisys->dsp_config);
                break;
            case AUDIO_DSP_TASK_AURISYS_LIB_PARAM:
                audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);
                if (task_common->task_aurisys && task_common->task_aurisys->manager) {
                    ipi_msg->param1 = UpdateAurisysLibParam(
                        task_common->task_aurisys,
                        data_buf,
                        data_size);
                }
                AUDIO_FREE_POINTER(data_buf);
                break;
            case AUDIO_DSP_TASK_AURISYS_PARAM_LIST:
                audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);

                /* param */
                param_list.data_size = data_size;
                param_list.memory_size = data_size;
                param_list.p_buffer = data_buf;

                ipi_msg->param1 = ApplyAurisysLibParamList(
                    task_common->task_aurisys,
                    &param_list,
                    task_common->task_scene);
                AUDIO_FREE_POINTER(data_buf);
                break;
            case AUDIO_DSP_TASK_AURISYS_SET_BUF: {
                data_buf_t data_buffer = {0};

                audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);
                data_buffer.p_buffer = data_buf;
                data_buffer.data_size = data_size;
                data_buffer.memory_size = data_size;

                /* aurisys_set_buf(task_common->task_aurisys->manager, "lib_name", &data_buffer, 0); */

                AUDIO_FREE_POINTER(data_buf);
                break;
            }
            case AUDIO_DSP_TASK_AURISYS_GET_BUF: {
                data_buf_t data_buffer = {0};
                struct aud_data_t *share_buf_info = (struct aud_data_t *)ipi_msg->payload;
                uint32_t buf_size = share_buf_info->memory_size;

                AUDIO_ALLOC_BUFFER(data_buffer.p_buffer, buf_size);
                data_buffer.memory_size = buf_size;
                data_buffer.data_size = 0;

                /* aurisys_get_buf(task_common->task_aurisys->manager, "lib_name", &data_buffer, 0); */

                audio_ipi_dma_write(share_buf_info->addr, data_buffer.p_buffer, data_buffer.data_size);
                share_buf_info->data_size = data_buffer.data_size;
                AUDIO_FREE_POINTER(data_buffer.p_buffer);
                break;
            }
#endif
            default:
                AUD_LOG_W("%s() id = %d task[%s]\n", __FUNCTION__,
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

void task_common_constructor(struct AudioTask *this)
{
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

    if (task_is_dl(task_common->task_scene)) {
#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
        task_common->mtask_output_buf = (char *)AUDIO_MALLOC(
                                            task_common->mtask_output_buf_size);
#endif
    }
    else if (task_is_ul(task_common->task_scene)) {
#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
        task_common->mtask_input_buf = (char *)AUDIO_MALLOC(
                                           task_common->mtask_input_buf_size);
#endif
    }
    else {
#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
        task_common->mtask_output_buf = (char *)AUDIO_MALLOC(
                                            task_common->mtask_output_buf_size);
#endif
        task_common->mtask_input_buf = (char *)AUDIO_MALLOC(
                                           task_common->mtask_input_buf_size);
    }
    task_common->mtask_processing_buf = (char *)AUDIO_MALLOC(
                                            task_common->mtask_processing_buf_out_size);
    AUD_ASSERT(this->msg_idx_queue != NULL);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    NEW_ALOCK(task_common->task_aurisys->mAurisysLibManagerLock);
#endif

}

void task_common_destructor(struct AudioTask *this)
{
    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;

    AUD_LOG_D("%s(), task_scene = %d", __func__, this->scene);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    FREE_ALOCK(task_common->task_aurisys->mAurisysLibManagerLock);
#endif

    if (task_is_dl(task_common->task_scene)) {
#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
        if (task_common->mtask_output_buf != NULL) {
            AUDIO_FREE(task_common->mtask_output_buf);
        }
#endif
    }
    else if (task_is_ul(task_common->task_scene)) {
#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
        if (task_common->mtask_input_buf != NULL) {
            AUDIO_FREE(task_common->mtask_input_buf);
        }
#endif
    }
    else {
#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
        if (task_common->mtask_output_buf != NULL) {
            AUDIO_FREE(task_common->mtask_output_buf);
        }
#endif
        if (task_common->mtask_input_buf != NULL) {
            AUDIO_FREE(task_common->mtask_input_buf);
        }

    }
    if (task_common->mtask_processing_buf != NULL) {
        AUDIO_FREE(task_common->mtask_processing_buf);
    }

    AUD_ASSERT(this != NULL);
    if (this->msg_array != NULL) {
        AUDIO_FREE(this->msg_array);
    }
}

void task_common_create_task_loop(struct AudioTask *this)
{
    /* Note: you can also bypass this function,
             and do kal_xTaskCreate until you really need it.
             Ex: create task after you do get the enable message. */

    BaseType_t xReturn = pdFAIL;

    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;

    xReturn = kal_xTaskCreate(
                  task_common_task_loop,
                  task_common->task_name,
                  task_common->task_stack_size,
                  (void *)this,
                  task_common->task_priority,
                  &this->freertos_task);

    AUD_ASSERT(xReturn == pdPASS);
}


status_t task_common_recv_message(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg)
{
    /* get task_common*/
    struct audio_task_common *task_common;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uint8_t queue_idx = 0, i = 0;

    if (this == NULL) {
        AUD_LOG_E("%s task null\n", __func__);
        return INVALID_OPERATION;
    }
    else if (ipi_msg == NULL) {
        AUD_LOG_E("%s ipi_msg null\n", __func__);
        return INVALID_OPERATION;
    }

    task_common = this->task_priv;
    if (task_common == NULL) {
        AUD_LOG_E("%s task_common null\n", __func__);
        return INVALID_OPERATION;
    }

    taskENTER_CRITICAL();
    this->num_queue_element++;
    queue_idx = get_queue_idx(this, task_common->max_queue_size);
    taskEXIT_CRITICAL();

    if (task_common == NULL) {
        AUD_LOG_E("task_common = %p\n", task_common);
        return UNKNOWN_ERROR;
    }

    if (this->msg_array[queue_idx].magic != 0) {
        AUD_LOG_E("queue_idx = %d\n", queue_idx);
        for (i = 0; i < task_common->max_queue_size; i++) {
            AUD_LOG_E("%s [%d] id = 0x%x\n", __FUNCTION__, i, this->msg_array[i].msg_id);
        }
        AUD_ASSERT(this->msg_array[queue_idx].magic == 0); /* item is clean */
    }

    // copy to array
    memcpy(&this->msg_array[queue_idx], ipi_msg, sizeof(ipi_msg_t));
    AUD_LOG_V("%s queue_idx [%d][%d] id = 0x%x\n", __FUNCTION__, queue_idx, i,
              this->msg_array[queue_idx].msg_id);

    if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
                                &xHigherPriorityTaskWoken) != pdTRUE) {
        return UNKNOWN_ERROR;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return NO_ERROR;
}


void task_common_irq_handler(
    struct AudioTask *this,
    uint32_t irq_type)
{
    /* get task_common*/
    struct audio_task_common *task_common = this->task_priv;
    ipi_msg_t ipi_msg_to_task;

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
    if (task_is_dl(task_common->task_scene)) {
        if (audio_dsp_hw_irq(this, task_common->playback_hwbuf_handle) < 0) {
            AUD_LOG_E("%s underflow\n", __func__);
            return;
        }

        packing_ipimsg(&ipi_msg_to_task, this->scene, AUDIO_IPI_LAYER_TO_DSP,
                       AUDIO_IPI_MSG_ONLY,
                       AUDIO_IPI_MSG_BYPASS_ACK,
                       AUDIO_DSP_TASK_IRQDL,
                       0,
                       0,
                       0);
        send_task_message_fromisr(this, &ipi_msg_to_task, task_common->max_queue_size);
    }
    else if (task_is_ul(task_common->task_scene)) {
        if (audio_dsp_hw_irq(this, task_common->capture_hwbuf_handle) < 0) {
            AUD_LOG_E("%s overflow\n", __func__);
            return;
        }

        if (ref_enable(this) == true) {
            audio_dsp_hw_irq(this, task_common->ref_hwbuf_handle);
        }
        packing_ipimsg(&ipi_msg_to_task, this->scene, AUDIO_IPI_LAYER_TO_DSP,
                       AUDIO_IPI_MSG_ONLY,
                       AUDIO_IPI_MSG_BYPASS_ACK,
                       AUDIO_DSP_TASK_IRQUL,
                       0,
                       0,
                       0);
        send_task_message_fromisr(this, &ipi_msg_to_task, task_common->max_queue_size);
    }
    else {
        audio_dsp_hw_irq(this, task_common->playback_hwbuf_handle);
        audio_dsp_hw_irq(this, task_common->capture_hwbuf_handle);
        if (ref_enable(this) == true) {
            audio_dsp_hw_irq(this, task_common->ref_hwbuf_handle);
        }
    }
}

void task_common_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type) {
    struct audio_task_common *task_common = NULL;

    if (this == NULL || hal_type != 0) {
        AUD_LOG_W("%s(), this %p type %d fail!!", __func__, this, hal_type);
        return;
    }

    AUD_LOG_D("%s(), scene %d", __func__, this->scene);

    task_common = this->task_priv;
    if (task_common == NULL) {
        AUD_LOG_W("%s(), task_common %p fail!!", __func__, task_common);
        return;
    }

    if (task_common->task_aurisys && task_common->task_aurisys->manager) {
        DestroyAurisysLibManager(task_common->task_aurisys, task_common->task_scene);
        AUDIO_FREE_POINTER(task_common->task_aurisys->dsp_config);
    }
}

int init_common_task(struct audio_task_common
                     **task_common)
{

    struct audio_task_common *task_common_inst = NULL;

    if (*task_common != NULL) {
        AUD_LOG_E("%s\n", __FUNCTION__);
        return -1;
    }

    AUD_LOG_D("%s\n", __FUNCTION__);
    task_common_inst = (struct audio_task_common *)AUDIO_MALLOC(sizeof(
                                                                    struct audio_task_common));
    if (task_common_inst == NULL) {
        AUD_LOG_D("%s AUDIO_MALLOC task_common_inst fail\n", __FUNCTION__);
        return -1;
    }
    memset((void *)task_common_inst, 0, sizeof(struct audio_task_common));

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    task_common_inst->task_aurisys = (struct audio_aurisys_handle_t *)AUDIO_MALLOC(
                                         sizeof(
                                             struct audio_aurisys_handle_t));
    if (task_common_inst->task_aurisys == NULL) {
        AUD_LOG_D("%s AUDIO_MALLOC task_aurisys fail\n", __FUNCTION__);
        if (task_common_inst) {
            AUDIO_FREE(task_common_inst);
        }
        return -1;
    }
#endif
    memset((void *)task_common_inst->task_aurisys, 0,
           sizeof(struct audio_aurisys_handle_t));

    *task_common = task_common_inst;

    return 0;
}

int deinit_common_task(struct audio_task_common
                       **task_common)
{

    struct audio_task_common *task_common_inst = *task_common;

    if (task_common_inst == NULL) {
        AUD_LOG_E("%s task_common = NULL \n", __FUNCTION__);
        return -1;
    }

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (task_common_inst->task_aurisys) {
        AUDIO_FREE(task_common_inst->task_aurisys);
        task_common_inst->task_aurisys = NULL;
    }
#endif
    AUDIO_FREE(task_common_inst);

    *task_common = NULL;

    return 0;
}

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
struct audio_aurisys_handle_t *get_task_aurisys(struct audio_task_common
                                                *task_common)
{
    return task_common->task_aurisys;
}
#endif

