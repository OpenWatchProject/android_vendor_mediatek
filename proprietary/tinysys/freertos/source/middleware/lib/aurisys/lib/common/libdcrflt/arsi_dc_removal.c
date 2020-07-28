#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <assert.h>

#include <arsi_api.h> // implement MTK AURISYS API
#include <wrapped_audio.h>

#include "arsi_dc_removal.h"




static debug_log_fp_t print_log;

//-----------------------------------------------------------
// private function
//-----------------------------------------------------------

status_t dcremoval_arsi_set_key_value_pair(
    const string_buf_t *key_value_pair,
    void               *p_handler);
//-----------------------------------------------------------


status_t dcremoval_arsi_query_working_buf_size(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    uint32_t                 *p_working_buf_size,
    const debug_log_fp_t      debug_log_fp) {
    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_working_buf_size == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }
    unsigned int internal_buf_size_in_bytes = 0;

    DCR_GetBufferSize(&internal_buf_size_in_bytes);

    /* L.R coef buffer */
    *p_working_buf_size = internal_buf_size_in_bytes + sizeof(dcremoval_lib_handle_t);

    debug_log_fp("%s(), working_buf_size(%u) = working (%u) + libhandle (%u)\n", __func__, *p_working_buf_size, internal_buf_size_in_bytes, sizeof(dcremoval_lib_handle_t));
    return NO_ERROR;
}

status_t dcremoval_arsi_create_handler(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    data_buf_t               *p_working_buf,
    void                    **pp_handler,
    const debug_log_fp_t      debug_log_fp) {
    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL ||
        p_working_buf == NULL ||
        pp_handler == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    dcremoval_lib_handle_t *lib_handler = (dcremoval_lib_handle_t *)p_working_buf->p_buffer;
    unsigned int handler_size = sizeof(dcremoval_lib_handle_t);
    unsigned int channel;
    unsigned int samplerate;
    signed char *p_internal_buf;

    lib_handler->mLibWorkingBuf.mInternalBufSize = p_working_buf->memory_size - handler_size;
    lib_handler->mLibWorkingBuf.mpInternalBuf = p_working_buf->p_buffer + handler_size;
    p_internal_buf = lib_handler->mLibWorkingBuf.mpInternalBuf;
    /*set attribute*/
    channel = p_arsi_task_config->output_device_info.num_channels;
    samplerate = p_arsi_task_config->output_device_info.sample_rate;
    /*bit depth*/
    if (p_arsi_lib_config->audio_format == AUDIO_FORMAT_PCM_16_BIT) {
        lib_handler->mdcr_depth = DCREMOVE_BIT16;
    } else {
        lib_handler->mdcr_depth = DCREMOVE_BIT24;
    }
    /* work mode*/
    lib_handler->mdcrmode = DCR_MODE_3;
    /*currently use default param, and did not refernce mdcrmode */
    lib_handler->mDCRemoveHandle = DCR_Open(p_internal_buf, channel, samplerate, lib_handler->mdcrmode);

    if (!lib_handler->mDCRemoveHandle) {
        debug_log_fp("[DCR] Fail to get DCR Handle\n");
        return NO_INIT;
    }
    *pp_handler = lib_handler;

    return NO_ERROR;
}


status_t dcremoval_arsi_process_ul_buf(
    audio_buf_t *p_ul_buf_in,
    audio_buf_t *p_ul_buf_out,
    audio_buf_t *p_ul_ref_bufs,
    data_buf_t  *p_debug_dump_buf,
    void        *p_handler) {

    if (p_ul_buf_in == NULL ||
        p_ul_buf_out == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }

    return NO_ERROR;
}


