/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef MTK_ARSI_API_H
#define MTK_ARSI_API_H

#include "arsi_type.h"
#include "wrapped_errors.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * =============================================================================
 *                     APIs
 * =============================================================================
 */

typedef struct AurisysLibInterface {

    /**
     * =========================================================================
     *  @brief get library's version
     *
     *  @param version_buf the string to keep library version (max length 128 bytes)
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_get_lib_version)(string_buf_t *version_buf);


    /**
     * =========================================================================
     *  @brief Query the size of the working buffer
     *
     *  @param p_arsi_task_config the task configure
     *  @param p_arsi_lib_config the lib configure
     *  @param p_working_buf_size the working buffer size
     *  @param debug_log_fp debug print function pointer
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_query_working_buf_size)(
        const arsi_task_config_t *p_arsi_task_config,
        const arsi_lib_config_t  *p_arsi_lib_config,
        uint32_t                 *p_working_buf_size,
        const debug_log_fp_t      debug_log_fp);


    /**
     * =========================================================================
     *  @brief Create handler and initialize it
     *
     *  @param p_arsi_task_config the task configure
     *  @param p_arsi_lib_config the lib configure
     *  @param p_param_buf the enhancement parameters and lib related configure settings
     *  @param p_working_buf the allocated buffer and the size is from arsi_create_handler()
     *  @param pp_handler the handler of speech enhancement
     *  @param debug_log_fp debug print function pointer
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_create_handler)(
        const arsi_task_config_t *p_arsi_task_config,
        const arsi_lib_config_t  *p_arsi_lib_config,
        const data_buf_t         *p_param_buf,
        data_buf_t               *p_working_buf,
        void                    **pp_handler,
        const debug_log_fp_t      debug_log_fp);


    /**
     * =========================================================================
     *  @brief Query the max size of debug dump buffer in processing
     *
     *  @param p_debug_dump_buf the debug dump buffer for ul/dl processing
     *             => assign p_debug_dump_buf->memory_size only
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_query_max_debug_dump_buf_size)(
        data_buf_t *p_debug_dump_buf,
        void       *p_handler);


    /**
     * =========================================================================
     *  @brief Processing microphone/uplink data
     *
     *  @param p_ul_buf_in the microphone/uplink data to be processed
     *         buffer arrangement: chunks of audio destined for different channels
                   => mic1(1 frame sz) + ... + micN (1 frame sz)
     *  @param p_ul_buf_out the processed microphone/uplink data
     *  @param p_ul_ref_bufs reference bufers (ex: AEC reference data, call info, ...)
     *  @param p_debug_dump_buf dump lib proprietary debug binary file
     *                          - write data in p_debug_dump_buf->p_buffer
     *                          - update buffer length in p_debug_dump_buf->data_size
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_process_ul_buf)(
        audio_buf_t *p_ul_buf_in,
        audio_buf_t *p_ul_buf_out,
        audio_buf_t *p_ul_ref_bufs,
        data_buf_t  *p_debug_dump_buf,
        void        *p_handler);


    /**
     * =========================================================================
     *  @brief Processing playback/downlink data
     *
     *  @param p_dl_buf_in the playback/downlink data to be Processed
     *  @param p_dl_buf_out the processed playback/downlink data
     *  @param p_dl_ref_bufs reference bufers (ex: I/V Buffer, call info, ...)
     *  @param p_debug_dump_buf dump lib proprietary debug binary file
     *                          - write data in p_debug_dump_buf->p_buffer
     *                          - update buffer length in p_debug_dump_buf->data_size
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_process_dl_buf)(
        audio_buf_t *p_dl_buf_in,
        audio_buf_t *p_dl_buf_out,
        audio_buf_t *p_dl_ref_bufs,
        data_buf_t  *p_debug_dump_buf,
        void        *p_handler);


    /**
     * =========================================================================
     *  @brief reset handler to init state
     *
     *  @param p_arsi_task_config the task configure
     *  @param p_arsi_lib_config the lib configure
     *  @param p_param_buf the enhancement parameters and lib related configure settings
     *  @param p_working_buf the allocated buffer and the size is from arsi_create_handler()
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_reset_handler)(
        const arsi_task_config_t *p_arsi_task_config,
        const arsi_lib_config_t  *p_arsi_lib_config,
        const data_buf_t         *p_param_buf,
        void                     *p_handler);


    /**
     * =========================================================================
     *  @brief deinitialize handler and destroy it (no need to free the working buffer)
     *
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_destroy_handler)(void *p_handler);


    /**
     * =========================================================================
     *  @brief Update task device info
     *
     *  @param p_arsi_task_config the task configure
     *  @param p_arsi_lib_config the lib configure
     *  @param p_param_buf the enhancement parameters and lib related configure settings
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_update_device)(
        const arsi_task_config_t *p_arsi_task_config,
        const arsi_lib_config_t  *p_arsi_lib_config,
        const data_buf_t         *p_param_buf,
        void                     *p_handler);


    /**
     * =========================================================================
     *  @brief Update speech enhancement parameters
     *
     *  @param p_arsi_task_config the task configure
     *  @param p_arsi_lib_config the lib configure
     *  @param p_param_buf the enhancement parameters and lib related configure settings
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_update_param)(
        const arsi_task_config_t *p_arsi_task_config,
        const arsi_lib_config_t  *p_arsi_lib_config,
        const data_buf_t         *p_param_buf,
        void                     *p_handler);


    /**
     * =========================================================================
     *  @brief phase out!! no need to imeplement
     * =========================================================================
     */
    status_t (*arsi_query_param_buf_size)(
        const arsi_task_config_t *p_arsi_task_config,
        const arsi_lib_config_t  *p_arsi_lib_config,
        const string_buf_t       *product_info,
        const string_buf_t       *param_file_path,
        const int32_t             enhancement_mode,
        uint32_t                 *p_param_buf_size,
        const debug_log_fp_t      debug_log_fp);


    /**
     * =========================================================================
     *  @brief phase out!! no need to imeplement
     * =========================================================================
     */
    status_t (*arsi_parsing_param_file)(
        const arsi_task_config_t *p_arsi_task_config,
        const arsi_lib_config_t  *p_arsi_lib_config,
        const string_buf_t       *product_info,
        const string_buf_t       *param_file_path,
        const int32_t             enhancement_mode,
        data_buf_t               *p_param_buf,
        const debug_log_fp_t      debug_log_fp);


    /**
     * =========================================================================
     *  @brief Set value at a specified address
     *
     *  @param addr the specified address
     *  @param value the value to be assigned at the specified address
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_set_addr_value)(
        const uint32_t addr,
        const uint32_t value,
        void          *p_handler);


    /**
     * =========================================================================
     *  @brief Get value from the specified address
     *
     *  @param addr the specified address
     *  @param p_value the value at the specified address
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_get_addr_value)(
        const uint32_t addr,
        uint32_t      *p_value,
        void          *p_handler);


    /**
     * =========================================================================
     *  @brief set key_value string to library
     *
     *  @param key_value_pair the "key=value" string
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_set_key_value_pair)(
        const string_buf_t *key_value_pair,
        void               *p_handler);


    /**
     * =========================================================================
     *  @brief get key_value string from library
     *
     *  @param key_value_pair there is only "key" when input,
               and then library need rewrite "key=value" to key_value_pair
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_get_key_value_pair)(
        string_buf_t *key_value_pair,
        void         *p_handler);


    /**
     * =========================================================================
     *  @brief set uplink digital gain
     *
     *  @param ul_analog_gain_ref_only uplink PGA gain. For reference only.
     *              The unit is 0.25 dB.
     *              value: 0x0000 => 0.0dB, 0xFFFC = -1.0dB, 0x0020 = +8.0dB
     *  @param ul_digital_gain uplink gain.
     *              The unit is 0.25 dB.
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_set_ul_digital_gain)(
        const int16_t ul_analog_gain_ref_only,
        const int16_t ul_digital_gain,
        void         *p_handler);


    /**
     * =========================================================================
     *  @brief set downlink digital gain
     *
     *  @param dl_analog_gain_ref_only downlink PGA gain. For reference only.
     *              The unit is 0.25 dB.
     *              value: 0x0000 => 0.0dB, 0xFFFC = -1.0dB, 0x0020 = +8.0dB
     *  @param dl_digital_gain downlink gain.
                    The unit is 0.25 dB.
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_set_dl_digital_gain)(
        const int16_t dl_analog_gain_ref_only,
        const int16_t dl_digital_gain,
        void         *p_handler);


    /**
     * =========================================================================
     *  @brief mute/unmute uplink
     *
     *  @param b_mute_on mute uplink or not.
     *             - true: mute
     *             - false: unmute(default state)
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_set_ul_mute)(const uint8_t b_mute_on, void *p_handler);


    /**
     * =========================================================================
     *  @brief mute/unmute downlink
     *
     *  @param b_mute_on mute downlink or not.
     *             - true: mute
     *             - false: unmute(default state)
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_set_dl_mute)(const uint8_t b_mute_on, void *p_handler);


    /**
     * =========================================================================
     *  @brief enable/disable uplink enhancement function
     *
     *  @param b_enhance_on enable uplink speech enhancement or not.
     *             true: enable(default state)
     *             false: disable
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_set_ul_enhance)(const uint8_t b_enhance_on, void *p_handler);


    /**
     * =========================================================================
     *  @brief enable/disable downlink enhancement function
     *
     *  @param b_enhance_on enable downlink speech enhancement or not.
     *             - true: enable(default state)
     *             - false: disable
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_set_dl_enhance)(const uint8_t b_enhance_on, void *p_handler);


    /**
     * =========================================================================
     *  @brief set debug log print callback function
     *
     *  @param debug_log log print function pointer
     *
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_set_debug_log_fp)(const debug_log_fp_t debug_log_fp,
                                      void *p_handler);


    /**
     * =========================================================================
     *  @brief get basic consumption for uplink/downlink processing
     *
     *  @param p_uplink_size need at least such amount of bytes to process
     *  @param p_downlink_size need at least such amount of bytes to process
     *             - 0: able to process any amount of data at one time
     *
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_query_process_unit_bytes)(
        uint32_t *p_uplink_bytes,
        uint32_t *p_downlink_bytes,
        void     *p_handler);


    /**
     * =========================================================================
     *  @brief load param, ex: fopen & fread, to library private buffer
     *
     *  @param product_info the product info by "key1=value1[,key2=value2[,key3=value3]]",
     *           there some dedicated key like
     *             1. "platform" from "adb shell getprop ro.mediatek.platform"
     *             2. "device"   from "adb shell getprop ro.product.device"
     *             3. "model"    from "adb shell getprop ro.product.model"
     *           each key-value is seperated by ','
     *         For example, product_info->p_string may like:
     *             "platform=MT3967,device=k3967_64,model=k3967_64_adsp"
     *  @param param_file_path the speech enhancement param file (fullset)
     *  @param debug_log_fp debug print function pointer
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_load_param)(
        const string_buf_t       *product_info,
        const string_buf_t       *param_file_path,
        const debug_log_fp_t      debug_log_fp);


    /**
     * =========================================================================
     *  @brief Query the buffer size to keep speech enhancement parameters
     *         Implemented in HAL, but not in OpenDSP.
     *
     *  @param p_arsi_task_config the task configure
     *  @param p_arsi_lib_config the lib configure
     *  @param product_info the product info by "key1=value1[,key2=value2[,key3=value3]]",
     *           there some dedicated key like
     *             1. "platform" from "adb shell getprop ro.mediatek.platform"
     *             2. "device"   from "adb shell getprop ro.product.device"
     *             3. "model"    from "adb shell getprop ro.product.model"
     *           each key-value is seperated by ','
     *         For example, product_info->p_string may like:
     *             "platform=MT3967,device=k3967_64,model=k3967_64_adsp"
     *  @param param_file_path the speech enhancement param file (fullset)
     *  @param custom_info the customized informations
     *  @param p_param_buf_size need how much memory size to keep the enhancement
               parameters for the specific device/mode.
     *  @param debug_log_fp debug print function pointer
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_query_param_buf_size_by_custom_info)(
        const arsi_task_config_t *p_arsi_task_config,
        const arsi_lib_config_t  *p_arsi_lib_config,
        const string_buf_t       *product_info,
        const string_buf_t       *param_file_path,
        const string_buf_t       *custom_info,
        uint32_t                 *p_param_buf_size,
        const debug_log_fp_t      debug_log_fp);


    /**
     * =========================================================================
     *  @brief Parsing param file to get parameters into p_param_buf
     *         Implemented in HAL, but not in OpenDSP.
     *
     *  @param p_arsi_task_config the task configure
     *  @param p_arsi_lib_config the lib configure
     *  @param product_info the product info by "key1=value1[,key2=value2[,key3=value3]]",
     *           there some dedicated key like
     *             1. "platform" from "adb shell getprop ro.mediatek.platform"
     *             2. "device"   from "adb shell getprop ro.product.device"
     *             3. "model"    from "adb shell getprop ro.product.model"
     *           each key-value is seperated by ','
     *         For example, product_info->p_string may like:
     *             "platform=MT3967,device=k3967_64,model=k3967_64_adsp"
     *  @param param_file_path the speech enhancement param file (fullset)
     *  @param custom_info the customized informations
     *  @param p_param_buf the enhancement parameters for the specific device/mode.
     *  @param debug_log_fp debug print function pointer
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_parsing_param_file_by_custom_info)(
        const arsi_task_config_t *p_arsi_task_config,
        const arsi_lib_config_t  *p_arsi_lib_config,
        const string_buf_t       *product_info,
        const string_buf_t       *param_file_path,
        const string_buf_t       *custom_info,
        data_buf_t               *p_param_buf,
        const debug_log_fp_t      debug_log_fp);


    /**
     * =========================================================================
     *  @brief set data buffer to library
     *
     *  @param id id for library to identify buffer usage
     *  @param p_data_buf data buffer set to library
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
    status_t (*arsi_set_buf)(
        const uint32_t    id,
        const data_buf_t *p_data_buf,
        void             *p_handler);


    /**
     * =========================================================================
     *  @brief get data buffer from library
     *
     *  @param id id for library to identify buffer usage
     *  @param p_data_buf data buffer get from library
     *  @param p_handler the handler of speech enhancement
     *
     *  @return status_t
     * =========================================================================
     */
     status_t (*arsi_get_buf)(
        const uint32_t id,
        data_buf_t    *p_data_buf,
        void          *p_handler);


} AurisysLibInterface;




#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of MTK_ARSI_API_H */

