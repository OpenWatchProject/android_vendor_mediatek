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

#include <main.h>
#include "FreeRTOS.h"
#include "task.h"
#include <dvfs_common.h>    //log
#include <feature_manager.h>
#include <dvfs_swip_table.h>
#include <dvfs_config_parser.h>
#include <wrapped_audio.h>

#include <aurisys_lib_handler.h>
#include <arsi_type.h>
#include <audio_task.h>
#include "driver_api.h"

#ifdef CFG_A2DP_OFFLOAD_SUPPORT
#define __MCPS_PARSER_SBC_ENCODER_SUPPORT__
#endif
#define __MCPS_PARSER_BESLOUDNESS_SUPPORT__
#define __MCPS_PARSER_DCREMOVAL_SUPPORT__
#define __MCPS_PARSER_BLISRC_SUPPORT__
#define __MCPS_PARSER_SHIFTER_SUPPORT__
#define __MCPS_PARSER_LIMITER_SUPPORT__
#define __MCPS_PARSER_CALL_SUPPORT__  //Only define either of PARSER_DEMO and __MCPS_PARSER_CALL_SUPPORT__

#define DEBUG_ID_MCPS_PARSER_INTERVAL                           1000
#define DEBUG_ID_MCPS_PARSER_START_INDEX_BASE                   0
#define DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_BASE             (DEBUG_ID_MCPS_PARSER_START_INDEX_BASE + DEBUG_ID_MCPS_PARSER_INTERVAL)         /* 1000 */
#define DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_INPUT_CONFIG     (DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_BASE + 1)
#define DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_DL_OUT           (DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_BASE + 2)
#define DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_INIT_PARAM       (DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_BASE + 3)
#define DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_BASE             (DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_BASE + DEBUG_ID_MCPS_PARSER_INTERVAL)   /* 2000 */
#define DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_INPUT_CONFIG     (DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_BASE + 1)
#define DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_DL_OUT           (DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_BASE + 2)
#define DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_LIB_HANDLE       (DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_BASE + 3)
#define DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_INIT_PARAM       (DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_BASE + 4)
#define DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_MODE_PARAM       (DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_BASE + 5)
#define DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_CUSTOM_PARAM     (DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_BASE + 6)
#define DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_FILTER_COEF_L    (DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_BASE + 7)
#define DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_FILTER_COEF_R    (DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_BASE + 8)
#define DEBUG_ID_MCPS_PARSER_DCREMOVAL_INDEX_BASE               (DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_BASE + DEBUG_ID_MCPS_PARSER_INTERVAL)   /* 3000 */
#define DEBUG_ID_MCPS_PARSER_DCREMOVAL_INDEX_INPUT_CONFIG       (DEBUG_ID_MCPS_PARSER_DCREMOVAL_INDEX_BASE + 1)
#define DEBUG_ID_MCPS_PARSER_DCREMOVAL_INDEX_DL_OUT             (DEBUG_ID_MCPS_PARSER_DCREMOVAL_INDEX_BASE + 2)
#define DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_BASE                  (DEBUG_ID_MCPS_PARSER_DCREMOVAL_INDEX_BASE + DEBUG_ID_MCPS_PARSER_INTERVAL)     /* 4000 */
#define DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_INPUT_CONFIG          (DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_BASE + 1)
#define DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_DL_IN                 (DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_BASE + 2)
#define DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_DL_OUT                (DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_BASE + 3)
#define DEBUG_ID_MCPS_PARSER_SHIFTER_INDEX_BASE                 (DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_BASE + DEBUG_ID_MCPS_PARSER_INTERVAL)        /* 5000 */
#define DEBUG_ID_MCPS_PARSER_SHIFTER_INDEX_INPUT_CONFIG         (DEBUG_ID_MCPS_PARSER_SHIFTER_INDEX_BASE + 1)
#define DEBUG_ID_MCPS_PARSER_SHIFTER_INDEX_DL_OUT               (DEBUG_ID_MCPS_PARSER_SHIFTER_INDEX_BASE + 2)
#define DEBUG_ID_MCPS_PARSER_LIMITER_INDEX_BASE                 (DEBUG_ID_MCPS_PARSER_SHIFTER_INDEX_BASE + DEBUG_ID_MCPS_PARSER_INTERVAL)       /* 6000 */
#define DEBUG_ID_MCPS_PARSER_LIMITER_INDEX_INPUT_CONFIG         (DEBUG_ID_MCPS_PARSER_LIMITER_INDEX_BASE + 1)
#define DEBUG_ID_MCPS_PARSER_LIMITER_INDEX_DL_OUT               (DEBUG_ID_MCPS_PARSER_LIMITER_INDEX_BASE + 2)

#if defined(__MCPS_PARSER_SBC_ENCODER_SUPPORT__)
#include "sbc_encoder.h"

#define SBC_ENCODER_SAMPLING_RATE_NUMBER    4   /* Sampling rate : 48kHz, 44.1kHz, 32kHz, 16kHz */
#define SBC_ENCODER_BLOCK_NUMBER            4   /* Block : 16, 12, 8, 4 */

const uint16_t sbc_encoder_mcps_table[SBC_ENCODER_BLOCK_NUMBER][SBC_ENCODER_SAMPLING_RATE_NUMBER] = {
    {18, 16, 12,  6},   /* Block 16 : 48kHz, 44.1kHz, 32kHz, 16kHz */
    {20, 17, 13,  7},   /* Block 12 : 48kHz, 44.1kHz, 32kHz, 16kHz */
    {20, 17, 13,  7},   /* Block  8 : 48kHz, 44.1kHz, 32kHz, 16kHz */
    {23, 20, 15,  8}    /* Block  4 : 48kHz, 44.1kHz, 32kHz, 16kHz */
};
#endif  /* __MCPS_PARSER_SBC_ENCODER_SUPPORT__ */

#if defined(__MCPS_PARSER_BESLOUDNESS_SUPPORT__)
#include "arsi_besloudness.h"

/* s2m2s ratio = ceiling(s2m2s_mcps / mono_mcps * 65536) */
#define S2M2S_RATIO 69658
/* s2m2s_mcps = round(mono_mcps * S2M2S_RATIO / 65536) */
#define MCPS_CONVERSION_FROM_MONO_TO_S2M2S(mono_mcps)   (((mono_mcps) * S2M2S_RATIO + 32768) / 65536)
#define MAX_SAMPLING_RATE   192000
#define MCPS_OFFSET 1
/* target_mcps = round(max_mcps * target_sr / max_sr) + offset */
#define MCPS_CONVERSION_FROM_MAX_SAMPLING_RATE_TO_TARGET(max_mcps, sr_val)  ((((max_mcps) * (sr_val) + (MAX_SAMPLING_RATE >> 1)) / MAX_SAMPLING_RATE) + MCPS_OFFSET)

#define DRC_BAND_NUMBER             8   /* Band : 8, 7, 6, 5, 4, 3, 2, 1 */
#define DRC_CHANNEL_SETTING_NUMBER  2   /* Channel setting : (stereo), (s2m2s or mono) */
#define DRC_BAND_MAX_NUMBER         DRC_BAND_NUMBER
#define DRC_BAND_MIN_NUMBER         1

const uint16_t drc_mcps_table[DRC_CHANNEL_SETTING_NUMBER][DRC_BAND_NUMBER] = {
    {297, 252, 211, 171, 132, 100,  69,  46},   /* stereo        : band 8 ~ 1 */
    {159, 137, 118,  98,  76,  60,  46,  33}    /* s2m2s or mono : band 8 ~ 1 */
};

#define ACF_BIQUAD_NUMBER           11  /* Band : 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 */
#define ACF_CHANNEL_SETTING_NUMBER  2   /* Channel setting : (stereo with L / R seperation or stereo), (s2m2s or mono) */
#define ACF_BIQUAD_MAX_NUMBER       ACF_BIQUAD_NUMBER
#define ACF_BIQUAD_MIN_NUMBER       1

const uint16_t acf_mcps_table[ACF_CHANNEL_SETTING_NUMBER][ACF_BIQUAD_NUMBER] = {
    { 94,  90,  85,  80,  75,  70,  65,  60,  56,  51,  46},    /* stereo with L / R seperation or stereo : biquad 11 ~ 1 */
    { 73,  67,  63,  59,  56,  52,  49,  45,  41,  38,  34}     /* s2m2s or mono                          : biquad 11 ~ 1 */
};
#endif  /* __MCPS_PARSER_BESLOUDNESS_SUPPORT__ */

#if defined(__MCPS_PARSER_DCREMOVAL_SUPPORT__)
#define DCREMOVAL_SAMPLING_RATE_NUMBER  15  /* Sampling rate : 192kHz, 176.4kHz, 128kHz, 96kHz, 88.2kHz, 64kHz, 48kHz, 44.1kHz, 32kHz, 24kHz, 22.05kHz, 16kHz, 12kHz, 11.025kHz, 8kHz */
#define DCREMOVAL_CHANNEL_NUMBER        2   /* Channel number : 2 (stereo), 1 (mono) */
#define DCREMOVAL_AUDIO_FORMAT_NUMBER   2   /* Audio format : Q1.31, Q1.15 */

