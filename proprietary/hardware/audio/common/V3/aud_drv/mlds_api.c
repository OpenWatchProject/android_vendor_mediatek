#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mlds_api.h"


typedef struct {
    uint32_t param1;
    uint32_t param2;
} my_private_enh_param_t;

typedef struct {
    mlds_task_config_t task_config;
    uint32_t tmp_buf_size;
    uint32_t my_private_var;
} my_private_handler_t;


lib_status_t mlds_query_working_buf_size(
    const mlds_task_config_t *p_mlds_task_config,
    uint32_t *p_working_buf_size) {
    /* calculate working buffer working_buf_size by task_config */
    printf("%s(), p_mlds_task_config->task_scene = %u\n",
           __func__, p_mlds_task_config->task_scene);
    printf("%s(), p_mlds_task_config->frame_size_ms = %u\n",
           __func__, p_mlds_task_config->frame_size_ms);

    printf("%s(), p_mlds_task_config->stream_uplink.device = %u\n",
           __func__, p_mlds_task_config->stream_uplink.device);

    printf("%s(), p_mlds_task_config->stream_uplink.sample_rate_in = %lu\n",
           __func__, p_mlds_task_config->stream_uplink.sample_rate_in);
    printf("%s(), p_mlds_task_config->stream_uplink.sample_rate_out = %lu\n",
           __func__, p_mlds_task_config->stream_uplink.sample_rate_out);

    printf("%s(), p_mlds_task_config->stream_uplink.bit_format_in = %d\n",
           __func__, p_mlds_task_config->stream_uplink.bit_format_in);
    printf("%s(), p_mlds_task_config->stream_uplink.bit_format_out = %d\n",
           __func__, p_mlds_task_config->stream_uplink.bit_format_out);

    printf("%s(), p_mlds_task_config->stream_uplink.num_channels_in = %d\n",
           __func__, p_mlds_task_config->stream_uplink.num_channels_in);
    printf("%s(), p_mlds_task_config->stream_uplink.num_channels_out = %d\n",
           __func__, p_mlds_task_config->stream_uplink.num_channels_out);


    printf("%s(), p_mlds_task_config->stream_downlink.device = %d\n",
           __func__, p_mlds_task_config->stream_downlink.device);

    printf("%s(), p_mlds_task_config->stream_downlink.sample_rate_in = %lu\n",
           __func__, p_mlds_task_config->stream_downlink.sample_rate_in);
    printf("%s(), p_mlds_task_config->stream_downlink.sample_rate_out = %lu\n",
           __func__, p_mlds_task_config->stream_downlink.sample_rate_out);

    printf("%s(), p_mlds_task_config->stream_downlink.bit_format_in = %d\n",
           __func__, p_mlds_task_config->stream_downlink.bit_format_in);
    printf("%s(), p_mlds_task_config->stream_downlink.bit_format_out = %d\n",
           __func__, p_mlds_task_config->stream_downlink.bit_format_out);

    printf("%s(), p_mlds_task_config->stream_downlink.num_channels_in = %d\n",
           __func__, p_mlds_task_config->stream_downlink.num_channels_in);
    printf("%s(), p_mlds_task_config->stream_downlink.num_channels_out = %d\n",
           __func__, p_mlds_task_config->stream_downlink.num_channels_out);

    *p_working_buf_size = sizeof(my_private_handler_t);

    printf("%s(), working_buf_size = %lu\n", __func__, *p_working_buf_size);
    return LIB_OK;
}


lib_status_t mlds_create_handler(
    const mlds_task_config_t *p_mlds_task_config,
    const uint32_t param_buf_size,
    void *p_param_buf,
    const uint32_t working_buf_size,
    void *p_working_buf,
    void **pp_handler) {
    printf("%s()\n", __func__);

    /* init handler by task_config */
    *pp_handler = p_working_buf;

    my_private_handler_t *my_private_handler = (my_private_handler_t *)*pp_handler;

    memcpy(&my_private_handler->task_config, p_mlds_task_config,
           sizeof(mlds_task_config_t));
    my_private_handler->tmp_buf_size = 640;
    my_private_handler->my_private_var = 0x5566;

    return LIB_OK;
}


lib_status_t mlds_process_ul_buf(
    void *p_ul_buf_in,
    void *p_ul_buf_out,
    void *p_aec_buf_in,
    const uint32_t delay_ms,
    void *p_handler,
    void *arg) {
    my_private_handler_t *my_private_handler = (my_private_handler_t *)p_handler;

    memcpy(p_ul_buf_out, p_ul_buf_in, my_private_handler->tmp_buf_size);
    return LIB_OK;
}


lib_status_t mlds_process_dl_buf(
    void *p_dl_buf_in,
    void *p_dl_buf_out,
    void *p_handler,
    void *arg) {
    my_private_handler_t *my_private_handler = (my_private_handler_t *)p_handler;

    memcpy(p_dl_buf_out, p_dl_buf_in, my_private_handler->tmp_buf_size);
    return LIB_OK;
}


