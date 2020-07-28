#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "arsi_api.h" // implement MTK AURISYS API
#include "wrapped_audio.h"

#include "enh_api.h"
#include <log/log.h>

typedef struct
{
    bool enh_ul_isMute;
    bool enh_dl_isMute;
} enh_drv_info_t;

extern status_t mtk_arsi_parsing_param_file_by_custom_info(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const string_buf_t       *product_info,
    const string_buf_t       *param_file_path,
    const string_buf_t       *custom_info,
    data_buf_t               *p_param_buf,
    const debug_log_fp_t      debug_log_fp);

status_t mtk_arsi_update_param(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler);

status_t mtk_arsi_set_debug_log_fp(const debug_log_fp_t debug_log,
                                   void *p_handler);


debug_log_fp_t enh_debug_log;
static enh_drv_info_t enh_drv_info;

#define printEnhLog(format, args...) \
    if(NULL != enh_debug_log) \
        enh_debug_log(format, ##args)

status_t mtk_arsi_get_lib_version(string_buf_t *version_buf)
{
    if (version_buf == NULL)
    {
        return BAD_VALUE;
    }

    if (version_buf->memory_size < (strlen(LIB_VERSION) + 1))
    {
        version_buf->string_size = 0;
        return NOT_ENOUGH_DATA;
    }

    strncpy(version_buf->p_string, LIB_VERSION, version_buf->memory_size-1);
    version_buf->string_size = strlen(LIB_VERSION);

    return OK;
}

status_t mtk_arsi_query_working_buf_size(
    const arsi_task_config_t *p_arsi_task_config __unused,
    const arsi_lib_config_t  *p_arsi_lib_config __unused,
    uint32_t                 *p_working_buf_size,
    const debug_log_fp_t      debug_log_fp __unused)
{
    *p_working_buf_size = ENH_API_Get_Memory(NULL);

    return OK;
}


status_t mtk_arsi_create_handler(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,  // CONTROL
    data_buf_t               *p_working_buf,  // ENH MEM
    void                    **pp_handler,
    const debug_log_fp_t      debug_log_fp)

{

    void *mem_ptr;
    // SPH_ENH_MEN should always leading the memory.
    SPH_ENH_ctrl_struct *p_SPH_ENH_ctrl_struct = (SPH_ENH_ctrl_struct *)p_param_buf->p_buffer;
    mem_ptr = p_working_buf->p_buffer;

    // SPH_ENH_MEM *Sph_Enh_mem = p_working_buf->p_buffer;
    debug_log_fp("mtk_arsi_create_handler\n");
    ALOGD(LIB_VERSION);

    if (p_arsi_lib_config == NULL)
    {
        return BAD_VALUE;
    }
    if (p_arsi_lib_config->p_ul_buf_in && p_arsi_lib_config->p_ul_buf_out)
    {
        debug_log_fp("ul_buf_in->num_channels = %d , ul_buf_out->num_channels = %d \n", p_arsi_lib_config->p_ul_buf_in->num_channels , p_arsi_lib_config->p_ul_buf_out->num_channels);
    }
    if (p_arsi_lib_config->p_dl_buf_in && p_arsi_lib_config->p_dl_buf_out)
    {
        debug_log_fp("dl_buf_in->num_channels = %d , dl_buf_out->num_channels = %d \n", p_arsi_lib_config->p_dl_buf_in->num_channels , p_arsi_lib_config->p_dl_buf_out->num_channels);
    }

    p_SPH_ENH_ctrl_struct->Fea_Cfg_table = 511;
    p_SPH_ENH_ctrl_struct->MMI_ctrl = 0xFFFFFFFD;
#if defined(MTK_VOIP_HANDSFREE_DMNR)
    if(p_arsi_task_config->max_input_device_num_channels>1)
    {
        p_SPH_ENH_ctrl_struct->MMI_ctrl |= 0x2;
    }
#endif

    p_SPH_ENH_ctrl_struct->sample_rate_codec = p_arsi_lib_config->sample_rate;
    p_SPH_ENH_ctrl_struct->frame_rate = p_arsi_lib_config->frame_size_ms; // shoule be 20
    p_SPH_ENH_ctrl_struct->DP2_AGC_GAIN = 0; // AP default value to avoid unexpected gain

    ENH_API_Alloc(p_SPH_ENH_ctrl_struct, (Word32 *)mem_ptr);
    *pp_handler = p_SPH_ENH_ctrl_struct->SCH_mem;
    mtk_arsi_set_debug_log_fp(debug_log_fp, *pp_handler);

    ALOGD("Current sample rate, %d \n", p_SPH_ENH_ctrl_struct->sample_rate_codec);
    if (p_SPH_ENH_ctrl_struct->sample_rate_codec == 48000)
    {
        if (p_SPH_ENH_ctrl_struct->App_table != STEREO_RECORD &&
            p_SPH_ENH_ctrl_struct->App_table != MONO_RECORD &&
            p_SPH_ENH_ctrl_struct->App_table != LOW_LATENCY_RECORD)
        {
            ALOGE("sample rate wrong, %d \n", p_SPH_ENH_ctrl_struct->sample_rate_codec);
            return BAD_VALUE;
        }
        else if (p_SPH_ENH_ctrl_struct->App_table == STEREO_RECORD ||
                 p_SPH_ENH_ctrl_struct->App_table == MONO_RECORD)
        {
            if (p_arsi_lib_config->frame_size_ms < 20)
            {
                ALOGE("Not LOW_LATENCY_RECORD, frame_size_ms wrong, %d \n", p_arsi_lib_config->frame_size_ms);
                return BAD_VALUE;
            }
        }
    }
    else if (p_SPH_ENH_ctrl_struct->sample_rate_codec == 16000)
    {
        if (p_SPH_ENH_ctrl_struct->App_table == STEREO_RECORD ||
            p_SPH_ENH_ctrl_struct->App_table == MONO_RECORD)
        {
            ALOGE("sample rate wrong, %d \n", p_SPH_ENH_ctrl_struct->sample_rate_codec);
            return BAD_VALUE;
        }
        else if (p_SPH_ENH_ctrl_struct->App_table != LOW_LATENCY_RECORD)
        {
            if (p_arsi_lib_config->frame_size_ms < 20)
            {
                ALOGE("Not LOW_LATENCY_RECORD, frame_size_ms wrong, %d \n", p_arsi_lib_config->frame_size_ms);
                return BAD_VALUE;
            }
        }
    }
    /*else if (p_SPH_ENH_ctrl_struct->sample_rate_codec == 8000)
    {
        if (p_SPH_ENH_ctrl_struct->App_table == NB_VOIP)
        {

        }
        else
        {
            debug_log_fp("sample rate wrong, %d \n", p_SPH_ENH_ctrl_struct->sample_rate_codec);
            return BAD_VALUE;
        }
    }*/
    else if (p_SPH_ENH_ctrl_struct->App_table != PHONE_CALL)
    {
        ALOGE("sample rate wrong, %d \n", p_SPH_ENH_ctrl_struct->sample_rate_codec);
        return BAD_VALUE;
    }

    ENH_API_Init(*pp_handler);

    debug_log_fp("lib_config: task_scene = %d, audio_mode: %d, max_ch_in: %d, input_source = %d\n",
                 p_arsi_task_config->task_scene, p_arsi_task_config->audio_mode,
                 p_arsi_task_config->max_input_device_num_channels, p_arsi_task_config->input_source);
    debug_log_fp("lib_config: interleve = %d, ms: %d, fs: %u, fmt = 0x%x\n",
                 p_arsi_lib_config->b_interleave, p_arsi_lib_config->frame_size_ms,
                 p_arsi_lib_config->sample_rate, p_arsi_lib_config->audio_format);
    mtk_arsi_update_param(p_arsi_task_config, p_arsi_lib_config, p_param_buf, *pp_handler);

    enh_drv_info.enh_dl_isMute = false;
    enh_drv_info.enh_ul_isMute = false;

    if(p_arsi_lib_config->audio_format == AUDIO_FORMAT_PCM_8_24_BIT
        && ((p_SPH_ENH_ctrl_struct->App_table == MONO_RECORD)
            || (p_SPH_ENH_ctrl_struct->App_table == STEREO_RECORD)
            || (p_SPH_ENH_ctrl_struct->App_table == LOW_LATENCY_RECORD))) {
        p_SPH_ENH_ctrl_struct->audio_format = ENH_API_AUDIO_FORMAT_PCM_8_24_BIT;
    } else {
        p_SPH_ENH_ctrl_struct->audio_format = ENH_API_AUDIO_FORMAT_PCM_16_BIT;
    }

    return OK;
}

status_t mtk_arsi_query_max_debug_dump_buf_size(
    data_buf_t *p_debug_dump_buf,
    void       *p_handler __unused)
{
    //  SCH_mem_struct *sch_mem = (SCH_mem_struct *)p_handler;

    p_debug_dump_buf->memory_size = sizeof(short) * 8640;

    return OK;
}

void memcpy_toBit16(short *to, void *from, int fromOffset, int sampleNum, bool needConvert)
{
    if(needConvert) {
        int *fromInt;
        int i;

        fromInt = ((int *)from) + fromOffset;
        for(i=0; i<sampleNum; i++)
        {
            to[i]= (short)(fromInt[i]>>8);
        }
    } else {
        memcpy(to, ((short *)from) + fromOffset, sampleNum*sizeof(short));
    }
}
void memcpy_fromBit16(void *to, int toOffset, short *from, int sampleNum, bool needConvert)
{
    if(needConvert) {
        int *toInt;
        int i;

        toInt = ((int *)to) + toOffset;
        for(i=0; i<sampleNum; i++)
        {
            if(from[i]<0) {
                toInt[i]= (int)(0xff000000 | (from[i]<<8));
            } else {
                toInt[i]= (int)(from[i]<<8);
            }
        }
    } else {
        //bits 16 to bits16
        memcpy(((short *)to) + toOffset, from, sampleNum*sizeof(short));
    }
}

status_t mtk_arsi_process_ul_buf(
    audio_buf_t *p_ul_buf_in,
    audio_buf_t *p_ul_buf_out,
    audio_buf_t *p_ul_ref_bufs,
    data_buf_t  *p_debug_dump_buf,
    void        *p_handler)
{
    SPH_ENH_ctrl_struct *Sph_Enh_ctrl = ENH_API_Get_ENH_ctrl_ptr(p_handler);
    // short *in_ptr;
    // short *out_ptr;
    int PCM_frame_length = (Sph_Enh_ctrl->sample_rate_codec == 48000) ? 960 : 320;
    short enh_lib_frame_len = ENH_API_Get_frame_length(p_handler); // value may change when low latency
    bool isProc24Bit = ( Sph_Enh_ctrl->App_table == MONO_RECORD
                      || Sph_Enh_ctrl->App_table == STEREO_RECORD
                      || Sph_Enh_ctrl->App_table == LOW_LATENCY_RECORD);

    printEnhLog("proc ul app=0x%x\n", Sph_Enh_ctrl->App_table);
    if (p_ul_buf_in == NULL || p_ul_buf_in->data_buf.p_buffer == NULL
        || p_ul_buf_out == NULL || p_ul_buf_out->data_buf.p_buffer == NULL)
    {
        return BAD_VALUE;
    }

    // EPL setting
    if (p_debug_dump_buf == NULL || p_debug_dump_buf->p_buffer == NULL)
    {
        Sph_Enh_ctrl->EPL_buffer = NULL;
    }
    else
    {
        Sph_Enh_ctrl->EPL_buffer = p_debug_dump_buf->p_buffer;
        p_debug_dump_buf->data_size = sizeof(short) * 8640; // get_EPL_dump_size(&sch_mem->EPL_logger);
    }

    printEnhLog("UL raw: type = %d, ch: %d, ch mask: 0x%x, fs for pcm: %u, fs for content: %u, fmt = 0x%x, frameSamples=%d\n",
                p_ul_buf_in->data_buf_type, p_ul_buf_in->num_channels, p_ul_buf_in->channel_mask,
                p_ul_buf_in->sample_rate_buffer, p_ul_buf_in->sample_rate_content, p_ul_buf_in->audio_format, enh_lib_frame_len);

    if (NULL != enh_debug_log) { enh_debug_log("Enhance = %d, mute = %d\n", ENH_API_Get_ul_enh_value(p_handler), enh_drv_info.enh_ul_isMute); }

    // EPL_logger->UL_enh_start_flag = 1;
#ifdef ipCheck
    if (sch_mem->UL_delay_frame_counter < sch_mem->common_delay_enhancement)
    {
        sch_mem->UL_delay_frame_counter = L_cyc_add(sch_mem->UL_delay_frame_counter, 16384, 1);
        EPL_logger->UL_enh_start_flag = 0;
    }
    else
    {
        sch_mem->UL_delay_frame_counter = L_cyc_add(sch_mem->UL_delay_frame_counter, 16384, 1);
        EPL_logger->UL_enh_start_flag = 1;
    }
#endif


    if (p_ul_buf_out->num_channels == p_ul_buf_in->num_channels)
    {
        p_ul_buf_out->data_buf.data_size = p_ul_buf_in->data_buf.data_size;
    }
    else if (p_ul_buf_in->num_channels == 1)
    {
        p_ul_buf_out->data_buf.data_size = p_ul_buf_in->data_buf.data_size << 1;
    }
    else if (p_ul_buf_in->num_channels == 2)
    {
        p_ul_buf_out->data_buf.data_size = p_ul_buf_in->data_buf.data_size >> 1;
    }
    else if (p_ul_buf_in->num_channels == 3)
    {
        p_ul_buf_out->data_buf.data_size = p_ul_buf_in->data_buf.data_size / 3 * p_ul_buf_out->num_channels;
    }

    if(isProc24Bit)  {

        int frame_len_in_byte = enh_lib_frame_len * sizeof(int);

        // expect 24bits input when processing in 24bits
        if (p_ul_buf_in->data_buf.data_size != enh_lib_frame_len * p_ul_buf_in->num_channels * sizeof(int))
        {
            return BAD_VALUE;
        }

        memcpy(Sph_Enh_ctrl->PCM_buffer_32, p_ul_buf_in->data_buf.p_buffer, frame_len_in_byte);
        if (p_ul_buf_in->num_channels >= 2)
        {
            memcpy(Sph_Enh_ctrl->PCM_buffer_32 + PCM_frame_length , (int *)(p_ul_buf_in->data_buf.p_buffer) + enh_lib_frame_len , frame_len_in_byte);
        }

        p_ul_buf_in->data_buf.data_size = 0;

        // 24-bits process only support record, so no reference data provide

    } else {

        bool needCovert = (AUDIO_FORMAT_PCM_8_24_BIT == p_ul_buf_in->audio_format);

        memcpy_toBit16(Sph_Enh_ctrl->PCM_buffer, p_ul_buf_in->data_buf.p_buffer, 0, enh_lib_frame_len, needCovert);
        if (p_ul_buf_in->num_channels >= 2)
        {
            memcpy_toBit16(Sph_Enh_ctrl->PCM_buffer + enh_lib_frame_len , p_ul_buf_in->data_buf.p_buffer, enh_lib_frame_len, enh_lib_frame_len, needCovert);
        }

        if ( Sph_Enh_ctrl->App_table == INCALL_RECORD && Sph_Enh_ctrl->Device_mode == 2 && p_ul_buf_in->num_channels == 1) {
            // For INCALL_RECORD UL only mode format, DL: ch1, UL: ch2
            // Copy UL to lib ch2 input.
            memcpy_toBit16(Sph_Enh_ctrl->PCM_buffer + enh_lib_frame_len , p_ul_buf_in->data_buf.p_buffer, 0, enh_lib_frame_len, needCovert);
        }

        p_ul_buf_in->data_buf.data_size = 0;

        if (p_ul_ref_bufs != NULL && p_ul_ref_bufs->data_buf.p_buffer != NULL)
        {
            memcpy_toBit16(Sph_Enh_ctrl->PCM_buffer + 3 * enh_lib_frame_len, p_ul_ref_bufs->data_buf.p_buffer, 0, enh_lib_frame_len, needCovert);
            p_ul_ref_bufs->data_buf.data_size = 0;
        }
        else
        {
            memset(Sph_Enh_ctrl->PCM_buffer + 3 * enh_lib_frame_len, 0, enh_lib_frame_len*sizeof(short));
        }
    }

    ENH_API_Process(Sph_Enh_ctrl);

    // Overwrite unwanted one.
    if (enh_drv_info.enh_ul_isMute)
    {
        memset(p_ul_buf_out->data_buf.p_buffer, 0, p_ul_buf_out->data_buf.data_size);
    }
    else
    {

        if(isProc24Bit)  {

            memcpy(p_ul_buf_out->data_buf.p_buffer , Sph_Enh_ctrl->PCM_buffer_32, enh_lib_frame_len * sizeof(int));
            if (p_ul_buf_out->num_channels >= 2)
            {
                memcpy((int *)(p_ul_buf_out->data_buf.p_buffer)+enh_lib_frame_len , Sph_Enh_ctrl->PCM_buffer_32 + PCM_frame_length ,  enh_lib_frame_len * sizeof(int));
            }

        } else {
            bool needCovert = (AUDIO_FORMAT_PCM_8_24_BIT == p_ul_buf_out->audio_format);

            // For incall recording pure DL or pure UL mode, 2 channel are required to be same at all.
            if (Sph_Enh_ctrl->App_table == INCALL_RECORD)
            {
                // under this case, no device mode information exist. the "Device_mode" is to know the recording mode
                // 0: both UL and DL are available, 1: uplink only, 2: downlink only
                if (Sph_Enh_ctrl->Device_mode == 2)   // UL
                {
                    memcpy(Sph_Enh_ctrl->PCM_buffer,  Sph_Enh_ctrl->PCM_buffer + enh_lib_frame_len, sizeof(short)* enh_lib_frame_len);
                }
                else if (Sph_Enh_ctrl->Device_mode == 1)  // DL
                {
                    memcpy(Sph_Enh_ctrl->PCM_buffer + enh_lib_frame_len, Sph_Enh_ctrl->PCM_buffer, sizeof(short)* enh_lib_frame_len);
                }
            }

            memcpy_fromBit16(p_ul_buf_out->data_buf.p_buffer, 0, Sph_Enh_ctrl->PCM_buffer, enh_lib_frame_len, needCovert);
            if (p_ul_buf_out->num_channels >= 2)
            {
                if (Sph_Enh_ctrl->App_table == PHONE_CALL)
                {
                    memcpy_fromBit16(p_ul_buf_out->data_buf.p_buffer, enh_lib_frame_len, Sph_Enh_ctrl->PCM_buffer, enh_lib_frame_len, needCovert);
                }
                else
                {
                    memcpy_fromBit16(p_ul_buf_out->data_buf.p_buffer, enh_lib_frame_len , Sph_Enh_ctrl->PCM_buffer + enh_lib_frame_len , enh_lib_frame_len, needCovert);
                }
            }
        }

    }

    if (p_debug_dump_buf != NULL && p_debug_dump_buf->p_buffer != NULL)
    {
        p_debug_dump_buf->data_size = ENH_API_Get_EPL_data_size(p_handler);
    }

    return OK;
}

status_t mtk_arsi_process_dl_buf(
    audio_buf_t *p_dl_buf_in,
    audio_buf_t *p_dl_buf_out,
    audio_buf_t *p_dl_ref_bufs __unused,
    data_buf_t  *p_debug_dump_buf,
    void        *p_handler)
{
    // SPH_ENH_MEM *sch_mem = (SPH_ENH_MEM *)p_handler;
    SPH_ENH_ctrl_struct *enh_param = (SPH_ENH_ctrl_struct *)ENH_API_Get_ENH_ctrl_ptr(p_handler);
    // EPL_logger_struct* EPL_logger = ENH_API_Get_EPL_logger_ptr(p_handler);
    short enh_lib_frame_len = ENH_API_Get_frame_length(p_handler);

    if (p_dl_buf_in == NULL || p_dl_buf_in->data_buf.p_buffer == NULL
        || p_dl_buf_out == NULL || p_dl_buf_out->data_buf.p_buffer == NULL)
    {
        return BAD_VALUE;
    }

    if (p_dl_buf_in->num_channels != 1
        || p_dl_buf_out->num_channels != 1
        || p_dl_buf_in->data_buf.data_size != enh_lib_frame_len * sizeof(short))
    {
        return BAD_VALUE;
    }

    if (p_debug_dump_buf == NULL || p_debug_dump_buf->p_buffer == NULL)
    {
        enh_param->EPL_buffer = NULL;
    }
    else
    {
        enh_param->EPL_buffer = p_debug_dump_buf->p_buffer;
        p_debug_dump_buf->data_size = sizeof(short) * 4800; // get_EPL_dump_size(&sch_mem->EPL_logger);
    }

    // EPL_logger->DL_enh_start_flag = 1;
#ifdef ipCheck
    if (sch_mem->DL_delay_frame_counter < sch_mem->common_delay_enhancement)
    {
        sch_mem->DL_delay_frame_counter = L_cyc_add(sch_mem->DL_delay_frame_counter, 16384, 1);
        EPL_logger->DL_enh_start_flag = 0;
    }
    else
    {
        sch_mem->DL_delay_frame_counter = L_cyc_add(sch_mem->DL_delay_frame_counter, 16384, 1);
        EPL_logger->DL_enh_start_flag = 1;
    }
#endif

    // start_EPL_logger_DL(EPL_logger, enh_param->EPL_buffer);


    p_dl_buf_out->data_buf.data_size = p_dl_buf_in->data_buf.data_size;
    memcpy(enh_param->PCM_buffer + 2 * enh_lib_frame_len, (short *)p_dl_buf_in->data_buf.p_buffer, p_dl_buf_out->data_buf.data_size);
    p_dl_buf_in->data_buf.data_size = 0;
    //  sch_mem->print_log("DL raw: type = %d, ch: %d, ch mask: 0x%x, fs for pcm: %u, fs for content: %u, fmt = 0x%x\n",
    //                           p_dl_buf_in->data_buf_type, p_dl_buf_in->num_channels, p_dl_buf_in->channel_mask,
    //                           p_dl_buf_in->sample_rate_buffer, p_dl_buf_in->sample_rate_content, p_dl_buf_in->audio_format);

    // write_EPL_logger(EPL_logger, PCM_CHANNEL_DL0, enh_param->PCM_buffer + 2 * enh_lib_frame_len, enh_lib_frame_len);


    /*-----------------------------------*/
    /*          ENH DL Process           */
    /*-----------------------------------*/

    ENH_API_DL_Process(enh_param);


    // write_EPL_logger(EPL_logger, PCM_CHANNEL_DL1, enh_param->PCM_buffer + 2 * enh_lib_frame_len, enh_lib_frame_len);

    if (enh_drv_info.enh_dl_isMute)
    {
        memset(p_dl_buf_out->data_buf.p_buffer, 0, p_dl_buf_out->data_buf.data_size);
    }
    else
    {
        memcpy((short *)p_dl_buf_out->data_buf.p_buffer, enh_param->PCM_buffer + 2 * enh_lib_frame_len, p_dl_buf_out->data_buf.data_size);
    }
    //  sch_mem->print_log("DL processed: type = %d, ch: %d, ch mask: 0x%x, fs for pcm: %u, fs for content: %u, fmt = 0x%x\n",
    //                           p_dl_buf_out->data_buf_type, p_dl_buf_out->num_channels, p_dl_buf_out->channel_mask,
    //                           p_dl_buf_out->sample_rate_buffer, p_dl_buf_out->sample_rate_content, p_dl_buf_out->audio_format);


    //VCE
    // Winnie: remember to add VCE here

    // end_EPL_logger(EPL_logger, enh_param);

    return OK;
}



status_t mtk_arsi_destroy_handler(void *p_handler)
{

    printEnhLog("Start destroy\n");
    ENH_API_Free(p_handler);

    return OK;
}


status_t mtk_arsi_update_param(
    const arsi_task_config_t *p_arsi_task_config __unused,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler)
{
    // SPH_ENH_MEM *Sph_Enh_mem = (SPH_ENH_MEM *)p_handler;
    SPH_ENH_ctrl_struct          *p_SPH_ENH_ctrl_struct = (SPH_ENH_ctrl_struct *)p_param_buf->p_buffer;
    p_SPH_ENH_ctrl_struct->Fea_Cfg_table = 511;
    p_SPH_ENH_ctrl_struct->MMI_ctrl = 0xFFFFFFFD;
#if defined(MTK_VOIP_HANDSFREE_DMNR)
    if(p_arsi_task_config->max_input_device_num_channels > 1)
    {
        p_SPH_ENH_ctrl_struct->MMI_ctrl |= 0x2;
    }
#endif
    p_SPH_ENH_ctrl_struct->SCH_mem = p_handler;

    p_SPH_ENH_ctrl_struct->sample_rate_codec = p_arsi_lib_config->sample_rate;
    p_SPH_ENH_ctrl_struct->frame_rate = p_arsi_lib_config->frame_size_ms;

    if (p_param_buf->data_size != sizeof(SPH_ENH_ctrl_struct))
    {
        if (NULL != enh_debug_log) { enh_debug_log("param size %u error!!\n", p_param_buf->data_size); }
        return BAD_VALUE;
    }

    printEnhLog("Update param\n");

    ENH_API_Update_Parameter(p_SPH_ENH_ctrl_struct);

    printEnhLog("Reset enhancement\n");

    ENH_API_Reset(p_handler);

    printEnhLog("Update Param done\n");

    return OK;
}


status_t mtk_arsi_reset_handler(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler)
{

    mtk_arsi_update_param(p_arsi_task_config, p_arsi_lib_config, p_param_buf, p_handler);
    return OK;

#if 0
    SPH_ENH_MEM *Sph_Enh_mem = p_handler;
    SPH_ENH_ctrl_struct *p_SPH_ENH_ctrl_struct = (SPH_ENH_ctrl_struct *)p_param_buf->p_buffer;

    // TODO: temp remove DEBUG_PRINT
    if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_SPEAKER)   // SPK mode
    {
        p_SPH_ENH_ctrl_struct->Device_mode = 2;
        // DEBUG_PRINT(("ENH_API", "Handfree Mode on"));
    }
    else
    {
        p_SPH_ENH_ctrl_struct->Device_mode = 0;
        // DEBUG_PRINT(("ENH_API", "Handset Mode on"));
    }
    Sph_Enh_mem->offset = 0;
    ENH_API_Alloc(p_SPH_ENH_ctrl_struct, (Word32 *)Sph_Enh_mem);


    mtk_arsi_update_param(p_arsi_task_config, p_arsi_lib_config, p_param_buf, p_handler);

    // Frame Index
    Sph_Enh_mem->Frame_Index = 0;

    return OK;
#endif
}

