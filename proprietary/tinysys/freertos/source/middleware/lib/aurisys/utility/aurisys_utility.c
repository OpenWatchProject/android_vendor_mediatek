#include "aurisys_utility.h"

#include <stdlib.h> /* atoi */

#include <string.h>

#include <wrapped_audio.h>

#include <audio_log_hal.h>
#include <audio_assert.h>
#include <audio_memory_control.h>

#include <audio_task.h>
#include <aurisys_scenario_dsp.h>

#include <arsi_type.h>
#include <aurisys_config.h>



#ifdef __cplusplus
extern "C" {
#endif


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "[AURI]"



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define AUDIO_FORMAT_TO_MASK(format)        (1 << (format))
#define AUDIO_FRAME_MS_TO_MASK(ms)          (1 << (ms))
#define AUDIO_CHANNEL_NUMBER_TO_MASK(ch)    (1 << (ch))



/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

typedef uint32_t audio_support_format_mask_t;

enum { /* audio_support_format_mask_t */
    AUDIO_SUPPORT_FORMAT_PCM_16_BIT_MASK    = AUDIO_FORMAT_TO_MASK(AUDIO_FORMAT_PCM_16_BIT),
    AUDIO_SUPPORT_FORMAT_PCM_32_BIT_MASK    = AUDIO_FORMAT_TO_MASK(AUDIO_FORMAT_PCM_32_BIT),
    AUDIO_SUPPORT_FORMAT_PCM_8_24_BIT_MASK  = AUDIO_FORMAT_TO_MASK(AUDIO_FORMAT_PCM_8_24_BIT)
};


typedef uint32_t audio_support_frame_ms_mask_t;

enum { /* audio_support_frame_ms_mask_t */
    AUDIO_SUPPORT_FRAME_00_MS_MASK  = AUDIO_FRAME_MS_TO_MASK(0),
    AUDIO_SUPPORT_FRAME_01_MS_MASK  = AUDIO_FRAME_MS_TO_MASK(1),
    AUDIO_SUPPORT_FRAME_03_MS_MASK  = AUDIO_FRAME_MS_TO_MASK(3),
    AUDIO_SUPPORT_FRAME_05_MS_MASK  = AUDIO_FRAME_MS_TO_MASK(5),
    AUDIO_SUPPORT_FRAME_10_MS_MASK  = AUDIO_FRAME_MS_TO_MASK(10),
    AUDIO_SUPPORT_FRAME_20_MS_MASK  = AUDIO_FRAME_MS_TO_MASK(20)
};


typedef uint32_t audio_support_channel_number_mask_t;

enum { /* audio_support_channel_number_mask_t */
    AUDIO_SUPPORT_CHANNEL_NUMBER_1_MASK = AUDIO_CHANNEL_NUMBER_TO_MASK(1),
    AUDIO_SUPPORT_CHANNEL_NUMBER_2_MASK = AUDIO_CHANNEL_NUMBER_TO_MASK(2),
    AUDIO_SUPPORT_CHANNEL_NUMBER_3_MASK = AUDIO_CHANNEL_NUMBER_TO_MASK(3),
    AUDIO_SUPPORT_CHANNEL_NUMBER_4_MASK = AUDIO_CHANNEL_NUMBER_TO_MASK(4)
};


typedef void (*match_mask_by_string_fp_t)(uint32_t *p_mask, const char *string, const uint32_t string_size);



/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */

static const string_to_enum_pair_t g_s2e_aurisys_scenario_dsp[] = {
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_PRIMARY", AURISYS_SCENARIO_DSP_PRIMARY),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_OFFLOAD", AURISYS_SCENARIO_DSP_OFFLOAD),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_DEEP_BUF", AURISYS_SCENARIO_DSP_DEEP_BUF),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_FAST", AURISYS_SCENARIO_DSP_FAST),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_PLAYBACK", AURISYS_SCENARIO_DSP_PLAYBACK),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_RECORD", AURISYS_SCENARIO_DSP_RECORD),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_RECORD_FAST", AURISYS_SCENARIO_DSP_RECORD_FAST),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_PHONE_CALL", AURISYS_SCENARIO_DSP_PHONE_CALL),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_VOIP", AURISYS_SCENARIO_DSP_VOIP),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_CALL_FINAL", AURISYS_SCENARIO_DSP_CALL_FINAL),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_PLAYBACK_SMARTPA", AURISYS_SCENARIO_DSP_PLAYBACK_SMARTPA),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_KTV", AURISYS_SCENARIO_DSP_KTV),
    STRING_TO_ENUM("AURISYS_SCENARIO_DSP_ALL", AURISYS_SCENARIO_DSP_ALL)
};


