#ifndef DEMO_TYPE_H
#define DEMO_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



/* demo lib data structures */
typedef struct {
    uint32_t param1[16];
    uint16_t param2[32];
} demo_sph_param_t;

typedef struct {
    uint32_t frame_index;

    int8_t process_type; // 0: uplink, 1: downlink
    char pcm_debug_buf[1280];

    int16_t  analog_gain;
    int16_t  digital_gain;
    int16_t  mute_on;
    int16_t  enhance_on;
} demo_epl_buf_t;

typedef struct {
    arsi_task_config_t task_config;
    arsi_lib_config_t  lib_config;

    debug_log_fp_t print_log;
    demo_sph_param_t sph_param;

    uint32_t tmp_buf_size;
    uint32_t my_private_var;

    int16_t  ul_analog_gain;
    int16_t  ul_digital_gain;
    int16_t  dl_analog_gain;
    int16_t  dl_digital_gain;

    int16_t  b_ul_mute_on;
    int16_t  b_dl_mute_on;

    int16_t  b_ul_enhance_on;
    int16_t  b_dl_enhance_on;

    uint32_t value_at_addr_0x1234;

    uint32_t frame_index;
    char     data_buf[512];
} demo_lib_handle_t;



#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of DEMO_TYPE_H */

