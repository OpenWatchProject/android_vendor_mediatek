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

#include <FreeRTOS.h>
#include <task.h>
#include <feature_manager.h>
#include <mt_reg_base.h>
#include <driver_api.h>
#include "dvfs_common.h"
#include <adsp_ipi.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif


#include <dvfs_config_parser.h>

/*****************************************************
 * New Implement
 ****************************************************/
//static void feature_register_handler(int id, void *data, unsigned int len);

/* @return: mcps value */
uint32_t (*swip_mcps_parser[ADSP_NUM_SWIP_ID])(const dvfs_swip_config_t *p_swip_config);

struct feature_node feature_table[ADSP_NUM_FEATURE_ID] = {
    [SYSTEM_FEATURE_ID]           = SYSTEM_UNIT("system", 0),
    [ADSP_LOGGER_FEATURE_ID]      = FEATURE_UNIT("logger", 0),
    [FORMATTER_FEATURE_ID]        = SYSTEM_UNIT("formatter", 0),
    [PRIMARY_FEATURE_ID]          = FEATURE_UNIT("primary", 0),
    [DEEPBUF_FEATURE_ID]          = FEATURE_UNIT("deepbuf", 0),
    [OFFLOAD_FEATURE_ID]          = FEATURE_UNIT("offload", 10),
    [AUDIO_PLAYBACK_FEATURE_ID]   = FEATURE_UNIT("audplayback", 0),
    [A2DP_PLAYBACK_FEATURE_ID]    = FEATURE_UNIT("a2dp_playback", 0),
    [AUDIO_DATAPROVIDER_FEATURE_ID]   = FEATURE_UNIT("dataprovider", 0),
    [SPK_PROTECT_FEATURE_ID]      = FEATURE_UNIT("spk_protect", 0),
    [VOICE_CALL_FEATURE_ID]       = FEATURE_UNIT("voice_call", 0),
    [VOIP_FEATURE_ID]             = FEATURE_UNIT("voip", 0),
    [CAPTURE_UL1_FEATURE_ID]      = FEATURE_UNIT("capture_ul1", 0),
    [CALL_FINAL_FEATURE_ID]       = FEATURE_UNIT("call_final", 0),
    [KTV_FEATURE_ID]             = FEATURE_UNIT("ktv", 0),
};

#ifdef PARSER_DEMO

/* test function */
void test_case_add_swip(void)
{
    dvfs_swip_config_t phone_call_config;
    dvfs_swip_config_t phone_call_config_1;

    dvfs_register_swip_mcps_parser(SWIP_ID_PHONE_CALL_ENH,
                                   (void *)phone_call_enh_example);

    phone_call_config.output_sample_rate = 8000;
    phone_call_config.output_devices = AUDIO_DEVICE_OUT_EARPIECE;
    phone_call_config_1.output_sample_rate = 32000;
    phone_call_config_1.output_devices = AUDIO_DEVICE_OUT_SPEAKER;

    feature_table[VOICE_CALL_FEATURE_ID].enable = true;
    dvfs_add_swip(VOICE_CALL_FEATURE_ID, SWIP_ID_PHONE_CALL_ENH, &phone_call_config);
    dvfs_add_swip(VOICE_CALL_FEATURE_ID, SWIP_ID_PHONE_CALL_ENH, &phone_call_config_1);

    dvfs_delete_swip(VOICE_CALL_FEATURE_ID, SWIP_ID_PHONE_CALL_ENH, &phone_call_config);
    dvfs_add_swip(VOICE_CALL_FEATURE_ID, SWIP_ID_PHONE_CALL_ENH, &phone_call_config_1);

    dvfs_delete_swip(VOICE_CALL_FEATURE_ID, SWIP_ID_PHONE_CALL_ENH, &phone_call_config_1);
    dvfs_delete_swip(VOICE_CALL_FEATURE_ID, SWIP_ID_PHONE_CALL_ENH, &phone_call_config_1);

}
#endif

void feature_manager_init(void)
{
    reset_feature_table();
//    adsp_ipi_registration(IPI_DVFS_REGISTER, (ipi_handler_t)feature_register_handler, "feature register");
}

