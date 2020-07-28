#include "aurisys_lib_manager.h"

#include <stdlib.h>
#include <string.h>

#include <uthash.h> /* uthash */
#include <utlist.h> /* linked list */

#include <audio_log.h>
#include <audio_assert.h>
#include <audio_memory_control.h>
#include <audio_lock.h>
#include <audio_ringbuf.h>

#include <aurisys_scenario.h>
#include <aurisys_utility.h>

#include <arsi_type.h>
#include <aurisys_config.h>

#include <audio_pool_buf_handler.h>

#include <aurisys_lib_handler.h>



#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aurisys_lib_manager"


#ifdef AURISYS_DUMP_LOG_V
#undef  AUD_LOG_V
#define AUD_LOG_V(x...) AUD_LOG_D(x)
#endif


/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */

static struct alock_t *g_aurisys_lib_manager_lock;



/*
 * =============================================================================
 *                     public function implementation
 * =============================================================================
 */

void aurisys_lib_manager_c_file_init(void) {
    if (g_aurisys_lib_manager_lock == NULL) {
        NEW_ALOCK(g_aurisys_lib_manager_lock);
    }
}


void aurisys_lib_manager_c_file_deinit(void) {
#if 0 /* singleton lock */
    if (g_aurisys_lib_manager_lock != NULL) {
        FREE_ALOCK(g_aurisys_lib_manager_lock);
    }
#endif
}


aurisys_lib_manager_t *new_aurisys_lib_manager(
    aurisys_config_t *aurisys_config,
    const struct aurisys_lib_manager_config_t *manager_config) {
    aurisys_scene_lib_table_t *the_scene_lib_table = NULL;
    aurisys_lib_manager_t *new_lib_manager = NULL;

    aurisys_library_name_t *itor_library_name = NULL;
    aurisys_library_name_t *tmp_library_name = NULL;

    char *gain_handler_name = NULL;
    aurisys_lib_handler_t *the_handler = NULL;
    aurisys_lib_handler_t *gain_handler = NULL;


    LOCK_ALOCK_MS(g_aurisys_lib_manager_lock, 1000);

    /* get scene_lib_table */
    HASH_FIND_INT(
        aurisys_config->scene_lib_table_hh,
        &manager_config->aurisys_scenario,
        the_scene_lib_table);
    if (the_scene_lib_table == NULL) {
        AUD_ASSERT(the_scene_lib_table != NULL);
        UNLOCK_ALOCK(g_aurisys_lib_manager_lock);
        return NULL;
    }

    /* create manager */
    AUDIO_ALLOC_STRUCT(aurisys_lib_manager_t, new_lib_manager);

    NEW_ALOCK(new_lib_manager->lock);
    LOCK_ALOCK_MS(new_lib_manager->lock, 1000);

    new_lib_manager->self = new_lib_manager;


    /* create all_library_name_list libs */
    if (the_scene_lib_table->all_library_name_list != NULL) {
        aurisys_create_lib_handler_list_xlink(
            the_scene_lib_table->all_library_name_list,
            aurisys_config->library_config_hh,
            &new_lib_manager->all_lib_handler_list,
            &new_lib_manager->num_all_library_hanlder,
            manager_config);
    }

    /* copy lib handler pointer to UL */
    if ((the_scene_lib_table->uplink_library_name_list != NULL) &&
        (manager_config->arsi_process_type == ARSI_PROCESS_TYPE_UL_ONLY ||
         manager_config->arsi_process_type == ARSI_PROCESS_TYPE_UL_AND_DL)) {
        HASH_ITER(hh, the_scene_lib_table->uplink_library_name_list, itor_library_name, tmp_library_name) {
            HASH_FIND(hh_all,
                      new_lib_manager->all_lib_handler_list,
                      itor_library_name->name,
                      uthash_strlen(itor_library_name->name),
                      the_handler);
            if (the_handler == NULL) {
                AUD_LOG_E("%s(), UL lib %s not created in manager!!",
                          __FUNCTION__, itor_library_name->name);
                AUD_ASSERT(the_handler != NULL);
                continue;
            }
            HASH_ADD_KEYPTR(hh_ul_list, new_lib_manager->uplink_lib_handler_list,
                            the_handler->lib_name, strlen(the_handler->lib_name),
                            the_handler);
            new_lib_manager->num_uplink_library_hanlder++;
        }
    }

    /* copy lib handler pointer to DL */
    if ((the_scene_lib_table->downlink_library_name_list != NULL) &&
        (manager_config->arsi_process_type == ARSI_PROCESS_TYPE_DL_ONLY ||
         manager_config->arsi_process_type == ARSI_PROCESS_TYPE_UL_AND_DL)) {
        HASH_ITER(hh, the_scene_lib_table->downlink_library_name_list, itor_library_name, tmp_library_name) {
            HASH_FIND(hh_all,
                      new_lib_manager->all_lib_handler_list,
                      itor_library_name->name,
                      uthash_strlen(itor_library_name->name),
                      the_handler);
            if (the_handler == NULL) {
                AUD_LOG_E("%s(), DL lib %s not created in manager!!",
                          __FUNCTION__, itor_library_name->name);
                AUD_ASSERT(the_handler != NULL);
                continue;
            }
            HASH_ADD_KEYPTR(hh_dl_list, new_lib_manager->downlink_lib_handler_list,
                            the_handler->lib_name, strlen(the_handler->lib_name),
                            the_handler);
            new_lib_manager->num_downlink_library_hanlder++;
        }
    }

    if (new_lib_manager->uplink_lib_handler_list != NULL) {
        AUDIO_ALLOC_STRUCT(audio_pool_buf_formatter_t, new_lib_manager->ul_out_pool_formatter);

        gain_handler_name = the_scene_lib_table->uplink_digital_gain_lib_name;
        if (strlen(gain_handler_name) > 0) {
            AUD_LOG_V("UL gain set to %s", gain_handler_name);

            HASH_FIND(hh_ul_list,
                      new_lib_manager->uplink_lib_handler_list,
                      gain_handler_name,
                      (unsigned)uthash_strlen(gain_handler_name),
                      gain_handler);

            if (gain_handler == NULL) {
                AUD_ASSERT(gain_handler != NULL);
            } else {
                AUD_LOG_V("UL gain set to %s %p", gain_handler->lib_name, gain_handler);
                new_lib_manager->uplink_lib_handler_for_digital_gain = gain_handler;
            }
        }
    }

    if (new_lib_manager->downlink_lib_handler_list != NULL) {
        AUDIO_ALLOC_STRUCT(audio_pool_buf_formatter_t, new_lib_manager->dl_out_pool_formatter);

        gain_handler_name = the_scene_lib_table->downlink_digital_gain_lib_name;
        if (strlen(gain_handler_name) > 0) {
            AUD_LOG_V("DL gain set to %s", gain_handler_name);

            HASH_FIND(hh_dl_list,
                      new_lib_manager->downlink_lib_handler_list,
                      gain_handler_name,
                      (unsigned)uthash_strlen(gain_handler_name),
                      gain_handler);

            if (gain_handler == NULL) {
                AUD_ASSERT(gain_handler != NULL);
            } else {
                AUD_LOG_V("DL gain set to %s %p", gain_handler->lib_name, gain_handler);
                new_lib_manager->downlink_lib_handler_for_digital_gain = gain_handler;
            }
        }
    }

    UNLOCK_ALOCK(new_lib_manager->lock);
    UNLOCK_ALOCK(g_aurisys_lib_manager_lock);

    AUD_LOG_V("%s() done, manager %p, num_uplink_library_hanlder %d, num_downlink_library_hanlder %d",
              __FUNCTION__, new_lib_manager, new_lib_manager->num_uplink_library_hanlder, new_lib_manager->num_downlink_library_hanlder);
    return new_lib_manager;
}


