#include <aurisys_config_parser.h>

#include <uthash.h>

#include <audio_log_hal.h>
#include <audio_memory_control.h>

#include <aurisys_scenario_dsp.h>

#include <arsi_type.h>
#include <aurisys_config.h>

#include <arsi_api.h>

#include <arsi_library_entry_points.h>



#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "[AURI][PARSE]"


#ifdef AURISYS_DUMP_LOG_V
#ifdef AUD_LOG_V
#undef AUD_LOG_V
#endif
#define AUD_LOG_V(x...) AUD_LOG_D(x)
#endif



/*
 * =============================================================================
 *                     utilities declaration
 * =============================================================================
 */

static void add_library_config(aurisys_library_config_t **hash_head, aurisys_library_config_t *new_library_config) {
    HASH_ADD_KEYPTR(
        hh,
        *hash_head,
        new_library_config->name,
        strlen(new_library_config->name),
        new_library_config);
}


static void add_arsi_component(aurisys_component_t **hash_head, aurisys_component_t *new_component) {
    HASH_ADD_INT(
        *hash_head,
        aurisys_scenario,
        new_component);
}


static void add_scene_lib_table(aurisys_scene_lib_table_t **hash_head, aurisys_scene_lib_table_t *new_scene_lib_table) {
    HASH_ADD_INT(
        *hash_head,
        aurisys_scenario,
        new_scene_lib_table);
}


static void add_library_name(aurisys_library_name_t **hash_head, aurisys_library_name_t *new_library_name) {
    HASH_ADD_KEYPTR(
        hh,
        *hash_head,
        new_library_name->name,
        strlen(new_library_name->name),
        new_library_name);
}


static void dump_scene_lib_table(aurisys_scene_lib_table_t *scene_lib_table) {
    aurisys_scene_lib_table_t *itor_scene_lib_table = NULL;
    aurisys_scene_lib_table_t *tmp_scene_lib_table = NULL;

    aurisys_library_name_t *itor_library_name = NULL;
    aurisys_library_name_t *tmp_library_name = NULL;


    HASH_ITER(hh, scene_lib_table, itor_scene_lib_table, tmp_scene_lib_table) {
        AUD_LOG_D("aurisys_scenario %d", itor_scene_lib_table->aurisys_scenario);

        if (itor_scene_lib_table->uplink_library_name_list) {
            HASH_ITER(hh, itor_scene_lib_table->uplink_library_name_list, itor_library_name, tmp_library_name) {
                AUD_LOG_D("uplink_library_name_list name %s", itor_library_name->name);
            }
            if (strlen(itor_scene_lib_table->uplink_digital_gain_lib_name) > 0) {
                AUD_LOG_D("=> UL apply gain to %s", itor_scene_lib_table->uplink_digital_gain_lib_name);
            } else {
                AUD_LOG_D("=> UL have not apply gain to library");
            }
        }
        if (itor_scene_lib_table->downlink_library_name_list) {
            HASH_ITER(hh, itor_scene_lib_table->downlink_library_name_list, itor_library_name, tmp_library_name) {
                AUD_LOG_D("downlink_library_name_list name %s", itor_library_name->name);
            }
            if (strlen(itor_scene_lib_table->downlink_digital_gain_lib_name) > 0) {
                AUD_LOG_D("=> DL apply gain to %s", itor_scene_lib_table->downlink_digital_gain_lib_name);
            } else {
                AUD_LOG_D("=> DL have not apply gain to library");
            }
        }
        if (itor_scene_lib_table->all_library_name_list) {
            HASH_ITER(hh, itor_scene_lib_table->all_library_name_list, itor_library_name, tmp_library_name) {
                AUD_LOG_D("all_library_name_list name %s", itor_library_name->name);
            }
        }
    }
}


