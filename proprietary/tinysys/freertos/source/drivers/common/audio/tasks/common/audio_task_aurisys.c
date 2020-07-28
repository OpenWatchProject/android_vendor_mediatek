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

#include <audio_task_aurisys.h>

#include <string.h>


#include <audio_log.h>

#include <wrapped_audio.h>
#include <audio_ringbuf_pure.h>
#include <audio_pool_buf_handler.h>

#include <audio_task.h>

#include <aurisys_config.h>

#include <aurisys_utility.h>
#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#include <feature_manager.h>
#include <dvfs_config_parser.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "[AURI]"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "%s(), " fmt "\n", __func__


void init_dvfs_for_swip(struct aurisys_lib_manager_t *manager, unsigned char task_scene) {
    struct aurisys_lib_handler_t *itor_lib_hanlder = NULL;
    struct aurisys_lib_handler_t *tmp_lib_handler = NULL;
    struct dvfs_swip_config_t dvfs_config;

    uint32_t swip_id;
    uint32_t feature_id = get_feature_id(task_scene);

    HASH_ITER(hh_all, manager->all_lib_handler_list, itor_lib_hanlder, tmp_lib_handler) {
        LOCK_ALOCK(itor_lib_hanlder->lock);
        map_aurisys_config_to_dvfs_config(itor_lib_hanlder, &dvfs_config);
        swip_id = get_swip_id(itor_lib_hanlder->lib_name, itor_lib_hanlder->task_config->task_scene);
        ALOGV("%s(), lib name:%s feature_id = %d, swip_id = %d",
              __FUNCTION__, itor_lib_hanlder->lib_name, feature_id, swip_id);
        dvfs_add_swip(feature_id, swip_id, &dvfs_config);
        UNLOCK_ALOCK(itor_lib_hanlder->lock);
    }

}


void deinit_dvfs_for_swip(struct aurisys_lib_manager_t *manager, unsigned char task_scene) {
    struct aurisys_lib_handler_t *itor_lib_hanlder = NULL;
    struct aurisys_lib_handler_t *tmp_lib_handler = NULL;
    struct dvfs_swip_config_t dvfs_config;
    memset(&dvfs_config, 0, sizeof(struct dvfs_swip_config_t));

    uint32_t swip_id;
    uint32_t feature_id = get_feature_id(task_scene);

    HASH_ITER(hh_all, manager->all_lib_handler_list, itor_lib_hanlder, tmp_lib_handler) {
        map_aurisys_config_to_dvfs_config(itor_lib_hanlder, &dvfs_config);
        LOCK_ALOCK(itor_lib_hanlder->lock);
        swip_id = get_swip_id(itor_lib_hanlder->lib_name, itor_lib_hanlder->task_config->task_scene);
        ALOGV("%s(), lib name:%s feature_id = %d, swip_id = %d",
               __FUNCTION__, itor_lib_hanlder->lib_name, feature_id, swip_id);
        dvfs_delete_swip(feature_id, swip_id, &dvfs_config);
        UNLOCK_ALOCK(itor_lib_hanlder->lock);
    }
}