static const string_to_enum_pair_t g_s2e_audio_format[] = {
    STRING_TO_ENUM("AUDIO_FORMAT_PCM_16_BIT", AUDIO_FORMAT_PCM_16_BIT),
    STRING_TO_ENUM("AUDIO_FORMAT_PCM_8_BIT", AUDIO_FORMAT_PCM_8_BIT),
    STRING_TO_ENUM("AUDIO_FORMAT_PCM_32_BIT", AUDIO_FORMAT_PCM_32_BIT),
    STRING_TO_ENUM("AUDIO_FORMAT_PCM_8_24_BIT", AUDIO_FORMAT_PCM_8_24_BIT),
};

static const string_to_enum_pair_t g_s2e_data_buf_type[] = {
    STRING_TO_ENUM("DATA_BUF_UPLINK_IN", DATA_BUF_UPLINK_IN),
    STRING_TO_ENUM("DATA_BUF_UPLINK_OUT", DATA_BUF_UPLINK_OUT),
    STRING_TO_ENUM("DATA_BUF_DOWNLINK_IN", DATA_BUF_DOWNLINK_IN),
    STRING_TO_ENUM("DATA_BUF_DOWNLINK_OUT", DATA_BUF_DOWNLINK_OUT),
    STRING_TO_ENUM("DATA_BUF_ECHO_REF", DATA_BUF_ECHO_REF),
    STRING_TO_ENUM("DATA_BUF_IV_BUFFER", DATA_BUF_IV_BUFFER),
    STRING_TO_ENUM("DATA_BUF_CALL_INFO", DATA_BUF_CALL_INFO),
};


/*
 * =============================================================================
 *                     private function declaration
 * =============================================================================
 */


/*
 * =============================================================================
 *                     utilities declaration
 * =============================================================================
 */


/*
 * =============================================================================
 *                     public function implementation
 * =============================================================================
 */

uint8_t map_aurisys_scenario_to_task_scene(const uint32_t aurisys_scenario) {
    uint8_t task_scene = TASK_SCENE_INVALID;

    switch (aurisys_scenario) {
    case AURISYS_SCENARIO_DSP_PRIMARY:
        task_scene = TASK_SCENE_PRIMARY;
        break;
    case AURISYS_SCENARIO_DSP_OFFLOAD:
        task_scene = TASK_SCENE_PLAYBACK_MP3;
        break;
    case AURISYS_SCENARIO_DSP_DEEP_BUF:
        task_scene = TASK_SCENE_DEEPBUFFER;
        break;
    case AURISYS_SCENARIO_DSP_PLAYBACK:
    case AURISYS_SCENARIO_DSP_PLAYBACK_SMARTPA:
        task_scene = TASK_SCENE_AUDPLAYBACK;
        break;
    case AURISYS_SCENARIO_DSP_RECORD:
        task_scene = TASK_SCENE_RECORD;
        break;
    case AURISYS_SCENARIO_DSP_PHONE_CALL:
        task_scene = TASK_SCENE_PHONE_CALL;
        break;
    case AURISYS_SCENARIO_DSP_VOIP:
        task_scene = TASK_SCENE_VOIP;
        break;
    case AURISYS_SCENARIO_DSP_CALL_FINAL:
        task_scene = TASK_SCENE_CALL_FINAL;
        break;
    case AURISYS_SCENARIO_DSP_KTV:
        task_scene = TASK_SCENE_KTV;
        break;
    case AURISYS_SCENARIO_DSP_FAST:
    case AURISYS_SCENARIO_DSP_RECORD_FAST:
    default:
        AUD_LOG_E("%s(), aurisys_scenario %u not support!",
                  __FUNCTION__, aurisys_scenario);
        AUD_ASSERT(0);
        break;
    }

    return task_scene;
}


