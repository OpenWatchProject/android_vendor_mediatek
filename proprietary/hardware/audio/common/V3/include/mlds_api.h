/**
 ******************************************************************************
 * @file    mlds_api.h
 * @author  MTK audio driver team
 * @version V0.7
 * @date    29-June-2015
 * @brief   MLDS, Melodious Sound.
 *          Define audio/speech enhancement APIs for various usages.
 ******************************************************************************
 */

#ifndef MTK_MLDS_API_H
#define MTK_MLDS_API_H

#include <stdint.h>


#define MAKE_MLDS_API_VERSION(major, minor) (((uint8_t)(major)) << 8 | ((uint8_t)(minor)))
#define MLDS_API_VERSION MAKE_MLDS_API_VERSION(0, 7)


/*==============================================================================
 *                     typedefs
 *============================================================================*/

typedef enum {
    LIB_UNSUPPORTED  = -1,
    LIB_OK           =  0,
    LIB_ERROR        =  1,
    LIB_NULL_POINTER =  2,
} lib_status_t;


typedef enum {
    TASK_SCENE_PHONE_CALL,
    TASK_SCENE_VOW,
    TASK_SCENE_PLAYBACK_MP3,
    TASK_SCENE_RECORD,
    TASK_SCENE_VOIP,
    TASK_SCENE_SPEAKER_PROTECTION,
} task_scene_t;


typedef enum {
    TASK_DEVICE_IN_NONE             = 0,
    TASK_DEVICE_IN_PHONE_MIC        = (1 << 0),
    TASK_DEVICE_IN_HEADSET_MIC      = (1 << 1),
    TASK_DEVICE_IN_BLUETOOTH_MIC    = (1 << 2),
} task_device_in_t;


typedef enum {
    TASK_DEVICE_OUT_NONE        = 0,
    TASK_DEVICE_OUT_RECEIVER    = (1 << 0),
    TASK_DEVICE_OUT_SPEAKER     = (1 << 1),
    TASK_DEVICE_OUT_HEADPHONE   = (1 << 2),
    TASK_DEVICE_OUT_BLUETOOTH   = (1 << 3),
} task_device_out_t;


typedef enum {
    BIT_FORMAT_S16_LE = 0,  /* 16-bit signed pcm (2-bytes per channel) */
    BIT_FORMAT_S24_LE,      /* 24-bit signed pcm (4-bytes per channel) */
    BIT_FORMAT_S32_LE,      /* 32-bit signed pcm (4-bytes per channel) */
} bit_format_t;


typedef struct stream_info_t {
    uint16_t device;                /* task_device_in_t OR task_device_out_t */
    uint16_t device_extra_info;     /* reserve */
    uint32_t sample_rate_in;        /* 8000, 16000, 24000, 32000, ... */
    uint32_t sample_rate_out;       /* 8000, 16000, 24000, 32000, ... */
    uint8_t  bit_format_in;         /* bit_format_t */
    uint8_t  bit_format_out;        /* bit_format_t */
    uint8_t  num_channels_in;       /* 1, 2, 3, 4 */
    uint8_t  num_channels_out;      /* 1, 2, 3, 4 */
} stream_info_t;


typedef struct mlds_task_config_t {
    uint16_t api_version;           /* ex, 0x0120 => ver 1.32 */
    uint16_t vendor_id;             /* ex, 0x0E8D */
    uint8_t  task_scene;            /* task_scene_t */
    uint16_t frame_size_ms;         /* ex, 20ms */

    stream_info_t stream_uplink;    /* for record & phone call */
    stream_info_t stream_downlink;  /* for playback & phone call */
    union {
        stream_info_t stream_echo_ref;  /* for record & phone call */
        stream_info_t stream_dl_fbk;    /* for playback only */
    };
} mlds_task_config_t;