status_t mtk_arsi_update_device(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const data_buf_t         *p_param_buf,
    void                     *p_handler)
{
    if (NULL != enh_debug_log)
    {
        enh_debug_log("Update device\n");
        enh_debug_log("in_fs=%d, out_fs=%d\n", p_arsi_lib_config->p_ul_buf_in->sample_rate_buffer, p_arsi_lib_config->p_ul_buf_out->sample_rate_buffer);
        enh_debug_log("in_ch=%d, out_ch=%d\n", p_arsi_lib_config->p_ul_buf_in->num_channels, p_arsi_lib_config->p_ul_buf_out->num_channels);
        enh_debug_log("in->audio_format=%d, out = %d\n", p_arsi_lib_config->p_ul_buf_in->audio_format, p_arsi_lib_config->p_ul_buf_out->audio_format);
    }

    mtk_arsi_reset_handler(p_arsi_task_config, p_arsi_lib_config, p_param_buf, p_handler);

    printEnhLog("Device changed\n");
    return OK;
}


status_t mtk_arsi_query_param_buf_size_by_custom_info(
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
        custom_info == NULL ||
        p_param_buf_size == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    *p_param_buf_size = sizeof(SPH_ENH_ctrl_struct);
    return OK;
}



status_t mtk_arsi_set_addr_value(
    const uint32_t addr __unused,
    const uint32_t value __unused,
    void          *p_handler __unused)
{
    return INVALID_OPERATION;
}