lib_status_t mlds_destroy_handler(void *p_handler) {
    printf("%s(), p_handler = %p\n", __func__, p_handler);
    return LIB_OK;
}


lib_status_t mlds_update_device(
    const mlds_task_config_t *p_mlds_task_config,
    const uint32_t param_buf_size,
    void *p_param_buf,
    void *p_handler) {
    my_private_handler_t *my_private_handler = (my_private_handler_t *)p_handler;

    printf("%s(), my_private_handler->my_private_var = 0x%lx\n", __func__,
           my_private_handler->my_private_var);

    task_device_in_t device_in_old =
        my_private_handler->task_config.stream_uplink.device;
    task_device_in_t device_in_new =
        p_mlds_task_config->stream_uplink.device;

    task_device_out_t device_out_old =
        my_private_handler->task_config.stream_uplink.device;
    task_device_out_t device_out_new =
        p_mlds_task_config->stream_uplink.device;

    printf("%s(), input device: 0x%x => 0x%x\n", __func__, device_in_old,
           device_in_new);
    printf("%s(), output device: 0x%x => 0x%x\n", __func__, device_out_old,
           device_out_new);

    return LIB_OK;
}


lib_status_t mlds_update_param(
    const mlds_task_config_t *p_mlds_task_config,
    const uint32_t param_buf_size,
    void *p_param_buf,
    void *p_handler) {
    my_private_handler_t *my_private_handler = (my_private_handler_t *)p_handler;

    printf("%s(), my_private_handler->my_private_var = 0x%lx\n", __func__,
           my_private_handler->my_private_var);

    /* pick up new param and update handler */

    return LIB_OK;
}


lib_status_t mlds_query_param_buf_size(
    const mlds_task_config_t *p_mlds_task_config,
    const char *platform_name,
    const char *param_file_path,
    const int enhancement_mode,
    uint32_t *p_param_buf_size) {
    *p_param_buf_size = sizeof(my_private_enh_param_t);
    printf("%s(), get param buf size %lu for speech mode %d from file %s\n",
           __func__, *p_param_buf_size, enhancement_mode, param_file_path);

    return LIB_OK;
}


lib_status_t mlds_parsing_param_file(
    const mlds_task_config_t *p_mlds_task_config,
    const char *platform_name,
    const char *param_file_path,
    const int enhancement_mode,
    const uint32_t param_buf_size,
    void *p_param_buf) {
    my_private_enh_param_t *p_my_private_enh_param = NULL;

    printf("%s(), parsing file %s...\n", __func__, param_file_path);

    p_my_private_enh_param = (my_private_enh_param_t *)p_param_buf;

    p_my_private_enh_param->param1 = 0x12; // should get param from param_file_path
    p_my_private_enh_param->param2 = 0x34; // should get param from param_file_path

    return LIB_OK;
}


lib_status_t mlds_set_addr_value(
    uint32_t addr,
    uint32_t value,
    void *p_handler) {
    //my_private_handler_t *my_private_handler = (my_private_handler_t *)p_handler;
    printf("%s(), set value %lu at addr %p\n", __func__, value, (void *)addr);
    return LIB_OK;
}


lib_status_t mlds_get_addr_value(
    uint32_t addr,
    uint32_t *p_value,
    void *p_handler) {
    //my_private_handler_t *my_private_handler = (my_private_handler_t *)p_handler;
    *p_value = 0x1234; // should get param from handler

    printf("%s(), value %lu at addr %p\n", __func__, *p_value, (void *)addr);
    return LIB_OK;
}


lib_status_t mlds_set_ul_digital_gain(
    const int16_t ul_analog_gain_ref_only,
    const int16_t ul_digital_gain,
    void *p_handler) {
    printf("%s(), ul_digital_gain = %d\n", __func__, ul_digital_gain);
    return LIB_OK;
}


lib_status_t mlds_set_dl_digital_gain(
    const int16_t dl_analog_gain_ref_only,
    const int16_t dl_digital_gain,
    void *p_handler) {
    printf("%s(), dl_digital_gain = %d\n", __func__, dl_digital_gain);
    return LIB_OK;
}


lib_status_t mlds_set_ul_mute(uint8_t b_mute_on, void *p_handler) {
    printf("%s(), b_mute_on = %d\n", __func__, b_mute_on);
    return LIB_OK;
}


lib_status_t mlds_set_dl_mute(uint8_t b_mute_on, void *p_handler) {
    printf("%s(), b_mute_on = %d\n", __func__, b_mute_on);
    return LIB_OK;
}


lib_status_t mlds_set_ul_enhance(uint8_t b_enhance_on, void *p_handler) {
    printf("%s(), b_enhance_on = %d\n", __func__, b_enhance_on);
    return LIB_OK;
}


lib_status_t mlds_set_dl_enhance(uint8_t b_enhance_on, void *p_handler) {
    printf("%s(), b_enhance_on = %d\n", __func__, b_enhance_on);
    return LIB_OK;
}