typedef struct mlds_interface {
    lib_status_t (*mlds_query_working_buf_size)(const mlds_task_config_t *p_mlds_task_config, uint32_t *p_working_buf_size);
    lib_status_t (*mlds_create_handler)(
        const mlds_task_config_t *p_mlds_task_config,
        const uint32_t param_buf_size,
        void *p_param_buf,
        const uint32_t working_buf_size,
        void *p_working_buf,
        void **pp_handler);
    lib_status_t (*mlds_process_ul_buf)(
        void *p_ul_buf_in,
        void *p_ul_buf_out,
        void *p_aec_buf_in,
        const uint32_t delay_ms,
        void *p_handler,
        void *arg);
    lib_status_t(* mlds_process_dl_buf)(
        void *p_dl_buf_in,
        uint32_t *InSize,
        void *p_dl_buf_out,
        uint32_t *OutSize,
        void *p_ref_buf,
        uint32_t *RefBufSize,
        void *p_handler,
        void *arg);
    lib_status_t (*mlds_destroy_handler)(void *p_handler);
    lib_status_t (*mlds_update_device)(
        const mlds_task_config_t *p_mlds_task_config,
        const uint32_t param_buf_size,
        void *p_param_buf,
        void *p_handler);
    lib_status_t (*mlds_update_param)(
        const mlds_task_config_t *p_mlds_task_config,
        const uint32_t param_buf_size,
        void *p_param_buf,
        void *p_handler);
    lib_status_t (*mlds_query_param_buf_size)(
        const mlds_task_config_t *p_mlds_task_config,
        const char *platform_name,
        const char *param_file_path,
        const int enhancement_mode,
        uint32_t *p_param_buf_size);
    lib_status_t (*mlds_parsing_param_file)(
        const mlds_task_config_t *p_mlds_task_config,
        const char *platform_name,
        const char *param_file_path,
        const int enhancement_mode,
        const uint32_t param_buf_size,
        void *p_param_buf);
    lib_status_t (*mlds_set_addr_value)(
        uint32_t addr,
        uint32_t value,
        void *p_handler);
    lib_status_t (*mlds_get_addr_value)(
        uint32_t addr,
        uint32_t *p_value,
        void *p_handler);
    lib_status_t (*mlds_set_ul_digital_gain)(
        const int16_t ul_analog_gain_ref_only,
        const int16_t ul_digital_gain,
        void *p_handler);
    lib_status_t (*mlds_set_dl_digital_gain)(
        const int16_t dl_analog_gain_ref_only,
        const int16_t dl_digital_gain,
        void *p_handler);
    lib_status_t (*mlds_set_ul_mute)(uint8_t b_mute_on, void *p_handler);
    lib_status_t (*mlds_set_dl_mute)(uint8_t b_mute_on, void *p_handler);
    lib_status_t (*mlds_set_ul_enhance)(uint8_t b_enhance_on, void *p_handler);
    lib_status_t (*mlds_set_dl_enhance)(uint8_t b_enhance_on, void *p_handler);
} mlds_interface;



/*==============================================================================
 *                     APIs
 *============================================================================*/

/**
 ******************************************************************************
 *  @brief Query the size of the working buffer
 *
 *  @param p_mlds_task_config the task configure
 *  @param p_working_buf_size the working buffer size.
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_query_working_buf_size(
    const mlds_task_config_t *p_mlds_task_config,
    uint32_t *p_working_buf_size);


/**
 ******************************************************************************
 *  @brief Create handler and initialize it
 *
 *  @param p_mlds_task_config the task configure
 *  @param param_buf_size the size of param_buf
 *  @param p_param_buf the enhancement parameters and lib related configure settings
 *  @param working_buf_size the size of working buffer
 *  @param p_working_buf the allocated buffer and the size is from mlds_create_handler()
 *  @param pp_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_create_handler(
    const mlds_task_config_t *p_mlds_task_config,
    const uint32_t param_buf_size,
    void *p_param_buf,
    const uint32_t working_buf_size,
    void *p_working_buf,
    void **pp_handler);


/**
 ******************************************************************************
 *  @brief Processing microphone/uplink data
 *
 *  @param p_ul_buf_in the microphone/uplink data to be Processed
 *                   ul_buf_in = ul_buf_mic1(1 frame) + ... + ul_buf_micN (1 frame)
 *  @param p_ul_buf_out the processed microphone/uplink data
 *  @param p_aec_buf_in the AEC reference data
 *  @param delay_ms the delay time(ms) of aec buf
 *  @param p_handler the handler of speech enhancement
 *  @param arg reserved field
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_process_ul_buf(
    void *p_ul_buf_in,
    void *p_ul_buf_out,
    void *p_aec_buf_in,
    const uint32_t delay_ms,
    void *p_handler,
    void *arg);


/**
 ******************************************************************************
 *  @brief Processing playback/downlink data
 *
 *  @param p_dl_buf_in the playback/downlink data to be Processed
 *  @InSize  Input:Inputsize of p_dl_buf_in / Output:buffersize consume p_dl_buf_in
 *  @param p_dl_buf_out the processed playback/downlink data
 *  @OutSize  Input:OutSize of p_dl_buf_out / Output:buffersize consume p_dl_buf_out
 *  @param p_ref_buf the reference buffer
 *  @OutSize  Input:OutSize of p_ref_buf / Output:buffersize consume p_ref_buf
 *  @param p_handler the handler of speech enhancement
 *  @param arg reserved field
 *
 *  @return lib_status_t
 ******************************************************************************
 */

lib_status_t mlds_process_dl_buf(
    void *p_dl_buf_in,
    uint32_t *InSize,
    void *p_dl_buf_out,
    uint32_t *OutSize,
    void *p_ref_buf,
    uint32_t *RefBufSize,
    void *p_handler,
    void *arg);


