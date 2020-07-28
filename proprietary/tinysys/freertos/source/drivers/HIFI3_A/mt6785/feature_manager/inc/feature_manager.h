/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _FEATURE_MANAGER_
#define _FEATURE_MANAGER_

#include <stdint.h>
#include <driver_api.h>
#include <dvfs_config_parser.h>
#include <wrapped_audio.h>

#define FEATURE_SWIP_MAX_NUM    (10)

/* adsp feature ID list */
typedef enum {
    SYSTEM_FEATURE_ID             = 0,
    ADSP_LOGGER_FEATURE_ID        = 1,
    FORMATTER_FEATURE_ID          = 10,
    PRIMARY_FEATURE_ID            = 20,
    DEEPBUF_FEATURE_ID,
    OFFLOAD_FEATURE_ID,
    AUDIO_PLAYBACK_FEATURE_ID,
    A2DP_PLAYBACK_FEATURE_ID,
    AUDIO_DATAPROVIDER_FEATURE_ID,
    SPK_PROTECT_FEATURE_ID,
    VOICE_CALL_FEATURE_ID,
    VOIP_FEATURE_ID,
    CAPTURE_UL1_FEATURE_ID,
    CALL_FINAL_FEATURE_ID,
    KTV_FEATURE_ID,
    ADSP_NUM_FEATURE_ID,
} adsp_feature_id;

/* audio swip ID List */
typedef enum {
    SWIP_ID_CODEC_HEAAC_DECODER,
    SWIP_ID_CODEC_MP3_DECODER,
    SWIP_ID_CODEC_HEAAC_ENCODER,
    SWIP_ID_CODEC_SBC_ENCODER,
    SWIP_ID_PROCESSING_SMARTPA,
    SWIP_ID_PROCESSING_KTV,
    SWIP_ID_PROCESSING_BESLOUDNESS,
    SWIP_ID_PROCESSING_DC_REMOVAL,
    SWIP_ID_PROCESSING_SRC,
    SWIP_ID_PROCESSING_SHIFTER,
    SWIP_ID_PROCESSING_LIMITER,
    SWIP_ID_PROCESSING_DIRAC,
    SWIP_ID_EFFECT_AEC,
    SWIP_ID_EFFECT_RECORD,
    SWIP_ID_VOIP_DL_ENH,    /* merge DL/UL to one, use config to seperate */
    SWIP_ID_VOIP_UL_ENH,
    SWIP_ID_PHONE_CALL_ENH,
    SWIP_ID_AURISYS_DEMO,     /* for dummy lib */
    SWIP_ID_SYSTEM_TEST,
    ADSP_NUM_SWIP_ID,
} adsp_swip_id;

struct swip_node {
    adsp_swip_id    id;
    uint32_t        enable;
    uint32_t        mcps;
};

struct feature_node {
    const char            *name;
    uint32_t            enable;
    uint32_t            total_mcps;
    uint32_t            feature_mcps;
    struct swip_node    swip_list[FEATURE_SWIP_MAX_NUM];
};

#define FEATURE_UNIT(fname, ffreq) {.name = fname, .feature_mcps = ffreq}
#define SYSTEM_UNIT(fname, ffreq) {.name = fname, .feature_mcps = ffreq, \
                                   .enable = true}

extern struct feature_node adsp_feature_table[ADSP_NUM_FEATURE_ID];
extern uint32_t (*swip_mcps_parser[ADSP_NUM_SWIP_ID])(const dvfs_swip_config_t *p_swip_config);


void feature_manager_init(void);
uint32_t calculate_total_mcps(void);
int32_t reset_feature_table(void);
uint32_t update_system_loading(uint32_t mcps);
/* public API */
void dvfs_register_swip_mcps_parser(
        adsp_swip_id swip_id,
        void *func);

int32_t dvfs_register_feature(
        adsp_feature_id feature_id);

int32_t dvfs_deregister_feature(
        adsp_feature_id feature_id);

uint32_t dvfs_update_swip_mcps_info(
        bool enable,
        adsp_feature_id feature_id,
        adsp_swip_id swip_id,
        const dvfs_swip_config_t *p_swip_config);

#define dvfs_add_swip(feature, swip_id, p_swip_config) \
    dvfs_update_swip_mcps_info((true), (feature), (swip_id), (p_swip_config))
#define dvfs_delete_swip(feature, swip_id, p_swip_config) \
    dvfs_update_swip_mcps_info((false), (feature), (swip_id), (p_swip_config))

int32_t add_swip_node_into_feature(struct feature_node *fnode,
                                            adsp_swip_id swip_id,
                                            int mcps);
int32_t delete_swip_node_from_feature(struct feature_node *fnode,
                                            adsp_swip_id swip_id,
                                            int mcps);

#ifdef PARSER_DEMO
void test_case_add_swip(void);
#endif

#endif /*_FEATURE_MANAGER_*/