static void dump_library_config(aurisys_library_config_t *library_config) {
    aurisys_library_config_t *itor_lib = NULL;
    aurisys_library_config_t *tmp_lib = NULL;

    aurisys_component_t *itor_comp = NULL;
    aurisys_component_t *tmp_comp = NULL;

    uint8_t data_buf_type = 0;

    string_buf_t lib_version;

    int i = 0;


    HASH_ITER(hh, library_config, itor_lib, tmp_lib) {
        AUD_LOG_D("library name %s", itor_lib->name);
        AUD_LOG_V("library lib_path \"%s\"", itor_lib->lib_path);
        AUD_LOG_V("library lib64_path \"%s\"", itor_lib->lib64_path);
        AUD_LOG_V("library param_path \"%s\"", itor_lib->param_path);
        AUD_LOG_V("library lib_dump_path \"%s\"", itor_lib->lib_dump_path);
        AUD_LOG_V("library adb_cmd_key %s", itor_lib->adb_cmd_key);

        if (itor_lib->api &&
            itor_lib->api->arsi_get_lib_version != NULL) {
            lib_version.memory_size = 128;
            lib_version.string_size = 0;
            lib_version.p_string = NULL;
            AUDIO_ALLOC_CHAR_BUFFER(lib_version.p_string, lib_version.memory_size);
            itor_lib->api->arsi_get_lib_version(&lib_version);
            AUD_LOG_D("lib_version: \"%s\"", lib_version.p_string);
            AUDIO_FREE_POINTER(lib_version.p_string);
        } else {
            AUD_LOG_E("unknown lib_version");
        }

        HASH_ITER(hh, itor_lib->component_hh, itor_comp, tmp_comp) {
            AUD_LOG_D("aurisys_scenario %d", itor_comp->aurisys_scenario);
            AUD_LOG_V("sample_rate_mask 0x%x", itor_comp->sample_rate_mask);
            AUD_LOG_V("max sample_rate %d", itor_comp->lib_config.sample_rate);
            AUD_LOG_V("audio_format %d", itor_comp->lib_config.audio_format);
            AUD_LOG_V("support_format_mask 0x%x", itor_comp->support_format_mask);
            AUD_LOG_V("support_frame_ms_mask 0x%x", itor_comp->support_frame_ms_mask);
            AUD_LOG_V("frame_size_ms %d", itor_comp->lib_config.frame_size_ms);
            AUD_LOG_V("b_interleave %d", itor_comp->lib_config.b_interleave);
            AUD_LOG_V("enable_log %d", itor_comp->enable_log);
            AUD_LOG_V("enable_raw_dump %d", itor_comp->enable_raw_dump);
            AUD_LOG_V("enable_lib_dump %d", itor_comp->enable_lib_dump);
            AUD_LOG_V("enhancement_mode %d", itor_comp->enhancement_mode);
            AUD_LOG_V("num_ul_ref_buf_array %d", itor_comp->lib_config.num_ul_ref_buf_array);
            AUD_LOG_V("num_dl_ref_buf_array %d", itor_comp->lib_config.num_dl_ref_buf_array);


            /* UL */
            if (itor_comp->lib_config.p_ul_buf_in != NULL) {
                AUD_LOG_V("UL buf_in type %d", itor_comp->lib_config.p_ul_buf_in->data_buf_type);
                AUD_LOG_V("UL buf_in ch   %d", itor_comp->lib_config.p_ul_buf_in->num_channels);
                data_buf_type = itor_comp->lib_config.p_ul_buf_in->data_buf_type;
                if (data_buf_type < NUM_DATA_BUF_TYPE) {
                    AUD_LOG_V("UL buf_in ch_mask 0x%x",
                              itor_comp->support_channel_number_mask[data_buf_type]);
                }
            }

            if (itor_comp->lib_config.p_ul_buf_out != NULL) {
                AUD_LOG_V("UL buf_out type %d", itor_comp->lib_config.p_ul_buf_out->data_buf_type);
                AUD_LOG_V("UL buf_out ch   %d", itor_comp->lib_config.p_ul_buf_out->num_channels);
                data_buf_type = itor_comp->lib_config.p_ul_buf_out->data_buf_type;
                if (data_buf_type < NUM_DATA_BUF_TYPE) {
                    AUD_LOG_V("UL buf_out ch_mask 0x%x",
                              itor_comp->support_channel_number_mask[data_buf_type]);
                }
            }

            if (itor_comp->lib_config.p_ul_ref_bufs != NULL) {
                for (i = 0; i < itor_comp->lib_config.num_ul_ref_buf_array; i++) {
                    AUD_LOG_V("UL buf_ref[%d] type %d", i, itor_comp->lib_config.p_ul_ref_bufs[i].data_buf_type);
                    AUD_LOG_V("UL buf_ref[%d] ch   %d", i, itor_comp->lib_config.p_ul_ref_bufs[i].num_channels);
                    data_buf_type = itor_comp->lib_config.p_ul_ref_bufs[i].data_buf_type;
                    if (data_buf_type < NUM_DATA_BUF_TYPE) {
                        AUD_LOG_V("UL buf_ref[%d] ch_mask 0x%x",
                                  i, itor_comp->support_channel_number_mask[data_buf_type]);
                    }
                }
            }

            /* DL */
            if (itor_comp->lib_config.p_dl_buf_in != NULL) {
                AUD_LOG_V("DL buf_in type %d", itor_comp->lib_config.p_dl_buf_in->data_buf_type);
                AUD_LOG_V("DL buf_in ch   %d", itor_comp->lib_config.p_dl_buf_in->num_channels);
                data_buf_type = itor_comp->lib_config.p_dl_buf_in->data_buf_type;
                if (data_buf_type < NUM_DATA_BUF_TYPE) {
                    AUD_LOG_V("DL buf_in ch_mask 0x%x",
                              itor_comp->support_channel_number_mask[data_buf_type]);
                }
            }

            if (itor_comp->lib_config.p_dl_buf_out != NULL) {
                AUD_LOG_V("DL buf_out type %d", itor_comp->lib_config.p_dl_buf_out->data_buf_type);
                AUD_LOG_V("DL buf_out ch   %d", itor_comp->lib_config.p_dl_buf_out->num_channels);
                data_buf_type = itor_comp->lib_config.p_dl_buf_out->data_buf_type;
                if (data_buf_type < NUM_DATA_BUF_TYPE) {
                    AUD_LOG_V("DL buf_out ch_mask 0x%x",
                              itor_comp->support_channel_number_mask[data_buf_type]);
                }
            }

            if (itor_comp->lib_config.p_dl_ref_bufs != NULL) {
                for (i = 0; i < itor_comp->lib_config.num_dl_ref_buf_array; i++) {
                    AUD_LOG_V("DL buf_ref[%d] type %d", i, itor_comp->lib_config.p_dl_ref_bufs[i].data_buf_type);
                    AUD_LOG_V("DL buf_ref[%d] ch   %d", i, itor_comp->lib_config.p_dl_ref_bufs[i].num_channels);
                    data_buf_type = itor_comp->lib_config.p_dl_ref_bufs[i].data_buf_type;
                    if (data_buf_type < NUM_DATA_BUF_TYPE) {
                        AUD_LOG_V("DL buf_ref[%d] ch_mask 0x%x",
                                  i, itor_comp->support_channel_number_mask[data_buf_type]);
                    }
                }
            }
        }
    }
}