uint32_t calculate_total_mcps(void)
{
    uint32_t fid = 0, sid = 0;
    uint32_t feature_swip_mcps;
    uint32_t system_total_mcps = 0;

    for (fid = 0; fid < ADSP_NUM_FEATURE_ID; fid ++) {
        if (feature_table[fid].enable != true)
            continue;
        feature_swip_mcps = 0;
        DVFS_LOG_D("Feature #%d\n", fid);
        for (sid = 0; sid < FEATURE_SWIP_MAX_NUM; sid ++) {
            if (feature_table[fid].swip_list[sid].enable != false) {
                DVFS_LOG_D("   swip_id=%d, mcps=%d\n",
                         feature_table[fid].swip_list[sid].id,
                         feature_table[fid].swip_list[sid].mcps);
                feature_swip_mcps += feature_table[fid].swip_list[sid].mcps;
            }
        }
        feature_table[fid].total_mcps = feature_table[fid].feature_mcps +
                                        feature_swip_mcps;
        DVFS_LOG_D("=> mcps sum=%3d driver=%3d, ip=%3d\n",
                   feature_table[fid].total_mcps,
                   feature_table[fid].feature_mcps, feature_swip_mcps);
        system_total_mcps += feature_table[fid].total_mcps;
    }

    /* debug log */
/*
    for (fid = 0; fid < ADSP_NUM_FEATURE_ID; fid ++) {
        DVFS_LOG_D("fid=%d, mcps=%d\n", fid, feature_table[fid].total_mcps);
    }
*/
    DVFS_LOG_D("system total mcps= %d\n", system_total_mcps);

    return system_total_mcps;
}

int32_t reset_swip_list(struct feature_node *fnode)
{
    uint32_t i = 0;

    for (i = 0; i < FEATURE_SWIP_MAX_NUM; i++) {
        fnode->swip_list[i].enable = false;
        fnode->swip_list[i].id = 0;
        fnode->swip_list[i].mcps = 0;
    }
    DVFS_LOG_V("%s done\n", __FUNCTION__);
    return 0;
}

int32_t reset_feature_table(void)
{
    uint32_t id = 0;

    for (id = 0; id < ADSP_NUM_FEATURE_ID; id++) {
        if (feature_table[id].name != NULL) {
            feature_table[id].enable = false;
            feature_table[id].total_mcps = 0;
            feature_table[id].feature_mcps = 0;
            reset_swip_list(&feature_table[id]);
        }
        if (id == SYSTEM_FEATURE_ID || id == FORMATTER_FEATURE_ID) {
            feature_table[id].enable = true;
        }
    }
    DVFS_LOG_V("%s done\n", __FUNCTION__);
    return 0;
}

uint32_t update_system_loading(uint32_t mcps)
{
    feature_table[SYSTEM_FEATURE_ID].feature_mcps = mcps;
    return 0;
}

int32_t add_swip_node_into_feature(struct feature_node *fnode,
                                            adsp_swip_id swip_id,
                                            int mcps)
{
    uint32_t i = 0;

    for (i = 0; i < FEATURE_SWIP_MAX_NUM; i++) {
        if (fnode->swip_list[i].enable == false) {
            fnode->swip_list[i].enable = true;
            fnode->swip_list[i].id = swip_id;
            fnode->swip_list[i].mcps = mcps;
            return 0;
        }
    }
    DVFS_LOG_E("%s swip array is full\n", __FUNCTION__);
    return -1;
}

int32_t delete_swip_node_from_feature(struct feature_node *fnode,
                                            adsp_swip_id swip_id,
                                            int mcps)
{
    uint32_t i = 0;

    for (i = 0; i < FEATURE_SWIP_MAX_NUM; i++) {
        if (fnode->swip_list[i].id == swip_id && fnode->swip_list[i].mcps == mcps) {
            fnode->swip_list[i].enable = false;
            fnode->swip_list[i].id = 0;
            fnode->swip_list[i].mcps = 0;
            return 0;
        }
    }
    DVFS_LOG_E("%s() cannot find swip node. feature=%s, sid=%d, mcps=%d\n",
                __FUNCTION__, fnode->name, swip_id, mcps);
    return -1;
}

uint32_t dvfs_update_swip_mcps_info(
        bool enable,
        adsp_feature_id feature_id,
        adsp_swip_id swip_id,
        const dvfs_swip_config_t *p_swip_config)
{
    struct feature_node *p_feature_node = &feature_table[feature_id];
    int mcps = 0;
    int ret = 0;

    if (swip_mcps_parser[swip_id] == NULL) {
        DVFS_LOG_E("%s(), swip_mcps_parser not registered !! feature_id = %d, swip_id = %d\n",
                 __FUNCTION__, feature_id, swip_id);
        return 0;
    }

    mcps = swip_mcps_parser[swip_id](p_swip_config);
    if (enable) {
        ret = add_swip_node_into_feature(p_feature_node, swip_id, mcps);
    } else {
        ret = delete_swip_node_from_feature(p_feature_node, swip_id, mcps);
    }
    if (ret != 0) {
        DVFS_LOG_E("%s(-1), fid=%d, sid=%d, mcps=%d\n",
                __FUNCTION__, feature_id, swip_id, mcps);
    }

    DVFS_LOG_D("%s(), enable=%d, fid=%d, sid=%d, mcps=%d\n",
            __FUNCTION__, enable, feature_id, swip_id, mcps);

    update_dvfs_freq();

    return 0;
}