/**
 ******************************************************************************
 *  @brief deinitialize handler and destroy it (no need to free the working buffer)
 *
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_destroy_handler(void *p_handler);


/**
 ******************************************************************************
 *  @brief Update task device info
 *
 *  @param p_mlds_task_config the task configure
 *  @param param_buf_size the size of param_buf
 *  @param p_param_buf the enhancement parameters and lib related configure settings
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_update_device(
    const mlds_task_config_t *p_mlds_task_config,
    const uint32_t param_buf_size,
    void *p_param_buf,
    void *p_handler);


/**
 ******************************************************************************
 *  @brief Update speech enhancement parameters
 *
 *  @param p_mlds_task_config the task configure
 *  @param param_buf_size the size of param_buf
 *  @param p_param_buf the enhancement parameters and lib related configure settings
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_update_param(
    const mlds_task_config_t *p_mlds_task_config,
    const uint32_t param_buf_size,
    void *p_param_buf,
    void *p_handler);


/**
 ******************************************************************************
 *  @brief Query the buffer size to keep speech enhancement parameters(single mode)
 *         Implemented in HAL, but not OpenDSP.
 *
 *  @param p_mlds_task_config the task configure
 *  @param platform_name the platform name by "adb shell getprop ro.product.model"
 *  @param param_file_path the speech enhancement param file (fullset)
 *  @param enhancement_mode the speech enhancement mode by apk
 *  @param p_param_buf_size the pointer to a variable to keep the size of param buffer
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_query_param_buf_size(
    const mlds_task_config_t *p_mlds_task_config,
    const char *platform_name,
    const char *param_file_path,
    const int enhancement_mode,
    uint32_t *p_param_buf_size);


/**
 ******************************************************************************
 *  @brief Parsing param file to get param buffer(single mode)
 *         Implemented in HAL, but not OpenDSP.
 *
 *  @param p_mlds_task_config the task configure
 *  @param platform_name the platform name by "adb shell getprop ro.product.model"
 *  @param param_file_path the speech enhancement param file (fullset)
 *  @param enhancement_mode the speech enhancement mode by apk
 *  @param param_buf_size the size of param buffer
 *  @param p_param_buf the param buffer(single mode)
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_parsing_param_file(
    const mlds_task_config_t *p_mlds_task_config,
    const char *platform_name,
    const char *param_file_path,
    const int enhancement_mode,
    const uint32_t param_buf_size,
    void *p_param_buf);


/**
 ******************************************************************************
 *  @brief Set value at a specified address
 *
 *  @param addr the specified address
 *  @param value the value to be setted at the specified address
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_set_addr_value(
    uint32_t addr,
    uint32_t value,
    void *p_handler);


/**
 ******************************************************************************
 *  @brief Get value at a specified address
 *
 *  @param addr the specified address
 *  @param p_value the pointer to a variable to get the value at the specified address
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_get_addr_value(
    uint32_t addr,
    uint32_t *p_value,
    void *p_handler);


/**
 ******************************************************************************
 *  @brief set uplink digital gain
 *
 *  @param ul_analog_gain_ref_only uplink PGA gain. For reference only.
 *              The unit is 0.25 dB.
 *              value: 0x0000 => 0.0dB, 0xFFFC = -1.0dB, 0x0020 = +8.0dB
 *  @param ul_digital_gain uplink gain.
 *              The unit is 0.25 dB.
 *              So gain value [0, 120] => 0 to 30dB
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_set_ul_digital_gain(
    const int16_t ul_analog_gain_ref_only,
    const int16_t ul_digital_gain,
    void *p_handler);


/**
 ******************************************************************************
 *  @brief set downlink digital gain
 *
 *  @param dl_analog_gain_ref_only downlink PGA gain. For reference only.
 *              The unit is 0.25 dB.
 *              value: 0x0000 => 0.0dB, 0xFFFC = -1.0dB, 0x0020 = +8.0dB
 *  @param dl_digital_gain downlink gain.
                The unit is 0.25 dB.
 *              So gain value [-256, 0] => -64 to 0 dB
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_set_dl_digital_gain(
    const int16_t dl_analog_gain_ref_only,
    const int16_t dl_digital_gain,
    void *p_handler);


/**
 ******************************************************************************
 *  @brief mute/unmute uplink
 *
 *  @param b_mute_on mute uplink or not.
 *             - true: mute
 *             - false: unmute(default state)
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_set_ul_mute(uint8_t b_mute_on, void *p_handler);


/**
 ******************************************************************************
 *  @brief mute/unmute downlink
 *
 *  @param b_mute_on mute downlink or not.
 *             - true: mute
 *             - false: unmute(default state)
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_set_dl_mute(uint8_t b_mute_on, void *p_handler);


/**
 ******************************************************************************
 *  @brief enable/disable uplink enhancement function
 *
 *  @param b_enhance_on enable uplink speech enhancement or not.
 *             true: enable(default state)
 *             false: disable
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_set_ul_enhance(uint8_t b_enhance_on, void *p_handler);


/**
 ******************************************************************************
 *  @brief enable/disable downlink enhancement function
 *
 *  @param b_enhance_on enable downlink speech enhancement or not.
 *             - true: enable(default state)
 *             - false: disable
 *  @param p_handler the handler of speech enhancement
 *
 *  @return lib_status_t
 ******************************************************************************
 */
lib_status_t mlds_set_dl_enhance(uint8_t b_enhance_on, void *p_handler);


#endif // end of MTK_MLDS_API_H

