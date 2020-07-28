#include "aurisys_adb_command.h"

#include <stdlib.h>
#include <string.h>

#include <audio_log.h>
#include <audio_assert.h>

#include <aurisys_scenario.h>
#include <aurisys_config.h>
#include <aurisys_utility.h>


#ifdef __cplusplus
extern "C" {
#endif




/*
 * =============================================================================
 *                     log
 * =============================================================================
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aurisys_adb_command"

#ifdef AURISYS_DUMP_LOG_V
#undef  AUD_LOG_V
#define AUD_LOG_V(x...) AUD_LOG_D(x)
#endif



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define ADB_CMD_STR_PARAM_FILE       "PARAM_FILE"
#define ADB_CMD_STR_LIB_DUMP_FILE    "LIB_DUMP_FILE"

#define ADB_CMD_STR_ENABLE_LOG       "ENABLE_LOG"
#define ADB_CMD_STR_ENABLE_RAW_DUMP  "ENABLE_RAW_DUMP"
#define ADB_CMD_STR_ENABLE_LIB_DUMP  "ENABLE_LIB_DUMP"

#define ADB_CMD_STR_APPLY_PARAM      "APPLY_PARAM"
#define ADB_CMD_STR_ADDR_VALUE       "ADDR_VALUE"
#define ADB_CMD_STR_KEY_VALUE        "KEY_VALUE"


#define MAX_ADB_CMD_LEN      (512)
#define MAX_ADB_CMD_COPY_LEN ((MAX_ADB_CMD_LEN) - 1) /* -1: reserve for '\0' */

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */


/*
 * =============================================================================
 *                     private global members
 * =============================================================================
 */



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

static void parse_adb_cmd_target(char **current, aurisys_adb_command_t *adb_cmd);
static void parse_adb_cmd_aurisys_scenario(char **current, aurisys_adb_command_t *adb_cmd);
static void parse_adb_cmd_key(char **current, aurisys_adb_command_t *adb_cmd);
static void parse_adb_cmd_type_and_data(char **current, aurisys_adb_command_t *adb_cmd);



/*
 * =============================================================================
 *                     public function implementation
 * =============================================================================
 */

int parse_adb_cmd(const char *key_value_pair, aurisys_adb_command_t *adb_cmd) {
    char key_value_pair_copy[MAX_ADB_CMD_LEN];
    char *current = NULL;

    AUD_ASSERT(key_value_pair != NULL);
    AUD_ASSERT(adb_cmd != NULL);

    /* copy key_value_pair to key_value_pair_copy */
    if (strlen(key_value_pair) > MAX_ADB_CMD_COPY_LEN) {
        AUD_LOG_W("strlen(%u) > MAX_ADB_CMD_COPY_LEN %d !!",
                  (uint32_t)strlen(key_value_pair), MAX_ADB_CMD_COPY_LEN);
        return -1;
    }
    memset(key_value_pair_copy, 0, sizeof(key_value_pair_copy));
    strncpy(key_value_pair_copy, key_value_pair, MAX_ADB_CMD_COPY_LEN);
    current = key_value_pair_copy;


    adb_cmd->guard_head = AURISYS_GUARD_HEAD_VALUE;

    /* get target */
    parse_adb_cmd_target(&current, adb_cmd);
    if (adb_cmd->target == ADB_CMD_TARGET_INVALID) {
        return -1;
    }

    /* get aurisys_scenario */
    parse_adb_cmd_aurisys_scenario(&current, adb_cmd);
    if (adb_cmd->aurisys_scenario == 0xFFFFFFFF) {
        return -1;
    }

    /* get adb_cmd_key */
    parse_adb_cmd_key(&current, adb_cmd);
    if (strlen(adb_cmd->adb_cmd_key) == 0) {
        return -1;
    }

    /* get adb_cmd type & data */
    parse_adb_cmd_type_and_data(&current, adb_cmd);
    if (adb_cmd->adb_cmd_type == ADB_CMD_INVALID) {
        return -1;
    }

    adb_cmd->guard_tail = AURISYS_GUARD_TAIL_VALUE;

    /* final check */
    if (adb_cmd->target == ADB_CMD_TARGET_INVALID ||
        adb_cmd->aurisys_scenario == 0xFFFFFFFF ||
        strlen(adb_cmd->adb_cmd_key) == 0 ||
        adb_cmd->adb_cmd_type == ADB_CMD_INVALID) {
        AUD_LOG_W("adb cmd parsing %s fail!!", key_value_pair);
        return -1;
    }

    return 0;
}


/*
 * =============================================================================
 *                     utilities implementation
 * =============================================================================
 */

static void parse_adb_cmd_target(char **current, aurisys_adb_command_t *adb_cmd) {
    const char target_dsp[] = "DSP";
    const char target_hal[] = "HAL";

    char *target = NULL;
    char *end = NULL;


    adb_cmd->target = ADB_CMD_TARGET_INVALID;

    end = strstr(*current, ",");
    if (end == NULL) {
        AUD_LOG_W("%s() fail", __FUNCTION__);
        return;
    }
    *end = '\0';
    target = *current;
    *current = end + 1;

    if (strncmp(target, target_hal, strlen(target)) == 0) {
        adb_cmd->target = ADB_CMD_TARGET_HAL;
    } else if (strncmp(target, target_dsp, strlen(target)) == 0) {
        adb_cmd->target = ADB_CMD_TARGET_DSP;
    } else {
        AUD_LOG_W("target: %s invalid!!", target);
        adb_cmd->target = ADB_CMD_TARGET_INVALID;
    }

    AUD_LOG_V("target: %d", adb_cmd->target);
}


static void parse_adb_cmd_aurisys_scenario(char **current, aurisys_adb_command_t *adb_cmd) {
    char aurisys_scenario_str[MAX_ADB_CMD_LEN];
    const char *scenario_prefix = NULL;
    uint8_t core_type = 0;

    char *scenario = NULL;
    char *scenario_suffix = NULL;
    char *end   = NULL;


    adb_cmd->aurisys_scenario = 0xFFFFFFFF;

    if (adb_cmd->target == ADB_CMD_TARGET_HAL) {
        scenario_prefix = "AURISYS_SCENARIO_";
        core_type = AURISYS_CORE_HAL;
    } else if (adb_cmd->target == ADB_CMD_TARGET_DSP) {
        scenario_prefix = "AURISYS_SCENARIO_DSP_";
        core_type = AURISYS_CORE_HIFI3;
    } else {
        return;
    }


    end = strstr(*current, ",");
    if (end == NULL) {
        AUD_LOG_W("%s() fail", __FUNCTION__);
        return;
    }
    *end = '\0';
    scenario = *current;
    scenario_suffix = *current;
    *current = end + 1;


    if (!strncmp(scenario_prefix, scenario, strlen(scenario_prefix))) {
        AUD_LOG_V("scenario: %s", scenario);
        strncpy(aurisys_scenario_str, scenario, MAX_ADB_CMD_COPY_LEN);
    } else {
        AUD_LOG_V("scenario_suffix: %s", scenario_suffix);
        snprintf(aurisys_scenario_str, MAX_ADB_CMD_COPY_LEN, "%s%s", scenario_prefix, scenario_suffix);
    }

    AUD_LOG_V("aurisys_scenario_str: %s", aurisys_scenario_str);
    adb_cmd->aurisys_scenario = get_enum_by_string_aurisys_scenario(core_type, aurisys_scenario_str);
    AUD_LOG_V("aurisys_scenario: %u", adb_cmd->aurisys_scenario);
}


static void parse_adb_cmd_key(char **current, aurisys_adb_command_t *adb_cmd) {
    char *adb_cmd_key = NULL;

    char *end   = NULL;

    end = strstr(*current, ",");
    if (end == NULL) {
        AUD_LOG_W("%s() fail", __FUNCTION__);
        return;
    }
    *end = '\0';
    adb_cmd_key = *current;
    *current = end + 1;

    memset(adb_cmd->adb_cmd_key, 0, MAX_ADB_CMD_KEY_SIZE);
    strcpy_safe(adb_cmd->adb_cmd_key, adb_cmd_key, MAX_ADB_CMD_KEY_SIZE);
    AUD_LOG_V("adb_cmd_key: %s", adb_cmd->adb_cmd_key);
}


static void parse_adb_cmd_type_and_data(char **current, aurisys_adb_command_t *adb_cmd) {
    char *adb_cmd_str = NULL;
    char *end = NULL;
    char *data = NULL;
    char *comma = NULL;


    adb_cmd->adb_cmd_type = ADB_CMD_INVALID;

    if (adb_cmd->direction != AURISYS_SET_PARAM &&
        adb_cmd->direction != AURISYS_GET_PARAM) {
        AUD_LOG_W("%s() direction %d error!!", __FUNCTION__, adb_cmd->direction);
        return;
    }


    if (adb_cmd->direction == AURISYS_SET_PARAM) {
        end = strstr(*current, "=SET");
        if (end == NULL) {
            AUD_LOG_W("%s() fail", __FUNCTION__);
            return;
        }
        *end = '\0';
    }
    adb_cmd_str = *current;


    AUD_LOG_V("%s(+) %s", __FUNCTION__, adb_cmd_str);

    if (!strncmp(adb_cmd_str, ADB_CMD_STR_PARAM_FILE, strlen(ADB_CMD_STR_PARAM_FILE))) {
        if (adb_cmd->direction == AURISYS_SET_PARAM) {
            data = adb_cmd_str + strlen(ADB_CMD_STR_PARAM_FILE) + 1; /* +1: skip ',' */
            AUD_ASSERT(strlen(data) != 0);
            strcpy_safe(adb_cmd->param_path, data, MAX_PARAM_PATH_SIZE);
            adb_cmd->adb_cmd_type = ADB_CMD_PARAM_FILE;
            AUD_LOG_V("%s: %s", ADB_CMD_STR_PARAM_FILE, adb_cmd->param_path);
        } else if (adb_cmd->direction == AURISYS_GET_PARAM) {
            adb_cmd->adb_cmd_type = ADB_CMD_PARAM_FILE;
        }
    } else if (!strncmp(adb_cmd_str, ADB_CMD_STR_LIB_DUMP_FILE, strlen(ADB_CMD_STR_LIB_DUMP_FILE))) {
        if (adb_cmd->direction == AURISYS_SET_PARAM) {
            data = adb_cmd_str + strlen(ADB_CMD_STR_LIB_DUMP_FILE) + 1; /* +1: skip ',' */
            AUD_ASSERT(strlen(data) != 0);
            strcpy_safe(adb_cmd->lib_dump_path, data, MAX_DUMP_FILE_PATH_SIZE);
            adb_cmd->adb_cmd_type = ADB_CMD_LIB_DUMP_FILE;
            AUD_LOG_V("%s: %s", ADB_CMD_STR_LIB_DUMP_FILE, adb_cmd->lib_dump_path);
        } else if (adb_cmd->direction == AURISYS_GET_PARAM) {
            adb_cmd->adb_cmd_type = ADB_CMD_LIB_DUMP_FILE;
        }
    } else if (!strncmp(adb_cmd_str, ADB_CMD_STR_ENABLE_LOG, strlen(ADB_CMD_STR_ENABLE_LOG))) {
        if (adb_cmd->direction == AURISYS_SET_PARAM) {
            data = adb_cmd_str + strlen(ADB_CMD_STR_ENABLE_LOG) + 1; /* +1: skip ',' */
            AUD_ASSERT(strlen(data) != 0);
            adb_cmd->enable_log = (*data == '0') ? 0 : 1;
            adb_cmd->adb_cmd_type = ADB_CMD_ENABLE_LOG;
            AUD_LOG_V("%s: %d", ADB_CMD_STR_ENABLE_LOG, adb_cmd->enable_log);
        } else if (adb_cmd->direction == AURISYS_GET_PARAM) {
            adb_cmd->adb_cmd_type = ADB_CMD_ENABLE_LOG;
        }
    } else if (!strncmp(adb_cmd_str, ADB_CMD_STR_ENABLE_RAW_DUMP, strlen(ADB_CMD_STR_ENABLE_RAW_DUMP))) {
        if (adb_cmd->direction == AURISYS_SET_PARAM) {
            data = adb_cmd_str + strlen(ADB_CMD_STR_ENABLE_RAW_DUMP) + 1; /* +1: skip ',' */
            AUD_ASSERT(strlen(data) != 0);
            adb_cmd->enable_raw_dump = (*data == '0') ? 0 : 1;
            adb_cmd->adb_cmd_type = ADB_CMD_ENABLE_RAW_DUMP;
            AUD_LOG_V("%s: %d", ADB_CMD_STR_ENABLE_RAW_DUMP, adb_cmd->enable_raw_dump);
        } else if (adb_cmd->direction == AURISYS_GET_PARAM) {
            adb_cmd->adb_cmd_type = ADB_CMD_ENABLE_RAW_DUMP;
        }
    } else if (!strncmp(adb_cmd_str, ADB_CMD_STR_ENABLE_LIB_DUMP, strlen(ADB_CMD_STR_ENABLE_LIB_DUMP))) {
        if (adb_cmd->direction == AURISYS_SET_PARAM) {
            data = adb_cmd_str + strlen(ADB_CMD_STR_ENABLE_LIB_DUMP) + 1; /* +1: skip ',' */
            AUD_ASSERT(strlen(data) != 0);
            adb_cmd->enable_lib_dump = (*data == '0') ? 0 : 1;
            adb_cmd->adb_cmd_type = ADB_CMD_ENABLE_LIB_DUMP;
            AUD_LOG_V("%s: %d", ADB_CMD_STR_ENABLE_LIB_DUMP, adb_cmd->enable_lib_dump);
        } else if (adb_cmd->direction == AURISYS_GET_PARAM) {
            adb_cmd->adb_cmd_type = ADB_CMD_ENABLE_LIB_DUMP;
        }
    } else if (!strncmp(adb_cmd_str, ADB_CMD_STR_APPLY_PARAM, strlen(ADB_CMD_STR_APPLY_PARAM))) {
        if (adb_cmd->direction == AURISYS_SET_PARAM) {
            data = adb_cmd_str + strlen(ADB_CMD_STR_APPLY_PARAM) + 1; /* +1: skip ',' */
            AUD_ASSERT(strlen(data) != 0);
            adb_cmd->enhancement_mode = atol(data);
            adb_cmd->adb_cmd_type = ADB_CMD_APPLY_PARAM;
            AUD_LOG_V("%s: %u", ADB_CMD_STR_APPLY_PARAM, adb_cmd->enhancement_mode);
        } else if (adb_cmd->direction == AURISYS_GET_PARAM) {
            adb_cmd->adb_cmd_type = ADB_CMD_APPLY_PARAM;
        }
    } else if (!strncmp(adb_cmd_str, ADB_CMD_STR_ADDR_VALUE, strlen(ADB_CMD_STR_ADDR_VALUE))) {
        if (adb_cmd->direction == AURISYS_SET_PARAM) {
            data = adb_cmd_str + strlen(ADB_CMD_STR_ADDR_VALUE) + 1; /* +1: skip ',' */
            sscanf(data, "%x,%x", &adb_cmd->addr_value_pair.addr, &adb_cmd->addr_value_pair.value);
            adb_cmd->adb_cmd_type = ADB_CMD_ADDR_VALUE;
            AUD_LOG_V("%s: *0x%x = 0x%x", ADB_CMD_STR_ADDR_VALUE,
                      adb_cmd->addr_value_pair.addr, adb_cmd->addr_value_pair.value);
        } else if (adb_cmd->direction == AURISYS_GET_PARAM) {
            data = adb_cmd_str + strlen(ADB_CMD_STR_ADDR_VALUE) + 1; /* +1: skip ',' */
            sscanf(data, "%x", &adb_cmd->addr_value_pair.addr);
            adb_cmd->adb_cmd_type = ADB_CMD_ADDR_VALUE;
            AUD_LOG_V("%s: 0x%x", ADB_CMD_STR_ADDR_VALUE, adb_cmd->addr_value_pair.addr);
        }
    } else if (!strncmp(adb_cmd_str, ADB_CMD_STR_KEY_VALUE, strlen(ADB_CMD_STR_KEY_VALUE))) {
        data = adb_cmd_str + strlen(ADB_CMD_STR_KEY_VALUE) + 1; /* +1: skip ',' */
        if (adb_cmd->direction == AURISYS_SET_PARAM) {
            comma = strstr(data, ",");
            AUD_ASSERT(comma != NULL);
            *comma = '=';
        }
        strcpy_safe(adb_cmd->key_value, data, MAX_ADB_CMD_KEY_VALUE_SIZE);
        adb_cmd->adb_cmd_type = ADB_CMD_KEY_VALUE;
        AUD_LOG_V("%s: %s, %u", ADB_CMD_STR_KEY_VALUE,
                  adb_cmd->key_value, (uint32_t)strlen(adb_cmd->key_value));
    } else {
        AUD_LOG_W("%s not support!!", adb_cmd_str);
    }

    AUD_LOG_V("%s(-) %s, adb_cmd_type %d", __FUNCTION__, adb_cmd_str, adb_cmd->adb_cmd_type);
}



#ifdef __cplusplus
}  /* extern "C" */
#endif