/* init parser */
void dvfs_register_swip_mcps_parser(adsp_swip_id swip_id, void *func)
{
    swip_mcps_parser[swip_id] = func;
}

/* api */
int32_t dvfs_register_feature(adsp_feature_id feature_id)
{
    uint32_t sid = 0;

    DVFS_LOG_D("%s() feature id=%s(%d)\n", __FUNCTION__,
               feature_table[feature_id].name, feature_id);
    if (feature_id >= ADSP_NUM_FEATURE_ID) {
        DVFS_LOG_W("%s(), invalid feature id=%d\n", __FUNCTION__, feature_id);
        return -1;
    }
    if (feature_table[feature_id].enable == true) {
        DVFS_LOG_W("%s(), feature is registered !! feature=%s(%d)\n",
                 __FUNCTION__, feature_table[feature_id].name, feature_id);
        return -1;
    }

    feature_table[feature_id].enable = true;
    for (sid = 0; sid < FEATURE_SWIP_MAX_NUM; sid ++) {
        if (feature_table[feature_id].swip_list[sid].enable != false) {
            DVFS_LOG_D("register after than IP added\n");
            update_dvfs_freq();
            return 0;
        }
    }

    return 0;
}

int32_t dvfs_deregister_feature(adsp_feature_id feature_id)
{
    DVFS_LOG_D("%s()feature id=%s(%d)\n", __FUNCTION__,
               feature_table[feature_id].name, feature_id);
    if (feature_id >= ADSP_NUM_FEATURE_ID) {
        DVFS_LOG_W("%s(), invalid feature id=%d\n", __FUNCTION__, feature_id);
        return -1;
    }

    if (feature_table[feature_id].enable == false) {
        DVFS_LOG_W("%s(), feature not registered !! feature=%s(%d)\n",
                 __FUNCTION__, feature_table[feature_id].name, feature_id);
        return -1;
    }

    feature_table[feature_id].enable = false;
    update_dvfs_freq();

    return 0;
}

static const char* swip_name[ADSP_NUM_SWIP_ID] =
{
    "CODEC_HEAAC_DECODER", "CODEC_MP3_DECODER", "CODEC_HEAAC_ENCODER", "CODEC_SBC_ENCODER",
    "PROCESSING_BESLOUDNES", "PROCESSING_DC_REMOVAL", "PROCESSING_SRC", "PROCESSING_SHIFTER",
    "PROCESSING_LIMITER", "EFFECT_AEC", "EFFECT_RECORD", "VOIP_DL_ENH",
    "VOIP_UL_ENH", "PHONE_CALL_ENH", "AURISYS_DEMO", "SYSTEM_TEST",
};

uint32_t dump_feature_status(char *buf, int size)
{
    uint32_t fid = 0, sid = 0, n = 0;
    uint32_t system_total_mcps = 0;
    struct feature_node *f_node = NULL;

    n += snprintf(buf + n, size - n, "  feature/swip       enable    mcps\n");
    n += snprintf(buf + n, size - n, "-----------------------------------\n");

    for (fid = 0; fid < ADSP_NUM_FEATURE_ID; fid ++) {
        f_node = &feature_table[fid];
        if (f_node->name == NULL)
            continue;

        n += snprintf(buf + n, size - n, "%-20s %6u %7u\n",
                      f_node->name, f_node->enable, f_node->feature_mcps);

        for (sid = 0; sid < FEATURE_SWIP_MAX_NUM; sid ++) {
            if (f_node->swip_list[sid].enable == false)
                continue;

            n += snprintf(buf + n, size - n, "   %-17s %6u %7u\n",
                          swip_name[f_node->swip_list[sid].id],
                          f_node->swip_list[sid].enable,
                          f_node->swip_list[sid].mcps);
        }

        if (f_node->enable == true)
            system_total_mcps += f_node->total_mcps;
    }

    n += snprintf(buf + n, size - n, "--total_mcps--              %7u\n\n", system_total_mcps);
    return n;
}