static uint32_t get_enum_by_string(
    const string_to_enum_pair_t table[],
    uint32_t num_pairs,
    const char *string) {
    uint32_t enum_value = 0xFFFFFFFF;
    uint32_t i = 0;

    for (i = 0; i < num_pairs; i++) { /* TODO: use hash? */
        if (strcmp(string, table[i].string) == 0) {
            enum_value = table[i].enum_value;
            AUD_LOG_VV("%s: enum_value = %u", string, enum_value);
            break;
        }
    }
    if (i == num_pairs) {
        AUD_LOG_E("string %s not found!!", string);
        AUD_ASSERT(enum_value != 0xFFFFFFFF);
    }

    return enum_value;
}

const char *get_string_by_enum(
    const string_to_enum_pair_t table[],
    uint32_t num_pairs,
    uint32_t enum_value) {

    const char *str = NULL;
    uint32_t i = 0;

    for (i = 0; i < num_pairs; i++) { /* TODO: use hash? */
        if (enum_value == table[i].enum_value) {
            str = table[i].string;
            AUD_LOG_VV("%s: enum_value = %u", str, enum_value);
            break;
        }
    }
    if (i == num_pairs) {
        AUD_LOG_E("enum %u string not found!!", enum_value);
        AUD_ASSERT(str != NULL);
    }

    return str;
}


uint32_t get_enum_by_string_aurisys_scenario(const char *string) {
    return get_enum_by_string(g_s2e_aurisys_scenario_dsp,
                              ARRAY_SIZE(g_s2e_aurisys_scenario_dsp),
                              string);
}


const char *get_string_by_enum_aurisys_scenario(uint32_t enum_value) {
    return get_string_by_enum(g_s2e_aurisys_scenario_dsp,
                              ARRAY_SIZE(g_s2e_aurisys_scenario_dsp),
                              enum_value);
}


uint32_t get_enum_by_string_audio_format(const char *string) {
    return get_enum_by_string(g_s2e_audio_format,
                              ARRAY_SIZE(g_s2e_audio_format),
                              string);
}


uint32_t get_enum_by_string_data_buf_type(const char *string) {
    return get_enum_by_string(g_s2e_data_buf_type,
                              ARRAY_SIZE(g_s2e_data_buf_type),
                              string);
}


void dump_task_config(const struct arsi_task_config_t *p_arsi_task_config) {
    /* input device */
    AUD_LOG_D("input dev: 0x%x, fmt = 0x%x, fs: %u, max fs: %u, ch: %d, max ch: %d, ch maks: 0x%x, hw_info_mask: 0x%x"
              "; output dev: 0x%x, fmt = 0x%x, fs: %u, max fs: %u, ch: %d, max ch: %d, ch maks: 0x%x, hw_info_mask: 0x%x",
              p_arsi_task_config->input_device_info.devices,
              p_arsi_task_config->input_device_info.audio_format,
              p_arsi_task_config->input_device_info.sample_rate,
              p_arsi_task_config->max_input_device_sample_rate,
              p_arsi_task_config->input_device_info.num_channels,
              p_arsi_task_config->max_input_device_num_channels,
              p_arsi_task_config->input_device_info.channel_mask,
              p_arsi_task_config->input_device_info.hw_info_mask,
              p_arsi_task_config->output_device_info.devices,
              p_arsi_task_config->output_device_info.audio_format,
              p_arsi_task_config->output_device_info.sample_rate,
              p_arsi_task_config->max_output_device_sample_rate,
              p_arsi_task_config->output_device_info.num_channels,
              p_arsi_task_config->max_output_device_num_channels,
              p_arsi_task_config->output_device_info.channel_mask,
              p_arsi_task_config->output_device_info.hw_info_mask);

    AUD_LOG_D("task_scene: %d, audio_mode: %d, stream_type: %d output_flags: 0x%x, input_source: %u, input_flags: 0x%x"
              "; enhancement_feature_mask: 0x%x",
              p_arsi_task_config->task_scene,
              p_arsi_task_config->audio_mode,
              p_arsi_task_config->stream_type,
              p_arsi_task_config->output_flags,
              p_arsi_task_config->input_source,
              p_arsi_task_config->input_flags,
              p_arsi_task_config->enhancement_feature_mask);
}