const uint16_t dcremoval_mcps_table[DCREMOVAL_AUDIO_FORMAT_NUMBER][DCREMOVAL_CHANNEL_NUMBER][DCREMOVAL_SAMPLING_RATE_NUMBER] = {
    {{33, 31, 22, 17, 16, 11,  4,  4,  3,  2,  2,  2,  1,  1,  1},  /* Q1.31 Stereo : 192kHz ~ 8kHz */
     {16, 15, 11,  9,  8,  6,  2,  2,  2,  1,  1,  1,  1,  1,  1}}, /* Q1.31 Mono   : 192kHz ~ 8kHz */
    {{35, 32, 23, 18, 16, 12,  2,  2,  2,  1,  1,  1,  1,  1,  1},  /* Q1.15 Stereo : 192kHz ~ 8kHz */
     {17, 16, 12,  9,  8,  6,  2,  1,  1,  1,  1,  1,  1,  1,  1}}  /* Q1.15 Mono   : 192kHz ~ 8kHz */
};
#endif  /* __MCPS_PARSER_DCREMOVAL_SUPPORT__ */

#if defined(__MCPS_PARSER_BLISRC_SUPPORT__)
#define BLISRC_INPUT_SAMPLING_RATE_NUMBER   12  /* Sampling rate : 96kHz, 88.2kHz, 64kHz, 48kHz, 44.1kHz, 32kHz, 24kHz, 22.05kHz, 16kHz, 12kHz, 11.025kHz, 8kHz */
#define BLISRC_OUTPUT_SAMPLING_RATE_NUMBER  2   /* Sampling rate : 192kHz, 48kHz */
#define BLISRC_CHANNEL_COMBINATION_NUMBER   6   /* Channel number : 4, 3, s2s, s2m, m2s, m2m */
#define BLISRC_AUDIO_FORMAT_NUMBER          2   /* Audio format : 32-bit (Q1.31 / Q9.23), 16-bit (Q1.15) */

const uint16_t blisrc_mcps_table[BLISRC_AUDIO_FORMAT_NUMBER][BLISRC_CHANNEL_COMBINATION_NUMBER][BLISRC_OUTPUT_SAMPLING_RATE_NUMBER][BLISRC_INPUT_SAMPLING_RATE_NUMBER] = {
    {{{ 63,  92,  65,  64, 168,  62,  62, 167,  63,  67, 634,  60},     /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz 4 channels to output 192kHz 4 channels */
      { 23,  31,  18,   2,  24,  16,  17,  33,  17,  17,  45,  16}},    /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz 4 channels to output  48kHz 4 channels */
     {{ 77, 112,  80,  78, 205,  76,  76, 204,  77,  82, 774,  73},     /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz 3 channels to output 192kHz 3 channels */
      { 28,  38,  22,   2,  30,  19,  21,  40,  20,  21,  55,  20}},    /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz 3 channels to output  48kHz 3 channels */
     {{ 24,  35,  25,  24,  63,  23,  24,  62,  24,  25, 236,  23},     /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz stereo     to output 192kHz stereo     */
      {  9,  12,   7,   1,   9,   6,   7,  13,   6,   7,  17,   6}},    /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz stereo     to output  48kHz stereo     */
     {{ 23,  33,  23,  23,  59,  22,  22,  59,  22,  24, 222,  21},     /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz stereo     to output 192kHz mono       */
      {  8,  11,   7,   1,   9,   6,   6,  12,   6,   6,  16,   6}},    /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz stereo     to output  48kHz mono       */
     {{ 23,  34,  24,  23,  61,  23,  23,  61,  23,  25, 230,  22},     /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz mono       to output 192kHz stereo     */
      {  9,  12,   7,   1,   9,   6,   7,  12,   6,   7,  17,   6}},    /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz mono       to output  48kHz stereo     */
     {{ 22,  32,  23,  22,  57,  21,  22,  57,  22,  23, 216,  21},     /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz mono       to output 192kHz mono       */
      {  8,  11,   7,   1,   9,   6,   6,  12,   6,   6,  16,   6}}},   /* 32-bit (Q1.31 / Q9.23), input 96kHz ~ 8kHz mono       to output  48kHz mono       */
    {{{ 58,  88,  62,  61, 161,  59,  60, 160,  60,  64, 510,  57},     /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz 4 channels to output 192kHz 4 channels */
      { 23,  32,  19,   1,  25,  16,  18,  34,  17,  18,  47,  16}},    /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz 4 channels to output  48kHz 4 channels */
     {{ 72, 109,  77,  76, 199,  74,  74, 199,  75,  79, 632,  71},     /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz 3 channels to output 192kHz 3 channels */
      { 29,  40,  24,   1,  31,  20,  22,  42,  21,  22,  58,  19}},    /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz 3 channels to output  48kHz 3 channels */
     {{ 26,  40,  28,  28,  72,  27,  27,  72,  27,  29, 228,  26},     /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz stereo     to output 192kHz stereo     */
      { 11,  15,   9,   1,  11,   8,   8,  16,   8,   8,  21,   7}},    /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz stereo     to output  48kHz stereo     */
     {{ 23,  34,  24,  24,  62,  23,  23,  61,  23,  25, 195,  22},     /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz stereo     to output 192kHz mono       */
      {  9,  13,   8,   1,  10,   7,   7,  13,   7,   7,  18,   6}},    /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz stereo     to output  48kHz mono       */
     {{ 22,  33,  24,  23,  60,  22,  23,  60,  23,  24, 190,  22},     /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz mono       to output 192kHz stereo     */
      {  9,  12,   7,   1,  10,   6,   7,  13,   7,   7,  18,   6}},    /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz mono       to output  48kHz stereo     */
     {{ 22,  33,  24,  23,  60,  22,  23,  60,  23,  24, 190,  22},     /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz mono       to output 192kHz mono       */
      {  9,  12,   7,   1,  10,   6,   7,  13,   7,   7,  18,   6}}}    /* 16-bit (Q1.15)        , input 96kHz ~ 8kHz mono       to output  48kHz mono       */
};
#endif  /* __MCPS_PARSER_BLISRC_SUPPORT__ */

#if defined(__MCPS_PARSER_LIMITER_SUPPORT__)
#define LIMITER_SAMPLING_RATE_NUMBER    15  /* Sampling rate : 192kHz, 176.4kHz, 128kHz, 96kHz, 88.2kHz, 64kHz, 48kHz, 44.1kHz, 32kHz, 24kHz, 22.05kHz, 16kHz, 12kHz, 11.025kHz, 8kHz */
#define LIMITER_CHANNEL_NUMBER          2   /* Channel number : 2 (stereo), 1 (mono) */

const uint16_t limiter_mcps_table[LIMITER_CHANNEL_NUMBER][LIMITER_SAMPLING_RATE_NUMBER] = {
    {29, 25, 18, 14, 13,  9,  7,  7,  5,  4,  4,  3,  2,  2,  2},   /* Stereo : 192kHz ~ 8kHz */
    {16, 15, 11,  8,  8,  6,  4,  4,  3,  2,  2,  2,  1,  1,  1}    /* Mono   : 192kHz ~ 8kHz */
};
#endif  /* __MCPS_PARSER_LIMITER_SUPPORT__ */

#ifdef PARSER_DEMO  //Only define either of PARSER_DEMO and __MCPS_PARSER_CALL_SUPPORT__
const uint32_t phonecall_dl_enh_mcps[DVFS_PHONECALL_NUM_TYPE] = {
     [DVFS_PHONECALL_8K_RECEIVER_2MIC]   = 120,
     [DVFS_PHONECALL_8K_SPEAKER_2MIC]    = 130,
     [DVFS_PHONECALL_8K_RECEIVER_3MIC]   = 0,
     [DVFS_PHONECALL_8K_SPEAKER_3MIC]    = 0,
     [DVFS_PHONECALL_16K_RECEIVER_2MIC]  = 180,
     [DVFS_PHONECALL_16K_SPEAKER_2MIC]   = 200,
     [DVFS_PHONECALL_16K_RECEIVER_3MIC]  = 0,
     [DVFS_PHONECALL_16K_SPEAKER_3MIC]   = 0,
     [DVFS_PHONECALL_32K_RECEIVER_2MIC]  = 195,
     [DVFS_PHONECALL_32K_SPEAKER_2MIC]   = 215,
     [DVFS_PHONECALL_32K_RECEIVER_3MIC]  = 0,
     [DVFS_PHONECALL_32K_SPEAKER_3MIC]   = 0,
     [DVFS_PHONECALL_48K_RECEIVER_2MIC]  = 0,
     [DVFS_PHONECALL_48K_SPEAKER_2MIC]   = 0,
     [DVFS_PHONECALL_48K_RECEIVER_3MIC]  = 0,
     [DVFS_PHONECALL_48K_SPEAKER_3MIC]   = 0,
};

uint32_t phone_call_enh_example(dvfs_swip_config_t *p_config)
{
    DVFS_LOG_E("%s [output]sample_rate=%x, device=%x\n",
                __FUNCTION__,
                p_config->dl_out.sample_rate,
                p_config->output_devices);
    switch (p_config->dl_out.sample_rate) {
    // support 2mic only for now
    case 8000:
        if (p_config->output_devices == AUDIO_DEVICE_OUT_EARPIECE)
            return phonecall_dl_enh_mcps[DVFS_PHONECALL_8K_RECEIVER_2MIC];
        if (p_config->output_devices == AUDIO_DEVICE_OUT_SPEAKER)
            return phonecall_dl_enh_mcps[DVFS_PHONECALL_8K_SPEAKER_2MIC];
    case 16000:
        if (p_config->output_devices == AUDIO_DEVICE_OUT_EARPIECE)
            return phonecall_dl_enh_mcps[DVFS_PHONECALL_16K_RECEIVER_2MIC];
        if (p_config->output_devices == AUDIO_DEVICE_OUT_SPEAKER)
            return phonecall_dl_enh_mcps[DVFS_PHONECALL_16K_SPEAKER_2MIC];
    case 32000:
        if (p_config->output_devices == AUDIO_DEVICE_OUT_EARPIECE)
            return phonecall_dl_enh_mcps[DVFS_PHONECALL_32K_RECEIVER_2MIC];
        if (p_config->output_devices == AUDIO_DEVICE_OUT_SPEAKER)
            return phonecall_dl_enh_mcps[DVFS_PHONECALL_32K_SPEAKER_2MIC];
    default:
        return 0;
    }
}
#endif /* PARSER_DEMO */