status_t dcremoval_arsi_process_dl_buf(
    audio_buf_t *p_dl_buf_in,
    audio_buf_t *p_dl_buf_out,
    audio_buf_t *p_dl_ref_bufs,
    data_buf_t  *p_debug_dump_buf,
    void        *p_handler) {

    if (p_dl_buf_in == NULL ||
        p_dl_buf_out == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }
    dcremoval_lib_handle_t *lib_handler = NULL;
    lib_handler = (dcremoval_lib_handle_t *)p_handler;
    DCRemove_Handle *mHandle = lib_handler->mDCRemoveHandle;

    if (mHandle) {
        uint32_t inputBufSize  = p_dl_buf_in->data_buf.data_size;
        uint32_t outputBufSize = inputBufSize;
        void *pInputBuffer = p_dl_buf_in->data_buf.p_buffer;
        void *pOutputBuffer = p_dl_buf_out->data_buf.p_buffer;

        if (lib_handler->mdcr_depth == DCREMOVE_BIT24) {
            outputBufSize = DCR_Process_24(mHandle, (int *)pInputBuffer, &inputBufSize, (int *)pOutputBuffer, &outputBufSize);
        } else {
            outputBufSize = DCR_Process(mHandle, (short *)pInputBuffer, &inputBufSize, (short *)pOutputBuffer, &outputBufSize);
        }

        p_dl_buf_out->data_buf.data_size = outputBufSize;
        p_dl_buf_in->data_buf.data_size -= outputBufSize;  //remained input

    } else {
        //no init bypass the process
        memcpy(p_dl_buf_out->data_buf.p_buffer, p_dl_buf_in->data_buf.p_buffer,
               p_dl_buf_in->data_buf.data_size);
        p_dl_buf_out->data_buf.data_size = p_dl_buf_in->data_buf.data_size;
        p_dl_buf_in->data_buf.data_size = 0;
        print_log("Bypass other fast process\n");
        return NO_ERROR;
    }

    print_log("- DL raw: in->data_buf.p_buffer = %p in->data_buf.data_size = %d, out->data_buf.p_buffer = %p, out->data_buf.data_size = %d\n",
              p_dl_buf_in->data_buf.p_buffer,
              p_dl_buf_in->data_buf.data_size,
              p_dl_buf_out->data_buf.p_buffer,
              p_dl_buf_out->data_buf.data_size
             );
    return NO_ERROR;
}


status_t dcremoval_arsi_reset_handler(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler) {

    dcremoval_lib_handle_t *lib_handler = NULL;

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (dcremoval_lib_handle_t *)p_handler;
    DCRemove_Handle *mHandle = lib_handler->mDCRemoveHandle;
    unsigned int channel;
    unsigned int samplerate;
    if (mHandle == NULL) {
        return NO_INIT;
    }

    /*set attribute*/
    channel = p_arsi_task_config->output_device_info.num_channels;
    samplerate = p_arsi_task_config->output_device_info.sample_rate;
    /*bit depth*/
    if (p_arsi_lib_config->audio_format == AUDIO_FORMAT_PCM_16_BIT) {
        lib_handler->mdcr_depth = DCREMOVE_BIT16;
    } else {
        lib_handler->mdcr_depth = DCREMOVE_BIT24;
    }
    /* work mode*/
    lib_handler->mdcrmode = DCR_MODE_3;

    mHandle = DCR_ReConfig(mHandle, channel, samplerate, lib_handler->mdcrmode);
    print_log("%s(), done\n", __func__);

    return NO_ERROR;
}


status_t dcremoval_arsi_destroy_handler(void *p_handler) {

    dcremoval_lib_handle_t *lib_handler = (dcremoval_lib_handle_t *)p_handler;

    if (lib_handler->mDCRemoveHandle) {
        DCR_Close(lib_handler->mDCRemoveHandle);
    }

    print_log("%s(), p_handler = %p\n", __func__, p_handler);

    return NO_ERROR;
}


status_t dcremoval_arsi_update_device(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler) {
    //dcremoval_lib_handle_t *lib_handler = NULL;

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL ||
        p_handler == NULL) {
        return NO_INIT;
    }
    // should not happened? should reset buffer and re-open again?

    return NO_ERROR;
}


