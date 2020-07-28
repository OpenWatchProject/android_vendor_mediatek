#include "aurisys_adb_command.h"

#include <stdlib.h>

#include <audio_log_hal.h>
#include <audio_assert.h>



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
#define LOG_TAG "[AURI][ADB]"

#ifdef AURISYS_DUMP_LOG_V
#undef  AUD_LOG_V
#define AUD_LOG_V(x...) AUD_LOG_D(x)
#endif



/*
 * =============================================================================
 *                     public function implementation
 * =============================================================================
 */


struct aurisys_adb_command_t *parse_adb_cmd(uint8_t *data_buf, uint32_t data_size) {
    struct aurisys_adb_command_t *adb_cmd = NULL;

    if (!data_buf) {
        AUD_LOG_W("%s(), data_buf %p error!!", __FUNCTION__, data_buf);
        return NULL;
    }
    if (data_size != sizeof(struct aurisys_adb_command_t)) {
        AUD_LOG_W("%s(), data_size %u != %u error!!", __FUNCTION__,
                  data_size, sizeof(struct aurisys_adb_command_t));
        return NULL;
    }

    adb_cmd = (struct aurisys_adb_command_t *)data_buf;
    if (adb_cmd->guard_head != AURISYS_GUARD_HEAD_VALUE ||
        adb_cmd->guard_tail != AURISYS_GUARD_TAIL_VALUE) {
        AUD_LOG_W("%s(), head 0x%x tail 0x%x!!", __FUNCTION__,
                  adb_cmd->guard_head, adb_cmd->guard_tail);
        return NULL;
    }
    if (adb_cmd->target != ADB_CMD_TARGET_DSP ||
        adb_cmd->aurisys_scenario == 0xFFFFFFFF ||
        strlen(adb_cmd->adb_cmd_key) == 0 ||
        adb_cmd->adb_cmd_type == ADB_CMD_INVALID) {
        return NULL;
    }

    return adb_cmd;
}






#ifdef __cplusplus
}  /* extern "C" */
#endif