void mcps_parser_check_null_pointer (uint32_t index, void *pointer)
{
    if (pointer == NULL) {
        DVFS_LOG_E("[mcps_parser] Error : NULL pointer detected for index %d\n", index);
    }
}

int32_t mcps_parser_get_sampling_rate_index (adsp_swip_id id, uint32_t sampling_rate_value)
{
    int32_t sampling_rate_index;
    if (id == SWIP_ID_CODEC_SBC_ENCODER) {
        switch (sampling_rate_value) {
            case 48000: sampling_rate_index = 0; break;
            case 44100: sampling_rate_index = 1; break;
            case 32000: sampling_rate_index = 2; break;
            case 16000: sampling_rate_index = 3; break;
            default:
                sampling_rate_index = 0;
                DVFS_LOG_E("[mcps_parser](SBC Encoder) Invalid sampling rate = %d\n", sampling_rate_value);
                break;
        }
    } else {
        switch (sampling_rate_value) {
            case 192000:    sampling_rate_index =  0;    break;
            case 176400:    sampling_rate_index =  1;    break;
            case 128000:    sampling_rate_index =  2;    break;
            case  96000:    sampling_rate_index =  3;    break;
            case  88200:    sampling_rate_index =  4;    break;
            case  64000:    sampling_rate_index =  5;    break;
            case  48000:    sampling_rate_index =  6;    break;
            case  44100:    sampling_rate_index =  7;    break;
            case  32000:    sampling_rate_index =  8;    break;
            case  24000:    sampling_rate_index =  9;    break;
            case  22050:    sampling_rate_index = 10;    break;
            case  16000:    sampling_rate_index = 11;    break;
            case  12000:    sampling_rate_index = 12;    break;
            case  11025:    sampling_rate_index = 13;    break;
            case   8000:    sampling_rate_index = 14;    break;
            default:
                sampling_rate_index = 0;
                DVFS_LOG_E("[mcps_parser](ID %d) Invalid sampling rate = %d\n", id, sampling_rate_value);
                break;
        }
    }
    return sampling_rate_index;
}

int32_t mcps_parser_get_channel_number_index (adsp_swip_id id, uint32_t channel_number_value)
{
    int32_t channel_number_index;
    if (id == SWIP_ID_PROCESSING_SRC) {
        switch (channel_number_value) {
            case 4: channel_number_index = 0;   break;
            case 3: channel_number_index = 1;   break;
            case 2: channel_number_index = 2;   break;
            case 1: channel_number_index = 3;   break;
            default:
                channel_number_index = 0;
                DVFS_LOG_E("[mcps_parser](BLISRC) Invalid channel number = %d\n", channel_number_value);
                break;
        }
    } else {
        switch (channel_number_value) {
            case 2: channel_number_index = 0;   break;
            case 1: channel_number_index = 1;   break;
            default:
                channel_number_index = 0;
                DVFS_LOG_E("[mcps_parser](ID %d) Invalid channel number = %d\n", id, channel_number_value);
                break;
        }
    }
    return channel_number_index;
}

#if defined(__MCPS_PARSER_SBC_ENCODER_SUPPORT__)
static int32_t mcps_parser_get_block_number_index (adsp_swip_id id, sbc_encoder_block_number_t block_number_value)
{
    int32_t block_number_index = 3;
    if (id == SWIP_ID_CODEC_SBC_ENCODER) {
        switch (block_number_value) {
            case SBC_ENCODER_BLOCK_NUMBER_16:   block_number_index = 0; break;
            case SBC_ENCODER_BLOCK_NUMBER_12:   block_number_index = 1; break;
            case SBC_ENCODER_BLOCK_NUMBER_8:    block_number_index = 2; break;
            case SBC_ENCODER_BLOCK_NUMBER_4:    block_number_index = 3; break;
            default:
                DVFS_LOG_E("[mcps_parser](SBC Encoder) Invalid block number = %d\n", block_number_value);
                break;
        }
    } else {
        DVFS_LOG_E("[mcps_parser] Invalid SWIP ID %d for block number %d\n", id, block_number_value);
    }
    return block_number_index;
}
#endif  /* __MCPS_PARSER_SBC_ENCODER_SUPPORT__ */

#if defined(__MCPS_PARSER_BESLOUDNESS_SUPPORT__)
static uint32_t mcps_parser_get_single_channel_biquad_number(BLOUD_HD_FilterCoef *p_coef)
{
    uint32_t biquad_num = 0;
    {   /* Check HPF */
        int32_t idx;
        for (idx = 0; idx < 2; idx++) {
            uint32_t coef_0 = p_coef->HPF_COEF[idx][0];
            uint32_t coef_1 = p_coef->HPF_COEF[idx][1];
            uint32_t coef_2 = p_coef->HPF_COEF[idx][2];
            uint32_t coef_3 = p_coef->HPF_COEF[idx][3];
            uint32_t coef_4 = p_coef->HPF_COEF[idx][4];
            if ((coef_0 | coef_1 | coef_2 | coef_3 | coef_4) != 0) {
                biquad_num++;
            }
        }
    }
    {   /* Check BPF */
        int32_t idx;
        for (idx = 0; idx < 8; idx++) {
            uint32_t coef_0 = p_coef->BPF_COEF[idx][0];
            uint32_t coef_1 = p_coef->BPF_COEF[idx][1];
            uint32_t coef_2 = p_coef->BPF_COEF[idx][2];
            uint32_t coef_3 = p_coef->BPF_COEF[idx][3];
            uint32_t coef_4 = p_coef->BPF_COEF[idx][4];
            uint32_t coef_5 = p_coef->BPF_COEF[idx][5];
            if ((coef_0 | coef_1 | coef_2 | coef_3 | coef_4 | coef_5) != 0) {
                biquad_num++;
            }
        }
    }
    {   /* Check LPF */
        uint32_t coef_0 = p_coef->LPF_COEF[0];
        uint32_t coef_1 = p_coef->LPF_COEF[1];
        uint32_t coef_2 = p_coef->LPF_COEF[2];
        if ((coef_0 | coef_1 | coef_2) != 0) {
            biquad_num++;
        }
    }
    return biquad_num;
}

static uint32_t mcps_parser_get_biquad_number(BLOUD_HD_FilterCoef *p_coef_l, BLOUD_HD_FilterCoef *p_coef_r, uint32_t lr_flag)
{
    uint32_t biquad_num;
    uint32_t biquad_l_num;
    uint32_t biquad_r_num;
    {   /* Check L channel coefficients */
        mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_FILTER_COEF_L, (void *)p_coef_l);
        biquad_l_num = mcps_parser_get_single_channel_biquad_number(p_coef_l);
    }
    if (lr_flag != 0) { /* Check R channel coefficients */
        mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_FILTER_COEF_R, (void *)p_coef_r);
        biquad_r_num = mcps_parser_get_single_channel_biquad_number(p_coef_r);
    } else {
        biquad_r_num = 0;
    }
    biquad_num = MAXIMUM(biquad_l_num, biquad_r_num);
    biquad_num = BOUNDED(biquad_num, ACF_BIQUAD_MAX_NUMBER, ACF_BIQUAD_MIN_NUMBER);
    return biquad_num;
}
#endif  /* __MCPS_PARSER_BESLOUDNESS_SUPPORT__ */

int32_t mcps_parser_get_audio_format_index (adsp_swip_id id, uint32_t audio_format_value)
{
    int32_t audio_format_index;
    if (id == SWIP_ID_PROCESSING_SRC) {
        switch (audio_format_value) {
            case AUDIO_FORMAT_PCM_32_BIT:   audio_format_index = 0; break;  /* Q1.31 */
            case AUDIO_FORMAT_PCM_8_24_BIT: audio_format_index = 1; break;  /* Q9.23 */
            case AUDIO_FORMAT_PCM_16_BIT:   audio_format_index = 2; break;  /* Q1.15 */
            default:
                audio_format_index = 0;
                DVFS_LOG_E("[mcps_parser](BLISRC) Invalid audio format = %d\n", audio_format_value);
                break;
        }
    } else {
        switch (audio_format_value) {
            case AUDIO_FORMAT_PCM_32_BIT:   audio_format_index = 0; break;  /* Q1.31 */
            case AUDIO_FORMAT_PCM_16_BIT:   audio_format_index = 1; break;  /* Q1.15 */
            default:
                audio_format_index = 0;
                DVFS_LOG_E("[mcps_parser](ID %d) Invalid audio format = %d\n", id, audio_format_value);
                break;
        }
    }
    return audio_format_index;
}

uint32_t dummy_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    /* do config parsing here */
    return 0;
}
/* codec_heaac_decoder_lib_mcps[HEAAC or AAC-LC] */
/*                             [stereo or mono]  */
/*                             [48k, 24k or 12k] */
const uint32_t codec_heaac_decoder_lib_mcps[2][2][3] = {
    /* HEAAC stereo,    HEAAC mono*/
    { { 44, 22, 22 },  { 26, 13, 13 } },
    /* AAC-LC stereo,   AAC-LC mono*/
    { { 23,  9,  6 },  { 12,  6,  3 } }
};

