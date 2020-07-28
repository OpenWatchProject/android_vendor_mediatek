#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <wrapped_audio.h>
#include <audio_task.h>
#include <arsi_call_type.h>

#include <arsi_api.h>
#include <arsi_library_entry_points.h> // declared library entry point

#include <aurisys_demo_lib_private_type.h>

#ifdef __cplusplus
extern "C" {
#endif


#define DEMO_LIB_VERSION "Ex: Version 1.3.0.24, Data: 2018-03-24"

static void *g_private_buf;


/* implement APIs */

status_t demo_arsi_get_lib_version(string_buf_t *version_buf)
{
    if (version_buf == NULL) {
        return BAD_VALUE;
    }

    if (version_buf->memory_size < (strlen(DEMO_LIB_VERSION) + 1)) {
        version_buf->string_size = 0;
        return NOT_ENOUGH_DATA;
    }

    strncpy(version_buf->p_string, DEMO_LIB_VERSION, version_buf->memory_size);
    version_buf->string_size = strlen(DEMO_LIB_VERSION);

    return NO_ERROR;
}


status_t demo_arsi_query_working_buf_size(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    uint32_t                 *p_working_buf_size,
    const debug_log_fp_t      debug_log_fp)
{
    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_working_buf_size == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    /* task scene */
    debug_log_fp("%s(), scene = %d\n", __func__, p_arsi_task_config->task_scene);

    /* input device */
    debug_log_fp("input dev: 0x%x, fmt = 0x%x, fs: %u, max fs: %u, ch: %d, max ch: %d\n",
                 p_arsi_task_config->input_device_info.devices,
                 p_arsi_task_config->input_device_info.audio_format,
                 p_arsi_task_config->input_device_info.sample_rate,
                 p_arsi_task_config->max_input_device_sample_rate,
                 p_arsi_task_config->input_device_info.num_channels,
                 p_arsi_task_config->max_input_device_num_channels);

    /* output device */
    debug_log_fp("output dev: 0x%x, fmt = 0x%x, fs: %u, max fs: %u, ch: %d, max ch: %d\n",
                 p_arsi_task_config->output_device_info.devices,
                 p_arsi_task_config->output_device_info.audio_format,
                 p_arsi_task_config->output_device_info.sample_rate,
                 p_arsi_task_config->max_output_device_sample_rate,
                 p_arsi_task_config->output_device_info.num_channels,
                 p_arsi_task_config->max_output_device_num_channels);


    /* lib */
    debug_log_fp("lib, working fs: %u, fmt: 0x%x, frame = %d, b_interleave = %d\n",
                 p_arsi_lib_config->sample_rate,
                 p_arsi_lib_config->audio_format,
                 p_arsi_lib_config->frame_size_ms,
                 p_arsi_lib_config->b_interleave);

    /* buffer */
    if (p_arsi_lib_config->p_ul_buf_in) {
        debug_log_fp("ul in, ch: %d, buf fs: %u, read data fs: %u, fmt: 0x%x\n",
                     p_arsi_lib_config->p_ul_buf_in->num_channels,
                     p_arsi_lib_config->p_ul_buf_in->sample_rate_buffer,
                     p_arsi_lib_config->p_ul_buf_in->sample_rate_content,
                     p_arsi_lib_config->p_ul_buf_in->audio_format);
    }

    if (p_arsi_lib_config->p_ul_buf_out) {
        debug_log_fp("ul out, ch: %d, buf fs: %u, read data fs: %u, fmt: 0x%x\n",
                     p_arsi_lib_config->p_ul_buf_out->num_channels,
                     p_arsi_lib_config->p_ul_buf_out->sample_rate_buffer,
                     p_arsi_lib_config->p_ul_buf_out->sample_rate_content,
                     p_arsi_lib_config->p_ul_buf_out->audio_format);
    }

    if (p_arsi_lib_config->p_dl_buf_in) {
        debug_log_fp("dl in, ch: %d, buf fs: %u, read data fs: %u, fmt: 0x%x\n",
                     p_arsi_lib_config->p_dl_buf_in->num_channels,
                     p_arsi_lib_config->p_dl_buf_in->sample_rate_buffer,
                     p_arsi_lib_config->p_dl_buf_in->sample_rate_content,
                     p_arsi_lib_config->p_dl_buf_in->audio_format);
    }

    if (p_arsi_lib_config->p_dl_buf_out) {
        debug_log_fp("dl out, ch: %d, buf fs: %u, read data fs: %u, fmt: 0x%x\n",
                     p_arsi_lib_config->p_dl_buf_out->num_channels,
                     p_arsi_lib_config->p_dl_buf_out->sample_rate_buffer,
                     p_arsi_lib_config->p_dl_buf_out->sample_rate_content,
                     p_arsi_lib_config->p_dl_buf_out->audio_format);
    }


    /* calculate working buffer working_buf_size by task config & lib config*/
    *p_working_buf_size = sizeof(demo_lib_handle_t);

    debug_log_fp("%s(), working_buf_size = %lu\n", __func__, *p_working_buf_size);

    return NO_ERROR;
}


status_t demo_arsi_create_handler(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    data_buf_t               *p_working_buf,
    void                    **pp_handler,
    const debug_log_fp_t      debug_log_fp)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL ||
        p_working_buf == NULL ||
        pp_handler == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    /* init handler by task_config */
    *pp_handler = p_working_buf->p_buffer;
    lib_handler = (demo_lib_handle_t *)*pp_handler;

    memcpy(&lib_handler->task_config,
           p_arsi_task_config,
           sizeof(arsi_task_config_t));

    memcpy(&lib_handler->lib_config,
           p_arsi_lib_config,
           sizeof(arsi_lib_config_t));

    lib_handler->print_log = debug_log_fp;

    if (p_param_buf->data_size != sizeof(demo_sph_param_t)) {
        debug_log_fp("param size %u error!!\n", p_param_buf->data_size);
        return BAD_VALUE;
    } else {
        memcpy(&lib_handler->sph_param,
               p_param_buf->p_buffer,
               sizeof(demo_sph_param_t));
    }

    lib_handler->tmp_buf_size = 1280;
    lib_handler->my_private_var = 0x5566;

    /* 0 dB */
    lib_handler->ul_analog_gain = 0;
    lib_handler->ul_digital_gain = 0;
    lib_handler->dl_analog_gain = 0;
    lib_handler->dl_digital_gain = 0;

    /* unmute dB */
    lib_handler->b_ul_mute_on = 0;
    lib_handler->b_dl_mute_on = 0;

    /* enhancement on */
    lib_handler->b_ul_enhance_on = 1;
    lib_handler->b_dl_enhance_on = 1;

    lib_handler->value_at_addr_0x1234 = 0;

    lib_handler->frame_index = 0;

    return NO_ERROR;
}