status_t dcremoval_arsi_update_param(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler) {

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        p_param_buf == NULL  ||
        p_handler == NULL) {
        return NO_INIT;
    }

    return NO_ERROR;
}


status_t dcremoval_arsi_query_param_buf_size(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const string_buf_t       *product_name,
    const string_buf_t       *param_file_path,
    const int32_t             enhancement_mode,
    uint32_t                 *p_param_buf_size,
    const debug_log_fp_t      debug_log_fp) {

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        product_name == NULL ||
        param_file_path == NULL ||
        p_param_buf_size == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    return NO_ERROR;
}


status_t dcremoval_arsi_parsing_param_file(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const string_buf_t       *product_name,
    const string_buf_t       *param_file_path,
    const int32_t             enhancement_mode,
    data_buf_t               *p_param_buf,
    const debug_log_fp_t      debug_log_fp) {

    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        product_name == NULL ||
        param_file_path == NULL ||
        p_param_buf == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    return NO_ERROR;
}


status_t dcremoval_arsi_set_addr_value(
    const uint32_t addr,
    const uint32_t value,
    void          *p_handler) {

    if (p_handler == NULL) {
        return NO_INIT;
    }

    dcremoval_lib_handle_t *lib_handler = NULL;
    lib_handler = (dcremoval_lib_handle_t *)p_handler;

    print_log("%s(), set value 0x%x at addr 0x%x\n",
              __func__, value, addr);

    return NO_ERROR;
}


status_t dcremoval_arsi_get_addr_value(
    const uint32_t addr,
    uint32_t      *p_value,
    void          *p_handler) {

    if (p_value == NULL || p_handler == NULL) {
        return NO_INIT;
    }

    dcremoval_lib_handle_t *lib_handler = NULL;
    lib_handler = (dcremoval_lib_handle_t *)p_handler;

    return NO_ERROR;
}


status_t dcremoval_arsi_set_key_value_pair(
    const string_buf_t *key_value_pair,
    void               *p_handler) {
    dcremoval_lib_handle_t *lib_handler = NULL;

    if (key_value_pair == NULL || p_handler == NULL) {
        return NO_INIT;
    }

    int set_value = 0;
    char *strval = NULL;
    char *pch = NULL;

    lib_handler = (dcremoval_lib_handle_t *)p_handler;
    pch = strchr(key_value_pair->p_string, '=');
    if (pch != NULL) {  // this line contain '=' character
        strval = pch + 1;
        set_value = atoi(strval);
        print_log("%s(), key value pair = %s value = %s\n",
                  __func__, key_value_pair->p_string, strval);
    } else {
        return BAD_VALUE;
    }

    return NO_ERROR;
}

status_t dcremoval_arsi_get_key_value_pair(
    string_buf_t *key_value_pair,
    void         *p_handler) {

    if (key_value_pair == NULL || p_handler == NULL) {
        return NO_INIT;
    }

    dcremoval_lib_handle_t *lib_handler = NULL;
    lib_handler = (dcremoval_lib_handle_t *)p_handler;
    print_log("+%s(), key = %s\n",
              __func__, key_value_pair->p_string);
    return NO_ERROR;
}


status_t dcremoval_arsi_set_ul_digital_gain(
    const int16_t ul_analog_gain_ref_only,
    const int16_t ul_digital_gain,
    void         *p_handler) {
    dcremoval_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }
    lib_handler = (dcremoval_lib_handle_t *)p_handler;
    return NO_ERROR;
}


status_t dcremoval_arsi_set_dl_digital_gain(
    const int16_t dl_analog_gain_ref_only,
    const int16_t dl_digital_gain,
    void         *p_handler) {
    dcremoval_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (dcremoval_lib_handle_t *)p_handler;
    return NO_ERROR;
}


status_t dcremoval_arsi_set_ul_mute(const uint8_t b_mute_on, void *p_handler) {
    dcremoval_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (dcremoval_lib_handle_t *)p_handler;
    return NO_ERROR;
}