/*
 * =============================================================================
 *                     public function implementation
 * =============================================================================
 */

static aurisys_library_name_t *merge_all_library_name_list(
    aurisys_library_name_t *uplink_library_name_list,
    aurisys_library_name_t *downlink_library_name_list) {

    aurisys_library_name_t *all_library_name_list = NULL;

    aurisys_library_name_t *new_library_name = NULL;
    aurisys_library_name_t *itor_library_name = NULL;
    aurisys_library_name_t *tmp_library_name = NULL;
    aurisys_library_name_t *the_library_name = NULL;

    if (uplink_library_name_list) {
        HASH_ITER(hh, uplink_library_name_list, itor_library_name, tmp_library_name) {
            AUD_LOG_V("merge UL name %s", itor_library_name->name);

            AUDIO_ALLOC_STRUCT(aurisys_library_name_t, new_library_name);

            /* lib name will not change => use strdup */
            new_library_name->name = strdup_safe(itor_library_name->name, MAX_LIB_NAME_SIZE);

            add_library_name(&all_library_name_list, new_library_name);
            new_library_name = NULL;
        }
    }
    if (downlink_library_name_list) {
        HASH_ITER(hh, downlink_library_name_list, itor_library_name, tmp_library_name) {
            AUD_LOG_V("merge DL name %s", itor_library_name->name);

            HASH_FIND_STR(all_library_name_list, itor_library_name->name, the_library_name);
            if (the_library_name != NULL) {
                AUD_LOG_V("already added DL lib name %s in UL", the_library_name->name);
                continue;
            }

            AUDIO_ALLOC_STRUCT(aurisys_library_name_t, new_library_name);

            /* lib name will not change => use strdup */
            new_library_name->name = strdup_safe(itor_library_name->name, MAX_LIB_NAME_SIZE);

            add_library_name(&all_library_name_list, new_library_name);
            new_library_name = NULL;
        }
    }

    return all_library_name_list;
}