void dump_lib_config(const struct arsi_lib_config_t *p_arsi_lib_config) {
    int i = 0;

    /* lib */
    AUD_LOG_D("lib, working fs: %u, fmt: 0x%x, frame = %d, b_interleave = %d, num_ul_ref_buf_array = %d, num_dl_ref_buf_array = %d",
              p_arsi_lib_config->sample_rate,
              p_arsi_lib_config->audio_format,
              p_arsi_lib_config->frame_size_ms,
              p_arsi_lib_config->b_interleave,
              p_arsi_lib_config->num_ul_ref_buf_array,
              p_arsi_lib_config->num_dl_ref_buf_array);

    /* buffer */
    if (p_arsi_lib_config->p_ul_buf_in && p_arsi_lib_config->p_ul_buf_out) {
        AUD_LOG_D("ul in[type:%d], ch: %d, ch_mask: 0x%x, buf fs: %u, buf content fs: %u, fmt: 0x%x"
                  "; ul out[type:%d], ch: %d, ch_mask: 0x%x, buf fs: %u, buf content fs: %u, fmt: 0x%x",
                  p_arsi_lib_config->p_ul_buf_in->data_buf_type,
                  p_arsi_lib_config->p_ul_buf_in->num_channels,
                  p_arsi_lib_config->p_ul_buf_in->channel_mask,
                  p_arsi_lib_config->p_ul_buf_in->sample_rate_buffer,
                  p_arsi_lib_config->p_ul_buf_in->sample_rate_content,
                  p_arsi_lib_config->p_ul_buf_in->audio_format,
                  p_arsi_lib_config->p_ul_buf_out->data_buf_type,
                  p_arsi_lib_config->p_ul_buf_out->num_channels,
                  p_arsi_lib_config->p_ul_buf_out->channel_mask,
                  p_arsi_lib_config->p_ul_buf_out->sample_rate_buffer,
                  p_arsi_lib_config->p_ul_buf_out->sample_rate_content,
                  p_arsi_lib_config->p_ul_buf_out->audio_format);
    }

    if (p_arsi_lib_config->p_ul_ref_bufs) {
        for (i = 0; i < p_arsi_lib_config->num_ul_ref_buf_array; i++) {
            AUD_LOG_D("ul ref[type:%d], ch: %d, ch_mask: 0x%x, buf fs: %u, buf content fs: %u, fmt: 0x%x",
                      p_arsi_lib_config->p_ul_ref_bufs[i].data_buf_type,
                      p_arsi_lib_config->p_ul_ref_bufs[i].num_channels,
                      p_arsi_lib_config->p_ul_ref_bufs[i].channel_mask,
                      p_arsi_lib_config->p_ul_ref_bufs[i].sample_rate_buffer,
                      p_arsi_lib_config->p_ul_ref_bufs[i].sample_rate_content,
                      p_arsi_lib_config->p_ul_ref_bufs[i].audio_format);
        }
    }


    if (p_arsi_lib_config->p_dl_buf_in && p_arsi_lib_config->p_dl_buf_out) {
        AUD_LOG_D("dl in[type:%d], ch: %d, ch_mask: 0x%x, buf fs: %u, buf content fs: %u, fmt: 0x%x"
                  "; dl out[type:%d], ch: %d, ch_mask: 0x%x, buf fs: %u, buf content fs: %u, fmt: 0x%x",
                  p_arsi_lib_config->p_dl_buf_in->data_buf_type,
                  p_arsi_lib_config->p_dl_buf_in->num_channels,
                  p_arsi_lib_config->p_dl_buf_in->channel_mask,
                  p_arsi_lib_config->p_dl_buf_in->sample_rate_buffer,
                  p_arsi_lib_config->p_dl_buf_in->sample_rate_content,
                  p_arsi_lib_config->p_dl_buf_in->audio_format,
                  p_arsi_lib_config->p_dl_buf_out->data_buf_type,
                  p_arsi_lib_config->p_dl_buf_out->num_channels,
                  p_arsi_lib_config->p_dl_buf_out->channel_mask,
                  p_arsi_lib_config->p_dl_buf_out->sample_rate_buffer,
                  p_arsi_lib_config->p_dl_buf_out->sample_rate_content,
                  p_arsi_lib_config->p_dl_buf_out->audio_format);
    }

    if (p_arsi_lib_config->p_dl_ref_bufs) {
        for (i = 0; i < p_arsi_lib_config->num_dl_ref_buf_array; i++) {
            AUD_LOG_D("dl ref[type:%d], ch: %d, ch_mask: 0x%x, buf fs: %u, buf content fs: %u, fmt: 0x%x",
                      p_arsi_lib_config->p_dl_ref_bufs[i].data_buf_type,
                      p_arsi_lib_config->p_dl_ref_bufs[i].num_channels,
                      p_arsi_lib_config->p_dl_ref_bufs[i].channel_mask,
                      p_arsi_lib_config->p_dl_ref_bufs[i].sample_rate_buffer,
                      p_arsi_lib_config->p_dl_ref_bufs[i].sample_rate_content,
                      p_arsi_lib_config->p_dl_ref_bufs[i].audio_format);
        }
    }
}


