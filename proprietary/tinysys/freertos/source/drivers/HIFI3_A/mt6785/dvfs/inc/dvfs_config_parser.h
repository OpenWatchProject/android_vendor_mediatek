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

#ifndef _DVFS_CONFIG_PARSER_
#define _DVFS_CONFIG_PARSER_

#include <stdint.h>
#include <aurisys_lib_handler.h>
#include <arsi_type.h>

//#define PARSER_DEMO

/* typedef */
typedef struct audio_profile {
    uint32_t sample_rate;           /* 8000, 16000, 24000, 32000, ... */
    uint32_t channels;              /* 1, 2, 3, 4, ..., from channel_mask */
    uint32_t format;                /* audio_format_t */
} audio_profile;

typedef struct dvfs_swip_config_t {
    uint32_t output_devices;        /* audio_devices_t */
    uint32_t intput_devices;        /* audio_devices_t */
    audio_profile dl_in;            /* downlink library input */
    audio_profile dl_out;           /* downlink library output */
    audio_profile ul_in;            /* uplink library input */
    audio_profile ul_out;           /* uplink library output */
    uint32_t num_mics;              /* 1, 2, 3, 4, ... */
    uint32_t input_source;          /* audio_source_t, for record only */
    uint32_t bit_rate;              /* reserved */
    uint32_t audio_mode;            /* audio_mode_t */

    void      *p_config;            /* extended */
} dvfs_swip_config_t;

#ifdef PARSER_DEMO
uint32_t phone_call_enh_example(dvfs_swip_config_t *p_config);
#endif
uint32_t codec_heaac_decoder_lib_mcps_parser(dvfs_swip_config_t *p_config);
uint32_t codec_heaac_encoder_lib_mcps_parser(dvfs_swip_config_t *p_config);
uint32_t codec_mp3_decoder_lib_mcps_parser(dvfs_swip_config_t *p_config);
uint32_t codec_effect_record_lib_mcps_parser(dvfs_swip_config_t *p_config);
uint32_t swip_voip_enh_parser(dvfs_swip_config_t *p_config);

/* API */
void config_parser_init(void);

/* Aurisys config to dvfs config */
void map_aurisys_config_to_dvfs_config(
    struct aurisys_lib_handler_t *lib_handler,
    struct dvfs_swip_config_t *dvfs_config);

uint32_t get_feature_id(unsigned char task_scene);
uint32_t get_swip_id(const char *lib_name, unsigned char task_scene);

#endif /*_DVFS_CONFIG_PARSER_*/