static void trans_scene_lib_table_dsp_to_hal(
    struct aurisys_scene_lib_table_dsp_t *dsp,
    aurisys_scene_lib_table_t **hal) {

    struct aurisys_scene_lib_table_dsp_t *scene_lib_table_dsp = NULL;

    aurisys_scene_lib_table_t *new_scene_lib_table = NULL;
    aurisys_library_name_t *new_library_name = NULL;

    char *lib_name;


    uint32_t idx_scene = 0;

    uint32_t num_ul_lib = 0;
    uint32_t num_dl_lib = 0;

    uint32_t idx_lib = 0;

    if (!dsp || !hal) {
        ALOGW("%s(), %p %p fail!!", __FUNCTION__, dsp, hal);
        return;
    }

    for (idx_scene = 0; idx_scene < AURISYS_SCENARIO_DSP_SIZE; idx_scene++) {
        scene_lib_table_dsp = &dsp[idx_scene];
        num_ul_lib = scene_lib_table_dsp->num_uplink_library_name_list;
        num_dl_lib = scene_lib_table_dsp->num_downlink_library_name_list;

        if (num_ul_lib == 0 && num_dl_lib == 0) {
            continue;
        }


        AUDIO_ALLOC_STRUCT(aurisys_scene_lib_table_t, new_scene_lib_table);
        new_scene_lib_table->aurisys_scenario = idx_scene;

        /* UL */
        for (idx_lib = 0; idx_lib < num_ul_lib; idx_lib++) {
            lib_name = scene_lib_table_dsp->uplink_library_name_list[idx_lib];

            AUDIO_ALLOC_STRUCT(aurisys_library_name_t, new_library_name);
            new_library_name->name = strdup_safe(lib_name, MAX_LIB_NAME_SIZE);

            add_library_name(&new_scene_lib_table->uplink_library_name_list,
                             new_library_name);
            new_library_name = NULL;
        }

        /* DL */
        for (idx_lib = 0; idx_lib < num_dl_lib; idx_lib++) {
            lib_name = scene_lib_table_dsp->downlink_library_name_list[idx_lib];

            AUDIO_ALLOC_STRUCT(aurisys_library_name_t, new_library_name);
            new_library_name->name = strdup_safe(lib_name, MAX_LIB_NAME_SIZE);

            add_library_name(&new_scene_lib_table->downlink_library_name_list,
                             new_library_name);
            new_library_name = NULL;
        }

        /* ALL */
        new_scene_lib_table->all_library_name_list =
            merge_all_library_name_list(
                new_scene_lib_table->uplink_library_name_list,
                new_scene_lib_table->downlink_library_name_list);
        AUD_ASSERT(new_scene_lib_table->all_library_name_list != NULL);


        if (num_ul_lib != 0) {
            new_scene_lib_table->uplink_digital_gain_lib_name =
                strdup_safe(scene_lib_table_dsp->uplink_digital_gain_lib_name, MAX_LIB_NAME_SIZE);
        }

        if (num_dl_lib != 0) {
            new_scene_lib_table->downlink_digital_gain_lib_name =
                strdup_safe(scene_lib_table_dsp->downlink_digital_gain_lib_name, MAX_LIB_NAME_SIZE);
        }

        add_scene_lib_table(hal, new_scene_lib_table);
        new_scene_lib_table = NULL;
    }

    dump_scene_lib_table(*hal);
}