const uint32_t voip_enh_mcps[DVFS_VOIP_NUM_TYPE] = {
     [DVFS_VOIP_DL_16K_SPEAKER_2MIC]     = 31,
     [DVFS_VOIP_DL_16K_RECEIVER_2MIC]    = 34,
     [DVFS_VOIP_DL_16K_HEADPHONE_2MIC]   = 34,
     [DVFS_VOIP_DL_16K_HEADSET_HSMIC]    = 34,
     [DVFS_VOIP_DL_32K_SPEAKER_2MIC]     = 56,
     [DVFS_VOIP_DL_32K_RECEIVER_2MIC]    = 54,
     [DVFS_VOIP_DL_32K_HEADPHONE_2MIC]   = 54,
     [DVFS_VOIP_DL_32K_HEADSET_HSMIC]    = 54,
     [DVFS_VOIP_DL_48K_SPEAKER_2MIC]     = 35,
     [DVFS_VOIP_DL_48K_RECEIVER_2MIC]    = 29,
     [DVFS_VOIP_DL_48K_HEADPHONE_2MIC]   = 29,
     [DVFS_VOIP_DL_48K_HEADSET_HSMIC]    = 26,
     [DVFS_VOIP_UL_16K_SPEAKER_2MIC]     = 187,
     [DVFS_VOIP_UL_16K_RECEIVER_2MIC]    = 185,
     [DVFS_VOIP_UL_16K_HEADPHONE_2MIC]   = 185,
     [DVFS_VOIP_UL_16K_HEADSET_HSMIC]    = 185,
     [DVFS_VOIP_UL_32K_SPEAKER_2MIC]     = 239,
     [DVFS_VOIP_UL_32K_RECEIVER_2MIC]    = 220,
     [DVFS_VOIP_UL_32K_HEADPHONE_2MIC]   = 220,
     [DVFS_VOIP_UL_32K_HEADSET_HSMIC]    = 220,
     [DVFS_VOIP_UL_48K_SPEAKER_2MIC]     = 277,
     [DVFS_VOIP_UL_48K_RECEIVER_2MIC]    = 192,
     [DVFS_VOIP_UL_48K_HEADPHONE_2MIC]   = 192,
     [DVFS_VOIP_UL_48K_HEADSET_HSMIC]    = 112,
};

const uint32_t smartpa_mcps[DVFS_SMARTPA_NUM_TYPE] = {
#if MT6660_SUPPORT
    [DVFS_SMARTPA_MUSIC] = 70,
    [DVFS_SMARTPA_RINGTONE] = 70,
    [DVFS_SMARTPA_VOICE] = 70,
#elif NXP_SMARTPA_SUPPORT
    [DVFS_SMARTPA_MUSIC] = 72,
    [DVFS_SMARTPA_RINGTONE] = 72,
    [DVFS_SMARTPA_VOICE] = 68,
#else
    [DVFS_SMARTPA_MUSIC] = 0,
    [DVFS_SMARTPA_RINGTONE] = 0,
    [DVFS_SMARTPA_VOICE] = 0,
#endif
};

uint32_t codec_heaac_decoder_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    uint32_t sbr_value = 0;
    uint32_t sbr_index = 0;
    uint32_t ch_index = 0;
    uint32_t hz_index = 0;

    if ( (p_config->p_config) == (void *)NULL ) {
        DVFS_LOG_E("%s SBR flag pointer is NULL\n",
                   __FUNCTION__);
        return( codec_heaac_decoder_lib_mcps[0][0][0] );
    }

    if ( (p_config->dl_out.channels != 1) &&
        (p_config->dl_out.channels != 2)    ) {
        DVFS_LOG_E("%s channel number is invalid: %d\n",
                   __FUNCTION__,
                   p_config->dl_out.channels);
        return( codec_heaac_decoder_lib_mcps[0][0][0] );
    }

    if ( (p_config->dl_out.sample_rate > 48000) ||
        (p_config->dl_out.sample_rate == 0)       ) {
        DVFS_LOG_E("%s sample rate is invalid: %d\n",
                   __FUNCTION__,
                   p_config->dl_out.sample_rate);
        return( codec_heaac_decoder_lib_mcps[0][0][0] );
    }

    sbr_value = *((uint32_t *)(p_config->p_config));

    DVFS_LOG_E("%s [output]SBR=%d, ch=%d, Hz=%d\n",
                __FUNCTION__,
                sbr_value,
                p_config->dl_out.channels,
                p_config->dl_out.sample_rate);

    if ( sbr_value != 0 ) {
        sbr_index = 0;
    }
    else {
        sbr_index = 1;
    }

    if ( p_config->dl_out.channels == 2 ) {
        ch_index = 0;
    }
    else {
        ch_index = 1;
    }

    if ( 12000 >= p_config->dl_out.sample_rate ) {
        hz_index = 2;
    }
    else if ( 24000 >= p_config->dl_out.sample_rate ) {
        hz_index = 1;
    }
    else {
        hz_index = 0;
    }

    return( codec_heaac_decoder_lib_mcps[sbr_index][ch_index][hz_index] );
}


/* codec_heaac_encoder_lib_mcps[HEAAC or AAC-LC] */
/*                             [stereo or mono]  */
/*                             [48k, 24k or 12k] */
const uint32_t codec_heaac_encoder_lib_mcps[2][2][3] = {
    /* HEAAC stereo,    HEAAC mono*/
    { { 73, 29, 29 },  { 39, 19, 19 } },
    /* AAC-LC stereo,   AAC-LC mono*/
    { { 55, 28, 16 },  { 31, 19,  9 } }
};

uint32_t codec_heaac_encoder_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    uint32_t sbr_value = 0;
    uint32_t sbr_index = 0;
    uint32_t ch_index = 0;
    uint32_t hz_index = 0;

    if ( (p_config->p_config) == (void *)NULL ) {
        DVFS_LOG_E("%s SBR flag pointer is NULL\n",
                   __FUNCTION__);
        return( codec_heaac_encoder_lib_mcps[0][0][0] );
    }

    if ( (p_config->ul_in.channels != 1) &&
        (p_config->ul_in.channels != 2)    ) {
        DVFS_LOG_E("%s channel number is invalid: %d\n",
                   __FUNCTION__,
                   p_config->ul_in.channels);
        return( codec_heaac_encoder_lib_mcps[0][0][0] );
    }

    if ( (p_config->ul_in.sample_rate > 48000) ||
        (p_config->ul_in.sample_rate == 0)       ) {
        DVFS_LOG_E("%s sample rate is invalid: %d\n",
                   __FUNCTION__,
                   p_config->ul_in.sample_rate);
        return( codec_heaac_encoder_lib_mcps[0][0][0] );
    }

    sbr_value = *((uint32_t *)(p_config->p_config));

    DVFS_LOG_E("%s [input]SBR=%d, ch=%d, Hz=%d\n",
                __FUNCTION__,
                sbr_value,
                p_config->ul_in.channels,
                p_config->ul_in.sample_rate);

    if ( sbr_value != 0 ) {
        sbr_index = 0;
    }
    else {
        sbr_index = 1;
    }

    if ( p_config->ul_in.channels == 2 ) {
        ch_index = 0;
    }
    else {
        ch_index = 1;
    }

    if ( 12000 >= p_config->ul_in.sample_rate ) {
        hz_index = 2;
    }
    else if ( 24000 >= p_config->ul_in.sample_rate ) {
        hz_index = 1;
    }
    else {
        hz_index = 0;
    }

    return( codec_heaac_encoder_lib_mcps[sbr_index][ch_index][hz_index] );
}


/* codec_mp3_decoder_lib_mcps[stereo or mono]  */
/*                           [48k, 24k or 12k] */
const uint32_t codec_mp3_decoder_lib_mcps[2][3] = {
    /* MP3 stereo*/
    { 18, 14,  7 },
    /* MP3 mono*/
    { 15,  9,  5 }
};

uint32_t codec_mp3_decoder_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    uint32_t ch_index = 0;
    uint32_t hz_index = 0;

    if ( (p_config->dl_out.channels != 1) &&
        (p_config->dl_out.channels != 2)    ) {
        DVFS_LOG_E("%s channel number is invalid: %d\n",
                   __FUNCTION__,
                   p_config->dl_out.channels);
        return( codec_mp3_decoder_lib_mcps[0][0] );
    }

    if ( (p_config->dl_out.sample_rate > 48000) ||
                   (p_config->dl_out.sample_rate == 0)    ) {
        DVFS_LOG_E("%s sample rate is invalid: %d\n",
                   __FUNCTION__,
                   p_config->dl_out.sample_rate);
        return( codec_mp3_decoder_lib_mcps[0][0] );
    }

    DVFS_LOG_E("%s [output]ch=%d, Hz=%d\n",
                __FUNCTION__,
                p_config->dl_out.channels,
                p_config->dl_out.sample_rate);

    if ( p_config->dl_out.channels == 2 ) {
        ch_index = 0;
    }
    else {
        ch_index = 1;
    }

    if ( 12000 >= p_config->dl_out.sample_rate ) {
        hz_index = 2;
    }
    else if ( 24000 >= p_config->dl_out.sample_rate ) {
        hz_index = 1;
    }
    else {
        hz_index = 0;
    }

    return( codec_mp3_decoder_lib_mcps[ch_index][hz_index] );
}


const uint32_t codec_record_lib_mcps[1] = {
    /* input_source*/
     267
};

uint32_t codec_effect_record_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    uint32_t index = 0;

    switch (p_config->input_source) {
        case AUDIO_SOURCE_MIC :
            index = 0;
        default :
            index = 0;
    }
     return( codec_record_lib_mcps[index] );
}