static void match_support_format_mask(uint32_t *p_mask, const char *format, const uint32_t string_size) {
    AUD_LOG_V("%s(), format = %s", __FUNCTION__, format);

    if (strncmp(format, "AUDIO_FORMAT_PCM_16_BIT", string_size) == 0) {
        *p_mask |= AUDIO_SUPPORT_FORMAT_PCM_16_BIT_MASK;
    } else if (strncmp(format, "AUDIO_FORMAT_PCM_32_BIT", string_size) == 0) {
        *p_mask |= AUDIO_SUPPORT_FORMAT_PCM_32_BIT_MASK;
    } else if (strncmp(format, "AUDIO_FORMAT_PCM_8_24_BIT", string_size) == 0) {
        *p_mask |= AUDIO_SUPPORT_FORMAT_PCM_8_24_BIT_MASK;
    }
}


static void match_support_frame_ms_mask(uint32_t *p_mask, const char *frame_ms, const uint32_t string_size __unused) {
    switch (atoi(frame_ms)) {
    case 0:
        *p_mask |= AUDIO_SUPPORT_FRAME_00_MS_MASK;
        break;
    case 1:
        *p_mask |= AUDIO_SUPPORT_FRAME_01_MS_MASK;
        break;
    case 3:
        *p_mask |= AUDIO_SUPPORT_FRAME_03_MS_MASK;
        break;
    case 5:
        *p_mask |= AUDIO_SUPPORT_FRAME_05_MS_MASK;
        break;
    case 10:
        *p_mask |= AUDIO_SUPPORT_FRAME_10_MS_MASK;
        break;
    case 20:
        *p_mask |= AUDIO_SUPPORT_FRAME_20_MS_MASK;
        break;
    default:
        AUD_LOG_W("%s(), %s not support!!", __FUNCTION__, frame_ms);
    }
}


static void match_support_channel_number_mask(uint32_t *p_mask, const char *channel_number, const uint32_t string_size __unused) {
    switch (atoi(channel_number)) {
    case 1:
        *p_mask |= AUDIO_SUPPORT_CHANNEL_NUMBER_1_MASK;
        break;
    case 2:
        *p_mask |= AUDIO_SUPPORT_CHANNEL_NUMBER_2_MASK;
        break;
    case 3:
        *p_mask |= AUDIO_SUPPORT_CHANNEL_NUMBER_3_MASK;
        break;
    case 4:
        *p_mask |= AUDIO_SUPPORT_CHANNEL_NUMBER_4_MASK;
        break;
    default:
        AUD_LOG_W("%s(), %s not support!!", __FUNCTION__, channel_number);
    }
}