static void trans_library_config_dsp_to_hal(
    struct aurisys_library_config_dsp_t *dsp,
    aurisys_library_config_t **hal) {

    aurisys_library_config_t *new_library_config = NULL;
    aurisys_component_t *new_component = NULL;
    arsi_lib_config_t *lib_config = NULL;
    audio_buf_t *buf_in = NULL;
    audio_buf_t *buf_out = NULL;
    audio_buf_t *buf_refs = NULL;

    struct aurisys_library_config_dsp_t *lib_cfg_dsp = NULL;
    struct aurisys_component_dsp_t *comp_dsp = NULL;
    struct arsi_lib_config_dsp_t *lib_config_dsp = NULL;

    uint32_t idx_lib = 0;
    uint32_t idx_comp = 0;
    uint32_t idx_ref = 0;


    if (!dsp || !hal) {
        ALOGW("%s(), %p %p fail!!", __FUNCTION__, dsp, hal);
        return;
    }

    for (idx_lib = 0; idx_lib < MAX_TOTAL_LIB_NUM; idx_lib++) {
        lib_cfg_dsp = &dsp[idx_lib];
        if (strlen(lib_cfg_dsp->name) == 0) {
            break;
        }

        AUDIO_ALLOC_STRUCT(aurisys_library_config_t, new_library_config);

        new_library_config->name          = strdup_safe(lib_cfg_dsp->name, MAX_LIB_NAME_SIZE);
        new_library_config->lib_path      = strdup_safe(lib_cfg_dsp->lib_path, MAX_LIB_PATH_SIZE);
        new_library_config->lib64_path    = strdup_safe(lib_cfg_dsp->lib64_path, MAX_LIB_PATH_SIZE);
        new_library_config->param_path    = strdup_safe(lib_cfg_dsp->param_path, MAX_PARAM_PATH_SIZE);
        new_library_config->lib_dump_path = strdup_safe(lib_cfg_dsp->lib_dump_path, MAX_DUMP_FILE_PATH_SIZE);
        new_library_config->adb_cmd_key   = strdup_safe(lib_cfg_dsp->adb_cmd_key, MAX_ADB_CMD_KEY_SIZE);

        AUDIO_ALLOC_STRUCT(struct AurisysLibInterface, new_library_config->api);
        AURISYS_LINK_LIB_NAME_TO_API(new_library_config->name,
                                     new_library_config->api);

        for (idx_comp = 0; idx_comp < AURISYS_SCENARIO_DSP_SIZE; idx_comp++) {
            comp_dsp = &lib_cfg_dsp->component[idx_comp];
            if (comp_dsp->sample_rate_mask == 0) {
                break;
            }

            AUDIO_ALLOC_STRUCT(aurisys_component_t, new_component);
            lib_config = &new_component->lib_config;
            lib_config_dsp = &comp_dsp->lib_config;

            new_component->aurisys_scenario = comp_dsp->aurisys_scenario;
            new_component->sample_rate_mask = comp_dsp->sample_rate_mask;
            new_component->support_format_mask = comp_dsp->support_format_mask;
            new_component->support_frame_ms_mask = comp_dsp->support_frame_ms_mask;

            new_component->enable_log = comp_dsp->enable_log;
            new_component->enable_raw_dump = comp_dsp->enable_raw_dump;
            new_component->enable_lib_dump = comp_dsp->enable_lib_dump;
            new_component->enhancement_mode = comp_dsp->enhancement_mode;


            lib_config->sample_rate = lib_config_dsp->sample_rate;
            lib_config->audio_format = lib_config_dsp->audio_format;
            lib_config->frame_size_ms = lib_config_dsp->frame_size_ms;
            lib_config->b_interleave = lib_config_dsp->b_interleave;
            lib_config->num_ul_ref_buf_array = lib_config_dsp->num_ul_ref_buf_array;
            lib_config->num_dl_ref_buf_array = lib_config_dsp->num_dl_ref_buf_array;

            memcpy(new_component->support_channel_number_mask,
                   comp_dsp->support_channel_number_mask,
                   sizeof(new_component->support_channel_number_mask));

            /* UL */
            if (lib_config_dsp->p_ul_buf_in.num_channels != 0 &&
                lib_config_dsp->p_ul_buf_out.num_channels != 0) {
                AUDIO_ALLOC_STRUCT(audio_buf_t, buf_in);
                buf_in->data_buf_type = lib_config_dsp->p_ul_buf_in.data_buf_type;
                buf_in->num_channels = lib_config_dsp->p_ul_buf_in.num_channels;
                lib_config->p_ul_buf_in = buf_in;
                buf_in = NULL;

                AUDIO_ALLOC_STRUCT(audio_buf_t, buf_out);
                buf_out->data_buf_type = lib_config_dsp->p_ul_buf_out.data_buf_type;
                buf_out->num_channels = lib_config_dsp->p_ul_buf_out.num_channels;
                lib_config->p_ul_buf_out = buf_out;
                buf_out = NULL;

                if (lib_config->num_ul_ref_buf_array > 0) {
                    AUDIO_ALLOC_STRUCT_ARRAY(audio_buf_t, lib_config->num_ul_ref_buf_array, buf_refs);
                    for (idx_ref = 0; idx_ref < lib_config->num_ul_ref_buf_array; idx_ref++) {
                        buf_refs[idx_ref].data_buf_type = lib_config_dsp->p_ul_ref_bufs[idx_ref].data_buf_type;
                        buf_refs[idx_ref].num_channels = lib_config_dsp->p_ul_ref_bufs[idx_ref].num_channels;
                    }
                    lib_config->p_ul_ref_bufs = buf_refs;
                    buf_refs = NULL;
                }
            }

            /* dl */
            if (lib_config_dsp->p_dl_buf_in.num_channels != 0 &&
                lib_config_dsp->p_dl_buf_out.num_channels != 0) {
                AUDIO_ALLOC_STRUCT(audio_buf_t, buf_in);
                buf_in->data_buf_type = lib_config_dsp->p_dl_buf_in.data_buf_type;
                buf_in->num_channels = lib_config_dsp->p_dl_buf_in.num_channels;
                lib_config->p_dl_buf_in = buf_in;
                buf_in = NULL;

                AUDIO_ALLOC_STRUCT(audio_buf_t, buf_out);
                buf_out->data_buf_type = lib_config_dsp->p_dl_buf_out.data_buf_type;
                buf_out->num_channels = lib_config_dsp->p_dl_buf_out.num_channels;
                lib_config->p_dl_buf_out = buf_out;
                buf_out = NULL;

                if (lib_config->num_dl_ref_buf_array > 0) {
                    AUDIO_ALLOC_STRUCT_ARRAY(audio_buf_t, lib_config->num_dl_ref_buf_array, buf_refs);
                    for (idx_ref = 0; idx_ref < lib_config->num_dl_ref_buf_array; idx_ref++) {
                        buf_refs[idx_ref].data_buf_type = lib_config_dsp->p_dl_ref_bufs[idx_ref].data_buf_type;
                        buf_refs[idx_ref].num_channels = lib_config_dsp->p_dl_ref_bufs[idx_ref].num_channels;
                    }
                    lib_config->p_dl_ref_bufs = buf_refs;
                    buf_refs = NULL;
                }
            }
            add_arsi_component(&new_library_config->component_hh, new_component);
            new_component = NULL;
        }
        add_library_config(hal, new_library_config);
        new_library_config = NULL;
    }

    dump_library_config(*hal);
}


