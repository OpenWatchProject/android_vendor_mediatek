#include "aurisys_controller.h"

#include <stdbool.h>
#include <errno.h>

#include <uthash.h> /* uthash */

#include <audio_log_hal.h>
#include <audio_assert.h>
#include <audio_memory_control.h>
#include <audio_lock.h>

#include <aurisys_scenario_dsp.h>

#include <arsi_type.h>
#include <aurisys_config.h>

#include <audio_pool_buf_handler.h>
#include <audio_ipi_dma.h>


#include <aurisys_utility.h>
#include <aurisys_config_parser.h>
#include <aurisys_lib_manager.h>
#include <aurisys_lib_handler.h>

#include <aurisys_adb_command.h>


#ifdef __cplusplus
extern "C" {
#endif


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "[AURI][CTRL]"


#ifdef AURISYS_DUMP_LOG_V
#undef  AUD_LOG_V
#define AUD_LOG_V(x...) AUD_LOG_D(x)
#endif



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */


#define AURISYS_CTRL_LOCK_TIMEOUT_MS (2000)

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

typedef struct aurisys_controller_t {
    struct alock_t *lock;

    aurisys_config_t *aurisys_config;

    aurisys_lib_manager_t *manager_hh;

    bool aurisys_on;
} aurisys_controller_t;


/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */

static struct alock_t *g_aurisys_controller_lock;

static aurisys_controller_t *g_controller; /* singleton */


/*
 * =============================================================================
 *                     private function declaration
 * =============================================================================
 */

static int aurisys_set_parameter_to_component(
    aurisys_library_config_t *library_config,
    aurisys_component_t *component,
    aurisys_adb_command_t *adb_cmd);



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

int init_aurisys_controller(void *cfg_buf, uint32_t cfg_size) {
    AUD_LOG_D("%s(+), heap(%d/%d)", __FUNCTION__,
              xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);

    if (cfg_buf == NULL || cfg_size != sizeof(struct aurisys_config_dsp_t)) {
        AUD_LOG_E("%s(-), %p %u error!! return",
                  __FUNCTION__, cfg_buf, cfg_size);
        return -EINVAL;
    }

    if (g_aurisys_controller_lock == NULL) {
        NEW_ALOCK(g_aurisys_controller_lock);
    }

    /* create controller */
    LOCK_ALOCK_MS(g_aurisys_controller_lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    if (g_controller != NULL) {
        AUD_LOG_E("%s(-), g_controller != NULL!! return", __FUNCTION__);
        UNLOCK_ALOCK(g_aurisys_controller_lock);
        return -EEXIST;
    }

    AUDIO_ALLOC_STRUCT(aurisys_controller_t, g_controller);
    NEW_ALOCK(g_controller->lock);
    UNLOCK_ALOCK(g_aurisys_controller_lock);


    /* init manager & handler */
    audio_pool_buf_handler_c_file_init();
    aurisys_lib_manager_c_file_init();
    aurisys_lib_handler_c_file_init();

    /* parse aurisys_config */
    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    trans_aurisys_config_dsp_to_hal(cfg_buf, &g_controller->aurisys_config);

    /* enable */
    g_controller->aurisys_on = true;
    UNLOCK_ALOCK(g_controller->lock);


    AUD_LOG_D("%s(-), heap(%d/%d)", __FUNCTION__,
              xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);

    return 0;
}


int deinit_aurisys_controller(void) {
    if (!g_aurisys_controller_lock) {
        AUD_LOG_W("%s(), lock not init. bypass", __FUNCTION__);
        return 0;
    }

    AUD_LOG_D("%s()", __FUNCTION__);

    LOCK_ALOCK_MS(g_aurisys_controller_lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    if (g_controller == NULL) {
        AUD_LOG_E("%s(-), g_controller == NULL!! return", __FUNCTION__);
        UNLOCK_ALOCK(g_aurisys_controller_lock);
        return -1;
    }

    /* delete aurisys_config */
    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    delete_aurisys_config(g_controller->aurisys_config);
    g_controller->aurisys_config = NULL;
    UNLOCK_ALOCK(g_controller->lock);

    /* deinit manager & handler */
    aurisys_lib_handler_c_file_deinit();
    aurisys_lib_manager_c_file_deinit();
    audio_pool_buf_handler_c_file_deinit();


    /* destroy controller */
    FREE_ALOCK(g_controller->lock);
    AUDIO_FREE_POINTER(g_controller);
    g_controller = NULL;
    UNLOCK_ALOCK(g_aurisys_controller_lock);

#if 0 /* singleton lock */
    FREE_ALOCK(g_aurisys_controller_lock);
#endif

    return 0;
}


aurisys_lib_manager_t *create_aurisys_lib_manager(
    const struct aurisys_lib_manager_config_t *manager_config) {
    aurisys_lib_manager_t *new_manager = NULL;

    if (!g_controller) {
        AUD_LOG_E("%s(), aurisys_scenario %d, g_controller NULL!!",
                  __FUNCTION__, manager_config->aurisys_scenario);
        return NULL;
    }

    AUD_LOG_D("%s(+), aurisys_scenario %d, heap(%d/%d)", __FUNCTION__,
              manager_config->aurisys_scenario,
              xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);

    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    new_manager = new_aurisys_lib_manager(
                      g_controller->aurisys_config,
                      manager_config);
    HASH_ADD_PTR(g_controller->manager_hh, self, new_manager);
    UNLOCK_ALOCK(g_controller->lock);

    AUD_LOG_D("%s(-), aurisys_scenario %d, heap(%d/%d)", __FUNCTION__,
              manager_config->aurisys_scenario,
              xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);

    return new_manager;
}


int destroy_aurisys_lib_manager(aurisys_lib_manager_t *manager) {
    if (manager == NULL) {
        AUD_LOG_E("%s(), manager == NULL!! return", __FUNCTION__);
        return -1;
    }
    if (!g_controller) {
        AUD_LOG_E("%s(), g_controller NULL!!", __FUNCTION__);
        return -1;
    }

    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);

    /* destroy manager */
    HASH_DEL(g_controller->manager_hh, manager);
    delete_aurisys_lib_manager(manager);

    UNLOCK_ALOCK(g_controller->lock);

    return 0;
}


bool aurisys_set_parameter(struct aurisys_adb_command_t *adb_cmd) {
    aurisys_config_t *aurisys_config = NULL;
    uint32_t scenario_all = 0xFFFFFFFF;
    uint32_t scenario_size = 0;

    aurisys_library_config_t *library_config = NULL;
    aurisys_component_t *component = NULL;

    aurisys_library_config_t *itor_library_config = NULL;
    aurisys_library_config_t *tmp_library_config = NULL;

    uint32_t scenario_idx = 0xFFFFFFFF;

    bool set_adb_done = false;

    bool pass = false;
    int ret = 0;

    if (!adb_cmd) {
        return false;
    }


    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    AUD_LOG_V("%s(+), %d", __FUNCTION__, adb_cmd->adb_cmd_type);

    aurisys_config = g_controller->aurisys_config;
    scenario_all = AURISYS_SCENARIO_DSP_ALL;
    scenario_size = AURISYS_SCENARIO_DSP_SIZE;


    /* find library_config by adb_cmd_key */
    AUD_ASSERT(aurisys_config != NULL);
    AUD_ASSERT(aurisys_config->library_config_hh != NULL);
    HASH_ITER(hh, aurisys_config->library_config_hh, itor_library_config, tmp_library_config) {
        if (!strcmp(adb_cmd->adb_cmd_key, itor_library_config->adb_cmd_key)) {
            library_config = itor_library_config;
            break;
        }
    }
    if (library_config == NULL) {
        AUD_LOG_W("%s not found for any <library>!! return fail!!", adb_cmd->adb_cmd_key);
        ret = -1;
        goto AURISYS_SET_PARAM_EXIT;
    }


    /* for the lib config */
    switch (adb_cmd->adb_cmd_type) {
    case ADB_CMD_PARAM_FILE:
        strcpy_safe(library_config->param_path, adb_cmd->param_path, MAX_PARAM_PATH_SIZE);
        AUD_LOG_D("lib_name %s, ADB_CMD_PARAM_FILE: %s", library_config->name, library_config->param_path);
        set_adb_done = true;
        ret = 0;
        break;
    case ADB_CMD_LIB_DUMP_FILE:
        strcpy_safe(library_config->lib_dump_path, adb_cmd->lib_dump_path, MAX_DUMP_FILE_PATH_SIZE);
        AUD_LOG_D("lib_name %s, ADB_CMD_LIB_DUMP_FILE: %s", library_config->name, library_config->lib_dump_path);
        set_adb_done = true;
        ret = 0;
        break;
    default:
        set_adb_done = false;
        break;
    }
    if (set_adb_done == true) {
        goto AURISYS_SET_PARAM_EXIT;
    }


    /* find component by aurisys_scenario */
    AUD_ASSERT(library_config->component_hh != NULL);
    if (adb_cmd->aurisys_scenario != scenario_all) {
        HASH_FIND_INT(library_config->component_hh, &adb_cmd->aurisys_scenario, component);
        if (component == NULL) {
            AUD_LOG_W("%s not support aurisys_scenario %u!! return fail!!",
                      library_config->name, adb_cmd->aurisys_scenario);
            ret = -1;
            goto AURISYS_SET_PARAM_EXIT;
        }
        ret = aurisys_set_parameter_to_component(library_config, component, adb_cmd);
    } else {
        for (scenario_idx = 0; scenario_idx < scenario_size; scenario_idx++) {
            HASH_FIND_INT(library_config->component_hh, &scenario_idx, component);
            if (component != NULL) {
                if (aurisys_set_parameter_to_component(library_config, component, adb_cmd) != 0) {
                    ret = -1;
                }
            }
        }
    }



AURISYS_SET_PARAM_EXIT:
    pass = (ret == 0) ? true : false;
    AUD_LOG_V("%s(-), %d, pass: %d", __FUNCTION__, adb_cmd->adb_cmd_type, pass);
    UNLOCK_ALOCK(g_controller->lock);
    return pass;
}


static int aurisys_set_parameter_to_component(
    aurisys_library_config_t *library_config,
    aurisys_component_t *component,
    aurisys_adb_command_t *adb_cmd) {
    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    uint32_t num_handler = 0;
    bool set_adb_done = false;
    int ret = 0;

    num_handler = HASH_CNT(hh_component,
                           component->lib_handler_list_for_adb_cmd);
    AUD_LOG_V("%s(), lib_name %s, scene %u, cmd_type %d, num_handler %u",
              __FUNCTION__,
              library_config->name,
              component->aurisys_scenario,
              adb_cmd->adb_cmd_type,
              num_handler);

    switch (adb_cmd->adb_cmd_type) {
    /* not dynamically open/close pcm dump */
    case ADB_CMD_ENABLE_RAW_DUMP:
        component->enable_raw_dump = adb_cmd->enable_raw_dump;
        AUD_LOG_D("lib_name %s, scene %u, ADB_CMD_ENABLE_RAW_DUMP: %d",
                  library_config->name, component->aurisys_scenario, component->enable_raw_dump);
        set_adb_done = true;
        ret = -1; /* disabled in DSP */
        break;
    case ADB_CMD_ENABLE_LIB_DUMP:
        component->enable_lib_dump = adb_cmd->enable_lib_dump;
        AUD_LOG_D("lib_name %s, scene %u, ADB_CMD_ENABLE_LIB_DUMP: %d",
                  library_config->name, component->aurisys_scenario, component->enable_lib_dump);
        set_adb_done = true;
        ret = 0;
        break;
    /* dynamically apply to all handler later */
    case ADB_CMD_ENABLE_LOG:
        component->enable_log = adb_cmd->enable_log;
        AUD_LOG_D("lib_name %s, scene %u, ADB_CMD_ENABLE_LOG: %d",
                  library_config->name, component->aurisys_scenario, component->enable_log);
        if (num_handler == 0) {
            set_adb_done = true;
            ret = 0;
        } else {
            set_adb_done = false;
        }
        break;
    case ADB_CMD_APPLY_PARAM:
        component->enhancement_mode = adb_cmd->enhancement_mode;
        AUD_LOG_D("lib_name %s, scene %u, ADB_CMD_APPLY_PARAM: %u",
                  library_config->name, component->aurisys_scenario, component->enhancement_mode);
        if (num_handler == 0) {
            set_adb_done = true;
            ret = 0;
        } else {
            set_adb_done = false;
        }
        break;
    /* for single library hanlder */
    case ADB_CMD_ADDR_VALUE:
    case ADB_CMD_KEY_VALUE:
        /* HASH_ITER all library hanlder later */
        if (num_handler == 0) {
            AUD_LOG_W("lib_name %s, scene %u, no active handler!!",
                      library_config->name, component->aurisys_scenario);
            set_adb_done = true;
            ret = 0;
        } else {
            set_adb_done = false;
        }
        break;
    default:
        AUD_LOG_W("%s(), adb_cmd->adb_cmd_type %d fail", __FUNCTION__, adb_cmd->adb_cmd_type);
        set_adb_done = true;
        ret = -1;
        break;
    }
    if (set_adb_done == true) {
        return ret;
    }


    /* all active lib handlers in the same component */
    HASH_ITER(hh_component, component->lib_handler_list_for_adb_cmd, itor_lib_handler, tmp_lib_handler) {
        if (aurisys_arsi_run_adb_cmd(itor_lib_handler, adb_cmd) != 0) {
            AUD_LOG_W("%s() fail!! lib_name %s, scene %u, ret: %d", __FUNCTION__,
                      library_config->name, component->aurisys_scenario, ret);
            ret = -1; // any one set fail => report result fail
        }
    }

    return ret;
}


bool aurisys_get_parameter(
    struct aurisys_adb_command_t *adb_cmd,
    void *wb_addr,
    uint32_t wb_mem_size,
    uint32_t *wb_data_size) {
    aurisys_config_t *aurisys_config = NULL;
    uint32_t scenario_all = 0xFFFFFFFF;
    uint32_t scenario_size = 0;

    aurisys_library_config_t *library_config = NULL;
    aurisys_component_t *component = NULL;

    aurisys_library_config_t *itor_library_config = NULL;
    aurisys_library_config_t *tmp_library_config = NULL;

    aurisys_lib_handler_t *itor_lib_handler = NULL;
    aurisys_lib_handler_t *tmp_lib_handler = NULL;

    uint32_t scenario_idx = 0xFFFFFFFF;

    bool pass = false;
    int ret = 0;

    if (!adb_cmd) {
        return false;
    }
    if (sizeof(struct aurisys_adb_command_t) != wb_mem_size) {
        AUD_LOG_W("%s(), adb cmd struct sz %u > wb sz %u", __FUNCTION__,
                  sizeof(struct aurisys_adb_command_t), wb_mem_size);
    }


    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    AUD_LOG_V("%s(+), %d", __FUNCTION__, adb_cmd->adb_cmd_type);

    aurisys_config = g_controller->aurisys_config;
    scenario_all = AURISYS_SCENARIO_DSP_ALL;
    scenario_size = AURISYS_SCENARIO_DSP_SIZE;


    /* find library_config by adb_cmd_key */
    AUD_ASSERT(aurisys_config != NULL);
    AUD_ASSERT(aurisys_config->library_config_hh != NULL);
    HASH_ITER(hh, aurisys_config->library_config_hh, itor_library_config, tmp_library_config) {
        if (!strcmp(adb_cmd->adb_cmd_key, itor_library_config->adb_cmd_key)) {
            library_config = itor_library_config;
            break;
        }
    }
    if (library_config == NULL) {
        AUD_LOG_W("%s not found for any <library>!! return fail!!", adb_cmd->adb_cmd_key);
        ret = -1;
        goto AURISYS_GET_PARAM_EXIT;
    }


    /* find component by aurisys_scenario */
    AUD_ASSERT(library_config->component_hh != NULL);
    if (adb_cmd->aurisys_scenario != scenario_all) {
        HASH_FIND_INT(library_config->component_hh, &adb_cmd->aurisys_scenario, component);
        if (component == NULL) {
            AUD_LOG_W("%s not support aurisys_scenario %u!! return fail!!",
                      library_config->name, adb_cmd->aurisys_scenario);
            ret = -1;
            goto AURISYS_GET_PARAM_EXIT;
        }
    } else {
        /* get the first matched component for all scenario */
        for (scenario_idx = 0; scenario_idx < scenario_size; scenario_idx++) {
            HASH_FIND_INT(library_config->component_hh, &scenario_idx, component);
            if (component != NULL &&
                HASH_CNT(hh_component, component->lib_handler_list_for_adb_cmd) != 0) {
                AUD_LOG_W("%s get scenario all!! only get aurisys_scenario %u...",
                          library_config->name, adb_cmd->aurisys_scenario);
                break;
            }
        }
    }


    if (adb_cmd->adb_cmd_type != ADB_CMD_ADDR_VALUE &&
        adb_cmd->adb_cmd_type != ADB_CMD_KEY_VALUE) {
        AUD_LOG_W("adb_cmd->adb_cmd_type %u fail!!", adb_cmd->adb_cmd_type);
        ret = -1;
        goto AURISYS_GET_PARAM_EXIT;
    }


    /* get value from handler */
    HASH_ITER(hh_component, component->lib_handler_list_for_adb_cmd, itor_lib_handler, tmp_lib_handler) {
        ret = aurisys_arsi_run_adb_cmd(itor_lib_handler, adb_cmd);
        if (ret == 0) {
            break; /* only to the first lib handler */
        }
    }



AURISYS_GET_PARAM_EXIT:
    pass = (ret == 0) ? true : false;
    if (pass == true) {
        audio_ipi_dma_write(wb_addr, adb_cmd, sizeof(struct aurisys_adb_command_t));
        *wb_data_size = sizeof(struct aurisys_adb_command_t);
    }
    AUD_LOG_V("%s(-), %d, pass: %d", __FUNCTION__, adb_cmd->adb_cmd_type, pass);
    UNLOCK_ALOCK(g_controller->lock);
    return pass;
}


void set_aurisys_on(const bool aurisys_on) {
    AUD_LOG_D("%s(), %d => %d", __func__, g_controller->aurisys_on, aurisys_on);
    g_controller->aurisys_on = aurisys_on;
}


bool get_aurisys_on() {
    return (g_controller) ? g_controller->aurisys_on : false;
}



#ifdef __cplusplus
}  /* extern "C" */
#endif