uint32_t parse_segment_in_string(const char *string, match_mask_by_string_fp_t match_func) {
    uint32_t mask = 0;

    char *copy_string = NULL;
    uint32_t copy_size = 0;

    char *current = NULL;
    char *end = NULL;
    char *comma = NULL;

    if (string == NULL) {
        AUD_LOG_W("%s(), string == NULL!! return 0", __FUNCTION__);
        return 0;
    }
    if (strlen(string) == 0) {
        AUD_LOG_W("%s(), strlen(string) == 0!! return 0", __FUNCTION__);
        return 0;
    }


    AUD_LOG_V("%s", string);


    copy_size = strlen(string) + 1;
    AUDIO_ALLOC_CHAR_BUFFER(copy_string, copy_size + 1);
    strncpy(copy_string, string, copy_size);

    current = copy_string;
    end = copy_string + copy_size - 1;
    comma = strstr(current, ",");
    while (comma != NULL) {
        *comma = '\0';
        match_func(&mask, current, end - current);

        current = comma + 1;
        if (current >= end) {
            break;
        }

        comma = strstr(current, ",");
    }
    if (current < end) {
        match_func(&mask, current, end - current);
    }

    AUDIO_FREE(copy_string);
    return mask;
}


uint32_t get_support_format_mask(const char *string) {
    return parse_segment_in_string(string, match_support_format_mask);
}


uint32_t get_format_from_mask(const uint32_t mask) {
    uint32_t audio_format = 0;

    /* higher bit first */
    if (mask & AUDIO_SUPPORT_FORMAT_PCM_32_BIT_MASK) {
        audio_format = AUDIO_FORMAT_PCM_32_BIT;
    } else if (mask & AUDIO_SUPPORT_FORMAT_PCM_8_24_BIT_MASK) {
        audio_format = AUDIO_FORMAT_PCM_8_24_BIT;
    } else if (mask & AUDIO_SUPPORT_FORMAT_PCM_16_BIT_MASK) {
        audio_format = AUDIO_FORMAT_PCM_16_BIT;
    } else {
        audio_format = AUDIO_FORMAT_PCM_16_BIT;
        AUD_LOG_W("%s(), mask 0x%x not support!! use 16 bit", __FUNCTION__, mask);
    }

    return audio_format;
}


uint32_t get_dedicated_format_from_mask(const uint32_t masks, const uint32_t the_audio_format) {
    uint32_t audio_format = 0;
    uint32_t the_mask = 0;

    if (the_audio_format != AUDIO_FORMAT_PCM_16_BIT &&
        the_audio_format != AUDIO_FORMAT_PCM_32_BIT &&
        the_audio_format != AUDIO_FORMAT_PCM_8_24_BIT) {
        audio_format = get_format_from_mask(masks);
        AUD_LOG_W("%s(), the_audio_format %u not support!! use %u",
                  __FUNCTION__, the_audio_format, audio_format);
        return audio_format;
    }

    /* audio_format to mask */
    the_mask = AUDIO_FORMAT_TO_MASK(the_audio_format);

    /* get dedicated format */
    if (the_mask & masks) {
        audio_format = get_format_from_mask(the_mask);
    } else {
        audio_format = get_format_from_mask(masks);
        AUD_LOG_V("%s(), masks 0x%x and the_audio_format %u not match!! use %u",
                  __FUNCTION__, masks, the_audio_format, audio_format);
    }

    return audio_format;
}


uint32_t get_support_frame_ms_mask(const char *string) {
    return parse_segment_in_string(string, match_support_frame_ms_mask);
}


uint8_t get_frame_ms_from_mask(const uint32_t mask) {
    uint8_t frame_ms = 0;

    /* lower frame ms first */
    if (mask & AUDIO_SUPPORT_FRAME_00_MS_MASK) {
        frame_ms = 0;
    } else if (mask & AUDIO_SUPPORT_FRAME_01_MS_MASK) {
        frame_ms = 1;
    } else if (mask & AUDIO_SUPPORT_FRAME_05_MS_MASK) {
        frame_ms = 5;
    } else if (mask & AUDIO_SUPPORT_FRAME_10_MS_MASK) {
        frame_ms = 10;
    } else if (mask & AUDIO_SUPPORT_FRAME_20_MS_MASK) {
        frame_ms = 20;
    } else {
        frame_ms = 20;
        AUD_LOG_W("%s(), mask 0x%x not support!! use 20 ms", __FUNCTION__, mask);
    }

    return frame_ms;
}