uint32_t swip_voip_enh_parser(dvfs_swip_config_t *p_config)
{
    /* DL process MCPS*/
    if (p_config->dl_in.sample_rate == 16000) {
        if (p_config->intput_devices == AUDIO_DEVICE_IN_BUILTIN_MIC ||
            p_config->intput_devices == AUDIO_DEVICE_IN_BACK_MIC) {
            switch (p_config->output_devices) {
                case AUDIO_DEVICE_OUT_SPEAKER:
                    return voip_enh_mcps[DVFS_VOIP_DL_16K_SPEAKER_2MIC];
                case AUDIO_DEVICE_OUT_EARPIECE:
                    return voip_enh_mcps[DVFS_VOIP_DL_16K_RECEIVER_2MIC];
                case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                    return voip_enh_mcps[DVFS_VOIP_DL_16K_HEADPHONE_2MIC];
                default:
                    DVFS_LOG_E("%s(), Unsupport output devices", __FUNCTION__);
            }
        } else if (p_config->intput_devices == AUDIO_DEVICE_IN_WIRED_HEADSET) {
            switch (p_config->output_devices) {
                case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                    return voip_enh_mcps[DVFS_VOIP_DL_16K_HEADSET_HSMIC];
                default:
                    DVFS_LOG_E("%s(), Unsupport output device", __FUNCTION__);
            }
        }
        DVFS_LOG_E("%s(), cannot find divce pare, return DL %d mic-spk instead, in dev = 0x%x, out dev = 0x%x",
              __FUNCTION__, p_config->dl_in.sample_rate,
              p_config->intput_devices,
              p_config->output_devices);
        return voip_enh_mcps[DVFS_VOIP_DL_16K_SPEAKER_2MIC];
    } else if (p_config->dl_in.sample_rate == 32000) {
        if (p_config->intput_devices == AUDIO_DEVICE_IN_BUILTIN_MIC ||
            p_config->intput_devices == AUDIO_DEVICE_IN_BACK_MIC) {
                switch (p_config->output_devices) {
                    case AUDIO_DEVICE_OUT_SPEAKER:
                        return voip_enh_mcps[DVFS_VOIP_DL_32K_SPEAKER_2MIC];
                    case AUDIO_DEVICE_OUT_EARPIECE:
                        return voip_enh_mcps[DVFS_VOIP_DL_32K_RECEIVER_2MIC];
                    case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                        return voip_enh_mcps[DVFS_VOIP_DL_32K_HEADPHONE_2MIC];
                    default:
                        DVFS_LOG_E("%s(), Unsupport output devices", __FUNCTION__);
                }
            } else if (p_config->intput_devices == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                switch (p_config->output_devices) {
                    case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                        return voip_enh_mcps[DVFS_VOIP_DL_32K_HEADSET_HSMIC];
                    default:
                        DVFS_LOG_E("%s(), Unsupport output devices", __FUNCTION__);
                }
            }
            DVFS_LOG_E("%s(), cannot find divce pare, return DL %d mic-spk instead, in dev = 0x%x, out dev = 0x%x",
                  __FUNCTION__, p_config->dl_in.sample_rate,
                  p_config->intput_devices,
                  p_config->output_devices);
            return voip_enh_mcps[DVFS_VOIP_DL_32K_SPEAKER_2MIC];
    } else if (p_config->dl_in.sample_rate == 48000) {
        if (p_config->intput_devices == AUDIO_DEVICE_IN_BUILTIN_MIC ||
            p_config->intput_devices == AUDIO_DEVICE_IN_BACK_MIC) {
            switch (p_config->output_devices) {
                case AUDIO_DEVICE_OUT_SPEAKER:
                    return voip_enh_mcps[DVFS_VOIP_DL_48K_SPEAKER_2MIC];
                case AUDIO_DEVICE_OUT_EARPIECE:
                    return voip_enh_mcps[DVFS_VOIP_DL_48K_RECEIVER_2MIC];
                case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                    return voip_enh_mcps[DVFS_VOIP_DL_48K_HEADPHONE_2MIC];
                default:
                    DVFS_LOG_E("%s(), Unsupport output devices", __FUNCTION__);
            }
        } else if (p_config->intput_devices == AUDIO_DEVICE_IN_WIRED_HEADSET) {
            switch (p_config->output_devices) {
                case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                    return voip_enh_mcps[DVFS_VOIP_DL_48K_HEADSET_HSMIC];
                default:
                    DVFS_LOG_E("%s(), Unsupport output devices", __FUNCTION__);
            }
        }
        DVFS_LOG_E("%s(), cannot find divce pare, return DL %d mic-spk instead, in dev = 0x%x, out dev = 0x%x",
              __FUNCTION__, p_config->dl_in.sample_rate,
              p_config->intput_devices,
              p_config->output_devices);
        return voip_enh_mcps[DVFS_VOIP_DL_48K_SPEAKER_2MIC];
    }

    /* UL process MCPS*/
    if (p_config->ul_in.sample_rate == 16000) {
        if (p_config->intput_devices == AUDIO_DEVICE_IN_BUILTIN_MIC ||
            p_config->intput_devices == AUDIO_DEVICE_IN_BACK_MIC) {
            switch (p_config->output_devices) {
                case AUDIO_DEVICE_OUT_SPEAKER:
                    return voip_enh_mcps[DVFS_VOIP_UL_16K_SPEAKER_2MIC];
                case AUDIO_DEVICE_OUT_EARPIECE:
                    return voip_enh_mcps[DVFS_VOIP_UL_16K_RECEIVER_2MIC];
                case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                    return voip_enh_mcps[DVFS_VOIP_UL_16K_HEADPHONE_2MIC];
                default:
                    DVFS_LOG_E("%s(), Unsupport output devices", __FUNCTION__);
            }
        } else if (p_config->intput_devices == AUDIO_DEVICE_IN_WIRED_HEADSET) {
            switch (p_config->output_devices) {
                case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                    return voip_enh_mcps[DVFS_VOIP_UL_16K_HEADSET_HSMIC];
                default:
                    DVFS_LOG_E("%s(), Unsupport output device", __FUNCTION__);
            }
        }
        DVFS_LOG_E("%s(), cannot find divce pare, return UL %d mic-spk instead, in dev = 0x%x, out dev = 0x%x",
              __FUNCTION__, p_config->ul_in.sample_rate,
              p_config->intput_devices,
              p_config->output_devices);
        return voip_enh_mcps[DVFS_VOIP_UL_16K_SPEAKER_2MIC];
    } else if (p_config->ul_in.sample_rate == 32000) {
        if (p_config->intput_devices == AUDIO_DEVICE_IN_BUILTIN_MIC ||
            p_config->intput_devices == AUDIO_DEVICE_IN_BACK_MIC) {
            switch (p_config->output_devices) {
                case AUDIO_DEVICE_OUT_SPEAKER:
                    return voip_enh_mcps[DVFS_VOIP_UL_32K_SPEAKER_2MIC];
                case AUDIO_DEVICE_OUT_EARPIECE:
                    return voip_enh_mcps[DVFS_VOIP_UL_32K_RECEIVER_2MIC];
                case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                    return voip_enh_mcps[DVFS_VOIP_UL_32K_HEADPHONE_2MIC];
                default:
                    DVFS_LOG_E("%s(), Unsupport output devices", __FUNCTION__);
            }
        } else if (p_config->intput_devices == AUDIO_DEVICE_IN_WIRED_HEADSET) {
            switch (p_config->output_devices) {
                case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                    return voip_enh_mcps[DVFS_VOIP_UL_32K_HEADSET_HSMIC];
                default:
                    DVFS_LOG_E("%s(), Unsupport output devices", __FUNCTION__);
            }
        }
        DVFS_LOG_E("%s(), cannot find divce pare, return UL %d mic-spk instead, in dev = 0x%x, out dev = 0x%x",
              __FUNCTION__, p_config->ul_in.sample_rate,
              p_config->intput_devices,
              p_config->output_devices);
        return voip_enh_mcps[DVFS_VOIP_UL_32K_SPEAKER_2MIC];
    } else if (p_config->ul_in.sample_rate == 48000) {
        if (p_config->intput_devices == AUDIO_DEVICE_IN_BUILTIN_MIC ||
            p_config->intput_devices == AUDIO_DEVICE_IN_BACK_MIC) {
            switch (p_config->output_devices) {
                case AUDIO_DEVICE_OUT_SPEAKER:
                    return voip_enh_mcps[DVFS_VOIP_UL_48K_SPEAKER_2MIC];
                case AUDIO_DEVICE_OUT_EARPIECE:
                    return voip_enh_mcps[DVFS_VOIP_UL_48K_RECEIVER_2MIC];
                case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                    return voip_enh_mcps[DVFS_VOIP_UL_48K_HEADPHONE_2MIC];
                default:
                    DVFS_LOG_E("%s(), Unsupport output devices", __FUNCTION__);
            }
        } else if (p_config->intput_devices == AUDIO_DEVICE_IN_WIRED_HEADSET) {
            switch (p_config->output_devices) {
                case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                    return voip_enh_mcps[DVFS_VOIP_UL_48K_HEADSET_HSMIC];
                default:
                    DVFS_LOG_E("%s(), Unsupport output devices", __FUNCTION__);
            }
        }
        DVFS_LOG_E("%s(), cannot find divce pare, return UL %d mic-spk instead, in dev = 0x%x, out dev = 0x%x",
              __FUNCTION__, p_config->ul_in.sample_rate,
              p_config->intput_devices,
              p_config->output_devices);
        return voip_enh_mcps[DVFS_VOIP_UL_48K_SPEAKER_2MIC];
    }

    DVFS_LOG_E("%s(), UnSupport sample rate!!, return 48k UL mic-spk instead", __FUNCTION__);
    return voip_enh_mcps[DVFS_VOIP_UL_48K_SPEAKER_2MIC];
}