static void InitBufferConfig(struct audio_aurisys_handle_t *aurisys_handle) {
    struct aurisys_lib_manager_t *manager = aurisys_handle->manager;
    struct aurisys_lib_manager_config_t *manager_config = &aurisys_handle->dsp_config->manager_config;
    struct arsi_task_config_t *task_config = &manager_config->task_config;

    struct stream_attribute_dsp *attribute_in = NULL;
    struct stream_attribute_dsp *attribute_out = NULL;
    struct stream_attribute_dsp *attribute_aec = NULL;
    struct stream_attribute_dsp *attribute_iv = NULL;

    uint8_t arsi_process_type = manager_config->arsi_process_type;

    uint8_t b_interleave = 0;
    uint8_t frame_size_ms = 0;

    if (task_config->task_scene == TASK_SCENE_PHONE_CALL) {
        /* modem EMI data format */
        b_interleave = 0;
        frame_size_ms = 20;
    } else {
        /* Android / Audio AFE data format */
        b_interleave = 1;
        frame_size_ms = 0;
    }


    /* UL  */
    if (arsi_process_type == ARSI_PROCESS_TYPE_UL_ONLY ||
        arsi_process_type == ARSI_PROCESS_TYPE_UL_AND_DL) {
        /* UL in */
        aurisys_handle->mAudioPoolBufUlIn = create_audio_pool_buf(manager, DATA_BUF_UPLINK_IN, 0);
        attribute_in = &aurisys_handle->dsp_config->attribute[DATA_BUF_UPLINK_IN];

        aurisys_handle->mAudioPoolBufUlIn->buf->b_interleave = b_interleave;
        aurisys_handle->mAudioPoolBufUlIn->buf->frame_size_ms = frame_size_ms;
        aurisys_handle->mAudioPoolBufUlIn->buf->num_channels = attribute_in->num_channels;
        aurisys_handle->mAudioPoolBufUlIn->buf->channel_mask = get_input_ch_mask(aurisys_handle->mAudioPoolBufUlIn->buf->num_channels);
        aurisys_handle->mAudioPoolBufUlIn->buf->sample_rate_buffer = attribute_in->sample_rate;
        aurisys_handle->mAudioPoolBufUlIn->buf->sample_rate_content = attribute_in->sample_rate;
        aurisys_handle->mAudioPoolBufUlIn->buf->audio_format = attribute_in->audio_format;


        /* UL out */
        aurisys_handle->mAudioPoolBufUlOut = create_audio_pool_buf(manager, DATA_BUF_UPLINK_OUT, 0);
        attribute_out = &aurisys_handle->dsp_config->attribute[DATA_BUF_UPLINK_OUT];

        aurisys_handle->mAudioPoolBufUlOut->buf->b_interleave = b_interleave;
        aurisys_handle->mAudioPoolBufUlOut->buf->frame_size_ms = frame_size_ms;
        aurisys_handle->mAudioPoolBufUlOut->buf->num_channels = attribute_out->num_channels;
        aurisys_handle->mAudioPoolBufUlOut->buf->channel_mask = get_input_ch_mask(aurisys_handle->mAudioPoolBufUlOut->buf->num_channels);
        aurisys_handle->mAudioPoolBufUlOut->buf->sample_rate_buffer = attribute_out->sample_rate;
        aurisys_handle->mAudioPoolBufUlOut->buf->sample_rate_content = attribute_out->sample_rate;
        aurisys_handle->mAudioPoolBufUlOut->buf->audio_format = attribute_out->audio_format;

        /* AEC */
        if (aurisys_handle->dsp_config->aec_on) {
            aurisys_handle->mAudioPoolBufUlAec = create_audio_pool_buf(manager, DATA_BUF_ECHO_REF, 0);
            attribute_aec = &aurisys_handle->dsp_config->attribute[DATA_BUF_ECHO_REF];

            aurisys_handle->mAudioPoolBufUlAec->buf->b_interleave = b_interleave;
            aurisys_handle->mAudioPoolBufUlAec->buf->frame_size_ms = frame_size_ms;
            aurisys_handle->mAudioPoolBufUlAec->buf->num_channels = attribute_aec->num_channels;
            aurisys_handle->mAudioPoolBufUlAec->buf->channel_mask = get_input_ch_mask(aurisys_handle->mAudioPoolBufUlAec->buf->num_channels);
            aurisys_handle->mAudioPoolBufUlAec->buf->sample_rate_buffer = attribute_aec->sample_rate;
            aurisys_handle->mAudioPoolBufUlAec->buf->sample_rate_content = attribute_aec->sample_rate;
            aurisys_handle->mAudioPoolBufUlAec->buf->audio_format = attribute_aec->audio_format;
        }
    }


    /* DL  */
    if (arsi_process_type == ARSI_PROCESS_TYPE_DL_ONLY ||
        arsi_process_type == ARSI_PROCESS_TYPE_UL_AND_DL) {
        /* DL in */
        aurisys_handle->mAudioPoolBufDlIn = create_audio_pool_buf(manager, DATA_BUF_DOWNLINK_IN, 0);
        attribute_in = &aurisys_handle->dsp_config->attribute[DATA_BUF_DOWNLINK_IN];

        aurisys_handle->mAudioPoolBufDlIn->buf->b_interleave = b_interleave;
        aurisys_handle->mAudioPoolBufDlIn->buf->frame_size_ms = frame_size_ms;
        aurisys_handle->mAudioPoolBufDlIn->buf->num_channels = attribute_in->num_channels;
        aurisys_handle->mAudioPoolBufDlIn->buf->channel_mask = get_input_ch_mask(aurisys_handle->mAudioPoolBufDlIn->buf->num_channels);
        aurisys_handle->mAudioPoolBufDlIn->buf->sample_rate_buffer = attribute_in->sample_rate;
        aurisys_handle->mAudioPoolBufDlIn->buf->sample_rate_content = attribute_in->sample_rate;
        aurisys_handle->mAudioPoolBufDlIn->buf->audio_format = attribute_in->audio_format;


        /* DL out */
        aurisys_handle->mAudioPoolBufDlOut = create_audio_pool_buf(manager, DATA_BUF_DOWNLINK_OUT, 0);
        attribute_out = &aurisys_handle->dsp_config->attribute[DATA_BUF_DOWNLINK_OUT];

        aurisys_handle->mAudioPoolBufDlOut->buf->b_interleave = b_interleave;
        aurisys_handle->mAudioPoolBufDlOut->buf->frame_size_ms = frame_size_ms;
        aurisys_handle->mAudioPoolBufDlOut->buf->num_channels = attribute_out->num_channels;
        aurisys_handle->mAudioPoolBufDlOut->buf->channel_mask = get_input_ch_mask(aurisys_handle->mAudioPoolBufDlOut->buf->num_channels);
        aurisys_handle->mAudioPoolBufDlOut->buf->sample_rate_buffer = attribute_out->sample_rate;
        aurisys_handle->mAudioPoolBufDlOut->buf->sample_rate_content = attribute_out->sample_rate;
        aurisys_handle->mAudioPoolBufDlOut->buf->audio_format = attribute_out->audio_format;

        if (aurisys_handle->dsp_config->iv_on) {
            aurisys_handle->mAudioPoolBufDlIV = create_audio_pool_buf(manager, DATA_BUF_IV_BUFFER, 0);
            attribute_iv = &aurisys_handle->dsp_config->attribute[DATA_BUF_IV_BUFFER];  //maybe don't need attribute?
            aurisys_handle->mAudioPoolBufDlIV->buf->b_interleave = 1; /* LRLRLRLR*/
            aurisys_handle->mAudioPoolBufDlIV->buf->frame_size_ms = 0;
            aurisys_handle->mAudioPoolBufDlIV->buf->num_channels = attribute_iv->num_channels;;
            aurisys_handle->mAudioPoolBufDlIV->buf->sample_rate_buffer = attribute_iv->sample_rate;
            aurisys_handle->mAudioPoolBufDlIV->buf->sample_rate_content = attribute_iv->sample_rate;
            aurisys_handle->mAudioPoolBufDlIV->buf->audio_format = attribute_iv->audio_format;
        }
    }
}