status_t mtk_arsi_get_addr_value(
    const uint32_t addr __unused,
    uint32_t      *p_value,
    void          *p_handler __unused)
{
    *p_value = 0x1234; // should get param from handler

    return INVALID_OPERATION;
}

status_t mtk_arsi_set_key_value_pair(
    const string_buf_t *key_value_pair,
    void               *p_handler __unused)
{
#if 0
    if (strstr(key_value_pair->p_string,
               KEY_SET_AUDIO_CUSTOM_SCENE) != NULL)
    {
        char *value = key_value_pair->p_string + strlen(KEY_SET_AUDIO_CUSTOM_SCENE);

        if (!strcmp(value, ""))
        {
            strncpy(gScene, "Default", SCENE_NAME_MAX_LEN - 1);
        }
        else
        {
            strncpy(gScene, value, SCENE_NAME_MAX_LEN - 1);
        }

        printEnhLog("%s(), Scene = %s (%s)", __FUNCTION__, gScene, key_value_pair->p_string);

        return NO_ERROR;
    }

    return INVALID_OPERATION;
#endif
    printEnhLog("mtk_arsi_set_key_value_pair = %s", key_value_pair->p_string);
    return INVALID_OPERATION;
}

status_t mtk_arsi_get_key_value_pair(
    string_buf_t *key_value_pair,
    void         *p_handler __unused)
{
    printEnhLog("mtk_arsi_get_key_value_pair = %s", key_value_pair->p_string);

    return INVALID_OPERATION;
}