#if defined(__MCPS_PARSER_SBC_ENCODER_SUPPORT__)
static uint32_t codec_sbc_encoder_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    audio_profile *p_dl_out;
    sbc_encoder_initial_parameter_t *p_init_param;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_INPUT_CONFIG, (void *)p_config);
    p_dl_out = &p_config->dl_out;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_DL_OUT, (void *)p_dl_out);
    p_init_param = (sbc_encoder_initial_parameter_t *)p_config->p_config;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_SBC_ENCODER_INDEX_INIT_PARAM, (void *)p_init_param);
    {
        uint32_t sr_val = p_dl_out->sample_rate;
        sbc_encoder_block_number_t blk_val = p_init_param->block_number;
        int32_t sr_idx  = mcps_parser_get_sampling_rate_index(SWIP_ID_CODEC_SBC_ENCODER, sr_val);
        int32_t blk_idx = mcps_parser_get_block_number_index (SWIP_ID_CODEC_SBC_ENCODER, blk_val);
        uint32_t mcps_value = (uint32_t)sbc_encoder_mcps_table[blk_idx][sr_idx];
        return mcps_value;
    }
}
#endif  /* __MCPS_PARSER_SBC_ENCODER_SUPPORT__ */

static uint32_t processing_smartpa_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    if (p_config->audio_mode == AUDIO_MODE_NORMAL) {
        return smartpa_mcps[DVFS_SMARTPA_MUSIC];
    } else if (p_config->audio_mode == AUDIO_MODE_RINGTONE) {
        return smartpa_mcps[DVFS_SMARTPA_RINGTONE];
    } else if (p_config->audio_mode == AUDIO_MODE_IN_CALL ||
               p_config->audio_mode == AUDIO_MODE_IN_COMMUNICATION) {
        return smartpa_mcps[DVFS_SMARTPA_VOICE];
    } else {
        /* default use music parameter */
        DVFS_LOG_E("%s(), invalid mode! return music as default param", __FUNCTION__);
        return smartpa_mcps[DVFS_SMARTPA_MUSIC];
    }
}

static uint32_t processing_ktv_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    return 250;
}

#if defined(__MCPS_PARSER_BESLOUDNESS_SUPPORT__)
static uint32_t processing_besloudness_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    audio_profile *p_dl_out;
    audioloud_lib_handle_t *p_lib_hdl;
    BLOUD_HD_InitParam *p_init_param;
    BLOUD_HD_ModeParam *p_mode_param;
    BLOUD_HD_CustomParam *p_custom_param;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_INPUT_CONFIG, (void *)p_config);
    p_dl_out = &p_config->dl_out;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_DL_OUT, (void *)p_dl_out);
    p_lib_hdl = (audioloud_lib_handle_t *)p_config->p_config;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_LIB_HANDLE, (void *)p_lib_hdl);
    p_init_param = &p_lib_hdl->mInitParam;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_INIT_PARAM, (void *)p_init_param);
    p_mode_param = p_init_param->pMode_Param;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_MODE_PARAM, (void *)p_mode_param);
    p_custom_param = p_mode_param->pCustom_Param;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BESLOUDNESS_INDEX_CUSTOM_PARAM, (void *)p_custom_param);
    {
        uint32_t mcps_value = 0;
        uint32_t sr_val = p_dl_out->sample_rate;
        uint32_t ch_val = p_dl_out->channels;
        uint32_t bypass_flag = p_lib_hdl->mBypassFlag;
        uint32_t drc_flag = p_mode_param->Loudness_Mode != HD_LOUD_MODE_NONE && (bypass_flag & FILTER_BYPASS_DRC) == 0 ? 1 : 0;
        uint32_t acf_flag = p_mode_param->Filter_Mode != HD_FILT_MODE_NONE && (bypass_flag & FILTER_BYPASS_ACF) == 0 ? 1 : 0;
        uint32_t s2m2s_flag = p_mode_param->S2M_Mode == HD_S2M_MODE_ST2MO2ST ? 1 : 0;
        mcps_parser_get_sampling_rate_index (SWIP_ID_PROCESSING_BESLOUDNESS, sr_val);   /* only to check sampling rate */
        mcps_parser_get_channel_number_index(SWIP_ID_PROCESSING_BESLOUDNESS, ch_val);   /* only to check channel number */
        if (drc_flag != 0) {
            uint32_t drc_mcps;
            uint32_t band_num = BOUNDED(p_custom_param->Num_Bands, DRC_BAND_MAX_NUMBER, DRC_BAND_MIN_NUMBER);
            uint32_t band_idx = DRC_BAND_MAX_NUMBER - band_num;  /* band 8 ~ 1 --> index 0 ~ 7 */
            if (ch_val == 2 && s2m2s_flag == 0) {  /* stereo */
                drc_mcps = drc_mcps_table[0][band_idx]; /* 192kHz MCPS */
            } else {    /* s2m2s, mono */
                drc_mcps = drc_mcps_table[1][band_idx]; /* 192kHz MCPS */
                drc_mcps = ch_val == 2 ? MCPS_CONVERSION_FROM_MONO_TO_S2M2S(drc_mcps) : drc_mcps;   /* s2m2s */
            }
            if (sr_val != MAX_SAMPLING_RATE) {
                drc_mcps = MCPS_CONVERSION_FROM_MAX_SAMPLING_RATE_TO_TARGET (drc_mcps, sr_val);
            }
            mcps_value += drc_mcps;
        }
        if (acf_flag != 0) {
            uint32_t acf_mcps;
            uint32_t lr_flag = p_custom_param->Sep_LR_Filter != 0 ? 1 : 0;
            BLOUD_HD_FilterCoef *p_coef_l = p_mode_param->pFilter_Coef_L;
            BLOUD_HD_FilterCoef *p_coef_r = p_mode_param->pFilter_Coef_R;
            uint32_t biquad_num = mcps_parser_get_biquad_number(p_coef_l, p_coef_r, lr_flag);
            uint32_t biquad_idx = ACF_BIQUAD_MAX_NUMBER - biquad_num;    /* biquad 11 ~ 1 --> index 0 ~ 10 */
            if (ch_val == 2 && (lr_flag != 0 || s2m2s_flag == 0)) {  /* stereo with L / R seperation, stereo */
                acf_mcps = acf_mcps_table[0][biquad_idx];
            } else {    /* s2m2s, mono */
                acf_mcps = acf_mcps_table[1][biquad_idx];
                acf_mcps = ch_val == 2 ? MCPS_CONVERSION_FROM_MONO_TO_S2M2S(acf_mcps) : acf_mcps;   /* s2m2s */
            }
            if (sr_val != MAX_SAMPLING_RATE) {
                acf_mcps = MCPS_CONVERSION_FROM_MAX_SAMPLING_RATE_TO_TARGET (acf_mcps, sr_val);
            }
            mcps_value += acf_mcps;
        }
        return mcps_value;
    }
}
#endif  /* __MCPS_PARSER_BESLOUDNESS_SUPPORT__ */

static uint32_t processing_dirac_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    if (p_config->output_devices == AUDIO_DEVICE_OUT_WIRED_HEADSET ||
        p_config->output_devices == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
        return 227;
    } else {
        return 169;
    }
}
#if defined(__MCPS_PARSER_DCREMOVAL_SUPPORT__)
static uint32_t processing_dcremoval_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    audio_profile *p_dl_out;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_DCREMOVAL_INDEX_INPUT_CONFIG, (void *)p_config);
    p_dl_out = &p_config->dl_out;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_DCREMOVAL_INDEX_DL_OUT, (void *)p_dl_out);
    {
        uint32_t sr_val = p_dl_out->sample_rate;
        uint32_t ch_val = p_dl_out->channels;
        uint32_t fmt_val = p_dl_out->format;
        int32_t sr_idx  = mcps_parser_get_sampling_rate_index (SWIP_ID_PROCESSING_DC_REMOVAL, sr_val);
        int32_t ch_idx  = mcps_parser_get_channel_number_index(SWIP_ID_PROCESSING_DC_REMOVAL, ch_val);
        int32_t fmt_idx = mcps_parser_get_audio_format_index  (SWIP_ID_PROCESSING_DC_REMOVAL, fmt_val);
        uint32_t mcps_value = (uint32_t)dcremoval_mcps_table[fmt_idx][ch_idx][sr_idx];
        return mcps_value;
    }
}
#endif  /* __MCPS_PARSER_DCREMOVAL_SUPPORT__ */