int delete_aurisys_lib_manager(aurisys_lib_manager_t *manager) {
    aurisys_lib_handler_t *itor_lib_hanlder = NULL;
    aurisys_lib_handler_t *tmp_lib_hanlder = NULL;

    int i = 0;

    AUD_LOG_D("%s(), manager %p", __FUNCTION__, manager);

    if (manager == NULL) {
        AUD_LOG_E("%s(), manager == NULL!! return", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(g_aurisys_lib_manager_lock, 1000);

    LOCK_ALOCK_MS(manager->lock, 1000);


    /* delete uplink_lib_handler_list */
    if (manager->uplink_lib_handler_list != NULL) {
        AUD_LOG_V("num_uplink_library_hanlder %d", manager->num_uplink_library_hanlder);
        HASH_ITER(hh_ul_list, manager->uplink_lib_handler_list, itor_lib_hanlder, tmp_lib_hanlder) {
            HASH_DELETE(hh_ul_list, manager->uplink_lib_handler_list, itor_lib_hanlder);
        }
        manager->uplink_lib_handler_list = NULL;
        AUDIO_FREE_POINTER(manager->ul_out_pool_formatter);
    }

    /* delete downlink_lib_handler_list */
    if (manager->downlink_lib_handler_list != NULL) {
        AUD_LOG_V("num_downlink_library_hanlder %d", manager->num_downlink_library_hanlder);
        HASH_ITER(hh_dl_list, manager->downlink_lib_handler_list, itor_lib_hanlder, tmp_lib_hanlder) {
            HASH_DELETE(hh_dl_list, manager->downlink_lib_handler_list, itor_lib_hanlder);
        }
        manager->downlink_lib_handler_list = NULL;
        AUDIO_FREE_POINTER(manager->dl_out_pool_formatter);
    }

    /* delete all_lib_handler_list */
    if (manager->all_lib_handler_list != NULL) {
        AUD_LOG_V("all_lib_handler_list %d", manager->num_all_library_hanlder);
        aurisys_destroy_lib_handler_list(&manager->all_lib_handler_list);
    }


    /* free in_out_bufs */
    for (i = 0; i < NUM_DATA_BUF_TYPE; i++) {
        destroy_audio_pool_buf(manager->in_out_bufs[i]);
    }


    UNLOCK_ALOCK(manager->lock);

    /* delete manager */
    FREE_ALOCK(manager->lock);
    AUDIO_FREE_POINTER(manager);

    UNLOCK_ALOCK(g_aurisys_lib_manager_lock);

    return 0;
}


int aurisys_parsing_param_file(aurisys_lib_manager_t *manager) {
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    if (manager == NULL) {
        AUD_LOG_E("%s(), manager == NULL!! return", __FUNCTION__);
        return -1;
    }


    LOCK_ALOCK_MS(manager->lock, 1000);

    HASH_ITER(hh_all, manager->all_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
        aurisys_arsi_parsing_param_file(itor_lib_handler);
    }

    UNLOCK_ALOCK(manager->lock);

    return 0;
}


static void *get_write_ptr(struct data_buf_t *data_buf, uint32_t need_size) {
    uint8_t *write_ptr = NULL;

    if (!data_buf) {
        return NULL;
    }
    if ((data_buf->data_size + need_size) > data_buf->memory_size) {
        return NULL;
    }

    write_ptr = ((uint8_t *)data_buf->p_buffer) + data_buf->data_size;
    data_buf->data_size += need_size;

    return write_ptr;
}


static int aurisys_get_param_list_xlink(
    struct aurisys_lib_handler_t *lib_handler_list,
    uint32_t num_library_hanlder,
    struct data_buf_t *param_list) {
    uint32_t *p_num_library_hanlder = NULL;
    uint32_t *lib_name_sz = 0;
    uint32_t lib_name_sz_align = 0;
    char *lib_name = NULL;
    uint32_t *param_data_sz = 0;
    uint32_t param_data_sz_align = 0;
    char *param_data = NULL;

    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    if (param_list == NULL) {
        AUD_LOG_E("%s(), param_list NULL!! return", __FUNCTION__);
        return -1;
    }
    if (param_list->p_buffer == NULL) {
        AUD_LOG_E("%s(), param_list->p_buffer NULL!! return", __FUNCTION__);
        return -1;
    }


    p_num_library_hanlder = (uint32_t *)get_write_ptr(param_list, sizeof(uint32_t));
    if (p_num_library_hanlder == NULL) {
        return -1;
    }
    *p_num_library_hanlder = num_library_hanlder;
    AUD_LOG_V("%s(), %p *p_num_library_hanlder %u", __FUNCTION__,
              p_num_library_hanlder, *p_num_library_hanlder);

    HASH_ITER(hh_all, lib_handler_list, itor_lib_handler, tmp_lib_handler) {
        /* lib name */
        lib_name_sz = (uint32_t *)get_write_ptr(param_list, sizeof(uint32_t));
        if (lib_name_sz == NULL) {
            return -1;
        }
        *lib_name_sz = strlen(itor_lib_handler->lib_name) + 1;
        AUD_LOG_V("%s(), %p *lib_name_sz %u", __FUNCTION__, lib_name_sz, *lib_name_sz);

        if ((*lib_name_sz) & 0x7) {
            lib_name_sz_align = (((*lib_name_sz) / 8) + 1) * 8;
        } else {
            lib_name_sz_align = *lib_name_sz;
        }
        lib_name = (char *)get_write_ptr(param_list, lib_name_sz_align);
        if (lib_name == NULL) {
            return -1;
        }
        strcpy_safe(lib_name, itor_lib_handler->lib_name, (*lib_name_sz));
        AUD_LOG_V("%s(), %p lib_name %s", __FUNCTION__, lib_name, lib_name);

        /* param data */
        param_data_sz = (uint32_t *)get_write_ptr(param_list, sizeof(uint32_t));
        if (param_data_sz == NULL) {
            return -1;
        }
        *param_data_sz = itor_lib_handler->param_buf.data_size;
        AUD_LOG_V("%s(), %p *param_data_sz %u", __FUNCTION__, param_data_sz, *param_data_sz);

        if ((*param_data_sz) != 0) {
            if ((*param_data_sz) & 0x7) {
                param_data_sz_align = (((*param_data_sz) / 8) + 1) * 8;
            } else {
                param_data_sz_align = *param_data_sz;
            }
            param_data = (char *)get_write_ptr(param_list, param_data_sz_align);
            if (param_data == NULL) {
                return -1;
            }
            AUD_LOG_V("%s(), %p param_data", __FUNCTION__, param_data);
            memcpy(param_data, itor_lib_handler->param_buf.p_buffer, (*param_data_sz));
        }
    }

    return 0;
}


int aurisys_get_param_list(
    aurisys_lib_manager_t *manager,
    struct data_buf_t *param_list) {
    uint32_t *p_guard_head = NULL;
    uint32_t *p_guard_tail = NULL;
    int retval = 0;

    if (manager == NULL) {
        AUD_LOG_E("%s(), manager NULL!! return", __FUNCTION__);
        return -1;
    }
    if (param_list == NULL) {
        AUD_LOG_E("%s(), param_list NULL!! return", __FUNCTION__);
        return -1;
    }
    if (param_list->p_buffer == NULL) {
        AUD_LOG_E("%s(), param_list->p_buffer NULL!! return", __FUNCTION__);
        return -1;
    }

    p_guard_head = (uint32_t *)get_write_ptr(param_list, sizeof(uint32_t));
    if (p_guard_head == NULL) {
        return -1;
    }
    *p_guard_head = AURISYS_GUARD_HEAD_VALUE;

    retval = aurisys_get_param_list_xlink(
                 manager->all_lib_handler_list,
                 manager->num_all_library_hanlder,
                 param_list);
    if (retval != 0) {
        return retval;
    }

    p_guard_tail = (uint32_t *)get_write_ptr(param_list, sizeof(uint32_t));
    if (p_guard_tail == NULL) {
        return -1;
    }
    *p_guard_tail = AURISYS_GUARD_TAIL_VALUE;

    return 0;
}


int aurisys_create_arsi_handlers(aurisys_lib_manager_t *manager) {
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    if (manager == NULL) {
        AUD_LOG_E("%s(), manager == NULL!! return", __FUNCTION__);
        return -1;
    }


    LOCK_ALOCK_MS(manager->lock, 1000);

    HASH_ITER(hh_all, manager->all_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
        aurisys_arsi_create_handler(itor_lib_handler);
    }

    UNLOCK_ALOCK(manager->lock);

    return 0;
}


int aurisys_destroy_arsi_handlers(aurisys_lib_manager_t *manager) {
    aurisys_lib_handler_t *itor_lib_hanlder = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    if (manager == NULL) {
        AUD_LOG_E("%s(), manager == NULL!! return", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(manager->lock, 1000);

    HASH_ITER(hh_all, manager->all_lib_handler_list, itor_lib_hanlder, tmp_lib_handler) {
        aurisys_arsi_destroy_handler(itor_lib_hanlder);
    }

    UNLOCK_ALOCK(manager->lock);

    return 0;
}


audio_pool_buf_t *create_audio_pool_buf(
    aurisys_lib_manager_t *manager,
    const data_buf_type_t  data_buf_type,
    const uint32_t         memory_size) {
    audio_pool_buf_t *pool_buf = NULL;

    if (manager == NULL) {
        AUD_LOG_E("%s(), manager == NULL!! return", __FUNCTION__);
        return NULL;
    }
    if (data_buf_type >= NUM_DATA_BUF_TYPE) {
        AUD_LOG_E("%s(), type %d error!!", __FUNCTION__, data_buf_type);
        return NULL;
    }
    if (manager->in_out_bufs[data_buf_type]) {
        AUD_LOG_W("%s(), manager %p type %d exist!!", __FUNCTION__,
                  manager, data_buf_type);
        return manager->in_out_bufs[data_buf_type];
    }

    AUDIO_ALLOC_STRUCT(audio_pool_buf_t, pool_buf);
    AUDIO_ALLOC_STRUCT(audio_buf_t, pool_buf->buf);

    pool_buf->buf->data_buf_type = data_buf_type;

    if (memory_size > 0) {
        AUDIO_ALLOC_CHAR_BUFFER(pool_buf->ringbuf.base, memory_size);
        pool_buf->ringbuf.read  = pool_buf->ringbuf.base;
        pool_buf->ringbuf.write = pool_buf->ringbuf.base;
        pool_buf->ringbuf.size  = memory_size;

        config_data_buf_by_ringbuf(
            &pool_buf->buf->data_buf,
            &pool_buf->ringbuf);
    }

    manager->in_out_bufs[data_buf_type] = pool_buf;
    return pool_buf;
}


void destroy_audio_pool_buf(struct audio_pool_buf_t *pool_buf) {
    if (pool_buf) {
        AUDIO_FREE_POINTER(pool_buf->ringbuf.base);
        AUDIO_FREE_POINTER(pool_buf->buf);
        AUDIO_FREE_POINTER(pool_buf);
    }
}


int aurisys_pool_buf_formatter_init(aurisys_lib_manager_t *manager) {
#ifndef AURISYS_BYPASS_ALL_LIBRARY
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;
#endif

    audio_pool_buf_t *ul_in = NULL;
    audio_pool_buf_t *ul_out = NULL;
    audio_pool_buf_t *aec = NULL;
    audio_pool_buf_t *dl_in = NULL;
    audio_pool_buf_t *dl_out = NULL;

    audio_pool_buf_formatter_t *formatter = NULL;
    audio_buf_t *source = NULL;
    audio_buf_t *target = NULL;

    if (manager == NULL) {
        AUD_LOG_E("%s(), manager == NULL!! return", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(manager->lock, 1000);

    ul_in   = manager->in_out_bufs[DATA_BUF_UPLINK_IN];
    ul_out  = manager->in_out_bufs[DATA_BUF_UPLINK_OUT];
    dl_in   = manager->in_out_bufs[DATA_BUF_DOWNLINK_IN];
    dl_out  = manager->in_out_bufs[DATA_BUF_DOWNLINK_OUT];
    aec     = manager->in_out_bufs[DATA_BUF_ECHO_REF];


    /* TODO: abstract xlink */
    if (manager->num_uplink_library_hanlder != 0) {
        AUD_ASSERT(ul_in != 0);
        AUD_ASSERT(ul_out != 0);
        AUD_ASSERT(ul_in->buf != 0);
        AUD_ASSERT(ul_out->buf != 0);

        /* init in => out */
        manager->ul_out_pool_formatter->pool_source = ul_in;
        manager->ul_out_pool_formatter->pool_target = ul_out;
#ifndef AURISYS_BYPASS_ALL_LIBRARY
        HASH_ITER(hh_ul_list, manager->uplink_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
            /* insert a lib hanlder into formatter chain */
            formatter = &itor_lib_handler->ul_pool_formatter;
            formatter->pool_source = manager->ul_out_pool_formatter->pool_source;
            formatter->pool_target =                      &itor_lib_handler->ul_pool_in;
            manager->ul_out_pool_formatter->pool_source = &itor_lib_handler->ul_pool_out;

            source = formatter->pool_source->buf;
            target = formatter->pool_target->buf;
            AUD_LOG_D("UL Lib, lib_name %s, %p, sample_rate: %d => %d, num_channels: %d => %d, audio_format: 0x%x => 0x%x, interleave: %d => %d, frame: %d => %d",
                      itor_lib_handler->lib_name,
                      itor_lib_handler,
                      source->sample_rate_buffer, target->sample_rate_buffer,
                      source->num_channels, target->num_channels,
                      source->audio_format, target->audio_format,
                      source->b_interleave, target->b_interleave,
                      source->frame_size_ms, target->frame_size_ms);
            audio_pool_buf_formatter_init(formatter);


            /* aec */
            if (itor_lib_handler->aec_pool_in != NULL) {
                AUD_ASSERT(aec != 0);
                AUD_ASSERT(aec->buf != 0);

                AUD_ASSERT(manager->aec_pool_formatter == NULL); /* only 1 aec in 1 manager, */
                AUDIO_ALLOC_STRUCT(audio_pool_buf_formatter_t, manager->aec_pool_formatter);
                manager->aec_pool_formatter->pool_source = aec;
                manager->aec_pool_formatter->pool_target = itor_lib_handler->aec_pool_in;
            }
        }
#endif
        formatter = manager->ul_out_pool_formatter;
        source = formatter->pool_source->buf;
        target = formatter->pool_target->buf;
        AUD_LOG_D("UL out, sample_rate: %d => %d, num_channels: %d => %d, audio_format: 0x%x => 0x%x, interleave: %d => %d, frame: %d => %d",
                  source->sample_rate_buffer, target->sample_rate_buffer,
                  source->num_channels, target->num_channels,
                  source->audio_format, target->audio_format,
                  source->b_interleave, target->b_interleave,
                  source->frame_size_ms, target->frame_size_ms);
        audio_pool_buf_formatter_init(formatter);

        if (manager->aec_pool_formatter != NULL) {
            formatter = manager->aec_pool_formatter;
            source = formatter->pool_source->buf;
            target = formatter->pool_target->buf;
            AUD_LOG_D("AEC, sample_rate: %d => %d, num_channels: %d => %d, audio_format: 0x%x => 0x%x, interleave: %d => %d, frame: %d => %d",
                      source->sample_rate_buffer, target->sample_rate_buffer,
                      source->num_channels, target->num_channels,
                      source->audio_format, target->audio_format,
                      source->b_interleave, target->b_interleave,
                      source->frame_size_ms, target->frame_size_ms);
            audio_pool_buf_formatter_init(formatter);
        }
    }

    if (manager->num_downlink_library_hanlder != 0) {
        AUD_ASSERT(dl_in != 0);
        AUD_ASSERT(dl_out != 0);
        AUD_ASSERT(dl_in->buf != 0);
        AUD_ASSERT(dl_out->buf != 0);

        /* init in => out */
        manager->dl_out_pool_formatter->pool_source = dl_in;
        manager->dl_out_pool_formatter->pool_target = dl_out;
#ifndef AURISYS_BYPASS_ALL_LIBRARY
        HASH_ITER(hh_dl_list, manager->downlink_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
            /* insert a lib hanlder into formatter chain */
            formatter = &itor_lib_handler->dl_pool_formatter;
            formatter->pool_source = manager->dl_out_pool_formatter->pool_source;
            formatter->pool_target =                      &itor_lib_handler->dl_pool_in;
            manager->dl_out_pool_formatter->pool_source = &itor_lib_handler->dl_pool_out;

            source = formatter->pool_source->buf;
            target = formatter->pool_target->buf;
            AUD_LOG_D("DL Lib, lib_name %s, %p, sample_rate: %d => %d, num_channels: %d => %d, audio_format: 0x%x => 0x%x, interleave: %d => %d, frame: %d => %d",
                      itor_lib_handler->lib_name,
                      itor_lib_handler,
                      source->sample_rate_buffer, target->sample_rate_buffer,
                      source->num_channels, target->num_channels,
                      source->audio_format, target->audio_format,
                      source->b_interleave, target->b_interleave,
                      source->frame_size_ms, target->frame_size_ms);
            audio_pool_buf_formatter_init(formatter);
        }
#endif
        formatter = manager->dl_out_pool_formatter;
        source = formatter->pool_source->buf;
        target = formatter->pool_target->buf;
        AUD_LOG_D("DL out, sample_rate: %d => %d, num_channels: %d => %d, audio_format: 0x%x => 0x%x, interleave: %d => %d, frame: %d => %d",
                  source->sample_rate_buffer, target->sample_rate_buffer,
                  source->num_channels, target->num_channels,
                  source->audio_format, target->audio_format,
                  source->b_interleave, target->b_interleave,
                  source->frame_size_ms, target->frame_size_ms);
        audio_pool_buf_formatter_init(formatter);
    }


    UNLOCK_ALOCK(manager->lock);

    return 0;
}


int aurisys_pool_buf_formatter_deinit(aurisys_lib_manager_t *manager) {
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    if (manager == NULL) {
        AUD_LOG_E("%s(), manager == NULL!! return", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(manager->lock, 1000);

    if (manager->num_uplink_library_hanlder != 0) {
        HASH_ITER(hh_ul_list, manager->uplink_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
            audio_pool_buf_formatter_deinit(&itor_lib_handler->ul_pool_formatter);
        }
        audio_pool_buf_formatter_deinit(manager->ul_out_pool_formatter);

        if (manager->aec_pool_formatter != NULL) {
            audio_pool_buf_formatter_deinit(manager->aec_pool_formatter);
            AUDIO_FREE_POINTER(manager->aec_pool_formatter);
        }
    }


    if (manager->num_downlink_library_hanlder != 0) {
        HASH_ITER(hh_dl_list, manager->downlink_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
            audio_pool_buf_formatter_deinit(&itor_lib_handler->dl_pool_formatter);
        }
        audio_pool_buf_formatter_deinit(manager->dl_out_pool_formatter);
    }

    UNLOCK_ALOCK(manager->lock);

    return 0;
}


int aurisys_process_ul_only(
    aurisys_lib_manager_t *manager,
    audio_pool_buf_t *buf_in,
    audio_pool_buf_t *buf_out,
    audio_pool_buf_t *buf_aec) {
#ifndef AURISYS_BYPASS_ALL_LIBRARY
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    uint32_t process_count = 0;
#endif
    uint32_t need_format_count = 0;

    if (!manager || !buf_in || !buf_out) {
        AUD_LOG_E("%s(), NULL! return", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(manager->lock, 1000);


    AUD_ASSERT(manager->uplink_lib_handler_list != NULL);
    AUD_ASSERT(manager->num_uplink_library_hanlder != 0);
    AUD_ASSERT(manager->ul_out_pool_formatter != NULL);


    AUD_LOG_V("%s(+), rb_in  data_count %u, free_count %u", __FUNCTION__,
              audio_ringbuf_count(&buf_in->ringbuf),
              audio_ringbuf_free_space(&buf_in->ringbuf));
    AUD_LOG_V("%s(+), rb_out data_count %u, free_count %u", __FUNCTION__,
              audio_ringbuf_count(&buf_out->ringbuf),
              audio_ringbuf_free_space(&buf_out->ringbuf));


    need_format_count = audio_ringbuf_count(&buf_in->ringbuf);

#ifndef AURISYS_BYPASS_ALL_LIBRARY
    HASH_ITER(hh_ul_list, manager->uplink_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
        /* in src */
        if (need_format_count > 0) {
            audio_pool_buf_formatter_process(&itor_lib_handler->ul_pool_formatter);
        }

        /* aec src */
        if (itor_lib_handler->aec_pool_in != NULL && buf_aec != NULL) {
            AUD_LOG_V("%s(+), rb_aec data_count %u, free_count %u", __FUNCTION__,
                      audio_ringbuf_count(&buf_aec->ringbuf),
                      audio_ringbuf_free_space(&buf_aec->ringbuf));
            AUD_ASSERT(manager->aec_pool_formatter != NULL);
            AUD_ASSERT(audio_ringbuf_count(&buf_aec->ringbuf) != 0); /* TODO: test & remove */
            audio_pool_buf_formatter_process(manager->aec_pool_formatter);
        }

        process_count = aurisys_arsi_process_ul_only(itor_lib_handler);
        need_format_count = process_count;
    }
#endif
    if (need_format_count) {
        audio_pool_buf_formatter_process(manager->ul_out_pool_formatter);
    }


    if (buf_aec != NULL) {
        AUD_LOG_V("%s(-), rb_aec data_count %u, free_count %u", __FUNCTION__,
                  audio_ringbuf_count(&buf_aec->ringbuf),
                  audio_ringbuf_free_space(&buf_aec->ringbuf));
    }
    AUD_LOG_VV("%s(-), rb_in  data_count %u, free_count %u", __FUNCTION__,
               audio_ringbuf_count(&buf_in->ringbuf), audio_ringbuf_free_space(&buf_in->ringbuf));
    AUD_LOG_V("%s(-), rb_out data_count %u, free_count %u", __FUNCTION__,
              audio_ringbuf_count(&buf_out->ringbuf), audio_ringbuf_free_space(&buf_out->ringbuf));

    UNLOCK_ALOCK(manager->lock);

    return 0;
}


/* TODO: abstract xlink */
int aurisys_process_dl_only(
    aurisys_lib_manager_t *manager,
    struct audio_pool_buf_t *buf_in,
    struct audio_pool_buf_t *buf_out) {
#ifndef AURISYS_BYPASS_ALL_LIBRARY
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    uint32_t process_count = 0;
#endif
    uint32_t need_format_count = 0;

    if (!manager || !buf_in || !buf_out) {
        AUD_LOG_E("%s(), NULL! return", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(manager->lock, 1000);


    AUD_ASSERT(manager->downlink_lib_handler_list != NULL);
    AUD_ASSERT(manager->num_downlink_library_hanlder != 0);
    AUD_ASSERT(manager->dl_out_pool_formatter != NULL);


    AUD_LOG_V("%s(+), rb_in  data_count %u, free_count %u", __FUNCTION__,
              audio_ringbuf_count(&buf_in->ringbuf), audio_ringbuf_free_space(&buf_in->ringbuf));
    AUD_LOG_V("%s(+), rb_out data_count %u, free_count %u", __FUNCTION__,
              audio_ringbuf_count(&buf_out->ringbuf), audio_ringbuf_free_space(&buf_out->ringbuf));


    need_format_count = audio_ringbuf_count(&buf_in->ringbuf);

#ifndef AURISYS_BYPASS_ALL_LIBRARY
    HASH_ITER(hh_dl_list, manager->downlink_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
        if (need_format_count > 0) {
            audio_pool_buf_formatter_process(&itor_lib_handler->dl_pool_formatter);
        }
        process_count = aurisys_arsi_process_dl_only(itor_lib_handler);
        need_format_count = process_count;
    }
#endif
    if (need_format_count) {
        audio_pool_buf_formatter_process(manager->dl_out_pool_formatter);
    }


    AUD_LOG_VV("%s(-), rb_in  data_count %u, free_count %u", __FUNCTION__,
               audio_ringbuf_count(&buf_in->ringbuf), audio_ringbuf_free_space(&buf_in->ringbuf));
    AUD_LOG_V("%s(-), rb_out data_count %u, free_count %u", __FUNCTION__,
              audio_ringbuf_count(&buf_out->ringbuf), audio_ringbuf_free_space(&buf_out->ringbuf));

    UNLOCK_ALOCK(manager->lock);

    return 0;
}


int aurisys_process_ul_and_dl(
    aurisys_lib_manager_t *manager,
    struct audio_pool_buf_t *ul_buf_in,
    struct audio_pool_buf_t *ul_buf_out,
    struct audio_pool_buf_t *ul_buf_aec,
    struct audio_pool_buf_t *dl_buf_in,
    struct audio_pool_buf_t *dl_buf_out) {
    if (!manager ||
        !ul_buf_in || !ul_buf_out || !ul_buf_aec ||
        !dl_buf_in || !dl_buf_out) {
        AUD_LOG_E("%s(), NULL! return", __FUNCTION__);
        return -1;
    }

    return 0;
}


int aurisys_set_ul_digital_gain(
    aurisys_lib_manager_t *manager,
    const int16_t ul_analog_gain_ref_only,
    const int16_t ul_digital_gain) {
    aurisys_lib_handler_t *gain_hanlder = NULL;

    if (!manager) {
        AUD_LOG_E("%s(), manager NULL! return", __FUNCTION__);
        return -1;
    }

    gain_hanlder = manager->uplink_lib_handler_for_digital_gain;
    if (!gain_hanlder) {
        AUD_LOG_E("%s(), gain_hanlder NULL! return", __FUNCTION__);
        return -1;
    }


    LOCK_ALOCK_MS(manager->lock, 1000);

    aurisys_arsi_set_ul_digital_gain(
        gain_hanlder,
        ul_analog_gain_ref_only,
        ul_digital_gain);

    UNLOCK_ALOCK(manager->lock);
    return 0;
}


int aurisys_set_dl_digital_gain(
    aurisys_lib_manager_t *manager,
    const int16_t dl_analog_gain_ref_only,
    const int16_t dl_digital_gain) {
    aurisys_lib_handler_t *gain_hanlder = NULL;

    if (!manager) {
        AUD_LOG_E("%s(), manager NULL! return", __FUNCTION__);
        return -1;
    }

    gain_hanlder = manager->downlink_lib_handler_for_digital_gain;
    if (!gain_hanlder) {
        AUD_LOG_E("%s(), gain_hanlder NULL! return", __FUNCTION__);
        return -1;
    }


    LOCK_ALOCK_MS(manager->lock, 1000);

    aurisys_arsi_set_dl_digital_gain(
        gain_hanlder,
        dl_analog_gain_ref_only,
        dl_digital_gain);

    UNLOCK_ALOCK(manager->lock);
    return 0;
}


int aurisys_set_ul_enhance(
    aurisys_lib_manager_t *manager,
    const uint8_t b_enhance_on) {
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    if (!manager) {
        AUD_LOG_E("%s(), manager NULL! return", __FUNCTION__);
        return -1;
    }
    if (manager->uplink_lib_handler_list == NULL) {
        AUD_LOG_E("%s(), uplink_lib_handler_list NULL! return", __FUNCTION__);
        return -1;
    }
    if (manager->num_uplink_library_hanlder == 0) {
        AUD_LOG_E("%s(), num_uplink_library_hanlder 0! return", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(manager->lock, 1000);

    HASH_ITER(hh_ul_list, manager->uplink_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
        aurisys_arsi_set_ul_enhance(itor_lib_handler, b_enhance_on);
    }

    UNLOCK_ALOCK(manager->lock);
    return 0;
}


int aurisys_set_dl_enhance(
    aurisys_lib_manager_t *manager,
    const uint8_t b_enhance_on) {
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    if (!manager) {
        AUD_LOG_E("%s(), manager NULL! return", __FUNCTION__);
        return -1;
    }
    if (manager->downlink_lib_handler_list == NULL) {
        AUD_LOG_E("%s(), downlink_lib_handler_list NULL! return", __FUNCTION__);
        return -1;
    }
    if (manager->num_downlink_library_hanlder == 0) {
        AUD_LOG_E("%s(), num_downlink_library_hanlder 0! return", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(manager->lock, 1000);

    HASH_ITER(hh_dl_list, manager->downlink_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
        aurisys_arsi_set_dl_enhance(itor_lib_handler, b_enhance_on);
    }

    UNLOCK_ALOCK(manager->lock);
    return 0;
}


int aurisys_set_ul_mute(
    aurisys_lib_manager_t *manager,
    const uint8_t b_mute_on) {
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    if (!manager) {
        AUD_LOG_E("%s(), manager NULL! return", __FUNCTION__);
        return -1;
    }
    if (manager->uplink_lib_handler_list == NULL) {
        AUD_LOG_E("%s(), uplink_lib_handler_list NULL! return", __FUNCTION__);
        return -1;
    }
    if (manager->num_uplink_library_hanlder == 0) {
        AUD_LOG_E("%s(), num_uplink_library_hanlder 0! return", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(manager->lock, 1000);

    HASH_ITER(hh_ul_list, manager->uplink_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
        aurisys_arsi_set_ul_mute(itor_lib_handler, b_mute_on);
    }

    UNLOCK_ALOCK(manager->lock);
    return 0;
}


int aurisys_set_dl_mute(
    aurisys_lib_manager_t *manager,
    const uint8_t b_mute_on) {
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    if (!manager) {
        AUD_LOG_E("%s(), manager NULL! return", __FUNCTION__);
        return -1;
    }
    if (manager->downlink_lib_handler_list == NULL) {
        AUD_LOG_E("%s(), downlink_lib_handler_list NULL! return", __FUNCTION__);
        return -1;
    }
    if (manager->num_downlink_library_hanlder == 0) {
        AUD_LOG_E("%s(), num_downlink_library_hanlder 0! return", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(manager->lock, 1000);

    HASH_ITER(hh_dl_list, manager->downlink_lib_handler_list, itor_lib_handler, tmp_lib_handler) {
        aurisys_arsi_set_dl_mute(itor_lib_handler, b_mute_on);
    }

    UNLOCK_ALOCK(manager->lock);
    return 0;
}



#ifdef __cplusplus
}  /* extern "C" */
#endif