void CreateAurisysLibManager(
    struct audio_aurisys_handle_t *aurisys_handle,
    struct data_buf_t *param_list,
    unsigned char task_scene) {
    struct aurisys_dsp_config_t *dsp_config = NULL;
    struct aurisys_lib_manager_config_t *manager_config = NULL;

    int ret = 0;


    if (!get_aurisys_on()) {
        ALOGD("bypass aurisys");
        return;
    }

    if (!aurisys_handle) {
        AUD_WARNING("aurisys_handle NULL!!");
        return;
    }

    dsp_config = aurisys_handle->dsp_config;
    if (!dsp_config) {
        AUD_WARNING("dsp_config NULL!!");
        return;
    }
    manager_config = &dsp_config->manager_config;
    ALOGD("manager config: aurisys_scenario %u, core_type %d, " \
          "arsi_process_type %d, frame_size_ms %d, " \
          "num_channels_ul %d, num_channels_dl %d, " \
          "audio_format %u, sample_rate %u",
          manager_config->aurisys_scenario,
          manager_config->core_type,
          manager_config->arsi_process_type,
          manager_config->frame_size_ms,
          manager_config->num_channels_ul,
          manager_config->num_channels_dl,
          manager_config->audio_format,
          manager_config->sample_rate);

    /* debug only */
    AUD_ASSERT(dsp_config->guard_head == AURISYS_GUARD_HEAD_VALUE);
    AUD_ASSERT(dsp_config->guard_tail == AURISYS_GUARD_TAIL_VALUE);
    dump_task_config(&manager_config->task_config);

    /* create manager */
    LOCK_ALOCK(aurisys_handle->mAurisysLibManagerLock);

    aurisys_handle->manager = create_aurisys_lib_manager(manager_config);
    InitBufferConfig(aurisys_handle);

    ret = aurisys_apply_param_list(aurisys_handle->manager, param_list);
    AUD_ASSERT(ret == 0);

    aurisys_create_arsi_handlers(aurisys_handle->manager);
    aurisys_pool_buf_formatter_init(aurisys_handle->manager);

    /* set UL digital gain */
    aurisys_set_ul_digital_gain(aurisys_handle->manager,
                                dsp_config->gain_config.ul_analog_gain,
                                dsp_config->gain_config.ul_digital_gain);

    AUDIO_ALLOC_STRUCT(struct data_buf_t, aurisys_handle->mLinearOut);
    init_dvfs_for_swip(aurisys_handle->manager, task_scene);

    UNLOCK_ALOCK(aurisys_handle->mAurisysLibManagerLock);
}