#if defined(__MCPS_PARSER_BLISRC_SUPPORT__)
static uint32_t processing_blisrc_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    audio_profile *p_dl_in;
    audio_profile *p_dl_out;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_INPUT_CONFIG, (void *)p_config);
    p_dl_in = &p_config->dl_in;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_DL_IN, (void *)p_dl_in);
    p_dl_out = &p_config->dl_out;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_BLISRC_INDEX_DL_OUT, (void *)p_dl_out);
    {
        uint32_t in_sr_val   = p_dl_in->sample_rate;
        uint32_t in_ch_val   = p_dl_in->channels;
        uint32_t in_fmt_val  = p_dl_in->format;
        uint32_t out_sr_val  = p_dl_out->sample_rate;
        uint32_t out_ch_val  = p_dl_out->channels;
        uint32_t out_fmt_val = p_dl_out->format;
        int32_t in_sr_idx   = mcps_parser_get_sampling_rate_index (SWIP_ID_PROCESSING_SRC, in_sr_val);
        int32_t in_ch_idx   = mcps_parser_get_channel_number_index(SWIP_ID_PROCESSING_SRC, in_ch_val);
        int32_t in_fmt_idx  = mcps_parser_get_audio_format_index  (SWIP_ID_PROCESSING_SRC, in_fmt_val);
        int32_t out_sr_idx  = mcps_parser_get_sampling_rate_index (SWIP_ID_PROCESSING_SRC, out_sr_val);
        int32_t out_ch_idx  = mcps_parser_get_channel_number_index(SWIP_ID_PROCESSING_SRC, out_ch_val);
        int32_t out_fmt_idx = mcps_parser_get_audio_format_index  (SWIP_ID_PROCESSING_SRC, out_fmt_val);
        uint32_t mcps_value;
        if (in_fmt_idx != out_fmt_idx) {    /* Audio format error handling */
            out_fmt_idx = in_fmt_idx;
            DVFS_LOG_E("[mcps_parser](BLISRC) input audio format %d does not equal to output audio format %d\n", in_fmt_val, out_fmt_val);
        }
        if ((in_ch_idx < 2 || out_ch_idx < 2) && in_ch_idx != out_ch_idx) { /* Channel number error handling */
            out_ch_idx = in_ch_idx;
            DVFS_LOG_E("[mcps_parser](BLISRC) invalid input / output channel number = %d / %d\n", in_ch_val, out_ch_val);
        }
        if (in_sr_val == 48000 && out_sr_val == 16000) {    /* special case 1 : 48kHz --> 16kHz */
            if (out_ch_idx < 2) {
                mcps_value = 12;    /* 48kHz to 16kHz, Q1.15, 4 channels */
            } else {
                mcps_value =  6;    /* 48kHz to 16kHz, Q1.15, s2s */
            }
        } else if (in_sr_val == 48000 && out_sr_val == 32000) { /* special case 2 : 48kHz --> 32kHz */
            if (out_ch_idx < 2) {
                mcps_value = 12;    /* 48kHz to 32kHz, Q1.15, 4 channels */
            } else {
                mcps_value =  6;    /* 48kHz to 32kHz, Q1.15, s2s */
            }
        } else if (out_fmt_val == AUDIO_FORMAT_PCM_16_BIT && in_ch_val == 2 && out_ch_val == 2 && out_sr_val == 44100) {
            /* special case 3 : 48kHz stereo 16-bit --> 44.1kHz stereo 16-bit */
            mcps_value = 10;
        } else {    /* Other cases */
            if (in_sr_idx < 3) {    /* Input sampling rate error handling */
                in_sr_idx = 3;
                DVFS_LOG_E("[mcps_parser](BLISRC) invalid input / output sampling rate = %d / %d\n", in_sr_val, out_sr_val);
            }
            in_sr_idx -= 3; /* only 96kHz ~ 8kHz */
            if (out_sr_idx != 0 && out_sr_idx != 6) {   /* Output sampling rate error handling */
                DVFS_LOG_E("[mcps_parser](BLISRC) invalid input / output sampling rate = %d / %d\n", in_sr_val, out_sr_val);
            }
            out_sr_idx = out_sr_idx < 6 ? 0 : 1;    /* 0 --> 192kHz, 1 --> 48kHz */
            if (in_ch_val == 2) {           /* stereo input */
                if (out_ch_val == 2) {      /* s2s */
                    out_ch_idx = 2;
                } else {                    /* s2m */
                    out_ch_idx = 3;
                }
            } else if (in_ch_val == 1) {    /* mono input */
                if (out_ch_val == 2) {      /* m2s */
                    out_ch_idx = 4;
                } else {                    /* m2m */
                    out_ch_idx = 5;
                }
            }
            out_fmt_idx = out_fmt_idx == 2 ? 1 : 0; /* 0 --> Q1.31 / Q9.23, 1 --> Q1.15 */
            mcps_value = (uint32_t)blisrc_mcps_table[out_fmt_idx][out_ch_idx][out_sr_idx][in_sr_idx];
        }
        return mcps_value;
    }
}
#endif  /* __MCPS_PARSER_BLISRC_SUPPORT__ */

#if defined(__MCPS_PARSER_SHIFTER_SUPPORT__)
static uint32_t processing_shifter_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    audio_profile *p_dl_out;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_SHIFTER_INDEX_INPUT_CONFIG, (void *)p_config);
    p_dl_out = &p_config->dl_out;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_SHIFTER_INDEX_DL_OUT, (void *)p_dl_out);
    {
        uint32_t sr_val = p_dl_out->sample_rate;
        uint32_t ch_val = p_dl_out->channels;
        int32_t sr_idx = mcps_parser_get_sampling_rate_index (SWIP_ID_PROCESSING_SHIFTER, sr_val);
        int32_t ch_idx = mcps_parser_get_channel_number_index(SWIP_ID_PROCESSING_SHIFTER, ch_val);
        uint32_t mcps_value = ch_idx == 0 && sr_idx < 3 ? 2 : 1;
        return mcps_value;
    }
}
#endif  /* __MCPS_PARSER_SHIFTER_SUPPORT__ */

#if defined(__MCPS_PARSER_LIMITER_SUPPORT__)
static uint32_t processing_limiter_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
    audio_profile *p_dl_out;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_LIMITER_INDEX_INPUT_CONFIG, (void *)p_config);
    p_dl_out = &p_config->dl_out;
    mcps_parser_check_null_pointer(DEBUG_ID_MCPS_PARSER_LIMITER_INDEX_DL_OUT, (void *)p_dl_out);
    {
        uint32_t sr_val = p_dl_out->sample_rate;
        uint32_t ch_val = p_dl_out->channels;
        int32_t sr_idx = mcps_parser_get_sampling_rate_index (SWIP_ID_PROCESSING_LIMITER, sr_val);
        int32_t ch_idx = mcps_parser_get_channel_number_index(SWIP_ID_PROCESSING_LIMITER, ch_val);
        uint32_t mcps_value = (uint32_t)limiter_mcps_table[ch_idx][sr_idx];
        return mcps_value;
    }
}
#endif  /* __MCPS_PARSER_LIMITER_SUPPORT__ */


#if defined(__MCPS_PARSER_CALL_SUPPORT__)
#define PHONECALL_MICROPHONE_NUMBER       1   /* Microphone number : 2-mic */
#define PHONECALL_DEVICE_NUMBER           2   /* Device number     : handset, handsfree */
#define PHONECALL_SAMPLING_RATE_NUMBER    3   /* Sampling rate     : 8kHz, 16kHz, 32kHz */

const uint16_t phone_call_enh_mcps_table[PHONECALL_MICROPHONE_NUMBER][PHONECALL_DEVICE_NUMBER][PHONECALL_SAMPLING_RATE_NUMBER] = {
     {{120, 180, 270}, {130, 200, 290}},  /* 2-mic handset : 8kHz ~ 48kHz */ /* 2-mic handsfree : 8kHz ~ 48kHz */
};

uint32_t processing_call_lib_mcps_parser(dvfs_swip_config_t *p_config)
{
	  uint32_t mic_idx, device_idx, sr_idx;
    DVFS_LOG_E("%s [output]num_mics=%d, device=%d, sample_rate=%d\n",
                __FUNCTION__,
                p_config->num_mics,
                p_config->output_devices,
                p_config->dl_out.sample_rate);

    switch (p_config->num_mics) {// support 2mic only for now
    case 2:
        mic_idx = 0;
        break;
    default:
        mic_idx = 0;
        break;
    }

    switch (p_config->output_devices) {
    case AUDIO_DEVICE_OUT_SPEAKER:
        device_idx = 1;
        break;
    default:
        device_idx = 0;
        break;
    }

    switch (p_config->dl_out.sample_rate) {
    case 8000:
        sr_idx = 0;
        break;
    case 16000:
        sr_idx = 1;
        break;
    case 32000:
        sr_idx = 2;
        break;
    default:
        sr_idx = 0;
        break;
    }
    DVFS_LOG_E("%s [output]mic_idx=%d, device_idx=%d, sr_idx=%d\n",
                __FUNCTION__,
                mic_idx,
                device_idx,
                sr_idx);
    AUD_ASSERT((mic_idx < PHONECALL_MICROPHONE_NUMBER) && (device_idx < PHONECALL_DEVICE_NUMBER) && (sr_idx < PHONECALL_SAMPLING_RATE_NUMBER));
    return phone_call_enh_mcps_table[mic_idx][device_idx][sr_idx];
}
#endif /* __MCPS_PARSER_CALL_SUPPORT__ */