void trans_aurisys_config_dsp_to_hal(
    struct aurisys_config_dsp_t *dsp,
    struct aurisys_config_t **hal) {
    struct aurisys_config_t *new_cfg = NULL;

    if (!dsp || !hal) {
        ALOGW("%s(), %p %p fail!!", __FUNCTION__, dsp, hal);
        return;
    }

    AUDIO_ALLOC_STRUCT(aurisys_config_t, new_cfg);

    /* copy scene_lib_table */
    trans_scene_lib_table_dsp_to_hal(dsp->scene_lib_table,
                                     &new_cfg->scene_lib_table_hh);

    /* copy library_config */
    trans_library_config_dsp_to_hal(dsp->library_config,
                                    &new_cfg->library_config_hh);

    *hal = new_cfg;
}


void delete_aurisys_config(aurisys_config_t *aurisys_config) {
    aurisys_library_config_t *itor_lib = NULL;
    aurisys_library_config_t *tmp_lib = NULL;

    aurisys_component_t *itor_comp = NULL;
    aurisys_component_t *tmp_comp = NULL;

    aurisys_scene_lib_table_t *itor_scene_lib_table = NULL;
    aurisys_scene_lib_table_t *tmp_scene_lib_table = NULL;

    aurisys_library_name_t *itor_library_name = NULL;
    aurisys_library_name_t *tmp_library_name = NULL;

    int i = 0;

    if (aurisys_config == NULL) {
        AUD_LOG_E("%s aurisys_config is NULL!! return", __FUNCTION__);
        return;
    }


    /* delete library_config */
    HASH_ITER(hh, aurisys_config->library_config_hh, itor_lib, tmp_lib) {
        HASH_DEL(aurisys_config->library_config_hh, itor_lib);

        AUDIO_FREE_POINTER(itor_lib->name);
        AUDIO_FREE_POINTER(itor_lib->lib_path);
        AUDIO_FREE_POINTER(itor_lib->lib64_path);
        AUDIO_FREE_POINTER(itor_lib->param_path);
        AUDIO_FREE_POINTER(itor_lib->lib_dump_path);
        AUDIO_FREE_POINTER(itor_lib->adb_cmd_key);
        AUDIO_FREE_POINTER(itor_lib->api);


        /* delete component */
        HASH_ITER(hh, itor_lib->component_hh, itor_comp, tmp_comp) {
            HASH_DEL(itor_lib->component_hh, itor_comp);

            /* UL */
            if (itor_comp->lib_config.p_ul_buf_in != NULL) {
                //AUDIO_FREE_POINTER(itor_comp->lib_config.p_ul_buf_in->data_buf.p_buffer);
                AUDIO_FREE_POINTER(itor_comp->lib_config.p_ul_buf_in);
            }

            if (itor_comp->lib_config.p_ul_buf_out != NULL) {
                //AUDIO_FREE_POINTER(itor_comp->lib_config.p_ul_buf_out->data_buf.p_buffer);
                AUDIO_FREE_POINTER(itor_comp->lib_config.p_ul_buf_out);
            }

            if (itor_comp->lib_config.p_ul_ref_bufs != NULL) {
                for (i = 0; i < itor_comp->lib_config.num_ul_ref_buf_array; i++) {
                    //AUDIO_FREE_POINTER(itor_comp->lib_config.p_ul_ref_bufs[i].data_buf.p_buffer);
                }
                AUDIO_FREE_POINTER(itor_comp->lib_config.p_ul_ref_bufs);
            }


            /* DL */
            if (itor_comp->lib_config.p_dl_buf_in != NULL) {
                //AUDIO_FREE_POINTER(itor_comp->lib_config.p_dl_buf_in->data_buf.p_buffer);
                AUDIO_FREE_POINTER(itor_comp->lib_config.p_dl_buf_in);
            }

            if (itor_comp->lib_config.p_dl_buf_out != NULL) {
                //AUDIO_FREE_POINTER(itor_comp->lib_config.p_dl_buf_out->data_buf.p_buffer);
                AUDIO_FREE_POINTER(itor_comp->lib_config.p_dl_buf_out);
            }

            if (itor_comp->lib_config.p_dl_ref_bufs != NULL) {
                for (i = 0; i < itor_comp->lib_config.num_dl_ref_buf_array; i++) {
                    //AUDIO_FREE_POINTER(itor_comp->lib_config.p_dl_ref_bufs[i].data_buf.p_buffer);
                }
                AUDIO_FREE_POINTER(itor_comp->lib_config.p_dl_ref_bufs);
            }

            AUDIO_FREE_POINTER(itor_comp);
        }

        AUDIO_FREE_POINTER(itor_lib);
    }


    /* delete scene_lib_table */
    HASH_ITER(hh, aurisys_config->scene_lib_table_hh, itor_scene_lib_table, tmp_scene_lib_table) {
        HASH_DEL(aurisys_config->scene_lib_table_hh, itor_scene_lib_table);

        /* delete uplink_library_name */
        HASH_ITER(hh, itor_scene_lib_table->uplink_library_name_list, itor_library_name, tmp_library_name) {
            HASH_DEL(itor_scene_lib_table->uplink_library_name_list, itor_library_name);
            AUDIO_FREE_POINTER(itor_library_name->name);
            AUDIO_FREE_POINTER(itor_library_name);
        }

        /* delete downlink_library_name */
        HASH_ITER(hh, itor_scene_lib_table->downlink_library_name_list, itor_library_name, tmp_library_name) {
            HASH_DEL(itor_scene_lib_table->downlink_library_name_list, itor_library_name);
            AUDIO_FREE_POINTER(itor_library_name->name);
            AUDIO_FREE_POINTER(itor_library_name);
        }

        /* delete all_library_name_list */
        HASH_ITER(hh, itor_scene_lib_table->all_library_name_list, itor_library_name, tmp_library_name) {
            HASH_DEL(itor_scene_lib_table->all_library_name_list, itor_library_name);
            AUDIO_FREE_POINTER(itor_library_name->name);
            AUDIO_FREE_POINTER(itor_library_name);
        }

        AUDIO_FREE_POINTER(itor_scene_lib_table->uplink_digital_gain_lib_name);
        AUDIO_FREE_POINTER(itor_scene_lib_table->downlink_digital_gain_lib_name);

        AUDIO_FREE_POINTER(itor_scene_lib_table);
    }

    AUDIO_FREE_POINTER(aurisys_config);
}


#ifdef __cplusplus
}  /* extern "C" */
#endif

