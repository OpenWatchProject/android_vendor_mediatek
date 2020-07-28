#ifndef MTK_AURISYS_UTILITY_H
#define MTK_AURISYS_UTILITY_H

#include <stdint.h>



#ifdef __cplusplus
extern "C" {
#endif


#ifdef ARRAY_SIZE
#undef ARRAY_SIZE
#endif
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


/*
 * =============================================================================
 *                     ref struct
 * =============================================================================
 */

struct arsi_task_config_t;
struct arsi_lib_config_t;


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define STRING_TO_ENUM(string, enum_value) { string, enum_value }


/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

typedef struct string_to_enum_pair_t {
    char *string;
    uint32_t enum_value;
} string_to_enum_pair_t;



/*
 * =============================================================================
 *                     struct definition
 * =============================================================================
 */


/*
 * =============================================================================
 *                     hook function
 * =============================================================================
 */


/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

uint8_t map_aurisys_scenario_to_task_scene(
    const uint8_t core_type,
    const uint32_t aurisys_scenario);

uint32_t get_enum_by_string_aurisys_scenario(
    const uint8_t core_type,
    const char *string);

const char *get_string_by_enum_aurisys_scenario(
    const uint8_t core_type,
    uint32_t enum_value);

uint32_t get_enum_by_string_audio_format(const char *string);
uint32_t get_enum_by_string_data_buf_type(const char *string);

const char *get_string_by_enum(
    const string_to_enum_pair_t table[],
    uint32_t num_pairs,
    uint32_t enum_value);

void dump_task_config(const struct arsi_task_config_t *p_arsi_task_config);
void dump_lib_config(const struct arsi_lib_config_t *p_arsi_lib_config);


/**
 * audio format
 *     mask:  audio_support_format_mask_t
 *     value: audio_format_t
 */
uint32_t get_support_format_mask(const char *string);
uint32_t get_format_from_mask(const uint32_t mask);
uint32_t get_dedicated_format_from_mask(const uint32_t masks, const uint32_t the_audio_format);


/**
 * frame ms
 *     mask:  audio_support_frame_ms_mask_t
 *     value: uint8_t
 */
uint32_t get_support_frame_ms_mask(const char *string);
uint8_t get_frame_ms_from_mask(const uint32_t mask);
uint8_t get_dedicated_frame_ms_from_mask(const uint32_t masks, const uint8_t the_frame_ms);


/**
 * channel number
 *     mask:  audio_support_channel_number_mask_t
 *     value: uint8_t
 */
uint32_t get_support_channel_number_mask(const char *string);
uint8_t get_channel_number_from_mask(const uint32_t mask);
uint8_t get_dedicated_channel_number_from_mask(const uint32_t masks, const uint8_t the_channel_number);



#ifndef AUDIO_CHANNEL_IN_3MIC
#define AUDIO_CHANNEL_IN_3MIC (AUDIO_CHANNEL_IN_LEFT | AUDIO_CHANNEL_IN_RIGHT | AUDIO_CHANNEL_IN_BACK)
#endif
#ifndef AUDIO_CHANNEL_IN_4MIC
#define AUDIO_CHANNEL_IN_4MIC (AUDIO_CHANNEL_IN_LEFT | AUDIO_CHANNEL_IN_RIGHT | AUDIO_CHANNEL_IN_BACK) // ch4 == ch3
#endif


uint8_t get_input_ch_mask(const uint8_t num_channels);
uint8_t get_output_ch_mask(const uint8_t num_channels);





#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of MTK_AURISYS_UTILITY_H */