status_t demo_arsi_query_max_debug_dump_buf_size(
    data_buf_t *p_debug_dump_buf,
    void       *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL || p_debug_dump_buf == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    p_debug_dump_buf->memory_size = sizeof(demo_epl_buf_t);
    lib_handler->print_log("p_debug_dump_buf->memory_size = 0x%x\n",
                           p_debug_dump_buf->memory_size);

    return NO_ERROR;
}


status_t demo_arsi_process_ul_buf(
    audio_buf_t *p_ul_buf_in,
    audio_buf_t *p_ul_buf_out,
    audio_buf_t *p_ul_ref_bufs,
    data_buf_t  *p_debug_dump_buf,
    void        *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;
    extra_call_arg_t *p_extra_call_arg = NULL;
    uint32_t copy_size = 0;

    uint32_t i = 0;


    if (p_ul_buf_in == NULL ||
        p_ul_buf_out == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("UL raw: type = %d, ch: %d, ch mask: 0x%x, fs for pcm: %u, fs for content: %u, fmt = 0x%x\n",
                           p_ul_buf_in->data_buf_type,
                           p_ul_buf_in->num_channels,
                           p_ul_buf_in->channel_mask,
                           p_ul_buf_in->sample_rate_buffer,
                           p_ul_buf_in->sample_rate_content,
                           p_ul_buf_in->audio_format);

    lib_handler->print_log("UL processed: type = %d, ch: %d, ch mask: 0x%x, fs for pcm: %u, fs for content: %u, fmt = 0x%x\n",
                           p_ul_buf_out->data_buf_type,
                           p_ul_buf_out->num_channels,
                           p_ul_buf_out->channel_mask,
                           p_ul_buf_out->sample_rate_buffer,
                           p_ul_buf_out->sample_rate_content,
                           p_ul_buf_out->audio_format);

    if (lib_handler->b_ul_mute_on == 1) {
        memset(p_ul_buf_out->data_buf.p_buffer, 0, p_ul_buf_out->data_buf.memory_size);
        p_ul_buf_in->data_buf.data_size = 0;
        p_ul_buf_out->data_buf.data_size = p_ul_buf_out->data_buf.memory_size;
        return NO_ERROR;
    } else if (lib_handler->b_ul_enhance_on == 0) {
        // bypass (TODO: in ch / out ch)
        memcpy(
            p_ul_buf_out->data_buf.p_buffer,
            p_ul_buf_in->data_buf.p_buffer,
            p_ul_buf_in->data_buf.data_size);
        p_ul_buf_out->data_buf.data_size = p_ul_buf_in->data_buf.data_size;
        p_ul_buf_in->data_buf.data_size = 0;
        return NO_ERROR;
    }

    copy_size = p_ul_buf_in->data_buf.data_size / p_ul_buf_in->num_channels * p_ul_buf_out->num_channels;

    // process: only bypass 1ch from raw data
    memcpy(
        p_ul_buf_out->data_buf.p_buffer,
        p_ul_buf_in->data_buf.p_buffer,
        copy_size);
    p_ul_buf_out->data_buf.data_size = copy_size;
    p_ul_buf_in->data_buf.data_size = 0;

    if (p_ul_ref_bufs != NULL) {
        for (i = 0; i < lib_handler->lib_config.num_ul_ref_buf_array; i++) {
            lib_handler->print_log("data type %d\n", p_ul_ref_bufs[i].data_buf_type);

            switch (p_ul_ref_bufs[i].data_buf_type) {
            case DATA_BUF_ECHO_REF: {
                // demo clean echo ref buf
                memset(p_ul_ref_bufs[i].data_buf.p_buffer,
                       0,
                       p_ul_ref_bufs[i].data_buf.data_size);
                p_ul_ref_bufs[i].data_buf.data_size = 0;
                break;
            }
            case DATA_BUF_CALL_INFO: {
                p_extra_call_arg = (extra_call_arg_t *)p_ul_ref_bufs[i].data_buf.p_buffer;
                lib_handler->print_log("call_band_type: %d, call_net_type: %d\n",
                                       p_extra_call_arg->call_band_type,
                                       p_extra_call_arg->call_net_type);
                break;
            }
            default: {
                lib_handler->print_log("do nothing for data type %d\n",
                                       p_ul_ref_bufs[i].data_buf_type);
            }
            }
        }
    }

    lib_handler->frame_index++;
    //lib_handler->print_log("[UL] frame_index %u\n", lib_handler->frame_index);

    if (p_debug_dump_buf != NULL) { // might be NULL when debug dump turn off!!
        if (p_debug_dump_buf->p_buffer == NULL) {
            return NO_INIT;
        }
        if (p_debug_dump_buf->memory_size < sizeof(demo_epl_buf_t)) {
            return BAD_VALUE;
        }

        demo_epl_buf_t *demo_epl_buf = (demo_epl_buf_t *)p_debug_dump_buf->p_buffer;
        demo_epl_buf->frame_index = lib_handler->frame_index;
        demo_epl_buf->process_type = 0;
        memcpy(demo_epl_buf->pcm_debug_buf,
               (char *)p_ul_buf_in->data_buf.p_buffer + 1280, // copy UL ch2 as debug info
               sizeof(demo_epl_buf->pcm_debug_buf));

        demo_epl_buf->analog_gain = lib_handler->ul_analog_gain;
        demo_epl_buf->digital_gain = lib_handler->ul_digital_gain;
        demo_epl_buf->mute_on = lib_handler->b_ul_mute_on;
        demo_epl_buf->enhance_on = lib_handler->b_ul_enhance_on;

        p_debug_dump_buf->data_size = sizeof(demo_epl_buf_t);
    }

    return NO_ERROR;
}


status_t demo_arsi_process_dl_buf(
    audio_buf_t *p_dl_buf_in,
    audio_buf_t *p_dl_buf_out,
    audio_buf_t *p_dl_ref_bufs,
    data_buf_t  *p_debug_dump_buf,
    void        *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;
    extra_call_arg_t *p_extra_call_arg = NULL;

    uint32_t i = 0;

    if (p_dl_buf_in == NULL ||
        p_dl_buf_out == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("DL raw: type = %d, ch: %d, ch mask: 0x%x, fs for pcm: %u, fs for content: %u, fmt = 0x%x\n",
                           p_dl_buf_in->data_buf_type,
                           p_dl_buf_in->num_channels,
                           p_dl_buf_in->channel_mask,
                           p_dl_buf_in->sample_rate_buffer,
                           p_dl_buf_in->sample_rate_content,
                           p_dl_buf_in->audio_format);

    lib_handler->print_log("DL processed: type = %d, ch: %d, ch mask: 0x%x, fs for pcm: %u, fs for content: %u, fmt = 0x%x\n",
                           p_dl_buf_out->data_buf_type,
                           p_dl_buf_out->num_channels,
                           p_dl_buf_out->channel_mask,
                           p_dl_buf_out->sample_rate_buffer,
                           p_dl_buf_out->sample_rate_content,
                           p_dl_buf_out->audio_format);


    if (lib_handler->b_dl_mute_on == 1) {
        memset(p_dl_buf_out->data_buf.p_buffer, 0, p_dl_buf_out->data_buf.memory_size);
        p_dl_buf_in->data_buf.data_size = 0;
        p_dl_buf_out->data_buf.data_size = p_dl_buf_out->data_buf.memory_size;
        return NO_ERROR;
    } else if (lib_handler->b_dl_enhance_on == 0) {
        // bypass (TODO: in ch / out ch)
        memcpy(
            p_dl_buf_out->data_buf.p_buffer,
            p_dl_buf_in->data_buf.p_buffer,
            p_dl_buf_in->data_buf.data_size);
        p_dl_buf_out->data_buf.data_size = p_dl_buf_in->data_buf.data_size;
        p_dl_buf_in->data_buf.data_size = 0;
        return NO_ERROR;
    }


    // process: only bypass 1ch from raw data
    memcpy(
        p_dl_buf_out->data_buf.p_buffer,
        p_dl_buf_in->data_buf.p_buffer,
        p_dl_buf_in->data_buf.data_size);
    p_dl_buf_out->data_buf.data_size = p_dl_buf_in->data_buf.data_size;
    p_dl_buf_in->data_buf.data_size = 0;

    if (p_dl_ref_bufs != NULL) {
        for (i = 0; i < lib_handler->lib_config.num_dl_ref_buf_array; i++) {
            lib_handler->print_log("data type %d\n", p_dl_ref_bufs[i].data_buf_type);

            switch (p_dl_ref_bufs[i].data_buf_type) {
            case DATA_BUF_CALL_INFO: {
                p_extra_call_arg = (extra_call_arg_t *)p_dl_ref_bufs[i].data_buf.p_buffer;
                lib_handler->print_log("call_band_type: %d, call_net_type: %d\n",
                                       p_extra_call_arg->call_band_type,
                                       p_extra_call_arg->call_net_type);
                break;
            }
            default: {
                lib_handler->print_log("do nothing for data type %d\n",
                                       p_dl_ref_bufs[i].data_buf_type);
            }
            }
        }
    }

    lib_handler->frame_index++;
    //lib_handler->print_log("[DL] frame_index %u\n", lib_handler->frame_index);

    if (p_debug_dump_buf != NULL) { // might be NULL when debug dump turn off!!
        if (p_debug_dump_buf->p_buffer == NULL) {
            return NO_INIT;
        }
        if (p_debug_dump_buf->memory_size < sizeof(demo_epl_buf_t)) {
            return BAD_VALUE;
        }

        demo_epl_buf_t *demo_epl_buf = (demo_epl_buf_t *)p_debug_dump_buf->p_buffer;
        demo_epl_buf->frame_index = lib_handler->frame_index;
        demo_epl_buf->process_type = 1;
        memcpy(demo_epl_buf->pcm_debug_buf,
               p_dl_buf_in->data_buf.p_buffer, // copy DL ch1 as debug info
               sizeof(demo_epl_buf->pcm_debug_buf));

        demo_epl_buf->analog_gain = lib_handler->dl_analog_gain;
        demo_epl_buf->digital_gain = lib_handler->dl_digital_gain;
        demo_epl_buf->mute_on = lib_handler->b_dl_mute_on;
        demo_epl_buf->enhance_on = lib_handler->b_dl_enhance_on;

        p_debug_dump_buf->data_size = sizeof(demo_epl_buf_t);
    }

    return NO_ERROR;
}


status_t demo_arsi_reset_handler(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL ||
        p_handler == NULL) {
        return BAD_VALUE;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("%s(), p_handler = %p\n", __func__, p_handler);

    /* reset all private variables to init state */
    lib_handler->tmp_buf_size = 1280;
    lib_handler->my_private_var = 0x5566;

    /* 0 dB */
    lib_handler->ul_analog_gain = 0;
    lib_handler->ul_digital_gain = 0;
    lib_handler->dl_analog_gain = 0;
    lib_handler->dl_digital_gain = 0;

    /* unmute dB */
    lib_handler->b_ul_mute_on = 0;
    lib_handler->b_dl_mute_on = 0;

    /* enhancement on */
    lib_handler->b_ul_enhance_on = 1;
    lib_handler->b_dl_enhance_on = 1;

    lib_handler->value_at_addr_0x1234 = 0;

    return NO_ERROR;
}


status_t demo_arsi_destroy_handler(void *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("%s(), p_handler = %p\n", __func__, p_handler);

    return NO_ERROR;
}


status_t demo_arsi_update_device(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    audio_devices_t input_device_old =
        lib_handler->task_config.input_device_info.devices;
    audio_devices_t input_device_new =
        p_arsi_task_config->input_device_info.devices;

    audio_devices_t output_devices_old =
        lib_handler->task_config.output_device_info.devices;
    audio_devices_t output_devices_new =
        p_arsi_task_config->output_device_info.devices;

    lib_handler->print_log("input device: 0x%x => 0x%x\n",
                           input_device_old,
                           input_device_new);
    lib_handler->print_log("output device: 0x%x => 0x%x\n",
                           output_devices_old,
                           output_devices_new);

    lib_handler->print_log("input fs for pcm: %u -> %u, fs for content: %u -> %d\n",
                           lib_handler->lib_config.p_ul_buf_in->sample_rate_buffer,
                           p_arsi_lib_config->p_ul_buf_in->sample_rate_buffer,
                           lib_handler->lib_config.p_ul_buf_in->sample_rate_content,
                           p_arsi_lib_config->p_ul_buf_in->sample_rate_content);

    lib_handler->print_log("output fs for pcm: %u -> %u, fs for content: %u -> %d\n",
                           lib_handler->lib_config.p_ul_buf_in->sample_rate_buffer,
                           p_arsi_lib_config->p_ul_buf_in->sample_rate_buffer,
                           lib_handler->lib_config.p_ul_buf_in->sample_rate_content,
                           p_arsi_lib_config->p_ul_buf_in->sample_rate_content);

    if (p_param_buf->data_size != sizeof(demo_sph_param_t)) {
        lib_handler->print_log("param size %u error!!\n", p_param_buf->data_size);
        return BAD_VALUE;
    } else {
        memcpy(&lib_handler->sph_param,
               p_param_buf->p_buffer,
               sizeof(demo_sph_param_t));
    }

    /* update info */
    memcpy(&lib_handler->task_config,
           p_arsi_task_config,
           sizeof(arsi_task_config_t));

    memcpy(&lib_handler->lib_config,
           p_arsi_lib_config,
           sizeof(arsi_lib_config_t));

    return NO_ERROR;
}


status_t demo_arsi_update_param(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;


    /* pick up new param and update handler */
    if (p_param_buf->data_size != sizeof(demo_sph_param_t)) {
        lib_handler->print_log("param size %u error!!\n", p_param_buf->data_size);
        return BAD_VALUE;
    } else {
        memcpy(&lib_handler->sph_param,
               p_param_buf->p_buffer,
               sizeof(arsi_lib_config_t));
    }

    return NO_ERROR;
}


status_t demo_arsi_load_param(
    const string_buf_t       *product_info,
    const string_buf_t       *param_file_path,
    const debug_log_fp_t      debug_log_fp)
{
    if (product_info == NULL ||
        param_file_path == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    debug_log_fp("%s(), product_info %s, parsing file %s\n",
                 __func__, product_info->p_string, param_file_path->p_string);

    if (g_private_buf == NULL) {
        g_private_buf = malloc(1024);
        // NOTE: must use kal_pvPortMalloc() in FreeRTOS. #include <FreeRTOS.h>
    }

    // do fopen & fread to local private buf
    memset(g_private_buf, 0, 1024);


    debug_log_fp("%s(), alloc private buf %p\n", __func__, g_private_buf);

    return NO_ERROR;
}


status_t demo_arsi_query_param_buf_size_by_custom_info(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const string_buf_t       *product_info,
    const string_buf_t       *param_file_path,
    const string_buf_t       *custom_info,
    uint32_t                 *p_param_buf_size,
    const debug_log_fp_t      debug_log_fp)
{
    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        product_info == NULL ||
        param_file_path == NULL ||
        p_param_buf_size == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    *p_param_buf_size = sizeof(demo_sph_param_t);

    debug_log_fp("%s(), get param buf size %u for enh mode %s from file %s\n",
                 __func__, *p_param_buf_size, custom_info->p_string, param_file_path->p_string);

    return NO_ERROR;
}


status_t demo_arsi_parsing_param_file_by_custom_info(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const string_buf_t       *product_info,
    const string_buf_t       *param_file_path,
    const string_buf_t       *custom_info,
    data_buf_t               *p_param_buf,
    const debug_log_fp_t      debug_log_fp)
{
    demo_sph_param_t *p_sph_param = NULL;

    char key[32];
    char value[64];

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        product_info == NULL ||
        param_file_path == NULL ||
        p_param_buf == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    debug_log_fp("%s(), parsing file %s, custom_info = %s, private buf %p\n",
                 __func__, param_file_path->p_string, custom_info->p_string,
                 g_private_buf);

    p_sph_param = (demo_sph_param_t *)p_param_buf->p_buffer;
    debug_log_fp("p_sph_param: %p, memory_size: %u, sizeof(demo_sph_param_t) = %lu\n",
                 p_sph_param, p_param_buf->memory_size, sizeof(demo_sph_param_t));

    char *ptr = product_info->p_string;
    char *pkey = NULL;
    char *pvalue = NULL;
    while (*ptr != '\0' && strchr(ptr, '=') != NULL) {
        // get key
        pkey = key;
        while (*ptr != '=') {
            *pkey++ = *ptr++;
        }
        *pkey = '\0';

        // skip '='
        ptr++;

        // get value
        pvalue = value;
        while (*ptr != ',' && *ptr != '\0') {
            *pvalue++ = *ptr++;
        }
        *pvalue = '\0';
        debug_log_fp("product info: key: %s, value: %s\n", key, value);

        // skip ','
        if (*ptr == ',') {
            ptr++;
        }
    }

    // should get param from param_file_path


    memset((void *)p_sph_param->param1, 0x55, sizeof(p_sph_param->param1));
    memset((void *)p_sph_param->param2, 0x66, sizeof(p_sph_param->param2));
    p_param_buf->data_size = sizeof(demo_sph_param_t);

    return NO_ERROR;
}


status_t demo_arsi_set_addr_value(
    const uint32_t addr,
    const uint32_t value,
    void          *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("%s(), set value 0x%x at addr 0x%x\n",
                           __func__, value, addr);

    if (addr == 0x1234) {
        lib_handler->value_at_addr_0x1234 = value;
    }

    return NO_ERROR;
}


status_t demo_arsi_get_addr_value(
    const uint32_t addr,
    uint32_t      *p_value,
    void          *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_value == NULL || p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    if (addr == 0x1234) {
        *p_value = lib_handler->value_at_addr_0x1234;
    }

    lib_handler->print_log("%s(), value 0x%x at addr 0x%x\n",
                           __func__, *p_value, addr);
    return NO_ERROR;
}


status_t demo_arsi_set_key_value_pair(
    const string_buf_t *key_value_pair,
    void               *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (key_value_pair == NULL || p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("%s(), key value pair = %s\n",
                           __func__, key_value_pair->p_string);

    if (strcmp(key_value_pair->p_string, "HAHA=on") == 0) {
        lib_handler->my_private_var = 0x5566;
    } else if (strcmp(key_value_pair->p_string, "HAHA=off") == 0) {
        lib_handler->my_private_var = 0;
    }

    return NO_ERROR;
}


status_t demo_arsi_get_key_value_pair(
    string_buf_t *key_value_pair,
    void         *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;
    uint32_t left_space = 0;

    if (key_value_pair == NULL || p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("+%s(), key = %s, %u, %u\n",
                           __func__,
                           key_value_pair->p_string,
                           key_value_pair->memory_size,
                           key_value_pair->string_size);
    if (key_value_pair->memory_size <= (key_value_pair->string_size + 1)) {
        return NO_MEMORY;
    }
    left_space = key_value_pair->memory_size - (key_value_pair->string_size + 1);

    if (strcmp(key_value_pair->p_string, "HAHA") == 0) {
        if (lib_handler->my_private_var == 0) {
            strncat(key_value_pair->p_string, "=off", left_space);
        } else if (lib_handler->my_private_var == 0x5566) {
            strncat(key_value_pair->p_string, "=on", left_space);
        }
    }
    key_value_pair->string_size = strlen(key_value_pair->p_string);

    lib_handler->print_log("-%s(), key value pair => %s, %u, %u\n",
                           __func__,
                           key_value_pair->p_string,
                           key_value_pair->memory_size,
                           key_value_pair->string_size);
    return NO_ERROR;
}


status_t demo_arsi_set_ul_digital_gain(
    const int16_t ul_analog_gain_ref_only,
    const int16_t ul_digital_gain,
    void         *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("ul_digital_gain = %d\n", ul_digital_gain);

    lib_handler->ul_analog_gain  = ul_analog_gain_ref_only;
    lib_handler->ul_digital_gain = ul_digital_gain;

    return NO_ERROR;
}


status_t demo_arsi_set_dl_digital_gain(
    const int16_t dl_analog_gain_ref_only,
    const int16_t dl_digital_gain,
    void         *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("dl_digital_gain = %d\n", dl_digital_gain);

    lib_handler->dl_analog_gain  = dl_analog_gain_ref_only;
    lib_handler->dl_digital_gain = dl_digital_gain;

    return NO_ERROR;
}


status_t demo_arsi_set_ul_mute(const uint8_t b_mute_on, void *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("b_mute_on = %d\n", b_mute_on);

    lib_handler->b_ul_mute_on = b_mute_on;

    return NO_ERROR;
}


status_t demo_arsi_set_dl_mute(const uint8_t b_mute_on, void *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("b_mute_on = %d\n", b_mute_on);

    lib_handler->b_dl_mute_on = b_mute_on;

    return NO_ERROR;
}


status_t demo_arsi_set_ul_enhance(const uint8_t b_enhance_on, void *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("b_enhance_on = %d\n", b_enhance_on);

    lib_handler->b_ul_enhance_on = b_enhance_on;

    return NO_ERROR;
}


status_t demo_arsi_set_dl_enhance(const uint8_t b_enhance_on, void *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log("b_enhance_on = %d\n", b_enhance_on);

    lib_handler->b_dl_enhance_on = b_enhance_on;

    return NO_ERROR;
}


status_t demo_arsi_set_debug_log_fp(const debug_log_fp_t debug_log,
                                    void *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    lib_handler->print_log = debug_log;
    return NO_ERROR;
}


status_t demo_arsi_query_process_unit_bytes(
    uint32_t *p_uplink_bytes,
    uint32_t *p_downlink_bytes,
    void     *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;
    arsi_lib_config_t  *lib_config = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }
    if (p_uplink_bytes == NULL || p_downlink_bytes == NULL) {
        return INVALID_OPERATION;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;
    lib_config = &lib_handler->lib_config;

    if (lib_config->frame_size_ms == 0) { /* non frame base */
        /* assume at least 4K byte for each process */
        *p_uplink_bytes = 4096;
        *p_downlink_bytes = 4096;
    } else { /* frame base */
        /* UL */
        if (lib_config->p_ul_buf_in == NULL) {
            *p_uplink_bytes = 0;
        } else {
            *p_uplink_bytes =
                (AUDIO_BYTES_PER_SAMPLE(lib_config->p_ul_buf_in->audio_format) *
                 lib_config->p_ul_buf_in->num_channels *
                 lib_config->p_ul_buf_in->sample_rate_buffer *
                 lib_config->frame_size_ms) / 1000;
        }

        /* DL */
        if (lib_config->p_dl_buf_in == NULL) {
            *p_downlink_bytes = 0;
        } else {
            *p_downlink_bytes =
                (AUDIO_BYTES_PER_SAMPLE(lib_config->p_dl_buf_in->audio_format) *
                 lib_config->p_dl_buf_in->num_channels *
                 lib_config->p_dl_buf_in->sample_rate_buffer *
                 lib_config->frame_size_ms) / 1000;
        }
    }

    return NO_ERROR;
}


status_t demo_arsi_set_buf(
    const uint32_t    id,
    const data_buf_t *p_data_buf,
    void             *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    if (p_data_buf == NULL) {
        return BAD_VALUE;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    if (p_data_buf->data_size != sizeof(lib_handler->data_buf)) {
        return INVALID_OPERATION;
    }

    memcpy(lib_handler->data_buf, p_data_buf->p_buffer, p_data_buf->data_size);
    lib_handler->print_log("%s(), id = %d, p_data_buf: %p, %u, %u\n",
                           __func__, id, p_data_buf->p_buffer,
                           p_data_buf->data_size, p_data_buf->memory_size);


    return NO_ERROR;
}


status_t demo_arsi_get_buf(
    const uint32_t id,
    data_buf_t    *p_data_buf,
    void          *p_handler)
{
    demo_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    if (p_data_buf == NULL) {
        return BAD_VALUE;
    }

    lib_handler = (demo_lib_handle_t *)p_handler;

    if (p_data_buf->memory_size < sizeof(lib_handler->data_buf)) {
        return INVALID_OPERATION;
    }

    p_data_buf->data_size = sizeof(lib_handler->data_buf);
    memcpy(p_data_buf->p_buffer, lib_handler->data_buf, p_data_buf->data_size);
    lib_handler->print_log("%s(), id = %d, p_data_buf: %p, %u, %u\n",
                           __func__, id, p_data_buf->p_buffer,
                           p_data_buf->data_size, p_data_buf->memory_size);

    return NO_ERROR;
}


/* For static link, like libXXX.a in FreeRTOS --> */
void demo_arsi_assign_lib_fp(AurisysLibInterface *lib)
{
    lib->arsi_get_lib_version = demo_arsi_get_lib_version;
    lib->arsi_query_working_buf_size = demo_arsi_query_working_buf_size;
    lib->arsi_create_handler = demo_arsi_create_handler;
    lib->arsi_query_max_debug_dump_buf_size = demo_arsi_query_max_debug_dump_buf_size;
    lib->arsi_process_ul_buf = demo_arsi_process_ul_buf;
    lib->arsi_process_dl_buf = demo_arsi_process_dl_buf;
    lib->arsi_reset_handler = demo_arsi_reset_handler;
    lib->arsi_destroy_handler = demo_arsi_destroy_handler;
    lib->arsi_update_device = demo_arsi_update_device;
    lib->arsi_update_param = demo_arsi_update_param;
    lib->arsi_query_param_buf_size = NULL;
    lib->arsi_parsing_param_file = NULL;
    lib->arsi_set_addr_value = demo_arsi_set_addr_value;
    lib->arsi_get_addr_value = demo_arsi_get_addr_value;
    lib->arsi_set_key_value_pair = demo_arsi_set_key_value_pair;
    lib->arsi_get_key_value_pair = demo_arsi_get_key_value_pair;
    lib->arsi_set_ul_digital_gain = demo_arsi_set_ul_digital_gain;
    lib->arsi_set_dl_digital_gain = demo_arsi_set_dl_digital_gain;
    lib->arsi_set_ul_mute = demo_arsi_set_ul_mute;
    lib->arsi_set_dl_mute = demo_arsi_set_dl_mute;
    lib->arsi_set_ul_enhance = demo_arsi_set_ul_enhance;
    lib->arsi_set_dl_enhance = demo_arsi_set_dl_enhance;
    lib->arsi_set_debug_log_fp = demo_arsi_set_debug_log_fp;
    lib->arsi_query_process_unit_bytes = demo_arsi_query_process_unit_bytes;
    lib->arsi_load_param = demo_arsi_load_param;
    lib->arsi_query_param_buf_size_by_custom_info = demo_arsi_query_param_buf_size_by_custom_info;
    lib->arsi_parsing_param_file_by_custom_info = demo_arsi_parsing_param_file_by_custom_info;
    lib->arsi_set_buf = demo_arsi_set_buf;
    lib->arsi_get_buf = demo_arsi_get_buf;
}
/* <-- For static link, like libXXX.a in FreeRTOS */


#if 1
/* ONLY for dynamic link, like libXXX.so in HAL --> */
void dynamic_link_arsi_assign_lib_fp(AurisysLibInterface *lib)
{
    demo_arsi_assign_lib_fp(lib); /* like */
}
/* <-- ONLY for dynamic link, like libXXX.so in HAL */
#endif



#ifdef __cplusplus
}  /* extern "C" */
#endif

