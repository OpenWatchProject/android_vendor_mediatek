#include "aurisys_controller.h"

#include <errno.h>
#include <cutils/properties.h>

#include <uthash.h> /* uthash */

#include <audio_log.h>
#include <audio_assert.h>
#include <audio_memory_control.h>
#include <audio_lock.h>

#include <aurisys_scenario.h>

#include <arsi_type.h>
#include <arsi_api.h>
#include <aurisys_config.h>

#include <audio_pool_buf_handler.h>

#include <aurisys_utility.h>
#include <aurisys_config_parser.h>
#include <aurisys_lib_manager.h>
#include <aurisys_lib_handler.h>

#include <aurisys_adb_command.h>

#ifdef MTK_AUDIODSP_SUPPORT
#include <audio_task.h>
#include <audio_messenger_ipi.h>
#include <audio_dsp_service.h>
#include <audio_controller_msg_id.h>
#include <AudioAurisysPcmDump.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aurisys_controller"


#ifdef AURISYS_DUMP_LOG_V
#undef  AUD_LOG_V
#define AUD_LOG_V(x...) AUD_LOG_D(x)
#endif



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define MAX_PARAM_PATH_LEN      (256)
#define MAX_PARAM_PATH_COPY_LEN ((MAX_PARAM_PATH_LEN) - 1) /* -1: reserve for '\0' */

#define AURISYS_CTRL_LOCK_TIMEOUT_MS (2000)

#define AURISYS_PROP_KEY_PLATFORM "ro.vendor.mediatek.platform"
#define AURISYS_PROP_KEY_DEVICE   "ro.product.device"
#define AURISYS_PROP_KEY_MODEL    "ro.product.model"

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

typedef struct aurisys_controller_t {
    struct alock_t *lock;

    aurisys_config_t *aurisys_config;
#ifdef MTK_AUDIODSP_SUPPORT
    aurisys_config_t *aurisys_config_hifi3;
#endif

    aurisys_lib_manager_t *manager_hh;

    int set_raram_result; /* for adb cmd. -1: invalid, 0: fail, 1: pass */

    bool aurisys_on;
} aurisys_controller_t;


/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */

static struct alock_t *g_aurisys_controller_lock;

static aurisys_controller_t *g_controller; /* singleton */

static char gProductInfoBuf[256];
static string_buf_t gProductInfo;

/*
 * =============================================================================
 *                     private function declaration
 * =============================================================================
 */

static int aurisys_set_parameter_to_component(
    aurisys_library_config_t *library_config,
    aurisys_component_t *component,
    aurisys_adb_command_t *adb_cmd);


#ifdef MTK_AUDIODSP_SUPPORT
static int aurisys_run_parameter_to_dsp(struct aurisys_adb_command_t *adb_cmd);
#endif

static void arsi_lib_printf(const char *message, ...) {
    static char printf_msg[256];

    va_list args;
    va_start(args, message);

    vsnprintf(printf_msg, sizeof(printf_msg), message, args);
    AUD_LOG_D("[LIB] %s", printf_msg);

    va_end(args);
}


static void char_to_string(string_buf_t *target, char *source, uint32_t mem_sz) {
    target->memory_size = mem_sz;
    target->string_size = strlen(source);
    target->p_string = source;

    AUD_LOG_VV("memory_size = %u", target->memory_size);
    AUD_LOG_VV("string_size = %u", target->string_size);
    AUD_LOG_VV("p_string = %s",    target->p_string);
}

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