void config_parser_init(void)
{
    /* add parser callback here */
    dvfs_register_swip_mcps_parser(SWIP_ID_AURISYS_DEMO,
                                   (void *)dummy_lib_mcps_parser);

    dvfs_register_swip_mcps_parser(SWIP_ID_CODEC_HEAAC_DECODER,
                                   (void *)codec_heaac_decoder_lib_mcps_parser);

    dvfs_register_swip_mcps_parser(SWIP_ID_CODEC_HEAAC_ENCODER,
                                   (void *)codec_heaac_encoder_lib_mcps_parser);

    dvfs_register_swip_mcps_parser(SWIP_ID_CODEC_MP3_DECODER,
                                   (void *)codec_mp3_decoder_lib_mcps_parser);

    dvfs_register_swip_mcps_parser(SWIP_ID_EFFECT_RECORD,
                                   (void *)codec_effect_record_lib_mcps_parser);

    dvfs_register_swip_mcps_parser(SWIP_ID_VOIP_DL_ENH,
                                   (void *)swip_voip_enh_parser);

#if defined(__MCPS_PARSER_SBC_ENCODER_SUPPORT__)
    dvfs_register_swip_mcps_parser(SWIP_ID_CODEC_SBC_ENCODER,
                                   (void *)codec_sbc_encoder_lib_mcps_parser);
#endif  /* __MCPS_PARSER_SBC_ENCODER_SUPPORT__ */

    dvfs_register_swip_mcps_parser(SWIP_ID_PROCESSING_SMARTPA,
                                   (void *)processing_smartpa_lib_mcps_parser);

    dvfs_register_swip_mcps_parser(SWIP_ID_PROCESSING_KTV,
                                   (void *)processing_ktv_lib_mcps_parser);

#if defined(__MCPS_PARSER_BESLOUDNESS_SUPPORT__)
    dvfs_register_swip_mcps_parser(SWIP_ID_PROCESSING_BESLOUDNESS,
                                   (void *)processing_besloudness_lib_mcps_parser);
#endif  /* __MCPS_PARSER_BESLOUDNESS_SUPPORT__ */

    dvfs_register_swip_mcps_parser(SWIP_ID_PROCESSING_DIRAC,
                                   (void *)processing_dirac_lib_mcps_parser);

#if defined(__MCPS_PARSER_DCREMOVAL_SUPPORT__)
    dvfs_register_swip_mcps_parser(SWIP_ID_PROCESSING_DC_REMOVAL,
                                   (void *)processing_dcremoval_lib_mcps_parser);
#endif  /* __MCPS_PARSER_DCREMOVAL_SUPPORT__ */

#if defined(__MCPS_PARSER_BLISRC_SUPPORT__)
    dvfs_register_swip_mcps_parser(SWIP_ID_PROCESSING_SRC,
                                   (void *)processing_blisrc_lib_mcps_parser);
#endif  /* __MCPS_PARSER_BLISRC_SUPPORT__ */

#if defined(__MCPS_PARSER_SHIFTER_SUPPORT__)
    dvfs_register_swip_mcps_parser(SWIP_ID_PROCESSING_SHIFTER,
                                   (void *)processing_shifter_lib_mcps_parser);
#endif  /* __MCPS_PARSER_SHIFTER_SUPPORT__ */

#if defined(__MCPS_PARSER_LIMITER_SUPPORT__)
    dvfs_register_swip_mcps_parser(SWIP_ID_PROCESSING_LIMITER,
                                   (void *)processing_limiter_lib_mcps_parser);
#endif  /* __MCPS_PARSER_LIMITER_SUPPORT__ */

#ifdef PARSER_DEMO
    dvfs_register_swip_mcps_parser(SWIP_ID_PHONE_CALL_ENH,
                                   (void *)phone_call_enh_example);
#endif

#if defined(__MCPS_PARSER_CALL_SUPPORT__)
    dvfs_register_swip_mcps_parser(SWIP_ID_PHONE_CALL_ENH,
                                   (void *)processing_call_lib_mcps_parser);
#endif
}

/* Aurisys config to dvfs config */
void map_aurisys_config_to_dvfs_config(
    struct aurisys_lib_handler_t *lib_handler,
    struct dvfs_swip_config_t *dvfs_config) {
    const struct arsi_task_config_t *task_config = NULL;
    struct arsi_lib_config_t *arsi_lib_config = NULL;

    if (!lib_handler || !dvfs_config) {
        ALOGE("%s, lib_handler and dvfs_config should not be NULL", __FUNCTION__);
        return;
    }

    memset(dvfs_config, 0, sizeof(struct dvfs_swip_config_t));
    task_config = lib_handler->task_config;
    arsi_lib_config = &lib_handler->lib_config;

    dvfs_config->output_devices = task_config->output_device_info.devices;
    dvfs_config->intput_devices = task_config->input_device_info.devices;
    dvfs_config->audio_mode = task_config->audio_mode;

    if (arsi_lib_config->p_ul_buf_in && arsi_lib_config->p_ul_buf_out) {
        dvfs_config->ul_in.channels = arsi_lib_config->p_ul_buf_in->num_channels;
        dvfs_config->ul_out.channels = arsi_lib_config->p_ul_buf_out->num_channels;
        dvfs_config->ul_in.format = arsi_lib_config->p_ul_buf_in->audio_format;
        dvfs_config->ul_out.format = arsi_lib_config->p_ul_buf_out->audio_format;
        dvfs_config->ul_in.sample_rate = arsi_lib_config->p_ul_buf_in->sample_rate_buffer;
        dvfs_config->ul_out.sample_rate = arsi_lib_config->p_ul_buf_out->sample_rate_buffer;
    }

    if (arsi_lib_config->p_dl_buf_in && arsi_lib_config->p_dl_buf_out) {
        dvfs_config->dl_in.channels = arsi_lib_config->p_dl_buf_in->num_channels;
        dvfs_config->dl_out.channels = arsi_lib_config->p_dl_buf_out->num_channels;
        dvfs_config->dl_in.format = arsi_lib_config->p_dl_buf_in->audio_format;
        dvfs_config->dl_out.format = arsi_lib_config->p_dl_buf_out->audio_format;
        dvfs_config->dl_in.sample_rate = arsi_lib_config->p_dl_buf_in->sample_rate_buffer;
        dvfs_config->dl_out.sample_rate = arsi_lib_config->p_dl_buf_out->sample_rate_buffer;
    }
    dvfs_config->num_mics = task_config->max_input_device_num_channels;
    dvfs_config->input_source = task_config->input_source;
    dvfs_config->p_config = lib_handler->arsi_handler;
}

uint32_t get_feature_id(unsigned char task_scene) {
    switch (task_scene) {
        case TASK_SCENE_PHONE_CALL:
            return VOICE_CALL_FEATURE_ID;
        case TASK_SCENE_PLAYBACK_MP3:
            return OFFLOAD_FEATURE_ID;
        case TASK_SCENE_VOIP:
            return VOIP_FEATURE_ID;
        case TASK_SCENE_SPEAKER_PROTECTION:
            return SPK_PROTECT_FEATURE_ID;
        case TASK_SCENE_PRIMARY:
            return PRIMARY_FEATURE_ID;
        case TASK_SCENE_DEEPBUFFER:
            return DEEPBUF_FEATURE_ID;
        case TASK_SCENE_AUDPLAYBACK:
            return AUDIO_PLAYBACK_FEATURE_ID;
        case TASK_SCENE_CAPTURE_UL1:
            return CAPTURE_UL1_FEATURE_ID;
        case TASK_SCENE_A2DP:
            return A2DP_PLAYBACK_FEATURE_ID;
        case TASK_SCENE_DATAPROVIDER:
            return AUDIO_DATAPROVIDER_FEATURE_ID;
        case TASK_SCENE_CALL_FINAL:
            return CALL_FINAL_FEATURE_ID;
        case TASK_SCENE_KTV:
            return KTV_FEATURE_ID;
        default:
            ALOGE("%s(), Unsupport task scene: %d", __FUNCTION__, task_scene);
            AUD_ASSERT(0);
            return TASK_SCENE_PRIMARY;
    }
}

uint32_t get_swip_id(const char *lib_name, unsigned char task_scene) {
    if (!strcmp(lib_name, "aurisys_demo")) {
        return SWIP_ID_AURISYS_DEMO;
    } else if (!strcmp(lib_name, "smartpa_mt6660") ||
               !strcmp(lib_name, "smartpa_tfaxxxx")) {
        return SWIP_ID_PROCESSING_SMARTPA;
    } else if (!strcmp(lib_name, "ktv")) {
        return SWIP_ID_PROCESSING_KTV;
    } else if (!strcmp(lib_name, "mtk_bessound")) {
        return SWIP_ID_PROCESSING_BESLOUDNESS;
    } else if (!strcmp(lib_name, "mtk_dcrflt")) {
        return SWIP_ID_PROCESSING_DC_REMOVAL;
    }  else if (!strcmp(lib_name, "dirac")) {
        return SWIP_ID_PROCESSING_DIRAC;
    } else if (!strcmp(lib_name, "fv_speech")) {
        if (task_scene == TASK_SCENE_PHONE_CALL) {
            return SWIP_ID_PHONE_CALL_ENH;
        } else if (task_scene == TASK_SCENE_VOIP) {
            return SWIP_ID_VOIP_DL_ENH;
        } else {
            return SWIP_ID_EFFECT_RECORD;
        }
    } else if (!strcmp(lib_name, "nxp_speech")) {
        if (task_scene == TASK_SCENE_PHONE_CALL) {
            return SWIP_ID_PHONE_CALL_ENH;
        } else {
            return SWIP_ID_VOIP_DL_ENH;
        }
    } else if (!strcmp(lib_name, "nxp_record")) {
        return SWIP_ID_EFFECT_RECORD;
    } else {
        ALOGE("%s(), Unsupport lib name: %s", __FUNCTION__, lib_name);
        AUD_ASSERT(0);
        return SWIP_ID_AURISYS_DEMO;
    }
}

uint32_t get_swip_mcps (adsp_swip_id id, dvfs_swip_config_t *p_config)
{
	if (swip_mcps_parser[id] == NULL) {
        DVFS_LOG_E("%s(), swip_mcps_parser not registered !! swip_id = %d\n",
                 __FUNCTION__, id);
        return 0;
	}
	return swip_mcps_parser[id](p_config);
}
