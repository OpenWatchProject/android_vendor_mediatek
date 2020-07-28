/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#include "drStd.h"
#include "DrApi/DrApi.h"

#include "gf_dr_common.h"
#include "gf_dr_ipch.h"

#define LOG_TAG "[gf_dr_ipch] "

static uint32_t g_auth_finger_id = 0;
static uint64_t g_auth_time = 0;
static uint64_t g_session_id = 0;

static uint32_t m_finger_list[MAX_FINGER_COUNT];
static uint32_t m_current_finger_count = 0;
static uint32_t m_authenticator_version = 0;

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
            uint64_t sleep_time_in_us = cmd->data.sleep.miliseconds * 1000;
            uint64_t start_time_in_us = 0;
            uint64_t cur_time_in_us = 0;

            GF_LOGD(LOG_TAG "[%s] sleep (%d) ms start\n", __func__,
                    cmd->data.sleep.miliseconds);

            ret = drApiGetSecureTimestamp(&start_time_in_us);
            cur_time_in_us = start_time_in_us;

            for (; (cur_time_in_us - start_time_in_us) < sleep_time_in_us;) {
                cur_time_in_us = 0;
                ret = drApiGetSecureTimestamp(&cur_time_in_us);
            }

            GF_LOGD(LOG_TAG "[%s] sleep(%d)ms, real sleep(%d)ms\n", __func__,
                    (uint32_t) cmd->data.sleep.miliseconds,
                    (uint32_t) ((cur_time_in_us - start_time_in_us) / 1000));
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

        case GF_DR_CMD_SYNC_FINGER_LIST: {
            if (cmd->data.sync_finger_list.ids != NULL) {
                uint32_t min_size =
                        cmd->data.sync_finger_list.count > MAX_FINGER_COUNT ?
                                MAX_FINGER_COUNT : cmd->data.sync_finger_list.count;
                memset(m_finger_list, 0, sizeof(m_finger_list));
                memcpy(m_finger_list, cmd->data.sync_finger_list.ids, min_size * sizeof(uint32_t));
                m_current_finger_count = min_size;
            }

            break;
        }

        case GF_DR_CMD_SYNC_AUTHENTICATOR_VERSION: {
            m_authenticator_version = cmd->data.sync_authenticator_version.version;
            break;
        }

        case GF_DR_CMD_IFAA_GET_FINGER_LIST: {
            if ((cmd->data.get_finger_list.ids != NULL) && (m_current_finger_count > 0)) {
                memcpy(cmd->data.get_finger_list.ids, m_finger_list,
                        m_current_finger_count * sizeof(uint32_t));
                cmd->data.get_finger_list.count = m_current_finger_count;
            }
            break;
        }
        case GF_DR_CMD_IFAA_GET_AUTHENTICATOR_VERSION: {
            cmd->data.get_authenticator_version.version = m_authenticator_version;
            break;
        }
    }

    return ret;
}