// FIXME: There is a bug that this function is called beyond initialization phase
//        That is, in comporessor_48k init phase, MIC_DG and MMI_MIC_GAIN is invalid
//        BUT compressor_48k do utilize these 2 values during init phase!!!!
status_t mtk_arsi_set_ul_digital_gain(
    const int16_t ul_analog_gain_ref_only,
    const int16_t ul_digital_gain,
    void         *p_handler)
{
    // SPH_ENH_MEM *mem = (SPH_ENH_MEM *)p_handler;
    SPH_ENH_ctrl_struct *enh_param = ENH_API_Get_ENH_ctrl_ptr(p_handler);

    if (NULL != enh_debug_log)
    {
        enh_debug_log("PGA = %d, ", ul_analog_gain_ref_only);
        enh_debug_log("DG = %d/4 dB\n", ul_digital_gain);
        enh_debug_log("Set ul_gian\n");
    }

    // enh_param->MMI_MIC_GAIN = 192 + 4 * (ul_analog_gain_ref_only + ul_digital_gain - 34);  // MT6351_EM setting
    enh_param->MMI_MIC_GAIN = 56 + ul_analog_gain_ref_only + ul_digital_gain;
    if (NULL != enh_debug_log) { enh_debug_log("MMI_MIC_GAIN=%d\n", enh_param->MMI_MIC_GAIN); }
    enh_param->DP2_AGC_GAIN = ul_digital_gain>>2;

    ENH_API_Update_Parameter(enh_param);

    if (NULL != enh_debug_log) { enh_debug_log("Reset enhancement\n"); }
    ENH_API_Reset(p_handler);

    return OK;
}