void DestroyAurisysLibManager(
    struct audio_aurisys_handle_t *aurisys_handle,
    unsigned char task_scene) {
    if (!get_aurisys_on()) {
        ALOGD("bypass aurisys");
        return;
    }
    if (!aurisys_handle) {
        AUD_WARNING("aurisys_handle NULL!!");
        return;
    }

    LOCK_ALOCK(aurisys_handle->mAurisysLibManagerLock);

    deinit_dvfs_for_swip(aurisys_handle->manager, task_scene);

    aurisys_destroy_arsi_handlers(aurisys_handle->manager);
    aurisys_pool_buf_formatter_deinit(aurisys_handle->manager);
    destroy_aurisys_lib_manager(aurisys_handle->manager);
    aurisys_handle->manager = NULL;

    aurisys_handle->mAudioPoolBufUlIn = NULL;
    aurisys_handle->mAudioPoolBufUlOut = NULL;
    aurisys_handle->mAudioPoolBufUlAec = NULL;
    aurisys_handle->mAudioPoolBufDlIn = NULL;
    aurisys_handle->mAudioPoolBufDlOut = NULL;
    aurisys_handle->mAudioPoolBufDlIV = NULL;

    AUDIO_FREE_POINTER(aurisys_handle->mLinearOut->p_buffer);
    AUDIO_FREE_POINTER(aurisys_handle->mLinearOut);

    UNLOCK_ALOCK(aurisys_handle->mAurisysLibManagerLock);
}


bool UpdateAurisysLibParam(
    struct audio_aurisys_handle_t *aurisys_handle,
    void *lib_name_param,
    uint32_t lib_name_param_sz) {
    char *lib_name = NULL;
    uint32_t lib_name_sz = 0;
    uint32_t lib_name_sz_align = 0;

    struct data_buf_t param_buf;

    int ret = 0;

    if (!aurisys_handle) {
        AUD_WARNING("aurisys_handle NULL!!");
        return false;
    }
    if (!lib_name_param || !lib_name_param_sz) {
        ALOGE("lib_name_param %p, lib_name_param_sz %u!!",
              lib_name_param, lib_name_param_sz);
        AUD_WARNING("aurisys_handle NULL!!");
        return false;
    }

    LOCK_ALOCK(aurisys_handle->mAurisysLibManagerLock);

    /* lib name */
    lib_name = (char *)lib_name_param;
    lib_name_sz = strlen(lib_name) + 1;
    lib_name_sz_align = lib_name_sz;
    if (lib_name_sz_align & 0x7) {
        lib_name_sz_align = ((lib_name_sz / 8) + 1) * 8;
    }

    /* param */
    param_buf.p_buffer = (uint8_t *)lib_name_param + lib_name_sz_align;
    param_buf.data_size = lib_name_param_sz - lib_name_sz_align;
    param_buf.memory_size = param_buf.data_size;

    ALOGV("LIB %s, param sz %u", lib_name, param_buf.data_size);

    ret = aurisys_update_param(
        aurisys_handle->manager,
        lib_name,
        &param_buf);

    UNLOCK_ALOCK(aurisys_handle->mAurisysLibManagerLock);

    return (ret == 0) ? true : false;
}


bool ApplyAurisysLibParamList(
    struct audio_aurisys_handle_t *aurisys_handle,
    struct data_buf_t *param_list,
    unsigned char task_scene) {

    int ret = 0;

    if (!aurisys_handle) {
        AUD_WARNING("aurisys_handle NULL!!");
        return false;
    }

    if (!param_list) {
        AUD_WARNING("param_list NULL!!");
        return false;
    }

    if (!aurisys_handle->manager) {
        AUD_WARNING("aurisys manager NULL!!");
        return false;
    }

    ALOGD("%s(), task_scene = %d", __FUNCTION__, task_scene);

    LOCK_ALOCK(aurisys_handle->mAurisysLibManagerLock);

    ret = aurisys_apply_param_list(aurisys_handle->manager, param_list);
    AUD_ASSERT(ret == 0);

    ret = aurisys_update_param_list(aurisys_handle->manager);
    AUD_ASSERT(ret == 0);

    UNLOCK_ALOCK(aurisys_handle->mAurisysLibManagerLock);

    return (ret == 0) ? true : false;
}



