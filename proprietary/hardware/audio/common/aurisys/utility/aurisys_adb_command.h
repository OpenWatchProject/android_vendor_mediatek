#ifndef MTK_AURISYS_ADB_COMMAND_H
#define MTK_AURISYS_ADB_COMMAND_H

#include <stdint.h>
#include <stdbool.h>

#include <arsi_type.h>
#include <aurisys_config.h>



#ifdef __cplusplus
extern "C" {
#endif



/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define MAX_ADB_CMD_TYPE_DATA_SIZE (512)
#define MAX_ADB_CMD_KEY_VALUE_SIZE (512)


/*
 * =============================================================================
 *                     enum
 * =============================================================================
 */

enum {
    AURISYS_DIRECTION_INVALID,
    AURISYS_SET_PARAM,
    AURISYS_GET_PARAM
};


enum {
    ADB_CMD_TARGET_HAL,
    ADB_CMD_TARGET_DSP,
    ADB_CMD_TARGET_INVALID
};


enum { /* adb_cmd_type_t */
    /* for all same lib */
    ADB_CMD_PARAM_FILE,
    ADB_CMD_LIB_DUMP_FILE,

    /* for all same component */
    ADB_CMD_ENABLE_LOG,
    ADB_CMD_ENABLE_RAW_DUMP,
    ADB_CMD_ENABLE_LIB_DUMP,
    ADB_CMD_APPLY_PARAM,

    /* for single library hanlder */
    ADB_CMD_ADDR_VALUE,
    ADB_CMD_KEY_VALUE,

    ADB_CMD_SIZE,
    ADB_CMD_INVALID
};


struct addr_value_pair_t {
    uint32_t addr;
    uint32_t value;
};


/*
 * =============================================================================
 *                     struct definition
 * =============================================================================
 */

typedef struct aurisys_adb_command_t {
    uint32_t guard_head;
    uint32_t reserved1;

    uint8_t direction;
    uint8_t target;
    uint8_t adb_cmd_type;
    uint8_t reserved2;

    uint32_t aurisys_scenario;

    char adb_cmd_key[MAX_ADB_CMD_KEY_SIZE];

    /* the data only corresponds to one adb_cmd_type_t => union */
    union {
        /* dummy for reserve buf */
        uint8_t dummy_cmd_type[MAX_ADB_CMD_TYPE_DATA_SIZE];

        /* ADB_CMD_PARAM_FILE */
        char param_path[MAX_PARAM_PATH_SIZE];

        /* ADB_CMD_LIB_DUMP_FILE */
        char lib_dump_path[MAX_DUMP_FILE_PATH_SIZE];

        /* ADB_CMD_ENABLE_LOG */
        bool enable_log;

        /* ADB_CMD_ENABLE_RAW_DUMP */
        bool enable_raw_dump;

        /* ADB_CMD_ENABLE_LIB_DUMP */
        bool enable_lib_dump;

        /* ADB_CMD_APPLY_PARAM (nothing, or enhancement_mode) */
        uint32_t enhancement_mode;

        /* ADB_CMD_ADDR_VALUE */
        struct addr_value_pair_t addr_value_pair;

        /* ADB_CMD_KEY_VALUE */
        char key_value[MAX_ADB_CMD_KEY_VALUE_SIZE];
    };

    uint32_t reserved3;
    uint32_t guard_tail;
} aurisys_adb_command_t;



/*
 * =============================================================================
 *                     public function
 * =============================================================================
 */

int parse_adb_cmd(const char *key_value_pair, aurisys_adb_command_t *adb_cmd);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of MTK_AURISYS_ADB_COMMAND_H */