status_t dcremoval_arsi_set_dl_mute(const uint8_t b_mute_on, void *p_handler) {
    dcremoval_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (dcremoval_lib_handle_t *)p_handler;

    print_log("%s b_mute_on = %d\n", __func__, b_mute_on);
    return NO_ERROR;
}


status_t dcremoval_arsi_set_ul_enhance(const uint8_t b_enhance_on,
                                       void *p_handler) {
    if (p_handler == NULL) {
        return NO_INIT;
    }
    return NO_ERROR;
}


status_t dcremoval_arsi_set_dl_enhance(const uint8_t b_enhance_on,
                                       void *p_handler) {
    dcremoval_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (dcremoval_lib_handle_t *)p_handler;

    return NO_ERROR;
}


status_t dcremoval_arsi_set_debug_log_fp(const debug_log_fp_t debug_log,
                                         void *p_handler) {
    dcremoval_lib_handle_t *lib_handler = NULL;

    if (p_handler == NULL) {
        return NO_INIT;
    }

    lib_handler = (dcremoval_lib_handle_t *)p_handler;
    print_log = debug_log;

    return NO_ERROR;
}

/*
status_t dcremoval_arsi_get_lib_version(string_buf_t *version_buf) {

    return NO_ERROR;
}
*/

status_t dcremoval_query_process_unit_bytes(uint32_t *p_uplink_bytes,
                                            uint32_t *p_downlink_bytes,
                                            void     *p_handler) {

    if (p_uplink_bytes == NULL ||
        p_downlink_bytes == NULL ||
        p_handler == NULL) {
        return BAD_VALUE;
    }

    *p_uplink_bytes = 0; // no uplink process
    *p_downlink_bytes = 0; // no need specific process size
    return NO_ERROR;
}
void dcremoval_arsi_assign_lib_fp(AurisysLibInterface *lib) {
    lib->arsi_get_lib_version = NULL;//dcremoval_arsi_get_lib_version;
    lib->arsi_query_working_buf_size = dcremoval_arsi_query_working_buf_size;
    lib->arsi_create_handler = dcremoval_arsi_create_handler;
    lib->arsi_process_ul_buf = dcremoval_arsi_process_ul_buf;
    lib->arsi_process_dl_buf = dcremoval_arsi_process_dl_buf;
    lib->arsi_reset_handler = dcremoval_arsi_reset_handler;
    lib->arsi_destroy_handler = dcremoval_arsi_destroy_handler;
    lib->arsi_update_device = dcremoval_arsi_update_device;
    lib->arsi_update_param = dcremoval_arsi_update_param;
    lib->arsi_query_param_buf_size = dcremoval_arsi_query_param_buf_size;
    lib->arsi_parsing_param_file = dcremoval_arsi_parsing_param_file;
    lib->arsi_set_addr_value = dcremoval_arsi_set_addr_value;
    lib->arsi_get_addr_value = dcremoval_arsi_get_addr_value;
    lib->arsi_set_key_value_pair = dcremoval_arsi_set_key_value_pair;
    lib->arsi_get_key_value_pair = dcremoval_arsi_get_key_value_pair;
    lib->arsi_set_ul_digital_gain = dcremoval_arsi_set_ul_digital_gain;
    lib->arsi_set_dl_digital_gain = dcremoval_arsi_set_dl_digital_gain;
    lib->arsi_set_ul_mute = dcremoval_arsi_set_ul_mute;
    lib->arsi_set_dl_mute = dcremoval_arsi_set_dl_mute;
    lib->arsi_set_ul_enhance = dcremoval_arsi_set_ul_enhance;
    lib->arsi_set_dl_enhance = dcremoval_arsi_set_dl_enhance;
    lib->arsi_set_debug_log_fp = dcremoval_arsi_set_debug_log_fp;
    lib->arsi_query_process_unit_bytes = dcremoval_query_process_unit_bytes;

}