status_t mtk_arsi_set_dl_digital_gain(
    const int16_t dl_analog_gain_ref_only __unused,
    const int16_t dl_digital_gain __unused,
    void         *p_handler __unused)
{
    return OK;
}

status_t mtk_arsi_set_ul_mute(const uint8_t b_mute_on, void *p_handler)
{
    enh_drv_info.enh_ul_isMute = b_mute_on;
    ENH_API_Set_ul_mute(b_mute_on, p_handler);

    if (NULL != enh_debug_log) { enh_debug_log("ul_mute_on = %d\n", b_mute_on); }
    return OK;
}

status_t mtk_arsi_set_dl_mute(const uint8_t b_mute_on, void *p_handler)
{
    enh_drv_info.enh_dl_isMute = b_mute_on;
    ENH_API_Set_dl_mute(b_mute_on, p_handler);

    if (NULL != enh_debug_log) { enh_debug_log("dl_mute_on = %d\n", b_mute_on); }
    return OK;
}

status_t mtk_arsi_set_ul_enhance(const uint8_t b_enhance_on, void *p_handler)
{
    ENH_API_Set_ul_enhance(b_enhance_on, p_handler);

    if (NULL != enh_debug_log) { enh_debug_log("ul_enhance_on = %d\n", b_enhance_on); }
    return OK;
}

