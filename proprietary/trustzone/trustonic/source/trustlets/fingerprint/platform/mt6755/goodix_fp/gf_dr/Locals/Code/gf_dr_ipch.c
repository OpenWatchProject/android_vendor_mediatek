#include "drStd.h"
#include "DrApi/DrApi.h"

#include "gf_dr_common.h"

#define LOG_TAG "[gf_dr_ipch] "

static uint32_t g_auth_finger_id = 0;
static uint64_t g_auth_time = 0;
static uint64_t g_session_id = 0;

drApiResult_t gf_dr_invoke_cmd_entry_point(gf_dr_cmd_t *cmd, threadid_t ipcClient) {

    if (NULL == cmd) {
        return E_DRAPI_INVALID_PARAMETER;
    }

    drApiResult_t ret = DRAPI_OK;
    switch (cmd->cmd_id) {
        case GF_DR_CMD_GET_TIMESTAMP: {
            ret = drApiGetSecureTimestamp(&cmd->data.get_timestamp.timestamp);
            break;
        }

        case GF_DR_CMD_SLEEP: {
            ret = drApiThreadSleep(cmd->data.sleep.miliseconds);
            break;
        }

        case GF_DR_CMD_SYNC_AUTH_RESULT: {
            g_auth_finger_id = cmd->data.auth_result.finger_id;
            g_auth_time = cmd->data.auth_result.time;
            break;
        }

        case GF_DR_CMD_ALIPAY_API_GET_AUTH_ID: {
            cmd->data.get_auth_id.finger_id = g_auth_finger_id;
            cmd->data.get_auth_id.time = g_auth_time;
            break;
        }

        case GF_DR_CMD_SHARE_MEMORY_PREFORMANCE: {
            uint64_t begin_time = 0;
            uint64_t end_time = 0;
            drApiGetSecureTimestamp(&begin_time);
            if (cmd->data.share_memory_performance.len > 0) {
                drUtilsMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
                        cmd->data.share_memory_performance.buf,
                        cmd->data.share_memory_performance.len, MAP_READABLE | MAP_WRITABLE);
            }
            drApiGetSecureTimestamp(&end_time);
            cmd->data.share_memory_performance.time = end_time - begin_time;
            break;
        }

        case GF_DR_CMD_WECHAT_API_GET_SESSION_ID: {
            cmd->data.get_session_id.session_id = g_session_id;
            break;
        }

        case GF_DR_CMD_WECHAT_API_SET_SESSION_ID: {
            g_session_id = cmd->data.set_session_id.session_id;
            break;
        }

        case GF_DR_CMD_WECHAT_API_GET_AUTH_ID: {
            cmd->data.wechat_get_auth_id.finger_id = g_auth_finger_id;
            cmd->data.wechat_get_auth_id.time = g_auth_time;
            break;
        }
    }

    return ret;
}
