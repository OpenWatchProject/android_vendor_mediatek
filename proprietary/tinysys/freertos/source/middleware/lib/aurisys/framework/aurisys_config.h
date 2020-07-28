#ifndef MTK_AURISYS_CONFIG_H
#define MTK_AURISYS_CONFIG_H

#include <stdbool.h>

#include <uthash.h> /* uthash */

#include <audio_sample_rate.h>
#include <aurisys_scenario_dsp.h>

#include <arsi_type.h>


#ifdef __cplusplus
extern "C" {
#endif



/*
 * =============================================================================
 *                     macro
 * =============================================================================
 */

#define MAX_TOTAL_LIB_NUM (8)


#define MAX_NUM_LIB_IN_SCENE (4)
#define MAX_LIB_NAME_SIZE (32)
#define MAX_LIB_PATH_SIZE (64)
#define MAX_PARAM_PATH_SIZE (128)
#define MAX_DUMP_FILE_PATH_SIZE (128)
#define MAX_ADB_CMD_KEY_SIZE (16)
#define MAX_REF_BUF_SIZE (4)
#define MAX_CUSTOM_INFO_LEN (64)

#define AURISYS_GUARD_HEAD_VALUE (0x55667788)
#define AURISYS_GUARD_TAIL_VALUE (0xAABBCCDD)



/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct aurisys_lib_handler_t;
struct AurisysLibInterface;


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

/* --> for <aurisys_scenarios> */
typedef struct aurisys_library_name_t {
    char *name; /* key */

    UT_hash_handle hh; /* makes this structure hashable */
} aurisys_library_name_t;


typedef struct aurisys_scene_lib_table_t {
    uint32_t aurisys_scenario;  /* key */

    aurisys_library_name_t *uplink_library_name_list;
    aurisys_library_name_t *downlink_library_name_list;
    aurisys_library_name_t *all_library_name_list;

    char *uplink_digital_gain_lib_name;
    char *downlink_digital_gain_lib_name;

    UT_hash_handle hh; /* makes this structure hashable */
} aurisys_scene_lib_table_t;
/* <-- for <aurisys_scenarios> */


/* --> for <hal_librarys> */
typedef struct aurisys_component_t {
    uint32_t aurisys_scenario;  /* key */

    audio_sample_rate_mask_t sample_rate_mask;

    uint32_t support_format_mask; /* audio_support_format_mask_t */
    uint32_t support_frame_ms_mask; /* audio_support_frame_ms_mask_t */

    uint32_t support_channel_number_mask[NUM_DATA_BUF_TYPE]; /* audio_support_channel_number_mask_t */

    arsi_lib_config_t lib_config; /* keep max sample rate in component's lib_config */

    bool enable_log;
    bool enable_raw_dump;
    bool enable_lib_dump;
    uint32_t enhancement_mode;


    struct aurisys_lib_handler_t *lib_handler_list_for_adb_cmd; /* keep pointers for set/get param */

    UT_hash_handle hh; /* makes this structure hashable */
} aurisys_component_t;


typedef struct aurisys_library_config_t {
    char *name; /* key */
    char *lib_path;
    char *lib64_path;
    char *param_path;
    char *lib_dump_path;
    char *adb_cmd_key;

    void *dlopen_handle;
    struct AurisysLibInterface *api;

    aurisys_component_t *component_hh;

    UT_hash_handle hh; /* makes this structure hashable */
} aurisys_library_config_t;
/* <-- for <hal_librarys> */


typedef struct aurisys_config_t {
    aurisys_scene_lib_table_t *scene_lib_table_hh;
    aurisys_library_config_t  *library_config_hh;
} aurisys_config_t;



/*
 * =============================================================================
 *                     wrapper for dsp
 * =============================================================================
 */

/* --> for <aurisys_scenarios> */
struct aurisys_scene_lib_table_dsp_t {
    char uplink_library_name_list[MAX_NUM_LIB_IN_SCENE][MAX_LIB_NAME_SIZE];
    uint32_t num_uplink_library_name_list;

    char downlink_library_name_list[MAX_NUM_LIB_IN_SCENE][MAX_LIB_NAME_SIZE];
    uint32_t num_downlink_library_name_list;

    char uplink_digital_gain_lib_name[MAX_LIB_NAME_SIZE];
    char downlink_digital_gain_lib_name[MAX_LIB_NAME_SIZE];
};
/* <-- for <aurisys_scenarios> */


struct arsi_lib_config_dsp_t {
    /* Uplink */
    audio_buf_t p_ul_buf_in;   /* UL unprocessed buffer */
    audio_buf_t p_ul_buf_out;  /* UL processed buffer */
    audio_buf_t p_ul_ref_bufs[MAX_REF_BUF_SIZE]; /* UL reference buffer array */

    /* Downlink */
    audio_buf_t p_dl_buf_in;   /* DL unprocessed buffer */
    audio_buf_t p_dl_buf_out;  /* DL processed buffer */
    audio_buf_t p_dl_ref_bufs[MAX_REF_BUF_SIZE]; /* DL reference buffer array */

    uint32_t sample_rate;   /* 8000, 16000, 24000, 32000, ... */
    uint32_t audio_format;  /* audio_format_t */

    uint8_t  frame_size_ms; /* ex, 20ms; 0: not frame base. */
    uint8_t  b_interleave;  /* 0: non-interleave, 1: interleave */

    uint8_t  num_ul_ref_buf_array; /* how much ref bufs for ul processing */
    uint8_t  num_dl_ref_buf_array; /* how much ref bufs for dl processing */
    uint32_t reserve2;
};


/* --> for <hal_librarys> */
struct aurisys_component_dsp_t {
    uint32_t aurisys_scenario;

    uint32_t sample_rate_mask; /* audio_sample_rate_mask_t */

    uint32_t support_format_mask; /* audio_support_format_mask_t */
    uint32_t support_frame_ms_mask; /* audio_support_frame_ms_mask_t */

    union {
        uint32_t support_channel_number_mask[NUM_DATA_BUF_TYPE]; /* audio_support_channel_number_mask_t */
        uint32_t dummy_for_array[(NUM_DATA_BUF_TYPE + 1) & (~0x1)];
    };

    struct arsi_lib_config_dsp_t lib_config; /* keep max sample rate in component's lib_config */

    uint8_t enable_log;
    uint8_t enable_raw_dump;
    uint8_t enable_lib_dump;
    uint8_t __reserved;

    uint32_t enhancement_mode;
};


struct aurisys_library_config_dsp_t {
    char name[MAX_LIB_NAME_SIZE];
    char lib_path[MAX_LIB_PATH_SIZE];
    char lib64_path[MAX_LIB_PATH_SIZE];
    char param_path[MAX_PARAM_PATH_SIZE];
    char lib_dump_path[MAX_DUMP_FILE_PATH_SIZE];
    char adb_cmd_key[MAX_ADB_CMD_KEY_SIZE];

    struct aurisys_component_dsp_t component[AURISYS_SCENARIO_DSP_SIZE];

    union {
        struct AurisysLibInterface *api;
        uint32_t dummy_for_ptr[2]; /* reserve for 64 bits pointer only */
    };
};
/* <-- for <hal_librarys> */


struct aurisys_config_dsp_t {
    struct aurisys_scene_lib_table_dsp_t scene_lib_table[AURISYS_SCENARIO_DSP_SIZE];
    struct aurisys_library_config_dsp_t  library_config[MAX_TOTAL_LIB_NUM];
};


/*
 * =============================================================================
 *                     control config
 * =============================================================================
 */

enum { /* aurisys_core_type_t */
    AURISYS_CORE_HAL,
    AURISYS_CORE_HIFI3,
    NUM_AURISYS_PARSE_CONFIG
};


struct aurisys_lib_manager_config_t {
    struct arsi_task_config_t task_config;
    uint32_t aurisys_scenario;
    uint8_t  core_type;             /* aurisys_core_type_t */
    uint8_t  arsi_process_type;     /* arsi_process_type_t */
    uint8_t  frame_size_ms;         /* ex, 20ms; 0: not frame base. */
    uint8_t  num_channels_ul: 4;    /* 1, 2, 3, 4, ... */
    uint8_t  num_channels_dl: 4;    /* 1, 2, 3, 4, ... */

    char custom_info[MAX_CUSTOM_INFO_LEN];
    uint32_t audio_format;          /* audio_format_t */
    uint32_t sample_rate;           /* 8000, 16000, 24000, 32000, ... */
    uint8_t  dsp_task_scene;        /* hifi3 task scene*/
    uint8_t  reserved_8_1;
    uint8_t  reserved_8_2;
    uint8_t  reserved_8_3;
    uint32_t reserved;
}; /* 72 + 88 = 160 bytes */


struct stream_attribute_dsp {
    uint32_t audio_format;
    uint32_t audio_offload_format;
    uint32_t audio_channel_mask;
    uint32_t mAudioOutputFlags;
    uint32_t mAudioInputFlags;
    uint32_t output_devices;
    uint32_t input_device;
    uint32_t input_source;
    uint32_t num_channels;
    uint32_t sample_rate;
    uint32_t acoustics_mask;
    uint32_t latency_ms;
}; /* 48 bytes */


struct aurisys_gain_config_t {
    int16_t ul_digital_gain;
    int16_t ul_analog_gain;
    int16_t dl_digital_gain;
    int16_t dl_analog_gain;
};


struct aurisys_dsp_config_t {
    uint32_t guard_head;

    struct aurisys_lib_manager_config_t manager_config;
    struct stream_attribute_dsp attribute[NUM_DATA_BUF_TYPE];
    struct aurisys_gain_config_t gain_config;
    uint32_t __reserved;

    uint32_t voip_on : 1;
    uint32_t aec_on : 1;
    uint32_t iv_on : 1;
    uint32_t __reserved_32: 29;

    uint32_t guard_tail;
}; /* 520 bytes */


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of MTK_AURISYS_CONFIG_H */