status_t mtk_arsi_set_dl_enhance(const uint8_t b_enhance_on, void *p_handler)
{
    ENH_API_Set_dl_enhance(b_enhance_on, p_handler);

    if (NULL != enh_debug_log) { enh_debug_log("dl_enhance_on = %d\n", b_enhance_on); }
    return OK;
}

status_t mtk_arsi_set_debug_log_fp(const debug_log_fp_t debug_log,
                                   void *p_handler)
{
    // SPH_ENH_MEM *my_private_handler;
    if (debug_log == NULL)
    {
        return NO_MEMORY;
    }

    enh_debug_log = debug_log;

    if (p_handler != NULL)
    {
        // my_private_handler = (SPH_ENH_MEM *)p_handler;
        debug_log("set_debug_log_fp\n");
    }
    else
    {
        debug_log("No debugfp\n");
        return NO_MEMORY;
    }

    SET_SCH_mem_print_log(p_handler, debug_log);
    debug_log("set log ptr\n");
    return OK;
}

void mtk_sp_team_arsi_assign_lib_fp(AurisysLibInterface *lib)
{
    lib->arsi_get_lib_version = mtk_arsi_get_lib_version;
    lib->arsi_query_working_buf_size = mtk_arsi_query_working_buf_size;
    lib->arsi_create_handler = mtk_arsi_create_handler;
    lib->arsi_query_max_debug_dump_buf_size = mtk_arsi_query_max_debug_dump_buf_size;
    lib->arsi_process_ul_buf = mtk_arsi_process_ul_buf;
    lib->arsi_process_dl_buf = mtk_arsi_process_dl_buf;
    lib->arsi_reset_handler = mtk_arsi_reset_handler;
    lib->arsi_destroy_handler = mtk_arsi_destroy_handler;
    lib->arsi_update_device = mtk_arsi_update_device;
    lib->arsi_update_param = mtk_arsi_update_param;
    lib->arsi_set_addr_value = mtk_arsi_set_addr_value;
    lib->arsi_get_addr_value = mtk_arsi_get_addr_value;
    lib->arsi_set_key_value_pair = mtk_arsi_set_key_value_pair;
    lib->arsi_get_key_value_pair = mtk_arsi_get_key_value_pair;
    lib->arsi_set_ul_digital_gain = mtk_arsi_set_ul_digital_gain;
    lib->arsi_set_dl_digital_gain = mtk_arsi_set_dl_digital_gain;
    lib->arsi_set_ul_mute = mtk_arsi_set_ul_mute;
    lib->arsi_set_dl_mute = mtk_arsi_set_dl_mute;
    lib->arsi_set_ul_enhance = mtk_arsi_set_ul_enhance;
    lib->arsi_set_dl_enhance = mtk_arsi_set_dl_enhance;
    lib->arsi_set_debug_log_fp = mtk_arsi_set_debug_log_fp;
    lib->arsi_query_param_buf_size_by_custom_info = mtk_arsi_query_param_buf_size_by_custom_info;
    lib->arsi_parsing_param_file_by_custom_info = mtk_arsi_parsing_param_file_by_custom_info;
}

void dynamic_link_arsi_assign_lib_fp(AurisysLibInterface *lib)
{
    mtk_sp_team_arsi_assign_lib_fp(lib);
}