uint8_t get_dedicated_frame_ms_from_mask(const uint32_t masks, const uint8_t the_frame_ms) {
    uint32_t frame_ms = 0;
    uint32_t the_mask = 0;

    if (the_frame_ms > 20) {
        frame_ms = get_frame_ms_from_mask(masks);
        AUD_LOG_W("%s(), the_frame_ms %d not support!! use %d",
                  __FUNCTION__, the_frame_ms, frame_ms);
        return frame_ms;
    }

    /* frame_ms to mask */
    the_mask = AUDIO_FRAME_MS_TO_MASK(the_frame_ms);

    /* get dedicated frame_ms */
    if (the_mask & masks) {
        frame_ms = get_frame_ms_from_mask(the_mask);
    } else {
        frame_ms = get_frame_ms_from_mask(masks);
        AUD_LOG_V("%s(), masks 0x%x and the_frame_ms %d not match!! use %d",
                  __FUNCTION__, masks, the_frame_ms, frame_ms);
    }

    return frame_ms;
}



uint32_t get_support_channel_number_mask(const char *string) {
    return parse_segment_in_string(string, match_support_channel_number_mask);
}


uint8_t get_channel_number_from_mask(const uint32_t mask) {
    uint8_t channel_number = 0;

    /* higher channel number first */
    if (mask & AUDIO_SUPPORT_CHANNEL_NUMBER_4_MASK) {
        channel_number = 4;
    } else if (mask & AUDIO_SUPPORT_CHANNEL_NUMBER_3_MASK) {
        channel_number = 3;
    } else if (mask & AUDIO_SUPPORT_CHANNEL_NUMBER_2_MASK) {
        channel_number = 2;
    } else if (mask & AUDIO_SUPPORT_CHANNEL_NUMBER_1_MASK) {
        channel_number = 1;
    } else {
        channel_number = 2;
        AUD_LOG_W("%s(), mask 0x%x not support!! use 2 ch", __FUNCTION__, mask);
    }

    return channel_number;
}


uint8_t get_dedicated_channel_number_from_mask(const uint32_t masks, const uint8_t the_channel_number) {
    uint32_t channel_number = 0;
    uint32_t the_mask = 0;

    if (the_channel_number > 4 || the_channel_number == 0) {
        channel_number = get_channel_number_from_mask(masks);
        AUD_LOG_W("%s(), the_channel_number %d not support!! use %d",
                  __FUNCTION__, the_channel_number, channel_number);
        return channel_number;
    }

    /* channel number to mask */
    the_mask = AUDIO_CHANNEL_NUMBER_TO_MASK(the_channel_number);

    /* get dedicated channel number */
    if (the_mask & masks) {
        channel_number = get_channel_number_from_mask(the_mask);
    } else {
        channel_number = get_channel_number_from_mask(masks);
        AUD_LOG_V("%s(), masks 0x%x and the_channel_number %d not match!! use %d",
                  __FUNCTION__, masks, the_channel_number, channel_number);
    }

    return channel_number;
}


uint8_t get_input_ch_mask(const uint8_t num_channels) {
    uint8_t mask = 0;

    if (num_channels == 3) {
        mask = AUDIO_CHANNEL_IN_3MIC;
    } else if (num_channels == 2) {
        mask = AUDIO_CHANNEL_IN_STEREO;
    } else if (num_channels == 1) {
        mask = AUDIO_CHANNEL_IN_MONO;
    } else {
        AUD_LOG_W("Not support num_channels %d, use 2 instead", num_channels);
        mask = AUDIO_CHANNEL_IN_STEREO;
    }

    return mask;
}


uint8_t get_output_ch_mask(const uint8_t num_channels) {
    uint8_t mask = 0;

    if (num_channels == 2) {
        mask = AUDIO_CHANNEL_OUT_STEREO;
    } else if (num_channels == 1) {
        mask = AUDIO_CHANNEL_OUT_MONO;
    } else {
        AUD_LOG_W("Not support num_channels %d, use 2 instead", num_channels);
        mask = AUDIO_CHANNEL_OUT_STEREO;
    }

    return mask;
}



#ifdef __cplusplus
}  /* extern "C" */
#endif