int init_aurisys_controller(void) {
    if (g_aurisys_controller_lock == NULL) {
        NEW_ALOCK(g_aurisys_controller_lock);
    }

    /* create controller */
    LOCK_ALOCK_MS(g_aurisys_controller_lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    if (g_controller != NULL) {
        AUD_LOG_E("%s(), g_controller != NULL!! return", __FUNCTION__);
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

    char platform[PROPERTY_VALUE_MAX] = {0};
    char device[PROPERTY_VALUE_MAX] = {0};
    char model[PROPERTY_VALUE_MAX] = {0};

    property_get(AURISYS_PROP_KEY_PLATFORM, platform, "invalid");
    property_get(AURISYS_PROP_KEY_DEVICE, device, "invalid");
    property_get(AURISYS_PROP_KEY_MODEL, model, "invalid");

    snprintf(gProductInfoBuf, sizeof(gProductInfoBuf),
             "platform=%s,device=%s,model=%s", platform, device, model);

    char_to_string(&gProductInfo, gProductInfoBuf, sizeof(gProductInfoBuf));

    /* parse aurisys_config */
    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    g_controller->aurisys_config = parse_aurisys_config(AURISYS_CORE_HAL);
#ifdef MTK_AUDIODSP_SUPPORT
    g_controller->aurisys_config_hifi3 = parse_aurisys_config(AURISYS_CORE_HIFI3);
    wrap_config_to_dsp();
    init_aurisys_dsp_dump();
#endif

    g_controller->set_raram_result = -1;

    /* enable */
    g_controller->aurisys_on = true;
    UNLOCK_ALOCK(g_controller->lock);

    return 0;
}


int deinit_aurisys_controller(void) {
    AUD_LOG_D("%s()", __FUNCTION__);

    LOCK_ALOCK_MS(g_aurisys_controller_lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    if (g_controller == NULL) {
        AUD_LOG_E("%s(-), g_controller == NULL!! return", __FUNCTION__);
        UNLOCK_ALOCK(g_aurisys_controller_lock);
        return -1;
    }

    /* delete aurisys_config */
    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);

#ifdef MTK_AUDIODSP_SUPPORT
    delete_aurisys_config(g_controller->aurisys_config_hifi3, AURISYS_CORE_HIFI3);
    g_controller->aurisys_config_hifi3 = NULL;
    deinit_aurisys_dsp_dump();
#endif
    delete_aurisys_config(g_controller->aurisys_config, AURISYS_CORE_HAL);
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


int wrap_config_to_dsp(void) {
#ifndef MTK_AUDIODSP_SUPPORT
    return 0;
#else
    struct aurisys_config_dsp_t *aurisys_config_hifi3_wrap = NULL;

    struct ipi_msg_t ipi_msg;
    int ret = 0;

    if (!g_controller) {
        ALOGW("%s(), ctr %p NULL!!", __FUNCTION__, g_controller);
        return -EINVAL;
    }

    if (g_controller->aurisys_config_hifi3 == NULL) {
        AUD_ASSERT(g_controller->aurisys_config_hifi3 != NULL);
        return -ENODEV;
    }

    adsp_register_feature(AURISYS_FEATURE_ID);
    AUDIO_ALLOC_STRUCT(struct aurisys_config_dsp_t, aurisys_config_hifi3_wrap);

    trans_aurisys_config_hal_to_dsp(
        g_controller->aurisys_config_hifi3,
        aurisys_config_hifi3_wrap);

    ret = audio_send_ipi_msg(&ipi_msg,
                             TASK_SCENE_AUDIO_CONTROLLER_HIFI3_A,
                             AUDIO_IPI_LAYER_TO_DSP,
                             AUDIO_IPI_DMA,
                             AUDIO_IPI_MSG_NEED_ACK,
                             AUD_CTL_MSG_A2D_AURISYS_CFG,
                             sizeof(struct aurisys_config_dsp_t),
                             0,
                             aurisys_config_hifi3_wrap);
    ret = audio_send_ipi_msg(&ipi_msg,
                             TASK_SCENE_AUDIO_CONTROLLER_HIFI3_B,
                             AUDIO_IPI_LAYER_TO_DSP,
                             AUDIO_IPI_DMA,
                             AUDIO_IPI_MSG_NEED_ACK,
                             AUD_CTL_MSG_A2D_AURISYS_CFG,
                             sizeof(struct aurisys_config_dsp_t),
                             0,
                             aurisys_config_hifi3_wrap);

    AUDIO_FREE_POINTER(aurisys_config_hifi3_wrap);
    adsp_deregister_feature(AURISYS_FEATURE_ID);
    return 0;
#endif
}


aurisys_lib_manager_t *create_aurisys_lib_manager(
    const struct aurisys_lib_manager_config_t *manager_config) {
    aurisys_lib_manager_t *new_manager = NULL;
    aurisys_config_t *aurisys_config = NULL;

    if (!g_controller) {
        AUD_LOG_E("%s(), aurisys_scenario %d, g_controller NULL!!",
                  __FUNCTION__, manager_config->aurisys_scenario);
        return NULL;
    }

    AUD_LOG_V("%s(), aurisys_scenario = %d", __FUNCTION__,
              manager_config->aurisys_scenario);

    if (manager_config->core_type == AURISYS_CORE_HAL) {
        aurisys_config = g_controller->aurisys_config;
    } else if (manager_config->core_type == AURISYS_CORE_HIFI3) {
#ifdef MTK_AUDIODSP_SUPPORT
        aurisys_config = g_controller->aurisys_config_hifi3;
#else
        aurisys_config = NULL;
        return NULL;
#endif
    } else {
        AUD_LOG_E("%s(), aurisys_scenario %d, core_type %d error!!",
                  __FUNCTION__,
                  manager_config->aurisys_scenario,
                  manager_config->core_type);
        return NULL;
    }

    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    new_manager = new_aurisys_lib_manager(
                      aurisys_config,
                      manager_config);
    HASH_ADD_PTR(g_controller->manager_hh, self, new_manager);
    UNLOCK_ALOCK(g_controller->lock);

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


int aurisys_set_parameter(const char *key_value_pair) {
    aurisys_adb_command_t adb_cmd;

    aurisys_config_t *aurisys_config = NULL;
    uint32_t scenario_all = 0xFFFFFFFF;
    uint32_t scenario_size = 0;

    aurisys_library_config_t *library_config = NULL;
    aurisys_component_t *component = NULL;

    aurisys_library_config_t *itor_library_config = NULL;
    aurisys_library_config_t *tmp_library_config = NULL;

    uint32_t scenario_idx = 0xFFFFFFFF;

    bool set_adb_done = false;
    int ret = 0;


    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
    AUD_LOG_V("%s(+), %s", __FUNCTION__, key_value_pair);

#ifdef MTK_AUDIODSP_SUPPORT
    adsp_register_feature(AURISYS_FEATURE_ID);
#endif

    /* parse adb command */
    memset(&adb_cmd, 0, sizeof(adb_cmd));
    adb_cmd.direction = AURISYS_SET_PARAM;
    ret = parse_adb_cmd(key_value_pair, &adb_cmd);
    if (ret != 0) {
        AUD_LOG_W("%s parsing error!! return fail!!", key_value_pair);
        goto AURISYS_SET_PARAM_EXIT;
    }


    /* wrap hal/dsp config */
    if (adb_cmd.target == ADB_CMD_TARGET_HAL) {
        aurisys_config = g_controller->aurisys_config;
        scenario_all = AURISYS_SCENARIO_ALL;
        scenario_size = AURISYS_SCENARIO_SIZE;
    } else if (adb_cmd.target == ADB_CMD_TARGET_DSP) {
#ifdef MTK_AUDIODSP_SUPPORT
        aurisys_config = g_controller->aurisys_config_hifi3;
        scenario_all = AURISYS_SCENARIO_DSP_ALL;
        scenario_size = AURISYS_SCENARIO_DSP_SIZE;
#else
        ret = -1;
        goto AURISYS_SET_PARAM_EXIT;
#endif
    } else {
        AUD_LOG_W("target not support, cmd %s", key_value_pair);
        ret = -1;
        goto AURISYS_SET_PARAM_EXIT;
    }


    /* find library_config by adb_cmd_key */
    AUD_ASSERT(aurisys_config != NULL);
    AUD_ASSERT(aurisys_config->library_config_hh != NULL);
    HASH_ITER(hh, aurisys_config->library_config_hh, itor_library_config, tmp_library_config) {
        if (!strcmp(adb_cmd.adb_cmd_key, itor_library_config->adb_cmd_key)) {
            library_config = itor_library_config;
            break;
        }
    }
    if (library_config == NULL) {
        AUD_LOG_W("%s not found for any <library>!! return fail!!", adb_cmd.adb_cmd_key);
        ret = -1;
        goto AURISYS_SET_PARAM_EXIT;
    }


    /* for the lib config */
    switch (adb_cmd.adb_cmd_type) {
    case ADB_CMD_PARAM_FILE:
        strcpy_safe(library_config->param_path, adb_cmd.param_path, MAX_PARAM_PATH_SIZE);
        AUD_LOG_D("lib_name %s, ADB_CMD_PARAM_FILE: %s", library_config->name, library_config->param_path);
        if (library_config->api != NULL && library_config->api->arsi_load_param != NULL) {
            string_buf_t param_file_path;
            char_to_string(&param_file_path, library_config->param_path, strlen(library_config->param_path) + 1);
            library_config->api->arsi_load_param(
                &gProductInfo,
                &param_file_path,
                arsi_lib_printf);
        }
        set_adb_done = true;
        ret = 0;
        break;
    case ADB_CMD_LIB_DUMP_FILE:
        strcpy_safe(library_config->lib_dump_path, adb_cmd.lib_dump_path, MAX_DUMP_FILE_PATH_SIZE);
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
    if (adb_cmd.aurisys_scenario != scenario_all) {
        HASH_FIND_INT(library_config->component_hh, &adb_cmd.aurisys_scenario, component);
        if (component == NULL) {
            AUD_LOG_W("%s not support aurisys_scenario %u!! return fail!!",
                      library_config->name, adb_cmd.aurisys_scenario);
            ret = -1;
            goto AURISYS_SET_PARAM_EXIT;
        }
        ret = aurisys_set_parameter_to_component(library_config, component, &adb_cmd);
    } else {
        for (scenario_idx = 0; scenario_idx < scenario_size; scenario_idx++) {
            HASH_FIND_INT(library_config->component_hh, &scenario_idx, component);
            if (component != NULL) {
                if (aurisys_set_parameter_to_component(library_config, component, &adb_cmd) != 0) {
                    ret = -1;
                }
            }
        }
    }



AURISYS_SET_PARAM_EXIT:
#ifdef MTK_AUDIODSP_SUPPORT
    if (ret == 0 && adb_cmd.target == ADB_CMD_TARGET_DSP) {
        ret = aurisys_run_parameter_to_dsp(&adb_cmd);
    }
#endif

    g_controller->set_raram_result = (ret == 0) ? 1 : 0;
    AUD_LOG_V("%s(-), %s, pass: %d", __FUNCTION__, key_value_pair, g_controller->set_raram_result);
#ifdef MTK_AUDIODSP_SUPPORT
    adsp_deregister_feature(AURISYS_FEATURE_ID);
#endif
    UNLOCK_ALOCK(g_controller->lock);
    return g_controller->set_raram_result;
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
        ret = 0;
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
            ret = -1;
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


char *aurisys_get_parameter(const char *key) {
    static char local_get_buf[MAX_PARAM_PATH_LEN]; /* for return string */

    aurisys_adb_command_t adb_cmd;

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

    char *equal = NULL;

    bool get_adb_done = false;
    int ret = 0;


    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);
#ifdef MTK_AUDIODSP_SUPPORT
    adsp_register_feature(AURISYS_FEATURE_ID);
#endif

    AUD_LOG_V("%s(+), %s", __FUNCTION__, key);

    /* parse adb command */
    memset(&adb_cmd, 0, sizeof(adb_cmd));
    adb_cmd.direction = AURISYS_GET_PARAM;
    ret = parse_adb_cmd(key, &adb_cmd);
    if (ret != 0) {
        AUD_LOG_W("%s parsing error!! return fail!!", key);
        goto AURISYS_GET_PARAM_EXIT;
    }


    /* wrap hal/dsp config */
    if (adb_cmd.target == ADB_CMD_TARGET_HAL) {
        aurisys_config = g_controller->aurisys_config;
        scenario_all = AURISYS_SCENARIO_ALL;
        scenario_size = AURISYS_SCENARIO_SIZE;
    } else if (adb_cmd.target == ADB_CMD_TARGET_DSP) {
#ifdef MTK_AUDIODSP_SUPPORT
        aurisys_config = g_controller->aurisys_config_hifi3;
        scenario_all = AURISYS_SCENARIO_DSP_ALL;
        scenario_size = AURISYS_SCENARIO_DSP_SIZE;
#else
        ret = -1;
        goto AURISYS_GET_PARAM_EXIT;
#endif
    } else {
        AUD_LOG_W("target not support, cmd %s", key);
        ret = -1;
        goto AURISYS_GET_PARAM_EXIT;
    }


    /* find library_config by adb_cmd_key */
    AUD_ASSERT(aurisys_config != NULL);
    AUD_ASSERT(aurisys_config->library_config_hh != NULL);
    HASH_ITER(hh, aurisys_config->library_config_hh, itor_library_config, tmp_library_config) {
        if (!strcmp(adb_cmd.adb_cmd_key, itor_library_config->adb_cmd_key)) {
            library_config = itor_library_config;
            break;
        }
    }
    if (library_config == NULL) {
        AUD_LOG_W("%s not found for any <library>!! return fail!!", adb_cmd.adb_cmd_key);
        ret = -1;
        goto AURISYS_GET_PARAM_EXIT;
    }


    /* find component by aurisys_scenario */
    AUD_ASSERT(library_config->component_hh != NULL);
    if (adb_cmd.aurisys_scenario != scenario_all) {
        HASH_FIND_INT(library_config->component_hh, &adb_cmd.aurisys_scenario, component);
        if (component == NULL) {
            AUD_LOG_W("%s not support aurisys_scenario %u!! return fail!!",
                      library_config->name, adb_cmd.aurisys_scenario);
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
                          library_config->name, adb_cmd.aurisys_scenario);
                break;
            }
        }
    }


    memset(&local_get_buf, 0, MAX_PARAM_PATH_LEN);

    switch (adb_cmd.adb_cmd_type) {
    /* for the lib config */
    case ADB_CMD_PARAM_FILE:
        snprintf(local_get_buf, MAX_PARAM_PATH_LEN, "%s", library_config->param_path);
        get_adb_done = true;
        ret = 0;
        break;
    case ADB_CMD_LIB_DUMP_FILE:
        snprintf(local_get_buf, MAX_PARAM_PATH_LEN, "%s", library_config->lib_dump_path);
        get_adb_done = true;
        ret = 0;
        break;
    /* for the component */
    case ADB_CMD_ENABLE_LOG:
        snprintf(local_get_buf, MAX_PARAM_PATH_LEN, "%d", component->enable_log);
        get_adb_done = true;
        ret = 0;
        break;
    case ADB_CMD_ENABLE_RAW_DUMP:
        snprintf(local_get_buf, MAX_PARAM_PATH_LEN, "%d", component->enable_raw_dump);
        get_adb_done = true;
        ret = 0;
        break;
    case ADB_CMD_ENABLE_LIB_DUMP:
        snprintf(local_get_buf, MAX_PARAM_PATH_LEN, "%d", component->enable_lib_dump);
        get_adb_done = true;
        ret = 0;
        break;
    case ADB_CMD_APPLY_PARAM:
        snprintf(local_get_buf, MAX_PARAM_PATH_LEN, "%d", component->enhancement_mode);
        get_adb_done = true;
        ret = 0;
        break;
    /* for single library hanlder */
    case ADB_CMD_ADDR_VALUE:
    case ADB_CMD_KEY_VALUE:
        get_adb_done = false;
        break;
    default:
        AUD_LOG_W("%s(), adb_cmd->adb_cmd_type %d fail", __FUNCTION__, adb_cmd.adb_cmd_type);
        get_adb_done = true;
        ret = -1;
        break;
    }
    if (get_adb_done == true) {
        goto AURISYS_GET_PARAM_EXIT;
    }


    /* get value from handler */
    HASH_ITER(hh_component, component->lib_handler_list_for_adb_cmd, itor_lib_handler, tmp_lib_handler) {
        if (adb_cmd.target == ADB_CMD_TARGET_HAL) {
            ret = aurisys_arsi_run_adb_cmd(itor_lib_handler, &adb_cmd);
        } else if (adb_cmd.target == ADB_CMD_TARGET_DSP) {
#ifdef MTK_AUDIODSP_SUPPORT
            ret = aurisys_run_parameter_to_dsp(&adb_cmd);
#else
            ret = -1;
#endif
        }
        if (ret == 0) {
            break; /* only to the first lib handler */
        }
    }
    if (ret != 0) {
        goto AURISYS_GET_PARAM_EXIT;
    }


    /* print value to string */
    if (adb_cmd.adb_cmd_type == ADB_CMD_ADDR_VALUE) {
        snprintf(local_get_buf, MAX_PARAM_PATH_LEN, "0x%x", adb_cmd.addr_value_pair.value);
        ret = 0;
    } else if (adb_cmd.adb_cmd_type == ADB_CMD_KEY_VALUE) {
        equal = strstr(adb_cmd.key_value, "=");
        if (equal == NULL) {
            AUD_LOG_W("%s(), adb_cmd.key_value %s error!!", __FUNCTION__, adb_cmd.key_value);
            ret = -1;
        } else {
            snprintf(local_get_buf, MAX_PARAM_PATH_LEN, "%s", equal + 1); /* +1: skip '=' */
            ret = 0;
        }
    }



AURISYS_GET_PARAM_EXIT:
    if (ret != 0) {
        snprintf(local_get_buf, MAX_PARAM_PATH_LEN, "GET_FAIL");
    }
    AUD_LOG_V("%s(-), %s = %s", __FUNCTION__, key, local_get_buf);
#ifdef MTK_AUDIODSP_SUPPORT
    adsp_deregister_feature(AURISYS_FEATURE_ID);
#endif
    UNLOCK_ALOCK(g_controller->lock);
    return local_get_buf;
}


int get_aurisys_set_raram_result(void) {
    int ret = 0;

    LOCK_ALOCK_MS(g_controller->lock, AURISYS_CTRL_LOCK_TIMEOUT_MS);

    AUD_ASSERT(g_controller->set_raram_result != -1);
    ret = g_controller->set_raram_result;
    g_controller->set_raram_result = -1;

    UNLOCK_ALOCK(g_controller->lock);

    AUD_LOG_V("%s(), %d", __func__, ret);
    return ret;
}



#ifdef MTK_AUDIODSP_SUPPORT
static int aurisys_run_parameter_to_dsp(struct aurisys_adb_command_t *adb_cmd) {
    struct ipi_msg_t ipi_msg;
    uint16_t msg_id = 0;
    uint32_t wb_size = 0;

    int ret = 0;

    if (adb_cmd->direction == AURISYS_SET_PARAM) {
        msg_id = AUD_CTL_MSG_A2D_AURISYS_SET_PARAM;
        wb_size = 0;
    } else if (adb_cmd->direction == AURISYS_GET_PARAM) {
        msg_id = AUD_CTL_MSG_A2D_AURISYS_GET_PARAM;
        wb_size = sizeof(struct aurisys_adb_command_t);
    }

    ret = audio_send_ipi_msg(&ipi_msg,
                             TASK_SCENE_AUDIO_CONTROLLER_HIFI3_A,
                             AUDIO_IPI_LAYER_TO_DSP,
                             AUDIO_IPI_DMA,
                             AUDIO_IPI_MSG_NEED_ACK,
                             msg_id,
                             sizeof(struct aurisys_adb_command_t),
                             wb_size,
                             adb_cmd);
    if (ret != 0) {
        AUD_LOG_W("%s(), adb_cmd->adb_cmd_type %d fail", __FUNCTION__,
                  adb_cmd->adb_cmd_type);
    } else {
        ret = (ipi_msg.scp_ret == 1) ? 0 : -1;
    }

    return ret;
}
#endif


void set_aurisys_on(const bool aurisys_on) {
#ifdef MTK_AUDIODSP_SUPPORT
    struct ipi_msg_t ipi_msg;
#endif

    AUD_LOG_D("%s(), %d => %d", __func__, g_controller->aurisys_on, aurisys_on);
    g_controller->aurisys_on = aurisys_on;

#ifdef MTK_AUDIODSP_SUPPORT
    adsp_register_feature(AURISYS_FEATURE_ID);
    audio_send_ipi_msg(&ipi_msg,
                       TASK_SCENE_AUDIO_CONTROLLER_HIFI3_A,
                       AUDIO_IPI_LAYER_TO_DSP,
                       AUDIO_IPI_MSG_ONLY,
                       AUDIO_IPI_MSG_BYPASS_ACK,
                       AUD_CTL_MSG_A2D_AURISYS_ENABLE,
                       aurisys_on,
                       0,
                       NULL);
    audio_send_ipi_msg(&ipi_msg,
                       TASK_SCENE_AUDIO_CONTROLLER_HIFI3_B,
                       AUDIO_IPI_LAYER_TO_DSP,
                       AUDIO_IPI_MSG_ONLY,
                       AUDIO_IPI_MSG_BYPASS_ACK,
                       AUD_CTL_MSG_A2D_AURISYS_ENABLE,
                       aurisys_on,
                       0,
                       NULL);
    adsp_deregister_feature(AURISYS_FEATURE_ID);
#endif
}


bool get_aurisys_on() {
    return (g_controller) ? g_controller->aurisys_on : false;
}



#ifdef __cplusplus
}  /* extern "C" */
#endif

